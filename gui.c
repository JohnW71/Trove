#define _CRT_SECURE_NO_WARNINGS

#include "gui.h"

//TODO implement encryption
//TODO make password generation faster
//TODO CtrlA does not work in editboxes

static char tempFile[] = "temp.db";
static char iniFile[] = "trove.ini";
static bool running = true;
static uint8_t iv[IV_SIZE];
static int entryCount = 0;
static int passwordSize = MINPW;
static int minSpecial = 0;
static int minNumeric = 0;
static int minUppercase = 0;
static LRESULT selectedRow;
static HINSTANCE instance;
static HWND lbList, bEdit, bDelete;
static HWND addHwnd;
static HWND editHwnd;
static HWND settingsHwnd;

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,
					PWSTR pCmdLine, int nShowCmd)
{
	instance = hInstance;
	MSG msg;
	WNDCLASSEX wc = {0};

	// reset log file
	FILE *lf = fopen("log.txt", "w");
	if (lf == NULL)
		MessageBox(NULL, "Can't open log file", "Error", MB_ICONEXCLAMATION | MB_OK);
	fclose(lf);

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

	readSettings();
	ShowWindow(hwnd, nShowCmd);

	while (running)
	{
		while (GetMessage(&msg, NULL, 0, 0))
		{
			if (!IsDialogMessage(hwnd, &msg) &&
				!IsDialogMessage(editHwnd, &msg) &&
				!IsDialogMessage(settingsHwnd, &msg) &&
				!IsDialogMessage(addHwnd, &msg))
			{
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}
		}
	}

	return (int)msg.wParam;
}

LRESULT CALLBACK mainWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	static HWND bAdd, bSettings, eFind, bFind, bCancel;

	switch (msg)
	{
		case WM_CREATE:
			centerWindow(hwnd);

			// first row
			bAdd = CreateWindowEx(WS_EX_LEFT, "Button", "Add",
						WS_VISIBLE | WS_CHILD | WS_TABSTOP,
						10, 10, 80, 25, hwnd, (HMENU)ID_MAIN_ADD, NULL, NULL);
			bEdit = CreateWindowEx(WS_EX_LEFT, "Button", "Edit",
						WS_VISIBLE | WS_CHILD | WS_TABSTOP | WS_DISABLED,
						100, 10, 80, 25, hwnd, (HMENU)ID_MAIN_EDIT, NULL, NULL);
			bDelete = CreateWindowEx(WS_EX_LEFT, "Button", "Delete",
						WS_VISIBLE | WS_CHILD | WS_TABSTOP | WS_DISABLED,
						190, 10, 80, 25, hwnd, (HMENU)ID_MAIN_DELETE, NULL, NULL);
			bSettings = CreateWindowEx(WS_EX_LEFT, "Button", "Settings",
						WS_VISIBLE | WS_CHILD | WS_TABSTOP,
						280, 10, 80, 25, hwnd, (HMENU)ID_MAIN_SETTINGS, NULL, NULL);

			// second row
			eFind = CreateWindowEx(WS_EX_LEFT, "Edit", NULL,
						WS_VISIBLE | WS_CHILD | WS_TABSTOP | WS_BORDER,
						10, 45, 190, 25, hwnd, (HMENU)ID_MAIN_FINDTEXT, NULL, NULL);
			bFind = CreateWindowEx(WS_EX_LEFT, "Button", "Find",
						WS_VISIBLE | WS_CHILD | WS_TABSTOP | WS_DISABLED,
						205, 45, 40, 25, hwnd, (HMENU)ID_MAIN_FIND, NULL, NULL);
			bCancel = CreateWindowEx(WS_EX_LEFT, "Button", "Quit",
						WS_VISIBLE | WS_CHILD | WS_TABSTOP,
						280, 45, 80, 25, hwnd, (HMENU)ID_MAIN_QUIT, NULL, NULL);

			// listbox
			lbList = CreateWindowEx(WS_EX_LEFT, "ListBox", NULL,
						WS_VISIBLE | WS_CHILD | LBS_STANDARD,
						10, 80, 350, 475, hwnd, (HMENU)ID_MAIN_LISTBOX, NULL, NULL);

			loadEntries();
			updateListbox();
			break;
		case WM_COMMAND:
			if (LOWORD(wParam) == ID_MAIN_ADD)
			{
				SetWindowTextW(eFind, L"");
				addEntry();
			}

			if (LOWORD(wParam) == ID_MAIN_EDIT)
			{
				SetWindowTextW(eFind, L"");
				editEntry();
			}

			if (LOWORD(wParam) == ID_MAIN_DELETE)
			{
				SetWindowTextW(eFind, L"");
				deleteEntry();
			}

			if (LOWORD(wParam) == ID_MAIN_SETTINGS)
			{
				SetWindowTextW(eFind, L"");
				editSettings();
				writeSettings();
			}

			if (LOWORD(wParam) == ID_MAIN_FINDTEXT)
			{
				if (HIWORD(wParam) == EN_SETFOCUS)
				{
					// deselect all entries
					SendMessage(lbList, LB_SETCURSEL, -1, 0);
				}

				// find text was changed
				if (HIWORD(wParam) == EN_CHANGE)
				{
					// get text length
					wchar_t findText[MAXTITLE];
					GetWindowTextW(eFind, findText, MAXTITLE);

					// enable Find button if text is present
					if (wcslen(findText) > 0)
						EnableWindow(bFind, TRUE);
					else
						EnableWindow(bFind, FALSE);

					// deselect all entries
					SendMessage(lbList, LB_SETCURSEL, -1, 0);
				}
			}

			if (LOWORD(wParam) == ID_MAIN_FIND)
			{
				// find button was clicked
				if (HIWORD(wParam) == BN_CLICKED)
				{
					wchar_t find[MAXTITLE];
					GetWindowTextW(eFind, find, MAXTITLE);

					// deselect all entries
					SendMessage(lbList, LB_SETCURSEL, -1, 0);

					static int i = 0;
					int attempts = 0;

					while (attempts <= entryCount)
					{
						if (i != selectedRow && wcsstr(entries[i].title, find))
						{
							selectedRow = i;
							break;
						}

						++attempts;

						if (++i == entryCount)
							i = 0;
					}

					if (selectedRow != LB_ERR) // select matching row
						SendMessage(lbList, LB_SETCURSEL, selectedRow, 0);
				}
			}

			if (LOWORD(wParam) == ID_MAIN_QUIT)
			{
				saveEntries();
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

				// a row was double-clicked
				if (HIWORD(wParam) == LBN_DBLCLK)
				{
					// get row index
					selectedRow = SendMessage(lbList, LB_GETCURSEL, 0, 0);

					if (selectedRow != LB_ERR)
						editEntry();
				}
			}
			break;
		case WM_DESTROY:
			saveEntries();
			PostQuitMessage(0);
			running = false;
			break;
	}
	return DefWindowProc(hwnd, msg, wParam, lParam);
}

void addEntry(void)
{
	static WNDCLASSEX wcAdd = {0};
	static bool addClassRegistered = false;

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

	addHwnd = CreateWindowEx(WS_EX_LEFT,
								wcAdd.lpszClassName,
								"Add",
								WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX | WS_VISIBLE,
								CW_USEDEFAULT, CW_USEDEFAULT,
								530, 200,
								NULL, NULL,
								instance, NULL);

	if (!addHwnd)
	{
		MessageBox(NULL, "Add window creation failed", "Error", MB_ICONEXCLAMATION | MB_OK);
		return;
	}

	ShowWindow(addHwnd, SW_SHOW);
}

LRESULT CALLBACK addWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	static HWND bOK, bCancel, bGenerate, lTitle, eTitle, lId, eId, lPw, ePw, lMisc, eMisc;

	switch (msg)
	{
		case WM_CREATE:
			centerWindow(hwnd);

			lTitle = CreateWindowEx(WS_EX_LEFT, "Static", "Title",
							WS_VISIBLE | WS_CHILD,
							10, 10, 80, 25, hwnd, (HMENU)ID_EDIT_TITLE_LABEL, NULL, NULL);
			eTitle = CreateWindowEx(WS_EX_LEFT, "Edit", "",
							WS_VISIBLE | WS_CHILD | WS_BORDER | WS_TABSTOP,
							90, 10, 170, 25, hwnd, (HMENU)ID_EDIT_TITLE, NULL, NULL);

			lId = CreateWindowEx(WS_EX_LEFT, "Static", "ID",
							WS_VISIBLE | WS_CHILD,
							10, 45, 80, 25, hwnd, (HMENU)ID_EDIT_ID_LABEL, NULL, NULL);
			eId = CreateWindowEx(WS_EX_LEFT, "Edit", "",
							WS_VISIBLE | WS_CHILD | WS_BORDER | WS_TABSTOP,
							90, 45, 170, 25, hwnd, (HMENU)ID_EDIT_ID, NULL, NULL);

			lPw = CreateWindowEx(WS_EX_LEFT, "Static", "Password",
							WS_VISIBLE | WS_CHILD,
							10, 80, 80, 25, hwnd, (HMENU)ID_EDIT_PW_LABEL, NULL, NULL);
			ePw = CreateWindowEx(WS_EX_LEFT, "Edit", "",
							WS_VISIBLE | WS_CHILD | WS_BORDER | WS_TABSTOP,
							90, 80, 170, 25, hwnd, (HMENU)ID_EDIT_PW, NULL, NULL);

			lMisc = CreateWindowEx(WS_EX_LEFT, "Static", "Misc",
							WS_VISIBLE | WS_CHILD,
							10, 115, 80, 25, hwnd, (HMENU)ID_EDIT_MISC_LABEL, NULL, NULL);
			eMisc = CreateWindowEx(WS_EX_LEFT, "Edit", "",
							WS_VISIBLE | WS_CHILD | WS_BORDER | WS_TABSTOP,
							90, 115, 410, 25, hwnd, (HMENU)ID_EDIT_MISC, NULL, NULL);

			bOK = CreateWindowEx(WS_EX_LEFT, "Button", "OK",
							WS_VISIBLE | WS_CHILD | WS_TABSTOP,
							320, 10, 80, 25, hwnd, (HMENU)ID_OK, NULL, NULL);

			bCancel = CreateWindowEx(WS_EX_LEFT, "Button", "Cancel",
							WS_VISIBLE | WS_CHILD | WS_TABSTOP,
							320, 45, 80, 25, hwnd, (HMENU)ID_CANCEL, NULL, NULL);

			bGenerate = CreateWindowEx(WS_EX_LEFT, "Button", "Generate password",
							WS_VISIBLE | WS_CHILD | WS_TABSTOP,
							280, 80, 160, 25, hwnd, (HMENU)ID_GENERATE, NULL, NULL);

			SetFocus(eTitle);
			break;
		case WM_COMMAND:
			if (LOWORD(wParam) == ID_OK)
			{
				// get fields from edit boxes
				wchar_t title[MAXTITLE];
				wchar_t id[MAXID];
				wchar_t pw[MAXPW];
				wchar_t misc[MAXMISC];
				GetWindowTextW(eTitle, title, MAXTITLE);
				GetWindowTextW(eId, id, MAXID);
				GetWindowTextW(ePw, pw, MAXPW);
				GetWindowTextW(eMisc, misc, MAXMISC);

				// test that title field is not empty
				if (wcslen(title) == 0)
				{
					SetFocus(eTitle);
					return DefWindowProc(hwnd, msg, wParam, lParam);
				}

				// test for pre-existing title
				for (int i = 0; i < entryCount; ++i)
				{
					if (wcscmp(entries[i].title, title) == 0 && i != selectedRow)
					{
						MessageBox(NULL, "That title is already in use", "Error", MB_ICONEXCLAMATION | MB_OK);
						SetFocus(eTitle);
						return DefWindowProc(hwnd, msg, wParam, lParam);
					}
				}

				// add to listbox & entries
				SendMessageW(lbList, LB_ADDSTRING, 0, (LPARAM)title);
				entries = realloc(entries, (entryCount+1) * sizeof(*entries));
				wcscpy(entries[entryCount].title, title);
				wcscpy(entries[entryCount].id, id);
				wcscpy(entries[entryCount].pw, pw);
				wcscpy(entries[entryCount].misc, misc);
				++entryCount;

				DestroyWindow(hwnd);
				updateListbox();
			}

			if (LOWORD(wParam) == ID_CANCEL)
			{
				DestroyWindow(hwnd);
			}

			if (LOWORD(wParam) == ID_GENERATE)
			{
				wchar_t password[MAXPW];
				generatePassword(password);
				SetWindowTextW(ePw, password);
			}
			break;
		case WM_DESTROY:
			DestroyWindow(hwnd);
			break;
	}
	return DefWindowProc(hwnd, msg, wParam, lParam);
}

void editEntry(void)
{
	static WNDCLASSEX wcEdit = {0};
	static bool editClassRegistered = false;

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

	editHwnd = CreateWindowEx(WS_EX_LEFT,
								wcEdit.lpszClassName,
								"Edit",
								WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX | WS_VISIBLE,
								CW_USEDEFAULT, CW_USEDEFAULT,
								530, 200,
								NULL, NULL,
								instance, NULL);

	if (!editHwnd)
	{
		MessageBox(NULL, "Edit window creation failed", "Error", MB_ICONEXCLAMATION | MB_OK);
		return;
	}

	ShowWindow(editHwnd, SW_SHOW);
}

LRESULT CALLBACK editWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	static HWND bOK, bCancel, bGenerate, lTitle, eTitle, lId, eId, lPw, ePw, lMisc, eMisc;

	switch (msg)
	{
		case WM_CREATE:
			centerWindow(hwnd);

			lTitle = CreateWindowEx(WS_EX_LEFT, "Static", "Title",
							WS_VISIBLE | WS_CHILD,
							10, 10, 80, 25, hwnd, (HMENU)ID_EDIT_TITLE_LABEL, NULL, NULL);
			eTitle = CreateWindowEx(WS_EX_LEFT, "Edit", "",
							WS_VISIBLE | WS_CHILD | WS_BORDER | WS_TABSTOP,
							90, 10, 170, 25, hwnd, (HMENU)ID_EDIT_TITLE, NULL, NULL);

			lId = CreateWindowEx(WS_EX_LEFT, "Static", "ID",
							WS_VISIBLE | WS_CHILD,
							10, 45, 80, 25, hwnd, (HMENU)ID_EDIT_ID_LABEL, NULL, NULL);
			eId = CreateWindowEx(WS_EX_LEFT, "Edit", "",
							WS_VISIBLE | WS_CHILD | WS_BORDER | WS_TABSTOP,
							90, 45, 170, 25, hwnd, (HMENU)ID_EDIT_ID, NULL, NULL);

			lPw = CreateWindowEx(WS_EX_LEFT, "Static", "Password",
							WS_VISIBLE | WS_CHILD,
							10, 80, 80, 25, hwnd, (HMENU)ID_EDIT_PW_LABEL, NULL, NULL);
			ePw = CreateWindowEx(WS_EX_LEFT, "Edit", "",
							WS_VISIBLE | WS_CHILD | WS_BORDER | WS_TABSTOP,
							90, 80, 170, 25, hwnd, (HMENU)ID_EDIT_PW, NULL, NULL);

			lMisc = CreateWindowEx(WS_EX_LEFT, "Static", "Misc",
							WS_VISIBLE | WS_CHILD,
							10, 115, 80, 25, hwnd, (HMENU)ID_EDIT_MISC_LABEL, NULL, NULL);
			eMisc = CreateWindowEx(WS_EX_LEFT, "Edit", "",
							WS_VISIBLE | WS_CHILD | WS_BORDER | WS_TABSTOP,
							90, 115, 410, 25, hwnd, (HMENU)ID_EDIT_MISC, NULL, NULL);

			bOK = CreateWindowEx(WS_EX_LEFT, "Button", "OK",
							WS_VISIBLE | WS_CHILD | WS_TABSTOP,
							320, 10, 80, 25, hwnd, (HMENU)ID_OK, NULL, NULL);

			bCancel = CreateWindowEx(WS_EX_LEFT, "Button", "Cancel",
							WS_VISIBLE | WS_CHILD | WS_TABSTOP,
							320, 45, 80, 25, hwnd, (HMENU)ID_CANCEL, NULL, NULL);

			bGenerate = CreateWindowEx(WS_EX_LEFT, "Button", "Generate password",
							WS_VISIBLE | WS_CHILD | WS_TABSTOP,
							280, 80, 160, 25, hwnd, (HMENU)ID_GENERATE, NULL, NULL);

			// get selected row
			wchar_t title[MAXLINE];
			SendMessageW(lbList, LB_GETTEXT, selectedRow, (LPARAM)title);

			// populate fields
			for (int i = 0; i < entryCount; ++i)
			{
				if (wcscmp(entries[i].title, title) == 0)
				{
					SetWindowTextW(eTitle, entries[i].title);
					SetWindowTextW(eId, entries[i].id);
					SetWindowTextW(ePw, entries[i].pw);
					SetWindowTextW(eMisc, entries[i].misc);
					break;
				}
			}

			SetFocus(bCancel);
			break;
		case WM_COMMAND:
			if (LOWORD(wParam) == ID_OK)
			{
				// get fields from edit boxes
				wchar_t title[MAXTITLE];
				wchar_t id[MAXID];
				wchar_t pw[MAXPW];
				wchar_t misc[MAXMISC];
				GetWindowTextW(eTitle, title, MAXTITLE);
				GetWindowTextW(eId, id, MAXID);
				GetWindowTextW(ePw, pw, MAXPW);
				GetWindowTextW(eMisc, misc, MAXMISC);

				// test that title field is not empty
				if (wcslen(title) == 0)
				{
					SetFocus(eTitle);
					return DefWindowProc(hwnd, msg, wParam, lParam);
				}

				// test for pre-existing title
				for (int i = 0; i < entryCount; ++i)
				{
					if (wcscmp(entries[i].title, title) == 0 && i != selectedRow)
					{
						MessageBox(NULL, "That title is already in use", "Error", MB_ICONEXCLAMATION | MB_OK);
						SetFocus(eTitle);
						return DefWindowProc(hwnd, msg, wParam, lParam);
					}
				}

				// replace entry in listbox & entries
				SendMessage(lbList, LB_DELETESTRING, selectedRow, 0);
				SendMessageW(lbList, LB_ADDSTRING, 0, (LPARAM)title);
				wcscpy(entries[selectedRow].title, title);
				wcscpy(entries[selectedRow].id, id);
				wcscpy(entries[selectedRow].pw, pw);
				wcscpy(entries[selectedRow].misc, misc);

				DestroyWindow(hwnd);
				updateListbox();
			}

			if (LOWORD(wParam) == ID_CANCEL)
			{
				DestroyWindow(hwnd);
			}

			if (LOWORD(wParam) == ID_GENERATE)
			{
				wchar_t password[MAXPW];
				generatePassword(password);
				SetWindowTextW(ePw, password);
			}
			break;
		case WM_DESTROY:
			DestroyWindow(hwnd);
			break;
	}
	return DefWindowProc(hwnd, msg, wParam, lParam);
}

void editSettings(void)
{
	static WNDCLASSEX wcSettings = {0};
	static bool settingsClassRegistered = false;

	if (!settingsClassRegistered)
	{
		wcSettings.cbSize = sizeof(WNDCLASSEX);
		wcSettings.cbClsExtra = 0;
		wcSettings.cbWndExtra = 0;
		wcSettings.hbrBackground = (HBRUSH)COLOR_WINDOW;
		wcSettings.hCursor = LoadCursor(NULL, IDC_ARROW);
		wcSettings.hIcon = NULL;
		wcSettings.hIconSm = NULL;
		wcSettings.hInstance = instance;
		wcSettings.lpfnWndProc = settingsWndProc;
		wcSettings.lpszClassName = "TroveSettings";
		wcSettings.lpszMenuName = NULL;
		wcSettings.style = CS_HREDRAW | CS_VREDRAW;

		if (!RegisterClassEx(&wcSettings))
		{
			MessageBox(NULL, "Settings window registration failed", "Error", MB_ICONEXCLAMATION | MB_OK);
			return;
		}
		else
			settingsClassRegistered = true;
	}

	settingsHwnd = CreateWindowEx(WS_EX_LEFT,
								wcSettings.lpszClassName,
								"Settings",
								WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX | WS_VISIBLE,
								CW_USEDEFAULT, CW_USEDEFAULT,
								410, 260,
								NULL, NULL,
								instance, NULL);

	if (!settingsHwnd)
	{
		MessageBox(NULL, "Settings window creation failed", "Error", MB_ICONEXCLAMATION | MB_OK);
		return;
	}

	ShowWindow(settingsHwnd, SW_SHOW);
}

LRESULT CALLBACK settingsWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	static HWND bOK, bCancel, lPassword, lSpecial, lNumeric, lUppercase, lKeygen, eKeygen, bGenerate, cPassword, cSpecial, cNumeric, cUppercase;

	switch (msg)
	{
		case WM_CREATE:
			char title[40];
			sprintf(title, "Password length from %d to %d", MINPW, MAXPW-1);

			lPassword = CreateWindowEx(WS_EX_LEFT, "Static", title,
							WS_VISIBLE | WS_CHILD,
							10, 15, 220, 25, hwnd, (HMENU)ID_SETTINGS_PW_LABEL, NULL, NULL);
			cPassword = CreateWindowEx(WS_EX_LEFT, "combobox", "",
							WS_VISIBLE | WS_CHILD | WS_TABSTOP | CBS_DROPDOWN | WS_VSCROLL,
							225, 10, 40, 100, hwnd, (HMENU)ID_SETTINGS_PW_LENGTH, NULL, NULL);
			fillDropdown(cPassword, MINPW, MAXPW-1);

			lSpecial = CreateWindowEx(WS_EX_LEFT, "Static", "Minimum special characters",
							WS_VISIBLE | WS_CHILD,
							10, 50, 220, 25, hwnd, (HMENU)ID_SETTINGS_SPECIAL_LABEL, NULL, NULL);
			cSpecial = CreateWindowEx(WS_EX_LEFT, "combobox", "",
							WS_VISIBLE | WS_CHILD | WS_TABSTOP | CBS_DROPDOWN | WS_VSCROLL,
							225, 45, 35, 100, hwnd, (HMENU)ID_SETTINGS_SPECIAL_LENGTH, NULL, NULL);
			fillDropdown(cSpecial, 0, MAXCHARS);

			lNumeric = CreateWindowEx(WS_EX_LEFT, "Static", "Minimum numeric characters",
							WS_VISIBLE | WS_CHILD,
							10, 85, 220, 25, hwnd, (HMENU)ID_SETTINGS_NUMERIC_LABEL, NULL, NULL);
			cNumeric = CreateWindowEx(WS_EX_LEFT, "combobox", "",
							WS_VISIBLE | WS_CHILD | WS_TABSTOP | CBS_DROPDOWN | WS_VSCROLL,
							225, 80, 35, 100, hwnd, (HMENU)ID_SETTINGS_NUMERIC_LENGTH, NULL, NULL);
			fillDropdown(cNumeric, 0, MAXCHARS);

			lUppercase = CreateWindowEx(WS_EX_LEFT, "Static", "Minimum uppercase characters",
							WS_VISIBLE | WS_CHILD,
							10, 120, 220, 25, hwnd, (HMENU)ID_SETTINGS_UPPERCASE_LABEL, NULL, NULL);
			cUppercase = CreateWindowEx(WS_EX_LEFT, "combobox", "",
							WS_VISIBLE | WS_CHILD | WS_TABSTOP | CBS_DROPDOWN | WS_VSCROLL,
							225, 115, 35, 100, hwnd, (HMENU)ID_SETTINGS_UPPERCASE_LENGTH, NULL, NULL);
			fillDropdown(cUppercase, 0, MAXCHARS);

			lKeygen = CreateWindowEx(WS_EX_LEFT, "Static", "Encryption keygen",
							WS_VISIBLE | WS_CHILD,
							10, 155, 220, 25, hwnd, (HMENU)ID_SETTINGS_KEYGEN_LABEL, NULL, NULL);
			eKeygen = CreateWindowEx(WS_EX_LEFT, "Edit", "",
							WS_VISIBLE | WS_CHILD | WS_BORDER | ES_READONLY,
							225, 150, 150, 25, hwnd, (HMENU)ID_SETTINGS_KEYGEN_LENGTH, NULL, NULL);

			bGenerate = CreateWindowEx(WS_EX_LEFT, "Button", "Generate keygen",
							WS_VISIBLE | WS_CHILD | WS_TABSTOP,
							235, 180, 130, 25, hwnd, (HMENU)ID_GENERATE, NULL, NULL);

			bOK = CreateWindowEx(WS_EX_LEFT, "Button", "OK",
							WS_VISIBLE | WS_CHILD | WS_TABSTOP,
							295, 10, 80, 25, hwnd, (HMENU)ID_OK, NULL, NULL);

			bCancel = CreateWindowEx(WS_EX_LEFT, "Button", "Cancel",
							WS_VISIBLE | WS_CHILD | WS_TABSTOP,
							295, 45, 80, 25, hwnd, (HMENU)ID_CANCEL, NULL, NULL);

			// populate each box
			char buf[4];
			sprintf(buf, "%d", passwordSize);
			SetWindowText(cPassword, buf);
			sprintf(buf, "%d", minNumeric);
			SetWindowText(cNumeric, buf);
			sprintf(buf, "%d", minSpecial);
			SetWindowText(cSpecial, buf);
			sprintf(buf, "%d", minUppercase);
			SetWindowText(cUppercase, buf);
			SetWindowText(eKeygen, iv);

			centerWindow(hwnd);
			break;
		case WM_COMMAND:
			if (LOWORD(wParam) == ID_OK)
			{
				bool valid = true;
				char password[5];
				char special[5];
				char numeric[5];
				char uppercase[5];
				GetWindowText(cPassword, password, 5);
				GetWindowText(cSpecial, special, 5);
				GetWindowText(cNumeric, numeric, 5);
				GetWindowText(cUppercase, uppercase, 5);

				// check password length
				if (strlen(password) == 0) // check if blank
				{
					char buf[5];
					sprintf(buf, "%d", passwordSize);
					SetWindowText(cPassword, buf);
					valid = false;
				}
				else // check if non-numeric
				{
					if (isNumeric(password))
						password[2] = '\0';
					else
					{
						char buf[5];
						sprintf(buf, "%d", passwordSize);
						SetWindowText(cPassword, buf);
						valid = false;
					}
				}

				// check special length
				if (strlen(special) == 0) // check if blank
				{
					char buf[5];
					sprintf(buf, "%d", minSpecial);
					SetWindowText(cSpecial, buf);
					valid = false;
				}
				else // check if non-numeric
				{
					if (isNumeric(special))
						special[1] = '\0';
					else
					{
						char buf[5];
						sprintf(buf, "%d", minSpecial);
						SetWindowText(cSpecial, buf);
						valid = false;
					}
				}

				// check numeric length
				if (strlen(numeric) == 0) // check if blank
				{
					char buf[5];
					sprintf(buf, "%d", minNumeric);
					SetWindowText(cNumeric, buf);
					valid = false;
				}
				else // check if non-numeric
				{
					if (isNumeric(numeric))
						numeric[1] = '\0';
					else
					{
						char buf[5];
						sprintf(buf, "%d", minNumeric);
						SetWindowText(cNumeric, buf);
						valid = false;
					}
				}

				// check uppercase length
				if (strlen(uppercase) == 0) // check if blank
				{
					char buf[5];
					sprintf(buf, "%d", minUppercase);
					SetWindowText(cUppercase, buf);
					valid = false;
				}
				else // check if non-numeric
				{
					if (isNumeric(uppercase))
						uppercase[1] = '\0';
					else
					{
						char buf[5];
						sprintf(buf, "%d", minUppercase);
						SetWindowText(cUppercase, buf);
						valid = false;
					}
				}

				int newPasswordSize = atoi(password);
				int newMinSpecial = atoi(special);
				int newMinNumeric = atoi(numeric);
				int newMinUppercase = atoi(uppercase);

				// check if combined sizes are a problem
				if (newMinSpecial + newMinNumeric + newMinUppercase > newPasswordSize)
				{
					MessageBox(NULL, "Combined minimum characters exceeds password size", "Error", MB_ICONEXCLAMATION | MB_OK);
					valid = false;
				}

				// check password size is between limits
				if (newPasswordSize < MINPW || newPasswordSize > MAXPW)
				{
					MessageBox(NULL, "Password size is outside min/max", "Error", MB_ICONEXCLAMATION | MB_OK);
					valid = false;
					SetFocus(cPassword);
				}

				if (valid)
				{
					passwordSize = newPasswordSize;
					minSpecial = newMinSpecial;
					minNumeric = newMinNumeric;
					minUppercase = newMinUppercase;
					writeSettings();
					DestroyWindow(hwnd);
				}
			}

			if (LOWORD(wParam) == ID_CANCEL)
			{
				DestroyWindow(hwnd);
			}

			if (LOWORD(wParam) == ID_GENERATE)
			{
				generateKeygen(iv);
				SetWindowText(eKeygen, iv);
			}
			break;
		case WM_DESTROY:
			DestroyWindow(hwnd);
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

void updateListbox(void)
{
	// deselect all entries
	SendMessage(lbList, LB_SETCURSEL, -1, 0);
	EnableWindow(bEdit, FALSE);
	EnableWindow(bDelete, FALSE);
	selectedRow = LB_ERR;
	sortEntries();
}

void deleteEntry(void)
{
	if (selectedRow != LB_ERR)
	{
		if (MessageBox(NULL, "Are you sure you want to delete this entry?", "Confirm delete", MB_YESNO | MB_ICONWARNING) == IDYES)
		{
			// delete row
			SendMessage(lbList, LB_DELETESTRING, selectedRow, 0);
			entries[selectedRow].title[0] = '\0';
			updateListbox();
		}
	}
}

//TODO replace with encrypted database
void loadEntries(void)
{
	FILE *f = fopen(tempFile, "r");
	if (f == NULL)
		return;

	char line[MAXLINE];
	wchar_t data[MAXLINE];
	entryCount = 0;
	entries = NULL;

	while (!feof(f))
	{
		if (fgets(line, MAXLINE, f) != NULL)
		{
			entries = realloc(entries, (entryCount+1) * sizeof(*entries));
			int field = 0;
			int line_ctr = 0;
			int data_ctr = 0;

			while (line[line_ctr] != '\0')
			{
				if (line[line_ctr] == ',' || line[line_ctr] == '\n')
				{
					++line_ctr;
					data[data_ctr] = '\0';
					data_ctr = 0;

					switch (field)
					{
						case 0:
							wcscpy(entries[entryCount].title, data);
							break;
						case 1:
							wcscpy(entries[entryCount].id, data);
							break;
						case 2:
							wcscpy(entries[entryCount].pw, data);
							break;
						case 3:
							wcscpy(entries[entryCount].misc, data);
							break;
					}
					++field;
					continue;
				}
				data[data_ctr++] = line[line_ctr++];
			}
			++entryCount;
		}
	}
	fclose(f);

	// remove all entries
	SendMessage(lbList, LB_RESETCONTENT, 0, 0);

	// add entries to listbox
	for (int i = 0; i < entryCount; ++i)
	{
		wchar_t row[MAXLINE];
		wcscpy(row, entries[i].title);
		SendMessageW(lbList, LB_ADDSTRING, i, (LPARAM)row);
	}
}

//TODO replace with encrypted database
void saveEntries(void)
{
	FILE *f = fopen(tempFile, "w");
	if (f == NULL)
	{
		puts("Error saving entries!");
		return;
	}

	for (int i = 0; i < entryCount; ++i)
		if (entries[i].title[0] != '\0')
			fprintf(f, "%ls,%ls,%ls,%ls\n", entries[i].title, entries[i].id, entries[i].pw, entries[i].misc);

	fclose(f);
}

void sortEntries(void)
{
	bool changed;

	do
	{
		changed = false;

		for (int i = 0; i < entryCount - 1; ++i)
		{
			if (wcscmp(entries[i].title, entries[i + 1].title) > 0) // s1 > s2
			{
				changed = true;
				struct Entry tmp;

				wcscpy(tmp.title, entries[i].title);
				wcscpy(tmp.id, entries[i].id);
				wcscpy(tmp.pw, entries[i].pw);
				wcscpy(tmp.misc, entries[i].misc);

				wcscpy(entries[i].title, entries[i + 1].title);
				wcscpy(entries[i].id, entries[i + 1].id);
				wcscpy(entries[i].pw, entries[i + 1].pw);
				wcscpy(entries[i].misc, entries[i + 1].misc);

				wcscpy(entries[i + 1].title, tmp.title);
				wcscpy(entries[i + 1].id, tmp.id);
				wcscpy(entries[i + 1].pw, tmp.pw);
				wcscpy(entries[i + 1].misc, tmp.misc);
			}
		}
	} while (changed);
}

// random chars from 33 to 126, except 44 (commas)
void generatePassword(wchar_t *buf)
{
	int specialCount;
	int numericCount;
	int uppercaseCount;

	do
	{
		specialCount = 0;
		numericCount = 0;
		uppercaseCount = 0;

		for (int i = 0; i < passwordSize; ++i)
		{
			int rn = rand() % 127;
			if (rn < 33 || rn == 44)
			{
				--i;
				continue;
			}
			buf[i] = (char)rn;

			if (ispunct((char)rn)) ++specialCount;
			if (isdigit((char)rn)) ++numericCount;
			if (isupper((char)rn)) ++uppercaseCount;
		}
		buf[passwordSize] = '\0';
	} while (specialCount < minSpecial ||
			numericCount < minNumeric ||
			uppercaseCount < minUppercase);
}

void outw(wchar_t *s)
{
	FILE *lf = fopen("log.txt", "a");
	if (lf == NULL)
	{
		MessageBox(NULL, "Can't open log file", "Error", MB_ICONEXCLAMATION | MB_OK);
		return;
	}

	fprintf(lf, "%ls\n", s);
	fclose(lf);
}

void outs(char *s)
{
	FILE *lf = fopen("log.txt", "a");
	if (lf == NULL)
	{
		MessageBox(NULL, "Can't open log file", "Error", MB_ICONEXCLAMATION | MB_OK);
		return;
	}

	fprintf(lf, "%s\n", s);
	fclose(lf);
}

// generate 16 random hex chars
void generateKeygen(char *buf)
{
	int rn;
	for (int i = 0; i < 16; ++i)
	{
		rn = rand() % 16;
		sprintf(buf + i, "%X", rn);
	}
}

//TODO handle 2nd char in password?
bool isNumeric(char *buf)
{
	if ((int)buf[0] < 48 || (int)buf[0] > 57)
		return false;
	else
		return true;
}

void fillDropdown(HWND hwnd, int min, int max)
{
	for (int i = min; i <= max; ++i)
	{
		char c[1];
		sprintf(c, "%d", i);
		SendMessage(hwnd, CB_ADDSTRING, 0, (LPARAM)c);
	}
}

void readSettings(void)
{
	FILE *f = fopen(iniFile, "r");
	if (f == NULL)
	{
		outs("Created settings file");
		writeSettings();
		return;
	}

	char line[MAXLINE];

	while (fgets(line, MAXLINE, f) != NULL)
	{
		char setting[MAXLINE];
		char value[MAXLINE];
		char *l = line;
		char *s = setting;
		char *v = value;

		for (int i = 0; i < MAXLINE; ++i)
		{
			setting[i] = '\0';
			value[i] = '\0';
		}

		// find setting
		while (*l && *l != '=')
		{
			*s = *l;
			s++;
			l++;
		}
		*s = '\0';

		// find value
		++l;
		while (*l)
		{
			*v = *l;
			l++;
			v++;
		}
		*v = '\0';

		if (strcmp(setting, "password_size") == 0)	passwordSize = atoi(value);
		if (strcmp(setting, "min_special") == 0)	minSpecial = atoi(value);
		if (strcmp(setting, "min_numeric") == 0)	minNumeric = atoi(value);
		if (strcmp(setting, "min_uppercase") == 0)	minUppercase = atoi(value);
		if (strcmp(setting, "keygen") == 0)			strncpy((char *)iv, value, 16);
	}
	fclose(f);
}

void writeSettings(void)
{
	FILE *f = fopen(iniFile, "w");
	if (f == NULL)
	{
		outs("Error saving entries!");
		return;
	}

	fprintf(f, "password_size=%d\n", passwordSize);
	fprintf(f, "min_special=%d\n", minSpecial);
	fprintf(f, "min_numeric=%d\n", minNumeric);
	fprintf(f, "min_uppercase=%d\n", minUppercase);
	fprintf(f, "keygen=%s\n", (char *)iv);
	fclose(f);
}
