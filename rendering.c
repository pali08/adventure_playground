#include "rendering.h"
#include <stdint.h>

// ... existing includes ...

void DrawItemsToRenderTexture(RenderTexture2D target,
                              const Item *items, int itemCount,
                              const int *itemIndices) {
  BeginTextureMode(target);


  for (int i = 0; i < itemCount; i++) {
    const Animation *anim = &items[i].anim;
    int idx = itemIndices[i];

    if (!anim->active || anim->frames.animFrameCount == 0 || idx >= anim->frames.animFrameCount) {
      continue;
    }

    DrawTexture(anim->frames.animTextures[idx], (int)anim->position.x,
                (int)anim->position.y, WHITE);
  }

  EndTextureMode();
}

void DrawPersonsToRenderTexture(RenderTexture2D target,
                                const Person *persons, int personCount,
                                const int *personIndices) {

  // NOTE: This *overwrites* background. If you want layering, draw items first.
  BeginTextureMode(target);

  for (int i = 0; i < personCount; i++) {
    const Animation *anim = &persons[i].anim;
    int idx = personIndices[i];

    if (!anim->active || anim->frames.animFrameCount == 0 || idx >= anim->frames.animFrameCount) {
      continue;
    }

    DrawTexture(anim->frames.animTextures[idx], (int)anim->position.x,
                (int)anim->position.y, WHITE);
  }

  EndTextureMode();
}

void HandleItemClicks(const Item *items, int itemCount,
                      const int *itemIndices, Vector2 virtualMouse) {
  if (!IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) return;

  for (int i = 0; i < itemCount; i++) {
    const Animation *anim = &items[i].anim;
    if (!anim->active) continue;

    int idx = itemIndices[i];
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
          TraceLog(LOG_INFO, "click on item %d: %s", i, items[i].name);
        }
      }
    }
  }
}

void HandlePersonClicks(const Person *persons, int personCount,
                        const int *personIndices, Vector2 virtualMouse) {
  if (!IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) return;

  for (int i = 0; i < personCount; i++) {
    const Animation *anim = &persons[i].anim;
    if (!anim->active) continue;

    int idx = personIndices[i];
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
          TraceLog(LOG_INFO, "click on person %d: %s", i, persons[i].name);
        }
      }
    }
  }
}

// ... rest unchanged ...

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