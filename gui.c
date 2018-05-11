#include <windows.h>
#include <commctrl.h>

#define ID_ADD 1
#define ID_EDIT 2
#define ID_DELETE 3
#define ID_SETTINGS 4
#define ID_QUIT 5
#define ID_TEXT 6
#define ID_FIND 7
#define ID_LIST 8

#define MAXTITLE 21
#define MAXID 21
#define MAXPW 21
#define MAXMISC 51
// #define MAXLINE 80
// #define MINPW 6

LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
void centerWindow(HWND);

typedef struct entry
{
	wchar_t title[MAXTITLE];
	wchar_t id[MAXID];
	wchar_t pw[MAXPW];
	wchar_t misc[MAXMISC];
} Entries;

Entries entries[] = {
	{ L"Title1", L"ID1", L"PW1", L"Misc1"},
	{ L"Title2", L"ID2", L"PW2", L"Misc2"},
	{ L"Title3", L"ID3", L"PW3", L"Misc3"},
	{ L"Title4", L"ID4", L"PW4", L"Misc4"},
};

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,
					PWSTR pCmdLine, int nCmdShow)
{
	MSG msg;
	WNDCLASSW wc = {0};

	wc.style = CS_HREDRAW | CS_VREDRAW;
	wc.lpszClassName = L"Trove";
	wc.hInstance = hInstance;
	wc.hbrBackground = GetSysColorBrush(COLOR_3DFACE);
	wc.lpfnWndProc = WndProc;
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	wc.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	RegisterClassW(&wc);

	CreateWindowW(wc.lpszClassName, L"Trove",
				WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX | WS_VISIBLE,
					0, 0, 390, 600, NULL, NULL, hInstance, NULL);

	while (GetMessage(&msg, NULL, 0, 0))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	return (int)msg.wParam;
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	static HWND wAdd, wEdit, wDelete, wSettings, wText, wFind, wQuit, wList;

	switch (msg)
	{
		case WM_CREATE:
			centerWindow(hwnd);

			wAdd = CreateWindowW(L"Button", L"Add",
						  WS_VISIBLE | WS_CHILD,
						  10, 10, 80, 25, hwnd, (HMENU)ID_ADD, NULL, NULL);
			wEdit = CreateWindowW(L"Button", L"Edit",
						  WS_VISIBLE | WS_CHILD | WS_DISABLED,
						  100, 10, 80, 25, hwnd, (HMENU)ID_EDIT, NULL, NULL);
			wDelete = CreateWindowW(L"Button", L"Delete",
						  WS_VISIBLE | WS_CHILD | WS_DISABLED,
						  190, 10, 80, 25, hwnd, (HMENU)ID_DELETE, NULL, NULL);
			wSettings = CreateWindowW(L"Button", L"Settings",
						  WS_VISIBLE | WS_CHILD,
						  280, 10, 80, 25, hwnd, (HMENU)ID_SETTINGS, NULL, NULL);

			wText = CreateWindowW(L"Edit", NULL,
						  WS_CHILD | WS_VISIBLE | WS_BORDER,
						  10, 45, 190, 25, hwnd, (HMENU)ID_TEXT, NULL, NULL);
			wFind = CreateWindowW(L"Button", L"Find",
						  WS_VISIBLE | WS_CHILD | WS_DISABLED,
						  205, 45, 40, 25, hwnd, (HMENU)ID_FIND, NULL, NULL);
			wQuit = CreateWindowW(L"Button", L"Quit",
						  WS_VISIBLE | WS_CHILD,
						  280, 45, 80, 25, hwnd, (HMENU)ID_QUIT, NULL, NULL);

			wList = CreateWindowW(WC_LISTBOXW, NULL, WS_CHILD | WS_VISIBLE | LBS_NOTIFY,
						  10, 80, 350, 475, hwnd, (HMENU)ID_LIST, NULL, NULL);

			for (int i = 0; i < ARRAYSIZE(entries); ++i)
			{
				SendMessageW(wList, LB_ADDSTRING, 0, (LPARAM)entries[i].title);
			}
			break;
		case WM_COMMAND:
			if (LOWORD(wParam) == ID_ADD)
			{
				// add();
			}

			if (LOWORD(wParam) == ID_EDIT)
			{
				// edit();
			}

			if (LOWORD(wParam) == ID_DELETE)
			{
				// delete();
			}

			if (LOWORD(wParam) == ID_SETTINGS)
			{
				// settings();
			}

			if (LOWORD(wParam) == ID_FIND)
			{
				// find();
			}

			if (LOWORD(wParam) == ID_QUIT)
			{
				PostQuitMessage(0);
			}

			if (LOWORD(wParam) == ID_LIST)
			{
				if (HIWORD(wParam) == LBN_SELCHANGE)
				{
					EnableWindow(wEdit, TRUE);
					EnableWindow(wDelete, TRUE);
				}
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

	SetWindowPos(hwnd, HWND_TOP, (screenWidth - windowWidth) / 2,
				(screenHeight - windowHeight) / 2, 0, 0, SWP_NOSIZE);
}
