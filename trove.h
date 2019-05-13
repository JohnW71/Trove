#pragma once

#include <stdint.h>
#include <stdbool.h>

#define APP_VERSION "\nTrove v1.40"

void add(void);
void find(void);
void edit(void);
void delEntry(void);
void showEntry(int);
void clipboard(void);
void updateSettings(void);
void handleParameters(char *);
void setPasswordSize(void);
void setMinSpecial(void);
void setMinNumeric(void);
void setMinUppercase(void);
void setNewKeygen(void);
void getDBpassword(char *);
#ifdef _WIN32
void getPasswordWindows(char *);
#else
void getPasswordLinux(char *);
#endif
void importFromUPM();
void readUntilComma(char *, int);
void readMisc(char *, int);
bool copyBuffer(char *, char *, int *, int, int);
