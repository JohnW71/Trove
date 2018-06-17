#define _CRT_SECURE_NO_WARNINGS

#include "trove.h"
#include "aes.h"

#define IV_SIZE 16

char *buffer;
char *paddedBuffer;
int bufferSize = 0;
int paddedSize = 0;
uint8_t iv[IV_SIZE] = "498135354687683";

void readEntries()
{
	puts("readEntries()");

	// read encrypted data into buffer
	readFile();

	if (strlen(DBpassword) == 0)
	{
		if (bufferSize == 0)
			printf("\nDatabase empty or missing!\nEnter new password: ");
		else
			printf("\nEnter password: ");

		if (fgets(DBpassword, MAXPW, stdin) == NULL)
		{
			puts("Null!");
			exit(1);
		}
		if (DBpassword[0] == '\n')
		{
			puts("Blank line entered!");
			exit(1);
		}

		DBpassword[strlen(DBpassword) - 1] = '\0';
	}

	if (bufferSize == 0)
		return;

	// decrypt buffer into buffer
	decrypt_cbc(buffer, iv);

	// load data from buffer and split into entries
	loadEncryptedEntries();
}

void readFile()
{
	puts("readFile()");

	FILE *f;
	f = fopen(dbFile, "rb");
	if (f == NULL)
	{
		puts("File open error/database not found");
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
		puts("Memory error");
		return;
	}

	size_t result = fread(buffer, 1, fileSize, f);
	if (result != fileSize)
	{
		puts("Reading error");
		return;
	}

	buffer[fileSize] = '\0';
	fclose(f);
}

void decrypt_cbc(char *text, char *init)
{
	puts("decrypt_cbc()");

	struct AES_ctx ctx;
	AES_init_ctx_iv(&ctx, DBpassword, init);
	AES_CBC_decrypt_buffer(&ctx, text, bufferSize);
}

void loadEncryptedEntries()
{
	puts("loadEncryptedEntries()");

	entryCount = 0;
	entries = NULL;
	char *tokens;
	tokens = strtok(buffer, ",\n");

	// test for header "Trove" to confirm password was valid
	char *header;
	header = tokens;
	if (strcmp(header, "Trove") != 0)
	{
		puts("\nWrong password!");
		exit(1);
	}

	tokens = strtok(NULL, ",\n");
	while(tokens != NULL)
	{
		entries = realloc(entries, (entryCount + 1) * sizeof(*entries));

		strcpy(entries[entryCount].title, tokens);
		// printf("\ntitle: %s, ", tokens);
		tokens = strtok(NULL, ",\n");

		strcpy(entries[entryCount].id, tokens);
		// printf("id: %s, ", tokens);
		tokens = strtok(NULL, ",\n");

		strcpy(entries[entryCount].pw, tokens);
		// printf("pw: %s, ", tokens);
		tokens = strtok(NULL, ",\n");

		strcpy(entries[entryCount].misc, tokens);
		// printf("misc: %s ", tokens);
		tokens = strtok(NULL, ",\n");

		++entryCount;
	}

	// printf("\nentryCount: %d\n", entryCount);
}

void saveEntries()
{
	puts("saveEntries()");

	// replace buffer with current entries list
	updateBuffer();

	// add padding to buffer into paddedBuffer
	addPadding(buffer);
	// printf("PaddedBuffer (%d):\n%s\n", paddedSize, paddedBuffer);

	// encrypt paddedBuffer
	encrypt_cbc(paddedBuffer, iv);

	// save paddedBuffer to dbFile
	writeFile();
}

void updateBuffer()
{
	puts("updateBuffer()");

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

	// printf("\nNew buffer (%zd):\n%s\n", strlen(buffer), buffer);
}

void addPadding(char *text)
{
	puts("addPadding()");

	int currentSize = (int)strlen(text);
	paddedSize = currentSize + (16 - (currentSize % 16));

	paddedBuffer = NULL;
	paddedBuffer = (char *)malloc(sizeof(char) * paddedSize);
	paddedBuffer[0] = 0;
	strcpy(paddedBuffer, text);

	for (int i = currentSize; i < paddedSize; ++i)
		paddedBuffer[i] = '\0';
}

void encrypt_cbc(char *text, char *init)
{
	puts("encrypt_cbc()");

	struct AES_ctx ctx;
	AES_init_ctx_iv(&ctx, DBpassword, init);
	AES_CBC_encrypt_buffer(&ctx, text, paddedSize);
}

void writeFile()
{
	puts("writeFile()");

	FILE *f;
	f = fopen(dbFile, "wb");
	fwrite(paddedBuffer, paddedSize, 1, f);
	fclose(f);
}
