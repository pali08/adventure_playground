#ifndef ANIMATION_H
#define ANIMATION_H

#include "raylib.h"
#include "constants.h"

// Internal: holds frames (textures, images, sizes, paths) for *one* animation
typedef struct AnimationFrameSet {
  Texture animTextures[MAX_FRAMES];
  Image animImages[MAX_FRAMES];
  Vector2 animSizes[MAX_FRAMES];
  char animPaths[MAX_FRAMES][MAX_PATH_LEN];
  int animFrameCount;
} AnimationFrameSet;

// Single animation definition: its frames + playback & position
typedef struct {
  AnimationFrameSet frames;
  float fps;
  Vector2 position;
  bool active; // to enable/disable animations
} Animation;

Animation LoadAnimation(const char *folder, float fps, Vector2 position);
void UnloadAnimation(Animation *anim);
int LoadAnimations(Animation *animations);
void UpdateAnimations(float *animTimers, int *animIndices, int animCount, const Animation *anims);

#endif // ANIMATION_H