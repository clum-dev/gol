#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <time.h>

#include "gol.h"
#include "clum-lib/file.h"
#include "clum-lib/strings.h"
#include "clum-lib/errors.h"


/** TODO:
 *  -   Wrap around borders of board
 *  -   
*/

//
Board* board_init(size_t width, size_t height) {

    if (width == 0 || height == 0 || width > MAX_BOARD_SIZE || height > MAX_BOARD_SIZE) {
        error_msg(E_ERROR, 1, "Invalid board size", true);
    }

    Board* board = (Board*)malloc(sizeof(Board));

    board->width = width;
    board->height = height;

    board->maxWidth = width + 2;
    board->maxHeight = height + 2;


    board->cells = (int**)malloc(sizeof(int*) * board->maxHeight);
    for (size_t i = 0; i < board->maxHeight; i++) {
        board->cells[i] = (int*)malloc(sizeof(int) * board->maxWidth);
        
        // init to 0
        for (size_t j = 0; j < board->maxWidth; j++) {
            board->cells[i][j] = 0;
        }
    }

    return board;
}

//
void board_free(Board* board) {

    if (board != NULL) {
        if (board->cells != NULL) {
            for (size_t i = 0; i < board->maxHeight; i++) {
                free(board->cells[i]);
            }
            free(board->cells);
            board->cells = NULL;
        }

        free(board);
        board = NULL;
    }
}

//
Board* board_clone(Board* board) {

    Board* new = board_init(board->width, board->height);

    for (size_t i = 0; i < new->maxHeight; i++) {
        for (size_t j = 0; j < new->maxWidth; j++) {
            new->cells[i][j] = board->cells[i][j];
        }
    }

    return new;
}

//
void board_print(Board* board) {

    for (size_t i = 1; i <= board->height; i++) {
        for (size_t j = 1; j <= board->width; j++) {
            printf("%d", board->cells[i][j]);
            printf(" ");
        }
        printf("\n");
    }
}

//
void board_print_padding(Board* board) {

    for (size_t i = 0; i < board->maxHeight; i++) {
        for (size_t j = 0; j < board->maxWidth; j++) {
            printf("%d", board->cells[i][j]);
            printf(" ");
        }
        printf("\n");
    }

}

//
void board_print_text(Board* board) {
    for (size_t i = 1; i <= board->height; i++) {
        for (size_t j = 1; j <= board->width; j++) {
            if (board->cells[i][j] == ALIVE) {
                printf("▀");
            } else {
                printf(" ");
            }
            printf(" ");
        }
        printf("\n");
    }
}

//
void board_fill(Board* board, char val) {
    for (size_t i = 1; i <= board->height; i++) {
        for (size_t j = 1; j <= board->width; j++) {
            board->cells[i][j] = val;
        }
    }
}

// Checks if board width is the same for all rows
bool valid_board(StringList* lines) {

    size_t initWidth;
    for (size_t i = 0; i < lines->size; i++) {
        if (i == 0) {
            initWidth = lines->strings[i]->len;
        } else {
            if (lines->strings[i]->len != initWidth) {
                return false;
            }
        }
    }

    return true;
}

//
void board_set_row(Board* board, size_t row, String* vals) {
    if (row + 1 > board->height) {
        return;
    }
    
    for (size_t i = 0; i < vals->len; i++) {
        board->cells[row+1][i+1] = vals->text[i] - '0';
    }
}

//
int get_convolution(Board* board, size_t row, size_t col) {
    
    // ignore padding
    if (row == 0 || row == board->maxHeight - 1 
            || col == 0 || col == board->maxWidth - 1) {
        return 0;
    }
    
    int total = 0;
    for (int i = -1; i <= 1; i++) {
        for (int j = -1; j <= 1; j++) {
            
            // TODO: WRAP AROUND BORDERS

            // int rowVal; int colVal;

            // if (row + i == 0) {
            //     rowVal = board->width;
            // } else if (row + i == board->maxWidth - 1) {
            //     rowVal = 0;
            // } else {
            //     rowVal = row + i;
            // }

            // if (col + j == 0) {
            //     colVal = board->height;
            // } else if (col + j == board->maxHeight - 1) {
            //     colVal = 0;
            // } else {
            //     colVal = col + j;
            // }
            
            if (!(i == 0 && j == 0)) {
                total += board->cells[row + i][col + j];
                // total += board->cells[rowVal][colVal];
            }
        }
    }

    return total;
}

//
int is_alive(int val, int neighbours) {
    
    // alive with 2 or 3 neighbours -> alive
    if (val == ALIVE) {
        if (neighbours == 2 || neighbours == 3) {
            return ALIVE;
        }
    
    // dead with 3 neighbours -> alive
    } else if (val == DEAD) {
        if (neighbours == 3) {
            return ALIVE;
        }
    }

    // everything else -> dead
    return DEAD;
}

//
void board_convolve(Board* board, Board* swap) {
        
    for (size_t i = 1; i <= board->height; i++) {
        for (size_t j = 1; j <= board->width; j++) {
            swap->cells[i][j] = is_alive(board->cells[i][j], get_convolution(board, i, j));
        }
    }
}

//
Board* board_load_from_file(char* filepath) {

    FILE* fp = open_file(filepath, "r");
    StringList* lines = get_file_lines(fp);
    size_t width; size_t height;

    if (valid_board(lines)) {
        width = lines->strings[0]->len;
        height = lines->size;
    } else {
        error_msg(E_ERROR, 0, "Board dimensions must be consistent", true);
    }

    Board* board = board_init(width, height);

    // set data
    for (size_t i = 0; i < lines->size; i++) {
        board_set_row(board, i, lines->strings[i]);
    }

    strlist_free(lines);
    fclose(fp);

    return board;
}

//
void run(Board* board, size_t stepcount, size_t msInterval) {

    if (board == NULL) {
        return;
    }

    size_t step = 0;
    Board* swap = board_clone(board);

    while (step < stepcount) {
        
        system("clear");
        if (step % 2 == 0) {
            board_convolve(board, swap);
            board_print_text(swap);
        } else {
            board_convolve(swap, board);
            board_print_text(board);
        }


        usleep(msInterval * 1000);

        step++;
    }

    board_free(board);
    board_free(swap);
}

//
void load_and_run(char* filename) {
    Board* board = board_load_from_file(filename);
    printf("Running...\n");
    run(board, MAX_STEP_COUNT, STEP_INTERVAL_MS);
}

// Prompts a user with a message and records their input as a string
String* prompt(char* msg) {
    printf("%s\n> ", msg);

    String* temp = dynamic_read(stdin, true);
    fflush(stdin);

    return temp;
}

// Prompts a user with a messange, records their input, 
// and checks against the given accept string
// Returns true if it matches
bool accept_prompt(char* msg, char* accept) {

    bool out = false;
    String* temp = prompt(msg);

    if (!strcmp(temp->text, accept)) {
        out = true;
    }
    str_free(temp);

    return out;
}

//
void save_board(Board* board, char* filename) {

    String* out = str_init(NULL);

    for (size_t i = 1; i <= board->height; i++) {
        for (size_t j = 1; j < board->width; j++) {
            str_concat_char(out, board->cells[i][j] + '0');
        }
        str_concat_char(out, '\n');
    }

    printf("Writing board to '%s'\n", filename);
    FILE* fp = open_file(filename, "w");
    fprintf(fp, "%s", out->text);
    fclose(fp);

    str_free(out);    
}

//
void store_random(Board* board, int weight) {
    srand(time(NULL));

    int upper = 10; int lower = 0;
    if (weight > upper) {
        weight = upper;
    } else if (weight < 0) {
        weight = 0;
    }

    for (size_t i = 1; i <= board->height; i++) {
        for (size_t j = 1; j <= board->width; j++) {
            int temp = (rand() % (upper - lower + 1)) + lower;

            if (temp > weight) {
                board->cells[i][j] = ALIVE;
            }
        }
    }    
}

//
void get_user_board() {

    printf("GOL: Creating new board\n\n");
    String* widthStr = prompt("Board width?");
    String* heightStr = prompt("Board height?");

    Board* board = board_init((size_t)atoi(widthStr->text), (size_t)atoi(heightStr->text));
    str_free(widthStr);
    str_free(heightStr);

    if (accept_prompt("Random cells? (y/n)", "y")) {
        String* weight = prompt("Random weight? (0-10)");
        store_random(board, atoi(weight->text));
        str_free(weight);
    }

    String* filename = prompt("Enter filename to save");
    save_board(board, filename->text);
    str_free(filename);
    
    board_free(board);    
}

//
void random_run() {

    printf("GOL: Creating new random board\n\n");
    String* size = prompt("Size?");
    Board* board = board_init((size_t)atoi(size->text), (size_t)atoi(size->text));
    str_free(size);

    String* weight = prompt("Random weight? (0-10)");
    store_random(board, atoi(weight->text));
    str_free(weight);

    run(board, MAX_STEP_COUNT, STEP_INTERVAL_MS);
}

//
int main(int argc, char** argv) {

    // 
    if (argc == 2) {
        if (!strcmp(argv[1], "-b")) {
            get_user_board();
        } else if (!strcmp(argv[1], "-r")) {
            random_run();
        } else {
            load_and_run(argv[1]);
        }

    } else {
        error_msg(E_ERROR, 1, "Usage: ./gol <filename> | -b | -r", true);
    }
   
    return 0;
}