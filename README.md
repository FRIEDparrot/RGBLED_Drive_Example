This project is an example for drive Common Cathode RGB LCD run on stm32f103c8t6 MCU platform 

the header are put in the HardWare\RGBLED folder. 

* Use RGBLED_Init(mode, color) for Initialize RGBLED on port PB6, PB7, PB8
* Use RGBLED_SetPrescaler(r,g,b) for adjusting the prescaler for each RGB component 

* 4 modes are available
MODE0: ONLY USE ONE COLOR LIGHT
MODE1: ONE COLOR LIGHT AND DIM
MODE2: MULTI COLOR LIGHT AND DIM
MODE3(BLEND): MULTI COLOR LIGHT TRANSFORM

* 7 Colors are available
RGB_RED       
RGB_YELLOW    
RGB_GREEN     
RGB_BLUE      
RGB_LIGHTBLUE 
RGB_LIGHTGEEEN
RGB_WHITE     

if you want to add more color, define it at RGBLED.h and change **RGBColorList** and **RGBColorList_Size** Parameter at RGBLED.cpp

this code use TIM4 and GPIOB - PB5, PB6, PB7 as the Auxiliary Output Compare pin and control the LED by Timer4 interruption

