#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <signal.h>
#include <stdarg.h>
#include <getopt.h>
#include <time.h>
#include <sys/resource.h>

#include <ncurses.h>

#include "clk.h"
#include "gpio.h"
#include "dma.h"
#include "pwm.h"
#include "version.h"

#include "ws2811.h"


#define ARRAY_SIZE(stuff)       (sizeof(stuff) / sizeof(stuff[0]))
#define nRED     1
#define nYELLOW  2
#define nGREEN   3
#define nCYAN    4
#define nBLUE    5
#define nPURPLE  6
#define nWHITE   7
#define nBLACK   8

// defaults for cmdline options
#define TARGET_FREQ             WS2811_TARGET_FREQ
#define GPIO_PIN                18
#define DMA                     10
#define STRIP_TYPE            WS2811_STRIP_GRB		// WS2812/SK6812RGB integrated chip+leds
//#define STRIP_TYPE              WS2811_STRIP_GBR		// WS2812/SK6812RGB integrated chip+leds
//#define STRIP_TYPE            SK6812_STRIP_RGBW		// SK6812RGBW (NOT SK6812RGB)

#define WIDTH                   31
#define HEIGHT                  16
#define LED_COUNT               (WIDTH * HEIGHT)

#define SLOW                    256



int width = WIDTH;
int height = HEIGHT;
int led_count = LED_COUNT;

ws2811_led_t grid[LED_COUNT];
ws2811_led_t gridB[LED_COUNT];


int clear_on_exit = 0;


ws2811_t ledstring =
{
    .freq = TARGET_FREQ,
    .dmanum = DMA,
    .channel =
    {
        [0] =
        {
            .gpionum = GPIO_PIN,
            .count = LED_COUNT,
            .invert = 0,
            .brightness = 255,
            .strip_type = STRIP_TYPE,
        },
        [1] =
        {
            .gpionum = 0,
            .count = 0,
            .invert = 0,
            .brightness = 0,
        },
    },
};

static uint8_t running = 1;

static void ctrl_c_handler(int signum)
{
	(void)(signum);
    running = 0;
}

static void setup_handlers(void)
{
    struct sigaction sa =
    {
        .sa_handler = ctrl_c_handler,
    };

    sigaction(SIGINT, &sa, NULL);
    sigaction(SIGTERM, &sa, NULL);
}

ws2811_led_t getNeighbor(int i,int n){
    int y = i/WIDTH;
    int x = i%WIDTH;
    switch(n){
        case 0:
            x+=-1;
            y+=-1;
            break;
        case 1:
            x+=-1;
            y+=0;
            break;
        case 2:
            x+=-1;
            y+=1;
            break;
        case 3:
            x+=0;
            y+=-1;
            break;
        case 4:
            x+=0;
            y+=1;
            break;
        case 5:
            x+=1;
            y+=-1;
            break;
        case 6:
            x+=1;
            y+=0;
            break;
        case 7:
        default:
            x+=1;
            y+=1;
            break;
    }
    while(x<0){x+=WIDTH;}
    while(x>=WIDTH){x-=WIDTH;}
    while(y<0){y+=HEIGHT;}
    while(y>=HEIGHT){y-=HEIGHT;}
    return grid[x+y*WIDTH];
}

void compCell(int i, ws2811_led_t c){
    int n=0;
    for(int j=0;j<8;j++){
        if((getNeighbor(i,j)&c)>0){
            n++;
        }
    }
    if((grid[i]&c)>0){//Alive?
        if(!(n==2 || n==3)){//Too lonely/crouded?
            gridB[i] = grid[i]&(~c);//Kill
        }
    }else{//Dead
        if(n==3){//Just right?
           gridB[i] = grid[i]|c;//Birth
        }
    }
}

ws2811_led_t RED = 0x00FF0000;
ws2811_led_t GREEN = 0x0000FF00;
ws2811_led_t BLUE = 0x000000FF;


void preview(){
    for(int i=0;i<LED_COUNT;i++){
        int x = i%WIDTH;
        int y = i/WIDTH;
        int color = nRED;
        switch(gridB[i]){
            case 0x00FF0000:
                color=nRED;
                break;
            case 0x00FFFF00:
                color=nYELLOW;
                break;
            case 0x0000FF00:
                color=nGREEN;
                break;
            case 0x0000FFFF:
                color=nCYAN;
                break;
            case 0x000000FF:
                color=nBLUE;
                break;
            case 0x00FF00FF:
                color=nPURPLE;
                break;
            case 0x00FFFFFF:
                color=nWHITE;
                break;
            case 0x00000000:
            default:
                color=nBLACK;
                break;
        }
        attron(COLOR_PAIR(color));
        mvaddch(y,x,'*');
        attroff(COLOR_PAIR(color));
    }
    refresh();
}


bool render(){
    ws2811_return_t ret;
    for(int i=0;i<LED_COUNT;i++){
        int x = i%WIDTH;
        int y = i/WIDTH;
        int j = i;
        if(y%2){
            j=(WIDTH-1-x)+y*WIDTH;
        }
        ledstring.channel[0].leds[j]=gridB[i];
    }
    if ((ret = ws2811_render(&ledstring)) != WS2811_SUCCESS){//Render that framebuffer
        fprintf(stderr, "ws2811_render failed: %s\n", ws2811_get_return_t_str(ret));
        return false;
    }
    return true;
}

void initRandomLife(){
    for(int i=0;i<LED_COUNT;i++){//Init LEDS to random;
        int r = rand()&15;
        grid[i]=0;
        if(r<8){
            if((r&4)>0){grid[i]|=RED;}
            if((r&2)>0){grid[i]|=GREEN;}
            if((r&1)>0){grid[i]|=BLUE;}
        }
    }
}


int main(int argc, char **argv){
    ws2811_return_t ret;
    //initscr();
    //start_color();
    init_pair(nRED,COLOR_RED,COLOR_BLACK);
    init_pair(nYELLOW,COLOR_YELLOW,COLOR_BLACK);
    init_pair(nGREEN,COLOR_GREEN,COLOR_BLACK);
    init_pair(nCYAN,COLOR_CYAN,COLOR_BLACK);
    init_pair(nBLUE,COLOR_BLUE,COLOR_BLACK);
    init_pair(nPURPLE,COLOR_MAGENTA,COLOR_BLACK);
    init_pair(nWHITE,COLOR_WHITE,COLOR_BLACK);
    init_pair(nBLACK,COLOR_BLACK,COLOR_BLACK);
    setup_handlers();

    if ((ret = ws2811_init(&ledstring)) != WS2811_SUCCESS)
    {
        fprintf(stderr, "ws2811_init failed: %s\n", ws2811_get_return_t_str(ret));
        return ret;
    }

    srand(time(NULL));

    int loops = -1;
    if(argc>1){
       loops = atoi(argv[1]);
    }
    initRandomLife();
    struct timespec ts;
    
    while(running && (loops==-1 || loops>0)){
        if(loops>0){loops--;}
        initRandomLife();
        for(int frame=0;frame<1800 && running;frame++){
            clock_gettime(CLOCK_REALTIME, &ts); 
            uint64_t t0 = ts.tv_nsec/1000+ts.tv_sec*1000000;
            for(int i=0;i<LED_COUNT;i++){
                compCell(i, RED);
                compCell(i, GREEN);
                compCell(i, BLUE);
            }
            //preview();
            if(!render()){break;}
            bool allSame=true;
            for(int i=0;i<LED_COUNT;i++){
                if(grid[i]!=gridB[i]){allSame=false;}
                grid[i]=gridB[i];
            }
            if(allSame){
                usleep(1000000);
                initRandomLife();
                frame=0;
            }
            clock_gettime(CLOCK_REALTIME, &ts); 
            uint64_t t1 = ts.tv_nsec/1000+ts.tv_sec*1000000;
            usleep(1000000 / 30 - (t1-t0));
        }
    }

    ws2811_fini(&ledstring);

    //printf ("\n");
    endwin();
    return ret;
}


