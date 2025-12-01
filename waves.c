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
int array[2048]; //carga
int array2[2048]; //descarga
int array3[4096]; //onda triangular
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
	float valor;
	for(i = 0; i < 2048; i++){
	valor = 1-exp((-1)*i/512.0);
	array[i] = (unsigned int)(valor * 4095);
	}
}

void descarga(){
	int i;
	float valor;
	for(i = 0; i < 2048; i++){
	valor = exp((-1)*i/512.0);
	array2[i] = (unsigned int)(valor * 4095);
	}

}

void recta(){
	int i,j=0;
	float tiempo,valor;
	for(i = 0; i < 2048; i++){
	tiempo = i /2047.0; // va de 0 a 1
	valor = tiempo * 4095;
	array3[i] = (unsigned int) valor;
	}
	for(i = 2048; i < 4096; i++){
	tiempo =  j /2047.0;
	valor = (1- tiempo)*4095; // va de (1 a 0 )*4095
	array3[i] = (unsigned int) valor;
	j++;
	}
}

int main()
{
    printf("Hello from Nios II!\n");
    printf("EE604 Introduccion a Microcontroladores - 4to Laboratorio 2025-2\n");
    unsigned int valor_switch = 0;
    unsigned int valor_dac = 0;
    unsigned int valor_adc = 0;
    float voltaje = 0;
	int ciclos=0;
	int flag=0;
	descarga();
	carga();
	recta();
	
    while (1) {
    int i,j;
	
	float tiempo;
		valor_adc= *adc & 0xFFF; // lee los 12 bits del canal CH0
		voltaje= (valor_adc*4.096)/4095.0;
		valor_switch= *sws & 0x3FF;	
		if(valor_switch > 0x1F && valor_switch < 0x3F){
				if( flag==0) flag=1; // flag no volvera a ser cero
				if (flag!=1) ciclos=0; // reinicia el ciclo si viene de otro patron	
				flag=1; // asigna el flag al patron
				for(i = 0; i < 4096; i++){				
					*dac= array3[i]; //onda triangular				
					}
				ciclos++;
			}
			else{
				if(voltaje < 2.5){
						
						if( flag==0) flag=2;
						if (flag!=2) ciclos=0;	
						flag=2;
						for(i = 0; i < 2048; i++){			
							*dac= array3[i];// recta_posi				
						}
					
						for(i = 0; i < 2048; i++){
							*dac= array2[i]; // descarga
						}		
					ciclos++;

					}
				else{
					
						if( flag==0) flag=3;	
						if (flag!=3) ciclos=0;	
						flag=3;				
						for(i = 0; i < 2048; i++){ 
							*dac= array[i];	// carga
						}
						for(i = 2048; i < 4096; i++){				
							*dac= array3[i]; //recta_nega				
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
