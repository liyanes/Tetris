#pragma once
#include <Windows.h>
#include "global.h"
#define SOCKET_IP "192.168.1.1"
#define SOCKET_PORT 14355

#define RUNSTATE_ACCEPT 0
#define RUNSTATE_CONNECTED 1

#define MSG_LEN ((7 + WIDTH * HEIGTH * 2 + 32 + 1)*sizeof(char))
extern SOCKET sock;
extern char recvmsg[MSG_LEN + 1];

struct in_addr* GetLocalIP();
int connect_player(BOOL isServer, struct in_addr, BOOL(*deal)(char* message, unsigned int runstate), BOOL(*state)(INPUT_RECORD inp,unsigned int runstate), BOOL(*timer)(BOOL));