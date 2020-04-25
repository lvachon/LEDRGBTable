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
#define NUM_STARS               32
#define WARP_FACTOR             1.05
#define MIN_RAD

int width = WIDTH;
int height = HEIGHT;
int led_count = LED_COUNT;

struct Star {
    float x;
    float y;
    ws2811_led_t c;
};

struct Star stars[NUM_STARS];

ws2811_led_t gridB[LED_COUNT];


int clear_on_exit = 0;
int loops=-1;

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




void preview(){
    for(int i=0;i<LED_COUNT;i++){
        int x = i%WIDTH;
        int y = i/WIDTH;
        int color = nRED;
        if((gridB[i]&0xFF)>0x80){//Red
            if((gridB[i]&0xFF00)>0x8000){//Green
                if((gridB[i]&0xFF0000)>0x800000){//Blue
                    color=nWHITE;
                }else{//No blue
                    color=nYELLOW;
                }
            }else{//No Green
                if((gridB[i]&0xFF0000)>0x800000){//Blue
                    color=nPURPLE;
                }else{//No blue
                    color=nRED;
                }
            }
        }else{
            if((gridB[i]&0xFF00)>0x8000){//Green
                if((gridB[i]&0xFF0000)>0x800000){//Blue
                    color=nCYAN;
                }else{//No blue
                    color=nGREEN;
                }
            }else{//No Green
                if((gridB[i]&0xFF0000)>0x800000){//Blue
                    color=nBLUE;
                }else{//No blue
                    color=nBLACK;
                }
            }
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



void parseArgs(int argc, char *argv[]){
    if(argc>1){
        loops = atoi(argv[1]);
    }
    if(argc>2){
        unsigned char brightness = atoi(argv[2]);
        ledstring.channel[0].brightness=brightness;
    }
}

ws2811_led_t hue2rgb(unsigned char hue){
   unsigned char r=0;
   unsigned char g=0;
   unsigned char b=0;
   if(hue<85){
        r = (hue * 3);
        g = (255 - hue*3);
        b = 0;
    }else{
        if(hue < 170){
            hue -= 85;
            r = 255 - hue*3;
            g = 0;
            b = hue*3;
        }else{
            hue -= 170;
            r = 0;
            g = hue*3;
            b = 255 - hue*3;
        }
    }
    return (ws2811_led_t)(r<<16|g<<8|b);
}


int main(int argc, char **argv){
    ws2811_return_t ret;
    initscr();
    start_color();
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

    parseArgs(argc, argv);

    
    struct timespec ts;
    for(int i=0;i<NUM_STARS;i++){
        stars[i].x=WIDTH*((float)(rand())/(float)(RAND_MAX)-0.5);
        stars[i].y=HEIGHT*((float)(rand())/(float)(RAND_MAX)-0.5);
        stars[i].c=hue2rgb(rand()&0xFF);
    }
    
    while(running && (loops==-1 || loops>0)){
        if(loops>0){loops--;}
        
        for(int frame=0;frame<900 && running;frame++){

            clock_gettime(CLOCK_REALTIME, &ts); 
            uint64_t t0 = ts.tv_nsec/1000+ts.tv_sec*1000000;

            for(int i=0;i<LED_COUNT;i++){
                if((gridB[i]&0xFF) > 0x0F){
                    gridB[i]-=0x0F;
                }else{
                    gridB[i]&=0xFFFFFF00;
                }
                if((gridB[i]&0xFF00) > 0x0F00){
                    gridB[i]-=0x0F00;
                }else{
                    gridB[i]&=0xFFFF00FF;
                }
                if((gridB[i]&0xFF0000) > 0x0F0000){
                    gridB[i]-=0x0F0000;
                }else{
                    gridB[i]&=0xFF00FFFF;
                }
            }
            for(int i=0;i<NUM_STARS;i++){
                stars[i].x*=WARP_FACTOR;
                stars[i].y*=WARP_FACTOR;
                int x = (int)(stars[i].x+WIDTH/2.0);
                int y = (int)(stars[i].y+HEIGHT/2.0);
                if(x<0 || x>=WIDTH || y<0 || y>=HEIGHT){
                    stars[i].x=0.1*(float)(rand())/(float)(RAND_MAX)-0.05;
                    stars[i].y=0.1*(float)(rand())/(float)(RAND_MAX)-0.05;
                    stars[i].c=hue2rgb(rand()&0xFF);
                    x=WIDTH/2;
                    y=HEIGHT/2;
                }
                if(stars[i].x*stars[i].x+stars[i].y*stars[i].y >= MIN_RAD){
                    int j=x+WIDTH*y;
                    if((gridB[j]&0xFF) < (stars[i].c&0xFF)){
                        gridB[j]+=stars[i].c&0xFF;
                    }else{
                        gridB[j]|=0xFF;
                    }
                    if((gridB[j]&0xFF00) < (stars[i].c&0xFF00)){
                        gridB[j]+=stars[i].c&0xFF00;
                    }else{
                        gridB[j]|=0xFF00;
                    }
                    if((gridB[j]&0xFF0000) < (stars[i].c&0xFF0000)){
                        gridB[j]+=stars[i].c&0xFF0000;
                    }else{
                        gridB[j]|=0xFF0000;
                    }
                }
            }

            preview();
            if(!render()){break;}
  
            clock_gettime(CLOCK_REALTIME, &ts); 
            uint64_t t1 = ts.tv_nsec/1000+ts.tv_sec*1000000;
            usleep(1000000 / 60 - (t1-t0));
        }
    }

    ws2811_fini(&ledstring);

    //printf ("\n");
    endwin();
    return ret;
}


