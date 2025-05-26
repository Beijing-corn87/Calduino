#include <Wire.h>

#define NANO_SLAVE_ADDRESS 0x08

<<<<<<< Updated upstream
<<<<<<< Updated upstream
<<<<<<< Updated upstream
// Define pins for buttons 0-9
const int button_pins_numbers[10] = {2, 3, 4, 5, 6, 7, 8, 9, 10, 11}; // Pins D2-D11 for 0-9
// Define pins for other buttons
const int button_pin_add = A0;       // D12
const int button_pin_subtract = A1;  // D13
const int button_pin_backspace = 12; // A0
const int button_pin_equals = 13;    // A1
=======
=======
>>>>>>> Stashed changes
=======
>>>>>>> Stashed changes
const int button_pins_numbers[10] = {2, 3, 4, 5, 6, 7, 8, 9, 10, 11};
const int button_pin_add = 12;      
const int button_pin_subtract = 13; 
const int button_pin_backspace = A0;
const int button_pin_equals = A1;   
>>>>>>> Stashed changes

// Characters to send for each button
const char button_chars_numbers[10] = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9'};
const char char_add = '+';
const char char_subtract = '-';
const char char_backspace = 'B'; // 'B' for Backspace
const char char_equals = '=';

volatile char char_to_send = 0;

// Keep track of the last button state to detect new presses
// Buttons are pulled HIGH when pressed, so idle state is LOW.
int last_button_states_numbers[10];
int last_button_state_add;
int last_button_state_subtract;
int last_button_state_backspace;
int last_button_state_equals;

unsigned long last_debounce_time = 0;
unsigned long debounce_delay = 50; // 50ms debounce time

void setup() {
<<<<<<< Updated upstream
<<<<<<< Updated upstream
<<<<<<< Updated upstream
=======
  char_to_send = 0; 
>>>>>>> Stashed changes
=======
  char_to_send = 0; 
>>>>>>> Stashed changes
=======
  char_to_send = 0; 
>>>>>>> Stashed changes
  Wire.begin(NANO_SLAVE_ADDRESS);
  Wire.onRequest(requestEvent);

  // IMPORTANT: For these buttons to work as "pulled HIGH when pressed",
  // you MUST use external pull-down resistors (e.g., 10kOhms) on each button pin,
  // connecting the pin to GND. The button then connects the pin to VCC (5V) when pressed.

  // Initialize number button pins
  for (int i = 0; i < 10; i++) {
    pinMode(button_pins_numbers[i], INPUT); // Set as INPUT
    last_button_states_numbers[i] = LOW;    // Assume buttons are not pressed initially (pin is LOW)
  }
<<<<<<< Updated upstream
  // Initialize other button pins
  pinMode(button_pin_add, INPUT);
  last_button_state_add = LOW;
  pinMode(button_pin_subtract, INPUT);
  last_button_state_subtract = LOW;
  pinMode(button_pin_backspace, INPUT);
  last_button_state_backspace = LOW;
  pinMode(button_pin_equals, INPUT);
  last_button_state_equals = LOW;

  Serial.begin(9600); // Optional: for debugging
=======
  pinMode(button_pin_add, INPUT); last_button_state_add = LOW;
  pinMode(button_pin_subtract, INPUT); last_button_state_subtract = LOW;
  pinMode(button_pin_backspace, INPUT); last_button_state_backspace = LOW;
  pinMode(button_pin_equals, INPUT); last_button_state_equals = LOW;
<<<<<<< Updated upstream
<<<<<<< Updated upstream
>>>>>>> Stashed changes
=======
>>>>>>> Stashed changes
=======
>>>>>>> Stashed changes
}

void loop() {
  if ((millis() - last_debounce_time) > debounce_delay) {
    bool button_pressed_this_cycle = false;

    // Check number buttons
    for (int i = 0; i < 10; i++) {
      int reading = digitalRead(button_pins_numbers[i]);
      if (reading == HIGH && last_button_states_numbers[i] == LOW) { // Button pressed (pin goes HIGH)
        char_to_send = button_chars_numbers[i];
        button_pressed_this_cycle = true;
      }
      last_button_states_numbers[i] = reading;
    }

    // Check Add button
    int reading_add = digitalRead(button_pin_add);
    if (reading_add == HIGH && last_button_state_add == LOW) {
      char_to_send = char_add;
      button_pressed_this_cycle = true;
    }
    last_button_state_add = reading_add;

    // Check Subtract button
    int reading_subtract = digitalRead(button_pin_subtract);
    if (reading_subtract == HIGH && last_button_state_subtract == LOW) {
      char_to_send = char_subtract;
      button_pressed_this_cycle = true;
    }
    last_button_state_subtract = reading_subtract;

    // Check Backspace button
    int reading_backspace = digitalRead(button_pin_backspace);
    if (reading_backspace == HIGH && last_button_state_backspace == LOW) {
      char_to_send = char_backspace;
      button_pressed_this_cycle = true;
    }
    last_button_state_backspace = reading_backspace;

    // Check Equals button
    int reading_equals = digitalRead(button_pin_equals);
    if (reading_equals == HIGH && last_button_state_equals == LOW) {
      char_to_send = char_equals;
      button_pressed_this_cycle = true;
    }
    last_button_state_equals = reading_equals;

    if (button_pressed_this_cycle) {
<<<<<<< Updated upstream
      // Serial.print("Nano sending: "); Serial.println(char_to_send); // Debug
      last_debounce_time = millis(); // Reset debounce timer
=======
      char_to_send = current_char_detected; 
      last_debounce_time = millis(); 
>>>>>>> Stashed changes
    }
  }
}

void requestEvent() {
  if (char_to_send != 0) {
<<<<<<< Updated upstream
    Wire.write(char_to_send);
    // Serial.print("Nano sent: "); Serial.println(char_to_send); // Debug
    char_to_send = 0;
=======
    Wire.write(char_to_send); 
    char_to_send = 0; 
>>>>>>> Stashed changes
  } else {
    Wire.write((byte)0);
  }
}