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

#define SLOW                    256



int width = WIDTH;
int height = HEIGHT;
int led_count = LED_COUNT;

ws2811_led_t grid[LED_COUNT];


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

void setLEDi(int i, ws2811_led_t c){
    if(i<0||i>=LED_COUNT){return;}
    ledstring.channel[0].leds[i]=c;
}

void setLED(int x, int y, ws2811_led_t c){
    int i=x+y*WIDTH;
    if(y%2){
        i=(WIDTH-1-x)+y*WIDTH;
    }
    setLEDi(i,c);
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
        if(getNeighbor(i,j)&c>0){
            n++;
        }
    }
    if(grid[i]&c>0){//Alive?
        if(!(n==2 || n==3)){//Too lonely/crouded?
            setLEDi(i, grid[i]&(~c));//Kill
        }
    }else{//Dead
        if(n==3){
            setLEDi(i, grid[i]|c);//Birth
        }
    }
}

ws2811_led_t RED = 0x00FF0000;
ws2811_led_t GREEN = 0x0000FF00;
ws2811_led_t BLUE = 0x000000FF;


int main(int argc, char **argv){
    ws2811_return_t ret;
    
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

    for(int i=0;i<LED_COUNT;i++){//Init LEDS to random;
        int r = rand()&7;
        ledstring.channel[0].leds[i]=0;
        if(r&4){ledstring.channel[0].leds[i]|=RED;}
        if(r&2){ledstring.channel[0].leds[i]|=GREEN;}
        if(r&1){ledstring.channel[0].leds[i]|=BLUE;}
    }

    while(running && (loops==-1 || loops>0)){
        loops--;
        for(int frame=0;frame<1800 && running;frame++){
            for(int i=0;i<LED_COUNT;i++){
                compCell(i, RED);
                compCell(i, GREEN);
                compCell(i, BLUE);
            }

            if ((ret = ws2811_render(&ledstring)) != WS2811_SUCCESS){//Render that framebuffer
                fprintf(stderr, "ws2811_render failed: %s\n", ws2811_get_return_t_str(ret));
                break;
            }
            for(int i=0;i<LED_COUNT;i++){
                grid[i]=ledstring.channel[i].leds[i];
            }
            usleep(1000000 / 30);
        }
    }

    ws2811_fini(&ledstring);

    printf ("\n");
    return ret;
}


