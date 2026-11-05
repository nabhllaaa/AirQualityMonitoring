#include <SoftwareSerial.h>
#include <TinyGPS++.h>

// Definisi Pin Baru:
// RX NodeMCU = Pin D2 (GPIO4) -> terhubung ke TX GPS
// TX NodeMCU = Pin D1 (GPIO5) -> terhubung ke RX GPS
static const int RXPin = 4; // D2
static const int TXPin = 5; // D1
static const uint32_t GPSBaud = 9600;

TinyGPSPlus gps;
SoftwareSerial gpsSerial(RXPin, TXPin);

void setup() {
  Serial.begin(9600); // Pastikan Serial Monitor diset ke 115200 baud
  gpsSerial.begin(GPSBaud);

  Serial.println();
  Serial.println("==========================================");
  Serial.println("   Tes GPS NEO-6M (Pin D2 RX & D1 TX)     ");
  Serial.println("==========================================");
}

void loop() {
  // Membaca stream data dari modul GPS
  while (gpsSerial.available() > 0) {
    gps.encode(gpsSerial.read());
  }

  // Menampilkan data setiap 2 detik
  static unsigned long lastPrint = 0;
  if (millis() - lastPrint > 2000) {
    lastPrint = millis();

    Serial.println("------------------------------------------");
    
    if (gps.location.isValid()) {
      Serial.print("Latitude  : ");
      Serial.println(gps.location.lat(), 6);
      Serial.print("Longitude : ");
      Serial.println(gps.location.lng(), 6);
      Serial.print("Satelit   : ");
      Serial.println(gps.satellites.value());
    } else {
      Serial.print("Status    : Mencari Satelit... (Terdeteksi: ");
      Serial.print(gps.satellites.value());
      Serial.println(" satelit)");
    }
  }
}