#define _CRT_SECURE_NO_WARNINGS

#include "shared.h"
#include "ctype.h"

void sortEntries(void)
{
	bool changed;
	do
	{
		changed = false;

		for (int i = 0; i < state.entryCount - 1; ++i)
		{
			char t1[MAXTITLE];
			char t2[MAXTITLE];

			for (int j = 0; j < strlen(entries[i].title); ++j)
				t1[j] = (char)toupper(entries[i].title[j]);
			for (int j = 0; j < strlen(entries[i + 1].title); ++j)
				t2[j] = (char)toupper(entries[i + 1].title[j]);

			// if (strcmp(entries[i].title, entries[i + 1].title) > 0) // s1 > s2
			if (strcmp(t1, t2) > 0)
			{
				changed = true;
				struct Entry tmp;

				strcpy(tmp.title, entries[i].title);
				strcpy(tmp.id, entries[i].id);
				strcpy(tmp.pw, entries[i].pw);
				strcpy(tmp.misc, entries[i].misc);

				strcpy(entries[i].title, entries[i + 1].title);
				strcpy(entries[i].id, entries[i + 1].id);
				strcpy(entries[i].pw, entries[i + 1].pw);
				strcpy(entries[i].misc, entries[i + 1].misc);

				strcpy(entries[i + 1].title, tmp.title);
				strcpy(entries[i + 1].id, tmp.id);
				strcpy(entries[i + 1].pw, tmp.pw);
				strcpy(entries[i + 1].misc, tmp.misc);
			}
		}
	}
	while (changed);
}

void generatePassword(char *buf)
{
	int rpos;

	// clear password array first
	for (int i = 0; i < MAXPW; ++i)
		buf[i] = '\0';

	// add random special chars
	for (int specialCount = 0; specialCount < settings.minSpecial; ++specialCount)
	{
		int rn = rand() % 127;
		if (rn < 33 || rn == 44)
		{
			--specialCount;
			continue;
		}

		if (ispunct((char)rn))
		{
			do
			{
				rpos = rand() % settings.passwordSize;
			}
			while (buf[rpos] != '\0');

			buf[rpos] = (char)rn;
		}
		else
		{
			--specialCount;
			continue;
		}
	}

	// add random numeric chars
	for (int numericCount = 0; numericCount < settings.minNumeric; ++numericCount)
	{
		int rn = rand() % 127;
		if (rn < 33 || rn == 44)
		{
			--numericCount;
			continue;
		}

		if (isdigit((char)rn))
		{
			do
			{
				rpos = rand() % settings.passwordSize;
			}
			while (buf[rpos] != '\0');

			buf[rpos] = (char)rn;
		}
		else
		{
			--numericCount;
			continue;
		}
	}

	// add random uppercase chars
	for (int uppercaseCount = 0; uppercaseCount < settings.minUppercase; ++uppercaseCount)
	{
		int rn = rand() % 127;
		if (rn < 33 || rn == 44)
		{
			--uppercaseCount;
			continue;
		}

		if (isupper((char)rn))
		{
			do
			{
				rpos = rand() % settings.passwordSize;
			}
			while (buf[rpos] != '\0');

			buf[rpos] = (char)rn;
		}
		else
		{
			--uppercaseCount;
			continue;
		}
	}

	// fill in the remaining positions
	for (int i = 0; i < settings.passwordSize; ++i)
	{
		if (buf[i] == '\0')
		{
			int rn = rand() % 127;
			if (rn < 33 || rn == 44)
			{
				--i;
				continue;
			}
			buf[i] = (char)rn;
		}
	}
}

void generateKeygen(char *buf)
{
	for (int i = 0; i < 16; ++i)
	{
		int rn = rand() % 16;
		sprintf(buf + i, "%X", rn);
	}
}

void readSettings(char *iniFile)
{
	FILE *f = fopen(iniFile, "r");
	if (f == NULL)
	{
		settings.passwordSize = MINPW;
		writeSettings(iniFile);
		return;
	}

	settings.passwordSize = MINPW;
	settings.minSpecial = 0;
	settings.minNumeric = 0;
	settings.minUppercase = 0;
	settings.screenRow = 0;
	settings.screenCol = 0;
	char line[MAXLINE];

	while (fgets(line, MAXLINE, f) != NULL)
	{
		if (line[0] == '#')
			continue;

		char setting[MAXLINE];
		char value[MAXLINE];
		char *l = line;
		char *s = setting;
		char *v = value;

		for (int i = 0; i < MAXLINE; ++i)
		{
			setting[i] = '\0';
			value[i] = '\0';
		}

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

		if (strcmp(setting, "password_size") == 0)	settings.passwordSize = atoi(value);
		if (strcmp(setting, "min_special") == 0)	settings.minSpecial = atoi(value);
		if (strcmp(setting, "min_numeric") == 0)	settings.minNumeric = atoi(value);
		if (strcmp(setting, "min_uppercase") == 0)	settings.minUppercase = atoi(value);
		if (strcmp(setting, "window_row") == 0)		settings.screenRow = atoi(value);
		if (strcmp(setting, "window_col") == 0)		settings.screenCol = atoi(value);
		if (strcmp(setting, "keygen") == 0)
		{
			strncpy(settings.iv, value, 16);
			settings.iv[16] = '\0';
		}
	}

	if (strlen(settings.iv) < IV_SIZE - 1) // keygen is missing
		writeSettings(iniFile);

	fclose(f);
}

void removeCommas(char *text, int length)
{
	for (int i = 0; i < length; ++i)
		if (text[i] == ',')
			text[i] = ' ';
}
