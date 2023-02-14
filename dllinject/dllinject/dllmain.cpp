#include "stdafx.h"
#include <stdio.h>
#include <windows.h>
#include <commctrl.h>
#include <array>

#pragma comment(lib, "comctl32.lib")

static DWORD dwTlsIndex;

const wchar_t PropertyZoneSizeID[] = L"FancyZones_ZoneSize";
const wchar_t PropertyZoneOriginID[] = L"FancyZones_ZoneOrigin";

LRESULT CALLBACK hookWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam, UINT_PTR uIdSubclass, DWORD_PTR dwRefData);
bool AddHook(HWND hwnd);
bool RemoveHook();
void LOG(const char* message);
void LOG_MSG(const char* message, const char* args);
BOOL GetZoneSizeAndOrigin(HWND window, POINT& zoneSize, POINT& zoneOrigin) noexcept;


/******************************************************************************
* DLL Entrypoint
******************************************************************************/
INT APIENTRY DllMain(HMODULE hDLL, DWORD Reason, LPVOID Reserved) {
	LPVOID lpvData;

	switch (Reason) {
	case DLL_PROCESS_ATTACH:
		LOG_MSG("Attaching DLL to: %i\n", (char*)GetCurrentProcessId());
		// Allocate a Thread Local Storage (TLS) index, so each
		// hooked thread can independently store its HWND handle.
		if ((dwTlsIndex = TlsAlloc()) == TLS_OUT_OF_INDEXES)
		{ 
			LOG("Error allocating TLS index!\n");
			return FALSE;
		}
		break;
	case DLL_PROCESS_DETACH:
		LOG_MSG("Detaching DLL from: %i\n", (char*)GetCurrentProcessId());

		RemoveHook();
		TlsFree(dwTlsIndex);
		break;
	case DLL_THREAD_ATTACH:
		LOG_MSG("Attaching to thread: %i\n", (char *)GetCurrentThreadId());
		break;
	case DLL_THREAD_DETACH:
		LOG_MSG("Detaching from thread: %i\n", (char *)GetCurrentThreadId());

		//RemoveHook();
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

		RECT c;
		GetClientRect(hwnd, &c);

		MONITORINFO mi;

		mi.cbSize = sizeof(mi);

		if (GetMonitorInfo(MonitorFromWindow(hwnd, MONITOR_DEFAULTTONEAREST), &mi))
		{
			LOG("Monitor\n");
			LOG_MSG("width: %i\n", (char*)(mi.rcWork.right - mi.rcWork.left));
			LOG_MSG("height: %i\n", (char*)(mi.rcWork.bottom - mi.rcWork.top));

			LOG("Window\n");
			LOG_MSG("width: %i\n", (char *)(c.right - c.left));
			LOG_MSG("height: %i\n", (char *)(c.bottom - c.top));

			if (((mi.rcWork.right - mi.rcWork.left) == (c.right - c.left) &&
				((mi.rcWork.bottom - mi.rcWork.top) == (c.bottom - c.top))))
			{
				Beep(1000, 1000);
				break;
			}
		}

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

	case WM_DESTROY:
		LOG("Entered WM_DESTROY\n");
		RemoveHook();
		break;

	case WM_APP+667:
		LOG("Received wndProc Unhook Message\n");
		RemoveHook();
		return 1;
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
	LOG_MSG("Hooking wndProc of %#010X\n", (char *)hwnd);

	if (!SetWindowSubclass(hwnd, &hookWndProc, 1, 0)) {
		LOG(" Error!\n");
		return FALSE;
	}

	// Save handle to subclassed window
	if (!TlsSetValue(dwTlsIndex, hwnd))
	{
		LOG(" Error saving hwnd for thread!\n");
		return FALSE;
	}

	LOG(" Success!\n");
	return TRUE;
}

bool RemoveHook()
{
	HWND hwnd = (HWND)TlsGetValue(dwTlsIndex);
	LOG_MSG("Unhooking wndProc: %#010X\n", (char*)hwnd);

	if (hwnd == NULL)
	{
		LOG(" Skipping, thread wasn't hooked!\n");
		return TRUE;
	}

	if (!RemoveWindowSubclass(hwnd, &hookWndProc, 1)) {
		LOG(" Error, couldn't remove subclass!\n");
		return FALSE;
	}

	TlsSetValue(dwTlsIndex, 0);

	LOG(" Success!\n");
	return TRUE;
}

extern "C" __declspec(dllexport) 
LRESULT CALLBACK getMsgProc(int code, WPARAM wParam, LPARAM lParam) {
	if (code < 0) // Do not process
		goto end;

	// Only process the message after it's been removed from the message queue, 
	// otherwise we may accidentally process it more than once.
	if (wParam != PM_REMOVE)
		goto end; 

	auto msg = reinterpret_cast<MSG*>(lParam);
	if (msg->message == WM_APP + 666)
	{
		LOG("Received AddHook message\n");
		auto hwnd = reinterpret_cast<HWND>(msg->wParam);
		AddHook(hwnd);
	}
	else if (msg->message == WM_APP+667)
	{
		LOG("Received RemoveHook message\n");
		auto hwnd = reinterpret_cast<HWND>(msg->wParam);
		RemoveHook();
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
	return;
	FILE* file;
	fopen_s(&file, "c:\\users\\me\\debug\\dllmain.txt", "a+");

	fprintf(file, message);

	fclose(file);
}

void LOG_MSG(const char* message, const char* args)
{	
	return;
	char buf[100];
	sprintf(buf, message, args);
	LOG(buf);
}

