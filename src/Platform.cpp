#include <jade/Platform.h>

#if defined(_WIN32) || defined(WIN32)
#include <Windows.h>

#include <iostream>

namespace {
	HWND g_ConsoleHandle = NULL;

	HWND GetConsoleWindowHWND() {
		if (g_ConsoleHandle) {
			return g_ConsoleHandle;
		}
		HWND hwnd;
		char pszWindowTitle[768];
		GetConsoleTitle(pszWindowTitle, 768);
		int i = strlen(pszWindowTitle);
		_itoa(GetTickCount(), &pszWindowTitle[i], 16);
		_itoa(GetCurrentProcessId(), &pszWindowTitle[strlen(pszWindowTitle)], 16);
		SetConsoleTitle(pszWindowTitle);
		Sleep(50);
		hwnd = FindWindow(NULL, pszWindowTitle);
		pszWindowTitle[i] = 0;
		SetConsoleTitle(pszWindowTitle);
		return g_ConsoleHandle = hwnd;
	}
}

bool jade::IsConsoleWindowFocused() {
	return GetForegroundWindow() == GetConsoleWindowHWND();
}

std::string jade::GetClipboardTextContent() {
	if (!OpenClipboard(nullptr)) {
		return {};
	}
	HANDLE hData = GetClipboardData(CF_TEXT);
	char* text = static_cast<char*>(GlobalLock(hData));

	if (text == nullptr) {
		return {};
	}
	std::string result = text;

	GlobalUnlock(hData);
	CloseClipboard();

	return text;
}

#endif // WIN32