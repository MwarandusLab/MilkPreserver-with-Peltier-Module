#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define WIRE Wire

const int buttonPin = 10;   // Pin connected to the push button
const int Relay_1 = 8;      // Pin connected to the first Relay
const int Relay_2 = 11;     // Pin connected to the second Relay
int LM35 = A0;

int RelayState1 = LOW;       // Initial state of the first Relay (OFF)
int RelayState2 = HIGH;      // Initial state of the second Relay (ON)

int lastButtonState = LOW;   // Previous state of the push button
unsigned long lastDebounceTime = 0;  // Last time the button state was toggled
unsigned long debounceDelay = 100;    // Debounce time (milliseconds)

unsigned long timerStartTime = 0; // Time when the timer started
unsigned long timerDuration = 120000; // 2 minutes in milliseconds

unsigned long lastTemperatureUpdate = 0; // Time of the last temperature update
unsigned long temperatureUpdateInterval = 1000; // Refresh temperature every 1 second

Adafruit_SSD1306 display(128, 32, &Wire);

void setup() {
  Serial.begin(9600);
  pinMode(buttonPin, INPUT_PULLUP); // Enable internal pull-up resistor on the button pin
  pinMode(Relay_1, OUTPUT);
  pinMode(Relay_2, OUTPUT);

  digitalWrite(Relay_1, RelayState1);  // Initialize Relays
  digitalWrite(Relay_2, RelayState2);

  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
  display.display();
  delay(1000);
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(6, 1);
  display.println("      SYSTEM");
  display.println("\n   INITIALIZATION");
  display.setCursor(0, 0);
  display.display();
  delay(3000);
}

void loop() {
  unsigned long currentMillis = millis();
  int reading = digitalRead(buttonPin);

  // Check if the button state has changed and debounce it
  if (reading != lastButtonState) {
    lastDebounceTime = currentMillis;
  }

  // If the button state has been stable for the debounce delay, update the Relay states
  if (currentMillis - lastDebounceTime >= debounceDelay) {
    if (reading == LOW) {
      RelayState1 = !RelayState1;
      RelayState2 = !RelayState2;

      // If RelayState1 is LOW (Heating), start the timer
      if (RelayState1 == LOW) {
        timerStartTime = currentMillis;
      }
      digitalWrite(Relay_1, RelayState1);
      digitalWrite(Relay_2, RelayState2);
    }
  }

  lastButtonState = reading;

  // Check if the timer is active and elapsed
  if (RelayState1 == LOW && currentMillis - timerStartTime >= timerDuration) {
    // Timer has elapsed, return to Cooling state
    RelayState1 = HIGH;
    RelayState2 = LOW;
    digitalWrite(Relay_1, RelayState1);
    digitalWrite(Relay_2, RelayState2);
  }
    // Update temperature display only once per second
  if (currentMillis - lastTemperatureUpdate >= temperatureUpdateInterval) {
    int sensorValue = analogRead(LM35);
    float voltage = sensorValue * (5.0 / 1024.0);
    float temperatureC = voltage * 100;
    //temperatureC = temperatureC - 470;
    
    // Update the display based on the Relay state
    display.clearDisplay();
    display.setCursor(0, 0);
    display.print("  SMART MILK BOTTLE\n");
    display.println("\nTEMPERATURE: " + String(temperatureC, 1) + " C");
    
    if (RelayState1 == HIGH) {
      display.println("HEATING...");
    } else {
      display.println("COOLING...");
    }
    
    display.display();
    
    lastTemperatureUpdate = currentMillis; // Update the last update time
  }
  
}
