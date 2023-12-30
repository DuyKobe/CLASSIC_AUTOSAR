/*
 * Copyright (c) 2014 - 2016, Freescale Semiconductor, Inc.
 * Copyright (c) 2016 - 2018, NXP.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 *
 * 3. Neither the name of the copyright holder nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY NXP "AS IS" AND ANY EXPRESSED OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL NXP OR ITS CONTRIBUTORS BE LIABLE FOR ANY DIRECT,
 * INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING
 * IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGE.
 */

/*!
 * Description:
 * ==================================================================================================
 * This project provides common initialization for clocks and an LPIT channel counter function.
 * Core clock is set to 80 MHz. LPIT0 channel 0 is configured to count one second of SPLL clocks.
 * Software polls the channel�s timeout flag and toggles the GPIO output to the LED when the flag sets.
 */

#include "device_registers.h"            /* include peripheral declarations S32K144 */
#include "clocks_and_modes.h"
#include "Dio.h" 
#include "Port.h"

uint8_t LedOldStatus = 0;
uint8_t PressBTN1 = 0;
int LedStatus =  0;

void Init_clock();
void LedOnOff(Dio_ChannelType LED);
void LPIT0_Duration();
void LPIT0_init (void)
{
	/*!
	 * LPIT Clocking:
	 * ==============================
	 */
  PCC->PCCn[PCC_LPIT_INDEX] = PCC_PCCn_PCS(6);    /* Clock Src = 6 (SPLL2_DIV2_CLK)*/
  PCC->PCCn[PCC_LPIT_INDEX] |= PCC_PCCn_CGC_MASK; /* Enable clk to LPIT0 regs 		*/

  /*!
   * LPIT Initialization:
   */
  LPIT0->MCR |= LPIT_MCR_M_CEN_MASK;  /* DBG_EN-0: Timer chans stop in Debug mode */
                              	  	  /* DOZE_EN=0: Timer chans are stopped in DOZE mode */
                              	  	  /* SW_RST=0: SW reset does not reset timer chans, regs */
                                      /* M_CEN=1: enable module clk (allows writing other LPIT0 regs) */
  LPIT0->TMR[0].TVAL = 60000000;      /* Chan 0 Timeout period: 40M clocks */

  LPIT0->TMR[0].TCTRL |= LPIT_TMR_TCTRL_T_EN_MASK;
                              /* T_EN=1: Timer channel is enabled */
                              /* CHAIN=0: channel chaining is disabled */
                              /* MODE=0: 32 periodic counter mode */
                              /* TSOT=0: Timer decrements immediately based on restart */
                              /* TSOI=0: Timer does not stop after timeout */
                              /* TROT=0 Timer will not reload on trigger */
                              /* TRG_SRC=0: External trigger soruce */
                              /* TRG_SEL=0: Timer chan 0 trigger source is selected*/
}

int main(void)
{
	/*!
	 * Initialization:
	 * =======================
	 */
  SOSC_init_8MHz();       /* Initialize system oscilator for 8 MHz xtal */
  SPLL_init_160MHz();     /* Initialize SPLL to 160 MHz with 8 MHz SOSC */
  NormalRUNmode_80MHz();  /* Init clocks: 80 MHz sysclk & core, 40 MHz bus, 20 MHz flash */
  LPIT0_init();           /* Initialize PIT0 for 1 second timeout  */
  
  Init_clock();
  Port_Init(&Port_Config);
  LedOnOff(DioConf_DioChannel_LED_RED);
  
	  for (;;)
	  {
            if(1 == Dio_ReadChannel((Dio_ChannelType) DioConf_DioChannel_BTN1))
            {
              PressBTN1 = 1;
            }
            if(1 == PressBTN1)
            {
              //press_BTN1 = 0;
               LPIT0_Duration();
               
                if((0 == LedStatus) && (0 == LedOldStatus))
                {    
                  LedOldStatus = 1;
                  LedOnOff( (Dio_ChannelType) DioConf_DioChannel_LED_RED);
                }
                else if((1 == LedStatus) && (0 == LedOldStatus))
                {    
                  LedOldStatus = 1;
                  LedOnOff( (Dio_ChannelType) DioConf_DioChannel_LED_GREEN);
                }
                else if((2 == LedStatus) && (0 == LedOldStatus))
                {    
                  LedOldStatus = 1;
                  LedOnOff( (Dio_ChannelType) DioConf_DioChannel_LED_BLUE);
                }
                
                if(1 == Dio_ReadChannel((Dio_ChannelType) DioConf_DioChannel_BTN2))
                {
                  while(1 == Dio_ReadChannel((Dio_ChannelType) DioConf_DioChannel_BTN2)) {}
                }
                
            }
                
	  }
}

void Init_clock()
{
  PCC-> PCCn[PCC_PORTA_INDEX] = PCC_PCCn_CGC_MASK;
  PCC-> PCCn[PCC_PORTB_INDEX] = PCC_PCCn_CGC_MASK;
  PCC-> PCCn[PCC_PORTC_INDEX] = PCC_PCCn_CGC_MASK;
  PCC-> PCCn[PCC_PORTD_INDEX] = PCC_PCCn_CGC_MASK;
  PCC-> PCCn[PCC_PORTE_INDEX] = PCC_PCCn_CGC_MASK;
}

void LedOnOff(Dio_ChannelType LED)
{
  Dio_WriteChannel( (Dio_ChannelType) DioConf_DioChannel_LED_RED, (Dio_LevelType) STD_HIGH);
  Dio_WriteChannel( (Dio_ChannelType) DioConf_DioChannel_LED_BLUE, (Dio_LevelType) STD_HIGH);
  Dio_WriteChannel( (Dio_ChannelType) DioConf_DioChannel_LED_GREEN, (Dio_LevelType) STD_HIGH);
  
  Dio_WriteChannel( (Dio_ChannelType) LED, (Dio_LevelType) STD_LOW);
}

void LPIT0_Duration()
{
  if(1 == (LPIT0->MSR & LPIT_MSR_TIF0_MASK))
  {
     LPIT0->MSR |= LPIT_MSR_TIF0_MASK;
     if( LedStatus >= 2 )
     {
       LedStatus = 0;
       LedOldStatus = 0;
     }
     else
     {
       LedStatus++;
       LedOldStatus = 0;
     }
   }
}