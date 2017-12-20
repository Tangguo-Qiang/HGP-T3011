#include "system.h"

#define INITVAL	0x11223345
/*NTC 10K3950 */
//const uint16_t NTC_ad[]={
//3883,3870,3856,3842,3827,3811,3795,3777,3759,3740,3720,/*-30-(-20)*/
//	   3700,3679,3656,3633,3609,3584,3559,3532,3504,3476,/*-21-(-10)*/
//	   3446,3416,3385,3353,3320,3286,3251,3215,3179,3141,/*-9--0*/
//	   3103,3065,3025,2984,2943,2902,2860,2817,2773,2730,/*1--10 */
//	   2686,2641,2596,2551,2505,2460,2414,2368,2322,2276,/*11--20*/
//	   2230,2185,2139,2093,2048,2003,1958,1914,1870,1827,/*21--30*/
//	   1783,1741,1699,1658,1617,1575,1536,1497,1459,1421,/*31--40*/
//1384,1347,1312,1277,1242};/*41--45*/
	
/*NTC 10K3435 */
const uint16_t NTC_T2[]={
3773, 3757, 3740, 3723, 3705, 3687, 3668, 3648, 3627, 3606, 3584, /*-30-(-20)*/
			3562, 3538, 3515, 3490, 3465, 3439, 3412, 3385, 3357, 3328, /*-21-(-10)*/
	    3299, 3269, 3238, 3207, 3175, 3143, 3109, 3076, 3042, 3007, /*-9--0*/
	    2972, 2936, 2900, 2863, 2827, 2789, 2751, 2714, 2675, 2637, /*1--10 */
	    2598, 2559, 2519, 2480, 2441, 2401, 2362, 2322, 2283, 2243, /*11--20*/
	    2204, 2165, 2126, 2087, 2048, 2010, 1971, 1933, 1895, 1858, /*21--30*/
	    1821, 1784, 1748, 1712, 1676, 1641, 1607, 1572, 1539, 1505, 
	    1473, 1440, 1408, 1377, 1346, /*41--45*/	
};

/*神荣RH1 10K 3380K */
const uint16_t NTC_T1[]={
3764, 3748, 3731, 3713, 3695, 3676, 3657, 3637, 3616, 3595, 3572, /*-30-(-20)*/
	    3550, 3526, 3502, 3478, 3452, 3426, 3400, 3372, 3345, 3316, /*-21-(-10)*/
			3287, 3256, 3226, 3195, 3163, 3130, 3097, 3064, 3030, 2995, /*-9--0*/
	    2961, 2925, 2890, 2853, 2817, 2780, 2743, 2705, 2667, 2629, /*1--10 */
	    2591, 2552, 2514, 2475, 2436, 2397, 2358, 2319, 2280, 2241, /*11--20*/
	    2202, 2163, 2125, 2086, 2048, 2010, 1972, 1934, 1897, 1860, /*21--30*/
	    1823, 1791, 1750, 1714, 1636, 1644, 1610, 1576, 1542, 1509, /*31--40*/
	    1476, 1444, 1412, 1381, 1350,/*41--45*/	
};


static void ADC1_Convert(uint16_t* pvalue)
{
	uint16_t rh[10]= {0},temp2[10]={0},temp1[10]={0};
	uint16_t temp;
	uint16_t i;

  /* ADC1 regular channel10-13 configuration */ 
	ADC_ChannelConfig(ADC1, ADC_Channel_0,  ADC_SampleTime_239_5Cycles);			
	ADC_ChannelConfig(ADC1, ADC_Channel_1,  ADC_SampleTime_239_5Cycles);			
	ADC_ChannelConfig(ADC1, ADC_Channel_4,  ADC_SampleTime_239_5Cycles);			

	ADC_Cmd(ADC1,ENABLE);
	/* Wait the ADRDY flag */
	i=0;
  while(!ADC_GetFlagStatus(ADC1, ADC_FLAG_ADRDY))
	{
		if(++i==0)
		{
			ADC_Cmd(ADC1, DISABLE);
			return;
		}
	}
	
	for(i=0;i<10;i++)
	{
		/* ADC1 regular Software Start Conv */ 
		ADC_StartOfConversion(ADC1);

		TimeOut1ms =10;
    while ((ADC_GetFlagStatus(ADC1,ADC_FLAG_EOC) == RESET)&&TimeOut1ms){};			
		ADC_ClearFlag(ADC1, ADC_FLAG_EOC);
		rh[i] = ADC_GetConversionValue(ADC1);
		TimeOut1ms =10;
    while ((ADC_GetFlagStatus(ADC1,ADC_FLAG_EOC) == RESET)&&TimeOut1ms){};			
		ADC_ClearFlag(ADC1, ADC_FLAG_EOC);
		temp2[i] = ADC_GetConversionValue(ADC1);
		TimeOut1ms =10;
    while ((ADC_GetFlagStatus(ADC1,ADC_FLAG_EOC) == RESET)&&TimeOut1ms){};			
		ADC_ClearFlag(ADC1, ADC_FLAG_EOC);
		temp1[i] = ADC_GetConversionValue(ADC1);
			
	}
	
  ADC_Cmd(ADC1, DISABLE);
	for(i=1;i<10;i++)
	{
		if(temp1[0]<temp1[i])
		{
			temp = temp1[0];
			temp1[0]= temp1[i];
			temp1[i]=temp;
		}
	}
	for(i=1;i<9;i++)
	{
		if(temp1[9]<temp1[i])
		{
			temp = temp1[9];
			temp1[9]= temp1[i];
			temp1[i]=temp;
		}
	}
	temp =0;
	for(i=1;i<9;i++)
		temp += temp1[i];	
	temp /= 8;
	*pvalue++ = temp;
	
	for(i=1;i<10;i++)
	{
		if(temp2[0]<temp2[i])
		{
			temp = temp2[0];
			temp2[0]= temp2[i];
			temp2[i]=temp;
		}
	}
	for(i=1;i<9;i++)
	{
		if(temp2[9]<temp2[i])
		{
			temp = temp2[9];
			temp2[9]= temp2[i];
			temp2[i]=temp;
		}
	}
	temp =0;
	for(i=1;i<9;i++)
		temp += temp2[i];	
	temp /= 8;
	*pvalue++ = temp;
	
	for(i=1;i<10;i++)
	{
		if(rh[0]<rh[i])
		{
			temp = rh[0];
			rh[0]= rh[i];
			rh[i]=temp;
		}
	}
	for(i=1;i<9;i++)
	{
		if(rh[9]<rh[i])
		{
			temp = rh[9];
			rh[9]= rh[i];
			rh[i]=temp;
		}
	}
	temp =0;
	for(i=1;i<9;i++)
		temp += rh[i];	
	temp /= 8;
	*pvalue = temp;		 
}

static void ADC1_DataGet(int8_t* pvalue)
{
	uint16_t value[3];
	uint8_t i;
	uint32_t temp,sum;
	
	ADC1_Convert(value);
	temp = value[0];	
	for(i=0;i<76;i++)
	{
		if(temp>NTC_T1[i])
			break;
	}
	if(i==0)
	{
		*pvalue = -30;	//最低温度 -30C
	}
  else if(i!=76)
	{
		sum = NTC_T1[i]+NTC_T1[i-1];
		sum /= 2;
		if(temp<sum)
			*pvalue=i;
		else
			*pvalue =i-1;
		*pvalue -= 30;
	}
	else
		*pvalue = -50;
	
	pvalue++;
	temp = value[1];	
	for(i=0;i<76;i++)
	{
		if(temp>NTC_T2[i])
			break;
	}
	if(i==0)
	{
		*pvalue = -30;	//最低温度 -30C
	}
  else if(i!=76)
	{
		sum = NTC_T2[i]+NTC_T2[i-1];
		sum /= 2;
		if(temp<sum)
			*pvalue=i;
		else
			*pvalue =i-1;
		*pvalue -= 30;
	}
	else
		*pvalue = -50;

	pvalue++;
	temp = value[2];
	temp *= 1000;
//	temp *= 33;
	temp /= 0xfff; //amplify 1000 times
//	temp /= 33;  // V/0.033
	
	if(temp>1000)
		*pvalue = 99;
	else
		*pvalue = temp/10;
	
}

void Init_ADC1(void)
{
	GPIO_InitTypeDef GPIO_InitStruct;
	ADC_InitTypeDef ADC_InitStructure;
	
  /*Enable or disable the AHB peripheral clock */
  RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOA, ENABLE);

  /** ADC GPIO Configuration  
  PA0   ------> ADC_IN0
  PA1   ------> ADC_IN1
  PA4   ------> ADC_IN4
  */
  /*ADC TEMP1 TEMP2 Configure GPIO pin : PA */
  GPIO_InitStruct.GPIO_Pin = GPIO_Pin_0|GPIO_Pin_1|GPIO_Pin_4;
  GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AN;
  GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_NOPULL;
  GPIO_Init(GPIOA, &GPIO_InitStruct);

  /* Enable ADC1 clock */
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1, ENABLE);
  /* ADC1 configuration */
	ADC_InitStructure.ADC_Resolution = ADC_Resolution_12b;
  ADC_InitStructure.ADC_ContinuousConvMode = DISABLE;
  ADC_InitStructure.ADC_ExternalTrigConvEdge = ADC_ExternalTrigConvEdge_None;
  ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;
	ADC_InitStructure.ADC_ScanDirection = ADC_ScanDirection_Upward;
  ADC_Init(ADC1, &ADC_InitStructure);
	
	/* ADC Calibration */
  ADC_GetCalibrationFactor(ADC1);	

	System.Device.Adc1.ADC1_DataGet=ADC1_DataGet;
}
