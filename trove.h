#pragma once

#include <stdint.h>
#include <stdbool.h>

#define APP_VERSION "\nTrove v1.5"

static void add(void);
static void find(void);
static void edit(void);
static void delEntry(void);
static void showEntry(int);
static void clipboard(void);
static void updateSettings(void);
static void handleParameters(char *);
static void setPasswordSize(void);
static void setMinSpecial(void);
static void setMinNumeric(void);
static void setMinUppercase(void);
static void setNewKeygen(void);
#ifdef _WIN32
static void getPasswordWindows(char *);
#else
static void getPasswordLinux(char *);
#endif
static void importFromUPM();
static void readUntilComma(char *, int);
static void readMisc(char *, int);
static bool copyBuffer(char *, char *, int *, int, int);
void getDBpassword(char *);
bool setDBpassword(void);
