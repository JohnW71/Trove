#define _CRT_SECURE_NO_WARNINGS

#include "gui.h"
#include "shared.h"
#include "aes.h"

bool noDatabase = false;

extern bool debugging;
extern bool readVerified;
extern int entryCount;
extern uint8_t DBpassword[];
extern uint8_t iv[IV_SIZE];

static char dbFile[] = "gui.db"; // "trove.db";
static char *buffer;
static char *paddedBuffer;
static int bufferSize = 0;
static int paddedSize = 0;

void readEntries(void)
{
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

	if (!buffer)
	{
		outs("Memory allocation error for buffer");
		MessageBox(NULL, "Memory allocation error for buffer", "Error", MB_ICONEXCLAMATION | MB_OK);
		fclose(f);
		return;
	}
	buffer[0] = 0;
	bufferSize = sizeof(char) * fileSize;

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

	if (strcmp(header, "Trove") != 0)
	{
		outs("Invalid password entered");
		readVerified = false;
		return;
	}
	readVerified = true;

	tokens = strtok(NULL, ",\n");
	while (tokens != NULL)
	{
		struct Entry *temp = realloc(entries, (entryCount + 1) * sizeof(*entries));
		if (temp == NULL)
		{
			outs("Failure reallocating memory for new entry");
			MessageBox(NULL, "Failed reallocating memory for new entry", "Error", MB_ICONEXCLAMATION | MB_OK);
			return;
		}
		entries = temp;

		strcpy(entries[entryCount].title, tokens);
		tokens = strtok(NULL, ",\n");
		strcpy(entries[entryCount].id, tokens);
		tokens = strtok(NULL, ",\n");
		strcpy(entries[entryCount].pw, tokens);
		tokens = strtok(NULL, ",\n");
		strcpy(entries[entryCount].misc, tokens);
		tokens = strtok(NULL, ",\n");

//if (debugging)
//{
//	char row[120];
//	snprintf(row, 120, "%s,%s,%s,%s\n",
//				entries[entryCount].title,
//				entries[entryCount].id,
//				entries[entryCount].pw,
//				entries[entryCount].misc);
//	outs("row loaded in=");
//	outs(row);
//}
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
	if (entryCount > 0)
		buffer = (char *)malloc(entryCount * maxRowSize);
	else
		buffer = (char *)malloc(16);

	if (!buffer)
	{
		outs("Failure allocating memory for update buffer");
		MessageBox(NULL, "Failure allocating memory for update buffer", "Error", MB_ICONEXCLAMATION | MB_OK);
		return;
	}
	buffer[0] = 0;

	char *row = (char *)malloc(sizeof(char) * maxRowSize);
	if (!row)
	{
		outs("Failure allocating memory for update buffer row");
		MessageBox(NULL, "Failure allocating memory for update buffer row", "Error", MB_ICONEXCLAMATION | MB_OK);
		return;
	}
	row[0] = 0;

	strcat(buffer, "Trove\n");

	for (int i = 0; i < entryCount; ++i)
	{
		snprintf(row, maxRowSize, "%s,%s,%s,%s\n", entries[i].title,
													entries[i].id,
													entries[i].pw,
													entries[i].misc);
//if (debugging)
//{
//	outs("row to save to buffer=");
//	outs(row);
//}
		strcat(buffer, row);
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

	if (!paddedBuffer)
	{
		outs("Failure allocating memory for add padding");
		MessageBox(NULL, "Failure allocating memory for add padding", "Error", MB_ICONEXCLAMATION | MB_OK);
		return;
	}
	paddedBuffer[0] = 0;

	strcpy(paddedBuffer, text);

	for (int i = currentSize; i < paddedSize; ++i)
		paddedBuffer[i] = '\0';
}

void encrypt_cbc(uint8_t *text, uint8_t *init)
{
	if (debugging)
		outs("encrypt()");

//if (debugging)
//{
//	outs("buffer to encrypt=");
//	outs(text);
//	outs("iv to encrypt=");
//	outs(init);
//}
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
