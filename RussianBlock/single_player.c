#include <stdio.h>
#include <Windows.h>
#include <time.h>
#include <stdlib.h>
#include <conio.h>
#include <math.h>
#include "playtime.h"
#include "global.h"

#pragma warning(disable:4018)
HANDLE sOut;
//方块:┌ └ ┐ ┘ ─ │ ├ ┤ ┬ ┴ ┼□
void Sgn_Init();
void WaitForStart();
unsigned short randblock();
void spawnrand();
void repaint();
void RunTimer();
void ShowEnd();
BOOL isWin8plus();
void Sgn_Fin();

unsigned char conblock[WIDTH][HEIGTH] = { 0 };
unsigned short curblock, nextblock;
unsigned char blockcolor, nextblockcolor;

unsigned int blocktime = 0;
unsigned int grade = 0;
time_t starttime;


int single_play() {
	Sgn_Init();
	spawnrand();
	repaint(sOut);
	WaitForStart();
	starttime = time(NULL);
	RunTimer();
	ShowEnd();
	Sgn_Fin();
	return 0;
}
void Sgn_Init() {
	memset(conblock, 0, sizeof(unsigned char) * WIDTH * HEIGTH);

	SetConsoleScreenBufferSize(hOut, (COORD) { 2 * WIDTH + 6, HEIGTH + 3 });
	SetConsoleWindowInfo(hOut, TRUE, &(SMALL_RECT){0, 0, 2 * WIDTH + 5, HEIGTH + 2});
	SetConsoleScreenBufferSize(hOut, (COORD) { 2 * WIDTH + 6, HEIGTH + 3 });

	ClearScr(hOut);

	sOut = CreateConsoleScreenBuffer(GENERIC_WRITE, 0, NULL, CONSOLE_TEXTMODE_BUFFER, NULL);
	if (sOut == INVALID_HANDLE_VALUE) {
		puts("CreateConsoleScreenBuffer Function Failed");
		exit(1);
	}
	SetConsoleWindowInfo(sOut, TRUE, &(SMALL_RECT){0, 0, 2 * WIDTH + 5, HEIGTH + 2});
	SetConsoleScreenBufferSize(sOut, (COORD) { 2 * WIDTH + 6, HEIGTH + 3 });
	SetConsoleCursorInfo(sOut, &(CONSOLE_CURSOR_INFO){1, 0});
}
void Sgn_Fin() {
	ClearScr(hOut);
	SetStdHandle(STD_OUTPUT_HANDLE, hOut);
	SetConsoleActiveScreenBuffer(hOut);
	CloseHandle(sOut);
}
void WaitForStart() {
	unsigned int recnum = wcslen(L"按下任意键开始");
	SetConsoleCursorPosition(hOut, (COORD) { WIDTH + 2 - recnum, 4 });
	WriteConsole(hOut, L"按下任意键开始", recnum, &recnum, NULL);
	FlushFileBuffers(hOut);
	if (_getch()) return;
}
void WaitForContinue() {
	unsigned int recnum = wcslen(L"按下任意键继续");
	SetConsoleCursorPosition(hOut, (COORD) { WIDTH + 2 - recnum, 4 });
	WriteConsole(hOut, L"按下任意键继续", recnum, &recnum, NULL);
	FlushFileBuffers(hOut);
	if (_getch()) return;
}
void ShowEnd() {
	CONSOLE_SCREEN_BUFFER_INFO csinfo;
	WCHAR buffer[256];
	GetConsoleScreenBufferInfo(hOut, &csinfo);
	unsigned int recnum;
	FillConsoleOutputAttribute(hOut, FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE, csinfo.dwSize.X * csinfo.dwSize.Y, (COORD) { 0, 0 }, & recnum);
	FillConsoleOutputCharacter(hOut, L' ', csinfo.dwSize.X * csinfo.dwSize.Y, (COORD) { 0, 0 }, & recnum);
	SetConsoleCursorPosition(hOut, (COORD) { 0, 0 });

	SetConsoleTextAttribute(hOut, FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
	WriteConsole(hOut, L"成绩:", wcslen(L"成绩:"), &recnum, NULL);
	SetConsoleTextAttribute(hOut, FOREGROUND_GREEN);
	swprintf_s(buffer, 256, L"%d\n", grade);
	WriteConsole(hOut, buffer, wcslen(buffer), &recnum, NULL);

	SetConsoleTextAttribute(hOut, FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
	WriteConsole(hOut, L"耗时:", wcslen(L"耗时:"), &recnum, NULL);
	SetConsoleTextAttribute(hOut, FOREGROUND_GREEN);
	swprintf_s(buffer, 256, L"%lfs\n", difftime(time(NULL), starttime));
	WriteConsole(hOut, buffer, wcslen(buffer), &recnum, NULL);

	SetConsoleTextAttribute(hOut, FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
	WriteConsole(hOut, L"方块掉落个数:", wcslen(L"方块掉落个数:"), &recnum, NULL);
	SetConsoleTextAttribute(hOut, FOREGROUND_GREEN);
	swprintf_s(buffer, 256, L"%d\n", blocktime);
	WriteConsole(hOut, buffer, wcslen(buffer), &recnum, NULL);

	FlushConsoleInputBuffer(hIn);

	SetConsoleTextAttribute(hOut, FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
	WriteConsole(hOut, L"任意键继续", wcslen(L"任意键继续"), &recnum, NULL);
	FlushFileBuffers(hOut);
	if (_getch());
	return;
}
void flushfullline(BOOL spe) {
	unsigned short line = 0;
	for (; line < HEIGTH; line++) {
		unsigned short index = 0;
		while (conblock[index][line] && index++ < WIDTH);
		if (index >= WIDTH) {//满行
			grade++;//计分板+1
			unsigned short tmpline, tmpx;
			for (tmpx = 0; tmpx < WIDTH; tmpx++) {
				tmpline = line;
				while (tmpline) {
					conblock[tmpx][tmpline] = conblock[tmpx][tmpline - 1];
					tmpline--;
				}
				*conblock[tmpx] = 0;
			}
			if (spe) {
				//特效
				repaint();
				SetConsoleTextAttribute(hOut, SCOLOR_BACKGROUND);
				unsigned int recnum;
				for (tmpx = 0; tmpx < WIDTH / 2; tmpx++) {
					//SetConsoleCursorPosition(hOut, (COORD) { WIDTH - 2 * tmpx, line + 1 });
					//WriteConsole(hOut, L"  ", 2, &recnum, NULL);
					FillConsoleOutputCharacter(hOut, L' ', 2, (COORD) { WIDTH - 2 * tmpx, line + 1 }, & recnum);
					FillConsoleOutputAttribute(hOut, 0, 1, (COORD) { WIDTH - 2 * tmpx, line + 1 }, & recnum);
					//SetConsoleCursorPosition(hOut, (COORD) { WIDTH + 2 + 2 * tmpx, line + 1 });
					//WriteConsole(hOut, L"  ", 2, &recnum, NULL);
					FillConsoleOutputCharacter(hOut, L' ', 2, (COORD) { WIDTH + 2 + 2 * tmpx, line + 1 }, & recnum);
					FillConsoleOutputAttribute(hOut, 0, 1, (COORD) { WIDTH + 2 + 2 * tmpx, line + 1 }, & recnum);
					FlushFileBuffers(hOut);
					Sleep(20);
				}
			}
		}
	}
}
void repaint() {
	unsigned int recnum, record;
	short i, i2;
	SetConsoleCursorPosition(sOut, (COORD) { 0, 0 });
	SetConsoleTextAttribute(sOut, SCOLOR_FORM);
	if (isWinUpper) {//Windows10系统处理字符错误,需进行检查
		WriteConsole(sOut, L"┌", 2, &recnum, NULL);
		record = WIDTH;
		while (record--) WriteConsole(sOut, L"─", 2, &recnum, NULL);
		WriteConsole(sOut, L"┐", 2, &recnum, NULL);
		SetConsoleCursorPosition(sOut, (COORD) { 0, 1 });
		for (i = 0; i < HEIGTH; i++) {
			SetConsoleTextAttribute(sOut, SCOLOR_FORM);
			SetConsoleCursorPosition(sOut, (COORD) { 0, i + 1 });
			WriteConsole(sOut, L"│", 2, &recnum, NULL);
			for (i2 = 0; i2 < WIDTH; i2++) {
				if (conblock[i2][i]) {
					SetConsoleTextAttribute(sOut, conblock[i2][i]);
					WriteConsole(sOut, L"□", 1, &recnum, NULL);
				}
				else {
					SetConsoleTextAttribute(sOut, SCOLOR_BACKGROUND);
					WriteConsole(sOut, L"  ", 2, &recnum, NULL);
				}
			}
			SetConsoleTextAttribute(sOut, SCOLOR_FORM);
			WriteConsole(sOut, L"│", 2, &recnum, NULL);
		}
		SetConsoleCursorPosition(sOut, (COORD) { 0, HEIGTH + 1 });
		WriteConsole(sOut, L"└", 2, &recnum, NULL);
		record = GetBlockInfo(blockpos, curblock) >> 8;
		while (record--) WriteConsole(sOut, L"─", 2, &recnum, NULL);
		SetConsoleTextAttribute(sOut, SCOLOR_DOWNPLACE);
		record = GetBlockInfo(blockpos, curblock) & 0xff;
		while (record--) WriteConsole(sOut, L"─", 2, &recnum, NULL);
		SetConsoleTextAttribute(sOut, SCOLOR_FORM);
		record = GetBlockInfo(blockpos, curblock);
		record = WIDTH - (record >> 8) - record & 0xff;
		while (record--) WriteConsole(sOut, L"─", 2, &recnum, NULL);
		WriteConsole(sOut, L"┘", 2, &recnum, NULL);
		SetConsoleTextAttribute(sOut, blockcolor);
		for (i2 = blockpos.Y + 3; i2 >= blockpos.Y; i2--) {
			if (i2 < 0) break;
			for (i = blockpos.X + 3; i >= blockpos.X; i--) {
				if (curblock & ((0x8000 >> ((i2 - blockpos.Y) * 4) >> (i - blockpos.X)))) {
					SetConsoleCursorPosition(sOut, (COORD) { 2 * i + 2, i2 + 1 });
					WriteConsole(sOut, L"□", 1, &recnum, NULL);
				}
			}
		}
	}
	else {
		WriteConsole(sOut, L"┌", 1, &recnum, NULL);
		record = WIDTH;
		while (record--) WriteConsole(sOut, L"─", 1, &recnum, NULL);
		WriteConsole(sOut, L"┐", 1, &recnum, NULL);
		SetConsoleCursorPosition(sOut, (COORD) { 0, 1 });
		for (i = 0; i < HEIGTH; i++) {
			SetConsoleTextAttribute(sOut, SCOLOR_FORM);
			SetConsoleCursorPosition(sOut, (COORD) { 0, i + 1 });
			WriteConsole(sOut, L"│", 1, &recnum, NULL);
			for (i2 = 0; i2 < WIDTH; i2++) {
				if (conblock[i2][i]) {
					SetConsoleTextAttribute(sOut, conblock[i2][i]);
					WriteConsole(sOut, L"□", 1, &recnum, NULL);
				}
				else {
					SetConsoleTextAttribute(sOut, SCOLOR_BACKGROUND);
					WriteConsole(sOut, L"  ", 2, &recnum, NULL);
				}
			}
			SetConsoleTextAttribute(sOut, SCOLOR_FORM);
			WriteConsole(sOut, L"│", 1, &recnum, NULL);
		}
		SetConsoleCursorPosition(sOut, (COORD) { 0, HEIGTH + 1 });
		WriteConsole(sOut, L"└", 1, &recnum, NULL);
		record = GetBlockInfo(blockpos, curblock) >> 8;
		while (record--) WriteConsole(sOut, L"─", 1, &recnum, NULL);
		SetConsoleTextAttribute(sOut, SCOLOR_DOWNPLACE);
		record = GetBlockInfo(blockpos, curblock) & 0xff;
		while (record--) WriteConsole(sOut, L"─", 1, &recnum, NULL);
		SetConsoleTextAttribute(sOut, SCOLOR_FORM);
		record = GetBlockInfo(blockpos, curblock);
		record = WIDTH - (record >> 8) - record & 0xff;
		while (record--) WriteConsole(sOut, L"─", 1, &recnum, NULL);
		WriteConsole(sOut, L"┘", 1, &recnum, NULL);
		SetConsoleTextAttribute(sOut, blockcolor);
		for (i2 = blockpos.Y + 3; i2 >= blockpos.Y; i2--) {
			if (i2 < 0) break;
			for (i = blockpos.X + 3; i >= blockpos.X; i--) {
				if (curblock & ((0x8000 >> ((i2 - blockpos.Y) * 4) >> (i - blockpos.X)))) {
					SetConsoleCursorPosition(sOut, (COORD) { 2 * i + 2, i2 + 1 });
					WriteConsole(sOut, L"□", 1, &recnum, NULL);
				}
			}
		}
	}
	SetConsoleActiveScreenBuffer(sOut);
	FlushFileBuffers(sOut);
	exchange(hOut, sOut, HANDLE);
}
void spawnrand() {
	curblock = nextblock;
	blockcolor = nextblockcolor;
	nextblock = randblock();
	nextblockcolor = randcolor();
	blockpos.X = START;
	if (curblock & 0x000f) blockpos.Y = -4;
	else if (curblock & 0x00f0) blockpos.Y = -3;
	else if (curblock & 0x0f00) blockpos.Y = -2;
	else if (curblock & 0xf000) blockpos.Y = -1;
}

int abletoset(unsigned short block, COORD blockpos) {
	if (block & 0x000f) if (blockpos.Y >= HEIGTH - 3) return 0;
	if (block & 0x00f0) if (blockpos.Y >= HEIGTH - 2) return 0;
	if (block & 0x0f00) if (blockpos.Y >= HEIGTH - 1) return 0;
	if (block & 0xf000) if (blockpos.Y >= HEIGTH) return 0;
	if (block & 0x8888) { if (blockpos.X < 0) return 0; if (blockpos.X >= WIDTH) return 0; }
	if (block & 0x4444) { if (blockpos.X < -1) return 0; if (blockpos.X >= WIDTH - 1) return 0; }
	if (block & 0x2222) { if (blockpos.X < -2) return 0; if (blockpos.X >= WIDTH - 2) return 0; }
	if (block & 0x1111) { if (blockpos.X < -3) return 0; if (blockpos.X >= WIDTH - 3) return 0; }

	short x, y;
 	for (y = blockpos.Y + 3; y >= blockpos.Y; y--) {
		if (y < 0) break;
		for (x = blockpos.X + 3; x >= blockpos.X; x--) {
			if (block & ((0x8000 >> ((y - blockpos.Y) * 4) >> (x - blockpos.X)))) {
				if (conblock[x][y]) return 0;
			}
		}
	}
	return 1;
}
void layblock() {
	short x, y;
	for (y = blockpos.Y + 3; y >= blockpos.Y; y--) {
		if (y < 0)break;
		for (x = blockpos.X + 3; x >= blockpos.X; x--) {
			if (curblock & ((0x8000 >> ((y - blockpos.Y) * 4) >> (x - blockpos.X)))) {
				conblock[x][y] = blockcolor;
			}
		}
	}
}
//用户操作
int UserOP(INPUT_RECORD inp) {
	unsigned short tmpblock;
	switch (inp.EventType) {
	case KEY_EVENT:
		if (!inp.Event.KeyEvent.bKeyDown) break;
		switch (inp.Event.KeyEvent.wVirtualKeyCode) {
		case VK_UP:
			if (abletoset(tmpblock = aclockwise(curblock, 1), blockpos))curblock = tmpblock;
			break;
		case VK_LEFT:
			if (abletoset(curblock, (COORD) { blockpos.X - 1, blockpos.Y })) blockpos.X--;
			break;
		case VK_RIGHT:
			if (abletoset(curblock, (COORD) { blockpos.X + 1, blockpos.Y })) blockpos.X++;
			break;
		case VK_DOWN:
			if (abletoset(curblock, (COORD) { blockpos.X, blockpos.Y + 1 }))blockpos.Y++;
			else { layblock();  return 1; }
			break;
		case VK_SPACE:
			while (abletoset(curblock, (COORD) { blockpos.X, ++blockpos.Y }));
			--blockpos.Y;
			layblock();
			return 1;
		case VK_ESCAPE:
			return 2;
		}
		switch (inp.Event.KeyEvent.uChar.UnicodeChar) {
		case 3:
			break;
		case 'p':
		case 'P':
			WaitForContinue();
			break;
		}
		repaint();
		break;
	case FOCUS_EVENT:
		if (!inp.Event.FocusEvent.bSetFocus) WaitForContinue();
		break;
	}
	return 0;//继续执行掉落操作
}
BOOL isContinue(unsigned short block, COORD blockpos) {
	if (block & 0xf000) if (blockpos.Y <= 0) return 0;
	if (block & 0x0f00) if (blockpos.Y <= -1) return 0;
	if (block & 0x00f0) if (blockpos.Y <= -2) return 0;
	if (block & 0x000f) if (blockpos.Y <= -3) return 0;
}
#pragma warning(disable:28159)
void RunTimer() {
	int gamestate = 1;
	do {
		spawnrand();
		repaint();
		blocktime++;//方块生成次数+1
		do {
			unsigned int timer = GetTickCount(); int quickloop = 0;
			long sleeptime = SLEEPCLOCK;
			do {
				unsigned int evenum;
				if (!GetNumberOfConsoleInputEvents(hIn, &evenum)) {
					printf("Error Happened!%d", GetLastError());
					exit(1);
				}
				while (evenum--) {
					INPUT_RECORD inp; unsigned recnum;
					ReadConsoleInput(hIn, &inp, 1, &recnum);
					if (quickloop = UserOP(inp)) break;
				}
				if (quickloop == 2) return;
				if (quickloop) break;
				Sleep(10);
			} while (GetTickCount() <= timer + sleeptime);
			if (quickloop) break;
			//掉落
			if (abletoset(curblock, (COORD) { blockpos.X, blockpos.Y + 1 }))blockpos.Y++;
			else { layblock(); break; }
			repaint();
		} while (abletoset(curblock, blockpos));
		//判断满行与游戏结束
		flushfullline(TRUE);
		gamestate = isContinue(curblock, blockpos);
	} while (gamestate);
}
#pragma warning(default:28195)
