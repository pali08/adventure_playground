#ifndef RENDERING_H
#define RENDERING_H

#include "animation.h"
#include "entities.h"
#include "raylib.h"

// Draw
void DrawItemsToRenderTexture(RenderTexture2D target, const Item *items,
                              int itemCount);

void DrawPersonsToRenderTexture(RenderTexture2D target, const Person *persons,
                                int personCount);

void DrawInventoryToRenderTexture(RenderTexture2D target,
                                  const Inventory *inventory, const Item *items,
                                  int itemCount);

// Present
void PresentToScreen(RenderTexture2D target, float scale, int gameWidth,
                     int gameHeight);

#endif // RENDERING_H