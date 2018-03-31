#include <msp430.h> 
#include "inc/user/System.h"

int main(void)
{
    WDTCTL = WDTPW | WDTHOLD;               //Stop watchdog timer

    System_init();

    _enable_interrupts();

    while(1)
    {
        menu();
    }
}

