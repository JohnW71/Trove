#define _CRT_SECURE_NO_WARNINGS

#include <windows.h>
#include <stdbool.h>

#define ID_MAIN_ADD 1
#define ID_MAIN_EDIT 2
#define ID_MAIN_DELETE 3
#define ID_MAIN_SETTINGS 4
#define ID_MAIN_QUIT 5
#define ID_MAIN_TEXTBOX 6
#define ID_MAIN_FIND 7
#define ID_MAIN_LISTBOX 8

#define ID_EDIT_TITLE_LABEL 10
#define ID_EDIT_TITLE 11
#define ID_EDIT_ID_LABEL 12
#define ID_EDIT_ID 13
#define ID_EDIT_PW_LABEL 14
#define ID_EDIT_PW 15
#define ID_EDIT_MISC_LABEL 16
#define ID_EDIT_MISC 17
#define ID_EDIT_CANCEL 18
#define ID_EDIT_OK 19

#define MAXTITLE 21
#define MAXID 21
#define MAXPW 21
#define MAXMISC 51
#define MAXLINE 110

// #define MINPW 6
// #define MINCHARS 0
// #define MAXCHARS 6
// #define MAXNAME 10
// #define KEYSIZE 32

void centerWindow(HWND);
void updateListbox();
void addEntry();
void editEntry();
void deleteEntry();
void findEntry();
void settings();

LRESULT CALLBACK mainWndProc(HWND, UINT, WPARAM, LPARAM);
LRESULT CALLBACK addWndProc(HWND, UINT, WPARAM, LPARAM);
LRESULT CALLBACK editWndProc(HWND, UINT, WPARAM, LPARAM);
LRESULT CALLBACK settingsWndProc(HWND, UINT, WPARAM, LPARAM);

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
	{ L"Title5", L"ID5", L"PW5", L"Misc5"},
	{ L"Title6", L"ID6", L"PW6", L"Misc6"},
};

static bool running = true;
static bool addClassRegistered = false;
static bool editClassRegistered = false;
static bool settingsClassRegistered = false;
static int entryCount = 0;
static int showCmd;
static LRESULT selectedRow = LB_ERR;
static HINSTANCE instance;
static HWND lbList, bEdit, bDelete;

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,
					PWSTR pCmdLine, int nShowCmd)
{
	instance = hInstance;
	showCmd = nShowCmd;
	MSG msg;
	WNDCLASSEX wc = {0};

	wc.cbSize = sizeof(WNDCLASSEX);
	wc.cbClsExtra = 0;
	wc.cbWndExtra = 0;
	wc.hbrBackground = (HBRUSH)COLOR_WINDOW;
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	wc.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	wc.hIconSm = LoadIcon(NULL, IDI_APPLICATION);
	wc.hInstance = hInstance;
	wc.lpfnWndProc = mainWndProc;
	wc.lpszClassName = "Trove";
	wc.lpszMenuName = NULL;
	wc.style = CS_HREDRAW | CS_VREDRAW;

	if (!RegisterClassEx(&wc))
	{
		MessageBox(NULL, "Window registration failed", "Error", MB_ICONEXCLAMATION | MB_OK);
		return 0;
	}

	HWND hwnd = CreateWindowEx(WS_EX_LEFT,
						wc.lpszClassName,
						"Trove",
						WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX | WS_VISIBLE,
						CW_USEDEFAULT, CW_USEDEFAULT, 390, 600,
						NULL, NULL,	hInstance, NULL);

	if (!hwnd)
	{
		MessageBox(NULL, "Window creation failed", "Error", MB_ICONEXCLAMATION | MB_OK);
		return 0;
	}

	ShowWindow(hwnd, nShowCmd);

	while (running)
	{
		if (GetMessage(&msg, NULL, 0, 0))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}

	return (int)msg.wParam;
}

LRESULT CALLBACK mainWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	static HWND bAdd, bSettings, tFind, bFind, bCancel, tTitle;

	switch (msg)
	{
		case WM_CREATE:
			centerWindow(hwnd);

			// first row
			bAdd = CreateWindowEx(WS_EX_LEFT, "Button", "Add",
						WS_VISIBLE | WS_CHILD,
						10, 10, 80, 25, hwnd, (HMENU)ID_MAIN_ADD, NULL, NULL);
			bEdit = CreateWindowEx(WS_EX_LEFT, "Button", "Edit",
						WS_VISIBLE | WS_CHILD | WS_DISABLED,
						100, 10, 80, 25, hwnd, (HMENU)ID_MAIN_EDIT, NULL, NULL);
			bDelete = CreateWindowEx(WS_EX_LEFT, "Button", "Delete",
						WS_VISIBLE | WS_CHILD | WS_DISABLED,
						190, 10, 80, 25, hwnd, (HMENU)ID_MAIN_DELETE, NULL, NULL);
			bSettings = CreateWindowEx(WS_EX_LEFT, "Button", "Settings",
						WS_VISIBLE | WS_CHILD,
						280, 10, 80, 25, hwnd, (HMENU)ID_MAIN_SETTINGS, NULL, NULL);

			// second row
			tFind = CreateWindowEx(WS_EX_LEFT, "Edit", NULL,
						WS_VISIBLE | WS_CHILD | WS_BORDER,
						10, 45, 190, 25, hwnd, (HMENU)ID_MAIN_TEXTBOX, NULL, NULL);
			bFind = CreateWindowEx(WS_EX_LEFT, "Button", "Find",
						WS_VISIBLE | WS_CHILD | WS_DISABLED,
						205, 45, 40, 25, hwnd, (HMENU)ID_MAIN_FIND, NULL, NULL);
			bCancel = CreateWindowEx(WS_EX_LEFT, "Button", "Quit",
						WS_VISIBLE | WS_CHILD,
						280, 45, 80, 25, hwnd, (HMENU)ID_MAIN_QUIT, NULL, NULL);

			// listbox
			lbList = CreateWindowEx(WS_EX_LEFT, "ListBox", NULL,
						WS_VISIBLE | WS_CHILD | LBS_STANDARD,
						10, 80, 350, 475, hwnd, (HMENU)ID_MAIN_LISTBOX, NULL, NULL);

			updateListbox();
			break;
		case WM_COMMAND:
			if (LOWORD(wParam) == ID_MAIN_ADD)
			{
				addEntry();
				updateListbox();
			}

			if (LOWORD(wParam) == ID_MAIN_EDIT)
			{
				editEntry();
				updateListbox();
			}

			if (LOWORD(wParam) == ID_MAIN_DELETE)
			{
				deleteEntry();
				updateListbox();
			}

			if (LOWORD(wParam) == ID_MAIN_SETTINGS)
			{
				// settings();
				updateListbox();
			}

			if (LOWORD(wParam) == ID_MAIN_FIND)
			{
				// find();
				// iIndex = SendMessage (hwndList, LB_SELECTSTRING, iIndex, (LPARAM) szSearchString);
			}

			if (LOWORD(wParam) == ID_MAIN_QUIT)
			{
				PostQuitMessage(0);
				running = false;
			}

			if (LOWORD(wParam) == ID_MAIN_LISTBOX)
			{
				// a row was selected
				if (HIWORD(wParam) == LBN_SELCHANGE)
				{
					// get row index
					selectedRow = SendMessage(lbList, LB_GETCURSEL, 0, 0);

					if (selectedRow != LB_ERR)
					{
						EnableWindow(bEdit, TRUE);
						EnableWindow(bDelete, TRUE);
					}
				}
			}
			break;
		case WM_DESTROY:
			PostQuitMessage(0);
			running = false;
			break;
	}
	return DefWindowProc(hwnd, msg, wParam, lParam);
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

void updateListbox()
{
	SendMessage(lbList, LB_RESETCONTENT, 0, 0);

	// add entries to listbox
	for (int i = 0; i < ARRAYSIZE(entries); ++i)
	{
		//TODO skip deleted entries

		wchar_t row[MAXLINE];
		wcscpy(row, entries[i].title);
		wcscat(row, L", ");
		wcscat(row, entries[i].id);
		wcscat(row, L", ");
		wcscat(row, entries[i].pw);
		wcscat(row, L", ");
		wcscat(row, entries[i].misc);

		SendMessageW(lbList, LB_ADDSTRING, i, (LPARAM)row);
		// if (!SendMessageW(lbList, LB_ADDSTRING, 0, (LPARAM)row))
		// {
		// 	MessageBox(NULL, "Listbox population failed", "Error", MB_ICONEXCLAMATION | MB_OK);
		// 	return;
		// }
	}

	// deselect all entries
	SendMessage(lbList, LB_SETCURSEL, -1, 0);
	EnableWindow(bEdit, FALSE);
	EnableWindow(bDelete, FALSE);
	selectedRow = LB_ERR;
}

void addEntry()
{
	static WNDCLASSEX wcAdd = {0};

	if (!addClassRegistered)
	{
		wcAdd.cbSize = sizeof(WNDCLASSEX);
		wcAdd.cbClsExtra = 0;
		wcAdd.cbWndExtra = 0;
		wcAdd.hbrBackground = (HBRUSH)COLOR_WINDOW;
		wcAdd.hCursor = LoadCursor(NULL, IDC_ARROW);
		wcAdd.hIcon = NULL;
		wcAdd.hIconSm = NULL;
		wcAdd.hInstance = instance;
		wcAdd.lpfnWndProc = addWndProc;
		wcAdd.lpszClassName = "TroveAdd";
		wcAdd.lpszMenuName = NULL;
		wcAdd.style = CS_HREDRAW | CS_VREDRAW;

		if (!RegisterClassEx(&wcAdd))
		{
			MessageBox(NULL, "Add window registration failed", "Error", MB_ICONEXCLAMATION | MB_OK);
			return;
		}
		else
			addClassRegistered = true;
	}

	HWND hwnd = CreateWindowEx(WS_EX_LEFT,
								wcAdd.lpszClassName,
								"Add",
								WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX | WS_VISIBLE,
								CW_USEDEFAULT, CW_USEDEFAULT,
								375, 200,
								NULL, NULL,
								instance, NULL);

	if (!hwnd)
	{
		MessageBox(NULL, "Add window creation failed", "Error", MB_ICONEXCLAMATION | MB_OK);
		return;
	}

	ShowWindow(hwnd, showCmd);
}

LRESULT CALLBACK addWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	static HWND bOK, bCancel, lTitle, tTitle, lId, tId, lPw, tPw, lMisc, tMisc;

	switch (msg)
	{
		case WM_CREATE:
			centerWindow(hwnd);

			lTitle = CreateWindowEx(WS_EX_LEFT, "Static", "Title",
							WS_VISIBLE | WS_CHILD,
							10, 10, 80, 25, hwnd, (HMENU)ID_EDIT_TITLE_LABEL, NULL, NULL);
			tTitle = CreateWindowEx(WS_EX_LEFT, "Edit", "",
							WS_VISIBLE | WS_CHILD | WS_BORDER,
							90, 10, 150, 25, hwnd, (HMENU)ID_EDIT_TITLE, NULL, NULL);

			lId = CreateWindowEx(WS_EX_LEFT, "Static", "ID",
							WS_VISIBLE | WS_CHILD,
							10, 45, 80, 25, hwnd, (HMENU)ID_EDIT_ID_LABEL, NULL, NULL);
			tId = CreateWindowEx(WS_EX_LEFT, "Edit", "",
							WS_VISIBLE | WS_CHILD | WS_BORDER,
							90, 45, 150, 25, hwnd, (HMENU)ID_EDIT_ID, NULL, NULL);

			lPw = CreateWindowEx(WS_EX_LEFT, "Static", "Password",
							WS_VISIBLE | WS_CHILD,
							10, 80, 80, 25, hwnd, (HMENU)ID_EDIT_PW_LABEL, NULL, NULL);
			tPw = CreateWindowEx(WS_EX_LEFT, "Edit", "",
							WS_VISIBLE | WS_CHILD | WS_BORDER,
							90, 80, 150, 25, hwnd, (HMENU)ID_EDIT_PW, NULL, NULL);

			lMisc = CreateWindowEx(WS_EX_LEFT, "Static", "Misc",
							WS_VISIBLE | WS_CHILD,
							10, 115, 80, 25, hwnd, (HMENU)ID_EDIT_MISC_LABEL, NULL, NULL);
			tMisc = CreateWindowEx(WS_EX_LEFT, "Edit", "",
							WS_VISIBLE | WS_CHILD | WS_BORDER,
							90, 115, 150, 25, hwnd, (HMENU)ID_EDIT_MISC, NULL, NULL);

			bOK = CreateWindowEx(WS_EX_LEFT, "Button", "OK",
							WS_VISIBLE | WS_CHILD,
							270, 10, 80, 25, hwnd, (HMENU)ID_EDIT_OK, NULL, NULL);
			bCancel = CreateWindowEx(WS_EX_LEFT, "Button", "Cancel",
							WS_VISIBLE | WS_CHILD,
							270, 45, 80, 25, hwnd, (HMENU)ID_EDIT_CANCEL, NULL, NULL);

			SetFocus(tTitle);
			break;
		case WM_COMMAND:
			if (LOWORD(wParam) == ID_EDIT_OK)
			{
				// add record
				// entries = realloc(entries, (entryCount+1) * sizeof *entries);
				// GetWindowText(tTitle, entries[entryCount].title, MAXTITLE);
				// GetWindowText(tId, entries[entryCount].id, MAXID);
				// GetWindowText(tPw, entries[entryCount].pw, MAXPW);
				// GetWindowText(tMisc, entries[entryCount].misc, MAXMISC);
				// ++entryCount;
				DestroyWindow(hwnd);
			}
			if (LOWORD(wParam) == ID_EDIT_CANCEL)
			{
				DestroyWindow(hwnd);
			}
			break;
		case WM_DESTROY:
			DestroyWindow(hwnd);
			break;
	}
	return DefWindowProc(hwnd, msg, wParam, lParam);
}

void editEntry()
{
	static WNDCLASSEX wcEdit = {0};

	if (!editClassRegistered)
	{
		wcEdit.cbSize = sizeof(WNDCLASSEX);
		wcEdit.cbClsExtra = 0;
		wcEdit.cbWndExtra = 0;
		wcEdit.hbrBackground = (HBRUSH)COLOR_WINDOW;
		wcEdit.hCursor = LoadCursor(NULL, IDC_ARROW);
		wcEdit.hIcon = NULL;
		wcEdit.hIconSm = NULL;
		wcEdit.hInstance = instance;
		wcEdit.lpfnWndProc = editWndProc;
		wcEdit.lpszClassName = "TroveEdit";
		wcEdit.lpszMenuName = NULL;
		wcEdit.style = CS_HREDRAW | CS_VREDRAW;

		if (!RegisterClassEx(&wcEdit))
		{
			MessageBox(NULL, "Edit window registration failed", "Error", MB_ICONEXCLAMATION | MB_OK);
			return;
		}
		else
			editClassRegistered = true;
	}

	HWND hwnd = CreateWindowEx(WS_EX_LEFT,
								wcEdit.lpszClassName,
								"Edit",
								WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX | WS_VISIBLE,
								CW_USEDEFAULT, CW_USEDEFAULT,
								375, 200,
								NULL, NULL,
								instance, NULL);

	if (!hwnd)
	{
		MessageBox(NULL, "Edit window creation failed", "Error", MB_ICONEXCLAMATION | MB_OK);
		return;
	}

	ShowWindow(hwnd, showCmd);
}

LRESULT CALLBACK editWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	static HWND bOK, bCancel, lTitle, tTitle, lId, tId, lPw, tPw, lMisc, tMisc;

	switch (msg)
	{
		case WM_CREATE:
			centerWindow(hwnd);

			lTitle = CreateWindowEx(WS_EX_LEFT, "Static", "Title",
							WS_VISIBLE | WS_CHILD,
							10, 10, 80, 25, hwnd, (HMENU)ID_EDIT_TITLE_LABEL, NULL, NULL);
			tTitle = CreateWindowEx(WS_EX_LEFT, "Edit", "",
							WS_VISIBLE | WS_CHILD | WS_BORDER,
							90, 10, 150, 25, hwnd, (HMENU)ID_EDIT_TITLE, NULL, NULL);

			lId = CreateWindowEx(WS_EX_LEFT, "Static", "ID",
							WS_VISIBLE | WS_CHILD,
							10, 45, 80, 25, hwnd, (HMENU)ID_EDIT_ID_LABEL, NULL, NULL);
			tId = CreateWindowEx(WS_EX_LEFT, "Edit", "",
							WS_VISIBLE | WS_CHILD | WS_BORDER,
							90, 45, 150, 25, hwnd, (HMENU)ID_EDIT_ID, NULL, NULL);

			lPw = CreateWindowEx(WS_EX_LEFT, "Static", "Password",
							WS_VISIBLE | WS_CHILD,
							10, 80, 80, 25, hwnd, (HMENU)ID_EDIT_PW_LABEL, NULL, NULL);
			tPw = CreateWindowEx(WS_EX_LEFT, "Edit", "",
							WS_VISIBLE | WS_CHILD | WS_BORDER,
							90, 80, 150, 25, hwnd, (HMENU)ID_EDIT_PW, NULL, NULL);

			lMisc = CreateWindowEx(WS_EX_LEFT, "Static", "Misc",
							WS_VISIBLE | WS_CHILD,
							10, 115, 80, 25, hwnd, (HMENU)ID_EDIT_MISC_LABEL, NULL, NULL);
			tMisc = CreateWindowEx(WS_EX_LEFT, "Edit", "",
							WS_VISIBLE | WS_CHILD | WS_BORDER,
							90, 115, 150, 25, hwnd, (HMENU)ID_EDIT_MISC, NULL, NULL);

			bOK = CreateWindowEx(WS_EX_LEFT, "Button", "OK",
							WS_VISIBLE | WS_CHILD,
							270, 10, 80, 25, hwnd, (HMENU)ID_EDIT_OK, NULL, NULL);
			bCancel = CreateWindowEx(WS_EX_LEFT, "Button", "Cancel",
							WS_VISIBLE | WS_CHILD,
							270, 45, 80, 25, hwnd, (HMENU)ID_EDIT_CANCEL, NULL, NULL);

			SetWindowTextW(tTitle, entries[selectedRow].title);
			SetWindowTextW(tId, entries[selectedRow].id);
			SetWindowTextW(tPw, entries[selectedRow].pw);
			SetWindowTextW(tMisc, entries[selectedRow].misc);
			SetFocus(bCancel);
			break;
		case WM_COMMAND:
			if (LOWORD(wParam) == ID_EDIT_OK)
			{
				// update record
				GetWindowTextW(tTitle, entries[selectedRow].title, MAXTITLE);
				GetWindowTextW(tId, entries[selectedRow].id, MAXID);
				GetWindowTextW(tPw, entries[selectedRow].pw, MAXPW);
				GetWindowTextW(tMisc, entries[selectedRow].misc, MAXMISC);
				DestroyWindow(hwnd);
			}
			if (LOWORD(wParam) == ID_EDIT_CANCEL)
			{
				DestroyWindow(hwnd);
			}
			break;
		case WM_DESTROY:
			DestroyWindow(hwnd);
			break;
	}
	return DefWindowProc(hwnd, msg, wParam, lParam);
}

//TODO if this is all there is, move back to mainWndProc
void deleteEntry()
{
	// SendMessage (hwndList, LB_DELETESTRING, iIndex, i);
	if (selectedRow != LB_ERR)
		entries[selectedRow].title[0] = '\0';
}
