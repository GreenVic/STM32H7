/*

	Register based STM32H7 SPI configuration with PLL1 (c) by Roger Kupari

	Register based STM32H7 SPI configuration with PLL1 is licensed under a
	Creative Commons Attribution-NonCommercial-NoDerivatives 4.0 International License.

	You should have received a copy of the license along with this
	work. If not, see <http://creativecommons.org/licenses/by-nc-nd/4.0/>.
	
*/


#include "stm32h743xx.h"
#include "stdlib.h"



/*
		If defined TRANSMIT_TEST 
		0xFF will be transmitted in 1 byte single frame
		just after SPI initialization
		PA5 = SCK
		PA7 = MOSI
*/
#define TRANSMIT_TEST 


void pllInit(void);
void spiRelInit(void);
void spiInit(void);


int main(){
	
	// PLL1 enable integer mode, clock source CSI selected
	pllInit();
	
	// SPI output pins + alternate functions + high level clocks to SPI & GPIO
	spiRelInit();
	
	// SPI register initialization
	spiInit();
	
	
	/*
			SPI kernel clock = 200 MHz
	
			SPI mode 0
			- CPOL & CPHA = 0
	
	
	
			SPI speed calculation
	
			- PLL1 source = 4 MHz
			- multiplication factor for VCO = 50x
			-- 4x50 = 200 MHz
	
			
			The SPI1 kernel clock is by default pll1_q_ck (RCC->D2CCIP1R & SPI123SEL[2:0] = 000)
			And in PLL1 configuration the divider of PLL1DIVR_Q1 is zero = disabled
			- So we have 200 MHz kernel clock to SPI
			
			In this case the we have SPI clock to set 200 MHz.
			
			The prescaler in SPI init is /32, so:
			200/32 = 6 MHz master clock to SPI bus
			
	
	
	
	*/
	
}


void pllInit(){
	
	/*
			PLL1 setup
	
	*/
	
	// Switch internal CSI (~4 MHz) clock on and wait when its ready
	RCC->CR |= RCC_CR_CSION;
	while(((RCC->CR)&RCC_CR_CSIRDY) != RCC_CR_CSIRDY){};
	
	
	
	// CSI selected to PLL1 clock
	RCC->PLLCKSELR &= ~RCC_PLLCKSELR_PLLSRC;
	RCC->PLLCKSELR |= RCC_PLLCKSELR_PLLSRC_CSI;
	
	// Prescaler reset & set /1
	RCC->PLLCKSELR &= ~RCC_PLLCKSELR_DIVM1;
	RCC->PLLCKSELR |= RCC_PLLCKSELR_DIVM1_0;
	
	// Medium VCO (150-420) MHz
	RCC->PLLCFGR |= RCC_PLLCFGR_PLL1VCOSEL;
	// PLL1 input fqu between 4 and 8 MHz
	RCC->PLLCFGR |= RCC_PLLCFGR_PLL1RGE;
	// fractional latch = 0
	RCC->PLLCFGR &= ~RCC_PLLCFGR_PLL1FRACEN;
	
	// PLL1 DIVP divider setup, first check PLL1 state, if on or ready -> shutdown
	if(((RCC->CR)&RCC_CR_PLL1ON) == RCC_CR_PLL1ON || ((RCC->CR)&RCC_CR_PLL1RDY) == RCC_CR_PLL1RDY){
		RCC->CR &= ~RCC_CR_PLL1ON;
		while(((RCC->CR)&RCC_CR_PLL1RDY) != RCC_CR_PLL1RDY){};
	}
	RCC->PLLCFGR &= ~RCC_PLLCFGR_DIVP1EN;
	RCC->PLLCFGR |= RCC_PLLCFGR_DIVP1EN;
	
	//PLL1 DIVQ divider enable
	RCC->PLLCFGR &= ~RCC_PLLCFGR_DIVQ1EN;
	RCC->PLLCFGR |= RCC_PLLCFGR_DIVQ1EN;
	
	//PLL1 DIVR divider enable
	RCC->PLLCFGR &= ~RCC_PLLCFGR_DIVR1EN;
	RCC->PLLCFGR |= RCC_PLLCFGR_DIVR1EN;
	
	// VCO selected 150-420
	// input fqu selected 4-8
	// multiplication factor for VCO will be 50 (0x32)
	// so output fqu will be 200 - 400 MHz
	RCC->PLL1DIVR &= ~RCC_PLL1DIVR_N1;
	RCC->PLL1DIVR |= RCC_PLL1DIVR_N1&0x32;
	
	
	
	
	
	// PLL1 DIVP division factor = 128
	RCC->PLL1DIVR &= ~RCC_PLL1DIVR_P1;
	// PLL1 DIVQ division factor = 128
	RCC->PLL1DIVR &= ~RCC_PLL1DIVR_Q1;
	// PLL1 DIVR division factor = 128
	RCC->PLL1DIVR &= ~RCC_PLL1DIVR_R1;
	
	
	// PLL1 enable and wait ready flag to continue
	RCC->CR |= RCC_CR_PLL1ON;
	while(((RCC->CR)&RCC_CR_PLL1RDY) != RCC_CR_PLL1RDY){};
	
	
}



void spiRelInit(){
	
	// PORTA clock enable
	RCC->AHB4ENR |= RCC_AHB4ENR_GPIOAEN;
	
	//SPI1 clock enable 
	RCC->APB2ENR |= RCC_APB2ENR_SPI1EN;
	
	
	/*
			PA5 = SCK
			PA7 = MOSI
	
	*/
	
	
	// GPIOA pin 7, 5 = alternate function mode
	GPIOA->MODER &= ~((GPIO_MODER_MODE7_0) | (GPIO_MODER_MODE5_0));
	GPIOA->MODER |= ((GPIO_MODER_MODE7_1) | (GPIO_MODER_MODE5_1));
	
	// alternate mux
	// PA7 = AF5 (MOSI)
	// PA5 = AF5 (SCK)
	GPIOA->AFR[0] |= 5<<28 | 5<<20;
	
}





void spiInit(){
	
	
	// CR1
	// crc calculation zero patterns
	SPI1->CR1 &= ~SPI_CR1_TCRCINI;
	// polynomial not used
	SPI1->CR1 &= ~SPI_CR1_CRC33_17;
	
	
	// CFG1
	// master clock /64
	SPI1->CFG1 |= SPI_CFG1_MBR_2;
	// CRC computation disable
	SPI1->CFG1 &= ~SPI_CFG1_CRCEN;
	// CRCSIZE = 0
	SPI1->CFG1 &= ~SPI_CFG1_CRCSIZE;
	// tx dma disable
	SPI1->CFG1 &= ~I2C_CR1_TXDMAEN;
	// rx dma disable
	SPI1->CFG1 &= ~SPI_CFG1_RXDMAEN;
	// fifo threshold level = 1-data
	SPI1->CFG1 &= ~SPI_CFG1_FTHLV;
	// Dsize = 8bits
	SPI1->CFG1 &= ~SPI_CFG1_DSIZE;
	SPI1->CFG1 |= SPI_CFG1_DSIZE_0 | SPI_CFG1_DSIZE_1 | SPI_CFG1_DSIZE_2;
	
	
	// CFG2
	// SS SW-management of SS, internal state by SSI bit
	SPI1->CFG2 |= SPI_CFG2_SSM;
	// CR1
	// SS bit high
	SPI1->CR1 |= SPI_CR1_SSI;
	
	
	
	// SPI master mode
	SPI1->CFG2 |= SPI_CFG2_MASTER;
	// simplex transmitter
	SPI1->CFG2 |= SPI_CFG2_COMM_0;
	

/*------------------- TRANSMIT TEST -------------------------*/

#ifdef TRANSMIT_TEST

// data to be transmitted
uint8_t testdata = 0xFF;

// This value will point limit to HW's counter
SPI1->CR2 |= (SPI_CR2_TSIZE & sizeof(testdata));

// SPI enable
SPI1->CR1 |= SPI_CR1_SPE;

/*

			If global SPI interrupt needed
			- Enable it here, after SPI enable (errata)
			  and before CSTART

*/

// start transmission
SPI1->CR1 |= SPI_CR1_CSTART;

// if TxFIFO has enough free location to host 1 data packet
// --> put data to TXDR
if(((SPI1->SR) & SPI_SR_TXP) == SPI_SR_TXP){
	*((__IO uint8_t*)&SPI1->TXDR) = testdata;
	
}

// wait end of the transmission
while(((SPI1->SR) & SPI_SR_EOT) != SPI_SR_EOT){};
	
// SPI disable (errata)
SPI1->CR1 |= SPI_CR1_SPE;

#endif
	
	
}

