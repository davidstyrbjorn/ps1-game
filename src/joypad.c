#include "joypad.h"
#include <libetc.h>

static u_long padstate;

void JoyPadInit(void)
{
  PadInit(0);
}

void JoyPadReset(void)
{
  padstate = 0;
}

void JoyPadUpdate(void)
{
  u_long pad = PadRead(0);
  padstate = pad;
}

int JoyPadCheck(int p)
{
  return padstate & p;
}