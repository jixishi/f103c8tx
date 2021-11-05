/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2021 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under BSD 3-Clause license,
  * the "License"; You may not use this file except in compliance with the
  * License. You may obtain a copy of the License at:
  *                        opensource.org/licenses/BSD-3-Clause
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "adc.h"
#include "dma.h"
#include "i2c.h"
#include "tim.h"
#include "usart.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <stdio.h>
#include <stdbool.h>
#include "valuepack.h"
#include "retarget.h"
#include "oled.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define ADC_MAX_NUM 3*100
#define ADC_CHANNEL_CNT 3
#define FREQ_T 0.048
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */
uint16_t ADC_Values[ADC_MAX_NUM]={0};
uint16_t adc_value_flg=0;
double vpp=0,v_max=0,v_min=5,ff=0,tim=0;
double ft=0.0,f_tp[5]={0};
bool fb=0,zb=0,sb=0,mv=0;
uint8_t *s_vpp="",*s_max="",*s_min="",*ad="ADC:    v",*ch_mv="%.1f",*ch_v="%.3f";
uint8_t *ch="";
int f=0;
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */

void start_adc(){
    HAL_TIM_Base_Start_IT(&htim2);
    HAL_TIM_Base_Start_IT(&htim3);
    HAL_TIM_PWM_Start(&htim2,TIM_CHANNEL_2);
    HAL_ADC_Start_DMA(&hadc1,(uint32_t *)ADC_Values,ADC_MAX_NUM);
}
/**
void get_ADC_Channel_Val(void)
{
    uint8_t i = 0;
    for(i=0;i<ADC_CHANNEL_CNT;i++)
    {
        ADC_eva[i] = 0;
    }

    for(i=0;i<ADC_CHANNEL_CNT;i++)
    {
        ADC_eva[0] +=  ADC_Values[i*3+0];
        ADC_eva[1] +=  ADC_Values[i*3+1];
        ADC_eva[2] +=  ADC_Values[i*3+2];
    }

    for(i=0;i<ADC_CHANNEL_CNT;i++)
    {
        ADC_eva[i] /= 5;
    }
}
*/
/**
uint16_t get_adc() {
    HAL_ADC_Start(&hadc1);
    HAL_ADC_PollForConversion(&hadc1, 10);

    if (HAL_IS_BIT_SET(HAL_ADC_GetState(&hadc1), HAL_ADC_STATE_REG_EOC)) {
        return HAL_ADC_GetValue(&hadc1);
    }
    return 0;
}*/
//ADC转换完成自动调用函数
void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef* hadc){

    //获取值并存储
    ADC_Values[adc_value_flg++]=HAL_ADC_GetValue(hadc);
    //HAL_GPIO_TogglePin(GPIOC, GPIO_PIN_13);
    if(adc_value_flg==ADC_MAX_NUM)
    {
        adc_value_flg=0;//清零下标
    }


}
void get_ff(void)
{
    double num=0;
    for(int i=0;i<5;i++){
        num+=f_tp[i];
    }
    ff=num/5;
}
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
    if(htim==(&htim2))
    {
        HAL_ADC_Start_DMA(&hadc1,(uint32_t *)ADC_Values,ADC_MAX_NUM);
        ft=((float )ADC_Values[0]/4096*3.3);

    }
    if(htim==(&htim3))
    {
        int tim1=0,tim2=0;
        HAL_GPIO_TogglePin(GPIOC, GPIO_PIN_13);
        float max=0,min=0,n;
        for(int i=0;i<100;i++)
        {
            n=((float )ADC_Values[i*3]/4096*3.3);
            if(ADC_Values[i*3-3]<ADC_Values[i*3]&&ADC_Values[i*3]>ADC_Values[i*3+3]&&tim1==0){tim1=i;}
            else if(ADC_Values[i*3-3]<ADC_Values[i*3]&&ADC_Values[i*3]>ADC_Values[i*3+3]&&tim2==0){tim2=i;}
            else if(tim2>tim1){
                tim=tim2-tim1;
                switch (f) {
                    case 0:
                        f_tp[f]=1/(0.000001*tim)*FREQ_T;
                        f++;
                        break;
                    case 1:
                        f_tp[f]=1/(0.000001*tim)*FREQ_T;
                        f++;
                        break;
                    case 2:
                        f_tp[f]=1/(0.000001*tim)*FREQ_T;
                        f++;
                        break;
                    case 3:
                        f_tp[f]=1/(0.000001*tim)*FREQ_T;
                        f++;
                        break;
                    case 4:
                        f_tp[f]=1/(0.000001*tim)*FREQ_T;
                        f=0;
                        break;
                }

            }
            if(n>max){max=n;min=-max;}else{vpp=max-min;}

        }
        if(vpp<=1.0){
            vpp=1000*vpp;
            s_vpp="Vpp:     mv";
            v_max=1000*max;
            s_max="Vmax:     mv";
            v_min=1000*min;
            s_min="Vmin:      mv";
            mv=1;
        } else if(vpp>1.0)
        {
            v_max=max;v_min=min;
            s_vpp="Vpp:     v ";
            s_max="Vmax:      v ";
            s_min="Vmin:       v  ";
            mv=0;
        }
        if(f==0){get_ff();}
    }

    //v_temp1=((float )ADC_Values[0]/4096*3.3);
    //v_temp2=((float )ADC_Values[1]/4096*3.3);
    //v_temp3=((float )ADC_Values[2]/4096*3.3);
    //get_ADC_Channel_Val();
}

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */
  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_I2C1_Init();
  MX_USART1_UART_Init();
  MX_DMA_Init();
  MX_ADC1_Init();
  MX_TIM2_Init();
  MX_TIM3_Init();
  /* USER CODE BEGIN 2 */
  OLED_Init();
  OLED_ON();
  OLED_CLS();
  HAL_ADCEx_Calibration_Start(&hadc1);
  RetargetInit(&huart1);
  OLED_ShowChinese(0,0,0,16);
  OLED_ShowChinese(16,0,1,16);
  OLED_ShowChinese(32,0,2,16);
  OLED_ShowChinese(48,0,3,16);
  OLED_Refresh();
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  start_adc();
    while (1)
    {
        if (mv){
            ch=ch_mv;
        } else{ch=ch_v;}
        printf("采样值：%.4f\t峰峰值：%.3f\n",ft,vpp);
        OLED_ShowString(0,16,ad,12);
        OLED_ShowFloat(24,16,ft,12,"%.2f");
        OLED_ShowString(0,28,s_vpp,12);
        OLED_ShowFloat(24,28,vpp,12,ch);
        OLED_ShowString(0,40,s_max,12);
        OLED_ShowFloat(32,40,v_max,12,ch);
        OLED_ShowString(0,52,s_min,12);
        OLED_ShowFloat(32,52,v_min,12,ch);
        OLED_ShowString(72,16,"Freq:",12);
        OLED_ShowFloat(72,32,ff,12,"%.2f");
        OLED_Refresh();
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
        unsigned char Tx_pack[150];
        startValuePack(Tx_pack);
        putBool(fb);
        putBool(zb);
        putBool(sb);
        putFloat(vpp);
        putFloat(ft);
        putFloat(v_max);
        putFloat(v_min);
        putFloat(ff);
        sendBuffer(Tx_pack,(endValuePack()));
        HAL_Delay(5);
        //v_min=v_max=vpp=0;
    }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};
  RCC_PeriphCLKInitTypeDef PeriphClkInit = {0};

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.HSEPredivValue = RCC_HSE_PREDIV_DIV1;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL9;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }
  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    Error_Handler();
  }
  PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_ADC;
  PeriphClkInit.AdcClockSelection = RCC_ADCPCLK2_DIV6;
  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */
void sendBuffer(unsigned char *p,unsigned short length)
{
    for(int i=0;i<length;i++)
    {
        HAL_UART_Transmit(&huart1, p++,1,0xFFFF);
        while(__HAL_UART_GET_FLAG(&huart1, UART_FLAG_RXNE))
        {}
    }
    printf("\n");
}
/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
    /* User can add his own implementation to report the HAL error return state */
    __disable_irq();
    while (1)
    {
    }
  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
