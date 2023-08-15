/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * File Name          : freertos.c
  * Description        : Code for freertos applications
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2023 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "FreeRTOS.h"
#include "task.h"
#include "main.h"
#include "cmsis_os.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "stdio.h"
#include "LCD_interface.h"
#include "queue.h"

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

typedef enum
{
	eSender1,
	eSender2
} DataSource_t;

typedef struct
{
	uint16_t ucValue;
	DataSource_t eDataSource;
} Data_t;

static Data_t xStructsToSend[2] =
{
 { 1, eSender1 }, /* Used by Sender1. */
 { 1, eSender2 } /* Used by Sender2. */
};

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN Variables */

uint32_t counter=0;
QueueHandle_t  xQueueHandle1;
QueueHandle_t  xQueueHandle2;

/* USER CODE END Variables */
/* Definitions for receiver */
osThreadId_t receiverHandle;
const osThreadAttr_t receiver_attributes = {
  .name = "receiver",
  .stack_size = 128 * 4,
  .priority = (osPriority_t) osPriorityLow,
};
/* Definitions for sender1 */
osThreadId_t sender1Handle;
const osThreadAttr_t sender1_attributes = {
  .name = "sender1",
  .stack_size = 128 * 4,
  .priority = (osPriority_t) osPriorityNormal,
};
/* Definitions for sender2 */
osThreadId_t sender2Handle;
const osThreadAttr_t sender2_attributes = {
  .name = "sender2",
  .stack_size = 128 * 4,
  .priority = (osPriority_t) osPriorityNormal,
};
/* Definitions for myBinarySem01 */
osSemaphoreId_t myBinarySem01Handle;
const osSemaphoreAttr_t myBinarySem01_attributes = {
  .name = "myBinarySem01"
};

/* Private function prototypes -----------------------------------------------*/

/* USER CODE BEGIN FunctionPrototypes */


/* USER CODE END FunctionPrototypes */

void vReceiverTask(void *argument);
void vSenderTask(void *argument);

void MX_FREERTOS_Init(void); /* (MISRA C 2004 rule 8.1) */

/**
  * @brief  FreeRTOS initialization
  * @param  None
  * @retval None
  */
void MX_FREERTOS_Init(void) {
  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* USER CODE BEGIN RTOS_MUTEX */
  /* add mutexes, ... */
  /* USER CODE END RTOS_MUTEX */

  /* Create the semaphores(s) */
	
  /* creation of myBinarySem01 */
  myBinarySem01Handle = osSemaphoreNew(1, 1, &myBinarySem01_attributes);

  /* USER CODE BEGIN RTOS_SEMAPHORES */
  /* add semaphores, ... */
  /* USER CODE END RTOS_SEMAPHORES */

  /* USER CODE BEGIN RTOS_TIMERS */
  /* start timers, add new ones, ... */
  /* USER CODE END RTOS_TIMERS */

  /* USER CODE BEGIN RTOS_QUEUES */
	
  /* add queues, ... */
  xQueueHandle1 = xQueueCreate(3, sizeof(Data_t));

  /* USER CODE END RTOS_QUEUES */

  /* Create the thread(s) */
	
  /* creation of receiver */
  receiverHandle = osThreadNew(vReceiverTask, NULL, &receiver_attributes);

  /* creation of sender1 */
  sender1Handle = osThreadNew(vSenderTask, &(xStructsToSend[0]), &sender1_attributes);

  /* creation of sender2 */
  sender2Handle = osThreadNew(vSenderTask, &(xStructsToSend[1]), &sender2_attributes);

  /* USER CODE BEGIN RTOS_THREADS */

  /* add threads, ... */
  /* USER CODE END RTOS_THREADS */

  /* USER CODE BEGIN RTOS_EVENTS */
  /* add events, ... */
  /* USER CODE END RTOS_EVENTS */

}

/* USER CODE BEGIN Header_vReceiverTask */
/**
  * @brief  Function implementing the receiver thread.
  * @param  argument: Not used
  * @retval None
  */
/* USER CODE END Header_vReceiverTask */
void vReceiverTask(void *argument)
{
	/* USER CODE BEGIN vRecieverTask */
	BaseType_t RetVal = pdTRUE;
	Data_t xReceivedStructure;
	TickType_t lastWakeTime = xTaskGetTickCount();
	/* Infinite loop */

	for(;;)
	{
		RetVal = xQueueReceive(xQueueHandle1, &xReceivedStructure, HAL_MAX_DELAY);
		if(pdTRUE == RetVal)
		{
		  osSemaphoreAcquire(myBinarySem01Handle, HAL_MAX_DELAY);

		  if (xReceivedStructure.eDataSource == eSender1)
		  {
			  lcd_4bit_send_string_pos(&lcd_1, 1, 1, "FromSender1=>");
			  LCD_WriteNumber_Position(xReceivedStructure.ucValue, 1, 15);
		  }
		  else
		  {
			  lcd_4bit_send_string_pos(&lcd_1, 2, 1, "FromSender2=>");
			  LCD_WriteNumber_Position(xReceivedStructure.ucValue, 2, 15);
		  }

		  osSemaphoreRelease(myBinarySem01Handle);
		}
		else
		{
			  LCD_Clear(&lcd_1);
			  vTaskDelayUntil(&lastWakeTime, pdMS_TO_TICKS(100));
			  lcd_4bit_send_string_pos(&lcd_1, 1, 1, "QueueRecFailed");
			  vTaskDelayUntil(&lastWakeTime, pdMS_TO_TICKS(100));
		}
	}
}

/* USER CODE BEGIN Header_vSenderTask */
/**
* @brief Function implementing the sender1 thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_vSenderTask */
void vSenderTask(void *argument)
{
	BaseType_t RetVal = pdTRUE;
	Data_t *xSendedStructure = (Data_t *)argument;
	TickType_t lastWakeTime = xTaskGetTickCount();
	for ( ; ; )
	{
		RetVal = xQueueSendToBack(xQueueHandle1, argument, HAL_MAX_DELAY);
		if (RetVal != pdTRUE)
		{
			LCD_Clear(&lcd_1);
			vTaskDelayUntil(&lastWakeTime, pdMS_TO_TICKS(100));
			lcd_4bit_send_string_pos(&lcd_1, 1, 1, "SendingFailed");
		}
		vTaskDelayUntil(&lastWakeTime, pdMS_TO_TICKS(1000));
		xSendedStructure->ucValue++;
	}
}

/* Private application code --------------------------------------------------*/
/* USER CODE BEGIN Application */
/* Task to be created. */


/* USER CODE END Application */

