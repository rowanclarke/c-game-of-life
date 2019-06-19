
#include <windows.h>
#include <time.h> 
#include <GL/gl.h>

LRESULT CALLBACK WndProc (HWND hWnd, UINT message,
WPARAM wParam, LPARAM lParam);
void EnableOpenGL (HWND hWnd, HDC *hDC, HGLRC *hRC);
void DisableOpenGL (HWND hWnd, HDC hDC, HGLRC hRC);
int W;
int H;
const int RND = 3;					// -a-b-g-r
const unsigned int BACK = 0;	 	// 00000000
const unsigned int FORE = 16777215; // 00FFFFFF
int zoom = 1;

int WINAPI WinMain (HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int iCmdShow) {
    WNDCLASS wc;
    HWND hWnd;
    HDC hDC;
    HGLRC hRC;        
    MSG msg;
    BOOL bQuit = FALSE;
    BOOL* screen;
    BOOL* buffer;
    int* data;
    float theta = 0.0f;
    W = GetSystemMetrics(SM_CXSCREEN);
	H = GetSystemMetrics(SM_CYSCREEN);
    screen = calloc(sizeof(BOOL)*W*H, 1);
    buffer = calloc(sizeof(BOOL)*W*H, 1);
    data =  malloc(sizeof(int)*W*H*3);
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
      WS_POPUPWINDOW | WS_VISIBLE | WS_OVERLAPPED,
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
            
		    
		    for (i = 0; i < H; i++) {
		        for (j = 0; j < W; j++) {
		        	int col = BACK;
		        	if (screen[i*W+j]) col = FORE;
		            data[(i*W+j)*3] = ((col >> 0) & 0x000000ff) * 16843008;
					data[(i*W+j)*3+1] = ((col >> 8) & 0x000000ff) * 16843008;
					data[(i*W+j)*3+2] = ((col >> 16) & 0x000000ff) * 16843008;
		        }
		    }
			
		    glDrawPixels(W, H, GL_RGB, GL_UNSIGNED_INT, data);
            glPixelZoom(zoom, zoom);
            
	    	int k, l;
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
    case WM_DESTROY:
        return 0;
    case WM_CLOSE:
        PostQuitMessage (0);
        return 0;
        
    case WM_KEYDOWN:
        switch (wParam)
        {
        case VK_ESCAPE:
            PostQuitMessage(0);
            return 0;
        }
        return 0;
	case WM_MOUSEWHEEL:
		zoom += GET_WHEEL_DELTA_WPARAM(wParam) / 120;
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
