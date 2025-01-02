#pragma once

#include "globals.h"

char *FileRead(char *filename, u_long *length);

short GetShortLE(u_char *bytes, u_long *b);
short GetShortBE(u_char *bytes, u_long *b);

long GetLongBE(u_char *bytes, u_long *b);
long GetLongLE(u_char *bytes, u_long *b);

char GetChar(u_char *bytes, u_long *b);