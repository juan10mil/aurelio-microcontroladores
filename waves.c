#include <stdio.h>
#include <unistd.h>
#include <math.h>

// --- DEFINICIONES DE HARDWARE ---
#define ADC_ADDR 0x08001000
#define DAC_ADDR 0x08001020
#define LED_ADDR 0x08001030
#define SWS_ADDR 0x08001040

volatile unsigned int *adc = (unsigned int *) ADC_ADDR;
volatile unsigned int *dac = (unsigned int *) DAC_ADDR;
volatile unsigned int *led = (unsigned int *) LED_ADDR;
volatile unsigned int *sws = (unsigned int *) SWS_ADDR;

// --- ONDAS ---
unsigned int onda1[4096];   // Lineal + Descarga RC
unsigned int onda2[4096];   // Carga RC + Lineal invertida
unsigned int onda3[4096]; // Triangular

// Constante de tiempo RC
#define RC 512.0 

// --- MOSTRAR EN 7 SEGMENTOS ---
void mostrar_voltaje(float voltaje){
    unsigned int entero = (unsigned int) voltaje;
    unsigned int decimas = (unsigned int) ((voltaje - entero)*10)%10;
    unsigned int centesimas =(unsigned int) ((voltaje-entero)*100)%10;
    unsigned int milesimas = (unsigned int) ((voltaje - entero)* 1000)%10 ;
    unsigned int display = (entero << 12) | (decimas << 8) | (centesimas << 4) | milesimas;
    *led = display;
}

// --- CÁLCULO DE ONDAS ---
void calcular_ondas() {
    int i;
    float pendiente = 4095.0 / 2047.0; 

    for(i = 0; i < 4096; i++) {
        // ---------------------------------------------------------
        // ONDA 1: V < 2.5V
        // Mitad 1: Lineal (y = x)
        // Mitad 2: Descarga de Condensador (y = Vmax * e^(-t/RC))
        // ---------------------------------------------------------
        if (i < 2048) {
            onda1[i] = (unsigned int)((float)i * pendiente); 
        } else {
            float t = (float)(i - 2048); 
            float val = 4095.0 * exp(-t / RC); 
            onda1[i] = (unsigned int)val;
        }

        // ---------------------------------------------------------
        // ONDA 2: V > 2.5V
        // Mitad 1: Carga de Condensador (y = Vmax * (1 - e^(-t/RC)))
        // Mitad 2: Lineal negativa (y = -x)
        // ---------------------------------------------------------
        if (i < 2048) {
            float t = (float)i;
            float val = 4095.0 * (1.0 - exp(-t / RC_CONST));
            onda2[i] = (unsigned int)val;
        } else {
            float t = (float)(i - 2048);
            float val = 4095.0 - (t * pendiente);
            if(val < 0) val = 0;
            onda2[i] = (unsigned int)val;
        }

        // ---------------------------------------------------------
        // ONDA 3: 31 < SW < 63
        // Mitad 1: Subida lineal
        // Mitad 2: Bajada lineal
        // ---------------------------------------------------------
        if (i < 2048) {
            onda3[i] = (unsigned int)((float)i * pendiente);
        } else {
            float t = (float)(i - 2048);
            unsigned int val2 = (unsigned int)(4095.0 - (t * pendiente));
            if(val2 > 4095) val2 = 0;
            onda3[i] = val2;
        }
    }
}

int main()
{
	printf("Hello from Nios II!\n");
    printf("EE604 Introduccion a Microcontroladores - 4to Laboratorio 2025-2\n");

    calcular_ondas();
    
    unsigned int valor_adc = 0;
    unsigned int valor_sw = 0;
    float voltaje = 0;
    
    unsigned int *onda_actual = onda1; 

    int ciclos_contador = 0;

    while (1) {
    	
        // --- SELECCIÓN DE ONDA ---
        valor_adc = *adc & 0xFFF; 
        voltaje = (valor_adc * 4.096) / 4095.0;
        valor_sw = *sws & 0x3FF; 

        // Prioridad 1: Switches (Rango 31 - 63)
        if (valor_sw > 0x1F && valor_sw < 0x3F) {
            onda_actual = onda3;
        } 
        // Prioridad 2: Voltaje
        else {
            if (voltaje < 2.5) {
                onda_actual = onda1;
            } else {
                onda_actual = onda2;
            }
        }

        // --- GENERACIÓN DE SEÑAL ---
        int j;
        for(j = 0; j < 4096; j++){
            *dac = onda_actual[j];
        }

        // --- ACTUALIZACIÓN DE DISPLAY ---
        ciclos_contador++;
        
        if (ciclos_contador >= 200) {
            mostrar_voltaje(voltaje);
            ciclos_contador = 0;
        }
    }
    return 0;
}
