#pragma once

#include <types.h>
#include "libgpu.h"

#define VIDEO_MODE 0
#define SCREEN_RES_X 320
#define SCREEN_RES_Y 240
#define SCREEN_CENTER_X (SCREEN_RES_X >> 1) // divided by 2
#define SCREEN_CENTER_Y (SCREEN_RES_Y >> 1) // divided by 2
#define SCREEN_Z 320

// Display buffer (output to screen), and drawing buffer (render to)
typedef struct
{
  DRAWENV draw[2];
  DISPENV disp[2];
} double_buffer_t;

u_short GetCurrBuff(void);

void ScreenInit(void);
void DisplayFrame(void);