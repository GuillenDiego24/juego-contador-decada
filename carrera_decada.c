/* 
 * Author: Diego Guillén
 *
 * Created on 29 de enero de 2020, 09:17 AM
 */

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


#include <xc.h>


//******************************************************************************
// Definición de variables
//******************************************************************************
#define _XTAL_FREQ 8000000
char conta, contador, bandera, c_gamer1, c_gamer2, P0_old, P1_old;
unsigned char display_decadas[]= {0,1,2,4,8,16,32,64,128}; // valores del contador decada
#define A PORTDbits.RD0                                    // El mapeo de variables a los pines conectados al display
#define B PORTDbits.RD1
#define C PORTDbits.RD2
#define D PORTDbits.RD3
#define E PORTDbits.RD4
#define F PORTDbits.RD5
#define G PORTDbits.RD6
//******************************************************************************
// Definición e importación de librerías
//******************************************************************************
#include "Oscilador.h"
#include "SEG7.h"               //funcion que devuelve los valores de pines para display
//******************************************************************************
// Definición de funciones para que se puedan colocar después del main de lo 
// contrario hay que colocarlos todas las funciones antes del main
//******************************************************************************
void setup(void);
void initTimer1(void);


void __interrupt() isr(void){
    // Solo estaba probando las interrupciones
    if(PIR1bits.TMR1IF == 1){
        if(bandera == 1){
            contador++;
            if(contador >= 10){
                contador = 0;
            }
         }
        if(bandera == 0){
            contador--;
            if(contador <= 0){
                contador = 9;
            }
         }
        // valor a cargar al TMR1 5536 -> 0x15A0
        TMR1H = 0x5F;
        TMR1L = 0xFF;
        PIR1bits.TMR1IF = 0;
    }
}

//******************************************************************************
// Código Principal
//******************************************************************************
void main(void){
    setup();
 
    //**************************************************************************
    // Loop infinito
    //**************************************************************************
    // Contador regresivo de 3 segundos para comenzar el juego
    while (conta >> 0){
             segmentos7(conta);   // Se imprime el valor del contador en display
             __delay_ms(1000);
            conta = conta - 1;    
        }
    PORTAbits.RA2 = 0;            //Desactiva el pin de anodo comun del display
    while(1){
        // anti rebote de push que ferifica cambio de flanco asendente
         __delay_ms(10);                          //Delay para detectar el cambio de flanco
        if(PORTBbits.RB0 == 1 && P0_old == 0){    // push jugador 1
            c_gamer1 ++;                          // dentro del rebote se incrementa la variable c_gamer1
        }
         P0_old = PORTBbits.RB0;                  // Se guarda valor antiguo de push
         
          __delay_ms(10);  
        if(PORTBbits.RB1 == 1 && P1_old == 0){    //push jugador 2
            c_gamer2 ++;  
        }
         P1_old = PORTBbits.RB1;                  // Se guarda valor antiguo de push
         
         // Se impime en los leds el valor decada de cada jugador
         PORTC = display_decadas[c_gamer1];       
         PORTD = display_decadas[c_gamer2];
         
         //Fase final que muestra ganador victorios
         while (c_gamer1 == 9){      //loop infinito que muestra ganador 1 victorioso             
             PORTAbits.RA2 = 1;      //bit que activa display
             segmentos7(1);          // se imprime en display numero de jugador
         }
         while (c_gamer2 == 9){      //loop infinito que muestra ganador 2 victorioso 
             PORTAbits.RA2 = 1;      //bit que activa display
             segmentos7(2);          // se imprime en display numero de jugador
         }
        
    }
}
//******************************************************************************
// Función de Inicialización
//******************************************************************************
void setup(void){
    initOscilador(8);           // Oscilador interno de 8 MHz
    ANSEL = 0;
    ANSELH = 0;                 // Todos los puertos de forma digital
    TRISC = 0;
    TRISA = 0;
    TRISD  = 0;                 // Podemos modificar registros completos
    PORTD = 0;
    PORTC = 0;
    PORTA = 0;
    TRISBbits.TRISB0 = 1;       // Configuracion de pines de entrada
    TRISBbits.TRISB1 = 1;
    PORTAbits.RA2 = 1;
    initTimer1();               // Configuramos el timer 1 como temporizador
    INTCONbits.GIE = 1;         // Habilitamos interrupciones
    contador = 0;               // Valores iniciales de variables
    bandera = 0;
    conta = 3;
    c_gamer1 = 0;
    c_gamer2 = 0;
}
//******************************************************************************
// Función para inicializar el Timer 1 como temporizador con desborde a 30 mS
//******************************************************************************
void initTimer1(void){
    T1CONbits.T1GINV = 0;
    T1CONbits.TMR1GE = 0;       // Always counting
    T1CONbits.T1CKPS0 = 1;      // Prescaler 1:2
    T1CONbits.T1CKPS1 = 1;
    T1CONbits.T1OSCEN = 0;      // LP oscillator is off
    T1CONbits.TMR1CS = 0;       // Reloj Fosc/4 modo temporizador
    T1CONbits.TMR1ON = 1;       // Timer 1 ON
    
    // T1CON = 0x21;            // Otras formas de haberlo configurado directamente
    // T1CON = 33;
    // T1CON = 0b00010001;
    
    INTCONbits.PEIE = 1;        // Habilitamos interrupciones PEIE
    PIR1bits.TMR1IF = 0;        // Limpiamos la bandera de interrupción TMR1
    PIE1bits.TMR1IE = 1;        // Habilitamos interrupción TMR1
    
    
    // valor a cargar al TMR1 5536 -> 0x15A0
    TMR1H = 0x1F;
    TMR1L = 0xFF;
}
//******************************************************************************