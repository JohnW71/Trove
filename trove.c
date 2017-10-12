#define _CRT_SECURE_NO_WARNINGS

//TODO handle error checking on edit/delete
//TODO password protect database
//TODO encryption
//TODO settings
//TODO GUI

#include <stdio.h>
#include <stdlib.h> // exit(), rand()
#include <string.h> // strcmp(), strcpy()
#include <time.h> // srand()

#define MAXTITLE 21
#define MAXID 21
#define MAXPW 21
#define MAXMISC 51
#define MAXLINE 80

void list();
void add();
void find();
void edit();
void delete();
void readEntries();
void saveEntries();
void readSettings();
void generatePassword(char *buf);

struct entry
{
	char title[MAXTITLE];
	char id[MAXID];
	char pw[MAXPW];
	char misc[MAXMISC];
} *entries = NULL;

int entryCount = 0;

int main()
{
	readSettings();
	readEntries();

	int choice = -1;
	srand(time(NULL));

	while (choice != 0)
	{
		printf("\nTrove v0.1\n");
		printf("----------\n");
		printf("1 - List\n");
		printf("2 - Add\n");
		printf("3 - Find\n");
		printf("4 - Edit\n");
		printf("5 - Delete\n");
		printf("0 - Quit\n");
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
				list();
				break;
			case 2:
				add();
				break;
			case 3:
				find();
				break;
			case 4:
				edit();
				break;
			case 5:
				delete();
				break;
		}
	}
	return 0;
}

void list()
{
	printf("    Title                ID                   Password             Misc\n");

	for (int i = 0; i < entryCount; ++i)
		printf("%2d: %-*s%-*s%-*s%s\n", i, MAXTITLE, entries[i].title,
										  MAXID, entries[i].id,
										  MAXPW, entries[i].pw,
										  entries[i].misc);
}

void add()
{
	char line[MAXLINE];
	int line_ctr;
	int data_ctr;

	printf("Enter title up to %d chars: ", MAXTITLE - 1);
	if (fgets(line, MAXLINE, stdin) == NULL)
	{
		printf("No line\n");
		return;
	}

	if (line[0] == '\n')
		return;

	entries = realloc(entries, (entryCount+1) * sizeof *entries);

	line_ctr = 0;
	data_ctr = 0;
	while (line[line_ctr] != '\n' && line_ctr < MAXTITLE)
	{
		entries[entryCount].title[data_ctr++] = line[line_ctr++];
	}
	entries[entryCount].title[data_ctr] = '\0';

	printf("Enter ID up to %d chars: ", MAXID - 1);
	if (fgets(line, MAXLINE, stdin) == NULL)
	{
		printf("No line\n");
		return;
	}

	line_ctr = 0;
	data_ctr = 0;
	while (line[line_ctr] != '\n' && line_ctr < MAXID)
	{
		entries[entryCount].id[data_ctr++] = line[line_ctr++];
	}
	entries[entryCount].id[data_ctr] = '\0';

	printf("Enter password up to %d chars: (enter 'x' to generate random password)\n", MAXPW - 1);
	if (fgets(line, MAXLINE, stdin) == NULL)
	{
		printf("No line\n");
		return;
	}

	line_ctr = 0;
	data_ctr = 0;

	if (strcmp(line, "x\n") == 0)
	{
		generatePassword(entries[entryCount].pw);
	}
	else
	{
		while (line[line_ctr] != '\n' && line_ctr < MAXPW)
		{
			entries[entryCount].pw[data_ctr++] = line[line_ctr++];
		}
		entries[entryCount].pw[data_ctr] = '\0';
	}

	printf("Enter misc up to %d chars:\n", MAXMISC - 1);
	if (fgets(line, MAXLINE, stdin) == NULL)
	{
		printf("No line\n");
		return;
	}

	line_ctr = 0;
	data_ctr = 0;
	while (line[line_ctr] != '\n' && line_ctr < MAXMISC)
	{
		entries[entryCount].misc[data_ctr++] = line[line_ctr++];
	}
	entries[entryCount].misc[data_ctr] = '\0';

	++entryCount;
	saveEntries();
}

void find()
{
	char title[MAXTITLE];

	printf("Enter title to find: ");
	scanf("%s", title);
	getchar();
	//TODO catch errors

	for (int i = 0; i < entryCount; ++i)
	{
		if (strcmp(entries[i].title, title) == 0)
		{
			printf("\n    Title                ID                   Password             Misc\n");
			printf("%2d: %-*s%-*s%-*s%s\n", i, MAXTITLE, entries[i].title,
											MAXID, entries[i].id,
											MAXPW, entries[i].pw,
											entries[i].misc);
			return;
		}
	}
	printf("\nNot found\n");
}

void edit()
{
	int choice;

	printf("Enter # to edit: ");
	scanf("%d", &choice);
	getchar();
	//TODO catch errors

	if (choice >= 0 && choice < entryCount)
	{
		char line[MAXLINE];
		int line_ctr;
		int data_ctr;

		printf("\n    Title                ID                   Password             Misc\n");
		printf("%2d: %-*s%-*s%-*s%s\n", choice, MAXTITLE, entries[choice].title,
										MAXID, entries[choice].id,
										MAXPW, entries[choice].pw,
										entries[choice].misc);
		printf("\nEnter new title up to %d chars: ", MAXTITLE - 1);
		if (fgets(line, MAXLINE, stdin) == NULL)
		{
			printf("No line\n");
			return;
		}

		if (line[0] == '\n')
			return;

		line_ctr = 0;
		data_ctr = 0;
		while (line[line_ctr] != '\n' && line_ctr < MAXTITLE)
		{
			entries[choice].title[data_ctr++] = line[line_ctr++];
		}
		entries[choice].title[data_ctr] = '\0';

		printf("Enter new ID up to %d chars: ", MAXID - 1);
		if (fgets(line, MAXLINE, stdin) == NULL)
		{
			printf("No line\n");
			return;
		}

		line_ctr = 0;
		data_ctr = 0;
		while (line[line_ctr] != '\n' && line_ctr < MAXID)
		{
			entries[choice].id[data_ctr++] = line[line_ctr++];
		}
		entries[choice].id[data_ctr] = '\0';

		printf("Enter new password up to %d chars: (enter 'x' to generate random password)\n", MAXPW - 1);
		if (fgets(line, MAXLINE, stdin) == NULL)
		{
			printf("No line\n");
			return;
		}

		line_ctr = 0;
		data_ctr = 0;

		if (strcmp(line, "x\n") == 0)
		{
			generatePassword(entries[choice].pw);
		}
		else
		{
			while (line[line_ctr] != '\n' && line_ctr < MAXPW)
			{
				entries[choice].pw[data_ctr++] = line[line_ctr++];
			}
			entries[choice].pw[data_ctr] = '\0';
		}

		printf("Enter new misc up to %d chars:\n", MAXMISC - 1);
		if (fgets(line, MAXLINE, stdin) == NULL)
		{
			printf("No line\n");
			return;
		}

		line_ctr = 0;
		data_ctr = 0;
		while (line[line_ctr] != '\n' && line_ctr < MAXMISC)
		{
			entries[choice].misc[data_ctr++] = line[line_ctr++];
		}
		entries[choice].misc[data_ctr] = '\0';

		saveEntries();
	}
}

void delete()
{
	int choice;

	printf("Enter # to delete: ");
	scanf("%d", &choice);
	getchar();
	//TODO catch errors

	if (choice >= 0 && choice < entryCount)
	{
		entries[choice].title[0] = '\0';
		saveEntries();
		readEntries();
	}
}

void readEntries()
{
	FILE *f = fopen("trove.db", "r");
	if (f == NULL)
	{
		return;
	}

	char line[MAXLINE];
	char data[MAXLINE];
	entryCount = 0;
	entries = NULL;

	while (!feof(f))
	{
		if (fgets(line, MAXLINE, f) != NULL)
		{
			entries = realloc(entries, (entryCount+1) * sizeof *entries);
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
	FILE *f = fopen("trove.db", "w");
	if (f == NULL)
	{
		printf("Error saving entries!\n");
		exit(1);
	}

	for (int i = 0; i < entryCount; ++i)
	{
		if (entries[i].title[0] != '\0') // skip deleted entries
			fprintf(f, "%s,%s,%s,%s\n", entries[i].title,
										entries[i].id,
										entries[i].pw,
										entries[i].misc);
	}
	fclose(f);
}

void readSettings()
{
}

// random chars from 33 to 126, not 44 (commas)
void generatePassword(char *buf)
{
	int rn;

	for (int i = 0; i < MAXPW - 1; ++i)
	{
		rn = rand() % 127;
		if (rn < 33 || rn == 44)
		{
			--i;
			continue;
		}

		buf[i] = (char)rn;
	}

	buf[MAXPW] = '\0';
}
