#include "global.h"
#include "playtime.h"
#include <Windows.h>
#include <conio.h>
extern unsigned int grade,blocktime;
extern time_t starttime;
extern unsigned char conblock[WIDTH][HEIGTH];

int PaintStartMenu();
void showabout();

int wmain(int argc, WCHAR** argv) {
	unsigned int runstate;
	Init();
	FitScreen((COORD) { 2 * WIDTH + 6, HEIGTH + 3 },hOut);
	do {
		grade = blocktime = (unsigned)(starttime = 0);
		memset(conblock, 0, WIDTH * HEIGTH);
		switch (runstate = PaintStartMenu()) {
		case 1:
			single_play();
			break;
		case 2:
			dou_play();
			break;
		case -1:
			showabout();
		}
		ClearScr(hOut);
	} while (runstate);
	Fin(FALSE);
}

int PaintStartMenu() {
	SetConsoleTextAttribute(hOut, FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE | FOREGROUND_INTENSITY);
	unsigned int namelen = wcslen(GAMELNAME), recnum;
	SetConsoleCursorPosition(hOut, (COORD) { (WIDTH + 4 - namelen) / 2, 4 });
	WriteConsole(hOut, GAMELNAME, namelen, &recnum, NULL);

	SetConsoleCursorPosition(hOut, (COORD) { 5, 7 });
	SetConsoleTextAttribute(hOut, FOREGROUND_RED | FOREGROUND_GREEN);
	WriteConsole(hOut, L"1.������Ϸ", wcslen(L"1.������Ϸ"), &recnum, NULL);

	SetConsoleCursorPosition(hOut, (COORD) { 5, 8 });
	SetConsoleTextAttribute(hOut, FOREGROUND_RED | FOREGROUND_BLUE);
	WriteConsole(hOut, L"2.��������Ϸ", wcslen(L"2.��������Ϸ"), &recnum, NULL);

	//SetConsoleCursorPosition(hOut, (COORD) { 6, 8 });
	FillConsoleOutputAttribute(hOut, FOREGROUND_INTENSITY, 10, (COORD) { 5, 9 }, & recnum);
	FillConsoleOutputCharacter(hOut, L'-', 17, (COORD) { 5, 9 }, & recnum);

	SetConsoleCursorPosition(hOut, (COORD) { 5, 10 });
	SetConsoleTextAttribute(hOut, FOREGROUND_RED | FOREGROUND_BLUE|FOREGROUND_GREEN);
	WriteConsole(hOut, L"+.����", wcslen(L"+.����"), &recnum, NULL);

	SetConsoleCursorPosition(hOut, (COORD) { 5, 11 });
	SetConsoleTextAttribute(hOut, FOREGROUND_INTENSITY | FOREGROUND_RED);
	WriteConsole(hOut, L"0.�˳�", wcslen(L"0.�˳�"), &recnum, NULL);

	INPUT_RECORD inp;
	while (1) {
		ReadConsoleInput(hIn, &inp, 1, &recnum);
		if (inp.EventType != KEY_EVENT) continue;
		if (!inp.Event.KeyEvent.bKeyDown) continue;
		switch (inp.Event.KeyEvent.uChar.UnicodeChar) {
		case '1':
			return 1;
		case '2':
			return 2;
		case '0':
			return 0;
		case '+':
			return -1;
		}
	}
}

void FitScreen(COORD size,HANDLE hOut) {
	HWND hwnd = GetConsoleWindow();
	unsigned cx = GetSystemMetrics(SM_CXSCREEN);            /* ��Ļ��� */
	unsigned cy = GetSystemMetrics(SM_CYSCREEN);            /* ��Ļ�߶� */
	RECT rect;
	//CONSOLE_SCREEN_BUFFER_INFO cinfo;
	CONSOLE_FONT_INFOEX finfo = {sizeof(finfo)};
	GetWindowRect(hwnd, &rect);
	//GetConsoleScreenBufferInfo(hOut, &cinfo);
	GetCurrentConsoleFontEx(hOut, FALSE, &finfo);
	unsigned barheigth = rect.bottom - rect.top - finfo.dwFontSize.Y *size.Y;
	unsigned externwidth = rect.right - rect.left - finfo.dwFontSize.X * size.X;
	finfo.nFont = 0;
	finfo.dwFontSize.Y = (cy - barheigth) / size.Y;
	finfo.dwFontSize.X = (cx - externwidth) / size.X;
	if (finfo.dwFontSize.X * 2 < finfo.dwFontSize.Y) {
		finfo.dwFontSize.Y = finfo.dwFontSize.X * 2;
		SetWindowPos(hwnd, NULL, 0, (cy - barheigth - finfo.dwFontSize.Y * size.Y) / 2, 0, 0, SWP_NOSIZE);
	}
	else {
		finfo.dwFontSize.X = finfo.dwFontSize.Y / 2;
		SetWindowPos(hwnd, NULL, (cx - externwidth - finfo.dwFontSize.X * size.X) / 2, 0, 0, 0, SWP_NOSIZE);
	}
	SetCurrentConsoleFontEx(hOut, FALSE, &finfo);
	return;
}

void showabout() {
	ClearScr(hOut);
	DWORD recnum;
	SetConsoleTextAttribute(hOut, FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
	WriteConsole(hOut, L"����", wcslen(L"����"), &recnum, NULL);

	SetConsoleCursorPosition(hOut, (COORD) { 2, 2 });
	WriteConsole(hOut, L"����:Tetris", wcslen(L"����:Tetris"), &recnum, NULL);

	SetConsoleCursorPosition(hOut, (COORD) { 2, 3 });
	WriteConsole(hOut, L"����:LiYan", wcslen(L"����:LiYan"), &recnum, NULL);

	SetConsoleCursorPosition(hOut, (COORD) { 0, 7 });
	WriteConsole(hOut, L"������˳�", wcslen(L"������˳�"), &recnum, NULL);
	if (_getch());
	//return;
}