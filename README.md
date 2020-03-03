# STM32H7
Basic initializations by register based C, implemented on STM32H743. 


## SPI

[goto implementation][1]

- Master clock frequency = 200 MHz by PLL1 with internal oscillator (SPI divider = /32)
- SPI mode 0 (CPOL & CPHA = 0)
- 8-bit mode
- TxFIFO treshold level is 1-data & data size = 8-bit
    - 1-byte in single dataframe transmission







[1]: SPI/main.c