#include "main.h"

uint16_t convert_pixel_to_rgb565(unsigned char blue, unsigned char green, unsigned char red) {
    uint16_t r5 = (red >> 3) & 0x1F;   // 8-bit red -> 5-bit red (0-31)
    uint16_t g6 = (green >> 2) & 0x3F; // 8-bit green -> 6-bit green (0-63)
    uint16_t b5 = (blue >> 3) & 0x1F;  // 8-bit blue -> 5-bit blue (0-31)

    // RGB565 formatında birleştir: RRRRRGGGGGG BBBBB
    return (r5 << 11) | (g6 << 5) | b5;
}

int main(int argc, char *argv[]) {
    Display *display;
    Window root;
    XWindowAttributes attrs;
    XImage *x_image;
    
    int screen_width, screen_height;
    // uint16_t türünde piksel dizisi, her bir eleman bir pikseli RGB565 formatında saklar.
    uint16_t *pixel_array = NULL; 
    
    // Her kare arasında milisaniye cinsinden gecikme (örn. 33 ms = ~30 FPS)
    // Eğer çok hızlı akışa ihtiyacınız varsa düşürebilirsiniz, ancak CPU yükünü artırır.
    int delay_ms = 33; 

    // DISPLAY ortam değişkeninin ayarlı olduğundan emin olun
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

    root = DefaultRootWindow(display); // Tüm ekranı (kök pencereyi) al

    // Kök pencerenin özelliklerini al (genişlik, yükseklik)
    XGetWindowAttributes(display, root, &attrs);
    screen_width = attrs.width;
    screen_height = attrs.height;

    printf("Ana Ekran Boyutları: %dx%d piksel\n", screen_width, screen_height);

    // ILI9341 ekranın çözünürlüğünü tanımla (genellikle 320x240)
    // Eğer ILI9341'iniz farklıysa bu değerleri değiştirin!
    const int ili9341_width = 320;
    const int ili9341_height = 240;

    // Yakalanacak alan, ILI9341'in boyutlarıyla sınırlıdır
    int capture_width = screen_width;
    int capture_height = screen_height;

    // Eğer ekran görüntüsü ILI9341'den büyükse, onu ILI9341 boyutlarına kırp
    if (capture_width > ili9341_width) capture_width = ili9341_width;
    if (capture_height > ili9341_height) capture_height = ili9341_height;

    printf("ILI9341 için yakalanacak alan: %dx%d piksel (Ana ekranın sol üst köşesi)\n", capture_width, capture_height);

    // Yakalanan görüntüyü depolamak için bellek ayır.
    // Her piksel uint16_t (2 bayt) olacak.
    pixel_array = (uint16_t *)malloc(capture_width * capture_height * sizeof(uint16_t));
    if (!pixel_array) {
        fprintf(stderr, "Hata: Piksel array'i için bellek ayrılamadı.\n");
        XCloseDisplay(display);
        return 1;
    }

    // --- ILI9341 BAŞLATMA AYARLARI ---
    // Bu kısım ILI9341 sürücünüzün başlatma fonksiyonlarını çağırır.
    // Fonksiyon isimlerinin ve parametrelerinin kütüphanenizle uyumlu olduğundan emin olun.
    // 'ILI9341_Init' ve 'ILI9341_DisplayToggle(ILI9341_DISPLAY_ON)' örneklerdir.
    printf("ILI9341 başlatılıyor...\n");
    ILI9341_Init(); 
    ILI9341_DisplayToggle(ILI9341_DISPLAY_ON); 
    // Eğer ekran yönünü ayarlamanız gerekiyorsa buraya ekleyin (örn. ILI9341_SetRotation(rotation_angle);)
    printf("ILI9341 başlatıldı.\n");
    // --- ILI9341 BAŞLATMA AYARLARI SONU ---


    printf("X11 ekran görüntüsü yakalama ve ILI9341'e akış başladı.\n");
    printf("Çıkmak için Ctrl+C'ye basın.\n");

    while (1) {
        // X sunucusundan görüntüyü al
        // Yalnızca capture_width x capture_height boyutundaki alanı alıyoruz
        x_image = XGetImage(display, root, 0, 0, capture_width, capture_height, AllPlanes, ZPixmap);
        if (!x_image) {
            fprintf(stderr, "Hata: XGetImage başarısız oldu. X sunucusunun çalıştığından emin olun.\n");
            break; // Hata durumunda döngüden çık
        }

        // XGetPixel ile piksel verisini oku ve uint16_t (RGB565) array'ine dönüştür
        // Xlib'den gelen 32-bit piksel formatı genellikle BGRX veya BGRA'dır.
        for (int y = 0; y < capture_height; y++) {
            for (int x = 0; x < capture_width; x++) {
                unsigned long pixel = XGetPixel(x_image, x, y);
                
                // 32-bit pikselden 8-bit R, G, B bileşenlerini çıkar
                // (Genel BGRX/BGRA varsayımı)
                unsigned char blue = (pixel >> 0) & 0xFF;
                unsigned char green = (pixel >> 8) & 0xFF;
                unsigned char red = (pixel >> 16) & 0xFF;
                
                // 8-bit RGB'den 16-bit RGB565'e dönüştür
                pixel_array[y * capture_width + x] = convert_pixel_to_rgb565(blue, green, red);
            }
        }

        // XImage nesnesini serbest bırak
        XDestroyImage(x_image);

        // --- ILI9341 EKRANA YAZMA KISMI ---
        // Bu kısım, yakalanan ve dönüştürülen pixel_array'i ILI9341 ekrana gönderir.
        // ILI9341_DrawImage fonksiyonunuzun bu parametreleri (x, y, width, height, data)
        // doğru şekilde kabul ettiğinden emin olun.
        ILI9341_DrawImage(0, 0, capture_width, capture_height, pixel_array);
        // --- ILI9341 EKRANA YAZMA SONU ---
        
        // Belirtilen milisaniye kadar bekle
        usleep(delay_ms * 1000); 
    }

    // --- TEMİZLİK ---
    // Belleği serbest bırak
    free(pixel_array);
    // X sunucusu bağlantısını kapat
    XCloseDisplay(display);

    return 0;
}