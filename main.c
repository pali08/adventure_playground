#include "animation.h"
#include "constants.h"
#include "screen_setup.h"
#include "raylib.h"
#include "raymath.h"
#include <dirent.h>
#include <stddef.h>
#include <stdint.h>



Animation animations[MAX_ANIMATIONS];
int animCount = 0;

// --- Function prototypes ---
void InitializeWindow(int screenWidth, int screenHeight, int gameWidth,
                      int gameHeight);
RenderTexture2D InitializeRenderTexture(int width, int height);
void LoadAnimations();
Vector2 CalculateVirtualMouse(const Vector2 *mousePos, float scale,
                              int gameWidth, int gameHeight);
void UpdateAnimations(float *animTimers, int *animIndices, int animCount,
                      const Animation *anims);
void DrawAnimationsToRenderTexture(RenderTexture2D target,
                                   const Animation *anims, int animCount,
                                   int *animIndices, float scale,
                                   Vector2 virtualMouse);
void HandleClickDetection(const Animation *anims, int animCount,
                          int *animIndices, Vector2 virtualMouse);
void PresentToScreen(RenderTexture2D target, float scale, int gameWidth,
                     int gameHeight);
void Cleanup();

//------------------------------------------------------------------------------------
// Program main entry point
//------------------------------------------------------------------------------------
int main(void) {
  unsigned short *size = get_screen_size();
  const int screenWidth = size[0];
  const int screenHeight = size[1];

  InitializeWindow(screenWidth, screenHeight, GAME_WIDTH, GAME_HEIGHT);
  RenderTexture2D target = InitializeRenderTexture(GAME_WIDTH, GAME_HEIGHT);

  LoadAnimations(); // Load multiple animations

  float animTimers[MAX_ANIMATIONS] = {0};
  int animIndices[MAX_ANIMATIONS] = {0};

  SetTargetFPS(60);

  // --- Main game loop ---
  while (!WindowShouldClose()) {
    float scale = MIN((float)GetScreenWidth() / GAME_WIDTH,
                      (float)GetScreenHeight() / GAME_HEIGHT);

    Vector2 mouse = GetMousePosition();
    Vector2 virtualMouse =
        CalculateVirtualMouse(&mouse, scale, GAME_WIDTH, GAME_HEIGHT);

    UpdateAnimations(animTimers, animIndices, animCount, animations);

    DrawAnimationsToRenderTexture(target, animations, animCount, animIndices, scale, virtualMouse);
    HandleClickDetection(animations, animCount, animIndices, virtualMouse);
    PresentToScreen(target, scale, GAME_WIDTH, GAME_HEIGHT);
  }

  Cleanup();
  return 0;
}

//------------------------------------------------------------------------------------
// Implementation of helper functions
//------------------------------------------------------------------------------------
void InitializeWindow(int screenWidth, int screenHeight, int gameWidth,
                      int gameHeight) {
  (void)gameWidth;
  (void)gameHeight;

  SetConfigFlags(FLAG_WINDOW_RESIZABLE | FLAG_VSYNC_HINT);
  InitWindow(screenWidth, screenHeight, "Multi-Animation Demo");
  SetWindowMinSize(320, 240);
}

RenderTexture2D InitializeRenderTexture(int width, int height) {
  RenderTexture2D target = LoadRenderTexture(width, height);
  SetTextureFilter(target.texture, TEXTURE_FILTER_BILINEAR);
  return target;
}

void LoadAnimations() {
  // Example: load 2 animations manually for now (you can populate from a config)
  animations[0] = LoadAnimation("animation", 0.5f, (Vector2){GAME_WIDTH * 0.25f, GAME_HEIGHT * 0.25f});
  animations[1] = LoadAnimation("animation2", 2.0f, (Vector2){GAME_WIDTH * 0.6f, GAME_HEIGHT * 0.5f});
  animCount = 2;
}

Vector2 CalculateVirtualMouse(const Vector2 *mousePos, float scale,
                              int gameWidth, int gameHeight) {
  Vector2 virtualMouse = {
      (mousePos->x - (GetScreenWidth() - gameWidth * scale) * 0.5f) / scale,
      (mousePos->y - (GetScreenHeight() - gameHeight * scale) * 0.5f) / scale};
  return Vector2Clamp(virtualMouse, (Vector2){0, 0},
                      (Vector2){(float)gameWidth, (float)gameHeight});
}

void UpdateAnimations(float *animTimers, int *animIndices, int animCount,
                      const Animation *anims) {
  for (int i = 0; i < animCount; i++) {
    const Animation *anim = &anims[i];
    if (!anim->active || anim->fps <= 0.0f) continue;

    animTimers[i] += GetFrameTime();
    if (animTimers[i] >= 1.0f / anim->fps) {
      animTimers[i] -= 1.0f / anim->fps;
      animIndices[i] = (animIndices[i] + 1) % anim->frames.animFrameCount;
    }
  }
}

void DrawAnimationsToRenderTexture(RenderTexture2D target,
                                   const Animation *anims, int animCount,
                                   int *animIndices,
                                   float scale, Vector2 virtualMouse) {
  (void)virtualMouse;
  (void)scale;

  BeginTextureMode(target);
  ClearBackground(DARKGRAY);


  // Draw each animation
  for (int i = 0; i < animCount; i++) {
    const Animation *anim = &anims[i];
    int idx = animIndices[i];

    if (!anim->active || anim->frames.animFrameCount == 0 ||
        idx >= anim->frames.animFrameCount) {
      continue;
    }

    DrawTexture(anim->frames.animTextures[idx], (int)anim->position.x,
                (int)anim->position.y, WHITE);
  }

  // Debug text
  DrawText(TextFormat("Virtual Mouse: [%i , %i]", (int)virtualMouse.x,
                      (int)virtualMouse.y),
           350, 55, 20, YELLOW);

  EndTextureMode();
}

void HandleClickDetection(const Animation *anims, int animCount,
                          int *animIndices, Vector2 virtualMouse) {
  if (!IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) return;

  for (int i = 0; i < animCount; i++) {
    const Animation *anim = &anims[i];
    if (!anim->active) continue;

    int idx = animIndices[i];
    if (idx < 0 || idx >= anim->frames.animFrameCount) continue;

    Vector2 size = anim->frames.animSizes[idx];
    if (virtualMouse.x >= anim->position.x &&
        virtualMouse.x < anim->position.x + size.x &&
        virtualMouse.y >= anim->position.y &&
        virtualMouse.y < anim->position.y + size.y) {
      int px = (int)(virtualMouse.x - anim->position.x);
      int py = (int)(virtualMouse.y - anim->position.y);
      Image img = anim->frames.animImages[idx];
      if (px >= 0 && px < img.width && py >= 0 && py < img.height) {
        uint8_t *pixels = (uint8_t *)img.data;
        uint8_t alpha = pixels[(py * img.width + px) * 4 + 3];
        if (alpha > 128) {
          TraceLog(LOG_INFO, "click on animation %d", i);
        }
      }
    }
  }
}

void PresentToScreen(RenderTexture2D target, float scale, int gameWidth,
                     int gameHeight) {
  BeginDrawing();
  ClearBackground(BLACK);
  DrawTexturePro(target.texture,
                 (Rectangle){0.0f, 0.0f, (float)target.texture.width,
                             (float)-target.texture.height},
                 (Rectangle){(GetScreenWidth() - gameWidth * scale) * 0.5f,
                             (GetScreenHeight() - gameHeight * scale) * 0.5f,
                             gameWidth * scale, gameHeight * scale},
                 (Vector2){0, 0}, 0.0f, WHITE);
  EndDrawing();
}

void Cleanup() {
  for (int i = 0; i < animCount; i++) {
    UnloadAnimation(&animations[i]);
  }
  CloseWindow();
}