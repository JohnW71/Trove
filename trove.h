#ifndef TROVE_H
#define TROVE_H
#define MAXTITLE 21
#define MAXMISC 51
#define MAXLINE 80
#define MAXID 21
#define MAXPW 21
#define MINPW 6
#define MINCHARS 0
#define MAXCHARS 6
#define MAXNAME 10

#include <stdio.h>
#include <stdlib.h>	// exit(), rand(), malloc()
#include <string.h>	// strcmp(), strcpy(), strlen()
#include <time.h>	// time()
#include <ctype.h>	// isupper(), ispunct(), isdigit()
#include <stdint.h>	// uint8_t

void list();
void add();
void find();
void edit();
void delete();
void clipboard();
void readEntries();
void saveEntries();
void generatePassword(char *);
void setDBpassword();
// void removeDBpassword();
void readSettings();
void writeSettings();
void updateSettings();
void setPasswordSize();
void setMinSpecial();
void setMinNumeric();
void setMinUppercase();
// void encrypt();
// void write_default_file();
void writeFile();
void readFile();
void encrypt_cbc(char *, char *);
void decrypt_cbc(char *, char *);
void addPadding(char *);
void loadEncryptedEntries();
// void listEncrypted();
void updateBuffer();

struct Entry
{
	char title[MAXTITLE];
	char id[MAXID];
	char pw[MAXPW];
	char misc[MAXMISC];
} * entries;

int entryCount;
uint8_t DBpassword[32];
char dbFile[MAXNAME];

#endif
