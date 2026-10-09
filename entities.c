#include "entities.h"
#include "cJSON.h"
#include "constants.h"
#include "entities.h"
#include "raylib.h"
#include <stdio.h>
#include <string.h>

// Replace SetFrame() with:
void UpdateAnimation(Animation *anim) {
  if (!anim->active || anim->frames.animFrameCount <= 1 || anim->fps <= 0.0f)
    return;

  anim->animTimer += GetFrameTime();
  if (anim->animTimer >= 1.0f / anim->fps) {
    anim->animTimer -= 1.0f / anim->fps;
    anim->animIndex = (anim->animIndex + 1) % anim->frames.animFrameCount;
  }
}

void UpdateItems(int itemCount, Item *items) {
  for (int i = 0; i < itemCount; i++) {
    UpdateAnimation(&items[i].anim);
  }
}

void UpdatePersons(int personCount, Person *persons) {
  for (int i = 0; i < personCount; i++) {
    UpdateAnimation(&persons[i].anim);
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

static void LoadRoomMetadata(const cJSON *root, Room *room) {
  if (!root || !room)
    return;

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
}

static void LoadItemFromJSON(const cJSON *itemObj, Item *items,
                             Item *invisibleItems, int *itemCount,
                             int *invisibleItemCount, int maxItems,
                             int maxInvisibleItems) {
  if (!itemObj || !items || !invisibleItems || !itemCount ||
      !invisibleItemCount || *itemCount >= maxItems ||
      *invisibleItemCount >= maxInvisibleItems)
    return;

  // Load into temp first
  Item temp = {0};
  CopyStr(temp.name, itemObj->string, sizeof(temp.name));

  cJSON *coords = cJSON_GetObjectItem(itemObj, "coords");
  temp.anim.position = ParseVector2(coords);

  cJSON *imgDuration = cJSON_GetObjectItem(itemObj, "img_duration");
  float fps = (cJSON_IsNumber(imgDuration) && imgDuration->valuedouble > 0)
                  ? 60.0f / (float)imgDuration->valuedouble
                  : 1.0f;
  temp.anim.fps = fps;

  char animFolder[MAX_PATH_LEN];
  const char *ext = strrchr(temp.name, '.');
  if (ext) {
    int len = (int)(ext - temp.name);
    if (len >= MAX_PATH_LEN)
      len = MAX_PATH_LEN - 1;
    strncpy(animFolder, temp.name, len);
    animFolder[len] = '\0';
  } else {
    snprintf(animFolder, sizeof(animFolder), "%s/%s", ASSETS_ANIMATIONS_DIR,
             temp.name);
  }

  temp.anim = LoadAnimation(animFolder, fps, temp.anim.position);

  cJSON *visible = cJSON_GetObjectItem(itemObj, "visible");
  temp.anim.active = cJSON_IsTrue(visible);

  if (!temp.anim.active) {
    TraceLog(LOG_WARNING,
             "Failed to load animation frames for item '%s' in folder '%s'",
             temp.name, animFolder);
  }

  CopyStr(temp.description,
          cJSON_GetStringValue(cJSON_GetObjectItem(itemObj, "description")),
          sizeof(temp.description));
  CopyStr(temp.combinable_with,
          cJSON_GetStringValue(cJSON_GetObjectItem(itemObj, "combinable_with")),
          sizeof(temp.combinable_with));
  CopyStr(
      temp.resulting_item_name,
      cJSON_GetStringValue(cJSON_GetObjectItem(itemObj, "resulting_item_name")),
      sizeof(temp.resulting_item_name));
  CopyStr(temp.type, cJSON_GetStringValue(cJSON_GetObjectItem(itemObj, "type")),
          sizeof(temp.type));
  CopyStr(temp.take_message,
          cJSON_GetStringValue(cJSON_GetObjectItem(itemObj, "take_message")),
          sizeof(temp.take_message));
  CopyStr(temp.combine_message,
          cJSON_GetStringValue(cJSON_GetObjectItem(itemObj, "combine_message")),
          sizeof(temp.combine_message));

  cJSON *returnToInv =
      cJSON_GetObjectItem(itemObj, "return_to_inventory_after_combine");
  temp.return_to_inventory_after_combine = cJSON_IsTrue(returnToInv);

  temp.take_sound[0] = '\0'; // placeholder

  // Place into visible or invisible arrays
  if (temp.anim.active) {
    if (*itemCount < maxItems) {
      items[*itemCount] = temp;
      (*itemCount)++;
    }
  } else {
    if (*invisibleItemCount < maxInvisibleItems) {
      invisibleItems[*invisibleItemCount] = temp;
      (*invisibleItemCount)++;
    }
  }
}

static void LoadPersonFromJSON(const cJSON *personObj, Person *persons,
                               Person *invisiblePersons, int *personCount,
                               int *invisiblePersonCount, int maxPersons,
                               int maxInvisiblePersons) {
  if (!personObj || !persons || !invisiblePersons || !personCount ||
      !invisiblePersonCount || *personCount >= maxPersons ||
      *invisiblePersonCount >= maxInvisiblePersons)
    return;

  Person temp = {0};
  CopyStr(temp.name, personObj->string, sizeof(temp.name));

  cJSON *coords = cJSON_GetObjectItem(personObj, "coords");
  temp.anim.position = ParseVector2(coords);

  cJSON *imgDuration = cJSON_GetObjectItem(personObj, "img_duration");
  float fps = (cJSON_IsNumber(imgDuration) && imgDuration->valuedouble > 0)
                  ? 60.0f / (float)imgDuration->valuedouble
                  : 1.0f;
  temp.anim.fps = fps;

  char animFolder[MAX_PATH_LEN];
  snprintf(animFolder, sizeof(animFolder), "%s/%s", ASSETS_ANIMATIONS_DIR,
           temp.name);
  temp.anim = LoadAnimation(animFolder, fps, temp.anim.position);

  cJSON *visible = cJSON_GetObjectItem(personObj, "visible");
  temp.anim.active = cJSON_IsTrue(visible);

  CopyStr(temp.description,
          cJSON_GetStringValue(cJSON_GetObjectItem(personObj, "description")),
          sizeof(temp.description));
  CopyStr(
      temp.combinable_with,
      cJSON_GetStringValue(cJSON_GetObjectItem(personObj, "combinable_with")),
      sizeof(temp.combinable_with));
  CopyStr(temp.dialogue_id,
          cJSON_GetStringValue(cJSON_GetObjectItem(personObj, "dialogue_id")),
          sizeof(temp.dialogue_id));

  // Place into visible or invisible arrays
  if (temp.anim.active) {
    if (*personCount < maxPersons) {
      persons[*personCount] = temp;
      (*personCount)++;
    }
  } else {
    if (*invisiblePersonCount < maxInvisiblePersons) {
      invisiblePersons[*invisiblePersonCount] = temp;
      (*invisiblePersonCount)++;
    }
  }
}


// Update existing room loader to use it:
static void LoadItemsFromJSONRoom(const cJSON *jsonItems, Room *room,
                                  Item *items, Item *invisibleItems,
                                  int maxItems, int maxInvisibleItems) {
  if (!jsonItems || !room || !items || !invisibleItems) return;

  room->itemCount = 0;
  room->invisibleItemCount = 0;

  if (cJSON_IsObject(jsonItems)) {
    cJSON *itemObj = NULL;
    cJSON_ArrayForEach(itemObj, jsonItems) {
      LoadItemFromJSON(itemObj, items, invisibleItems, &room->itemCount,
                       &room->invisibleItemCount, maxItems, maxInvisibleItems);
    }
  }
}

static void LoadItemsFromJSONInventory(const cJSON *jsonItems,
                                       Inventory *inventory, Item *items,
                                       Item *invisibleItems, int maxItems,
                                       int maxInvisibleItems) {
  if (!jsonItems || !inventory || !items || !invisibleItems) return;
  inventory->itemCount = 0;
  inventory->invisibleItemCount = 0;
  if (cJSON_IsObject(jsonItems)) {
    cJSON *itemObj = NULL;
    cJSON_ArrayForEach(itemObj, jsonItems) {
      LoadItemFromJSON(itemObj, items, invisibleItems,
                       &inventory->itemCount, &inventory->invisibleItemCount,
                       maxItems, maxInvisibleItems);
    }
  }
}

static void LoadPersonsFromJSON(const cJSON *jsonPersons, Room *room,
                                Person *persons, Person *invisiblePersons,
                                int maxPersons, int maxInvisiblePersons) {
  if (!jsonPersons || !room || !persons || !invisiblePersons) return;

  room->personCount = 0;
  room->invisiblePersonCount = 0;

  if (cJSON_IsObject(jsonPersons)) {
    cJSON *personObj = NULL;
    cJSON_ArrayForEach(personObj, jsonPersons) {
      LoadPersonFromJSON(personObj, persons, invisiblePersons,
                         &room->personCount, &room->invisiblePersonCount,
                         maxPersons, maxInvisiblePersons);
    }
  }
}

// --- Top-level room loader (refactored) ---
void LoadRoom(const char *jsonFile, Room *room, Item *items,
              Item *invisibleItems, int maxItems, int maxInvisibleItems,
              Person *persons, Person *invisiblePersons, int maxPersons,
              int maxInvisiblePersons) {
  if (!room || !jsonFile) return;

  *room = (Room){0};

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

  LoadRoomMetadata(root, room);
  LoadItemsFromJSONRoom(cJSON_GetObjectItem(root, "items"), room, items,
                        invisibleItems, maxItems, maxInvisibleItems);
  LoadPersonsFromJSON(cJSON_GetObjectItem(root, "persons"), room, persons,
                      invisiblePersons, maxPersons, maxInvisiblePersons);

  cJSON_Delete(root);
}

void LoadInventory(const char *jsonFile, Inventory *inventory, Item *items,
                   Item *invisibleItems, int maxItems, int maxInvisibleItems) {
  if (!inventory || !jsonFile || !items || !invisibleItems) return;

  *inventory = (Inventory){0};
  inventory->rows = INVENTORY_ROWS;
  inventory->columns = INVENTORY_COLUMNS;

  char *jsonText = LoadFileText(jsonFile);
  if (!jsonText) {
    TraceLog(LOG_ERROR, "Failed to load JSON inventory file: %s", jsonFile);
    return;
  }

  cJSON *root = cJSON_Parse(jsonText);
  UnloadFileText(jsonText);
  if (!root) {
    TraceLog(LOG_ERROR, "Failed to parse inventory JSON: %s",
             cJSON_GetErrorPtr());
    return;
  }

  LoadItemsFromJSONInventory(cJSON_GetObjectItem(root, "items"), inventory,
                             items, invisibleItems, maxItems,
                             maxInvisibleItems);

  cJSON_Delete(root);
}