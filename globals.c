#include "globals.h"
#include "libgpu.h"

static u_long ordering_table[2][OT_LENGTH]; // 2 OTs, one for each buffer
static char primitive_buffer[2][PRIMITIVE_BUFFER_LENGTH];
static char *next_primitive; // pointer into primitive_buffer

void SetOTAt(u_short current_buffer, u_short index, u_long value)
{
  ordering_table[current_buffer][index] = value;
}

void EmptyOT(u_short current_buffer)
{
  ClearOTagR(ordering_table[current_buffer], OT_LENGTH);
}

u_long *GetOTAt(u_short current_buffer, u_short index)
{
  return &ordering_table[current_buffer][index];
}

void IncrementNextPrimitive(u_int size)
{
  next_primitive += size;
}

void SetNextPrimitive(char *value)
{
  next_primitive = value;
}

void ResetNextPrimitive(u_short current_buffer)
{
  next_primitive = primitive_buffer[current_buffer];
}

char *GetNextPrimitive(void)
{
  return next_primitive;
}