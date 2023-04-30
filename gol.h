#ifndef __GOL_H__
#define __GOL_H__

#include <stdlib.h>

#define DEAD 0
#define ALIVE 1
#define MAX_BOARD_SIZE 100
#define MAX_STEP_COUNT 1000
#define STEP_INTERVAL_MS 100

//
typedef enum CellState {

    CELL_ALIVE,
    CELL_DEAD,

} CellState;


//
typedef struct Board {

    // includes padding
    size_t maxWidth;
    size_t maxHeight;

    size_t width;
    size_t height;

    int** cells;

} Board;






#endif