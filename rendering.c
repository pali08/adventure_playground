#include "rendering.h"
#include <math.h>
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

// ... existing code ...

void DrawInventoryToRenderTexture(RenderTexture2D target,
                                  const Inventory *inventory,
                                  const Item *items,
                                  int itemCount,
                                  const int *itemIndices) {
  if (!inventory || !items || itemCount <= 0 || !itemIndices) return;

  int cellSize = GAME_WIDTH / inventory->columns; // 1920 / 10 = 192
  int gridHeight = inventory->rows * cellSize;

  BeginTextureMode(target);

  // Draw grid lines (only in top `gridHeight` area)
  for (int row = 0; row <= inventory->rows; row++) {
    int y = row * cellSize;
    DrawLine(0, y, GAME_WIDTH, y, BLACK);
  }
  for (int col = 0; col <= inventory->columns; col++) {
    int x = col * cellSize;
    DrawLine(x, 0, x, gridHeight, BLACK);
  }

  // Draw items in grid order (row-major)
  for (int i = 0; i < itemCount; i++) {
    const Animation *anim = &items[i].anim;
    if (!anim->active || anim->frames.animFrameCount == 0) continue;

    int idx = itemIndices[i];
    if (idx < 0 || idx >= anim->frames.animFrameCount) continue;

    Texture2D frame = anim->frames.animTextures[idx];
    float w = (float)anim->frames.animSizes[idx].x;
    float h = (float)anim->frames.animSizes[idx].y;

    // Calculate scale to fit into 192x192 without exceeding it
    float scale = 1.0f;
    if (w > cellSize || h > cellSize) {
      float scaleX = cellSize / w;
      float scaleY = cellSize / h;
      scale = fminf(scaleX, scaleY);
    }

    int drawW = (int)(w * scale);
    int drawH = (int)(h * scale);

    // Determine cell position (row-major)
    int row = i / inventory->columns;
    int col = i % inventory->columns;

    int cellX = col * cellSize;
    int cellY = row * cellSize;

    // Center in cell
    int offsetX = (cellSize - drawW) / 2;
    int offsetY = (cellSize - drawH) / 2;

    Rectangle destRect = {
        .x = cellX + offsetX,
        .y = cellY + offsetY,
        .width = drawW,
        .height = drawH
    };

    // Use sourceRect with positive height (no flipping)
    Rectangle sourceRect = {
        .x = 0,
        .y = 0,
        .width = (float)frame.width,
        .height = (float)frame.height
    };

    DrawTexturePro(frame, sourceRect, destRect, (Vector2){0, 0}, 0, WHITE);
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