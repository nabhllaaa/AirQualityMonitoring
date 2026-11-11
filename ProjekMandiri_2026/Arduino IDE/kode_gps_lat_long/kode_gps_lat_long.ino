#include <WiFi.h>
#include <HTTPClient.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <TinyGPS++.h>
#include <DHT.h>

// ==========================================
// KONTROL WIFI & SUPABASE
// ==========================================
const char* WIFI_SSID     = "Krisaturnus";     
const char* WIFI_PASSWORD = "1sampe8000"; 

const char* SUPABASE_URL  = "https://tvoicnwwvjqdfknfwhmj.supabase.co/rest/v1/airrespira_logs";
const char* SUPABASE_KEY  = "sb_publishable_pp6T6a_nFDWeKYtidPh2Rw_vxe6tscM"; 

// ==========================================
// HARDWARE PIN DEFINITION (ESP32)
// ==========================================
#define GPS_RX 26   // GPIO27 <- TX GPS
#define GPS_TX 25   // GPIO26 -> RX GPS
#define GPS_BAUD 9600

#define PIN_LED 32  // Pin 3 GP2Y (Active LOW)
#define PIN_VO  34  // Pin 5 GP2Y (Analog VP)

#define DHTPIN  4   // Pin DATA DHT22 -> GPIO4
#define DHTTYPE DHT22

const int samplingTime = 280;
const int deltaTime    = 40;
const int sleepTime    = 9680;

LiquidCrystal_I2C lcd(0x27, 16, 2);
TinyGPSPlus gps;
DHT dht(DHTPIN, DHTTYPE);

float calcVoltage = 0;
float dustDensity = 0;
float humidity    = 0;
float temperature = 0;

void setup() {
  Serial.begin(115200);
  delay(500);

  Serial2.begin(GPS_BAUD, SERIAL_8N1, GPS_RX, GPS_TX);
  dht.begin();

  pinMode(PIN_LED, OUTPUT);
  digitalWrite(PIN_LED, HIGH);

  lcd.init();
  lcd.backlight();
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(" AirRespirA IoT ");
  lcd.setCursor(0, 1);
  lcd.print(" Starting Up... ");

  // Inisialisasi Wi-Fi Sekali Saja saat Bootup
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
}

void sendToSupabase(String timestampWib, double lat, double lng, int sat, float temp, float hum, float vo, float pm) {
  // Hanya kirim jika Wi-Fi benar-benar terkoneksi
  if (WiFi.status() != WL_CONNECTED) {
    return;
  }

  HTTPClient http;
  http.begin(SUPABASE_URL);
  
  // Timeout 1.5 detik agar program utama & LCD tidak freeze jika sinyal buruk
  http.setTimeout(1500);

  http.addHeader("Content-Type", "application/json");
  http.addHeader("apikey", SUPABASE_KEY);
  http.addHeader("Authorization", String("Bearer ") + SUPABASE_KEY);
  http.addHeader("Prefer", "return=minimal");

  char jsonPayload[256];
  snprintf(jsonPayload, sizeof(jsonPayload),
    "{\"timestamp_wib\":\"%s\",\"latitude\":%.6f,\"longitude\":%.6f,\"satellites\":%d,\"temperature\":%.1f,\"humidity\":%.1f,\"vo_volt\":%.3f,\"pm25\":%.1f}",
    timestampWib.c_str(), lat, lng, sat, temp, hum, vo, pm);

  int httpResponseCode = http.POST(jsonPayload);
  
  if (httpResponseCode == 201 || httpResponseCode == 200) {
    Serial.println(" -> [Cloud Push OK]");
  } else {
    Serial.printf(" -> [Cloud Push Fail: %d]\n", httpResponseCode);
  }

  http.end();
}

void loop() {
  // 1. Stream data GPS nonstop
  while (Serial2.available() > 0) {
    gps.encode(Serial2.read());
  }

  // 2. Auto-reconnect Wi-Fi aman (Cek tiap 10 detik jika terputus)
  static unsigned long lastWifiCheck = 0;
  if (millis() - lastWifiCheck >= 10000) {
    lastWifiCheck = millis();
    if (WiFi.status() != WL_CONNECTED) {
      WiFi.reconnect();
    }
  }

  // 3. Update LCD & Sampling Sensor (Tiap 1 Detik)
  static unsigned long lastLcdUpdate = 0;
  static bool displayToggle = false;

  if (millis() - lastLcdUpdate >= 1000) {
    lastLcdUpdate = millis();

    // Moving Average 10 Sampel GP2Y
    float totalMv = 0;
    const int sampleCount = 10;
    for (int i = 0; i < sampleCount; i++) {
      digitalWrite(PIN_LED, LOW);
      delayMicroseconds(samplingTime);
      totalMv += analogReadMilliVolts(PIN_VO);
      delayMicroseconds(deltaTime);
      digitalWrite(PIN_LED, HIGH);
      delayMicroseconds(sleepTime);
    }

    float avgMv = totalMv / sampleCount;
    calcVoltage = avgMv / 1000.0f;

    if (calcVoltage >= 1.50) {
      dustDensity = (calcVoltage - 1.50) / 0.005f;
    } else {
      dustDensity = 0.0;
    }

    humidity = dht.readHumidity();
    temperature = dht.readTemperature();
    if (isnan(humidity) || isnan(temperature)) {
      humidity = 0.0;
      temperature = 0.0;
    }

    char line1[32];
    char line2[32];

    // Ganti Tampilan LCD tiap 2 detik antara (Jam & Lokasi) vs (Suhu & Hum)
    static int toggleCounter = 0;
    toggleCounter++;
    if (toggleCounter >= 2) {
      toggleCounter = 0;
      displayToggle = !displayToggle;
    }

    if (gps.location.isValid()) {
      int hourWib = (gps.time.hour() + 7) % 24;

      if (!displayToggle) {
        snprintf(line1, sizeof(line1), "%02d:%02d PM:%03.0f ug", hourWib, gps.time.minute(), dustDensity);
        snprintf(line2, sizeof(line2), "%.4f,%.4f", gps.location.lat(), gps.location.lng());
      } else {
        snprintf(line1, sizeof(line1), "T:%.1fC H:%.0f%%", temperature, humidity);
        snprintf(line2, sizeof(line2), "PM:%.0fug Sat:%02d", dustDensity, gps.satellites.value());
      }
    } else {
      snprintf(line1, sizeof(line1), "PM:%.0f T:%.1fC", dustDensity, temperature);
      snprintf(line2, sizeof(line2), "GPS Sat:%02d Lock..", gps.satellites.value());
    }

    // Refresh Layar LCD
    lcd.setCursor(0, 0);
    lcd.print("                ");
    lcd.setCursor(0, 0);
    lcd.print(line1);

    lcd.setCursor(0, 1);
    lcd.print("                ");
    lcd.setCursor(0, 1);
    lcd.print(line2);
  }

  // 4. Push ke Supabase (Tiap 3 Detik)
  static unsigned long lastCloudPush = 0;
  if (millis() - lastCloudPush >= 3000) {
    lastCloudPush = millis();

    if (gps.location.isValid()) {
      int hourWib = (gps.time.hour() + 7) % 24;
      char timeBuf[16];
      snprintf(timeBuf, sizeof(timeBuf), "%02d:%02d:%02d", hourWib, gps.time.minute(), gps.time.second());

      sendToSupabase(String(timeBuf), gps.location.lat(), gps.location.lng(), 
                     gps.satellites.value(), temperature, humidity, calcVoltage, dustDensity);
    }
  }
}