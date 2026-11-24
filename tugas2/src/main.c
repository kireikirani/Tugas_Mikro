#include "stm32f4xx.h"

volatile unsigned int pressed;

void gpio_Init(void)
{
    RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN | RCC_AHB1ENR_GPIOCEN;

    /* PC13 LED, output push-pull */
    GPIOC->MODER  &= ~(3UL << (13 * 2));
    GPIOC->MODER  |=  (1UL << (13 * 2));
    GPIOC->OTYPER &= ~(1UL << 13);
    GPIOC->PUPDR  &= ~(3UL << (13 * 2));

    /* PA0 input, pull-up*/
    GPIOA->MODER &= ~(3UL << (0 * 2));
    GPIOA->PUPDR &= ~(3UL << (0 * 2));
    GPIOA->PUPDR |=  (1UL << (0 * 2));
}

void Interrupt_Init(void)
{
    RCC->APB2ENR |= RCC_APB2ENR_SYSCFGEN;

    SYSCFG->EXTICR[0] &= ~SYSCFG_EXTICR1_EXTI0;   /* PA0 -> EXTI0 */

    EXTI->IMR  |=  (1UL << 0);    /* unmask EXTI0 */
    EXTI->EMR  &= ~(1UL << 0);    /* interrupt*/
    EXTI->FTSR |=  (1UL << 0);    /* falling edge (press) */
    EXTI->RTSR &= ~(1UL << 0);

    NVIC_SetPriority(EXTI0_IRQn, 1);
    NVIC_EnableIRQ(EXTI0_IRQn);
}

int main(void)
{
    pressed = 0;
    gpio_Init();
    Interrupt_Init();

    while (1) {
        if (pressed) {
            pressed = 0;
            GPIOC->ODR ^= (1UL << 13);    /* toggle LED */
        }
    }
}

/* Semaphore*/
void EXTI0_IRQHandler(void)
{
    if (EXTI->PR & (1UL << 0)) {
        pressed = 1;
        EXTI->PR = (1UL << 0);
    }
}