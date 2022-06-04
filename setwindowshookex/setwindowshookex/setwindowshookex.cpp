#include "stdafx.h"
#include <Windows.h>
#include <conio.h>
#include <stdio.h>
#include <winuser.h>
#include <array>

const wchar_t PropertyZoneSizeID[] = L"FancyZones_ZoneSize";
const wchar_t PropertyZoneOriginID[] = L"FancyZones_ZoneOrigin";
BOOL SaveZoneSizeAndOrigin(HWND window, POINT maxSize, POINT maxPosition) noexcept;

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

	// Set the max dimensions of window
	POINT maxSize{ 2200, 1200 };
	POINT maxPosition{ 500, 0 };

	if (!SaveZoneSizeAndOrigin(targetWnd, maxSize, maxPosition)) { return printError("Couldn't set max dimensions\n"); }

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
	if (handle == NULL) { return printError("WH_WNDPROC could not be hooked.\n"); }


	// Hook #3
	procAddr3 = (HOOKPROC)GetProcAddress(dll, "callWndProcRet");
	if (procAddr3 == NULL) { return printError("callWndProcRet not found.\n"); }

	handle = SetWindowsHookEx(WH_CALLWNDPROCRET, procAddr3, dll, threadID);
	if (handle == NULL) { return printError("WH_WNDPROCRET could not be hooked.\n"); }


	DWORD_PTR result;
	SendMessageTimeout(HWND_BROADCAST, WM_NULL, 0, 0, SMTO_ABORTIFHUNG, 10, &result);
		
	//// Unhook the function.
	printf("Program successfully hooked.\nPress enter to unhook the function and stop the program.\n");
	getchar();

	// TODO: Clean-up added window property
	UnhookWindowsHookEx(handle);
	RemoveProp(targetWnd, PropertyZoneSizeID);
	RemoveProp(targetWnd, PropertyZoneOriginID);

	return 0;
}


BOOL SaveZoneSizeAndOrigin(HWND window, POINT zoneSize, POINT zoneOrigin) noexcept
{
    if (GetPropW(window, PropertyZoneSizeID) && GetPropW(window, PropertyZoneOriginID))
    {
        // Both props are already set, abort!
        return true;
    }

	//DPIAware::InverseConvert(MonitorFromWindow(window, MONITOR_DEFAULTTONULL), width, height);
	//DPIAware::InverseConvert(MonitorFromWindow(window, MONITOR_DEFAULTTONULL), originX, originY);

	std::array<int, 2> windowSizeData = { static_cast<int>(zoneSize.x), static_cast<int>(zoneSize.y) };
	std::array<int, 2> windowOriginData = { static_cast<int>(zoneOrigin.x), static_cast<int>(zoneOrigin.y) };

	HANDLE rawData;

	memcpy(&rawData, windowSizeData.data(), sizeof rawData);
	if (!SetPropW(window, PropertyZoneSizeID, rawData))
	{
		return false;
	}

	memcpy(&rawData, windowOriginData.data(), sizeof rawData);
	if (!SetPropW(window, PropertyZoneOriginID, rawData))
	{
		return false;
	}

	return true;
}
