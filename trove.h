#define MAXTITLE 21
#define MAXMISC 51
#define MAXLINE 80
#define MAXID 21
#define MAXPW 21
#define MINPW 6
#define MINCHARS 0
#define MAXCHARS 6
#define DBPASSWORDSIZE 32
#define IV_SIZE 17

#include <stdio.h>
#include <stdlib.h>	// exit(), rand(), malloc()
#include <string.h>	// strcmp(), strcpy(), strlen()
#include <time.h>	// time()
#include <ctype.h>	// isupper(), ispunct(), isdigit()
#include <stdint.h>	// uint8_t
#include <stdbool.h>

void list(void);
void add(void);
void find(void);
void edit(void);
void delEntry(void);
void clipboard(void);
void readEntries(void);
void saveEntries(void);
void generatePassword(char *);
void generateKeygen(char *);
bool setDBpassword(void);
void readSettings(void);
void writeSettings(void);
void updateSettings(void);
void setPasswordSize(void);
void setMinSpecial(void);
void setMinNumeric(void);
void setMinUppercase(void);
void setNewKeygen(void);
void writeFile(void);
void readFile(void);
void encrypt_cbc(uint8_t *, uint8_t *);
void decrypt_cbc(uint8_t *, uint8_t *);
void addPadding(char *);
void loadEncryptedEntries(void);
void updateBuffer(void);
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
