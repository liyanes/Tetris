#include "connect.h"
#include "global.h"
#include <Windows.h>
#include <process.h>
#include <stdio.h>
#pragma comment(lib,"ws2_32.lib")

SOCKET sock;
static SOCKET tmpsock;
char recvmsg[MSG_LEN + 1];
HANDLE recvstate = NULL;
HANDLE recvcont = NULL;
HANDLE recvwait = NULL;
static unsigned _stdcall connect_player_s(void* pArguments) {
	/*
	while (1) {
		if (recvcont != NULL) WaitForSingleObject(recvcont, INFINITE);
		recvstate = CreateMutex(0, FALSE, L"RussianBlock-RecvState");
		if (!recvstate) {
			errno = GetLastError();
			return 0;
		}
		ReleaseMutex(recvwait);
		if (recv(sock, recvmsg, MSG_LEN, 0) == SOCKET_ERROR) {
			errno = WSAGetLastError();
			ReleaseMutex(recvstate);
			return 0;
		}
		recvcont = CreateMutex(0, FALSE, L"RussianBlock-RecvFinished");
		if (!recvcont) {
			errno = GetLastError();
			return 0;
		}
		ReleaseMutex(recvstate);
	}*/
	switch (recv(sock, recvmsg, MSG_LEN, 0)) {
	case SOCKET_ERROR:
		errno = WSAGetLastError();
		return -1;
	case 0:
		return 0;
	}
	return 1;
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
int connect_player(BOOL isServer, struct in_addr LocalAddr, BOOL (*deal)(char* message, unsigned int runstate), BOOL (*state)(INPUT_RECORD inp,unsigned int runstate), BOOL (*timer)(BOOL)) {
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
	if (LocalAddr.S_un.S_un_w.s_w1 == 0xA8C0)
		addr.sin_port = htons(SOCKET_PORT);
	else addr.sin_port = htons(49157);

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
		timer(TRUE);
		//recvwait = CreateMutex(0, FALSE, NULL);
		hTh = (HANDLE)_beginthreadex(NULL, 0, connect_player_s, NULL, 0, NULL);
		if (!hTh) {
			closesocket(tmpsock);
			closesocket(sock);
			return 3;
		}
		//WaitForSingleObject(recvwait,INFINITE);
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
			if (!timer(FALSE)) {
				closesocket(sock);
				return 0;
			}
			/*
			if ((ret = WaitForSingleObject(recvstate, 10)) == WAIT_TIMEOUT){
				int thret;
				GetExitCodeThread(hTh, &thret);
				if (thret != STILL_ACTIVE) {
					CloseHandle(hTh);
					CloseHandle(recvstate);
					closesocket(sock);
					closesocket(tmpsock);
					return 4;
				}
			}
			else {
				ret = GetLastError();
				if (recvstate == NULL || recvcont == NULL) {
					CloseHandle(hTh);
					CloseHandle(recvstate);
					CloseHandle(recvcont);
					closesocket(sock);
					closesocket(tmpsock);
					return 5;
				}
				if (!deal(recvmsg, RUNSTATE_CONNECTED)) {
					CloseHandle(hTh);
					CloseHandle(recvstate);
					CloseHandle(recvcont);
					closesocket(sock);
					closesocket(tmpsock);
					return 0;
				}
				recvwait = CreateMutex(0, FALSE, L"RussianBlock-Wait");
				ReleaseMutex(recvcont);
			}*/
			if (WaitForSingleObject(hTh, 10) != WAIT_TIMEOUT) {
				GetExitCodeThread(hTh, &ret);
				CloseHandle(hTh);
				switch (ret) {
				case 0:
					closesocket(sock);
					closesocket(tmpsock);
					return 6;
				case -1:
					closesocket(sock);
					closesocket(tmpsock);
					return 4;
				}
				if (!deal(recvmsg, RUNSTATE_CONNECTED)) {
					closesocket(sock);
					closesocket(tmpsock);
					return 0;
				}
				hTh = (HANDLE)_beginthreadex(NULL, 0, connect_player_s, NULL, 0, NULL);
				if (!hTh) {
					closesocket(tmpsock);
					closesocket(sock);
					return 3;
				}
			}
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
		//recvwait = CreateMutex(0, FALSE, NULL);
		HANDLE hTh = (HANDLE)_beginthreadex(NULL, 0, &connect_player_s, NULL, 0, NULL);
		if (!hTh) {
			closesocket(sock);
			return 3;
		}
		timer(TRUE);
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
			if (!timer(FALSE)) {
				closesocket(sock);
				return 0;
			}
			/*
			WaitForSingleObject(recvwait, INFINITE);
			CloseHandle(recvwait);
			if ((ret = WaitForSingleObject(recvstate, 10)) == WAIT_TIMEOUT) {
				int thret;
				GetExitCodeThread(hTh, &thret);
				if (thret != STILL_ACTIVE) {
					CloseHandle(hTh);
					CloseHandle(recvstate);
					closesocket(sock);
					return 4;
				}
			}
			else {
				ret = GetLastError();
				if (recvstate == NULL || recvcont == NULL) {
					CloseHandle(hTh);
					CloseHandle(recvstate);
					CloseHandle(recvcont);
					closesocket(sock);
					return 5;
				}
				if (!deal(recvmsg, RUNSTATE_CONNECTED)) {
					CloseHandle(hTh);
					CloseHandle(recvstate);
					CloseHandle(recvcont);
					closesocket(sock);
					return 0;
				}
				recvwait = CreateMutex(0, FALSE, L"RussianBlock-Wait");
				ReleaseMutex(recvcont);
			}*/
			if (WaitForSingleObject(hTh, 10) != WAIT_TIMEOUT) {
				GetExitCodeThread(hTh, &ret);
				CloseHandle(hTh);
				switch (ret) {
				case 0:
					closesocket(sock);
					return 6;
				case -1:
					closesocket(sock);
					return 4;
				}
				if (!deal(recvmsg, RUNSTATE_CONNECTED)) {
					closesocket(sock);
					return 0;
				}
				hTh = (HANDLE)_beginthreadex(NULL, 0, connect_player_s, NULL, 0, NULL);
				if (!hTh) {
					closesocket(sock);
					return 3;
				}
			}
		}
	}
	return 0;
}
#pragma warning(default:6387)
#pragma warning(default:6001)