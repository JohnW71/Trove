#pragma once

#include <stdint.h>	// uint8_t

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
void getDBpassword(uint8_t *);
#ifdef _WIN32
void getPasswordWindows(uint8_t *);
#else
void getPasswordLinux(uint8_t *);
#endif
