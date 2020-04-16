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

#define FONTH                   7
#define FONTW                   4

int width = WIDTH;
int height = HEIGHT;
int led_count = LED_COUNT;
unsigned char* nums;
char clockString[14];
time_t rawtime;
struct tm * timeinfo;

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

unsigned char* readFileBytes(const char *name)  
{  
    FILE *fl = fopen(name, "r");  
    fseek(fl, 0, SEEK_END);  
    long len = ftell(fl);
    unsigned char *ret = malloc(len);  
    fseek(fl, 0, SEEK_SET);  
    fread(ret, 1, len, fl);  
    fclose(fl);  
    return ret;  
}


void drawDigit(unsigned char n, unsigned char y, unsigned char x){
    int offset = 3*n*FONTH;
    int index=0;
    for(int dstY=y;dstY<y+FONTH;dstY++){
        for(int dstX=x;dstX<x+FONTW;dstX++){
            int mask = nums[index+offset]<<16|nums[index+offset+1]<<8|nums[index+offset+2];
            setLED(dstX,dstY,mask);
            offset+=3;
        }
    }
}

int c2i(char c){
    return (int)(c)-48;
}


int main(int argc, char **argv){
    ws2811_return_t ret;
    
    setup_handlers();

    nums = readFileBytes("7seg.raw");

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

    for(int i=0;i<LED_COUNT;i++){//Init LEDS to black;
        ledstring.channel[0].leds[i]=0;
    }

    while(running && (loops==-1 || loops>0)){
        loops--;
        for(int frame=0;frame<1800 && running;frame++){
            time (&rawtime);
            timeinfo = localtime (&rawtime);
    	    strftime(clockString,12,"%m%d%y%H%M%S",timeinfo);
            for(int n=0;n<6;n++){
                fprintf(stdout,"%d\n",c2i(clockString[11]));
                drawDigit(c2i(clockString[n]),n*5,0);
                drawDigit(c2i(clockString[n+6]),n*5,8);    
            }
            
            
            if ((ret = ws2811_render(&ledstring)) != WS2811_SUCCESS){//Render that framebuffer
                fprintf(stderr, "ws2811_render failed: %s\n", ws2811_get_return_t_str(ret));
                break;
            }
            usleep(1000000 / 30);
        }
    }

    ws2811_fini(&ledstring);

    printf ("\n");
    return ret;
}


