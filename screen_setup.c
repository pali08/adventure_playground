
#include <stdio.h>
#include <string.h>
#include <stdlib.h>


unsigned short *get_screen_size(void)
{
    // Fix: Size increased to 3 to safely hold width, height, and the terminator (-1)
    static unsigned short size[3];
    char *array[2];
    char screen_size[64];
    char *token = NULL;

    // Use xrandr to isolate the primary monitor's resolution line
    FILE *cmd = popen("xrandr | awk '/ primary / {print $4}'", "r");

    if (!cmd)
        return NULL;

    if (fgets(screen_size, sizeof(screen_size), cmd) == NULL) {
        pclose(cmd);
        return NULL;
    }
    pclose(cmd);

    // The token string looks like "1920x1080+0+0"
    // We split using 'x' and '+' to drop the offset parameters (+0+0)
    token = strtok(screen_size, "x+ \n");
    if (!token)
        return NULL;

    for (unsigned short i = 0; i < 2 && token != NULL; ++i) {
        array[i] = token;
        token = strtok(NULL, "x+ \n");
    }

    if (array[0] && array[1]) {
        size[0] = (unsigned short)atoi(array[0]);
        size[1] = (unsigned short)atoi(array[1]);
        size[2] = (unsigned short)-1; // Sentinal value termination
        return size;
    }

    return NULL;
}
