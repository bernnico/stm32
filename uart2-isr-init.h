/* UART variables ---------------------------------------------------------*/
uint8_t txData[] = "hello from uart 2!\n";
uint8_t rx_data;
uint8_t rx_index = 0;
uint8_t rx_buffer[30];

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
	UNUSED(huart);

	// all messages will end with a newline
	// max message length: 30 chars inclusive \n and \0
	if (rx_index == 0)
	{
		for (int i = 0; i < 30; i++)
		{
			rx_buffer[i] = 0;
		}
	}

	// end of message: LF, CR or length overflow
	// not LF/CR
	if (rx_data != 10 && rx_data != 13 && rx_index != 28)
	{
		rx_buffer[rx_index++] = rx_data;
	}
	else
	{
		// !!!
		// here the uart data register can be not empty
		// nevertheless
		rx_buffer[rx_index++] = '\n';
		rx_buffer[rx_index] = '\0';
		rx_index = 0;
		uint8_t rx_buffer_len = strlen((char *)rx_buffer);
		HAL_UART_Transmit(&huart2, rx_buffer, rx_buffer_len, rx_buffer_len << 1);
	}

	HAL_UART_Receive_IT(&huart2, &rx_data, 1);
}
