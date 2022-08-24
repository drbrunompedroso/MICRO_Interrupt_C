
// PIC18F4520 Configuration Bit Settings
// 'C' source line config statements

// CONFIG1H
#pragma config OSC = HS         // Oscillator Selection bits (HS oscillator)
#pragma config FCMEN = OFF      // Fail-Safe Clock Monitor Enable bit (Fail-Safe Clock Monitor disabled)
#pragma config IESO = OFF       // Internal/External Oscillator Switchover bit (Oscillator Switchover mode disabled)

// CONFIG2L
#pragma config PWRT = OFF       // Power-up Timer Enable bit (PWRT disabled)
#pragma config BOREN = OFF      // Brown-out Reset Enable bits (Brown-out Reset disabled in hardware and software)
#pragma config BORV = 3         // Brown Out Reset Voltage bits (Minimum setting)

// CONFIG2H
#pragma config WDT = OFF        // Watchdog Timer Enable bit (WDT disabled (control is placed on the SWDTEN bit))
#pragma config WDTPS = 32768    // Watchdog Timer Postscale Select bits (1:32768)

// CONFIG3H
#pragma config CCP2MX = PORTC   // CCP2 MUX bit (CCP2 input/output is multiplexed with RC1)
#pragma config PBADEN = OFF     // PORTB A/D Enable bit (PORTB<4:0> pins are configured as digital I/O on Reset)
#pragma config LPT1OSC = OFF    // Low-Power Timer1 Oscillator Enable bit (Timer1 configured for higher power operation)
#pragma config MCLRE = ON       // MCLR Pin Enable bit (MCLR pin enabled; RE3 input pin disabled)

// CONFIG4L
#pragma config STVREN = ON      // Stack Full/Underflow Reset Enable bit (Stack full/underflow will cause Reset)
#pragma config LVP = OFF        // Single-Supply ICSP Enable bit (Single-Supply ICSP disabled)
#pragma config XINST = OFF      // Extended Instruction Set Enable bit (Instruction set extension and Indexed Addressing mode disabled (Legacy mode))

// CONFIG5L
#pragma config CP0 = OFF        // Code Protection bit (Block 0 (000800-001FFFh) not code-protected)
#pragma config CP1 = OFF        // Code Protection bit (Block 1 (002000-003FFFh) not code-protected)
#pragma config CP2 = OFF        // Code Protection bit (Block 2 (004000-005FFFh) not code-protected)
#pragma config CP3 = OFF        // Code Protection bit (Block 3 (006000-007FFFh) not code-protected)

// CONFIG5H
#pragma config CPB = OFF        // Boot Block Code Protection bit (Boot block (000000-0007FFh) not code-protected)
#pragma config CPD = OFF        // Data EEPROM Code Protection bit (Data EEPROM not code-protected)

// CONFIG6L
#pragma config WRT0 = OFF       // Write Protection bit (Block 0 (000800-001FFFh) not write-protected)
#pragma config WRT1 = OFF       // Write Protection bit (Block 1 (002000-003FFFh) not write-protected)
#pragma config WRT2 = OFF       // Write Protection bit (Block 2 (004000-005FFFh) not write-protected)
#pragma config WRT3 = OFF       // Write Protection bit (Block 3 (006000-007FFFh) not write-protected)

// CONFIG6H
#pragma config WRTC = OFF       // Configuration Register Write Protection bit (Configuration registers (300000-3000FFh) not write-protected)
#pragma config WRTB = OFF       // Boot Block Write Protection bit (Boot block (000000-0007FFh) not write-protected)
#pragma config WRTD = OFF       // Data EEPROM Write Protection bit (Data EEPROM not write-protected)

// CONFIG7L
#pragma config EBTR0 = OFF      // Table Read Protection bit (Block 0 (000800-001FFFh) not protected from table reads executed in other blocks)
#pragma config EBTR1 = OFF      // Table Read Protection bit (Block 1 (002000-003FFFh) not protected from table reads executed in other blocks)
#pragma config EBTR2 = OFF      // Table Read Protection bit (Block 2 (004000-005FFFh) not protected from table reads executed in other blocks)
#pragma config EBTR3 = OFF      // Table Read Protection bit (Block 3 (006000-007FFFh) not protected from table reads executed in other blocks)

// CONFIG7H
#pragma config EBTRB = OFF      // Boot Block Table Read Protection bit (Boot block (000000-0007FFh) not protected from table reads executed in other blocks)

// #pragma config statements should precede project file includes.
// Use project enums instead of #define for ON and OFF.

#include <xc.h>
#include <stdio.h>
#include "LCD4b_EXSTO.h"
#define PumpMotor       PORTCbits.RC0
#define Valve1          PORTCbits.RC1
#define Valve2          PORTCbits.RC2
#define Lamp_LowLevel   PORTCbits.RC3
#define Lamp_MidLevel   PORTCbits.RC4
#define Lamp_HighLevel  PORTCbits.RC5
#define _XTAL_FREQ 8000000

unsigned int VdigADC_AN0;

float Voltage_AN0,
      Level_AN0;

int Level_Loop;

unsigned char BufferVoltage[32],
              BufferLevel[32];

void config_FOSC()
{
    OSCCON = 0X00;
    OSCTUNE = 0X00;
}

void config_IO()
{
    TRISB = 0X01;
    TRISC = 0X00;
}

void config_ADC()
{
    ADCON0 = 0X01;       // Seleção dos canais analógicos; Estado da conversão e Habilitação do Conversor A/D
    ADCON1 = 0X0E;       // Tensão de referência; Seleção de entrada analógica
    ADCON2 = 0X80;       // Alinhamento dos Bits (ADRES); Tempo de aquisição; Fonte de Clock para o converesor A/D
}

void conv_AN0()
{
    __delay_ms(50);
    ADCON0bits.GO = 1;                              // Inicia o ciclo de conversão
    while(ADCON0bits.GO);                           // Aguarda o término do ciclo de conversão
    VdigADC_AN0 = ADRESH;                           // Atribui os 2 bits + significativos do ADRES
    VdigADC_AN0 = (VdigADC_AN0 << 8) + ADRESL;      // Mantém os 2 bits + significativos e soma os 8 bits - significativos do ADRES
}

void equation_SENSOR()
{
    Voltage_AN0 = 0.0048875 * VdigADC_AN0;            // Conversão de valor Digital para Tensão Elétrica
    Level_AN0 = Voltage_AN0 / 0.002;              // Equação do sensor (Tensão x Litros) Tanque = 120L      
}

void Lcd_Print()
{
    sprintf(BufferVoltage,
            "Sensor: %.2f V ", 
            Voltage_AN0);
    
    sprintf(BufferLevel,
            "Nivel: %.0f L  ",
            Level_AN0);
    
    lcd_write(1,1,BufferLevel);
    lcd_write(2,1,BufferVoltage);  
}

void control_HIGH()
{
    if(Level_Loop == 0)
    {
        if(Level_AN0 == 0)
        {
            PumpMotor = 1;      
            Valve1 = 1;  
            Valve2 = 0;        
            Lamp_LowLevel = 1;
            Lamp_MidLevel = 0;  
            Lamp_HighLevel = 0;
            Level_Loop = 0;
        }
        else if(Level_AN0 > 0 && Level_AN0 <= (2500*0.25))
        {
            PumpMotor = 1;      
            Valve1 = 1;  
            Valve2 = 0;            
            Lamp_LowLevel = 1;
            Lamp_MidLevel = 0;  
            Lamp_HighLevel = 0;
        }
        else if(Level_AN0 > (2500*0.25) && Level_AN0 <= (2500*0.50))
        {
            PumpMotor = 1;      
            Valve1 = 1;  
            Valve2 = 0;            
            Lamp_LowLevel = 1;
            Lamp_MidLevel = 1;  
            Lamp_HighLevel = 0;
        }
        else if(Level_AN0 > (2500*0.50) && Level_AN0 <= (2500*0.75))
        {
            PumpMotor = 1;      
            Valve1 = 1;  
            Valve2 = 1;            
            Lamp_LowLevel = 0;
            Lamp_MidLevel = 1;  
            Lamp_HighLevel = 1;
        }
        else if (Level_AN0 > (2500*0.75) && Level_AN0 < (2500))
        {
            PumpMotor = 0;      
            Valve1 = 0;  
            Valve2 = 1;             
            Lamp_LowLevel = 0;
            Lamp_MidLevel = 0;  
            Lamp_HighLevel = 1;
        }
        else if (Level_AN0 == 2500)
        {
            PumpMotor = 0;      
            Valve1 = 0;  
            Valve2 = 1;             
            Lamp_LowLevel = 0;
            Lamp_MidLevel = 0;  
            Lamp_HighLevel = 1;
            Level_Loop = 1;
        }
    }
}

void control_LOW()
{
    if(Level_Loop == 1)
    {
        PumpMotor = 0;      
        Valve1 = 0;  
        Valve2 = 1;     
        if(Level_AN0 == 0)
        {
            Lamp_LowLevel = 1;
            Lamp_MidLevel = 0;  
            Lamp_HighLevel = 0;
            Level_Loop = 0;
        }
        else if(Level_AN0 > 0 && Level_AN0 <= (2500*0.25))
        {        
            Lamp_LowLevel = 1;
            Lamp_MidLevel = 0;  
            Lamp_HighLevel = 0;
        }
        else if(Level_AN0 > (2500*0.25) && Level_AN0 <= (2500*0.50))
        {
            Lamp_LowLevel = 1;
            Lamp_MidLevel = 1;  
            Lamp_HighLevel = 0;
        }
        else if(Level_AN0 > (2500*0.50) && Level_AN0 <= (2500*0.75))
        {  
            Lamp_LowLevel = 0;
            Lamp_MidLevel = 1;  
            Lamp_HighLevel = 0;
        }
        else if (Level_AN0 > (2500*0.75) && Level_AN0 < 2500)
        {                 
            Lamp_LowLevel = 0;
            Lamp_MidLevel = 1;  
            Lamp_HighLevel = 1;
            Level_Loop = 1;
        }
         else if (Level_AN0 == 2500)
        {                 
            Lamp_LowLevel = 0;
            Lamp_MidLevel = 0;  
            Lamp_HighLevel = 1;
            Level_Loop = 1;
        }
    }
}

void config_Interrupt()
{
    INTCON = 0X90;
    INTCON2 = 0XC0;
}

void __interrupt() config_HighLevel(void)
{
    if(INTCONbits.INT0IF == 1)
    {       
        while(Level_AN0 > 0)
        {
                conv_AN0();
                equation_SENSOR();
                PumpMotor = 0;      
                Valve1 = 0;  
                Valve2 = 1;
                Lamp_LowLevel = 0;
                Lamp_MidLevel = 0;  
                Lamp_HighLevel = 0;
                __delay_ms(200);
                Lamp_LowLevel = 1;
                Lamp_MidLevel = 1;  
                Lamp_HighLevel = 1;
                __delay_ms(200);

                lcd_write(1,1,"     ALERTA    ");
                lcd_write(2,1,"  NIVEL CRITICO  ");                
        }
        INTCONbits.INT0IF = 0;       
    }     
}   

void main()
{
    config_FOSC();
    config_IO();
    config_ADC();
    config_Interrupt();
    lcd_init();
    while(1)
    {
        conv_AN0();
        equation_SENSOR();
        Lcd_Print();
        control_HIGH();
        control_LOW();
    }
}