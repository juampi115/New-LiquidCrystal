// ---------------------------------------------------------------------------
// Created by Francisco Malpartida on 7.3.2012.
// Copyright 2011 - Under creative commons license 3.0:
//        Attribution-ShareAlike CC BY-SA
//
// This software is furnished "as is", without technical support, and with no 
// warranty, express or implied, as to its usefulness for any purpose.
//
// Thread Safe: No
// Extendable: Yes
//
// @file LiquidCrystal_SRG.h
// This file implements a basic liquid crystal library that comes as standard
// in the Arduino SDK but using a generic SHIFT REGISTER extension board.
// 
// @brief 
// This is a basic implementation of the LiquidCrystal library of the
// Arduino SDK. The original library has been reworked in such a way that 
// this class implements the all methods to command an LCD based
// on the Hitachi HD44780 and compatible chipsets using a 3 wire latching
// shift register. While it has been tested with a 74HC595N shift register
// it should also work with other latching shift registers such as the MC14094
// and the HEF4094
//
// This particular driver has been created as generic as possible to enable
// users to configure and connect their LCDs using just 3 digital IOs from the
// AVR or Arduino, and connect the LCD to the outputs of the shiftregister
// in any configuration. The library is configured by passing the IO pins
// that control the strobe, data and clock of the shift register and a map
// of how the shiftregister is connected to the LCD.
// 
//
//   +--------------------------------------------+
//   |                 MCU                        |
//   |   IO1           IO2           IO3          |
//   +----+-------------+-------------+-----------+
//        |             |             |
//        |             |             |
//   +----+-------------+-------------+-----------+
//   |    Strobe        Data          Clock       |
//   |          8-bit shift/latch register        |
//   |    Qa   Qb   Qc   Qd   Qe   Qf   Qg   Qh   |
//   +----+----+----+----+----+----+----+----+----+
//        |    |    |    |    |    |    |    
//        |11  |12  |13  |14  |6   |5   |4    (LCD pins)
//   +----+----+----+----+----+----+----+----+----+
//   |    DB4  DB5  DB6  DB7  E    Rw   RS        |
//   |                 LCD Module                 |
//
// The functionality provided by this class and its base class is identical
// to the original functionality of the Arduino LiquidCrystal library.
//
//
// @author F. Malpartida - fmalpartida@gmail.com
// ---------------------------------------------------------------------------
// flags for backlight control
#include <stdio.h>
#include <string.h>
#include <inttypes.h>

#if (ARDUINO <  100)
#include <WProgram.h>
#else
#include <Arduino.h>
#endif
#include "LiquidCrystal_SR3W.h"

#include "FastIO.h"

/*!
 @defined 
 @abstract   LCD_NOBACKLIGHT
 @discussion No BACKLIGHT MASK
 */
#define LCD_NOBACKLIGHT 0x00

/*!
 @defined 
 @abstract   LCD_BACKLIGHT
 @discussion BACKLIGHT MASK used when backlight is on
 */
#define LCD_BACKLIGHT   0xFF


// Default library configuration parameters used by class constructor with
// only the I2C address field.
// ---------------------------------------------------------------------------
/*!
 @defined 
 @abstract   Enable bit of the LCD
 @discussion Defines the IO of the expander connected to the LCD Enable
 */
#define EN 4  // Enable bit

/*!
 @defined 
 @abstract   Read/Write bit of the LCD
 @discussion Defines the IO of the expander connected to the LCD Rw pin
 */
#define RW 5  // Read/Write bit

/*!
 @defined 
 @abstract   Register bit of the LCD
 @discussion Defines the IO of the expander connected to the LCD Register select pin
 */
#define RS 6  // Register select bit

/*!
 @defined 
 @abstract   LCD dataline allocation this library only supports 4 bit LCD control
 mode.
 @discussion D4, D5, D6, D7 LCD data lines pin mapping of the extender module
 */
#define D4 0
#define D5 1
#define D6 2
#define D7 3



LiquidCrystal_SR3W::LiquidCrystal_SR3W(uint8_t data, uint8_t clk, uint8_t strobe)
{
   init( data, clk, strobe, RS, RW, EN, D4, D5, D6, D7 );
}

LiquidCrystal_SR3W::LiquidCrystal_SR3W(uint8_t data, uint8_t clk, uint8_t strobe,
                                       uint8_t backlighPin, t_backlighPol pol)
{
   init( data, clk, strobe, RS, RW, EN, D4, D5, D6, D7 );   
}

LiquidCrystal_SR3W::LiquidCrystal_SR3W(uint8_t data, uint8_t clk, uint8_t strobe,
                                       uint8_t En, uint8_t Rw, uint8_t Rs, 
                                       uint8_t d4, uint8_t d5, uint8_t d6, uint8_t d7 )
{
   init( data, clk, strobe, En, Rw, En, d4, d5, d6, d7 );
}

LiquidCrystal_SR3W::LiquidCrystal_SR3W(uint8_t data, uint8_t clk, uint8_t strobe, 
                                       uint8_t En, uint8_t Rw, uint8_t Rs, 
                                       uint8_t d4, uint8_t d5, uint8_t d6, uint8_t d7,
                                       uint8_t backlighPin, t_backlighPol pol)
{
   init( data, clk, strobe, En, Rw, En, d4, d5, d6, d7 );
}


void LiquidCrystal_SR3W::send(uint8_t value, uint8_t mode)
{
   // No need to use the delay routines since the time taken to write takes
   // longer that what is needed both for toggling and enable pin an to execute
   // the command.
   
   if ( mode == FOUR_BITS )
   {
      write4bits( (value & 0x0F), COMMAND );
   }
   else 
   {
      write4bits( (value >> 4), mode );
      write4bits( (value & 0x0F), mode);
   }   
}


void LiquidCrystal_SR3W::setBacklightPin ( uint8_t value, t_backlighPol pol = POSITIVE )
{
   _backlightPinMask = ( 1 << value );
   _polarity = pol;
}

void LiquidCrystal_SR3W::setBacklight ( uint8_t value )
{
   // Check if backlight is available
   // ----------------------------------------------------
   if ( _backlightPinMask != 0x0 )
   {
      // Check for polarity to configure mask accordingly
      // ----------------------------------------------------------
      if  (((_polarity == POSITIVE) && (value > 0)) || 
           ((_polarity == NEGATIVE ) && ( value == 0 )))
      {
         _backlightStsMask = _backlightPinMask & LCD_BACKLIGHT;
      }
      else 
      {
         _backlightStsMask = _backlightPinMask & LCD_NOBACKLIGHT;
      }
      loadSR( _backlightStsMask );
   }
}


// PRIVATE METHODS
// -----------------------------------------------------------------------------

int LiquidCrystal_SR3W::init(uint8_t data, uint8_t clk, uint8_t strobe, 
                             uint8_t Rs, uint8_t Rw, uint8_t En,
                             uint8_t d4, uint8_t d5, uint8_t d6, uint8_t d7)
{
   _data       = fio_pinToBit(data);
   _clk        = fio_pinToBit(clk);
   _strobe     = fio_pinToBit(strobe);
   _data_reg   = fio_pinToOutputRegister(data);
   _clk_reg    = fio_pinToOutputRegister(clk);
   _strobe_reg = fio_pinToOutputRegister(strobe);
   
   // LCD pin mapping
   _backlightPinMask = 0;
   _backlightStsMask = LCD_NOBACKLIGHT;
   _polarity = POSITIVE;
   
   _En = ( 1 << En );
   _Rw = ( 1 << Rw );
   _Rs = ( 1 << Rs );
   
   // Initialise pin mapping
   _data_pins[0] = ( 1 << d4 );
   _data_pins[1] = ( 1 << d5 );
   _data_pins[2] = ( 1 << d6 );
   _data_pins[3] = ( 1 << d7 );
   
   _displayfunction = LCD_4BITMODE | LCD_1LINE | LCD_5x10DOTS;
   
   return (1);
}

void LiquidCrystal_SR3W::write4bits(uint8_t value, uint8_t mode)
{
   uint8_t pinMapValue = 0;
   
   // Map the value to LCD pin mapping
   // --------------------------------
   for ( uint8_t i = 0; i < 4; i++ )
   {
      if ( ( value & 0x1 ) == 1 )
      {
         pinMapValue |= _data_pins[i];
      }
      value = ( value >> 1 );
   }
   
   // Is it a command or data
   // -----------------------
   if ( mode == DATA )
   {
      mode = _Rs;
   }
   
   pinMapValue |= mode | _backlightStsMask;
   loadSR ( pinMapValue | _En );  // Send with enable high
   loadSR ( pinMapValue & ~_En ); // Send with enable low
}


void LiquidCrystal_SR3W::loadSR(uint8_t value) 
{
   // Load the shift register with information
   fio_shiftOut(_data_reg, _data, _clk_reg, _clk, value, MSBFIRST);
   
   // Strobe the data into the latch
   ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
   {
      fio_digitalWrite_HIGH(_strobe_reg, _strobe);
      waitUsec( 1 ); // strobe pulse must be >450ns
      fio_digitalWrite_SWITCHTO(_strobe_reg, _strobe, LOW);
   }
   waitUsec( 40 ); // commands need > 37us to settle
}
