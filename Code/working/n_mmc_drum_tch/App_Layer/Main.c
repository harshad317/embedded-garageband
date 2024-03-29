/***********************************************************************************
  Filename: light_switch.c

  Description:  This application function either as a light or a
  switch toggling the ligh. The role of the
  application is chosen in the menu with the joystick at initialisation.

  Push S1 to enter the menu. Choose either switch or
  light and confirm choice with S1.
  Joystick Up: Sends data from switch to light

***********************************************************************************/

/***********************************************************************************
* INCLUDES
*/
#include <hal_led.h>
#include <hal_assert.h>
#include <hal_board.h>
#include <hal_int.h>
#include "hal_mcu.h"
#include "hal_button.h"
#include "hal_rf.h"
#include "basic_rf.h"
#include "uart_intfc.h"
#include "TouchScreen.h"
#include <mmc.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "hal_spi.h"
#include "TimerManager.h"
#include "Drum.h"

unsigned long cardSize = 0;
unsigned char status = 1;
unsigned int timeout = 0;
int i = 0;
unsigned char buffer[512];
char tx_buf[50];

unsigned char Check_Flag = 1;
unsigned int tmp_val = 0;

   //--------------------------------------------------------------------------
   //  Timers:
   //--------------------------------------------------------------------------
   TimerId timerID1 = TimerId_INVALID;
   TimerId timerID2 = TimerId_INVALID;


/***********************************************************************************
* CONSTANTS
*/


/***********************************************************************************
* LOCAL VARIABLES
*/


/***********************************************************************************
* LOCAL FUNCTIONS
*/
void MMC_Test();
void App_ISR1(TimerId timerId);
void App_ISR2(TimerId timerId);



/***********************************************************************************
* @fn          main
*
* @brief       This is the main entry of the "Light Switch" application.
*              After the application modes are chosen the switch can
*              send toggle commands to a light device.
*
* @param       basicRfConfig - file scope variable. Basic RF configuration
*              data
*              appState - file scope variable. Holds application state
*
* @return      none
*/
void main(void)
{

    // Initalise board peripherals
    halBoardInit();
    
    uart_intfc_init();
    
    tx1_send("Hello World, DRUM\n\r", 19);
    
    //TouchScreen();
    TimerInterface_Initialize();
    DrumSet();
    while(1);
    

   timerID2 = SetTimerReq(&App_ISR2,1200);
   
   timerID1 = SetTimerReq(&App_ISR1,2);
   
   while(1)
   {
     if(Check_Flag)
     {
       tmp_val = Capture_TimerReq(timerID2);
       sprintf(tx_buf, "Capture value: %d\n\r",tmp_val);
       tx1_send(tx_buf, strlen(tx_buf));
       Check_Flag = 0;
     }
   }
     
    
}


void App_ISR1(TimerId timerId)
{
  halLedToggle(1);
  timerID1 = TimerId_INVALID;
  timerID1 = SetTimerReq(&App_ISR1,2);
  Check_Flag = 1;
}


void App_ISR2(TimerId timerId)
{
  halLedToggle(3);
  timerID2 = TimerId_INVALID;
  timerID2 = SetTimerReq(&App_ISR2,1200);
}




void MMC_Test()
{
  //while ((mmcPing() != MMC_SUCCESS));      // Wait till card is inserted
  
    tx0_send("\n\rCard Inserted\n\r",sizeof("\n\rCard Inserted\n\r"));
  
    //Initialisation of the MMC/SD-card
  while (status != 0)                       // if return in not NULL an error did occur and the
                                            // MMC/SD-card will be initialized again 
  {
    status = mmcInit();
    timeout++;
    if (timeout == 150)                      // Try 50 times till error
    {
      sprintf (tx_buf, "No MMC/SD-card found!! %x\n", status);
      tx0_send(tx_buf,strlen(tx_buf));
      break;
    }
  }
  
  tx0_send("Intit Success\n\r",sizeof("Intit Success\n\r"));


  // Read the Card Size from the CSD Register
  cardSize =  mmcReadCardSize();
  
  sprintf(tx_buf, "Card Size: %d\n\r",cardSize);
  tx0_send(tx_buf,strlen(tx_buf));
    
// Clear Sectors on MMC
  for (i = 0; i < 512; i++) buffer[i] = 0;
  mmcWriteSector(0, buffer);                // write a 512 Byte big block beginning at the (aligned) adress

  for (i = 0; i < 512; i++) buffer[i] = 0;
  mmcWriteSector(1, buffer);                // write a 512 Byte big block beginning at the (aligned) adress


// Write Data to MMC  
  for (i = 0; i < 512; i++) buffer[i] = i;
  mmcWriteSector(0, buffer);                // write a 512 Byte big block beginning at the (aligned) adress

  for (i = 0; i < 512; i++) buffer[i] = i+64;
  mmcWriteSector(1, buffer);                // write a 512 Byte big block beginning at the (aligned) adress
  
  sprintf(tx_buf, "Buffer Write Complete\n\r");
  tx0_send(tx_buf,strlen(tx_buf));
  
  for (i = 0; i < 512; i++) buffer[i] = 0;

  mmcReadSector(0, buffer);                 // read a size Byte big block beginning at the address.
  
  sprintf(tx_buf, "Buffer Read Sector 0:\n\r");
  tx0_send(tx_buf,strlen(tx_buf));
  
  for (i = 0; i < 512; i++) tx0_send(&buffer[i], 1);

  mmcReadSector(1, buffer);                 // read a size Byte big block beginning at the address.
  
  sprintf(tx_buf, "Buffer Read Sector 1:\n\r");
  tx0_send(tx_buf,strlen(tx_buf));
  
  for (i = 0; i < 512; i++) tx0_send(&buffer[i], 1);

  for (i = 0; i < 512; i++)
    mmcReadSector(i, buffer);               // read a size Byte big block beginning at the address.

  mmcGoIdle();                              // set MMC in Idle mode

  while (1);
}




/****************************************************************************************
  Copyright 2007 Texas Instruments Incorporated. All rights reserved.

  IMPORTANT: Your use of this Software is limited to those specific rights
  granted under the terms of a software license agreement between the user
  who downloaded the software, his/her employer (which must be your employer)
  and Texas Instruments Incorporated (the "License").  You may not use this
  Software unless you agree to abide by the terms of the License. The License
  limits your use, and you acknowledge, that the Software may not be modified,
  copied or distributed unless embedded on a Texas Instruments microcontroller
  or used solely and exclusively in conjunction with a Texas Instruments radio
  frequency transceiver, which is integrated into your product.  Other than for
  the foregoing purpose, you may not use, reproduce, copy, prepare derivative
  works of, modify, distribute, perform, display or sell this Software and/or
  its documentation for any purpose.

  YOU FURTHER ACKNOWLEDGE AND AGREE THAT THE SOFTWARE AND DOCUMENTATION ARE
  PROVIDED �AS IS� WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESS OR IMPLIED,
  INCLUDING WITHOUT LIMITATION, ANY WARRANTY OF MERCHANTABILITY, TITLE,
  NON-INFRINGEMENT AND FITNESS FOR A PARTICULAR PURPOSE. IN NO EVENT SHALL
  TEXAS INSTRUMENTS OR ITS LICENSORS BE LIABLE OR OBLIGATED UNDER CONTRACT,
  NEGLIGENCE, STRICT LIABILITY, CONTRIBUTION, BREACH OF WARRANTY, OR OTHER
  LEGAL EQUITABLE THEORY ANY DIRECT OR INDIRECT DAMAGES OR EXPENSES
  INCLUDING BUT NOT LIMITED TO ANY INCIDENTAL, SPECIAL, INDIRECT, PUNITIVE
  OR CONSEQUENTIAL DAMAGES, LOST PROFITS OR LOST DATA, COST OF PROCUREMENT
  OF SUBSTITUTE GOODS, TECHNOLOGY, SERVICES, OR ANY CLAIMS BY THIRD PARTIES
  (INCLUDING BUT NOT LIMITED TO ANY DEFENSE THEREOF), OR OTHER SIMILAR COSTS.

  Should you have any questions regarding your right to use this Software,
  contact Texas Instruments Incorporated at www.TI.com.
***********************************************************************************/
