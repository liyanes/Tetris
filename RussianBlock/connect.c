#include "connect.h"
#include "global.h"
#include <Windows.h>
#include <process.h>
#include <stdio.h>
#pragma comment(lib,"ws2_32.lib")

SOCKET sock;
static SOCKET tmpsock;
char recvmsg[MSG_LEN + 1];
static unsigned _stdcall connect_player_s(void* pArguments) {
	memset(recvmsg, 0, sizeof(char) * MSG_LEN);
	if (recv(sock, recvmsg, MSG_LEN, 0) == SOCKET_ERROR) {
		if ((errno = WSAGetLastError()) == 10057) {
			__asm int 3;
		}
		return 0;
	}
	else {
		return 1;
	}
}

static unsigned _stdcall connect_player_sac(void* pArguments) {
	int size = sizeof(SOCKADDR_IN);
	if ((sock = accept(tmpsock, (SOCKADDR*)pArguments, &size)) == SOCKET_ERROR) {
		errno = WSAGetLastError();
		return 0;
	}
	else {
		return 1;
	}
}

struct in_addr* GetLocalIP()
{
	char szHostName[MAX_PATH] = { 0 };
	int nRetCode;
	nRetCode = gethostname(szHostName, MAX_PATH);
	PHOSTENT hostinfo;
	if (nRetCode != 0) {
		return NULL;
	}
	hostinfo = gethostbyname(szHostName);
	return (struct in_addr*) * hostinfo->h_addr_list;
}
#pragma warning(disable:6387)
#pragma warning(disable:6001)
int connect_player(BOOL isServer, struct in_addr LocalAddr, BOOL (*deal)(char* message, unsigned int runstate), BOOL (*state)(INPUT_RECORD inp,unsigned int runstate), BOOL (*timer)()) {
	memset(recvmsg, 0, MSG_LEN + 1);
	tmpsock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

	if (tmpsock == SOCKET_ERROR) {
		errno = WSAGetLastError();
		closesocket(tmpsock);
		return 2;
	}

	SOCKADDR_IN addr = { 0 };
	addr.sin_family = AF_INET;
	addr.sin_addr = LocalAddr;
	addr.sin_port = htons(SOCKET_PORT);

	if (isServer) {
		if (bind(tmpsock, (SOCKADDR*)&addr, sizeof(SOCKADDR_IN)) == SOCKET_ERROR) {
			errno = WSAGetLastError();
			closesocket(tmpsock);
			return 2;
		}
		if (listen(tmpsock, 1) == SOCKET_ERROR) {
			closesocket(tmpsock);
			return 3;
		}
		SOCKADDR_IN saddr = { 0 };
		HANDLE hTh = (HANDLE)_beginthreadex(NULL, 0, connect_player_sac, &saddr, 0, NULL);
		if (!hTh) {
			closesocket(tmpsock);
			return 3;
		}
		while (1) {
			unsigned int evenum; unsigned int ret;
			GetNumberOfConsoleInputEvents(hIn, &evenum);
			while (evenum--) {
				INPUT_RECORD inp;
				unsigned int recnum;
				ReadConsoleInput(hIn, &inp, 1, &recnum);
				if (!state(inp,RUNSTATE_ACCEPT)) {
					CloseHandle(hTh);
					closesocket(tmpsock);
					return 0;
				}
			}
			GetExitCodeThread(hTh, &ret);
			if (ret != STILL_ACTIVE) {
				if (ret) {
					break;//成功连接,退出循环
				}
				else {
					return 3;
				}
			}
			Sleep(10);
		}
		CloseHandle(hTh);
		send(sock, "connected", strlen("connected"), 0);
		hTh = (HANDLE)_beginthreadex(NULL, 0, connect_player_s, NULL, 0, NULL);
		if (!hTh) {
			closesocket(tmpsock);
			closesocket(sock);
			return 3;
		}
		while (1) {
			unsigned int evenum; unsigned int ret;
			GetNumberOfConsoleInputEvents(hIn, &evenum);
			while (evenum--) {
				INPUT_RECORD inp;
				unsigned int recnum;
				ReadConsoleInput(hIn, &inp, 1, &recnum);
				if (!state(inp, RUNSTATE_CONNECTED)) {
					CloseHandle(hTh);
					closesocket(tmpsock);
					closesocket(sock);
					return 0;
				}
			}
			GetExitCodeThread(hTh, &ret);
			if (ret != STILL_ACTIVE) {
				CloseHandle(hTh);
				if (!ret) {
					closesocket(sock);
					closesocket(tmpsock);
					return 4;
				}
				if (!deal(recvmsg,RUNSTATE_CONNECTED)) {
					closesocket(sock);
					return 0;
				}
				hTh = (HANDLE)_beginthreadex(NULL, 0, connect_player_s, NULL, 0, NULL);
				if (!hTh) {
					closesocket(sock);
					closesocket(tmpsock);
					return 3;
				}
			}
			if (!timer()) {
				closesocket(sock);
				return 0;
			}
			Sleep(10);
		}
	}
	else {
		sock = tmpsock;
		if (connect(sock, (SOCKADDR*)&addr, sizeof(SOCKADDR_IN)) == SOCKET_ERROR) {
			errno = WSAGetLastError();
			closesocket(sock);
			return 2;
		}
		INPUT_RECORD inp;
		HANDLE hTh = (HANDLE)_beginthreadex(NULL, 0, &connect_player_s, NULL, 0, NULL);
		if (!hTh) {
			closesocket(sock);
			return 3;
		}
		send(sock, "connected", strlen("connected"), 0);
		while (1) {
			unsigned int evenum;unsigned int ret;
			GetNumberOfConsoleInputEvents(hIn, &evenum);
			while (evenum--) {
				unsigned int recnum;
				ReadConsoleInput(hIn, &inp, 1, &recnum);
				if (!state(inp,RUNSTATE_CONNECTED)) {
					CloseHandle(hTh);
					closesocket(sock);
					return 0;
				}
			}
			GetExitCodeThread(hTh, &ret);
			if (ret != STILL_ACTIVE) {
				if (ret == 0) {//连接错误
					closesocket(sock);
					return 3;
				}
				if (!deal(recvmsg,RUNSTATE_CONNECTED)) {
					closesocket(sock);
					return 0;
				}
				CloseHandle(hTh);
				hTh = (HANDLE)_beginthreadex(NULL, 0, &connect_player_s, NULL, 0, NULL);
				if (!hTh) {
					closesocket(sock);
					return 3;
				}
			}
			if (!timer()) {
				closesocket(sock);
				return 0;
			}
			Sleep(10);
		}
	}
	return 0;
}
#pragma warning(default:6387)
#pragma warning(default:6001)