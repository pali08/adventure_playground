#include "entities.h"
#include "raylib.h"
// #include <string.h>

// --- Item loading ---
Item LoadItem(const char *animFolder, float animFPS, Vector2 animPosition) {
  Item item = {0};
  item.anim = LoadAnimation(animFolder, animFPS, animPosition);
  item.anim.active = true;
  item.return_to_inventory_after_combine = false;
  return item;
}

int LoadItems(Item *items, int maxCount) {
  if (maxCount < 1)
    return 0;
  items[0] = LoadItem("animation", 0.5,
                      (Vector2){GAME_WIDTH * 0.25f, GAME_HEIGHT * 0.25f});
  items[1] = LoadItem("animation2", 2,
                      (Vector2){GAME_WIDTH * 0.5f, GAME_HEIGHT * 0.5f});
  return 2;
}

// --- Person loading ---
Person LoadPerson(const char *animFolder, float animFPS, Vector2 animPosition) {
  Person person = {0};
  person.anim = LoadAnimation(animFolder, animFPS, animPosition);
  return person;
}

int LoadPersons(Person *persons, int maxCount) {
  if (maxCount < 1)
    return 0;
  persons[0] = LoadPerson("animation_person", 1,
                          (Vector2){GAME_WIDTH * 0.7f, GAME_HEIGHT * 0.7f});
  return 1;
}

void UpdateItems(float *animTimers, int *animIndices, int animCount,
                 const Item *items) {
  for (int i = 0; i < animCount; i++) {
    const Item *item = &items[i];
    if (!item->anim.active || item->anim.fps <= 0.0f)
      continue;

    animTimers[i] += GetFrameTime();
    if (animTimers[i] >= 1.0f / item->anim.fps) {
      animTimers[i] -= 1.0f / item->anim.fps;
      animIndices[i] = (animIndices[i] + 1) % item->anim.frames.animFrameCount;
    }
  }
}

void UpdatePersons(float *animTimers, int *animIndices, int animCount,
                   const Person *persons) {
  for (int i = 0; i < animCount; i++) {
    const Person *person = &person[i];
    if (!person->anim.active || person->anim.fps <= 0.0f)
      continue;

    animTimers[i] += GetFrameTime();
    if (animTimers[i] >= 1.0f / person->anim.fps) {
      animTimers[i] -= 1.0f / person->anim.fps;
      animIndices[i] =
          (animIndices[i] + 1) % person->anim.frames.animFrameCount;
    }
  }
}