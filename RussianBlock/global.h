#pragma once
#include <Windows.h>

#define GAMENAME "Tetris"
#define GAMELNAME L"Tetris"

#define HEIGTH 20
#define WIDTH 16
#define JUMPDIFTIME 2000
#define SCOLOR_FORM FOREGROUND_GREEN
#define SCOLOR_DOWNPLACE FOREGROUND_RED
#define SCOLOR_BLOCK FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE | FOREGROUND_INTENSITY
#define SCOLOR_BACKGROUND 0

#define START ((WIDTH-4)/2)
#define SLEEPCLOCK (long)(1/(log(grade+3)/log(3) + pow(1.02,grade)) * JUMPDIFTIME)
#define exchange(a, b, type) {type exctmp = a; a = b; b = exctmp;}

extern CONSOLE_SCREEN_BUFFER_INFOEX sinfo;
extern WCHAR title[MAX_PATH];
extern DWORD mode;
extern COORD blockpos;
extern HANDLE hOut, sOut, hIn, hWindow;
extern LONG SavedLong;
extern BOOL isWinUpper;

extern unsigned short block_num;
extern unsigned short blocks[];

BOOL WINAPI ErrProc(DWORD CtrlType);
void Init();
void Fin(BOOL isErr);
unsigned short randblock();
unsigned char randcolor();
unsigned short aclockwise(unsigned short clock, unsigned short times);
unsigned int GetBlockInfo(COORD blockpos, unsigned short block);
void ClearScr(HANDLE hOut);
