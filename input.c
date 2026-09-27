#include "input.h"
#include "raylib.h"
#include "raymath.h"

float CalculateScaleFactor() {
  return MIN((float)GetScreenWidth() / GAME_WIDTH,
             (float)GetScreenHeight() / GAME_HEIGHT);
}

Vector2 CalculateVirtualMouse(float scale) {
  Vector2 mouse = GetMousePosition();
  Vector2 virtualMouse = {
      (mouse.x - (GetScreenWidth() - GAME_WIDTH * scale) * 0.5f) / scale,
      (mouse.y - (GetScreenHeight() - GAME_HEIGHT * scale) * 0.5f) / scale};
  return Vector2Clamp(virtualMouse, (Vector2){0, 0},
                      (Vector2){(float)GAME_WIDTH, (float)GAME_HEIGHT});
}