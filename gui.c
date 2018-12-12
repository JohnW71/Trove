#define _CRT_SECURE_NO_WARNINGS

#include "gui.h"
#include "shared.h"

HWND lbList;
HWND bAdd;
HWND bEdit;
HWND bDelete;
HWND bSettings;
HWND bQuit;
HWND eFind;

static HINSTANCE instance;
static HWND mainHwnd;
static HWND addHwnd;
static HWND editHwnd;
static HWND settingsHwnd;
static HWND setPasswordHwnd;
static HWND getPasswordHwnd;
static HWND bFind;
static HWND bGetPasswordOK;
static HWND bSetPasswordOK;
static WNDPROC originalFindProc;
static WNDPROC originalListboxProc;
static WNDPROC originalAddProc;
static WNDPROC originalAddMiscProc;
static WNDPROC originalEditProc;
static WNDPROC originalEditMiscProc;
static WNDPROC originalSetPasswordProc;
static WNDPROC originalGetPasswordProc;

int WINAPI wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance,
	_In_ LPWSTR lpCmdLine, _In_ int nShowCmd)
{
	srand((unsigned int)time(NULL));
	instance = hInstance;
	MSG msg = {0};
	WNDCLASSEX wc = {0};

	// reset log file
	FILE *lf = fopen(LOG_FILE, "w");
	if (lf == NULL)
		MessageBox(NULL, "Can't open log file", "Error", MB_ICONEXCLAMATION | MB_OK);
	else
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

	state.entryCount = 0;
	state.debugging = false;
	state.readVerified = false;
	state.running = true;
	state.noDatabase = false;
	state.changingPassword = false;
	readSettings(INI_FILE);

	mainHwnd = CreateWindowEx(WS_EX_LEFT,
		wc.lpszClassName,
		"Trove",
		WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX,
		CW_USEDEFAULT, CW_USEDEFAULT, 390, 600,
		NULL, NULL, hInstance, NULL);

	if (!mainHwnd)
	{
		MessageBox(NULL, "Window creation failed", "Error", MB_ICONEXCLAMATION | MB_OK);
		return 0;
	}

	while (state.running)
	{
		while (GetMessage(&msg, NULL, 0, 0))
		{
			if (!IsDialogMessage(mainHwnd, &msg) &&
				!IsDialogMessage(addHwnd, &msg) &&
				!IsDialogMessage(editHwnd, &msg) &&
				!IsDialogMessage(settingsHwnd, &msg) &&
				!IsDialogMessage(setPasswordHwnd, &msg) &&
				!IsDialogMessage(getPasswordHwnd, &msg))
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
	switch (msg)
	{
		case WM_CREATE:
			if (settings.screenRow == 0 && settings.screenCol == 0)
				centerWindow(hwnd);
			else
				SetWindowPos(hwnd, HWND_TOP, settings.screenCol, settings.screenRow, 0, 0, SWP_NOSIZE);

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
			originalFindProc = (WNDPROC)SetWindowLongPtr(eFind, GWLP_WNDPROC, (LONG_PTR)customFindProc);

			bFind = CreateWindowEx(WS_EX_LEFT, "Button", "Find",
				WS_VISIBLE | WS_CHILD | WS_TABSTOP | WS_DISABLED,
				205, 45, 40, 25, hwnd, (HMENU)ID_MAIN_FIND, NULL, NULL);
			bQuit = CreateWindowEx(WS_EX_LEFT, "Button", "Quit",
				WS_VISIBLE | WS_CHILD | WS_TABSTOP,
				280, 45, 80, 25, hwnd, (HMENU)ID_MAIN_QUIT, NULL, NULL);

			// listbox
			lbList = CreateWindowEx(WS_EX_LEFT, "ListBox", NULL,
				WS_VISIBLE | WS_CHILD | LBS_STANDARD,
				10, 80, 350, 475, hwnd, (HMENU)ID_MAIN_LISTBOX, NULL, NULL);
			originalListboxProc = (WNDPROC)SetWindowLongPtr(lbList, GWLP_WNDPROC, (LONG_PTR)customListboxProc);

			readFile(DB_FILE);

			if (state.noDatabase)
				setNewDBpassword();
			else
				getDBpassword();

			break;
		case WM_COMMAND:
			if (LOWORD(wParam) == ID_MAIN_ADD)
			{
				SetWindowText(eFind, "");
				addEntry();
			}

			if (LOWORD(wParam) == ID_MAIN_EDIT)
			{
				SetWindowText(eFind, "");
				editEntry();
			}

			if (LOWORD(wParam) == ID_MAIN_DELETE)
			{
				SetWindowText(eFind, "");
				deleteEntry();
			}

			if (LOWORD(wParam) == ID_MAIN_SETTINGS)
			{
				SetWindowText(eFind, "");
				editSettings();
			}

			if (LOWORD(wParam) == ID_MAIN_FINDTEXT)
			{
				if (HIWORD(wParam) == EN_SETFOCUS)
				{
					// deselect all entries
					EnableWindow(bAdd, FALSE);
					EnableWindow(bEdit, FALSE);
					EnableWindow(bDelete, FALSE);
					SendMessage(lbList, LB_SETCURSEL, (WPARAM)-1, 0);
				}

				// find text was changed
				if (HIWORD(wParam) == EN_CHANGE)
				{
					// get text length
					char findText[MAXTITLE];
					GetWindowText(eFind, findText, MAXTITLE);

					// enable Find button if text is present
					if (strlen(findText) > 0)
					{
						EnableWindow(bAdd, FALSE);
						EnableWindow(bFind, TRUE);
					}
					else
					{
						EnableWindow(bAdd, TRUE);
						EnableWindow(bFind, FALSE);
					}

					// deselect all entries
					SendMessage(lbList, LB_SETCURSEL, (WPARAM)-1, 0);
				}
			}

			if (LOWORD(wParam) == ID_MAIN_FIND)
			{
				// find button was clicked
				if (HIWORD(wParam) == BN_CLICKED)
				{
					bool found = false;
					char find[MAXTITLE];
					GetWindowText(eFind, find, MAXTITLE);

					// deselect all entries
					SendMessage(lbList, LB_SETCURSEL, (WPARAM)-1, 0);

					static int i = 0;
					int attempts = 0;

					while (attempts <= state.entryCount)
					{
						if (i == state.selectedRow && strstr(entries[i].title, find))
							found = true;

						if (i != state.selectedRow && strstr(entries[i].title, find))
						{
							state.selectedRow = i;
							found = true;
							break;
						}

						++attempts;

						if (++i == state.entryCount)
							i = 0;
					}

					if (found && state.selectedRow != LB_ERR) // select matching row
					{
						SendMessage(lbList, LB_SETCURSEL, state.selectedRow, 0);
						EnableWindow(bAdd, FALSE);
						EnableWindow(bEdit, TRUE);
						EnableWindow(bDelete, TRUE);
						SendMessage(bEdit, BM_SETSTYLE, (WPARAM)BS_DEFPUSHBUTTON, TRUE);
						SetFocus(bEdit);
					}
				}
			}

			if (LOWORD(wParam) == ID_MAIN_QUIT)
			{
				shutDown(mainHwnd);
			}

			if (LOWORD(wParam) == ID_MAIN_LISTBOX)
			{
				// a row was selected
				if (HIWORD(wParam) == LBN_SELCHANGE)
				{
					// get row index
					state.selectedRow = SendMessage(lbList, LB_GETCURSEL, 0, 0);

					if (state.selectedRow != LB_ERR)
					{
						EnableWindow(bEdit, TRUE);
						EnableWindow(bDelete, TRUE);
					}
				}

				// a row was double-clicked
				if (HIWORD(wParam) == LBN_DBLCLK)
				{
					// get row index
					state.selectedRow = SendMessage(lbList, LB_GETCURSEL, 0, 0);

					if (state.selectedRow != LB_ERR)
						editEntry();
				}
			}
			break;
		case WM_DESTROY:
			shutDown(mainHwnd);
			break;
	}
	return DefWindowProc(hwnd, msg, wParam, lParam);
}

void addEntry(void)
{
	if (state.debugging)
		outs("addEntry()");

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
		640, 240,
		NULL, NULL,
		instance, NULL);

	if (!addHwnd)
	{
		MessageBox(NULL, "Add window creation failed", "Error", MB_ICONEXCLAMATION | MB_OK);
		return;
	}

	disableControls();
	ShowWindow(addHwnd, SW_SHOW);
}

LRESULT CALLBACK addWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	static HWND bOK, bCancel, bGenerate, lTitle, eTitle, lTitleCount;
	static HWND lId, eId, lIdCount, lPw, ePw, lPwCount, lMisc, eMisc, lMiscCount;

	switch (msg)
	{
		case WM_CREATE:
			SetTimer(hwnd, ID_TIMER3, 100, NULL);
			centerWindow(hwnd);

			lTitle = CreateWindowEx(WS_EX_LEFT, "Static", "Title",
				WS_VISIBLE | WS_CHILD,
				10, 10, 80, 25, hwnd, (HMENU)ID_EDIT_TITLE_LABEL, NULL, NULL);
			eTitle = CreateWindowEx(WS_EX_LEFT, "Edit", "",
				WS_VISIBLE | WS_CHILD | WS_BORDER | WS_TABSTOP,
				90, 10, 320, 25, hwnd, (HMENU)ID_EDIT_TITLE, NULL, NULL);
			lTitleCount = CreateWindowEx(WS_EX_LEFT, "Static", "0/0",
				WS_VISIBLE | WS_CHILD,
				415, 15, 40, 25, hwnd, (HMENU)ID_EDIT_TITLE_COUNT, NULL, NULL);
			originalAddProc = (WNDPROC)SetWindowLongPtr(eTitle, GWLP_WNDPROC, (LONG_PTR)customAddProc);

			lId = CreateWindowEx(WS_EX_LEFT, "Static", "ID",
				WS_VISIBLE | WS_CHILD,
				10, 45, 80, 25, hwnd, (HMENU)ID_EDIT_ID_LABEL, NULL, NULL);
			eId = CreateWindowEx(WS_EX_LEFT, "Edit", "",
				WS_VISIBLE | WS_CHILD | WS_BORDER | WS_TABSTOP,
				90, 45, 320, 25, hwnd, (HMENU)ID_EDIT_ID, NULL, NULL);
			lIdCount = CreateWindowEx(WS_EX_LEFT, "Static", "0/0",
				WS_VISIBLE | WS_CHILD,
				415, 50, 40, 25, hwnd, (HMENU)ID_EDIT_ID_COUNT, NULL, NULL);
			SetWindowLongPtr(eId, GWLP_WNDPROC, (LONG_PTR)customAddProc);

			lPw = CreateWindowEx(WS_EX_LEFT, "Static", "Password",
				WS_VISIBLE | WS_CHILD,
				10, 80, 80, 25, hwnd, (HMENU)ID_EDIT_PW_LABEL, NULL, NULL);
			ePw = CreateWindowEx(WS_EX_LEFT, "Edit", "",
				WS_VISIBLE | WS_CHILD | WS_BORDER | WS_TABSTOP,
				90, 80, 200, 25, hwnd, (HMENU)ID_EDIT_PW, NULL, NULL);
			lPwCount = CreateWindowEx(WS_EX_LEFT, "Static", "0/0",
				WS_VISIBLE | WS_CHILD,
				295, 85, 40, 25, hwnd, (HMENU)ID_EDIT_PW_COUNT, NULL, NULL);
			SetWindowLongPtr(ePw, GWLP_WNDPROC, (LONG_PTR)customAddProc);

			lMisc = CreateWindowEx(WS_EX_LEFT, "Static", "Misc",
				WS_VISIBLE | WS_CHILD,
				10, 115, 80, 25, hwnd, (HMENU)ID_EDIT_MISC_LABEL, NULL, NULL);
			eMisc = CreateWindowEx(WS_EX_LEFT, "Edit", "",
				WS_VISIBLE | WS_CHILD | WS_BORDER | WS_TABSTOP | ES_MULTILINE,
				90, 115, 200, 75, hwnd, (HMENU)ID_EDIT_MISC, NULL, NULL);
			lMiscCount = CreateWindowEx(WS_EX_LEFT, "Static", "0/0",
				WS_VISIBLE | WS_CHILD,
				295, 120, 40, 25, hwnd, (HMENU)ID_EDIT_MISC_COUNT, NULL, NULL);
			originalAddMiscProc = (WNDPROC)SetWindowLongPtr(eMisc, GWLP_WNDPROC, (LONG_PTR)customAddMiscProc);

			bOK = CreateWindowEx(WS_EX_LEFT, "Button", "OK",
				WS_VISIBLE | WS_CHILD | WS_TABSTOP,
				480, 45, 80, 25, hwnd, (HMENU)ID_OK, NULL, NULL);

			bCancel = CreateWindowEx(WS_EX_LEFT, "Button", "Cancel",
				WS_VISIBLE | WS_CHILD | WS_TABSTOP,
				480, 80, 80, 25, hwnd, (HMENU)ID_CANCEL, NULL, NULL);

			bGenerate = CreateWindowEx(WS_EX_LEFT, "Button", "Generate password",
				WS_VISIBLE | WS_CHILD | WS_TABSTOP,
				440, 115, 160, 25, hwnd, (HMENU)ID_GENERATE, NULL, NULL);

			SendMessage(eTitle, EM_LIMITTEXT, MAXTITLE-1, 0);
			SendMessage(eId, EM_LIMITTEXT, MAXID-1, 0);
			SendMessage(ePw, EM_LIMITTEXT, MAXPW-1, 0);
			SendMessage(eMisc, EM_LIMITTEXT, MAXMISC-1, 0);
			SetFocus(eTitle);
			break;
		case WM_TIMER:
			if (wParam == ID_TIMER3)
			{
				int i;
				char buf[MAXMISC];

				i = GetWindowTextLength(eTitle);
				sprintf(buf, "%2d/%2d", i, MAXTITLE-1);
				SetWindowText(lTitleCount, buf);

				i = GetWindowTextLength(eId);
				sprintf(buf, "%2d/%2d", i, MAXID-1);
				SetWindowText(lIdCount, buf);

				i = GetWindowTextLength(ePw);
				sprintf(buf, "%2d/%2d", i, MAXPW-1);
				SetWindowText(lPwCount, buf);

				i = GetWindowTextLength(eMisc);
				sprintf(buf, "%2d/%2d", i, MAXMISC-1);
				SetWindowText(lMiscCount, buf);
			}
			break;
		case WM_KEYUP:
			switch (wParam)
			{
				case VK_ESCAPE:
					DestroyWindow(addHwnd);
					break;
			}
			break;
		case WM_COMMAND:
			if (LOWORD(wParam) == ID_OK)
			{
				// get fields from edit boxes
				char title[MAXTITLE];
				char id[MAXID];
				char pw[MAXPW];
				char misc[MAXMISC];
				GetWindowText(eTitle, title, MAXTITLE);
				GetWindowText(eId, id, MAXID);
				GetWindowText(ePw, pw, MAXPW);
				GetWindowText(eMisc, misc, MAXMISC);

				// test that title field is not empty
				if (strlen(title) == 0)
					return DefWindowProc(hwnd, msg, wParam, lParam);

				// test for pre-existing title
				for (int i = 0; i < state.entryCount; ++i)
				{
					if (strcmp(entries[i].title, title) == 0 && i != state.selectedRow)
					{
						MessageBox(NULL, "That title is already in use", "Error", MB_ICONEXCLAMATION | MB_OK);
						SetFocus(eTitle);
						return DefWindowProc(hwnd, msg, wParam, lParam);
					}
				}

				// avoid any completely blank fields
				if (strlen(id) == 0) strcpy(id, " ");
				if (strlen(pw) == 0) strcpy(pw, " ");
				if (strlen(misc) == 0) strcpy(misc, " ");

				// add to listbox & entries
				SendMessage(lbList, LB_ADDSTRING, 0, (LPARAM)title);

				struct Entry *temp = realloc(entries, ((uint64_t)state.entryCount + 1) * sizeof(*entries));
				if (temp == NULL)
				{
					outs("Failure reallocating memory for new entry");
					MessageBox(NULL, "Failed reallocating memory for new entry", "Error", MB_ICONEXCLAMATION | MB_OK);
					return DefWindowProc(hwnd, msg, wParam, lParam);
				}
				entries = temp;

				removeCommas(title, (int)strlen(title));
				removeCommas(id, (int)strlen(id));
				removeCommas(misc, (int)strlen(misc));
				strcpy(entries[state.entryCount].title, title);
				strcpy(entries[state.entryCount].id, id);
				strcpy(entries[state.entryCount].pw, pw);
				strcpy(entries[state.entryCount].misc, misc);
				++state.entryCount;

				DestroyWindow(hwnd);
				updateListbox();
			}

			if (LOWORD(wParam) == ID_CANCEL)
			{
				DestroyWindow(hwnd);
			}

			if (LOWORD(wParam) == ID_GENERATE)
			{
				char password[MAXPW];
				generatePassword(password);
				SetWindowText(ePw, password);
			}
			break;
		case WM_DESTROY:
			KillTimer(addHwnd, ID_TIMER3);
			enableControls();
			DestroyWindow(hwnd);
			break;
	}
	return DefWindowProc(hwnd, msg, wParam, lParam);
}

void editEntry(void)
{
	if (state.debugging)
		outs("editEntry()");

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
		640, 240,
		NULL, NULL,
		instance, NULL);

	if (!editHwnd)
	{
		MessageBox(NULL, "Edit window creation failed", "Error", MB_ICONEXCLAMATION | MB_OK);
		return;
	}

	disableControls();
	ShowWindow(editHwnd, SW_SHOW);
}

LRESULT CALLBACK editWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	static HWND eTitle, eId, ePw, eMisc, lTitleCount, lIdCount, lPwCount, lMiscCount;

	switch (msg)
	{
		case WM_CREATE:
		{
			static HWND bOK, bCancel, bGenerate, lTitle, lId, lPw, lMisc;
			SetTimer(hwnd, ID_TIMER3, 100, NULL);
			centerWindow(hwnd);

			lTitle = CreateWindowEx(WS_EX_LEFT, "Static", "Title",
				WS_VISIBLE | WS_CHILD,
				10, 10, 80, 25, hwnd, (HMENU)ID_EDIT_TITLE_LABEL, NULL, NULL);
			eTitle = CreateWindowEx(WS_EX_LEFT, "Edit", "",
				WS_VISIBLE | WS_CHILD | WS_BORDER | WS_TABSTOP,
				90, 10, 320, 25, hwnd, (HMENU)ID_EDIT_TITLE, NULL, NULL);
			lTitleCount = CreateWindowEx(WS_EX_LEFT, "Static", "0/0",
				WS_VISIBLE | WS_CHILD,
				415, 15, 40, 25, hwnd, (HMENU)ID_EDIT_TITLE_COUNT, NULL, NULL);
			originalEditProc = (WNDPROC)SetWindowLongPtr(eTitle, GWLP_WNDPROC, (LONG_PTR)customEditProc);

			lId = CreateWindowEx(WS_EX_LEFT, "Static", "ID",
				WS_VISIBLE | WS_CHILD,
				10, 45, 80, 25, hwnd, (HMENU)ID_EDIT_ID_LABEL, NULL, NULL);
			eId = CreateWindowEx(WS_EX_LEFT, "Edit", "",
				WS_VISIBLE | WS_CHILD | WS_BORDER | WS_TABSTOP,
				90, 45, 320, 25, hwnd, (HMENU)ID_EDIT_ID, NULL, NULL);
			lIdCount = CreateWindowEx(WS_EX_LEFT, "Static", "0/0",
				WS_VISIBLE | WS_CHILD,
				415, 50, 40, 25, hwnd, (HMENU)ID_EDIT_ID_COUNT, NULL, NULL);
			SetWindowLongPtr(eId, GWLP_WNDPROC, (LONG_PTR)customEditProc);

			lPw = CreateWindowEx(WS_EX_LEFT, "Static", "Password",
				WS_VISIBLE | WS_CHILD,
				10, 80, 80, 25, hwnd, (HMENU)ID_EDIT_PW_LABEL, NULL, NULL);
			ePw = CreateWindowEx(WS_EX_LEFT, "Edit", "",
				WS_VISIBLE | WS_CHILD | WS_BORDER | WS_TABSTOP,
				90, 80, 200, 25, hwnd, (HMENU)ID_EDIT_PW, NULL, NULL);
			lPwCount = CreateWindowEx(WS_EX_LEFT, "Static", "0/0",
				WS_VISIBLE | WS_CHILD,
				295, 85, 40, 25, hwnd, (HMENU)ID_EDIT_PW_COUNT, NULL, NULL);
			SetWindowLongPtr(ePw, GWLP_WNDPROC, (LONG_PTR)customEditProc);

			lMisc = CreateWindowEx(WS_EX_LEFT, "Static", "Misc",
				WS_VISIBLE | WS_CHILD,
				10, 115, 80, 25, hwnd, (HMENU)ID_EDIT_MISC_LABEL, NULL, NULL);
			eMisc = CreateWindowEx(WS_EX_LEFT, "Edit", "",
				WS_VISIBLE | WS_CHILD | WS_BORDER | WS_TABSTOP | ES_MULTILINE,
				90, 115, 200, 75, hwnd, (HMENU)ID_EDIT_MISC, NULL, NULL);
			lMiscCount = CreateWindowEx(WS_EX_LEFT, "Static", "0/0",
				WS_VISIBLE | WS_CHILD,
				295, 120, 40, 25, hwnd, (HMENU)ID_EDIT_MISC_COUNT, NULL, NULL);
			originalEditMiscProc = (WNDPROC)SetWindowLongPtr(eMisc, GWLP_WNDPROC, (LONG_PTR)customEditMiscProc);

			bOK = CreateWindowEx(WS_EX_LEFT, "Button", "OK",
				WS_VISIBLE | WS_CHILD | WS_TABSTOP,
				480, 45, 80, 25, hwnd, (HMENU)ID_OK, NULL, NULL);

			bCancel = CreateWindowEx(WS_EX_LEFT, "Button", "Cancel",
				WS_VISIBLE | WS_CHILD | WS_TABSTOP,
				480, 80, 80, 25, hwnd, (HMENU)ID_CANCEL, NULL, NULL);

			bGenerate = CreateWindowEx(WS_EX_LEFT, "Button", "Generate password",
				WS_VISIBLE | WS_CHILD | WS_TABSTOP,
				440, 115, 160, 25, hwnd, (HMENU)ID_GENERATE, NULL, NULL);

			// get selected row
			char title[MAXLINE];
			int textLen = (int)SendMessage(lbList, LB_GETTEXT, state.selectedRow, (LPARAM)title);
			title[textLen] = '\0';

			// populate fields
			for (int i = 0; i < state.entryCount; ++i)
			{
				if (strcmp(entries[i].title, title) == 0)
				{
					SetWindowText(eTitle, entries[i].title);

					if (strlen(entries[i].id) == 1 && entries[i].id[0] == ' ')
						SetWindowText(eId, "");
					else
						SetWindowText(eId, entries[i].id);

					if (strlen(entries[i].pw) == 1 && entries[i].pw[0] == ' ')
						SetWindowText(ePw, "");
					else
						SetWindowText(ePw, entries[i].pw);

					if (strlen(entries[i].misc) == 1 && entries[i].misc[0] == ' ')
						SetWindowText(eMisc, "");
					else
						SetWindowText(eMisc, entries[i].misc);

					break;
				}
			}

			SendMessage(eTitle, EM_LIMITTEXT, MAXTITLE-1, 0);
			SendMessage(eId, EM_LIMITTEXT, MAXID-1, 0);
			SendMessage(ePw, EM_LIMITTEXT, MAXPW-1, 0);
			SendMessage(eMisc, EM_LIMITTEXT, MAXMISC-1, 0);
			SetFocus(eTitle);
			break;
		}
		case WM_TIMER:
			if (wParam == ID_TIMER3)
			{
				int i;
				char buf[MAXMISC];

				i = GetWindowTextLength(eTitle);
				sprintf(buf, "%2d/%2d", i, MAXTITLE-1);
				SetWindowText(lTitleCount, buf);

				i = GetWindowTextLength(eId);
				sprintf(buf, "%2d/%2d", i, MAXID-1);
				SetWindowText(lIdCount, buf);

				i = GetWindowTextLength(ePw);
				sprintf(buf, "%2d/%2d", i, MAXPW-1);
				SetWindowText(lPwCount, buf);

				i = GetWindowTextLength(eMisc);
				sprintf(buf, "%2d/%2d", i, MAXMISC-1);
				SetWindowText(lMiscCount, buf);
			}
			break;
		case WM_KEYUP:
			switch (wParam)
			{
				case VK_ESCAPE:
					DestroyWindow(addHwnd);
					break;
			}
			break;
		case WM_COMMAND:
			if (LOWORD(wParam) == ID_OK)
			{
				// get fields from edit boxes
				char title[MAXTITLE];
				char id[MAXID];
				char pw[MAXPW];
				char misc[MAXMISC];
				GetWindowText(eTitle, title, MAXTITLE);
				GetWindowText(eId, id, MAXID);
				GetWindowText(ePw, pw, MAXPW);
				GetWindowText(eMisc, misc, MAXMISC);

				// test that title field is not empty
				if (strlen(title) == 0)
					return DefWindowProc(hwnd, msg, wParam, lParam);

				// test for pre-existing title
				for (int i = 0; i < state.entryCount; ++i)
				{
					if (strcmp(entries[i].title, title) == 0 && i != state.selectedRow)
					{
						MessageBox(NULL, "That title is already in use", "Error", MB_ICONEXCLAMATION | MB_OK);
						SetFocus(eTitle);
						return DefWindowProc(hwnd, msg, wParam, lParam);
					}
				}

				// avoid any completely blank fields
				if (strlen(id) == 0) strcpy(id, " ");
				if (strlen(pw) == 0) strcpy(pw, " ");
				if (strlen(misc) == 0) strcpy(misc, " ");

				// replace entry in listbox & entries
				removeCommas(title, (int)strlen(title));
				removeCommas(id, (int)strlen(id));
				removeCommas(misc, (int)strlen(misc));
				SendMessage(lbList, LB_DELETESTRING, state.selectedRow, 0);
				SendMessage(lbList, LB_ADDSTRING, 0, (LPARAM)title);
				strcpy(entries[state.selectedRow].title, title);
				strcpy(entries[state.selectedRow].id, id);
				strcpy(entries[state.selectedRow].pw, pw);
				strcpy(entries[state.selectedRow].misc, misc);

				DestroyWindow(hwnd);
				updateListbox();
			}

			if (LOWORD(wParam) == ID_CANCEL)
			{
				DestroyWindow(hwnd);
				updateListbox();
			}

			if (LOWORD(wParam) == ID_GENERATE)
			{
				char password[MAXPW];
				generatePassword(password);
				SetWindowText(ePw, password);
			}
			break;
		case WM_DESTROY:
			KillTimer(editHwnd, ID_TIMER3);
			enableControls();
			DestroyWindow(hwnd);
			break;
	}
	return DefWindowProc(hwnd, msg, wParam, lParam);
}

void editSettings(void)
{
	if (state.debugging)
		outs("editSettings()");

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
		410, 300,
		NULL, NULL,
		instance, NULL);

	if (!settingsHwnd)
	{
		MessageBox(NULL, "Settings window creation failed", "Error", MB_ICONEXCLAMATION | MB_OK);
		return;
	}

	disableControls();
	ShowWindow(settingsHwnd, SW_SHOW);
}

LRESULT CALLBACK settingsWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	static HWND eKeygen, cPassword, cSpecial, cNumeric, cUppercase;

	switch (msg)
	{
		case WM_CREATE:
		{
			static HWND bOK, bCancel, bSetPassword, lPassword, lSpecial, lNumeric, lUppercase, lKeygen, bGenerate, lVersion;
			char title[40];
			sprintf(title, "Password length from %d to %d", MINPW, MAXPW - 1);

			lPassword = CreateWindowEx(WS_EX_LEFT, "Static", title,
				WS_VISIBLE | WS_CHILD,
				10, 15, 220, 25, hwnd, (HMENU)ID_SETTINGS_PW_LABEL, NULL, NULL);
			cPassword = CreateWindowEx(WS_EX_LEFT, "combobox", "",
				WS_VISIBLE | WS_CHILD | WS_TABSTOP | CBS_DROPDOWN | WS_VSCROLL,
				225, 10, 40, 100, hwnd, (HMENU)ID_SETTINGS_PW_LENGTH, NULL, NULL);
			fillDropdown(cPassword, MINPW, MAXPW - 1);

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

			bSetPassword = CreateWindowEx(WS_EX_LEFT, "Button", "Change database password",
				WS_VISIBLE | WS_CHILD | WS_TABSTOP,
				100, 215, 210, 25, hwnd, (HMENU)ID_SETTINGS_SET_PASSWORD, NULL, NULL);

			lVersion = CreateWindowEx(WS_EX_LEFT, "Static", APP_VERSION,
				WS_VISIBLE | WS_CHILD,
				320, 85, 210, 25, hwnd, (HMENU)ID_SETTINGS_VERSION, NULL, NULL);

			// populate each box
			char buf[4];
			sprintf(buf, "%d", settings.passwordSize);
			SetWindowText(cPassword, buf);
			sprintf(buf, "%d", settings.minNumeric);
			SetWindowText(cNumeric, buf);
			sprintf(buf, "%d", settings.minSpecial);
			SetWindowText(cSpecial, buf);
			sprintf(buf, "%d", settings.minUppercase);
			SetWindowText(cUppercase, buf);
			SetWindowText(eKeygen, settings.iv);

			centerWindow(hwnd);
			break;
		}
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
					sprintf(buf, "%d", settings.passwordSize);
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
						sprintf(buf, "%d", settings.passwordSize);
						SetWindowText(cPassword, buf);
						valid = false;
					}
				}

				// check special length
				if (strlen(special) == 0) // check if blank
				{
					char buf[5];
					sprintf(buf, "%d", settings.minSpecial);
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
						sprintf(buf, "%d", settings.minSpecial);
						SetWindowText(cSpecial, buf);
						valid = false;
					}
				}

				// check numeric length
				if (strlen(numeric) == 0) // check if blank
				{
					char buf[5];
					sprintf(buf, "%d", settings.minNumeric);
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
						sprintf(buf, "%d", settings.minNumeric);
						SetWindowText(cNumeric, buf);
						valid = false;
					}
				}

				// check uppercase length
				if (strlen(uppercase) == 0) // check if blank
				{
					char buf[5];
					sprintf(buf, "%d", settings.minUppercase);
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
						sprintf(buf, "%d", settings.minUppercase);
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
					settings.passwordSize = newPasswordSize;
					settings.minSpecial = newMinSpecial;
					settings.minNumeric = newMinNumeric;
					settings.minUppercase = newMinUppercase;
					writeSettings(INI_FILE);
					DestroyWindow(hwnd);
				}
			}

			if (LOWORD(wParam) == ID_CANCEL)
			{
				DestroyWindow(hwnd);
			}

			if (LOWORD(wParam) == ID_GENERATE)
			{
				generateKeygen(settings.iv);
				SetWindowText(eKeygen, settings.iv);
			}

			if (LOWORD(wParam) == ID_SETTINGS_SET_PASSWORD)
			{
				state.changingPassword = true;
				setNewDBpassword();
			}
			break;
		case WM_DESTROY:
			enableControls();
			DestroyWindow(hwnd);
			break;
	}
	return DefWindowProc(hwnd, msg, wParam, lParam);
}

void setNewDBpassword(void)
{
	if (state.debugging)
		outs("setNewDBpassword()");

	static WNDCLASSEX wcSetPassword = {0};
	static bool setPasswordClassRegistered = false;

	if (!setPasswordClassRegistered)
	{
		wcSetPassword.cbSize = sizeof(WNDCLASSEX);
		wcSetPassword.cbClsExtra = 0;
		wcSetPassword.cbWndExtra = 0;
		wcSetPassword.hbrBackground = (HBRUSH)COLOR_WINDOW;
		wcSetPassword.hCursor = LoadCursor(NULL, IDC_ARROW);
		wcSetPassword.hIcon = NULL;
		wcSetPassword.hIconSm = NULL;
		wcSetPassword.hInstance = instance;
		wcSetPassword.lpfnWndProc = setPasswordWndProc;
		wcSetPassword.lpszClassName = "TroveSetPassword";
		wcSetPassword.lpszMenuName = NULL;
		wcSetPassword.style = CS_HREDRAW | CS_VREDRAW;

		if (!RegisterClassEx(&wcSetPassword))
		{
			MessageBox(NULL, "Set password window registration failed", "Error", MB_ICONEXCLAMATION | MB_OK);
			return;
		}
		else
			setPasswordClassRegistered = true;
	}

	setPasswordHwnd = CreateWindowEx(WS_EX_LEFT,
		wcSetPassword.lpszClassName,
		"Set New DB Password",
		WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX | WS_VISIBLE,
		CW_USEDEFAULT, CW_USEDEFAULT,
		490, 170,
		NULL, NULL,
		instance, NULL);

	if (!setPasswordHwnd)
	{
		MessageBox(NULL, "Set Password window creation failed", "Error", MB_ICONEXCLAMATION | MB_OK);
		return;
	}

	disableControls();
	ShowWindow(setPasswordHwnd, SW_SHOW);
}

LRESULT CALLBACK setPasswordWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	static HWND bCancel, bGenerate, lMessage, lPassword1, lPassword2, eSetPassword1, eSetPassword2;
	static bool closeEverything = true;

	switch (msg)
	{
		case WM_CREATE:
			lPassword1 = CreateWindowEx(WS_EX_LEFT, "Static", "Enter new password",
				WS_VISIBLE | WS_CHILD,
				10, 15, 150, 25, hwnd, (HMENU)ID_PASSWORD_LABEL1, NULL, NULL);
			eSetPassword1 = CreateWindowEx(WS_EX_LEFT, "Edit", NULL,
				WS_VISIBLE | WS_CHILD | WS_TABSTOP | WS_BORDER,
				165, 10, 170, 25, hwnd, (HMENU)ID_PASSWORD_PASSWORD1, NULL, NULL);
			originalSetPasswordProc = (WNDPROC)SetWindowLongPtr(eSetPassword1, GWLP_WNDPROC, (LONG_PTR)customSetPasswordProc);

			lPassword2 = CreateWindowEx(WS_EX_LEFT, "Static", "Confirm new password",
				WS_VISIBLE | WS_CHILD,
				10, 50, 150, 25, hwnd, (HMENU)ID_PASSWORD_LABEL2, NULL, NULL);
			eSetPassword2 = CreateWindowEx(WS_EX_LEFT, "Edit", NULL,
				WS_VISIBLE | WS_CHILD | WS_TABSTOP | WS_BORDER,
				165, 45, 170, 25, hwnd, (HMENU)ID_PASSWORD_PASSWORD2, NULL, NULL);
			SetWindowLongPtr(eSetPassword2, GWLP_WNDPROC, (LONG_PTR)customSetPasswordProc);

			lMessage = CreateWindowEx(WS_EX_LEFT, "Static", "",
				WS_VISIBLE | WS_CHILD,
				50, 90, 250, 25, hwnd, (HMENU)ID_PASSWORD_MESSAGE, NULL, NULL);

			bSetPasswordOK = CreateWindowEx(WS_EX_LEFT, "Button", "OK",
				WS_VISIBLE | WS_CHILD | WS_TABSTOP,
				350, 10, 80, 25, hwnd, (HMENU)ID_OK, NULL, NULL);

			bCancel = CreateWindowEx(WS_EX_LEFT, "Button", "Cancel",
				WS_VISIBLE | WS_CHILD | WS_TABSTOP,
				350, 45, 80, 25, hwnd, (HMENU)ID_CANCEL, NULL, NULL);

			bGenerate = CreateWindowEx(WS_EX_LEFT, "Button", "Generate password",
				WS_VISIBLE | WS_CHILD | WS_TABSTOP,
				320, 90, 140, 25, hwnd, (HMENU)ID_GENERATE, NULL, NULL);

			centerWindow(hwnd);
			SendMessage(eSetPassword1, EM_LIMITTEXT, MAXPW-1, 0);
			SendMessage(eSetPassword2, EM_LIMITTEXT, MAXPW-1, 0);
			SetFocus(eSetPassword1);
			break;
		case WM_COMMAND:
			if (LOWORD(wParam) == ID_OK)
			{
				char pw1[MAXPW];
				char pw2[MAXPW];
				GetWindowText(eSetPassword1, pw1, MAXPW);
				GetWindowText(eSetPassword2, pw2, MAXPW);

				// check passwords match
				if (strcmp(pw1, pw2) != 0)
				{
					SetWindowText(lMessage, "Passwords do not match!");
					SetFocus(eSetPassword1);
					return DefWindowProc(hwnd, msg, wParam, lParam);
				}

				for (int i = 0; i < DBPASSWORDSIZE; ++i)
					state.DBpassword[i] = '\0';
				strcpy(state.DBpassword, pw1);
				if (state.debugging)
				{
					outs("new password set to=");
					outs(state.DBpassword);
				}

				if (!state.changingPassword)
				{
					fillListbox();
					ShowWindow(mainHwnd, SW_SHOW);
					saveEntries();
					readEntries();
					updateListbox();
				}

				closeEverything = false;
				DestroyWindow(hwnd);
			}

			if (LOWORD(wParam) == ID_CANCEL)
			{
				DestroyWindow(hwnd);
			}

			if (LOWORD(wParam) == ID_GENERATE)
			{
				char password[MAXPW];
				generatePassword(password);
				SetWindowText(eSetPassword1, password);
				SetWindowText(eSetPassword2, password);
			}
			break;
		case WM_DESTROY:
			if (closeEverything && !state.changingPassword)
			{
				DestroyWindow(hwnd);
				exit(EXIT_SUCCESS);
			}

			enableControls();
			DestroyWindow(hwnd);
			break;
	}
	return DefWindowProc(hwnd, msg, wParam, lParam);
}

void getDBpassword(void)
{
	if (state.debugging)
		outs("getDBpassword()");

	static WNDCLASSEX wcGetPassword = {0};
	static bool getPasswordClassRegistered = false;

	if (!getPasswordClassRegistered)
	{
		wcGetPassword.cbSize = sizeof(WNDCLASSEX);
		wcGetPassword.cbClsExtra = 0;
		wcGetPassword.cbWndExtra = 0;
		wcGetPassword.hbrBackground = (HBRUSH)COLOR_WINDOW;
		wcGetPassword.hCursor = LoadCursor(NULL, IDC_ARROW);
		wcGetPassword.hIcon = NULL;
		wcGetPassword.hIconSm = NULL;
		wcGetPassword.hInstance = instance;
		wcGetPassword.lpfnWndProc = getPasswordWndProc;
		wcGetPassword.lpszClassName = "TroveGetPassword";
		wcGetPassword.lpszMenuName = NULL;
		wcGetPassword.style = CS_HREDRAW | CS_VREDRAW;

		if (!RegisterClassEx(&wcGetPassword))
		{
			MessageBox(NULL, "Get password window registration failed", "Error", MB_ICONEXCLAMATION | MB_OK);
			return;
		}
		else
			getPasswordClassRegistered = true;
	}

	getPasswordHwnd = CreateWindowEx(WS_EX_LEFT,
		wcGetPassword.lpszClassName,
		"Enter DB Password",
		WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX | WS_VISIBLE,
		CW_USEDEFAULT, CW_USEDEFAULT,
		435, 150,
		NULL, NULL,
		instance, NULL);

	if (!getPasswordHwnd)
	{
		MessageBox(NULL, "Get Password window creation failed", "Error", MB_ICONEXCLAMATION | MB_OK);
		return;
	}

	ShowWindow(getPasswordHwnd, SW_SHOW);
}

LRESULT CALLBACK getPasswordWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	static HWND bGetPasswordCancel, lGetPassword, eGetPassword, lWarning;
	static bool closeEverything = true;

	switch (msg)
	{
		case WM_CREATE:
			SetTimer(hwnd, ID_TIMER1, 500, NULL);
			SetTimer(hwnd, ID_TIMER2, 2500, NULL);

			lGetPassword = CreateWindowEx(WS_EX_LEFT, "Static", "Enter password",
				WS_VISIBLE | WS_CHILD,
				10, 15, 150, 25, hwnd, (HMENU)ID_PASSWORD_LABEL1, NULL, NULL);

			eGetPassword = CreateWindowEx(WS_EX_LEFT, "Edit", NULL,
				WS_VISIBLE | WS_CHILD | WS_TABSTOP | WS_BORDER | ES_PASSWORD,
				130, 10, 170, 25, hwnd, (HMENU)ID_PASSWORD_PASSWORD1, NULL, NULL);
			originalGetPasswordProc = (WNDPROC)SetWindowLongPtr(eGetPassword, GWLP_WNDPROC, (LONG_PTR)customGetPasswordProc);

			bGetPasswordOK = CreateWindowEx(WS_EX_LEFT, "Button", "OK",
				WS_VISIBLE | WS_CHILD | WS_TABSTOP,
				320, 10, 80, 25, hwnd, (HMENU)ID_OK, NULL, NULL);

			bGetPasswordCancel = CreateWindowEx(WS_EX_LEFT, "Button", "Cancel",
				WS_VISIBLE | WS_CHILD | WS_TABSTOP,
				320, 45, 80, 25, hwnd, (HMENU)ID_CANCEL, NULL, NULL);

			lWarning = CreateWindowEx(WS_EX_LEFT, "Static", "",
				WS_VISIBLE | WS_CHILD,
				100, 60, 150, 25, hwnd, (HMENU)ID_PASSWORD_WARNING, NULL, NULL);

			SendMessage(eGetPassword, EM_LIMITTEXT, MAXPW-1, 0);
			centerWindow(hwnd);
			break;
		case WM_COMMAND:
			if (LOWORD(wParam) == ID_OK)
			{
				char pw[MAXPW];
				GetWindowText(eGetPassword, pw, MAXPW);
				strcpy(state.DBpassword, pw);

				if (state.debugging)
				{
					outs("password entered=");
					outs(state.DBpassword);
				}
				readEntries();
				if (state.readVerified)
				{
					fillListbox();
					ShowWindow(mainHwnd, SW_SHOW);
					updateListbox();
					closeEverything = false;
					DestroyWindow(hwnd);
				}
				else
				{
					for (int i = 0; i < MAXPW; ++i)
						state.DBpassword[i] = '\0';

					SetWindowText(lWarning, "Incorrect password");
					readFile(DB_FILE); // reset buffer to pre-decrypted
					SetFocus(eGetPassword);
				}
			}

			if (LOWORD(wParam) == ID_CANCEL)
			{
				DestroyWindow(hwnd);
			}
			break;
		case WM_TIMER:
			if (wParam == ID_TIMER1)
				SetFocus(eGetPassword);

			if (wParam == ID_TIMER2)
				SetWindowText(lWarning, "");
			break;
		case WM_DESTROY:
			KillTimer(hwnd, ID_TIMER1);
			KillTimer(hwnd, ID_TIMER2);

			if (closeEverything)
			{
				DestroyWindow(hwnd);
				exit(EXIT_SUCCESS);
			}

			DestroyWindow(hwnd);
			break;
	}
	return DefWindowProc(hwnd, msg, wParam, lParam);
}

LRESULT CALLBACK customFindProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg)
	{
		case WM_KEYUP:
			switch (wParam)
			{
				case VK_ESCAPE:
					SetWindowText(hwnd, "");
					EnableWindow(bAdd, TRUE);
					break;
				case VK_RETURN:
					SendMessage(bFind, BM_CLICK, 0, 0);
					break;
				case 'A':
					if (GetAsyncKeyState(VK_CONTROL))
						SendMessage(hwnd, EM_SETSEL, 0, -1);
					break;
			}
			break;
	}

	return CallWindowProc(originalFindProc, hwnd, msg, wParam, lParam);
}

LRESULT CALLBACK customListboxProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg)
	{
		case WM_KEYUP:
			switch (wParam)
			{
				case VK_RETURN:
					// get row index
					state.selectedRow = SendMessage(lbList, LB_GETCURSEL, 0, 0);

					if (state.selectedRow != LB_ERR)
						editEntry();
					break;
			}
			break;
	}

	return CallWindowProc(originalListboxProc, hwnd, msg, wParam, lParam);
}

LRESULT CALLBACK customAddProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg)
	{
		case WM_KEYUP:
			switch (wParam)
			{
				case VK_ESCAPE:
					DestroyWindow(addHwnd);
					break;
				case VK_RETURN: // catch Enter and send a Tab
					sendTab();
					break;
				case 'A': // CTRL A
					if (GetAsyncKeyState(VK_CONTROL))
						SendMessage(hwnd, EM_SETSEL, 0, -1);
					break;
			}
			break;
	}

	return CallWindowProc(originalAddProc, hwnd, msg, wParam, lParam);
}

LRESULT CALLBACK customEditProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg)
	{
		case WM_KEYUP:
			switch (wParam)
			{
				case VK_ESCAPE:
					DestroyWindow(editHwnd);
					break;
				case VK_RETURN: // catch Enter and send a Tab
					sendTab();
					break;
				case 'A': // CTRL A
					if (GetAsyncKeyState(VK_CONTROL))
						SendMessage(hwnd, EM_SETSEL, 0, -1);
					break;
			}
			break;
	}

	return CallWindowProc(originalEditProc, hwnd, msg, wParam, lParam);
}

LRESULT CALLBACK customAddMiscProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg)
	{
		case WM_GETDLGCODE: // removed this to allow message handling to accept Tab for multiline field
		{
			LRESULT res = CallWindowProc(originalAddMiscProc, hwnd, msg, wParam, lParam);
			res &= ~DLGC_WANTALLKEYS;
			return res;
		}
		case WM_KEYUP:
			switch (wParam)
			{
				case VK_ESCAPE:
					DestroyWindow(addHwnd);
					break;
				case VK_RETURN: // catch Enter and send a Tab
					if (!GetAsyncKeyState(VK_CONTROL))
						sendTab();
					break;
				case 'A': // CTRL A
					if (GetAsyncKeyState(VK_CONTROL))
						SendMessage(hwnd, EM_SETSEL, 0, -1);
					break;
			}
			break;
	}

	return CallWindowProc(originalAddProc, hwnd, msg, wParam, lParam);
}

LRESULT CALLBACK customEditMiscProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg)
	{
		case WM_GETDLGCODE: // removed this to allow message handling to accept Tab for multiline field
		{
			LRESULT res = CallWindowProc(originalEditMiscProc, hwnd, msg, wParam, lParam);
			res &= ~DLGC_WANTALLKEYS;
			return res;
		}
		case WM_KEYUP:
			switch (wParam)
			{
				case VK_ESCAPE:
					DestroyWindow(editHwnd);
					break;
				case VK_RETURN: // catch Enter and send a Tab
					if (!GetAsyncKeyState(VK_CONTROL))
						sendTab();
					break;
				case 'A': // CTRL A
					if (GetAsyncKeyState(VK_CONTROL))
						SendMessage(hwnd, EM_SETSEL, 0, -1);
					break;
			}
			break;
	}

	return CallWindowProc(originalEditProc, hwnd, msg, wParam, lParam);
}

LRESULT CALLBACK customSetPasswordProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg)
	{
		case WM_KEYUP:
			switch (wParam)
			{
				case VK_ESCAPE:
					DestroyWindow(setPasswordHwnd);
					break;
				case VK_RETURN:
					SendMessage(bSetPasswordOK, BM_CLICK, 0, 0);
					break;
				case 'A': // CTRL A
					if (GetAsyncKeyState(VK_CONTROL))
						SendMessage(hwnd, EM_SETSEL, 0, -1);
					break;
			}
			break;
	}

	return CallWindowProc(originalSetPasswordProc, hwnd, msg, wParam, lParam);
}

LRESULT CALLBACK customGetPasswordProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg)
	{
		case WM_KEYUP:
			switch (wParam)
			{
				case VK_ESCAPE:
					DestroyWindow(getPasswordHwnd);
					break;
				case VK_RETURN:
					SendMessage(bGetPasswordOK, BM_CLICK, 0, 0);
					break;
				case 'A': // CTRL A
					if (GetAsyncKeyState(VK_CONTROL))
						SendMessage(hwnd, EM_SETSEL, 0, -1);
					break;
			}
			break;
	}

	return CallWindowProc(originalGetPasswordProc, hwnd, msg, wParam, lParam);
}
