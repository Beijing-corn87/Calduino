#include <Wire.h>
#include <LiquidCrystal.h> 

// Nano's I2C Address
#define NANO_SLAVE_ADDRESS 0x08

// LCD Pin Configuration
const int rs = 2, en = 3, d4 = 4, d5 = 5, d6 = 6, d7 = 7;
LiquidCrystal lcd(rs, en, d4, d5, d6, d7);

// Uno's local buttons
const int button_pin_times = 10;
const int button_pin_divide = 11;

// Calculator state variables
char num1_str[17]; 
char num2_str[17]; 
char current_operator = 0; 
int num1_len = 0;
int num2_len = 0;
char result_str[17]; 
char lcd_error_message_char[17]; 

enum CalcState { INPUT_NUM1, INPUT_OPERATOR_AWAIT_NUM2, INPUT_NUM2, DISPLAY_RESULT, DISPLAY_ERROR };
CalcState current_calc_state = INPUT_NUM1; 

int last_button_state_times = LOW;
int last_button_state_divide = LOW;
unsigned long last_uno_debounce_time = 0;
const unsigned long debounce_delay = 50;

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
}

void set_calc_state(CalcState new_state) {
    if (current_calc_state != new_state) {
        Serial.print(F("DEBUG: State changed from '"));
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
        }
        Serial.println(F("'"));
    }
}

void update_lcd() {
  lcd.clear();
  lcd.setCursor(0, 0); 
  lcd.print(num1_str); 
  if (current_operator != 0 && num1_len > 0) {
    lcd.print(current_operator);
    if (num2_len > 0 || current_calc_state == INPUT_NUM2 || current_calc_state == INPUT_OPERATOR_AWAIT_NUM2) {
      lcd.print(num2_str); 
    }
  }
  lcd.setCursor(0, 1); 
  if (current_calc_state == DISPLAY_RESULT) {
    lcd.print("="); lcd.print(result_str); 
  } else if (current_calc_state == DISPLAY_ERROR) {
    lcd.print(lcd_error_message_char); 
  }
}

void setup() {
  Serial.begin(9600); 
  while (!Serial); 
  Serial.println(F("Uno Calduino Starting... (LCD Debug Version + Loading Bar)"));
  
  lcd.begin(16, 2); 
  lcd.print(F("Calduino")); // Project name
  lcd.setCursor(0, 1);    // Move to second line for loading bar
  Serial.println(F("DEBUG: LCD Initialized. Starting loading bar..."));

  // Loading bar
  int total_loading_duration = 4000; // 4 seconds
  int bar_width = 16;                // LCD width
  int delay_per_char = total_loading_duration / bar_width;

  for (int i = 0; i < bar_width; i++) {
    lcd.print("#"); 
    delay(delay_per_char);
  }
  Serial.println(F("DEBUG: Loading bar complete."));
  delay(500); // Short pause after loading bar

  Wire.begin(); 
  Serial.println(F("DEBUG: I2C Initialized."));
  
  pinMode(button_pin_times, INPUT);  
  pinMode(button_pin_divide, INPUT); 
  Serial.println(F("DEBUG: Local button pins initialized."));
  
  reset_calculator(); 
  Serial.println(F("Uno Calduino Ready and Reset."));
  print_current_state_debug(); 
  update_lcd(); 
}

void loop() {
  check_local_buttons();    
  check_nano_input();       
}

void reset_calculator() {
  Serial.println(F("DEBUG: reset_calculator called."));
  num1_str[0] = '\0'; num2_str[0] = '\0'; result_str[0] = '\0';
  num1_len = 0; num2_len = 0; current_operator = 0;
  lcd_error_message_char[0] = '\0'; 
  set_calc_state(INPUT_NUM1);
  update_lcd(); 
  Serial.println(F("DEBUG: Calculator reset complete."));
}

void append_digit(char digit) {
  Serial.print(F("DEBUG: append_digit called with '")); Serial.print(digit); Serial.println(F("'"));
  print_current_state_debug();
  if (current_calc_state == DISPLAY_RESULT || current_calc_state == DISPLAY_ERROR) {
    reset_calculator();
  }
  if (current_calc_state == INPUT_NUM1) {
    if (num1_len < (sizeof(num1_str) - 1)) { 
      num1_str[num1_len++] = digit; num1_str[num1_len] = '\0';
      Serial.print(F("DEBUG: Appended to num1_str: ")); Serial.println(num1_str);
    } else { Serial.println(F("DEBUG: num1_str is full.")); }
  } else if (current_calc_state == INPUT_OPERATOR_AWAIT_NUM2 || current_calc_state == INPUT_NUM2) {
    set_calc_state(INPUT_NUM2); 
    if (num2_len < (sizeof(num2_str) - 1)) { 
      num2_str[num2_len++] = digit; num2_str[num2_len] = '\0';
      Serial.print(F("DEBUG: Appended to num2_str: ")); Serial.println(num2_str);
    } else { Serial.println(F("DEBUG: num2_str is full.")); }
  }
  update_lcd();
}

void set_operator(char op_char) {
   Serial.print(F("DEBUG: set_operator called with '")); Serial.print(op_char); Serial.println(F("'"));
   print_current_state_debug();
  if (current_calc_state == DISPLAY_RESULT || current_calc_state == DISPLAY_ERROR) {
    if (strlen(result_str) > 0 && current_calc_state == DISPLAY_RESULT) {
        strncpy(num1_str, result_str, sizeof(num1_str) -1); num1_str[sizeof(num1_str)-1] = '\0';
        num1_len = strlen(num1_str);
        num2_str[0] = '\0'; num2_len = 0; result_str[0] = '\0'; 
        Serial.println(F("DEBUG: Using previous result as num1."));
    } else {
        reset_calculator(); 
        if (num1_len == 0) { Serial.println(F("DEBUG: set_operator - num1 empty post-reset.")); update_lcd(); return; }
    }
  }
  if (num1_len > 0) { 
      if (current_calc_state == INPUT_NUM2 && num2_len > 0) {
          Serial.println(F("DEBUG: Op with num2. Calculating intermediate..."));
          calculate_result(); 
          if (current_calc_state == DISPLAY_RESULT) { 
              strncpy(num1_str, result_str, sizeof(num1_str)-1); num1_str[sizeof(num1_str)-1] = '\0';
              num1_len = strlen(num1_str);
              num2_str[0] = '\0'; num2_len = 0; result_str[0] = '\0';
          } else { return; }
      }
      current_operator = op_char;
      set_calc_state(INPUT_OPERATOR_AWAIT_NUM2); 
      Serial.print(F("DEBUG: Operator stored: ")); Serial.println(current_operator); 
  } else { Serial.println(F("DEBUG: set_operator - num1_len is 0.")); }
  update_lcd();
}

void do_backspace() {
  Serial.println(F("DEBUG: do_backspace called."));
  print_current_state_debug();
  if (current_calc_state == DISPLAY_RESULT || current_calc_state == DISPLAY_ERROR) {
      reset_calculator(); return;
  }
  if (current_calc_state == INPUT_NUM2 && num2_len > 0) {
    num2_len--; num2_str[num2_len] = '\0'; Serial.print(F("DEBUG: BS num2: ")); Serial.println(num2_str);
  } else if (current_calc_state == INPUT_OPERATOR_AWAIT_NUM2 || (current_calc_state == INPUT_NUM2 && num2_len == 0) ) {
    current_operator = 0; set_calc_state(INPUT_NUM1); Serial.println(F("DEBUG: BS operator."));
  } else if (current_calc_state == INPUT_NUM1 && num1_len > 0) {
    num1_len--; num1_str[num1_len] = '\0'; Serial.print(F("DEBUG: BS num1: ")); Serial.println(num1_str);
     if (num1_len == 0 && current_operator == 0) set_calc_state(INPUT_NUM1);
  } else { Serial.println(F("DEBUG: Backspace - nothing to clear.")); }
  update_lcd();
}

void start_error_sequence(const __FlashStringHelper* fsh_msg) {
    Serial.print(F("DEBUG: start_error_sequence called with: ")); Serial.println(fsh_msg);
    strcpy_P(lcd_error_message_char, (PGM_P)fsh_msg); 
    lcd_error_message_char[sizeof(lcd_error_message_char)-1] = '\0';
    set_calc_state(DISPLAY_ERROR); 
    update_lcd(); 
    delay(2000); 
    lcd_error_message_char[0] = '\0'; 
    reset_calculator(); 
}

void calculate_result() {
  Serial.println(F("DEBUG: calculate_result called."));
  print_current_state_debug();
  lcd_error_message_char[0] = '\0';
  if (num1_len == 0 && current_operator == 0 && num2_len == 0) {
    Serial.println(F("DEBUG: calc_result - Nothing to calc. Result 0."));
    result_str[0] = '0'; result_str[1] = '\0';
    set_calc_state(DISPLAY_RESULT); update_lcd(); return;
  }
  if (num1_len > 0 && current_operator == 0 ) { 
      strncpy(result_str, num1_str, sizeof(result_str)-1); result_str[sizeof(result_str)-1] = '\0';
      Serial.print(F("DEBUG: Result is num1 (no op): '")); Serial.print(result_str); Serial.println(F("'"));
      set_calc_state(DISPLAY_RESULT); update_lcd(); return;
  }
  if (num1_len == 0 || current_operator == 0 ) { 
    Serial.print(F("DEBUG: Error - Incomplete. N1:'")); Serial.print(num1_str); Serial.print(F("',Op:'")); Serial.print(current_operator); Serial.println(F("'"));
    start_error_sequence(F("Incomplete")); return;
  }
   if (num2_len == 0) { 
      Serial.println(F("DEBUG: Error - num2 missing for binary op."));
      start_error_sequence(F("Need num2")); return;
  }
  long val1 = atol(num1_str); long val2 = atol(num2_str);
  long res_val = 0; bool error_flag = false;
  Serial.print(F("DEBUG: Calculating: ")); Serial.print(val1); Serial.print(current_operator); Serial.print(val2); Serial.println();
  switch (current_operator) {
    case '+': res_val = val1 + val2; break;
    case '-': res_val = val1 - val2; break;
    case '*': res_val = val1 * val2; break;
    case '/':
      if (val2 == 0) { error_flag = true; Serial.println(F("DEBUG: Error - Div by zero.")); strcpy_P(lcd_error_message_char, PSTR("Div by Zero"));} 
      else { res_val = val1 / val2; }
      break;
    default: error_flag = true; Serial.print(F("DEBUG: Error - Invalid op: ")); Serial.println(current_operator); strcpy_P(lcd_error_message_char, PSTR("Operator Err")); break;
  }
  if (error_flag) {
    Serial.println(F("DEBUG: Calc error occurred."));
    if (strcmp_P(lcd_error_message_char, PSTR("Div by Zero")) == 0) start_error_sequence(F("Div by Zero"));
    else if (strcmp_P(lcd_error_message_char, PSTR("Operator Err")) == 0) start_error_sequence(F("Operator Err"));
    else start_error_sequence(F("Calc Error"));
  } else {
    ltoa(res_val, result_str, 10); 
    Serial.print(F("DEBUG: Result(L):")); Serial.print(res_val); Serial.print(F(", Str:'")); Serial.print(result_str); Serial.println(F("'"));
    set_calc_state(DISPLAY_RESULT);
    update_lcd();
  }
} 

void check_local_buttons() {
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
  byte bytesReceived = Wire.requestFrom(NANO_SLAVE_ADDRESS, 1); 
  if (bytesReceived > 0) {
    char received_char = Wire.read();
    if (received_char != 0) { 
      Serial.print(F("DEBUG: Uno rcvd I2C: '")); Serial.print(received_char); Serial.println(F("'"));
      last_uno_debounce_time = millis(); 
      if (received_char >= '0' && received_char <= '9') { append_digit(received_char); }
      else {
        switch (received_char) {
          case '+': case '-': set_operator(received_char); break;
          case 'B': do_backspace(); break;
          case '=': Serial.println(F("DEBUG: Equals (=) from Nano.")); calculate_result(); break;
          default: Serial.print(F("DEBUG: Unknown char from Nano: '")); Serial.print(received_char); Serial.println(F("'")); break;
        }
      }
    }
  }
}