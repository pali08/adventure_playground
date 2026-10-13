#include "entities.h"
#include "cJSON.h"
#include "constants.h"
#include "raylib.h"
#include "json_loader.h"


// Replace SetFrame() with:
static void UpdateAnimation(Animation *anim) {
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