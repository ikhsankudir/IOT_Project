# Energy Meter HTTP API Publisher

Sistem monitoring energi listrik berbasis ESP32 yang menggunakan sensor ZMPT101B untuk tegangan dan SCT013 untuk arus, dengan kemampuan mengirim data ke HTTP API secara real-time.

## 📋 Deskripsi Proyek

Proyek ini adalah sistem monitoring energi yang:
- Mengambil sampel sensor setiap 1 detik
- Mengirim data rata-rata setiap 5 detik ke HTTP API
- Menampilkan informasi real-time pada OLED display
- Menyimpan kalibrasi sensor secara permanen
- Auto-reconnect WiFi dan monitoring kesehatan sistem

## 🔧 Komponen Hardware

### Sensor Utama
- **ZMPT101B**: Sensor tegangan AC (Pin 35)
- **SCT013**: Sensor arus AC (Pin 34)
- **ESP32**: Mikrokontroler utama
- **OLED SSD1306**: Display 128x64 (I2C)

### Spesifikasi Teknis
- Resolusi ADC: 12-bit
- Sensitivitas ZMPT101B: 500.0
- Kalibrasi default arus: 111.1
- Frekuensi sampling: 1 Hz
- Interval pengiriman data: 5 detik

## 📡 Konfigurasi

### Environment Variables
Proyek ini menggunakan file `config.h` untuk menyimpan data sensitif:

1. **Copy template konfigurasi:**
   ```bash
   cp config_example.h config.h
   ```
