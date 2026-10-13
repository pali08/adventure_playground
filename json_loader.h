#ifndef JSON_LOADER_H
#define JSON_LOADER_H

#include "cJSON.h"
#include "constants.h"
#include "entities.h"
#include "raylib.h"
#include <stdio.h>
#include <string.h>

void LoadRoomMetadata(const cJSON *root, Room *room);

// Update existing room loader to use it:
void LoadItemsFromJSONRoom(const cJSON *jsonItems, Room *room, Item *items,
                           Item *invisibleItems, int maxItems,
                           int maxInvisibleItems);

void LoadItemsFromJSONInventory(const cJSON *jsonItems, Inventory *inventory,
                                Item *items, Item *invisibleItems, int maxItems,
                                int maxInvisibleItems);

void LoadPersonsFromJSON(const cJSON *jsonPersons, Room *room, Person *persons,
                         Person *invisiblePersons, int maxPersons,
                         int maxInvisiblePersons);

#endif