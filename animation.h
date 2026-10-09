// animation.h
#ifndef ANIMATION_H
#define ANIMATION_H

#include "raylib.h"
#include "constants.h"

typedef struct AnimationFrameSet {
  Texture animTextures[MAX_FRAMES];
  Image animImages[MAX_FRAMES];
  Vector2 animSizes[MAX_FRAMES];
  char animPaths[MAX_FRAMES][MAX_PATH_LEN];
  int animFrameCount;
} AnimationFrameSet;

typedef struct {
  AnimationFrameSet frames;
  float fps;
  Vector2 position;
  float animTimer;   // runtime timer
  int animIndex;     // current frame index
  bool active;       // optional, for toggling animation
} Animation;

Animation LoadAnimation(const char *folder, float fps, Vector2 position);
void UnloadAnimation(Animation *anim);

#endif // ANIMATION_H