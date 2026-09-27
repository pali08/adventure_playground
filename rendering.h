#ifndef RENDERING_H
#define RENDERING_H

#include "animation.h"
#include "raylib.h"

void DrawAnimationsToRenderTexture(RenderTexture2D target,
                                   const Animation *anims, int animCount,
                                   int *animIndices, float scale,
                                   Vector2 virtualMouse);

void HandleClickDetection(const Animation *anims, int animCount,
                          int *animIndices, Vector2 virtualMouse);

void PresentToScreen(RenderTexture2D target, float scale, int gameWidth,
                     int gameHeight);

#endif // RENDERING_H