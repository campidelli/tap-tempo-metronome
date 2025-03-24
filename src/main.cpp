#include "Arduino.h"
#include "ArduinoTapTempo.h"
#include "ESP32_C3_TimerInterrupt.h"

#define TIMER1_INTERVAL_MS    20
#define LED_PIN               8
#define TAP_TEMPO_SWITCH_PIN  5
#define PWM_AUDIO_OUTPUT_PIN  4
#define CLICK_SOUND_FREQUENCY 1800
#define CLICK_SOUND_DURATION  5

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

  // Attach timer interrupt to check the button state every TIMER1_INTERVAL_MS
  if (ITimer1.attachInterruptInterval(TIMER1_INTERVAL_MS * 1000, TimerHandler1)) {
    Serial.println("Timer interrupt started.");
  } else {
    Serial.println("Failed to start timer interrupt.");
  }
}

void loop() {
  static unsigned long clickEndTime = 0;
  float bpm = tapTempo.getBPM();
  unsigned long interval = 60000 / bpm;

  if (bpm > 0 && millis() - lastClickTime >= interval) {
    lastClickTime += interval; // Keep timing accurate

    digitalWrite(LED_PIN, LOW);

    tone(PWM_AUDIO_OUTPUT_PIN, CLICK_SOUND_FREQUENCY, CLICK_SOUND_DURATION);

    clickEndTime = millis() + 20; // Schedule when to stop the sound

    Serial.print(millis());
    Serial.print(F(" ms - Current BPM: "));
    Serial.println(bpm);
  }

  // Stop sound after 20ms without blocking
  if (millis() >= clickEndTime) {
    noTone(PWM_AUDIO_OUTPUT_PIN);
    digitalWrite(LED_PIN, HIGH);
  }
}