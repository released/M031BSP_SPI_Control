/*_____ I N C L U D E S ____________________________________________________*/
#include <stdio.h>
#include <string.h>
#include "NuMicro.h"

#include	"project_config.h"


/*_____ D E C L A R A T I O N S ____________________________________________*/
#define SPI_TARGET_FREQ							(800000ul)	//(48000000ul)
#define MASTER_DATA_NUM						(4)
/*_____ D E F I N I T I O N S ______________________________________________*/
volatile uint32_t BitFlag = 0;
volatile uint32_t counter_tick = 0;


// SPI
uint8_t MasterToSlaveTestPattern[MASTER_DATA_NUM]={0};
uint32_t u32MasterToSlaveTestPattern[MASTER_DATA_NUM]={0};
uint8_t MasterRxBuffer[MASTER_DATA_NUM]={0};
uint16_t spiTxDataCount = 0;
//uint16_t spiTxDataTargetCount = 0;
uint16_t spiRxDataCount = 0;
//uint16_t spiRxDataTargetCount = 0;

/*_____ M A C R O S ________________________________________________________*/

#define MANUAL_CONTROL_CS

//#define USE_WIDTH_8_TRANSFER
#define USE_WIDTH_32_TRANSFER

/*_____ F U N C T I O N S __________________________________________________*/

void tick_counter(void)
{
	counter_tick++;
}

uint32_t get_tick(void)
{
	return (counter_tick);
}

void set_tick(uint32_t t)
{
	counter_tick = t;
}

void compare_buffer(uint8_t *src, uint8_t *des, int nBytes)
{
    uint16_t i = 0;	
	
    for (i = 0; i < nBytes; i++)
    {
        if (src[i] != des[i])
        {
            printf("error idx : %4d : 0x%2X , 0x%2X\r\n", i , src[i],des[i]);
			set_flag(flag_error , ENABLE);
        }
    }

	if (!is_flag_set(flag_error))
	{
    	printf("%s finish \r\n" , __FUNCTION__);	
		set_flag(flag_error , DISABLE);
	}

}

void reset_buffer(void *dest, unsigned int val, unsigned int size)
{
    uint8_t *pu8Dest;
//    unsigned int i;
    
    pu8Dest = (uint8_t *)dest;

	#if 1
	while (size-- > 0)
		*pu8Dest++ = val;
	#else
	memset(pu8Dest, val, size * (sizeof(pu8Dest[0]) ));
	#endif
	
}

void copy_buffer(void *dest, void *src, unsigned int size)
{
    uint8_t *pu8Src, *pu8Dest;
    unsigned int i;
    
    pu8Dest = (uint8_t *)dest;
    pu8Src  = (uint8_t *)src;


	#if 0
	  while (size--)
	    *pu8Dest++ = *pu8Src++;
	#else
    for (i = 0; i < size; i++)
        pu8Dest[i] = pu8Src[i];
	#endif
}

void dump_buffer(uint8_t *pucBuff, int nBytes)
{
    uint16_t i = 0;
    
    printf("dump_buffer : %2d\r\n" , nBytes);    
    for (i = 0 ; i < nBytes ; i++)
    {
        printf("0x%2X," , pucBuff[i]);
        if ((i+1)%8 ==0)
        {
            printf("\r\n");
        }            
    }
    printf("\r\n\r\n");
}

void  dump_buffer_hex(uint8_t *pucBuff, int nBytes)
{
    int     nIdx, i;

    nIdx = 0;
    while (nBytes > 0)
    {
        printf("0x%04X  ", nIdx);
        for (i = 0; i < 16; i++)
            printf("%02X ", pucBuff[nIdx + i]);
        printf("  ");
        for (i = 0; i < 16; i++)
        {
            if ((pucBuff[nIdx + i] >= 0x20) && (pucBuff[nIdx + i] < 127))
                printf("%c", pucBuff[nIdx + i]);
            else
                printf(".");
            nBytes--;
        }
        nIdx += 16;
        printf("\n");
    }
    printf("\n");
}

void delay(uint16_t dly)
{
/*
	delay(100) : 14.84 us
	delay(200) : 29.37 us
	delay(300) : 43.97 us
	delay(400) : 58.5 us	
	delay(500) : 73.13 us	
	
	delay(1500) : 0.218 ms (218 us)
	delay(2000) : 0.291 ms (291 us)	
*/

	while( dly--);
}


void delay_ms(uint16_t ms)
{
	TIMER_Delay(TIMER0, 1000*ms);
}


void SPI0_IRQHandler(void)
{
    /* Check RX EMPTY flag */
    while(SPI_GET_RX_FIFO_EMPTY_FLAG(SPI0) == 0)
    {
        /* Read RX FIFO */
        MasterRxBuffer[spiRxDataCount++] = SPI_READ_RX(SPI0);
    }
//    /* Check TX FULL flag and TX data count */
//    while((SPI_GET_TX_FIFO_FULL_FLAG(SPI0) == 0) && (spiTxDataCount < MASTER_DATA_NUM))
//    {
//        /* Write to TX FIFO */
//		#if 0
//		printf("SPI : 0x%2X\r\n" , MasterToSlaveTestPattern[spiTxDataCount]);
//		#endif
//        SPI_WRITE_TX(SPI0, MasterToSlaveTestPattern[spiTxDataCount++]);
//    }
	
//    if(spiTxDataCount >= MASTER_DATA_NUM)
//    {
//        SPI_DisableInt(SPI0, SPI_FIFO_TXTH_INT_MASK); /* Disable TX FIFO threshold interrupt */
//		
//		set_flag(flag_SPI_TX_Finish , ENABLE);
//		spiTxDataCount = 0;
//    }

    /* Check the RX FIFO time-out interrupt flag */
    if(SPI_GetIntFlag(SPI0, SPI_FIFO_RXTO_INT_MASK))
    {
        /* If RX FIFO is not empty, read RX FIFO. */
        while(SPI_GET_RX_FIFO_EMPTY_FLAG(SPI0) == 0)
            MasterRxBuffer[spiRxDataCount++] = SPI_READ_RX(SPI0);
    }

	if (spiRxDataCount >= MASTER_DATA_NUM)
	{
    	SPI_DisableInt(SPI0, SPI_FIFO_RXTO_INT_MASK);
	
		set_flag(flag_SPI_RX_Finish , ENABLE);
		spiRxDataCount = 0;
	}
	
}

void SPI_Master_u32Tx(uint32_t* u32bffer , uint16_t len)
{
	uint16_t i = 0;

	#if defined (MANUAL_CONTROL_CS)
	SPI_SET_SS_LOW(SPI0);
	#endif

    while (i < len)
    {
        SPI_WRITE_TX(SPI0, u32bffer[i++]);
		while(SPI_IS_BUSY(SPI0));
    }

	#if defined (MANUAL_CONTROL_CS)
    SPI_SET_SS_HIGH(SPI0);
	#endif
	
}

void SPI_Master_Tx(uint8_t* u8bffer , uint16_t len)
{
	uint16_t i = 0;

	#if defined (MANUAL_CONTROL_CS)
	SPI_SET_SS_LOW(SPI0);
	#endif

    while (i < len)
    {
        SPI_WRITE_TX(SPI0, u8bffer[i++]);
		while(SPI_IS_BUSY(SPI0));
    }

	#if defined (MANUAL_CONTROL_CS)
    SPI_SET_SS_HIGH(SPI0);
	#endif
	
}


void SPI_Master_Loop_Process(void)
{
//    uint32_t u32DataCount = 0;
    uint16_t i = 0;

	if (is_flag_set(flag_SPI_RX_Finish))	
	{		
		set_flag(flag_SPI_RX_Finish , DISABLE);	
		
		#if 1	//debug
		dump_buffer(MasterRxBuffer,MASTER_DATA_NUM);	
		#endif	
		reset_buffer(MasterRxBuffer , 0xFF , MASTER_DATA_NUM);	
	}	

	if (is_flag_set(flag_SPI_Transmit))
	{	
		set_flag(flag_SPI_Transmit , DISABLE);
			
		//CLEAR data
		reset_buffer(MasterToSlaveTestPattern , 0xFF , MASTER_DATA_NUM);
		reset_buffer(u32MasterToSlaveTestPattern , 0xFFFFFFFF , MASTER_DATA_NUM);		
//		reset_buffer(MasterRxBuffer , 0xFF , MASTER_DATA_NUM);
		
//		MasterToSlaveTestPattern[0] = 0x80;
//		MasterToSlaveTestPattern[1] = 0x71;
//		MasterToSlaveTestPattern[2] = 0x62;
//		MasterToSlaveTestPattern[3] = 0x53;

//		while (is_flag_set(flag_SPI_TX_Finish) == TRUE);
//		set_flag(flag_SPI_TX_Finish , DISABLE);

//		CLK_SysTickDelay(10000);

//		if (is_flag_set(flag_SPI_TX_Finish) == TRUE)
//		{
//			set_flag(flag_SPI_TX_Finish , DISABLE);
//	    	SPI_EnableInt(SPI0, SPI_FIFO_TXTH_INT_MASK | SPI_FIFO_RXTO_INT_MASK);
//			spiTxDataCount = 0;	
//			spiRxDataCount = 0;			
//		}		

		if (is_flag_set(flag_SPI_TX_DataChange))
		{
			set_flag(flag_SPI_TX_DataChange , DISABLE);

			#if defined (USE_WIDTH_8_TRANSFER)
			for (i = 0; i < MASTER_DATA_NUM; i++)
			{
				MasterToSlaveTestPattern[i] = 0x00 + i;
			}

			SPI_Master_Tx(MasterToSlaveTestPattern,MASTER_DATA_NUM);		
			
			#elif defined (USE_WIDTH_32_TRANSFER)
			for (i = 0; i < MASTER_DATA_NUM; i++)
			{
				u32MasterToSlaveTestPattern[i] = 0x12345670 | i;				
			}

			SPI_Master_u32Tx(u32MasterToSlaveTestPattern,MASTER_DATA_NUM);	
			#endif
		}
		else
		{
			set_flag(flag_SPI_TX_DataChange , ENABLE);

			#if defined (USE_WIDTH_8_TRANSFER)
			for (i = 0; i < MASTER_DATA_NUM; i++)
			{
				MasterToSlaveTestPattern[i] = 0x80 + i;		
			}

			SPI_Master_Tx(MasterToSlaveTestPattern,MASTER_DATA_NUM);	
			
			#elif defined (USE_WIDTH_32_TRANSFER)
			for (i = 0; i < MASTER_DATA_NUM; i++)
			{
				u32MasterToSlaveTestPattern[i] = 0x87654320 | i;					
			}

			SPI_Master_u32Tx(u32MasterToSlaveTestPattern,MASTER_DATA_NUM);	
			#endif
		
		}
	}	
}

void SPI_Master_PreInit(void)
{
    uint16_t i = 0;

	//prepare data
    for (i = 0; i < MASTER_DATA_NUM; i++)
    {
        MasterToSlaveTestPattern[i] = i;
        u32MasterToSlaveTestPattern[i] = i;
		
        MasterRxBuffer[i] = 0xFF;
    }

    /* Set TX FIFO threshold, enable TX FIFO threshold interrupt and RX FIFO time-out interrupt */
    SPI_SetFIFO(SPI0, 3, 3);
//    SPI_EnableInt(SPI0, SPI_FIFO_TXTH_INT_MASK | SPI_FIFO_RXTO_INT_MASK);
    SPI_EnableInt(SPI0, SPI_FIFO_RXTO_INT_MASK);

    spiTxDataCount = 0;
    spiRxDataCount = 0;
//	spiTxDataTargetCount = 64;
//	spiRxDataTargetCount = 64;
    NVIC_EnableIRQ(SPI0_IRQn);
	
}

//PA0 : SPI0_MOSI , PA1 : SPI0_MISO , PA2 : SPI0_CLK , PA3 : SPI0_SS
void SPI_Master_Init(void)
{
	#if defined (USE_WIDTH_8_TRANSFER)
    SPI_Open(SPI0, SPI_MASTER, SPI_MODE_0, 8, SPI_TARGET_FREQ);
	#elif defined (USE_WIDTH_32_TRANSFER)
    SPI_Open(SPI0, SPI_MASTER, SPI_MODE_0, 32, SPI_TARGET_FREQ);
	#endif

	#if defined (MANUAL_CONTROL_CS)
    SPI_DisableAutoSS(SPI0);
	#else
    /* Enable the automatic hardware slave select function. Select the SS pin and configure as low-active. */
    SPI_EnableAutoSS(SPI0, SPI_SS, SPI_SS_ACTIVE_LOW);
	#endif

//    SYS->GPA_MFPL &= ~( SYS_GPA_MFPL_PA3MFP_Msk);	
//    SYS->GPA_MFPL |=  SYS_GPA_MFPL_PA3MFP_GPIO;	
//	GPIO_SetMode(PA,BIT3,GPIO_MODE_OUTPUT);	

//	SPI_SET_SUSPEND_CYCLE(SPI0,5);
}



void GPIO_Init (void)
{
    GPIO_SetMode(PB, BIT14, GPIO_MODE_OUTPUT);

    GPIO_SetMode(PB, BIT15, GPIO_MODE_OUTPUT);	
}


void TMR1_IRQHandler(void)
{
//	static uint32_t LOG = 0;

	
    if(TIMER_GetIntFlag(TIMER1) == 1)
    {
        TIMER_ClearIntFlag(TIMER1);
		tick_counter();

		if ((get_tick() % 1000) == 0)
		{
//        	printf("%s : %4d\r\n",__FUNCTION__,LOG++);
			PB14 ^= 1;
		}

		if ((get_tick() % 2) == 0)
		{

			set_flag(flag_SPI_Transmit , ENABLE);
		}
    }
}


void TIMER1_Init(void)
{
    TIMER_Open(TIMER1, TIMER_PERIODIC_MODE, 1000);
    TIMER_EnableInt(TIMER1);
    NVIC_EnableIRQ(TMR1_IRQn);	
    TIMER_Start(TIMER1);
}

void UARTx_Process(void)
{
	uint8_t res = 0;
	res = UART_READ(UART0);

	if (res == 'x' || res == 'X')
	{
		NVIC_SystemReset();
	}

	if (res > 0x7F)
	{
		printf("invalid command\r\n");
	}
	else
	{
		switch(res)
		{
			case '1':
				break;

			case 'X':
			case 'x':
			case 'Z':
			case 'z':
				NVIC_SystemReset();		
				break;
		}
	}
}

void UART02_IRQHandler(void)
{

    if(UART_GET_INT_FLAG(UART0, UART_INTSTS_RDAINT_Msk | UART_INTSTS_RXTOINT_Msk))     /* UART receive data available flag */
    {
        while(UART_GET_RX_EMPTY(UART0) == 0)
        {
            UARTx_Process();
        }
    }

    if(UART0->FIFOSTS & (UART_FIFOSTS_BIF_Msk | UART_FIFOSTS_FEF_Msk | UART_FIFOSTS_PEF_Msk | UART_FIFOSTS_RXOVIF_Msk))
    {
        UART_ClearIntFlag(UART0, (UART_INTSTS_RLSINT_Msk| UART_INTSTS_BUFERRINT_Msk));
    }	
}

void UART0_Init(void)
{
    SYS_ResetModule(UART0_RST);

    /* Configure UART0 and set UART0 baud rate */
    UART_Open(UART0, 115200);
    UART_EnableInt(UART0, UART_INTEN_RDAIEN_Msk | UART_INTEN_RXTOIEN_Msk);
    NVIC_EnableIRQ(UART02_IRQn);
	
	#if (_debug_log_UART_ == 1)	//debug
	printf("\r\nCLK_GetCPUFreq : %8d\r\n",CLK_GetCPUFreq());
	printf("CLK_GetHXTFreq : %8d\r\n",CLK_GetHXTFreq());
	printf("CLK_GetLXTFreq : %8d\r\n",CLK_GetLXTFreq());	
	printf("CLK_GetPCLK0Freq : %8d\r\n",CLK_GetPCLK0Freq());
	printf("CLK_GetPCLK1Freq : %8d\r\n",CLK_GetPCLK1Freq());	
	#endif	

}

void SYS_Init(void)
{
    /* Unlock protected registers */
    SYS_UnlockReg();

    /* Enable HIRC clock (Internal RC 48MHz) */
    CLK_EnableXtalRC(CLK_PWRCTL_HIRCEN_Msk|CLK_PWRCTL_HXTEN_Msk);

    /* Wait for HIRC clock ready */
    CLK_WaitClockReady(CLK_STATUS_HIRCSTB_Msk|CLK_STATUS_HXTSTB_Msk);

    /* Enable HIRC clock (Internal RC 48MHz) */
    CLK_EnableXtalRC(CLK_PWRCTL_LIRCEN_Msk|CLK_PWRCTL_LXTEN_Msk);

    /* Wait for HIRC clock ready */
    CLK_WaitClockReady(CLK_STATUS_LIRCSTB_Msk|CLK_STATUS_LXTSTB_Msk);	

    /* Select HCLK clock source as HIRC and HCLK source divider as 1 */
    CLK_SetHCLK(CLK_CLKSEL0_HCLKSEL_HIRC, CLK_CLKDIV0_HCLK(1));

    CLK_EnableModuleClock(UART0_MODULE);
    CLK_SetModuleClock(UART0_MODULE, CLK_CLKSEL1_UART0SEL_HIRC, CLK_CLKDIV0_UART0(1));

    CLK_EnableModuleClock(TMR1_MODULE);
  	CLK_SetModuleClock(TMR1_MODULE, CLK_CLKSEL1_TMR1SEL_HIRC, 0);

    CLK_SetModuleClock(SPI0_MODULE, CLK_CLKSEL2_SPI0SEL_HIRC, MODULE_NoMsk);	
    CLK_EnableModuleClock(SPI0_MODULE);

    /* PA.3 is SPI0_SS,   PA.2 is SPI0_CLK,
       PA.1 is SPI0_MISO, PA.0 is SPI0_MOSI*/
   	SYS->GPA_MFPL &= ~(SYS_GPA_MFPL_PA0MFP_Msk | SYS_GPA_MFPL_PA1MFP_Msk| SYS_GPA_MFPL_PA2MFP_Msk| SYS_GPA_MFPL_PA3MFP_Msk);
    SYS->GPA_MFPL |= SYS_GPA_MFPL_PA0MFP_SPI0_MOSI | SYS_GPA_MFPL_PA1MFP_SPI0_MISO | SYS_GPA_MFPL_PA2MFP_SPI0_CLK | SYS_GPA_MFPL_PA3MFP_SPI0_SS;
	
    /* Set PB multi-function pins for UART0 RXD=PB.12 and TXD=PB.13 */
    SYS->GPB_MFPH = (SYS->GPB_MFPH & ~(SYS_GPB_MFPH_PB12MFP_Msk | SYS_GPB_MFPH_PB13MFP_Msk)) |
                    (SYS_GPB_MFPH_PB12MFP_UART0_RXD | SYS_GPB_MFPH_PB13MFP_UART0_TXD);

   /* Update System Core Clock */
    SystemCoreClockUpdate();

    /* Lock protected registers */
    SYS_LockReg();
}

/*
 * This is a template project for M031 series MCU. Users could based on this project to create their
 * own application without worry about the IAR/Keil project settings.
 *
 * This template application uses external crystal as HCLK source and configures UART0 to print out
 * "Hello World", users may need to do extra system configuration based on their system design.
 */

int main()
{
    SYS_Init();

	UART0_Init();

	SPI_Master_Init();
	SPI_Master_PreInit();
	
	GPIO_Init();
	TIMER1_Init();


    /* Got no where to go, just loop forever */
    while(1)
    {
		SPI_Master_Loop_Process();

    }
}

/*** (C) COPYRIGHT 2017 Nuvoton Technology Corp. ***/
