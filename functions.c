#define _CRT_SECURE_NO_WARNINGS

#include "gui.h"
#include "shared.h"

extern int passwordSize;
extern int minSpecial;
extern int minNumeric;
extern int minUppercase;
extern bool debugging;
extern char logFile[];
extern char iniFile[];
extern char iv[];
extern char DBpassword[];
extern HWND lbList, bEdit, bDelete;
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

void updateListbox(void)
{
	if (debugging)
		outs("updateListbox()");

	// deselect all entries
	SendMessage(lbList, LB_SETCURSEL, -1, 0);
	EnableWindow(bEdit, FALSE);
	EnableWindow(bDelete, FALSE);
	selectedRow = LB_ERR;
	sortEntries();
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
			entries[selectedRow].title[0] = '\0';
			updateListbox();
		}
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
		char c[2];
		sprintf(c, "%d", i);
		SendMessage(hwnd, CB_ADDSTRING, 0, (LPARAM)c);
	}
}

void writeSettings(void)
{
	if (debugging)
		outs("writeSettings()");

	FILE *f = fopen(iniFile, "w");
	if (f == NULL)
	{
		outs("Error saving entries!");
		return;
	}

	if (strlen(iv) < IV_SIZE - 1)
	{
		outs("Generating new keygen");
		generateKeygen(iv);
		outs(iv);
		writeSettings(); //FIX potential infinite loop here
	}

	fprintf(f, "password_size=%d\n", passwordSize);
	fprintf(f, "min_special=%d\n", minSpecial);
	fprintf(f, "min_numeric=%d\n", minNumeric);
	fprintf(f, "min_uppercase=%d\n", minUppercase);
	fprintf(f, "keygen=%s\n", iv);
	fclose(f);
}

bool setDBpassword(void)
{
	if (debugging)
		outs("setDBpassword()");

//TODO show password form

	strcpy(DBpassword, "poop");
	saveEntries();
	return true;
}

void getDBpassword(uint8_t *password)
{
	if (debugging)
		outs("getPassword()");

//TODO show password form

	strcpy(DBpassword, "poop");
}
