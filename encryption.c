#define _CRT_SECURE_NO_WARNINGS

//TODO this breaks when data rows are not the same length
//TODO is it a padding issue?

#include "trove.h"
#include "aes.h"

#define IV_SIZE 16
#define KEY_SIZE 32

char *buffer;
char *paddedBuffer;
int bufferSize = 0;
int paddedSize = 0;
uint8_t iv[IV_SIZE] = "498135354687683";
uint8_t pw[KEY_SIZE] = "passwordpasswordpasswordpassword";

void readEntries()
{
	puts("readEntries()");
	// char line[MAXLINE];
	// char data[MAXLINE];
	// char password[MAXLINE];

	// check for password
	// if (fgets(line, MAXPW, f) != NULL)
	// {
		// // find and convert CRLF endings
		// char *p = strrchr(line, '\r');
		// if (p && p[1] == '\n' && p[2] == '\0') {
		// 	p[0] = '\n';
		// 	p[1] = '\0';
		// }

		// if ((strcmp(line, ".\n") != 0) &&
		// 	(strcmp(line, DBpassword) != 0)) // DB has a password
		// {
			// printf("Enter password: ");

			// if (fgets(DBpassword, MAXPW, stdin) == NULL)
			// {
			// 	puts("Null!");
			// 	// fclose(f);
			// 	exit(1);
			// }

			// if (DBpassword[0] == '\n')
			// {
			// 	puts("Blank line entered!");
			// 	// fclose(f);
			// 	exit(1);
			// }

			// if (strcmp(password, line) != 0)
			// {
			// 	puts("Incorrect password entered!");
			// 	fclose(f);
			// 	exit(1);
			// }

			// strcpy(DBpassword, password);
	// 	}
	// }

	// read encrypted data into buffer
	readFile();

	//TODO don't ask if password was already entered
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

	// printf("pw length: %zd\n", strlen(DBpassword));
	DBpassword[strlen(DBpassword) - 1] = '\0';

	if (bufferSize == 0)
		return;

	// decrypt buffer into buffer
	decrypt_cbc(buffer, iv);
	printf("Decrypted data (%zd):\n%s\n", strlen(buffer), buffer);

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
printf("bufferSize:%d\n", bufferSize);
	fclose(f);
}

//TODO handle failure or wrong password here
void decrypt_cbc(char *text, char *init)
{
	puts("decrypt_cbc()");
printf("pw: %s\ninit: %s\n", DBpassword, init);
	struct AES_ctx ctx;
	AES_init_ctx_iv(&ctx, pw, init);
	AES_CBC_decrypt_buffer(&ctx, text, bufferSize);
}

//TODO does this need to get the password any more?
void loadEncryptedEntries()
{
	puts("loadEncryptedEntries()");

	// int firstRow = 1;
	// char *password;
	char *tokens;

	tokens = strtok(buffer, ",\n");

	puts("Loaded tokens:");

	while(tokens != NULL)
	{
		// if (firstRow)
		// {
		// 	password = tokens;
		// 	printf("Password: %s", password);
		// 	tokens = strtok(NULL, ",\n");
		// 	firstRow = 0;
		// }

		entries = realloc(entries, (entryCount + 1) * sizeof(*entries));

		strcpy(entries[entryCount].title, tokens);
		printf("\ntitle: %s, ", tokens);
		tokens = strtok(NULL, ",\n");

		strcpy(entries[entryCount].id, tokens);
		printf("id: %s, ", tokens);
		tokens = strtok(NULL, ",\n");

		strcpy(entries[entryCount].pw, tokens);
		printf("pw: %s, ", tokens);
		tokens = strtok(NULL, ",\n");

		strcpy(entries[entryCount].misc, tokens);
		printf("misc: %s, ", tokens);
		tokens = strtok(NULL, ",\n");

		++entryCount;
	}

	printf("\nentryCount: %d\n", entryCount);
}

void saveEntries()
{
	puts("saveEntries()");

	// replace buffer with current entries list
	updateBuffer();

	// add padding to buffer into paddedBuffer
	addPadding(buffer);
	printf("PaddedBuffer (%d):\n%s\n", paddedSize, paddedBuffer);

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
	char *row = (char *)malloc(sizeof(char) * maxRowSize);

	// if (strlen(DBpassword) > 0)
		// strcpy(buffer, DBpassword);
	// else
	// 	strcpy(buffer, ".\n");

	for (int i = 0; i < entryCount; ++i)
	{
		snprintf(row, maxRowSize, "%s,%s,%s,%s\n", entries[i].title,
													entries[i].id,
													entries[i].pw,
													entries[i].misc);
		strcat(buffer, row);
	}

	printf("\nNew buffer (%zd):\n%s\n", strlen(buffer), buffer);
}

void addPadding(char *text)
{
	puts("addPadding()");

	int currentSize = (int)strlen(text);
	paddedSize = currentSize + (16 - (currentSize % 16));

	paddedBuffer = NULL;
	paddedBuffer = (char *)malloc(sizeof(char) * paddedSize);
	strcpy(paddedBuffer, text);

	for (int i = currentSize; i < paddedSize; ++i)
		paddedBuffer[i] = '\0';
}

//TODO handle failure here
void encrypt_cbc(char *text, char *init)
{
	puts("encrypt_cbc()");
printf("pw: %s\ninit: %s\n", DBpassword, init);

	struct AES_ctx ctx;
	AES_init_ctx_iv(&ctx, pw, init);
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

// void encrypt()
// {
// 	uint8_t iv[IV_SIZE] = "4981353546876832";
// 	uint8_t pw[KEY_SIZE] = "passwordpasswordpasswordpassword";

//////////////////////////
// create encrypted data for testing

	// write_default_file();

	// read_file();
	// printf("\nDefault data (%zd):\n%s\n", strlen(buffer), buffer);

	// add_padding(buffer);
	// printf("PaddedBuffer (%d):\n%s\n", paddedSize, paddedBuffer);

	// encrypt_cbc(paddedBuffer, pw, iv);
	// write_file();

//////////////////////////
// load data as if from program start

	// // read encrypted data into buffer
	// read_file();

	// // decrypt buffer into buffer
	// decrypt_cbc(buffer, pw, iv);
	// printf("Decrypted data (%zd):\n%s\n", strlen(buffer), buffer);

	// // load data from buffer and split into entries
	// load_encrypted_entries();

	// // display full entries list
	// list_encrypted();

//////////////////////////
// save data to encrypted db

	// // replace buffer with current entries list
	// update_buffer();

	// // add padding to buffer into paddedBuffer
	// add_padding(buffer);
	// printf("PaddedBuffer (%d):\n%s\n", paddedSize, paddedBuffer);

	// // encrypt paddedBuffer
	// encrypt_cbc(paddedBuffer, pw, iv);

	// // save paddedBuffer to file.db
	// write_file();
// }

// void write_default_file()
// {
// 	FILE *f;
// 	f = fopen("file.bin", "wb");

// 	if (strlen(DBpassword) > 0)
// 		fprintf(f, "%s", DBpassword);
// 	else
// 		fprintf(f, ".\n");

// 	for (int i = 0; i < entryCount; ++i)
// 		if (entries[i].title[0] != '\0') // skip deleted entries
// 			fprintf(f, "%s,%s,%s,%s\n", entries[i].title,
// 										entries[i].id,
// 										entries[i].pw,
// 										entries[i].misc);
// 	fclose(f);
// }

// void list_encrypted()
// {
// 	puts("\nList encrypted entries loaded in:");
// 	puts("\n    Title                ID                   Password             Misc");
// 	for (int i = 0; i < eeCount; ++i)
// 		printf("%2d: %-*s%-*s%-*s%s\n", i + 1,	MAXTITLE, ee[i].title,
// 												MAXID, ee[i].id,
// 												MAXPW, ee[i].pw,
// 												ee[i].misc);
// }
