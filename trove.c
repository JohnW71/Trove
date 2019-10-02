#define _CRT_SECURE_NO_WARNINGS

#include "trove.h"
#include "shared.h"

#ifdef _WIN32
	#include <windows.h>
	#include <conio.h>
#else
	#include <termios.h>
#endif

static void add(void);
static void find(void);
static void edit(void);
static void delEntry(void);
static void showEntry(int);
static void clipboard(void);
static void updateSettings(void);
static void handleParameters(char *);
static void setPasswordSize(void);
static void setMinSpecial(void);
static void setMinNumeric(void);
static void setMinUppercase(void);
static void setNewKeygen(void);
#ifdef _WIN32
static void getPasswordWindows(char *);
#else
static void getPasswordLinux(char *);
#endif
static void importFromUPM();
static void readUntilComma(char *, int);
static void readMisc(char *, int);
static bool copyBuffer(char *, char *, int *, int, int);

int main(int argc, char *argv[])
{
	if (argc > 1)
	{
		handleParameters(argv[1]);
		exit(0);
	}

	state.entryCount = 0;
	state.noDatabase = false;
	readSettings(INI_FILE);
	readEntries();

	int choice = -1;
	srand((unsigned int)time(NULL));

	while (choice != 0)
	{
		puts(APP_VERSION);
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
				if (state.entryCount > 0)
				{
					sortEntries();
					for (int i = 0; i < state.entryCount; ++i)
						showEntry(i);
				}
				break;
			case 2:
				add();
				break;
			case 3:
				if (state.entryCount > 0)
					find();
				break;
			case 4:
				if (state.entryCount > 0)
					edit();
				break;
			case 5:
				if (state.entryCount > 0)
					delEntry();
				break;
			case 6:
				if (state.entryCount > 0)
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

static void add(void)
{
	char title[MAXLINE];
	char id[MAXLINE];
	char password[MAXLINE];
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

	for (i = 0; i < state.entryCount; ++i)
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
	{
		id[0] = ' ';
		id[1] = '\0';
	}

	i = -1;
	while (id[++i] != '\0')
		if (id[i] == '\n' || i == MAXID - 1)
			id[i] = '\0';

	printf("Enter password up to %d chars (no commas):\n(enter 'x' to generate random \
password of %d chars)\n: ", MAXPW - 1, settings.passwordSize);
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
		{
			password[0] = ' ';
			password[1] = '\0';
		}

		i = -1;
		while (password[++i] != '\0')
		{
			if (password[i] == ',')
			{
				puts("Commas not allowed");
				return;
			}
			if (password[i] == '\n' || i == MAXPW - 1)
				password[i] = '\0';
		}
	}

	printf("Enter misc up to %d chars:\n", MAXMISC - 1);
	if (fgets(misc, MAXLINE, stdin) == NULL)
	{
		puts("No line");
		return;
	}

	if (misc[0] == '\n')
	{
		misc[0] = ' ';
		misc[1] = '\0';
	}

	i = -1;
	while (misc[++i] != '\0')
	{
		if (misc[i] == '\n')
			misc[i] = ' ';
		if (i == MAXMISC - 1)
			misc[i] = '\0';
	}

	struct Entry *temp = realloc(entries, ((uint64_t)state.entryCount + 1) * sizeof(*entries));
	if (temp == NULL)
	{
		puts("Failure reallocating memory for new entry");
		return;
	}
	entries = temp;

	removeCommas(title, (int)strlen(title));
	removeCommas(id, (int)strlen(id));
	removeCommas(misc, (int)strlen(misc));
	strcpy(entries[state.entryCount].title, title);
	strcpy(entries[state.entryCount].id, id);
	strcpy(entries[state.entryCount].pw, password);
	strcpy(entries[state.entryCount].misc, misc);
	++state.entryCount;
	saveEntries();
}

static void find(void)
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

	// remove \n from entered line and convert to uppercase
	int i = -1;
	while (title[++i] != '\0')
	{
		title[i] = (char)toupper(title[i]);
		if (title[i] == '\n')
			title[i] = '\0';
	}

	// search for match
	for (i = 0; i < state.entryCount; ++i)
	{
		// convert to uppercase
		int j = 0;
		char upper[MAXTITLE];
		strcpy(upper, entries[i].title);
		while (upper[++j] != '\0')
			upper[j] = (char)toupper(upper[j]);

		if (strcmp(upper, title) == 0)
		{
			puts("");
			showEntry(i);
			return;
		}
	}
	puts("Not found");
}

static void edit(void)
{
	char line[MAXLINE];
	char title[MAXLINE];
	char id[MAXLINE];
	char password[MAXLINE];
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

	if (choice < 1 || choice > state.entryCount)
	{
		printf("Range is 1 to %d\n", state.entryCount);
		return;
	}

	--choice;
	puts("");
	showEntry(choice);
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

	printf("Enter new password up to %d chars: (no commas)\n(enter 'x' to generate random \
password of %d chars)\n: ", MAXPW - 1, settings.passwordSize);
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
			{
				if (password[i] == ',')
				{
					puts("Commas not allowed");
					return;
				}
				if (password[i] == '\n' || i == MAXPW - 1)
					password[i] = '\0';
			}
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

	removeCommas(title, (int)strlen(title));
	removeCommas(id, (int)strlen(id));
	removeCommas(misc, (int)strlen(misc));
	strcpy(entries[choice].title, title);
	strcpy(entries[choice].id, id);
	strcpy(entries[choice].pw, password);
	strcpy(entries[choice].misc, misc);
	saveEntries();
}

static void delEntry(void)
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

	if (choice < 1 || choice > state.entryCount)
	{
		printf("Range is 1 to %d\n", state.entryCount);
		return;
	}

	// recreate array without deleted row
	struct Entry *newEntries = (struct Entry *)malloc(sizeof(struct Entry) * ((uint64_t)state.entryCount - 1));

	for (int i = 0, j = 0; i < state.entryCount; ++i, ++j)
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
	--state.entryCount;
	saveEntries();
}

static void clipboard(void)
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

	if (choice < 1 || choice > state.entryCount)
	{
		printf("Range is 1 to %d\n", state.entryCount);
		return;
	}

	--choice;
	puts("");
	showEntry(choice);

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

void writeSettings(char *iniFile)
{
	if (strlen(settings.iv) < IV_SIZE - 1)
	{
		puts("Generating new keygen");
		generateKeygen(settings.iv);
		puts(settings.iv);
	}

	FILE *f = fopen(iniFile, "w");
	if (f == NULL)
	{
		puts("Error saving settings!");
		return;
	}

	fputs("#Manually editing the keygen value will make it\n", f);
	fputs("#impossible to decrypt your database again!\n", f);
	fprintf(f, "password_size=%d\n", settings.passwordSize);
	fprintf(f, "min_special=%d\n", settings.minSpecial);
	fprintf(f, "min_numeric=%d\n", settings.minNumeric);
	fprintf(f, "min_uppercase=%d\n", settings.minUppercase);
	fprintf(f, "keygen=%s\n", settings.iv);
	fclose(f);
}

static void updateSettings(void)
{
	int choice = -1;

	while (choice != 0)
	{
		puts("Change Settings");
		puts("---------------");
		printf("1 - Change database password\n");
		printf("2 - Set password generation size (%d)\n", settings.passwordSize);
		printf("3 - Set minimum special characters (%d)\n", settings.minSpecial);
		printf("4 - Set minimum numeric characters (%d)\n", settings.minNumeric);
		printf("5 - Set minimum uppercase characters (%d)\n", settings.minUppercase);
		printf("6 - Set new random keygen ID (%s)\n", settings.iv);
		printf("7 - Export database to text file\n");
		printf("8 - Import from UPM\n");
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
			case 7:
				exportDB();
				break;
			case 8:
				importFromUPM();
				break;
			case 0:
				return;
		}

		puts("");
	}
}

static void setPasswordSize(void)
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

	settings.passwordSize = atoi(line);
	writeSettings(INI_FILE);
}

static void setMinSpecial(void)
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

	settings.minSpecial = value;
	writeSettings(INI_FILE);
}

static void setMinNumeric(void)
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

	settings.minNumeric = value;
	writeSettings(INI_FILE);
}

static void setMinUppercase(void)
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

	settings.minUppercase = value;
	writeSettings(INI_FILE);
}

static void setNewKeygen(void)
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
		generateKeygen(settings.iv);
		writeSettings(INI_FILE);
		saveEntries();
		puts("Keygen updated & database saved");
	}
}

bool setDBpassword(void)
{
	char verifyPassword1[DBPASSWORDSIZE];
	char verifyPassword2[DBPASSWORDSIZE];

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

	clearArray(state.DBpassword, DBPASSWORDSIZE);
	strcpy(state.DBpassword, verifyPassword1);
	saveEntries();

#ifdef _WIN32
	puts("");
#endif

	return true;
}

void getDBpassword(char *password)
{
#ifdef _WIN32
	getPasswordWindows(password);
#else
	getPasswordLinux(password);
#endif
}

#ifdef _WIN32
static void getPasswordWindows(char *password)
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
static void getPasswordLinux(char *password)
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

	clearArray(password, DBPASSWORDSIZE);
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

static void handleParameters(char *parameter)
{
	if (strcmp(parameter, "-v") == 0 ||
		strcmp(parameter, "-version") == 0 ||
		strcmp(parameter, "-h") == 0 ||
		strcmp(parameter, "-help") == 0)
	{
		puts(APP_VERSION);
		puts("By John Wingfield\n");
		puts("Trove is an encrypted password database for Windows and Linux.");
		puts("It uses AES-256 bit encryption with the tiny-AES library. The encryption");
		puts("uses a random keygen stored in the INI file. You can generate a new key");
		puts("from the application settings menu if required. Changing the key manually");
		puts("will make it impossible to decrypt your database.\n");
		puts("The latest version can be downloaded from");
		puts("https://github.com/JohnW71/Trove/releases\n");
	}
	else
	{
		puts("Use just \"trove_cli\" or with \"-v\" for the info page\n");
	}
}

static void showEntry(int position)
{
	printf("%2d: %s, %s, %s\n", position + 1,
								entries[position].title,
								entries[position].id,
								entries[position].pw);

	if (strlen(entries[position].misc) > 0 && entries[position].misc[0] != ' ')
		printf("\t%s\n", entries[position].misc);
}

static bool running = true;
static FILE *f;

static void importFromUPM()
{
	f = fopen("upm.txt", "r");
	if (f == NULL)
	{
		puts("can't open file");
		return;
	}

	char title[MAXTITLE];
	char id[MAXID];
	char pw[MAXPW];
	char misc[MAXMISC];

	while (running)
	{
		clearArray(title, MAXTITLE);
		clearArray(id, MAXID);
		clearArray(pw, MAXPW);
		clearArray(misc, MAXMISC);

		readUntilComma(title, MAXTITLE-1);
		readUntilComma(id, MAXID-1);
		readUntilComma(pw, MAXPW-1);
		readUntilComma(misc, 1); // URL field
		readMisc(misc, MAXMISC-1);

		if (strlen(id) == 0) strcpy(id, " ");
		if (strlen(pw) == 0) strcpy(pw, " ");
		if (strlen(misc) == 0) strcpy(misc, " ");

		for (int i = 0; i < MAXMISC; ++i)
			if (misc[i] == '\n')
				misc[i] = ' ';

		if (strlen(title))
		{
			printf("%s, %s, %s, %s\n", title, id, pw, misc);

			struct Entry *temp = realloc(entries, ((uint64_t)state.entryCount + 1) * sizeof(*entries));
			if (temp == NULL)
			{
				puts("Failure reallocating memory for new entry");
				return;
			}
			entries = temp;

			removeCommas(title, (int)strlen(title));
			removeCommas(id, (int)strlen(id));
			removeCommas(pw, (int)strlen(pw));
			removeCommas(misc, (int)strlen(misc));
			strcpy(entries[state.entryCount].title, title);
			strcpy(entries[state.entryCount].id, id);
			strcpy(entries[state.entryCount].pw, pw);
			strcpy(entries[state.entryCount].misc, misc);
			++state.entryCount;
		}
	}
	saveEntries();
	fclose(f);
}

static void readUntilComma(char *text, int len)
{
	char c, charAhead;
	int pos = 0;
	bool inQuotes = false;

	if ((c = (char)fgetc(f)) == EOF)
		return;

	if (c == '"')
		inQuotes = true;

	while (c != EOF && pos < len)
	{
		if (c == ',' && !inQuotes)
			break;

		charAhead = (char)fgetc(f);

		switch (c)
		{
			case '"':
				if (charAhead == '"' && !inQuotes)
				{
					text[pos++] = c;
					inQuotes = true;
					break;
				}

				if (charAhead == ',')
				{
					inQuotes = false;
					break;
				}

				if (charAhead == '"' && inQuotes)
				{
					if (text[pos - 1] != '"')
						text[pos++] = c;
					break;
				}

				break;
			case ',':
				if (inQuotes)
					text[pos++] = c;
				else
				{
					text[pos] = '\0';
					return;
				}
				break;
			default:
				text[pos++] = c;
				break;
		}

		c = charAhead;
		if (c == EOF)
		{
			running = false;
			text[pos] = '\0';
			return;
		}
	}
}

static bool copyBuffer(char *text, char *buf, int *count, int pos, int len)
{
	if (*count >= len)
		return true;

	for (int t = *count, b = 0; b < pos; ++t, ++b)
	{
		if (buf[b] == '"' && buf[b + 1] == '"' && buf[b + 2] != '"')
			++b;
		if (buf[b] == '"' && buf[b + 1] == '"' && buf[b + 2] == '"')
			b += 2;

		text[t] = buf[b];

		if (++(*count) >= len)
		{
			text[*count] = '\0';
			return true;
		}
	}
	return false;
}

static void readMisc(char *text, int len)
{
	char buf[1000];
	char c;
	int pos = 0;
	int count = 0;
	int *pCount = &count;
	bool inQuotes = false;
	bool multiLine = false;
	bool finished = false;

	if ((c = (char)fgetc(f)) == EOF)
	{
		running = false;
		return;
	}

	if (c == '\n') // blank misc
		return;

	if (c == '"')
	{
		inQuotes = true;
		c = (char)fgetc(f);
	}

	while (!finished)
	{
		clearArray(buf, 1000);
		pos = 0;

		// read rest of line until \n
		while (c != EOF && c != '\n')
		{
			buf[pos++] = c;
			c = (char)fgetc(f);
		}

		// handle blank lines
		if ((c == '\n' || c == '\r') && pos == 0)
		{
			c = (char)fgetc(f);
			if ((c == '\n' || c == '\r') && pos == 0)
			{
				if (count < len)
				{
					text[count++] = '\n';
					c = (char)fgetc(f);
				}
			}
			continue;
		}

		if (c == EOF)
			finished = true;

		if (inQuotes && (c == '\n' || c == '\r') && buf[pos] != '"')
			multiLine = true;

		// normal text
		if ((c == '\n' || c == '\r') && !inQuotes && !multiLine)
		{
			// copy to text[]
			for (int t = 0, b = 0; b < pos; ++t, ++b)
				text[t] = buf[b];
			text[pos] = '\0';
			break;
		}

		buf[pos] = '\n';

		// multiline ending """\n
		if (inQuotes &&
			((buf[pos] == '\n'
				&& buf[pos - 1] == '\r'
				&& buf[pos - 2] == '"'
				&& buf[pos - 3] == '"'
				&& buf[pos - 4] == '"')
				||
				(buf[pos] == '\n'
					&& buf[pos - 1] == '"'
					&& buf[pos - 2] == '"'
					&& buf[pos - 3] == '"')))
		{
			if (copyBuffer(text, buf, pCount, pos, len))
				return;

			text[count-1] = '\0';
			finished = true;
			break;
		}

		// multiline ending "\n
		if (inQuotes &&
			((buf[pos] == '\n' &&
				buf[pos - 1] == '\r' &&
				buf[pos - 2] == '"' &&
				buf[pos - 3] != '"')
				||
				(buf[pos] == '\n' &&
					buf[pos - 1] == '"' &&
					buf[pos - 2] != '"')))
		{
			--pos; // hide the last "

			if (copyBuffer(text, buf, pCount, pos, len))
				return;

			text[count-1] = '\0';
			finished = true;
			break;
		}

		// multiline middle
		copyBuffer(text, buf, pCount, pos, len);

		if (count < len)
			text[count++] = ' ';
		else
			text[count-1] = '\0';
	}
}
