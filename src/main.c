#include "main.h"

// Piksel formatı dönüşümü için yardımcı fonksiyon
// Xlib genellikle 32-bit (BGRX/BGRA) formatında veri döner.
// Bunu 16-bit RGB565'e dönüştüreceğiz.
uint16_t convert_pixel_to_rgb565(unsigned char blue, unsigned char green, unsigned char red) {
    uint16_t r5 = (red >> 3) & 0x1F;   // 8-bit red -> 5-bit red
    uint16_t g6 = (green >> 2) & 0x3F; // 8-bit green -> 6-bit green
    uint16_t b5 = (blue >> 3) & 0x1F;  // 8-bit blue -> 5-bit blue
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

    // ILI9341 ekran boyutlarına uyacak şekilde ekran görüntüsü boyutlarını kırp veya ölçekle
    // Genellikle ILI9341 320x240 pikseldir.
    int capture_width = screen_width;
    int capture_height = screen_height;
    if (capture_width > 320) capture_width = 320;
    if (capture_height > 240) capture_height = 240;

    printf("ILI9341 için yakalanacak alan: %dx%d piksel\n", capture_width, capture_height);


    // Piksel array'i için bellek ayır
    pixel_array = (uint16_t *)malloc(capture_width * capture_height * sizeof(uint16_t));
    if (!pixel_array) {
        fprintf(stderr, "Hata: Piksel array'i için bellek ayrılamadı.\n");
        XCloseDisplay(display);
        return 1;
    }

    // --- ILI9341 İÇİN BAŞLANGIÇ AYARLARI ---
    // Bu kısımlar sizin ILI9341 kütüphanenizden geliyor.
    // Başlık dosyasını doğru dahil ettiğinizden ve fonksiyonların linklendiğinden emin olun.
    ILI9341_Init(); // Eğer ILI9341_Init fonksiyonu varsa
    ILI9341_DisplayToggle(ILI9341_DISPLAY_ON); // Ekranı aç
    // Gerekirse başka başlatma veya döndürme ayarları
    // --- ILI9341 BAŞLANGIÇ AYARLARI SONU ---


    printf("Sürekli ekran görüntüsü alma başladı (X11'den uint16_t array'e dönüştürülüyor).\n");
    printf("Yakalanan veriler doğrudan ILI9341 ekrana gönderiliyor. Çıkmak için Ctrl+C'ye basın.\n");

    while (1) {
        // X sunucusundan görüntüyü al
        x_image = XGetImage(display, root, 0, 0, capture_width, capture_height, AllPlanes, ZPixmap);
        if (!x_image) {
            fprintf(stderr, "Hata: XGetImage başarısız oldu. X sunucusunun çalıştığından emin olun.\n");
            // Hata durumunda döngüden çıkmak veya başka bir işlem yapmak isteyebilirsiniz
            break;
        }

        // x_image->data'dan piksel verisini oku ve uint16_t array'ine dönüştür
        // Dikkat: XGetImage'in döndürdüğü piksel formatı sisteminize göre değişebilir.
        // Genellikle 32-bit ZPixmap için BGRX (Blue, Green, Red, padding/alpha) yaygındır.
        // Eğer renkler bozuk gelirse, pixel değerinin nasıl ayrıldığını kontrol etmeniz gerekebilir.
        for (int y = 0; y < capture_height; y++) {
            for (int x = 0; x < capture_width; x++) {
                unsigned long pixel = XGetPixel(x_image, x, y);
                
                // Genellikle Xlib'de 32-bit renk derinliğinde BGRX sıralaması yaygındır:
                // Mavi en düşük baytta (bit 0-7), Yeşil (bit 8-15), Kırmızı (bit 16-23).
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
        // Bu kısım, yakalanan pixel_array'i ILI9341 ekrana gönderir.
        // ILI9341_DrawImage fonksiyonunuzun bu parametreleri kabul ettiğinden emin olun.
        ILI9341_DrawImage(0, 0, capture_width, capture_height, pixel_array);
        // --- ILI9341 EKRANA YAZMA SONU ---
        
        // Bir miktar gecikme ekle (örn. 33 ms = ~30 FPS için)
        // Burayı ihtiyaçlarınıza göre ayarlayın. Çok hızlı olursa CPU aşırı yüklenir.
        usleep(33000); // Mikrosaniye cinsinden
    }

    // Belleği serbest bırak ve X bağlantısını kapat
    free(pixel_array);
    XCloseDisplay(display);

    return 0;
}