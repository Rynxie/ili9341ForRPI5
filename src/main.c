#include "main.h"
#include "ili9341.h"

				
int main() {

	ILI9341_DisplayToggle(ILI9341_DISPLAY_ON);
    ILI9341_Init();
    ILI9341_FillRectangle(50,50,50,50,ILI9341_BLUE);
    ILI9341_WriteString(100, 100, "Arda'nin mk knk",Font_7x10, ILI9341_BLACK, ILI9341_WHITE);

    

}