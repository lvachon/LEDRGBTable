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
int frame = 0;
int frames = 0;
int loops = 1;
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

unsigned char* readFileBytes(const char *name)  
{  
    FILE *fl = fopen(name, "r");  
    fseek(fl, 0, SEEK_END);  
    long len = ftell(fl);
    frames = len/(3*LED_COUNT);
    unsigned char *ret = malloc(len);  
    fseek(fl, 0, SEEK_SET);  
    fread(ret, 1, len, fl);  
    fclose(fl);  
    return ret;  
}   

int main(int argc, char *argv[])
{
    ws2811_return_t ret;
    if(argc>1){
    	anim = readFileBytes(argv[1]);
    }
    if(argc>2){
    	loops = atoi(argv[2]);
    }
    setup_handlers();

    if ((ret = ws2811_init(&ledstring)) != WS2811_SUCCESS)
    {
        fprintf(stderr, "ws2811_init failed: %s\n", ws2811_get_return_t_str(ret));
        return ret;
    }

    
    for(int loop=0;loop<loops && running;loop++){
    	for(int frame=0;frame<frames && running;frame++){
            for(int i=0;i<LED_COUNT && running;i++){
	        	ledstring.channel[0].leds[i]=anim[3*(frame*LED_COUNT+i)+1]<<16|anim[3*(frame*LED_COUNT+i)+0]<<8|anim[3*(frame*LED_COUNT+i)+2];
	        }
	        if ((ret = ws2811_render(&ledstring)) != WS2811_SUCCESS)
	        {
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
