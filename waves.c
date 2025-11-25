#include <stdio.h>
#include <unistd.h>
#include <math.h>
#define ADC_ADDR 0x08001000
#define DAC_ADDR 0x08001020
#define LED_ADDR 0x08001030
#define SWS_ADDR 0x08001040

volatile unsigned int *adc = (unsigned int *) ADC_ADDR;
volatile unsigned int *dac = (unsigned int *) DAC_ADDR;
volatile unsigned int *led = (unsigned int *) LED_ADDR;
volatile unsigned int *sws = (unsigned int *) SWS_ADDR;
void mostrar_voltaje(float voltaje){
	unsigned int entero = (unsigned int) voltaje;
	unsigned int decimas = (unsigned int) ((voltaje - entero)*10)%10;
	unsigned int centesimas =(unsigned int) ((voltaje-entero)*100)%10;
	unsigned int milesimas = (unsigned int) ((voltaje - entero)* 1000)%10 ;
	unsigned int display = (entero << 12) | (decimas << 8) | (centesimas << 4) | milesimas;
	*led = display;
}
int main()
{
    printf("Hello from Nios II!\n");
    printf("EE604 Introduccion a Microcontroladores - 4to Laboratorio 2025-1\n");
    
    unsigned int valor_adc=0;
    unsigned int valor_switch=0;
    float voltaje =0;
    unsigned int valor_dac=0;
    while (1) {
    	int i;
		valor_adc= *adc & 0xFFF; // lee los 12 bits del canal CH0
		voltaje= (valor_adc*4.096)/4095.0;
		valor_switch= *sws;
		for( i=0; i< 4096; i++){
			float tiempo = (float) i /4095; // va de 0 a 1
			float onda =0.0;
			if (valor_switch >= 0x10){
				onda = sin(M_PI * tiempo);
			}
			else{
				if(voltaje < 2.5){
					if (tiempo <= 0.5 ){
						onda = sin(M_PI * tiempo);
					}else{
						float x = (tiempo - 0.5) * 2;
						onda = 1.0 - (x * x);
					}
				}else{
					if (tiempo <= 0.5 ){
						float x = tiempo* 2;
						onda = x * x;	
					}else{
						onda = sin(M_PI * (1.0 - tiempo));
					}
				}	
			}
			valor_dac = (unsigned int)(onda * 4095);
			*dac= valor_dac;
			if ((i%200) == 0){
				mostrar_voltaje(voltaje);
			}
			
		}
	
    }
  return 0;
}
