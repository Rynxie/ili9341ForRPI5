#include "main.h"
#include "ili9341.h"
#include <pthread.h>


#define TICK_PERIOD_MS 1  // 1 ms tick

static uint8_t buf1[320 * 240 / 10 * 2];
int tick;

void* tick_thread(void* arg) {
    while (1) {
        tick++;
        usleep(TICK_PERIOD_MS * 1000);  // 1 ms delay
    }
    return NULL;
}


int getTick(){
    return tick;
}
void my_flush_cb(lv_display_t * display, const lv_area_t * area, uint8_t * px_map)
{
    
    uint16_t * buf16 = (uint16_t *)px_map;
 

    ILI9341_DrawImage(area->x1, area->y1, area->x2 - area->x1 + 1, area->y2 - area->y1 + 1, buf16);


   

    lv_display_flush_ready(display);
    
} 
				
int main() {

	ILI9341_DisplayToggle(ILI9341_DISPLAY_ON);
    ILI9341_Init();

    lv_init();


    pthread_t tick_tid;
    pthread_create(&tick_tid, NULL, tick_thread, NULL);

    lv_tick_set_cb(getTick);

    lv_display_t * display = lv_display_create(ILI9341_HEIGHT, ILI9341_WIDTH);
    
    lv_display_set_buffers(display, buf1, NULL, sizeof(buf1), LV_DISPLAY_RENDER_MODE_PARTIAL);
    lv_display_set_flush_cb(display, my_flush_cb);
    ui_init();
    sleep(1);
    while (1)
    {
        uint32_t time_till_next = lv_timer_handler();
        if(time_till_next == LV_NO_TIMER_READY) time_till_next = LV_DEF_REFR_PERIOD; /*handle LV_NO_TIMER_READY. Another option is to `sleep` for longer*/
        usleep(time_till_next);
        
    }
    

    

}