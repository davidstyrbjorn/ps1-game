#pragma once

#include "globals.h"

typedef struct Camera
{
  VECTOR position;
  SVECTOR rotation;
  MATRIX look_at;
} Camera;

void LookAt(Camera *camera, VECTOR *eye, VECTOR *target, VECTOR *up);