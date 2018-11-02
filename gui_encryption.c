#define _CRT_SECURE_NO_WARNINGS

#include "gui.h"
#include "shared.h"
#include "aes.h"

extern bool debugging;
extern int entryCount;
extern uint8_t DBpassword[];
extern uint8_t iv[IV_SIZE];

// static char dbFile[] = "trove.db";
static char dbFile[] = "gui.db";
static char *buffer;
static char *paddedBuffer;
static int bufferSize = 0;
static int paddedSize = 0;
static bool noDatabase = false;

void readEntries(void)
{
	if (debugging)
		outs("readEntries()");

	// read encrypted data into buffer, if DB exists
	readFile();

	if (noDatabase)
	{
if (debugging)
	outs("No DB found");

		if (!setDBpassword())
		{
			outs("set DB password failed");
			exit(1);
		}
if (debugging)
{
	outs("new password set to=");
	outs(DBpassword);
}
	}
	else
	{
if (debugging)
	outs("User entering password");

		getDBpassword(DBpassword);

if (debugging)
{
	outs("user password set to=");
	outs(DBpassword);
}
	}

	if (noDatabase)
		readFile();

	// decrypt buffer into buffer
	decrypt_cbc(buffer, iv);

	// load data from buffer and split into entries
	loadEncryptedEntries();
}

void readFile(void)
{
	if (debugging)
		outs("readFile()");

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
		outs("Memory alloc error for buffer");
		fclose(f);
		return;
	}

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
	if (debugging)
		outs("decrypt()");

	struct AES_ctx ctx;
	AES_init_ctx_iv(&ctx, DBpassword, init);
	AES_CBC_decrypt_buffer(&ctx, text, bufferSize);
}

void loadEncryptedEntries(void)
{
	if (debugging)
		outs("loadEncryptedEntries()");

	entryCount = 0;
	entries = NULL;
	char *tokens;
	tokens = strtok(buffer, ",\n");

	// test for header "Trove" to confirm password was valid
	char *header;
	header = tokens;

if (debugging)
{
	outs("header=");
	outs(header);
}

	if (strcmp(header, "Trove") != 0)
	{
		outs("Invalid password");
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

if (debugging)
{
	char row[120];
	snprintf(row, 120, "%s,%s,%s,%s\n",
				entries[entryCount].title,
				entries[entryCount].id,
				entries[entryCount].pw,
				entries[entryCount].misc);
	outs("row loaded in=");
	outs(row);
}
		++entryCount;
	}

	free(buffer);
}

void saveEntries(void)
{
	if (debugging)
		outs("saveEntries()");

	// replace buffer with current entries list
	updateBuffer();

	// add padding to buffer into paddedBuffer
	addPadding(buffer);

	// encrypt paddedBuffer
	encrypt_cbc(paddedBuffer, iv);

	// save paddedBuffer to dbFile
	writeFile();
}

void updateBuffer(void)
{
	if (debugging)
		outs("updateBuffer()");

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
if (debugging)
{
	outs("row to save to buffer=");
	outs(row);
}
			strcat(buffer, row);
		}
	}

	free(row);
}

void addPadding(char *text)
{
	if (debugging)
		outs("addPadding()");

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
	if (debugging)
		outs("encrypt()");

if (debugging)
{
	outs("buffer to encrypt=");
	outs(text);
	outs("iv to encrypt=");
	outs(init);
}
	struct AES_ctx ctx;
	AES_init_ctx_iv(&ctx, DBpassword, init);
	AES_CBC_encrypt_buffer(&ctx, text, paddedSize);
}

void writeFile(void)
{
	if (debugging)
		outs("writeFile()");

	FILE *f;
	f = fopen(dbFile, "wb");
	fwrite(paddedBuffer, paddedSize, 1, f);
	fclose(f);
	free(paddedBuffer);
	free(buffer);
}
