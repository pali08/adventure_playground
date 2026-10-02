#include "entities.h"
#include "cJSON.h"
#include "entities.h"
#include "raylib.h"
#include <stdio.h>
#include <string.h>

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
    const Person *person = &persons[i];
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

static void CopyStr(char *dst, const char *src, size_t dstSize) {
  if (!src || !dst || dstSize == 0) {
    if (dst && dstSize > 0)
      dst[0] = '\0';
    return;
  }
  strncpy(dst, src, dstSize - 1);
  dst[dstSize - 1] = '\0';
}

// Helper: parse Vector2 from JSON array of 2 numbers
static Vector2 ParseVector2(cJSON *coords) {
  if (!coords || !cJSON_IsArray(coords) || cJSON_GetArraySize(coords) < 2)
    return (Vector2){0.0f, 0.0f};

  float x = (float)cJSON_GetNumberValue(cJSON_GetArrayItem(coords, 0));
  float y = (float)cJSON_GetNumberValue(cJSON_GetArrayItem(coords, 1));
  return (Vector2){x * GAME_WIDTH,
                   y * GAME_HEIGHT}; // convert to absolute coords
}

void LoadRoom(const char *jsonFile, Room *room, Item *items, int maxItems,
              Person *persons, int maxPersons) {
  if (!room || !jsonFile)
    return;

  // Reset room
  *room = (Room){0};

  // Load JSON file
  char *jsonText = LoadFileText(jsonFile);
  if (!jsonText) {
    TraceLog(LOG_ERROR, "Failed to load JSON file: %s", jsonFile);
    return;
  }

  cJSON *root = cJSON_Parse(jsonText);
  UnloadFileText(jsonText);
  if (!root) {
    TraceLog(LOG_ERROR, "Failed to parse JSON: %s", cJSON_GetErrorPtr());
    return;
  }

  // --- 1. Load Room metadata ---
  cJSON *background = cJSON_GetObjectItem(root, "background");
  CopyStr(room->background,
          cJSON_IsString(background) ? background->valuestring : "",
          sizeof(room->background));

  cJSON *sound = cJSON_GetObjectItem(root, "sound");
  CopyStr(room->sound, cJSON_IsString(sound) ? sound->valuestring : "",
          sizeof(room->sound));

  cJSON *firstVisitText = cJSON_GetObjectItem(root, "first_visit_text");
  CopyStr(room->firstVisitText,
          cJSON_IsString(firstVisitText) ? firstVisitText->valuestring : "",
          sizeof(room->firstVisitText));

  // --- 2. Load Items ---
  cJSON *jsonItems = cJSON_GetObjectItem(root, "items");
  room->itemCount = 0;
  if (cJSON_IsObject(jsonItems)) {
    cJSON *itemObj = NULL;
    cJSON_ArrayForEach(itemObj, jsonItems) {
      if (room->itemCount >= maxItems)
        break;

      Item *item = &items[room->itemCount];
      memset(item, 0, sizeof(Item));

      // name = key (e.g., "dead_body.png")
      CopyStr(item->name, itemObj->string, sizeof(item->name));

      // animation.position & active & fps
      cJSON *coords = cJSON_GetObjectItem(itemObj, "coords");
      item->anim.position = ParseVector2(coords);
      cJSON *visible = cJSON_GetObjectItem(itemObj, "visible");
      item->anim.active = cJSON_IsTrue(visible);
      cJSON *imgDuration = cJSON_GetObjectItem(itemObj, "img_duration");
      float fps = (cJSON_IsNumber(imgDuration) && imgDuration->valuedouble > 0)
                      ? 60.0f / (float)imgDuration->valuedouble
                      : 1.0f;
      item->anim.fps = fps;

      // Load animation frames (animation folder = item name without extension)
      char animFolder[MAX_PATH_LEN];
      const char *ext = strrchr(item->name, '.');
      if (ext) {
        int len = (int)(ext - item->name);
        if (len >= MAX_PATH_LEN)
          len = MAX_PATH_LEN - 1;
        strncpy(animFolder, item->name, len);
        animFolder[len] = '\0';
      } else {
        snprintf(animFolder, sizeof(animFolder), "%s/%s", ASSETS_ANIMATIONS_DIR,
                 item->name);
      }
      item->anim =
          LoadAnimation(animFolder, item->anim.fps, item->anim.position);
      if (!item->anim.active) {
        TraceLog(LOG_WARNING,
                 "Failed to load animation frames for item '%s' in folder '%s'",
                 item->name, animFolder);
        // still proceed (item visible but no frames)
      }

      // Copy optional fields
      CopyStr(item->description,
              cJSON_GetStringValue(cJSON_GetObjectItem(itemObj, "description")),
              sizeof(item->description));
      CopyStr(
          item->combinable_with,
          cJSON_GetStringValue(cJSON_GetObjectItem(itemObj, "combinable_with")),
          sizeof(item->combinable_with));
      CopyStr(item->resulting_item_name,
              cJSON_GetStringValue(
                  cJSON_GetObjectItem(itemObj, "resulting_item_name")),
              sizeof(item->resulting_item_name));
      CopyStr(item->type,
              cJSON_GetStringValue(cJSON_GetObjectItem(itemObj, "type")),
              sizeof(item->type));
      CopyStr(
          item->take_message,
          cJSON_GetStringValue(cJSON_GetObjectItem(itemObj, "take_message")),
          sizeof(item->take_message));
      CopyStr(
          item->combine_message,
          cJSON_GetStringValue(cJSON_GetObjectItem(itemObj, "combine_message")),
          sizeof(item->combine_message));

      // Handle nested type "searchable:X.png:Y.png" → fill
      // make_some_other_item...
      if (strcmp(item->type, "searchable") == 0) {
        const char *searchable =
            cJSON_GetStringValue(cJSON_GetObjectItem(itemObj, "type"));
        if (searchable && (searchable = strchr(searchable, ':')) != NULL) {
          searchable++; // skip ':'
          char *next = strchr(searchable, ':');
          if (next) {
            *next = '\0'; // temporarily null-terminate "searchable"
            CopyStr(item->make_some_other_item_in_room_takeable, searchable,
                    sizeof(item->make_some_other_item_in_room_takeable));
            *next = ':'; // restore
          }
        }
      }

      // Bool field
      cJSON *returnToInv =
          cJSON_GetObjectItem(itemObj, "return_to_inventory_after_combine");
      item->return_to_inventory_after_combine = cJSON_IsTrue(returnToInv);

      // Default: empty sound strings (not loaded from JSON — can add later)
      item->take_sound[0] = '\0';

      room->itemCount++;
    }
  }

  // --- 3. Load Persons ---
  cJSON *jsonPersons = cJSON_GetObjectItem(root, "persons");
  room->personCount = 0;
  if (cJSON_IsObject(jsonPersons)) {
    cJSON *personObj = NULL;
    cJSON_ArrayForEach(personObj, jsonPersons) {
      if (room->personCount >= maxPersons)
        break;

      Person *person = &persons[room->personCount];
      memset(person, 0, sizeof(Person));

      CopyStr(person->name, personObj->string, sizeof(person->name));

      cJSON *coords = cJSON_GetObjectItem(personObj, "coords");
      person->anim.position = ParseVector2(coords);
      cJSON *visible = cJSON_GetObjectItem(personObj, "visible");
      person->anim.active = cJSON_IsTrue(visible);
      cJSON *imgDuration = cJSON_GetObjectItem(personObj, "img_duration");
      float fps = (cJSON_IsNumber(imgDuration) && imgDuration->valuedouble > 0)
                      ? 60.0f / (float)imgDuration->valuedouble
                      : 1.0f;
      person->anim.fps = fps;

      char animFolder[MAX_PATH_LEN];
      snprintf(animFolder, sizeof(animFolder), "%s/%s", ASSETS_ANIMATIONS_DIR,
               person->name);
      person->anim =
          LoadAnimation(animFolder, person->anim.fps, person->anim.position);
      if (!person->anim.active) {
        TraceLog(
            LOG_WARNING,
            "Failed to load animation frames for person '%s' in folder '%s'",
            person->name, animFolder);
      }

      CopyStr(
          person->description,
          cJSON_GetStringValue(cJSON_GetObjectItem(personObj, "description")),
          sizeof(person->description));
      CopyStr(person->combinable_with,
              cJSON_GetStringValue(
                  cJSON_GetObjectItem(personObj, "combinable_with")),
              sizeof(person->combinable_with));
      CopyStr(
          person->dialogue_id,
          cJSON_GetStringValue(cJSON_GetObjectItem(personObj, "dialogue_id")),
          sizeof(person->dialogue_id));

      // Note: full dialogue struct not loaded yet (too complex for JSON alone —
      // will need custom parser later)
      room->personCount++;
    }
  }

  cJSON_Delete(root);
}