/*****************************************************************************
* | File      	:   EPD_7in5b_V2_test.c
* | Author      :   Waveshare team
* | Function    :   5.83inch B&C e-paper test demo
* | Info        :
*----------------
* |	This version:   V1.0
* | Date        :   2020-11-30
* | Info        :
#
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documnetation files (the "Software"), to deal
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
#include <Arduino.h>
#include "EPD_Test.h"
#include "EPD_7in5b_V2.h"
#include <time.h>

int displayTest(void)
{
    Debug("epaper test by WaveShare\r\n");

    clearToAllWhite();
	delay(2000);

    //Create a new image cache named IMAGE_BW and fill it with white
    uint8_t *BlackImage, *RYImage;

    uint32_t Imagesize = ((EPD_7IN5B_V2_WIDTH % 8 == 0)? (EPD_7IN5B_V2_WIDTH / 8 ): (EPD_7IN5B_V2_WIDTH / 8 + 1)) * EPD_7IN5B_V2_HEIGHT;

    if((BlackImage = (uint8_t *)malloc(Imagesize)) == NULL) {
        Debug("Failed to apply for black memory...\r\n");
        return -1;
    }

    if((RYImage = (uint8_t *)malloc(Imagesize)) == NULL) {
        Debug("Failed to apply for red memory...\r\n");
        return -1;
    }

    Debug("NewImage:BlackImage and RYImage\r\n");
    Paint_NewImage(BlackImage, EPD_7IN5B_V2_WIDTH, EPD_7IN5B_V2_HEIGHT , 0, WHITE);
    Paint_NewImage(RYImage, EPD_7IN5B_V2_WIDTH, EPD_7IN5B_V2_HEIGHT , 0, WHITE);

    //Select Image
    Paint_SelectImage(BlackImage);
    Paint_Clear(WHITE);
    Paint_SelectImage(RYImage);
    Paint_Clear(WHITE);

#if 0   // show bmp
	# no sd card or fs on my system.
    Debug("show window BMP-----------------\r\n");
    Paint_SelectImage(BlackImage);
    GUI_ReadBmp("./pic/2in9.bmp", 50, 100);
    Paint_SelectImage(RYImage);
    Paint_Clear(WHITE);
    GUI_ReadBmp("./pic/2in9.bmp", 350, 100);
    displayImages(BlackImage, RYImage);
    delay(2000);

    Debug("show red bmp------------------------\r\n");
    Paint_SelectImage(BlackImage);
    GUI_ReadBmp("./pic/7in5_V2_b.bmp", 0, 0);
    Paint_SelectImage(RYImage);
    GUI_ReadBmp("./pic/7in5_V2_r.bmp", 0, 0);
    displayImages(BlackImage, RYImage);
	delay(2000);
#endif

#if 1   // show image for array
    Debug("enter: show image for array\r\n");
    displayImages(gImage_7in5_V2_b, gImage_7in5_V2_ry);
    delay(2000);
    Debug("exit : show image for array\r\n");
#endif

#if 0   // Drawing on the image
    /*Horizontal screen*/
    //1.Draw black image
    Paint_SelectImage(BlackImage);
    Paint_Clear(WHITE);
    Paint_DrawPoint(10, 80, BLACK, DOT_PIXEL_1X1, DOT_STYLE_DFT);
    Paint_DrawPoint(10, 90, BLACK, DOT_PIXEL_2X2, DOT_STYLE_DFT);
    Paint_DrawPoint(10, 100, BLACK, DOT_PIXEL_3X3, DOT_STYLE_DFT);
    Paint_DrawPoint(10, 110, BLACK, DOT_PIXEL_3X3, DOT_STYLE_DFT);
    Paint_DrawLine(20, 70, 70, 120, BLACK, DOT_PIXEL_1X1, LINE_STYLE_SOLID);
    Paint_DrawLine(70, 70, 20, 120, BLACK, DOT_PIXEL_1X1, LINE_STYLE_SOLID);
    Paint_DrawRectangle(20, 70, 70, 120, BLACK, DOT_PIXEL_1X1, DRAW_FILL_EMPTY);
    Paint_DrawRectangle(80, 70, 130, 120, BLACK, DOT_PIXEL_1X1, DRAW_FILL_FULL);
    Paint_DrawString_EN(10, 0, "waveshare", &Font16, BLACK, WHITE);
    Paint_DrawNum(10, 50, 987654321, &Font16, WHITE, BLACK);

    //2.Draw red image
    Paint_SelectImage(RYImage);
    Paint_Clear(WHITE);
    Paint_DrawCircle(160, 95, 20, BLACK, DOT_PIXEL_1X1, DRAW_FILL_EMPTY);
    Paint_DrawCircle(210, 95, 20, BLACK, DOT_PIXEL_1X1, DRAW_FILL_FULL);
    Paint_DrawLine(85, 95, 125, 95, BLACK, DOT_PIXEL_1X1, LINE_STYLE_DOTTED);
    Paint_DrawLine(105, 75, 105, 115, BLACK, DOT_PIXEL_1X1, LINE_STYLE_DOTTED);
    Paint_DrawString_EN(10, 20, "hello world", &Font12, WHITE, BLACK);
    Paint_DrawNum(10, 33, 123456789, &Font12, BLACK, WHITE);

    displayImages(BlackImage, RYImage);
    delay(2000);
#endif

    clearToAllWhite();

    free(BlackImage);
    free(RYImage);

    BlackImage = NULL;
    RYImage = NULL;

    return 0;
}

