#define DIR_PIN 5   // Pin arah
#define STEP_PIN 4  // Pin step
#define ENABLE_PIN 6  // Pin enable (opsional)

unsigned long previousMillis = 0;
const long interval = 60000; // 1 detik antar set langkah
const int stepsPerCycle = 12000; // 10 langkah per siklus
int stepCount = 0;
bool stepState = LOW;

void setup() {
    pinMode(DIR_PIN, OUTPUT);
    pinMode(STEP_PIN, OUTPUT);
    pinMode(ENABLE_PIN, OUTPUT);

    digitalWrite(ENABLE_PIN, LOW); // Aktifkan driver
    digitalWrite(DIR_PIN, HIGH);   // Arah jarum jam
}

void loop() {
    unsigned long currentMillis = millis();
    
    if (currentMillis - previousMillis >= interval) {
        previousMillis = currentMillis; // Reset timer

        for (int i = 0; i < stepsPerCycle; i++) { // Loop untuk 10 langkah
            digitalWrite(STEP_PIN, HIGH);
            delayMicroseconds(100); // Tunggu 100 µs
            digitalWrite(STEP_PIN, LOW);
            delayMicroseconds(100); // Tunggu 500 µs
        }
    }

}
