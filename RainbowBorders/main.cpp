#include <windows.h>
#include <dwmapi.h>
#include "resource.h"

#include <chrono>
#include <thread>
#include <atomic>
#include <vector>

#define WNDCLASSNAME TEXT("rainbowborderwndclass")


void registerWndClass(HINSTANCE hInstance, HICON icon);
void createNotifyIcon(HWND hWnd, HICON icon);
std::vector<HWND> getAllWindows();
BOOL CALLBACK EnumWindowsProc(HWND hWnd, LPARAM lParam);
void changeBorderColor();
int hsvToGrb(float h, float s, float v);


std::atomic_bool g_running = true;
std::vector<HWND> g_windows;

LRESULT WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {

	switch (uMsg) {
	case WM_TRAYICON:
		switch (lParam) {
		case WM_LBUTTONDOWN:
		case WM_RBUTTONDOWN:
			HMENU menu = LoadMenu(nullptr, MAKEINTRESOURCE(IDR_TRAY_MENU));
			HMENU popup = GetSubMenu(menu, 0);

			POINT point;
			GetCursorPos(&point);
			SetForegroundWindow(hWnd);
			UINT hAlign = GetSystemMetrics(SM_MENUDROPALIGNMENT);
			BOOL result = TrackPopupMenuEx(popup, TPM_BOTTOMALIGN | hAlign | TPM_RETURNCMD, point.x, point.y, hWnd, NULL);
			DestroyMenu(menu);
			if (result == ID_TRAYMENU_EXIT) {
				PostQuitMessage(0);
				return 0;
			}
			break;
		}
		break;
	}

	return DefWindowProc(hWnd, uMsg, wParam, lParam);
}


BOOL WINAPI wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE, _In_ LPWSTR, _In_ int nCmdShow) {

	// prevent multiple instances from running in parallel
	CreateMutex(NULL, TRUE, TEXT("2a226fe06e5a4697bf21ce88ee4d2b25"));

	if (GetLastError() == ERROR_ALREADY_EXISTS) {
		MessageBox(NULL, TEXT("Another instance of RainbowBorders is already running"), TEXT("Unable to start"), MB_ICONERROR | MB_OK);
		return 1;
	}


	// set low priority and enable efficiency mode
	SetPriorityClass(GetCurrentProcess(), IDLE_PRIORITY_CLASS);
	PROCESS_POWER_THROTTLING_STATE powerThrottlingState = { 0 };
	powerThrottlingState.Version = PROCESS_POWER_THROTTLING_CURRENT_VERSION;
	powerThrottlingState.ControlMask = PROCESS_POWER_THROTTLING_EXECUTION_SPEED;
	powerThrottlingState.StateMask = PROCESS_POWER_THROTTLING_EXECUTION_SPEED;
	SetProcessInformation(GetCurrentProcess(), ProcessPowerThrottling, &powerThrottlingState, sizeof(powerThrottlingState));


	// start application logic
	HICON icon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_ICON));

	registerWndClass(hInstance, icon);

	g_windows = getAllWindows();

	HWND hWnd = CreateWindow(WNDCLASSNAME, TEXT("RainbowBorders"), 0, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, NULL, NULL, hInstance, 0);

	createNotifyIcon(hWnd, icon);
	changeBorderColor();

	MSG msg;
	while (GetMessage(&msg, NULL, NULL, NULL) > 0) {
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
}

BOOL CALLBACK EnumWindowsProc(HWND hWnd, LPARAM lParam) {
	std::vector<HWND>* windows = (std::vector<HWND>*) lParam;
	windows->push_back(hWnd);
	return TRUE;
}

std::vector<HWND> getAllWindows() {
	std::vector<HWND> windows;
	EnumWindows(EnumWindowsProc, (LPARAM) &windows);
	return windows;
}


void changeBorderColor() {
	std::thread([]() {
		uint16_t hue = 0;
		while (g_running) {
			for (uint16_t hue = 0; hue < 360; hue++) {
				std::this_thread::sleep_for(std::chrono::milliseconds(10));

				int color_brg = hsvToGrb(hue, 1, 1);
				for (HWND hWnd : g_windows) {
					DwmSetWindowAttribute(hWnd, DWMWA_BORDER_COLOR, &color_brg, sizeof(int));
				}

				if (hue % 10 == 0) {
					g_windows = getAllWindows();
				}
			}		
		}
	}).detach();
}

void createNotifyIcon(HWND hWnd, HICON icon) {
	NOTIFYICONDATA icondata = { 0 };
	icondata.cbSize = sizeof(icondata);
	icondata.uID = (int) (1 + hWnd);
	icondata.hWnd = hWnd;
	icondata.uFlags = NIF_ICON | NIF_TIP | NIF_MESSAGE;
	icondata.hIcon = icon;
	icondata.uCallbackMessage = WM_TRAYICON;
	wcscpy_s(icondata.szTip, L"RainbowBorderCPP");
	Shell_NotifyIcon(NIM_ADD, &icondata);
}

void registerWndClass(HINSTANCE hInstance, HICON icon) {
	WNDCLASS wc = { 0 };
	wc.lpfnWndProc = WndProc;
	wc.hInstance = hInstance;
	wc.lpszClassName = WNDCLASSNAME;
	wc.hIcon = icon;

	RegisterClass(&wc);
}

int hsvToGrb(float h, float s, float v) {
	float c = v * s;                    // chroma
	float x = c * (1 - std::abs(fmod(h / 60.0, 2) - 1));
	float m = v - c;

	float r_prime, g_prime, b_prime;

	if (0 <= h && h < 60) {
		r_prime = c;
		g_prime = x;
		b_prime = 0;
	}
	else if (60 <= h && h < 120) {
		r_prime = x;
		g_prime = c;
		b_prime = 0;
	}
	else if (120 <= h && h < 180) {
		r_prime = 0;
		g_prime = c;
		b_prime = x;
	}
	else if (180 <= h && h < 240) {
		r_prime = 0;
		g_prime = x;
		b_prime = c;
	}
	else if (240 <= h && h < 300) {
		r_prime = x;
		g_prime = 0;
		b_prime = c;
	}
	else {
		r_prime = c;
		g_prime = 0;
		b_prime = x;
	}

	int rgb;
	rgb = static_cast<int>((r_prime + m) * 255);
	rgb |= static_cast<int>((g_prime + m) * 255) << 8;
	rgb |= static_cast<int>((b_prime + m) * 255) << 16;

	return rgb;
}