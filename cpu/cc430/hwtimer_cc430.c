#include <legacymsp430.h>
#include <board.h>
#include <hwtimer.h>
#include <hwtimer_arch.h>
#include <cpu.h>

#define ENABLE_DEBUG (0)
#include <debug.h>

static uint32_t ticks = 0;

extern void (*int_handler)(int);
extern void T_unset(short timer);
extern uint16_t overflow_interrupt[ARCH_MAXTIMERS+1];
extern uint16_t timer_round;

void timerA_init(void)
{
    ticks = 0;                               // Set tick counter value to 0
    timer_round = 0;
    TA0CTL = TASSEL_1 + TACLR;   	  		 // Clear the timer counter, set ACLK
    TA0CTL &= ~TAIE;						 // Clear the IFG

    volatile unsigned int *ccr = &TA0CCR0;
    volatile unsigned int *ctl = &TA0CCTL0;

    for (int i = 0; i < TIMER_A_COUNT; i++) {
        *(ccr + i) = 0;
        *(ctl + i) &= ~(CCIFG);
        *(ctl + i) &= ~(CCIE);
    }

    TA0CTL |= MC_2;
}

void timerB_init(void)
{
    ticks = 0;								 // Set tick counter value to 0
    TBCTL = TBSSEL_1 + TBCLR;   	  		 // Clear the timer counter, set ACLK
    TBCTL &= ~TBIE;						     // Clear the IFG

    volatile unsigned int *ccr = &TBCCR0;
    volatile unsigned int *ctl = &TBCCTL0;

    for (int i = 0; i < TIMER_B_COUNT; i++) {
        *(ccr + i) = 0;
        *(ctl + i) &= ~(CCIFG);
        *(ctl + i) &= ~(CCIE);
    }

    TBCTL |= MC_2;
}

interrupt(TIMER0_A0_VECTOR) __attribute__((naked)) timer0_a0_isr(void)
{
    __enter_isr();
    timer_round += 1;
    __exit_isr();
}

interrupt(TIMER0_B0_VECTOR) __attribute__((naked)) timerb0_isr(void)
{
    __enter_isr();
    uint8_t timer = TIMER_A_COUNT;
    if(overflow_interrupt[timer] == timer_round) {
        T_unset(timer);
        int_handler(timer);
    }
    __exit_isr();
}

interrupt(TIMER0_A1_VECTOR) __attribute__((naked)) timer0_a1_5_isr(void)
{
    __enter_isr();

    uint8_t taiv = TA0IV;
    uint8_t timer;

    if (taiv & TAIFG) {
        DEBUG("Overflow\n");
    }
    else {
        timer = (taiv / 2);
        if(overflow_interrupt[timer] == timer_round) {
            T_unset(timer);
            int_handler(timer);
        }
    }

    __exit_isr();
}

interrupt(TIMER0_B1_VECTOR) __attribute__((naked)) timerb16_isr(void)
{
    __enter_isr();

    uint8_t tbiv = TBIV;
    uint8_t timer;

    if (tbiv & TBIV_TBIFG) {
        DEBUG("Overflow\n");
    }
    else {
        timer = (tbiv / 2) + TIMER_A_COUNT;
        if(overflow_interrupt[timer] == timer_round) {
            T_unset(timer);
            int_handler(timer);
        }
    }

    __exit_isr();
}
