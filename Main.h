#pragma once

#include <stdio.h>
#include <Windows.h>

#define GAME_NAME		"Game_B"
#define GAME_RES_WIDTH  384
#define GAME_RES_HEIGHT 216
#define GAME_BPP		32
#define GAME_DRAWING_AREA_MEMORY_SIZE	(GAME_RES_WIDTH * GAME_RES_HEIGHT * (32 / 8))

#define ERROR_OUT_OF_MEMORY 5
#define ERROR_ 1

typedef struct GAMEBITMAP
{
	BITMAPINFO BitmapInfo;
	void* Memory;

} GAMEBITMAP;

LRESULT CALLBACK MainWindowProcedure(
	_In_ HWND WindowHandle,        // handle to window
	_In_ UINT Message,             // message identifier
	_In_ WPARAM WideParameter,     // first message parameter
	_In_ LPARAM LongParameter);    // second message parameter

DWORD CreateMainGameWindow(void);

BOOL GameIsAlreadyRunning(void);

void ProcessPlayerInput(void);

void RenderFrameGraphics(void);