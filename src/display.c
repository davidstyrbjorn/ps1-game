#include "display.h"

#include "globals.h"

#include <libetc.h>
#include <libgpu.h> // Double buffering, display control, drawing primitives, etc
#include <libgte.h> // 3d transformations math etc

double_buffer_t screen;
short current_buffer;

u_short GetCurrBuff(void)
{
  return current_buffer;
}

void ScreenInit(void)
{
  // Reset the GPU
  ResetGraph(0);

  // Set the display area of the first buffer
  SetDefDispEnv(&screen.disp[0], 0, 0, SCREEN_RES_X, SCREEN_RES_Y);
  SetDefDrawEnv(&screen.draw[0], 0, SCREEN_RES_Y, SCREEN_RES_X, SCREEN_RES_Y);

  // Set the display area of the second buffer
  SetDefDispEnv(&screen.disp[1], 0, SCREEN_RES_Y, SCREEN_RES_X, SCREEN_RES_Y);
  SetDefDrawEnv(&screen.draw[1], 0, 0, SCREEN_RES_X, SCREEN_RES_Y);

  // Set the back/drawing buffer
  screen.draw[0].isbg = 1;
  screen.draw[1].isbg = 1;

  // Set the background clear color
  setRGB0(&screen.draw[0], 52, 21, 57);
  setRGB0(&screen.draw[1], 52, 21, 57);

  // Set current initial buffer
  current_buffer = 0;
  PutDispEnv(&screen.disp[current_buffer]);
  PutDrawEnv(&screen.draw[current_buffer]);

  // initialize and setup the GTE geomeetry offsets
  InitGeom();
  SetGeomOffset(SCREEN_CENTER_X, SCREEN_CENTER_Y);
  SetGeomScreen(SCREEN_Z);

  // Enable display
  SetDispMask(1);
}

void DisplayFrame(void)
{
  DrawSync(0);
  VSync(0);

  // Draw to the current buffer
  PutDispEnv(&screen.disp[current_buffer]);
  PutDrawEnv(&screen.draw[current_buffer]);

  DrawOTag(GetOTAt(current_buffer, OT_LENGTH - 1)); // End of OT, back to front

  current_buffer = !current_buffer; // Swap current buffer
  ResetNextPrimitive(current_buffer);
}