#include "stdafx.h"
#include <stdio.h>
#include <windows.h>
#include "dllmain.h"

HWND m_hwnd = NULL;
DWORD m_pid = 0;
LONG OldWndProc;

DWORD WINAPI init(LPVOID);
DWORD WINAPI deinit(LPVOID);
LRESULT CALLBACK NewWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

INT APIENTRY DllMain(HMODULE hDLL, DWORD Reason, LPVOID Reserved) {
	/* open file */
	FILE* file;
	fopen_s(&file, "c:\\users\\me\\temp.txt", "a+");

	switch (Reason) {
	case DLL_PROCESS_ATTACH:
	{
		fprintf(file, "DLL attach function called.\n");
		//CreateThread(0, NULL, (LPTHREAD_START_ROUTINE)&init, NULL, NULL, NULL);

		//Find the window of the Injectee using its title
		HWND hwnd = FindWindow(L"Notepad", NULL);

		//If the window found, then change it's window proc
		if (hwnd)
		{
			fprintf(file, "hwnd found\n");
			OldWndProc = SetWindowLongPtr(hwnd, GWLP_WNDPROC, (LONG_PTR)NewWndProc);
		}
		fprintf(file, "patched\n");
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

DWORD WINAPI init(LPVOID)
{
	HWND hwnd = FindWindow(L"Notepad", NULL);
	OldWndProc = SetWindowLongPtr(hwnd, GWLP_WNDPROC, (long)NewWndProc);

	return TRUE;
}

DWORD WINAPI deinit(LPVOID)
{
	HWND hwnd = FindWindow(L"Notepad", NULL);
	SetWindowLongPtr(hwnd, GWLP_WNDPROC, (long)OldWndProc);

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
		fopen_s(&file, "c:\\users\\me\\function4.txt", "a+");
		fprintf(file, "Function keyboard_hook called: %c\n", wParam);
		fclose(file);
		return(CallNextHookEx(NULL, code, wParam, lParam));
	}
}

// Reference: https://microsoft.public.vc.language.narkive.com/6oyFmtSV/wm-getminmaxinfo-setwindowshookex
extern "C" __declspec(dllexport) LRESULT CALLBACK callWndProcRet(int code, WPARAM wParam, LPARAM lParam) {

	// Only handle messages from other processes
	if (code >= 0)
	{
		auto data = reinterpret_cast<CWPRETSTRUCT*>(lParam);
		switch (data->message)
		{
		case WM_GETMINMAXINFO:
			//auto minmax = reinterpret_cast<MINMAXINFO *>(data->lParam);
//            minmax->ptMinTrackSize.x = 300;
//            minmax->ptMaxTrackSize.x = 300;
			LPPOINT lppt = (LPPOINT)data->lParam;
			lppt[3].x = 100; // Set minimum width to current width
			lppt[4].x = 100;


			break;
		}
	}


	return(CallNextHookEx(NULL, code, wParam, lParam));
}

LRESULT CALLBACK NewWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	FILE* file;
	fopen_s(&file, "c:\\users\\me\\function6.txt", "a+");
	fprintf(file, "Function wndproc called\n");
	fclose(file);

	switch (msg)
	{
	case WM_GETMINMAXINFO:
		//auto minmax = reinterpret_cast<MINMAXINFO *>(data->lParam);
//            minmax->ptMinTrackSize.x = 300;
//            minmax->ptMaxTrackSize.x = 300;
		LPPOINT lppt = (LPPOINT)lParam;
		lppt[3].x = 100; // Set minimum width to current width
		lppt[4].x = 100;

		FILE* file;
		fopen_s(&file, "c:\\users\\me\\function5.txt", "a+");
		fprintf(file, "Function wndproc called\n");
		fclose(file);

		break;
	}

	return CallWindowProc((WNDPROC)OldWndProc, hwnd, msg, wParam, lParam);
}
