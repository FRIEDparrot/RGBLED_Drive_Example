/* RGBLED_drive.cpp file */
#include "stdlib.h" // srand function
#include "RGBLED.h"

// Timer2 reload value
#define MAX_PWM_PERIOD 100

/* for adjust the mix proportion of each light */
static float RED_INTERNSE_PRESCALER   = 0.5;
static float GREEN_INTENSE_PERSCALER  = 0.5;
static float BLUE_INTENSE_PRESCALER   = 0.5;

/*  
    MODE0: ONLY USE ONE COLOR LIGHT
    MODE1: ONE COLOR LIGHT AND DIM
    MODE2: MULTI COLOR LIGHT AND DIM
    MODE3(BLEND): MULTI COLOR LIGHT TRANSFORM
*/
uint8_t RGBLED_MODE = 0;

/* @note: the method is compare with Compare register1 */
RGBColor RGBColorList[] = {RGB_RED, RGB_BLUE, RGB_GREEN, RGB_YELLOW, RGB_LIGHTBLUE, RGB_LIGHTGEEEN, RGB_WHITE};
uint8_t  RGBColorList_Size = 7;

// ****** used for light intense interpolation ***********
static float LIGHT_INTENSE_INTERP[100] = {0.00,0.01,0.03,0.04,0.05,0.06,0.07,0.08,0.09,0.10,0.10,0.12,0.13,0.14,0.15,0.17,0.18,0.20,0.22,0.24,0.26,0.29,0.33,0.37,0.42,0.47,0.52,0.57,0.61,0.66,0.70,0.73,0.77,0.81,0.84,0.87,0.90,0.92,0.94,0.96,0.97,0.98,0.98,0.99,0.99,0.99,1.00,1.00,1.00,1.00,1.00,1.00,1.00,1.00,1.00,0.99,0.99,0.98,0.98,0.97,0.96,0.94,0.93,0.91,0.89,0.86,0.84,0.81,0.77,0.73,0.69,0.65,0.60,0.55,0.50,0.44,0.39,0.34,0.29,0.23,0.21,0.19,0.17,0.16,0.14,0.13,0.12,0.11,0.10,0.09,0.08,0.07,0.06,0.06,0.05,0.04,0.04,0.03,0.02,0.01};
uint16_t LIGHT_INTENSE_R_BLEND[100];  // Red   Duty Array for blend 
uint16_t LIGHT_INTENSE_G_BLEND[100];  // Green Duty Array for blend 
uint16_t LIGHT_INTENSE_B_BLEND[100];  // Blue  Duty Array for blend

RGBColor curr_color = RGBColor(0,0,0); 
RGBColor next_color = RGBColor(0,0,0); // initialize by init function

uint16_t color_index1 = 0, color_index2 = 0;  // change light speed -> use color index2
uint8_t color_update_interval = 1;   // when color_index2 reach color_update_interval, color_index1++

/// @brief this function is called by mode3 in the interruption to calculate the color interpolation
void RGBColor_Blend_Update(void){
    for (int i = 0; i < 100; i++) {
        float coef = MAX_PWM_PERIOD/256.0;  uint16_t j = 100 - i;
        LIGHT_INTENSE_R_BLEND[i] = (int)( RED_INTERNSE_PRESCALER  * coef * (j * curr_color.R  + i * next_color.R)/100);
        LIGHT_INTENSE_G_BLEND[i] = (int)( GREEN_INTENSE_PERSCALER * coef * (j * curr_color.G  + i * next_color.G)/100);
        LIGHT_INTENSE_B_BLEND[i] = (int)( BLUE_INTENSE_PRESCALER  * coef * (j * curr_color.B  + i * next_color.B)/100);
    }
}

/* RGB LED Configuration Functions */

/// @brief Init GPIOB6, GPIOB7, GPIOB8 as output pins
void RGBLED_GPIO_Config(void){
    /*======================== GPIO Output Init =====================*/
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, FunctionalState::ENABLE);
    // Use PA1 as AFPP mode
    GPIO_InitTypeDef* GPIO_InitStruct = new GPIO_InitTypeDef();
    GPIO_InitStruct->GPIO_Mode = GPIOMode_TypeDef::GPIO_Mode_AF_PP;
    GPIO_InitStruct->GPIO_Pin = GPIO_Pin_6 | GPIO_Pin_7 | GPIO_Pin_8;
    GPIO_InitStruct->GPIO_Speed = GPIOSpeed_TypeDef::GPIO_Speed_50MHz;
    GPIO_Init(GPIOB, GPIO_InitStruct);
}

/// @brief Use Timer4 as the Timer for RGB LED -> Output Compare Mode in CH 0,1,2,3 channels for R, G, B */
void RGBLED_Timer_Config(void){
    /*======================= Timer Base Init ======================*/
    // Init TIM4 TimeBase Unit -> USE  CH2, CH3, CH4 all 3 ports, The Period is 50ms for smooth transform 
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM4, FunctionalState::ENABLE);

    TIM_DeInit(TIM4);
    TIM_InternalClockConfig(TIM4);                                  // use Internal clock as clock source;
    TIM_TimeBaseInitTypeDef* TIM4_TimeBaseInitStruct = new TIM_TimeBaseInitTypeDef();
    TIM_TimeBaseStructInit(TIM4_TimeBaseInitStruct);                   // counter: up
    TIM4_TimeBaseInitStruct->TIM_CounterMode   = TIM_CounterMode_Up;   
    TIM4_TimeBaseInitStruct->TIM_Prescaler     = 7200 - 1;             // 0.1ms per tick
    TIM4_TimeBaseInitStruct->TIM_Period        = MAX_PWM_PERIOD - 1;   // 0.01s per cycle
    TIM_TimeBaseInit(TIM4, TIM4_TimeBaseInitStruct);
    /* Init OC2 for RED */   /*@note: use the Channel 2 as port, use TIM_OC2Init and TIM_SetCompare2*/
    TIM_OCInitTypeDef* TIM4_OCInitStruct = new TIM_OCInitTypeDef();  // Output Compare Mode -> used for generate PWM wave
    TIM_OCStructInit(TIM4_OCInitStruct);
    TIM4_OCInitStruct -> TIM_OCMode     = TIM_OCMode_PWM1;          // TIMx_CNT < TIMx_CCRx high, otherwise low;
    TIM4_OCInitStruct -> TIM_OCPolarity = TIM_OCPolarity_High;      // CCER -> CC2P 
    TIM4_OCInitStruct -> TIM_OutputState= TIM_OutputState_Enable;   // TIM2 Output in CH2
    TIM4_OCInitStruct -> TIM_Pulse      = 0;                        // pulse value in Capture Compare Register.(CCR2)
    TIM_OC1Init(TIM4, TIM4_OCInitStruct);                           // output Channel2 (TIM2_CH2, PA1);
    TIM_OC2Init(TIM4, TIM4_OCInitStruct);    // output Channel3 (TIM2_CH3, PA2);
    TIM_OC3Init(TIM4, TIM4_OCInitStruct);    // output Channel4 (TIM2_CH4, PA3); -> all use one struct for init
    
    // Use Timer as Interrupt source for the array update
    if (RGBLED_MODE == 0){
        TIM_ITConfig(TIM4,  TIM_IT_Update, FunctionalState::DISABLE);
        NVIC_InitTypeDef* NVIC_InitStruct = new NVIC_InitTypeDef();
        NVIC_InitStruct->NVIC_IRQChannel = TIM4_IRQn;
        NVIC_InitStruct->NVIC_IRQChannelPreemptionPriority = 2;
        NVIC_InitStruct->NVIC_IRQChannelSubPriority = 1;
        NVIC_InitStruct->NVIC_IRQChannelCmd = FunctionalState::DISABLE;
        NVIC_Init(NVIC_InitStruct);

        // for mode0, initialize the init Duty
        float coef = MAX_PWM_PERIOD /256.0;
        TIM_SetCompare1(TIM4, (int)(RED_INTERNSE_PRESCALER * curr_color.R * coef));
        TIM_SetCompare2(TIM4, (int)(GREEN_INTENSE_PERSCALER * curr_color.G * coef));
        TIM_SetCompare3(TIM4, (int)(BLUE_INTENSE_PRESCALER * curr_color.B * coef));
    }
    else{
        TIM_ITConfig(TIM4,  TIM_IT_Update, FunctionalState::ENABLE);
        /* NVIC Configurations */
        NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
        NVIC_InitTypeDef* NVIC_InitStruct = new NVIC_InitTypeDef();
        NVIC_InitStruct->NVIC_IRQChannel = TIM4_IRQn;
        NVIC_InitStruct->NVIC_IRQChannelPreemptionPriority = 2;
        NVIC_InitStruct->NVIC_IRQChannelSubPriority = 1;
        NVIC_InitStruct->NVIC_IRQChannelCmd = FunctionalState::ENABLE;

        NVIC_Init(NVIC_InitStruct);
    }
    TIM_Cmd(TIM4, FunctionalState::ENABLE);
}

// for mode 1, 2, 3, need interruption
extern "C"{
    // note: use SetCurrentDataCounter in Interruption for sync all DMAs
    void TIM4_IRQHandler(void){
        if (TIM_GetITStatus(TIM4, TIM_IT_Update) == SET){
            // calculate color index
            if (color_index2 < color_update_interval){
                color_index2++;
            }else{
                color_index2 = 0;
                color_index1++;
                if (color_index1 >= 100){
                    color_index1 = 0;
                    // for mode 2 and 3, generate a new color
                    if (RGBLED_MODE == 2 || RGBLED_MODE == 3){
                        curr_color = next_color;
                        float r = (float)rand()/RAND_MAX * RGBColorList_Size;
                        uint16_t index = (int)r;
                        next_color = RGBColorList[index];
                    }
                    if (RGBLED_MODE == 3){
                        RGBColor_Blend_Update();
                    }
                }
            }
            // set the current color 
            switch (RGBLED_MODE)
            {
            case 1: // follow case 2 
            case 2: {
                float coef = LIGHT_INTENSE_INTERP[color_index1] * MAX_PWM_PERIOD/256.0;
                TIM_SetCompare1(TIM4, (int)(coef * curr_color.R * RED_INTERNSE_PRESCALER));
                TIM_SetCompare2(TIM4, (int)(coef * curr_color.G * GREEN_INTENSE_PERSCALER));
                TIM_SetCompare3(TIM4, (int)(coef * curr_color.B * BLUE_INTENSE_PRESCALER));
                break;
                }
            case 3:{
                TIM_SetCompare1(TIM4, LIGHT_INTENSE_R_BLEND[color_index1]);
                TIM_SetCompare2(TIM4, LIGHT_INTENSE_G_BLEND[color_index1]);
                TIM_SetCompare3(TIM4, LIGHT_INTENSE_B_BLEND[color_index1]);
                break;
                }
            default: break;
            }
        }
        TIM_ClearITPendingBit(TIM4, TIM_IT_Update);
    }
}

/// @brief Set Prescaler of duty of each color element 
/// @param R_presc 
/// @param G_presc 
/// @param B_presc 
void RGBLED_SetPrescaler(uint16_t R_presc, uint16_t G_presc, uint16_t B_presc){
    RED_INTERNSE_PRESCALER  = R_presc;
    GREEN_INTENSE_PERSCALER = G_presc;
    BLUE_INTENSE_PRESCALER  = B_presc;
}

/// @brief Use this function for start RGBLED or restart it as another mode 
/// @param mode choose from 0, 1, 2, 3 to initialize as different mode
/// @param color color at startup, refer to RGB_Color_Type group at RGBLED.h
void RGBLED_Init(uint8_t mode, RGBColor color){
    RGBLED_MODE = mode;

    RGBLED_GPIO_Config();
    TIM_Cmd(TIM4, FunctionalState::DISABLE);
    color_index1 = 0; color_index2 = 0;
    if (RGBLED_MODE == 0 || RGBLED_MODE == 1){
        curr_color = color;
    }else{
        curr_color = RGBColor(0,0,0);
    }
    next_color = color;
    RGBLED_Timer_Config();
}

// after stop, the GPIOB is still AFPP mode;
void RGBLED_Stop(){
    TIM_Cmd(TIM4, FunctionalState::DISABLE);
    TIM_ITConfig(TIM4,  TIM_IT_Update, FunctionalState::DISABLE);
    GPIO_ResetBits(GPIOB, GPIO_Pin_6 | GPIO_Pin_7 | GPIO_Pin_8);
    NVIC_InitTypeDef* NVIC_InitStruct = new  NVIC_InitTypeDef();
    NVIC_InitStruct->NVIC_IRQChannel = TIM4_IRQn;
    NVIC_InitStruct->NVIC_IRQChannelPreemptionPriority = 2;
    NVIC_InitStruct->NVIC_IRQChannelSubPriority = 1;
    NVIC_InitStruct->NVIC_IRQChannelCmd = FunctionalState::DISABLE;
    NVIC_Init(NVIC_InitStruct);

    RCC_APB2PeriphClockCmd(RCC_APB1Periph_TIM2, FunctionalState::DISABLE);
}
