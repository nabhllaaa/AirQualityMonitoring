#include <SoftwareSerial.h>
#include <TinyGPS++.h>

// Definisi Pin SoftwareSerial ESP8266
static const int RXPin = D6;   // Terhubung ke TX Modul GPS
static const int TXPin = D5;   // Terhubung ke RX Modul GPS
static const uint32_t GPSBaud = 9600;

TinyGPSPlus gps;
SoftwareSerial gpsSerial(RXPin, TXPin);

void setup() {
  Serial.begin(115200);
  gpsSerial.begin(GPSBaud);

  Serial.println();
  Serial.println("==============================================");
  Serial.println("   AirRespirA System - GPS & Timestamp UTC/WIB");
  Serial.println("==============================================");
}

void loop() {
  // 1. Parsing stream data GPS
  while (gpsSerial.available() > 0) {
    gps.encode(gpsSerial.read());
  }

  // 2. Tampilkan data setiap 2 detik
  static unsigned long lastReport = 0;
  if (millis() - lastReport >= 2000) {
    lastReport = millis();

    Serial.println("----------------------------------------------");
    
    if (gps.location.isValid()) {
      Serial.print("Status Sinyal : "); Serial.println("LOCKED (Valid)");
      Serial.print("Latitude      : "); Serial.println(gps.location.lat(), 6);
      Serial.print("Longitude     : "); Serial.println(gps.location.lng(), 6);
      Serial.print("Jumlah Satelit: "); Serial.println(gps.satellites.value());
      Serial.print("Ketinggian    : "); Serial.print(gps.altitude.meters()); Serial.println(" m");
      Serial.print("Kecepatan     : "); Serial.print(gps.speed.kmph()); Serial.println(" km/jam");

      // --- PARAMETER TIMESTAMP ---
      if (gps.date.isValid() && gps.time.isValid()) {
        // Formating Tanggal (DD/MM/YYYY)
        char dateStr[15];
        sprintf(dateStr, "%02d/%02d/%04d", gps.date.day(), gps.date.month(), gps.date.year());
        
        // Formating Waktu UTC (HH:MM:SS)
        char timeUtcStr[15];
        sprintf(timeUtcStr, "%02d:%02d:%02d", gps.time.hour(), gps.time.minute(), gps.time.second());

        // Konversi ke Waktu Indonesia Barat (WIB = UTC + 7 Jam)
        int hourWib = (gps.time.hour() + 7) % 24;
        char timeWibStr[15];
        sprintf(timeWibStr, "%02d:%02d:%02d", hourWib, gps.time.minute(), gps.time.second());

        // Format ISO 8601 Timestamp (Cocok untuk logging SD Card / API Database Backend)
        char isoTimestamp[30];
        sprintf(isoTimestamp, "%04d-%02d-%02dT%02d:%02d:%02d+07:00", 
                gps.date.year(), gps.date.month(), gps.date.day(),
                hourWib, gps.time.minute(), gps.time.second());

        Serial.print("Tanggal       : "); Serial.println(dateStr);
        Serial.print("Waktu UTC     : "); Serial.println(timeUtcStr);
        Serial.print("Waktu WIB     : "); Serial.println(timeWibStr);
        Serial.print("ISO Timestamp : "); Serial.println(isoTimestamp);
      } else {
        Serial.println("Timestamp     : Membaca waktu satelit...");
      }

    } else {
      Serial.print("Status Sinyal : Mencari Satelit... (Terdeteksi: ");
      Serial.print(gps.satellites.value());
      Serial.println(" satelit)");
    }
  }
}