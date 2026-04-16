/* main.c - Minimal bare-metal application for RZT2L
 * Owned and qualified by project.
 */

#include "mpu.h"
#include <stdint.h>

/* RZT2L GPIO — direct register access, no FSP dependency.
 * Reference: RZT2L Hardware User Manual, Port registers (R_PORT_SR safe region).
 * Board: RZT2L RSK  LED1 (Yellow) = P17_6, safe port region.
 *
 * R_PORT_SR base: 0x81030000
 *   P[n]   uint8_t  @ base + 0x000 + n     — output data (1 bit/pin)
 *   PM[n]  uint16_t @ base + 0x200 + n*2   — mode: 00=Hi-Z 01=In 10=Out 11=Out+feedback
 *   PMC[n] uint8_t  @ base + 0x400 + n     — 0=GPIO, 1=peripheral
 *
 * PRCR must be unlocked before writing PMC/PM (not needed for P).
 *   R_RWP_NS->PRCRN @ 0x80281A10, R_RWP_S->PRCRS @ 0x81281A00
 *   Unlock: reg = (reg | 0xA500) | 0x0004
 *   Lock:   reg = (reg | 0xA500) & ~0x0004
 */
#define PORT_NSR_BASE 0x800A0000UL   /* R_PORT_NSR — default RSELP after reset assigns all pins here */
#define PORT_P(n)   (*((volatile uint8_t  *)(PORT_NSR_BASE + 0x000U + (uint32_t)(n))))
#define PORT_PM(n)  (*((volatile uint16_t *)(PORT_NSR_BASE + 0x200U + (uint32_t)(n) * 2U)))
#define PORT_PMC(n) (*((volatile uint8_t  *)(PORT_NSR_BASE + 0x400U + (uint32_t)(n))))

#define PRCRN (*((volatile uint32_t *)0x80281A10UL))
#define PRCRS (*((volatile uint32_t *)0x81281A00UL))
#define PRCR_KEY 0xA500U
#define PRCR_GPIO_BIT 0x0004U

#define PRCR_GPIO_UNLOCK()                          \
    do                                              \
    {                                               \
        PRCRN = (PRCRN | PRCR_KEY) | PRCR_GPIO_BIT; \
        PRCRS = (PRCRS | PRCR_KEY) | PRCR_GPIO_BIT; \
    } while (0)
#define PRCR_GPIO_LOCK()                                         \
    do                                                           \
    {                                                            \
        PRCRN = (PRCRN | PRCR_KEY) & (uint32_t)(~PRCR_GPIO_BIT); \
        PRCRS = (PRCRS | PRCR_KEY) & (uint32_t)(~PRCR_GPIO_BIT); \
    } while (0)

#define LED1_PORT 17U /* P17_6 Yellow LED (active-low) */
#define LED1_PIN 6U

static void led_init(void)
{
    /* Unlock GPIO protection for PMC/PM writes — try both with PRCR off completely */
    PRCR_GPIO_UNLOCK();

    /* PMC = 0: GPIO mode (not peripheral) */
    PORT_PMC(LED1_PORT) = 0x00U;

    /* PM bit[13:12] = 10b: output. Write full register to avoid stale read. */
    PORT_PM(LED1_PORT) = (uint16_t)((PORT_PM(LED1_PORT) & ~(uint16_t)(0x3U << (LED1_PIN * 2U)))
                         | (uint16_t)(0x2U << (LED1_PIN * 2U)));

    /* Keep GPIO unlocked throughout — do NOT re-lock */

    /* LED off (active-high: P=0) */
    PORT_P(LED1_PORT) = 0x00U;
}

static void led_toggle(void)
{
    static uint8_t state = 0U;
    state ^= 1U;
    if (state)
    {
        PORT_P(LED1_PORT) |= (uint8_t)(1U << LED1_PIN); /* LED on  (active-high: P=1) */
    }
    else
    {
        PORT_P(LED1_PORT) &= (uint8_t)(~(1U << LED1_PIN)); /* LED off (active-high: P=0) */
    }
}

static void delay(volatile uint32_t count)
{
    while (count-- > 0U)
    { /* spin */
    }
}

int main(void)
{
    mpu_init();
    led_init();

    while (1)
    {
        led_toggle();
        delay(20000000U); /* ~100ms at 800MHz → 5Hz blink */
    }

    return 0;
}
