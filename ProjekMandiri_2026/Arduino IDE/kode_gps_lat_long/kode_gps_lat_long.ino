#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <TinyGPS++.h>
#include <DHT.h>

#define GPS_RX 26   // GPIO26 <- TX modul GPS
#define GPS_TX 25   // GPIO25 -> RX modul GPS
#define GPS_BAUD 9600
#define DHTPIN 4
#define DHTTYPE DHT22

LiquidCrystal_I2C lcd(0x27, 16, 2);
TinyGPSPlus gps;
DHT dht(DHTPIN, DHTTYPE);

void tampil(int baris, const char *teks) {
  char buf[17];
  snprintf(buf, sizeof(buf), "%-16s", teks);
  lcd.setCursor(0, baris);
  lcd.print(buf);
}

void setup() {
  Serial.begin(115200);
  delay(500);

  Serial2.setRxBufferSize(1024);
  Serial2.begin(GPS_BAUD, SERIAL_8N1, GPS_RX, GPS_TX);

  Wire.begin(21, 22);
  lcd.init();
  lcd.backlight();
  lcd.clear();
  dht.begin();

  tampil(0, " AirRespirA GPS");
  tampil(1, " System Ready!");
  delay(2000);
  lcd.clear();

  Serial.println("\n--- MONITORING AIRRESPIRA (GPS + DHT22 + LCD) ---");
  Serial.println("Waktu,Latitude,Longitude,Satelit,Suhu(C),Kelembapan(%)");
}

void loop() {
  while (Serial2.available() > 0) gps.encode(Serial2.read());

  static unsigned long lastReport = 0;
  static unsigned long lastDht = 0;
  static float suhu = NAN, hum = NAN;

  // Baca DHT22 tiap 2 detik (sensor ini lambat)
  if (millis() - lastDht >= 2000) {
    lastDht = millis();
    suhu = dht.readTemperature();
    hum = dht.readHumidity();
  }

  if (millis() - lastReport >= 1000) {
    lastReport = millis();

    char l1[17], l2[17];
    bool halamanGps = ((millis() / 3000) % 2 == 0);

    if (halamanGps) {
      if (gps.location.isValid()) {
        int jam = (gps.time.hour() + 7) % 24;
        snprintf(l1, sizeof(l1), "%02d:%02d:%02d Sat:%02d",
                 jam, gps.time.minute(), gps.time.second(),
                 (int)gps.satellites.value());
        snprintf(l2, sizeof(l2), "%.4f,%.4f",
                 gps.location.lat(), gps.location.lng());
      } else {
        snprintf(l1, sizeof(l1), "Mencari Satelit");
        snprintf(l2, sizeof(l2), "Terdeteksi: %d", (int)gps.satellites.value());
      }
    } else {
      if (isnan(suhu) || isnan(hum)) {
        snprintf(l1, sizeof(l1), "Suhu: Error");
        snprintf(l2, sizeof(l2), "Hum : Error");
      } else {
        snprintf(l1, sizeof(l1), "Suhu: %.1f C", suhu);
        snprintf(l2, sizeof(l2), "Hum : %.0f %%", hum);
      }
    }
    tampil(0, l1);
    tampil(1, l2);

    // Log serial
    if (gps.location.isValid()) {
      int jam = (gps.time.hour() + 7) % 24;
      Serial.printf("%02d:%02d:%02d, %.6f, %.6f, %d, %.1f, %.0f\n",
                    jam, gps.time.minute(), gps.time.second(),
                    gps.location.lat(), gps.location.lng(),
                    (int)gps.satellites.value(), suhu, hum);
    } else {
      Serial.printf("Mencari satelit... Suhu %.1f C, Hum %.0f %%\n", suhu, hum);
    }
  }
}