#include "stdafx.h"
#include <Windows.h>
#include <conio.h>
#include <stdio.h>
#include <winuser.h>
#include <array>

int printError(char* msg)
{
	printf("%s\n", msg);
	getchar();
	return -1;
}

int _tmain(int argc, _TCHAR* argv[])
{
	HHOOK handle = NULL;
	HOOKPROC procAddr = NULL;
	HOOKPROC procAddr2 = NULL;
	HOOKPROC procAddr3 = NULL;

	// Load library in which we'll be hooking our functions.
	HMODULE dll = LoadLibrary(L"dllinject.dll");
	if (dll == NULL) { return printError("The DLL could not be found.\n"); }

	DWORD procID;
	HWND targetWnd = FindWindow(L"Chrome_WidgetWin_1", NULL);
	//HWND targetWnd = FindWindow(L"Notepad", NULL);
	if (targetWnd == NULL) { return printError("Couldn't find app\n"); }

	auto threadID = GetWindowThreadProcessId(targetWnd, &procID);

	// Hook #1
	//procAddr = (HOOKPROC)GetProcAddress(dll, "keyboardProc");
	//if (procAddr== NULL) { return printError("keyboardProc not found.\n"); }

	//handle = SetWindowsHookEx(WH_KEYBOARD, procAddr, dll, threadID);
	//if (handle == NULL) { printf("WH_KEYBOARD could not be hooked.\n"); }


	// Hook #2
	procAddr2 = (HOOKPROC)GetProcAddress(dll, "callWndProc");
	if (procAddr2 == NULL) { return printError("callWndProc not found.\n"); }

	handle = SetWindowsHookEx(WH_CALLWNDPROC, procAddr2, dll, threadID);
	if (handle == NULL) { printf("WH_WNDPROC could not be hooked.\n"); }


	// Hook #3
	procAddr3 = (HOOKPROC)GetProcAddress(dll, "callWndProcRet");
	if (procAddr3 == NULL) { return printError("callWndProcRet not found.\n"); }

	handle = SetWindowsHookEx(WH_CALLWNDPROCRET, procAddr3, dll, threadID);
	if (handle == NULL) { printf("WH_WNDPROCRET could not be hooked.\n"); }


	DWORD_PTR result;
	SendMessageTimeout(HWND_BROADCAST, WM_NULL, 0, 0, SMTO_ABORTIFHUNG, 10, &result);
		
	//// Unhook the function.
	printf("Program successfully hooked.\nPress enter to unhook the function and stop the program.\n");
	getchar();

	UnhookWindowsHookEx(handle);

	return 0;
}


const wchar_t PropertyRestoreSizeID[] = L"FancyZones_RestoreSize";

void SaveWindowSizeAndOrigin(HWND window) noexcept
{
    HANDLE handle = GetPropW(window, PropertyRestoreSizeID);
    if (handle)
    {
        // Size already set, skip
        return;
    }

    RECT rect;
    if (GetWindowRect(window, &rect))
    {
        float width = static_cast<float>(rect.right - rect.left);
        float height = static_cast<float>(rect.bottom - rect.top);
        float originX = static_cast<float>(rect.left);
        float originY = static_cast<float>(rect.top);

        //DPIAware::InverseConvert(MonitorFromWindow(window, MONITOR_DEFAULTTONULL), width, height);
        //DPIAware::InverseConvert(MonitorFromWindow(window, MONITOR_DEFAULTTONULL), originX, originY);

        std::array<int, 2> windowSizeData = { static_cast<int>(width), static_cast<int>(height) };
        std::array<int, 2> windowOriginData = { static_cast<int>(originX), static_cast<int>(originY) };
        HANDLE rawData;
        memcpy(&rawData, windowSizeData.data(), sizeof rawData);
        SetPropW(window, PropertyRestoreSizeID, rawData);
        //memcpy(&rawData, windowOriginData.data(), sizeof rawData);
        //SetPropW(window, PropertyRestoreOriginID, rawData);
    }
}

