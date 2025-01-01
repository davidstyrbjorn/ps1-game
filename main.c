#include <libetc.h>
#include <libgpu.h> // Double buffering, display control, drawing primitives, etc
#include <libgte.h> // 3d transformations math etc
#include <stdio.h>

#include "joypad.h"
#include "globals.h"
#include "display.h"
#include "camera.h"

#define NUM_VERTICES 8
#define NUM_FACES 6

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

typedef struct
{
  SVECTOR rotation;
  VECTOR position;
  VECTOR scale;
  SVECTOR vertices[4];
  short faces[6];
} Floor;

POLY_G4 *poly_g4;
POLY_F3 *poly_f3;

Camera camera;

Cube cube_obj = {
    .rotation = {0, 0, 0},
    .position = {0, -400, 1800},
    .scale = {ONE, ONE, ONE},
    .velocity = {0, 0, 0},
    .acceleration = {0, 1, 0},
    .vertices = {
        {-128, -128, -128}, {128, -128, -128}, {128, -128, 128}, {-128, -128, 128}, {-128, 128, -128}, {128, 128, -128}, {128, 128, 128}, {-128, 128, 128}},
    .faces = {3, 2, 0, 1, // top quad
              0, 1, 4, 5, // front quad
              4, 5, 7, 6, // bottom quad
              1, 2, 5, 6, // right quad
              2, 3, 6, 7, // back quad
              3, 0, 7, 4}};

Floor floor_obj = {
    .rotation = {0, 0, 0},
    .position = {0, 450, 1800},
    .scale = {ONE, ONE, ONE},
    .vertices = {
        {-FLOOR_SIZE, 0, -FLOOR_SIZE},
        {-FLOOR_SIZE, 0, FLOOR_SIZE},
        {FLOOR_SIZE, 0, -FLOOR_SIZE},
        {FLOOR_SIZE, 0, FLOOR_SIZE}},
    .faces = {0, 1, 2, 1, 3, 2}};

MATRIX world = {0};
MATRIX view = {0};

void Setup()
{
  ScreenInit();

  ResetNextPrimitive(GetCurrBuff());

  JoyPadInit();

  VECTOR position = {500, -1000, -1000};
  camera.position.vx = position.vx;
  camera.position.vy = position.vy;
  camera.position.vz = position.vz;
  camera.look_at = (MATRIX){0};
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
  // if (JoyPadCheck(PAD1_CROSS))
  // {
  //   camera.position.vz += 50;
  // }
  // if (JoyPadCheck(PAD1_CIRCLE))
  // {
  //   camera.position.vz -= 50;
  // }

  // Update position based on acceleration, and velocity
  cube_obj.velocity.vx += cube_obj.acceleration.vx;
  cube_obj.velocity.vy += cube_obj.acceleration.vy;
  cube_obj.velocity.vz += cube_obj.acceleration.vz;
  cube_obj.position.vx += cube_obj.velocity.vx / 2;
  cube_obj.position.vy += cube_obj.velocity.vy / 2;
  cube_obj.position.vz += cube_obj.velocity.vz / 2;

  if (cube_obj.position.vy + 150 > floor_obj.position.vy)
  {
    cube_obj.velocity.vy *= -1;
    rotation_speed = ONE;
    rotation_direction *= -1;
  }

  // Compute the look at matrix for this cube
  LookAt(&camera, &camera.position, &cube_obj.position, &(VECTOR){0, -ONE, 0});

  RotMatrix(&cube_obj.rotation, &world);
  TransMatrix(&world, &cube_obj.position);
  ScaleMatrix(&world, &cube_obj.scale);

  // Combine world and look at matrix to get the... wait for it... view matrix
  CompMatrixLV(&camera.look_at, &world, &view);

  SetRotMatrix(&view);
  SetTransMatrix(&view);

  // Loop all faces of the cube
  for (int i = 0; i < NUM_FACES * 4; i += 4)
  {
    poly_g4 = (POLY_G4 *)GetNextPrimitive();
    SetPolyG4(poly_g4);
    setRGB0(poly_g4, 255, 255, 0); // Set the color of the polygon
    setRGB1(poly_g4, 255, 0, 255);
    setRGB2(poly_g4, 0, 255, 255);
    setRGB3(poly_g4, 100, 255, 100);

    // This function will let us discard the polygon if it is ocluded (<= 0)
    int nclip =
        RotAverageNclip4(&cube_obj.vertices[cube_obj.faces[i]], &cube_obj.vertices[cube_obj.faces[i + 1]],
                         &cube_obj.vertices[cube_obj.faces[i + 2]], &cube_obj.vertices[cube_obj.faces[i + 3]], (long *)&poly_g4->x0,
                         (long *)&poly_g4->x1, (long *)&poly_g4->x2, (long *)&poly_g4->x3, &p, &otz, &flag);
    if (nclip <= 0)
    {
      continue;
    }

    if ((otz > 0) && (otz < OT_LENGTH))
    {
      addPrim(GetOTAt(GetCurrBuff(), otz), poly_g4);
      IncrementNextPrimitive(sizeof(POLY_G4));
    }
  }

  cube_obj.rotation.vy += (rotation_speed / (ONE / 30)) * rotation_direction;
  rotation_speed -= ONE / 100;
  rotation_speed = (rotation_speed < 0) ? 0 : rotation_speed;

  // Floor
  RotMatrix(&floor_obj.rotation, &world);
  TransMatrix(&world, &floor_obj.position);
  ScaleMatrix(&world, &floor_obj.scale);

  // Combine world and look at matrix to get the... wait for it... view matrix
  CompMatrixLV(&camera.look_at, &world, &view);

  SetRotMatrix(&view);
  SetTransMatrix(&view);

  for (int i = 0; i < (2 * 3); i += 3)
  {
    poly_f3 = (POLY_F3 *)GetNextPrimitive();
    setPolyF3(poly_f3);
    setRGB0(poly_f3, 100, 100, 255);
    int nclip = RotAverageNclip3(
        &floor_obj.vertices[floor_obj.faces[i]], &floor_obj.vertices[floor_obj.faces[i + 1]],
        &floor_obj.vertices[floor_obj.faces[i + 2]], (long *)&poly_f3->x0, (long *)&poly_f3->x1,
        (long *)&poly_f3->x2, &p, &otz, &flag);

    if (nclip <= 0)
    {
      continue;
    }

    if ((otz > 0) && (otz < OT_LENGTH))
    {
      addPrim(GetOTAt(GetCurrBuff(), otz), poly_f3);
      IncrementNextPrimitive(sizeof(POLY_F3));
    }
  }

  floor_obj.rotation.vy -= 3;
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
