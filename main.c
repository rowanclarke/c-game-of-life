
#include <windows.h>
#include <time.h> 
#include <GL/gl.h>

LRESULT CALLBACK WndProc (HWND hWnd, UINT message,
WPARAM wParam, LPARAM lParam);
void EnableOpenGL (HWND hWnd, HDC *hDC, HGLRC *hRC);
void DisableOpenGL (HWND hWnd, HDC hDC, HGLRC hRC);
const int W = 399;
const int H = 399;
const int RND = 3;

int WINAPI WinMain (HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int iCmdShow) {
    WNDCLASS wc;
    HWND hWnd;
    HDC hDC;
    HGLRC hRC;        
    MSG msg;
    BOOL bQuit = FALSE;
    BOOL* screen;
    BOOL* buffer;
    float theta = 0.0f;
    screen = calloc(sizeof(BOOL)*W*H, 1);
    buffer = calloc(sizeof(BOOL)*W*H, 1);
    
    wc.style = CS_OWNDC;
    wc.lpfnWndProc = WndProc;
    wc.cbClsExtra = 0;
    wc.cbWndExtra = 0;
    wc.hInstance = hInstance;
    wc.hIcon = LoadIcon (NULL, IDI_APPLICATION);
    wc.hCursor = LoadCursor (NULL, IDC_ARROW);
    wc.hbrBackground = (HBRUSH) GetStockObject(BLACK_BRUSH);
    wc.lpszMenuName = NULL;
    wc.lpszClassName = "GoL";
    RegisterClass (&wc);

    
    hWnd = CreateWindow (
      "GoL", "Conway's Game of Life", 
      WS_CAPTION | WS_POPUPWINDOW | WS_VISIBLE,
      0, 0, W, H,
      NULL, NULL, hInstance, NULL);
      
    EnableOpenGL (hWnd, &hDC, &hRC);

	int i, j;

	srand(time(NULL));
	
	for (i = 0; i < H; i++) {
		for (j = 0; j < W; j++) {
			screen[i*W+j] = rand() % RND == 0;
		}
	}

    while (!bQuit) {
        if (PeekMessage (&msg, NULL, 0, 0, PM_REMOVE)) {
            if (msg.message == WM_QUIT) bQuit = TRUE;
            else {
                TranslateMessage (&msg);
                DispatchMessage (&msg);
            }
        }
        else {
        	glClearColor (0.0f, 0.0f, 0.0f, 0.0f);
            glClear (GL_COLOR_BUFFER_BIT);
            
		    int data[H][W][3]; 
		    for (i = 0; i < H; i++) {
		        for (j = 0; j < W; j++) {
		        	unsigned int col = 0;
		        	if (screen[i*W+j]) col = 4278190080;
		            data[i][j][0] = col;
		            data[i][j][1] = col;
		            data[i][j][2] = col;
		        }
		    }
		
		    glDrawPixels(W, H, GL_RGB, GL_UNSIGNED_INT, data);
            
	    	int i, j, k, l;
	    	for (i = 0; i < H; i++) {
				for (j = 0; j < W; j++) {
					int neighbours = 0;
					for (k = -1; k < 2; k++) {
						for (l = -1; l < 2; l++) {
							BOOL in = (i+k) >= 0 && (j+l) >= 0 && (i+k) < H && (j+l) < W;
							if (in && screen[(i+k)*W+(j+l)]) neighbours++;
						}
					}
					int pos = i*W+j;
					if (screen[pos]) neighbours--;
					// Any live cell with fewer than two live neighbours dies, as if by underpopulation.
					if (screen[pos] && neighbours < 2) buffer[pos] = FALSE;
					// Any live cell with more than three live neighbours dies, as if by overpopulation.
					else if (screen[pos] && neighbours > 3) buffer[pos] = FALSE;
					// Any dead cell with exactly three live neighbours becomes a live cell, as if by reproduction.
					else if (neighbours == 3) buffer[pos] = TRUE;
					// Any live cell with two or three live neighbours lives on to the next generation.
					else buffer[pos] = screen[pos];
				}
			}
			for (i = 0; i < H; i++) {
				for (j = 0; j < W; j++) {
					screen[i*W+j] = buffer[i*W+j];
				}
			}
			
            SwapBuffers(hDC);
        }
    }

    DisableOpenGL (hWnd, hDC, hRC);

    DestroyWindow (hWnd);

    return msg.wParam;
}

LRESULT CALLBACK WndProc (HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {

    switch (message) {
    case WM_CREATE:
        return 0;
    case WM_CLOSE:
        PostQuitMessage (0);
        return 0;

    case WM_DESTROY:
        return 0;

    case WM_KEYDOWN:
        switch (wParam)
        {
        case VK_ESCAPE:
            PostQuitMessage(0);
            return 0;
        }
        return 0;

    default:
        return DefWindowProc (hWnd, message, wParam, lParam);
    }
}


void EnableOpenGL (HWND hWnd, HDC *hDC, HGLRC *hRC) {
    PIXELFORMATDESCRIPTOR pfd;
    int iFormat;
    
    *hDC = GetDC (hWnd);
    
    ZeroMemory (&pfd, sizeof(pfd));
    pfd.nSize = sizeof(pfd);
    pfd.nVersion = 1;
    pfd.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
    pfd.iPixelType = PFD_TYPE_RGBA;
    pfd.cColorBits = 24;
    pfd.cDepthBits = 16;
    pfd.iLayerType = PFD_MAIN_PLANE;
    iFormat = ChoosePixelFormat (*hDC, &pfd);
    SetPixelFormat (*hDC, iFormat, &pfd);
    
    *hRC = wglCreateContext( *hDC );
    wglMakeCurrent( *hDC, *hRC );

}

void DisableOpenGL (HWND hWnd, HDC hDC, HGLRC hRC) {
    wglMakeCurrent (NULL, NULL);
    wglDeleteContext (hRC);
    ReleaseDC (hWnd, hDC);
}
