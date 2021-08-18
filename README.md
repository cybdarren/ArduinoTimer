# ArduinoTimer
Simple timer used to control and instrument external devices.
Code designed for Arduino Mega 2560 board with 2x16 line DF/D1 Robot lcd display.

SDA/Pin20 is the output to the target used to trigger the reset line.
SDA/PIN21 is the input from the target board. This is assumed to be active low.

Operation,
Press the Select button on the Arduino Display. This will take the MPU_RESET line (SDA/Pin 20) low causing the target to enter the reset state.
Release the Select button. The MPU_RESET line will go high and the time is noted.
The display counts up in milliseconds.

At a predefined point in time (perhaps inside your target application), the MPU_INT line is taken low to indicate the end of the timing period.
The Arduino timer will record this time (in an interrupt), stop the counter and display the value.

