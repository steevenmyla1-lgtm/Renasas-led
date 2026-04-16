/* mpu.c - Minimal Cortex-R52 MPU initialisation
 * Owned and qualified by project.
 * Direct CP15 register access — no CMSIS dependency.
 * Reference: Arm Cortex-R52 TRM (DDI0600), ARMv8-R Architecture Reference Manual
 */

#include "mpu.h"
#include <stdint.h>

/* Cortex-R52 PMSAv8 MPU CP15 register access macros
 * MCR/MRC p15, <opc1>, <Rt>, <CRn>, <CRm>, <opc2>
 * Reference: ARMv8-R AArch32 Architecture Reference Manual
 */
#define MPU_WRITE_RGNR(v)  __asm__ volatile("mcr p15, 0, %0, c6, c2, 0" :: "r"(v))  /* Region Number */
#define MPU_WRITE_PRBAR(v) __asm__ volatile("mcr p15, 0, %0, c6, c3, 0" :: "r"(v))  /* Region Base Address */
#define MPU_WRITE_PRLAR(v) __asm__ volatile("mcr p15, 0, %0, c6, c3, 1" :: "r"(v))  /* Region Limit Address */
#define MPU_WRITE_CTRL(v)  __asm__ volatile("mcr p15, 0, %0, c1, c0, 0" :: "r"(v))  /* SCTLR */
#define MPU_READ_CTRL(v)   __asm__ volatile("mrc p15, 0, %0, c1, c0, 0"  : "=r"(v)) /* SCTLR */
#define ISB()              __asm__ volatile("isb" ::: "memory")
#define DSB()              __asm__ volatile("dsb" ::: "memory")

/* PRBAR bits */
#define PRBAR_AP_RW_RW   (0x1U << 1)   /* PL1 RW, PL0 RW */
#define PRBAR_XN_NONE    (0x0U << 3)   /* execute allowed */
#define PRBAR_XN_EXEC    (0x2U << 3)   /* execute-never PL0+PL1 */

/* PRLAR bits */
#define PRLAR_EN         (1U << 0)     /* region enable */
#define PRLAR_ATTR(n)    ((n) << 1)    /* memory attribute index */

/* SCTLR bits */
#define SCTLR_M          (1U << 0)     /* MPU enable */
#define SCTLR_BR         (1U << 17)    /* background region enable */

/* Memory attribute indices (MAIR) — index 0=Normal WB, 1=Device */
#define ATTR_NORMAL_WB   0U
#define ATTR_DEVICE      1U

/* Region limit address: address of last byte in region, bits[4:0] = PRLAR attrs */
#define PRLAR_LIMIT(end_addr, attr, en) \
    (((end_addr) & ~0x3FU) | PRLAR_ATTR(attr) | ((en) ? PRLAR_EN : 0U))

#define MPU_REGION_COUNT  4U

typedef struct {
    uint32_t number;  /* region index */
    uint32_t prbar;   /* base address + AP + XN */
    uint32_t prlar;   /* limit address + attr + enable */
} mpu_region_t;

static const mpu_region_t mpu_regions[MPU_REGION_COUNT] = {
    /* Region 0: ATCM  0x00000000–0x0007FFFF  512KB  code+data  RW  cacheable */
    { 0U, 0x00000000U | PRBAR_AP_RW_RW | PRBAR_XN_NONE,
          PRLAR_LIMIT(0x0007FFFFU, ATTR_NORMAL_WB, 1U) },
    /* Region 1: BTCM  0x00100000–0x0010FFFF   64KB  stacks     RW  cacheable */
    { 1U, 0x00100000U | PRBAR_AP_RW_RW | PRBAR_XN_EXEC,
          PRLAR_LIMIT(0x0010FFFFU, ATTR_NORMAL_WB, 1U) },
    /* Region 2: SYSRAM 0x10000000–0x100FFFFF   1MB  data       RW  cacheable */
    { 2U, 0x10000000U | PRBAR_AP_RW_RW | PRBAR_XN_EXEC,
          PRLAR_LIMIT(0x100FFFFFU, ATTR_NORMAL_WB, 1U) },
    /* Region 3: Peripherals 0x40000000–0xFFFFFFFF  Device  RW  XN */
    { 3U, 0x40000000U | PRBAR_AP_RW_RW | PRBAR_XN_EXEC,
          PRLAR_LIMIT(0xFFFFFFC0U, ATTR_DEVICE, 1U) },
};

void mpu_init(void)
{
    uint32_t sctlr;
    uint32_t i;

    DSB();

    /* Disable MPU before configuration (clear SCTLR.M) */
    MPU_READ_CTRL(sctlr);
    MPU_WRITE_CTRL(sctlr & ~SCTLR_M);
    ISB();

    /* Configure each region */
    for (i = 0U; i < MPU_REGION_COUNT; i++) {
        MPU_WRITE_RGNR(mpu_regions[i].number);
        ISB();
        MPU_WRITE_PRBAR(mpu_regions[i].prbar);
        MPU_WRITE_PRLAR(mpu_regions[i].prlar);
    }

    /* Enable MPU + background region for privileged access (SCTLR.M + SCTLR.BR) */
    MPU_READ_CTRL(sctlr);
    MPU_WRITE_CTRL(sctlr | SCTLR_M | SCTLR_BR);
    ISB();
    DSB();
}
