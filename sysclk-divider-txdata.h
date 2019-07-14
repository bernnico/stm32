uint8_t cnt = 0;
uint32_t toMHz = 1000*1000;
uint8_t txData[] = "0123456789abcdef\r\n";

void setTxData(uint8_t *txData) {
  txData[0] = 's';
  txData[1] = 'c';
  txData[2] = (uint8_t)(HAL_RCC_GetSysClockFreq() / toMHz);
  txData[3] = '_';
  txData[4] = 'h';
  txData[5] = (uint8_t)(HAL_RCC_GetHCLKFreq() / toMHz);
  txData[6] = '_';
  txData[7] = 'p';
  txData[8] = '1';
  txData[9] = (uint8_t)(HAL_RCC_GetPCLK1Freq() / toMHz);
  txData[10] = '_';
  txData[11] = 'p';
  txData[12] = '1';
  txData[13] = (uint8_t)(HAL_RCC_GetPCLK2Freq() / toMHz);
  txData[14] = ' ';
  txData[15] = '0';
//  txData[16] = ' ';
//  txData[17] = ' ';

//  while(1) {
//    HAL_UART_Transmit(&huart2, txData, 18, 50);
//    HAL_Delay(1000);
//    txData[15] = cnt++;
//  }
}


  
