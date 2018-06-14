#define _CRT_SECURE_NO_WARNINGS

//TODO encryption
//TODO GUI

#include "trove.h"

#ifdef _WIN32
	#include <windows.h>
#endif

int generationSize = 12;
int minSpecial = 0;
int minNumeric = 0;
int minUppercase = 0;

char heading[] = "    Title                ID                   Password             Misc";
char iniFile[] = "trove.ini";
char dbFile[] = "trove.db";

int main()
{
	entryCount = 0;

	readSettings();
	readEntries();
	encrypt();

	int choice = 0; //TODO put this back! -1;
	srand((unsigned int)time(NULL));

	while (choice != 0)
	{
		puts("\nTrove v0.5");
		puts("----------");
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

		printf("\n");
		choice = (int)line[0] - 48;
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
					delete();
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
	return 0;
}

void list()
{
	puts(heading);
	for (int i = 0; i < entryCount; ++i)
		printf("%2d: %-*s%-*s%-*s%s\n", i + 1, MAXTITLE, entries[i].title,
			   MAXID, entries[i].id,
			   MAXPW, entries[i].pw,
			   entries[i].misc);
}

void add()
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

	for (int i = 0; i < entryCount; ++i)
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
password of %d chars)\n: ", MAXPW - 1, generationSize);
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
		if (misc[i] == '\n' || i == MAXMISC - 1)
			misc[i] = '\0';

	entries = realloc(entries, (entryCount + 1) * sizeof *entries);
	strcpy(entries[entryCount].title, title);
	strcpy(entries[entryCount].id, id);
	strcpy(entries[entryCount].pw, password);
	strcpy(entries[entryCount].misc, misc);
	++entryCount;
	saveEntries();
}

void find()
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

	for (int i = 0; i < entryCount; ++i)
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

void edit()
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
password of %d chars)\n: ", MAXPW - 1, generationSize);
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

void delete()
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

	entries[choice - 1].title[0] = '\0';
	saveEntries();
	readEntries();
}

void clipboard()
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
	memcpy(GlobalLock(hMem), output, len);
	GlobalUnlock(hMem);
	OpenClipboard(0);
	EmptyClipboard();
	SetClipboardData(CF_TEXT, hMem); // automatically does GlobalFree(hMem);
	CloseClipboard();

	puts("\nPassword copied to clipboard");
#endif
}

void readEntries()
{
	FILE *f = fopen(dbFile, "r");
	if (f == NULL)
		return;

	char line[MAXLINE];
	char data[MAXLINE];
	char password[MAXLINE];
	entryCount = 0;
	entries = NULL;

	// check for password
	if (fgets(line, MAXPW, f) != NULL)
	{
		// find and convert CRLF endings
		char *p = strrchr(line, '\r');
		if (p && p[1] == '\n' && p[2] == '\0') {
			p[0] = '\n';
			p[1] = '\0';
		}

		if ((strcmp(line, ".\n") != 0) &&
			(strcmp(line, DBpassword) != 0)) // DB has a password
		{
			printf("Enter password: ");

			if (fgets(password, MAXPW, stdin) == NULL)
			{
				puts("Null!");
				fclose(f);
				exit(1);
			}

			if (password[0] == '\n')
			{
				puts("Blank line entered!");
				fclose(f);
				exit(1);
			}

			if (strcmp(password, line) != 0)
			{
				puts("Incorrect password entered!");
				fclose(f);
				exit(1);
			}

			strcpy(DBpassword, line);
		}
	}

	while (!feof(f))
	{
		if (fgets(line, MAXLINE, f) != NULL)
		{
			entries = realloc(entries, (entryCount + 1) * sizeof *entries);
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
						strcpy(entries[entryCount].title, data);
						break;
					case 1:
						strcpy(entries[entryCount].id, data);
						break;
					case 2:
						strcpy(entries[entryCount].pw, data);
						break;
					case 3:
						strcpy(entries[entryCount].misc, data);
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
}

void saveEntries()
{
	FILE *f = fopen(dbFile, "w");
	if (f == NULL)
	{
		puts("Error saving entries!");
		exit(1);
	}

	if (strlen(DBpassword) > 0)
		fprintf(f, "%s", DBpassword);
	else
		fprintf(f, ".\n");

	for (int i = 0; i < entryCount; ++i)
		if (entries[i].title[0] != '\0') // skip deleted entries
			fprintf(f, "%s,%s,%s,%s\n", entries[i].title,
					entries[i].id,
					entries[i].pw,
					entries[i].misc);
	fclose(f);
}

// random chars from 33 to 126, not 44 (commas)
void generatePassword(char *buf)
{
	int rn;
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
			rn = rand() % 127;
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

void setDBpassword()
{
	printf("Enter password up to %d chars: ", MAXPW - 1);

	if (fgets(DBpassword, MAXPW, stdin) == NULL)
	{
		puts("No line");
		return;
	}

	if (DBpassword[0] == '\n')
		return;

	saveEntries();
	readEntries();
}

void removeDBpassword()
{
	strcpy(DBpassword, ".\n");
	saveEntries();
	readEntries();
	puts("Password removed");
}

void readSettings()
{
	FILE *f = fopen(iniFile, "r");
	if (f == NULL)
	{
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

		if (strcmp(setting, "password_size") == 0)	generationSize = atoi(value);
		if (strcmp(setting, "min_special") == 0)	minSpecial = atoi(value);
		if (strcmp(setting, "min_numeric") == 0)	minNumeric = atoi(value);
		if (strcmp(setting, "min_uppercase") == 0)	minUppercase = atoi(value);
	}
	fclose(f);
}

void writeSettings()
{
	FILE *f = fopen(iniFile, "w");
	if (f == NULL)
	{
		puts("Error saving entries!");
		return;
	}

	fprintf(f, "password_size=%d\n", generationSize);
	fprintf(f, "min_special=%d\n", minSpecial);
	fprintf(f, "min_numeric=%d\n", minNumeric);
	fprintf(f, "min_uppercase=%d\n", minUppercase);
	fclose(f);
}

void updateSettings()
{
	int choice = -1;

	while (choice != 0)
	{
		puts("\nChange Settings");
		puts("---------------");
		printf("1 - Set password generation size (%d)\n", generationSize);
		puts("2 - Set database password");
		puts("3 - Remove database password");
		printf("4 - Set minimum special characters (%d)\n", minSpecial);
		printf("5 - Set minimum numeric characters (%d)\n", minNumeric);
		printf("6 - Set minimum uppercase characters (%d)\n", minUppercase);
		puts("0 - Back");
		printf("\n-> ");

		char line[MAXLINE];
		if (fgets(line, MAXLINE, stdin) == NULL)
			continue;

		if (line[1] != '\n' || (int)line[0] < 48 || (int)line[0] > 57)
			continue;

		printf("\n");
		choice = (int)line[0] - 48;
		switch (choice)
		{
			case 1:
				setPasswordSize();
				break;
			case 2:
				setDBpassword();
				break;
			case 3:
				removeDBpassword();
				break;
			case 4:
				setMinSpecial();
				break;
			case 5:
				setMinNumeric();
				break;
			case 6:
				setMinUppercase();
				break;
			case 0:
				return;
		}
	}

}

void setPasswordSize()
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

	generationSize = atoi(line);
	writeSettings();
}

void setMinSpecial()
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

void setMinNumeric()
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

void setMinUppercase()
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
