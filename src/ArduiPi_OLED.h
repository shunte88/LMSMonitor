/*********************************************************************
This is a library for our Monochrome OLEDs based on SSD1306 drivers

  Pick one up today in the adafruit shop!
  ------> http://www.adafruit.com/category/63_98

These displays use SPI to communicate, 4 or 5 pins are required to  
interface

Adafruit invests time and resources providing this open source code, 
please support Adafruit and open-source hardware by purchasing 
products from Adafruit!

Written by Limor Fried/Ladyada  for Adafruit Industries.  
BSD license, check license.txt for more information
All text above, and the splash screen must be included in any redistribution

02/18/2013  Charles-Henri Hallard (http://hallard.me)
            Modified for compiling and use on Raspberry ArduiPi Board
            LCD size and connection are now passed as arguments on 
            the command line (no more #define on compilation needed)
            ArduiPi project documentation http://hallard.me/arduipi
            
07/26/2013  Charles-Henri Hallard (http://hallard.me)
            modified name for generic library using different OLED type
 
*********************************************************************/

#ifndef _ArduiPi_OLED_H
#define _ArduiPi_OLED_H

#include "./Adafruit_GFX.h"

/// fit into the SSD1306_ naming scheme
#define SSD1306_BLACK 0   ///< Draw 'off' pixels
#define SSD1306_WHITE 1   ///< Draw 'on' pixels
#define SSD1306_INVERSE 2 ///< Invert pixels

/// The following "raw" color names are kept for backwards client compatability
/// They can be disabled by predefining this macro before including the Adafruit
/// header client code will then need to be modified to use the scoped enum
/// values directly
#ifndef NO_ADAFRUIT_SSD1306_COLOR_COMPATIBILITY
#define BLACK SSD1306_BLACK     ///< Draw 'off' pixels
#define WHITE SSD1306_WHITE     ///< Draw 'on' pixels
#define INVERSE SSD1306_INVERSE ///< Invert pixels
#endif

/*=========================================================================
    SSDxxxx Common Displays
    -----------------------------------------------------------------------
    Common values to all displays
=========================================================================*/

#define SSD1306_MEMORYMODE 0x20          ///< See datasheet
#define SSD1306_COLUMNADDR 0x21          ///< See datasheet
#define SSD1306_PAGEADDR 0x22            ///< See datasheet
#define SSD1306_SETCONTRAST 0x81         ///< See datasheet
#define SSD1306_CHARGEPUMP 0x8D          ///< See datasheet
#define SSD1306_SEGREMAP 0xA0            ///< See datasheet
#define SSD1306_DISPLAYALLON_RESUME 0xA4 ///< See datasheet
#define SSD1306_DISPLAYALLON 0xA5        ///< Not currently used
#define SSD1306_NORMALDISPLAY 0xA6       ///< See datasheet
#define SSD1306_INVERTDISPLAY 0xA7       ///< See datasheet
#define SSD1306_SETMULTIPLEX 0xA8        ///< See datasheet
#define SSD1306_DISPLAYOFF 0xAE          ///< See datasheet
#define SSD1306_DISPLAYON 0xAF           ///< See datasheet
#define SSD1306_COMSCANINC 0xC0          ///< Not currently used
#define SSD1306_COMSCANDEC 0xC8          ///< See datasheet
#define SSD1306_SETDISPLAYOFFSET 0xD3    ///< See datasheet
#define SSD1306_SETDISPLAYCLOCKDIV 0xD5  ///< See datasheet
#define SSD1306_SETPRECHARGE 0xD9        ///< See datasheet
#define SSD1306_SETCOMPINS 0xDA          ///< See datasheet
#define SSD1306_SETVCOMDETECT 0xDB       ///< See datasheet

#define SSD1306_SETLOWCOLUMN 0x00  ///< Not currently used
#define SSD1306_SETHIGHCOLUMN 0x10 ///< Not currently used
#define SSD1306_SETSTARTLINE 0x40  ///< See datasheet

#define SSD1306_EXTERNALVCC 0x01  ///< External display voltage source
#define SSD1306_SWITCHCAPVCC 0x02 ///< Gen. display voltage from 3.3V

#define SSD1306_RIGHT_HORIZONTAL_SCROLL 0x26              ///< Init rt scroll
#define SSD1306_LEFT_HORIZONTAL_SCROLL 0x27               ///< Init left scroll
#define SSD1306_VERTICAL_AND_RIGHT_HORIZONTAL_SCROLL 0x29 ///< Init diag scroll
#define SSD1306_VERTICAL_AND_LEFT_HORIZONTAL_SCROLL 0x2A  ///< Init diag scroll
#define SSD1306_DEACTIVATE_SCROLL 0x2E                    ///< Stop scroll
#define SSD1306_ACTIVATE_SCROLL 0x2F                      ///< Start scroll
#define SSD1306_SET_VERTICAL_SCROLL_AREA 0xA3             ///< Set scroll range

//#define SSD_Command_Mode      0x80  /* DC bit is 0 */ Seeed set C0 to 1 why ?
#define SSD_Command_Mode 0x00 /* C0 and DC bit are 0         */
#define SSD_Data_Mode 0x40    /* C0 bit is 0 and DC bit is 1 */

#define SSD_Set_Segment_Remap 0xA0
#define SSD_Inverse_Display 0xA7
#define SSD_Set_Muliplex_Ratio 0xA8

#define SSD_Display_Off 0xAE
#define SSD_Display_On 0xAF

#define SSD_Set_ContrastLevel 0x81

#define SSD_External_Vcc 0x01
#define SSD_Internal_Vcc 0x02

#define SSD_Set_Column_Address 0x21
#define SSD_Set_Page_Address 0x22

#define SSD_Activate_Scroll 0x2F
#define SSD_Deactivate_Scroll 0x2E

#define SSD_Right_Horizontal_Scroll 0x26
#define SSD_Left_Horizontal_Scroll 0x27

#define Scroll_Left 0x00
#define Scroll_Right 0x01

#define Scroll_2Frames 0x07
#define Scroll_3Frames 0x04
#define Scroll_4Frames 0x05
#define Scroll_5Frames 0x00
#define Scroll_25Frames 0x06
#define Scroll_64Frames 0x01
#define Scroll_128Frames 0x02
#define Scroll_256Frames 0x03

#define VERTICAL_MODE 01
#define PAGE_MODE 01
#define HORIZONTAL_MODE 02

/*=========================================================================
    SSD1306 Displays
    -----------------------------------------------------------------------
    The driver is used in multiple displays (128x64, 128x32, etc.).
=========================================================================*/

#define SSD1306_Entire_Display_Resume 0xA4
#define SSD1306_Entire_Display_On 0xA5

#define SSD1306_Normal_Display 0xA6

#define SSD1306_Set_Display_Offset 0xD3
#define SSD1306_Set_Com_Pins 0xDA
#define SSD1306_Set_Vcomh_Deselect_Level 0xDB
#define SSD1306_Set_Display_Clock_Div 0xD5
#define SSD1306_Set_Precharge_Period 0xD9
#define SSD1306_Set_Lower_Column_Start_Address 0x00
#define SSD1306_Set_Higher_Column_Start_Address 0x10
#define SSD1306_Set_Start_Line 0x40
#define SSD1306_Set_Memory_Mode SSD1306_MEMORYMODE
#define SSD1306_Set_Com_Output_Scan_Direction_Normal 0xC0
#define SSD1306_Set_Com_Output_Scan_Direction_Remap 0xC8
#define SSD1306_Charge_Pump_Setting 0x8D

// Scrolling #defines
#define SSD1306_SET_VERTICAL_SCROLL_AREA 0xA3
#define SSD1306_VERTICAL_AND_RIGHT_HORIZONTAL_SCROLL 0x29
#define SSD1306_VERTICAL_AND_LEFT_HORIZONTAL_SCROLL 0x2A

/*=========================================================================
    SSD1308 Displays
    -----------------------------------------------------------------------
    The driver is used in multiple displays (128x64, 128x32, etc.).
=========================================================================*/
#define SSD1308_Normal_Display 0xA6

/*=========================================================================
    SSD1327 Displays
    -----------------------------------------------------------------------
    The driver is used in Seeed 96x96 display
=========================================================================*/
#define SSD1327_Set_Display_Start_Line 0xA1
#define SSD1327_Set_Display_Offset 0xA2
#define SSD1327_Normal_Display 0xA4
#define SSD1327_Set_Display_Clock_Div 0xB3
#define SSD1327_Set_Command_Lock 0xFD
#define SSD1327_Set_Column_Address 0x15
#define SSD1327_Set_Row_Address 0x75

/*=========================================================================
    SSD1322 Displays
    -----------------------------------------------------------------------
    The driver is used for 256x64 display (gray scale)
=========================================================================*/
#define SSD1322_Set_Display_Start_Line 0xA1
#define SSD1322_Set_Display_Offset 0xA2
#define SSD1322_Normal_Display 0xA4
#define SSD1322_Set_Display_Clock_Div 0xB3
#define SSD1322_Set_Command_Lock 0xFD
#define SSD1322_Set_Column_Address 0x15
#define SSD1322_Set_Row_Address 0x75

#define SSD1322_SETCOMMANDLOCK 0xFD
#define SSD1322_DISPLAYOFF 0xAE
#define SSD1322_DISPLAYON 0xAF
#define SSD1322_SETCLOCKDIVIDER 0xB3
#define SSD1322_SETDISPLAYOFFSET 0xA2
#define SSD1322_SETSTARTLINE 0xA1
#define SSD1322_SETREMAP 0xA0
#define SSD1322_FUNCTIONSEL 0xAB
#define SSD1322_DISPLAYENHANCE 0xB4
#define SSD1322_SETCONTRASTCURRENT 0xC1
#define SSD1322_MASTERCURRENTCONTROL 0xC7
#define SSD1322_SETPHASELENGTH 0xB1
#define SSD1322_DISPLAYENHANCEB 0xD1
#define SSD1322_SETPRECHARGEVOLTAGE 0xBB
#define SSD1322_SETSECONDPRECHARGEPERIOD 0xB6
#define SSD1322_SETVCOMH 0xBE
#define SSD1322_NORMALDISPLAY 0xA6
#define SSD1322_INVERSEDISPLAY 0xA7
#define SSD1322_SETMUXRATIO 0xCA
#define SSD1322_SETCOLUMNADDR 0x15
#define SSD1322_SETROWADDR 0x75
#define SSD1322_WRITERAM 0x5C
#define SSD1322_ENTIREDISPLAYON 0xA5
#define SSD1322_ENTIREDISPLAYOFF 0xA4
#define SSD1322_SETGPIO 0xB5
#define SSD1322_EXITPARTIALDISPLAY 0xA9
#define SSD1322_SELECTDEFAULTGRAYSCALE 0xB9

#define MIN_SEG 0x1C
#define MAX_SEG 0x5B

// Scrolling #defines
#define SSD1322_ACTIVATE_SCROLL 0x2F
#define SSD1322_DEACTIVATE_SCROLL 0x2E
#define SSD1322_SET_VERTICAL_SCROLL_AREA 0xA3
#define SSD1322_RIGHT_HORIZONTAL_SCROLL 0x26
#define SSD1322_LEFT_HORIZONTAL_SCROLL 0x27
#define SSD1322_VERTICAL_AND_RIGHT_HORIZONTAL_SCROLL 0x29
#define SSD1322_VERTICAL_AND_LEFT_HORIZONTAL_SCROLL 0x2A

/*=========================================================================
    SH1106 Displays
    -----------------------------------------------------------------------
    The driver is used in multiple displays (128x64, 128x32, etc.).
=========================================================================*/
#define SH1106_Set_Page_Address 0xB0

class ArduiPi_OLED : public Adafruit_GFX {
  public:
    ArduiPi_OLED();

    // SPI Init
    bool init(int8_t DC, int8_t RST, int8_t CS, uint8_t OLED_TYPE);

    // I2C Init
    bool init(int8_t RST, uint8_t OLED_TYPE, int8_t i2c_addr);
    bool init(int8_t RST, uint8_t OLED_TYPE);

    bool
    oled_is_spi_proto(uint8_t OLED_TYPE); /* to know protocol before init */
    bool select_oled(uint8_t OLED_TYPE, int8_t i2c_addr = 0);

    void begin(void);
    void close(void);

    void sendCommand(uint8_t c);
    void sendCommand(uint8_t c0, uint8_t c1);
    void sendCommand(uint8_t c0, uint8_t c1, uint8_t c2);
    void sendData(uint8_t c);

    void clearDisplay(void);
    void setGrayLevel(uint8_t grayLevel);
    void setBrightness(uint8_t Brightness);
    void invertDisplay(uint8_t i);
    void display(void);

    void setSeedTextXY(unsigned char Row, unsigned char Column);
    void putSeedChar(char C);
    void putSeedString(const char *String);

    int16_t getOledWidth(void);
    int16_t getOledHeight(void);
    int8_t getOledAddress(void);

    void startscrollright(uint8_t start, uint8_t stop);
    void startscrollleft(uint8_t start, uint8_t stop);

    void startscrolldiagright(uint8_t start, uint8_t stop);
    void startscrolldiagleft(uint8_t start, uint8_t stop);
    void setHorizontalScrollProperties(bool direction, uint8_t startRow,
                                       uint8_t endRow, uint8_t startColumn,
                                       uint8_t endColumn, uint8_t scrollSpeed);
    void stopscroll(void);

    void drawPixel(int16_t x, int16_t y, uint16_t color);

  private:
    uint8_t *poledbuff; // Pointer to OLED data buffer in memory
    int8_t _i2c_addr, dc, rst, cs;

    void displayOn(void);
    void displayOff(void);
    int16_t oled_width, oled_height;
    int16_t oled_buff_size;
    uint8_t vcc_type;
    uint8_t oled_type;
    uint8_t grayH, grayL;

    inline bool isI2C(void);
    inline bool isSPI(void);
    void fastSPIwrite(uint8_t c);
    void fastSPIwrite(char *tbuf, uint32_t len);
    void fastI2Cwrite(uint8_t c);
    void fastI2Cwrite(char *tbuf, uint32_t len);
    void slowSPIwrite(uint8_t c);

    //volatile uint8_t *dcport;
    //uint8_t dcpinmask;
};
#endif
