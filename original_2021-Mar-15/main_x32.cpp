#include <winsock2.h>
#include <ws2tcpip.h>
#include "windows.h"
#include "Shlobj.h"
#include "shlwapi.h"
#include <cstdio>
#pragma comment(lib,"Shlwapi.lib")
#pragma comment(lib,"Ws2_32.lib")

#define FULLPAGE 4096
#define MEGABYTE 1048576

char shellcodefilename[] = "\\shellcode";

WSADATA wsaData;				

unsigned long long kd_strlen(const char*);
char* kd_strcat(char*, const char*);
void kd_memcpy(void*, void*, unsigned long long);
void kd_zeroMemory(void *, unsigned long long);

int main(int argc, char ** argv)
{
	WSAStartup(MAKEWORD(2, 2), &wsaData);		// This was only included so that the ws2_32.dll library would be loaded; easier to set breakpoints!
	BOOL bResult = 0;							// Windows API result check variable
	char currentDirectory[1024];				// The current directory path
	char fullPath[1040];						// The full path to the shellcode file
	HANDLE hSCFile;								// Handle to shellcode file
	HANDLE hThread;								// Handle to the shellcode's thread
	DWORD threadID = 0;							// This will receive the shellcode's thread ID later
	DWORD fsLow = 0;							// Low 32-bits of file size
	DWORD fsHigh = 0;							// High 32-bits of file size
	DWORD vaSize = 0;							// Size of virtual allocation
	char* scBuffer;								// Pointer for the shellcode payload
	unsigned long long scBuffAddr;				// Shellcode buffer (as an integer)

	// Zero the memory of the path buffers above
	kd_zeroMemory(currentDirectory, 1024);
	kd_zeroMemory(fullPath, 1040);

	// Check for admin rights
	bResult = IsUserAnAdmin();
	if (bResult == FALSE) printf("[!] Warning: You may not be running with admin rights. It is recommended that you do so when analyzing shellcode :)\n\n");

	// Get the current working directory and print it
	GetCurrentDirectoryA(1024, currentDirectory);
	printf("[*] The current \"working\" directory is: %s\n\n", currentDirectory);

	// Check if the shellcode file exists
	memcpy(fullPath, currentDirectory, strlen(currentDirectory));
	kd_strcat(fullPath, shellcodefilename);
	bResult = PathFileExistsA(fullPath);
	if (bResult == TRUE) printf("[*] File found: %s\n\n", fullPath);
	else { printf("[!] File not found: %s\n\n", fullPath); return 1; }

	// Get the size of the shellcode file
	hSCFile = CreateFileA(fullPath, GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE, 0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
	fsLow = GetFileSize(hSCFile, &fsHigh);

	// Sanity check based on file size
	if (fsHigh > 0) { printf("[!] Sanity check error: There's no way the shellcode payload is that large!\n\n"); return 2; }
	printf("[*] Size of shellcode is %u bytes\n\n", fsLow);

	// Allocate space for the shellcode and zero out the space
	if (fsLow > FULLPAGE) vaSize = (fsLow + (FULLPAGE - (fsLow % FULLPAGE)));
	else vaSize = FULLPAGE;
	scBuffer = (char *)VirtualAlloc(0, vaSize, MEM_RESERVE | MEM_COMMIT, PAGE_EXECUTE_READWRITE);
	scBuffAddr = (unsigned int)scBuffer;
	kd_zeroMemory(scBuffer, vaSize);
	printf("[*] Shellcode buffer space of %u bytes allocated at address 0x%X\n\n", vaSize, (unsigned int)scBuffer);

	// Ensure the file pointer is at the beginning of the file, then read the shellcode into memory
	DWORD bytesRead = 0;
	SetFilePointer(hSCFile, 0, 0, FILE_BEGIN);
	bResult = ReadFile(hSCFile, scBuffer, fsLow, &bytesRead, 0);
	if (bResult == TRUE) printf("[*] Successfully read %u bytes into buffer!\n\n", fsLow);
	else printf("[!] Failed to read data from shellcode file!\n\n");

	// Create a suspended thread on the shellcode, adding the shellcode's location as an argument (in case it's needed)
	if (scBuffer != 0) {
		hThread = CreateThread(0, MEGABYTE, (LPTHREAD_START_ROUTINE)scBuffer, &scBuffAddr, CREATE_SUSPENDED, &threadID);
		printf("[*] Thread ID %u (0x%X) was spawned to launch the shellcode; check it in the debugger!", threadID, threadID);
		if (hThread != 0) WaitForSingleObject(hThread, INFINITE);
	}

	// Release the shellcode buffer space
	VirtualFree(scBuffer, 0, MEM_RELEASE);

	// End the program
	return 0;
}


unsigned long long kd_strlen(const char* str)
{
	unsigned long long result = 0;
	while (str[result] != (char)0x00) result++;
	return result;
}

char* kd_strcat(char* dst, const char* src)
{
	unsigned long long dstlen = strlen(dst);
	unsigned long long srclen = strlen(src);
	char* dststart = dst + dstlen;
	for (unsigned long long i = 0; i < srclen; i++) dststart[i] = src[i];
	return dst;
}

void kd_memcpy(void * dst, const void * src, unsigned long long length)
{
	for (unsigned long long i = 0; i < length; i++) ((char*)dst)[i] = ((char*)src)[i];
}

void kd_zeroMemory(void* loc, unsigned long long size)
{
	for (unsigned long long i = 0; i < size; i++) ((char*)loc)[i] = (char)0x00;
}
