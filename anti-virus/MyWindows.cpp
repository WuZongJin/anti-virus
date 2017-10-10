#include "MyDirectX.h"
using namespace std;
bool gameover = false;


//windows event handle
LRESULT WINAPI WinProc(HWND hWnd, UINT msg, WPARAM wParam, WPARAM lParam)
{
	switch (msg)
	{
	case WM_DESTROY:
		gameover = true;
		PostQuitMessage(0);
		return 0;
	}
	return DefWindowProc(hWnd, msg, wParam, lParam);
}


//windows entry point
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrestance, LPSTR lpCmdLine, int nCmdShow)
{
	WNDCLASSEX wc;
	wc.style = CS_HREDRAW | CS_VREDRAW;
	wc.cbSize = sizeof(WNDCLASSEX);
	wc.lpfnWndProc = (WNDPROC)WinProc;
	wc.cbClsExtra = 0;
	wc.cbWndExtra = 0;
	wc.hInstance = hInstance;
	wc.hIcon = NULL;
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	wc.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
	wc.lpszClassName = "MainWindowClass";
	wc.lpszMenuName = NULL;
	wc.hIconSm = NULL;
	RegisterClassEx(&wc);


	//create a new window
	HWND window = CreateWindow("MainWindowClass", APPTITLE.c_str(),
		WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT,
		SCREENW, SCREENH, NULL, NULL, hInstance, NULL);
	if (window == 0)
		return 0;

	//display the window
	ShowWindow(window, nCmdShow);
	UpdateWindow(window);

	//initialize the game
	if (!Game_Init(window))return 0;

	//main message loop
	MSG message;
	while (!gameover)
	{
		if (PeekMessage(&message, NULL, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&message);
			DispatchMessage(&message);
		}
		//process game loop
		Game_Run(window);
 	}

	//shutdown
	Game_End();
	return message.wParam;
}
