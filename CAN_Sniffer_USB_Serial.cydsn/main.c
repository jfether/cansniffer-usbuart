/*
   CAN-Bus Sniffer Tool with UART support

   Copyright 2017 Jonathan Fether

   Licensed under the Apache License, Version 2.0 (the "License");
   you may not use this file except in compliance with the License.
   You may obtain a copy of the License at

       http://www.apache.org/licenses/LICENSE-2.0

   Unless required by applicable law or agreed to in writing, software
   distributed under the License is distributed on an "AS IS" BASIS,
   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
   See the License for the specific language governing permissions and
   limitations under the License.
   
*/

#include <project.h>
#include <stdio.h>
#include <stdbool.h>
#include "usbprintf.h"

#define CAN_RX_MAILBOX_0_SHIFT      (1u)
#define CAN_RX_MAILBOX_1_SHIFT      (2u)
#define ONE_BYTE_OFFSET             (8u)
#define MAX_REGISTERS 64
#define CAN_NO_REGISTER 0x800

/* Function prototypes */
CY_ISR_PROTO(ISR_CAN);
CY_ISR_PROTO(ISR_SWITCHY);

/* Global variable used to store receive register number */
volatile bool canDirty = false;
volatile bool screenCleared = false;

typedef struct canregister {
    uint16 addr;
    uint8 len;
    uint8 data[8];
    bool printed;
} canregister_t;

volatile canregister_t registersFound[MAX_REGISTERS];

void printRegister(int r) {
    int i;
    // Clear existing data
    moveCursor((r/2)+1, 40);
    // Clear left if even, right if odd.
    // Move to correct position
    moveCursor((r/2)+1, ((r%2)*40)+1);
    usbprintf("%.3x: ",registersFound[r].addr);
    for(i=0; i<registersFound[r].len; i++) {
        usbprintf("%.2x ", registersFound[r].data[i]);
    }
}

void clearRegisters() {
    int i;
    for(i=0;i<MAX_REGISTERS; i++) {
        registersFound[i].addr = CAN_NO_REGISTER;
    }
}

int getRegisterIndex(uint16 addr) {
    int i;
    for(i=0;i<MAX_REGISTERS;i++) {
        if(registersFound[i].addr == addr) return i;
        if(registersFound[i].addr == CAN_NO_REGISTER) 
        {
            registersFound[i].addr = addr;
            return i;
        }
    }
    return -1;
}

void screenBlank() {
    int i;
    // Erase screen
    usbprintf(CLEAR_SCREEN HIDE_CURSOR);
    // Clear printed attribute for all occupied registers.
    for(i=0;i<MAX_REGISTERS; i++) {
        if(registersFound[i].addr == CAN_NO_REGISTER) return;
        registersFound[i].printed = false;
    }
}

void printAllRegisters() {
    int i;
    for(i=0;i<MAX_REGISTERS; i++) {
        if(registersFound[i].addr == CAN_NO_REGISTER) return;
        if(registersFound[i].printed) continue;
        printRegister(i);
    }
}

/*******************************************************************************
* Function Name: main
********************************************************************************
*
* Summary:
*
* Parameters:
*  None.
*
* Return:
*  None.
*
*******************************************************************************/
int main()
{
    uint8 rxData[64];
    
    CAN_Start();
    // Switchy_SetInterruptMode(Switchy_0_INTR, Switchy_INTR_BOTH);

    /* Set CAN interrupt handler to local routine */
    CyIntSetVector(CAN_ISR_NUMBER, ISR_CAN);    
    // CyIntSetVector(Switchy_0_INTR, ISR_SWITCHY);    
    
    CAN_SetOpMode(CAN_LISTEN_ONLY);

    CyGlobalIntEnable;
    clearRegisters();
    USBUART_1_Start(0, USBUART_1_5V_OPERATION);
    
    for(;;)
    {
        /* Host can send double SET_INTERFACE request. */
        if (0u != USBUART_1_IsConfigurationChanged())
        {
            /* Initialize IN endpoints when device is configured. */
            if (0u != USBUART_1_GetConfiguration())
            {
                /* Enumeration is done, enable OUT endpoint to receive data 
                 * from host. */
                USBUART_1_CDC_Init();
            }
        }
        /* Service USB CDC when device is configured. */
        if (0u != USBUART_1_GetConfiguration())
        {
            /* Check for input data from host. */
            if (USBUART_1_DataIsReady())
            {
                rxData[0] = 0;
                // Flash activity indicator
                LED_PIN_Write(!LED_PIN_Read());
                USBUART_1_GetAll(rxData);
                // "E" erases the access of all prior captured registers.
                if((rxData[0] == 'e') || (rxData[0] == 'E'))
                {
                    CAN_RX_INT_DISABLE(0);
                    clearRegisters();
                    CAN_RX_INT_ENABLE(0);
                }
                // Any other keystroke redraws the screen in case of a problem.
                screenCleared = false;
                printAllRegisters();
            }
        }
        
        if (canDirty)
        {
            // Temporary disable interrupt to prevent screen tearing
            CAN_RX_INT_DISABLE(0);
            if(!screenCleared) screenBlank();
            printAllRegisters();
            CAN_RX_INT_ENABLE(0);
            canDirty = false;
        }
    }
}


/*******************************************************************************
* Function Name: ISR_CAN
********************************************************************************
*
* Summary:
*  This ISR is executed at a Receive Message event and set receiveMailboxNumber
*  global variable with receive message mailbox number.
*
* Parameters:
*  None.
*
* Return:
*  None.
*
*******************************************************************************/
CY_ISR(ISR_CAN)
{
    // Flash activity indicator
    LED_PIN_Write(!LED_PIN_Read());
    int regIdx = getRegisterIndex(CAN_GET_RX_ID(0));
    if(regIdx==-1) return;  // No registers remain.
    registersFound[regIdx].len = CAN_GET_DLC(0);
    // Load registers with data bytes
    registersFound[regIdx].data[0] = CAN_RX_DATA_BYTE1(0);
    registersFound[regIdx].data[1] = CAN_RX_DATA_BYTE2(0);
    registersFound[regIdx].data[2] = CAN_RX_DATA_BYTE3(0);
    registersFound[regIdx].data[3] = CAN_RX_DATA_BYTE4(0);
    registersFound[regIdx].data[4] = CAN_RX_DATA_BYTE5(0);
    registersFound[regIdx].data[5] = CAN_RX_DATA_BYTE6(0);
    registersFound[regIdx].data[6] = CAN_RX_DATA_BYTE7(0);
    registersFound[regIdx].data[7] = CAN_RX_DATA_BYTE8(0);
    CAN_RX_ACK_MESSAGE(0);
    /* Clear Receive Message flag */
    CAN_INT_SR_REG.byte[1u] = CAN_RX_MESSAGE_MASK;
    canDirty = true;
}


/* [] END OF FILE */
