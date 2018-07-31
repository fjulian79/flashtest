/*
 * main.cpp
 *
 *  Created on: 17 Jul, 2018
 *      Author: julian
 *
 * Simple test application to test the bsp flash capabilies.
 * Start terminal by invoking
 *
 *   picocom -b 115200 /dev/ttyACM0 --imap=lfcrlf
 * 
 */

#include <bsp/bsp.h>
#include <bsp/bsp_gpio.h>
#include "bsp/bsp_tty.h"
#include "bsp/bsp_flash.h"

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
int8_t cmd_ver(char *argv[], uint8_t argc)
{
	unused(argv);
    unused(argc);

	printf("flashtest %s\n", VERSIONSTRING);
    printf("build:   %s, %s\n", __DATE__, __TIME__);
    return 0;
}

/**
 * @brief Tp print the help text
 * 
 * @param args      The argument list
 * @return          0
 */
int8_t cmd_help(char *argv[], uint8_t argc)
{
	unused(argv);
    unused(argc);

    printf("Supported commands:\n");
    printf("  ver               Used to print version infos.\n");
    printf("  info              Used to print flash information.\n");
    printf("  dump mode [...]   Dump either memory or a entire flash page.\n");
    printf("     mode     p     Page mode, further args: addr num  [ascii]\n");
    printf("              m     Memory mode, further args: page [ascii]\n");
    printf("     addr           Memory address as decimal or hex value.\n");
    printf("     num            Number of bytes to dump as hex or decimal value.\n");
    printf("     page           Page number in page mode.\n");
    printf("     ascci    a     Optional, dump as ascii text, default are hex values.\n");
    printf("  clr page [num]    clears the given pages.\n");
    printf("     page           First Page to be cleared.\n");
    printf("     num            Optional, defaults to one.\n");
    printf("  write addr val    To write to the flash.\n");
    printf("     addr           Memory address as decimal or hex value.\n");
    printf("     val            uint16_t value to write in hex or decimal format.\n");
    printf("  lock              To lock the flash.\n");
    printf("  unlock            To unlock the flash.\n");
    printf("  help              Prints this text.\n");

    return 0;
}

int8_t cmd_info(char *argv[], uint8_t argc)
{
	unused(argv);
    unused(argc);

    printf("Flash information:\n");
    printf("  Base Addr: 0x%x\n", FLASH_BASE);
    printf("  Page Size: 0x%x\n", FLASH_PAGE_SIZE);
    printf("  Page Cnt:  %d\n", BSP_FLASH_NUMPAGES);
    printf("  Size:      %dKb\n", (FLASH_BANK1_END-FLASH_BASE+1)/1024);

    return 0;
}

void memdump(uint8_t *addr, uint16_t num, bool ascii)
{
    uint8_t *wrapAddr=0;

    while(num > 0)
    {
        printf(" %x| ", addr);
        wrapAddr = addr+16;

        while (num > 0 && addr < wrapAddr)
        {
            if (!ascii)
                printf("%02x ", *addr++);
            else
                printf("%c", *addr++);
            num--;
        } 
    
        printf("\n");
    }
}

/**
 * @brief Used to print al lthe data registers.
 * 
 * @param args      The arguemnt list
 * @return          0
 */
int8_t cmd_dump(char *argv[], uint8_t argc)
{
    uint8_t *addr=0;
    uint16_t num=0;
    bool ascii = false;

    if (argc < 2)
        return -1;

    switch (*argv[0])
    {
        case 'p':
        {
            
            if(!cli.toUnsigned(argv[1], (void*)&num, sizeof(num)))
                return -3;

            if (num > BSP_FLASH_NUMPAGES)
            {
                printf("Invalid page number\n");
                return -3;
            }   

            if (argc == 3 && *argv[2] == 'a')
                ascii = true;

            addr = (uint8_t*)(FLASH_BASE + num*FLASH_PAGE_SIZE);
            num = FLASH_PAGE_SIZE;
            break;
        }

        case 'm':
        {
            if(!cli.toUnsigned(argv[1], (void*)&addr, sizeof(addr)))
                return -3;

            if(!cli.toUnsigned(argv[2], (void*)&num, sizeof(num)))
                return -4;

            if (argc == 4 && *argv[3] == 'a')
                ascii = true;

            break;
        }
        
        default:
            return -2;
    }

    memdump(addr, num, ascii);

    return 0;
}

int8_t cmd_clrPage(char *argv[], uint8_t argc)
{
    uint8_t page=0;
    uint8_t num = 1;
    uint32_t addr = FLASH_BASE;
    bspStatus_t ret = BSP_OK;

    if (argc < 1)
        return -1;

    if(!cli.toUnsigned(argv[0], (void*)&page, sizeof(page)))
        return -2;
    
    if (argc == 2 && !cli.toUnsigned(argv[1], (void*)&num, sizeof(num)))
        return -3;

    if (page > BSP_FLASH_NUMPAGES-1 || page+num > BSP_FLASH_NUMPAGES)
    {
        return -4;
    }

    if (page < 126)
    {
        printf("No access below page 126!\n");
        return -5;
    }

    addr += page*FLASH_PAGE_SIZE;
    
    while (num > 0)
    {
        printf("%0x| clr\n", addr);
        bspGpioSet(BSP_DEBUGPIN_0);
        ret = bspFlashErasePage(addr);
        bspGpioClear(BSP_DEBUGPIN_0);
        
        if (ret != BSP_OK) 
        {
            printf("bspStatus: %u\n", ret);
            printf("flash err: 0x%x\n", bspFlashGetErr());
            return -5;   
        }

        addr += FLASH_PAGE_SIZE;
        num--;
    }

    return 0;
}

int8_t cmd_write(char *argv[], uint8_t argc)
{
    bspStatus_t ret = BSP_OK;
    uint16_t *addr = 0;
    uint16_t val = 0;

    if (argc < 2)
        return -1;

    if(!cli.toUnsigned(argv[0], (void*)&addr, sizeof(addr)))
        return -2;

    if(!cli.toUnsigned(argv[1], (void*)&val, sizeof(val)))
        return -3;

    if (   (addr < (uint16_t *)(FLASH_BASE+(126*FLASH_PAGE_SIZE)))
        || (addr > (uint16_t *)(FLASH_BASE+(128*FLASH_PAGE_SIZE)-1)))
    {
        printf("Dont wont to write to other pages then 126 or 127\n");
        return -4;
    }

    bspGpioSet(BSP_DEBUGPIN_0);
    ret = bspFlashProgHalfWord(addr, val);
    bspGpioClear(BSP_DEBUGPIN_0);
    
    if (ret != BSP_OK) 
    {
        printf("bspStatus: %u\n", ret);
        printf("flash err: 0x%x\n", bspFlashGetErr());
        return -5;   
    }

    if (*addr != val)
        return -6;
    
    return 0;
}


int8_t cmd_lock(char *argv[], uint8_t argc)
{
    unused(argv);
    unused(argc);
    
    bspGpioSet(BSP_DEBUGPIN_0);
    bspFlashLock();

    return 0;
}

int8_t cmd_unlock(char *argv[], uint8_t argc)
{
    unused(argv);
    unused(argc);
    
    if (bspFlashUnlock() != BSP_OK)
        return -1;

    return 0;
}


cliCmd_t cmd_table[] =
{
   {"ver", cmd_ver},
   {"help", cmd_help},
   {"info", cmd_info},
   {"dump", cmd_dump},
   {"clr", cmd_clrPage},
   {"write", cmd_write},
   {"lock", cmd_lock},
   {"unlock", cmd_unlock},
   {0,      0}
};

int main(void)
{
    LL_GPIO_InitTypeDef init;    
    uint32_t sysTick = 0;
    uint32_t ledTick = 0;

    bspChipInit();

    init.Mode = LL_GPIO_MODE_OUTPUT;
    init.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
    init.Pull = LL_GPIO_PULL_DOWN;
    init.Speed = LL_GPIO_SPEED_FREQ_MEDIUM;
    bspGpioPinInit(BSP_DEBUGPIN_0, &init);
    bspGpioPinInit(BSP_DEBUGPIN_1, &init);
    bspGpioClear(BSP_DEBUGPIN_0);
    bspGpioClear(BSP_DEBUGPIN_1);

    cmd_ver(0, 0);    
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
        	cli.procByte((uint8_t) bspTTYGetChar());
        }
    }
}
