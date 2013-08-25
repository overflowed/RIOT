#include <msp430x16x.h>
#include <hwtimer.h>
#include <hwtimer_arch.h>
#include <cpu.h>

static uint32_t ticks = 0;

extern void (*int_handler)(int);
extern void T_unset(short timer);
extern uint16_t overflow_interrupt[ARCH_MAXTIMERS+1];
extern uint16_t timer_round;

void timerA_init(void)
{
    volatile unsigned int *ccr;
    volatile unsigned int *ctl;
    ticks = 0;                               // Set tick counter value to 0
    timer_round = 0;                         // Set to round 0
    TA0CTL = TASSEL_1 + TACLR;               // Clear the timer counter, set ACLK on TimerA
    TA0CTL &= ~TAIFG;                        // Clear the IFG on TimerA
    TA0CTL &= ~TAIE;                         // Disable the counter overflow interrupt on TimerA


    for (int i = 0; i < TIMER_A_COUNT; i++) {
        ccr = &TA0CCR0 + (i);
        ctl = &TA0CCTL0 + (i);
        *ccr = 0;
        *ctl &= ~(CCIFG);
        *ctl &= ~(CCIE);
    }

    TA0CTL |= MC_2;
}

void timerB_init(void)
{
    volatile unsigned int *ccr;
    volatile unsigned int *ctl;
    TBCTL = TBSSEL_1 + TBCLR;                // Clear the timer counter, set ACLK on TimerB
    TBCTL &= ~TBIFG;                         // Clear the IFG on TimerB
    TBCTL &= ~TBIE;                          // Disable the counter overflow interrupt on TimerB

    for (int i = 0; i < TIMER_B_COUNT; i++) {
        ccr = &TBCCR0 + (i);
        ctl = &TBCCTL0 + (i);
        *ccr = 0;
        *ctl &= ~(CCIFG);
        *ctl &= ~(CCIE);
    }

    TBCTL |= MC_2;
}

interrupt(TIMERA0_VECTOR) __attribute__((naked)) timer_isr_ccr0(void)
{
    __enter_isr();
    timer_round += 1;
    __exit_isr();

}

interrupt(TIMERB0_VECTOR) __attribute__((naked)) timerb0_isr_ccr0(void)
{
    __enter_isr();
    uint8_t timer = TIMER_A_COUNT;
    if(overflow_interrupt[timer] == timer_round) {
    	T_unset(timer);
    	int_handler(timer);
    }
    __exit_isr();

}

interrupt(TIMERA1_VECTOR) __attribute__((naked)) timer_isr(void)
{
    __enter_isr();

    uint8_t taiv = TA0IV;

    if (taiv & TAIFG) {
    } else {

        short timer = (taiv/2);
        if(overflow_interrupt[timer] == timer_round)
        {
            T_unset(timer);
            int_handler(timer);
        }
    }
    __exit_isr();
}

interrupt(TIMERB1_VECTOR) __attribute__((naked)) timer_isr_ccr1_6(void)
{
    __enter_isr();

    uint8_t tbiv = TBIV;

    if (tbiv & TBIV_TBIFG) {
    } else {

        short timer = (tbiv/2) + TIMER_A_COUNT;
        if(overflow_interrupt[timer] == timer_round)
        {
            T_unset(timer);
            int_handler(timer);
        }
    }
    __exit_isr();
}
