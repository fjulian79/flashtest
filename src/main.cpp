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

#include "fds/fds.hpp"
#include "cli/cli.h"
#include "generic/generic.h"

#include <stdio.h>
#include <stdint.h>
#include <string.h>

#define VERSIONSTRING       "rel_2_0_0"

#define MIN_PAGE            (BSP_FLASH_NUMPAGES - 5)

Cli cli;

/**
 * @brief Prints the version information
 * 
 * @param args      The argument list
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
    printf("     mode     p     Page mode, further args: addr num [ascii]\n");
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
    printf("  fds cmd [...]     Used to trigger one of the following fds commands:\n");
    printf("     format         To format the fds flash pages.\n");
    printf("     info           To print the fds status infos.\n");
    printf("     write i v n    To write data.\n");
    printf("                    i = fds data id.\n");
    printf("                    v = byte value.\n");
    printf("                    n = number of bytes with value v.\n");
    printf("     delete id      To delete the given ID.\n");
    printf("     dump           To print the stored data. \n");
    printf("  help              Prints this text.\n");

    return 0;
}

int8_t cmd_info(char *argv[], uint8_t argc)
{
    Fds *pFds = pFds->getInstance();
	unused(argv);
    unused(argc);

    printf("Flash information:\n");
    printf("  Base Addr: 0x%x\n", FLASH_BASE);
    printf("  Page Size: 0x%x\n", FLASH_PAGE_SIZE);
    printf("  Page Cnt:  %d\n", BSP_FLASH_NUMPAGES);
    printf("  Size:      %dKb\n", (FLASH_BANK1_END-FLASH_BASE+1)/1024);
    printf("\n");
    printf("FDS status:\n");
    pFds->info();
    printf("\n");

    return 0;
}

int8_t cmd_reg(char *argv[], uint8_t argc)
{
	unused(argv);
    unused(argc);

    printf("Flash registers:\n");
    printf("ACR: 0x%0lx\n", FLASH->ACR);
    printf("SR:  0x%0lx\n", FLASH->SR);
    printf("CR:  0x%0lx\n", FLASH->CR);

    return 0;
}

void memdump(uint8_t *addr, uint16_t num, bool ascii)
{
    uint8_t *wrapAddr=0;

    while(num > 0)
    {
        printf(" %lx| ", (uint32_t) addr);
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
 * @brief Used to print all the data registers.
 * 
 * @param args      The argument list
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

            if (num > BSP_FLASH_NUMPAGES - 1)
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
    uint16_t *addr = 0;
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

    if (page < MIN_PAGE)
    {
        printf("ERROR: Access below page %d prohibited!\n", MIN_PAGE);
        return -5;
    }

    addr = BSP_FLASH_PAGETOADDR(page);
    
    while (num > 0)
    {
        printf("%0lx| clr\n", (uint32_t)addr);
        bspGpioSet(BSP_DEBUGPIN_0);
        ret = bspFlashErasePage(addr);
        bspGpioClear(BSP_DEBUGPIN_0);
        
        if (ret != BSP_OK) 
        {
            printf("bspStatus: %u\n", ret);
            printf("flash err: 0x%lx\n", bspFlashGetErr());
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

    if ((addr < BSP_FLASH_PAGETOADDR(MIN_PAGE)) ||
        (addr > BSP_FLASH_PAGETOADDR(BSP_FLASH_NUMPAGES) - 1))
    {
        printf("ERROR: Access to address out of range prohibited!\n");
        return -4;
    }

    bspGpioSet(BSP_DEBUGPIN_0);
    ret = bspFlashProgHalfWord(addr, val);
    bspGpioClear(BSP_DEBUGPIN_0);
    
    if (ret != BSP_OK) 
    {
        printf("bspStatus: %u\n", ret);
        printf("flash err: 0x%lx\n", bspFlashGetErr());
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

int8_t fdswrite(char *argv[], uint8_t argc)
{
    Fds *pFds = pFds->getInstance();
    uint8_t uid = 0;;
    uint8_t val = 0;
    uint16_t siz = 0;
    uint8_t data[FDS_MAX_DATABYTES];

    if (argc < 3)
        return -1;

    if(!cli.toUnsigned(argv[0], (void*)&uid, sizeof(uid)))
        return -2;

    if(!cli.toUnsigned(argv[1], (void*)&val, sizeof(val)))
        return -3;

    if(!cli.toUnsigned(argv[2], (void*)&siz, sizeof(siz)))
        return -4;

    if(siz > sizeof(data))
        return -5;

    for (size_t i = 0; i < siz; i++)
        data[i] = val;
    
    return pFds->write(uid, data, siz);;
}

int8_t fdsdump(char *argv[], uint8_t argc)
{
    Fds *pFds = pFds->getInstance();
    uint8_t data[FDS_MAX_DATABYTES];
    size_t siz = 0;

    unused(argv);
    unused(argc);

    for (uint8_t id = 0; id <FDS_NUM_RECORDS; id++)
    {
        siz = pFds->read(id, data, sizeof(data));
        if (siz != 0)
        {
            printf("Got %u bytes for data Id %u:\n", siz, id);
            memdump(data, siz, false);    
        }
        else
        {
            printf("Data Id %u not found.\n", id);
        }
    }
    
    return 0;
}

int8_t fdsdel(char *argv[], uint8_t argc)
{
    Fds *pFds = pFds->getInstance();
    uint8_t uid = 0;

    if (argc < 1)
        return -1;

    if(!cli.toUnsigned(argv[0], (void*)&uid, sizeof(uid)))
        return -2;

    return pFds->del(uid);
}

int8_t cmd_fds(char *argv[], uint8_t argc)
{
    Fds *pFds = pFds->getInstance();
    uint8_t retval = 0;

    if (argc < 1)
        return -1;

    if(strcmp("format", argv[0]) == 0)
        retval = pFds->format();
    else if(strcmp("info", argv[0]) == 0)
        pFds->info();
    else if(strcmp("write", argv[0]) == 0)
        retval = fdswrite(&argv[1], argc-1);
    else if(strcmp("dump", argv[0]) == 0)
        retval = fdsdump(&argv[1], argc-1);
    else if(strcmp("delete", argv[0]) == 0)
        retval = fdsdel(&argv[1], argc-1);
    else
        retval = -2;

    return retval;
}

cliCmd_t cmd_table[] =
{
   {"ver", cmd_ver},
   {"help", cmd_help},
   {"info", cmd_info},
   {"reg", cmd_reg},
   {"dump", cmd_dump},
   {"clr", cmd_clrPage},
   {"write", cmd_write},
   {"lock", cmd_lock},
   {"unlock", cmd_unlock},
   {"fds", cmd_fds},
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

    printf("\n\n");
    cmd_ver(0, 0);
    printf("\n");
    cmd_info(0, 0);

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
