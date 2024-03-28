

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
#include <SPI.h>

// Define ALTERNATE_PINS to use non-standard GPIO pins for SPI bus
#define ALTERNATE_PINS

#define VSPI_MISO   2  // not used pick blue led
#define VSPI_MOSI   23 // 4
#define VSPI_SCLK   18 // 0
#define VSPI_SS     13 //33

#define VSPI_DC     22  // data or command

#if CONFIG_IDF_TARGET_ESP32S2 || CONFIG_IDF_TARGET_ESP32S3
#define VSPI FSPI
#endif

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

  pinMode(VSPI_DC, OUTPUT); // set data/command pin to output mode

#if 0
  for (int i = 0; i < 1000000; i++)
    {
        digitalWrite(VSPI_DC, 0);
        delay(10);
        digitalWrite(VSPI_DC, 1);
        delay(10);
    }
#endif

  Serial.println();
  Serial.println("HI MOM");
  printf("built on %s %s\n", __DATE__, __TIME__);
}

// the loop function runs over and over again until power down or reset
void loop() {
  //use the SPI buses
  sendData(0b00000101); // junk data to illustrate usage
  delay(100);
}
//---------------------------------------------------------------------------------------

void doSpiXfer(SPIClass *spi, byte data)
{
  //use it as you would the regular arduino SPI API
  spi->beginTransaction(SPISettings(spiClk, MSBFIRST, SPI_MODE0));
  digitalWrite(spi->pinSS(), LOW); //pull SS slow to prep other end for transfer
  spi->transfer(data);
  digitalWrite(spi->pinSS(), HIGH); //pull ss high to signify end of data transfer
  spi->endTransaction();
}
//---------------------------------------------------------------------------------------
void sendCommand(byte data)
{
    digitalWrite(VSPI_DC, 0);
    doSpiXfer(vspi,data);
    digitalWrite(VSPI_DC, 1); // not really needed
}
void sendData(byte data)
{
    digitalWrite(VSPI_DC, 0);  // debug only for scope (not really needed)
    digitalWrite(VSPI_DC, 1);
    doSpiXfer(vspi,data);
}


//---------------------------------------------------------------------------------------

