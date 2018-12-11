#define _CRT_SECURE_NO_WARNINGS

#include "gui.h"
#include "shared.h"

extern HWND lbList;
extern HWND bAdd;
extern HWND bEdit;
extern HWND bDelete;
extern HWND bSettings;
extern HWND bQuit;
extern HWND eFind;

void disableControls(void)
{
	EnableWindow(bAdd, FALSE);
	EnableWindow(bEdit, FALSE);
	EnableWindow(bDelete, FALSE);
	EnableWindow(bSettings, FALSE);
	EnableWindow(bQuit, FALSE);
	EnableWindow(lbList, FALSE);
	EnableWindow(eFind, FALSE);
}

void enableControls(void)
{
	EnableWindow(bAdd, TRUE);
	EnableWindow(bSettings, TRUE);
	EnableWindow(bQuit, TRUE);
	EnableWindow(lbList, TRUE);
	EnableWindow(eFind, TRUE);
}

void centerWindow(HWND hwnd)
{
	if (state.debugging)
		outs("centerWindow()");

	RECT rc = {0};

	GetWindowRect(hwnd, &rc);
	int windowWidth = rc.right - rc.left;
	int windowHeight = rc.bottom - rc.top;
	int screenWidth = GetSystemMetrics(SM_CXSCREEN);
	int screenHeight = GetSystemMetrics(SM_CYSCREEN);

	SetWindowPos(hwnd, HWND_TOP, (screenWidth - windowWidth) / 2,
		(screenHeight - windowHeight) / 2, 0, 0, SWP_NOSIZE);
}

void restoreWindow(HWND hwnd, int x, int y)
{
	SetWindowPos(hwnd, HWND_TOP, x, y, 0, 0, SWP_NOSIZE);
}

void fillListbox(void)
{
	for (int i = 0; i < state.entryCount; ++i)
	{
		char row[MAXLINE];
		strcpy(row, entries[i].title);
		SendMessage(lbList, LB_ADDSTRING, i, (LPARAM)row);
	}
}

void updateListbox(void)
{
	if (state.debugging)
		outs("updateListbox()");

	// deselect all entries
	SendMessage(lbList, LB_SETCURSEL, (WPARAM)-1, 0);
	EnableWindow(bEdit, FALSE);
	EnableWindow(bDelete, FALSE);
	sortEntries();
	state.selectedRow = LB_ERR;
}

void deleteEntry(void)
{
	if (state.debugging)
		outs("deleteEntry()");

	if (state.selectedRow != LB_ERR)
	{
		if (MessageBox(NULL, "Are you sure you want to delete this entry?", "Confirm delete", MB_YESNO | MB_ICONWARNING) == IDYES)
		{
			// delete row
			SendMessage(lbList, LB_DELETESTRING, state.selectedRow, 0);

			// recreate array without deleted row
			struct Entry *newEntries = (struct Entry *)malloc(sizeof(struct Entry) * ((uint64_t)state.entryCount - 1));
			if (!newEntries)
			{
				outs("Failed to allocate memory for new array during deletion");
				return;
			}

			for (int i = 0, j = 0; i < state.entryCount; ++i, ++j)
				if (i != (state.selectedRow))
				{
					strcpy(newEntries[j].title, entries[i].title);
					strcpy(newEntries[j].id, entries[i].id);
					strcpy(newEntries[j].pw, entries[i].pw);
					strcpy(newEntries[j].misc, entries[i].misc);
				}
				else
					--j;

			free(entries);
			entries = newEntries;
			newEntries = NULL;
			--state.entryCount;
		}
		updateListbox();
	}
}

void outs(char *s)
{
	FILE *lf = fopen(LOG_FILE, "a");
	if (lf == NULL)
	{
		MessageBox(NULL, "Can't open log file", "Error", MB_ICONEXCLAMATION | MB_OK);
		return;
	}

	fprintf(lf, "%s\n", s);
	fclose(lf);
}

//TODO handle 2nd char in password?
bool isNumeric(char *buf)
{
	if (state.debugging)
		outs("isNumeric()");

	if ((int)buf[0] < 48 || (int)buf[0] > 57)
		return false;
	else
		return true;
}

void fillDropdown(HWND hwnd, int min, int max)
{
	if (state.debugging)
		outs("fillDropdown()");

	for (int i = min; i <= max; ++i)
	{
		char c[3];
		sprintf(c, "%d", i);
		SendMessage(hwnd, CB_ADDSTRING, 0, (LPARAM)c);
	}
}

void writeSettings(char *iniFile)
{
	if (state.debugging)
		outs("writeSettings()");

	if (strlen(settings.iv) < IV_SIZE - 1)
	{
		outs("Generating new keygen");
		generateKeygen(settings.iv);
		outs(settings.iv);
	}

	FILE *f = fopen(iniFile, "w");
	if (f == NULL)
	{
		outs("Error saving entries!");
		return;
	}

	fputs("#Manually editing the keygen value will make it\n", f);
	fputs("#impossible to decrypt your database again!\n", f);
	fprintf(f, "password_size=%d\n", settings.passwordSize);
	fprintf(f, "min_special=%d\n", settings.minSpecial);
	fprintf(f, "min_numeric=%d\n", settings.minNumeric);
	fprintf(f, "min_uppercase=%d\n", settings.minUppercase);
	fprintf(f, "keygen=%s\n", settings.iv);
	fprintf(f, "window_row=%d\n", settings.screenRow);
	fprintf(f, "window_col=%d\n", settings.screenCol);
	fclose(f);
}

void shutDown(HWND hwnd)
{
	RECT rc = {0};
	GetWindowRect(hwnd, &rc);
	settings.screenRow = rc.top;
	settings.screenCol = rc.left;
	writeSettings(INI_FILE);
	saveEntries();
	PostQuitMessage(0);
	state.running = false;
}

void sendTab(void)
{
	KEYBDINPUT kb = {0};
	INPUT input = {0};

	// generate down
	kb.wVk = VK_TAB;
	input.type = INPUT_KEYBOARD;
	input.ki = kb;
	SendInput(1, &input, sizeof(input));

	// generate up
	ZeroMemory(&kb, sizeof(KEYBDINPUT));
	ZeroMemory(&input, sizeof(INPUT));
	kb.dwFlags = KEYEVENTF_KEYUP;
	kb.wVk = VK_TAB;
	input.type = INPUT_KEYBOARD;
	input.ki = kb;
	SendInput(1, &input, sizeof(input));
}
