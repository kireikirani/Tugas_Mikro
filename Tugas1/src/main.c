#include <stm32f4xx.h>
#include <stdint.h>

//Define pin LED

#define LED_HIJAU    1   // PB1 - TIM4 (2 s)
#define LED_KUNING   2   // PB2 - TIM3 (0.5 s)
#define LED_MERAH    3   // PB3 - TIM2 (1 s)

volatile uint8_t statusMerah  = 0;
volatile uint8_t statusKuning = 0;
volatile uint8_t statusHijau  = 0;

void GPIO_Init(void)
{
    // Enable GPIOB clock
    RCC->AHB1ENR |= RCC_AHB1ENR_GPIOBEN;

    // Configure PB1, PB2, PB3 as output
    GPIOB->MODER &= ~(
        (3U << (LED_MERAH * 2)) |
        (3U << (LED_KUNING * 2)) |
        (3U << (LED_HIJAU * 2))
    );

    GPIOB->MODER |= (
        (1U << (LED_MERAH * 2)) |
        (1U << (LED_KUNING * 2)) |
        (1U << (LED_HIJAU * 2))
    );

    GPIOB->OTYPER &= ~(
        (1U << LED_MERAH) |
        (1U << LED_KUNING) |
        (1U << LED_HIJAU)
    );

    GPIOB->OSPEEDR |= (
        (1U << (LED_MERAH * 2)) |
        (1U << (LED_KUNING * 2)) |
        (1U << (LED_HIJAU * 2))
    );

    // All LEDs OFF
    GPIOB->BSRR =
        (1U << (LED_MERAH + 16)) |
        (1U << (LED_KUNING + 16)) |
        (1U << (LED_HIJAU + 16));
}


// TIM2 INITIALIZATION - LED MERAH. Interval = 1s

void TIM2_Init(void)
{
    // Enable TIM2 clock
    RCC->APB1ENR |= RCC_APB1ENR_TIM2EN;

    // Stop timer during configuration
    TIM2->CR1 = 0;

    TIM2->PSC = 1600;
    TIM2->ARR = 9999;

    // Reset counter
    TIM2->CNT = 0;
    TIM2->EGR = TIM_EGR_UG;
    TIM2->SR &= ~TIM_SR_UIF;
    TIM2->DIER |= TIM_DIER_UIE;

    NVIC_SetPriority(TIM2_IRQn, 0);
    NVIC_EnableIRQ(TIM2_IRQn);

    // Enable timer
    TIM2->CR1 |= TIM_CR1_CEN;
}



// TIM3 INITIALIZATION - LED KUNING Interval: 0.5s

void TIM3_Init(void)
{
    // Enable TIM3 clock
    RCC->APB1ENR |= RCC_APB1ENR_TIM3EN;

    // Stop timer during configuration
    TIM3->CR1 = 0;
    TIM3->PSC = 1600;
    TIM3->ARR = 4999;

    // Reset counter
    TIM3->CNT = 0;
    TIM3->EGR = TIM_EGR_UG;
    TIM3->SR &= ~TIM_SR_UIF;
    TIM3->DIER |= TIM_DIER_UIE;
    NVIC_SetPriority(TIM3_IRQn, 1);
    NVIC_EnableIRQ(TIM3_IRQn);

    // Enable timer
    TIM3->CR1 |= TIM_CR1_CEN;
}

// TIM4 INITIALIZATION - LED HIJAU. Interval = 2s.

void TIM4_Init(void)
{
    // Enable TIM4 clock
    RCC->APB1ENR |= RCC_APB1ENR_TIM4EN;

    // Stop timer during configuration
    TIM4->CR1 = 0;
    TIM4->PSC = 1600;
    TIM4->ARR = 19999;
    
    // Reset counter
    TIM4->CNT = 0;
    TIM4->EGR = TIM_EGR_UG;
    TIM4->SR &= ~TIM_SR_UIF;
    TIM4->DIER |= TIM_DIER_UIE;
    NVIC_SetPriority(TIM4_IRQn, 2);
    NVIC_EnableIRQ(TIM4_IRQn);

    // Enable timer
    TIM4->CR1 |= TIM_CR1_CEN;
}

// TIM2 INTERRUPT HANDLER
// Control LED MERAH - PB1
void TIM2_IRQHandler(void){
    if (TIM2->SR & TIM_SR_UIF)
    {
        // Clear interrupt flag
        TIM2->SR &= ~TIM_SR_UIF;

        // Toggle LED Merah
        statusMerah = !statusMerah;

        if (statusMerah)
            GPIOB->BSRR = (1U << LED_MERAH);
        else
            GPIOB->BSRR = (1U << (LED_MERAH + 16));
    }
}
// TIM3 INTERRUPT HANDLER
// Control LED KUNING - PB2

void TIM3_IRQHandler(void)
{
    if (TIM3->SR & TIM_SR_UIF)
    {
        // Clear interrupt flag
        TIM3->SR &= ~TIM_SR_UIF;

        // Toggle LED Kuning
        statusKuning = !statusKuning;

        if (statusKuning)
            GPIOB->BSRR = (1U << LED_KUNING);
        else
            GPIOB->BSRR = (1U << (LED_KUNING + 16));
    }
}

// TIM4 INTERRUPT HANDLER
// Control LED HIJAU - PB3

void TIM4_IRQHandler(void)
{
    if (TIM4->SR & TIM_SR_UIF)
    {
        // Clear interrupt flag
        TIM4->SR &= ~TIM_SR_UIF;

        // Toggle LED Hijau
        statusHijau = !statusHijau;

        if (statusHijau)
            GPIOB->BSRR = (1U << LED_HIJAU);
        else
            GPIOB->BSRR = (1U << (LED_HIJAU + 16));
    }
}

int main(void)
{
    // Initialize GPIO
    GPIO_Init();

    // Initialize timers
    TIM2_Init();
    TIM3_Init();
    TIM4_Init();

    while (1)
    {
        // Main loop does nothing
    }
}