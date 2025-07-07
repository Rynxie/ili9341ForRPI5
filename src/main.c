#include "main.h"


// Piksel formatı dönüşümü için yardımcı fonksiyon
// Xlib genellikle 32-bit (BGRX/BGRA) formatında veri döner.
// Bunu 16-bit RGB565'e dönüştüreceğiz.
uint16_t convert_pixel_to_rgb565(unsigned char blue, unsigned char green, unsigned char red) {
    // 8-bit R, G, B değerlerini 5-bit R, 6-bit G, 5-bit B'ye sıkıştır
    uint16_t r5 = (red >> 3) & 0x1F;   // 8-bit red -> 5-bit red
    uint16_t g6 = (green >> 2) & 0x3F; // 8-bit green -> 6-bit green
    uint16_t b5 = (blue >> 3) & 0x1F;  // 8-bit blue -> 5-bit blue

    // RGB565 formatında birleştir: RRRRRGGGGGG BBBBB
    return (r5 << 11) | (g6 << 5) | b5;
}

int main(int argc, char *argv[]) {
    Display *display;
    Window root;
    XWindowAttributes attrs;
    XImage *x_image;
    
    int screen_width, screen_height;
    uint16_t *pixel_array = NULL; // 16-bit piksel verisi için array
    
    // DISPLAY ortam değişkenini kontrol et
    if (getenv("DISPLAY") == NULL) {
        fprintf(stderr, "Hata: DISPLAY ortam değişkeni ayarlanmamış.\n");
        fprintf(stderr, "Bu programı bir X11 oturumu içinde veya DISPLAY=:0.0 gibi bir değişkenle çalıştırın.\n");
        return 1;
    }

    // X sunucusuna bağlan
    display = XOpenDisplay(NULL);
    if (!display) {
        fprintf(stderr, "Hata: X sunucusuna bağlanılamadı. DISPLAY değişkenini kontrol edin veya X11'in çalıştığından emin olun.\n");
        return 1;
    }

    root = DefaultRootWindow(display); // Kök pencereyi (tüm ekranı) al

    // Kök pencerenin özelliklerini al
    XGetWindowAttributes(display, root, &attrs);
    screen_width = attrs.width;
    screen_height = attrs.height;

    printf("Ekran Boyutları: %dx%d piksel\n", screen_width, screen_height);

    // Piksel array'i için bellek ayır
    // Her piksel 16 bit (uint16_t) olacağı için boyut ona göre hesaplanır
    pixel_array = (uint16_t *)malloc(screen_width * screen_height * sizeof(uint16_t));
    if (!pixel_array) {
        fprintf(stderr, "Hata: Piksel array'i için bellek ayrılamadı.\n");
        XCloseDisplay(display);
        return 1;
    }

    printf("Sürekli ekran görüntüsü alma başladı (X11'den uint16_t array'e dönüştürülüyor). Çıkmak için Ctrl+C'ye basın.\n");
    printf("Her kare, 'pixel_array' pointer'ında güncellenmektedir.\n");

    ILI9341_DisplayToggle(ILI9341_DISPLAY_ON);
    ILI9341_Init();

    while (1) {
        // X sunucusundan görüntüyü al
        // XGetImage genellikle 32-bit (BGRX veya BGRA) formatında veri döner
        x_image = XGetImage(display, root, 0, 0, screen_width, screen_height, AllPlanes, ZPixmap);
        if (!x_image) {
            fprintf(stderr, "Hata: XGetImage başarısız oldu.\n");
            break;
        }

        // x_image->data'dan piksel verisini oku ve uint16_t array'ine dönüştür
        for (int y = 0; y < screen_height; y++) {
            for (int x = 0; x < screen_width; x++) {
                unsigned long pixel = XGetPixel(x_image, x, y);
                
                // XGetPixel'den gelen değeri R, G, B bileşenlerine ayır
                // Xlib'in varsayılan formatı genellikle BGRX/BGRA'dır,
                // yani Blue en düşük baytta, Red en yüksek baytta.
                // Bu değerlerin bit konumları sistemden sisteme değişebilir,
                // ancak yaygın Xlib ZPixmap davranışı şudur:
                unsigned char blue = (pixel >> x_image->blue_mask_shift) & (x_image->blue_mask >> x_image->blue_mask_shift);
                unsigned char green = (pixel >> x_image->green_mask_shift) & (x_image->green_mask >> x_image->green_mask_shift);
                unsigned char red = (pixel >> x_image->red_mask_shift) & (x_image->red_mask >> x_image->red_mask_shift);
                
                // Veya daha basit (genellikle 32-bit BGRX/BGRA için):
                // unsigned char blue = (pixel >> 0) & 0xFF;
                // unsigned char green = (pixel >> 8) & 0xFF;
                // unsigned char red = (pixel >> 16) & 0xFF;


                // 8-bit RGB'den 16-bit RGB565'e dönüştür
                pixel_array[y * screen_width + x] = convert_pixel_to_rgb565(blue, green, red);
            }
        }
        ILI9341_DrawImage(0, 0, 320, 240, pixel_array);

        // XImage nesnesini serbest bırak
        XDestroyImage(x_image);

        // Burada pixel_array[] artık güncel ekran görüntüsü verisini içeriyor.
        // Bu veriyi LCD'ye gönderme veya işleme logic'ini buraya ekleyebilirsin.
        // Örneğin: write_to_lcd_framebuffer("/dev/fb1", pixel_array, screen_width, screen_height);
        
        // Bir miktar gecikme ekle (örneğin 33 ms = yaklaşık 30 FPS için)
        usleep(33000); // Mikrosaniye cinsinden
    }

    // Belleği serbest bırak ve bağlantıyı kapat
    free(pixel_array);
    XCloseDisplay(display);

    return 0;
}