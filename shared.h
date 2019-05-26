#pragma once

#include <stdio.h>
#include <stdlib.h>	// exit(), rand(), malloc()
#include <string.h>	// strcmp(), strcpy(), strlen()
#include <time.h>	// time()
#include <ctype.h>	// isupper(), ispunct(), isdigit()
#include <stdint.h>
#include <stdbool.h>

#ifdef _WIN32
	#include <windows.h>
#endif

#define MAXTITLE 31
#define MAXID 31
#define MAXPW 21
#define MAXMISC 51
#define MAXLINE 150
#define MINPW 6
#define MINCHARS 0
#define MAXCHARS 6
#define DBPASSWORDSIZE 32
#define IV_SIZE 17

#define INI_FILE "trove.ini"
#define DB_FILE "trove.db"

void readEntries(void);
void saveEntries(void);
void sortEntries(void);
void generatePassword(char *);
void generateKeygen(char *);
bool setDBpassword(void);
void readSettings(char *);
void writeSettings(char *);
void writeFile(char *);
void readFile(char *);
void encrypt_cbc(uint8_t *, uint8_t *);
void decrypt_cbc(uint8_t *, uint8_t *);
void addPadding(char *);
void loadEncryptedEntries(void);
void updateBuffer(void);
void removeCommas(char *, int);
void clearArray(char *, int);
void exportDB(void);

struct Entry
{
	char title[MAXTITLE];
	char id[MAXID];
	char pw[MAXPW];
	char misc[MAXMISC];
} *entries;

struct Settings
{
	int passwordSize;
	int minSpecial;
	int minNumeric;
	int minUppercase;
	int screenRow;
	int screenCol;
	char iv[IV_SIZE];
} settings;

struct State
{
	char DBpassword[DBPASSWORDSIZE];
	bool noDatabase;
	int entryCount;
	int bufferSize;
	int paddedSize;

#ifdef _WIN32
	bool running;
	bool debugging;
	bool readVerified;
	bool changingPassword;
	LRESULT selectedRow;
#endif
} state;
