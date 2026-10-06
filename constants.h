#ifndef CONSTANTS_H
#define CONSTANTS_H

#define ASSETS_DIR "assets"
#define ASSETS_ROOMS_DIR "assets/rooms"
#define ASSETS_ANIMATIONS_DIR "assets/animations"
#define ASSETS_BACKGROUNDS_DIR "assets/backgrounds"
#define ASSETS_SOUNDS_DIR "assets/sounds"

#define MAX(a, b) ((a) > (b) ? (a) : (b))
#define MIN(a, b) ((a) < (b) ? (a) : (b))
#define MAX_FRAMES 64
#define MAX_PATH_LEN 256
#define GAME_WIDTH 1920
#define GAME_HEIGHT 1080

// Entities parameters length
#define MAX_NAME_LEN 30
#define MAX_DESC_LEN 100
#define MAX_MSG_LEN 301
#define MAX_TYPE_LEN 30

#define MAX_ITEMS 30
#define MAX_PERSONS 30
#define MAX_INVENTORY_ITEMS 20
#define INVENTORY_ROWS 2
#define INVENTORY_COLUMNS 10
#define INVENTORY_GRID_THICKNESS 3.0
#define INVENTORY_GRID_TOP_OFFSET 5
#define INVENTORY_GRID_ITEM_PADDING 7

#endif