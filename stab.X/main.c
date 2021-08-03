/* 
 * File:   main.c
 * Author: adm
 *
 * Created on 4 Февраль 2019 г., 10:43
 */

#include <stdio.h>
#include <stdlib.h>
#include <pic12f675.h>







// PIC12F675 Configuration Bit Settings

// 'C' source line config statements

#include <xc.h>

// #pragma config statements should precede project file includes.
// Use project enums instead of #define for ON and OFF.

// CONFIG
#pragma config FOSC = INTRCIO   // Oscillator Selection bits (INTOSC oscillator: I/O function on GP4/OSC2/CLKOUT pin, I/O function on GP5/OSC1/CLKIN)
#pragma config WDTE = OFF       // Watchdog Timer Enable bit (WDT disabled)
#pragma config PWRTE = OFF      // Power-Up Timer Enable bit (PWRT disabled)
#pragma config MCLRE = OFF      // GP3/MCLR pin function select (GP3/MCLR pin function is digital I/O, MCLR internally tied to VDD)
#pragma config BOREN = ON      // Brown-out Detect Enable bit (BOD enabled)
#pragma config CP = OFF         // Code Protection bit (Program Memory code protection is disabled)
#pragma config CPD = OFF        // Data Code Protection bit (Data memory code protection is disabled)





#define _XTAL_FREQ 4000000
#define trvhod      GP0
#define led         GP2
#define bolshe      GP5
#define menshe      GP4
#define ogon        GP1
#define button      GP3


unsigned char eeprom_read(unsigned char addr);
void eeprom_write(unsigned char addr, unsigned char value);
void button_action();
unsigned char start_flag,btn_counter,btn_flag,btn_release;
unsigned char adc_counter,adc_flag;
unsigned char reg_on_flag;
unsigned char const delta=8;       //гистререзис в +/-4 вольта при контроле (2ед на вольт)
unsigned char const delta_reg=3;       //гистререзис в +/-1,5 вольта при регулировании
unsigned char led_flash=0, led_flash_set=0;
unsigned int milisec=0, main_counter=0;
unsigned char measure_counter=0;
unsigned int measure,volt, volt_average, volt_set;
unsigned int volt_abs=490;          //защита на 245 вольт
unsigned char const flash_delay=200;
unsigned int const no_flash_delay=800;
unsigned int const flash_stop=3000;
unsigned int const main_cycle=200;       //частота проверки напряжения, мсек



int main() {
    TRISIO=0b00001001; // gp0, gp1 - in; gp2,4,5 - out
    GPIO=0;
    INTCON=0b11100000;  //TMRO interrupt
    PIE1=0b01000000;
    OPTION_REG=0b00000000;
    ADCON0=0b10000001;
    ANSEL=0b01000001;
    CMCON=0b00000111;
    led_flash_set=eeprom_read(0x00);
    if(led_flash_set>11){
            led_flash_set=4;
            button_action();
        }
    volt_set=420+led_flash_set*2*2;
    start_flag=1;
    
    while(1){
        if(led_flash_set>11){
            led_flash_set=4;
            button_action();
        }
        
        if(btn_flag){
            button_action();
            led=0;
            led_flash=led_flash_set;
            milisec=flash_stop-20;
            volt_set=420+led_flash_set*2*2;
        }
        
        if((volt_average!=0)&&(volt_average<(volt_abs-delta))&&start_flag==0){
            ogon=1;
        }
        if(volt_average>=volt_abs){
            ogon=0;
        }
        if(main_counter>3000){
            start_flag=0;
        }
        
        if(main_counter>main_cycle){
            if(start_flag==0){
                TRISIO=0b00001001; // gp0, gp1 - in; gp2,4,5 - out / reinit
                main_counter=0;
            }
            if(volt_average>(volt_set+delta)){
                reg_on_flag=1;
            }
            if(volt_average<(volt_set-delta)){
                reg_on_flag=1;
            }
        }
        
        if(reg_on_flag){
            
            if(volt_average>volt_set){
                menshe=1;
                bolshe=0;
                
            }
            if(volt_average<volt_set){
                bolshe=1;
                menshe=0;
                
            }

            if((volt_average>(volt_set-delta_reg))&&(volt_average<(volt_set+delta_reg))){
                bolshe=0;
                menshe=0;
                reg_on_flag=0;
                
                
            }
                 
                                   
        }
        
        
        if(adc_flag){
            measure=(ADRESH*255)+ADRESL;
            adc_flag=0;
            if(measure_counter<10){
                volt=volt+measure;
                measure_counter++;
            }
            else{
            volt_average=volt/10;
            volt=0;
            measure_counter=0;
            }
        }
        if(adc_counter>2&&GO_nDONE==0){
            adc_counter=0;
            GO_nDONE=1;
        }
        
        
    }
}


void button_action(){
    
    if(led_flash_set<11){
        led_flash_set++;
    }
    else{
        led_flash_set=1;
    }
    eeprom_write(0, led_flash_set);
    btn_flag=0;
}

void interrupt high_isr (void){
    if(ADIF){
        ADIF=0;
        adc_flag=1;
    }
    if(T0IF){
        T0IF=0;
        if(main_counter<10000){
            main_counter++;
        }
        
        if(adc_counter<250){
            adc_counter++;
        }
        
        if(milisec<10000){
            milisec++;
        }
        else{
            milisec=0;
        }
        
      
        if((milisec>=no_flash_delay)&&(led_flash<led_flash_set)){
            if(led==0){
                led=1;
                milisec=0;
                led_flash++;
            }
        }
        if((milisec>=flash_delay)&&(led==1)){
            led=0;
            milisec=0;
        }
        if((led_flash==led_flash_set)&&(milisec>=flash_stop)){
            led_flash=0;
            milisec=0;
        }
        
        
        if(button==1&&btn_counter<50){
            btn_counter++;
        }
        
        if(button==0&&btn_counter!=0&&btn_release<50){
            btn_release++;
        }
        
        if(button==0&&btn_counter>40&&btn_release>40){
            btn_flag=1;
            btn_counter=0;
            btn_release=0;
        }
        
        
        
        
    }
    
}
