#include "main.h"
#include "app_x-cube-ai.h"

/* Private includes ----------------------------------------------------------*/
#include <stdio.h>

/* Private variables ---------------------------------------------------------*/
UART_HandleTypeDef huart2;  // Terminale di uscita
UART_HandleTypeDef huart3;

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_USART3_UART_Init(void);
static void MX_USART2_UART_Init(void);

int main(void)
{
  /* MCU Configuration */
  HAL_Init();
  SystemClock_Config();
  MX_GPIO_Init();
  MX_USART3_UART_Init();
  MX_USART2_UART_Init();  // Inizializza huart2
  MX_X_CUBE_AI_Init();   // Inizializza il modello AI

  char buf[50];
  int buf_len = sprintf(buf, "\r\n\r\nSTM32 X-Cube-AI test\r\n");
  HAL_UART_Transmit(&huart2, (uint8_t *)buf, buf_len, 100);  // Usa huart2

  while (1)
  {
    MX_X_CUBE_AI_Process();  // Processa l'inferenza AI
  }
}

/* Configurazione del clock, UART e GPIO qui come nel tuo codice originale */

void Error_Handler(void)
{
  __disable_irq();
  while (1) {}
}

#ifdef  USE_FULL_ASSERT
void assert_failed(uint8_t *file, uint32_t line)
{
  // Implementazione per la gestione degli errori
}
#endif /* USE_FULL_ASSERT */
