#include "user_config.h"

Sin_table sin_table= 
{
 .Sin = { 2100,2127,2154,2181,2208,2235,2262,2289,2316,2343,2370,2397,2423,2450,2477,2503,2529,2556,2582,2608,2633,2659,
  2685,2710,2735,2761,2786,2810,2835,2859,2884,2908,2931,2955,2979,3002,3025,3048,3070,3092,3114,3136,3158,3179,
	3200,3221,3241,3261,3281,3301,3320,3339,3358,3376,3394,3412,3429,3446,3463,3479,3496,3511,3527,3542,3556,3571,
	3584,3598,3611,3624,3636,3649,3660,3671,3682,3693,3703,3713,3722,3731,3739,3747,3755,3762,3769,3776,3782,3787,
	3793,3797,3802,3806,3809,3812,3815,3817,3819,3820,3821,3822,3822,3822,3821,3820,3818,3816,3814,3811,3807,3804,
	3800,3795,3790,3785,3779,3773,3766,3759,3751,3743,3735,3726,3717,3708,3698,3688,3677,3666,3654,3643,3630,3618,
	3605,3591,3578,3563,3549,3534,3519,3503,3488,3471,3455,3438,3421,3403,3385,3367,3348,3330,3310,3291,3271,3251,
	3231,3210,3189,3168,3147,3125,3103,3081,3059,3036,3013,2990,2967,2943,2920,2896,2871,2847,2823,2798,2773,2748,
	2723,2697,2672,2646,2621,2595,2569,2542,2516,2490,2463,2437,2410,2383,2357,2330,2303,2276,2249,2222,2195,2168,
	2141,2114,2086,2059,2032,2005,1978,1951,1924,1897,1870,1843,1817,1790,1763,1737,1710,1684,1658,1631,1605,1579,
	1554,1528,1503,1477,1452,1427,1402,1377,1353,1329,1304,1280,1257,1233,1210,1187,1164,1141,1119,1097,1075,1053,
	1032,1011, 990, 969, 949, 929, 909, 890, 870, 852, 833, 815, 797, 779, 762, 745, 729, 712, 697, 681, 666, 651, 
	 637, 622, 609, 595, 582, 570, 557, 546, 534, 523, 512, 502, 492, 483, 474, 465, 457, 449, 441, 434, 427, 421,
 	 415, 410, 405, 400, 396, 393, 389, 386, 384, 382, 380, 379, 378, 378, 378, 379, 380, 381, 383, 385, 388, 391, 
	 394, 398, 403, 407, 413, 418, 424, 431, 438, 445, 453, 461, 469, 478, 487, 497, 507, 518, 529, 540, 551, 564, 
	 576, 589, 602, 616, 629, 644, 658, 673, 689, 704, 721, 737, 754, 771, 788, 806, 824, 842, 861, 880, 899, 919, 
	 939, 959, 979,1000,1021,1042,1064,1086,1108,1130,1152,1175,1198,1221,1245,1269,1292,1316,1341,1365,1390,1414,
	 1439,1465,1490,1515,1541,1567,1592,1618,1644,1671,1697,1723,1750,1777,1803,1830,1857,1884,1911,1938,1965,1992,
	 2019,2046,2073,2100,},
 .cnt = 0,
	
};
void Get_System_Clock(System_Cloock *pointer)
{
	RCC_ClocksTypeDef RCC_Clocks;
	RCC_GetClocksFreq(&RCC_Clocks);
	pointer->AHB_clk  = RCC_Clocks.HCLK_Frequency/1000000;
	pointer->APB1_clk = RCC_Clocks.PCLK1_Frequency/1000000;
	pointer->APB2_clk = RCC_Clocks.PCLK2_Frequency/1000000;
	pointer->sys_clk  = RCC_Clocks.SYSCLK_Frequency/1000000;
}
/*
系统时钟的关系为:
sys_clk = HSE/PLL_M*PLL_N/PLL_P

  * @brief  Configures the main PLL clock source, multiplication and division factors.
  * @note   This function must be used only when the main PLL is disabled.
  *  
  * @param  RCC_PLLSource: specifies the PLL entry clock source.
  *          This parameter can be one of the following values:
  *            @arg RCC_PLLSource_HSI: HSI oscillator clock selected as PLL clock entry
  *            @arg RCC_PLLSource_HSE: HSE oscillator clock selected as PLL clock entry
  * @note   This clock source (RCC_PLLSource) is common for the main PLL and PLLI2S.  
  *  
  * @param  PLLM: specifies the division factor for PLL VCO input clock
  *          This parameter must be a number between 0 and 63.
  * @note   You have to set the PLLM parameter correctly to ensure that the VCO input
  *         frequency ranges from 1 to 2 MHz. It is recommended to select a frequency
  *         of 2 MHz to limit PLL jitter.
  *  
  * @param  PLLN: specifies the multiplication factor for PLL VCO output clock
  *          This parameter must be a number between 192 and 432.
  * @note   You have to set the PLLN parameter correctly to ensure that the VCO
  *         output frequency is between 192 and 432 MHz.
  *   
  * @param  PLLP: specifies the division factor for main system clock (SYSCLK)
  *          This parameter must be a number in the range {2, 4, 6, or 8}.
  * @note   You have to set the PLLP parameter correctly to not exceed 168 MHz on
  *         the System clock frequency.
  *  
  * @param  PLLQ: specifies the division factor for OTG FS, SDIO and RNG clocks
  *          This parameter must be a number between 4 and 15.
  * @note   If the USB OTG FS is used in your application, you have to set the
  *         PLLQ parameter correctly to have 48 MHz clock for the USB. However,
  *         the SDIO and RNG need a frequency lower than or equal to 48 MHz to work
  *         correctly.
  *   
  * @retval None
*/
void System_Clock_Config(void)
{
	RCC_DeInit();
	 /* Set HSION bit */
  RCC->CR |= (uint32_t)0x00000001;

  /* Reset CFGR register */
  RCC->CFGR = 0x00000000;

  /* Reset HSEON, CSSON and PLLON bits */
  RCC->CR &= (uint32_t)0xFEF6FFFF;

  /* Reset PLLCFGR register */
  RCC->PLLCFGR = 0x24003010;

  /* Reset HSEBYP bit */
  RCC->CR &= (uint32_t)0xFFFBFFFF;

  /* Disable all interrupts */
  RCC->CIR = 0x00000000;
	RCC_HSEConfig(RCC_HSE_ON);//使能HSE 
	while(RCC_WaitForHSEStartUp() !=SUCCESS);//等待HSE起正
	//配置PLL相关参数
	RCC_PLLConfig(RCC_PLLSource_HSE,25,336,2,7);
	RCC_PLLCmd(ENABLE);
	RCC_SYSCLKConfig(RCC_SYSCLKSource_PLLCLK);
	RCC_HCLKConfig(RCC_SYSCLK_Div1);
	RCC_PCLK1Config(RCC_HCLK_Div4);
	RCC_PCLK2Config(RCC_HCLK_Div2);
}
