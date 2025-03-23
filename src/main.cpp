#include "Arduino.h"
#include "ArduinoTapTempo.h"
#include "ESP32_C3_TimerInterrupt.h"

#define LED_PIN              8
#define TAP_TEMPO_SWITCH_PIN 5
#define TIMER1_INTERVAL_MS   20
#define PWM_AUDIO_OUTPUT_PIN 4
#define PWM_CHANNEL          0
#define PWM_FREQ             4000 // Frequency in Hz
#define PWM_RESOLUTION       8    // 8-bit resolution (0-255)

ESP32Timer ITimer1(1);    // Timer to check the button state every TIMER1_INTERVAL_MS
ArduinoTapTempo tapTempo; // Used to calculate the BPM

unsigned long lastClickTime = 0;     // Last time a click sound was played
volatile bool buttonPressed = false; // Flag to indicate if the button is pressed

bool IRAM_ATTR TimerHandler1(void * timerNo) {
  if (digitalRead(TAP_TEMPO_SWITCH_PIN) == LOW && !buttonPressed) {
    buttonPressed = true;
  }
  if (digitalRead(TAP_TEMPO_SWITCH_PIN) == HIGH) {
    buttonPressed = false;
  }
  tapTempo.update(buttonPressed);
  return true;
}

void setup() {
  Serial.begin(9600);

  pinMode(TAP_TEMPO_SWITCH_PIN, INPUT_PULLUP);
  pinMode(LED_PIN, OUTPUT);

  // Set up PWM on the audio output pin
  ledcSetup(PWM_CHANNEL, PWM_FREQ, PWM_RESOLUTION);
  ledcAttachPin(PWM_AUDIO_OUTPUT_PIN, PWM_CHANNEL);

  // Attach timer interrupt to check the button state every TIMER1_INTERVAL_MS
  if (ITimer1.attachInterruptInterval(TIMER1_INTERVAL_MS * 1000, TimerHandler1)) {
    Serial.println("Timer interrupt started.");
  } else {
    Serial.println("Failed to start timer interrupt.");
  }
}

void loop() {
  float bpm = tapTempo.getBPM();
  unsigned long interval = 60000 / bpm; // Interval in milliseconds for each beat

  // Click sound and LED blink if it's time for a beat
  if (bpm > 0 && millis() - lastClickTime >= interval) {
    Serial.print(F("Current BPM: "));
    Serial.println(bpm);

    digitalWrite(LED_PIN, LOW);

    ledcWrite(PWM_CHANNEL, 200); // Set a moderate duty cycle
    delay(20);
    ledcWrite(PWM_CHANNEL, 0);   // Stop sound

    digitalWrite(LED_PIN, HIGH);

    lastClickTime = millis(); // Update the last click time
  }
}
