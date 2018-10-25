#include <stdio.h>
#include <stdlib.h>	// exit(), rand(), malloc()
#include <string.h>	// strcmp(), strcpy(), strlen()
#include <time.h>	// time()
#include <ctype.h>	// isupper(), ispunct(), isdigit()
#include <stdint.h>	// uint8_t
#include <stdbool.h>
#include <windows.h>

#define ID_MAIN_ADD 1
#define ID_MAIN_EDIT 2
#define ID_MAIN_DELETE 3
#define ID_MAIN_SETTINGS 4
#define ID_MAIN_QUIT 5
#define ID_MAIN_FINDTEXT 6
#define ID_MAIN_FIND 7
#define ID_MAIN_LISTBOX 8

#define ID_EDIT_TITLE_LABEL 10
#define ID_EDIT_TITLE 11
#define ID_EDIT_ID_LABEL 12
#define ID_EDIT_ID 13
#define ID_EDIT_PW_LABEL 14
#define ID_EDIT_PW 15
#define ID_EDIT_MISC_LABEL 16
#define ID_EDIT_MISC 17

#define ID_OK 20
#define ID_CANCEL 21
#define ID_GENERATE 22

#define ID_SETTINGS_PW_LABEL 30
#define ID_SETTINGS_PW_LENGTH 31
#define ID_SETTINGS_SPECIAL_LABEL 32
#define ID_SETTINGS_SPECIAL_LENGTH 33
#define ID_SETTINGS_NUMERIC_LABEL 34
#define ID_SETTINGS_NUMERIC_LENGTH 35
#define ID_SETTINGS_UPPERCASE_LABEL 36
#define ID_SETTINGS_UPPERCASE_LENGTH 37
#define ID_SETTINGS_KEYGEN_LABEL 38
#define ID_SETTINGS_KEYGEN_LENGTH 39

#define MAXTITLE 21
#define MAXID 21
#define MAXPW 21
#define MAXMISC 51
#define MAXLINE 120
#define MINPW 6
#define MAXCHARS 9
#define DBPASSWORDSIZE 32
#define IV_SIZE 17

void loadEntries(void);
void saveEntries(void);
void sortEntries(void);
void centerWindow(HWND);
void updateListbox(void);
void addEntry(void);
void editEntry(void);
void deleteEntry(void);
void findEntry(void);
void editSettings(void);
void readSettings(void);
void writeSettings(void);
void generatePassword(wchar_t *);
void generateKeygen(char *);
bool isNumeric(char *buf);
void fillDropdown(HWND, int, int);
void outw(wchar_t *);
void outs(char *);

LRESULT CALLBACK mainWndProc(HWND, UINT, WPARAM, LPARAM);
LRESULT CALLBACK addWndProc(HWND, UINT, WPARAM, LPARAM);
LRESULT CALLBACK editWndProc(HWND, UINT, WPARAM, LPARAM);
LRESULT CALLBACK settingsWndProc(HWND, UINT, WPARAM, LPARAM);

struct Entry
{
	wchar_t title[MAXTITLE];
	wchar_t id[MAXID];
	wchar_t pw[MAXPW];
	wchar_t misc[MAXMISC];
} *entries;
