#define _CRT_SECURE_NO_WARNINGS

#include "trove.h"
#include "aes.h"

#define IV_SIZE 16
#define KEY_SIZE 32

void write_default_file();
void write_file();
void read_file();
void encrypt_cbc(char *, char *, char *);
void decrypt_cbc(char *, char *, char *);
void add_padding(char *);
void load_encrypted_entries();

char *buffer;
char *paddedBuffer;
int bufferSize;
int paddedSize;

void encrypt()
{
	uint8_t iv[IV_SIZE] = "4981353546876832";
	uint8_t pw[KEY_SIZE] = "passwordpasswordpasswordpassword";

	write_default_file();

	read_file();
	printf("\nDefault data (%zd):\n%s\n", strlen(buffer), buffer);

	add_padding(buffer);
	printf("PaddedBuffer (%d):\n%s\n", paddedSize, paddedBuffer);

	encrypt_cbc(paddedBuffer, pw, iv);
	write_file();
//////////////////////////
	read_file();
	decrypt_cbc(buffer, pw, iv);
	printf("Decrypted data (%zd):\n%s\n", strlen(buffer), buffer);

	load_encrypted_entries();
}

void write_default_file()
{
	FILE *f;
	f = fopen("file.bin", "wb");

	if (strlen(DBpassword) > 0)
		fprintf(f, "%s", DBpassword);
	else
		fprintf(f, ".\n");

	for (int i = 0; i < entryCount; ++i)
		if (entries[i].title[0] != '\0') // skip deleted entries
			fprintf(f, "%s,%s,%s,%s\n", entries[i].title,
										entries[i].id,
										entries[i].pw,
										entries[i].misc);
	fclose(f);
}

void write_file()
{
	FILE *f;
	f = fopen("file.bin", "wb");
	fwrite(paddedBuffer, paddedSize, 1, f);
	fclose(f);
}

void read_file()
{
	FILE *f;
	f = fopen("file.bin", "rb");
	if (f == NULL)
	{
		puts("File error");
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
	fclose(f);
}

void add_padding(char *text)
{
	int currentSize = (int)strlen(text);
	paddedSize = currentSize + (16 - (currentSize % 16));

	paddedBuffer = NULL;
	paddedBuffer = (char *)malloc(sizeof(char) * paddedSize);
	strcpy(paddedBuffer, text);

	for (int i = currentSize; i < paddedSize; ++i)
		paddedBuffer[i] = '\0';
}

void encrypt_cbc(char *text, char *pw, char *init)
{
	struct AES_ctx ctx;
	AES_init_ctx_iv(&ctx, pw, init);
	AES_CBC_encrypt_buffer(&ctx, text, paddedSize);
}

void decrypt_cbc(char *text, char *pw, char *init)
{
	struct AES_ctx ctx;
	AES_init_ctx_iv(&ctx, pw, init);
	AES_CBC_decrypt_buffer(&ctx, text, bufferSize);
}

void load_encrypted_entries()
{
	Entry *ee = NULL;
	int eeCount = 0;
	int firstRow = 1;
	char *password;
	char *tokens;

	tokens = strtok(buffer, ",\n");

	puts("Loaded tokens:");

	while(tokens != NULL)
	{
		if (firstRow)
		{
			password = tokens;
			printf("Password: %s", password);
			tokens = strtok(NULL, ",\n");
			firstRow = 0;
		}

		++eeCount;
		ee = realloc(ee, (eeCount + 1) * sizeof(*ee));

		strcpy(ee[eeCount].title, tokens);
		printf("\ntitle: %s, ", tokens);
		tokens = strtok(NULL, ",\n");

		strcpy(ee[eeCount].id, tokens);
		printf("id: %s, ", tokens);
		tokens = strtok(NULL, ",\n");

		strcpy(ee[eeCount].pw, tokens);
		printf("pw: %s, ", tokens);
		tokens = strtok(NULL, ",\n");

		strcpy(ee[eeCount].misc, tokens);
		printf("misc: %s, ", tokens);
		tokens = strtok(NULL, ",\n");
	}

	printf("\neeCount: %d\n", eeCount);
}
