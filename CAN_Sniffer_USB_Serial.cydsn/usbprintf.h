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

#ifndef USBPRINTF_H
#include <project.h>
#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>

// Text formatting
#define CSI "\033["
#define CLEAR_SCREEN CSI "3J"
#define TEXT_NORMAL CSI "m"
#define TEXT_BOLD CSI "1m"
#define TEXT_UNDERLINE CSI "4m"
#define TEXT_INVERSE CSI "7m"
#define CLEAR_RIGHT CSI "1K"
#define CLEAR_LEFT CSI "0K"
#define HIDE_CURSOR CSI "?25l"
#define SHOW_CURSOR CSI "?25h"
    
#define MAX_PRINTF                  0xff  /* Max length of messages written by usbprintf() */
void usbprintf(const char *fmt, ...);
void moveCursor(uint8 line, uint8 col); // line and col are 1-based, not 0-based.
#endif
