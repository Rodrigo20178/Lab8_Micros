/*
 * File:   lab8.c
 * Author: Rodrigo García
 *
 * Created on 14 de abril de 2023, 9:41 AM
 */


// CONFIG1
#pragma config FOSC = INTRC_NOCLKOUT// Oscillator Selection bits (INTOSCIO oscillator: I/O function on RA6/OSC2/CLKOUT pin, I/O function on RA7/OSC1/CLKIN)
#pragma config WDTE = OFF       // Watchdog Timer Enable bit (WDT disabled and can be enabled by SWDTEN bit of the WDTCON register)
#pragma config PWRTE = OFF      // Power-up Timer Enable bit (PWRT disabled)
#pragma config MCLRE = OFF      // RE3/MCLR pin function select bit (RE3/MCLR pin function is digital input, MCLR internally tied to VDD)
#pragma config CP = OFF         // Code Protection bit (Program memory code protection is disabled)
#pragma config CPD = OFF        // Data Code Protection bit (Data memory code protection is disabled)
#pragma config BOREN = OFF      // Brown Out Reset Selection bits (BOR disabled)
#pragma config IESO = OFF       // Internal External Switchover bit (Internal/External Switchover mode is disabled)
#pragma config FCMEN = OFF      // Fail-Safe Clock Monitor Enabled bit (Fail-Safe Clock Monitor is disabled)
#pragma config LVP = OFF        // Low Voltage Programming Enable bit (RB3 pin has digital I/O, HV on MCLR must be used for programming)

// CONFIG2
#pragma config BOR4V = BOR40V   // Brown-out Reset Selection bit (Brown-out Reset set to 4.0V)
#pragma config WRT = OFF        // Flash Program Memory Self Write Enable bits (Write protection off)

// #pragma config statements should precede project file includes.
// Use project enums instead of #define for ON and OFF.
#include <xc.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

//Constantes
#define _XTAL_FREQ 1000000


//Variables
uint8_t flag;
char selection;
char RX;
uint8_t pot = 0;
char pot_char[];


//Prototipo de funciones
void setup (void);
void Print (unsigned char *word);
void desplegar( char selection);

//Interrupciones
void __interrupt() isr (void)
{
    if(PIR1bits.RCIF){        
        RX = RCREG;
    }
    
    if (PIR1bits.ADIF)          // Interrupción del ADC
    {
         if (ADCON0bits.CHS == 0b0000){
             pot = ADRESH;
             
         }

        PIR1bits.ADIF = 0;
        
    }
    
}

//MAIN
void main(void) {
    setup();
    while(1){
        
        if(ADCON0bits.GO == 0){             // No hay proceso de conversion
            
            ADCON0bits.GO = 1;              // Iniciamos proceso de conversión
        } 
        
        Print("\r Seleccione una de las siguientes opciones: \r");
        Print(" 1) Leer el valor del potenciometro \r");
        Print(" 2) Desplegar valor ASCII\r");
        Print("\r \r");

        flag = 1;

        while (flag) {

            while (PIR1bits.RCIF == 0); //Esperar a recibir dato
            
            selection = RX;

            switch (selection) {
                case ('1'):
                    Print("\r El valor del potenciometro es: \r");
                    itoa(pot_char,pot,10);
                    desplegar(pot_char);
                    Print("\r \r");
                    Print("\r Listo! \r");
                    flag = 0;
                    break;

                case ('2'):
                    Print("\r Ingrese un caracter para mostrar en ASCII el puerto B: ");
                    while (PIR1bits.RCIF == 0); //Esperar
                    PORTB = RX; //Pasar el valor al puerto A
                    desplegar(RX); //mostrar el caracter en la pantalla
                    Print("\r \r");
                    Print("\r Listo! \r");
                    flag = 0;
                    break;

                default:
                    Print("Error \r");
                    
            }
        }
    }      
    return;
}

//Configuraciones
void setup (void){
    
    // Configuración de los puertos
    ANSEL = 0b00000001;         // AN0 como entrada analógica;;                  
    ANSELH = 0;                 //I/O digitales
    
    TRISA = 0b00000001;
    TRISB = 0;                  // PORTB como salida 
    PORTB = 0;                  // Limpiamos PORTB
    
    // Configuración del oscilador
    OSCCONbits.IRCF = 0b0100;    // 1 MHz
    OSCCONbits.SCS = 1;         // Oscilador interno
    
    // Configuraciones de comunicacion serial
    TXSTAbits.SYNC = 0;         // Comunicación ascincrona (full-duplex)
    TXSTAbits.BRGH = 1;         // Baud rate de alta velocidad 
    BAUDCTLbits.BRG16 = 1;      // 16-bits para generar el baud rate
    
    SPBRG = 25;
    SPBRGH = 0;                 // Baud rate ~9600, error -> 0.16%
    
    RCSTAbits.SPEN = 1;         // Habilitamos comunicación
    TXSTAbits.TX9 = 0;          // Utilizamos solo 8 bits
    TXSTAbits.TXEN = 1;         // Habilitamos transmisor
    RCSTAbits.CREN = 1;         // Habilitamos receptor
    
    //COnfiguración del ADC
    ADCON1bits.ADFM = 0;        
    ADCON1bits.VCFG0 = 0;       
    ADCON1bits.VCFG1 = 0;       
    
    ADCON0bits.ADCS = 0b01;     
    ADCON0bits.CHS = 0b0000;    
    
    ADCON0bits.ADON = 1;        // Encender ADC
    __delay_us(50);
     
    //Configuración de las interrupciones
    INTCONbits.GIE = 1;         // Habilitamos interrupciones globales
    INTCONbits.PEIE = 1;        // Habilitamos interrupciones de los puertos
    
    PIE1bits.ADIE = 1;          // Habilitamos interrupciones del ADC
    PIE1bits.RCIE = 1;          // Habilitamos Interrupciones de recepción
    
    PIR1bits.ADIF = 0;          // Flag del ADC en 0
    

    
    
   
}

//Funciones
void Print (unsigned char *word)
{
while (*word != '\0')
{
    while (TXIF != 1);
    TXREG = *word;
    *word++;
}
return;    
}

void desplegar( char selection) {
    while (TXSTAbits.TRMT == 0);
    TXREG = selection;
}

