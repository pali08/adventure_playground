#ifndef ENTITIES_H
#define ENTITIES_H

#include "animation.h"
#include "constants.h"
#include "raylib.h"

// Common base for all animated entities
typedef struct {
  Animation anim; // <-- Reuse Animation struct
} Entity;

// Item-specific data (extends Animation)
typedef struct {
  Animation anim; // base animation
  bool return_to_inventory_after_combine;
  char name[MAX_NAME_LEN];        // e.g., "rusty key"
  char description[MAX_DESC_LEN]; // e.g., "A dusty iron key, covered in rust."
  char combinable_with[MAX_NAME_LEN]; // item name it can combine with
  char resulting_item_name[MAX_NAME_LEN];
  char type[MAX_TYPE_LEN];           // e.g., "key", "tool", "keyitem"
  char take_message[MAX_MSG_LEN];    // e.g., "You picked up the key."
  char combine_message[MAX_MSG_LEN]; // e.g., "You combine the key with the
                                     // door..."
  char make_some_other_item_in_room_takeable[MAX_NAME_LEN]; // optional
  char uncombinable_with[MAX_NAME_LEN];
  char uncombinable_message[MAX_MSG_LEN];
  char is_modified_by[MAX_NAME_LEN]; // what modifies this item
  char modifies_item[MAX_NAME_LEN];  // what item this modifies
  char modification_resulting_item_name[MAX_NAME_LEN];
  char modification_message[MAX_MSG_LEN];
  char take_sound[MAX_PATH_LEN]; // e.g., "assets/sfx/take_key.wav"
  char combine_sound[MAX_PATH_LEN];
  char modify_sound[MAX_PATH_LEN];
  char search_sound[MAX_PATH_LEN];
} Item;

// Person-specific data (extends Animation)
typedef struct {
  Animation anim; // base animation
  char name[MAX_NAME_LEN];
  char description[MAX_DESC_LEN];
  char combinable_with[MAX_NAME_LEN]; // e.g., "empty bottle"
  // Dialogue can be a struct later; for now, maybe index into a dialogue table
  // or store ID. Replace with real `Dialogue` later as needed.
  char dialogue_id[MAX_NAME_LEN]; // e.g., "guard_intro_01"
} Person;

typedef struct {
  Person persons[MAX_PERSONS];
  int personCount;

  Item items[MAX_ITEMS];
  int itemCount;

  char background[MAX_NAME_LEN];            
  char sound[MAX_NAME_LEN];                
  char firstVisitText[301];       // max 300 chars + null terminator
} Room;

int LoadItems(Item *items, int maxCount);
int LoadPersons(Person *persons, int maxCount);
void UpdateItems(float *animTimers, int *animIndices, int animCount,
                 const Item *items);
void UpdatePersons(float *animTimers, int *animIndices, int animCount,
                   const Person *persons);
void LoadRoom(const char *jsonFile, Room *room, Item *items, int maxItems,
              Person *persons, int maxPersons);
#endif // ENTITIES_H