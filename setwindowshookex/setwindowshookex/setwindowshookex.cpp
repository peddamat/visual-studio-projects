#include "stdafx.h"
#include <Windows.h>
#include <conio.h>
#include <stdio.h>
#include <winuser.h>

int printError(char* msg)
{
	printf("%s\n", msg);
	getchar();
	return -1;
}

int _tmain(int argc, _TCHAR* argv[])
{
	LONG oldWndProc;
	HHOOK handle = NULL;

	// Load library in which we'll be hooking our functions.
	HMODULE dll = LoadLibrary(L"dllinject.dll");
	if (dll == NULL) { return printError("The DLL could not be found.\n"); }

	DWORD procID;
	HWND targetWnd = FindWindow(L"Notepad", NULL);
	if (targetWnd == NULL) { return printError("Couldn't find app\n"); }

	auto threadID = GetWindowThreadProcessId(targetWnd, &procID);

	// Get the address of the function inside the DLL.
	HOOKPROC keyboardProcAddr = (HOOKPROC)GetProcAddress(dll, "keyboardProc");
	if (keyboardProcAddr == NULL) { return printError("keyboardProc not found.\n"); }

	//HOOKPROC callWndProcAddr = (HOOKPROC)GetProcAddress(dll, "NewWndProc");
	//if (callWndProcAddr == NULL) { return printError("callWndProcRet not found.\n"); }

	// Hook the function.
	handle = SetWindowsHookEx(WH_KEYBOARD, keyboardProcAddr, dll, threadID);
	if (handle == NULL) { printf("WH_KEYBOARD could not be hooked.\n"); }

	//handle = SetWindowsHookEx(WH_CALLWNDPROCRET, callWndProcAddr, dll, threadID);
	//if (handle == NULL) { printf("WH_WNDPROCRET could not be hooked.\n"); }

	// //Name of the dll to be injected into the target process [ hereafter "Injectee" ] 
	// char* szInjectionDLLName  = "dllmain.dll";

	// DWORD dwProcessID;
	// 
	// //Find the main window of the Injectee
	// HWND hwnd = ::FindWindow(NULL,TEXT("Notepad") );

	// //Get the process hanlde of injectee
	// GetWindowThreadProcessId(hwnd, &dwProcessID ); 

	// //Open the process
	// HANDLE hProcess = ::OpenProcess(  PROCESS_ALL_ACCESS,false, dwProcessID);

	// //Allocate the memory in the Injectee's address space
	// void* baseAddress =  VirtualAllocEx( hProcess,NULL,
	//   strlen(szInjectionDLLName), MEM_COMMIT, PAGE_READWRITE );

	// //Write the name of the dll to be injected into the Injectee
	// SIZE_T nbBytesWritten = 0;
	// WriteProcessMemory( hProcess,baseAddress,szInjectionDLLName,
	//   strlen(szInjectionDLLName),&nbBytesWritten );

	// //Get the load libraries address
	// FARPROC pLoadLib = GetProcAddress( GetModuleHandle(TEXT("kernel32.dll") ), "LoadLibraryA");

	// //Create the remote thread in the target process
	// DWORD dwThreadID = 0;
	// HANDLE hRemoteThread = CreateRemoteThread(hProcess,NULL,0,
	//   (LPTHREAD_START_ROUTINE)pLoadLib, baseAddress,0, &dwThreadID );

	// //Wait for the remote thread till it finish its job.
	// //Otherwise, the next statement [ VirtualFreeEx ] will be called
	// //which release the address in the target process
	// //and when remote thread refers the address it leads to crashing!!!
	// WaitForSingleObject( hRemoteThread,INFINITE );

	// //Now free the memory allocated in the injectee's address space
	// VirtualFreeEx( hProcess, baseAddress,0, MEM_RELEASE );

	DWORD_PTR result;
	SendMessageTimeout(HWND_BROADCAST, WM_NULL, 0, 0, SMTO_ABORTIFHUNG, 10, &result);
		
	//// Unhook the function.
	printf("Program successfully hooked.\nPress enter to unhook the function and stop the program.\n");
	getchar();

	UnhookWindowsHookEx(handle);

	return 0;
}



