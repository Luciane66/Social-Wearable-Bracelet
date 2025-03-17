#include <Adafruit_CircuitPlayground.h>

// ----- Melody for "Ayers Rock" from Top Gear 2 -----
// (Frequencies in Hz and note durations in milliseconds)

//alternative song for positive response
//const int topGearMelody[] = {266, 241, 269,320, 267,266, 241, 269,320, 267,312, 298,263, 196, 263, 312, 298, 238, 266};
//const int topGearDurations[] = {200, 200, 200, 400, 350,200, 200, 200, 400, 350,200, 200, 200, 300, 300,200, 200, 200, 300};
const int topGearMelody[] = {412, 427, 462,487};
const int topGearDurations[] = {200, 200, 200, 800};
const int melodyLength = sizeof(topGearMelody) / sizeof(topGearMelody[0]);

// ----- Button pin assignments -----
#define GREEN_BUTTON 1   // Green button on pin 1
#define RED_BUTTON   12  // Red button on pin 12

// External speaker connected to pin 6
#define SPEAKER_PIN 6

// Red tone settings
#define TONE_FREQUENCY_RED 250  // Red tone now uses 250 Hz

// Timing for red tone sequence (state machine)
#define INITIAL_DELAY 200       // Delay (ms) after button press before tone sequence starts
#define RED_TONE_DURATION 1000  // Each red tone plays for 1 second
#define RED_PAUSE_DURATION 500  // Pause 0.5 second between red tone segments
#define RED_CYCLES 3            // Repeat the red tone sequence 3 times

// Variables for edge detection on buttons (initialized as HIGH due to pull-ups)
bool lastGreenState = HIGH;
bool lastRedState   = HIGH;

// State machine variables for red button
enum RedState {IDLE, PLAY_TONE, WAIT};
RedState redState = IDLE;
unsigned long redPreviousMillis = 0;
int redCycleCount = 0;

void setup() {
  CircuitPlayground.begin();
  
  // Configure button pins with internal pull-ups
  pinMode(GREEN_BUTTON, INPUT_PULLUP);
  pinMode(RED_BUTTON, INPUT_PULLUP);
  
  // Configure the speaker pin as output
  pinMode(SPEAKER_PIN, OUTPUT);
}

void loop() {
  unsigned long currentMillis = millis();

  // Read current button states (LOW when pressed)
  bool currentGreen = (digitalRead(GREEN_BUTTON) == LOW);
  bool currentRed   = (digitalRead(RED_BUTTON) == LOW);

  // ----- Green Button Behavior -----
  // On new press of green (only if red isn't pressed), play the "Ayers Rock" melody.
  if (currentGreen && !lastGreenState && !currentRed) {
    setLEDs(0, 255, 0);  // Turn LEDs green
    delay(INITIAL_DELAY); // Brief pause before playing melody
    
    // Play the melody note by note
    for (int i = 0; i < melodyLength; i++) {
      tone(SPEAKER_PIN, topGearMelody[i], topGearDurations[i]);
      delay(topGearDurations[i] * 1.3);  // Slightly longer delay for separation
      noTone(SPEAKER_PIN);
    }
    // Turn off LEDs after the melody finishes
    setLEDs(0, 0, 0);
  }
  
  // ----- Red Button Behavior (State Machine) -----
  // On new press of red (only if green isn't pressed), start red tone sequence.
  if (currentRed && !lastRedState && !currentGreen) {
    redState = PLAY_TONE;
    redPreviousMillis = currentMillis;
    redCycleCount = 0;
    setLEDs(255, 0, 0);  // Turn LEDs red
  }
  
  // Handle the red tone sequence (repeating tone with pauses)
  if (redState != IDLE) {
    switch (redState) {
      case PLAY_TONE:
        if (currentMillis - redPreviousMillis >= INITIAL_DELAY) {
          tone(SPEAKER_PIN, TONE_FREQUENCY_RED); // Start red tone (250 Hz)
          redPreviousMillis = currentMillis;
          redState = WAIT;
        }
        break;
      case WAIT:
        if (currentMillis - redPreviousMillis >= RED_TONE_DURATION) {
          noTone(SPEAKER_PIN); // Stop red tone after 1 second
          redPreviousMillis = currentMillis;
          redCycleCount++;
          if (redCycleCount < RED_CYCLES) {
            delay(RED_PAUSE_DURATION);  // Pause 0.5 second before next cycle
            redState = PLAY_TONE;
          } else {
            redState = IDLE;
            setLEDs(0, 0, 0);  // Turn off LEDs after completing cycles
          }
        }
        break;
    }
  }
  
  // Update last button states for edge detection
  lastGreenState = digitalRead(GREEN_BUTTON);
  lastRedState = digitalRead(RED_BUTTON);
  
  delay(50); // Small delay to stabilize button readings
}

// Helper function to set all onboard NeoPixels to a specific color
void setLEDs(uint8_t red, uint8_t green, uint8_t blue) {
  for (int i = 0; i < 10; i++) {
    CircuitPlayground.setPixelColor(i, red, green, blue);
  }
  CircuitPlayground.strip.show();
}
