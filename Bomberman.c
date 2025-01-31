#define _POSIX_C_SOURCE 199309L
#include <ncurses.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>

struct timespec ts2 = {
        .tv_sec = 0,                    // nr of secs
        .tv_nsec = 600 * 1000000L  // nr of nanosecs
    };

struct character {
    int x, y;
    int direction;
    char appearences[4];
    int score;
    bool alive; // 0 N 1 W 2 S 3 E
};

struct bomb {
    int x, y;
    bool is_bomb_dropped;
    int time; // miliseconds
    int color; 
    int radius;
    char appearence;
};

struct enemy {
    int x, y;
    char appearence;
    char appearences[2];
    int color;
    int direction; // 1 N 2 W 3 S 4 E
    int time;
    bool alive; 
};

struct enemy2 {
    int x, y;
    char appearence;
    char appearences[2];
    int color;
    int direction; // 1 N 2 W 3 S 4 E
    int time;
    bool alive; 
};

struct transition {
    int x, y;
    char appearence;
    int color;
};

struct powerup {
    int x, y;
    char appearence;
    int color;
    bool taken;
};


struct lvl {
    int enemy1;
    int enemy2;
    int enemy3;
    bool last;
};

void generate_random_wall(int rows, int cols, char field[rows][cols]){
    int chance = 4;
    for  (int row = 0; row < 5; row++){
        for (int
         col = 5; col < cols-3; col++){
            if (field[row][col] == ' ' && field[row][col+1] != '#'){
                if (field[row+1][col] == '@') chance -=2;
                int random = rand() % chance;
                if (random == 0){
                    field[row][col] = '@';
                    random = rand() % (chance-1);
                    if (random == 0){
                        field[row][col+1] = '@';
                        random = rand() % (chance-2);
                        if (random == 0){
                            field[row][col+2] = '@';
                        }
                    }
                }
            }
            else if (field[row][col] == ' ' && field[row][col+1] == '#'){
                int random;
                random = rand() % 2;
                if (random == 0){
                    field[row][col] = '@';
                }
            }
            else continue;
        }
    }
    for  (int row = 4; row < rows; row++){
        for (int col = 0; col < cols-3; col++){
            if (field[row][col] == ' ' && field[row][col+1] != '#'){
                if (field[row+1][col] == '@') chance -=2;
                int random = rand() % chance;
                if (random == 0){
                    field[row][col] = '@';
                    random = rand() % (chance-1);
                    if (random == 0){
                        field[row][col+1] = '@';
                        random = rand() % (chance-2);
                        if (random == 0){
                            field[row][col+2] = '@';
                        }
                    }
                }
            }
            else if (field[row][col] == ' ' && field[row][col+1] == '#'){
                int random = rand() % 2;
                if (random == 0){
                    field[row][col] = '@';
                }
            }
            else continue;
        }
    }
}

void initialize_field(int rows, int cols, char field[rows][cols]) {
    for (int row = 0; row < rows; row++) {
        for (int col = 0; col < cols; col++) {
            field[row][col] = ' ';
        }
    }
    for (int col = 1; col < cols-1; col++) {
        field[0][col] = '-';
        field[rows - 1][col] = '-';
    }
    for (int row = 1; row < rows-1; row++) {
        field[row][0] = '|';
        field[row][cols - 1] = '|';
    }
    field[rows-1][cols-1] = '+';
    field[0][0] = '+';
    field[rows-1][0] = '+';
    field[0][cols-1] = '+';
    for (int row = 2; row < rows - 2; row += 2) {
        for (int col = 2; col < cols-2; col += 2) {
            field[row][col] = '#';
        }
    }
    generate_random_wall(rows, cols, field);


    // field[1][1] = '^';
    // field[2][1] = '@';
    // field[1][2] = '@';
    // field[2][3] = '@';
}

void printwc(char character, WINDOW *win) {
    if (character == '|' || character == '-' || character == '+' ) {
        wattron(win, COLOR_PAIR(1));
        waddch(win, character); // Render walls as solid blocks
        wattroff(win, COLOR_PAIR(1));
    } else if (character == '#') {
        wattron(win, COLOR_PAIR(1));
        waddch(win, '#'); // Render obstacles as #
        wattroff(win, COLOR_PAIR(1));
    } else if (character == '@') {
        wattron(win, COLOR_PAIR(4));
        waddch(win, '@'); // Render obstacles as #
        wattroff(win, COLOR_PAIR(4));
    } else if (character == 'Z' || character == 'N') {
        wattron(win, COLOR_PAIR(6));
        waddch(win, character); // Render obstacles as #
        wattroff(win, COLOR_PAIR(6));
    } else if (character == 'p' || character == 'd') {
        wattron(win, COLOR_PAIR(8));
        waddch(win, character); // Render obstacles as #
        wattroff(win, COLOR_PAIR(8));
    } else if (character == '>' || character == '^' || character == '<' || character == 'v') {
        wattron(win, COLOR_PAIR(3));
        waddch(win, character); // Render directional arrows
        wattroff(win, COLOR_PAIR(3));
    } else { 
        wattron(win, COLOR_PAIR(2));
        waddch(win, ' '); // Render empty spaces
        wattroff(win, COLOR_PAIR(2));
    }
}

void print_board(int rows, int cols, char field[rows][cols], WINDOW *win) {
    for (int row = 0; row < rows; row++) {
        for (int col = 0; col < cols; col++) {
            wmove(win, row + 1, col + 1); // Adjust for window border
            printwc(field[row][col], win);
        }
    }
    wrefresh(win); // Refresh the window to display changes
}

void blinking_character(struct character *ch, bool is_red,  WINDOW *win){
    int color;
    if (is_red == 1) color = 5;
    else color = 1;
    wattron(win, COLOR_PAIR(color));
    mvwaddch(win, ch->y, ch->x, ch->appearences[ch->direction]);
    wattroff(win, COLOR_PAIR(color));
}

void update_character_position(struct character *ch, struct bomb *bm, int rows, int cols, char field[rows][cols], int input,  WINDOW *win) {
    // if (field[ch->y-1][ch->x-1] == '*' || field[ch->y-1][ch->x-1] == 'Z' || field[ch->y-1][ch->x-1] == 'N'){
    //     ch->alive = false;
    //     return; // death anime
    // }
    // Clear previous position
    if (ch->alive == false){
        return;
    }
    field[ch->y-1][ch->x-1] = ' ';

    // Determine direction based on input

    // Update field with character's appearance
    // Clear previous position on screen
    mvwaddch(win, ch->y, ch->x, ' ');

    // Determine direction based on input and calculate new position
    int new_x = ch->x;
    int new_y = ch->y;

    switch (input) {
        case KEY_UP:    ch->direction = 0; if (ch->y > 2) new_y--; break;
        case KEY_LEFT:  ch->direction = 1; if (ch->x > 2) new_x--; break;
        case KEY_DOWN:  ch->direction = 2; if (ch->y < rows - 1) new_y++; break;
        case KEY_RIGHT: ch->direction = 3; if (ch->x < cols - 1) new_x++; break;
    }
    
    // Check if the new position is valid (e.g., not a wall or obstacle)
    if (field[new_y-1][new_x-1] == ' ' || field[new_y-1][new_x-1] == 'O' || field[new_y-1][new_x-1] == 'Z' || field[new_y-1][new_x-1] == 'N') {
        ch->x = new_x;
        ch->y = new_y;
    }

    if ((ch->x == bm->x) && (ch->y == bm->y) && bm->is_bomb_dropped){
        field[ch->y-1][ch->x-1] = 'O'; 
        int temp = (bm->time > 1000) ? (bm->time / 250) % 2 : (bm->time / 125) % 2;
        blinking_character(ch, temp, win);

    }
    else{
        field[ch->y-1][ch->x-1] = ch->appearences[ch->direction];

    // Draw the character in the new position
        wmove(win, ch->y, ch->x);
        printwc(ch->appearences[ch->direction], win);
    }

}

void blinking_bomb(struct bomb *bm, bool is_red,  WINDOW *win){
    if (is_red == 1) bm->color = 5;
    else bm->color = 1;
    wattron(win, COLOR_PAIR(bm->color));
    mvwaddch(win, bm->y, bm->x, bm->appearence);
    wattroff(win, COLOR_PAIR(bm->color));
}

void bomb(struct character *ch, struct bomb *bm, int rows, int cols, char field[rows][cols], int input,  WINDOW *win){
    bm->time -= 80;
    if (input == 'b' && !bm->is_bomb_dropped){
        bm->x = ch->x; // Set bomb's position to character's current position
        bm->y = ch->y;
        bm->time = 2000;
        // wprintw(win, "time %d", bm->time);
        // if (input == KEY_UP) wprintw(win, "test2 correct %d", input);
        // attron(COLOR_PAIR(5));
        // mvaddch(ch->y, ch->x, ch->appearences[ch->direction]);
        // attroff(COLOR_PAIR(5));
        bm->color = 5;
        bm->is_bomb_dropped = true;
    }
    // if (input = 'b' && bm->is_bomb_dropped){
    //     //error message
    // }
    if (bm->is_bomb_dropped && bm->time != 0 && (ch->x != bm->x || ch->y != bm->y)){
        int temp = (bm->time > 1000) ? (bm->time / 250) % 2 : (bm->time / 125) % 2;
        field[bm->y-1][bm->x-1] = 'O';
        blinking_bomb(bm, temp, win);
        
    }
    if (bm->is_bomb_dropped && bm->time <= 0){
        bm->is_bomb_dropped = false;
        field[bm->y-1][bm->x-1] = '*';
        mvwaddch(win, bm->y, bm->x, '*');
        for  (int i = 1; i < bm->radius+1; i++){ // left right
            if (field[bm->y-1][bm->x] == '#' || field[bm->y-1][bm->x-2] == '#') break;
            if ((bm->x+i-1 < cols-1)){
                field[bm->y-1][bm->x+i-1] = '*';
                mvwaddch(win, bm->y, bm->x+i, '*');
            }
            if ((bm->x-i-1 > 0)){
                field[bm->y-1][bm->x-i-1] = '*';
                mvwaddch(win, bm->y, bm->x-i, '*');
            }
        }
        for  (int i = 1; i < bm->radius+1; i++){ // left right
            if (field[bm->y+1-1][bm->x-1] == '#' || field[bm->y-1-1][bm->x-1] == '#') break;
            if ((bm->y+i-1 < rows-1)){
                field[bm->y+i-1][bm->x-1] = '*';
                mvwaddch(win, bm->y+i, bm->x, '*');
            }
            if ((bm->y-i-1 > 0)){
                field[bm->y-i-1][bm->x-1] = '*';
                mvwaddch(win, bm->y-i, bm->x, '*');
            }
        }
        
    }


}

void remove_stars(int rows, int cols, char field[rows][cols],  WINDOW *win){
    for (int row = 0; row < rows; row++) {
        for (int col = 0; col < cols; col++) {
            if (field[row][col] == '*') {
                field[row][col] = ' ';
                wmove(win, row+1, col+1);
                printwc(' ', win);
            }

        }
    }
}

void clear_getch_buffer(WINDOW *win) {
    int ch;
    while ((ch = wgetch(win)) != ERR) {}
}

void spawn_enemy(int rows, int cols, char field[rows][cols], struct enemy *en, WINDOW *win){
    int x, y;
    while (1){
        x = rand() % ((cols-1) - 4 + 1) + 4;
        y = rand() % ((rows-1) - 4 + 1) + 4;     
        if (field[y][x] == ' '){
            en->x = x+1;
            en->y = y+1;
            field[y][x] = en->appearences[0];
            wmove(win, en->y, en->x);
            printwc(en->appearences[0], win);
            return;
        }
    }
}

void update_enemy_pos(struct enemy *en, struct bomb *bm, int rows, int cols, char field[rows][cols], WINDOW *win, struct character *ch) {
    if (!en->alive){
        return;
    }
    if (field[en->y-1][en->x-1] == '*'){
        ch->score += 100;
        en->alive = false;
        return; // giving points
    }
    if (en->y < 1 || en->y > rows || en->x < 1 || en->x > cols) {
    // Enemy is out of bounds; reset its position
        spawn_enemy(rows, cols, field, en, win);
        return;
    }
    if (en->time <= 0){
        en->time = 200;
        return;
    }
    // Clear previous position
    field[en->y-1][en->x-1] = ' ';

    // Determine direction based on input

    // Update field with character's appearance
    // Clear previous position on screen
    mvwaddch(win, en->y, en->x, ' ');
    


    if (en->direction == 1){ // 1 N 2 W 3 S 4 E
        if (field[en->y-1-1][en->x-1] == 'O' || field[en->y-1-1][en->x-1] == '#' || field[en->y-1-1][en->x-1] == '@' || field[en->y-1-1][en->x-1] == '-'){
            while (en->direction == 1){
                en->direction = rand() % 4 + 1;
            }
            en->time -= 200;
            en->appearence = !en->appearence;
            field[en->y-1][en->x-1] = en->appearences[0];

    // Draw the character in the new position
            wmove(win, en->y, en->x);
            printwc(en->appearences[en->appearence], win);
        }
        else{
            en->appearence = !en->appearence;
            field[en->y-1-1][en->x-1] = en->appearences[0];
    // Draw the character in the new position
            wmove(win, en->y-1, en->x);
            printwc(en->appearences[en->appearence], win);
            en->y--;
            en->time -= 200;
        }
    }
    else if (en->direction == 2){ // 1 N 2 W 3 S 4 E
        if (field[en->y-1][en->x-1-1] == 'O' || field[en->y-1][en->x-1-1] == '#' || field[en->y-1][en->x-1-1] == '@' || field[en->y-1][en->x-1-1] == '|'){
            while (en->direction == 2){
                en->direction = rand() % 4 + 1;
            }
            en->time -= 200;
            en->appearence = !en->appearence;
            field[en->y-1][en->x-1] = en->appearences[0];

    // Draw the character in the new position
            wmove(win, en->y, en->x);
            printwc(en->appearences[en->appearence], win);
        }
        else{
            en->appearence = !en->appearence;
            field[en->y-1][en->x-1-1] = en->appearences[0];

    // Draw the character in the new position
            wmove(win, en->y, en->x-1);
            printwc(en->appearences[en->appearence], win);
            en->time -= 200;
            en->x--;
        }
    }
    else if (en->direction == 3){ // 1 N 2 W 3 S 4 E
        if (field[en->y+1-1][en->x-1] == 'O' || field[en->y+1-1][en->x-1] == '#' || field[en->y+1-1][en->x-1] == '@' || field[en->y+1-1][en->x-1] == '-'){
            while (en->direction == 3){
                en->direction = rand() % 4 + 1;
            }
            en->time -= 200;
            en->appearence = !en->appearence;
            field[en->y-1][en->x-1] = en->appearences[0];

    // Draw the character in the new position
            wmove(win, en->y, en->x);
            printwc(en->appearences[en->appearence], win);

        }
        else{
            en->appearence = !en->appearence;
            field[en->y+1-1][en->x-1] = en->appearences[0];

    // Draw the character in the new position
            wmove(win, en->y+1, en->x);
            printwc(en->appearences[en->appearence], win);
            en->time -= 200;
            en->y++;
        }
    }
    else if (en->direction == 4){ // 1 N 2 W 3 S 4 E
        if (field[en->y-1][en->x+1-1] == 'O' || field[en->y-1][en->x+1-1] == '#' || field[en->y-1][en->x+1-1] == '@' || field[en->y-1][en->x+1-1] == '|'){
            while (en->direction == 4){
                en->direction = rand() % 4 + 1;
            }
            en->time -= 200;
            // en->appearence = !en->appearence;
            field[en->y-1][en->x-1] = en->appearences[0];

    // Draw the character in the new position
            wmove(win, en->y, en->x);
            printwc(en->appearences[en->appearence], win);
        }
        else{
            en->appearence = !en->appearence;
            field[en->y-1][en->x+1-1] = en->appearences[0];

    // Draw the character in the new position
            wmove(win, en->y, en->x+1);
            printwc(en->appearences[en->appearence], win);
            en->time -= 200;
            en->x++;

        }
    }


}

void death_animation(struct character *ch, struct bomb *bm, int rows, int cols, char field[rows][cols], WINDOW *win, int i){   
    char appearences[] = {'<', '^', '>', 'v', 'X'};
    wattron(win, COLOR_PAIR(1));
    mvwaddch(win, ch->y, ch->x, appearences[i]);
    wattroff(win, COLOR_PAIR(1));
    // mvwprintw(win, rows+1, 0, "Success %d", i);
}

void next_level_symbol_pos(struct transition *tr, int rows, int cols, char field[rows][cols], WINDOW *win, char field_under[rows][cols]){
    int x, y;
    while (1){
        x = rand() % ((cols-1) - 4 + 1) + 4;
        y = rand() % ((rows-1) - 2 + 1) + 2;     
        if (field[y][x] == '@'){
            field_under[y][x] = '%';
            tr->x = x+1;
            tr->y = y+1;
            return;
        }

    }
}

int update_next_level_symbol(struct transition *tr, int rows, int cols, char field[rows][cols], WINDOW *win){
    if (field[tr->y-1][tr->x-1] == ' '){
        int colors[] = {3, 4, 7, 3, 6};
        wattron(win, COLOR_PAIR(colors[tr->color]));
        mvwaddch(win, tr->y, tr->x, tr->appearence);
        wattroff(win, COLOR_PAIR(colors[tr->color]));
        if (tr->color >= 4) tr->color = 0;
        tr->color++;
    }
    if (field[tr->y-1][tr->x-1] == '<' || field[tr->y-1][tr->x-1] == '^' || field[tr->y-1][tr->x-1] == '>' || field[tr->y-1][tr->x-1] == 'v'){
        return 1;
    }
    return 0;
}

void powerup_symbol_pos(struct powerup *pw, int rows, int cols, char field[rows][cols], WINDOW *win, char field_under[rows][cols]){
    int x, y;
    while (1){
        x = rand() % ((cols-1) - 4 + 1) + 4;
        y = rand() % ((rows-1) - 2 + 1) + 2;     
        if (field[y][x] == '@' && (field_under[y][x] != '%' && field_under[y][x] != '$')){
            field_under[y][x] = '$';
            pw->x = x+1;
            pw->y = y+1;
            return;
        }

    }
}

void update_powerup_symbol(struct character *ch, struct powerup *pw, struct bomb *bm, int rows, int cols, char field[rows][cols], WINDOW *win){
    if (field[pw->y-1][pw->x-1] == ' ' && !pw->taken){
        int colors[] = {3, 4, 7, 3, 6};
        wattron(win, COLOR_PAIR(colors[pw->color]));
        mvwaddch(win, pw->y, pw->x, pw->appearence);
        wattroff(win, COLOR_PAIR(colors[pw->color]));
        if (pw->color >= 4) pw->color = 0;
        pw->color++;
    }
    if ((ch->x == pw->x) && (ch->y == pw->y) && !pw->taken){
        pw->taken = true;
        bm->radius++;
    }
}

void copy_array(int rows, int cols, char field[rows][cols], char field_under[rows][cols]){
    for (int row = 0; row < rows; row++){
        for (int col = 0; col < cols; col++){
            field_under[row][col] = field[row][col];
        }
    }
}

void spawn_enemy2(int rows, int cols, char field[rows][cols], struct enemy2 *en, WINDOW *win){
    int x, y;
    while (1){
        x = rand() % ((cols-1) - 4 + 1) + 4;
        y = rand() % ((rows-1) - 4 + 1) + 4;     
        if (field[y][x] == ' '){
            en->x = x+1;
            en->y = y+1;
            field[y][x] = en->appearences[0];
            wmove(win, en->y, en->x);
            printwc(en->appearences[0], win);
            return;
        }
    }
}

void update_enemy_pos2(struct enemy2 *en, struct bomb *bm, int rows, int cols, char field[rows][cols], WINDOW *win, struct character *ch) {
    if (!en->alive){
        return;
    }
    if (field[en->y-1][en->x-1] == '*'){
        ch->score += 200;
        en->alive = false;
        return; // giving points
    }
    if (en->y < 1 || en->y > rows || en->x < 1 || en->x > cols) {
    // Enemy is out of bounds; reset its position
        spawn_enemy2(rows, cols, field, en, win);
        return;
    }
    if (en->time <= 0){
        en->time = 200;
        return;
    }
    // Clear previous position

    if (field[en->y-1][en->x-1] != '@'){
        field[en->y-1][en->x-1] = ' ';
        mvwaddch(win, en->y, en->x, ' ');
    }
    else{
        wmove(win,  en->y, en->x);
        printwc('@', win);
    }
    // Determine direction based on input

    // Update field with character's appearance
    // Clear previous position on screen
    


    if (en->direction == 1){ // 1 N 2 W 3 S 4 E
        if (field[en->y-1-1][en->x-1] == 'O' || field[en->y-1-1][en->x-1] == '#' || field[en->y-1-1][en->x-1] == '-'){
            while (en->direction == 1){
                en->direction = rand() % 4 + 1;
            }
            en->time -= 200;
            en->appearence = !en->appearence;
            field[en->y-1][en->x-1] = en->appearences[0];

    // Draw the character in the new position
            wmove(win, en->y, en->x);
            printwc(en->appearences[en->appearence], win);
        }
        else{
            if (field[en->y-1-1][en->x-1] == '@'){
                en->y--;
                en->time -= 200;
                wmove(win, en->y-1+1, en->x);
                wattron(win, COLOR_PAIR(8));
                waddch(win, '@'); // Render obstacles as #
                wattroff(win, COLOR_PAIR(8));
            }
            else{
                en->appearence = !en->appearence;
                field[en->y-1-1][en->x-1] = en->appearences[0];
        // Draw the character in the new position
                wmove(win, en->y-1, en->x);
                printwc(en->appearences[en->appearence], win);
                en->y--;
                en->time -= 200;
                en->direction = rand() % 4 + 1;
            }
        }
    }
    else if (en->direction == 2){ // 1 N 2 W 3 S 4 E
        if (field[en->y-1][en->x-1-1] == 'O' || field[en->y-1][en->x-1-1] == '#' || field[en->y-1][en->x-1-1] == '|'){
            while (en->direction == 2){
                en->direction = rand() % 4 + 1;
            }
            en->time -= 200;
            en->appearence = !en->appearence;
            field[en->y-1][en->x-1] = en->appearences[0];

    // Draw the character in the new position
            wmove(win, en->y, en->x);
            printwc(en->appearences[en->appearence], win);
        }
        else{
            if (field[en->y-1][en->x-1-1] == '@'){
                en->x--;
                en->time -= 200;
                wmove(win, en->y, en->x-1+1);
                wattron(win, COLOR_PAIR(8));
                waddch(win, '@'); // Render obstacles as #
                wattroff(win, COLOR_PAIR(8));

            }
            else{
                en->appearence = !en->appearence;
                field[en->y-1][en->x-1-1] = en->appearences[0];

        // Draw the character in the new position
                wmove(win, en->y, en->x-1);
                printwc(en->appearences[en->appearence], win);
                en->time -= 200;
                en->x--;
                en->direction = rand() % 4 + 1;
            }
        }
    }
    else if (en->direction == 3){ // 1 N 2 W 3 S 4 E
        if (field[en->y+1-1][en->x-1] == 'O' || field[en->y+1-1][en->x-1] == '#' || field[en->y+1-1][en->x-1] == '-'){
            while (en->direction == 3){
                en->direction = rand() % 4 + 1;
            }
            en->time -= 200;
            en->appearence = !en->appearence;
            field[en->y-1][en->x-1] = en->appearences[0];

    // Draw the character in the new position
            wmove(win, en->y, en->x);
            printwc(en->appearences[en->appearence], win);

        }
        else{
            if (field[en->y+1-1][en->x-1] == '@'){
                en->y++;
                en->time -= 200;
                wmove(win, en->y+1-1, en->x);
                wattron(win, COLOR_PAIR(8));
                waddch(win, '@'); // Render obstacles as #
                wattroff(win, COLOR_PAIR(8));

            }
            else{
                en->appearence = !en->appearence;
                field[en->y+1-1][en->x-1] = en->appearences[0];

        // Draw the character in the new position
                wmove(win, en->y+1, en->x);
                printwc(en->appearences[en->appearence], win);
                en->time -= 200;
                en->y++;
                en->direction = rand() % 4 + 1;
            }
        }
    }
    else if (en->direction == 4){ // 1 N 2 W 3 S 4 E
        if (field[en->y-1][en->x+1-1] == 'O' || field[en->y-1][en->x+1-1] == '#' || field[en->y-1][en->x+1-1] == '|'){
            while (en->direction == 4){
                en->direction = rand() % 4 + 1;
            }
            en->time -= 200;
            // en->appearence = !en->appearence;
            field[en->y-1][en->x-1] = en->appearences[0];

    // Draw the character in the new position
            wmove(win, en->y, en->x);
            printwc(en->appearences[en->appearence], win);
        }
        else{
            if (field[en->y-1][en->x+1-1] == '@'){
                en->x++;
                en->time -= 200;
                wmove(win, en->y, en->x+1-1);
                wattron(win, COLOR_PAIR(8));
                waddch(win, '@'); // Render obstacles as #
                wattroff(win, COLOR_PAIR(8));

            }
            else{
                en->appearence = !en->appearence;
                field[en->y-1][en->x+1-1] = en->appearences[0];

        // Draw the character in the new position
                wmove(win, en->y, en->x+1);
                printwc(en->appearences[en->appearence], win);
                en->time -= 200;
                en->x++;
                en->direction = rand() % 4 + 1;
            }

        }
    }


}

int lvl1(struct character *ch, struct bomb *bm, int rows, int cols, char field[rows][cols], char field_under[rows][cols], WINDOW *win){
    initialize_field(rows, cols, field);
    copy_array(rows, cols, field, field_under);
    print_board(rows, cols, field, win);
    field[ch->y-1][ch->x-1] = ch->appearences[ch->direction];
    wmove(win, ch->y, ch->x);
    printwc(ch->appearences[ch->direction], win);
    struct lvl lvl1 = {5, 0, 0, false};
    struct enemy enemy_1 = {999, 999, 0, {'Z', 'N'}, 6, rand() % 4 + 1, 0, true};
    struct enemy enemy_2 = {999, 999, 0, {'Z', 'N'}, 6, rand() % 4 + 1, 0, true};
    struct enemy enemy_3 = {999, 999, 0, {'Z', 'N'}, 6, rand() % 4 + 1, 0, true};
    struct enemy enemy_4 = {999, 999, 0, {'Z', 'N'}, 6, rand() % 4 + 1, 0, true};
    struct enemy enemy_5 = {999, 999, 0, {'Z', 'N'}, 6, rand() % 4 + 1, 0, true};
    // struct enemy2 enemy2_1 = {999, 999, 0, {'p', 'd'}, 6, rand() % 4 + 1, 0, true};
    // struct enemy2 enemy2_2 = {999, 999, 0, {'p', 'd'}, 6, rand() % 4 + 1, 0, true};
    struct transition trans = {999, 999, '%', 0};
    struct powerup power = {9999, 9999, '$', 0, false};
    struct powerup power2 = {9999, 9999, '$', 0, false};
    spawn_enemy(rows, cols, field, &enemy_1, win);
    spawn_enemy(rows, cols, field, &enemy_2, win);
    spawn_enemy(rows, cols, field, &enemy_3, win);
    spawn_enemy(rows, cols, field, &enemy_4, win);
    spawn_enemy(rows, cols, field, &enemy_5, win);
    // spawn_enemy2(rows, cols, field, &enemy2_1, win);
    // spawn_enemy2(rows, cols, field, &enemy2_2, win);
    next_level_symbol_pos(&trans, rows, cols, field, win, field_under);
    powerup_symbol_pos(&power, rows, cols, field, win, field_under);
    powerup_symbol_pos(&power2, rows, cols, field, win, field_under);
    wrefresh(win);


    struct timespec ts = {
        .tv_sec = 0,                    // nr of secs
        .tv_nsec = 100 * 1000000L  // nr of nanosecs
    };

    struct timespec ts2 = {
        .tv_sec = 0,                    // nr of secs
        .tv_nsec = 600 * 1000000L  // nr of nanosecs
    };
    


    int count_animation_death = 1;
    int input;
    while (1) {
        remove_stars(rows, cols, field, win);
        // mvprintw(LINES-4, 0, "                    ");
        // mvprintw(LINES-4, 0, "Time: %d", bomba.time);
        input = wgetch(win);
        // wprintw(parent_win, "test %d", input);

         // Get user input
        if (input == 'q') break; // Exit the loop when 'q' is pressed
        bomb(ch, bm, rows, cols, field, input, win);
        if ((field[ch->y-1][ch->x-1] == '*' || field[ch->y-1][ch->x-1] == 'Z' || field[ch->y-1][ch->x-1] == 'N'|| field[ch->y-1][ch->x-1] == 'p'|| field[ch->y-1][ch->x-1] == 'd' || !ch->alive)){
            mvwaddch(win, bm->y, bm->x, ' ');
            ch->alive = false;
            remove_stars(rows, cols, field, win);
            for (int i = 0; i < 5; i++){
                death_animation(ch, bm, rows, cols, field, win, i);
                wrefresh(win);
                nanosleep(&ts2, NULL);
            }
            break; //loose
        }
        update_enemy_pos(&enemy_1, bm, rows, cols, field, win, ch);
        update_enemy_pos(&enemy_2, bm, rows, cols, field, win, ch);
        update_enemy_pos(&enemy_3, bm, rows, cols, field, win, ch);
        update_enemy_pos(&enemy_4, bm, rows, cols, field, win, ch);
        update_enemy_pos(&enemy_5, bm, rows, cols, field, win, ch);
        // update_enemy_pos2(&enemy2_1, bm, rows, cols, field, win, ch);
        // update_enemy_pos2(&enemy2_2, bm, rows, cols, field, win, ch);
        if ((field[ch->y-1][ch->x-1] == '*' || field[ch->y-1][ch->x-1] == 'Z' || field[ch->y-1][ch->x-1] == 'N' || field[ch->y-1][ch->x-1] == 'p'|| field[ch->y-1][ch->x-1] == 'd'|| !ch->alive)){
            mvwaddch(win, bm->y, bm->x, ' ');
            ch->alive = false;
            for (int i = 0; i < 5; i++){
                death_animation(ch, bm, rows, cols, field, win, i);
                wrefresh(win);
                nanosleep(&ts2, NULL);
            }
            break;
        }
        update_character_position(ch, bm, rows, cols, field, input, win);
        if ((field[ch->y-1][ch->x-1] == '*' || field[ch->y-1][ch->x-1] == 'Z' || field[ch->y-1][ch->x-1] == 'N' || field[ch->y-1][ch->x-1] == 'p'|| field[ch->y-1][ch->x-1] == 'd'|| !ch->alive)){
            mvwaddch(win, bm->y, bm->x, ' ');
            ch->alive = false;
            for (int i = 0; i < 5; i++){
                death_animation(ch, bm, rows, cols, field, win, i);
                wrefresh(win);
                nanosleep(&ts2, NULL);
            }
            break;
        }


        int check = update_next_level_symbol(&trans, rows, cols, field, win);
        if (check) break;
        update_powerup_symbol(ch, &power, bm, rows, cols, field, win);
        update_powerup_symbol(ch, &power2, bm, rows, cols, field, win);
        mvwprintw(win, rows+2, 1, "LEVEL 1");
        // wprintw(win, " Element : %c.", field[power.y-1][power.x-1]);
        // mvwprintw(parent_win, win_height-2, 0, "Character -> y : %d, x : %d.", player.y, player.x);
        // wprintw(parent_win, " Element : %c.", field[player.y-1][player.x-1]);
        clear_getch_buffer(win); // Update character position
        wrefresh(win); // Refresh the screen to reflect changes
        nanosleep(&ts, NULL);
        mvwprintw(win, rows+2, (cols + 3 - 11) / 2, "SCORE : 0000");
        if (ch->score < 1000) mvwprintw(win, rows+2, (cols+3 - 11) / 2 + 9 , "%d", ch->score);
        else mvwprintw(win, rows+2, (cols + 3 - 11) / 2 + 8 , "%d", ch->score);
    }
    if (!ch->alive) return 0;
    return 1;

    
}

int lvl2(struct character *ch, struct bomb *bm, int rows, int cols, char field[rows][cols], char field_under[rows][cols], WINDOW *win){
    initialize_field(rows, cols, field);
    copy_array(rows, cols, field, field_under);
    print_board(rows, cols, field, win);
    field[ch->y-1][ch->x-1] = ch->appearences[ch->direction];
    wmove(win, ch->y, ch->x);
    printwc(ch->appearences[ch->direction], win);
    struct lvl lvl1 = {5, 0, 0, false};
    struct enemy enemy_1 = {999, 999, 0, {'Z', 'N'}, 6, rand() % 4 + 1, 0, true};
    struct enemy enemy_2 = {999, 999, 0, {'Z', 'N'}, 6, rand() % 4 + 1, 0, true};
    struct enemy enemy_3 = {999, 999, 0, {'Z', 'N'}, 6, rand() % 4 + 1, 0, true};
    struct enemy enemy_4 = {999, 999, 0, {'Z', 'N'}, 6, rand() % 4 + 1, 0, true};
    struct enemy enemy_5 = {999, 999, 0, {'Z', 'N'}, 6, rand() % 4 + 1, 0, true};
    struct enemy enemy_6 = {999, 999, 0, {'Z', 'N'}, 6, rand() % 4 + 1, 0, true};
    struct enemy enemy_7 = {999, 999, 0, {'Z', 'N'}, 6, rand() % 4 + 1, 0, true};
    struct enemy2 enemy2_1 = {999, 999, 0, {'p', 'd'}, 6, rand() % 4 + 1, 0, true};
    struct enemy2 enemy2_2 = {999, 999, 0, {'p', 'd'}, 6, rand() % 4 + 1, 0, true};
    struct transition trans = {999, 999, '%', 0};
    struct powerup power = {9999, 9999, '$', 0, false};
    struct powerup power2 = {9999, 9999, '$', 0, false};
    spawn_enemy(rows, cols, field, &enemy_1, win);
    spawn_enemy(rows, cols, field, &enemy_2, win);
    spawn_enemy(rows, cols, field, &enemy_3, win);
    spawn_enemy(rows, cols, field, &enemy_4, win);
    spawn_enemy(rows, cols, field, &enemy_5, win);
    spawn_enemy(rows, cols, field, &enemy_6, win);
    spawn_enemy(rows, cols, field, &enemy_7, win);
    spawn_enemy2(rows, cols, field, &enemy2_1, win);
    spawn_enemy2(rows, cols, field, &enemy2_2, win);

    next_level_symbol_pos(&trans, rows, cols, field, win, field_under);
    powerup_symbol_pos(&power, rows, cols, field, win, field_under);
    powerup_symbol_pos(&power2, rows, cols, field, win, field_under);
    wrefresh(win);


    struct timespec ts = {
        .tv_sec = 0,                    // nr of secs
        .tv_nsec = 100 * 1000000L  // nr of nanosecs
    };

    struct timespec ts2 = {
        .tv_sec = 0,                    // nr of secs
        .tv_nsec = 600 * 1000000L  // nr of nanosecs
    };
    


    int count_animation_death = 1;
    int input;
    while (1) {
        remove_stars(rows, cols, field, win);
        // mvprintw(LINES-4, 0, "                    ");
        // mvprintw(LINES-4, 0, "Time: %d", bomba.time);
        input = wgetch(win);
        // wprintw(parent_win, "test %d", input);

         // Get user input
        if (input == 'q') break; // Exit the loop when 'q' is pressed
        bomb(ch, bm, rows, cols, field, input, win);
        if ((field[ch->y-1][ch->x-1] == '*' || field[ch->y-1][ch->x-1] == 'Z' || field[ch->y-1][ch->x-1] == 'N' || field[ch->y-1][ch->x-1] == 'p'|| field[ch->y-1][ch->x-1] == 'd' || !ch->alive)){
            mvwaddch(win, bm->y, bm->x, ' ');
            ch->alive = false;
            remove_stars(rows, cols, field, win);
            for (int i = 0; i < 5; i++){
                death_animation(ch, bm, rows, cols, field, win, i);
                wrefresh(win);
                nanosleep(&ts2, NULL);
            }
            break; //loose
        }
        update_enemy_pos(&enemy_1, bm, rows, cols, field, win, ch);
        update_enemy_pos(&enemy_2, bm, rows, cols, field, win, ch);
        update_enemy_pos(&enemy_3, bm, rows, cols, field, win, ch);
        update_enemy_pos(&enemy_4, bm, rows, cols, field, win, ch);
        update_enemy_pos(&enemy_5, bm, rows, cols, field, win, ch);
        update_enemy_pos(&enemy_6, bm, rows, cols, field, win, ch);
        update_enemy_pos(&enemy_7, bm, rows, cols, field, win, ch);
        update_enemy_pos2(&enemy2_1, bm, rows, cols, field, win, ch);
        update_enemy_pos2(&enemy2_2, bm, rows, cols, field, win, ch);
        if ((field[ch->y-1][ch->x-1] == '*' || field[ch->y-1][ch->x-1] == 'Z' || field[ch->y-1][ch->x-1] == 'N' || field[ch->y-1][ch->x-1] == 'p'|| field[ch->y-1][ch->x-1] == 'd' || !ch->alive)){
            mvwaddch(win, bm->y, bm->x, ' ');
            ch->alive = false;
            for (int i = 0; i < 5; i++){
                death_animation(ch, bm, rows, cols, field, win, i);
                wrefresh(win);
                nanosleep(&ts2, NULL);
            }
            break;
        }
        update_character_position(ch, bm, rows, cols, field, input, win);
        if ((field[ch->y-1][ch->x-1] == '*' || field[ch->y-1][ch->x-1] == 'Z' || field[ch->y-1][ch->x-1] == 'N' || field[ch->y-1][ch->x-1] == 'p'|| field[ch->y-1][ch->x-1] == 'd'|| !ch->alive)){
            mvwaddch(win, bm->y, bm->x, ' ');
            ch->alive = false;
            for (int i = 0; i < 5; i++){
                death_animation(ch, bm, rows, cols, field, win, i);
                wrefresh(win);
                nanosleep(&ts2, NULL);
            }
            break;
        }
        int check = update_next_level_symbol(&trans, rows, cols, field, win);
        if (check) break;
        update_powerup_symbol(ch, &power, bm, rows, cols, field, win);
        update_powerup_symbol(ch, &power2, bm, rows, cols, field, win);
        mvwprintw(win, rows+2, 1, "LEVEL 2");
        // mvwprintw(parent_win, win_height-2, 0, "Powerup -> y : %d, x : %d.", power.y, power.x);
        // wprintw(parent_win, " Element : %c.", field[power.y-1][power.x-1]);
        // wprintw(parent_win, " Radius : %d.", bomba.radius);
        // mvwprintw(parent_win, win_height-2, 0, "Character -> y : %d, x : %d.", player.y, player.x);
        // wprintw(parent_win, " Element : %c.", field[player.y-1][player.x-1]);
        clear_getch_buffer(win); // Update character position
        wrefresh(win); // Refresh the screen to reflect changes
        nanosleep(&ts, NULL);
        mvwprintw(win, rows+2, (cols + 3 - 11) / 2, "SCORE : 0000");
        if (ch->score < 1000) mvwprintw(win, rows+2, (cols+3 - 11) / 2 + 9 , "%d", ch->score);
        else mvwprintw(win, rows+2, (cols + 3 - 11) / 2 + 8 , "%d", ch->score);
    }
    if (!ch->alive) return 0;
    return 1;
}

int lvl3(struct character *ch, struct bomb *bm, int rows, int cols, char field[rows][cols], char field_under[rows][cols], WINDOW *win){
    initialize_field(rows, cols, field);
    copy_array(rows, cols, field, field_under);
    print_board(rows, cols, field, win);
    field[ch->y-1][ch->x-1] = ch->appearences[ch->direction];
    wmove(win, ch->y, ch->x);
    printwc(ch->appearences[ch->direction], win);
    struct lvl lvl1 = {5, 0, 0, false};
    struct enemy enemy_1 = {999, 999, 0, {'Z', 'N'}, 6, rand() % 4 + 1, 0, true};
    struct enemy enemy_2 = {999, 999, 0, {'Z', 'N'}, 6, rand() % 4 + 1, 0, true};
    struct enemy enemy_3 = {999, 999, 0, {'Z', 'N'}, 6, rand() % 4 + 1, 0, true};
    struct enemy enemy_4 = {999, 999, 0, {'Z', 'N'}, 6, rand() % 4 + 1, 0, true};
    struct enemy enemy_5 = {999, 999, 0, {'Z', 'N'}, 6, rand() % 4 + 1, 0, true};
    struct enemy enemy_6 = {999, 999, 0, {'Z', 'N'}, 6, rand() % 4 + 1, 0, true};
    struct enemy enemy_7 = {999, 999, 0, {'Z', 'N'}, 6, rand() % 4 + 1, 0, true};
    struct enemy enemy_8 = {999, 999, 0, {'Z', 'N'}, 6, rand() % 4 + 1, 0, true};
    struct enemy enemy_9 = {999, 999, 0, {'Z', 'N'}, 6, rand() % 4 + 1, 0, true};
    struct enemy enemy_10 = {999, 999, 0, {'Z', 'N'}, 6, rand() % 4 + 1, 0, true};
    struct enemy2 enemy2_1 = {999, 999, 0, {'p', 'd'}, 6, rand() % 4 + 1, 0, true};
    struct enemy2 enemy2_2 = {999, 999, 0, {'p', 'd'}, 6, rand() % 4 + 1, 0, true};
    struct enemy2 enemy2_3 = {999, 999, 0, {'p', 'd'}, 6, rand() % 4 + 1, 0, true};
    struct enemy2 enemy2_4 = {999, 999, 0, {'p', 'd'}, 6, rand() % 4 + 1, 0, true};
    struct transition trans = {999, 999, '%', 0};
    struct powerup power = {9999, 9999, '$', 0, false};
    struct powerup power2 = {9999, 9999, '$', 0, false};
    struct powerup power3 = {9999, 9999, '$', 0, false};
    struct powerup power4 = {9999, 9999, '$', 0, false};
    struct powerup power5 = {9999, 9999, '$', 0, false};
    struct powerup power6 = {9999, 9999, '$', 0, false};
    struct powerup power7 = {9999, 9999, '$', 0, false};
    struct powerup power8 = {9999, 9999, '$', 0, false};
    struct powerup power9 = {9999, 9999, '$', 0, false};
    struct powerup power10 = {9999, 9999, '$', 0, false};
    spawn_enemy(rows, cols, field, &enemy_1, win);
    spawn_enemy(rows, cols, field, &enemy_2, win);
    spawn_enemy(rows, cols, field, &enemy_3, win);
    spawn_enemy(rows, cols, field, &enemy_4, win);
    spawn_enemy(rows, cols, field, &enemy_5, win);
    spawn_enemy(rows, cols, field, &enemy_6, win);
    spawn_enemy(rows, cols, field, &enemy_7, win);
    spawn_enemy(rows, cols, field, &enemy_8, win);
    spawn_enemy(rows, cols, field, &enemy_9, win);
    spawn_enemy(rows, cols, field, &enemy_10, win);
    spawn_enemy2(rows, cols, field, &enemy2_1, win);
    spawn_enemy2(rows, cols, field, &enemy2_2, win);
    spawn_enemy2(rows, cols, field, &enemy2_3, win);
    spawn_enemy2(rows, cols, field, &enemy2_4, win);

    next_level_symbol_pos(&trans, rows, cols, field, win, field_under);
    powerup_symbol_pos(&power, rows, cols, field, win, field_under);
    powerup_symbol_pos(&power2, rows, cols, field, win, field_under);
    powerup_symbol_pos(&power3, rows, cols, field, win, field_under);
    powerup_symbol_pos(&power4, rows, cols, field, win, field_under);
    powerup_symbol_pos(&power5, rows, cols, field, win, field_under);
    powerup_symbol_pos(&power6, rows, cols, field, win, field_under);
    powerup_symbol_pos(&power7, rows, cols, field, win, field_under);
    powerup_symbol_pos(&power8, rows, cols, field, win, field_under);
    powerup_symbol_pos(&power9, rows, cols, field, win, field_under);
    powerup_symbol_pos(&power10, rows, cols, field, win, field_under);
    wrefresh(win);


    struct timespec ts = {
        .tv_sec = 0,                    // nr of secs
        .tv_nsec = 100 * 1000000L  // nr of nanosecs
    };

    struct timespec ts2 = {
        .tv_sec = 0,                    // nr of secs
        .tv_nsec = 600 * 1000000L  // nr of nanosecs
    };
    


    int count_animation_death = 1;
    int input;
    while (1) {
        remove_stars(rows, cols, field, win);
        // mvprintw(LINES-4, 0, "                    ");
        // mvprintw(LINES-4, 0, "Time: %d", bomba.time);
        input = wgetch(win);
        // wprintw(parent_win, "test %d", input);

         // Get user input
        if (input == 'q') break; // Exit the loop when 'q' is pressed
        bomb(ch, bm, rows, cols, field, input, win);
        if ((field[ch->y-1][ch->x-1] == '*' || field[ch->y-1][ch->x-1] == 'Z' || field[ch->y-1][ch->x-1] == 'N' || field[ch->y-1][ch->x-1] == 'p'|| field[ch->y-1][ch->x-1] == 'd' || !ch->alive)){
            mvwaddch(win, bm->y, bm->x, ' ');
            ch->alive = false;
            remove_stars(rows, cols, field, win);
            for (int i = 0; i < 5; i++){
                death_animation(ch, bm, rows, cols, field, win, i);
                wrefresh(win);
                nanosleep(&ts2, NULL);
            }
            break; //loose
        }
        update_enemy_pos(&enemy_1, bm, rows, cols, field, win, ch);
        update_enemy_pos(&enemy_2, bm, rows, cols, field, win, ch);
        update_enemy_pos(&enemy_3, bm, rows, cols, field, win, ch);
        update_enemy_pos(&enemy_4, bm, rows, cols, field, win, ch);
        update_enemy_pos(&enemy_5, bm, rows, cols, field, win, ch);
        update_enemy_pos(&enemy_6, bm, rows, cols, field, win, ch);
        update_enemy_pos(&enemy_7, bm, rows, cols, field, win, ch);
        update_enemy_pos(&enemy_8, bm, rows, cols, field, win, ch);
        update_enemy_pos(&enemy_9, bm, rows, cols, field, win, ch);
        update_enemy_pos(&enemy_10, bm, rows, cols, field, win, ch);
        update_enemy_pos2(&enemy2_1, bm, rows, cols, field, win, ch);
        update_enemy_pos2(&enemy2_2, bm, rows, cols, field, win, ch);
        update_enemy_pos2(&enemy2_3, bm, rows, cols, field, win, ch);
        update_enemy_pos2(&enemy2_4, bm, rows, cols, field, win, ch);
        if ((field[ch->y-1][ch->x-1] == '*' || field[ch->y-1][ch->x-1] == 'Z' || field[ch->y-1][ch->x-1] == 'N' || field[ch->y-1][ch->x-1] == 'p'|| field[ch->y-1][ch->x-1] == 'd' || !ch->alive)){
            mvwaddch(win, bm->y, bm->x, ' ');
            ch->alive = false;
            for (int i = 0; i < 5; i++){
                death_animation(ch, bm, rows, cols, field, win, i);
                wrefresh(win);
                nanosleep(&ts2, NULL);
            }
            break;
        }
        update_character_position(ch, bm, rows, cols, field, input, win);
        if ((field[ch->y-1][ch->x-1] == '*' || field[ch->y-1][ch->x-1] == 'Z' || field[ch->y-1][ch->x-1] == 'N' || field[ch->y-1][ch->x-1] == 'p'|| field[ch->y-1][ch->x-1] == 'd'|| !ch->alive)){
            mvwaddch(win, bm->y, bm->x, ' ');
            ch->alive = false;
            for (int i = 0; i < 5; i++){
                death_animation(ch, bm, rows, cols, field, win, i);
                wrefresh(win);
                nanosleep(&ts2, NULL);
            }
            break;
        }
        int check = update_next_level_symbol(&trans, rows, cols, field, win);
        if (check) break;
        update_powerup_symbol(ch, &power, bm, rows, cols, field, win);
        update_powerup_symbol(ch, &power2, bm, rows, cols, field, win);
        mvwprintw(win, rows+2, 1, "LEVEL 3");
        // mvwprintw(parent_win, win_height-2, 0, "Powerup -> y : %d, x : %d.", power.y, power.x);
        // wprintw(parent_win, " Element : %c.", field[power.y-1][power.x-1]);
        // wprintw(parent_win, " Radius : %d.", bomba.radius);
        // mvwprintw(parent_win, win_height-2, 0, "Character -> y : %d, x : %d.", player.y, player.x);
        // wprintw(parent_win, " Element : %c.", field[player.y-1][player.x-1]);
        clear_getch_buffer(win); // Update character position
        wrefresh(win); // Refresh the screen to reflect changes
        nanosleep(&ts, NULL);
        mvwprintw(win, rows+2, (cols + 3 - 11) / 2, "SCORE : 0000");
        if (ch->score < 1000) mvwprintw(win, rows+2, (cols+3 - 11) / 2 + 9 , "%d", ch->score);
        else mvwprintw(win, rows+2, (cols + 3 - 11) / 2 + 8 , "%d", ch->score);
    }
    if (!ch->alive) return 0;
    return 1;
}


void starting_screen(){
    char logo[6][60] = {
        " ____                  _                                     ",
        "| __ )  ___  _ __ ___ | |__   ___ _ __ _ __ ___   __ _ _ __  ",
        "|  _ \\ / _ \\| '_ ` _ \\| '_ \\ / _ \\ '__| '_ ` _ \\ / _` | '_ \\ ",
        "| |_) | (_) | | | | | | |_) |  __/ |  | | | | | | (_| | | | |",
        "|____/ \\___/|_| |_| |_|_.__/ \\___|_|  |_| |_| |_|\__,_|_| |_|",
        "                                                              "
    };
    int len = strlen(logo[0]);
    for (int i = 0; i < 5; i++){
        move(3 + i, (COLS - len) / 2);
        for (int j = 0; j < len; j++){
            if (j > 5 && j < 12){
                // move(3 + i, (COLS - len) / 2);
                attron(COLOR_PAIR(5));
                printw("%c", logo[i][j]);
                attroff(COLOR_PAIR(5));
            }
            else{
                // move(3 + i, (COLS - len) / 2);
                printw("%c", logo[i][j]);
            }
        }
    }
    refresh();


}

void clear_shevrons(char prev[], int y_prev){
    int prev_len = strlen(prev);
    int prev_start = (COLS - prev_len) / 2;
    move(y_prev, prev_start - 1);  // Move to the position of the left chevron for `prev`
    printw(" ");                  // Clear left chevron
    move(y_prev, prev_start + prev_len); // Move to the position of the right chevron for `prev`
    printw(" ");                 // Print right chevron
}


void print_shevrons(char str[], int y) {
    // Print new chevrons
    int len = strlen(str);
    int start = (COLS - len) / 2; // Center position for `str`
    move(y, start - 1);           // Move to the position of the left chevron for `str`
    printw("<");                  // Print left chevron
    move(y, start + len);         // Move to the position of the right chevron for `str`
    printw(">");                  // Print right chevron
}

int choose(int input, int *pos) {
    char start[] = "START";
    char info[] = "INFORMATION";
    char exit[] = "EXIT";
    char made[] = "MADE BY MYKHAILO OLESKIV";

    // Always print the options (static part of the menu)
    move(13, (COLS - strlen(start)) / 2);
    printw("%s", start);
    move(15, (COLS - strlen(info)) / 2);
    printw("%s", info);
    move(17, (COLS - strlen(exit)) / 2);
    printw("%s", exit);
    move(LINES - 2, (COLS - strlen(made)) / 2);
    printw("%s", made);

    // Track previous position for chevron clearing
    static int prev_pos = 1;

    // Handle input and update `pos`
    switch (input) {
        case KEY_UP:
            *pos = (*pos > 0) ? *pos - 1 : *pos; // Move up
            break;
        case KEY_DOWN:
            *pos = (*pos < 2) ? *pos + 1 : *pos; // Move down
            break;
        case '\n': // Enter key
            return *pos; // Return the selected option (0, 1, or 2)
        default:
            break; // Do nothing for other keys
    }

    // Update chevrons only if the position changes
    if (prev_pos == 0) clear_shevrons(start, 13); // Clear chevrons for "START"
    else if (prev_pos == 1) clear_shevrons(info, 15); // Clear chevrons for "INFORMATION"
    else if (prev_pos == 2) clear_shevrons(exit, 17); // Clear chevrons for "EXIT"

    if (*pos == 0) print_shevrons(start, 13); // Add chevrons for "START"
    else if (*pos == 1) print_shevrons(info, 15); // Add chevrons for "INFORMATION"
    else if (*pos == 2) print_shevrons(exit, 17); // Add chevrons for "EXIT"

    prev_pos = *pos; // Update previous position

    return -1; // No valid selection yet
}

void info(){
    clear();
    cbreak();
    nodelay(stdscr, FALSE);
    mvprintw(3, 3, "-This game is my own interpretation of the Nintendo game BOMBERMAN.");
    mvprintw(5, 3, "-You can move the character by pressing the arrow keys.");
    mvprintw(7, 3, "-You can place bombs by pressing \"b\" on your keyboard. The bombs can break yellow wals ");
    attron(COLOR_PAIR(4));
    addch('@');
    attroff(COLOR_PAIR(4));
    printw(" and kill enemies.");
    mvprintw(9, 3, "-There are currently two kind of enemies: ");
    attron(COLOR_PAIR(6));
    addch('Z');
    attroff(COLOR_PAIR(6));
    printw(" zombie and ");
    attron(COLOR_PAIR(8));
    addch('p');
    attroff(COLOR_PAIR(8));
    printw(" ghost. For killing a zombie you gain a 100 points, while for killing a ghost 200.");
    mvprintw(11, 3, "-You can find powerups after destroying walls, which have a look of a $ sign. By picking one up you increase your bomb radius by 1.");
    mvprintw(13, 3, "-In order to go to the next level you need to find \"%%\" .");
    mvprintw(15, 3, "-GOOD LUCK!");
    getch();
    clear();
    nodelay(stdscr, TRUE);
}


int game(){
    nodelay(stdscr, TRUE);
    curs_set(FALSE);
    srand(time(NULL));
    keypad(stdscr, TRUE);
    refresh();
    noecho();
    int pos = 0;
    starting_screen(); // Initial selection is "START"
    choose(KEY_UP, &pos); // Initialize the menu and print chevrons
    refresh();
    while (1) {
        starting_screen();
        int input_choose = getch(); // Wait for user input
        int result = choose(input_choose, &pos); // Process the input

        if (result == 0) break; // "START" selected
        if (result == 1) info(); // Show "INFORMATION"
        if (result == 2) { endwin(); exit(0); } // Exit the program
        if (result == -1) 
        refresh(); // Refresh the screen to reflect changes
    }

    wclear(stdscr);
    refresh();
    // Board dimensions
    keypad(stdscr, FALSE);
    int rows = 13;
    int cols = 51;
    char field[rows][cols];
    char field_under[rows][cols];

    int win_height = rows + 3; // Extra space for border
    int win_width = cols + 3;
    int win_starty = (LINES - win_height) / 2; // Centered vertically
    int win_startx = (COLS - win_width) / 2;  // Centered horizontally
    WINDOW *parent_win = newwin(win_height+2, win_width, win_starty, win_startx);
    nodelay(parent_win, TRUE);
    keypad(parent_win, TRUE);
    // Draw border around the window

    // box(parent_win, 1, 1);
    wrefresh(parent_win);
    struct character player = {2, 2, 3, {'^', '<', 'v', '>'}, 0, true};
    struct bomb bomba = {player.x, player.y, false, 2000, 5, 2, 'O'};
    noecho();
    int check1, check2, check3;
    check1 = lvl1(&player, &bomba, rows, cols, field, field_under, parent_win);
    if (check1){
        player.x = 2;
        player.y = 2;
        check2 = lvl2(&player, &bomba, rows, cols, field, field_under, parent_win);
        if (check2){
            player.x = 2;
            player.y = 2;
            check3 = lvl3(&player, &bomba, rows, cols, field, field_under, parent_win);
            if (check3){
                mvwprintw(parent_win, rows+3, (cols + 3 -  strlen("CONGRATULATIONS! YOU'VE ACTUALLY WON! OMG")) / 2, "CONGRATULATIONS! YOU'VE ACTUALLY WON! OMG");
                nodelay(stdscr, FALSE);
                mvwprintw(parent_win, rows+4, (cols + 3 - strlen("PRESS ANY KEY TO CONTINUE...")) / 2, "PRESS ANY KEY TO CONTINUE...");
                wrefresh(parent_win);
                clear_getch_buffer(parent_win);
                getch();
                nodelay(stdscr, TRUE);
                delwin(parent_win); 
                clear();  
                return 1;
            } //win animation
            else{
                mvwprintw(parent_win, rows+3, (cols + 3 - 11) / 2, "GAME OVER!");
                nodelay(stdscr, FALSE);
                mvwprintw(parent_win, rows+4, (cols + 3 - strlen("PRESS ANY KEY TO CONTINUE...")) / 2, "PRESS ANY KEY TO CONTINUE...");
                wrefresh(parent_win);
                clear_getch_buffer(parent_win);
                getch();
                nodelay(stdscr, TRUE);
                delwin(parent_win);  
                clear();    
                return 1;
        }// loose animation
        }
        else {
            mvwprintw(parent_win, rows+3, (cols + 3 - 11) / 2, "GAME OVER!");
            nodelay(parent_win, FALSE);
            nodelay(stdscr, FALSE);
            mvwprintw(parent_win, rows+4, (cols + 3 - strlen("PRESS ANY KEY TO CONTINUE...")) / 2, "PRESS ANY KEY TO CONTINUE...");
            wrefresh(parent_win);
            clear_getch_buffer(parent_win);
            getch();
            nodelay(stdscr, TRUE);
            delwin(parent_win);    
            clear();  
            return 1;
        }; // loose animation
    } 
    else{
        mvwprintw(parent_win, rows+3, (cols + 3 - 11) / 2, "GAME OVER!");
        nodelay(stdscr, FALSE);
        mvwprintw(parent_win, rows+4, (cols + 3 - strlen("PRESS ANY KEY TO CONTINUE...")) / 2, "PRESS ANY KEY TO CONTINUE...");
        wrefresh(parent_win);
        clear_getch_buffer(parent_win);
        getch();
        nodelay(stdscr, TRUE);
        delwin(parent_win); 
        clear();  
        return 1;
    }
}

int main() {
    initscr();  
    start_color();
    
    
    // Initialize color pairs
    init_pair(1, COLOR_WHITE, COLOR_BLACK); // Walls
    init_pair(2, COLOR_BLACK, COLOR_BLACK); // Empty spaces
    init_pair(3, COLOR_CYAN, COLOR_BLACK);  // Character
    init_pair(4, COLOR_YELLOW, COLOR_BLACK);
    init_pair(5, COLOR_RED, COLOR_BLACK); // Obstacles
    init_pair(6, COLOR_MAGENTA, COLOR_BLACK);
    init_pair(7, COLOR_GREEN, COLOR_BLACK);
    init_pair(8, COLOR_BLUE, COLOR_BLACK);
    while (game()){};
    return 0;
}
