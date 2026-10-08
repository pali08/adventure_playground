

#include "animation.h"
#include "entities.h"
#include <math.h>
#include <stdint.h>

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
