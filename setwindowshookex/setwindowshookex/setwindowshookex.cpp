#include "stdafx.h"
#include <Windows.h>
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
	/******************************************************************************
	* Step 1: Set the WH_GETMESSAGE hook in the target window
	******************************************************************************/

	// Load the DLL that we'll be injecting
	auto dll = LoadLibrary(L"dllinject.dll");
	if (dll == NULL) 
	{ 
		return printError("The DLL could not be found\n"); 
	}

	// Get the address of the hook function
	auto hookAddress = (HOOKPROC)GetProcAddress(dll, "getMsgProc");
	if (hookAddress == NULL)
	{
		return printError("getMsgProc not found.\n"); 
	}

	// Get the handle of the window we'll be hooking
	auto targetWnd = FindWindow(NULL, L"New Tab - Google Chrome");
	if (targetWnd == NULL) 
	{ 
		return printError("Couldn't target window\n"); 
	}

	// Now get it's process and thread ID
	DWORD procID;
	auto threadID = GetWindowThreadProcessId(targetWnd, &procID);

	// Set the hook
	auto hookHandle = SetWindowsHookEx(WH_GETMESSAGE, hookAddress, dll, threadID);
	if (hookHandle == NULL) 
	{ 
		return printError("WH_GETMESSAGE could not be hooked\n"); 
	}


	/******************************************************************************
	* Step 2: Define a zone for the target window
	******************************************************************************/

	// Set the max dimensions of window
	//POINT maxSize{ 2200, 1200 };
	//POINT maxPosition{ 500, 100 };

	//if (!SaveZoneSizeAndOrigin(targetWnd, maxSize, maxPosition)) 
	//{ 
	//	return printError("Couldn't add zone property to window\n"); 
	//}


	/******************************************************************************
	* Step 3: Send the ADD_SUBCLASS message to the target window
	******************************************************************************/

	if (!PostMessage(targetWnd, WM_USER+666, (WPARAM)targetWnd, 0xFF))
	{
		printf("Couldn't trigger subclassing function\n");
	}

	// Unhook the function.
	printf("Program successfully hooked.\nPress enter to unhook the function and stop the program.\n");
	getchar();


	// Clean-up
	UnhookWindowsHookEx(hookHandle);
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

	memcpy(&rawData, windowSizeData.data(), sizeof windowSizeData);
	if (!SetPropW(window, PropertyZoneSizeID, rawData))
	{
		return false;
	}

	memcpy(&rawData, windowOriginData.data(), sizeof windowOriginData);
	if (!SetPropW(window, PropertyZoneOriginID, rawData))
	{
		return false;
	}

	return true;
}
