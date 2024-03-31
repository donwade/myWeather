

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

void Wait4Idle(void);
extern int displayTest(void);
#define LINE  Debug("%s:%d\n", __FUNCTION__, __LINE__)

#include <SPI.h>

#define DEBUG 1
#include "Debug.h"
#define NS  1000000000ULL // ns in a second

// Define ALTERNATE_PINS to use non-standard GPIO pins for SPI bus
#define ALTERNATE_PINS

#define VSPI_MISO   2  // not used pick blue led
#define VSPI_MOSI   23 // 4
#define VSPI_SCLK   18 // 0
#define VSPI_SS     13 //33

#define VSPI_DC     22  // data or command
#define VSPI_BUSY   14  //14
#define VSPI_RST    21
#define VSPI_POWER  26

#if CONFIG_IDF_TARGET_ESP32S2 || CONFIG_IDF_TARGET_ESP32S3
#define VSPI FSPI
#error what does this mean?
#endif

#define Debug printf
static const int spiClk = 1000000; // 2MHz=no change 10Mhz=FAIL

//uninitalised pointers to SPI objects
SPIClass * vspi = NULL;

void setup() {
    Serial.begin(115200);
    delay(100);
    Serial.println("");
    Serial.println("----------------------------------");
    Serial.println("start");

    WAIT4USER();
    #if 1
        adcAttachPin(VSPI_BUSY);
    #else
        pinMode(VSPI_BUSY, INPUT);    // set data/command pin to output mode
    #endif

    WAIT4USER();

    pinMode(VSPI_POWER, OUTPUT);    // apply power pin
    WAIT4USER();

    digitalWrite(VSPI_POWER, 1);// apply power to the display
    WAIT4USER();

    pinMode(VSPI_RST, OUTPUT);    // reset ?
    digitalWrite(VSPI_RST, 1);    // into reset
    delay(2);
    digitalWrite(VSPI_RST, 0);    // out of reset
    WAIT4USER();

    pinMode(VSPI_DC, OUTPUT);     // set data/command pin to output mode
    WAIT4USER();


    //initialise two instances of the SPIClass attached to VSPI and HSPI respectively
    vspi = new SPIClass(VSPI);
    Debug("verify that this is NOT 14   xxx = %d\n", vspi->pinSS());

    WAIT4USER();

    //clock miso mosi ss

    //alternatively route through GPIO pins of your choice
    vspi->begin(VSPI_SCLK, VSPI_MISO, VSPI_MOSI, VSPI_SS); //SCLK, MISO, MOSI, SS

    //set up slave select pins as outputs as the Arduino API
    //doesn't handle automatically pulling SS low
    pinMode(vspi->pinSS(), OUTPUT); //VSPI SS

    Debug("verify that this is NOT 14   xxx = %d\n", vspi->pinSS());
    WAIT4USER();

    Debug("hw init done\n");
    WAIT4USER();

    while(true)
    {

        displayInit();
        Debug("%s:%d\n", __FUNCTION__, __LINE__);

        clearToAllWhite();
        delay(3000);

        clearToAllBlack();
        delay(3000);

        clearToAllRed();
        delay(3000);

        Debug("built on %s %s\n", __DATE__, __TIME__);

        displayTest();   // call Waveshare test routines

        Debug("power shut down in 10 seconds\n");
        displaySleep();
        delay(10000);


        Debug("here we go again....\n");
     }
}

//---------------------------------------------------------------------------------------

int keypress(const char *file, int lineno)
{
    printf("waiting for keypress at %s:%d\n", file, lineno);
    while (!Serial.available())
    {
        delay(1);
    }

    int key =  (Serial.read());
    Serial.println("");
    return key;

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
    static uint32_t cnt;
    cnt++;
    char *msg;

    digitalWrite(VSPI_DC, 0);   // set to 0 for command transfer

    //while( !digitalRead(VSPI_BUSY)){}; //prevent sending if busy
    Wait4Idle();

    doSpiXfer(vspi,data);

    switch(data)
    {
        case 0:
            msg = "(PSR) panel setting register";
        break;

        case 1:
            msg = "(PWR) panel setting";
        break;

        case 2:
            msg = "(PWR) power setting";
        break;

        case 4:
            msg = "(PFS) power off seequence";
        break;

        case 7:
            msg = "(DSLP) deep sleep";
        break;


        case 0x10:
            msg = "(DTM1)start data xmit black buffer";
        break;

        case 0x12:
             msg = "(DRF) display refresh";
        break;

        case 0x13:
            msg = "(DTM2) start data xmit red buffer";
        break;

        case 0x50:
             msg = "(CDI) vert horiz timing";
        break;

        case 0x60:
            msg = "(TCON) gates";
        break;

        case 0x61:
            msg = "(TRES) resolution setting";
        break;

        case 0x65:
            msg = "(GSST) gate start setting";
        break;

        default:
            msg = "(UNKNOWN)";
        break;
    }
    Debug ("\t\t%s 0x%02X %s\n", __FUNCTION__, data, msg);

}
void sendData(uint8_t data)
{
    digitalWrite(VSPI_DC, 1);  // set to 1 for data transfer

    Wait4Idle();
    //while( !digitalRead(VSPI_BUSY)){}; //prevent sending if busy

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
}

/******************************************************************************
function :	send command
parameter:
     Reg : Command register
******************************************************************************/
//static void EPD_7IN5B_V2_SendCommand(uint8_t Reg)/
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
//static void EPD_7IN5B_V2_SendData(uint8_t Data)
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

#define MAX_VCC 3.1
#define VOLT2BINARY(x)  ((x) / MAX_VCC * 4096.)
#define BINARY2VOLT(x)  ( (float)(x) / 4096. * MAX_VCC)

static uint16_t peak_lo = 4096;
static uint16_t peak_hi = 0;
static uint16_t slice = VOLT2BINARY(.8);

void Wait4Idle(void)
{
#if 0
    struct timespec start={0,0}, finish={0,0};
    clock_gettime(CLOCK_REALTIME,&start);

    int32_t cnt = 0;
    bool pin;

    if (digitalRead(VSPI_BUSY)) return;

    // busy on entry.... profile it.
    if (digitalRead(VSPI_BUSY) != 1)
    {
    	do	{
    		delay(1);
            pin = digitalRead(VSPI_BUSY);
    	}while(!pin);
    }

    clock_gettime(CLOCK_REALTIME,&finish);

    Debug("busy for %ld mS\r\n",( (finish.tv_sec - start.tv_sec) + 1.0e-9 * (finish.tv_nsec  -
    start.tv_nsec)) / 1000);
#else
    uint16_t adc;
    uint16_t p2p;

    do {
        adc = analogRead(VSPI_BUSY);

        if (adc > peak_hi)
        {
            peak_hi = adc; //+= VOLT2BINARY(.1);
            p2p = (peak_hi - peak_lo) / 4;
            slice = peak_lo + p2p;   // noise is on the first half, move lower

            Debug("moved hi = %3.2fv            slice = %3.2f\n",
            BINARY2VOLT(peak_hi), BINARY2VOLT(slice));
            delay(500);
        }
        if (adc < peak_lo)
        {
            peak_lo = adc; //-= VOLT2BINARY(.1); // --;  //= adc;
            p2p = (peak_hi - peak_lo) / 4;
            slice = peak_lo + p2p;   // noise is on the first half, move lower

            Debug("moved =          lo =%3.2fv  slice = %3.2f\n",
            BINARY2VOLT(peak_lo), BINARY2VOLT(slice));
            delay(500);
        }

    } while (adc < slice);

/*
    printf("peak_lo = %3.2fv hi = %3.2fv adc > slice (%3.2fv > %3.2fv)\n",
        BINARY2VOLT(peak_lo),
        BINARY2VOLT(peak_hi),
        BINARY2VOLT(adc),
        BINARY2VOLT(slice)
    );
*/
#endif
}


/******************************************************************************
function :	Turn On Display
parameter:
******************************************************************************/
static void displayTurnOn(void)
{
    digitalWrite(VSPI_POWER, 1);// ensure power to the display
    Debug ("\n%s : start painting hardware\n", __FUNCTION__);

    struct timespec start, finish;
    clock_gettime(CLOCK_REALTIME, &start);

    sendCommand(0x12);	        //DISPLAY REFRESH


    // allow the display to paint, it can take up to 26 seconds.
    // this way when returning to caller, any "delay calls" will be from the
    // time the display is finished physically updating.

    Wait4Idle();                //let the display paint!!!

    clock_gettime(CLOCK_REALTIME, &finish);
    unsigned long long elapsed = ( (finish.tv_sec * NS + finish.tv_nsec)  - (start.tv_sec * NS + start.tv_nsec));

    Debug("\t\t>>>>>>>>>>>> %s : repainted in %lu mS\n\n", __FUNCTION__ , elapsed/1000000UL);

}

/******************************************************************************
function :	Initialize the e-Paper register
parameter:
******************************************************************************/
uint8_t displayInit(void)
{

    digitalWrite(VSPI_POWER, 1);     // apply power to the display
    WAIT4USER();
    delay(300);

    displayReset();
    WAIT4USER();

    Debug("starting init byte by byte\n");
    sendCommand(0x01);			//POWER SETTING
    sendData(0x07);
    sendData(0x07);    //VGH=20V,VGL=-20V
    sendData(0x3f);		//VDH=15V
    sendData(0x3f);		//VDL=-15V

    sendCommand(0x04); //POWER ON
    delay(100);

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

void setRedBuffer(uint8_t brightness) // 0 = white
{
    uint32_t Width, Height;
    Debug("%s set red intensity to %d\n", __FUNCTION__, brightness);

    Width =(EPD_7IN5B_V2_WIDTH % 8 == 0)?(EPD_7IN5B_V2_WIDTH / 8 ):(EPD_7IN5B_V2_WIDTH / 8 + 1);
    Height = EPD_7IN5B_V2_HEIGHT;

    // red buffer to red intensity
    sendCommand(0x13);
    for(int i=0; i<Width*Height; i++)	{
        sendData(brightness);

    }
}

void setWhiteBuffer(uint8_t brightness) // 0 = white
{
    uint32_t Width, Height;
    Debug("%s set white intensity to %d\n", __FUNCTION__, brightness);

    struct timespec start={0,0}, finish={0,0};
    clock_gettime(CLOCK_REALTIME,&start);

    Width =(EPD_7IN5B_V2_WIDTH % 8 == 0)?(EPD_7IN5B_V2_WIDTH / 8 ):(EPD_7IN5B_V2_WIDTH / 8 + 1);
    Height = EPD_7IN5B_V2_HEIGHT;

    // black buffer to black intensity
    sendCommand(0x10);
    for(int i=0; i<Width*Height; i++)	{
        sendData(brightness);

    }

    clock_gettime(CLOCK_REALTIME,&finish);
    unsigned long long elapsed = ( (finish.tv_sec * NS + finish.tv_nsec)  - start.tv_sec * NS + start.tv_nsec);
    Debug("\t\t>>>>>>>>>>>> %s : busy for %lu mS\r\n", __FUNCTION__ , elapsed/1000000UL);

}


void clearToAllRed(void)
{
    Debug("%s:%d\n", __FUNCTION__, __LINE__);

    setWhiteBuffer(0x00);
    setRedBuffer(0xFF);

    Debug("red on for 2 seconds .... %s:%d\n", __FUNCTION__, __LINE__);
    displayTurnOn();
    delay(2000);
}

void clearToAllWhite(void)
{
    Debug("%s:%d\n", __FUNCTION__, __LINE__);

    setWhiteBuffer(0xFF);
    setRedBuffer(0x00);

    displayTurnOn();
    delay(2000);
}

void clearToAllBlack(void)
{
    Debug("%s:%d\n", __FUNCTION__, __LINE__);

    setWhiteBuffer(0x00);
    setRedBuffer(0x00);

    displayTurnOn();
    delay(2000);
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
    sendCommand(0X07);  	//deep sleep
    sendData(0xA5);

    Debug("%s h/w power down\n", __FUNCTION__);
    digitalWrite(VSPI_POWER, 0);     // remove display power

}
