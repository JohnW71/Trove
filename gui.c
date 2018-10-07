#define _CRT_SECURE_NO_WARNINGS

#include "gui.h"

//TODO Quit sometimes takes multiple attempts
//TODO CtrlA does not work in editboxes
//TODO implement Find
//TODO implement encryption
//TODO implement Settings

static char tempFile[] = "temp.db";
static bool running = true;
static int entryCount = 0;
static int showCmd;
static int generationSize = 20;	//TODO load from settings
static int minSpecial = 4;		//TODO load from settings
static int minNumeric = 4;		//TODO load from settings
static int minUppercase = 4;	//TODO load from settings
static LRESULT selectedRow;
static HINSTANCE instance;
static HWND lbList, bEdit, bDelete;

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,
					PWSTR pCmdLine, int nShowCmd)
{
	instance = hInstance;
	showCmd = nShowCmd;
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

	ShowWindow(hwnd, nShowCmd);

	while (running)
	{
		while (GetMessage(&msg, NULL, 0, 0))
		{
			if (!IsDialogMessage(hwnd, &msg))
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
	static HWND bAdd, bSettings, tFind, bFind, bCancel, tTitle;

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
			tFind = CreateWindowEx(WS_EX_LEFT, "Edit", NULL,
						WS_VISIBLE | WS_CHILD | WS_TABSTOP | WS_BORDER,
						10, 45, 190, 25, hwnd, (HMENU)ID_MAIN_TEXTBOX, NULL, NULL);
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
				addEntry();
			}

			if (LOWORD(wParam) == ID_MAIN_EDIT)
			{
				editEntry();
			}

			if (LOWORD(wParam) == ID_MAIN_DELETE)
			{
				deleteEntry();
			}

			if (LOWORD(wParam) == ID_MAIN_SETTINGS)
			{
				// settings();
			}

			if (LOWORD(wParam) == ID_MAIN_FIND)
			{
				// find();
				// iIndex = SendMessage(hwndList, LB_SELECTSTRING, iIndex, (LPARAM) szSearchString);
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

void addEntry(void)
{
	MSG msg;
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

	HWND hwnd = CreateWindowEx(WS_EX_LEFT,
								wcAdd.lpszClassName,
								"Add",
								WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX | WS_VISIBLE,
								CW_USEDEFAULT, CW_USEDEFAULT,
								530, 200,
								NULL, NULL,
								instance, NULL);

	if (!hwnd)
	{
		MessageBox(NULL, "Add window creation failed", "Error", MB_ICONEXCLAMATION | MB_OK);
		return;
	}

	while (GetMessage(&msg, NULL, 0, 0))
	{
		if (!IsDialogMessage(hwnd, &msg))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}

	ShowWindow(hwnd, showCmd);
}

LRESULT CALLBACK addWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	static HWND bOK, bCancel, bGenerate, lTitle, tTitle, lId, tId, lPw, tPw, lMisc, tMisc;

	switch (msg)
	{
		case WM_CREATE:
			centerWindow(hwnd);

			lTitle = CreateWindowEx(WS_EX_LEFT, "Static", "Title",
							WS_VISIBLE | WS_CHILD,
							10, 10, 80, 25, hwnd, (HMENU)ID_EDIT_TITLE_LABEL, NULL, NULL);
			tTitle = CreateWindowEx(WS_EX_LEFT, "Edit", "",
							WS_VISIBLE | WS_CHILD | WS_BORDER | WS_TABSTOP,
							90, 10, 170, 25, hwnd, (HMENU)ID_EDIT_TITLE, NULL, NULL);

			lId = CreateWindowEx(WS_EX_LEFT, "Static", "ID",
							WS_VISIBLE | WS_CHILD,
							10, 45, 80, 25, hwnd, (HMENU)ID_EDIT_ID_LABEL, NULL, NULL);
			tId = CreateWindowEx(WS_EX_LEFT, "Edit", "",
							WS_VISIBLE | WS_CHILD | WS_BORDER | WS_TABSTOP,
							90, 45, 170, 25, hwnd, (HMENU)ID_EDIT_ID, NULL, NULL);

			lPw = CreateWindowEx(WS_EX_LEFT, "Static", "Password",
							WS_VISIBLE | WS_CHILD,
							10, 80, 80, 25, hwnd, (HMENU)ID_EDIT_PW_LABEL, NULL, NULL);
			tPw = CreateWindowEx(WS_EX_LEFT, "Edit", "",
							WS_VISIBLE | WS_CHILD | WS_BORDER | WS_TABSTOP,
							90, 80, 170, 25, hwnd, (HMENU)ID_EDIT_PW, NULL, NULL);

			lMisc = CreateWindowEx(WS_EX_LEFT, "Static", "Misc",
							WS_VISIBLE | WS_CHILD,
							10, 115, 80, 25, hwnd, (HMENU)ID_EDIT_MISC_LABEL, NULL, NULL);
			tMisc = CreateWindowEx(WS_EX_LEFT, "Edit", "",
							WS_VISIBLE | WS_CHILD | WS_BORDER | WS_TABSTOP,
							90, 115, 410, 25, hwnd, (HMENU)ID_EDIT_MISC, NULL, NULL);

			bOK = CreateWindowEx(WS_EX_LEFT, "Button", "OK",
							WS_VISIBLE | WS_CHILD | WS_TABSTOP,
							280, 10, 80, 25, hwnd, (HMENU)ID_EDIT_OK, NULL, NULL);
			bCancel = CreateWindowEx(WS_EX_LEFT, "Button", "Cancel",
							WS_VISIBLE | WS_CHILD | WS_TABSTOP,
							280, 45, 80, 25, hwnd, (HMENU)ID_EDIT_CANCEL, NULL, NULL);
			bGenerate = CreateWindowEx(WS_EX_LEFT, "Button", "Generate password",
							WS_VISIBLE | WS_CHILD | WS_TABSTOP,
							280, 80, 160, 25, hwnd, (HMENU)ID_EDIT_GENERATE, NULL, NULL);

			SetFocus(tTitle);
			break;
		case WM_COMMAND:
			if (LOWORD(wParam) == ID_EDIT_OK)
			{
				// get fields from edit boxes
				wchar_t title[MAXTITLE];
				wchar_t id[MAXID];
				wchar_t pw[MAXPW];
				wchar_t misc[MAXMISC];
				GetWindowTextW(tTitle, title, MAXTITLE);
				GetWindowTextW(tId, id, MAXID);
				GetWindowTextW(tPw, pw, MAXPW);
				GetWindowTextW(tMisc, misc, MAXMISC);

				// test that title field is not empty
				if (wcslen(title) == 0)
				{
					SetFocus(tTitle);
					return DefWindowProc(hwnd, msg, wParam, lParam);
				}

				// test for pre-existing title
				for (int i = 0; i < entryCount; ++i)
				{
					if (wcscmp(entries[i].title, title) == 0 && i != selectedRow)
					{
						MessageBox(NULL, "That title is already in use", "Error", MB_ICONEXCLAMATION | MB_OK);
						SetFocus(tTitle);
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
			if (LOWORD(wParam) == ID_EDIT_CANCEL)
			{
				DestroyWindow(hwnd);
			}
			if (LOWORD(wParam) == ID_EDIT_GENERATE)
			{
				wchar_t password[MAXPW];
				generatePassword(password);
				SetWindowTextW(tPw, password);
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
	MSG msg;
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

	HWND hwnd = CreateWindowEx(WS_EX_LEFT,
								wcEdit.lpszClassName,
								"Edit",
								WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX | WS_VISIBLE,
								CW_USEDEFAULT, CW_USEDEFAULT,
								530, 200,
								NULL, NULL,
								instance, NULL);

	if (!hwnd)
	{
		MessageBox(NULL, "Edit window creation failed", "Error", MB_ICONEXCLAMATION | MB_OK);
		return;
	}

	while (GetMessage(&msg, NULL, 0, 0))
	{
		if (!IsDialogMessage(hwnd, &msg))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}

	ShowWindow(hwnd, showCmd);
}

LRESULT CALLBACK editWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	static HWND bOK, bCancel, bGenerate, lTitle, tTitle, lId, tId, lPw, tPw, lMisc, tMisc;

	switch (msg)
	{
		case WM_CREATE:
			centerWindow(hwnd);

			lTitle = CreateWindowEx(WS_EX_LEFT, "Static", "Title",
							WS_VISIBLE | WS_CHILD,
							10, 10, 80, 25, hwnd, (HMENU)ID_EDIT_TITLE_LABEL, NULL, NULL);
			tTitle = CreateWindowEx(WS_EX_LEFT, "Edit", "",
							WS_VISIBLE | WS_CHILD | WS_BORDER | WS_TABSTOP,
							90, 10, 170, 25, hwnd, (HMENU)ID_EDIT_TITLE, NULL, NULL);

			lId = CreateWindowEx(WS_EX_LEFT, "Static", "ID",
							WS_VISIBLE | WS_CHILD,
							10, 45, 80, 25, hwnd, (HMENU)ID_EDIT_ID_LABEL, NULL, NULL);
			tId = CreateWindowEx(WS_EX_LEFT, "Edit", "",
							WS_VISIBLE | WS_CHILD | WS_BORDER | WS_TABSTOP,
							90, 45, 170, 25, hwnd, (HMENU)ID_EDIT_ID, NULL, NULL);

			lPw = CreateWindowEx(WS_EX_LEFT, "Static", "Password",
							WS_VISIBLE | WS_CHILD,
							10, 80, 80, 25, hwnd, (HMENU)ID_EDIT_PW_LABEL, NULL, NULL);
			tPw = CreateWindowEx(WS_EX_LEFT, "Edit", "",
							WS_VISIBLE | WS_CHILD | WS_BORDER | WS_TABSTOP,
							90, 80, 170, 25, hwnd, (HMENU)ID_EDIT_PW, NULL, NULL);

			lMisc = CreateWindowEx(WS_EX_LEFT, "Static", "Misc",
							WS_VISIBLE | WS_CHILD,
							10, 115, 80, 25, hwnd, (HMENU)ID_EDIT_MISC_LABEL, NULL, NULL);
			tMisc = CreateWindowEx(WS_EX_LEFT, "Edit", "",
							WS_VISIBLE | WS_CHILD | WS_BORDER | WS_TABSTOP,
							90, 115, 410, 25, hwnd, (HMENU)ID_EDIT_MISC, NULL, NULL);

			bOK = CreateWindowEx(WS_EX_LEFT, "Button", "OK",
							WS_VISIBLE | WS_CHILD | WS_TABSTOP,
							280, 10, 80, 25, hwnd, (HMENU)ID_EDIT_OK, NULL, NULL);
			bCancel = CreateWindowEx(WS_EX_LEFT, "Button", "Cancel",
							WS_VISIBLE | WS_CHILD | WS_TABSTOP,
							280, 45, 80, 25, hwnd, (HMENU)ID_EDIT_CANCEL, NULL, NULL);
			bGenerate = CreateWindowEx(WS_EX_LEFT, "Button", "Generate password",
							WS_VISIBLE | WS_CHILD | WS_TABSTOP,
							280, 80, 160, 25, hwnd, (HMENU)ID_EDIT_GENERATE, NULL, NULL);

			// get selected row
			wchar_t title[MAXLINE];
			SendMessageW(lbList, LB_GETTEXT, selectedRow, (LPARAM)title);

			// populate fields
			for (int i = 0; i < entryCount; ++i)
			{
				if (wcscmp(entries[i].title, title) == 0)
				{
					SetWindowTextW(tTitle, entries[i].title);
					SetWindowTextW(tId, entries[i].id);
					SetWindowTextW(tPw, entries[i].pw);
					SetWindowTextW(tMisc, entries[i].misc);
					break;
				}
			}

			SetFocus(bCancel);
			break;
		case WM_COMMAND:
			if (LOWORD(wParam) == ID_EDIT_OK)
			{
				// get fields from edit boxes
				wchar_t title[MAXTITLE];
				wchar_t id[MAXID];
				wchar_t pw[MAXPW];
				wchar_t misc[MAXMISC];
				GetWindowTextW(tTitle, title, MAXTITLE);
				GetWindowTextW(tId, id, MAXID);
				GetWindowTextW(tPw, pw, MAXPW);
				GetWindowTextW(tMisc, misc, MAXMISC);

				// test that title field is not empty
				if (wcslen(title) == 0)
				{
					SetFocus(tTitle);
					return DefWindowProc(hwnd, msg, wParam, lParam);
				}

				// test for pre-existing title
				for (int i = 0; i < entryCount; ++i)
				{
					if (wcscmp(entries[i].title, title) == 0 && i != selectedRow)
					{
						MessageBox(NULL, "That title is already in use", "Error", MB_ICONEXCLAMATION | MB_OK);
						SetFocus(tTitle);
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
			if (LOWORD(wParam) == ID_EDIT_CANCEL)
			{
				DestroyWindow(hwnd);
			}
			if (LOWORD(wParam) == ID_EDIT_GENERATE)
			{
				wchar_t password[MAXPW];
				generatePassword(password);
				SetWindowTextW(tPw, password);
			}
			break;
		case WM_DESTROY:
			DestroyWindow(hwnd);
			break;
	}
	return DefWindowProc(hwnd, msg, wParam, lParam);
}

void deleteEntry(void)
{
	if (selectedRow != LB_ERR)
	{
		if (MessageBox(NULL, "Are you sure you want to delete this entry?", "Confirm delete", MB_YESNO | MB_ICONWARNING) == IDYES)
		{
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

		for (int i = 0; i < generationSize; ++i)
		{
			int rn = rand() % 127;
			if (rn < 33 || rn == 44)
			{
				--i;
				continue;
			}
			buf[i] = (char)rn;

			if (ispunct((char)rn))
				++specialCount;
			if (isdigit((char)rn))
				++numericCount;
			if (isupper((char)rn))
				++uppercaseCount;
		}
		buf[generationSize] = '\0';
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
