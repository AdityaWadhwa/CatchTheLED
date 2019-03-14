#include <msp430.h>
#include <inttypes.h>
#include <lcd.h>
#include "rand.h"


#define CMD         0
#define DATA        1

#define LCD_OUT     P1OUT
#define LCD_DIR     P1DIR
#define D4          BIT4
#define D5          BIT3
#define D6          BIT2
#define D7          BIT1
#define RS          BIT0
#define EN          BIT5

#define R       BIT7
#define G       BIT7
#define B       BIT6
#define LED1    BIT3
#define LED2    BIT4
#define LED3    BIT5
#define LED4    BIT6

#define SW1     BIT2
#define SW2     BIT1
#define BUZZ    BIT0
/**
 * Random number generator.
 *
 * It is recommended that this function is run during setup, before any clocks
 * or timers have been set and before interrupts have been enabled. While it
 * does restore all used control registers to their original values, it also
 * expects that nothing interesting will happen when it modifies them.
 *
 * Algorithm from TI SLAA338:
 * http://www.ti.com/sc/docs/psheets/abstract/apps/slaa338.htm
 *
 * @return 16 random bits generated from a hardware source.
 */

volatile int a,b,c,d,k,m;
volatile int score=5;

unsigned int rand(void) {
    int i, j;
    unsigned int result = 0;

    /* Save state */
    unsigned int BCSCTL3_old = BCSCTL3;
    unsigned int TACCTL0_old = TACCTL0;
    unsigned int TACTL_old = TACTL;

    /* Halt timer */
    TACTL = 0x0;

    /* Set up timer */
    BCSCTL3 = (~LFXT1S_3 & BCSCTL3) | LFXT1S_2; // Source ACLK from VLO
    TACCTL0 = CAP | CM_1 | CCIS_1;            // Capture mode, positive edge
    TACTL = TASSEL_2 | MC_2;                  // SMCLK, continuous up

    /* Generate bits */
    for (i = 0; i < 16; i++) {
        unsigned int ones = 0;

        for (j = 0; j < 5; j++) {
            while (!(CCIFG & TACCTL0));       // Wait for interrupt

            TACCTL0 &= ~CCIFG;                // Clear interrupt

            if (1 & TACCR0)                   // If LSb set, count it
                ones++;
        }

        result >>= 1;                         // Save previous bits

        if (ones >= 3)                        // Best out of 5
            result |= 0x8000;                 // Set MSb
    }

    /* Restore state */
    BCSCTL3 = BCSCTL3_old;
    TACCTL0 = TACCTL0_old;
    TACTL = TACTL_old;

    return result;
}

/**
 * Pseudo-random number generator.
 *
 * Implemented by a 16-bit linear congruential generator.
 * NOTE: Only treat the MSB of the return value as random.
 *
 * @param state Previous state of the generator.
 * @return Next state of the generator.
 */
/*unsigned int prand(unsigned int state) {
    return M * state + I;                     // Generate the next state of the LCG
}*/



color(int i,int j)
{
    P1OUT &= ~R;
    P1OUT |= LED4;
    P2OUT |= (LED1 + LED2 + LED3);
    P2OUT &= ~(G+B);

    if(j==1)
    {
        P2OUT &= ~LED1;
    }
    else if(j==2)
    {
        P2OUT &= ~LED2;
    }
    else if(j==3)
    {
        P2OUT &= ~LED3;
    }
    else if(j==4)
    {
        P1OUT &= ~LED4;
    }


    if(i==1)
    {
        P1OUT |= R;
    }
    else if(i==2)
    {
        P2OUT |= G;
    }
    else if(i==3)
    {
        P2OUT |= B;
    }
    else if(i==4)
    {
        P1OUT |= R;
        P2OUT |= G;
    }
    else if(i==5)
    {
        P2OUT |= G;
        P2OUT |= B;
    }
    else if(i==6)
    {
        P1OUT |= R;
        P2OUT |= B;
    }
}

void lcd(void)
{
    lcd_setCursor(0,1);

    switch(a)
    {
    case 1:
        lcd_print(" CATCH THE RED ");
        break;

    case 2:
        lcd_print(" CATCH THE GREEN ");
        break;

    case 3:
        lcd_print(" CATCH THE BLUE ");
        break;

    case 4:
        lcd_print(" CATCH THE YELLOW ");
        break;

    case 5:
        lcd_print(" CATCH THE CYAN ");
        break;

    case 6:
        lcd_print(" CATCH THE PINK ");
        break;


    }

    lcd_setCursor(1,0);
    lcd_print("    SCORE :    ");
    lcd_setCursor(1,12);
    lcd_write((char)(score+48),1);
}

void cleanUp()
{
    P1OUT &= ~R;
    P1OUT |= LED4;
    P2OUT |= (LED1 + LED2 + LED3);
    P2OUT &= ~(G+B);

}


void main(void)
{
    WDTCTL = WDTPW + WDTHOLD; // stop watchdog
    __enable_interrupt();
    lcd_init(P1, D4, D5, D6, D7, P2, RS, P1, EN);
    lcd_setCursor(0,0);
    lcd_print("   WELCOME TO   ");
    lcd_setCursor(1,1);
    lcd_print(" CATCH THE LED ");

    P1DIR|=BUZZ;
    P1OUT&=~BUZZ;

    P2DIR&=~SW1;
    P2IE|=SW1;
    P2IFG&=~SW1;
    P2IES|=SW1;
    P2SEL &= ~(BIT6+BIT7);      //set 2.6 and 2.7 as GPIO pin
    P1DIR |= (R+LED4);
    P2DIR |= (LED1 + LED2 + LED3+G+B);

    a=rand()%6+1;
    __delay_cycles(5000000);
    lcd();

    while(1)
    {
        //a=rand()%6 + 1;

        if(score<0||score>9)
            break;

        for(k=0;k<20;k++)
        {
            b=rand()%6 + 1;
            c=rand()%6 + 1;
            d=rand()%6 + 1;
            for(m=0;m<200;m++)
            {
                color(a,1);
                __delay_cycles(250);
                color(b,2);
                __delay_cycles(250);
                color(c,3);
                __delay_cycles(250);
                color(d,4);
                __delay_cycles(250);
            }
        }
        __delay_cycles(50000);

    }
}

#pragma vector=PORT2_VECTOR;
    __interrupt void abc(void)
    {
           if((a==b)||(a==c)||(a==d))
           {
               score++;
               if(score>9)
               {
                      lcd_setCursor(0,0);
                      lcd_print("    YOU WON     ");
                      cleanUp();
                      exit(0);
               }

           }
           else
           {
               score--;
               P1OUT|=BUZZ;
               __delay_cycles(100000);
               P1OUT&=~BUZZ;
               __delay_cycles(100000);
               P1OUT|=BUZZ;
               __delay_cycles(100000);
               P1OUT&=~BUZZ;

               if(score<0)
               {
                     lcd_setCursor(0,0);
                     lcd_print("    YOU LOST    ");
                     cleanUp();
                     exit(0);
               }

           }

           //lcd_init(P1, D4, D5, D6, D7, P2, RS, P1, EN);
           /*lcd_setCursor(0,0);
           lcd_write(LCD_CLEARDISPLAY, CMD);                   // Clear screen
           delay(20);
           lcd_setCursor(1,0);
           lcd_write(LCD_CLEARDISPLAY, CMD);                   // Clear screen
           delay(20);*/

           lcd_setCursor(1,12);
           lcd_write((char)(score+48),1);


           for(m=0;m<2000;m++)
                       {
                           color(a,1);
                           __delay_cycles(250);
                           color(b,2);
                           __delay_cycles(250);
                           color(c,3);
                           __delay_cycles(250);
                           color(d,4);
                           __delay_cycles(250);
                       }

           a=rand()%6+1;

           lcd();

           P2IFG&=~SW1;
    }
