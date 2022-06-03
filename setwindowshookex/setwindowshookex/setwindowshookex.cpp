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

	HOOKPROC callWndProcAddr = (HOOKPROC)GetProcAddress(dll, "callWndProcRet");
	if (callWndProcAddr == NULL) { return printError("callWndProcRet not found.\n"); }

	// Hook the function.
	handle = SetWindowsHookEx(WH_KEYBOARD, keyboardProcAddr, dll, threadID);
	if (handle == NULL) { printf("WH_KEYBOARD could not be hooked.\n"); }

	handle = SetWindowsHookEx(WH_CALLWNDPROCRET, callWndProcAddr, dll, threadID);
	if (handle == NULL) { printf("WH_WNDPROCRET could not be hooked.\n"); }


	DWORD_PTR result;
	SendMessageTimeout(HWND_BROADCAST, WM_NULL, 0, 0, SMTO_ABORTIFHUNG, 10, &result);
		
	//// Unhook the function.
	printf("Program successfully hooked.\nPress enter to unhook the function and stop the program.\n");
	getchar();

	UnhookWindowsHookEx(handle);

	return 0;
}



