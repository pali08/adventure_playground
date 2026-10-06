#include "rendering.h"
#include "raylib.h"
#include <math.h>
#include <stdint.h>

// ... existing includes ...

void DrawItemsToRenderTexture(RenderTexture2D target, const Item *items,
                              int itemCount, const int *itemIndices) {
  BeginTextureMode(target);

  for (int i = 0; i < itemCount; i++) {
    const Animation *anim = &items[i].anim;
    int idx = itemIndices[i];

    if (!anim->active || anim->frames.animFrameCount == 0 ||
        idx >= anim->frames.animFrameCount) {
      continue;
    }

    DrawTexture(anim->frames.animTextures[idx], (int)anim->position.x,
                (int)anim->position.y, WHITE);
  }

  EndTextureMode();
}

void DrawPersonsToRenderTexture(RenderTexture2D target, const Person *persons,
                                int personCount, const int *personIndices) {

  // NOTE: This *overwrites* background. If you want layering, draw items first.
  BeginTextureMode(target);

  for (int i = 0; i < personCount; i++) {
    const Animation *anim = &persons[i].anim;
    int idx = personIndices[i];

    if (!anim->active || anim->frames.animFrameCount == 0 ||
        idx >= anim->frames.animFrameCount) {
      continue;
    }

    DrawTexture(anim->frames.animTextures[idx], (int)anim->position.x,
                (int)anim->position.y, WHITE);
  }

  EndTextureMode();
}

// ... existing code ...

void DrawInventoryToRenderTexture(RenderTexture2D target,
                                  const Inventory *inventory, const Item *items,
                                  int itemCount, const int *itemIndices) {
  if (!inventory || !items || itemCount <= 0 || !itemIndices)
    return;

  // Calculate grid dimensions based on screen height
  int screenHeight = GetScreenHeight();
  int cellSize = screenHeight / 10; // 1/14th of screen height
  int gridHeight = inventory->rows * cellSize;
  int inventoryWidth = inventory->columns * cellSize;

  // Center horizontally
  int offsetX = (GetScreenWidth() - inventoryWidth) / 2;

  // Add a small top margin (5 pixels)
  int offsetY = INVENTORY_GRID_TOP_OFFSET;

  // Reserve space for grid lines and padding
  // const int INVENTORY_GRID_THICKNESS = 2; // define if not already defined
  // elsewhere
  int maxDrawSize = cellSize - INVENTORY_GRID_THICKNESS -
                    INVENTORY_GRID_ITEM_PADDING; // e.g., 78 - 2 - 2 = 74

  BeginTextureMode(target);

  // Draw grid lines
  for (int row = 0; row <= inventory->rows; row++) {
    int y = offsetY + row * cellSize;
    DrawLineEx((Vector2){offsetX, y}, (Vector2){offsetX + inventoryWidth, y},
               INVENTORY_GRID_THICKNESS, BLACK);
  }
  for (int col = 0; col <= inventory->columns; col++) {
    int x = offsetX + col * cellSize;
    DrawLineEx((Vector2){x, offsetY}, (Vector2){x, offsetY + gridHeight},
               INVENTORY_GRID_THICKNESS, BLACK);
  }

  // Draw items in grid order (row-major)
  for (int i = 0; i < itemCount; i++) {
    const Animation *anim = &items[i].anim;
    if (!anim->active || anim->frames.animFrameCount == 0)
      continue;

    int idx = itemIndices[i];
    if (idx < 0 || idx >= anim->frames.animFrameCount)
      continue;

    Texture2D frame = anim->frames.animTextures[idx];
    float w = (float)anim->frames.animSizes[idx].x;
    float h = (float)anim->frames.animSizes[idx].y;

    // Scaling logic: only scale if larger than maxDrawSize, otherwise keep
    // original
    float scale = 1.0f;
    if (w > maxDrawSize || h > maxDrawSize) {
      float scaleX = (float)maxDrawSize / w;
      float scaleY = (float)maxDrawSize / h;
      scale = fminf(scaleX, scaleY);
    }

    int drawW = (int)(w * scale);
    int drawH = (int)(h * scale);

    // Determine cell position (row-major)
    int row = i / inventory->columns;
    int col = i % inventory->columns;

    int cellX = offsetX + col * cellSize;
    int cellY = offsetY + row * cellSize;

    // Center in cell
    int cellCenterX = cellX + (cellSize - drawW) / 2;
    int cellCenterY = cellY + (cellSize - drawH) / 2;

    // Use sourceRect with positive height (no flipping)
    Rectangle sourceRect = {.x = 0,
                            .y = 0,
                            .width = (float)frame.width,
                            .height = (float)frame.height};

    Rectangle destRect = {
        .x = cellCenterX, .y = cellCenterY, .width = drawW, .height = drawH};

    DrawTexturePro(frame, sourceRect, destRect, (Vector2){0, 0}, 0, WHITE);
  }

  EndTextureMode();
}

void HandleItemClicks(const Item *items, int itemCount, const int *itemIndices,
                      Vector2 virtualMouse) {
  if (!IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
    return;

  for (int i = 0; i < itemCount; i++) {
    const Animation *anim = &items[i].anim;
    if (!anim->active)
      continue;

    int idx = itemIndices[i];
    if (idx < 0 || idx >= anim->frames.animFrameCount)
      continue;

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
  if (!IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
    return;

  for (int i = 0; i < personCount; i++) {
    const Animation *anim = &persons[i].anim;
    if (!anim->active)
      continue;

    int idx = personIndices[i];
    if (idx < 0 || idx >= anim->frames.animFrameCount)
      continue;

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

void HandleInventoryItemClicks(const Item *items, int itemCount,
                               const int *itemIndices,
                               const Inventory *inventory,
                               Vector2 virtualMouse) {
  if (!IsMouseButtonPressed(MOUSE_LEFT_BUTTON) || !inventory)
    return;

  // Calculate grid dimensions based on screen height
  int screenHeight = GetScreenHeight();
  int cellSize = screenHeight / 10;
  int gridHeight = inventory->rows * cellSize;
  int inventoryWidth = inventory->columns * cellSize;

  int offsetX = (GetScreenWidth() - inventoryWidth) / 2;
  int offsetY = INVENTORY_GRID_TOP_OFFSET;

  const int maxDrawSize =
      cellSize - INVENTORY_GRID_THICKNESS - INVENTORY_GRID_ITEM_PADDING;

  for (int i = 0; i < itemCount; i++) {
    const Animation *anim = &items[i].anim;
    if (!anim->active || anim->frames.animFrameCount == 0)
      continue;

    int idx = itemIndices[i];
    if (idx < 0 || idx >= anim->frames.animFrameCount)
      continue;

    // Compute draw size (exactly as in DrawInventoryToRenderTexture)
    Vector2 size = anim->frames.animSizes[idx];
    float w = (float)size.x;
    float h = (float)size.y;

    float scale = 1.0f;
    if (w > maxDrawSize || h > maxDrawSize) {
      float scaleX = (float)maxDrawSize / w;
      float scaleY = (float)maxDrawSize / h;
      scale = fminf(scaleX, scaleY);
    }

    int drawW = (int)(w * scale);
    int drawH = (int)(h * scale);

    // Determine cell and center position (exactly as in
    // DrawInventoryToRenderTexture)
    int row = i / inventory->columns;
    int col = i % inventory->columns;
    int cellX = offsetX + col * cellSize;
    int cellY = offsetY + row * cellSize;
    int cellCenterX = cellX + (cellSize - drawW) / 2;
    int cellCenterY = cellY + (cellSize - drawH) / 2;

    // Check if mouse is inside the *drawn* item rectangle
    if (virtualMouse.x >= cellCenterX && virtualMouse.x < cellCenterX + drawW &&
        virtualMouse.y >= cellCenterY && virtualMouse.y < cellCenterY + drawH) {
      // Map mouse coordinates to original image coordinates *as DrawTexturePro
      // does*
      float dx = virtualMouse.x - cellCenterX;
      float dy = virtualMouse.y - cellCenterY;

      Image img = anim->frames.animImages[idx];
      float srcX = dx * (float)img.width / drawW;
      float srcY = dy * (float)img.height / drawH;

      int px = (int)srcX;
      int py = (int)srcY;

      // Clamp to valid range (defensive, though math above should keep it in
      // bounds)
      if (px >= 0 && px < img.width && py >= 0 && py < img.height) {
        uint8_t *pixels = (uint8_t *)img.data;
        uint8_t alpha = pixels[(py * img.width + px) * 4 + 3];
        if (alpha > 128) {
          TraceLog(LOG_INFO, "click on inventory item %d: %s", i,
                   items[i].name);
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