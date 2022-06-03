#include "stdafx.h"
#include <Windows.h>
#include <conio.h>
#include <stdio.h>


int _tmain(int argc, _TCHAR* argv[])
{

	/*
	 * Load library in which we'll be hooking our functions.
	 */
	HMODULE dll = LoadLibrary(L"dllinject.dll");
	if (dll == NULL) {
		printf("The DLL could not be found.\n");
		getchar();
		return -1;
	}

	/*
	 * Get the address of the function inside the DLL.
	 */
	HOOKPROC addr = (HOOKPROC)GetProcAddress(dll, "meconnect");
	if (addr == NULL) {
		printf("The function was not found.\n");
		getchar();
		return -1;
	}

	/*
	 * Window name
	 */
	DWORD procID;
	HWND targetWnd = FindWindow(L"Notepad", NULL);
	if (targetWnd == NULL) {
		printf("Couldn't find app\n");
	}
	auto threadID = GetWindowThreadProcessId(targetWnd, &procID);

	/*
	 * Hook the function.
	 */
	HHOOK handle = SetWindowsHookEx(WH_KEYBOARD, addr, dll, threadID);
	if (handle == NULL) {
		printf("The KEYBOARD could not be hooked.\n");
	}

	/*
	 * Unhook the function.
	 */
	printf("Program successfully hooked.\nPress enter to unhook the function and stop the program.\n");
	getchar();

	UnhookWindowsHookEx(handle);

	return 0;
}



