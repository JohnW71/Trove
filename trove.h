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

#include <stdio.h>
#include <stdlib.h>	// exit(), rand(), malloc()
#include <string.h>	// strcmp(), strcpy(), strlen()
#include <time.h>	// time()
#include <ctype.h>	// isupper(), ispunct(), isdigit()

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
void removeDBpassword();
void readSettings();
void writeSettings();
void updateSettings();
void setPasswordSize();
void setMinSpecial();
void setMinNumeric();
void setMinUppercase();
void encrypt();

typedef struct
{
	char title[MAXTITLE];
	char id[MAXID];
	char pw[MAXPW];
	char misc[MAXMISC];
} Entry;

Entry *entries;

int entryCount;
char DBpassword[MAXPW];

#endif
