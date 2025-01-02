#include <libetc.h>
#include <libgpu.h> // Double buffering, display control, drawing primitives, etc
#include <libgte.h> // 3d transformations math etc
#include <stdio.h>
#include <libcd.h>
#include <stdlib.h>

#include "joypad.h"
#include "globals.h"
#include "display.h"
#include "camera.h"
#include "util.h"
#include "object.h"

extern char __heap_start, __sp; // Nugget specific, needed for malloc to work

/*
    char c;  // -> 8 bits
    short s; // -> 16 bits
    int i;   // -> 32 bits
    long l;  // -> 32 bits
    // No 64 bit types in PS1

    u_short us; // -> unsigned 16 bits
    u_long ul;  // -> unsigned 32 bits
*/

const short FLOOR_SIZE = 600;

typedef struct
{
  SVECTOR rotation;
  VECTOR position;
  VECTOR scale;
  VECTOR velocity;
  VECTOR acceleration;
  SVECTOR vertices[8];
  short faces[24];
} Cube;

Object object = {};

POLY_F4 *poly_f4;

Camera camera;

MATRIX world = {0};
MATRIX view = {0};

void Setup()
{
  InitHeap3((unsigned long *)(&__heap_start), (&__sp - 0x5000) - &__heap_start);
  ScreenInit();
  CdInit(); // Initialize CD subsystem
  JoyPadInit();

  ResetNextPrimitive(GetCurrBuff());

  VECTOR position = {500, -1000, -1000};
  camera.position.vx = position.vx;
  camera.position.vy = position.vy;
  camera.position.vz = position.vz;
  camera.look_at = (MATRIX){0};

  // Create object (cube) from model file MODEL.BIN
  setVector(&object.position, 0, 0, 0);
  setVector(&object.rotation, 0, 0, 0);
  setVector(&object.scale, ONE, ONE, ONE);

  // Start reading from the file
  u_long length;
  char *bytes = FileRead("\\MODEL.BIN;1", &length);

  // Read vertices from buffer
  u_long b = 0; // Counter of bytes
  object.num_vertices = GetShortBE(bytes, &b);
  object.vertices = malloc3(object.num_vertices * sizeof(SVECTOR));
  for (int i = 0; i < object.num_vertices; i++)
  {
    object.vertices[i].vx = GetShortBE(bytes, &b);
    object.vertices[i].vy = GetShortBE(bytes, &b);
    object.vertices[i].vz = GetShortBE(bytes, &b);
  }

  // Read indices from buffer
  object.num_faces = GetShortBE(bytes, &b) * 4; // 4 indices per quad
  object.faces = malloc3(object.num_faces * sizeof(short));
  for (int i = 0; i < object.num_faces; i++)
  {
    object.faces[i] = GetShortBE(bytes, &b);
  }

  // Read colors from buffer
  object.num_colors = GetChar(bytes, &b);
  object.colors = malloc3(object.num_colors * sizeof(CVECTOR));
  for (int i = 0; i < object.num_colors; i++)
  {
    object.colors[i].r = GetChar(bytes, &b);
    object.colors[i].g = GetChar(bytes, &b);
    object.colors[i].b = GetChar(bytes, &b);
    object.colors[i].cd = GetChar(bytes, &b);
  }

  // Free the read file buffer
  free3(bytes);
}

short rotation_speed = ONE;
short rotation_direction = 1;

void Update()
{
  long otz, p, flag;

  EmptyOT(GetCurrBuff()); // Clear OT

  JoyPadUpdate();

  if (JoyPadCheck(PAD1_LEFT))
  {
    camera.position.vx -= 50;
  }
  if (JoyPadCheck(PAD1_RIGHT))
  {
    camera.position.vx += 50;
  }
  if (JoyPadCheck(PAD1_UP))
  {
    camera.position.vy -= 50;
  }
  if (JoyPadCheck(PAD1_DOWN))
  {
    camera.position.vy += 50;
  }

  // Compute the look at matrix for object
  LookAt(&camera, &camera.position, &object.position, &(VECTOR){0, -ONE, 0});

  RotMatrix(&object.rotation, &world);
  TransMatrix(&world, &object.position);
  ScaleMatrix(&world, &object.scale);

  // Combine world and look at matrix to get the... wait for it... view matrix
  CompMatrixLV(&camera.look_at, &world, &view);

  SetRotMatrix(&view);
  SetTransMatrix(&view);

  // Loop all faces of the object
  for (int i = 0, q = 0; i < object.num_faces; i += 4, q++)
  {
    poly_f4 = (POLY_F4 *)GetNextPrimitive();
    SetPolyF4(poly_f4);
    setRGB0(poly_f4, object.colors[q].r, object.colors[q].g, object.colors[q].b); // Set the color of the polygon

    // This function will let us discard the polygon if it is ocluded (<= 0)
    int nclip =
        RotAverageNclip4(&object.vertices[object.faces[i]], &object.vertices[object.faces[i + 1]],
                         &object.vertices[object.faces[i + 2]], &object.vertices[object.faces[i + 3]], (long *)&poly_f4->x0,
                         (long *)&poly_f4->x1, (long *)&poly_f4->x2, (long *)&poly_f4->x3, &p, &otz, &flag);
    if (nclip <= 0)
    {
      continue; // Discard pixel
    }

    if ((otz > 0) && (otz < OT_LENGTH))
    {
      addPrim(GetOTAt(GetCurrBuff(), otz), poly_f4);
      IncrementNextPrimitive(sizeof(POLY_F4));
    }
  }

  object.rotation.vy += 20;
}

void Render() { DisplayFrame(); }

int main()
{
  Setup();
  while (1)
  {
    Update();
    Render();
  }

  return 0;
}
