#define BLYNK_TEMPLATE_ID "TMPL3qcltYGbT"
#define BLYNK_TEMPLATE_NAME "smart helmet"
#define BLYNK_AUTH_TOKEN "w6Lam-38gEQPNvPJzCDKyBAPbTrpmDbR"

#define BLYNK_PRINT Serial

#include <Adafruit_Sensor.h>
#include <Adafruit_ADXL345_U.h>
#include <WiFi.h>
#include <BlynkSimpleEsp32.h>

#include <TinyGPSPlus.h>
#include <HardwareSerial.h>

// === WiFi Credentials ===
char auth[] = "w6Lam-38gEQPNvPJzCDKyBAPbTrpmDbR";
char ssid[] = "helmet";
char pass[] = "12345678";

// === Pin Definitions ===
#define LIMIT_SWITCH_PIN 4
#define RELAY_MOTOR_PIN 18
#define BUZZER_PIN 5

// === GPS Setup ===
TinyGPSPlus gps;
HardwareSerial GPS_Serial(2);   // RX=16, TX=17

// === Accelerometer ===
Adafruit_ADXL345_Unified accel = Adafruit_ADXL345_Unified();
float ax, ay;
int fallThresholdHigh = 9;
int fallThresholdLow = -9;

// === States ===
bool helmetWorn = false;
bool fallDetected = false;

void setup() {
  Serial.begin(115200);

  // GPS module
  GPS_Serial.begin(9600, SERIAL_8N1, 16, 17);

  // WiFi
  WiFi.begin(ssid, pass);
  while (WiFi.status() != WL_CONNECTED) {
    delay(300);
    Serial.print(".");
  }
  Serial.println("\nWiFi Connected");
  Blynk.begin(auth, ssid, pass);

  // Pins
  pinMode(LIMIT_SWITCH_PIN, INPUT_PULLUP);
  pinMode(RELAY_MOTOR_PIN, OUTPUT);
  pinMode(BUZZER_PIN, OUTPUT);

  // Accelerometer
  if (!accel.begin()) {
    Serial.println("ADXL345 Error!");
    while (1);
  }

  Serial.println("System Ready");
}

void loop() {
  Blynk.run();

  // Helmet state
  helmetWorn = (digitalRead(LIMIT_SWITCH_PIN) == LOW);

  // Fall detection
  fallDetected = detectFall();

  // GPS read
  while (GPS_Serial.available() > 0) {
    gps.encode(GPS_Serial.read());
  }

  // Motor control
  if (helmetWorn && !fallDetected) {
    digitalWrite(RELAY_MOTOR_PIN, HIGH);
  } else {
    digitalWrite(RELAY_MOTOR_PIN, LOW);
  }

  // Fall buzzer
  if (fallDetected) {
    activateAlert();
  }

  // === Send Helmet & Fall Status to Blynk ===
  Blynk.virtualWrite(V0, helmetWorn ? "Helmet Worn" : "Helmet Not Worn");
  Blynk.virtualWrite(V1, fallDetected ? "Fall Detected" : "No Fall");

  // === GPS LAT & LNG TO BLYNK ===
  if (gps.location.isValid()) {
    Blynk.virtualWrite(V2, gps.location.lat());
    Blynk.virtualWrite(V3, gps.location.lng());
  }

  delay(200);
}

// === Fall Detection ===
bool detectFall() {
  sensors_event_t event;
  accel.getEvent(&event);

  ax = event.acceleration.x;
  ay = event.acceleration.y;

  return (ax > fallThresholdHigh || ax < fallThresholdLow ||
          ay > fallThresholdHigh || ay < fallThresholdLow);
}

// === Buzzer Alert ===
void activateAlert() {
  digitalWrite(BUZZER_PIN, HIGH);
  delay(200);
  digitalWrite(BUZZER_PIN, LOW);
}
