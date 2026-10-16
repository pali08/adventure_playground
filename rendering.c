#include "rendering.h"
#include "raylib.h"
#include <math.h>
#include <stdint.h>

void DrawItemsToRenderTexture(RenderTexture2D target, const Item *items,
                              int itemCount) {
  BeginTextureMode(target);

  for (int i = 0; i < itemCount; i++) {
    const Animation *anim = &items[i].anim;
    if (!anim->active || anim->frames.animFrameCount == 0)
      continue;

    DrawTexture(
        anim->frames.animTextures[anim->animIndex], // ← use anim->animIndex
        (int)anim->position.x, (int)anim->position.y, WHITE);
  }

  EndTextureMode();
}

void DrawPersonsToRenderTexture(RenderTexture2D target, const Person *persons,
                                int personCount) {

  BeginTextureMode(target);

  for (int i = 0; i < personCount; i++) {
    const Animation *anim = &persons[i].anim;
    if (!anim->active || anim->frames.animFrameCount == 0)
      continue;

    DrawTexture(
        anim->frames.animTextures[anim->animIndex], // ← use anim->animIndex
        (int)anim->position.x, (int)anim->position.y, WHITE);
  }

  EndTextureMode();
}

static void GetItemDrawRect(const Animation *anim, int maxDrawSize,
                            Vector2 center, Rectangle *destRect) {
  if (!anim || !destRect || anim->frames.animFrameCount == 0) {
    if (destRect) {
      *destRect = (Rectangle){0, 0, 0, 0};
    }
    return;
  }

  int idx = anim->animIndex;
  if (idx < 0 || idx >= anim->frames.animFrameCount) {
    *destRect = (Rectangle){0, 0, 0, 0};
    return;
  }

  float w = (float)anim->frames.animSizes[idx].x;
  float h = (float)anim->frames.animSizes[idx].y;

  float scale = 1.0f;
  if (w > maxDrawSize || h > maxDrawSize) {
    float scaleX = (float)maxDrawSize / w;
    float scaleY = (float)maxDrawSize / h;
    scale = fminf(scaleX, scaleY);
  }

  int drawW = (int)(w * scale);
  int drawH = (int)(h * scale);

  destRect->x = center.x - drawW / 2.0f;
  destRect->y = center.y - drawH / 2.0f;
  destRect->width = drawW;
  destRect->height = drawH;
}

void DrawInventoryToRenderTexture(RenderTexture2D target,
                                  const Inventory *inventory, const Item *items,
                                  int itemCount) {
  if (!inventory || !items || itemCount <= 0)
    return;

  int screenHeight = GetScreenHeight();
  int cellSize = screenHeight / 10;
  int gridHeight = inventory->rows * cellSize;
  int inventoryWidth = inventory->columns * cellSize;

  int offsetX = (GetScreenWidth() - inventoryWidth) / 2;
  int offsetY = INVENTORY_GRID_TOP_OFFSET;

  const int maxDrawSize =
      cellSize - INVENTORY_GRID_THICKNESS - INVENTORY_GRID_ITEM_PADDING;

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

  // Draw items
  for (int i = 0; i < itemCount; i++) {
    const Animation *anim = &items[i].anim;
    if (!anim->active || anim->frames.animFrameCount == 0)
      continue;

    int idx = anim->animIndex;
    if (idx < 0 || idx >= anim->frames.animFrameCount)
      continue;

    Texture2D frame = anim->frames.animTextures[idx];

    int row = i / inventory->columns;
    int col = i % inventory->columns;
    int cellX = offsetX + col * cellSize;
    int cellY = offsetY + row * cellSize;
    Vector2 cellCenter = {(float)(cellX + cellSize / 2.0),
                          (float)(cellY + cellSize / 2.0)};

    Rectangle destRect;
    GetItemDrawRect(anim, maxDrawSize, cellCenter, &destRect);

    Rectangle sourceRect = {.x = 0,
                            .y = 0,
                            .width = (float)frame.width,
                            .height = (float)frame.height};

    DrawTexturePro(frame, sourceRect, destRect, (Vector2){0, 0}, 0, WHITE);
  }
  EndTextureMode();
}

void DrawCurrentItemToRenderTexture(RenderTexture2D target,
                                    const Item *currentItem,
                                    Vector2 mousePosition) {
  if (!currentItem || !currentItem->anim.active ||
      currentItem->anim.frames.animFrameCount == 0)
    return;

  const Animation *anim = &currentItem->anim;

  int cellSize = GetScreenHeight() / 10;
  int maxDrawSize =
      cellSize - INVENTORY_GRID_THICKNESS - INVENTORY_GRID_ITEM_PADDING;

  Vector2 center = mousePosition;
  Rectangle destRect;
  GetItemDrawRect(anim, maxDrawSize, center, &destRect);

  int idx = anim->animIndex;
  if (idx < 0 || idx >= anim->frames.animFrameCount)
    return;

  Texture2D frame = anim->frames.animTextures[idx];

  Rectangle sourceRect = {.x = 0,
                          .y = 0,
                          .width = (float)frame.width,
                          .height = (float)frame.height};

  BeginTextureMode(target);
  DrawTexturePro(frame, sourceRect, destRect, (Vector2){0, 0}, 0, WHITE);
  EndTextureMode();
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