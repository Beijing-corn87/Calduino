#include <Wire.h>

// Define the I2C slave address for the Nano
#define NANO_SLAVE_ADDRESS 0x08

// Define pins for buttons 0-9
const int button_pins_numbers[10] = {2, 3, 4, 5, 6, 7, 8, 9, 10, 11}; // Pins D2-D11 for 0-9
// Define pins for other buttons
const int button_pin_add = 12;       // D12
const int button_pin_subtract = 13;  // D13
const int button_pin_backspace = A0; // A0
const int button_pin_equals = A1;    // A1

// Characters to send for each button
const char button_chars_numbers[10] = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9'};
const char char_add = '+';
const char char_subtract = '-';
const char char_backspace = 'B'; // 'B' for Backspace
const char char_equals = '=';

// Variable to store the character to send, volatile because it's used in ISR context (though requestEvent isn't a true ISR)
volatile char char_to_send = 0; // 0 means no new key pressed

// Keep track of the last button state to detect new presses (simple debounce)
int last_button_states_numbers[10];
int last_button_state_add;
int last_button_state_subtract;
int last_button_state_backspace;
int last_button_state_equals;

unsigned long last_debounce_time = 0;
unsigned long debounce_delay = 50; // 50ms debounce time

void setup() {
  Wire.begin(NANO_SLAVE_ADDRESS);   // Initialize I2C as slave
  Wire.onRequest(requestEvent); // Register event handler for master requests

  // Initialize number button pins
  for (int i = 0; i < 10; i++) {
    pinMode(button_pins_numbers[i], INPUT_PULLUP);
    last_button_states_numbers[i] = HIGH; // Assume buttons are not pressed initially
  }
  // Initialize other button pins
  pinMode(button_pin_add, INPUT_PULLUP);
  last_button_state_add = HIGH;
  pinMode(button_pin_subtract, INPUT_PULLUP);
  last_button_state_subtract = HIGH;
  pinMode(button_pin_backspace, INPUT_PULLUP);
  last_button_state_backspace = HIGH;
  pinMode(button_pin_equals, INPUT_PULLUP);
  last_button_state_equals = HIGH;

  Serial.begin(9600); // Optional: for debugging Nano behavior
}

void loop() {
  // Only check buttons if enough time has passed since the last press
  if ((millis() - last_debounce_time) > debounce_delay) {
    bool button_pressed_this_cycle = false;

    // Check number buttons
    for (int i = 0; i < 10; i++) {
      int reading = digitalRead(button_pins_numbers[i]);
      if (reading == LOW && last_button_states_numbers[i] == HIGH) { // Button pressed
        char_to_send = button_chars_numbers[i];
        Serial.print("Nano sending: "); Serial.println(char_to_send); // Debug
        button_pressed_this_cycle = true;
      }
      last_button_states_numbers[i] = reading;
    }

    // Check Add button
    int reading_add = digitalRead(button_pin_add);
    if (reading_add == LOW && last_button_state_add == HIGH) {
      char_to_send = char_add;
      Serial.print("Nano sending: "); Serial.println(char_to_send); // Debug
      button_pressed_this_cycle = true;
    }
    last_button_state_add = reading_add;

    // Check Subtract button
    int reading_subtract = digitalRead(button_pin_subtract);
    if (reading_subtract == LOW && last_button_state_subtract == HIGH) {
      char_to_send = char_subtract;
      Serial.print("Nano sending: "); Serial.println(char_to_send); // Debug
      button_pressed_this_cycle = true;
    }
    last_button_state_subtract = reading_subtract;

    // Check Backspace button
    int reading_backspace = digitalRead(button_pin_backspace);
    if (reading_backspace == LOW && last_button_state_backspace == HIGH) {
      char_to_send = char_backspace;
      Serial.print("Nano sending: "); Serial.println(char_to_send); // Debug
      button_pressed_this_cycle = true;
    }
    last_button_state_backspace = reading_backspace;

    // Check Equals button
    int reading_equals = digitalRead(button_pin_equals);
    if (reading_equals == LOW && last_button_state_equals == HIGH) {
      char_to_send = char_equals;
      Serial.print("Nano sending: "); Serial.println(char_to_send); // Debug
      button_pressed_this_cycle = true;
    }
    last_button_state_equals = reading_equals;

    if (button_pressed_this_cycle) {
      last_debounce_time = millis(); // Reset debounce timer
    }
  }
}

// This function is called when the I2C master requests data
void requestEvent() {
  if (char_to_send != 0) {
    Wire.write(char_to_send); // Send the character
    Serial.print("Nano sent: "); Serial.println(char_to_send); // Debug
    char_to_send = 0; // Reset after sending
  } else {
    Wire.write((byte)0); // Send a null byte if no new key
  }
}