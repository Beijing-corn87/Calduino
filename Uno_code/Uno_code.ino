#include <Wire.h>
#include <LiquidCrystal.h> // Include the LCD library

// Nano's I2C Address
#define NANO_SLAVE_ADDRESS 0x08

// LCD Pin Configuration (as per your specification)
// RS: D2, E: D3, D4: D4, D5: D5, D6: D6, D7: D7
const int rs = 2, en = 3, d4 = 4, d5 = 5, d6 = 6, d7 = 7;
LiquidCrystal lcd(rs, en, d4, d5, d6, d7);

// Uno's local buttons - YOUR CONFIGURATION
const int button_pin_times = 10;
const int button_pin_divide = 11;

// Calculator state variables
char num1_str[17]; // Increased size slightly for potential full line on LCD + null
char num2_str[17]; 
char current_operator = 0; 
int num1_len = 0;
int num2_len = 0;
char result_str[17]; // For the result
String lcd_error_message = ""; // For displaying errors on LCD

enum CalcState { INPUT_NUM1, INPUT_OPERATOR_AWAIT_NUM2, INPUT_NUM2, DISPLAY_RESULT, DISPLAY_ERROR };
CalcState current_calc_state = INPUT_NUM1; 

// Debouncing for Uno's local buttons
int last_button_state_times = LOW;
int last_button_state_divide = LOW;
unsigned long last_uno_debounce_time = 0;
const unsigned long debounce_delay = 50;

// Function to print current state name using a switch (for Serial debugging)
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
    }
}

// Function to update the LCD display
void update_lcd() {
  lcd.clear();
  lcd.setCursor(0, 0); // Set cursor to the beginning of the first line

  String line1 = "";
  line1 += num1_str;

  if (current_operator != 0 && num1_len > 0) {
    line1 += current_operator;
    // Only show num2 if it has content or we are actively inputting it (or expecting it)
    if (num2_len > 0 || current_calc_state == INPUT_NUM2 || current_calc_state == INPUT_OPERATOR_AWAIT_NUM2) {
      line1 += num2_str;
    }
  }
  lcd.print(line1.substring(0, 16)); // Print first 16 chars of expression

  lcd.setCursor(0, 1); // Set cursor to the beginning of the second line
  if (current_calc_state == DISPLAY_RESULT) {
    String line2 = "=";
    line2 += result_str;
    lcd.print(line2.substring(0, 16));
  } else if (current_calc_state == DISPLAY_ERROR) {
    lcd.print(lcd_error_message.substring(0, 16));
  }
  // If just inputting, line 2 remains blank
}


void setup() {
  Serial.begin(9600); 
  while (!Serial); 
  Serial.println(F("Uno Calculator Starting... (LCD Version)"));
  
  // Initialize the LCD
  lcd.begin(16, 2); // 16 columns, 2 rows
  lcd.print(F("Calculator Boot"));
  Serial.println(F("DEBUG: LCD Initialized."));

  Wire.begin(); 
  Serial.println(F("DEBUG: I2C Initialized."));
  
  pinMode(button_pin_times, INPUT);  
  pinMode(button_pin_divide, INPUT); 
  Serial.println(F("DEBUG: Local button pins initialized."));

  reset_calculator(); 
  Serial.println(F("Uno Calculator Ready and Reset."));
  print_current_state_debug();
  delay(1000); // Show boot message for a bit
  update_lcd(); // Initial display
}

void loop() {
  check_local_buttons();    
  check_nano_input();       

  // No timed display handling needed for LCD in the same way as 7-seg
  // LCD is updated directly by action functions
}


void reset_calculator() {
  Serial.println(F("DEBUG: reset_calculator called."));
  num1_str[0] = '\0';
  num2_str[0] = '\0';
  result_str[0] = '\0';
  num1_len = 0;
  num2_len = 0;
  current_operator = 0;
  lcd_error_message = "";
  set_calc_state(INPUT_NUM1);
  update_lcd(); // Update display after reset
  Serial.println(F("DEBUG: Calculator reset complete."));
}

void append_digit(char digit) {
  Serial.print(F("DEBUG: append_digit called with '")); Serial.print(digit); Serial.println(F("'"));
  print_current_state_debug();

  if (current_calc_state == DISPLAY_RESULT || current_calc_state == DISPLAY_ERROR) {
    // If result or error is shown, first key press should clear for new input
    reset_calculator();
  }

  if (current_calc_state == INPUT_NUM1) {
    if (num1_len < (sizeof(num1_str) - 2)) { // Leave space for null
      num1_str[num1_len++] = digit;
      num1_str[num1_len] = '\0';
      Serial.print(F("DEBUG: Appended to num1_str: ")); Serial.println(num1_str);
    } else {
      Serial.println(F("DEBUG: num1_str is full."));
    }
  } else if (current_calc_state == INPUT_OPERATOR_AWAIT_NUM2 || current_calc_state == INPUT_NUM2) {
    set_calc_state(INPUT_NUM2); 
    if (num2_len < (sizeof(num2_str) - 2)) { // Leave space for null
      num2_str[num2_len++] = digit;
      num2_str[num2_len] = '\0';
      Serial.print(F("DEBUG: Appended to num2_str: ")); Serial.println(num2_str);
    } else {
      Serial.println(F("DEBUG: num2_str is full."));
    }
  }
  update_lcd();
}

void set_operator(char op_char) {
   Serial.print(F("DEBUG: set_operator called with '")); Serial.print(op_char); Serial.println(F("'"));
   print_current_state_debug();

  if (current_calc_state == DISPLAY_RESULT || current_calc_state == DISPLAY_ERROR) {
    // If result or error shown, and an operator is pressed,
    // use the previous result as num1.
    if (strlen(result_str) > 0 && current_calc_state == DISPLAY_RESULT) {
        strcpy(num1_str, result_str);
        num1_len = strlen(num1_str);
        num2_str[0] = '\0';
        num2_len = 0;
        result_str[0] = '\0'; // Clear old result
        Serial.println(F("DEBUG: Using previous result as num1."));
    } else {
        reset_calculator(); // Or just reset if no valid result to use
        // If resetting, we need num1 before an operator
        if (num1_len == 0) { // Check after reset if num1 is still empty
             Serial.println(F("DEBUG: set_operator - num1 is empty after potential reset. Cannot set operator."));
             update_lcd(); // Show the cleared state
             return;
        }
    }
  }


  if (num1_len > 0) { 
      // Allow changing operator if num2 hasn't started, or if num2 has started allow calculation then new op.
      // For simplicity now, if an operator is pressed and we are in INPUT_NUM2, calculate first.
      if (current_calc_state == INPUT_NUM2 && num2_len > 0) {
          Serial.println(F("DEBUG: Operator pressed with num2 present. Calculating intermediate result..."));
          calculate_result(); // Calculate current expression
          if (current_calc_state == DISPLAY_RESULT) { // if calculation was successful
              strcpy(num1_str, result_str); // Use result as new num1
              num1_len = strlen(num1_str);
              num2_str[0] = '\0';
              num2_len = 0;
              result_str[0] = '\0';
          } else { // Error during calculation, don't proceed with new operator
              return;
          }
      }
      current_operator = op_char;
      set_calc_state(INPUT_OPERATOR_AWAIT_NUM2); 
      Serial.print(F("DEBUG: Operator stored: ")); Serial.println(current_operator); 
  } else {
      Serial.println(F("DEBUG: set_operator - num1_len is 0. Cannot set operator yet."));
  }
  update_lcd();
}

void do_backspace() {
  Serial.println(F("DEBUG: do_backspace called."));
  print_current_state_debug();

  if (current_calc_state == DISPLAY_RESULT || current_calc_state == DISPLAY_ERROR) {
      reset_calculator(); // Backspace from result/error screen clears all
      return;
  }

  if (current_calc_state == INPUT_NUM2 && num2_len > 0) {
    num2_len--;
    num2_str[num2_len] = '\0';
    Serial.print(F("DEBUG: Backspace on num2. Num2 is now: '")); Serial.print(num2_str); Serial.println(F("'"));
  } else if (current_calc_state == INPUT_OPERATOR_AWAIT_NUM2 || (current_calc_state == INPUT_NUM2 && num2_len == 0) ) {
    current_operator = 0;
    set_calc_state(INPUT_NUM1); // Revert to inputting num1 or its operator
    Serial.println(F("DEBUG: Backspace on operator. Operator cleared."));
  } else if (current_calc_state == INPUT_NUM1 && num1_len > 0) {
    num1_len--;
    num1_str[num1_len] = '\0';
    Serial.print(F("DEBUG: Backspace on num1. Num1 is now: '")); Serial.print(num1_str); Serial.println(F("'"));
     if (num1_len == 0) set_calc_state(INPUT_NUM1); 
  } else {
    Serial.println(F("DEBUG: Backspace - nothing to clear."));
  }
  update_lcd();
}

void start_error_sequence(String msg = "Error!") {
    Serial.println(F("DEBUG: start_error_sequence called."));
    lcd_error_message = msg;
    set_calc_state(DISPLAY_ERROR); 
    update_lcd(); 
    delay(2000); // Show error for 2 seconds
    lcd_error_message = ""; 
    reset_calculator(); 
}

void calculate_result() {
  Serial.println(F("DEBUG: calculate_result called."));
  print_current_state_debug();

  if (num1_len == 0 || current_operator == 0) { // num2 might be empty if using previous result in some chained ops, but usually needed
    if (num1_len > 0 && current_operator == 0) { // Case: "5=" (treat as result is 5)
        strcpy(result_str, num1_str);
        Serial.print(F("DEBUG: Result is num1 as no operator/num2. Result: '")); Serial.print(result_str); Serial.println(F("'"));
        set_calc_state(DISPLAY_RESULT);
        update_lcd();
        return;
    }
    Serial.print(F("DEBUG: Error in calculate_result - Incomplete. num1: '")); Serial.print(num1_str);
    Serial.print(F("', op: '")); Serial.print(current_operator); Serial.print(F("', num2: '")); Serial.print(num2_str); Serial.println(F("'"));
    start_error_sequence(F("Incomplete Expr")); 
    return;
  }
  // If operator is present, num2 should also be present for binary operation
  if (num2_len == 0) {
      Serial.println(F("DEBUG: Error in calculate_result - num2 is missing for binary operation."));
      start_error_sequence(F("Need num2"));
      return;
  }


  long val1 = atol(num1_str);
  long val2 = atol(num2_str);
  long res_val = 0; 
  bool error_flag = false;

  Serial.print(F("DEBUG: Calculating: ")); Serial.print(val1); Serial.print(current_operator); Serial.print(val2); Serial.println();

  switch (current_operator) {
    case '+': res_val = val1 + val2; break;
    case '-': res_val = val1 - val2; break;
    case '*': res_val = val1 * val2; break;
    case '/':
      if (val2 == 0) {
        error_flag = true;
        Serial.println(F("DEBUG: Error - Division by zero."));
        lcd_error_message = F("Div by Zero"); // Specific error message
      } else {
        res_val = val1 / val2; 
      }
      break;
    default: error_flag = true; Serial.print(F("DEBUG: Error - Invalid operator: ")); Serial.println(current_operator); lcd_error_message = F("Operator Err"); break;
  }

  if (error_flag) {
    Serial.println(F("DEBUG: Calculation error occurred."));
    start_error_sequence(lcd_error_message); // Pass specific error message if set
  } else {
    ltoa(res_val, result_str, 10); 
    Serial.print(F("DEBUG: Result (long): ")); Serial.print(res_val);
    Serial.print(F(", Result (str): '")); Serial.print(result_str); Serial.println(F("'"));
    set_calc_state(DISPLAY_RESULT);
    update_lcd();
  }
} 

// No handle_result_display needed as LCD updates immediately
// No start_result_display_sequence needed in the same way

void check_local_buttons() {
  // Allow button presses even if result/error is shown, to start new calculation
  // if (current_calc_state == DISPLAY_RESULT || current_calc_state == DISPLAY_ERROR ) { 
  //     return; 
  // }
  
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
  // Allow button presses even if result/error is shown
  
  byte bytesReceived = Wire.requestFrom(NANO_SLAVE_ADDRESS, 1); 

  if (bytesReceived > 0) {
    char received_char = Wire.read();
    if (received_char != 0) { 
      Serial.print(F("DEBUG: Uno received from Nano via I2C: '")); Serial.print(received_char); Serial.println(F("'"));
      last_uno_debounce_time = millis(); // Reset debounce for any action

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
            if ( (current_calc_state == INPUT_NUM2 && num2_len > 0 && num1_len > 0 && current_operator != 0) ||
                 (current_calc_state == INPUT_OPERATOR_AWAIT_NUM2 && num2_len > 0 && num1_len > 0 && current_operator !=0 ) || // If num2 was entered before equals
                 (current_calc_state == INPUT_NUM1 && num1_len > 0 && current_operator == 0) // Case like "5="
                ) { 
                 calculate_result();
            } else {
                 Serial.print(F("DEBUG: Equals pressed but conditions not met. State: "));
                 print_current_state_debug(); 
                 Serial.print(F(", num1_len: ")); Serial.print(num1_len);
                 Serial.print(F(", op: ")); Serial.print(current_operator);
                 Serial.print(F(", num2_len: ")); Serial.println(num2_len);
                 // Optionally do nothing or show a brief "Syntax Error" if equals is pressed without enough info
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