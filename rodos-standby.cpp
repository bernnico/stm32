#include "rodos.h"
#include "lpdefs.h"

Application StandbyApplication("Standby Application", 33);

RCC_ClocksTypeDef rcc_clocks;
char clockSource[4];
void getClockSource(uint8_t asInt);



class StandbyController: public Thread {
public:
	StandbyController() :
			Thread("StandbyController") {
	}

	void init() {
//		RCC_GetClocksFreq(&rcc_clocks);
		getClockSource(RCC_GetSYSCLKSource());
	}

	void run() {
		int cnt = 0;
		uint32_t temp;
		TIME_LOOP(0, 1000 * MILLISECONDS) {
			temp = (uint32_t)RTC->TR;
			if ((temp & 0xFF) == 0x20) {
				PRINTF("|= SLEEP =|   RTC->TR: %ld", temp);
				for(int i = 0; i < 2000; i++);
//				standbyMode();
			}
			else {
				PRINTF("|=PWR=| Source Clock: %s | cnt: //%d\r\n", clockSource, cnt);
			}

			cnt++;
		}
	}

private:
	void standbyMode(void) {

		RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR, ENABLE);
//		PWR_BackupAccessCmd(ENABLE);

		/* Enable the Alarm Interrupt */
//		RTC_ITConfig(RTC_IT_ALRA, ENABLE);

		/* Enable the wakeup pin */
//		PWR_WakeUpPinCmd(ENABLE);

		/* Clear standby flag */
		PWR_ClearFlag(PWR_FLAG_SB);

		/* Clear Wakeup flag*/
		PWR_ClearFlag(PWR_FLAG_WU);

		/* Request to enter STANDBY mode */
		PWR_EnterSTANDBYMode();
	}

//	void RTC_Wakeup_Configuration(void) {
//		/* Disable the write protection for RTC registers */
//		RTC->WPR = 0xCA;
//		RTC->WPR = 0x53;
//
//		/* Wake up initialization can only be performed when wake up is disable */
//		RTC->CR &= ~RTC_CR_WUTE;		/* Disable wake up counter */
//
//		/* WUTWF: Wake up timer write flag
//		 * 0: Wake up timer configuration update not allow
//		 * 1: Wake up timer configuration update allow */
//		while ((RTC->ISR & RTC_ISR_WUTWF) == RESET);
//
//		/* WUCKSEL[2:0]: wake up clock selected
//		 * 10x: ck_spre (usually 1Hz) clock is selected */
//		RTC->CR &= ~RTC_CR_WUCKSEL;
//		RTC->CR |=  RTC_CR_WUCKSEL_2;	/* Select ck_spre (1Hz) */
//
//		/* RTC wake up timer register (Max = 0xFFFF) */
//		RTC->WUTR = 5;			/* The counter decrements by 1 every pulse of */
//								/* the clock selected by WUCKSEL */
//
//		/* Enable wake up counter and wake up interrupt */
//		RTC->CR |= RTC_CR_WUTIE;	/* Enable wake up interrupt */
//		RTC->CR |= RTC_CR_WUTE;		/* Enable wake up counter */
//
//		/* Enable the write protection for RTC registers */
//		RTC->WPR = 0xFF;
//
//	}
//
//	void Enter_SleepMode(void) {
//		/* Cortex system control register
//		 * 0 = sleep, 1 = deep sleep */
//		SCB->SCR &= ~SCB_SCR_SLEEPDEEP_Msk;
//
//		/* SLEEPONEXIT: Indicates sleep-on-exit when returning from handler
//		 * mode to thread mode:
//		 * 0 = do not sleep when returning to thread mode
//		 * 1 = enter sleep, or deep sleep, on return from an ISR */
//		SCB->SCR &= ~SCB_SCR_SLEEPONEXIT_Msk;
//
//		/* RTC wake up interrupt is connected to EXTI line 22 internally.
//		 * Enable EXTI22 interrupt */
//		EXTI->IMR |= 1UL << 22U;
//
//		/* Enable EXTI22 interrupt */
//		EXTI->EMR |= 1UL << 22U;
//
//		/* Select rising-edge trigger */
//		EXTI->RTSR |= 1UL << 22U;
//
//		NVIC_EnableIRQ(RTC_WKUP_IRQn);
//		NVIC_SetPriority(RTC_WKUP_IRQn, 0);
//
//		__DSB();		/* Ensure that the last store takes effect */
//		__WFI();		/* Switch processor into the sleep mode */
//	}

} StandbyController;

void getClockSource(uint8_t asInt) {
	clockSource[0] = 'H';
	clockSource[1] = 'S';
	switch (asInt) {
	case 0x00:
		clockSource[2] = 'I';
		break;
	case 0x04:
		clockSource[2] = 'E';
		break;
	case 0x08:
		clockSource[0] = 'P';
		clockSource[1] = 'L';
		clockSource[2] = 'L';
		break;
	}
}

//void allActiveInterrupts(FunctionalState stat) {
//	if (stat == ENABLE) {
//		Timer::start();
//		NVIC_EnableIRQ(TIMx_IRQn);
//		NVIC_EnableIRQ(EXTI0_IRQn);
//		NVIC_EnableIRQ(EXTI1_IRQn);
//		NVIC_EnableIRQ(EXTI2_IRQn);
//		NVIC_EnableIRQ(EXTI3_IRQn);
//		NVIC_EnableIRQ(EXTI4_IRQn);
//		NVIC_EnableIRQ(EXTI9_5_IRQn);
//		NVIC_EnableIRQ(EXTI15_10_IRQn);
//		NVIC_EnableIRQ(TIM5_IRQn);
//		NVIC_EnableIRQ(USART1_IRQn);
//	} else {
//		Timer::stop();
//		NVIC_DisableIRQ(TIMx_IRQn);
//		NVIC_DisableIRQ(EXTI0_IRQn);
//		NVIC_DisableIRQ(EXTI1_IRQn);
//		NVIC_DisableIRQ(EXTI2_IRQn);
//		NVIC_DisableIRQ(EXTI3_IRQn);
//		NVIC_DisableIRQ(EXTI4_IRQn);
//		NVIC_DisableIRQ(EXTI9_5_IRQn);
//		NVIC_DisableIRQ(EXTI15_10_IRQn);
//		NVIC_DisableIRQ(TIM5_IRQn);
//		NVIC_DisableIRQ(USART1_IRQn);
//	}
//}
//
void GPIO_AnalogConfig(void) {
	GPIO_InitTypeDef  GPIO_InitStructure;
	/* Configure GPIOs as Analog input to reduce current consumption*/

	RCC_AHB1PeriphClockCmd (RCC_AHB1Periph_GPIOA | RCC_AHB1Periph_GPIOB , ENABLE);
//	                        RCC_AHB1Periph_GPIOC | RCC_AHB1Periph_GPIOD |
//	                        RCC_AHB1Periph_GPIOE | RCC_AHB1Periph_GPIOH , ENABLE);

	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AN;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_All;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
	GPIO_Init(GPIOB, &GPIO_InitStructure);
//	GPIO_Init(GPIOC, &GPIO_InitStructure);
//	GPIO_Init(GPIOD, &GPIO_InitStructure);
//	GPIO_Init(GPIOE, &GPIO_InitStructure);
//	GPIO_Init(GPIOH, &GPIO_InitStructure);

	RCC_AHB1PeriphClockCmd (RCC_AHB1Periph_GPIOA | RCC_AHB1Periph_GPIOB , DISABLE);
//	                        RCC_AHB1Periph_GPIOC | RCC_AHB1Periph_GPIOD |
//	                        RCC_AHB1Periph_GPIOE | RCC_AHB1Periph_GPIOH , DISABLE);
}






