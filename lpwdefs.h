/* Cortex-M4 System Control Register */
#define LPW_SCR					(0xE000ED10UL)
#define LPW_SCR_SEVONPEND		(0x01UL << 4)
#define LPW_SCR_SLEEPDEEP		(0x01UL << 2)
#define LPW_SCR_SLEEPONEXIT		(0x01UL << 1)

#define LPW_PWR_CR				(0x40007000UL)

#define LPW_PWR_VOC_1			(0x11UL << 14)
#define LPW_PWR_VOC_2			(0x01UL << 14)
#define LPW_PWR_VOC_3			(0x10UL << 14)

#define LPW_PWR_PDDS			(0x01UL << 1)
#define LPW_PWR_LPDS			(0x01UL << 0)



__attribute__((always_inline)) __STATIC_INLINE void LPW_WFI(void) {
  __ASM volatile ("wfi");
}

__attribute__((always_inline)) __STATIC_INLINE void LPW_WFE(void) {
  __ASM volatile ("wfe");
}
