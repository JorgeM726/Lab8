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
#include <string.h>
/*------------------------------------------------------------------------------
 * CONSTANTES 
 ------------------------------------------------------------------------------*/
#define _XTAL_FREQ 1000000
#define IN_MIN 0                // entrada minima
#define IN_MAX 255              // entrada maxima
#define OUT_MIN 16               // min width
#define OUT_MAX 80             // max width


/*------------------------------------------------------------------------------
 * VARIABLES 
 ------------------------------------------------------------------------------*/
const char *cString = "1. Leer pot. \r\n2. Enviar Ascii\r\n";
const char dato = 65;
unsigned int numero = 0;
char input[2];
char ascii[4];

/*------------------------------------------------------------------------------
 * PROTOTIPO DE FUNCIONES 
 ------------------------------------------------------------------------------*/
void setup(void);
void USART_send(const char data);
void USART_print(const char *string);
char USART_read(void);
/*------------------------------------------------------------------------------
 * INTERRUPCIONES 
 ------------------------------------------------------------------------------*/
void __interrupt() isr (void){
    if(PIR1bits.ADIF){              // Revisar interrupción de ADC
        if(ADCON0bits.CHS == 12){    // Revisar canal 12
            PORTA = ADRESH;         // Mostrar conversión en PORTA
        }
        
        else 
            numero = ADRESH;            //Enviar número a display si es en el canal 10
            PIR1bits.ADIF = 0;          // limpiar interrupción
    }
    return;
}

/*------------------------------------------------------------------------------
 * CICLO PRINCIPAL
 ------------------------------------------------------------------------------*/
void main(void) {
    setup();
    while(1){
        
    if(ADCON0bits.GO == 0){                 
            if(ADCON0bits.CHS == 12) {   
                ADCON0bits.CHS = 10;    // Cambio de canal
            }
            else
                ADCON0bits.CHS = 12;    // Cambio de canal
                __delay_us(100);                 // Tiempo de adquisicion
                ADCON0bits.GO = 1;              // Iniciar conversion
        }
    
    USART_print(cString);
    input[0] = USART_read();
    input[1] = '\0'; 
    if (strcmp(input, "1") == 0){
    PORTD = numero;
} else if (strcmp(input, "2") == 0){
    USART_print("Ingrese el numero de caracter en ASCII (2 digitos): ");
    char highByte = USART_read(); // read the first character
    USART_send(highByte);
    char lowByte = USART_read(); // read the second character
    USART_send(lowByte);
    PORTD = (highByte - '0') * 10 + (lowByte - '0'); // combine the two characters into a single 2-digit ASCII code
}else {
    PORTD = 0;
}
    
            
        
    
        
    }
    
    return;
}

/*------------------------------------------------------------------------------
 * CONFIGURACION 
 ------------------------------------------------------------------------------*/
void setup(void){
    ANSEL = 0X00; 
    ANSELH = 0b00000011;
     TRISA = 0X00;
    PORTA = 0X00;
    TRISB = 0b00000011;
    PORTB = 0b00000011;
    TRISD = 0;
    PORTD = 0;
    
    // Configuraci?n reloj interno
    OSCCONbits.IRCF = 0b100;    // 4MHz
    OSCCONbits.SCS = 1;         // Oscilador interno
    
    
    TXSTAbits.SYNC = 0;
    TXSTAbits.BRGH = 1;
    BAUDCTLbits.BRG16 = 1;
    SPBRG = 25;
    SPBRGH = 0;
    
    RCSTAbits.SPEN = 1;
    RCSTAbits.RX9 = 0;
    RCSTAbits.CREN = 1;
    TXSTAbits.TXEN = 1;
    
     // CONFIGURACION ADC
    ADCON0bits.ADCS = 0b01;     // Fosc/8
    ADCON1bits.VCFG0 = 0;       // VDD 
    ADCON1bits.VCFG1 = 0;       // VSS 
    ADCON0bits.CHS = 12;    // Seleccionar el AN12
    
    
    ADCON1bits.ADFM = 0;        // Justificado a la izquierda
    __delay_us(50);             // Sample time
    ADCON0bits.ADON = 1;        // HAbilitar modulo ADC
    
    PIR1bits.ADIF = 0;          
    PIE1bits.ADIE = 1;  
    
    PIR1bits.RCIF = 0;
    PIE1bits.RCIE = 0;
    INTCONbits.PEIE = 1;
    INTCONbits.GIE = 1;
}


void USART_send(const char data)
{
    while(!TRMT);
    TXREG = data;
}

void USART_print(const char *string)
{
    int i = 0;
    
    for(i; string[i] != '\0'; i++)
    {
        USART_send(string[i]);
    }
}

char USART_read(void)
{
    while(!RCIF);
    return RCREG;
}