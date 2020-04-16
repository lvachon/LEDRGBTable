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

unsigned char baseHue=0;

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

ws2811_led_t xy2c(x,y){
    return hue2rgb(baseHue+x+y);
}

void drawDigit(unsigned char n, unsigned char x, unsigned char y){
    int offset = 3*n*FONTH*FONTW;
    int index=0;
    for(int dstY=y;dstY<y+FONTH;dstY++){
        for(int dstX=x;dstX<x+FONTW;dstX++){
            int mask = nums[index+offset]<<16|nums[index+offset+1]<<8|nums[index+offset+2];
            setLED(dstX,dstY,mask & xy2c(dstX,dstY));
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
    	    strftime(clockString,13,"%m%d%y%H%M%S",timeinfo);
            for(int n=0;n<6;n++){
                drawDigit(c2i(clockString[n]),n*5,0);
                drawDigit(c2i(clockString[n+6]),n*5,8);    
            }
            baseHue++;
            
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


