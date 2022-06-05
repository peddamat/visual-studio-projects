#include "stdafx.h"
#include <Windows.h>
#include <conio.h>
#include <stdio.h>
#include <winuser.h>
#include <array>
#include <commctrl.h>

#pragma comment(lib, "comctl32.lib")

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
	// Load library in which we'll be hooking our functions.
	HMODULE dll = LoadLibrary(L"dllinject.dll");
	if (dll == NULL) { return printError("The DLL could not be found.\n"); }

	// Hook #4
	HOOKPROC procAddr4 = (HOOKPROC)GetProcAddress(dll, "getMsgProc");
	if (procAddr4 == NULL) { return printError("getMsgProc not found.\n"); }

	HWND targetWnd = reinterpret_cast<HWND>(0x47162C);
	//HWND targetWnd = FindWindow(L"Chrome_WidgetWin_1", NULL);
	//HWND targetWnd = FindWindow(L"Notepad", NULL);
	if (targetWnd == NULL) { return printError("Couldn't find app\n"); }

	DWORD procID;
	auto threadID = GetWindowThreadProcessId(targetWnd, &procID);

	HHOOK handle4 = SetWindowsHookEx(WH_GETMESSAGE, procAddr4, dll, threadID);
	if (handle4 == NULL) { return printError("WH_GETMESSAGE could not be hooked.\n"); }


	// Set the max dimensions of window
	POINT maxSize{ 2200, 1200 };
	POINT maxPosition{ 500, 100 };

	if (!SaveZoneSizeAndOrigin(targetWnd, maxSize, maxPosition)) { return printError("Couldn't set max dimensions\n"); }

	if (!PostMessage(targetWnd, WM_USER+666, (WPARAM)targetWnd, 0xFF))
	{
		printf("FUCKED\n");
	}

	// Unhook the function.
	printf("Program successfully hooked.\nPress enter to unhook the function and stop the program.\n");
	getchar();


	// Clean-up
	UnhookWindowsHookEx(handle4);
	//UnhookWindowsHookEx(handle3);
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
