#include "global.h"
#include "playtime.h"
#include <Windows.h>
int PaintStartMenu();
int wmain(int argc, WCHAR** argv) {
	unsigned int runstate;
	Init();
	do {
		switch (runstate = PaintStartMenu()) {
		case 1:
			single_play();
			break;
		case 2:
			dou_play();
			break;
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
	WriteConsole(hOut, L"1.单人游戏", wcslen(L"1.单人游戏"), &recnum, NULL);

	SetConsoleCursorPosition(hOut, (COORD) { 5, 8 });
	SetConsoleTextAttribute(hOut, FOREGROUND_RED | FOREGROUND_BLUE);
	WriteConsole(hOut, L"2.局域网游戏", wcslen(L"2.局域网游戏"), &recnum, NULL);

	//SetConsoleCursorPosition(hOut, (COORD) { 6, 8 });
	FillConsoleOutputAttribute(hOut, FOREGROUND_INTENSITY, 10, (COORD) { 5, 9 }, & recnum);
	FillConsoleOutputCharacter(hOut, L'-', 10, (COORD) { 5, 9 }, & recnum);

	SetConsoleCursorPosition(hOut, (COORD) { 5, 10 });
	SetConsoleTextAttribute(hOut, FOREGROUND_INTENSITY | FOREGROUND_RED);
	WriteConsole(hOut, L"0.退出", wcslen(L"0.退出"), &recnum, NULL);

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
		}
	}
}