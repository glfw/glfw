#include <windows.h>
#include <iostream>
#include <stdio.h>
#include "GLApplication.h"

GLApplication application;
bool running = true;
HINSTANCE hInstance;

// The window callback function
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) 
{
	switch (message) {
		case WM_SIZE:
			{
				application.setSize(LOWORD(lParam), HIWORD(lParam));
				break;
			}

		case WM_KEYDOWN:
			{   
				if((unsigned short) wParam == 112) application.showDeferredRendering();
				if((unsigned short) wParam == 113) application.showRenderTargets();
				break;
			} 

		case WM_DESTROY:
			{
				PostQuitMessage(0);
				break;
			}
	}

	return DefWindowProc(hWnd, message, wParam, lParam);
}

// Main entry point
int WINAPI WinMain(HINSTANCE hInstance,
                     HINSTANCE hPrevInstance,
                     LPSTR    lpCmdLine,
                     int       nCmdShow) 
{
	MSG msg;
	WNDCLASS windowClass;
	HWND hWnd;
	DWORD dwExStyle = WS_EX_APPWINDOW | WS_EX_WINDOWEDGE;
	const int width = 1024;
	const int height = 768;

	hInstance = GetModuleHandle(NULL);

	windowClass.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
	windowClass.lpfnWndProc = (WNDPROC) WndProc;
	windowClass.cbClsExtra = 0;
	windowClass.cbWndExtra = 0;
	windowClass.hInstance = hInstance;
	windowClass.hIcon = LoadIcon(NULL, IDI_WINLOGO);
	windowClass.hCursor = LoadCursor(NULL, IDC_ARROW);
	windowClass.hbrBackground = NULL;
	windowClass.lpszMenuName = NULL;
	windowClass.lpszClassName = L"DeferredRenderingClass";

	if (!RegisterClass(&windowClass)) {
		return false;
	}

	hWnd = CreateWindowEx(dwExStyle, L"DeferredRenderingClass",  L"Deferred rendering tutorial", WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, 0, width, height, NULL, NULL, hInstance, NULL);


	if(!application.initialize(hWnd, width, height))
		return 0;

	ShowWindow(hWnd, SW_SHOW);
	UpdateWindow(hWnd);


	bool running = true;
	while (running)
	{
		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) 
		{
			if (msg.message == WM_QUIT)
			{
				running = false; 
			}
			else 
			{
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}
		}
		else 
		{
			application.update();
			application.render();
		}
	}

	application.release();

	return (int) msg.wParam;
}