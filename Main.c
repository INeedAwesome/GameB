#include "Main.h"

HWND gGameWindow;

BOOL gGameIsRunning;

void* Memory;
	
GAMEBITMAP gDrawingSurface;

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

	gDrawingSurface.BitmapInfo.bmiHeader.biSize = sizeof(gDrawingSurface.BitmapInfo.bmiHeader);
	gDrawingSurface.BitmapInfo.bmiHeader.biWidth = GAME_RES_WIDTH;
	gDrawingSurface.BitmapInfo.bmiHeader.biHeight = GAME_RES_HEIGHT;
	gDrawingSurface.BitmapInfo.bmiHeader.biBitCount = GAME_BPP;
	gDrawingSurface.BitmapInfo.bmiHeader.biCompression = BI_RGB;
	gDrawingSurface.BitmapInfo.bmiHeader.biPlanes = 1;
	if ((gDrawingSurface.Memory = VirtualAlloc(NULL, GAME_DRAWING_AREA_MEMORY_SIZE, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE)) == NULL)
	{
		MessageBoxA(NULL, "Failed to allocate memory for drawing surface! ", "Error!", MB_ICONEXCLAMATION | MB_OK);
		goto Exit;
	}

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
	WindowClass.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	WindowClass.lpszMenuName = NULL;
	WindowClass.lpszClassName = GAME_NAME "_WINDOWCLASS";

	if (!RegisterClassExA(&WindowClass))
	{
		Result = GetLastError();
		MessageBoxA(NULL, "Window Registration Failed!", "Error!", MB_ICONEXCLAMATION | MB_OK);
		goto Exit;
	}

	gGameWindow = CreateWindowExA(WS_EX_CLIENTEDGE, WindowClass.lpszClassName, GAME_NAME,
		WS_OVERLAPPEDWINDOW | WS_VISIBLE, CW_USEDEFAULT, CW_USEDEFAULT, 1280, 720, NULL, NULL, GetModuleHandleA(NULL), NULL);

	if (gGameWindow == NULL)
	{
		Result = GetLastError();
		MessageBoxA(NULL, "  Window Creation Failed!  ", "Error!", MB_ICONEXCLAMATION | MB_OK);
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
