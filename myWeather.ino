

/* The ESP32 has four SPi buses, however as of right now only two of
 * them are available to use, HSPI and VSPI. Simply using the SPI API
 * as illustrated in Arduino examples will use VSPI, leaving HSPI unused.
 *
 * However if we simply intialise two instance of the SPI class for both
 * of these buses both can be used. However when just using these the Arduino
 * way only will actually be outputting at a time.
 *
 * Logic analyser capture is in the same folder as this example as
 * "multiple_bus_output.png"
 *
 * created 30/04/2018 by Alistair Symonds
 */

 void Wait4Idle(uint32_t line);

#include <SPI.h>

// Define ALTERNATE_PINS to use non-standard GPIO pins for SPI bus
#define ALTERNATE_PINS

#define VSPI_MISO   2  // not used pick blue led
#define VSPI_MOSI   23 // 4
#define VSPI_SCLK   18 // 0
#define VSPI_SS     13 //33

#define VSPI_DC     22  // data or command
#define VSPI_BUSY   14
#define VSPI_RST    21
#define VSPI_POWER  26

#if CONFIG_IDF_TARGET_ESP32S2 || CONFIG_IDF_TARGET_ESP32S3
#define VSPI FSPI
#endif

#define Debug printf
static const int spiClk = 1000000; // 1 MHz

//uninitalised pointers to SPI objects
SPIClass * vspi = NULL;

void setup() {
    Serial.begin(115200);
    //initialise two instances of the SPIClass attached to VSPI and HSPI respectively
    vspi = new SPIClass(VSPI);

    //clock miso mosi ss

    //alternatively route through GPIO pins of your choice
    vspi->begin(VSPI_SCLK, VSPI_MISO, VSPI_MOSI, VSPI_SS); //SCLK, MISO, MOSI, SS

    //set up slave select pins as outputs as the Arduino API
    //doesn't handle automatically pulling SS low
    pinMode(vspi->pinSS(), OUTPUT); //VSPI SS

    pinMode(VSPI_POWER, OUTPUT);    // reset ?
    pinMode(VSPI_RST, OUTPUT);    // reset ?
    pinMode(VSPI_DC, OUTPUT);     // set data/command pin to output mode
    pinMode(VSPI_BUSY, INPUT);    // set data/command pin to output mode
    printf("\n");

    displayInit();
    printf("%s:%d\n", __FUNCTION__, __LINE__);

    Wait4Idle(__LINE__);

    displayClearBlack();
    printf("%s:%d\n", __FUNCTION__, __LINE__);
    Wait4Idle(__LINE__);
    delay(5000);

    displayClearRed();
    printf("%s:%d\n", __FUNCTION__, __LINE__);
    Wait4Idle(__LINE__);
    delay(1000);

    Serial.println();
    Serial.println("HI MOM");
    printf("built on %s %s\n", __DATE__, __TIME__);

    printf("shutting down in 10 seconds\n");
    displaySleep();

}

// the loop function runs over and over again until power down or reset
void loop() {
  //use the SPI buses
  //sendData(0b00000101); // junk data to illustrate usage
  delay(100);
}
//---------------------------------------------------------------------------------------

void doSpiXfer(SPIClass *spi, uint8_t data)
{
  //use it as you would the regular arduino SPI API
  spi->beginTransaction(SPISettings(spiClk, MSBFIRST, SPI_MODE0));
  digitalWrite(spi->pinSS(), LOW); //pull SS slow to prep other end for transfer
  spi->transfer(data);
  digitalWrite(spi->pinSS(), HIGH); //pull ss high to signify end of data transfer
  spi->endTransaction();
}
//---------------------------------------------------------------------------------------
void sendCommand(uint8_t data)
{
    //printf("%s %d 0x%X\n", __FUNCTION__, data, data);
    digitalWrite(VSPI_DC, 0);
    doSpiXfer(vspi,data);
    //digitalWrite(VSPI_DC, 1); // not really needed
}
void sendData(uint8_t data)
{
    //digitalWrite(VSPI_DC, 0);  // debug only for scope (not really needed)
    digitalWrite(VSPI_DC, 1);
    //printf("%s %d 0x%X\n", __FUNCTION__, data, data);
    doSpiXfer(vspi,data);
}


//---------------------------------------------------------------------------------------

/*****************************************************************************
* | File      	:	EPD_7IN5B_V2.c
* | Author      :   Waveshare team
* | Function    :   Electronic paper driver
* | Info        :
*----------------
* |	This version:   V1.0
* | Date        :   2020-11-30
* | Info        :
******************************************************************************
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documnetation files(the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to  whom the Software is
# furished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included in
# all copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS OR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
# THE SOFTWARE.
#
******************************************************************************/
#include "EPD_7in5b_V2.h"
//#include "Debug.h"

/******************************************************************************
function :	Software reset
parameter:
******************************************************************************/
static void displayReset(void)
{
    digitalWrite(VSPI_RST, 1);
    delay(200);
    digitalWrite(VSPI_RST, 0);
    delay(2);
    digitalWrite(VSPI_RST, 1);
    delay(200);
    Wait4Idle(__LINE__);
}

/******************************************************************************
function :	send command
parameter:
     Reg : Command register
******************************************************************************/
//static void EPD_7IN5B_V2_SendCommand(UBYTE Reg)/
//{
//    digitalWrite(EPD_DC_PIN, 0);
//    digitalWrite(EPD_CS_PIN, 0);
//    sendCommand(Reg);
//    digitalWrite(EPD_CS_PIN, 1);
//}

/******************************************************************************
function :	send data
parameter:
    Data : Write data
******************************************************************************/
//static void EPD_7IN5B_V2_SendData(UBYTE Data)
//{
//    digitalWrite(EPD_DC_PIN, 1);
//   digitalWrite(EPD_CS_PIN, 0);
//    sendCommand(Data);
//    digitalWrite(EPD_CS_PIN, 1);
//}

/******************************************************************************
function :	Wait until the busy_pin goes LOW
parameter:
******************************************************************************/
/*
Note 1.5-4:
This pin (BUSY_N) is BUSY_N state output pin. When BUSY_N is low,
the operation of chip should not be interrupted and any commands should not be
issued to the module. The driver IC will put BUSY_N pin low when the driver
IC is
working such as:
-Outputting display waveform; or
-Programming with OTP
-Communicating with digital temperature sensor
*/

void Wait4Idle(uint32_t line)
{
    bool pin;
    pin = digitalRead(VSPI_BUSY);
    Debug("%s ENTER @ %d pin = %d\r\n", __FUNCTION__, line, pin);

	do	{
		delay(20);
        pin = digitalRead(VSPI_BUSY);
	}while(!pin);

	delay(20);
	Debug("%s EXIT at %d pin = %d\r\n", __FUNCTION__,  line, pin);
}


/******************************************************************************
function :	Turn On Display
parameter:
******************************************************************************/
static void displayTurnOn(void)
{
    sendCommand(0x12);	//DISPLAY REFRESH
    delay(100);	        //!!!The delay here is necessary, 200uS at least!!!
    Wait4Idle(__LINE__);
}

/******************************************************************************
function :	Initialize the e-Paper register
parameter:
******************************************************************************/
uint8_t displayInit(void)
{

    digitalWrite(VSPI_POWER, 1);     // apply power to the display
    delay(300);

    displayReset();

    sendCommand(0x01);			//POWER SETTING
    sendData(0x07);
    sendData(0x07);    //VGH=20V,VGL=-20V
    sendData(0x3f);		//VDH=15V
    sendData(0x3f);		//VDL=-15V

    sendCommand(0x04); //POWER ON
    delay(100);
    Wait4Idle(__LINE__);

    sendCommand(0X00);	//PANNEL SETTING
    sendData(0x0F);     //KW-3f   KWR-2F	BWROTP 0f	BWOTP 1f

    sendCommand(0x61);  //tres
    sendData(0x03);		//source 800
    sendData(0x20);
    sendData(0x01);		//gate 480
    sendData(0xE0);

    sendCommand(0X15);
    sendData(0x00);

    sendCommand(0X50);	//VCOM AND DATA INTERVAL SETTING
    sendData(0x11);
    sendData(0x07);

    sendCommand(0X60);	//TCON SETTING
    sendData(0x22);

    sendCommand(0x65);  // Resolution setting
    sendData(0x00);
    sendData(0x00);//800*480
    sendData(0x00);
    sendData(0x00);

    return 0;
}

/******************************************************************************
function :	Clear screen
parameter:
******************************************************************************/
void displayClear(void)
{
    uint32_t Width, Height;
    Width =(EPD_7IN5B_V2_WIDTH % 8 == 0)?(EPD_7IN5B_V2_WIDTH / 8 ):(EPD_7IN5B_V2_WIDTH / 8 + 1);
    Height = EPD_7IN5B_V2_HEIGHT;

    uint32_t i;
    sendCommand(0x10);
    for(i=0; i<Width*Height; i++) {
        sendData(0xff);

    }
    sendCommand(0x13);
    for(i=0; i<Width*Height; i++)	{
        sendData(0x00);

    }
    displayTurnOn();
}

void displayClearRed(void)
{
    uint32_t Width, Height;
    Width =(EPD_7IN5B_V2_WIDTH % 8 == 0)?(EPD_7IN5B_V2_WIDTH / 8 ):(EPD_7IN5B_V2_WIDTH / 8 + 1);
    Height = EPD_7IN5B_V2_HEIGHT;

    uint32_t i;
    sendCommand(0x10);
    for(i=0; i<Width*Height; i++) {
        sendData(0xff);

    }
    sendCommand(0x13);
    for(i=0; i<Width*Height; i++)	{
        sendData(0xff);

    }
    displayTurnOn();
}

void displayClearBlack(void)
{
    uint32_t Width, Height;
    Width =(EPD_7IN5B_V2_WIDTH % 8 == 0)?(EPD_7IN5B_V2_WIDTH / 8 ):(EPD_7IN5B_V2_WIDTH / 8 + 1);
    Height = EPD_7IN5B_V2_HEIGHT;

    uint32_t i;
    sendCommand(0x10);
    for(i=0; i<Width*Height; i++) {
        sendData(0x00);

    }
    sendCommand(0x13);
    for(i=0; i<Width*Height; i++)	{
        sendData(0x00);

    }
    displayTurnOn();
}

/******************************************************************************
function :	Sends the image buffer in RAM to e-Paper and displays
parameter:
******************************************************************************/
void displayImages(const uint8_t *blackimage, const uint8_t *ryimage)
{
    uint32_t Width, Height;
    Width =(EPD_7IN5B_V2_WIDTH % 8 == 0)?(EPD_7IN5B_V2_WIDTH / 8 ):(EPD_7IN5B_V2_WIDTH / 8 + 1);
    Height = EPD_7IN5B_V2_HEIGHT;

 //send black data
    sendCommand(0x10);
    for (uint32_t j = 0; j < Height; j++) {
        for (uint32_t i = 0; i < Width; i++) {
            sendData(blackimage[i + j * Width]);
        }
    }

    //send red data
    sendCommand(0x13);
    for (uint32_t j = 0; j < Height; j++) {
        for (uint32_t i = 0; i < Width; i++) {
            sendData(~ryimage[i + j * Width]);
        }
    }
    displayTurnOn();
}

/******************************************************************************
function :	Enter sleep mode
parameter:
******************************************************************************/
void displaySleep(void)
{
    sendCommand(0X02);  	//power off
    Wait4Idle(__LINE__);
    sendCommand(0X07);  	//deep sleep
    sendData(0xA5);

    printf("%s sleeping\n", __FUNCTION__);
    digitalWrite(VSPI_POWER, 0);     // remove display power


}
