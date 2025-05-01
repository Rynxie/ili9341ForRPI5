#include "main.h"

#define GPIO_CHIP "/dev/gpiochip0" // RPi 5'te genellikle gpiochip4 kullanılır
#define GPIO_LED 22
#define GPIO_DC 27
#define GPIO_RESET 17
				
int main() {

	struct gpiod_chip *chip;
	struct gpiod_line *ledPin;
	struct gpiod_line *dcPin;
	struct gpiod_line *resetPin;

	chip = gpiod_chip_open(GPIO_CHIP);
	if(!chip){

		perror("Chip didn't run");
	}
	
	ledPin = gpiod_chip_get_line(chip, GPIO_LED);
	dcPin   = gpiod_chip_get_line(chip, GPIO_DC);
	resetPin = gpiod_chip_get_line(chip, GPIO_RESET);
	if(!ledPin || !dcPin || !resetPin){


		perror("Line error");

	}

	gpiod_line_request_output(ledPin, "LED", 2);
	gpiod_line_request_output(dcPin, "DC", 2);
	gpiod_line_request_output(resetPin, "RESET", 2);
	

	gpiod_line_set_value(ledPin, 1);
	gpiod_line_set_value(dcPin, 0);

	int spi_fd;
	const char *device = "/dev/spidev0.0";
	uint8_t mode = SPI_MODE_0;
	uint8_t bits = 8;
	uint32_t speed = 500000;

	spi_fd = open(device , O_RDWR);
	if(spi_fd < 0){
		perror("SPI error");
		return 1;
	}

	ioctl(spi_fd, SPI_IOC_WR_MODE &mode);
	ioctl(spi_fd, SPI_IOC_WR_BITS_PER_WORD, &bits);
	ioctl(spi_fd, SPI_IOC_WR_MAX_SPEED_HZ, &speed);

	uint8_t txBuf[] = { 0x04 , 0x00 , 0x00, 0x00, 0x00};
	uint8_t rxBuf[8] = { 0 };

	struct spi_ioc_transfer tr = {
        	.tx_buf = (unsigned long)txBuf,
        	.rx_buf = (unsigned long)rxBuf,
        	.len = sizeof(txBuf),
        	.speed_hz = speed,
        	.bits_per_word = bits,
    	};

    // SPI transferi
    if (ioctl(spi_fd, SPI_IOC_MESSAGE(1), &tr) < 1) {
        perror("SPI transfer hatası");
        close(spi_fd);
        return 1;
    }

    printf("Gelen byte: 0x%02X 0x%02X 0x%02X\n", rxBuf[1], rxBuf[2], rxBuf[3]);

    close(spi_fd);

}