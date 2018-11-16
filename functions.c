#define _CRT_SECURE_NO_WARNINGS

#include "gui.h"
#include "shared.h"

extern int entryCount;
extern int passwordSize;
extern int minSpecial;
extern int minNumeric;
extern int minUppercase;
extern int screenRow;
extern int screenCol;
extern bool debugging;
extern bool running;
extern char logFile[];
extern char iniFile[];
extern char iv[];
extern char DBpassword[];
extern HWND lbList;
extern HWND bMainEdit;
extern HWND bMainDelete;
extern LRESULT selectedRow;

void centerWindow(HWND hwnd)
{
	if (debugging)
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
	for (int i = 0; i < entryCount; ++i)
	{
		char row[MAXLINE];
		strcpy(row, entries[i].title);
		SendMessage(lbList, LB_ADDSTRING, i, (LPARAM)row);
	}
}

void updateListbox(void)
{
	if (debugging)
		outs("updateListbox()");

	// deselect all entries
	SendMessage(lbList, LB_SETCURSEL, -1, 0);
	EnableWindow(bMainEdit, FALSE);
	EnableWindow(bMainDelete, FALSE);
	sortEntries();
	selectedRow = LB_ERR;
}

void deleteEntry(void)
{
	if (debugging)
		outs("deleteEntry()");

	if (selectedRow != LB_ERR)
	{
		if (MessageBox(NULL, "Are you sure you want to delete this entry?", "Confirm delete", MB_YESNO | MB_ICONWARNING) == IDYES)
		{
			// delete row
			SendMessage(lbList, LB_DELETESTRING, selectedRow, 0);

			// recreate array without deleted row
			struct Entry *newEntries = (struct Entry *)malloc(sizeof(struct Entry) * (entryCount-1));

			for (int i = 0, j = 0; i < entryCount; ++i, ++j)
				if (i != (selectedRow))
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
			--entryCount;
		}
		updateListbox();
	}
}

void outs(char *s)
{
	FILE *lf = fopen(logFile, "a");
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
	if (debugging)
		outs("isNumeric()");

	if ((int)buf[0] < 48 || (int)buf[0] > 57)
		return false;
	else
		return true;
}

void fillDropdown(HWND hwnd, int min, int max)
{
	if (debugging)
		outs("fillDropdown()");

	for (int i = min; i <= max; ++i)
	{
		char c[3];
		sprintf(c, "%d", i);
		SendMessage(hwnd, CB_ADDSTRING, 0, (LPARAM)c);
	}
}

void writeSettings(void)
{
	if (debugging)
		outs("writeSettings()");

	if (strlen(iv) < IV_SIZE - 1)
	{
		outs("Generating new keygen");
		generateKeygen(iv);
		outs(iv);
	}

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
	fprintf(f, "keygen=%s\n", iv);
	fprintf(f, "window_row=%d\n", screenRow);
	fprintf(f, "window_col=%d\n", screenCol);
	fclose(f);
}

void shutDown(HWND hwnd)
{
	RECT rc = {0};
	GetWindowRect(hwnd, &rc);
	screenRow = rc.top;
	screenCol = rc.left;
	writeSettings();
	saveEntries();
	PostQuitMessage(0);
	running = false;
}
