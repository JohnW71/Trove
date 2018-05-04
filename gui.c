#include <windows.h>

#define ID_QUIT 1

LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
void centerWindow(HWND);

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,
					PWSTR pCmdLine, int nCmdShow)
{
	MSG msg;
	// HWND hwnd;
	WNDCLASSW wc = {0};

	wc.style = CS_HREDRAW | CS_VREDRAW;
	wc.lpszClassName = L"Trove";
	wc.hInstance = hInstance;
	wc.hbrBackground = GetSysColorBrush(COLOR_3DFACE);
	wc.lpfnWndProc = WndProc;
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	wc.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	RegisterClassW(&wc);

	/* hwnd = */ CreateWindowW(wc.lpszClassName, L"Trove",
						WS_OVERLAPPEDWINDOW | WS_VISIBLE,
						 0, 0, 800, 600, NULL, NULL, hInstance, NULL);

	// ShowWindow(hwnd, nCmdShow);
	// UpdateWindow(hwnd);

	while (GetMessage(&msg, NULL, 0, 0))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	return (int)msg.wParam;
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg)
	{
		case WM_CREATE:
			centerWindow(hwnd);
			CreateWindowW(L"Button", L"Quit",
						  WS_VISIBLE | WS_CHILD,
						  100, 50, 80, 25, hwnd, (HMENU)ID_QUIT, NULL, NULL);
			break;
		case WM_COMMAND:
			if (LOWORD(wParam) == ID_QUIT)
			{
				PostQuitMessage(0);
			}
			break;
		case WM_DESTROY:
			PostQuitMessage(0);
			break;
	}

	return DefWindowProcW(hwnd, msg, wParam, lParam);
}

void centerWindow(HWND hwnd)
{
	RECT rc = {0};

	GetWindowRect(hwnd, &rc);
	int windowWidth = rc.right - rc.left;
	int windowHeight = rc.bottom - rc.top;
	int screenWidth = GetSystemMetrics(SM_CXSCREEN);
	int screenHeight = GetSystemMetrics(SM_CYSCREEN);

	SetWindowPos(hwnd, HWND_TOP, (screenWidth - windowWidth) / 2, (screenHeight - windowHeight) / 2, 0, 0, SWP_NOSIZE);
}
