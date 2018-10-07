#include <stdio.h>
#include <stdbool.h>
#include <windows.h>

#define ID_MAIN_ADD 1
#define ID_MAIN_EDIT 2
#define ID_MAIN_DELETE 3
#define ID_MAIN_SETTINGS 4
#define ID_MAIN_QUIT 5
#define ID_MAIN_TEXTBOX 6
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
#define ID_EDIT_CANCEL 18
#define ID_EDIT_OK 19

#define MAXTITLE 21
#define MAXID 21
#define MAXPW 21
#define MAXMISC 51
#define MAXLINE 120

// #define MINPW 6
// #define MINCHARS 0
// #define MAXCHARS 6
// #define MAXNAME 10
// #define KEYSIZE 32

void loadEntries(void);
void saveEntries(void);
void sortEntries(void);
void centerWindow(HWND);
void updateListbox(void);
void addEntry(void);
void editEntry(void);
void deleteEntry(void);
void findEntry(void);
void settings(void);
// void splitRow(char[], struct Entry *);
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
