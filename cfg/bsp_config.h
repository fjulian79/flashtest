/*
 * bsp_config.h
 *
 *  Created on: Jun 25, 2017
 *      Author: julian
 */

#ifndef LIBBSP_NUCLEO_F103_BSP_CONFIG_H_
#define LIBBSP_NUCLEO_F103_BSP_CONFIG_H_

/**
 * BSP global enabled definitions.
 */
#define BSP_ENABLED                     1

/**
 * BSP global disabled definitions.
 */
#define BSP_DISABLED                    0

/**
 * Enable to use the internal oscilator.
 * 
 * Attetion:  If enable the system clock will be set to 64MHz
 *              
 *            If disabled or undefined the system clock we be generated from
 *            HSE bypass and will be set to 72Mhz. It is assumed that a 8MHz
 *            is provided to the external clock input.
 */
#define BSP_CLOCKSRC_HSI                BSP_ENABLED

/**
 * If enabled the bsp implements the sys tick interrupt and runs a tick counter.
 */
#define BSP_SYSTICK                     BSP_ENABLED

/**
 * Baud rate of the serial interface
 */
#define BSP_TTY_BAUDRATE                115200

/**
 * If enabled printf will use DMA
 */
#define BSP_TTY_TX_DMA                  BSP_ENABLED

/**
 * Defines the fifo size used for TTY DMA transmissions.
 */
#define BSP_TTY_TX_BUFSIZ               240

/**
 * If enabled the RX interrupt will be enabled and the incoming data will be
 * written to a internal ring buffer.
 */
#define BSP_TTY_RX_IRQ                  BSP_ENABLED

/**
 * Defines the RX fifo size used in case of enabled RX interrupt.
 */
#define BSP_TTY_RX_BUFSIZ               16

/**
 * If enabled _write (and therefore printf) will block until everything has
 * been written to the fifo. If not data which does not fit to the fifo will
 * be discarded.
 */
#define BSP_TTY_BLOCKING                BSP_ENABLED

/**
 * Interrupt priority configuration.
 *
 * See bsp.h for min max vaules and how they should be interpreted.
 */
#define BSP_SYSTICK_IRQ_PRIO            BSP_IRQPRIO_MAX
#define BSP_TTY_USART_IRQ_PRIO          (BSP_IRQPRIO_MAX + 1)
/**
 * GPIO definitions.
 *
 * The bsp will define all GPIO's within a enumeration type called bspGpioPin_t
 * in the following way BSP_GPIO_<PORT><PIN>. Here those names can be replaced
 * by redefining them to define special function pins.
 *
 * ATTENTION:  Hence that there are such definitions within bsp.h for those pins
 *             the BSP uses internally like the serial port etc.
 */

/**
 * I2C IO pins
 */
#define BSP_GPIO_B8                     BSP_GPIO_I2C1_SCL
#define BSP_GPIO_B9                     BSP_GPIO_I2C1_SDA

#define BSP_GPIO_B10                    BSP_GPIO_I2C2_SCL
#define BSP_GPIO_B11                    BSP_GPIO_I2C2_SDA

/**
 * Debug pins for use with a logic analyzer.
 */
#define BSP_GPIO_C3                     BSP_DEBUGPIN_0
#define BSP_GPIO_C2                     BSP_DEBUGPIN_1

/**
 * If enabled bsp_assert.h will implement assertions.
 */
#define BSP_DOASSERT                    BSP_ENABLED

#endif /* LIBBSP_NUCLEO_F103_BSP_CONFIG_H_ */
