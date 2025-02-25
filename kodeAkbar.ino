//Solar_Tracker

#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <SPI.h>
#include <SD.h>
#include <RTClib.h>

#define pinEna 6
#define pinDir 5
#define pinStep 4
#define pinCS 10
#define pinAmp A2
#define pinVol A3

LiquidCrystal_I2C lcd(0x27, 16, 2); 
RTC_DS3231 rtc;

// Waktu operasi (09:00 - 15:00)
const int START_HOUR = 15;
const int STOP_HOUR = 16;
// Interval antar langkah (600 detik = 10 menit)
const int STEP_INTERVAL = 72000; // dalam milidetik (600 detik = 10 menit)
unsigned long lastStepTime = 0; // Waktu terakhir motor melangkah
int stepCount = 0; // Menghitung jumlah langkah motor

void setup() {
    Serial.begin(9600);
    
    // Inisialisasi LCD
    lcd.init();
    lcd.backlight();
    lcd.setCursor(0, 0);
    lcd.print("Inisialisasi...");
    
    // Inisialisasi RTC
    if (!rtc.begin()) {
        Serial.println("RTC tidak ditemukan!");
        lcd.setCursor(0, 1);
        lcd.print("RTC Error!");
        while (1);
    }

    // Inisialisasi motor driver
    pinMode(pinStep, OUTPUT);
    pinMode(pinDir, OUTPUT);
    
    Serial.println("Sistem siap.");
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Sistem Siap");
}

void loop() {
    // Ambil waktu sekarang dari RTC
    DateTime now = rtc.now();
    int currentHour = now.hour();
    int currentMinute = now.minute();
    
    // Perbarui tampilan waktu di LCD
    lcd.setCursor(0, 0);
    lcd.print("Waktu: ");
    if (currentHour < 10) lcd.print("0");
    lcd.print(currentHour);
    lcd.print(":");
    if (currentMinute < 10) lcd.print("0");
    lcd.print(currentMinute);
    
    // Cek apakah dalam rentang waktu operasi
    if (currentHour >= START_HOUR && currentHour < STOP_HOUR) {
        if (millis() - lastStepTime >= STEP_INTERVAL) {
            lastStepTime = millis(); // Perbarui waktu langkah terakhir
            stepCount++; // Tambah jumlah langkah

            Serial.print("Motor berjalan pada: ");
            Serial.print(currentHour); Serial.print(":");
            Serial.println(currentMinute);

            digitalWrite(pinDir, HIGH); // Arah putaran (bisa disesuaikan)

            // Satu langkah saja setiap 10 menit
            digitalWrite(pinStep, HIGH);
            delayMicroseconds(500);
            digitalWrite(pinStep, LOW);
            delayMicroseconds(500);

            // Tampilkan status motor di LCD
            lcd.setCursor(0, 1);
            lcd.print("Motor on ");
        }

        // Tampilkan jumlah langkah motor di LCD
        lcd.setCursor(9, 1);
        lcd.print("Step:");
        lcd.print(stepCount);
    } else {
        Serial.print("Motor berhenti pada: ");
        Serial.print(currentHour); Serial.print(":");
        Serial.println(currentMinute);

        // Tampilkan status motor berhenti di LCD
        lcd.setCursor(0, 1);
        lcd.print("Motor: BERHENTI ");
    }
    
    delay(1000); // Loop setiap 1 detik
}
