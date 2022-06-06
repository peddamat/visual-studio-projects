#include "stdafx.h"
#include <stdio.h>
#include <windows.h>
#include <commctrl.h>
#include <array>

#pragma comment(lib, "comctl32.lib")

HWND m_hwnd = NULL;
const wchar_t PropertyZoneSizeID[] = L"FancyZones_ZoneSize";
const wchar_t PropertyZoneOriginID[] = L"FancyZones_ZoneOrigin";

LRESULT CALLBACK hookWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam, UINT_PTR uIdSubclass, DWORD_PTR dwRefData);
bool AddHook(HWND hwnd);
bool RemoveHook(HWND hwnd);
void LOG(const char* message);
BOOL GetZoneSizeAndOrigin(HWND window, POINT& zoneSize, POINT& zoneOrigin) noexcept;


/******************************************************************************
* DLL Entrypoint
******************************************************************************/
INT APIENTRY DllMain(HMODULE hDLL, DWORD Reason, LPVOID Reserved) {
	switch (Reason) {
	case DLL_PROCESS_DETACH:
		// If the hook is still in place, unhook it
		if (m_hwnd != NULL)
		{
			RemoveHook(m_hwnd);
		}
		break;
	}

	return TRUE;
}


/******************************************************************************
* FancyZones Window Process Subclass
******************************************************************************/
LRESULT CALLBACK hookWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam, UINT_PTR uIdSubclass, DWORD_PTR dwRefData)
{
	// Only process windows that are in a zone
	if (!GetPropW(hwnd, PropertyZoneSizeID))
		goto end;

	switch (msg)
	{

	/**
	  *	WM_WINDOWPOSCHANGING events are generated when a window is being sized
	  * or moved.  If the window is maximized, we override the default behavior 
	  * and manually define the window's size and position.
	  */
	case WM_WINDOWPOSCHANGING:
		LOG("Entered WM_WINDOWPOSCHANGING: \n");

		if (WS_MAXIMIZE & GetWindowLong(hwnd, GWL_STYLE))
		{
			POINT zoneSize = { 0,0 };
			POINT zoneOrigin = { 0,0 };

			if (GetZoneSizeAndOrigin(hwnd, zoneSize, zoneOrigin))
			{
				auto windowpos = reinterpret_cast<WINDOWPOS*>(lParam);

				windowpos->x = zoneOrigin.x;
				windowpos->y = zoneOrigin.y;

				windowpos->cx = zoneSize.x;
				windowpos->cy = zoneSize.y;

				LOG(" window moved\n");
				return 0;
			}
			else
			{
				LOG(" error getting window property!\n");
			}
		}
		break;

	}

	end:
	return DefSubclassProc(hwnd, msg, wParam, lParam);
}


/******************************************************************************
* Window Process Hook Functions
******************************************************************************/
bool AddHook(HWND hwnd)
{
	LOG("Hooking wndProc: ");

	if (!SetWindowSubclass(hwnd, &hookWndProc, 1, 0)) {
		LOG(" Error!\n");
		return FALSE;
	}

	// Save handle to subclassed window
	m_hwnd = hwnd;

	LOG(" Success!\n");
	return TRUE;
}

bool RemoveHook(HWND hwnd)
{
	LOG("Unhooking wndProc: ");

	if (!RemoveWindowSubclass(hwnd, &hookWndProc, 1)) {
		LOG(" Error!\n");
		return FALSE;
	}

	LOG(" Success!\n");
	return TRUE;
}

extern "C" __declspec(dllexport) 
LRESULT CALLBACK getMsgProc(int code, WPARAM wParam, LPARAM lParam) {
	if (code < 0) // Do not process
		goto end;

	auto msg = reinterpret_cast<MSG*>(lParam);
	if (msg->message == WM_USER + 666)
	{
		auto hwnd = reinterpret_cast<HWND>(msg->wParam);
		AddHook(hwnd);
	}
	else if (msg->message == WM_USER+667)
	{
		auto hwnd = reinterpret_cast<HWND>(msg->wParam);
		RemoveHook(hwnd);
	}

	end:
	return(CallNextHookEx(NULL, code, wParam, lParam));
}


/******************************************************************************
* Utility functions
******************************************************************************/
BOOL GetZoneSizeAndOrigin(HWND window, POINT &zoneSize, POINT &zoneOrigin) noexcept
{
	// Retrieve the zone size
    auto zsProp = GetPropW(window, PropertyZoneSizeID);
	if (!zsProp)
		return false;

	std::array<int, 2> zsArray;
	memcpy(zsArray.data(), &zsProp, sizeof zsArray);

	zoneSize.x = static_cast<float>(zsArray[0]);
	zoneSize.y = static_cast<float>(zsArray[1]);

	// Retrieve the zone position
    auto zoProp = GetPropW(window, PropertyZoneOriginID);
	if (!zoProp)
		return false;

	std::array<int, 2> zoArray;
	memcpy(zoArray.data(), &zoProp, sizeof zoArray);

	zoneOrigin.x = static_cast<float>(zoArray[0]);
	zoneOrigin.y = static_cast<float>(zoArray[1]);

	// {width, height}
	//DPIAware::Convert(MonitorFromWindow(window, MONITOR_DEFAULTTONULL), windowWidth, windowHeight);

	return true;
}


void LOG(const char* message)
{	
	FILE* file;
	fopen_s(&file, "c:\\users\\me\\debug\\dllmain.txt", "a+");

	fprintf(file, message);

	fclose(file);
}
