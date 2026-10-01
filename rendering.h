#ifndef RENDERING_H
#define RENDERING_H

#include "animation.h"
#include "entities.h"
#include "raylib.h"

// Draw
void DrawItemsToRenderTexture(RenderTexture2D target,
                              const Item *items, int itemCount,
                              const int *itemIndices);

void DrawPersonsToRenderTexture(RenderTexture2D target,
                                const Person *persons, int personCount,
                                const int *personIndices);

// Click handling
void HandleItemClicks(const Item *items, int itemCount,
                      const int *itemIndices, Vector2 virtualMouse);

void HandlePersonClicks(const Person *persons, int personCount,
                        const int *personIndices, Vector2 virtualMouse);

// Present
void PresentToScreen(RenderTexture2D target, float scale, int gameWidth,
                     int gameHeight);

#endif // RENDERING_H