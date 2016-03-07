/*

  This code is in the public domain.

  ---------------------------------------------------------------------

  This program demonstrates button detection, LCD text/number printing,
  and LCD backlight control on the Freetronics LCD & Keypad Shield, connected to an Arduino board.


  Pins used by LCD & Keypad Shield:

    A0: Buttons, analog input from voltage ladder
    A1: Thermistor read
    D3: Relay control pin
    D4: LCD bit 4
    D5: LCD bit 5
    D6: LCD bit 6
    D7: LCD bit 7
    D8: LCD RS
    D9: LCD E
    D10: LCD Backlight (high = on, also has pullup high so default is on)

  ADC voltages for the 5 buttons on analog input pin A0:

    RIGHT:  0.00V :   0 @ 8bit ;   0 @ 10 bit
    UP:     0.71V :  36 @ 8bit ; 145 @ 10 bit
    DOWN:   1.61V :  82 @ 8bit ; 329 @ 10 bit
    LEFT:   2.47V : 126 @ 8bit ; 505 @ 10 bit
    SELECT: 3.62V : 185 @ 8bit ; 741 @ 10 bit

    note: the voltages I measured were crazy different, but appear to be consistent. PJC
*/

#ifndef FILAMEAT_FULLTEST_LCDBUTTONS_H_
#define FILAMEAT_FULLTEST_LCDBUTTONS_H_

#include <LiquidCrystal.h>   // include LCD library

// ADC readings expected for the 5 buttons on the ADC input
#define RIGHT_10BIT_ADC         815  // right
#define UP_10BIT_ADC            930  // up
#define DOWN_10BIT_ADC          903  // down
#define LEFT_10BIT_ADC          856  // left
#define SELECT_10BIT_ADC        612  // select
#define BUTTONHYSTERESIS         10  // hysteresis for valid button sensing window
//return values for ReadButtons()
#define BUTTON_NONE               0  //
#define BUTTON_RIGHT              1  //
#define BUTTON_UP                 2  //
#define BUTTON_DOWN               3  //
#define BUTTON_LEFT               4  //
#define BUTTON_SELECT             5  //
//some example macros with friendly labels for LCD backlight/pin control, tested and can be swapped into the example code as you like
#define LCD_BACKLIGHT_OFF()     digitalWrite( LCD_BACKLIGHT_PIN, LOW )
#define LCD_BACKLIGHT_ON()      digitalWrite( LCD_BACKLIGHT_PIN, HIGH )
#define LCD_BACKLIGHT(state)    { if( state ){digitalWrite( LCD_BACKLIGHT_PIN, HIGH );}else{digitalWrite( LCD_BACKLIGHT_PIN, LOW );} }

/*--------------------------------------------------------------------------------------
  Variables
--------------------------------------------------------------------------------------*/
class LCDButtons{
public:
	byte buttonJustPressed  = false;         //this will be true after a ReadButtons() call if triggered
	byte buttonJustReleased = false;         //this will be true after a ReadButtons() call if triggered
	byte buttonWas          = BUTTON_NONE;   //used by ReadButtons() for detection of button events

	//constructor
	LCDButtons(LiquidCrystal lcd);

	byte ReadButtons();
};

#endif /* FILAMEAT_FULLTEST_LCDBUTTONS_H_ */
