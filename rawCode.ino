#include <RTClib.h>

//definisi motor
#define STEP_PIN 4   // Pin STEP untuk TB6600
#define DIR_PIN 5   // Pin DIR untuk arah putaran
#define ENABLE_PIN 6 // Pin ENABLE (opsional)

const int stepsPerCycle = 10;       // 250 langkah per siklus
const int cycleTime = 100;           // Waktu satu siklus (ms)
const unsigned long delayBetweenCycles = 5000; // 7 menit dalam milidetik

unsigned long previousCycleTime = 0;
unsigned long previousMillis = 0;

//rtc
RTC_DS3231 rtc;

void setup() {
  Serial.begin(115200);

  //rtc
  if (! rtc.begin()) {
    Serial.println("Couldn't find RTC");
    Serial.flush();
  }

  if (rtc.lostPower()) {
    Serial.println("RTC lost power, let's set the time!");
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
  }

  //definisi motor
  pinMode(STEP_PIN, OUTPUT);
  pinMode(DIR_PIN, OUTPUT);
  pinMode(ENABLE_PIN, OUTPUT);
  
  digitalWrite(ENABLE_PIN, LOW); // Aktifkan driver

}

// Fungsi untuk menggerakkan motor 250 langkah dalam 500 ms
void moveStepper() {
  unsigned long startTime = millis();
  digitalWrite(DIR_PIN, HIGH);   // Arah jarum jam
  while (millis() - startTime < cycleTime) {
    for (int i = 0; i < stepsPerCycle; i++) {
      digitalWrite(STEP_PIN, HIGH);
      delayMicroseconds(100);
      digitalWrite(STEP_PIN, LOW);
      delayMicroseconds(100);
    }
  }
}

void moveBackStepper() {
  unsigned long currentMillis = millis();
  digitalWrite(ENABLE_PIN, LOW);
  digitalWrite(DIR_PIN, LOW);   // Arah jarum jam

  if (currentMillis - previousMillis >= 800) {
        previousMillis = currentMillis; // Reset timer

      for (int i = 0; i < 5; i++) { // Loop untuk 10 langkah
                digitalWrite(STEP_PIN, HIGH);
                delayMicroseconds(100); // Tunggu 100 µs
                digitalWrite(STEP_PIN, LOW);
                delayMicroseconds(100); // Tunggu 500 µs
      }
  }
}

void motor() {
  unsigned long currentTime = millis();

  // Jika sudah waktunya bergerak (setiap 7 menit)
  if (currentTime - previousCycleTime >= delayBetweenCycles) {
    previousCycleTime = currentTime;
    moveStepper();  // Jalankan motor 250 langkah dalam 500 ms
  }
}

void loop() {
  DateTime now = rtc.now();
    Serial.print(now.hour(), DEC);
    Serial.print(':');
    Serial.print(now.minute(), DEC);
    Serial.print(':');
    Serial.print(now.second(), DEC);
    Serial.println();

  int startHour = 1, startMinute = 49;  // Waktu mulai motor berjalan
  int stopHour = 1, stopMinute = 50;    // Waktu berhenti motor

  int lockHour = 1, lockMinute = 50;  // Waktu motor dikunci
  int unlockHour = 1, unlockMinute = 53; // Waktu motor tidak terkunci lagi
  

  if (isTimeInRange(now.hour(), now.minute(), startHour, startMinute, stopHour, stopMinute)) {
    motor(); // Jalankan motor jika dalam rentang waktu
  }

  else if (isTimeInRange(now.hour(), now.minute(), lockHour, lockMinute, unlockHour, unlockMinute)) {
    lockMotor(); // Kunci motor di posisi terakhir
  } 
  else if (now.hour() == 2  && now.minute() == 20) {
    moveBackStepper();
  } 
  
  else if (now.hour() == 2 && now.minute() == 22 ) {
    moveBackStepper();
  } 

}

// Fungsi untuk mengecek apakah waktu sekarang dalam rentang aktif
bool isTimeInRange(int currentHour, int currentMinute, int startHour, int startMinute, int stopHour, int stopMinute) {
  if ((currentHour > startHour || (currentHour == startHour && currentMinute >= startMinute)) &&
      (currentHour < stopHour || (currentHour == stopHour && currentMinute < stopMinute))) {
    return true; // Waktu sekarang berada dalam rentang
  }
  return false; // Waktu sekarang di luar rentang
}

// Fungsi untuk mengunci motor di posisi terakhir
void lockMotor() {
  Serial.println("Motor dikunci di posisi terakhir!");
  digitalWrite(ENABLE_PIN, LOW); // Mengunci motor
}


