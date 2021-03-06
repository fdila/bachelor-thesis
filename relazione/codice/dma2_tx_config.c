hdma_usart6_tx.Instance = DMA2_Stream6;
hdma_usart6_tx.Init.Channel = DMA_CHANNEL_5;
hdma_usart6_tx.Init.Direction = DMA_MEMORY_TO_PERIPH;
hdma_usart6_tx.Init.PeriphInc = DMA_PINC_DISABLE;
hdma_usart6_tx.Init.MemInc = DMA_MINC_ENABLE;
hdma_usart6_tx.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
hdma_usart6_tx.Init.MemDataAlignment = DMA_MDATAALIGN_BYTE;
hdma_usart6_tx.Init.Mode = DMA_CIRCULAR;
hdma_usart6_tx.Init.Priority = DMA_PRIORITY_LOW;
