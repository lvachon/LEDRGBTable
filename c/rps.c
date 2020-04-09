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


#include "clk.h"
#include "gpio.h"
#include "dma.h"
#include "pwm.h"
#include "version.h"

#include "ws2811.h"


#define ARRAY_SIZE(stuff)       (sizeof(stuff) / sizeof(stuff[0]))

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

int width = WIDTH;
int height = HEIGHT;
int led_count = LED_COUNT;

int clear_on_exit = 0;

unsigned char* anim;
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

ws2811_led_t grid[LED_COUNT];

ws2811_led_t getNeighbor(int i,int n){
    int y = i/WIDTH;
    int x = i%WIDTH;
    switch(n){
        case 0:
            x+=1;
            y+=0;
            break;
        case 1:
            x+=1;
            y+=1;
            break;
        case 2:
            x+=0;
            y+=1;
            break;
        case 3:
            x+=0;
            y+=-1;
            break;
        case 4:
            x+=-1;
            y+=-1;
            break;
        case 5:
            x+=-1;
            y+=0;
            break;
        case 6:
            x+=-1;
            y+=1;
            break;
        case 7:
            x+=0;
            y+=1;
            break;
        case 8:
        default:
            x+=-1;
            y+=1;
            break;
    }
    while(x<0){x+=WIDTH;}
    while(x>=WIDTH){x-=WIDTH;}
    while(y<0){y+=HEIGHT;}
    while(y>=WIDTH){x-=WIDTH;}
    return grid[x+y*WIDTH];
}

ws2811_led_t ROCK = 0x00FF0000;
ws2811_led_t PAPER = 0x0000FF00;
ws2811_led_t SCISSORS = 0x000000FF;

void setLED(i,c){
    ledstring.channel[0].leds[i]=c;
}

void compCell(int i){
    ws2811_led_t myType = grid[i];
    int r,p,s = 0;
    for(int j=0;j<9;j++){
        ws2811_led_t n = getNeighbor(i,j);
        if(n==ROCK){r++;continue;}
        if(n==PAPER){p++;continue;}
        if(n==SCISSORS){s++;continue;}
    }
    if(myType==ROCK && p>r && p>s){
        setLED(i,PAPER);
    }
    if(myType==PAPER && s>r && s>p){
        setLED(i,SCISSORS);
    }
    if(myType==SCISSORS && r>p && r>s){
        setLED(i,ROCK);
    }
}

int main(){
    ws2811_return_t ret;
    
    setup_handlers();

    if ((ret = ws2811_init(&ledstring)) != WS2811_SUCCESS)
    {
        fprintf(stderr, "ws2811_init failed: %s\n", ws2811_get_return_t_str(ret));
        return ret;
    }

    srand(time(NULL));

    for(int i=0;i<LED_COUNT;i++){//Init LEDS to a random state
        unsigned int rnd = rand()&3;
        if(rnd==0){ledstring.channel[0].leds[i]=ROCK;continue;}
        if(rnd==1){ledstring.channel[0].leds[i]=PAPER;continue;}
        if(rnd==2){ledstring.channel[0].leds[i]=SCISSORS;continue;}
    }

    while(running){
        for(int i=0;i<LED_COUNT;i++){//Copy framebuffer back to memory for next step
            grid[i]=ledstring.channel[0].leds[i];
        }
        for(int i=0;i<LED_COUNT;i++){//Compute next step, saving it into framebugge
            compCell(i);
        }
        if ((ret = ws2811_render(&ledstring)) != WS2811_SUCCESS){//Render that framebuffer
            fprintf(stderr, "ws2811_render failed: %s\n", ws2811_get_return_t_str(ret));
            break;
        }
        usleep(1000000 / 30);
    }
    ws2811_fini(&ledstring);

    printf ("\n");
    return ret;
}

