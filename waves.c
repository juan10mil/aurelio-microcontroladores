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
float array[2048]; //carga
float array2[2048]; //descarga
float array3[4096]; //recta
float onda = 0.0;
void mostrar_voltaje(float voltaje){
	unsigned int entero = (unsigned int) voltaje;
	unsigned int decimas = (unsigned int) ((voltaje - entero)*10)%10;
	unsigned int centesimas =(unsigned int) ((voltaje-entero)*100)%10;
	unsigned int milesimas = (unsigned int) ((voltaje - entero)* 1000)%10 ;
	unsigned int display = (entero << 12) | (decimas << 8) | (centesimas << 4) | milesimas;
	*led = display;
}

void carga(){
	int i;
	for(i = 0; i < 2048; i++){
	array[i]= 1-exp((-1)*i/512);
	}
}

void descarga(){
	int i;
	for(i = 0; i < 2048; i++){
	array2[i]= exp((-1)*i/512);
	}

}

void recta(){
	int i,j=0;
	float tiempo;
	for(i = 0; i < 2048; i++){
	tiempo = i /2047; // va de 0 a 0.5
	array3[i]= tiempo;
	}
	for(i = 2048; i < 4096; i++){
	tiempo = j /2047;
	array3[i]= 1- tiempo;
	j++;
	}
}

int main()
{
    printf("Hello from Nios II!\n");
    printf("EE604 Introduccion a Microcontroladores - 4to Laboratorio 2025-1\n");
    
    unsigned int valor_adc = 0;
    unsigned int valor_switch = 0;
    float voltaje = 0;
	
    unsigned int valor_dac = 0;
	descarga();
	carga();
int ciclos=0;
	recta();
int flag=0;
    while (1) {
    	int i,j;
	
	float tiempo;
		valor_adc= *adc & 0xFFF; // lee los 12 bits del canal CH0
		voltaje= (valor_adc*4.096)/4095.0;
		valor_switch= *sws;	
		if(valor_switch > 0x1F && valor_switch < 0x3F){
				if( flag==0) flag=1; // flag no volvera a ser cero
				if (flag!=1) ciclos=0; // reinicia el ciclo si viene de otro patron	
				flag=1; // asigna el flag al patron
				for(i = 0; i < 2048; i++){			
				onda = array3[i];// recta_posi
				valor_dac = (unsigned int)(onda * 4095);
				*dac= valor_dac;				
					}
				for(i = 2048; i < 4096; i++){				
				onda = array3[i];//recta_nega
				valor_dac = (unsigned int)(onda * 4095);
				*dac= valor_dac;
								
					}
				ciclos++;
			}
			else{
				if(voltaje < 2.5){
						
						if( flag==0) flag=2;
						if (flag!=2) ciclos=0;	
						flag=2;
						for(i = 0; i < 2048; i++){			
						onda = array3[i]; // recta_posi
						valor_dac = (unsigned int)(onda * 4095);
						*dac= valor_dac;				
						}
					
						for(i = 0; i < 2048; i++){
						onda = array2[i]; // descarga
						valor_dac = (unsigned int)(onda * 4095);
						*dac= valor_dac;
						}		
					ciclos++;

					}
				else{
					
						if( flag==0) flag=3;	
						if (flag!=3) ciclos=0;	
						flag=3;				
						for(i = 0; i < 2048; i++){
						onda = array[i]; // carga
						valor_dac = (unsigned int)(onda * 4095);
						*dac= valor_dac;
						}
						for(i = 2048; i < 4096; i++){				
						onda = array3[i]; //recta_nega
						valor_dac = (unsigned int)(onda * 4095);
						*dac= valor_dac;				
						}	
						ciclos++;
				}	
			}
			if(ciclos==200) {
				mostrar_voltaje(voltaje);
				ciclos=0;
				}
		
    }
  return 0;
}
