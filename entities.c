#include "entities.h"
#include "cJSON.h"
#include "constants.h"
#include "entities.h"
#include "raylib.h"
#include <stdio.h>
#include <string.h>

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

static void LoadItemFromJSON(const cJSON *itemObj, Item *item, size_t itemIndex,
                             int maxItems) {
  if (!itemObj || !item || itemIndex >= maxItems)
    return;

  memset(item, 0, sizeof(Item));

  // name = key
  CopyStr(item->name, itemObj->string, sizeof(item->name));

  // position
  cJSON *coords = cJSON_GetObjectItem(itemObj, "coords");
  item->anim.position = ParseVector2(coords);

  // visibility
  cJSON *visible = cJSON_GetObjectItem(itemObj, "visible");
  item->anim.active = cJSON_IsTrue(visible);

  // FPS
  cJSON *imgDuration = cJSON_GetObjectItem(itemObj, "img_duration");
  float fps = (cJSON_IsNumber(imgDuration) && imgDuration->valuedouble > 0)
                  ? 60.0f / (float)imgDuration->valuedouble
                  : 1.0f;
  item->anim.fps = fps;

  // Load animation folder: strip extension from name
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

  item->anim = LoadAnimation(animFolder, item->anim.fps, item->anim.position);
  if (!item->anim.active) {
    TraceLog(LOG_WARNING,
             "Failed to load animation frames for item '%s' in folder '%s'",
             item->name, animFolder);
  }

  // Optional fields
  CopyStr(item->description,
          cJSON_GetStringValue(cJSON_GetObjectItem(itemObj, "description")),
          sizeof(item->description));
  CopyStr(item->combinable_with,
          cJSON_GetStringValue(cJSON_GetObjectItem(itemObj, "combinable_with")),
          sizeof(item->combinable_with));
  CopyStr(
      item->resulting_item_name,
      cJSON_GetStringValue(cJSON_GetObjectItem(itemObj, "resulting_item_name")),
      sizeof(item->resulting_item_name));
  CopyStr(item->type,
          cJSON_GetStringValue(cJSON_GetObjectItem(itemObj, "type")),
          sizeof(item->type));
  CopyStr(item->take_message,
          cJSON_GetStringValue(cJSON_GetObjectItem(itemObj, "take_message")),
          sizeof(item->take_message));
  CopyStr(item->combine_message,
          cJSON_GetStringValue(cJSON_GetObjectItem(itemObj, "combine_message")),
          sizeof(item->combine_message));

  // Special "searchable:X.png:Y.png" handling
  if (strcmp(item->type, "searchable") == 0) {
    const char *searchable =
        cJSON_GetStringValue(cJSON_GetObjectItem(itemObj, "type"));
    if (searchable && (searchable = strchr(searchable, ':')) != NULL) {
      searchable++;
      char *next = strchr(searchable, ':');
      if (next) {
        *next = '\0';
        CopyStr(item->make_some_other_item_in_room_takeable, searchable,
                sizeof(item->make_some_other_item_in_room_takeable));
        *next = ':';
      }
    }
  }

  // Bool
  cJSON *returnToInv =
      cJSON_GetObjectItem(itemObj, "return_to_inventory_after_combine");
  item->return_to_inventory_after_combine = cJSON_IsTrue(returnToInv);

  // Sound placeholder
  item->take_sound[0] = '\0';
}

static int LoadItemsFromJSONGeneric(const cJSON *jsonItems, Item *items,
                                    int maxItems) {
  if (!jsonItems || !items || maxItems <= 0)
    return 0;

  int count = 0;
  if (cJSON_IsObject(jsonItems)) {
    cJSON *itemObj = NULL;
    cJSON_ArrayForEach(itemObj, jsonItems) {
      if (count >= maxItems)
        break;
      LoadItemFromJSON(itemObj, &items[count], count, maxItems);
      count++;
    }
  } else if (cJSON_IsArray(jsonItems)) {
    // Handle array format (e.g., [ {"coords":[0.5,0.5], ...}, ... ])
    for (int i = 0; i < cJSON_GetArraySize(jsonItems); i++) {
      cJSON *itemObj = cJSON_GetArrayItem(jsonItems, i);
      if (!cJSON_IsObject(itemObj))
        continue;
      if (count >= maxItems)
        break;
      LoadItemFromJSON(itemObj, &items[count], count, maxItems);
      count++;
    }
  }
  return count;
}

// Update existing room loader to use it:
static void LoadItemsFromJSONRoom(const cJSON *jsonItems, Room *room,
                                  Item *items, int maxItems) {
  if (!jsonItems || !room || !items || maxItems <= 0)
    return;
  room->itemCount = LoadItemsFromJSONGeneric(jsonItems, items, maxItems);
}

static void LoadItemsFromJSONInventory(const cJSON *jsonItems,
                                       Inventory *inventory, Item *items,
                                       int maxItems) {
  if (!jsonItems || !items || maxItems <= 0)
    return;
  inventory->itemCount = LoadItemsFromJSONGeneric(jsonItems, items, maxItems);
}

static void LoadPersonFromJSON(const cJSON *personObj, Person *person,
                               size_t personIndex, int maxPersons) {
  if (!personObj || !person || personIndex >= maxPersons)
    return;

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
    TraceLog(LOG_WARNING,
             "Failed to load animation frames for person '%s' in folder '%s'",
             person->name, animFolder);
  }

  CopyStr(person->description,
          cJSON_GetStringValue(cJSON_GetObjectItem(personObj, "description")),
          sizeof(person->description));
  CopyStr(
      person->combinable_with,
      cJSON_GetStringValue(cJSON_GetObjectItem(personObj, "combinable_with")),
      sizeof(person->combinable_with));
  CopyStr(person->dialogue_id,
          cJSON_GetStringValue(cJSON_GetObjectItem(personObj, "dialogue_id")),
          sizeof(person->dialogue_id));
}

static void LoadPersonsFromJSON(const cJSON *jsonPersons, Room *room,
                                Person *persons, int maxPersons) {
  if (!jsonPersons || !room || !persons || maxPersons <= 0)
    return;

  room->personCount = 0;
  if (cJSON_IsObject(jsonPersons)) {
    cJSON *personObj = NULL;
    cJSON_ArrayForEach(personObj, jsonPersons) {
      if (room->personCount >= maxPersons)
        break;

      LoadPersonFromJSON(personObj, &persons[room->personCount],
                         room->personCount, maxPersons);
      room->personCount++;
    }
  }
}

// --- Top-level room loader (refactored) ---
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

  LoadRoomMetadata(root, room);
  LoadItemsFromJSONRoom(cJSON_GetObjectItem(root, "items"), room, items, maxItems);
  LoadPersonsFromJSON(cJSON_GetObjectItem(root, "persons"), room, persons,
                      maxPersons);

  cJSON_Delete(root);
}

void LoadInventory(const char *jsonFile, Inventory *inventory, Item *items,
                   int maxItems) {
  if (!inventory || !jsonFile || !items || maxItems <= 0)
    return;

  *inventory = (Inventory){0};

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

  // cJSON *jsonItems = cJSON_GetObjectItem(root, "items");
  // if (jsonItems) {
  //  inventory->itemCount = LoadItemsFromJSONGeneric(jsonItems, items, maxItems);
  //}
  LoadItemsFromJSONInventory(cJSON_GetObjectItem(root, "items"), inventory, items, maxItems);


  cJSON_Delete(root);
}