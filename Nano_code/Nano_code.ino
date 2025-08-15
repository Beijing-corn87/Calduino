#include <Wire.h>

// Define the I2C slave address for the Nano
#define NANO_SLAVE_ADDRESS 0x08

// Define pins for buttons 0-9
const int button_pins_numbers[10] = {2, 3, 4, 5, 6, 7, 8, 9, 10, 11}; 
const int button_pin_add = 12;      
const int button_pin_subtract = 13; 
const int button_pin_backspace = A0;
const int button_pin_equals = A1;   

const char button_chars_numbers[10] = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9'};
const char char_add = '+';
const char char_subtract = '-';
const char char_backspace = 'B'; 
const char char_equals = '=';

volatile char char_to_send = 0; 

int last_button_states_numbers[10];
int last_button_state_add;
int last_button_state_subtract;
int last_button_state_backspace;
int last_button_state_equals;

unsigned long last_debounce_time = 0;
unsigned long debounce_delay = 50; 

void setup() {
  Serial.begin(9600);
  while (!Serial); 
  Serial.println(F("Nano Keypad Starting (Debug Version - Rel Impr Round)"));

  char_to_send = 0; 

  Wire.begin(NANO_SLAVE_ADDRESS);
  Wire.onRequest(requestEvent); 

  for (int i = 0; i < 10; i++) {
    pinMode(button_pins_numbers[i], INPUT); 
    last_button_states_numbers[i] = LOW;    
  }
  pinMode(button_pin_add, INPUT); last_button_state_add = LOW;
  pinMode(button_pin_subtract, INPUT); last_button_state_subtract = LOW;
  pinMode(button_pin_backspace, INPUT); last_button_state_backspace = LOW;
  pinMode(button_pin_equals, INPUT); last_button_state_equals = LOW;
  Serial.println(F("Nano Pins Initialized. Waiting for button presses..."));
}

void loop() {
  if ((millis() - last_debounce_time) > debounce_delay) {
    bool button_pressed_this_cycle = false;
    char current_char_detected = 0; 

    for (int i = 0; i < 10; i++) {
      int reading = digitalRead(button_pins_numbers[i]);
      if (reading == HIGH && last_button_states_numbers[i] == LOW) { 
        current_char_detected = button_chars_numbers[i];
        button_pressed_this_cycle = true;
      }
      last_button_states_numbers[i] = reading;
    }

    int reading_add = digitalRead(button_pin_add);
    if (reading_add == HIGH && last_button_state_add == LOW) {
      current_char_detected = char_add; button_pressed_this_cycle = true;
    }
    last_button_state_add = reading_add;

    int reading_subtract = digitalRead(button_pin_subtract);
    if (reading_subtract == HIGH && last_button_state_subtract == LOW) {
      current_char_detected = char_subtract; button_pressed_this_cycle = true;
    }
    last_button_state_subtract = reading_subtract;

    int reading_backspace = digitalRead(button_pin_backspace);
    if (reading_backspace == HIGH && last_button_state_backspace == LOW) {
      current_char_detected = char_backspace; button_pressed_this_cycle = true;
    }
    last_button_state_backspace = reading_backspace;

    int reading_equals = digitalRead(button_pin_equals);
    if (reading_equals == HIGH && last_button_state_equals == LOW) {
      current_char_detected = char_equals; button_pressed_this_cycle = true;
    }
    last_button_state_equals = reading_equals;

    if (button_pressed_this_cycle) {
      char_to_send = current_char_detected; 
      Serial.print(F("Nano Button: '")); Serial.print(current_char_detected);
      Serial.println(F("' - Pending I2C send."));
      last_debounce_time = millis(); 
    }
  }
}

void requestEvent() {
  if (char_to_send != 0) {
    Wire.write(char_to_send); 
    Serial.print(F("Nano I2C Sent: '")); Serial.print(char_to_send); Serial.println(F("'"));
    char_to_send = 0; 
  } else {
    Wire.write((byte)0); 
  }
}