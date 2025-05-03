/* vim: set ai et ts=4 sw=4: */

#include "ili9341.h"

struct gpiod_chip *chip;
struct gpiod_line *ledPin;
struct gpiod_line *dcPin;
struct gpiod_line *resetPin;


#define SPI_CHANNEL 0    // SPI0 (spidev0.0)
#define SPI_SPEED   50000000  // 50 MHz

static void ILI9341_Select() {
   // HAL_GPIO_WritePin(ILI9341_CS_GPIO_Port, ILI9341_CS_Pin, GPIO_PIN_RESET);

   
}

static void spiSendData(uint8_t * data, size_t len){
    if (wiringPiSPIDataRW(SPI_CHANNEL, data, len) == -1) {
        perror("SPI veri gönderme/alma hatası");
        return 1;
    }

}

void ILI9341_DisplayToggle(uint8_t status){
    switch (status)
    {
    case ILI9341_DISPLAY_ON:
        
        break;
    
    default:
        break;
    }
}

void ILI9341_Unselect() {
    //HAL_GPIO_WritePin(ILI9341_CS_GPIO_Port, ILI9341_CS_Pin, GPIO_PIN_SET);
}

static void ILI9341_Reset() {
    gpiod_line_set_value(resetPin, 0);
    sleep(1);
    gpiod_line_set_value(resetPin, 1);
}

static void ILI9341_WriteCommand(uint8_t cmd) {
    gpiod_line_set_value(dcPin, 0);
    spiSendData(&cmd,1);
    
}

static void ILI9341_WriteData(uint8_t* buff, size_t buff_size) {

   

    
    gpiod_line_set_value(dcPin, 1);
   

 
    /* while(buff_size > 0) {
        uint16_t chunk_size = buff_size > 32768 ? 32768 : buff_size; */
        spiSendData(buff, buff_size);
   /*      buff += chunk_size;
        buff_size -= chunk_size;
    } */
}

static void ILI9341_SetAddressWindow(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1) {
    // column address set
    ILI9341_WriteCommand(0x2A); // CASET
    {
        uint8_t data[] = { (x0 >> 8) & 0xFF, x0 & 0xFF, (x1 >> 8) & 0xFF, x1 & 0xFF };
        ILI9341_WriteData(data, sizeof(data));
    }

    // row address set
    ILI9341_WriteCommand(0x2B); // RASET
    {
        uint8_t data[] = { (y0 >> 8) & 0xFF, y0 & 0xFF, (y1 >> 8) & 0xFF, y1 & 0xFF };
        ILI9341_WriteData(data, sizeof(data));
    }

    // write to RAM
    ILI9341_WriteCommand(0x2C); // RAMWR
}

void ILI9341_Init() {

    chip = gpiod_chip_open(GPIO_CHIP);
	if(!chip){

		perror("Chip didn't run");
	}
	
	ledPin = gpiod_chip_get_line(chip, ILI9341_LED_Pin);
	dcPin   = gpiod_chip_get_line(chip, ILI9341_DC_Pin);
	resetPin = gpiod_chip_get_line(chip, ILI9341_RES_Pin);
	if(!ledPin || !dcPin || !resetPin){


		perror("Line error");

	}


    gpiod_line_request_output(ledPin, "LED", 2);
	gpiod_line_request_output(dcPin, "DC", 2);
	gpiod_line_request_output(resetPin, "RESET", 2);
    gpiod_line_set_value(ledPin, 1);
	gpiod_line_set_value(dcPin, 0);


    if (wiringPiSPISetup(SPI_CHANNEL, SPI_SPEED) < 0) {
        perror("SPI başlatılamadı");
        return 1;
    }



    
    ILI9341_Reset();

    // command list is based on https://github.com/martnak/STM32-ILI9341

    // SOFTWARE RESET
    //SOFTWARE RESET
    ILI9341_WriteCommand(0x01);
    sleep(1);
        
    //POWER CONTROL A
    ILI9341_WriteCommand(0xCB);
    ILI9341_WriteData(0x39);
    ILI9341_WriteData(0x2C);
    ILI9341_WriteData(0x00);
    ILI9341_WriteData(0x34);
    ILI9341_WriteData(0x02);

    //POWER CONTROL B
    ILI9341_WriteCommand(0xCF);
    ILI9341_WriteData(0x00);
    ILI9341_WriteData(0xC1);
    ILI9341_WriteData(0x30);

    //DRIVER TIMING CONTROL A
    ILI9341_WriteCommand(0xE8);
    ILI9341_WriteData(0x85);
    ILI9341_WriteData(0x00);
    ILI9341_WriteData(0x78);

    //DRIVER TIMING CONTROL B
    ILI9341_WriteCommand(0xEA);
    ILI9341_WriteData(0x00);
    ILI9341_WriteData(0x00);

    //POWER ON SEQUENCE CONTROL
    ILI9341_WriteCommand(0xED);
    ILI9341_WriteData(0x64);
    ILI9341_WriteData(0x03);
    ILI9341_WriteData(0x12);
    ILI9341_WriteData(0x81);

    //PUMP RATIO CONTROL
    ILI9341_WriteCommand(0xF7);
    ILI9341_WriteData(0x20);

    //POWER CONTROL,VRH[5:0]
    ILI9341_WriteCommand(0xC0);
    ILI9341_WriteData(0x23);

    //POWER CONTROL,SAP[2:0];BT[3:0]
    ILI9341_WriteCommand(0xC1);
    ILI9341_WriteData(0x10);

    //VCM CONTROL
    ILI9341_WriteCommand(0xC5);
    ILI9341_WriteData(0x3E);
    ILI9341_WriteData(0x28);

    //VCM CONTROL 2
    ILI9341_WriteCommand(0xC7);
    ILI9341_WriteData(0x86);

    //MEMORY ACCESS CONTROL
    ILI9341_WriteCommand(0x36);
    ILI9341_WriteData(0x48);

    //PIXEL FORMAT
    ILI9341_WriteCommand(0x3A);
    ILI9341_WriteData(0x55);

    //FRAME RATIO CONTROL, STANDARD RGB COLOR
    ILI9341_WriteCommand(0xB1);
    ILI9341_WriteData(0x00);
    ILI9341_WriteData(0x18);

    //DISPLAY FUNCTION CONTROL
    ILI9341_WriteCommand(0xB6);
    ILI9341_WriteData(0x08);
    ILI9341_WriteData(0x82);
    ILI9341_WriteData(0x27);

    //3GAMMA FUNCTION DISABLE
    ILI9341_WriteCommand(0xF2);
    ILI9341_WriteData(0x00);

    //GAMMA CURVE SELECTED
    ILI9341_WriteCommand(0x26);
    ILI9341_WriteData(0x01);

    //POSITIVE GAMMA CORRECTION
    ILI9341_WriteCommand(0xE0);
    ILI9341_WriteData(0x0F);
    ILI9341_WriteData(0x31);
    ILI9341_WriteData(0x2B);
    ILI9341_WriteData(0x0C);
    ILI9341_WriteData(0x0E);
    ILI9341_WriteData(0x08);
    ILI9341_WriteData(0x4E);
    ILI9341_WriteData(0xF1);
    ILI9341_WriteData(0x37);
    ILI9341_WriteData(0x07);
    ILI9341_WriteData(0x10);
    ILI9341_WriteData(0x03);
    ILI9341_WriteData(0x0E);
    ILI9341_WriteData(0x09);
    ILI9341_WriteData(0x00);

    //NEGATIVE GAMMA CORRECTION
    ILI9341_WriteCommand(0xE1);
    ILI9341_WriteData(0x00);
    ILI9341_WriteData(0x0E);
    ILI9341_WriteData(0x14);
    ILI9341_WriteData(0x03);
    ILI9341_WriteData(0x11);
    ILI9341_WriteData(0x07);
    ILI9341_WriteData(0x31);
    ILI9341_WriteData(0xC1);
    ILI9341_WriteData(0x48);
    ILI9341_WriteData(0x08);
    ILI9341_WriteData(0x0F);
    ILI9341_WriteData(0x0C);
    ILI9341_WriteData(0x31);
    ILI9341_WriteData(0x36);
    ILI9341_WriteData(0x0F);
}

void ILI9341_DrawPixel(uint16_t x, uint16_t y, uint16_t color) {
    if((x >= ILI9341_WIDTH) || (y >= ILI9341_HEIGHT))
        return;

    ILI9341_Select();

    ILI9341_SetAddressWindow(x, y, x+1, y+1);
    uint8_t data[] = { color >> 8, color & 0xFF };
    ILI9341_WriteData(data, sizeof(data));

    ILI9341_Unselect();
}

static void ILI9341_WriteChar(uint16_t x, uint16_t y, char ch, FontDef font, uint16_t color, uint16_t bgcolor) {
    uint32_t i, b, j;

    ILI9341_SetAddressWindow(x, y, x+font.width-1, y+font.height-1);

    for(i = 0; i < font.height; i++) {
        b = font.data[(ch - 32) * font.height + i];
        for(j = 0; j < font.width; j++) {
            if((b << j) & 0x8000)  {
                uint8_t data[] = { color >> 8, color & 0xFF };
                ILI9341_WriteData(data, sizeof(data));
            } else {
                uint8_t data[] = { bgcolor >> 8, bgcolor & 0xFF };
                ILI9341_WriteData(data, sizeof(data));
            }
        }
    }
}

void ILI9341_WriteString(uint16_t x, uint16_t y, const char* str, FontDef font, uint16_t color, uint16_t bgcolor) {
    ILI9341_Select();

    while(*str) {
        if(x + font.width >= ILI9341_WIDTH) {
            x = 0;
            y += font.height;
            if(y + font.height >= ILI9341_HEIGHT) {
                break;
            }

            if(*str == ' ') {
                // skip spaces in the beginning of the new line
                str++;
                continue;
            }
        }

        ILI9341_WriteChar(x, y, *str, font, color, bgcolor);
        x += font.width;
        str++;
    }

    ILI9341_Unselect();
}

void ILI9341_FillRectangle(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t color) {
    // clipping
    if((x >= ILI9341_WIDTH) || (y >= ILI9341_HEIGHT)) return;
    if((x + w - 1) >= ILI9341_WIDTH) w = ILI9341_WIDTH - x;
    if((y + h - 1) >= ILI9341_HEIGHT) h = ILI9341_HEIGHT - y;

    ILI9341_Select();
    ILI9341_SetAddressWindow(x, y, x+w-1, y+h-1);

    uint8_t data[] = { color >> 8, color & 0xFF };
    gpiod_line_set_value(dcPin, 1);
    
    for(y = h; y > 0; y--) {
        for(x = w; x > 0; x--) {
           spiSendData(data,sizeof(data));
        }
    }

    ILI9341_Unselect();
}

void ILI9341_FillScreen(uint16_t color) {
    ILI9341_FillRectangle(0, 0, ILI9341_WIDTH, ILI9341_HEIGHT, color);
}

void ILI9341_DrawImage(uint16_t x, uint16_t y, uint16_t w, uint16_t h, const uint16_t* data) {
    if((x >= ILI9341_WIDTH) || (y >= ILI9341_HEIGHT)) return;
    if((x + w - 1) >= ILI9341_WIDTH) return;
    if((y + h - 1) >= ILI9341_HEIGHT) return;

    ILI9341_Select();
    ILI9341_SetAddressWindow(x, y, x+w-1, y+h-1);
    ILI9341_WriteData((uint8_t*)data, sizeof(uint16_t)*w*h);
    ILI9341_Unselect();
}

void ILI9341_InvertColors(bool invert) {
    ILI9341_Select();
    ILI9341_WriteCommand(invert ? 0x21 /* INVON */ : 0x20 /* INVOFF */);
    ILI9341_Unselect();
}

