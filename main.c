#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<ncurses.h>
#include<string.h>

struct cell_t {
  bool alive;    // true if alive, false if dead
  int neighbors; // number of alive neighbors
};

typedef struct cell_t cell;

struct board_t {
  int width;
  int height;
  cell** cells;
};

typedef struct board_t* board;

cell init_cell() {
  cell c;
  c.alive = false;
  c.neighbors = 0;
  return c;
}

// returns equivelent of true if given strings are
// equal to eachother
int streq(const char* s1, const char* s2) {
  return !strcmp(s1, s2);
}

void set_alive(board b, int h, int w, bool alive) {
  b->cells[h][w].alive = alive;
}

int is_alive(board b, int h, int w) {
  return b->cells[h][w].alive;
}

void toggle_alive(board b, int h, int w) {
  if(is_alive(b, h, w)) {
    set_alive(b,h, w, false);
  } else {
    set_alive(b,h, w, true);
  }
}

cell* init_row(int width) {
  cell* cells = malloc(sizeof(cell)*width);
  int i;
  for(i = 0; i < width; i++) {
    cells[i] = init_cell();
  }
  return cells;
}

board init_board(int height, int width) {
  board b = malloc(sizeof(struct board_t));
  b->width = width;
  b->height = height;
  /* since cells is a 2d array of cells,
     each element of cells is an array of cells,
     or, cell*.  so we initialize cells memory
     to be the size of an array of cells multiplied
     by the number of rows there are
  */
  b->cells = malloc(sizeof(cell*)*height);
  int i;
  for(i = 0; i < height; i++) {
    b->cells[i] = init_row(width);
  }
  return b;
}

board init_board_from_chars(char* pic, int h, int w) {
  board b = init_board(h, w);
  int i,j;
  for(i=0; i<h; i++) {
    for(j=0; j<w; j++) {
      if(pic[i*w+j] == '.') {
        set_alive(b, i, j, false);
      } else if(pic[i*w+j] == 'X') {
        set_alive(b, i, j, true);
      } else {
        printf("FAIL %c\n", pic[i*w+j]);
      }
    }
  }
  return b;
}

board init_board_from_file(const char* file) {
  FILE* f = open(file, "r");
  return init_board(1,1);
}

board clone_board(board b) {
  board newboard = init_board(b->height, b->width);
  int i,j;
  for(i=0; i<b->height; i++) {
    for(j=0; j<b->width; j++) {
      newboard->cells[i][j] = b->cells[i][j];
    }
  }
  return newboard;
}

void free_board(board b) {
  int i;
  for(i = 0; i < b->height; i++) {
    free(b->cells[i]);
  }
  free(b->cells);
  free(b);
}

board change_board_size(board b, int diff_x, int diff_y) {
  // takes a board, and some x,y values which alter the size
  // of said board, growing/shrinking the boards dimentions
  // according to them
  // NOTE: this breaks when the board grows as cloning
  // a board to a size larger than itself is undefined behavior
  b->width = b->width + diff_x;
  b->height = b->height + diff_y;
  return clone_board(b);
}

bool alive_at(board b, int x, int y) {
  if(x >= b->width)
    x = 0;
  if(x < 0)
    x = b->width-1;
  if(y >= b->height)
    y = 0;
  if(y < 0)
    y = b->height-1;
  return is_alive(b, y, x);
}

int num_neighbors(board b, int y, int x) {
  int count = 0;
  if(alive_at(b, x-1, y-1))
    count++;
  if(alive_at(b, x-1, y))
    count++;
  if(alive_at(b, x, y-1))
    count++;
  if(alive_at(b, x+1, y))
    count++;
  if(alive_at(b, x, y+1))
    count++;
  if(alive_at(b, x-1, y+1))
    count++;
  if(alive_at(b, x+1, y-1))
    count++;
  if(alive_at(b, x+1, y+1))
    count++;

  return count;
}

void compute_neighbors(board b) {
  int i, j;
  for(i=0; i<b->height; i++) {
    for(j=0; j<b->width; j++) {
      b->cells[i][j].neighbors = num_neighbors(b, i, j);
    }
  }
}

cell iterate_cell(cell c) {
  if(c.alive) {
    // cell dies to starvation
    if(c.neighbors < 2) {
      c.alive = false;
      return c;
    }
    // cell stays alive
    if(c.neighbors <= 3 && c.neighbors >= 2) {
      return c;
    }
    // cell is overcrowded and dies
    if(c.neighbors > 3) {
      c.alive = false;
      return c;
    }
  } else {
    // cell becomes alive
    if(c.neighbors == 3) {
      c.alive = true;
      return c;
    }
  }
  return c;
}

board iterate_board(board b) {
  // because the whole board needs to tick
  // at the same time as a result of eachother,
  // we can't modify the cells in place
  // NOTE: actually we could as long as we
  // only modify the alive state and not the
  // neighbors state
  board newboard = clone_board(b);
  int i,j;
  for(i=0; i<b->height; i++) {
    for(j=0; j<b->width; j++) {
      newboard->cells[i][j] = iterate_cell(b->cells[i][j]);
    }
  }
  return newboard;
}

void iterate_board2(board b) {
  int i,j;
  for(i=0; i<b->height; i++) {
    for(j=0; j<b->width; j++) {
      b->cells[i][j] = iterate_cell(b->cells[i][j]);
    }
  }
}

board tick_board(board b) {
  compute_neighbors(b);
  return iterate_board(b);
}

void print_board(int y, int x, board b) {
  int i;
  int j;
  move(y,x);
  for(i = 0; i < b->height; i++) {
    for(j = 0; j < b->width; j++) {
      cell c = b->cells[i][j];
      if(c.alive) {
        printw("X ");
      } else {
        printw(". ");
      }
    }
    printw("\n");
  }
  printw("\n");
}

void print_help() {
  printf("Conways Game of life\n\n");
  printf("-h,--help\tdisplay this message\n");
  printf("-d\t\tDebug mode, prints the key code of a key press,\n\t\twaits for you to hit another key then exits\n");
  printf("-e,--editor\tStart the interactive editor\n");
  printf("--engine\tRun the engine example\n");
  printf("--glider\tRun the glider example\n");
  printf("--gun\t\tRun the glider gun example\n");
  printf("--pulsar\tRun the pulsar example\n");
  printf("--pentimino\tRun the R-pentimino example\n");
}

board create_board_from_name(const char* name) {
  board b;
  if(streq(name, "-h") || streq(name, "--help")) {
    print_help();
    exit(1);
  } else if(streq(name, "--engine")) {
    int engine_x = 40;
    int engine_y = 30;
    char engine[][40] = {"........................................",
                         "........................................",
                         "........................................",
                         "........................................",
                         "........................................",
                         "........................................",
                         "........................................",
                         "........................................",
                         "........................................",
                         "........................................",
                         "........................................",
                         "........................................",
                         "........................................",
                         "........................................",
                         "........................................",
                         "...........X............................",
                         ".........X.XX...........................",
                         ".........X.X............................",
                         ".........X..............................",
                         ".......X................................",
                         ".....X.X................................",
                         "........................................",
                         "........................................",
                         "........................................",
                         "........................................",
                         "........................................",
                         "........................................",
                         "........................................",
                         "........................................",
                         "........................................",
    };
    b = init_board_from_chars(&engine[0][0], engine_y, engine_x);
  } else if(streq(name, "--glider")) {
    int glider_x = 20;
    int glider_y = 20;
    char glider[][20]  = {"....................",
                          "...X................",
                          "....X...............",
                          "..XXX...............",
                          "....................",
                          "....................",
                          "....................",
                          "....................",
                          "....................",
                          "....................",
                          "....................",
                          "....................",
                          "....................",
                          "....................",
                          "....................",
                          "....................",
                          "....................",
                          "....................",
                          "....................",
                          "...................."};
    b = init_board_from_chars(&glider[0][0], glider_y, glider_x);
  } else if(streq(name, "--gun")) {
    int gun_x = 42;
    int gun_y = 20;
    char gun[][42] = {"..........................................",
                      "..........................................",
                      "..........................................",
                      "..........................................",
                      "..........................X...............",
                      "........................X.X...............",
                      "..............XX......XX............XX....",
                      ".............X...X....XX............XX....",
                      "..XX........X.....X...XX..................",
                      "..XX........X...X.XX....X.X...............",
                      "............X.....X.......X...............",
                      ".............X...X........................",
                      "..............XX..........................",
                      "..........................................",
                      "..........................................",
                      "..........................................",
                      "..........................................",
                      "..........................................",
                      "..........................................",
                      ".........................................."};
    b = init_board_from_chars(&gun[0][0], gun_y, gun_x);
  } else if(streq(name, "--pulsar")) {
    int pulsar_x = 19;
    int pulsar_y = 17;
    char pulsar[][19] = {"...................",
                         "...................",
                         ".....XXX...XXX.....",
                         "...................",
                         "...X....X.X....X...",
                         "...X....X.X....X...",
                         "...X....X.X....X...",
                         ".....XXX...XXX.....",
                         "...................",
                         ".....XXX...XXX.....",
                         "...X....X.X....X...",
                         "...X....X.X....X...",
                         "...X....X.X....X...",
                         "...................",
                         ".....XXX...XXX.....",
                         "...................",
                         "...................",};
    b = init_board_from_chars(&pulsar[0][0], pulsar_y, pulsar_x);
  } else if(streq(name, "--pentimino")) {
    int pen_x = 75;
    int pen_y = 32;
    char pen[][75] = {"...........................................................................",
                      "...........................................................................",
                      "...........................................................................",
                      "...........................................................................",
                      "...........................................................................",
                      "...........................................................................",
                      "...........................................................................",
                      "...........................................................................",
                      "...........................................................................",
                      "...........................................................................",
                      "...........................................................................",
                      "...........................................................................",
                      "...........................................................................",
                      "...........................................XX..............................",
                      "..........................................XX...............................",
                      "...........................................X...............................",
                      "...........................................................................",
                      "...........................................................................",
                      "...........................................................................",
                      "...........................................................................",
                      "...........................................................................",
                      "...........................................................................",
                      "...........................................................................",
                      "...........................................................................",
                      "...........................................................................",
                      "...........................................................................",
                      "...........................................................................",
                      "...........................................................................",
                      "...........................................................................",
                      "...........................................................................",
                      "...........................................................................",
                      "...........................................................................",
    };
    b = init_board_from_chars(&pen[0][0], pen_y, pen_x);
  } else {
    echo();
    curs_set(1);
    print_help();
    endwin();
    exit(1);
  }
  return b;
}

int get_dim(const char* prompt) {
  curs_set(1);
  nocbreak();
  echo();
  char string[5];
  mvprintw(0,0, prompt);
  getstr(string);
  int i;
  sscanf(string, "%5i", &i);
  return i;
}

void print_instructions() {
  printw("hit q to run simulation\tArrow keys to move\n");
  printw("Enter to toggle alive/dead and move down a row\n");
  printw("Space to toggle alive/dead and move right a column\n");
}

board start_editor() {
  int h = get_dim("enter height: ");
  clear();
  int w = get_dim("enter width: ");
  curs_set(1);
  noecho();
  cbreak();
  move(20,20);
  int x = 0;
  int x_1 = 0;
  int y = 0;
  board b = init_board(h, w);
  int key;
  print_board(0, 0, b);
  print_instructions();
  move(y,x);
  while((key = getch()) != 113) { // wait until user hits q
    print_board(0, 0, b);
    print_instructions();
    move(y,x);
    if(key == 10) { // enter key
      toggle_alive(b,y,x_1);
      print_board(0,0,b);
      print_instructions();
      y++;
      move(y,x);
      refresh();
    }
    if(key == 32) { // space bar
      toggle_alive(b,y,x_1);
      print_board(0,0,b);
      print_instructions();
      x+=2;
      x_1++;
      move(y,x);
      refresh();
    }
    if(key == 260 && x_1 > 0) { // left arrow key
      x-=2;
      x_1--;
      move(y,x);
    }
    if(key == 261 && x_1 < w-1) { // right arrow key
      x+=2;
      x_1++;
      move(y,x);
    }
    if(key == 259 && y > 0) { // up arrow key
      y-=1;
      move(y,x);
    }
    if(key == 258 && y < h-1) { // down arrow key
      y+=1;
      move(y,x);
    }
    if(key == 544) { // control left
      board newboard = change_board_size(b, -1, 0);
      free_board(b);
      b = newboard;
      w--;
      print_board(0,0,b);
      print_instructions();
      move(y,x);
      refresh();
    }
    if(key == 559) {
      board newboard = change_board_size(b, 1, 0);
      free_board(b);
      b = newboard;
      w++;
      print_board(0,0,b);
      print_instructions();
      move(y,x);
      refresh();
    }
    if(key == 565) { // control up
      board newboard = change_board_size(b, 0, -1);
      free_board(b);
      b = newboard;
      h--;
      print_board(0,0,b);
      print_instructions();
      move(y,x);
      refresh();
    }
    if(key == 524) { // control down
      board newboard = change_board_size(b, 0, 1);
      free_board(b);
      b = newboard;
      h++;
      print_board(0,0,b);
      print_instructions();
      move(y,x);
      refresh();
    }
    refresh();
  }
  curs_set(0);
  clear();
  return b;
}

int main(int argc, char* argv[]) {
  board b;

  if(argc < 2) {
    print_help();
    exit(1);
  }

  initscr();
  cbreak();
  curs_set(0);
  keypad(stdscr, TRUE);
  noecho();

  if(streq(argv[1], "-d")) {
    int x = getch();
    clear();
    mvprintw(0,0,"%i", x);
    getch();
    echo();
    curs_set(1);
    endwin();
    exit(0);
  }
  if(streq(argv[1], "-e") || streq(argv[1], "--editor")) {
    b = start_editor();
  } else {
    b = create_board_from_name(argv[1]);
  }
  int generation = 0;

  while(true) {
    mvprintw(0,0,"Generation: %d", generation);
    generation++;
    print_board(1, 0, b);
    refresh();
    board b1 = tick_board(b);
    free(b);
    b = b1;
    long one_second = 1000000;
    usleep(one_second/6);
  }

  free_board(b);
  echo();
  curs_set(1);
  endwin();
  return 0;
}
