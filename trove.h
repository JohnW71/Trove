#define MAXTITLE 21
#define MAXMISC 51
#define MAXLINE 80
#define MAXID 21
#define MAXPW 21
#define MINPW 6
#define MINCHARS 0
#define MAXCHARS 6
#define MAXNAME 10
#define KEYSIZE 32

#include <stdio.h>
#include <stdlib.h>	// exit(), rand(), malloc()
#include <string.h>	// strcmp(), strcpy(), strlen()
#include <time.h>	// time()
#include <ctype.h>	// isupper(), ispunct(), isdigit()
#include <stdint.h>	// uint8_t
#include <stdbool.h>

void list();
void add();
void find();
void edit();
void delEntry();
void clipboard();
void readEntries();
void saveEntries();
void generatePassword(char *);
bool setDBpassword();
void readSettings();
void writeSettings();
void updateSettings();
void setPasswordSize();
void setMinSpecial();
void setMinNumeric();
void setMinUppercase();
void writeFile();
void readFile();
void encrypt_cbc(uint8_t *, uint8_t *);
void decrypt_cbc(uint8_t *, uint8_t *);
void addPadding(char *);
void loadEncryptedEntries();
void updateBuffer();
void getPassword(uint8_t *);
#ifdef _WIN32
void getPasswordWindows(uint8_t *);
#else
void getPasswordLinux(uint8_t *);
#endif

struct Entry
{
	char title[MAXTITLE];
	char id[MAXID];
	char pw[MAXPW];
	char misc[MAXMISC];
} *entries;
