#define _CRT_SECURE_NO_WARNINGS

#include "gui.h"
#include "shared.h"
#include "aes.h"

static char *buffer;
static char *paddedBuffer;

void readEntries(void)
{
	if (state.noDatabase)
		readFile(DB_FILE);

	// decrypt buffer into buffer
	decrypt_cbc(buffer, settings.iv);

	// load data from buffer and split into entries
	loadEncryptedEntries();
}

void readFile(char *dbFile)
{
	if (state.debugging)
		outs("readFile()");

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
		outs("Memory allocation error for buffer");
		MessageBox(NULL, "Memory allocation error for buffer", "Error", MB_ICONEXCLAMATION | MB_OK);
		fclose(f);
		return;
	}
	buffer[0] = 0;
	state.bufferSize = sizeof(char) * fileSize;

	long result = (long)fread(buffer, 1, fileSize, f);
	if (result != fileSize)
	{
		outs("Error reading db into buffer");
		fclose(f);
		return;
	}

	fclose(f);
}

void decrypt_cbc(uint8_t *text, uint8_t *init)
{
	if (state.debugging)
		outs("decrypt()");

	struct AES_ctx ctx;
	AES_init_ctx_iv(&ctx, state.DBpassword, init);
	AES_CBC_decrypt_buffer(&ctx, text, state.bufferSize);
}

void loadEncryptedEntries(void)
{
	if (state.debugging)
		outs("loadEncryptedEntries()");

	state.entryCount = 0;
	entries = NULL;
	char *tokens;
	tokens = strtok(buffer, ",\n");

	// test for header "Trove" to confirm password was valid
	char *header;
	header = tokens;

	if (strcmp(header, "Trove") != 0)
	{
		outs("Invalid password entered");
		state.readVerified = false;
		return;
	}
	state.readVerified = true;

	tokens = strtok(NULL, ",\n");
	while (tokens != NULL)
	{
		struct Entry *temp = realloc(entries, ((uint64_t)state.entryCount + 1) * sizeof(*entries));
		if (temp == NULL)
		{
			outs("Failure reallocating memory for new entry");
			MessageBox(NULL, "Failed reallocating memory for new entry", "Error", MB_ICONEXCLAMATION | MB_OK);
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

		if (state.debugging)
		{
			char row[120];
			snprintf(row, 120, "%s,%s,%s,%s\n",
						entries[state.entryCount].title,
						entries[state.entryCount].id,
						entries[state.entryCount].pw,
						entries[state.entryCount].misc);
			outs("row loaded in=");
			outs(row);
		}

		++state.entryCount;
	}

	free(buffer);
}

void saveEntries(void)
{
	if (state.debugging)
		outs("saveEntries()");

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
	if (state.debugging)
		outs("updateBuffer()");

	buffer = NULL;
	if (state.entryCount > 0)
		buffer = (char *)malloc(state.entryCount * MAXLINE);
	else
		buffer = (char *)malloc(16);

	if (!buffer)
	{
		outs("Failure allocating memory for update buffer");
		MessageBox(NULL, "Failure allocating memory for update buffer", "Error", MB_ICONEXCLAMATION | MB_OK);
		return;
	}
	buffer[0] = 0;

	char *row = (char *)malloc(sizeof(char) * MAXLINE);
	if (!row)
	{
		outs("Failure allocating memory for update buffer row");
		MessageBox(NULL, "Failure allocating memory for update buffer row", "Error", MB_ICONEXCLAMATION | MB_OK);
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
		if (state.debugging)
		{
			outs("row to save to buffer=");
			outs(row);
		}

		strcat(buffer, row);
	}

	free(row);
}

void addPadding(char *text)
{
	if (state.debugging)
		outs("addPadding()");

	int currentSize = (int)strlen(text);
	state.paddedSize = currentSize + (16 - (currentSize % 16));
	paddedBuffer = NULL;
	paddedBuffer = (char *)malloc(sizeof(char) * state.paddedSize);

	if (!paddedBuffer)
	{
		outs("Failure allocating memory for add padding");
		MessageBox(NULL, "Failure allocating memory for add padding", "Error", MB_ICONEXCLAMATION | MB_OK);
		return;
	}
	paddedBuffer[0] = 0;

	strcpy(paddedBuffer, text);

	for (int i = currentSize; i < state.paddedSize; ++i)
		paddedBuffer[i] = '\0';
}

void encrypt_cbc(uint8_t *text, uint8_t *init)
{
	if (state.debugging)
		outs("encrypt()");

	if (state.debugging)
	{
		outs("buffer to encrypt=");
		outs(text);
		outs("iv to encrypt=");
		outs(init);
	}

	struct AES_ctx ctx;
	AES_init_ctx_iv(&ctx, state.DBpassword, init);
	AES_CBC_encrypt_buffer(&ctx, text, state.paddedSize);
}

void writeFile(char *dbFile)
{
	if (state.debugging)
		outs("writeFile()");

	FILE *f;
	f = fopen(dbFile, "wb");
	fwrite(paddedBuffer, state.paddedSize, 1, f);
	fclose(f);
	free(paddedBuffer);
	free(buffer);
}
