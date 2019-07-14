static void myClockConfig()
{
		// 1 set oscillator
		RCC->CR |= RCC_CR_HSEON; //  (0x1UL << (16U))
		// is HSE ready
		while ((RCC->CR & RCC_CR_HSERDY) != RCC_CR_HSERDY); // (0x1UL << (17U))

		// 2 clock configuration and latency update
		/*------------------ PCLK1 Configuration ---------------------*/
//		MODIFY_REG(RCC->CFGR, RCC_CFGR_PPRE1, RCC_HCLK_DIV1);
		RCC->CFGR &= ~(7U << 10);
		RCC->CFGR |= RCC_HCLK_DIV1;
		/*------------_----- PCLK2 Configuration ---------_-----------*/
//		MODIFY_REG(RCC->CFGR, RCC_CFGR_PPRE2, (RCC_HCLK_DIV1 << 3));
		RCC->CFGR &= ~(7U << 13);
		RCC->CFGR |= RCC_HCLK_DIV1 << 3;
		/*-------------------- HCLK Configuration ---------------------*/
//		MODIFY_REG(RCC->CFGR, RCC_CFGR_HPRE, RCC_SYSCLK_DIV1);
		RCC->CFGR &= ~(7U << 4);
		RCC->CFGR |= RCC_SYSCLK_DIV4;
		/*------------------- SYSCLK Configuration --------------------*/
//		__HAL_RCC_SYSCLK_CONFIG(RCC_SYSCLKSOURCE_HSE);
		RCC->CFGR &= ~RCC_CFGR_SW;
		RCC->CFGR |= RCC_SYSCLKSOURCE_HSE;

		while ((RCC->CFGR & RCC_CFGR_SWS) != (RCC_SYSCLKSOURCE_HSE << RCC_CFGR_SWS_Pos)) { }

//		__HAL_FLASH_SET_LATENCY(FLASH_LATENCY_0);
//	    if(__HAL_FLASH_GET_LATENCY() != FLASH_LATENCY_0) return HAL_ERROR;

		/* Update the SystemCoreClock global variable */
		SystemCoreClockUpdate();

}
