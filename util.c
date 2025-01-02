#include "util.h"

#include <libcd.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>

#define BytesPerSector 2048

char *FileRead(char *filename, u_long *length)
{
  CdlFILE filepos;
  int numsectors;
  char *buffer;

  buffer = NULL;

  if (CdSearchFile(&filepos, filename) == NULL)
  {
    printf("%s file not found in the CD.\n", filename);
  }
  else
  {
    printf("Found %s in the CD.\n", filename);
    numsectors = (filepos.size + 2047) / 2048;   // compute the number of sectors to read from the file
    buffer = (char *)malloc3(2048 * numsectors); // allocate buffer for the file
    if (!buffer)
    {
      printf("Error allocating %d sectors!\n", numsectors);
    }
    CdControl(CdlSetloc, (u_char *)&filepos.pos, 0);    // set read target to the file
    CdRead(numsectors, (u_long *)buffer, CdlModeSpeed); // start reading from the CD
    CdReadSync(0, 0);                                   // wait until the read is complete
  }

  *length = filepos.size;

  return buffer;
}

short GetShortLE(u_char *bytes, u_long *b)
{
  unsigned short value = 0;
  value |= bytes[(*b)++] << 0;
  value |= bytes[(*b)++] << 8;
  return (short)value;
}

short GetShortBE(u_char *bytes, u_long *b)
{
  unsigned short value = 0;
  value |= bytes[(*b)++] << 8;
  value |= bytes[(*b)++] << 0;
  return (short)value;
}

char GetChar(u_char *bytes, u_long *b)
{
  return bytes[(*b)++];
}

long GetLongLE(u_char *bytes, u_long *b)
{
  u_long value = 0;
  value |= bytes[(*b)++] << 0;
  value |= bytes[(*b)++] << 8;
  value |= bytes[(*b)++] << 16;
  value |= bytes[(*b)++] << 24;
  return (long)value;
}

long GetLongBE(u_char *bytes, u_long *b)
{
  u_long value = 0;
  value |= bytes[(*b)++] << 24;
  value |= bytes[(*b)++] << 16;
  value |= bytes[(*b)++] << 8;
  value |= bytes[(*b)++] << 0;
  return (long)value;
}