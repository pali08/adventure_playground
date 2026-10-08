#include "animation.h"
#include "constants.h"
#include "entities.h"
#include "handle_input.h"
#include "input.h"
#include "raylib.h"
#include "rendering.h"
#include "screen_setup.h"
#include <stdio.h>

int main(void) {
  unsigned short *size = get_screen_size();
  const int screenWidth = size[0];
  const int screenHeight = size[1];

  InitializeWindow(screenWidth, screenHeight, GAME_WIDTH, GAME_HEIGHT);
  RenderTexture2D target = InitializeRenderTexture(GAME_WIDTH, GAME_HEIGHT);

  // Animation animations[MAX_ANIMATIONS];
  Item items[MAX_ITEMS];
  Person persons[MAX_PERSONS];
  Item inventoryItems[MAX_INVENTORY_ITEMS];
  Room room = {0};
  Inventory inventory;

  LoadRoom(ASSETS_ROOMS_DIR "/descriptions_nadr.json", &room, items, MAX_ITEMS,
           persons, MAX_PERSONS);
  LoadInventory(ASSETS_ROOMS_DIR "/descriptions_inventory.json", &inventory,
                inventoryItems, MAX_INVENTORY_ITEMS);

  printf("persons: %f", persons[0].anim.fps);
  printf("persons: %f", persons[1].anim.fps);

  float animItemTimers[MAX_ITEMS] = {0};
  float animPersonTimers[MAX_PERSONS] = {0};
  float animInventoryTimers[MAX_INVENTORY_ITEMS] = {0};

  int animItemIndices[MAX_ITEMS] = {0};
  int animPersonIndices[MAX_PERSONS] = {0};
  int animInventoryIndices[MAX_INVENTORY_ITEMS] = {0};

  char bgPath[MAX_PATH_LEN];
  snprintf(bgPath, sizeof(bgPath), "%s/%s", ASSETS_BACKGROUNDS_DIR,
           room.background);
  Texture2D background = LoadTexture(bgPath);

  char soundPath[MAX_PATH_LEN];
  snprintf(soundPath, sizeof(soundPath), "%s/%s", ASSETS_SOUNDS_DIR,
           room.sound);

  SetTargetFPS(60);

  // ... earlier initialization code ...

  while (!WindowShouldClose()) {
    float scale = CalculateScaleFactor();
    Vector2 virtualMouse = CalculateVirtualMouse(scale);

    UpdateItems(animItemTimers, animItemIndices, room.itemCount, items);
    UpdatePersons(animPersonTimers, animPersonIndices, room.personCount,
                  persons);
    UpdateItems(animInventoryTimers, animInventoryIndices, inventory.itemCount,
                inventoryItems);

    BeginTextureMode(target);
    ClearBackground(DARKGRAY);
    DrawTexture(background, 0, 0, WHITE);
    EndTextureMode();

    DrawItemsToRenderTexture(target, items, room.itemCount, animItemIndices);
    DrawPersonsToRenderTexture(target, persons, room.personCount,
                               animPersonIndices);
    DrawInventoryToRenderTexture(target, &inventory, inventoryItems,
                                 inventory.itemCount, animInventoryIndices);

    HandleItemClicks(items, room.itemCount, animItemIndices, virtualMouse);
    HandlePersonClicks(persons, room.personCount, animPersonIndices,
                       virtualMouse);
    HandleInventoryItemClicks(inventoryItems, inventory.itemCount, animInventoryIndices, &inventory,
                              virtualMouse);

    PresentToScreen(target, scale, GAME_WIDTH, GAME_HEIGHT);
  }

  for (int i = 0; i < room.personCount; i++)
    UnloadAnimation(&persons[i].anim);
  for (int i = 0; i < room.itemCount; i++)
    UnloadAnimation(&items[i].anim);
  for (int i = 0; i < inventory.itemCount; i++)
    UnloadAnimation(&items[i].anim);
  UnloadRenderTexture(target);
  CloseWindow();
  return 0;
}