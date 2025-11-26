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
    
    unsigned int valor_adc = 0;
    unsigned int valor_switch = 0;
    float voltaje = 0;
    float array[4096];
    unsigned int valor_dac = 0;
    int flag=0;
    while (1) {
    	int i,j;
		valor_adc= *adc & 0xFFF; // lee los 12 bits del canal CH0
		voltaje= (valor_adc*4.096)/4095.0;
		if( flag == 0) {
		valor_switch= *sws;	
		for( i = 0; i < 4096; i++){
			float tiempo = (float) i /4095; // va de 0 a 1
			float onda = 0.0;
			if(valor_swtich > 0x1F && valor_switch < 0x3F){
				if (tiempo <= 0.5 ){
						onda = tiempo;
						array[i] = onda;
					}else{
						onda = 1.0 - tiempo;
						array[i] = onda;
					}
			}
			else{
				if(voltaje < 2.5){
					if (tiempo <= 0.5 ){
						onda = tiempo;
						array[i] = onda;
					}else{
						onda = 1.0 - (tiempo * tiempo);
						array[i] = onda;
					}
				}else{
					if (tiempo <= 0.5 ){
						onda = tiempo * tiempo;	
						array[i] = onda;
					}else{
						onda = (1.0 - tiempo);
						array[i] = onda;
					}
				}	
			}
			valor_dac = (unsigned int)(onda * 4095);
			*dac= valor_dac;
			if ((i%200) == 0){
				mostrar_voltaje(voltaje);
			}
			if(i == 4095)flag = 1;
			}
		}else{
			for(j=0;j<4096;j++){
				valor_dac = (unsigned int)(array[j]*4095);
				*dac = valor_dac;
			}
			if(valor_switch != *sws){
				flag = 0;
			}
			
		}
    }
  return 0;
}
