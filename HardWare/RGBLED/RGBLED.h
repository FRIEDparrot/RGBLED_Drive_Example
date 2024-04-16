#ifndef __RGBLED__H
#define __RGBLED__H
/* 
    Date: 2024/4.16 03:18 P.M.
    Author: FriedParrot 
    Run at Platform: stm32f103c8t6 

    * Use RGBLED_Init(mode, color) for Initialize RGBLED on port PB6, PB7, PB8
    * Use RGBLED

    * 4 modes are available
    MODE0: ONLY USE ONE COLOR LIGHT
    MODE1: ONE COLOR LIGHT AND DIM
    MODE2: MULTI COLOR LIGHT AND DIM
    MODE3(BLEND): MULTI COLOR LIGHT TRANSFORM
*/
#include "stm32f10x.h"

//******   Color Type Definition     ****** 
struct RGBColor{
    uint8_t R = 0;
    uint8_t G = 0;
    uint8_t B = 0;
    RGBColor(uint8_t R, uint8_t G, uint8_t B){
        this->R = R;
        this->G = G;
        this->B = B;
    }
};

// @defgroup RGB_Color_Type {}
#define RGB_RED        RGBColor(255, 0  , 0  )
#define RGB_YELLOW     RGBColor(255, 255, 0  )
#define RGB_GREEN      RGBColor(0  , 128, 0  )
#define RGB_BLUE       RGBColor(0  , 0  , 128)
#define RGB_LIGHTBLUE  RGBColor(51 , 102, 255)
#define RGB_LIGHTGEEEN RGBColor(204, 255, 204)
#define RGB_WHITE      RGBColor(255, 255, 255)


void RGBLED_Init(uint8_t mode, RGBColor color);
void RGBLED_SetPrescaler(uint16_t R_presc, uint16_t G_presc, uint16_t B_presc);

#endif // !__RGBLED__H
