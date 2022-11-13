/*===========================================================================
 * main.c
 * Copyright (c)2022 Kevin Boone, GPL v3.0
 * ========================================================================*/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <hardware/i2c.h>
#include <i2c_lcd/i2c_lcd.h>
#include <usb_kbd/usb_kbd.h>
#include <kbd/kbd.h>
#include "bsp/board.h"
#include "config.h"

I2C_LCD *i2c_lcd;

/*===========================================================================
 * blink_led_task
 * Called in the main scanning loop. We flash the LED just to indicate that
 * the program hasn't crashed.
 * ========================================================================*/
void blink_led_task (void)
  {
  const uint32_t interval_ms = 1000;
  static uint32_t start_ms = 0;
  static bool led_state = false;
  if(board_millis() - start_ms < interval_ms) { return; }
  start_ms += interval_ms;
  board_led_write(led_state);
  led_state = !led_state;
  }

/*===========================================================================
 * kbd_raw_key_down is called by the USB HID code whenever a
 * key is pressed. The value 'code' does not take account of which
 * modifiers are pressed -- call kbd_to_ascii to deal wity that.
 * ========================================================================*/
void kbd_raw_key_down (int code, int flags)
  {
  //char s[10];
  //sprintf (s, "%d %02X ", code, flags);
  //sprintf (s, "%d ", c, flags);
  //i2c_lcd_print_string (i2c_lcd, s);
  switch (code)
    {
    case KBD_KEY_UP:
      i2c_lcd_scrollback_line_up (i2c_lcd);
      break;
    case KBD_KEY_DOWN:
      i2c_lcd_scrollback_line_down (i2c_lcd);
      break;
    // TODO scrollback page up
    default:
      char c = kbd_to_ascii (code, flags);
      i2c_lcd_print_char (i2c_lcd, c);
    }
  }

/*===========================================================================
 * start here
 * ========================================================================*/
int main (void)
  {
  i2c_lcd = i2c_lcd_new (LCD_WIDTH, LCD_HEIGHT, I2C_LCD_ADDRESS, 
     PICO_DEFAULT_I2C_INSTANCE,
     PICO_DEFAULT_I2C_SDA_PIN, PICO_DEFAULT_I2C_SCL_PIN, I2C_BAUD, 
     SCROLLBACK_PAGES);

  i2c_lcd_set_cursor (i2c_lcd, 0, 0);
  i2c_lcd_print_string (i2c_lcd, "Hello ");

  usb_kbd_init();

  while (1) 
    {
    usb_kbd_scan();
    blink_led_task();
    }
  }

