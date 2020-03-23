#include "global.h"
#include <Windows.h>
#include <stdio.h>
#include <time.h>
CONSOLE_SCREEN_BUFFER_INFOEX sinfo = { sizeof(CONSOLE_SCREEN_BUFFER_INFOEX) };
CONSOLE_FONT_INFOEX finfo = {sizeof(finfo)};
WCHAR title[MAX_PATH];
DWORD mode;
COORD blockpos;
HANDLE hOut, hIn, hWindow;
LONG SavedLong;
BOOL isWinUpper;

unsigned short block_num = 0;
unsigned short blocks[] = { 0x0F00,0x2260,0x4460,0x0660,0x0630,0x0360,0x0270,0 };

BOOL WINAPI ErrProc(DWORD CtrlType) {
	printf("Error Happened!\nCtrlType:%d\nLastError:%d", CtrlType, GetLastError());
	Fin(TRUE);
	exit(-1);
	return TRUE;
}

BOOL isWin8plus() {
	typedef void(__stdcall* NTPROC)(DWORD*, DWORD*, DWORD*);
	HINSTANCE hinst = LoadLibrary(L"ntdll.dll");
	if (!hinst) return FALSE;
	DWORD dwMajor, dwMinor, dwBuildNumber;
	NTPROC proc = (NTPROC)GetProcAddress(hinst, (LPCSTR)"RtlGetNtVersionNumbers");
	FreeLibrary(hinst);
	proc(&dwMajor, &dwMinor, &dwBuildNumber);
	if (dwMajor == 6 && dwMinor == 3)	//win 8.1
		return TRUE;
	if (dwMajor == 10 && dwMinor == 0)	//win 10
		return TRUE;
	return FALSE;
}

void Fin(BOOL isErr) {
	SetConsoleScreenBufferInfoEx(hOut, &sinfo);
	SetConsoleCursorInfo(hOut, &(CONSOLE_CURSOR_INFO){1, 1});
	SetConsoleMode(hIn, mode);
	SetCurrentConsoleFontEx(hOut, FALSE, &finfo);
	if (!isErr) {
		SetWindowLong(hWindow, GWL_STYLE, SavedLong);
		SetWindowPos(hWindow, NULL, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_FRAMECHANGED);

		if (*title) SetConsoleTitle(title);
	}
}
void Init() {

	errno = 0;
	SetConsoleCtrlHandler(ErrProc, TRUE);
	srand((unsigned int)time(NULL));

	hIn = GetStdHandle(STD_INPUT_HANDLE);
	hOut = GetStdHandle(STD_OUTPUT_HANDLE);
	hWindow = GetConsoleWindow();
	GetCurrentConsoleFontEx(hOut, FALSE, &finfo); 
	if (!GetConsoleScreenBufferInfoEx(hOut, &sinfo)) memset(&sinfo, 0, sizeof(sinfo));
	SetConsoleCursorInfo(hOut, &(CONSOLE_CURSOR_INFO){1, 0});
	SetConsoleCursorPosition(hOut, (COORD) { 0, 0 });

	SetWindowLong(hWindow, GWL_STYLE, (SavedLong = GetWindowLong(hWindow, GWL_STYLE)) & ~WS_MAXIMIZEBOX & ~WS_SIZEBOX);
	SetWindowPos(hWindow, NULL, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_FRAMECHANGED);

	if (!GetConsoleTitle((LPWSTR)&title, MAX_PATH))memset(&title, 0, sizeof(WCHAR) * MAX_PATH);
	SetConsoleTitle(GAMELNAME);

	GetConsoleMode(hIn, &mode);
	SetConsoleMode(hIn, mode & ~ENABLE_QUICK_EDIT_MODE & ~ENABLE_INSERT_MODE);

	isWinUpper = isWin8plus();

	SetConsoleScreenBufferSize(hOut, (COORD) { 2 * WIDTH + 6, HEIGTH + 3 });
	SetConsoleWindowInfo(hOut, TRUE, &(SMALL_RECT){0, 0, 2 * WIDTH + 5, HEIGTH + 2});
	SetConsoleScreenBufferSize(hOut, (COORD) { 2 * WIDTH + 6, HEIGTH + 3 });

	ClearScr(hOut);
}
unsigned short randblock() {
	if (!block_num) {
		unsigned short* myblock = blocks;
		while (*myblock++) block_num++;
	}
	return aclockwise(blocks[rand() % block_num], rand() % 4);
}
unsigned char randcolor() {
	unsigned char color = rand() % 7 + 1;
	return (color << 4) | color | FOREGROUND_INTENSITY;
}

unsigned short aclockwise(unsigned short clock, unsigned short times) {
	times %= 4;
	while (times--) {
		clock = ((clock & 0x1000) << 3) |
			((clock & 0x100) << 6) |
			((clock & 0x10) << 9) |
			((clock & 0x1) << 12) |
			((clock & 0x2000) >> 2) |
			((clock & 0x200) << 1) |
			((clock & 0x20) << 4) |
			((clock & 0x2) << 7) |
			((clock & 0x4000) >> 7) |
			((clock & 0x400) >> 4) |
			((clock & 0x40) >> 1) |
			((clock & 0x4) << 2) |
			((clock & 0x8000) >> 12) |
			((clock & 0x800) >> 9) |
			((clock & 0x80) >> 6) |
			((clock & 0x8) >> 3);
	}
	return clock;
}

//·µ»ØÖµ Left:Width
unsigned int GetBlockInfo(COORD blockpos, unsigned short block) {
	if (block & 0x8888) {
		if (block & 0x1111) return blockpos.X << 8 | 4;
		if (block & 0x2222) return blockpos.X << 8 | 3;
		if (block & 0x4444) return blockpos.X << 8 | 2;
		return blockpos.X << 8 | 1;
	}
	if (block & 0x4444) {
		if (block & 0x1111) return (blockpos.X + 1) << 8 | 3;
		if (block & 0x2222) return (blockpos.X + 1) << 8 | 2;
		return (blockpos.X + 1) << 8 | 1;
	}
	if (block & 0x2222) {
		if (block & 0x1111) return (blockpos.X + 2) << 8 | 2;
		return (blockpos.X + 2) << 8 | 1;
	}
	if (block) return (blockpos.X + 3) << 8 | 1;
	return blockpos.X << 8;
}

void ClearScr(HANDLE hOut) {
	CONSOLE_SCREEN_BUFFER_INFO csinfo; unsigned int recnum;
	GetConsoleScreenBufferInfo(hOut, &csinfo);
	FillConsoleOutputAttribute(hOut, 0, csinfo.dwSize.X * csinfo.dwSize.Y, (COORD) { 0, 0 }, & recnum);
	SetConsoleCursorPosition(hOut, (COORD) { 0, 0 });
}