#include "animation.h"
#include "constants.h"
#include "screen_setup.h"
#include "input.h"
#include "rendering.h"
#include "raylib.h"
#include "entities.h"

int main(void) {
  unsigned short *size = get_screen_size();
  const int screenWidth = size[0];
  const int screenHeight = size[1];

  InitializeWindow(screenWidth, screenHeight, GAME_WIDTH, GAME_HEIGHT);
  RenderTexture2D target = InitializeRenderTexture(GAME_WIDTH, GAME_HEIGHT);

  // Animation animations[MAX_ANIMATIONS];
  Item items[MAX_ITEMS];
  Person persons[MAX_PERSONS];
  // int animCount = LoadAnimations(animations);
  int animItemCount = LoadItems(items, MAX_ITEMS);
  int animPersonCount = LoadPersons(persons, MAX_PERSONS);
  Texture2D background = LoadTexture("nadr.png");

  float animItemTimers[MAX_ITEMS] = {0};
  float animPersonTimers[MAX_PERSONS] = {0};

  int animItemIndices[MAX_ITEMS] = {0};
  int animPersonIndices[MAX_PERSONS] = {0};


  SetTargetFPS(60);

  while (!WindowShouldClose()) {
    float scale = CalculateScaleFactor();
    Vector2 virtualMouse = CalculateVirtualMouse(scale);

    // UpdateAnimations(animTimers, animIndices, animCount, animations);
    UpdateItems(animItemTimers, animItemIndices, animItemCount, items);
    UpdatePersons(animPersonTimers, animPersonIndices, animPersonCount, persons);
    DrawAnimationsToRenderTexture(target, animations, animCount, animIndices, scale, virtualMouse, background);
    HandleClickDetection(animations, animCount, animIndices, virtualMouse);
    PresentToScreen(target, scale, GAME_WIDTH, GAME_HEIGHT);
  }

  for (int i = 0; i < animPersonCount; i++) UnloadAnimation(&persons[i].anim);
  for (int i = 0; i < animItemCount; i++) UnloadAnimation(&items[i].anim);
  UnloadRenderTexture(target);
  CloseWindow();
  return 0;
}