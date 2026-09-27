#ifndef SCREEN_SETUP_H
#define SCREEN_SETUP_H

#include "raylib.h"

void InitializeWindow(int screenWidth, int screenHeight, int gameWidth, int gameHeight);
RenderTexture2D InitializeRenderTexture(int width, int height);
unsigned short *get_screen_size(void);

#endif // SCREEN_SETUP_H