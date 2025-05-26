#include <Wire.h>

// Nano's I2C Address
#define NANO_SLAVE_ADDRESS 0x08

<<<<<<< Updated upstream
// 7-Segment Display Pins (a, b, c, d, e, f, g, dp) - YOUR CONFIGURATION
const int segment_pins[8] = {6, 8, 7, 5, 3, 4, 2, 9}; 
=======
// LCD Pin Configuration
const int rs = 2, en = 3, d4 = 4, d5 = 5, d6 = 6, d7 = 7;
LiquidCrystal lcd(rs, en, d4, d5, d6, d7);
>>>>>>> Stashed changes

// Uno's local buttons
const int button_pin_times = 10;
const int button_pin_divide = 11;

<<<<<<< Updated upstream
// --- PATTERNS ---
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

char num1_str[12]; 
char num2_str[12]; 
char current_operator = 0; 
int num1_len = 0;
int num2_len = 0;
=======
// Calculator state variables
char num1_str[17]; 
char num2_str[17]; 
char current_operator = 0; 
int num1_len = 0;
int num2_len = 0;
char result_str[17]; 
char lcd_error_message_char[17]; // Using char array for error messages
>>>>>>> Stashed changes

enum CalcState { INPUT_NUM1, INPUT_OPERATOR_AWAIT_NUM2, INPUT_NUM2, DISPLAY_RESULT, DISPLAY_ERROR };
CalcState current_calc_state = INPUT_NUM1; 
const char* const state_names[] PROGMEM = {
  "INPUT_NUM1", "INPUT_OPERATOR_AWAIT_NUM2", "INPUT_NUM2", "DISPLAY_RESULT", "DISPLAY_ERROR"
};

unsigned long timed_display_end_time = 0;
bool is_timed_display_active = false;
byte current_display_pattern[8]; 
char result_str[12];
int result_char_idx = 0;
bool result_display_on_phase = false; 
unsigned long next_result_action_time = 0;
int last_button_state_times = LOW;
int last_button_state_divide = LOW;
unsigned long last_uno_debounce_time = 0;
const unsigned long debounce_delay = 50;

// Helper function to print PROGMEM strings
void printPgmString(PGM_P s) {
  char c;
  if (!s) { Serial.print(F("[NULL_PGM_STR]")); return; }
  uint16_t i = 0;
  byte first_byte = pgm_read_byte(s);
  if (first_byte == 0 || first_byte == 0xFF ) { 
     if (first_byte == 0) Serial.print(F("[EMPTY_PGM_STR]"));
     else Serial.print(F("[BLANK_FLASH_PGM_STR?]"));
     return;
  }
  
  while ((c = pgm_read_byte(s + i))) {
    Serial.print(c);
    i++;
    if (i > 60) { 
      Serial.print(F("[PGM_STR_TOO_LONG]"));
      break;
    }
  }
}

void print_current_state() {
    Serial.print(F("Current State: "));
    if (current_calc_state >= 0 && current_calc_state < (sizeof(state_names) / sizeof(state_names[0]))) {
        PGM_P state_name_ptr = (PGM_P)pgm_read_ptr(&(state_names[current_calc_state]));
        printPgmString(state_name_ptr);
    } else {
        Serial.print(F("[INVALID_STATE_INDEX:")); Serial.print((int)current_calc_state); Serial.print(F("]"));
    }
    Serial.println();
}

void set_calc_state(CalcState new_state) {
    if (current_calc_state != new_state) {
        CalcState old_state = current_calc_state;
        current_calc_state = new_state;
        Serial.print(F("DEBUG: State changed from '"));
<<<<<<< Updated upstream
        if (old_state >= 0 && old_state < (sizeof(state_names) / sizeof(state_names[0]))) {
             PGM_P old_state_name_ptr = (PGM_P)pgm_read_ptr(&(state_names[old_state]));
             printPgmString(old_state_name_ptr);
        } else {
            Serial.print(F("UNKNOWN_OLD_STATE"));
        }
        Serial.print(F("' to -> '"));

        if (new_state == INPUT_OPERATOR_AWAIT_NUM2) { // Diagnostic from previous step
            Serial.print(F("INPUT_OPERATOR_AWAIT_NUM2 (Print logic modified for test)"));
        } else if (new_state >= 0 && new_state < (sizeof(state_names) / sizeof(state_names[0]))) {
            PGM_P new_state_name_ptr = (PGM_P)pgm_read_ptr(&(state_names[new_state]));
            printPgmString(new_state_name_ptr);
        } else {
            Serial.print(F("[INVALID_NEW_STATE_INDEX:")); Serial.print((int)new_state); Serial.print(F("]"));
=======
        switch (current_calc_state) { 
            case INPUT_NUM1: Serial.print(F("INPUT_NUM1")); break;
            case INPUT_OPERATOR_AWAIT_NUM2: Serial.print(F("INPUT_OPERATOR_AWAIT_NUM2")); break;
            case INPUT_NUM2: Serial.print(F("INPUT_NUM2")); break;
            case DISPLAY_RESULT: Serial.print(F("DISPLAY_RESULT")); break;
            case DISPLAY_ERROR: Serial.print(F("DISPLAY_ERROR")); break;
            default: Serial.print(F("UNKNOWN_OLD_STATE")); break;
        }
        Serial.print(F("' to -> '"));
        current_calc_state = new_state; 
        switch (current_calc_state) { 
            case INPUT_NUM1: Serial.print(F("INPUT_NUM1")); break;
            case INPUT_OPERATOR_AWAIT_NUM2: Serial.print(F("INPUT_OPERATOR_AWAIT_NUM2")); break;
            case INPUT_NUM2: Serial.print(F("INPUT_NUM2")); break;
            case DISPLAY_RESULT: Serial.print(F("DISPLAY_RESULT")); break;
            case DISPLAY_ERROR: Serial.print(F("DISPLAY_ERROR")); break;
            default: Serial.print(F("UNKNOWN_NEW_STATE")); break;
>>>>>>> Stashed changes
        }
        Serial.println(F("'"));
    }
}

<<<<<<< Updated upstream
void setup() {
  Serial.begin(9600); 
  while (!Serial); 
  Serial.println(F("Uno Calculator Starting... (Code Version: May 25 - Full Debug)")); // Updated version marker
  Serial.println(F("DEBUG: Initializing Pins and I2C..."));
=======
// Function to update the LCD display using char arrays
void update_lcd() {
  lcd.clear();
  lcd.setCursor(0, 0); 

  // Line 1: Expression
  lcd.print(num1_str); 
  if (current_operator != 0 && num1_len > 0) {
    lcd.print(current_operator);
    if (num2_len > 0 || current_calc_state == INPUT_NUM2 || current_calc_state == INPUT_OPERATOR_AWAIT_NUM2) {
      lcd.print(num2_str); 
    }
  }

  lcd.setCursor(0, 1); 
  if (current_calc_state == DISPLAY_RESULT) {
    lcd.print("=");
    lcd.print(result_str); 
  } else if (current_calc_state == DISPLAY_ERROR) {
    lcd.print(lcd_error_message_char); 
  }
}


void setup() {
  Serial.begin(9600); 
  while (!Serial); 
  Serial.println(F("Uno Calculator Starting... (LCD Debug Version - Reliability Impr.)"));
  
  lcd.begin(16, 2); 
  lcd.print(F("Calculator Boot"));
  Serial.println(F("DEBUG: LCD Initialized."));
>>>>>>> Stashed changes

  Wire.begin(); 

  for (int i = 0; i < 8; i++) {
    pinMode(segment_pins[i], OUTPUT);
  }
  Serial.println(F("DEBUG: Segment pins initialized."));
  
  pinMode(button_pin_times, INPUT);  
  pinMode(button_pin_divide, INPUT); 
  Serial.println(F("DEBUG: Local button pins initialized."));
<<<<<<< Updated upstream

  Serial.println(F("DEBUG: Testing PROGMEM state_names printing at startup:"));
  for (int i = 0; i < (sizeof(state_names) / sizeof(state_names[0])); i++) {
    Serial.print(F("  State [")); Serial.print(i); Serial.print(F("]: '"));
    PGM_P name_ptr = (PGM_P)pgm_read_ptr(&(state_names[i]));
    printPgmString(name_ptr);
    Serial.println(F("'"));
  }
  Serial.println(F("DEBUG: PROGMEM state_names test complete."));

  reset_calculator(); 
  Serial.println(F("Uno Calculator Ready and Reset."));
  print_current_state();
}

void loop() {
  handle_timed_display();
  check_local_buttons();    // These functions are now defined below
  check_nano_input();       // These functions are now defined below

  if (current_calc_state == DISPLAY_RESULT) {
    handle_result_display();  // This function is now defined below
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
  Serial.print(F("DEBUG: show_pattern_timed called. Pattern[0] (seg 'a'): ")); Serial.print(pattern[0]); 
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

=======
  
  reset_calculator(); 
  Serial.println(F("Uno Calculator Ready and Reset."));
  print_current_state_debug(); 
  delay(1000); 
  update_lcd(); 
}

void loop() {
  check_local_buttons();    
  check_nano_input();       
}

>>>>>>> Stashed changes
void reset_calculator() {
  Serial.println(F("DEBUG: reset_calculator called."));
  num1_str[0] = '\0';
  num2_str[0] = '\0';
  num1_len = 0;
  num2_len = 0;
  current_operator = 0;
<<<<<<< Updated upstream
  set_calc_state(INPUT_NUM1);
  is_timed_display_active = false; 
  for(int i=0; i<8; i++) current_display_pattern[i] = pattern_blank[i]; 
=======
  lcd_error_message_char[0] = '\0'; // Clear error message char array
  set_calc_state(INPUT_NUM1);
  update_lcd(); 
>>>>>>> Stashed changes
  Serial.println(F("DEBUG: Calculator reset complete."));
}

void append_digit(char digit) {
  Serial.print(F("DEBUG: append_digit called with '")); Serial.print(digit); Serial.println(F("'"));
  print_current_state();

<<<<<<< Updated upstream
  if (is_timed_display_active && current_calc_state != DISPLAY_RESULT && current_calc_state != DISPLAY_ERROR) {
      Serial.println(F("DEBUG: append_digit - timed display active, returning."));
      return;
  }

  if (current_calc_state == INPUT_NUM1) {
    if (num1_len < (sizeof(num1_str) - 1)) {
=======
  if (current_calc_state == DISPLAY_RESULT || current_calc_state == DISPLAY_ERROR) {
    reset_calculator();
  }

  if (current_calc_state == INPUT_NUM1) {
    if (num1_len < (sizeof(num1_str) - 1)) { // Ensure space for null terminator
>>>>>>> Stashed changes
      num1_str[num1_len++] = digit;
      num1_str[num1_len] = '\0';
      Serial.print(F("DEBUG: Appended to num1_str: ")); Serial.println(num1_str);
      show_pattern_timed(digit_patterns[digit - '0'], 1000);
    } else {
      Serial.println(F("DEBUG: num1_str is full."));
    }
  } else if (current_calc_state == INPUT_OPERATOR_AWAIT_NUM2 || current_calc_state == INPUT_NUM2) {
    set_calc_state(INPUT_NUM2); 
<<<<<<< Updated upstream
    if (num2_len < (sizeof(num2_str) - 1)) {
=======
    if (num2_len < (sizeof(num2_str) - 1)) { // Ensure space for null terminator
>>>>>>> Stashed changes
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
   print_current_state();

<<<<<<< Updated upstream
   if (is_timed_display_active && current_calc_state != DISPLAY_RESULT && current_calc_state != DISPLAY_ERROR) {
      Serial.println(F("DEBUG: set_operator - timed display active, returning."));
      return;
  }

  if (num1_len > 0) { 
      if (current_calc_state == INPUT_NUM1 && num1_len > 0) {
        current_operator = op_char;
        set_calc_state(INPUT_OPERATOR_AWAIT_NUM2); 
        Serial.print(F("DEBUG: Operator stored: ")); Serial.println(current_operator); 
        Serial.println(F("DEBUG: Operator display on 7-seg temporarily SKIPPED for testing."));
      } else if ( (current_calc_state == INPUT_NUM2 && num2_len > 0) || (current_calc_state == INPUT_OPERATOR_AWAIT_NUM2) ) { 
        Serial.println(F("DEBUG: set_operator - Operator changed/set after num1."));
        current_operator = op_char;
        set_calc_state(INPUT_OPERATOR_AWAIT_NUM2); 
        num2_str[0] = '\0'; 
        num2_len = 0;
        Serial.print(F("DEBUG: Operator changed to: ")); Serial.println(current_operator);
        Serial.println(F("DEBUG: num2 cleared, awaiting new num2."));
        Serial.println(F("DEBUG: Operator display on 7-seg temporarily SKIPPED for testing."));
      } else {
        Serial.println(F("DEBUG: set_operator - Condition not met to set operator cleanly. Current State:"));
        print_current_state(); 
=======
  if (current_calc_state == DISPLAY_RESULT || current_calc_state == DISPLAY_ERROR) {
    if (strlen(result_str) > 0 && current_calc_state == DISPLAY_RESULT) {
        strncpy(num1_str, result_str, sizeof(num1_str) -1);
        num1_str[sizeof(num1_str)-1] = '\0';
        num1_len = strlen(num1_str);
        num2_str[0] = '\0';
        num2_len = 0;
        result_str[0] = '\0'; 
        Serial.println(F("DEBUG: Using previous result as num1."));
    } else {
        reset_calculator(); 
        if (num1_len == 0) { 
             Serial.println(F("DEBUG: set_operator - num1 is empty after reset. Cannot set op."));
             update_lcd(); 
             return;
        }
    }
  }

  if (num1_len > 0) { 
      if (current_calc_state == INPUT_NUM2 && num2_len > 0) {
          Serial.println(F("DEBUG: Operator pressed with num2. Calculating intermediate..."));
          calculate_result(); 
          if (current_calc_state == DISPLAY_RESULT) { 
              strncpy(num1_str, result_str, sizeof(num1_str)-1); 
              num1_str[sizeof(num1_str)-1] = '\0';
              num1_len = strlen(num1_str);
              num2_str[0] = '\0';
              num2_len = 0;
              result_str[0] = '\0';
          } else { 
              return; // Error occurred during intermediate calculation
          }
>>>>>>> Stashed changes
      }
  } else {
      Serial.println(F("DEBUG: set_operator - num1_len is 0. Cannot set operator yet."));
  }
}

void do_backspace() {
  Serial.println(F("DEBUG: do_backspace called."));
<<<<<<< Updated upstream
  print_current_state();
  if (is_timed_display_active && current_calc_state != DISPLAY_RESULT && current_calc_state != DISPLAY_ERROR) {
      Serial.println(F("DEBUG: do_backspace - timed display active, returning."));
=======
  print_current_state_debug();

  if (current_calc_state == DISPLAY_RESULT || current_calc_state == DISPLAY_ERROR) {
      reset_calculator(); 
>>>>>>> Stashed changes
      return;
  }
  show_pattern_timed(pattern_clear, 1000); 
  if (current_calc_state == INPUT_NUM2 && num2_len > 0) {
    num2_len--; num2_str[num2_len] = '\0';
    Serial.print(F("DEBUG: BS num2: ")); Serial.println(num2_str);
  } else if (current_calc_state == INPUT_OPERATOR_AWAIT_NUM2 || (current_calc_state == INPUT_NUM2 && num2_len == 0) ) {
<<<<<<< Updated upstream
    current_operator = 0;
    set_calc_state(INPUT_NUM1); 
    Serial.println(F("DEBUG: Backspace on operator. Operator cleared. Reverted to INPUT_NUM1."));
=======
    current_operator = 0; set_calc_state(INPUT_NUM1); 
    Serial.println(F("DEBUG: BS operator."));
>>>>>>> Stashed changes
  } else if (current_calc_state == INPUT_NUM1 && num1_len > 0) {
    num1_len--; num1_str[num1_len] = '\0';
    Serial.print(F("DEBUG: BS num1: ")); Serial.println(num1_str);
     if (num1_len == 0 && current_operator == 0) set_calc_state(INPUT_NUM1);
  } else {
    Serial.println(F("DEBUG: Backspace - nothing to clear."));
  }
<<<<<<< Updated upstream
=======
  update_lcd();
}

// Error messages are now copied from PROGMEM using F() into a global char array
void start_error_sequence(const __FlashStringHelper* fsh_msg) {
    Serial.println(F("DEBUG: start_error_sequence called."));
    strcpy_P(lcd_error_message_char, (PGM_P)fsh_msg); 
    // Ensure null termination if fsh_msg is longer than buffer - 1
    // strcpy_P should handle this if source is null terminated. Max length is 16 for display.
    // For safety, we can truncate, though lcd.print will also truncate.
    lcd_error_message_char[sizeof(lcd_error_message_char)-1] = '\0';

    set_calc_state(DISPLAY_ERROR); 
    update_lcd(); 
    delay(2000); 
    lcd_error_message_char[0] = '\0'; // Clear message
    reset_calculator(); 
>>>>>>> Stashed changes
}

void calculate_result() {
  Serial.println(F("DEBUG: calculate_result called."));
<<<<<<< Updated upstream
  print_current_state();
  if (is_timed_display_active && current_calc_state != DISPLAY_RESULT && current_calc_state != DISPLAY_ERROR) {
      Serial.println(F("DEBUG: calculate_result - timed display active, returning.")); // This was the line from the error
      return; // Ensure return is here if the condition is met
  } // Make sure this '}' matches the 'if' above

  if (num1_len == 0 || num2_len == 0 || current_operator == 0) {
    Serial.print(F("DEBUG: Error in calculate_result - Incomplete. num1: '")); Serial.print(num1_str);
    Serial.print(F("', op: '")); Serial.print(current_operator); Serial.print(F("', num2: '")); Serial.print(num2_str); Serial.println(F("'"));
    start_error_sequence(); 
    return;
  }

  long val1 = atol(num1_str);
  long val2 = atol(num2_str);
  long res_val = 0; 
  bool error_flag = false;
=======
  print_current_state_debug();
  
  lcd_error_message_char[0] = '\0'; // Clear any previous specific error message

  if (num1_len == 0 && current_operator == 0 && num2_len == 0) {
    Serial.println(F("DEBUG: calculate_result - Nothing to calculate. Result is 0."));
    result_str[0] = '0'; result_str[1] = '\0';
    set_calc_state(DISPLAY_RESULT); update_lcd(); return;
  }
  if (num1_len > 0 && current_operator == 0 ) { 
      strncpy(result_str, num1_str, sizeof(result_str)-1);
      result_str[sizeof(result_str)-1] = '\0';
      Serial.print(F("DEBUG: Result is num1 (no op): '")); Serial.print(result_str); Serial.println(F("'"));
      set_calc_state(DISPLAY_RESULT); update_lcd(); return;
  }
  if (num1_len == 0 || current_operator == 0 ) { // num2_len check removed to allow unary type ops if defined later, but for now binary needs op
    Serial.print(F("DEBUG: Error - Incomplete. N1:'")); Serial.print(num1_str); Serial.print(F("',Op:'")); Serial.print(current_operator); Serial.println(F("'"));
    start_error_sequence(F("Incomplete")); return;
  }
   if (num2_len == 0) { // For binary operations, num2 is essential
      Serial.println(F("DEBUG: Error in calculate_result - num2 is missing for binary operation."));
      start_error_sequence(F("Need num2")); return;
  }

  long val1 = atol(num1_str); long val2 = atol(num2_str);
  long res_val = 0; bool error_flag = false;
>>>>>>> Stashed changes

  Serial.print(F("DEBUG: Calculating: ")); Serial.print(val1); Serial.print(current_operator); Serial.print(val2); Serial.println();

  switch (current_operator) {
    case '+': res_val = val1 + val2; break;
    case '-': res_val = val1 - val2; break;
    case '*': res_val = val1 * val2; break;
    case '/':
<<<<<<< Updated upstream
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
    Serial.print(F("DEBUG: Result (long): ")); Serial.print(res_val);
    Serial.print(F(", Result (str): '")); Serial.print(result_str); Serial.println(F("'"));
    start_result_display_sequence();
=======
      if (val2 == 0) { error_flag = true; Serial.println(F("DEBUG: Error - Div by zero.")); strcpy_P(lcd_error_message_char, PSTR("Div by Zero"));} 
      else { res_val = val1 / val2; }
      break;
    default: error_flag = true; Serial.print(F("DEBUG: Error - Invalid op: ")); Serial.println(current_operator); strcpy_P(lcd_error_message_char, PSTR("Operator Err")); break;
  }

  if (error_flag) {
    Serial.println(F("DEBUG: Calc error occurred."));
    // If lcd_error_message_char was set by specific error, use it, else default.
    start_error_sequence(strlen(lcd_error_message_char) > 0 ? (const __FlashStringHelper*)lcd_error_message_char : F("Calc Error")); 
    // Note: Casting lcd_error_message_char to F() is not right here if it's already in SRAM.
    // Better:
    // start_error_sequence_sram(lcd_error_message_char); // if we make a char* version
    // Or simply, ensure start_error_sequence uses its default if lcd_error_message_char is empty
    if (strlen(lcd_error_message_char) > 0) { // If a specific message was set
        // Need to convert char[] to __FlashStringHelper* if start_error_sequence expects it
        // This is tricky. Simpler to have start_error_sequence take char* or use its default.
        // For now, let's make sure default is "Calc Error" and specific errors set lcd_error_message_char
        // and start_error_sequence displays lcd_error_message_char if not empty.
        // The call below will be tricky. Let's refine start_error_sequence or the call.
         char temp_err_msg[17]; // Create a temporary SRAM copy for the function if it expects Flash string.
         strcpy(temp_err_msg, lcd_error_message_char);
         // This is getting complicated. Let's simplify the error sequence call.
         // The current start_error_sequence takes F(), so we must pass F().
         if (strcmp_P(lcd_error_message_char, PSTR("Div by Zero")) == 0) start_error_sequence(F("Div by Zero"));
         else if (strcmp_P(lcd_error_message_char, PSTR("Operator Err")) == 0) start_error_sequence(F("Operator Err"));
         else start_error_sequence(F("Calc Error")); // Default if not set or unknown
    } else { // Error flag true, but no specific message set in lcd_error_message_char
         start_error_sequence(F("Calc Error"));
    }

  } else {
    ltoa(res_val, result_str, 10); 
    Serial.print(F("DEBUG: Result(L):")); Serial.print(res_val); Serial.print(F(", Str:'")); Serial.print(result_str); Serial.println(F("'"));
    set_calc_state(DISPLAY_RESULT);
    update_lcd();
>>>>>>> Stashed changes
  }
} // This is the closing brace for calculate_result

<<<<<<< Updated upstream
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

=======
void check_local_buttons() {
>>>>>>> Stashed changes
  if ((millis() - last_uno_debounce_time) > debounce_delay) {
    bool pressed_this_cycle = false;
    if (digitalRead(button_pin_times) == HIGH && last_button_state_times == LOW) { 
      Serial.println(F("DEBUG: Uno Times (*) button pressed."));
      set_operator('*'); pressed_this_cycle = true;
    }
    last_button_state_times = digitalRead(button_pin_times);
    if (digitalRead(button_pin_divide) == HIGH && last_button_state_divide == LOW) { 
      Serial.println(F("DEBUG: Uno Divide (/) button pressed."));
      set_operator('/'); pressed_this_cycle = true;
    }
    last_button_state_divide = digitalRead(button_pin_divide);
    if (pressed_this_cycle) { last_uno_debounce_time = millis(); }
  }
}

void check_nano_input() {
<<<<<<< Updated upstream
  if (current_calc_state == DISPLAY_RESULT || current_calc_state == DISPLAY_ERROR ) { 
    return;
  }
  if (is_timed_display_active) {
    return; 
  }

=======
>>>>>>> Stashed changes
  byte bytesReceived = Wire.requestFrom(NANO_SLAVE_ADDRESS, 1); 
  if (bytesReceived > 0) {
    char received_char = Wire.read();
    if (received_char != 0) { 
<<<<<<< Updated upstream
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
                (current_calc_state == INPUT_OPERATOR_AWAIT_NUM2 && num2_len > 0 && num1_len > 0) ) { // Ensure num1 also has content for op await
                 calculate_result();
            } else {
                 Serial.print(F("DEBUG: Equals pressed but conditions not met. State: "));
                 print_current_state(); 
                 Serial.print(F(", num1_len: ")); Serial.print(num1_len);
                 Serial.print(F(", num2_len: ")); Serial.println(num2_len);
            }
            break;
          default:
            Serial.print(F("DEBUG: Unknown char received from Nano: '")); Serial.print(received_char); Serial.println(F("'"));
            break;
=======
      Serial.print(F("DEBUG: Uno rcvd I2C: '")); Serial.print(received_char); Serial.println(F("'"));
      last_uno_debounce_time = millis(); 
      if (received_char >= '0' && received_char <= '9') { append_digit(received_char); }
      else {
        switch (received_char) {
          case '+': case '-': set_operator(received_char); break;
          case 'B': do_backspace(); break;
          case '=': Serial.println(F("DEBUG: Equals (=) from Nano.")); calculate_result(); break;
          default: Serial.print(F("DEBUG: Unknown char from Nano: '")); Serial.print(received_char); Serial.println(F("'")); break;
>>>>>>> Stashed changes
        }
      }
    }
  }
}