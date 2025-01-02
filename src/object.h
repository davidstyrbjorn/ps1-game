#pragma once

#include "globals.h"

typedef struct Object
{
  VECTOR position;
  SVECTOR rotation;
  VECTOR scale;

  short num_vertices;
  SVECTOR *vertices;

  short num_faces;
  short *faces;

  short num_colors;
  CVECTOR *colors;
} Object;