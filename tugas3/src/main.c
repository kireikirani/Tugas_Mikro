#include "stm32f4xx.h"

volatile unsigned int pressed = 0;        /* 1 = button held (debounced) */
volatile unsigned int edge_flag = 0;
volatile unsigned long ms_ticks = 0;

void gpio_Init(void)
{
    RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN | RCC_AHB1ENR_GPIOCEN;

    /* PC13: LED, output push-pull */
    GPIOC->MODER  &= ~(3UL << (13 * 2));
    GPIOC->MODER  |=  (1UL << (13 * 2));
    GPIOC->OTYPER &= ~(1UL << 13);
    GPIOC->PUPDR  &= ~(3UL << (13 * 2));
    /* PA0: input, pull-up (button connects PA0 to GND) */
    GPIOA->MODER &= ~(3UL << (0 * 2));
    GPIOA->PUPDR &= ~(3UL << (0 * 2));
    GPIOA->PUPDR |=  (1UL << (0 * 2));

    GPIOC->BSRR = (1UL << 13);/* LED off (active-low) */
}

void Interrupt_Init(void)
{
    RCC->APB2ENR |= RCC_APB2ENR_SYSCFGEN;

    SYSCFG->EXTICR[0] &= ~SYSCFG_EXTICR1_EXTI0;   /* PA0 -> EXTI0 */

    EXTI->IMR  |=  (1UL << 0);    /* unmask EXTI0 */
    EXTI->EMR  &= ~(1UL << 0);    /* interrupt, not event */
    EXTI->FTSR |=  (1UL << 0);    /* falling edge/press*/
    EXTI->RTSR |=  (1UL << 0);    /* rising edge/release*/
    EXTI->PR    =  (1UL << 0);

    NVIC_SetPriority(EXTI0_IRQn, 1);
    NVIC_EnableIRQ(EXTI0_IRQn);
}

void tim2_Init(void)
{
    RCC->APB1ENR |= RCC_APB1ENR_TIM2EN;

    TIM2->CR1 = 0;
    TIM2->PSC = 1600;
    TIM2->ARR = 9;
    TIM2->EGR = TIM_EGR_UG; /* load PSC and ARR now */
    TIM2->SR  = 0;
    TIM2->CR1 = TIM_CR1_CEN; /* start, no interrupt */
}

int main(void)
{
    unsigned long last_toggle = 0;
    unsigned int  raw_last    = 1;    /* last pin sample, 1 = released */
    unsigned int  stable      = 0;    /* samples in a row matching raw_last */

    SystemCoreClockUpdate();
    gpio_Init();
    Interrupt_Init();
    tim2_Init();

    while (1) {
        if (TIM2->SR & TIM_SR_UIF) {
            TIM2->SR &= ~TIM_SR_UIF;
            ms_ticks++;

            __disable_irq();
            if (edge_flag) {
                edge_flag = 0;
                stable = 0;
            }
            __enable_irq();

            /* Debounce: the level must hold for 20 consecutive samples */
            unsigned int raw = (GPIOA->IDR & (1UL << 0)) ? 1U : 0U;
            if (raw != raw_last) {
                raw_last = raw;
                stable = 0;
            } else if (stable < 20) {
                stable++;
            }

            if (stable >= 20) {
                pressed = (raw_last == 0U) ? 1U : 0U;   /* low = pressed */
            }

            /* Output */
            if (pressed) {
                if ((ms_ticks - last_toggle) >= 150) {  /* blink every 150 ms */
                    last_toggle = ms_ticks;
                    GPIOC->ODR ^= (1UL << 13);
                }
            } else {
                GPIOC->BSRR = (1UL << 13);/* released: LED off */
            }
        }
    }
}

/* ISR: only records that PA0 changed. No delays, no LED writes. */
void EXTI0_IRQHandler(void)
{
    if (EXTI->PR & (1UL << 0)) {
        EXTI->PR = (1UL << 0);     /* clear pending (write 1) */
        edge_flag = 1;
    }
}