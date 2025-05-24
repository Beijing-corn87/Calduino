// NANO - SIMPLIFIED BUTTON TEST SKETCH

// Define pins for buttons 0-9
const int button_pins_numbers[10] = {2, 3, 4, 5, 6, 7, 8, 9, 10, 11}; // Pins D2-D11 for 0-9
// Define pins for other buttons
const int button_pin_add = 12;       // D12
const int button_pin_subtract = 13;  // D13
const int button_pin_backspace = A0; // A0
const int button_pin_equals = A1;    // A1

// Characters for each button (for printing)
const char button_chars_numbers[10] = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9'};
const char char_add = '+';
const char char_subtract = '-';
const char char_backspace = 'B';
const char char_equals = '=';

// Keep track of the last button state to detect new presses
int last_button_states_numbers[10];
int last_button_state_add;
int last_button_state_subtract;
int last_button_state_backspace;
int last_button_state_equals;

unsigned long last_debounce_time = 0;
unsigned long debounce_delay = 50; // 50ms debounce time

void setup() {
  Serial.begin(9600);
  while (!Serial); // Wait for Serial port to connect (important for some Arduinos)
  Serial.println("Nano Simplified Button Test Started.");
  Serial.println("Ensure pull-down resistors are used. Button press should make pin HIGH.");

  // Initialize number button pins
  for (int i = 0; i < 10; i++) {
    pinMode(button_pins_numbers[i], INPUT); // Set as INPUT (requires external pull-down)
    last_button_states_numbers[i] = LOW;    // Assume buttons are not pressed initially (pin is LOW)
    Serial.print("Number Button Pin "); Serial.print(button_pins_numbers[i]); Serial.println(" initialized.");
  }
  // Initialize other button pins
  pinMode(button_pin_add, INPUT);
  last_button_state_add = LOW;
  Serial.print("Add Button Pin "); Serial.print(button_pin_add); Serial.println(" initialized.");

  pinMode(button_pin_subtract, INPUT);
  last_button_state_subtract = LOW;
  Serial.print("Subtract Button Pin "); Serial.print(button_pin_subtract); Serial.println(" initialized.");
  
  pinMode(button_pin_backspace, INPUT);
  last_button_state_backspace = LOW;
  Serial.print("Backspace Button Pin "); Serial.print(button_pin_backspace); Serial.println(" initialized.");

  pinMode(button_pin_equals, INPUT);
  last_button_state_equals = LOW;
  Serial.print("Equals Button Pin "); Serial.print(button_pin_equals); Serial.println(" initialized.");
  
  Serial.println("--- Waiting for button presses ---");
}

void loop() {
  if ((millis() - last_debounce_time) > debounce_delay) {
    bool button_pressed_this_cycle = false;
    char pressed_key_char = ' ';

    // Check number buttons
    for (int i = 0; i < 10; i++) {
      int reading = digitalRead(button_pins_numbers[i]);
      if (reading == HIGH && last_button_states_numbers[i] == LOW) { // Button pressed (pin goes HIGH)
        pressed_key_char = button_chars_numbers[i];
        button_pressed_this_cycle = true;
      }
      last_button_states_numbers[i] = reading;
    }

    // Check Add button
    int reading_add = digitalRead(button_pin_add);
    if (reading_add == HIGH && last_button_state_add == LOW) {
      pressed_key_char = char_add;
      button_pressed_this_cycle = true;
    }
    last_button_state_add = reading_add;

    // Check Subtract button
    int reading_subtract = digitalRead(button_pin_subtract);
    if (reading_subtract == HIGH && last_button_state_subtract == LOW) {
      pressed_key_char = char_subtract;
      button_pressed_this_cycle = true;
    }
    last_button_state_subtract = reading_subtract;

    // Check Backspace button
    int reading_backspace = digitalRead(button_pin_backspace);
    if (reading_backspace == HIGH && last_button_state_backspace == LOW) {
      pressed_key_char = char_backspace;
      button_pressed_this_cycle = true;
    }
    last_button_state_backspace = reading_backspace;

    // Check Equals button
    int reading_equals = digitalRead(button_pin_equals);
    if (reading_equals == HIGH && last_button_state_equals == LOW) {
      pressed_key_char = char_equals;
      button_pressed_this_cycle = true;
    }
    last_button_state_equals = reading_equals;

    if (button_pressed_this_cycle) {
      Serial.print("Nano Detected Button Press: "); 
      Serial.println(pressed_key_char);
      last_debounce_time = millis(); // Reset debounce timer
    }
  }
}