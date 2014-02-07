#include <ncurses.h>
#include <stdio.h>
#include "visualizer.h"
#include <math.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>

#define SERVER_PATH "/var/run/mysocket3"
#define BUFFER_LENGTH 250
#define FALSE 0

static buffer[BUFFER_LENGTH];
static struct sockaddr_un serveraddr;
static int sd = -1;
static int rc = -1;

void init_screen() {
    initscr();
    start_color();
    init_color(COLOR_WHITE, 500, 500, 500);
    init_pair(1, COLOR_RED, COLOR_BLACK);
    init_pair(2, COLOR_GREEN, COLOR_BLACK);
    init_pair(3, COLOR_WHITE, COLOR_BLACK);
    init_pair(4, COLOR_RED, COLOR_WHITE);
    init_pair(5, COLOR_GREEN, COLOR_WHITE);
    init_pair(6, COLOR_BLACK, COLOR_WHITE);
    printf("INITIALIZED WINDOW PERHAPS\n");
    sd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (sd < 0)
        mvaddch(0,1,'B');
    memset(&serveraddr, 0, sizeof(serveraddr));
    serveraddr.sun_family = AF_UNIX;
    strcpy(serveraddr.sun_path, SERVER_PATH);

    rc = connect(sd, (struct sockaddr *)&serveraddr, SUN_LEN(&serveraddr));
    if (rc < 0)
        mvaddch(0,0,'A');
    refresh();
}

void end_screen() {
    endwin();
    close(sd);
}

#define FLASH 0

void draw_buf(short loud[], int N) {
    ////clear();
    //attron(COLOR_PAIR(1));
    ////attrset(COLOR_PAIR(0));
    //int i;
    //for (i=0; i<N/4; i+=2){
    //    int j;
    //    int length = (abs(fftBufRe[i])/50) + 3;
    //    if (length > LINES) {
    //        attron(COLOR_PAIR(2));
    //    }
    //    for(j=0; j<LINES;j++){
    //        if(j<length) {
    //            mvaddch(LINES-j,i,'#');
    //            mvaddch(LINES-j,i+1,'#');//ACS_BLOCK);
    //        } else {
    //            mvaddch(LINES-j,i,' ');
    //            mvaddch(LINES-j,i+1,' ');//ACS_BLOCK);
    //        }
    //    }
    //    /*int length2 = abs(fftBufIm[i])/50 + 3;
    //    for(j=0;j<length2 && j<LINES-10;j++){
    //        mvaddch(LINES-j,i+1,ACS_BLOCK);
    //    }*/
    //    //mvprintw(i, 0, "%d", fftBufRe[i]);
    //}
    int flash = 0;

    int i, j;

    for (i = 0; i < N; i ++) {
        if (loud[i] > FLASH) {
            flash = 1;
        }
    }

    memcpy(buffer, loud, BUFFER_LENGTH);
    rc = send(sd, buffer, sizeof(buffer), 0);

    int base = 0;
    for (i = 0; i < 12; i++) {
        if (loud[i] > -25) {
            base += loud[i] + 25;
        }
    }
    base = base/3;
    int cy = LINES/2;
    int cx = COLS/2;
    char *sizes = ".:-=+*#%%@#";

    for (i = 0; i < COLS; i ++) {
        int length = (loud[i]/4) + 20;
        for (j = 0; j < LINES; j ++) {
            if (flash) {
                attron(COLOR_PAIR(4));
            } else {
                attron(COLOR_PAIR(1));
            }
            int r = (i-cx)*(i-cx)+(j-cy)*(j-cy); 
            if (LINES-j < length) {
                if (LINES - j > length - 4) {
                    if (flash) {
                        attron(COLOR_PAIR(5));
                    } else {
                        attron(COLOR_PAIR(2));
                    }
                }
                mvaddch(j, i, '#');
            } else if (r < (base * base + rand() % 9)) {
                if (flash) {
                    attron(COLOR_PAIR(6));
                } else {
                    attron(COLOR_PAIR(3));
                }
                mvaddch(j, i, sizes[(int)(sqrt(r)/2)%10]);
            } else {
                mvaddch(j, i, ' ');
            }
        }
    }
    refresh();
}
