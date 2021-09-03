
#include "Arduino.h"
#include "OXRS_OLED.h"



OXRS_OLED::OXRS_OLED ()
{
  _oled_found = 0;
 
  _last_oled_trigger = 0L;
  _last_event_display = 0L;  
  
  memset(_io_values, 0, sizeof(_io_values));

}

void OXRS_OLED::begin (uint32_t ontime_display, uint32_t ontime_event)
{
  // Scan for OLED  
  Serial.println(F("Scanning for OLED on the I2C bus..."));
  Serial.print(F(" - 0x"));
  Serial.print(OLED_I2C_ADDRESS, HEX);
  Serial.print(F("..."));
  
  // Check if OLED is anything responding
  _oled_found = 0;
  Wire.beginTransmission(OLED_I2C_ADDRESS);
  if (Wire.endTransmission() == 0)
  {
    _oled_found = 1;
    Serial.println(F("OLED"));
  }
  else
  {
    Serial.println(F("empty"));
  }

  if (!_oled_found) {return;}
  
  // If OLED was found then initialise
  #ifdef OLED_TYPE_SSD1306
    Serial.print(F("SSD1306 "));
    #if OLED_RESET_PIN >= 0
      oled.begin(&Adafruit128x64, OLED_I2C_ADDRESS, OLED_RESET_PIN);
    #else
      oled.begin(&Adafruit128x64, OLED_I2C_ADDRESS);
    #endif
  #endif
  #ifdef OLED_TYPE_SH1106
    Serial.print(F("SH1106 "));
    #if OLED_RESET_PIN >= 0
      oled.begin(&SH1106_128x64, OLED_I2C_ADDRESS, OLED_RESET_PIN);
    #else
      oled.begin(&SH1106_128x64, OLED_I2C_ADDRESS);
    #endif
  #endif
  
  oled.clear();
  oled.setFont(Adafruit5x7);  
  oled.println(F("Initialising..."));

  _ontime_display = ontime_display;
  _ontime_event = ontime_event;
  
}


// combines (bitwise or) single pattern to complete one (multiple inputs per port)
void OXRS_OLED::_update_bit_map (uint8_t pattern)
{
  for (uint8_t i = 0; i < PATTERN_WIDTH; i++)
  {
    _bit_map[i] = _bit_map[i] | pgm_read_byte(&pattern_table[pattern][i]); 
  }
}

// fills bit map with desired pattern
void OXRS_OLED::_set_bit_map (uint8_t pattern)
{
  memcpy_P(_bit_map, pattern_table[pattern], PATTERN_WIDTH);
}

// writes bit map to OLED at x/y 
void OXRS_OLED::_write_bit_map (uint8_t x, uint8_t y)
{
  uint8_t i;
  oled.setCursor(x, y);
  for (i = 0; i < PATTERN_WIDTH-1; i++)
  {
    oled.ssd1306WriteRamBuf(_bit_map[i]);
  }
  oled.ssd1306WriteRam(_bit_map[i]); // writes buffer to display
}

// writes a pattern to OLED at x/y
void OXRS_OLED::_write_pattern (uint8_t pattern, uint8_t x, uint8_t y)
{
  _set_bit_map (pattern);
  _write_bit_map(x, y);
}

void OXRS_OLED::process (int mcp, uint16_t io_value)
{
  int i, port;
  uint16_t changed;
  
  // Compare with last stored value
  changed = io_value ^ _io_values[mcp];
  // change detected
  if (changed)
  {
    port = mcp * 4;
    for (i = 0; i < 4; i++)
    {
      if ((changed >> (i * 4)) & 0x000F)
      {
        oled.ssd1306WriteCmd(SSD1306_DISPLAYON);
        oled.setContrast(OLED_CONTRAST_ON);
        _last_oled_trigger = millis(); 

        _animate(port + i, ~io_value >> i*4);
      }
    }

    // Need to store so we can detect changes for port animation
    _io_values[mcp] = io_value;
  }
}


/**
  animation of input state in ports view
  Ports:    | 1 | 3 | 5 | 7 |     Index:      | 1 : 3 | 9 : 11|
            +---+---+---+---+....             |.......|.......|
            | 2 | 4 | 6 | 8 |                 | 2 : 4 | 10: 12|
                                              +-------+-------+......
                                              | 5 : 7 | 13: 15|
                                              |.......|.......|
                                              | 6 : 8 | 14: 16|                                             
*/
void OXRS_OLED::_animate (int port, uint16_t port_val)
{
  uint8_t x, y;

  // determin the input levels of the port
  _set_bit_map(FRAME_SOLID);
  if (port_val & 0x01) _update_bit_map(INP_TOP_LEFT);
  if (port_val & 0x02) _update_bit_map(INP_BOTTOM_LEFT);
  if (port_val & 0x04) _update_bit_map(INP_TOP_RIGHT);
  if (port_val & 0x08) _update_bit_map(INP_BOTTOM_RIGHT);
  x = (port/2)*PATTERN_WIDTH + (port/8)*4;   //  i/2*PATTERN_WIDTH (port dist) + i/8*4 (port group dist)
  y = 4 + (port & 0x01);
  _write_bit_map(x, y);
}


/** 
  draw logo and header on OLED
 */
void OXRS_OLED::draw_logo(char * firmware_version)
{ 
  if (!_oled_found) {return;}
  
  oled.clear();
  // display logo
  _write_pattern (6, 0, 0);
  _write_pattern (7, 8, 0);
  _write_pattern (8, 0, 1);
  _write_pattern (9, 8, 1);
  
  oled.setCursor(25, 0);
  oled.println(F("SuperHouse.TV"));
  oled.setCursor(25, 1);
  oled.print(F("USM v"));
  oled.print(firmware_version);
  oled.setCursor(0, 7);
}

/** 
  draw port outline on OLED
 */
 void OXRS_OLED::draw_ports (uint8_t mcps_found)
{
  if (!_oled_found) {return;}
  
  for (uint8_t i = 0; i < 12; i++)
  {
    uint8_t c = (bitRead(mcps_found, i>>1)) ? FRAME_SOLID : FRAME_DASHED;
    uint8_t x = i*PATTERN_WIDTH + i/4*4;
    _write_pattern (c, x, 4);
    _write_pattern (c, x, 5);
  }  
} 


void OXRS_OLED::show_IP (IPAddress ip)
{
  if (!_oled_found) {return;}
  
  oled.setCursor(25, 2);
  oled.print(ip); 
}

void OXRS_OLED::show_MAC (uint8_t mac[])
{
  if (!_oled_found) {return;}
  
  char mac_address[18];
  sprintf_P(mac_address, PSTR("%02X:%02X:%02X:%02X:%02X:%02X"), mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);

  oled.setCursor(25, 3);
  oled.print(mac_address); 
}


void OXRS_OLED::show_event (char s_event[])
{
  if (!_oled_found) {return;}
  
  // Show last input event on bottom line
  oled.setCursor(0, 7);
  oled.setInvertMode(true);
  oled.print(s_event); 
  oled.setInvertMode(false);    
  _last_event_display = millis(); 
}


void OXRS_OLED::update(void)
{
  if (!_oled_found) {return;}
  
  // Clear event display if timed out
  if (_ontime_event && _last_event_display)
  {
    if ((millis() - _last_event_display) > _ontime_event)
    {
      oled.setCursor(0, 7);
      oled.clearToEOL();
      
      _last_event_display = 0L;
    }
  }

  // Dim OLED if timed out
  if (_ontime_display && _last_oled_trigger)
  {
    if ((millis() - _last_oled_trigger) > _ontime_display)
    {
      // Turn OLED OFF if OLED_CONTRAST_DIM is set to 0
      if (OLED_CONTRAST_DIM == 0)
      {
        oled.ssd1306WriteCmd(SSD1306_DISPLAYOFF);
      }
      else
      { 
        oled.setContrast(OLED_CONTRAST_DIM);
      }
      
      _last_oled_trigger = 0L;
    }
  }
} 
