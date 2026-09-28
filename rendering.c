#include "rendering.h"
#include <stdint.h>

void DrawAnimationsToRenderTexture(RenderTexture2D target,
                                   const Animation *anims, int animCount,
                                   int *animIndices, float scale,
                                   Vector2 virtualMouse, Texture2D background) {
  (void)virtualMouse;
  (void)scale;

  BeginTextureMode(target);
  ClearBackground(DARKGRAY);

  // Draw background filling the render texture size
  // DrawTexturePro(background,
  //                (Rectangle){0, 0, (float)background.width, (float)-background.height},
  //                (Rectangle){0, 0, (float)target.texture.width, (float)target.texture.height},
  //                (Vector2){0, 0}, 0.0f, WHITE);
  DrawTexture(background, 0, 0, WHITE);


  for (int i = 0; i < animCount; i++) {
    const Animation *anim = &anims[i];
    int idx = animIndices[i];

    if (!anim->active || anim->frames.animFrameCount == 0 || idx >= anim->frames.animFrameCount) {
      continue;
    }

    DrawTexture(anim->frames.animTextures[idx], (int)anim->position.x,
                (int)anim->position.y, WHITE);
  }

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