#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <SPI.h>
#include <SD.h>
#include <RTClib.h>

// Pin Konfigurasi Stepper
#define ENA 6
#define DIR 5
#define PUL 4

// Pin Sensor
#define VOLTAGE_SENSOR A0
#define CURRENT_SENSOR A1
#define SD_CS 10

// Parameter Stepper
#define STEPS_PER_REV 200
const int totalSteps = 26; // 26 langkah untuk mencapai 90 derajat
const int stepInterval = 840; // 14 menit dalam detik
const float stepAngle = 3.5; // 3.5 derajat per langkah
const int stepsPerMove = 1; // 1 langkah per interval

// Inisialisasi Modul
LiquidCrystal_I2C lcd(0x27, 16, 2);
RTC_DS3231 rtc;
File dataFile;

// Variabel Sensor
float voltage = 0.0;
float current = 0.0;
const float vRef = 5.0;
const float currentOffset = 2.5;
const float sensitivity = 0.185;

// Variabel Stepper
int stepCount = 0;
unsigned long lastStepTime = 0;

void setup() {
    lcd.init();
    lcd.backlight();
    Serial.begin(9600);
    
    pinMode(ENA, OUTPUT);
    pinMode(DIR, OUTPUT);
    pinMode(PUL, OUTPUT);
    
    digitalWrite(ENA, LOW); // Pastikan motor dalam keadaan aktif
    delay(10); // Tambahkan delay untuk memastikan arus masuk ke motor

    // Inisialisasi RTC
    if (!rtc.begin()) {
        Serial.println("RTC tidak ditemukan!");
        while (1);
    }

    // Inisialisasi SD Card
    if (!SD.begin(SD_CS)) {
        Serial.println("SD Card gagal diinisialisasi!");
        lcd.setCursor(0, 0);
        lcd.print("SD Card Error");
        while (1);
    }
}

// Fungsi Gerak Stepper dengan Kecepatan Normal
void moveStepper(int steps, bool direction) {
    digitalWrite(DIR, direction);
    digitalWrite(ENA, LOW); // Pastikan motor aktif
    delay(5); // Delay tambahan untuk memastikan sinyal stabil

    for (int i = 0; i < steps; i++) {
        digitalWrite(PUL, HIGH);
        delayMicroseconds(500);
        digitalWrite(PUL, LOW);
        delayMicroseconds(500);
    }
}

// Fungsi Gerak Stepper dengan Kecepatan Tinggi
void moveStepperFast(int steps, bool direction) {
    digitalWrite(DIR, direction);
    digitalWrite(ENA, LOW); // Pastikan motor aktif
    delay(5);

    for (int i = 0; i < steps; i++) {
        digitalWrite(PUL, HIGH);
        delayMicroseconds(300); // Kurangi delay untuk pergerakan lebih cepat
        digitalWrite(PUL, LOW);
        delayMicroseconds(300);
    }
}

void loop() {
    DateTime now = rtc.now();

    // Pembacaan Sensor
    int rawVoltage = analogRead(VOLTAGE_SENSOR);
    voltage = (rawVoltage * vRef) / 1023.0 * 5.0;
    float rawCurrent = analogRead(CURRENT_SENSOR) * vRef / 1023.0;
    current = (rawCurrent - currentOffset) / sensitivity;

    // Menampilkan Waktu pada LCD
    lcd.setCursor(0, 0);
    if (now.hour() < 10) lcd.print("0");
    lcd.print(now.hour());
    lcd.print(":");
    if (now.minute() < 10) lcd.print("0");
    lcd.print(now.minute());
    lcd.print(":");
    if (now.second() < 10) lcd.print("0");
    lcd.print(now.second());

    // Menampilkan Tegangan dan Arus pada LCD
    lcd.setCursor(0, 1);
    lcd.print("V=");
    lcd.print(voltage, 2);
    lcd.print(" I=");
    lcd.print(current, 2);

    // Menyimpan Data ke SD Card hanya saat tracking (jam 9 pagi - 3 sore)
    if (now.hour() >= 9 && now.hour() < 15) {
        dataFile = SD.open("data_log.csv", FILE_WRITE);
        if (dataFile) {
            dataFile.print(now.year());
            dataFile.print("/");
            dataFile.print(now.month());
            dataFile.print("/");
            dataFile.print(now.day());
            dataFile.print(",");
            dataFile.print(now.hour());
            dataFile.print(":");
            dataFile.print(now.minute());
            dataFile.print(":");
            dataFile.print(now.second());
            dataFile.print(",");
            dataFile.print(voltage, 2);
            dataFile.print(",");
            dataFile.println(current, 2);
            dataFile.close();
        }
    }

    // Kontrol Stepper Motor Berdasarkan Waktu
    if (now.hour() >= 9 && now.hour() < 15) {  // Tracking dari jam 9 pagi hingga 3 sore
        if (stepCount < totalSteps && (now.unixtime() - lastStepTime) >= stepInterval) {
            Serial.println("Menggerakkan stepper satu langkah ke arah barat (CCW)");
            moveStepper(stepsPerMove, false);
            stepCount++;
            lastStepTime = now.unixtime();
            Serial.print("Jumlah langkah saat ini: ");
            Serial.println(stepCount);
        }
    } else if (now.hour() >= 15 && now.hour() < 18) {  // Mengunci posisi di barat dari jam 3 sore hingga 6 sore
        Serial.println("Mengunci posisi stepper di barat");
        digitalWrite(ENA, LOW); // Pastikan motor tetap aktif
    } else if (now.hour() >= 18) {  // Pukul 18:00 atau lebih, reset ke timur
        Serial.println("Pukul 18:00 - Menggerakkan stepper 25 langkah ke timur secara cepat");
        moveStepperFast(25, true);
        digitalWrite(ENA, LOW); // Pastikan motor tetap aktif setelah kembali ke timur
    } else if (now.hour() == 6) {  // Pukul 06:00 reset ke timur lagi untuk standby
        Serial.println("Pukul 06:00 - Menggerakkan stepper 25 langkah ke timur secara cepat untuk reset");
        moveStepperFast(25, true);
        stepCount = 0;
        digitalWrite(ENA, LOW); // Pastikan motor tetap aktif setelah reset
    }

    delay(1000);
}
