#pragma once

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
#define ID_SETTINGS_SET_PASSWORD 40
#define ID_SETTINGS_VERSION 41

#define ID_PASSWORD_LABEL1 50
#define ID_PASSWORD_PASSWORD1 51
#define ID_PASSWORD_LABEL2 52
#define ID_PASSWORD_PASSWORD2 53
#define ID_PASSWORD_MESSAGE 54
#define ID_PASSWORD_WARNING 55

#define ID_TIMER1 1
#define ID_TIMER2 2

void centerWindow(HWND);
void fillListbox(void);
void updateListbox(void);
void addEntry(void);
void editEntry(void);
void deleteEntry(void);
void editSettings(void);
bool isNumeric(char *buf);
void fillDropdown(HWND, int, int);
void outs(char *);
void getDBpassword(void);
void setNewDBpassword(void);
void shutDown(HWND);

LRESULT CALLBACK mainWndProc(HWND, UINT, WPARAM, LPARAM);
LRESULT CALLBACK addWndProc(HWND, UINT, WPARAM, LPARAM);
LRESULT CALLBACK editWndProc(HWND, UINT, WPARAM, LPARAM);
LRESULT CALLBACK settingsWndProc(HWND, UINT, WPARAM, LPARAM);
LRESULT CALLBACK setPasswordWndProc(HWND, UINT, WPARAM, LPARAM);
LRESULT CALLBACK getPasswordWndProc(HWND, UINT, WPARAM, LPARAM);
LRESULT CALLBACK customGetPasswordEditProc(HWND, UINT, WPARAM, LPARAM);
LRESULT CALLBACK customEditProc(HWND, UINT, WPARAM, LPARAM);
