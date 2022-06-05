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

		//case WM_SYSCOMMAND:
		//	if (SC_MAXIMIZE == (wParam & ~HTCAPTION))
		//	{
		//		FILE* file;
		//		fopen_s(&file, "c:\\users\\me\\debug\\dllmain2.txt", "a+");
		//		fprintf(file, "Entered WM_SYSCOMMAND\n");

		//		POINT zoneSize = { 0,0 };
		//		POINT zoneOrigin = { 0,0 };

		//		if (GetZoneSizeAndOrigin(hwnd, zoneSize, zoneOrigin))
		//		{
		//			fprintf(file, "- found maxsize prop\n");
		//			SetWindowPos(hwnd, NULL, zoneOrigin.x, zoneOrigin.y, zoneSize.x, zoneSize.y, SWP_SHOWWINDOW);
		//			SetWindowLongPtr(hwnd, GWL_STYLE, WS_MAXIMIZE | GetWindowLong(hwnd, GWL_STYLE));
		//			fclose(file);
		//			return 0;
		//		}
		//		else
		//		{
		//			fprintf(file, "!! couldn't find maxsize prop!\n");
		//			fclose(file);
		//		}

		//		return 0;
		//	}
		//	break;
		case WM_GETMINMAXINFO:
		{
			FILE* file;
			fopen_s(&file, "c:\\users\\me\\debug\\dllmain2.txt", "a+");
			fprintf(file, "Entered WM_GETMINMAX\n");

			POINT zoneSize = { 0,0 };
			POINT zoneOrigin = { 0,0 };

			if (GetZoneSizeAndOrigin(hwnd, zoneSize, zoneOrigin))
			{
				fprintf(file, "- found zone info\n");
				auto minmax = reinterpret_cast<MINMAXINFO*>(lParam);
				minmax->ptMaxSize.x = zoneSize.x;
				minmax->ptMaxSize.y = zoneSize.y;

				minmax->ptMinTrackSize.x = zoneSize.x;
				minmax->ptMinTrackSize.y = zoneSize.y;

				minmax->ptMaxTrackSize.x = zoneSize.x;
				minmax->ptMaxTrackSize.y = zoneSize.y;

				minmax->ptMaxPosition.x = zoneOrigin.x;
				minmax->ptMaxPosition.y = zoneOrigin.y;

				fclose(file);
				return 0;
			}
			else
			{
				fprintf(file, "!! couldn't find maxsize prop!\n");
				fclose(file);
				return DefSubclassProc(hwnd, msg, wParam, lParam);
			}

		}
		break;
		case WM_WINDOWPOSCHANGING:
		{
			FILE* file;
			fopen_s(&file, "c:\\users\\me\\debug\\dllmain2.txt", "a+");
			fprintf(file, "Entered WM_GETMINMAX\n");

			POINT zoneSize = { 0,0 };
			POINT zoneOrigin = { 0,0 };

			if (GetZoneSizeAndOrigin(hwnd, zoneSize, zoneOrigin))
			{
				fprintf(file, "- found zone info\n");
				auto windowpos = reinterpret_cast<WINDOWPOS*>(lParam);
				windowpos->flags = SWP_NOMOVE | SWP_NOSIZE | SWP_SHOWWINDOW;

				windowpos->x = zoneOrigin.x;
				windowpos->y = zoneOrigin.y;

				windowpos->cx = zoneSize.x;
				windowpos->cy = zoneSize.y;

				fclose(file);
				return 0;
			}
			else
			{
				fprintf(file, "!! couldn't find maxsize prop!\n");
				fclose(file);
				return DefSubclassProc(hwnd, msg, wParam, lParam);
			}
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