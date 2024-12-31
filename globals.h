#pragma once

#include <types.h>

#define OT_LENGTH 8192
#define PRIMITIVE_BUFFER_LENGTH 2048

void EmptyOT(u_short current_buffer);
void SetOTAt(u_short current_buffer, u_short index, u_long value);
u_long *GetOTAt(u_short current_buffer, u_short index);

void IncrementNextPrimitive(u_int size);
void SetNextPrimitive(char *value);
void ResetNextPrimitive(u_short current_buffer);
char *GetNextPrimitive(void);