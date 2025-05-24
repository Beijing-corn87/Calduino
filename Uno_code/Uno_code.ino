#include <Wire.h>

// Nano's I2C Address
#define NANO_SLAVE_ADDRESS 0x08

// 7-Segment Display Pins (a, b, c, d, e, f, g, dp) - YOUR CONFIGURATION
// Segment 'a' -> Pin D6
// Segment 'b' -> Pin D8
// Segment 'c' -> Pin D7
// Segment 'd' -> Pin D5
// Segment 'e' -> Pin D3
// Segment 'f' -> Pin D4
// Segment 'g' -> Pin D2
// Segment 'dp'-> Pin D9
const int segment_pins[8] = {6, 8, 7, 5, 3, 4, 2, 9}; 

// Uno's local buttons - YOUR CONFIGURATION (matches previous)
const int button_pin_times = 10;  // D10
const int button_pin_divide = 11; // D11

// 7-Segment patterns for digits 0-9, common cathode (1 = ON, 0 = OFF)
// {a, b, c, d, e, f, g, dp}
const byte digit_patterns[10][8] = {
  {1,1,1,1,1,1,0,0}, // 0
  {0,1,1,0,0,0,0,0}, // 1
  {1,1,0,1,1,0,1,0}, // 2
  {1,1,1,1,0,0,1,0}, // 3
  {0,1,1,0,0,1,1,0}, // 4
  {1,0,1,1,0,1,1,0}, // 5
  {1,0,1,1,1,1,1,0}, // 6
  {1,1,1,0,0,0,0,0}, // 7
  {1,1,1,1,1,1,1,0}, // 8
  {1,1,1,0,0,1,1,0}  // 9
};

// Patterns for other symbols
const byte pattern_minus[8]   = {0,0,0,0,0,0,1,0}; // '-' (g segment)
const byte pattern_error_E[8] = {1,0,0,1,1,1,1,0}; // 'E'
const byte pattern_error_r[8] = {0,0,0,0,1,0,1,0}; // 'r' (lowercase)
const byte pattern_clear[8]   = {0,0,1,1,1,0,0,0}; // 'C' like shape for clear/backspace
const byte pattern_blank[8]   = {0,0,0,0,0,0,0,0}; // All off

// Patterns for operator indication (using a single dedicated segment)
const byte pattern_op_add[8]    = {1,0,0,0,0,0,0,0}; // Segment A for Add
const byte pattern_op_subtract[8]= {0,0,0,0,0,0,1,0}; // Segment G for Subtract (same as minus)
const byte pattern_op_multiply[8]= {0,0,0,1,0,0,0,0}; // Segment D for Multiply
const byte pattern_op_divide[8]  = {0,0,1,0,0,0,0,0}; // Segment C for Divide

// Calculator state variables
char num1_str[12]; 
char num2_str[12]; 
char current_operator = 0; 
int num1_len = 0;
int num2_len = 0;

enum CalcState { INPUT_NUM1, INPUT_OPERATOR_AWAIT_NUM2, INPUT_NUM2, DISPLAY_RESULT, DISPLAY_ERROR };
CalcState current_calc_state = INPUT_NUM1;

// Display timing variables
unsigned long timed_display_end_time = 0;
bool is_timed_display_active = false;
byte current_display_pattern[8]; 

// For result display sequence
char result_str[12];
int result_char_idx = 0;
bool result_display_on_phase = false; 
unsigned long next_result_action_time = 0;

// Debouncing for Uno's local buttons
int last_button_state_times = LOW;
int last_button_state_divide = LOW;
unsigned long last_uno_debounce_time = 0;
const unsigned long debounce_delay = 50;


void setup() {
  Serial.begin(9600); 
  Wire.begin(); 

  for (int i = 0; i < 8; i++) {
    pinMode(segment_pins[i], OUTPUT);
  }
  
  pinMode(button_pin_times, INPUT);  
  pinMode(button_pin_divide, INPUT); 

  reset_calculator(); 
  Serial.println("Uno Calculator Ready");
}

void loop() {
  handle_timed_display();
  check_local_buttons();
  check_nano_input();

  if (current_calc_state == DISPLAY_RESULT) {
    handle_result_display();
  } else if (current_calc_state == DISPLAY_ERROR) {
    // Error display is currently blocking in start_error_sequence.
  }
  
  update_physical_display();
}

// ------------------------
// Display Helper Functions
// ------------------------

void update_physical_display() {
  for (int i = 0; i < 8; i++) {
    digitalWrite(segment_pins[i], current_display_pattern[i] == 1 ? HIGH : LOW);
  }
}

void show_pattern_timed(const byte pattern[8], unsigned long duration_ms) {
  for(int i=0; i<8; i++) current_display_pattern[i] = pattern[i];
  timed_display_end_time = millis() + duration_ms;
  is_timed_display_active = true;
  update_physical_display(); 
}

void handle_timed_display() {
  if (is_timed_display_active && millis() >= timed_display_end_time) {
    for(int i=0; i<8; i++) current_display_pattern[i] = pattern_blank[i]; 
    is_timed_display_active = false;
  }
}

// ----------------------------
// Calculator Logic Functions
// ----------------------------

void reset_calculator() {
  num1_str[0] = '\0';
  num2_str[0] = '\0';
  num1_len = 0;
  num2_len = 0;
  current_operator = 0;
  current_calc_state = INPUT_NUM1;
  is_timed_display_active = false; 
  for(int i=0; i<8; i++) current_display_pattern[i] = pattern_blank[i]; 
  Serial.println("Calculator Reset");
}

void append_digit(char digit) {
  if (is_timed_display_active && current_calc_state != DISPLAY_RESULT && current_calc_state != DISPLAY_ERROR) {
      return;
  }

  if (current_calc_state == INPUT_NUM1) {
    if (num1_len < (sizeof(num1_str) - 1)) {
      num1_str[num1_len++] = digit;
      num1_str[num1_len] = '\0';
      show_pattern_timed(digit_patterns[digit - '0'], 1000);
      Serial.print("Num1: "); Serial.println(num1_str);
    }
  } else if (current_calc_state == INPUT_OPERATOR_AWAIT_NUM2 || current_calc_state == INPUT_NUM2) {
    current_calc_state = INPUT_NUM2; 
    if (num2_len < (sizeof(num2_str) - 1)) {
      num2_str[num2_len++] = digit;
      num2_str[num2_len] = '\0';
      show_pattern_timed(digit_patterns[digit - '0'], 1000);
      Serial.print("Num2: "); Serial.println(num2_str);
    }
  }
}

void set_operator(char op_char) {
   if (is_timed_display_active && current_calc_state != DISPLAY_RESULT && current_calc_state != DISPLAY_ERROR) {
      return;
  }

  if (num1_len > 0) { 
      if (current_calc_state == INPUT_NUM1 && num1_len > 0) {
        current_operator = op_char;
        current_calc_state = INPUT_OPERATOR_AWAIT_NUM2;
        Serial.print("Operator: "); Serial.println(current_operator);
        switch (op_char) {
          case '+': show_pattern_timed(pattern_op_add, 1000); break;
          case '-': show_pattern_timed(pattern_op_subtract, 1000); break;
          case '*': show_pattern_timed(pattern_op_multiply, 1000); break;
          case '/': show_pattern_timed(pattern_op_divide, 1000); break;
        }
      } else if (current_calc_state == INPUT_NUM2 && num2_len > 0) {
        Serial.println("Chained operation - not fully implemented. Operator changed.");
        current_operator = op_char; 
      }
  } else {
      Serial.println("Enter first number before an operator.");
  }
}

void do_backspace() {
  if (is_timed_display_active && current_calc_state != DISPLAY_RESULT && current_calc_state != DISPLAY_ERROR) {
      return;
  }
  show_pattern_timed(pattern_clear, 1000); 
  if (current_calc_state == INPUT_NUM2 && num2_len > 0) {
    num2_len--;
    num2_str[num2_len] = '\0';
    Serial.print("Backspace. Num2: "); Serial.println(num2_str);
  } else if (current_calc_state == INPUT_OPERATOR_AWAIT_NUM2 || (current_calc_state == INPUT_NUM2 && num2_len == 0) ) {
    current_operator = 0;
    current_calc_state = INPUT_NUM1; 
    Serial.println("Backspace. Operator cleared.");
  } else if (current_calc_state == INPUT_NUM1 && num1_len > 0) {
    num1_len--;
    num1_str[num1_len] = '\0';
    Serial.print("Backspace. Num1: "); Serial.println(num1_str);
     if (num1_len == 0) current_calc_state = INPUT_NUM1; 
  }
}

void calculate_result() {
  if (is_timed_display_active && current_calc_state != DISPLAY_RESULT && current_calc_state != DISPLAY_ERROR) {
      return;
  }

  if (num1_len == 0 || num2_len == 0 || current_operator == 0) {
    Serial.println("Error: Incomplete expression");
    start_error_sequence(); 
    return;
  }

  long val1 = atol(num1_str);
  long val2 = atol(num2_str);
  long res_val = 0; 
  bool error_flag = false;

  Serial.print("Calculating: "); Serial.print(val1); Serial.print(current_operator); Serial.print(val2); Serial.println();

  switch (current_operator) {
    case '+': res_val = val1 + val2; break;
    case '-': res_val = val1 - val2; break;
    case '*': res_val = val1 * val2; break;
    case '/':
      if (val2 == 0) {
        error_flag = true;
        Serial.println("Error: Division by zero");
      } else {
        res_val = val1 / val2; 
      }
      break;
    default: error_flag = true; Serial.println("Error: Invalid operator"); break;
  }

  if (error_flag) {
    start_error_sequence();
  } else {
    ltoa(res_val, result_str, 10); 
    Serial.print("Result: "); Serial.println(result_str);
    start_result_display_sequence();
  }
}

// ---------------------------------
// Result and Error Display Sequence
// ---------------------------------

void start_result_display_sequence() {
  current_calc_state = DISPLAY_RESULT;
  result_char_idx = 0;
  result_display_on_phase = true; 
  next_result_action_time = millis(); 
  is_timed_display_active = false; 
}

void handle_result_display() {
  if (millis() >= next_result_action_time) {
    if (result_display_on_phase) { 
      if (result_char_idx < strlen(result_str)) {
        char c = result_str[result_char_idx];
        if (c == '-') {
          for(int i=0; i<8; i++) current_display_pattern[i] = pattern_minus[i];
        } else if (c >= '0' && c <= '9') {
          for(int i=0; i<8; i++) current_display_pattern[i] = digit_patterns[c - '0'][i];
        } else { 
          for(int i=0; i<8; i++) current_display_pattern[i] = pattern_blank[i];
        }
        next_result_action_time = millis() + 1000; 
        result_display_on_phase = false; 
      } else { 
        reset_calculator(); 
      }
    } else { 
      for(int i=0; i<8; i++) current_display_pattern[i] = pattern_blank[i];
      next_result_action_time = millis() + 500; 
      result_display_on_phase = true; 
      result_char_idx++; 
      if (result_char_idx >= strlen(result_str) && strlen(result_str) > 0) {
          // Handled in the next on_phase check
      } else if (strlen(result_str) == 0 && result_char_idx >0 ) { 
          reset_calculator();
      }
    }
  }
}

void start_error_sequence() {
    current_calc_state = DISPLAY_ERROR; 
    is_timed_display_active = false; 

    for(int i=0; i<8; i++) current_display_pattern[i] = pattern_error_E[i];
    update_physical_display(); 
    delay(1000);
    for(int i=0; i<8; i++) current_display_pattern[i] = pattern_blank[i];
    update_physical_display();
    delay(500);

    for(int i=0; i<8; i++) current_display_pattern[i] = pattern_error_r[i];
    update_physical_display();
    delay(1000);
    for(int i=0; i<8; i++) current_display_pattern[i] = pattern_blank[i];
    update_physical_display();
    delay(500);

    for(int i=0; i<8; i++) current_display_pattern[i] = pattern_error_r[i];
    update_physical_display();
    delay(1000);
    reset_calculator(); 
}

// -----------------------------
// Input Handling Functions
// -----------------------------

void check_local_buttons() {
  if (current_calc_state == DISPLAY_RESULT || current_calc_state == DISPLAY_ERROR ) { 
      return; 
  }
   if (is_timed_display_active) return; 

  if ((millis() - last_uno_debounce_time) > debounce_delay) {
    bool pressed_this_cycle = false;
    int reading_times = digitalRead(button_pin_times);
    if (reading_times == HIGH && last_button_state_times == LOW) { 
      set_operator('*');
      pressed_this_cycle = true;
    }
    last_button_state_times = reading_times;

    int reading_divide = digitalRead(button_pin_divide);
    if (reading_divide == HIGH && last_button_state_divide == LOW) { 
      set_operator('/');
      pressed_this_cycle = true;
    }
    last_button_state_divide = reading_divide;
    
    if (pressed_this_cycle) {
        last_uno_debounce_time = millis(); 
    }
  }
}

void check_nano_input() {
  if (current_calc_state == DISPLAY_RESULT || current_calc_state == DISPLAY_ERROR ) { 
    return;
  }
  if (is_timed_display_active) return; 

  Wire.requestFrom(NANO_SLAVE_ADDRESS, 1); 
  if (Wire.available()) {
    char received_char = Wire.read();
    if (received_char != 0) { 
      Serial.print("Uno received: "); Serial.println(received_char);
      last_uno_debounce_time = millis(); 

      if (received_char >= '0' && received_char <= '9') {
        append_digit(received_char);
      } else {
        switch (received_char) {
          case '+':
          case '-':
            set_operator(received_char);
            break;
          case 'B': 
            do_backspace();
            break;
          case '=': 
            if ((current_calc_state == INPUT_NUM2 && num2_len > 0) || (current_calc_state == INPUT_OPERATOR_AWAIT_NUM2 && num2_len > 0) ) { 
                 calculate_result();
            } else {
                 Serial.println("Equals pressed: num2 required.");
            }
            break;
        }
      }
    }
  }
}