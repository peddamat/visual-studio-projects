#include "stdafx.h"
#include <stdio.h>
#include <windows.h>
#include <commctrl.h>
#include <array>

#pragma comment(lib, "comctl32.lib")

BOOL m_hooked = FALSE;
extern "C" __declspec(dllexport) LRESULT CALLBACK hookWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam, UINT_PTR uIdSubclass, DWORD_PTR dwRefData);

HWND m_hwnd = NULL;
BOOL m_maximized = FALSE;
const wchar_t PropertyZoneSizeID[] = L"FancyZones_ZoneSize";
const wchar_t PropertyZoneOriginID[] = L"FancyZones_ZoneOrigin";
BOOL GetZoneSizeAndOrigin(HWND window, POINT& zoneSize, POINT& zoneOrigin) noexcept;


INT APIENTRY DllMain(HMODULE hDLL, DWORD Reason, LPVOID Reserved) {
	/* open file */
	FILE* file;
	fopen_s(&file, "c:\\users\\me\\debug\\dllmain.txt", "a+");

	switch (Reason) {
	case DLL_PROCESS_ATTACH:
		fprintf(file, "DLL attach function called.\n");
	break;
	case DLL_PROCESS_DETACH:
		fprintf(file, "DLL detach function called.\n");
		if (RemoveWindowSubclass(m_hwnd, &hookWndProc, 1)) {
			fprintf(file, "- wndProc unsubclassed!\n");
		}
		else {
			fprintf(file, "!! wndProc NOT ubsubclassed!\n");
		}

		//CreateThread(0, NULL, (LPTHREAD_START_ROUTINE)&deinit, NULL, NULL, NULL);
		break;
	case DLL_THREAD_ATTACH:
		fprintf(file, "DLL thread attach function called.\n");
		break;
	case DLL_THREAD_DETACH:
		fprintf(file, "DLL thread detach function called.\n");
		break;
	}

	/* close file */
	fclose(file);

	return TRUE;
}

extern "C" __declspec(dllexport) LRESULT CALLBACK getMsgProc(int code, WPARAM wParam, LPARAM lParam) {
	if (code < 0) // Do not process
		goto end;

	auto msg = reinterpret_cast<MSG*>(lParam);

	if (msg->message != WM_USER+666)
		goto end;

	FILE* file;
	fopen_s(&file, "c:\\users\\me\\debug\\dllmain.txt", "a+");
	fprintf(file, "Entered getMsgProc\n");

	HWND hwnd = reinterpret_cast<HWND>(msg->wParam);
	if (SetWindowSubclass(hwnd, &hookWndProc, 1, 0)) {
		fprintf(file, "- wndProc subclassed\n");
	}
	else {
		fprintf(file, "!! wndProc NOT subclassed\n");
	}

	// Save handle to subclassed window
	m_hwnd = hwnd;

	fclose(file);

	end:
	return(CallNextHookEx(NULL, code, wParam, lParam));
}

extern "C" __declspec(dllexport) LRESULT CALLBACK hookWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam, UINT_PTR uIdSubclass, DWORD_PTR dwRefData)
{
	if (!GetPropW(hwnd, PropertyZoneSizeID))
		goto end;

	switch (msg)
	{
	case WM_SYSCOMMAND:
		if (wParam == SC_MAXIMIZE)
			m_maximized = TRUE;
		else if (wParam == SC_RESTORE)
			m_maximized = FALSE;
		break;

	case WM_WINDOWPOSCHANGING:
	{
		if (!m_maximized) 
			goto end;

		FILE* file;
		fopen_s(&file, "c:\\users\\me\\debug\\dllmain.txt", "a+");
		fprintf(file, "Entered hookWndProc\n");

		POINT zoneSize = { 0,0 };
		POINT zoneOrigin = { 0,0 };

		if (GetZoneSizeAndOrigin(hwnd, zoneSize, zoneOrigin))
		{
			fprintf(file, "- found maxsize prop\n");
			// Reference: https://www.betaarchive.com/wiki/index.php/Microsoft_KB_Archive/67166
			auto minmax = reinterpret_cast<WINDOWPOS*>(lParam);

			minmax->cx = zoneSize.x;
			minmax->cy = zoneSize.y;

			minmax->x = zoneOrigin.x;
			minmax->y = zoneOrigin.y;

			//minmax->flags = SWP_NOMOVE | SWP_NOSIZE;
		}
		else
		{
			fprintf(file, "!! couldn't find maxsize prop!\n");
		}
		fclose(file);

		return 0;
	}
	break;
	}

	end:
	return DefSubclassProc(hwnd, msg, wParam, lParam);
}


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