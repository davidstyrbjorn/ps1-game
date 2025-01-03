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

typedef struct Texture
{
  u_long tim_mode; // pixel mode of the TIM
  RECT tim_prect;  // store the X and Y location of the texture
  RECT tim_crect;  // -||- CLUT
} Texture;

/*
    char c;  // -> 8 bits
    short s; // -> 16 bits
    int i;   // -> 32 bits
    long l;  // -> 32 bits
    // No 64 bit types in PS1

    u_short us; // -> unsigned 16 bits
    u_long ul;  // -> unsigned 32 bits
*/

Object object = {};
Texture brick_texture;

Object lava_object = {};
Texture lava_texture;

POLY_FT4 *poly_ft4;

Camera camera;

MATRIX world = {0};
MATRIX view = {0};

void LoadModel(char *file_name, Object *object)
{
  // Start reading from the file
  u_long length;
  char *bytes = FileRead(file_name, &length);

  // Read vertices from buffer
  u_long b = 0; // Counter of bytes
  object->num_vertices = GetShortBE(bytes, &b);
  object->vertices = malloc3(object->num_vertices * sizeof(SVECTOR));
  for (int i = 0; i < object->num_vertices; i++)
  {
    object->vertices[i].vx = GetShortBE(bytes, &b);
    object->vertices[i].vy = GetShortBE(bytes, &b);
    object->vertices[i].vz = GetShortBE(bytes, &b);
  }

  // Read indices from buffer
  object->num_faces = GetShortBE(bytes, &b) * 4; // 4 indices per quad
  object->faces = malloc3(object->num_faces * sizeof(short));
  for (int i = 0; i < object->num_faces; i++)
  {
    object->faces[i] = GetShortBE(bytes, &b);
  }

  // Read colors from buffer
  object->num_colors = GetChar(bytes, &b);
  object->colors = malloc3(object->num_colors * sizeof(CVECTOR));
  for (int i = 0; i < object->num_colors; i++)
  {
    object->colors[i].r = GetChar(bytes, &b);
    object->colors[i].g = GetChar(bytes, &b);
    object->colors[i].b = GetChar(bytes, &b);
    object->colors[i].cd = GetChar(bytes, &b);
  }

  // Free the read file buffer
  free3(bytes);
}

Texture LoadTexture(char *file_name)
{
  // Start reading from the file
  u_long length;
  TIM_IMAGE tim;
  u_long b = 0; // Counter of bytes
  u_long *bytes;

  bytes = (u_long *)FileRead(file_name, &length);
  OpenTIM(bytes);
  ReadTIM(&tim);

  LoadImage(tim.prect, tim.paddr); // This is ASYNC
  DrawSync(0);                     // Wait for copy to VRAM to complete

  int hasCLUT = tim.mode & 0x8;
  if (hasCLUT)
  {
    LoadImage(tim.crect, tim.caddr); // This is ASYNC
    DrawSync(0);                     // Wait for copy to VRAM to complete
  }

  free3(bytes);

  return (Texture){
      .tim_mode = tim.mode,
      .tim_prect = *tim.prect,
      .tim_crect = *tim.crect,
  };
}

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

  LoadModel("\\MODEL.BIN;1", &object);
  brick_texture = LoadTexture("\\BRICKS.TIM;1");

  setVector(&lava_object.position, 0, 200, 0);
  setVector(&lava_object.rotation, 0, 0, 0);
  setVector(&lava_object.scale, ONE * 4, ONE * 0.1, ONE * 4);

  LoadModel("\\MODEL.BIN;1", &lava_object);
  lava_texture = LoadTexture("\\LAVA.TIM;1");
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
    poly_ft4 = (POLY_FT4 *)GetNextPrimitive();
    SetPolyFT4(poly_ft4);

    poly_ft4->r0 = 128;
    poly_ft4->g0 = 128;
    poly_ft4->b0 = 128;

    // Setting our UV coordinates
    poly_ft4->u0 = 0;
    poly_ft4->v0 = 0;

    poly_ft4->u1 = 63;
    poly_ft4->v1 = 0;

    poly_ft4->u2 = 0;
    poly_ft4->v2 = 63;

    poly_ft4->u3 = 63;
    poly_ft4->v3 = 63;

    poly_ft4->tpage = getTPage(brick_texture.tim_mode & 0x3, 0, brick_texture.tim_prect.x, brick_texture.tim_prect.y);
    poly_ft4->clut = getClut(brick_texture.tim_crect.x, brick_texture.tim_crect.y);

    // This function will let us discard the polygon if it is ocluded (<= 0)
    int nclip =
        RotAverageNclip4(&object.vertices[object.faces[i]], &object.vertices[object.faces[i + 1]],
                         &object.vertices[object.faces[i + 2]], &object.vertices[object.faces[i + 3]], (long *)&poly_ft4->x0,
                         (long *)&poly_ft4->x1, (long *)&poly_ft4->x2, (long *)&poly_ft4->x3, &p, &otz, &flag);
    if (nclip <= 0)
    {
      continue; // Discard pixel
    }

    if ((otz > 0) && (otz < OT_LENGTH))
    {
      addPrim(GetOTAt(GetCurrBuff(), otz), poly_ft4);
      IncrementNextPrimitive(sizeof(POLY_FT4));
    }
  }

  object.rotation.vy += 20;

  RotMatrix(&lava_object.rotation, &world);
  TransMatrix(&world, &lava_object.position);
  ScaleMatrix(&world, &lava_object.scale);

  // Combine world and look at matrix to get the... wait for it... view matrix
  CompMatrixLV(&camera.look_at, &world, &view);

  SetRotMatrix(&view);
  SetTransMatrix(&view);

  // Loop all faces of the object
  for (int i = 0, q = 0; i < lava_object.num_faces; i += 4, q++)
  {
    poly_ft4 = (POLY_FT4 *)GetNextPrimitive();
    SetPolyFT4(poly_ft4);

    poly_ft4->r0 = 128;
    poly_ft4->g0 = 128;
    poly_ft4->b0 = 128;

    // Setting our UV coordinates
    poly_ft4->u0 = 0;
    poly_ft4->v0 = 0;

    poly_ft4->u1 = 63;
    poly_ft4->v1 = 0;

    poly_ft4->u2 = 0;
    poly_ft4->v2 = 63;

    poly_ft4->u3 = 63;
    poly_ft4->v3 = 63;

    poly_ft4->tpage = getTPage(lava_texture.tim_mode & 0x3, 0, lava_texture.tim_prect.x, lava_texture.tim_prect.y);
    poly_ft4->clut = getClut(lava_texture.tim_crect.x, lava_texture.tim_crect.y);

    // This function will let us discard the polygon if it is ocluded (<= 0)
    int nclip =
        RotAverageNclip4(&lava_object.vertices[lava_object.faces[i]], &lava_object.vertices[lava_object.faces[i + 1]],
                         &lava_object.vertices[lava_object.faces[i + 2]], &lava_object.vertices[lava_object.faces[i + 3]], (long *)&poly_ft4->x0,
                         (long *)&poly_ft4->x1, (long *)&poly_ft4->x2, (long *)&poly_ft4->x3, &p, &otz, &flag);
    if (nclip <= 0)
    {
      continue; // Discard pixel
    }

    if ((otz > 0) && (otz < OT_LENGTH))
    {
      addPrim(GetOTAt(GetCurrBuff(), otz), poly_ft4);
      IncrementNextPrimitive(sizeof(POLY_FT4));
    }
  }
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
