#define _CRT_SECURE_NO_WARNINGS
#define IV_SIZE 16

#include "trove.h"
#include "aes.h"

extern int entryCount;
extern uint8_t DBpassword[];

static char dbFile[MAXNAME] = "trove.db";
static char *buffer;
static char *paddedBuffer;
static int bufferSize = 0;
static int paddedSize = 0;
static uint8_t iv[IV_SIZE] = "498135354687683";
static bool noDatabase = false;

void readEntries()
{
	// read encrypted data into buffer, if DB exists
	readFile();

	if (noDatabase)
	{
		puts("\nNo database found...\n");

		if (!setDBpassword())
		{
			puts("");
			exit(1);
		}
	}
	else
	{
		printf("\nEnter database password: ");
		getPassword(DBpassword);
	}

#ifdef _WIN32
	puts("");
#endif

	if (noDatabase)
		readFile();

	// decrypt buffer into buffer
	decrypt_cbc(buffer, iv);

	// load data from buffer and split into entries
	loadEncryptedEntries();
}

void readFile()
{
	FILE *f;
	f = fopen(dbFile, "rb");
	if (f == NULL)
	{
		noDatabase = true;
		return;
	}

	fseek(f, 0, SEEK_END);
	long fileSize = ftell(f);
	rewind(f);

	buffer = NULL;
	buffer = (char *)malloc(sizeof(char) * fileSize);
	buffer[0] = 0;
	bufferSize = sizeof(char) * fileSize;

	if (buffer == NULL)
	{
		puts("Memory allocation error for buffer");
		return;
	}

	long result = fread(buffer, 1, fileSize, f);
	if (result != fileSize)
	{
		puts("Error reading database into buffer");
		return;
	}

	buffer[fileSize] = '\0';
	fclose(f);
}

void decrypt_cbc(uint8_t *text, uint8_t *init)
{
	struct AES_ctx ctx;
	AES_init_ctx_iv(&ctx, DBpassword, init);
	AES_CBC_decrypt_buffer(&ctx, text, bufferSize);
}

void loadEncryptedEntries()
{
	entryCount = 0;
	entries = NULL;
	char *tokens;
	tokens = strtok(buffer, ",\n");

	// test for header "Trove" to confirm password was valid
	char *header;
	header = tokens;
	if (strcmp(header, "Trove") != 0)
	{
		puts("\nWrong password...");
#ifndef _WIN32
		puts("");
#endif
		exit(1);
	}

	tokens = strtok(NULL, ",\n");
	while(tokens != NULL)
	{
		entries = realloc(entries, (entryCount + 1) * sizeof(*entries));

		strcpy(entries[entryCount].title, tokens);
		tokens = strtok(NULL, ",\n");

		strcpy(entries[entryCount].id, tokens);
		tokens = strtok(NULL, ",\n");

		strcpy(entries[entryCount].pw, tokens);
		tokens = strtok(NULL, ",\n");

		strcpy(entries[entryCount].misc, tokens);
		tokens = strtok(NULL, ",\n");

		++entryCount;
	}
}

void saveEntries()
{
	// replace buffer with current entries list
	updateBuffer();

	// add padding to buffer into paddedBuffer
	addPadding(buffer);

	// encrypt paddedBuffer
	encrypt_cbc(paddedBuffer, iv);

	// save paddedBuffer to dbFile
	writeFile();
}

void updateBuffer()
{
	int maxRowSize = MAXTITLE + MAXID + MAXPW + MAXMISC;
	buffer = NULL;
	buffer = (char *)malloc(entryCount * maxRowSize);
	buffer[0] = 0;
	char *row = (char *)malloc(sizeof(char) * maxRowSize);
	row[0] = 0;

	strcat(buffer, "Trove\n");

	for (int i = 0; i < entryCount; ++i)
	{
		if (entries[i].title[0] != '\0')
		{
			snprintf(row, maxRowSize, "%s,%s,%s,%s\n", entries[i].title,
														entries[i].id,
														entries[i].pw,
														entries[i].misc);
			strcat(buffer, row);
		}
	}
}

void addPadding(char *text)
{
	int currentSize = (int)strlen(text);
	paddedSize = currentSize + (16 - (currentSize % 16));
	paddedBuffer = NULL;
	paddedBuffer = (char *)malloc(sizeof(char) * paddedSize);
	paddedBuffer[0] = 0;
	strcpy(paddedBuffer, text);

	for (int i = currentSize; i < paddedSize; ++i)
		paddedBuffer[i] = '\0';
}

void encrypt_cbc(uint8_t *text, uint8_t *init)
{
	struct AES_ctx ctx;
	AES_init_ctx_iv(&ctx, DBpassword, init);
	AES_CBC_encrypt_buffer(&ctx, text, paddedSize);
}

void writeFile()
{
	FILE *f;
	f = fopen(dbFile, "wb");
	fwrite(paddedBuffer, paddedSize, 1, f);
	fclose(f);
}
