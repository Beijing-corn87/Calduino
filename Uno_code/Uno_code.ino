#include <Wire.h>

// Nano's I2C Address
#define NANO_SLAVE_ADDRESS 0x08

// 7-Segment Display Pins (a, b, c, d, e, f, g, dp) - YOUR CONFIGURATION
const int segment_pins[8] = {6, 8, 7, 5, 3, 4, 2, 9}; 

// Uno's local buttons - YOUR CONFIGURATION
const int button_pin_times = 10;
const int button_pin_divide = 11;

// --- PATTERNS (These are const, so they are stored in Flash, not SRAM) ---
const byte digit_patterns[10][8] = {
  {1,1,1,1,1,1,0,0}, {0,1,1,0,0,0,0,0}, {1,1,0,1,1,0,1,0}, {1,1,1,1,0,0,1,0}, {0,1,1,0,0,1,1,0},
  {1,0,1,1,0,1,1,0}, {1,0,1,1,1,1,1,0}, {1,1,1,0,0,0,0,0}, {1,1,1,1,1,1,1,0}, {1,1,1,0,0,1,1,0}
};
const byte pattern_minus[8]   = {0,0,0,0,0,0,1,0};
const byte pattern_error_E[8] = {1,0,0,1,1,1,1,0};
const byte pattern_error_r[8] = {0,0,0,0,1,0,1,0};
const byte pattern_clear[8]   = {0,0,1,1,1,0,0,0};
const byte pattern_blank[8]   = {0,0,0,0,0,0,0,0};
const byte pattern_op_add[8]    = {1,0,0,0,0,0,0,0};
const byte pattern_op_subtract[8]= {0,0,0,0,0,0,1,0};
const byte pattern_op_multiply[8]= {0,0,0,1,0,0,0,0};
const byte pattern_op_divide[8]  = {0,0,1,0,0,0,0,0};
// --- END PATTERNS ---

// Calculator state variables
char num1_str[12]; 
char num2_str[12]; 
char current_operator = 0; 
int num1_len = 0;
int num2_len = 0;

enum CalcState { INPUT_NUM1, INPUT_OPERATOR_AWAIT_NUM2, INPUT_NUM2, DISPLAY_RESULT, DISPLAY_ERROR };
CalcState current_calc_state = INPUT_NUM1; 
<<<<<<< Updated upstream
// For state_names, the strings themselves are in Flash. The array of pointers is in SRAM.
const char* const state_names[] PROGMEM = {"INPUT_NUM1", "INPUT_OPERATOR_AWAIT_NUM2", "INPUT_NUM2", "DISPLAY_RESULT", "DISPLAY_ERROR"};
=======

// Note: state_names array is removed as we'll use a switch for debug printing states
>>>>>>> Stashed changes


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

<<<<<<< Updated upstream
// Helper function to print PROGMEM strings
void printPgmStr(const char* str) {
  char buffer[30]; // Adjust buffer size as needed for the longest state name
  strcpy_P(buffer, str);
  Serial.print(buffer);
}

void print_current_state() {
    Serial.print(F("Current State: "));
    // To print PROGMEM string from an array of PROGMEM strings:
    char buffer[30]; // Make sure buffer is large enough
    strcpy_P(buffer, (char*)pgm_read_ptr(&(state_names[current_calc_state])));
    Serial.println(buffer);
=======
// Function to print current state name using a switch (more robust for debugging)
void print_current_state_debug() {
    Serial.print(F("Current State: "));
    switch (current_calc_state) {
        case INPUT_NUM1: Serial.print(F("INPUT_NUM1")); break;
        case INPUT_OPERATOR_AWAIT_NUM2: Serial.print(F("INPUT_OPERATOR_AWAIT_NUM2")); break;
        case INPUT_NUM2: Serial.print(F("INPUT_NUM2")); break;
        case DISPLAY_RESULT: Serial.print(F("DISPLAY_RESULT")); break;
        case DISPLAY_ERROR: Serial.print(F("DISPLAY_ERROR")); break;
        default: Serial.print(F("UNKNOWN_STATE")); break;
    }
    Serial.println();
>>>>>>> Stashed changes
}

void set_calc_state(CalcState new_state) {
    if (current_calc_state != new_state) {
<<<<<<< Updated upstream
        current_calc_state = new_state;
        Serial.print(F("DEBUG: State changed to -> "));
        char buffer[30];
        strcpy_P(buffer, (char*)pgm_read_ptr(&(state_names[current_calc_state])));
        Serial.println(buffer);
=======
        Serial.print(F("DEBUG: State changed from '"));
        switch (current_calc_state) { // Print old state name
            case INPUT_NUM1: Serial.print(F("INPUT_NUM1")); break;
            case INPUT_OPERATOR_AWAIT_NUM2: Serial.print(F("INPUT_OPERATOR_AWAIT_NUM2")); break;
            case INPUT_NUM2: Serial.print(F("INPUT_NUM2")); break;
            case DISPLAY_RESULT: Serial.print(F("DISPLAY_RESULT")); break;
            case DISPLAY_ERROR: Serial.print(F("DISPLAY_ERROR")); break;
            default: Serial.print(F("UNKNOWN_OLD_STATE")); break;
        }
        Serial.print(F("' to -> '"));
        current_calc_state = new_state; // Update state
        switch (current_calc_state) { // Print new state name
            case INPUT_NUM1: Serial.print(F("INPUT_NUM1")); break;
            case INPUT_OPERATOR_AWAIT_NUM2: Serial.print(F("INPUT_OPERATOR_AWAIT_NUM2")); break;
            case INPUT_NUM2: Serial.print(F("INPUT_NUM2")); break;
            case DISPLAY_RESULT: Serial.print(F("DISPLAY_RESULT")); break;
            case DISPLAY_ERROR: Serial.print(F("DISPLAY_ERROR")); break;
            default: Serial.print(F("UNKNOWN_NEW_STATE")); break;
        }
        Serial.println(F("'"));
>>>>>>> Stashed changes
    }
}

void setup() {
  Serial.begin(9600); 
  while (!Serial); 
<<<<<<< Updated upstream
  Serial.println(F("Uno Calculator Starting..."));
=======
  Serial.println(F("Uno Calculator Starting... (Code Version: May 25 - State Print Fix)"));
>>>>>>> Stashed changes
  Serial.println(F("DEBUG: Initializing Pins and I2C..."));

  Wire.begin(); 

  for (int i = 0; i < 8; i++) {
    pinMode(segment_pins[i], OUTPUT);
  }
  Serial.println(F("DEBUG: Segment pins initialized."));
  
  pinMode(button_pin_times, INPUT);  
  pinMode(button_pin_divide, INPUT); 
  Serial.println(F("DEBUG: Local button pins initialized."));

<<<<<<< Updated upstream
=======
  // Test printing state names using the new switch method (optional, as set_calc_state will use it)
  Serial.println(F("DEBUG: Testing state name printing via set_calc_state:"));
  CalcState test_states[] = {INPUT_NUM1, INPUT_OPERATOR_AWAIT_NUM2, INPUT_NUM2, DISPLAY_RESULT, DISPLAY_ERROR};
  CalcState original_state = current_calc_state; // Save original state
  for (int i = 0; i < 5; i++) {
    set_calc_state(test_states[i]); // This will print the change if different from previous
    current_calc_state = test_states[i]; // Force set for next iteration's "old state"
  }
  set_calc_state(original_state); // Restore original state (will trigger print if different)
  Serial.println(F("DEBUG: State name printing test complete."));


>>>>>>> Stashed changes
  reset_calculator(); 
  Serial.println(F("Uno Calculator Ready and Reset."));
  print_current_state_debug(); // Use new debug print function
}

void loop() {
  handle_timed_display();
<<<<<<< Updated upstream
  check_local_buttons();
  check_nano_input();

  if (current_calc_state == DISPLAY_RESULT) {
    handle_result_display();
=======
  check_local_buttons();    
  check_nano_input();       

  if (current_calc_state == DISPLAY_RESULT) {
    handle_result_display();  
>>>>>>> Stashed changes
  } else if (current_calc_state == DISPLAY_ERROR) {
    // Error display is currently blocking
  }
  
  update_physical_display();
}

void update_physical_display() {
  for (int i = 0; i < 8; i++) {
    digitalWrite(segment_pins[i], current_display_pattern[i] == 1 ? HIGH : LOW);
  }
}

void show_pattern_timed(const byte pattern[8], unsigned long duration_ms) {
  Serial.print(F("DEBUG: show_pattern_timed called. Pattern[0]: ")); Serial.print(pattern[0]); 
  Serial.print(F(", Duration: ")); Serial.println(duration_ms);
  for(int i=0; i<8; i++) current_display_pattern[i] = pattern[i];
  timed_display_end_time = millis() + duration_ms;
  is_timed_display_active = true;
  update_physical_display(); 
}

void handle_timed_display() {
  if (is_timed_display_active && millis() >= timed_display_end_time) {
    Serial.println(F("DEBUG: Timed display ended. Clearing pattern."));
    for(int i=0; i<8; i++) current_display_pattern[i] = pattern_blank[i]; 
    is_timed_display_active = false;
  }
}

void reset_calculator() {
  Serial.println(F("DEBUG: reset_calculator called."));
  num1_str[0] = '\0';
  num2_str[0] = '\0';
  num1_len = 0;
  num2_len = 0;
  current_operator = 0;
<<<<<<< Updated upstream
  set_calc_state(INPUT_NUM1); // This will print the new state
=======
  set_calc_state(INPUT_NUM1); // Will use the new set_calc_state with switch for printing
>>>>>>> Stashed changes
  is_timed_display_active = false; 
  for(int i=0; i<8; i++) current_display_pattern[i] = pattern_blank[i]; 
  Serial.println(F("DEBUG: Calculator reset complete."));
}

void append_digit(char digit) {
  Serial.print(F("DEBUG: append_digit called with '")); Serial.print(digit); Serial.println(F("'"));
<<<<<<< Updated upstream
  // print_current_state(); // State is printed by set_calc_state if it changes
=======
  print_current_state_debug(); // Use new debug print function
>>>>>>> Stashed changes

  if (is_timed_display_active && current_calc_state != DISPLAY_RESULT && current_calc_state != DISPLAY_ERROR) {
      Serial.println(F("DEBUG: append_digit - timed display active, returning."));
      return;
  }

  if (current_calc_state == INPUT_NUM1) {
    if (num1_len < (sizeof(num1_str) - 1)) {
      num1_str[num1_len++] = digit;
      num1_str[num1_len] = '\0';
      Serial.print(F("DEBUG: Appended to num1_str: ")); Serial.println(num1_str);
      show_pattern_timed(digit_patterns[digit - '0'], 1000);
    } else {
      Serial.println(F("DEBUG: num1_str is full."));
    }
  } else if (current_calc_state == INPUT_OPERATOR_AWAIT_NUM2 || current_calc_state == INPUT_NUM2) {
    set_calc_state(INPUT_NUM2); 
    if (num2_len < (sizeof(num2_str) - 1)) {
      num2_str[num2_len++] = digit;
      num2_str[num2_len] = '\0';
      Serial.print(F("DEBUG: Appended to num2_str: ")); Serial.println(num2_str);
      show_pattern_timed(digit_patterns[digit - '0'], 1000);
    } else {
      Serial.println(F("DEBUG: num2_str is full."));
    }
  }
}

void set_operator(char op_char) {
   Serial.print(F("DEBUG: set_operator called with '")); Serial.print(op_char); Serial.println(F("'"));
<<<<<<< Updated upstream
   // print_current_state(); // State is printed by set_calc_state
=======
   print_current_state_debug(); // Use new debug print function
>>>>>>> Stashed changes

   if (is_timed_display_active && current_calc_state != DISPLAY_RESULT && current_calc_state != DISPLAY_ERROR) {
      Serial.println(F("DEBUG: set_operator - timed display active, returning."));
      return;
  }

  if (num1_len > 0) { 
      if (current_calc_state == INPUT_NUM1 && num1_len > 0) {
        current_operator = op_char;
<<<<<<< Updated upstream
        set_calc_state(INPUT_OPERATOR_AWAIT_NUM2);
        Serial.print(F("DEBUG: Operator stored: ")); Serial.println(current_operator);
=======
        set_calc_state(INPUT_OPERATOR_AWAIT_NUM2); 
        Serial.print(F("DEBUG: Operator stored: ")); Serial.println(current_operator); 

        // RESTORED OPERATOR DISPLAY
        Serial.println(F("DEBUG: Attempting to display operator on 7-seg."));
>>>>>>> Stashed changes
        switch (op_char) {
          case '+': show_pattern_timed(pattern_op_add, 1000); break;
          case '-': show_pattern_timed(pattern_op_subtract, 1000); break;
          case '*': show_pattern_timed(pattern_op_multiply, 1000); break;
          case '/': show_pattern_timed(pattern_op_divide, 1000); break;
        }
<<<<<<< Updated upstream
      } else if (current_calc_state == INPUT_NUM2 && num2_len > 0) { // Also allow changing operator if num2 has started
        Serial.println(F("DEBUG: set_operator - Operator changed/set after num2 input started."));
=======
      } else if ( (current_calc_state == INPUT_NUM2 && num2_len > 0) || (current_calc_state == INPUT_OPERATOR_AWAIT_NUM2) ) { 
        Serial.println(F("DEBUG: set_operator - Operator changed/set after num1."));
>>>>>>> Stashed changes
        current_operator = op_char;
        set_calc_state(INPUT_OPERATOR_AWAIT_NUM2); 
        num2_str[0] = '\0'; // Clear num2 as operator changed, expect new num2
        num2_len = 0;
        Serial.print(F("DEBUG: Operator changed to: ")); Serial.println(current_operator);
        Serial.println(F("DEBUG: num2 cleared, awaiting new num2."));
<<<<<<< Updated upstream
         switch (op_char) { 
          case '+': show_pattern_timed(pattern_op_add, 1000); break;
          case '-': show_pattern_timed(pattern_op_subtract, 1000); break;
          case '*': show_pattern_timed(pattern_op_multiply, 1000); break;
          case '/': show_pattern_timed(pattern_op_divide, 1000); break;
        }
      } else if (current_calc_state == INPUT_OPERATOR_AWAIT_NUM2) { // Allow changing operator if already waiting for num2
        Serial.println(F("DEBUG: set_operator - Operator changed while awaiting num2."));
        current_operator = op_char;
        // State remains INPUT_OPERATOR_AWAIT_NUM2
        Serial.print(F("DEBUG: Operator changed to: ")); Serial.println(current_operator);
         switch (op_char) { 
=======
        Serial.println(F("DEBUG: Attempting to display new operator on 7-seg."));
        switch (op_char) { // RESTORED OPERATOR DISPLAY
>>>>>>> Stashed changes
          case '+': show_pattern_timed(pattern_op_add, 1000); break;
          case '-': show_pattern_timed(pattern_op_subtract, 1000); break;
          case '*': show_pattern_timed(pattern_op_multiply, 1000); break;
          case '/': show_pattern_timed(pattern_op_divide, 1000); break;
        }
      } else {
<<<<<<< Updated upstream
        Serial.println(F("DEBUG: set_operator - Condition not met to set operator cleanly. State:"));
        print_current_state();
=======
        Serial.println(F("DEBUG: set_operator - Condition not met to set operator cleanly. Current State:"));
        print_current_state_debug(); 
>>>>>>> Stashed changes
      }
  } else {
      Serial.println(F("DEBUG: set_operator - num1_len is 0. Cannot set operator yet."));
  }
}

void do_backspace() {
  Serial.println(F("DEBUG: do_backspace called."));
<<<<<<< Updated upstream
  // print_current_state(); // State is printed by set_calc_state if it changes

=======
  print_current_state_debug();
>>>>>>> Stashed changes
  if (is_timed_display_active && current_calc_state != DISPLAY_RESULT && current_calc_state != DISPLAY_ERROR) {
      Serial.println(F("DEBUG: do_backspace - timed display active, returning."));
      return;
  }
  show_pattern_timed(pattern_clear, 1000); 
  if (current_calc_state == INPUT_NUM2 && num2_len > 0) {
    num2_len--;
    num2_str[num2_len] = '\0';
    Serial.print(F("DEBUG: Backspace on num2. Num2 is now: '")); Serial.print(num2_str); Serial.println(F("'"));
  } else if (current_calc_state == INPUT_OPERATOR_AWAIT_NUM2 || (current_calc_state == INPUT_NUM2 && num2_len == 0) ) {
    current_operator = 0;
    set_calc_state(INPUT_NUM1); 
    Serial.println(F("DEBUG: Backspace on operator. Operator cleared. Reverted to INPUT_NUM1."));
  } else if (current_calc_state == INPUT_NUM1 && num1_len > 0) {
    num1_len--;
    num1_str[num1_len] = '\0';
    Serial.print(F("DEBUG: Backspace on num1. Num1 is now: '")); Serial.print(num1_str); Serial.println(F("'"));
     if (num1_len == 0) set_calc_state(INPUT_NUM1); 
  } else {
    Serial.println(F("DEBUG: Backspace - nothing to clear."));
  }
}

void calculate_result() {
  Serial.println(F("DEBUG: calculate_result called."));
<<<<<<< Updated upstream
  // print_current_state(); // State changes to DISPLAY_RESULT or DISPLAY_ERROR

  if (is_timed_display_active && current_calc_state != DISPLAY_RESULT && current_calc_state != DISPLAY_ERROR) {
      Serial.println(F("DEBUG: calculate_result - timed display active, returning."));
      return;
  }
=======
  print_current_state_debug();
  if (is_timed_display_active && current_calc_state != DISPLAY_RESULT && current_calc_state != DISPLAY_ERROR) {
      Serial.println(F("DEBUG: calculate_result - timed display active, returning.")); 
      return; 
  } 
>>>>>>> Stashed changes

  if (num1_len == 0 || num2_len == 0 || current_operator == 0) {
    Serial.print(F("DEBUG: Error in calculate_result - Incomplete expression. num1: '")); Serial.print(num1_str);
    Serial.print(F("', op: '")); Serial.print(current_operator); Serial.print(F("', num2: '")); Serial.print(num2_str); Serial.println(F("'"));
    start_error_sequence(); 
    return;
  }

  long val1 = atol(num1_str);
  long val2 = atol(num2_str);
  long res_val = 0; 
  bool error_flag = false;

  Serial.print(F("DEBUG: Calculating: num1_str='")); Serial.print(num1_str); Serial.print(F("' (val1=")); Serial.print(val1);
  Serial.print(F("), op='")); Serial.print(current_operator); Serial.print(F("', num2_str='")); Serial.print(num2_str);
  Serial.print(F("' (val2=")); Serial.print(val2); Serial.println(F(")"));

  switch (current_operator) {
    case '+': res_val = val1 + val2; break;
    case '-': res_val = val1 - val2; break;
    case '*': res_val = val1 * val2; break;
    case '/':
      if (val2 == 0) {
        error_flag = true;
        Serial.println(F("DEBUG: Error - Division by zero."));
      } else {
        res_val = val1 / val2; 
      }
      break;
    default: error_flag = true; Serial.print(F("DEBUG: Error - Invalid operator: ")); Serial.println(current_operator); break;
  }

  if (error_flag) {
    Serial.println(F("DEBUG: Calculation error occurred."));
    start_error_sequence();
  } else {
    ltoa(res_val, result_str, 10); 
    Serial.print(F("DEBUG: Calculation successful. Result (long): ")); Serial.print(res_val);
    Serial.print(F(", Result (string): '")); Serial.print(result_str); Serial.println(F("'"));
    start_result_display_sequence();
  }
<<<<<<< Updated upstream
}
=======
} 
>>>>>>> Stashed changes

void start_result_display_sequence() {
  Serial.println(F("DEBUG: start_result_display_sequence called."));
  set_calc_state(DISPLAY_RESULT);
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
        Serial.println(F("DEBUG: Result display finished."));
        reset_calculator(); 
      }
    } else { 
      for(int i=0; i<8; i++) current_display_pattern[i] = pattern_blank[i];
      next_result_action_time = millis() + 500; 
      result_display_on_phase = true; 
      result_char_idx++; 
      if (strlen(result_str) == 0 && result_char_idx >0 ) { 
          Serial.println(F("DEBUG: Result string was empty, resetting."));
          reset_calculator();
      }
    }
  }
}

void start_error_sequence() {
    Serial.println(F("DEBUG: start_error_sequence called."));
    set_calc_state(DISPLAY_ERROR); 
    is_timed_display_active = false; 

    for(int i=0; i<8; i++) current_display_pattern[i] = pattern_error_E[i];
    update_physical_display(); 
    delay(1000); // Delay is okay in error sequence as it's an exceptional state
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
    Serial.println(F("DEBUG: Error sequence finished. Resetting calculator."));
    reset_calculator(); 
}

void check_local_buttons() {
  if (current_calc_state == DISPLAY_RESULT || current_calc_state == DISPLAY_ERROR ) { 
      return; 
  }
   if (is_timed_display_active) {
      return; 
   }

  if ((millis() - last_uno_debounce_time) > debounce_delay) {
    bool pressed_this_cycle = false;
    int reading_times = digitalRead(button_pin_times);
    if (reading_times == HIGH && last_button_state_times == LOW) { 
      Serial.println(F("DEBUG: Uno Times (*) button pressed."));
      set_operator('*');
      pressed_this_cycle = true;
    }
    last_button_state_times = reading_times;

    int reading_divide = digitalRead(button_pin_divide);
    if (reading_divide == HIGH && last_button_state_divide == LOW) { 
      Serial.println(F("DEBUG: Uno Divide (/) button pressed."));
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
  if (is_timed_display_active) {
    return; 
  }

  byte bytesReceived = Wire.requestFrom(NANO_SLAVE_ADDRESS, 1); 

  if (bytesReceived > 0) {
    char received_char = Wire.read();
    if (received_char != 0) { 
      Serial.print(F("DEBUG: Uno received from Nano via I2C: '")); Serial.print(received_char); Serial.println(F("'"));
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
            Serial.println(F("DEBUG: Equals (=) received from Nano."));
            if ((current_calc_state == INPUT_NUM2 && num2_len > 0) || 
<<<<<<< Updated upstream
                (current_calc_state == INPUT_OPERATOR_AWAIT_NUM2 && num2_len > 0) ) { // Ensure num2 has some content
                 calculate_result();
            } else {
                 Serial.print(F("DEBUG: Equals pressed but conditions not met. State: "));
                 char buffer[30];
                 strcpy_P(buffer, (char*)pgm_read_ptr(&(state_names[current_calc_state])));
                 Serial.print(buffer);
=======
                (current_calc_state == INPUT_OPERATOR_AWAIT_NUM2 && num2_len > 0 && num1_len > 0) ) { 
                 calculate_result();
            } else {
                 Serial.print(F("DEBUG: Equals pressed but conditions not met. State: "));
                 print_current_state_debug(); 
                 Serial.print(F(", num1_len: ")); Serial.print(num1_len);
>>>>>>> Stashed changes
                 Serial.print(F(", num2_len: ")); Serial.println(num2_len);
            }
            break;
          default:
            Serial.print(F("DEBUG: Unknown char received from Nano: '")); Serial.print(received_char); Serial.println(F("'"));
            break;
        }
      }
    }
  }
}