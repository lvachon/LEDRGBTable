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



void setLED(int i, ws2811_led_t c){
    ledstring.channel[0].leds[i]=c;
}



unsigned char dropY[WIDTH];

void parseArgs(int argc, char *argv[]){
    if(argc>1){
        loops = atoi(argv[1]);
    }
    if(argc>2){
        unsigned char brightness = atoi(argv[2]);
        ledstring.channel[0].brightness=brightness;
    }
}

int main(int argc, char **argv){
    ws2811_return_t ret;
    
    setup_handlers();
    parseArgs(argc, argv);
    for(int i=0;i<WIDTH;i++){
        dropY[i]=0;
    }

    if ((ret = ws2811_init(&ledstring)) != WS2811_SUCCESS)
    {
        fprintf(stderr, "ws2811_init failed: %s\n", ws2811_get_return_t_str(ret));
        return ret;
    }

    srand(time(NULL));
    



    for(int i=0;i<LED_COUNT;i++){//Init LEDS to a random state
        ledstring.channel[0].leds[i]=0;
    }

    while(running && (loops==-1 || loops>0)){
        if(loops>0){loops--;}
        for(int frame=0;frame<1800 && running;frame++){
    	    for(int i=0;i<LED_COUNT;i++){//Fade
                unsigned char r = (ledstring.channel[0].leds[i] & 0x000000FF);
                unsigned char g = (ledstring.channel[0].leds[i] & 0x0000FF00)>>8;
                if(g>7){
                    g-=7;
                }
                if(r>63){
                    r-=63;
                }
                ledstring.channel[0].leds[i] = r<<16|g<<8|r;
            }
            for(int i=0;i<WIDTH;i++){
                dropY[i]++;
                int x=i;
                if(!(dropY[i]%2)){
                    x=WIDTH-1-x;
                }
                if(dropY[i]>=HEIGHT){
                    if(!(rand()&63) || dropY[i]==200){
                        dropY[i]=-1;
                    }
                }else{
                    ledstring.channel[0].leds[x+dropY[i]*WIDTH]=0x00FFFFFF;
                }
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


