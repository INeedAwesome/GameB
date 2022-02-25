#include "Main.h"

HWND gGameWindow;

BOOL gGameIsRunning;

void* Memory;
	
GAMEBITMAP gBackBuffer;

MONITORINFO gMonitorInfo = { sizeof(MONITORINFO) } ;

int32_t gMonitorWidth;
int32_t gMonitorHeight;

int WINAPI WinMain(HINSTANCE Instance, HINSTANCE PrevInstance, PSTR CommandLine, INT CmdShowWindow)
{
	if (GameIsAlreadyRunning() == TRUE)
	{
		MessageBoxA(NULL, "Another instance is already running!", "Error!", MB_ICONEXCLAMATION | MB_OK);
		goto Exit;
	}
	
	if (CreateMainGameWindow() != ERROR_SUCCESS)
	{
		MessageBoxA(NULL, "Failed to create window! ", "Error!", MB_ICONEXCLAMATION | MB_OK);
		goto Exit;
	}

	gBackBuffer.BitmapInfo.bmiHeader.biSize = sizeof(gBackBuffer.BitmapInfo.bmiHeader);
	gBackBuffer.BitmapInfo.bmiHeader.biWidth = GAME_RES_WIDTH;
	gBackBuffer.BitmapInfo.bmiHeader.biHeight = GAME_RES_HEIGHT;
	gBackBuffer.BitmapInfo.bmiHeader.biBitCount = GAME_BPP;
	gBackBuffer.BitmapInfo.bmiHeader.biCompression = BI_RGB;
	gBackBuffer.BitmapInfo.bmiHeader.biPlanes = 1;
	gBackBuffer.Memory = VirtualAlloc(NULL, GAME_DRAWING_AREA_MEMORY_SIZE, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
	if (gBackBuffer.Memory == NULL)
	{
		MessageBoxA(NULL, "Failed to allocate memory for drawing surface! ", "Error!", MB_ICONEXCLAMATION | MB_OK);
		goto Exit;
	}

	memset(gBackBuffer.Memory, 0x7F, GAME_DRAWING_AREA_MEMORY_SIZE);

	gGameIsRunning = TRUE;
	MSG Message = { 0 };
	
	while (gGameIsRunning) 
	{ 
		while (PeekMessageA(&Message, gGameWindow, 0, 0, PM_REMOVE)) 
		{
			DispatchMessageA(&Message);
		}

		// Process player input
		ProcessPlayerInput();

		// Render frame graphics
		RenderFrameGraphics();

		Sleep(1);
	}

Exit:

	return 0;
}

void ProcessPlayerInput(void)
{
	short EscapeKeyIsDown = GetAsyncKeyState(VK_ESCAPE);
	if (EscapeKeyIsDown)
	{
		SendMessageA(gGameWindow, WM_CLOSE, 0, 0);
	}
}

void RenderFrameGraphics(void)
{
	//memset(gBackBuffer.Memory, 0xFF, GAME_RES_HEIGHT * GAME_RES_WIDTH * 4);

	PIXEL32 Pixel = { 0 };
	Pixel.Blue = 0xff;
	Pixel.Green = 0;
	Pixel.Red = 0;
	Pixel.Alpha = 0xff;

	for (int x = 0; x < GAME_RES_HEIGHT* GAME_RES_WIDTH; x++)
	{
		memcpy((PIXEL32*)gBackBuffer.Memory + x, & Pixel, 4, sizeof(PIXEL32));
	}


	HDC DeviceContext = GetDC(gGameWindow);

	StretchDIBits(DeviceContext, 0, 0, 
		gMonitorWidth, gMonitorHeight - 2, 
		0, 0, 
		GAME_RES_WIDTH, GAME_RES_HEIGHT, 
		gBackBuffer.Memory, &gBackBuffer.BitmapInfo,
		DIB_RGB_COLORS, SRCCOPY);

	ReleaseDC(gGameWindow, DeviceContext);
}

LRESULT CALLBACK MainWindowProcedure(_In_ HWND WindowHandle, _In_ UINT Message, _In_ WPARAM WideParameter, _In_ LPARAM LongParameter)
{
	LRESULT Result = 0;
	
	switch (Message)
	{
		case WM_CLOSE: 
		{
			gGameIsRunning = FALSE;
			PostQuitMessage(0);
			break;
		}
		default: 
		{
			Result = DefWindowProcA(WindowHandle, Message, WideParameter, LongParameter);
		}
	}

	return Result;
}

DWORD CreateMainGameWindow(void) 
{
	DWORD Result = ERROR_SUCCESS;

	WNDCLASSEXA WindowClass = { 0 };


	WindowClass.cbSize = sizeof(WNDCLASSEXA);
	WindowClass.style = 0;
	WindowClass.lpfnWndProc = MainWindowProcedure;
	WindowClass.cbClsExtra = 0;
	WindowClass.cbWndExtra = 0;
	WindowClass.hInstance = GetModuleHandleA(NULL);
	WindowClass.hIcon = LoadIconA(NULL, IDI_APPLICATION);
	WindowClass.hIconSm = LoadIconA(NULL, IDI_APPLICATION);
	WindowClass.hCursor = LoadCursorA(NULL, IDC_ARROW);
	WindowClass.hbrBackground = CreateSolidBrush(RGB(255, 0, 255));
	WindowClass.lpszMenuName = NULL;
	WindowClass.lpszClassName = GAME_NAME "_WINDOWCLASS";

	if (!RegisterClassExA(&WindowClass))
	{
		Result = GetLastError();
		MessageBoxA(NULL, "Window Registration Failed!", "Error!", MB_ICONEXCLAMATION | MB_OK);
		goto Exit;
	}

	gGameWindow = CreateWindowExA(WS_EX_CLIENTEDGE, WindowClass.lpszClassName, GAME_NAME,
		WS_OVERLAPPEDWINDOW | WS_VISIBLE, CW_USEDEFAULT, CW_USEDEFAULT, 
		GAME_RES_WIDTH, GAME_RES_HEIGHT, NULL, NULL, GetModuleHandleA(NULL), NULL);

	if (gGameWindow == NULL)
	{
		Result = GetLastError();
		MessageBoxA(NULL, "Window Creation Failed!", "Error!", MB_ICONEXCLAMATION | MB_OK);
		goto Exit;
	}

	if (GetMonitorInfoA(MonitorFromWindow(gGameWindow, MONITOR_DEFAULTTOPRIMARY), &gMonitorInfo) == FALSE)
	{
		Result = ERROR_MONITOR_INFO_FAILED;
		goto Exit;
	}

	gMonitorWidth = gMonitorInfo.rcMonitor.right - gMonitorInfo.rcMonitor.left;
	gMonitorHeight = gMonitorInfo.rcMonitor.bottom - gMonitorInfo.rcMonitor.top;

	if (SetWindowLongPtrA(gGameWindow, GWL_STYLE, (WS_OVERLAPPEDWINDOW | WS_VISIBLE) & ~WS_OVERLAPPEDWINDOW) == 0)//  taking away the WS_OVERLAPPEDWINDOW except WS_VISIBLE
	{
		Result = GetLastError();
		goto Exit;
	}
	if (SetWindowPos(gGameWindow, HWND_TOP, gMonitorInfo.rcMonitor.left, gMonitorInfo.rcMonitor.top, gMonitorWidth, gMonitorHeight, SWP_NOOWNERZORDER | SWP_FRAMECHANGED) == 0)
	{
		Result = GetLastError();
		goto Exit;
	}

	

Exit:
	return Result;
}

BOOL GameIsAlreadyRunning(void)
{
	// Mutual Exclution / only one instance allowed
	HANDLE Mutex = NULL; 
	
	Mutex = CreateMutexA(NULL, GAME_NAME, "_GameMutex");

	if (GetLastError() == ERROR_ALREADY_EXISTS)
	{
		return TRUE;
	}
	else
	{
		return FALSE;
	}
}
