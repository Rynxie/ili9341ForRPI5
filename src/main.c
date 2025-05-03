#include "main.h"
#include "ili9341.h"
#include <pthread.h>


#define TICK_PERIOD_MS 1  // 1 ms tick

static uint8_t buf1[320 * 240 / 10 * 2];

void* tick_thread(void* arg) {
    while (1) {
        lv_tick_inc(TICK_PERIOD_MS);
        usleep(TICK_PERIOD_MS * 1000);  // 1 ms delay
    }
    return NULL;
}

void my_flush_cb(lv_display_t * display, const lv_area_t * area, uint8_t * px_map)
{
    
    uint16_t * buf16 = (uint16_t *)px_map;
    int32_t x, y;
    for(y = area->y1; y <= area->y2; y++) {
        for(x = area->x1; x <= area->x2; x++) {
            ILI9341_DrawPixel(y,x,*buf16);
            buf16++;
            
        }

    }
   

    lv_display_flush_ready(display);
    
} 
				
int main() {

	ILI9341_DisplayToggle(ILI9341_DISPLAY_ON);
    ILI9341_Init();

    lv_init();


    pthread_t tick_tid;
    pthread_create(&tick_tid, NULL, tick_thread, NULL);

    lv_display_t * display = lv_display_create(ILI9341_HEIGHT, ILI9341_WIDTH);
    
    lv_display_set_buffers(display, buf1, NULL, sizeof(buf1), LV_DISPLAY_RENDER_MODE_PARTIAL);
    lv_display_set_flush_cb(display, my_flush_cb);
    ui_init();

    while (1)
    {
        lv_timer_handler();
    }
    

    

}