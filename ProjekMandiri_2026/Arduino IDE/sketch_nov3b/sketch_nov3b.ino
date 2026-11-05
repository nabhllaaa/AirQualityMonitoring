#include <DHT.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

// Alamat I2C 0x27 (Ukuran LCD 16x2)
// Pin SDA = GPIO 21, Pin SCL = GPIO 22 pada ESP32 DevKit V1
LiquidCrystal_I2C lcd(0x27, 16, 2);

// Tentukan pin digital ESP32 untuk DHT22 (Menggunakan GPIO 4)
#define DHTPIN 4 
#define DHTTYPE DHT22   

DHT dht(DHTPIN, DHTTYPE);

// Array teks pesan panjang yang akan di-scroll vertikal (maksimal 16 karakter per elemen)
String pesanPembuka[] = {
  "Sistem Pemantau ",
  "Kualitas Udara  ",
  "Metode Mobile   ",
  "Z.A Pagar Alam  "
};
int jumlahPesan = sizeof(pesanPembuka) / sizeof(pesanPembuka[0]);

// --- FUNGSI KUSTOM SCROLL VERTIKAL ---
void scrollVertikal(String arrPesan[], int totalPesan, int delaySpeed) {
  for (int i = 0; i < totalPesan; i++) {
    lcd.clear();
    
    // Tampilkan kalimat saat ini di Baris Atas (Baris 0)
    lcd.setCursor(0, 0);
    lcd.print(arrPesan[i]);
    
    // Jika ada kalimat berikutnya, tampilkan di Baris Bawah (Baris 1)
    if (i + 1 < totalPesan) {
      lcd.setCursor(0, 1);
      lcd.print(arrPesan[i + 1]);
    }
    
    delay(delaySpeed); // Tahan beberapa saat sebelum bergeser
  }
}

void setup() {
  // Serial monitor ESP32 menggunakan baud rate 115200
  Serial.begin(9600);
  dht.begin();
  
  // Inisialisasi LCD 16x2 I2C
  lcd.init();                      
  lcd.backlight(); 
  
  // --- TAMPILAN PEMBUKA SCROLL VERTIKAL ---
  scrollVertikal(pesanPembuka, jumlahPesan, 1500); // Bergeser tiap 1.5 detik
  
  lcd.clear();
}

void loop() {
  delay(2000); // Jeda pembacaan DHT22 (minimal 2 detik)

  float h = dht.readHumidity();
  float t = dht.readTemperature();

  // Cek apakah pembacaan sensor gagal
  if (isnan(h) || isnan(t)) {
    Serial.println(F("Gagal membaca data dari sensor DHT22!"));
    lcd.setCursor(0, 0);
    lcd.print("ERR: Sensor DHT ");
    lcd.setCursor(0, 1);
    lcd.print("Cek Pin GPIO 4! ");
    return;
  }

  // Tampilkan data ke Serial Monitor
  Serial.print(F("Suhu: "));
  Serial.print(t, 1);
  Serial.print(F(" °C | Kelembapan: "));
  Serial.print(h, 1);
  Serial.println(F(" %"));

  // --- EFEK SCROLL VERTIKAL UNTUK DATA SENSOR ---
  // Step 1: Data Suhu muncul di baris bawah
  lcd.clear();
  lcd.setCursor(0, 1);
  lcd.print("Suhu  : " + String(t, 1) + (char)223 + "C   ");
  delay(1200);

  // Step 2: Data Suhu naik ke baris atas, Data Kelembapan muncul di baris bawah
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Suhu  : " + String(t, 1) + (char)223 + "C   ");
  lcd.setCursor(0, 1);
  lcd.print("Lembap: " + String(h, 1) + "%    ");
  delay(2500);

  // Step 3: Data Suhu hilang, Kelembapan naik ke baris atas
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Lembap: " + String(h, 1) + "%    ");
  delay(1200);
}