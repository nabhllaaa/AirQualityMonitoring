#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <TinyGPS++.h>
#include <DHT.h>

// Penstabil daya ESP32
#include "soc/soc.h"
#include "soc/rtc_cntl_reg.h"

// --- DEFINISI PIN ESP32 DEVKIT V1 ---
#define DHTPIN       4      // Pin D4 (GPIO 4)
#define DHTTYPE      DHT22

#define GP2Y_LED     25     // Pin D25 (GPIO 25) -> Penganti D7 Arduino
#define GP2Y_VO      34     // Pin D34 (GPIO 34) -> Penganti A0 Arduino

#define GPS_RX       16     // Pin D16 (RX2 ESP32 ke TX GPS)
#define GPS_TX       17     // Pin D17 (TX2 ESP32 ke RX GPS)

// Timing Pulsa Sensor GP2Y
const unsigned int samplingTime = 280;
const unsigned int deltaTime    = 40;
const unsigned int sleepTime    = 9680;

// Objek Sensor & Display
DHT dht(DHTPIN, DHTTYPE);
LiquidCrystal_I2C lcd(0x27, 16, 2);
TinyGPSPlus gps;

unsigned long lastDisplayTime = 0;
int displayState = 0;

void setup() {
  // Matikan Brownout Detector
  WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0);

  Serial.begin(115200);
  Serial2.begin(9600, SERIAL_8N1, GPS_RX, GPS_TX); // Hardware Serial 2

  // Inisialisasi Pin GP2Y
  pinMode(GP2Y_LED, OUTPUT);
  pinMode(GP2Y_VO, INPUT);
  digitalWrite(GP2Y_LED, HIGH); // Default OFF (Active Low)

  // Inisialisasi I2C (D21 = SDA, D22 = SCL)
  Wire.begin(21, 22);
  dht.begin();

  // Inisialisasi LCD
  lcd.init();
  lcd.backlight();
  lcd.setCursor(0, 0);
  lcd.print("AirRespirA System");
  lcd.setCursor(0, 1);
  lcd.print("Initializing...");
  delay(2000);
  lcd.clear();
}

void loop() {
  // Parsing data GPS
  while (Serial2.available() > 0) {
    gps.encode(Serial2.read());
  }

  // Update sensor & layar setiap 2 detik
  if (millis() - lastDisplayTime >= 2000) {
    lastDisplayTime = millis();

    // 1. BACA SENSOR DEBU GP2Y
    digitalWrite(GP2Y_LED, LOW); // LED ON
    delayMicroseconds(samplingTime);
    int voRaw = analogRead(GP2Y_VO);
    delayMicroseconds(deltaTime);
    digitalWrite(GP2Y_LED, HIGH); // LED OFF
    delayMicroseconds(sleepTime);

    // Konversi Tegangan & Kerapatan Debu
    float volt = voRaw * (3.3 / 4095.0);
    float dustDensity = (0.17 * volt - 0.1) * 1000.0;
    if (dustDensity < 0) dustDensity = 0.0;

    // 2. BACA SENSOR DHT22
    float hum = dht.readHumidity();
    float temp = dht.readTemperature();

    // 3. LOG SERIAL MONITOR
    Serial.println("==========================================");
    Serial.printf("Suhu: %.1f C | Kelembapan: %.1f %%\n", temp, hum);
    Serial.printf("Dust Raw: %d | Tegangan: %.2f V | PM: %.1f ug/m3\n", voRaw, volt, dustDensity);
    
    if (gps.location.isValid()) {
      Serial.printf("GPS: Lat %.6f, Lng %.6f | Sat: %d\n", gps.location.lat(), gps.location.lng(), gps.satellites.value());
    } else {
      Serial.printf("GPS: Searching Satellite... (%d detected)\n", gps.satellites.value());
    }

    // 4. BERGANTI TAMPILAN LCD
    lcd.clear();
    if (displayState == 0) {
      // Tampilan 1: Suhu & Kelembapan
      lcd.setCursor(0, 0);
      if (isnan(temp)) lcd.print("Temp: Error");
      else { lcd.print("Temp: "); lcd.print(temp, 1); lcd.write(223); lcd.print("C"); }

      lcd.setCursor(0, 1);
      if (isnan(hum)) lcd.print("Hum : Error");
      else { lcd.print("Hum : "); lcd.print(hum, 0); lcd.print("%"); }

      displayState = 1;
    } 
    else if (displayState == 1) {
      // Tampilan 2: Sensor Debu
      lcd.setCursor(0, 0);
      lcd.print("Volt: "); lcd.print(volt, 2); lcd.print(" V");

      lcd.setCursor(0, 1);
      lcd.print("Debu: "); lcd.print(dustDensity, 0); lcd.print(" ug/m3");

      displayState = 2;
    } 
    else {
      // Tampilan 3: Status GPS
      lcd.setCursor(0, 0);
      if (gps.location.isValid()) {
        lcd.print("Lat:"); lcd.print(gps.location.lat(), 4);
      } else {
        lcd.print("GPS: Searching");
      }

      lcd.setCursor(0, 1);
      if (gps.location.isValid()) {
        lcd.print("Lng:"); lcd.print(gps.location.lng(), 4);
      } else {
        lcd.print("Satellites: "); lcd.print(gps.satellites.value());
      }

      displayState = 0;
    }
  }
}