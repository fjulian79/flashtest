/*
 * main.cpp
 *
 *  Created on: 17 Jul, 2018
 *      Author: julian
 *
 * Simple test application to test the bsp eeprom capabilies.
 * Start terminal by invoking
 *
 *   picocom -b 115200 /dev/ttyACM0 --imap=lfcrlf
 * 
 */

#include <bsp/bsp.h>
#include <bsp/bsp_gpio.h>
#include "bsp/bsp_tty.h"

#include "cli/cli.h"
#include "generic/generic.h"

#include <stdio.h>
#include <stdint.h>

#define VERSIONSTRING      "rel_1_0_0"

Cli cli;

/**
 * @brief Prints the version inforamtion
 * 
 * @param args      The arguemnt list
 * @return          0
 */
int8_t cmd_ver(void* args)
{
	unused(args);

	printf("eepromtest %s\n", VERSIONSTRING);
    printf("build:   %s, %s\n", __DATE__, __TIME__);
    return 0;
}

/**
 * @brief Tp print the help text
 * 
 * @param args      The argument list
 * @return          0
 */
int8_t cmd_help(void* args)
{
    unused(args);

    printf("Supported commands:\n");
    printf("  ver      Used to print version infos.\n");
    printf("  help     Prints this text.\n");

    return 0;
}

cli_cmd_t cmd_table[] =
{
   {"ver", cmd_ver},
   {"help", cmd_help},
   {0,      0}
};

int main(void)
{
    uint32_t sysTick = 0;
    uint32_t ledTick = 0;

    bspChipInit();

    cmd_ver(0);    
    cli.init(cmd_table, arraysize(cmd_table));

    while (1)
    {
        sysTick = bspGetSysTick();

        if (sysTick - ledTick >= 250)
        {
            ledTick= sysTick;
            bspGpioToggle(BSP_GPIO_LED);
        }

        if (bspTTYDataAvailable())
        {
        	cli.proc_byte((uint8_t) bspTTYGetChar());
        }
    }
}
