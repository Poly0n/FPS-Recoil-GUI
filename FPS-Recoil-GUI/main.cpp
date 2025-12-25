#include <windows.h>
#include <tchar.h>
#include <string.h>
#include <stdlib.h>
#include <process.h>

static TCHAR szWindowClass[] = _T("DesktopApp");
static TCHAR szTitle[] = _T("FPS Recoil App");

// Define button IDs
#define ID_BUTTON_ADD_RECOIL  1001
#define ID_BUTTON_SUB_RECOIL  1002
#define ID_BUTTON_START       1003
#define ID_BUTTON_RESET       1004

HINSTANCE hInst;
int g_recoilAmount = 0;
bool g_isRunning = false;
HANDLE g_recoilThread = NULL;
unsigned int g_threadID = 0;

LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);

unsigned __stdcall RecoilThread(void* param);
void StopRecoilScript();

int WINAPI WinMain(
	_In_ HINSTANCE hInstance,
	_In_opt_ HINSTANCE hPrevInstance,
	_In_ LPSTR lpCmdLine,
	_In_ int nCmdShow
) {
	WNDCLASSEX wcex;

	wcex.cbSize = sizeof(WNDCLASSEX);
	wcex.style = CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc = WndProc;
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = 0;
	wcex.hInstance = hInstance;
	wcex.hIcon = LoadIcon(wcex.hInstance, IDI_APPLICATION);
	wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wcex.lpszMenuName = NULL;
	wcex.lpszClassName = szWindowClass;
	wcex.hIconSm = LoadIcon(wcex.hInstance, IDI_APPLICATION);

	if (!RegisterClassEx(&wcex)) {
		MessageBox(NULL, _T("Call to RegisterClassEx Failed!"), _T("FPS Recoil App"), NULL);
		return 1;
	}

	hInst = hInstance;

	HWND hWnd = CreateWindowEx(
		WS_EX_OVERLAPPEDWINDOW,
		szWindowClass,
		szTitle,
		WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, CW_USEDEFAULT,
		550, 300,
		NULL,
		NULL,
		hInstance,
		NULL
	);

	if (!hWnd) {
		MessageBox(NULL, _T("Call to CreateWindowEx failed!"), _T("FPS Recoil App"), NULL);
		return 1;
	}

	CreateWindow(
		_T("BUTTON"),
		_T("+1 Recoil"),
		WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON,
		50, 100, 100, 50,
		hWnd,
		(HMENU)ID_BUTTON_ADD_RECOIL,
		hInstance,
		NULL
	);

	CreateWindow(
		_T("BUTTON"),
		_T("-1 Recoil"),
		WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON,
		160, 100, 100, 50,
		hWnd,
		(HMENU)ID_BUTTON_SUB_RECOIL,
		hInstance,
		NULL
	);

	HWND hStartButton = CreateWindow(
		_T("BUTTON"),
		_T("START"),
		WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON,
		270, 100, 100, 50,
		hWnd,
		(HMENU)ID_BUTTON_START,
		hInstance,
		NULL
	);

	CreateWindow(
		_T("BUTTON"),
		_T("Reset"),
		WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON,
		380, 100, 100, 50,
		hWnd,
		(HMENU)ID_BUTTON_RESET,
		hInstance,
		NULL
	);

	ShowWindow(hWnd, nCmdShow);
	UpdateWindow(hWnd);

	MSG msg;
	while (GetMessage(&msg, NULL, 0, 0)) {
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	if (g_isRunning) {
		g_isRunning = false;
		if (g_recoilThread) {
			WaitForSingleObject(g_recoilThread, 1000);
			CloseHandle(g_recoilThread);
		}
	}

	return (int)msg.wParam;
}

unsigned __stdcall RecoilThread(void* param) {
	HWND hWnd = (HWND)param;

	while (g_isRunning) {
		if (!g_isRunning) {
			break;
		}

		if ((GetAsyncKeyState(VK_LBUTTON) & 0x8000) &&
			(GetAsyncKeyState(VK_RBUTTON) & 0x8000)) {

			INPUT input = { 0 };
			input.type = INPUT_MOUSE;
			input.mi.dy = g_recoilAmount;
			input.mi.dwFlags = MOUSEEVENTF_MOVE;
			SendInput(1, &input, sizeof(INPUT));

			Sleep(25);
		}

		Sleep(1);
	}

	return 0;
}

void StopRecoilScript() {
	g_isRunning = false;

	if (g_recoilThread) {
		WaitForSingleObject(g_recoilThread, 1000);
		CloseHandle(g_recoilThread);
		g_recoilThread = NULL;
	}
}

LRESULT CALLBACK WndProc(
	_In_ HWND hWnd,
	_In_ UINT message,
	_In_ WPARAM wParam,
	_In_ LPARAM lParam
) {
	PAINTSTRUCT ps;
	HDC hdc;
	TCHAR greeting[] = _T("FPS Recoil Controller");
	TCHAR mess1[] = _T("Adjust Recoil Amount:");
	TCHAR statusLabel[] = _T("Status: ");
	static TCHAR statusText[50] = _T("Ready");

	switch (message) {
	case WM_PAINT:
	{
		hdc = BeginPaint(hWnd, &ps);

		TextOut(hdc, 180, 20, greeting, _tcslen(greeting));
		TextOut(hdc, 50, 80, mess1, _tcslen(mess1));

		TextOut(hdc, 50, 170, statusLabel, _tcslen(statusLabel));
		TextOut(hdc, 110, 170, statusText, _tcslen(statusText));

		TCHAR amountText[50];
		_stprintf_s(amountText, _T("Current Recoil: %d"), g_recoilAmount);
		TextOut(hdc, 50, 200, amountText, _tcslen(amountText));

		TCHAR instructions[] = _T("Set recoil amount, then click START");
		TextOut(hdc, 50, 230, instructions, _tcslen(instructions));

		EndPaint(hWnd, &ps);
	}
	break;

	case WM_COMMAND:
	{
		int wmId = LOWORD(wParam);

		switch (wmId) {
		case ID_BUTTON_ADD_RECOIL:
			g_recoilAmount++;
			_tcscpy_s(statusText, _T("Recoil increased"));
			InvalidateRect(hWnd, NULL, TRUE);
			break;

		case ID_BUTTON_SUB_RECOIL:
			if (g_recoilAmount > 0) {
				g_recoilAmount--;
				_tcscpy_s(statusText, _T("Recoil decreased"));
			}
			else {
				_tcscpy_s(statusText, _T("Recoil is already 0"));
			}
			InvalidateRect(hWnd, NULL, TRUE);
			break;

		case ID_BUTTON_START:
			if (!g_isRunning) {
				g_isRunning = true;
				_tcscpy_s(statusText, _T("RUNNING - Press STOP to end"));

				SetWindowText(GetDlgItem(hWnd, ID_BUTTON_START), _T("STOP"));

				g_recoilThread = (HANDLE)_beginthreadex(
					NULL,
					0, 
					RecoilThread,
					hWnd,
					0, 
					&g_threadID 
				);

				if (g_recoilThread == NULL) {
					MessageBox(hWnd, _T("Failed to create recoil thread!"),
						_T("Error"), MB_OK | MB_ICONERROR);
					g_isRunning = false;
					_tcscpy_s(statusText, _T("Thread creation failed"));
					SetWindowText(GetDlgItem(hWnd, ID_BUTTON_START), _T("START"));
				}
			}
			else {
				StopRecoilScript();
				_tcscpy_s(statusText, _T("Stopped"));

				SetWindowText(GetDlgItem(hWnd, ID_BUTTON_START), _T("START"));
			}
			InvalidateRect(hWnd, NULL, TRUE);
			break;

		case ID_BUTTON_RESET:
			if (g_isRunning) {
				StopRecoilScript();
			}

			g_recoilAmount = 0;
			_tcscpy_s(statusText, _T("Reset to defaults"));
			SetWindowText(GetDlgItem(hWnd, ID_BUTTON_START), _T("START"));
			InvalidateRect(hWnd, NULL, TRUE);
			break;
		}
	}
	break;

	case WM_DESTROY:
		if (g_isRunning) {
			StopRecoilScript();
		}
		PostQuitMessage(0);
		break;

	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}

	return 0;
}