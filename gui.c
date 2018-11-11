#define _CRT_SECURE_NO_WARNINGS

#include "gui.h"
#include "shared.h"

//TODO add retry for failed password
//TODO tidy up db/pw loading functions
//TODO catch Enter keypress on edit & password windows
//FIX find selects a row even when no match
//FIX after deleting an entry, can't do Edit OK on another entry
//TODO test everything fully
//TODO add change password option to settings
//TODO make password generation faster
//TODO remove debugging stuff
//TODO use same database & ini file?
//FIX CtrlA does not work in editboxes

bool debugging = true;
bool running = true;
int entryCount = 0;
int passwordSize = MINPW;
int minSpecial = 0;
int minNumeric = 0;
int minUppercase = 0;
int screenCol = 0;
int screenRow = 0;
char iv[IV_SIZE];
char iniFile[] = "gui.ini"; // "trove.ini";
char logFile[] = "gui_log.txt";
char DBpassword[DBPASSWORDSIZE];
LRESULT selectedRow;
HWND lbList;
HWND bMainAdd;
HWND bMainEdit;
HWND bMainDelete;
HWND bMainSettings;
HWND bMainQuit;

static HINSTANCE instance;
static HWND mainHwnd;
static HWND addHwnd;
static HWND editHwnd;
static HWND settingsHwnd;
static HWND setPasswordHwnd;
static HWND getPasswordHwnd;

extern bool noDatabase;

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,
					PWSTR pCmdLine, int nShowCmd)
{
	srand((unsigned int)time(NULL));
	instance = hInstance;
	MSG msg;
	WNDCLASSEX wc = {0};

	// reset log file
	FILE *lf = fopen(logFile, "w");
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

	readSettings();

	mainHwnd = CreateWindowEx(WS_EX_LEFT,
						wc.lpszClassName,
						"Trove",
						WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX,
						CW_USEDEFAULT, CW_USEDEFAULT, 390, 600,
						NULL, NULL,	hInstance, NULL);

	if (!mainHwnd)
	{
		MessageBox(NULL, "Window creation failed", "Error", MB_ICONEXCLAMATION | MB_OK);
		return 0;
	}

	while (running)
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
	static HWND eFind, bFind;

	switch (msg)
	{
		case WM_CREATE:
			if (screenRow == 0 && screenRow == 0)
				centerWindow(hwnd);
			else
				restoreWindow(hwnd, screenCol, screenRow);

			// first row
			bMainAdd = CreateWindowEx(WS_EX_LEFT, "Button", "Add",
						WS_VISIBLE | WS_CHILD | WS_TABSTOP,
						10, 10, 80, 25, hwnd, (HMENU)ID_MAIN_ADD, NULL, NULL);
			bMainEdit = CreateWindowEx(WS_EX_LEFT, "Button", "Edit",
						WS_VISIBLE | WS_CHILD | WS_TABSTOP | WS_DISABLED,
						100, 10, 80, 25, hwnd, (HMENU)ID_MAIN_EDIT, NULL, NULL);
			bMainDelete = CreateWindowEx(WS_EX_LEFT, "Button", "Delete",
						WS_VISIBLE | WS_CHILD | WS_TABSTOP | WS_DISABLED,
						190, 10, 80, 25, hwnd, (HMENU)ID_MAIN_DELETE, NULL, NULL);
			bMainSettings = CreateWindowEx(WS_EX_LEFT, "Button", "Settings",
						WS_VISIBLE | WS_CHILD | WS_TABSTOP,
						280, 10, 80, 25, hwnd, (HMENU)ID_MAIN_SETTINGS, NULL, NULL);

			// second row
			eFind = CreateWindowEx(WS_EX_LEFT, "Edit", NULL,
						WS_VISIBLE | WS_CHILD | WS_TABSTOP | WS_BORDER,
						10, 45, 190, 25, hwnd, (HMENU)ID_MAIN_FINDTEXT, NULL, NULL);
			bFind = CreateWindowEx(WS_EX_LEFT, "Button", "Find",
						WS_VISIBLE | WS_CHILD | WS_TABSTOP | WS_DISABLED,
						205, 45, 40, 25, hwnd, (HMENU)ID_MAIN_FIND, NULL, NULL);
			bMainQuit = CreateWindowEx(WS_EX_LEFT, "Button", "Quit",
						WS_VISIBLE | WS_CHILD | WS_TABSTOP,
						280, 45, 80, 25, hwnd, (HMENU)ID_MAIN_QUIT, NULL, NULL);

			// listbox
			lbList = CreateWindowEx(WS_EX_LEFT, "ListBox", NULL,
						WS_VISIBLE | WS_CHILD | LBS_STANDARD,
						10, 80, 350, 475, hwnd, (HMENU)ID_MAIN_LISTBOX, NULL, NULL);

			readFile();

			if (noDatabase)
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
					SendMessage(lbList, LB_SETCURSEL, -1, 0);
				}

				// find text was changed
				if (HIWORD(wParam) == EN_CHANGE)
				{
					// get text length
					char findText[MAXTITLE];
					GetWindowText(eFind, findText, MAXTITLE);

					// enable Find button if text is present
					if (strlen(findText) > 0)
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
					char find[MAXTITLE];
					GetWindowText(eFind, find, MAXTITLE);

					// deselect all entries
					SendMessage(lbList, LB_SETCURSEL, -1, 0);

					static int i = 0;
					int attempts = 0;

					while (attempts <= entryCount)
					{
						if (i != selectedRow && strstr(entries[i].title, find))
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
				shutDown(mainHwnd);
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
						EnableWindow(bMainEdit, TRUE);
						EnableWindow(bMainDelete, TRUE);
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
			shutDown(mainHwnd);
			break;
	}
	return DefWindowProc(hwnd, msg, wParam, lParam);
}

void addEntry(void)
{
	if (debugging)
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
				{
					SetFocus(eTitle);
					return DefWindowProc(hwnd, msg, wParam, lParam);
				}

				// test for pre-existing title
				for (int i = 0; i < entryCount; ++i)
				{
					if (strcmp(entries[i].title, title) == 0 && i != selectedRow)
					{
						MessageBox(NULL, "That title is already in use", "Error", MB_ICONEXCLAMATION | MB_OK);
						SetFocus(eTitle);
						return DefWindowProc(hwnd, msg, wParam, lParam);
					}
				}

				// add to listbox & entries
				SendMessage(lbList, LB_ADDSTRING, 0, (LPARAM)title);

				struct Entry *temp = realloc(entries, (entryCount+1) * sizeof(*entries));
				if (temp == NULL)
				{
					outs("Failure reallocating memory for new entry");
					MessageBox(NULL, "Failed reallocating memory for new entry", "Error", MB_ICONEXCLAMATION | MB_OK);
					return DefWindowProc(hwnd, msg, wParam, lParam);
				}
				entries = temp;

				strcpy(entries[entryCount].title, title);
				strcpy(entries[entryCount].id, id);
				strcpy(entries[entryCount].pw, pw);
				strcpy(entries[entryCount].misc, misc);
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
				char password[MAXPW];
				generatePassword(password);
				SetWindowText(ePw, password);
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
	if (debugging)
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
			char title[MAXLINE];
			SendMessage(lbList, LB_GETTEXT, selectedRow, (LPARAM)title);

			// populate fields
			for (int i = 0; i < entryCount; ++i)
			{
				if (strcmp(entries[i].title, title) == 0)
				{
					SetWindowText(eTitle, entries[i].title);
					SetWindowText(eId, entries[i].id);
					SetWindowText(ePw, entries[i].pw);
					SetWindowText(eMisc, entries[i].misc);
					break;
				}
			}

			SetFocus(bCancel);
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
				{
					SetFocus(eTitle);
					return DefWindowProc(hwnd, msg, wParam, lParam);
				}

				// test for pre-existing title
				for (int i = 0; i < entryCount; ++i)
				{
					if (strcmp(entries[i].title, title) == 0 && i != selectedRow)
					{
						MessageBox(NULL, "That title is already in use", "Error", MB_ICONEXCLAMATION | MB_OK);
						SetFocus(eTitle);
						return DefWindowProc(hwnd, msg, wParam, lParam);
					}
				}

				// replace entry in listbox & entries
				SendMessage(lbList, LB_DELETESTRING, selectedRow, 0);
				SendMessage(lbList, LB_ADDSTRING, 0, (LPARAM)title);
				strcpy(entries[selectedRow].title, title);
				strcpy(entries[selectedRow].id, id);
				strcpy(entries[selectedRow].pw, pw);
				strcpy(entries[selectedRow].misc, misc);

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
			DestroyWindow(hwnd);
			break;
	}
	return DefWindowProc(hwnd, msg, wParam, lParam);
}

void editSettings(void)
{
	if (debugging)
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

void setNewDBpassword(void)
{
	if (debugging)
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

	ShowWindow(setPasswordHwnd, SW_SHOW);
}

LRESULT CALLBACK setPasswordWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	static HWND bOK, bQuit, bGenerate, lMessage, lPassword1, lPassword2, ePassword1, ePassword2;
	static bool closeEverything = true;

	switch (msg)
	{
		case WM_CREATE:
			lPassword1 = CreateWindowEx(WS_EX_LEFT, "Static", "Enter new password",
							WS_VISIBLE | WS_CHILD,
							10, 15, 150, 25, hwnd, (HMENU)ID_PASSWORD_LABEL1, NULL, NULL);
			ePassword1 = CreateWindowEx(WS_EX_LEFT, "Edit", NULL,
							WS_VISIBLE | WS_CHILD | WS_TABSTOP | WS_BORDER,
							165, 10, 170, 25, hwnd, (HMENU)ID_PASSWORD_PASSWORD1, NULL, NULL);

			lPassword2 = CreateWindowEx(WS_EX_LEFT, "Static", "Confirm new password",
							WS_VISIBLE | WS_CHILD,
							10, 50, 150, 25, hwnd, (HMENU)ID_PASSWORD_LABEL2, NULL, NULL);
			ePassword2 = CreateWindowEx(WS_EX_LEFT, "Edit", NULL,
							WS_VISIBLE | WS_CHILD | WS_TABSTOP | WS_BORDER,
							165, 45, 170, 25, hwnd, (HMENU)ID_PASSWORD_PASSWORD2, NULL, NULL);

			lMessage = CreateWindowEx(WS_EX_LEFT, "Static", "",
							WS_VISIBLE | WS_CHILD,
							50, 90, 250, 25, hwnd, (HMENU)ID_PASSWORD_MESSAGE, NULL, NULL);

			bOK = CreateWindowEx(WS_EX_LEFT, "Button", "OK",
							WS_VISIBLE | WS_CHILD | WS_TABSTOP,
							350, 10, 80, 25, hwnd, (HMENU)ID_OK, NULL, NULL);

			bQuit = CreateWindowEx(WS_EX_LEFT, "Button", "Cancel",
							WS_VISIBLE | WS_CHILD | WS_TABSTOP,
							350, 45, 80, 25, hwnd, (HMENU)ID_CANCEL, NULL, NULL);

			bGenerate = CreateWindowEx(WS_EX_LEFT, "Button", "Generate password",
							WS_VISIBLE | WS_CHILD | WS_TABSTOP,
							320, 90, 140, 25, hwnd, (HMENU)ID_GENERATE, NULL, NULL);

			centerWindow(hwnd);
			SetFocus(ePassword1);
			break;
		case WM_COMMAND:
			if (LOWORD(wParam) == ID_OK)
			{
				char pw1[MAXPW];
				char pw2[MAXPW];
				GetWindowText(ePassword1, pw1, MAXPW);
				GetWindowText(ePassword2, pw2, MAXPW);

				// check passwords match
				if (strcmp(pw1, pw2) != 0)
				{
					SetWindowText(lMessage,"Passwords do not match!");
					SetFocus(ePassword1);
					return DefWindowProc(hwnd, msg, wParam, lParam);
				}

				strcpy(DBpassword, pw1);
if (debugging)
{
	outs("new password set to=");
	outs(DBpassword);
}
				fillListbox();
				ShowWindow(mainHwnd, SW_SHOW);
				saveEntries();
				readEntries();
				updateListbox();
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
				SetWindowText(ePassword1, password);
				SetWindowText(ePassword2, password);
			}
			break;
		case WM_DESTROY:
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

void getDBpassword(void)
{
	if (debugging)
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
								435, 125,
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
	static HWND bOK, bQuit, lPassword, ePassword;
	static bool closeEverything = true;

	switch (msg)
	{
		case WM_CREATE:
			lPassword = CreateWindowEx(WS_EX_LEFT, "Static", "Enter password",
							WS_VISIBLE | WS_CHILD,
							10, 15, 150, 25, hwnd, (HMENU)ID_PASSWORD_LABEL1, NULL, NULL);

			ePassword = CreateWindowEx(WS_EX_LEFT, "Edit", NULL,
							WS_VISIBLE | WS_CHILD | WS_TABSTOP | WS_BORDER | ES_PASSWORD,
							130, 10, 170, 25, hwnd, (HMENU)ID_PASSWORD_PASSWORD1, NULL, NULL);

			bOK = CreateWindowEx(WS_EX_LEFT, "Button", "OK",
							WS_VISIBLE | WS_CHILD | WS_TABSTOP,
							320, 10, 80, 25, hwnd, (HMENU)ID_OK, NULL, NULL);

			bQuit = CreateWindowEx(WS_EX_LEFT, "Button", "Cancel",
							WS_VISIBLE | WS_CHILD | WS_TABSTOP,
							320, 45, 80, 25, hwnd, (HMENU)ID_CANCEL, NULL, NULL);

			centerWindow(hwnd);
			SetFocus(ePassword);
			break;
		case WM_COMMAND:
			if (LOWORD(wParam) == ID_OK)
			{
				char pw[MAXPW];
				GetWindowText(ePassword, pw, MAXPW);
				strcpy(DBpassword, pw);

if (debugging)
{
	outs("password entered=");
	outs(DBpassword);
}
				readEntries();
				fillListbox();
				ShowWindow(mainHwnd, SW_SHOW);
				updateListbox();
				closeEverything = false;
				DestroyWindow(hwnd);
			}

			if (LOWORD(wParam) == ID_CANCEL)
			{
				DestroyWindow(hwnd);
			}
			break;
		case WM_DESTROY:
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
