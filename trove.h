#pragma once

#include <stdint.h>

#define APP_VERSION "\nTrove v1.35"

void list(void);
void add(void);
void find(void);
void edit(void);
void delEntry(void);
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
