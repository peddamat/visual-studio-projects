#include "stdafx.h"
#include <stdio.h>
#include <windows.h>
#include <commctrl.h>

#pragma comment(lib, "comctl32.lib")

BOOL m_hooked = FALSE;
LRESULT CALLBACK hookWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam, UINT_PTR uIdSubclass, DWORD_PTR dwRefData);

INT APIENTRY DllMain(HMODULE hDLL, DWORD Reason, LPVOID Reserved) {
	/* open file */
	FILE* file;
	fopen_s(&file, "c:\\users\\me\\debug\\temp.txt", "a+");

	switch (Reason) {
	case DLL_PROCESS_ATTACH:
	{
		fprintf(file, "DLL attach function called.\n");
	}
	break;
	case DLL_PROCESS_DETACH:
		fprintf(file, "DLL detach function called.\n");
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


extern "C" __declspec(dllexport) LRESULT CALLBACK keyboardProc(int code, WPARAM wParam, LPARAM lParam) {
	if (code < 0) // Do not process
	{
		return CallNextHookEx(NULL, code, wParam, lParam);
	}
	else
	{
		FILE* file;
		fopen_s(&file, "c:\\users\\me\\debug\\function4.txt", "a+");
		fprintf(file, "Function keyboard_hook called: %c\n", wParam);
		fclose(file);
		return(CallNextHookEx(NULL, code, wParam, lParam));
	}
}

// Reference: https://microsoft.public.vc.language.narkive.com/6oyFmtSV/wm-getminmaxinfo-setwindowshookex
extern "C" __declspec(dllexport) LRESULT CALLBACK callWndProc(int code, WPARAM wParam, LPARAM lParam)
{
	// Only handle messages from other processes
	if (code >= 0)
	{
		auto data = reinterpret_cast<CWPSTRUCT*>(lParam);
		auto hwnd = data->hwnd;

		switch (data->message)
		{
		case WM_GETMINMAXINFO:
			FILE* file;
			fopen_s(&file, "c:\\users\\me\\debug\\callWndProc.txt", "a+");
			fprintf(file, "Entered callWndProc\n");

			if (hwnd && (m_hooked == FALSE)) {
				if (GetAncestor(hwnd, GA_ROOT) != hwnd)
				{
					fprintf(file, "wasn't parent\n");
					goto end;
				}
				m_hooked = SetWindowSubclass(data->hwnd, &hookWndProc, 1, 0);
				if (m_hooked)
				{
					fprintf(file, "subclassed\n");
				}
				else
				{
					fprintf(file, "error subclassing\n");
				}
			}

			end:
			fclose(file);
			break;
		}
	}

	return(CallNextHookEx(NULL, code, wParam, lParam));
}

extern "C" __declspec(dllexport) LRESULT CALLBACK callWndProcRet(int code, WPARAM wParam, LPARAM lParam)
{
	// Only handle messages from other processes
	if (code >= 0)
	{
		auto data = reinterpret_cast<CWPRETSTRUCT*>(lParam);
		auto hwnd = data->hwnd;

		switch (data->message)
		{
		case WM_GETMINMAXINFO:
			FILE* file;
			fopen_s(&file, "c:\\users\\me\\debug\\callWndProc.txt", "a+");
			fprintf(file, "Entered callWndProcRet\n");

			// Remove our hook
			if (hwnd && (m_hooked == TRUE))
			{
				if (RemoveWindowSubclass(data->hwnd, &hookWndProc, 1))
				{
					m_hooked = FALSE;
					fprintf(file, "unsubclassed\n");
				}
				else
				{
					fprintf(file, "error removing subclass\n");
				}
			}
			fclose(file);
			break;
		}
	}

	return(CallNextHookEx(NULL, code, wParam, lParam));
}

LRESULT CALLBACK hookWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam, UINT_PTR uIdSubclass, DWORD_PTR dwRefData)
{
	switch (msg)
	{
	case WM_MOUSEMOVE:
		break;

	case WM_LBUTTONDOWN:
		break;

	case WM_GETMINMAXINFO:
	{
		auto minmax = reinterpret_cast<MINMAXINFO *>(lParam);
		minmax->ptMinTrackSize.x = 2200;
		minmax->ptMinTrackSize.y = 1400;
		minmax->ptMaxTrackSize.x = 2200;
		minmax->ptMaxTrackSize.y = 1400;
	}
	break;

	case WM_NCDESTROY:
		//RemoveWindowSubclass(hwnd, &hookWndProc, 1);
		break;
	}

	return DefSubclassProc(hwnd, msg, wParam, lParam);
}

