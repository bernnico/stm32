#include "rodos.h"
#include "my-defs.h"

Application RTC_Application("RTC Application", 33);

void RTC_Clock_enable(void);
void RTC_init(void);
void RTC_Set_Alarm(void);
void RTC_Alarm_Enable(void);

RTC_TimeTypeDef time;
RTC_DateTypeDef date;
RTC_AlarmTypeDef alarm;

class RTC_Controller: public Thread {
private:
//	void RTC_Clock_enable(void);
//	void RTC_init(void);
//	void RTC_Set_Alarm(void);
//	void RTC_Alarm_Enable(void);

public:
	RTC_Controller() :
			Thread("RTC Controller") {
	}

	void init() {

	}

	void run() {
		uint32_t cnt = 0;
		long int counter = 0;

		TIME_LOOP(0, 1000 * MILLISECONDS)
		{
			counter = NOW();
			if (cnt == 5) {
				PRINTF("|=RTC INIT=| -> ");

				RTC_Clock_enable();
				RTC_init();
				RTC_Set_Alarm();
				RTC_Alarm_Enable();

				if ((RCC->BDCR & RCC_BDCR_LSEON) == RCC_BDCR_LSEON) {
					for(int i = 0; i < 2000; i ++);
					PRINTF("|=== RTC ===| LSE ON\r\n");
					for(int i = 0; i < 2000; i ++);
				}
				if ((RCC->CSR & RCC_CSR_LSION) == RCC_CSR_LSION) {
					for(int i = 0; i < 2000; i ++);
					PRINTF("|=== RTC ===| LSI ON\r\n");
					for(int i = 0; i < 2000; i ++);
				}

//				initRTC();
//				PRINTF("|=== ALARM INIT ===|\r\n");
//				initRTCAlarm();
//				PRINTF("|===== Sleep =====|\r\n");
//
//				for(int i = 0; i < 20000; i ++);
////				allActiveInterrupts(DISABLE);
////				__WFI();
////				allActiveInterrupts(ENABLE);
////				for(int i = 0; i < 2000; i ++);
//				PRINTF("|===== Wake up =====|\r\n");

			} else if (cnt > 5) {
				RTC_GetDate(RTC_Format_BIN, &date);
				PRINTF("|=RTC=| %2d.%2d.%d | ",
						date.RTC_Date, date.RTC_Month, date.RTC_Year);

				RTC_GetTime(RTC_Format_BIN, &time);
				PRINTF("%d:%2d:%2d:%3ld   ",
						time.RTC_Hours, time.RTC_Minutes, time.RTC_Seconds, RTC->SSR);

			} else {
				PRINTF("|=RTC=| cnt: %ld   ", cnt);
			}

			cnt++;

			counter = NOW() - counter;
			PRINTF("|=RTC=| NOW(): %ld", counter);
			PRINTF("\r\n");
		}
	}

	/**
	 * AN4759 Rev 4 S.8 (befor LSI/LSE init)
	 * Steps to initialize the calendar
	 */
	void initRTC(void) {
		/* ---------------- LSE INIT --------------------- */
		/* Enable Power Clock*/
		RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR, ENABLE);
		/* Allow access to the backup domain (RTC registers, RTC
		 *     backup data registers) */
										//	RCC_DBP_TIMEOUT_VALUE = 2 ms
		PWR_BackupAccessCmd(ENABLE);	//	PWR->CR |= PWR_CR_DBP;

		/* LSI/LSE used as RTC source clock*/
		/* The RTC Clock may varies due to LSI frequency dispersion. */
		/* Enable the LSI/LSE OSC */
		RCC_LSICmd(DISABLE);
		RCC_LSEConfig(RCC_LSE_ON);

		/* Wait till LSI/LSE is ready */
	//	while (RCC_GetFlagStatus(RCC_FLAG_LSIRDY) == RESET);
		while (RCC_GetFlagStatus(RCC_FLAG_LSERDY) == RESET);

		/* Select the RTC Clock Source */
	//	RCC_RTCCLKConfig(RCC_RTCCLKSource_LSI);
		RCC_RTCCLKConfig(RCC_RTCCLKSource_LSE);

		if ((RCC->BDCR & RCC_BDCR_LSEON) == RCC_BDCR_LSEON) {
			for(int i = 0; i < 2000; i ++);
			PRINTF("|=== RTC ===| LSE ON\r\n");
			for(int i = 0; i < 2000; i ++);
		}

		if ((RCC->CSR & RCC_CSR_LSION) == RCC_CSR_LSION) {
			for(int i = 0; i < 2000; i ++);
			PRINTF("|=== RTC ===| LSI ON\r\n");
			for(int i = 0; i < 2000; i ++);
		}

		/* ================ LSE INIT END ================= */

		/* Enable the RTC Clock */
		RCC_RTCCLKCmd(ENABLE);

		/* Wait for RTC APB registers synchronization */
		RTC_WaitForSynchro();

		/* LSI/LSE LOWPOWER_MODE */
		RCC_LSEModeConfig(RCC_LSE_LOWPOWER_MODE);

		/* Disable the RTC registers write protection */
		RTC->WPR = 0xCA;
		RTC->WPR = 0x53;

		/* Enter Initialization mode */
		RTC->ISR &= ~(RTC_ISR_INIT);
		RTC->ISR |= RTC_ISR_INIT;

		/* Wait for the confirmation */
		while ((RTC->ISR & RTC_ISR_INITF) == RESET);



		/* Program the prescaler values
		 * sub-sec = 2^(-14)
		 * */
		RTC->PRER =     1UL << 16U;
		RTC->PRER = 16383UL <<  0U;

		/* Load time and date values in the shadow registers */
		/* BCD = Binary-coded decimal */
		/* set test value: 16.05.2019 12:33:25 */
		RTC->TR = (1UL << 20U) | (2UL << 16U) | \
				  (3UL << 12U) | (3UL <<  8U) | \
				  (2UL <<  4U) | (5UL <<  0U);

		RTC->DR = (1UL << 20U) | (9UL << 16U) | \
				  (0UL << 12U) | (5UL <<  8U) | \
				  (1UL <<  4U) | (6UL <<  0U);

		/* Configure the time format (12h or 24h) */
		RTC->CR |= RTC_CR_FMT; // 1UL << 6;

		/* Exit Initialization mode */
		RTC->ISR &= ~(RTC_ISR_INIT);

		/* Enable the RTC Registers Write Protection */
		RTC->WPR = 0xFF;
	}

	/**
	* AN4759 Rev 4 S.12
	* Steps to configure the alarm A (B is analog)
	*/
	void initRTCAlarm(void) {
		/* Disable the write protection for RTC registers */
		RTC->WPR = 0xCA;
		RTC->WPR = 0x53;

		/* Disable alarm A */
		RTC->CR &= ~(RTC_CR_ALRAE);


		PRINTF("|===== ALARM INIT =====|   Zeile 168\r\n");


		/* Check that the RTC_ALRMAR register can be accessed */
		uint32_t timeout = 100000;
		while ( (RTC->ISR & RTC_ISR_ALRAWF) == RESET || (--timeout) == 0);


		PRINTF("|===== ALARM INIT =====|   Zeile 176\r\n");


		/* Configure the alarm */

		/* Date, hours and minutes are ignored */
		RTC->ALRMAR |= RTC_ALRMAR_MSK4 | RTC_ALRMAR_MSK3 | RTC_ALRMAR_MSK2;

		/* every 30 seconds */
		RTC->ALRMAR |= (1UL <<  4U) | (0UL <<  0U);


		PRINTF("|===== ALARM INIT =====|   Zeile 188\r\n");


		/* Re-enable alarm A */
		RTC->CR |= RTC_CR_ALRAE;

		/* Enable the RTC Registers Write Protection */
		RTC->WPR = 0xFF;

		RTC_ITConfig(RTC_IT_ALRA, ENABLE);

		NVIC_SetPriority(RTC_Alarm_IRQn, 255);
		NVIC_EnableIRQ(RTC_Alarm_IRQn);
	}

} RTC_Controller;

void RTC_Clock_enable(void) {
	/* Enable write access to the backup domain */
	if ((RCC->APB1ENR & RCC_APB1ENR_PWREN) == RESET) {
		/* Enable power interface clock */
		RCC->APB1ENR |= RCC_APB1ENR_PWREN;
		/* Short delay after enabling an RCC peripheral clock */
		(void) RCC->APB1ENR;
	}

	/* Select LSE as RTC clock source */
	/* RTC Clock:
	 * (1) LSE is in the backup domain
	 * (2) HSE and LSI are not */
	if ((PWR->CR & PWR_CR_DBP) == RESET) {
		/* Enable write access to RTC and registers in backup domain */
		PWR->CR |= PWR_CR_DBP;
		/* Wait until the backup domain write protection has been disabled */
		while ((PWR->CR & PWR_CR_DBP) == RESET);
	}

/* Disable LSI bits */
RCC->CSR &= ~(RCC_CSR_LSION);

	/* Reset LSEON and LSEBYP bits before configuring LSE */
	/* BDCR = Backup Domain Control Register */
	RCC->BDCR &= ~(RCC_BDCR_LSEON | RCC_BDCR_LSEBYP);

	/* RTC Clock selection can changed only if the backup is reset */
	RCC->BDCR |=  RCC_BDCR_BDRST;
	RCC->BDCR &= ~RCC_BDCR_BDRST;


	RCC->BDCR |= RCC_BDCR_LSEON;			/* Enable LSE oscillator */
	/* Wait until LSE clock is ready */
	while ((RCC->BDCR & RCC_BDCR_LSERDY) == RESET) {
//		RCC->BDCR |= RCC_BDCR_LSEON;		/* Enable LSE oscillator */
	}

	/* Select LSE as RTC clock source */
	/* RTCSET[1:0]: 00 = No Clock, 01 = LSE, 11 = HSE */
	RCC->BDCR &= ~RCC_BDCR_RTCSEL; 			/* Clear RTCSEL bits */
	RCC->BDCR |= RCC_BDCR_RTCSEL_0;			/* Select LSE as RTC clock */

	/* Disable power interface clock */
	RCC->APB1ENR &= ~RCC_APB1ENR_PWREN;

}

void RTC_init(void) {
	/* Enable RTC clock */
	RCC->BDCR |= RCC_BDCR_RTCEN;	/* Enable the clock for RTC */

	/* disable write protection of RTC registers by writing disarm keys */
	RTC->WPR = 0xCAU;
	RTC->WPR = 0x53U;

	/* Enter initialization mode to program TR and DR registers */
	RTC->ISR |= RTC_ISR_INIT;
	/* Wait until INITF has been set */
	while ((RTC->ISR & RTC_ISR_INITF) == RESET);

	/* Hour format: 0 = 24 hour/day; 1 = AM/PM hour */
	RTC->CR &= ~RTC_CR_FMT;

	/* Generate a 1Hz clock for the RTC time counter
	 * LSE = 32.758 kHz = 2^15 Hz */
	RTC->PRER |= ((2U<<6) - 1) << 16; /* Asynch_Presscaler = 127 */
	RTC->PRER |= ((2U<<7) - 1);		/* Synch_Prescaler   = 255 */

	/* Set time as 18:35:05 */
	RTC->TR = 0U<<22 | 1U<<20 | 8U<<16 | 3U<<12 | 2U<<8 |  0U<<4 | 5U;
	/* Set date as 2019/05/21 */
	RTC->DR =		   1U<<20 | 9U<<16 | 0U<<12 | 5U<<8 |  2U<<4 | 1U;

	/* Exit initialization mode */
	RTC->ISR &= ~RTC_ISR_INIT;

	/* Enable write protection for RTC registers */
	RTC->WPR = 0xFFU;

}

void RTC_Set_Alarm(void) {

	uint32_t AlarmTimeReg;

	/* Disable alarm A */
	RTC->CR &= ~RTC_CR_ALRAE;

	/* Disable write protection of RTC registers by writing disarm keys */
	RTC->WPR = 0xCAU;
	RTC->WPR = 0x53U;

	/* DisabLe alarm A and its interrupt */
	RTC->CR &= ~RTC_CR_ALRAE;	/* Clear alarm A enable bit */
	RTC->CR &= ~RTC_CR_ALRAIE;	/* Clear alarm A's interrupt enable bit */

	/* Wait until access to alarm registers is allowed
	 * Write flag (ALRAWF) is set by hardware if alarm A can be changed */
	while((RTC->ISR & RTC_ISR_ALRAWF) == RESET);

	/* Set off alarm A if the second is 30
	 * Bits[6 :4} = Ten’s digit for the second in BCD format
	 * Bits[3 :6} = Unit's digit for the second in BCD format */
	AlarmTimeReg = 30U << 4;

	/* Set alarm mask field to compare only the second */
	AlarmTimeReg |= RTC_ALRMAR_MSK4;		/* 1: Ignore day of week in comparison */
	AlarmTimeReg |= RTC_ALRMAR_MSK3;		/* 1: Ignore hour in comparison */
	AlarmTimeReg |= RTC_ALRMAR_MSK2;		/* 1: Ignore minute in alarm comparison */
	AlarmTimeReg &= ~RTC_ALRMAR_MSK1;		/* 0: Alarm sets off if the second match */

	/* RTC alarm A register (ALRMAR) */
	RTC->ALRMAR = AlarmTimeReg;
	RTC->ALRMASSR = 0U;

	/* Enable alarm A and its interrupt */
	RTC->CR |= RTC_CR_ALRAE;				/* Enable alarm A */
	RTC->CR |= RTC_CR_ALRAIE;				/* Enable alarm A interrupt */

	/* Enable write protection for RTC registers */
	RTC->WPR = 0xFFU;

}

void RTC_Alarm_Enable(void) {
	/* Other initialization see RTC_init() */

	/* Configure EXTI 17 */
	/* Select triggering edge */
	EXTI->RTSR |= EXTI_RTSR_TR17; 			/* 1 = Trigger at rising edge */

	/* Interrupt mask register */
	EXTI->IMR |= EXTI_IMR_MR17;				/* 1 = Enable EXTI 17 line */

	/* Event mask register */
	EXTI->EMR |= EXTI_EMR_MR17;				/* 1 = Enable EXTI 17 line */

	/* Interrupt pending register */
	EXTI->PR |= EXTI_PR_PR17;				/* Write 1 to clear pending interrupts */

	/* Enable RTC interrupt*/
	// NVIC->ISER[1] |= 1U << 9;			/* RTC_Alarm_IRQn = 41 */
	NVIC_EnableIRQ(RTC_Alarm_IRQn);

	/* Set interrupt priority as the most urgent */
	NVIC_SetPriority(RTC_Alarm_IRQn , 0);

}

//#if defined  RTC_Alarm_IRQHandler
//#undef RTC_Alarm_IRQHandler
//#endif


/**
  * @brief This function handles RTC alarms A and B interrupt through EXTI line 17.
  */

extern "C" {
void RTC_Alarm_IRQHandler(void);
void RTC_Alarm_IRQHandler(void) {
	/* RTC initialization and status register (RTC_ISR)
	 * Hardware sets the Alarm A flag (ALRAF) when the time/date registers
	 * (RTC_TR and RTC_DR) match the Alarm A register (RTC_ALRMAR), according
	 * to the mask bits
	 */

	if (RTC->ISR & RTC_ISR_ALRAF) {
		/* Print something */
		PRINTF("\n|========= ALARM =========|\n");
		/* Clear the Alarm A interrupt flag */
		RTC->ISR &= ~(RTC_ISR_ALRAF);
	}


	/* Clear the EXTI line 17 */
	EXTI->PR |= EXTI_PR_PR17;				/* Write 1 to clear pending interrupts */
//	NVIC_ClearPendingIRQ(RTC_Alarm_IRQn);

}
}






