#define _CRT_SECURE_NO_WARNINGS

#include <stdio.h>
#include <stdlib.h> // exit()
#include <string.h> // strcmp()

#define MAXENTRIES 100
#define MAXTITLE 20
#define MAXID 20
#define MAXPW 20
// #define MAXMISC 50

void list();
void add();
void find();
void edit();
void delete();
void readEntries();
void saveEntries();
void readSettings();

struct entry
{
	char title[MAXTITLE];
	char id[MAXID];
	char pw[MAXPW];
	//	char misc[MAXMISC];
} entries[MAXENTRIES];

int entryCount = 0;

int main()
{
	readSettings();
	readEntries();

	int choice = -1;

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

		scanf("%d", &choice);
		printf("\n");

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
	printf("   Title               ID                  Password            Misc\n");

	for (int i = 0; i < entryCount; ++i)
		printf("%d: %-*s%-*s%-*s\n", i, MAXTITLE, entries[i].title,
										MAXID, entries[i].id,
										MAXPW, entries[i].pw);
}

void add()
{
	printf("Enter title ID password:\n");
	scanf("%s %s %s", entries[entryCount].title,
					  entries[entryCount].id,
					  entries[entryCount].pw);
	++entryCount;
	saveEntries();
}

void find()
{
	char title[MAXTITLE];

	printf("Enter title to find:\n");
	scanf("%s", title);

	for (int i = 0; i < entryCount; ++i)
	{
		if (strcmp(entries[i].title, title) == 0)
		{
			printf("\n   Title               ID                  Password            Misc\n");
			printf("%d: %-*s%-*s%-*s\n", i, MAXTITLE, entries[i].title,
											MAXID, entries[i].id,
											MAXPW, entries[i].pw);
			return;
		}
	}

	printf("\nNot found\n");
}

void edit()
{
	int choice;

	printf("Enter # to edit:\n");
	scanf("%d", &choice);

	if (choice >= 0 && choice < entryCount)
	{
		printf("Enter updated title ID password:\n");
		scanf("%s %s %s", entries[choice].title,
						  entries[choice].id,
						  entries[choice].pw);
		saveEntries();
	}
}

void delete()
{
	int choice;

	printf("Enter # to delete:\n");
	scanf("%d", &choice);

	if (choice >= 0 && choice < entryCount)
	{
		entries[choice].title[0] = '\0';
		saveEntries();
		readEntries();
	}
}

void readEntries()
{
	entryCount = 0;

	FILE *f = fopen("trove.txt", "r");
	if (f == NULL)
	{
		printf("\nNo existing entries found\n");
		return;
	}

	while (!feof(f))
	{
		fscanf(f, "%s %s %s\n", entries[entryCount].title,
								entries[entryCount].id,
								entries[entryCount].pw);
		++entryCount;
	}

	fclose(f);
}

void saveEntries()
{
	FILE *f = fopen("trove.txt", "w");
	if (f == NULL)
	{
		printf("Error saving entries!\n");
		exit(1);
	}

	for (int i = 0; i < entryCount; ++i)
	{
		if (entries[i].title[0] != '\0') // skip deleted entries
			fprintf(f, "%s %s %s\n", entries[i].title,
									 entries[i].id,
									 entries[i].pw);
	}

	fclose(f);
}

void readSettings()
{
}
