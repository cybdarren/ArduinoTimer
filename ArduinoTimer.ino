/*
  Arduino Timer MS
  Arduino Timer with the capability to trigger an output line the start
  (to reset a board or processor) and then count up in milliseconds
  until the target toggles a line.

  Uses 2x16 line display


*/

// include the liquid cyrstal code:
#include <LiquidCrystal.h>

// pins used for the MPU timer
#define MPU_RESET 20
#define MPU_INT   21

// initialize the library by associating any needed LCD interface pin
// with the arduino pin number it is connected to
const int rs = 8, en = 9, d4 = 4, d5 = 5, d6 = 6, d7 = 7;
LiquidCrystal lcd(rs, en, d4, d5, d6, d7);

// panel buttons
int lcd_key     = 0;
int adc_key_in  = 0;
#define btnRIGHT  0
#define btnUP     1
#define btnDOWN   2
#define btnLEFT   3
#define btnSELECT 4
#define btnNONE   5

// time current sequence started
unsigned long timerStartMillis = 0;


// captured value taken when input is signalled
volatile unsigned long long captureTime = 0;
volatile bool captureRunning = false;

typedef enum {
  IDLE,         // do nothing
  RESET_HOLD,   // hold output line in reset until button released
  RUNNING,      // timer is running
  CAPTURED,     // event is captured
  CANCEL,       // cancel timing sequence
} TIMER_STATE;
TIMER_STATE timerState = IDLE;

// read the buttons
int read_LCD_buttons()
{
  adc_key_in = analogRead(0); // read the value from the sensor
  if (adc_key_in > 1000) return btnNONE; // We make this the 1st option for speed reasons since it will be the most likely result
  // For V1.1 use this threshold
  if (adc_key_in < 50)   return btnRIGHT;
  if (adc_key_in < 250)  return btnUP;
  if (adc_key_in < 450)  return btnDOWN;
  if (adc_key_in < 650)  return btnLEFT;
  if (adc_key_in < 850)  return btnSELECT;
  return btnNONE;  // when all others fail, return this...
}
void setup() {
  // set up the LCD's number of columns and rows:
  lcd.begin(16, 2);
  lcd.setCursor(0, 0);
  lcd.clear();
  lcd.print("Event timer");

  // configure reset line to MPU
  pinMode(MPU_RESET, INPUT);
  digitalWrite(MPU_RESET, HIGH);

  // configure interrupt line from MPU
  pinMode(MPU_INT, INPUT);
  attachInterrupt(digitalPinToInterrupt(MPU_INT), ISR_input_set, FALLING);
  
  captureTime = 0;
  captureRunning = false;
  timerState = IDLE;
}

void loop() { 
  unsigned long currentTime;

  // get any button currently pressed
  lcd_key = read_LCD_buttons();
  
  // execute the timer state machine
  switch (timerState) {
    case IDLE:
      switch (lcd_key) {
        case btnSELECT:
          digitalWrite(MPU_RESET, LOW);
          pinMode(MPU_RESET, OUTPUT);
          timerState = RESET_HOLD;
          break;
        case btnUP:
          break;
        case btnDOWN:
          break;
        case btnLEFT:
          break;
        case btnRIGHT:
          break; 
        default:
          break;
      }
      break;
      
    case RESET_HOLD:
      // wait until button release before releasing output line
      if (lcd_key == btnNONE) {
        // display timer is running
        lcd.setCursor(0, 0);
        lcd.print("Running     ");
        lcd.setCursor(0, 1);
        lcd.print("0               ");
        
        pinMode(MPU_RESET, INPUT);
        timerState = RUNNING;    

        // record the start time
        timerStartMillis = millis();

        captureTime = 0;
        captureRunning = true;
      }
      break;
      
    case RUNNING:
      if (lcd_key != btnNONE) {
        // any key pressed whilst running causes reset of timer
        timerState = CANCEL;
      }

      if (captureRunning == false) {
        timerState = CAPTURED;  
      }
      
      // take current ms timer and display an elapsed value on the display
      currentTime = millis() - timerStartMillis;
      lcd.setCursor(0, 1);
      lcd.print(currentTime);     
      break;
      
    case CAPTURED:
      // display the captured value
      lcd.setCursor(0, 0);
      lcd.print("Startup Time");
      currentTime = captureTime - timerStartMillis;
      lcd.setCursor(0, 1);
      lcd.print(currentTime);
      lcd.print(" ms");
      
      // go to IDLE state
      timerState = IDLE;
      break;

    case CANCEL:
      // wait for release of select to cancel
      do { 
        delay(500);
        lcd_key = read_LCD_buttons();
      } while (lcd_key != btnNONE);
      
      timerState = IDLE;
      break;
  }

}

void ISR_input_set(void)
{
  // only capture the value if we are running
  if (captureRunning && (digitalRead(MPU_INT) == LOW))
  {
    captureTime = millis();
    captureRunning = false;
  }
}
