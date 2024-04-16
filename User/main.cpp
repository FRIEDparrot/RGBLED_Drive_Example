#include "stm32f10x.h"
#include "RGBLED.h"

// use usart3 (PB10, PB11 as the uart)
int main(){
    RGBLED_Init(3,RGB_RED);
	while(1){
        
    }
}
