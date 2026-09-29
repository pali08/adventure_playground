#include "animation.h"
#include "entities.h"
#include <dirent.h>
#include <stdio.h>
#include <string.h>

static AnimationFrameSet LoadAnimationFrameSet(const char *folder) {
  AnimationFrameSet ad = {0};

  DIR *dir = opendir(folder);
  if (!dir)
    return ad;

  int count = 0;
  struct dirent *entry;

  while ((entry = readdir(dir)) != NULL && count < MAX_FRAMES) {
    int len = strlen(entry->d_name);
    if (len > 4 && strcmp(entry->d_name + len - 4, ".png") == 0) {
      int folderLen = strlen(folder);
      if (folderLen + 1 + len < MAX_PATH_LEN) {
        sprintf(ad.animPaths[count], "%s/%s", folder, entry->d_name);

        Image img = LoadImage(ad.animPaths[count]);
        ad.animImages[count] = img;
        ad.animTextures[count] = LoadTextureFromImage(img);
        ad.animSizes[count] = (Vector2){(float)img.width, (float)img.height};
        count++;
      }
    }
  }
  closedir(dir);

  // Sort paths alphabetically
  for (int i = 0; i < count - 1; i++) {
    for (int j = i + 1; j < count; j++) {
      if (strcmp(ad.animPaths[i], ad.animPaths[j]) > 0) {
        char tmpPath[MAX_PATH_LEN];
        strcpy(tmpPath, ad.animPaths[i]);
        strcpy(ad.animPaths[i], ad.animPaths[j]);
        strcpy(ad.animPaths[j], tmpPath);

        Texture tmpTex = ad.animTextures[i];
        ad.animTextures[i] = ad.animTextures[j];
        ad.animTextures[j] = tmpTex;

        Image tmpImg = ad.animImages[i];
        ad.animImages[i] = ad.animImages[j];
        ad.animImages[j] = tmpImg;

        Vector2 tmpSz = ad.animSizes[i];
        ad.animSizes[i] = ad.animSizes[j];
        ad.animSizes[j] = tmpSz;
      }
    }
  }

  ad.animFrameCount = count;
  return ad;
}

Animation LoadAnimation(const char *folder, float fps, Vector2 position) {
  Animation anim = {0};
  anim.frames = LoadAnimationFrameSet(folder);
  anim.fps = fps;
  anim.position = position;
  anim.active = anim.frames.animFrameCount > 0;
  return anim;
}

void UnloadAnimation(Animation *anim) {
  if (!anim || !anim->active)
    return;
  for (int i = 0; i < anim->frames.animFrameCount; i++) {
    UnloadTexture(anim->frames.animTextures[i]);
    UnloadImage(anim->frames.animImages[i]);
  }
  anim->active = false;
  anim->frames.animFrameCount = 0;
}

// int LoadAnimations(Animation *animations) {
//   // Example: load 2 animations manually for now (you can populate from a
//   config) animations[0] = LoadAnimation("animation", 0.5f,
//   (Vector2){GAME_WIDTH * 0.25f, GAME_HEIGHT * 0.25f}); animations[1] =
//   LoadAnimation("animation2", 2.0f, (Vector2){GAME_WIDTH * 0.6f, GAME_HEIGHT
//   * 0.5f}); int animCount = 2; return animCount;
// }

void UpdateAnimations(float *animTimers, int *animIndices, int animCount,
                      const Animation *anims) {
  for (int i = 0; i < animCount; i++) {
    const Animation *anim = &anims[i];
    if (!anim->active || anim->fps <= 0.0f)
      continue;

    animTimers[i] += GetFrameTime();
    if (animTimers[i] >= 1.0f / anim->fps) {
      animTimers[i] -= 1.0f / anim->fps;
      animIndices[i] = (animIndices[i] + 1) % anim->frames.animFrameCount;
    }
  }
}