
/* ----------------------------------------------
🔹 SOLAR TRACKER OTOMATIS metode BLIND SETTING dengan Arduino 🔹
----------------------------------------------
- Menggunakan motor stepper NEMA 17 + driver TB6600 Sebagai penggerak
- Tracking matahari otomatis dari jam 9 pagi - 3 sore
- Panel dikunci di barat pada jam 3 sore - 6 sore
- Panel kembali ke posisi awal pada jam 6 pagi
- Menyimpan data tegangan & arus ke SD Card saat tracking
----------------------------------------------
*/

//Library yang digunakan
#include <Wire.h>
#include <LiquidCrystal_I2C.h>  // Untuk LCD I2C
#include <SPI.h>                // Untuk SD Card
#include <SD.h>                 // Untuk SD Card
#include <RTClib.h>             // Untuk RTC DS3231
#include <ACS712.h>

//Definisi pin
#define PUL 4             // Pin PUL+ (Pulse) dari TB6600
#define DIR 5             // Pin DIR+ (Direction) dari TB6600
#define ENA 6             // Pin ENA+ (Enable) dari TB6600
#define VOLTAGE_SENSOR A0 // Sensor tegangan
#define CURRENT_SENSOR A1 // Sensor arus
#define SD_CS 10          // Pin Chip Select SD Card

//Inisialisasi LCD, RTC, dan SD Card
LiquidCrystal_I2C lcd(0x27, 16, 2);
RTC_DS3231 rtc;
File dataFile;

//konfigurasi sensor acs712
ACS712  ACS(CURRENT_SENSOR, 5.0, 1023, 66); //ACS712 30A uses  66 mV per A

//inisialisai motor
const int stepInterval = 2; // Waktu antar langkah dalam milidetik
bool stepState = LOW; // Menyimpan status sinyal STEP

//inisialisasi waktu
unsigned long previousMillis = 0;

void setup() {
    Serial.begin(9600);
    lcd.begin(16, 2);
    lcd.backlight();
    
    //Inisialisasi pin motor stepper
    pinMode(PUL, OUTPUT);
    pinMode(DIR, OUTPUT);
    pinMode(ENA, OUTPUT);
    
    //Inisialisasi sensor tegangan & arus
    pinMode(VOLTAGE_SENSOR, INPUT);
    pinMode(CURRENT_SENSOR, INPUT);

    //Inisialisasi RTC
    if (!rtc.begin()) {
        Serial.println("RTC tidak ditemukan!");
        lcd.setCursor(0, 0);
        lcd.print("RTC ERROR");
        while (1);
    }

    //Inisialisasi SD Card
    if (!SD.begin(SD_CS)) {
        Serial.println("SD Card gagal!");
        lcd.setCursor(0, 1);
        lcd.print("SD ERROR");
        while (1);
    }

    //inisalisasi sensor acs712
    ACS.autoMidPoint();

    //inisialisasi motor
    pinMode(STEP_PIN, OUTPUT);
    pinMode(DIR_PIN, OUTPUT);
    pinMode(ENABLE_PIN, OUTPUT);

    motorEnable(true);   // Aktifkan driver
    setDirection(true);  // Atur arah CW
}

void loop() {
    //Ambil waktu saat ini
    DateTime now = rtc.now();
    unsigned long currentMillis = now.hour() * 3600 + now.minute() * 60 + now.second();

    //Ambil data sensor tegangan & arus
    float voltage = getAverageVoltage();
    float current = getAverageCurrent();

    if (currentMillis - previousMillis >= 100) {
        previousMillis = currentMillis;
        
        //Tampilkan waktu & sensor ke LCD
        lcd.setCursor(0, 0);
        lcd.print("Time: ");
        lcd.print(now.hour()); lcd.print(":");
        lcd.print(now.minute()); lcd.print(":");
        lcd.print(now.second());

        lcd.setCursor(0, 1);
        lcd.print("V:");
        lcd.print(voltage, 2);
        lcd.print(" I:");
        lcd.print(current, 2);
        lcd.print("mA  ");
    }
    
    // if (currentMillis - previousMillis >= 100) {
    //     previousMillis = currentMillis;

    // }

    motor(); // Jalankan pergerakan motor

}

// ----------------------------------------------
// FUNGSI UNTUK MENGGERAKKAN MOTOR STEPPER
// ----------------------------------------------

// Fungsi untuk mengaktifkan atau menonaktifkan motor driver
void motorEnable(bool enable) {
  digitalWrite(ENABLE_PIN, enable ? LOW : HIGH); // LOW = Aktif, HIGH = Nonaktif
}

// Fungsi untuk mengatur arah putaran
void setDirection(bool clockwise) {
  digitalWrite(DIR_PIN, clockwise ? HIGH : LOW);
}

void motor() {
  unsigned long currentTime = millis();

  if (currentTime - previousStepTime >= stepInterval) {
    previousStepTime = currentTime;
    
    // Kirim satu pulsa STEP (HIGH lalu LOW)
    digitalWrite(STEP_PIN, HIGH);
    delayMicroseconds(5);  // Pulsa minimum agar terbaca TB6600
    digitalWrite(STEP_PIN, LOW);
  }
}

// ----------------------------------------------
//FUNGSI UNTUK MENYIMPAN DATA KE SD CARD
// ----------------------------------------------
void logData(DateTime now, float voltage, float current) {
    dataFile = SD.open("datalog.csv", FILE_WRITE);
    if (dataFile) {
        dataFile.print(now.year()); dataFile.print("/");
        dataFile.print(now.month()); dataFile.print("/");
        dataFile.print(now.day()); dataFile.print(",");
        dataFile.print(now.hour()); dataFile.print(":");
        dataFile.print(now.minute()); dataFile.print(":");
        dataFile.print(now.second()); dataFile.print(",");
        dataFile.print(voltage, 2); dataFile.print(",");
        dataFile.print(current, 2); dataFile.println();
        dataFile.close();
    } else {
        Serial.println("Gagal menulis ke SD Card");
    }
}

// ----------------------------------------------
//FUNGSI UNTUK MEMBACA SENSOR TEGANGAN & ARUS
// ----------------------------------------------
float getAverageCurrent() {
    int mA = ACS.mA_DC();

    // Serial.println(A);

    return mA;
}

float getAverageVoltage() {
  int adc = analogRead(VOLTAGE_SENSOR);
  // Serial.print("adc");
  // Serial.println(adc);
  float voltage = adc * (5.0 / 1023) * 2; // 2 adalah nilai pembagi tegangan dengan rumus (R1 + R2) / R2
  // Serial.print("voltage");
  // Serial.println(voltage);
  return voltage;
}
