# M031BSP_SPI_Control
 M031BSP_SPI_Control

update @ 2021/05/03

1. initial SPI (800K) , PA0 : SPI0_MOSI , PA1 : SPI0_MISO , PA2 : SPI0_CLK , PA3 : SPI0_SS

2. base on M031 EVM , with define and measure LA waveform as below 

- MANUAL_CONTROL_CS

- USE_WIDTH_8_TRANSFER

- USE_WIDTH_32_TRANSFER

3. below is wavefom capture ( 0 : CS , 1 : CLK , 2 : MISO , 3 : MOSI)

with enable define : USE_WIDTH_8_TRANSFER

with no enable define : MANUAL_CONTROL_CS , 

![image](https://github.com/released/M031BSP_SPI_Control/blob/main/u8_autoSS_wait_busy_4_bytes.jpg)

with enable define : MANUAL_CONTROL_CS , 

![image](https://github.com/released/M031BSP_SPI_Control/blob/main/u8_manualSS_wait_busy_4_bytes.jpg)

with enable define : USE_WIDTH_32_TRANSFER

with no enable define : MANUAL_CONTROL_CS , 

![image](https://github.com/released/M031BSP_SPI_Control/blob/main/u32_autoSS_wait_busy_4_bytes.jpg)

with enable define : MANUAL_CONTROL_CS , 

![image](https://github.com/released/M031BSP_SPI_Control/blob/main/u32_manualSS_wait_busy_4_bytes.jpg)


