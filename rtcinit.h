/**
  * @brief  Configures the RTC peripheral and select the clock source.
  * @param  None
  * @retval None
  */
static void RTC_Config(void)
{
  RTC_InitTypeDef  RTC_InitStructure;
  RTC_TimeTypeDef  RTC_TimeStruct;
  RTC_DateTypeDef  RTC_DateStruct;

  /* Enable the PWR clock */
  RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR, ENABLE);

  /* Allow access to RTC */
  PWR_BackupAccessCmd(ENABLE);

  /* Reset RTC Domain */
  RCC_BackupResetCmd(ENABLE);
  RCC_BackupResetCmd(DISABLE);

  /* Enable the LSE OSC */
  RCC_LSEConfig(RCC_LSE_ON);

  /* Wait till LSE is ready */
  while(RCC_GetFlagStatus(RCC_FLAG_LSERDY) == RESET)
  {
  }

  /* Select the RTC Clock Source */
  RCC_RTCCLKConfig(RCC_RTCCLKSource_LSE);

  /* Enable the RTC Clock */
  RCC_RTCCLKCmd(ENABLE);

  /* Wait for RTC APB registers synchronisation */
  RTC_WaitForSynchro();

  /* Configure the RTC data register and RTC prescaler */
  /* ck_spre(1Hz) = RTCCLK(LSI) /(AsynchPrediv + 1)*(SynchPrediv + 1)*/
  RTC_InitStructure.RTC_AsynchPrediv = 0x7F;
  RTC_InitStructure.RTC_SynchPrediv  = 0xFF;
  RTC_InitStructure.RTC_HourFormat   = RTC_HourFormat_24;
  RTC_Init(&RTC_InitStructure);

  /* Set the time to 00h 00mn 00s AM */
  RTC_TimeStruct.RTC_H12     = RTC_H12_AM;
  RTC_TimeStruct.RTC_Hours   = 9;
  RTC_TimeStruct.RTC_Minutes = 31;
  RTC_TimeStruct.RTC_Seconds = 0;
  RTC_SetTime(RTC_Format_BIN, &RTC_TimeStruct);

  /* Wait for RTC APB registers synchronisation */
  RTC_WaitForSynchro();

  RTC_DateStruct.RTC_Date = 25;
  RTC_DateStruct.RTC_Month = 10;
  RTC_DateStruct.RTC_WeekDay = 5;  // 1 is monday
  RTC_DateStruct.RTC_Year = 13;
  RTC_SetDate(RTC_Format_BIN, &RTC_DateStruct);
}
