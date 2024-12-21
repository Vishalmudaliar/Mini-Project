#include <Wire.h>
#include <LiquidCrystal_I2C.h>

// Define the I2C address for the LCD
LiquidCrystal_I2C lcd(0x27, 16, 2); // Adjust I2C address if necessary

// Define the pin for the heartbeat sensor
#define HEARTBEAT_SENSOR_PIN A0

// Variables for heartbeat calculation
int heartbeatValue = 0;
int threshold = 700; // Adjust this threshold to reduce noise
bool heartbeatDetected = false;

unsigned long lastBeatTime = 0;     // Time of the last detected beat
unsigned long currentTime = 0;      // Current time
unsigned long beatInterval = 0;     // Time between two beats
int bpm = 0;                        // Beats per minute
int bpmArray[5] = {0};              // Rolling buffer for averaging BPM
int bpmIndex = 0;                   // Index for the rolling buffer
int numBeats = 0;                   // Number of valid beats
unsigned long noBeatTimeout = 5000; // Timeout to reset BPM (in ms)

// Helper function to calculate the average BPM
int calculateAverageBPM() {
  int total = 0;
  for (int i = 0; i < numBeats; i++) {
    total += bpmArray[i];
  }
  return (numBeats > 0) ? total / numBeats : 0;
}

void setup() {
  // Initialize the LCD
  lcd.begin();
  lcd.backlight();

  // Print a welcome message
  lcd.setCursor(0, 0);
  lcd.print("Heartbeat Monitor");
  delay(1000);
  lcd.clear();

  // Initialize the serial monitor for debugging
  Serial.begin(230400); // High baud rate for faster updates
  Serial.println("Heartbeat Sensor Initialized");
}

void loop() {
  // Read the heartbeat sensor value
  heartbeatValue = analogRead(HEARTBEAT_SENSOR_PIN);
  currentTime = millis();

  // Detect heartbeat
  if (heartbeatValue > threshold && !heartbeatDetected) {
    heartbeatDetected = true;

    // Calculate BPM
    if (lastBeatTime > 0) {
      beatInterval = currentTime - lastBeatTime;

      if (beatInterval > 300) { // Minimum interval to avoid noise (200ms = 300 BPM max)
        bpm = 60000 / beatInterval;

        // Store BPM in the rolling buffer
        bpmArray[bpmIndex] = bpm;
        bpmIndex = (bpmIndex + 1) % 5; // Circular buffer
        numBeats = min(numBeats + 1, 5); // Increment beats up to 5
      }
    }

    lastBeatTime = currentTime;

    // Display heartbeat detection on the LCD
    lcd.setCursor(0, 0);
    lcd.print("Heartbeat Detected");
  }

  // Reset heartbeat detection when the signal drops
  if (heartbeatValue <= threshold && heartbeatDetected) {
    heartbeatDetected = false;
  }

  // Timeout to reset BPM if no heartbeat detected
  if (currentTime - lastBeatTime > noBeatTimeout) {
    bpm = 0;  // Reset BPM to zero
    numBeats = 0; // Reset rolling buffer
  }

  // Display signal value on the LCD
  lcd.setCursor(0, 1);
  lcd.print("Sig: ");
  lcd.print(heartbeatValue);

  // Send signal value to serial port for plotting
  Serial.print(heartbeatValue);  // Send signal value

  Serial.print(",");           // Delimiter between values
  Serial.println(bpm > 0 ? calculateAverageBPM() : 0);  // Send BPM if detected, else 0

  delay(100); // 100 Hz update rate
}
