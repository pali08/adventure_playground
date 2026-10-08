#ifndef HANDLE_INPUT_H
#define HANDLE_INPUT_H

#include "entities.h"


// Click handling
void HandleItemClicks(const Item *items, int itemCount, const int *itemIndices,
                      Vector2 virtualMouse);

void HandlePersonClicks(const Person *persons, int personCount,
                        const int *personIndices, Vector2 virtualMouse);
void HandleInventoryItemClicks(const Item *items, int itemCount,
                               const int *itemIndices,
                               const Inventory *inventory,
                               Vector2 virtualMouse);

#endif // RENDERING_H