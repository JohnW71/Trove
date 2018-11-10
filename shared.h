#pragma once

#include <stdio.h>
#include <stdlib.h>	// exit(), rand(), malloc()
#include <string.h>	// strcmp(), strcpy(), strlen()
#include <time.h>	// time()
#include <ctype.h>	// isupper(), ispunct(), isdigit()
#include <stdint.h>	// uint8_t
#include <stdbool.h>

#define MAXTITLE 21
#define MAXID 21
#define MAXPW 21
#define MAXMISC 51
#define MAXLINE 120
#define MINPW 6
#define MINCHARS 0
#define MAXCHARS 6
#define DBPASSWORDSIZE 32
#define IV_SIZE 17

void readEntries(void);
void saveEntries(void);
void sortEntries(void);
void generatePassword(char *);
void generateKeygen(char *);
bool setDBpassword(void);
void readSettings(void);
void writeSettings(void);
void writeFile(void);
void readFile(void);
void encrypt_cbc(uint8_t *, uint8_t *);
void decrypt_cbc(uint8_t *, uint8_t *);
void addPadding(char *);
void loadEncryptedEntries(void);
void updateBuffer(void);

struct Entry
{
	char title[MAXTITLE];
	char id[MAXID];
	char pw[MAXPW];
	char misc[MAXMISC];
} *entries;
