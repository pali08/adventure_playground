#include "animation.h"
#include "screen_setup.h"
#include "input.h"
#include "rendering.h"
#include "raylib.h"

int main(void) {
  unsigned short *size = get_screen_size();
  const int screenWidth = size[0];
  const int screenHeight = size[1];

  InitializeWindow(screenWidth, screenHeight, GAME_WIDTH, GAME_HEIGHT);
  RenderTexture2D target = InitializeRenderTexture(GAME_WIDTH, GAME_HEIGHT);

  Animation animations[MAX_ANIMATIONS];
  int animCount = LoadAnimations(animations);
  Texture2D background = LoadTexture("nadr.png");

  float animTimers[MAX_ANIMATIONS] = {0};
  int animIndices[MAX_ANIMATIONS] = {0};

  SetTargetFPS(60);

  while (!WindowShouldClose()) {
    float scale = CalculateScaleFactor();
    Vector2 virtualMouse = CalculateVirtualMouse(scale);

    UpdateAnimations(animTimers, animIndices, animCount, animations);
    DrawAnimationsToRenderTexture(target, animations, animCount, animIndices, scale, virtualMouse, background);
    HandleClickDetection(animations, animCount, animIndices, virtualMouse);
    PresentToScreen(target, scale, GAME_WIDTH, GAME_HEIGHT);
  }

  for (int i = 0; i < animCount; i++) UnloadAnimation(&animations[i]);
  UnloadRenderTexture(target);
  CloseWindow();
  return 0;
}