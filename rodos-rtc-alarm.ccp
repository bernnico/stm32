#include "rodos.h"
#include "lpdefs.h"

Application RTC_Application("RTC Application", 33);

RTC_TimeTypeDef time;
RTC_DateTypeDef date;



class RTC_Controller: public Thread {
public:
	RTC_Controller() : Thread("RTC Controller") { }
	void init() {
		RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR, ENABLE);
		PWR_BackupAccessCmd(ENABLE);

		/* Initialize RTC only once */
		if (PWR_GetFlagStatus(PWR_FLAG_SB) != SET) {
			RTC_Clock_enable();
			RTC_init();
		}

		RTC_Set_Alarm();
		RTC_Alarm_Enable();

//		PWR_BackupAccessCmd(DISABLE);
		RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR, DISABLE);
	}

	void run() {
		uint32_t cnt = 0;

		TIME_LOOP(0, 1000 * MILLISECONDS) {
			if (cnt == 0) {

				if ((RCC->BDCR & RCC_BDCR_LSEON) == RCC_BDCR_LSEON) {
					PRINTF("|=RTC=| LSE ON\r\n");
				}
				if ((RCC->CSR & RCC_CSR_LSION) == RCC_CSR_LSION) {
					PRINTF("|=RTC=| LSI ON\r\n");
				}
			}
			RTC_GetDate(RTC_Format_BIN, &date);
			PRINTF("|=RTC=| %2d.%2d.%d | ",
					date.RTC_Date, date.RTC_Month, date.RTC_Year);

			RTC_GetTime(RTC_Format_BIN, &time);
			PRINTF("%d:%2d:%2d:",
					time.RTC_Hours, time.RTC_Minutes, time.RTC_Seconds);

			PRINTF("  cnt: %ld\r\n", cnt);
			cnt++;
		}
	}

private:
	void RTC_Clock_enable(void) {
		/* Enable write access to the backup domain */
		if ((RCC->APB1ENR & RCC_APB1ENR_PWREN) == RESET) {
//			RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR, ENABLE);
			RCC->APB1ENR |= RCC_APB1ENR_PWREN;			/* Enable power interface clock */

			(void) RCC->APB1ENR;						/* Short delay after enabling an RCC peripheral clock */
		}

		/* Select LSE as RTC clock source */
		/* RTC Clock:
		 * (1) LSE is in the backup domain
		 * (2) HSE and LSI are not */
		if ((PWR->CR & PWR_CR_DBP) == RESET) {
//			PWR_BackupAccessCmd(ENABLE);
			PWR->CR |= PWR_CR_DBP;						/* Enable write access to RTC and registers in backup domain */

			while ((PWR->CR & PWR_CR_DBP) == RESET);	/* Wait until the backup domain write protection has been disabled */
		}

//		RCC->CSR &= ~(RCC_CSR_LSION);					/* Disable LSI bits. If it was initialized by RODOS */


		RCC->BDCR &= ~(RCC_BDCR_LSEON|RCC_BDCR_LSEBYP);	/* Reset LSEON and LSEBYP bits before configuring LSE */

		RCC->BDCR |=  RCC_BDCR_BDRST;					/* RTC Clock selection can changed only if the backup is reset */
		RCC->BDCR &= ~RCC_BDCR_BDRST;

		RCC->BDCR |= RCC_BDCR_LSEON;					/* Enable LSE oscillator */
		while ((RCC->BDCR & RCC_BDCR_LSERDY) == RESET);	/* Wait until LSE clock is ready */
//		RCC_LSEConfig(RCC_LSE_ON);
//		while (RCC_GetFlagStatus(RCC_FLAG_LSERDY) == RESET);

		/* Select LSE as RTC clock source */
		/* RTCSET[1:0]: 00 = No Clock, 01 = LSE, 11 = HSE */
		RCC->BDCR &= ~RCC_BDCR_RTCSEL; 					/* Clear RTCSEL bits */
		RCC->BDCR |= RCC_BDCR_RTCSEL_0;					/* Select LSE as RTC clock */
//		RCC_RTCCLKConfig(RCC_RTCCLKSource_LSE);

		RCC->APB1ENR &= ~RCC_APB1ENR_PWREN;				/* Disable power interface clock */
//		RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR, DISABLE);
	}

	void RTC_init(void) {
		RCC->BDCR |= RCC_BDCR_RTCEN;					/* Enable the clock for RTC */
//		RCC_RTCCLKCmd(ENABLE);

		RTC->WPR = 0xCA;								/* disable write protection of RTC registers */
		RTC->WPR = 0x53;								/* by writing disarm keys */

///* Clear RSF flag */
//RTC->ISR &= ((uint32_t)0xFFFFFF5F);
//while ((RTC->ISR & RTC_ISR_RSF) == RESET);

		RCC_LSEModeConfig(RCC_LSE_LOWPOWER_MODE);		/* LSE LOWPOWER_MODE */

		RTC->ISR |= RTC_ISR_INIT;						/* Enter initialization mode to program TR and DR registers */
		while ((RTC->ISR & RTC_ISR_INITF) == RESET);	/* Wait until INITF has been set */

		RTC->CR &= ~RTC_CR_FMT;							/* Hour format: 0 = 24 hour/day; 1 = AM/PM hour */

		/* Generate a 1Hz clock for the RTC time counter, LSE = 32.758 kHz = 2^15 Hz */
		RTC->PRER |= ((2U<<6) - 1) << 16; 				/* Asynch_Presscaler = 127 */
		RTC->PRER |= ((2U<<7) - 1);		  				/* Synch_Prescaler   = 255 */

		/* Set time as 18:32:02 */
		RTC->TR = 0U<<22 | 1U<<20 | 8U<<16 | 3U<<12 | 2U<<8 |  0U<<4 | 2U;
		/* Set date as 2019/05/21 */
		RTC->DR =		   1U<<20 | 9U<<16 | 0U<<12 | 5U<<8 |  2U<<4 | 1U;

		RTC->ISR &= ~RTC_ISR_INIT;						/* Exit initialization mode */

		RTC->WPR = 0xFF;								/* Enable write protection for RTC registers */
	}

	void RTC_Set_Alarm(void) {
		uint32_t AlarmTimeReg;

		RTC->CR &= ~RTC_CR_ALRAE;						/* Disable alarm A */

		RTC->WPR = 0xCA;								/* Disable write protection of RTC registers */
		RTC->WPR = 0x53;								/* by writing disarm keys */

														/* DisabLe alarm A and its interrupt */
		RTC->CR &= ~RTC_CR_ALRAE;						/* Clear alarm A enable bit */
		RTC->CR &= ~RTC_CR_ALRAIE;						/* Clear alarm A's interrupt enable bit */

		while((RTC->ISR & RTC_ISR_ALRAWF) == RESET);	/* Wait until access to alarm registers is allowed */
		 	 	 	 	 	 	 	 	 	 	 	 	/* Write flag (ALRAWF) is set by hardware if alarm A can be changed */

		/* Set off alarm A if the second is 30
		 * Bits[6 :4} = Ten’s digit for the second in BCD format
		 * Bits[3 :6} = Unit's digit for the second in BCD format */
		AlarmTimeReg = (3U << 4) | 0U;
														/* Set alarm mask field to compare only the second */
		AlarmTimeReg |= RTC_ALRMAR_MSK4;				/* 1: Ignore day of week in comparison */
		AlarmTimeReg |= RTC_ALRMAR_MSK3;				/* 1: Ignore hour in comparison */
		AlarmTimeReg |= RTC_ALRMAR_MSK2;				/* 1: Ignore minute in alarm comparison */
		AlarmTimeReg &= ~RTC_ALRMAR_MSK1;				/* 0: Alarm sets off if the second match */

		RTC->ALRMAR = AlarmTimeReg;						/* RTC alarm A register (ALRMAR) */
		RTC->ALRMASSR = 0U;

														/* Enable alarm A and its interrupt */
		RTC->CR |= RTC_CR_ALRAE;						/* Enable alarm A */
		RTC->CR |= RTC_CR_ALRAIE;						/* Enable alarm A interrupt */

		RTC->WPR = 0xFF;								/* Enable write protection for RTC registers */
	}

	void RTC_Alarm_Enable(void) {
		/* Configure EXTI 17 */
														/* Select triggering edge */
		EXTI->RTSR |= EXTI_RTSR_TR17; 					/* 1 = Trigger at rising edge */

														/* Interrupt mask register */
		EXTI->IMR |= EXTI_IMR_MR17;						/* 1 = Enable EXTI 17 line */

														/* Event mask register */
		EXTI->EMR |= EXTI_EMR_MR17;						/* 1 = Enable EXTI 17 line */

														/* Interrupt pending register */
		EXTI->PR |= EXTI_PR_PR17;						/* Write 1 to clear pending interrupts */

														/* Enable RTC interrupt*/
		// NVIC->ISER[1] |= 1U << 9;					/* RTC_Alarm_IRQn = 41 */
		NVIC_EnableIRQ(RTC_Alarm_IRQn);

		NVIC_SetPriority(RTC_Alarm_IRQn , 0);			/* Set interrupt priority as the most urgent */
	}

} RTC_Controller;



extern "C" {
/**
  * @brief This function handles RTC alarms A and B interrupt through EXTI line 17.
  */
void RTC_Alarm_IRQHandler(void) {
	/* RTC initialization and status register (RTC_ISR)
	 * Hardware sets the Alarm A flag (ALRAF) when the time/date registers
	 * (RTC_TR and RTC_DR) match the Alarm A register (RTC_ALRMAR), according
	 * to the mask bits
	 */

	if (RTC->ISR & RTC_ISR_ALRAF) {
		PRINTF("\n|=== ALARM ===|\n\n");		/* Print something */

		RTC->ISR &= ~(RTC_ISR_ALRAF);			/* Clear the Alarm A interrupt flag */
	}

	/* Clear the EXTI line 17 */
	EXTI->PR |= EXTI_PR_PR17;					/* Write 1 to clear pending interrupts */
//	NVIC_ClearPendingIRQ(RTC_Alarm_IRQn);
}
}
