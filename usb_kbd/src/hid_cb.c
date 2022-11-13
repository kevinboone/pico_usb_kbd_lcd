/*===========================================================================
 * usb_kbd/hid_cb.c
 *
 * USB HID callbacks, called by TinyUSB
 *
 * Copyright (c)2022 Kevin Boone, GPL v3.0
 * ========================================================================*/

#include <kbd/kbd.h>
#include "bsp/board.h"
#include "tusb.h"

#define HID_MAX_KEYS 0x67

// TODO: fill in the rest of the non-ASCII key codes
// There's a nice list here:
// https://gist.github.com/ekaitz-zarraga/2b25b94b711684ba4e969e5a5723969b
static int conv_table_us[128][2] = 
{
    {0     , 0      }, /* 0x00 */ \
    {0     , 0      }, /* 0x01 */ \
    {0     , 0      }, /* 0x02 */ \
    {0     , 0      }, /* 0x03 */ \
    {'a'   , 'A'    }, /* 0x04 */ \
    {'b'   , 'B'    }, /* 0x05 */ \
    {'c'   , 'C'    }, /* 0x06 */ \
    {'d'   , 'D'    }, /* 0x07 */ \
    {'e'   , 'E'    }, /* 0x08 */ \
    {'f'   , 'F'    }, /* 0x09 */ \
    {'g'   , 'G'    }, /* 0x0a */ \
    {'h'   , 'H'    }, /* 0x0b */ \
    {'i'   , 'I'    }, /* 0x0c */ \
    {'j'   , 'J'    }, /* 0x0d */ \
    {'k'   , 'K'    }, /* 0x0e */ \
    {'l'   , 'L'    }, /* 0x0f */ \
    {'m'   , 'M'    }, /* 0x10 */ \
    {'n'   , 'N'    }, /* 0x11 */ \
    {'o'   , 'O'    }, /* 0x12 */ \
    {'p'   , 'P'    }, /* 0x13 */ \
    {'q'   , 'Q'    }, /* 0x14 */ \
    {'r'   , 'R'    }, /* 0x15 */ \
    {'s'   , 'S'    }, /* 0x16 */ \
    {'t'   , 'T'    }, /* 0x17 */ \
    {'u'   , 'U'    }, /* 0x18 */ \
    {'v'   , 'V'    }, /* 0x19 */ \
    {'w'   , 'W'    }, /* 0x1a */ \
    {'x'   , 'X'    }, /* 0x1b */ \
    {'y'   , 'Y'    }, /* 0x1c */ \
    {'z'   , 'Z'    }, /* 0x1d */ \
    {'1'   , '!'    }, /* 0x1e */ \
    {'2'   , '@'    }, /* 0x1f */ \
    {'3'   , '#'    }, /* 0x20 */ \
    {'4'   , '$'    }, /* 0x21 */ \
    {'5'   , '%'    }, /* 0x22 */ \
    {'6'   , '^'    }, /* 0x23 */ \
    {'7'   , '&'    }, /* 0x24 */ \
    {'8'   , '*'    }, /* 0x25 */ \
    {'9'   , '('    }, /* 0x26 */ \
    {'0'   , ')'    }, /* 0x27 */ \
    {KBD_KEY_ENTER  , KBD_KEY_ENTER   }, /* 0x28 */ \
    {'\x1b', '\x1b' }, /* 0x29 */ \
    {KBD_KEY_BS  , KBD_KEY_BS }, /* 0x2a */ \
    {'\t'  , '\t'   }, /* 0x2b */ \
    {' '   , ' '    }, /* 0x2c */ \
    {'-'   , '_'    }, /* 0x2d */ \
    {'='   , '+'    }, /* 0x2e */ \
    {'['   , '{'    }, /* 0x2f */ \
    {']'   , '}'    }, /* 0x30 */ \
    {'\\'  , '|'    }, /* 0x31 */ \
    {'#'   , '~'    }, /* 0x32 */ \
    {';'   , ':'    }, /* 0x33 */ \
    {'\''  , '\"'   }, /* 0x34 */ \
    {'`'   , '~'    }, /* 0x35 */ \
    {','   , '<'    }, /* 0x36 */ \
    {'.'   , '>'    }, /* 0x37 */ \
    {'/'   , '?'    }, /* 0x38 */ \
                                  \
    {0     , 0      }, /* 0x39 */ \
    {0     , 0      }, /* 0x3a */ \
    {0     , 0      }, /* 0x3b */ \
    {0     , 0      }, /* 0x3c */ \
    {0     , 0      }, /* 0x3d */ \
    {0     , 0      }, /* 0x3e */ \
    {0     , 0      }, /* 0x3f */ \
    {0     , 0      }, /* 0x40 */ \
    {0     , 0      }, /* 0x41 */ \
    {0     , 0      }, /* 0x42 */ \
    {0     , 0      }, /* 0x43 */ \
    {0     , 0      }, /* 0x44 */ \
    {0     , 0      }, /* 0x45 */ \
    {0     , 0      }, /* 0x46 */ \
    {0     , 0      }, /* 0x47 */ \
    {0     , 0      }, /* 0x48 */ \
    {0     , 0      }, /* 0x49 */ \
    {KBD_KEY_HOME , KBD_KEY_HOME}, /* 0x4a */ \
    {KBD_KEY_PGUP , KBD_KEY_PGUP}, /* 0x4b */ \
    {0     , 0      }, /* 0x4c */ \
    {KBD_KEY_END, KBD_KEY_END}, /* 0x4d */ \
    {KBD_KEY_PGDN , KBD_KEY_PGDN }, /* 0x4e */ \
    {KBD_KEY_RIGHT, KBD_KEY_RIGHT }, /* 0x4f */ \
    {KBD_KEY_LEFT, KBD_KEY_LEFT }, /* 0x50 */ \
    {KBD_KEY_DOWN, KBD_KEY_DOWN }, /* 0x51 */ \
    {KBD_KEY_UP, KBD_KEY_UP }, /* 0x52 */ \
    {0     , 0      }, /* 0x53 */ \
                                  \
    {'/'   , '/'    }, /* 0x54 */ \
    {'*'   , '*'    }, /* 0x55 */ \
    {'-'   , '-'    }, /* 0x56 */ \
    {'+'   , '+'    }, /* 0x57 */ \
    {'\r'  , '\r'   }, /* 0x58 */ \
    {'1'   , 0      }, /* 0x59 */ \
    {'2'   , 0      }, /* 0x5a */ \
    {'3'   , 0      }, /* 0x5b */ \
    {'4'   , 0      }, /* 0x5c */ \
    {'5'   , '5'    }, /* 0x5d */ \
    {'6'   , 0      }, /* 0x5e */ \
    {'7'   , 0      }, /* 0x5f */ \
    {'8'   , 0      }, /* 0x60 */ \
    {'9'   , 0      }, /* 0x61 */ \
    {'0'   , 0      }, /* 0x62 */ \
    {'0'   , 0      }, /* 0x63 */ \
    {'='   , '='    }, /* 0x67 */ \
};

/*===========================================================================
 * is_key_held 
 * Check whether the current key scancode is a repetition of the previous
 * one. In the longer term, we need to handle hold-down auto-repeat. For
 * now, it's easier just to filter the repeats. 
 * ========================================================================*/
static inline bool is_key_held (hid_keyboard_report_t const *report, 
    uint8_t keycode)
  {
  for(uint8_t i=0; i<6; i++) 
    {
    if (report->keycode[i] == keycode) 
      {
      return true;
      }
    }
  return false;
  }

/*===========================================================================
 * process_kbd_report 
 * ========================================================================*/
static void process_kbd_report (hid_keyboard_report_t const *report)
  {
  static hid_keyboard_report_t prev_report = { 0, 0, {0} };
	
  for (uint8_t i=0; i < 6; i++) 
    {
    if (report->keycode[i] ) 
      {
      if (!is_key_held (&prev_report, report->keycode[i])) 
        {
	bool const is_shift_pressed = report->modifier 
          & (KEYBOARD_MODIFIER_LEFTSHIFT | KEYBOARD_MODIFIER_RIGHTSHIFT);
        int ch = conv_table_us [report->keycode[i]][is_shift_pressed];
	bool const is_ctrl_pressed = report->modifier 
          & (KEYBOARD_MODIFIER_LEFTCTRL | KEYBOARD_MODIFIER_RIGHTCTRL);
	bool const is_alt_pressed = report->modifier 
          & (KEYBOARD_MODIFIER_LEFTALT | KEYBOARD_MODIFIER_RIGHTALT);
        int flags = 0;
        if (is_shift_pressed) flags |= KBD_FLAG_SHIFT;
        if (is_ctrl_pressed) flags |= KBD_FLAG_CONTROL;
        if (is_alt_pressed) flags |= KBD_FLAG_ALT;
        kbd_raw_key_down (ch, flags); 
	}
      }
    }
  prev_report = *report;
  }

  
/*===========================================================================
 * tuh_hid_mount_cb
 * Called by TinyUSB whenever a USB device is detected 
 * ========================================================================*/
void tuh_hid_mount_cb (uint8_t dev_addr, uint8_t instance, 
    uint8_t const* desc_report, uint16_t desc_len)
  {
  (void) desc_len; (void) desc_report;

  uint8_t const itf_protocol = tuh_hid_interface_protocol(dev_addr, instance);
  if (itf_protocol == HID_ITF_PROTOCOL_NONE) 
    {
    printf("Device with address %d, instance %d is not a keyboard or mouse.\r\n", dev_addr, instance);
    return;
    }
  
  const char* protocol_str[] = { "None", "Keyboard", "Mouse" };
  printf("Device with address %d, instance %d is a %s.\r\n", 
    dev_addr, instance, protocol_str[itf_protocol]);

  tuh_hid_receive_report(dev_addr, instance);
  }

/*===========================================================================
 * tuh_hid_report_received_cb
 * Called by TinyUSB whenever USB data is received 
 * ========================================================================*/
void tuh_hid_report_received_cb  (uint8_t dev_addr, uint8_t instance, 
      uint8_t const* report, uint16_t len)
  {
  (void) instance; (void) len;
  switch (tuh_hid_interface_protocol (dev_addr, instance)) 
    {
    case HID_ITF_PROTOCOL_KEYBOARD:
      process_kbd_report ((hid_keyboard_report_t const*) report);
      break;
    /*
    // We could handle mouse here, too, if necessary
    case HID_ITF_PROTOCOL_MOUSE:
      process_mouse_report ((hid_mouse_report_t const*) report);
      break;
    */
    }

  tuh_hid_receive_report (dev_addr, instance);
  }

/*===========================================================================
 * tuh_hid_unmount_cb
 * Called by TinyUSB whenever a USB device is removed 
 * ========================================================================*/
void tuh_hid_umount_cb (uint8_t dev_addr, uint8_t instance)
  {
  (void) dev_addr;
  (void) instance;
  }

