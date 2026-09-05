#include <stdio.h>
#include <locale.h>
#include <time.h>
#include <errno.h>
#include <unistd.h>
#include <curses.h>
#include <signal.h>
#include <stdlib.h>

#define CAT "󰄛"  // Nerd Font cat

int userResized = 0;
int slowerCats = 0;

#define RED     "\x1b[31m"
#define GREEN   "\x1b[32m"
#define YELLOW  "\x1b[33m"
#define BLUE    "\x1b[34m"
#define MAGENTA "\x1b[35m"
#define CYAN    "\x1b[36m"
#define RESET   "\x1b[0m"



typedef struct
{
    const char *cat;
} Cats;

Cats cats[] = {
    {RED CAT RESET},
    {GREEN CAT RESET},
    {YELLOW CAT RESET},
    {BLUE CAT RESET},
    {MAGENTA CAT RESET},
    {CYAN CAT RESET}
};

typedef struct {
    int col;
    int row;
    int speed;
    const char *cat;
} Catp;

typedef struct
{
    Catp *c; //pointer to array of cats
    int size; //no. of cats currently stored.
    int capacity; //current no. of cats
} cat_vector;

Catp create();
void cat_fall(Catp *c);
void cat_show(Catp *d);

void v_init(cat_vector *v,int cap);
void v_free(cat_vector *v);
void v_delete(cat_vector *v);
void v_resize(cat_vector *v,int newCap);
void v_add(cat_vector *v,Catp c);

void initCurses();
void exitCurses();

int pRand(int min, int max);
int getNumOfCats();
void handleResize(int sig);
void exitErr(const char *err) __attribute__((noreturn));
void usage();

int mssleep(long msec)
{
    struct timespec ts;
    int res;

    if (msec < 0)
    {
        errno = EINVAL;
        return -1;
    }

    ts.tv_sec  = msec / 1000;
    ts.tv_nsec = (msec % 1000) * 1000000;

    do {
        res = nanosleep(&ts, &ts);
    } while (res && errno == EINTR);

    return res;
}

Catp create()
{
    Catp c;
    c.col = pRand(0,COLS);
    c.row = pRand(0,LINES);
    c.cat = cats[rand()%6].cat;
    if(slowerCats)
    {
        c.speed = pRand(1,3);
    }
    else
    {
        c.speed = pRand(1,6);
    }
    return c;
}

void cat_fall(Catp *c)
{
    c->row = c->row + c->speed;
    if(c->row>=LINES)
        c->row = 0;
}

void cat_show(Catp *c)
{
    int colorIndex = rand() % 6 + 1;
    attron(COLOR_PAIR(colorIndex));
    mvprintw(c->row, c->col, CAT);
    attroff(COLOR_PAIR(colorIndex));    
}

void v_init(cat_vector *v,int cap)
{
    if(cap> 0 && v!=0)
    {
        v->c = (Catp *)malloc(sizeof(Catp)*cap);
        if(v->c!=NULL)
        {
            v->size = 0;
            v->capacity = cap;
        }
        else
            exitErr("\n*Cat array is >NULL<*\n");
    }
    else
        exitErr("\n*VECTOR INIT FAILED*\n");    
}

void v_free(cat_vector *v)
{
    if(v->c!=NULL)
    {
        free(v->c);
        v->c = NULL;
    }
    v->size = 0;
    v->capacity = 0;
}

void v_delete(cat_vector *v)
{
    v_free(v);
}

void v_resize(cat_vector *v, int newCap)
{
    cat_vector newVec;
    v_init(&newVec,newCap);
    for(int i = 0;i<newCap;i++)
    {
        v_add(&newVec, create());
    }
    v_free(v);
    *v = newVec;
}

void v_add(cat_vector *v, Catp c)
{
    if(v->size >= v->capacity)
        v_resize(v,2 * v->capacity);

    v->c[v->size] = c;
    v->size++;
}

Catp *v_getAt(cat_vector *v, int pos)
{
    if((pos<v->size) && (pos>=0))
        return &(v->c[pos]);

    exitErr("\n*BAD ACCESS*\n");
}

void initCurses()
{
    initscr();
    noecho();
    cbreak();
    keypad(stdscr,1);
    curs_set(0);

    if((curs_set(0))==1)
       exitErr("\n*Terminal emulator lacks capabilities.*\n");
    
    
    start_color();
    init_pair(1, COLOR_RED,     COLOR_BLACK);
    init_pair(2, COLOR_GREEN,   COLOR_BLACK);
    init_pair(3, COLOR_YELLOW,  COLOR_BLACK);
    init_pair(4, COLOR_BLUE,    COLOR_BLACK);
    init_pair(5, COLOR_MAGENTA, COLOR_BLACK);
    init_pair(6, COLOR_CYAN,    COLOR_BLACK);

    timeout(0);
    signal(SIGWINCH, handleResize);
}

void exitCurses()
{
    curs_set(1);
    clear();
    refresh();
    resetty();
    endwin();
}

int pRand(int min, int max)
{
    return min + rand()%(max-min + 1);
}

void handleResize(int sig)
{
    endwin();

    userResized = 1;

    refresh();
    erase();
}

void exitErr(const char *err)
{
    exitCurses();
    printf("%s", err);
    exit(0);
}

int getNumOfCats()
{
    int nCats = 0;
    if((LINES<20 && COLS>100) || (COLS < 100 && LINES < 40))
    { 
        nCats = (int)(COLS *0.75);
        slowerCats = 1;
    }
    else
    {
        nCats = (int)(COLS * 1.5);
        slowerCats = 0;
    }
    return nCats;
}

int main(void)
{
    setlocale(LC_ALL, "");
    srand((unsigned int)getpid());
    initCurses();

    int nCats = getNumOfCats();
    cat_vector catsVec;
    v_init(&catsVec, nCats);


    for (int i = 0; i < nCats; i++)
        v_add(&catsVec, create());

    int key;


    while (1)
    {
        if (userResized)
        {
            mssleep(100);
            nCats = getNumOfCats();
            v_resize(&catsVec, nCats);
            userResized = 0;
        }

        for (int i = 0; i < catsVec.size; i++)
        {
            cat_fall(v_getAt(&catsVec, i));
            cat_show(v_getAt(&catsVec, i));
        }

        mssleep(50); // Frame delay

        if ((key = wgetch(stdscr)) == 'q')
            break;

        erase();
    }

    v_delete(&catsVec);
    exitCurses();
    return 0;
    
    /*for(int i = 0;i<6;i++)
    {
        printf("%s\t",cats[i].cat);
    }
    return 0;*/
}

