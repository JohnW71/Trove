#define _CRT_SECURE_NO_WARNINGS

#include "trove.h"
#include "shared.h"
#include "aes.h"

static char *buffer;
static char *paddedBuffer;

void readEntries(void)
{
	// read encrypted data into buffer, if DB exists
	readFile(DB_FILE);

	if (state.noDatabase)
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
		getDBpassword(state.DBpassword);
	}

#ifdef _WIN32
	puts("");
#endif

	if (state.noDatabase)
		readFile(DB_FILE);

	// decrypt buffer into buffer
	decrypt_cbc(buffer, settings.iv);

	// load data from buffer and split into entries
	loadEncryptedEntries();
}

void readFile(char *dbFile)
{
	FILE *f;
	f = fopen(dbFile, "rb");
	if (f == NULL)
	{
		state.noDatabase = true;
		return;
	}

	fseek(f, 0, SEEK_END);
	long fileSize = ftell(f);
	rewind(f);

	buffer = NULL;
	buffer = (char *)malloc(sizeof(char) * fileSize);

	if (!buffer)
	{
		puts("Memory allocation error for buffer");
		fclose(f);
		return;
	}
	buffer[0] = 0;
	state.bufferSize = sizeof(char) * fileSize;

	long result = (long)fread(buffer, 1, fileSize, f);
	if (result != fileSize)
	{
		puts("Error reading database into buffer");
		fclose(f);
		return;
	}

	fclose(f);
}

void decrypt_cbc(uint8_t *text, uint8_t *init)
{
	struct AES_ctx ctx;
	AES_init_ctx_iv(&ctx, state.DBpassword, init);
	AES_CBC_decrypt_buffer(&ctx, text, state.bufferSize);
}

void loadEncryptedEntries(void)
{
	// test for header to confirm password was valid
	char header[] = "Trove\n";

	for (int i = 0; i < 6; ++i)
		if (buffer[i] != header[i])
		{
			puts("\nWrong password...");
#ifndef _WIN32
			puts("");
#endif
			free(buffer);
			exit(1);
		}

	state.entryCount = 0;
	entries = NULL;
	char *tokens;
	tokens = strtok(buffer, ",\n");
	tokens = strtok(NULL, ",\n");
	while(tokens != NULL)
	{
		struct Entry *temp = realloc(entries, ((uint64_t)state.entryCount + 1) * sizeof(*entries));
		if (temp == NULL)
		{
			puts("Failure reallocating memory for new entry");
			return;
		}
		entries = temp;

		strcpy(entries[state.entryCount].title, tokens);
		tokens = strtok(NULL, ",\n");
		strcpy(entries[state.entryCount].id, tokens);
		tokens = strtok(NULL, ",\n");
		strcpy(entries[state.entryCount].pw, tokens);
		tokens = strtok(NULL, ",\n");
		strcpy(entries[state.entryCount].misc, tokens);
		tokens = strtok(NULL, ",\n");

		++state.entryCount;
	}

	free(buffer);
}

void saveEntries(void)
{
	// replace buffer with current entries list
	updateBuffer();

	// add padding to buffer into paddedBuffer
	addPadding(buffer);

	// encrypt paddedBuffer
	encrypt_cbc(paddedBuffer, settings.iv);

	// save paddedBuffer to dbFile
	writeFile(DB_FILE);
}

void updateBuffer(void)
{
	buffer = NULL;
	if (state.entryCount > 0)
		buffer = (char *)malloc(state.entryCount * MAXLINE);
	else
		buffer = (char *)malloc(16);

	if (!buffer)
	{
		puts("Failure allocating memory for update buffer");
		return;
	}
	buffer[0] = 0;

	char *row = (char *)malloc(sizeof(char) * MAXLINE);
	if (!row)
	{
		puts("Failure allocating memory for update buffer row");
		return;
	}
	row[0] = 0;

	strcat(buffer, "Trove\n");

	for (int i = 0; i < state.entryCount; ++i)
	{
		snprintf(row, MAXLINE, "%s,%s,%s,%s\n", entries[i].title,
												entries[i].id,
												entries[i].pw,
												entries[i].misc);
		strcat(buffer, row);
	}
	free(row);
}

void addPadding(char *text)
{
	int currentSize = (int)strlen(text);
	state.paddedSize = currentSize + (16 - (currentSize % 16));
	paddedBuffer = NULL;
	paddedBuffer = (char *)malloc(sizeof(char) * state.paddedSize);

	if (!paddedBuffer)
	{
		puts("Failure allocating memory for add padding");
		return;
	}
	paddedBuffer[0] = 0;

	strcpy(paddedBuffer, text);

	for (int i = currentSize; i < state.paddedSize; ++i)
		paddedBuffer[i] = '\0';
}

void encrypt_cbc(uint8_t *text, uint8_t *init)
{
	struct AES_ctx ctx;
	AES_init_ctx_iv(&ctx, state.DBpassword, init);
	AES_CBC_encrypt_buffer(&ctx, text, state.paddedSize);
}

void writeFile(char *dbFile)
{
	FILE *f;
	f = fopen(dbFile, "wb");
	fwrite(paddedBuffer, state.paddedSize, 1, f);
	fclose(f);
	free(paddedBuffer);
	free(buffer);
}
