#include "camera.h"

void VectorCross(VECTOR *a, VECTOR *b, VECTOR *out)
{
  OuterProduct12(a, b, out);
}

void LookAt(Camera *camera, VECTOR *eye, VECTOR *target, VECTOR *up)
{
  VECTOR x_right;
  VECTOR y_up;
  VECTOR z_forward;
  VECTOR x, y, z;
  VECTOR position;
  VECTOR translation;

  // Z forward is just pointing straight at our target
  z_forward.vx = target->vx - eye->vx;
  z_forward.vy = target->vy - eye->vy;
  z_forward.vz = target->vz - eye->vz;
  VectorNormal(&z_forward, &z);

  // Calculating x right and y up by cross product
  VectorCross(&z, up, &x_right);
  VectorNormal(&x_right, &x);

  VectorCross(&z, &x, &y_up);
  VectorNormal(&y_up, &y);

  camera->look_at.m[0][0] = x.vx;
  camera->look_at.m[0][1] = x.vy;
  camera->look_at.m[0][2] = x.vz;

  camera->look_at.m[1][0] = y.vx;
  camera->look_at.m[1][1] = y.vy;
  camera->look_at.m[1][2] = y.vz;

  camera->look_at.m[2][0] = z.vx;
  camera->look_at.m[2][1] = z.vy;
  camera->look_at.m[2][2] = z.vz;

  position.vx = -eye->vx;
  position.vy = -eye->vy;
  position.vz = -eye->vz;

  ApplyMatrixLV(&camera->look_at, &position, &translation);
  TransMatrix(&camera->look_at, &translation);
}