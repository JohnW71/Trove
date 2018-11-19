#define _CRT_SECURE_NO_WARNINGS

#include "trove.h"
#include "shared.h"

#ifdef _WIN32
	#include <windows.h>
	#include <conio.h>
#else
	#include <termios.h>
#endif

int entryCount = 0;
int passwordSize = 12;
int minSpecial = 0;
int minNumeric = 0;
int minUppercase = 0;
int screenCol = 0; // not used
int screenRow = 0; // not used
char iniFile[] = "trove.ini";
uint8_t iv[IV_SIZE];
uint8_t DBpassword[DBPASSWORDSIZE];

static char heading[] = "    Title                ID                   Password             Misc";

int main(void)
{
	readSettings();
	readEntries();

	int choice = -1;
	srand((unsigned int)time(NULL));

	while (choice != 0)
	{
		puts("\nTrove v1.31");
		puts("-----------");
		puts("1 - List");
		puts("2 - Add");
		puts("3 - Find");
		puts("4 - Edit");
		puts("5 - Delete");
#ifdef _WIN32
		puts("6 - Copy to clipboard");
#endif
		puts("7 - Change settings");
		puts("0 - Quit");
		printf("\n-> ");

		char line[MAXLINE];
		if (fgets(line, MAXLINE, stdin) == NULL)
			continue;

		if (line[1] != '\n' || (int)line[0] < 48 || (int)line[0] > 57)
			continue;

		choice = (int)line[0] - 48;
		if (choice != 0)
			puts("");

		switch (choice)
		{
			case 1:
				if (entryCount > 0)
					list();
				break;
			case 2:
				add();
				break;
			case 3:
				if (entryCount > 0)
					find();
				break;
			case 4:
				if (entryCount > 0)
					edit();
				break;
			case 5:
				if (entryCount > 0)
					delEntry();
				break;
			case 6:
				if (entryCount > 0)
					clipboard();
				break;
			case 7:
				updateSettings();
				break;
		}
	}

	free(entries);

#ifndef _WIN32
	puts("");
#endif
	return 0;
}

void list(void)
{
	sortEntries();
	puts(heading);
	for (int i = 0; i < entryCount; ++i)
		printf("%2d: %-*s%-*s%-*s%s\n", i + 1, MAXTITLE, entries[i].title,
												MAXID, entries[i].id,
												MAXPW, entries[i].pw,
												entries[i].misc);
}

void add(void)
{
	char title[MAXLINE];
	char password[MAXLINE];
	char id[MAXLINE];
	char misc[MAXLINE];

	printf("Enter title up to %d chars: ", MAXTITLE - 1);
	if (fgets(title, MAXLINE, stdin) == NULL)
	{
		puts("No line");
		return;
	}

	if (title[0] == '\n')
		return;

	int i = -1;
	while (title[++i] != '\0')
		if (title[i] == '\n' || i == MAXTITLE - 1)
			title[i] = '\0';

	for (i = 0; i < entryCount; ++i)
		if (strcmp(entries[i].title, title) == 0)
		{
			puts("Title is already in use");
			return;
		}

	printf("Enter ID up to %d chars: ", MAXID - 1);
	if (fgets(id, MAXLINE, stdin) == NULL)
	{
		printf("No line\n");
		return;
	}

	if (id[0] == '\n')
		return;

	i = -1;
	while (id[++i] != '\0')
		if (id[i] == '\n' || i == MAXID - 1)
			id[i] = '\0';

	printf("Enter password up to %d chars:\n(enter 'x' to generate random \
password of %d chars)\n: ", MAXPW - 1, passwordSize);
	if (fgets(password, MAXLINE, stdin) == NULL)
	{
		puts("No line");
		return;
	}

	if (strcmp(password, "x\n") == 0)
		generatePassword(password);
	else
	{
		i = -1;
		while (password[++i] != '\0')
			if (password[i] == '\n' || i == MAXPW - 1)
				password[i] = '\0';
	}

	printf("Enter misc up to %d chars:\n", MAXMISC - 1);
	if (fgets(misc, MAXLINE, stdin) == NULL)
	{
		puts("No line");
		return;
	}

	i = -1;
	while (misc[++i] != '\0')
	{
		if (misc[i] == '\n')
			misc[i] = ' ';
		if (i == MAXMISC - 1)
			misc[i] = '\0';
	}

	struct Entry *temp = realloc(entries, (entryCount+1) * sizeof(*entries));
	if (temp == NULL)
	{
		puts("Failure reallocating memory for new entry");
		return;
	}
	entries = temp;

	strcpy(entries[entryCount].title, title);
	strcpy(entries[entryCount].id, id);
	strcpy(entries[entryCount].pw, password);
	strcpy(entries[entryCount].misc, misc);
	++entryCount;
	saveEntries();
}

void find(void)
{
	char title[MAXTITLE];
	printf("Enter title to find: ");

	if (fgets(title, MAXTITLE, stdin) == NULL)
	{
		puts("No line");
		return;
	}

	if (title[0] == '\n')
		return;

	// remove \n from entered line
	int i = -1;
	while (title[++i] != '\0')
		if (title[i] == '\n')
			title[i] = '\0';

	for (i = 0; i < entryCount; ++i)
		if (strcmp(entries[i].title, title) == 0)
		{
			printf("\n%s\n", heading);
			printf("%2d: %-*s%-*s%-*s%s\n", i + 1,
				   MAXTITLE, entries[i].title,
				   MAXID, entries[i].id,
				   MAXPW, entries[i].pw,
				   entries[i].misc);
			return;
		}

	puts("Not found");
}

void edit(void)
{
	char line[MAXLINE];
	char title[MAXLINE];
	char password[MAXLINE];
	char id[MAXLINE];
	char misc[MAXLINE];
	int i;

	printf("Enter # to edit: ");
	if (fgets(line, MAXLINE, stdin) == NULL)
	{
		puts("No line");
		return;
	}

	if (line[0] == '\n')
		return;

	int choice = atoi(line);

	if (choice < 1 || choice > entryCount)
	{
		printf("Range is 1 to %d\n", entryCount);
		return;
	}

	--choice;
	printf("\n%s\n", heading);
	printf("%2d: %-*s%-*s%-*s%s\n", choice + 1,
		   MAXTITLE, entries[choice].title,
		   MAXID, entries[choice].id,
		   MAXPW, entries[choice].pw,
		   entries[choice].misc);
	printf("\nEnter new title up to %d chars: ", MAXTITLE - 1);
	if (fgets(title, MAXLINE, stdin) == NULL)
	{
		puts("No line");
		return;
	}

	if (title[0] == '\n')
		strcpy(title, entries[choice].title);
	else
	{
		i = -1;
		while (title[++i] != '\0')
			if (title[i] == '\n' || i == MAXTITLE - 1)
				title[i] = '\0';
	}

	printf("Enter new ID up to %d chars: ", MAXID - 1);
	if (fgets(id, MAXLINE, stdin) == NULL)
	{
		puts("No line");
		return;
	}

	if (id[0] == '\n')
		strcpy(id, entries[choice].id);
	else
	{
		i = -1;
		while (id[++i] != '\0')
			if (id[i] == '\n' || i == MAXID - 1)
				id[i] = '\0';
	}

	printf("Enter new password up to %d chars:\n(enter 'x' to generate random \
password of %d chars)\n: ", MAXPW - 1, passwordSize);
	if (fgets(password, MAXLINE, stdin) == NULL)
	{
		puts("No line");
		return;
	}

	if (strcmp(password, "x\n") == 0)
		generatePassword(password);
	else
	{
		if (password[0] == '\n')
			strcpy(password, entries[choice].pw);
		else
		{
			i = -1;
			while (password[++i] != '\0')
				if (password[i] == '\n' || i == MAXPW - 1)
					password[i] = '\0';
		}
	}

	printf("Enter new misc up to %d chars:\n", MAXMISC - 1);
	if (fgets(misc, MAXLINE, stdin) == NULL)
	{
		puts("No line");
		return;
	}

	if (misc[0] == '\n')
		strcpy(misc, entries[choice].misc);
	else
	{
		i = -1;
		while (misc[++i] != '\0')
			if (misc[i] == '\n' || i == MAXMISC - 1)
				misc[i] = '\0';
	}

	strcpy(entries[choice].title, title);
	strcpy(entries[choice].id, id);
	strcpy(entries[choice].pw, password);
	strcpy(entries[choice].misc, misc);
	saveEntries();
}

void delEntry(void)
{
	char line[MAXLINE];
	printf("Enter # to delete: ");

	if (fgets(line, MAXLINE, stdin) == NULL)
	{
		puts("No line");
		return;
	}

	if (line[0] == '\n')
		return;

	int choice = atoi(line);

	if (choice < 1 || choice > entryCount)
	{
		printf("Range is 1 to %d\n", entryCount);
		return;
	}

	// recreate array without deleted row
	struct Entry *newEntries = (struct Entry *)malloc(sizeof(struct Entry) * (entryCount-1));

	for (int i = 0, j = 0; i < entryCount; ++i, ++j)
		if (i != (choice-1))
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
	saveEntries();
}

void clipboard(void)
{
#ifdef _WIN32
	char line[MAXLINE];

	printf("Enter # to copy to clipboard: ");
	if (fgets(line, MAXLINE, stdin) == NULL)
	{
		puts("No line");
		return;
	}

	if (line[0] == '\n')
		return;

	int choice = atoi(line);

	if (choice < 1 || choice > entryCount)
	{
		printf("Range is 1 to %d\n", entryCount);
		return;
	}

	--choice;
	printf("\n%s\n", heading);
	printf("%2d: %-*s%-*s%-*s%s\n", choice + 1,
		   MAXTITLE, entries[choice].title,
		   MAXID, entries[choice].id,
		   MAXPW, entries[choice].pw,
		   entries[choice].misc);

	const char *output = entries[choice].pw;
	const size_t len = strlen(output) + 1;
	HGLOBAL hMem = GlobalAlloc(GMEM_MOVEABLE, len);
	if (!hMem)
	{
		puts("Failure allocating memory for clipboard");
		return;
	}

	hMem = GlobalLock(hMem);
	if (!hMem)
	{
		puts("Failure locking memory for clipboard");
		return;
	}

	memcpy(hMem, output, len);
	GlobalUnlock(hMem);
	OpenClipboard(0);
	EmptyClipboard();

	// automatically does GlobalFree(hMem) if it succeeds
	if (!SetClipboardData(CF_TEXT, hMem))
		if (hMem)
			GlobalFree(hMem);

	CloseClipboard();

	puts("\nPassword copied to clipboard");
#endif
}

void writeSettings(void)
{
	if (strlen(iv) < IV_SIZE - 1)
	{
		puts("Generating new keygen");
		generateKeygen(iv);
		puts(iv);
	}

	FILE *f = fopen(iniFile, "w");
	if (f == NULL)
	{
		puts("Error saving entries!");
		return;
	}

	fprintf(f, "password_size=%d\n", passwordSize);
	fprintf(f, "min_special=%d\n", minSpecial);
	fprintf(f, "min_numeric=%d\n", minNumeric);
	fprintf(f, "min_uppercase=%d\n", minUppercase);
	fprintf(f, "keygen=%s\n", (char *)iv);
	fclose(f);
}

void updateSettings(void)
{
	int choice = -1;

	while (choice != 0)
	{
		puts("Change Settings");
		puts("---------------");
		printf("1 - Change database password\n");
		printf("2 - Set password generation size (%d)\n", passwordSize);
		printf("3 - Set minimum special characters (%d)\n", minSpecial);
		printf("4 - Set minimum numeric characters (%d)\n", minNumeric);
		printf("5 - Set minimum uppercase characters (%d)\n", minUppercase);
		printf("6 - Set new random keygen ID (%s)\n", (char *)iv);
		printf("0 - Back\n");
		printf("\n-> ");

		char line[MAXLINE];
		if (fgets(line, MAXLINE, stdin) == NULL)
			continue;

		if (line[1] != '\n' || (int)line[0] < 48 || (int)line[0] > 57)
			continue;

		choice = (int)line[0] - 48;
		if (choice != 0)
			puts("");

		switch (choice)
		{
			case 1:
				setDBpassword();
				break;
			case 2:
				setPasswordSize();
				break;
			case 3:
				setMinSpecial();
				break;
			case 4:
				setMinNumeric();
				break;
			case 5:
				setMinUppercase();
				break;
			case 6:
				setNewKeygen();
				break;
			case 0:
				return;
		}

		puts("");
	}
}

void setPasswordSize(void)
{
	char line[MAXLINE];

	printf("Enter new size for password generation, from %d to %d: ", MINPW, MAXPW - 1);
	if (fgets(line, MAXLINE, stdin) == NULL)
	{
		puts("No line");
		return;
	}

	if (line[0] == '\n')
		return;

	int i = -1;
	while (line[++i] != '\0' && line[i] != '\n')
		if ((int)line[0] < 48 || (int)line[0] > 57)
			return;

	if (atoi(line) < MINPW)
	{
		printf("Minimum password length is %d\n", MINPW);
		return;
	}

	if (atoi(line) > MAXPW - 1)
	{
		printf("Maximum password length is %d\n", MAXPW - 1);
		return;
	}

	passwordSize = atoi(line);
	writeSettings();
}

void setMinSpecial(void)
{
	char line[MAXLINE];

	printf("Enter new size for minimum special characters, from %d to %d: ", MINCHARS, MAXCHARS);
	if (fgets(line, MAXLINE, stdin) == NULL)
	{
		puts("No line");
		return;
	}

	if (line[0] == '\n')
		return;

	int i = -1;
	while (line[++i] != '\0' && line[i] != '\n')
		if ((int)line[0] < 48 || (int)line[0] > 57)
			return;

	int value = atoi(line);
	if (value < MINCHARS || value > MAXCHARS)
	{
		printf("Range is %d to %d\n", MINCHARS, MAXCHARS);
		return;
	}

	minSpecial = value;
	writeSettings();
}

void setMinNumeric(void)
{
	char line[MAXLINE];

	printf("Enter new size for minimum numeric characters, from %d to %d: ", MINCHARS, MAXCHARS);
	if (fgets(line, MAXLINE, stdin) == NULL)
	{
		puts("No line");
		return;
	}

	if (line[0] == '\n')
		return;

	int i = -1;
	while (line[++i] != '\0' && line[i] != '\n')
		if ((int)line[0] < 48 || (int)line[0] > 57)
			return;

	int value = atoi(line);
	if (value < MINCHARS || value > MAXCHARS)
	{
		printf("Range is %d to %d\n", MINCHARS, MAXCHARS);
		return;
	}

	minNumeric = value;
	writeSettings();
}

void setMinUppercase(void)
{
	char line[MAXLINE];

	printf("Enter new size for minimum uppercase characters, from %d to %d: ", MINCHARS, MAXCHARS);
	if (fgets(line, MAXLINE, stdin) == NULL)
	{
		puts("No line");
		return;
	}

	if (line[0] == '\n')
		return;

	int i = -1;
	while (line[++i] != '\0' && line[i] != '\n')
		if ((int)line[0] < 48 || (int)line[0] > 57)
			return;

	int value = atoi(line);
	if (value < MINCHARS || value > MAXCHARS)
	{
		printf("Range is %d to %d\n", MINCHARS, MAXCHARS);
		return;
	}

	minUppercase = value;
	writeSettings();
}

void setNewKeygen(void)
{
	char line[MAXLINE];

	printf("Keygen can be changed here only, do not edit it in the INI file!\n\n");
	printf("Are you sure? Enter \"YES\" to confirm: ");

	if (fgets(line, MAXLINE, stdin) == NULL)
	{
		puts("No line");
		return;
	}

	if (line[0] == '\n')
		return;

	if (strcmp(line, "YES\n") == 0)
	{
		generateKeygen(iv);
		writeSettings();
		saveEntries();
		puts("Keygen updated & database saved");
	}
}

bool setDBpassword(void)
{
	uint8_t verifyPassword1[DBPASSWORDSIZE];
	uint8_t verifyPassword2[DBPASSWORDSIZE];

	printf("Enter new password up to %d chars, press enter for blank: ", DBPASSWORDSIZE);
	getDBpassword(verifyPassword1);
	printf("\nEnter same password again to confirm: ");
	getDBpassword(verifyPassword2);

	if (strcmp(verifyPassword1, verifyPassword2) != 0)
	{
		puts("\nPasswords do not match!");
		return false;
	}
	puts("\n\nPassword set");

	for (int i = 0; i < DBPASSWORDSIZE; ++i)
		DBpassword[i] = '\0';

	strcpy(DBpassword, verifyPassword1);
	saveEntries();

#ifdef _WIN32
	puts("");
#endif

	return true;
}

void getDBpassword(uint8_t *password)
{
#ifdef _WIN32
	getPasswordWindows(password);
#else
	getPasswordLinux(password);
#endif
}

#ifdef _WIN32
void getPasswordWindows(uint8_t *password)
{
	char c;
	int i = -1;

	do
	{
		++i;
		c = (char)_getch();

		if (c != '\r')
		{
			printf("*");
			password[i] = c;
		}
	}
	while (c != '\r' && i < DBPASSWORDSIZE);

	password[i] = '\0';
}
#else
void getPasswordLinux(uint8_t *password)
{
	struct termios termCurrent;
	struct termios termNew;

	// store current terminal settings
	if (tcgetattr(fileno(stdin), &termCurrent) != 0)
	{
		puts("Failed storing terminal attributes");
		exit(1);
	}

	// modify new terminal attributes
	termNew = termCurrent;
	termNew.c_lflag &= ~ICANON;
	termNew.c_lflag &= ~ECHO;

	// set new terminal attributes
	if (tcsetattr(fileno(stdin), TCSAFLUSH, &termNew) != 0)
	{
		puts("Failed setting terminal attributes");
		exit(1);
	}

	for (int i = 0; i < DBPASSWORDSIZE; ++i)
		password[i] = '\0';

	char c;
	int i = -1;

	do
	{
		++i;
		c = getchar();

		if (c != '\n')
		{
			printf("*");
			password[i] = c;
		}
	}
	while (c != '\n' && i < DBPASSWORDSIZE);

	password[i] = '\0';

	// revert terminal attributes
	if (tcsetattr(fileno(stdin), TCSAFLUSH, &termCurrent) != 0)
	{
		puts("Failed resetting terminal attributes");
		exit(1);
	}
	puts("");
}
#endif
