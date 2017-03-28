/*
   printf wrapper for Cypress USB-Serial Adapter

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
#include "usbprintf.h"
#define USB_TIMEOUT 30

void usbprintf(const char *fmt, ...) {
    int toCount = USB_TIMEOUT;
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
    if (0u != USBUART_1_GetConfiguration())
    {
        static char outie[MAX_PRINTF];
        va_list ap;
        va_start(ap, fmt);
        vsnprintf(outie, MAX_PRINTF, fmt, ap);
        va_end(ap);
        while(toCount && !USBUART_1_CDCIsReady()) {
            toCount--;
        };
        if(toCount) USBUART_1_PutString(outie);
    }

}

void moveCursor(uint8 line, uint8 col) 
    { usbprintf(CSI "%d;%dH", line, col); }
