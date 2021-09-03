
#ifndef OXRS_OLED_H
#define OXRS_OLED_H

#include "Arduino.h"
#include <IPAddress.h>
#include "SSD1306Ascii.h"             // For OLED display
#include "SSD1306AsciiWire.h"         // For OLED display

/* OLED - ONE and ONLY ONE OLED_TYPE should be defined (comment out an others) */
//#define       OLED_TYPE_SSD1306                           // Mostly the .96"
#define       OLED_TYPE_SH1106                              // Mostly the 1.3"
#define       OLED_I2C_ADDRESS        0x3c                  // I2c address of OLED used
#define       OLED_RESET_PIN          -1                    // Reset pin (or -1 if sharing Arduino reset pin)
#define       OLED_CONTRAST_ON        128                   // OLED contrast when ON, i.e. after an event
#define       OLED_CONTRAST_DIM       5                     // OLED contrast when DIMMED (0 == OFF), i.e. after OLED_ON_MS expires
#define       OLED_ON_MS              10000                 // How long to turn on the OLED after an event
#define       OLED_EVENT_MS           3000                  // How long to display an event in the bottom line

#define PATTERN_WIDTH 10

// user defined grapic elements in 10x8 pixel
static const uint8_t PROGMEM pattern_table[][PATTERN_WIDTH] =
{
0xff,0x8f,0x8f,0x8f,0x8f,0x81,0x81,0x81, 0x81,0xff,  // 0 tl
0xff,0xf1,0xf1,0xf1,0xf1,0x81,0x81,0x81, 0x81,0xff,  // 1 bl
0xff,0x81,0x81,0x81,0x81,0x8f,0x8f,0x8f, 0x8f,0xff,  // 2 tr
0xff,0x81,0x81,0x81,0x81,0xf1,0xf1,0xf1, 0xf1,0xff,  // 3 br
0xff,0x81,0x81,0x81,0x81,0x81,0x81,0x81, 0x81,0xff,  // 4 outer frame solid
0xdb,0x81,0x00,0x00,0x81,0x81,0x00,0x00, 0x81,0xdb,  // 5 outer frame dashed

0xc0,0x70,0x1c,0x04,0x46,0xe2,0x73,0x79, 0x00,0x00, // 06 sh logo tl
0x71,0xe3,0x52,0x06,0x04,0x1c,0xf0,0x00, 0x00,0x00, // 07 sh logo tr
0x03,0x0e,0x38,0x20,0x67,0x45,0xc4,0x80, 0x00,0x00, // 08 sh logo bl
0x87,0xcd,0x48,0x6c,0x24,0x07,0x01,0x00, 0x00,0x00  // 19 sh logo br
};

#define FRAME_SOLID 4
#define FRAME_DASHED 5
#define INP_TOP_LEFT 0
#define INP_BOTTOM_LEFT 1
#define INP_TOP_RIGHT 2
#define INP_BOTTOM_RIGHT 3

class OXRS_OLED
{
  public:
    OXRS_OLED();
    void begin (uint32_t ontime_display=OLED_ON_MS, uint32_t ontime_event=OLED_EVENT_MS);
    void draw_logo(char * firmware_version);
    void draw_ports (uint8_t mcps_found);
    void show_IP (IPAddress ip);
    void show_MAC (byte mac[]);
    void show_event (char s_event[]);
    void process (int mcp, uint16_t io_value);
    void update();
    
  private:  
    // flag that represents if OLED is found
    uint8_t _oled_found;
    
    // for timeout (clear) of bottom line input event display
    uint32_t _last_event_display;
    
    // for timeout (dim) of OLED
    uint32_t _last_oled_trigger;

    // holds bit map for 1 pattern , 8 bits high, PATTERN_WIDTH wide
    uint8_t _bit_map[PATTERN_WIDTH];

    uint32_t _ontime_display;
    uint32_t _ontime_event;

    // history buffer of io_values to extract changes
    uint16_t _io_values[8];
        
    // OLED
    SSD1306AsciiWire oled;

    void _animate (int port, uint16_t io_val);
    void _update_bit_map (uint8_t pattern);
    void _set_bit_map (uint8_t pattern);
    void _write_bit_map (uint8_t x, uint8_t y);
    void _write_pattern (uint8_t pattern, uint8_t x, uint8_t y);
};
#endif
