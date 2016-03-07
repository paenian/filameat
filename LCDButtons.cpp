/*
 * LCDButtons.c
 *
 *  Created on: Mar 7, 2016
 *      Author: pjchase
 */

#include "LCDButtons.h"

LCDButtons::LCDButtons(LiquidCrystal lcd){
	//button adc input
	pinMode( BUTTON_ADC_PIN, INPUT );         //ensure A0 is an input
	digitalWrite( BUTTON_ADC_PIN, LOW );      //ensure pullup is off on A0
}

/*--------------------------------------------------------------------------------------
  ReadButtons()
  Detect the button pressed and return the value
  Uses global values buttonWas, buttonJustPressed, buttonJustReleased.
--------------------------------------------------------------------------------------*/
byte LCDButtons::ReadButtons()
{
   unsigned int buttonVoltage;
   unsigned int thermistorVoltage;
   byte button = BUTTON_NONE;   // return no button pressed if the below checks don't write to btn

   //read the button ADC pin voltage
   buttonVoltage = analogRead( BUTTON_ADC_PIN );
   thermistorVoltage = analogRead (THERMISTOR_PIN);
   lcd.setCursor( 9, 1 );
   lcd.print( thermistorVoltage, DEC );
   lcd.print( " " ); lcd.print( " " );
   //delay( 300 );
   //sense if the voltage falls within valid voltage windows
   if(    buttonVoltage >= ( RIGHT_10BIT_ADC - BUTTONHYSTERESIS )
       && buttonVoltage <= ( RIGHT_10BIT_ADC + BUTTONHYSTERESIS ) )
   {
      button = BUTTON_RIGHT;
   }
   else if(   buttonVoltage >= ( UP_10BIT_ADC - BUTTONHYSTERESIS )
           && buttonVoltage <= ( UP_10BIT_ADC + BUTTONHYSTERESIS ) )
   {
      button = BUTTON_UP;
   }
   else if(   buttonVoltage >= ( DOWN_10BIT_ADC - BUTTONHYSTERESIS )
           && buttonVoltage <= ( DOWN_10BIT_ADC + BUTTONHYSTERESIS ) )
   {
      button = BUTTON_DOWN;
   }
   else if(   buttonVoltage >= ( LEFT_10BIT_ADC - BUTTONHYSTERESIS )
           && buttonVoltage <= ( LEFT_10BIT_ADC + BUTTONHYSTERESIS ) )
   {
      button = BUTTON_LEFT;
   }
   else if(   buttonVoltage >= ( SELECT_10BIT_ADC - BUTTONHYSTERESIS )
           && buttonVoltage <= ( SELECT_10BIT_ADC + BUTTONHYSTERESIS ) )
   {
      button = BUTTON_SELECT;
   }
   //handle button flags for just pressed and just released events
   if( ( buttonWas == BUTTON_NONE ) && ( button != BUTTON_NONE ) )
   {
      //the button was just pressed, set buttonJustPressed, this can optionally be used to trigger a once-off action for a button press event
      //it's the duty of the receiver to clear these flags if it wants to detect a new button change event
      buttonJustPressed  = true;
      buttonJustReleased = false;
   }
   if( ( buttonWas != BUTTON_NONE ) && ( button == BUTTON_NONE ) )
   {
      buttonJustPressed  = false;
      buttonJustReleased = true;
   }

   //save the latest button value, for change event detection next time round
   buttonWas = button;

   return( button );
}
