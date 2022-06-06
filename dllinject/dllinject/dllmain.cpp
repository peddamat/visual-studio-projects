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
	// Only process windows that are in a zone
	if (!GetPropW(hwnd, PropertyZoneSizeID))
		goto return_default;

	//FILE* file;
	//fopen_s(&file, "c:\\users\\me\\debug\\dllmain2.txt", "a+");

	switch (msg)
	{

	/**
	  *	WM_SYSCOMMAND events are generated when a window is maximized.
	  *	We override the default behavior and manually place the window
	  *	to avoid...
	  */
	case WM_SYSCOMMAND:

		/* Clicking the 'maximize' button generates a SC_MAXIMIZE message. 
		 * Double-clicking the title bar generates a SC_MAXIMIZE | HTCAPTION message.
		 */
		if (SC_MAXIMIZE == (wParam & ~HTCAPTION))
		{
			//fprintf(file, "Entered WM_SYSCOMMAND\n");

			POINT zoneSize = { 0,0 };
			POINT zoneOrigin = { 0,0 };

			if (GetZoneSizeAndOrigin(hwnd, zoneSize, zoneOrigin))
			{
				// Resize the window to fit within the zone
				SetWindowPos(hwnd, NULL, zoneOrigin.x, zoneOrigin.y, zoneSize.x, zoneSize.y, SWP_SHOWWINDOW);
				// Set the WS_MAXIMIZE window style, which changes the titlebar 'maximize' icon to a 'restore' icon,
				// Setting this bit also prevents moving or resizing the window
				SetWindowLongPtr(hwnd, GWL_STYLE, WS_MAXIMIZE | GetWindowLong(hwnd, GWL_STYLE));
				// Redraw the window so the updated icon is displayed
				SetWindowPos(hwnd, NULL, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE | SWP_FRAMECHANGED);

				//fprintf(file, "- window resized\n");
				goto return_override;
			}
			else
			{
				//fprintf(file, "!! couldn't find maxsize prop!\n");
				goto return_default;
			}
		}
		break;

	/**
	  *	WM_WINDOWPOSCHANGING events are generated when a window is being sized
	  * or moved.  If the window is maximized, we override the default behavior 
	  * and manually define the window's size and position.
	  */
	case WM_WINDOWPOSCHANGING:
		//fprintf(file, "Entered WM_WINDOWPOSCHANGING\n");

		if (WS_MAXIMIZE & GetWindowLong(hwnd, GWL_STYLE))
		{
			//fprintf(file, "- window is maximized\n");

			POINT zoneSize = { 0,0 };
			POINT zoneOrigin = { 0,0 };

			if (GetZoneSizeAndOrigin(hwnd, zoneSize, zoneOrigin))
			{
				//fprintf(file, "- found zone info\n");
				auto windowpos = reinterpret_cast<WINDOWPOS*>(lParam);
				//windowpos->flags = SWP_NOMOVE | SWP_NOSIZE | SWP_SHOWWINDOW;

				windowpos->x = zoneOrigin.x;
				windowpos->y = zoneOrigin.y;

				windowpos->cx = zoneSize.x;
				windowpos->cy = zoneSize.y;

				goto return_override;
			}
			else
			{
				//fprintf(file, "!! couldn't find maxsize prop!\n");
				goto return_default;
			}
		}
		else
		{
			//fprintf(file, "- window isn't maximized\n");
			goto return_default;
		}
		break;

	default:
		goto return_default;
		break;
	}

	return_override:
	//fclose(file);
	return 0;

	return_default:
	//fclose(file);
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