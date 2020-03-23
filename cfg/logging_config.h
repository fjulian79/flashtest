/*
 * liblogging, a minimalistic approach to handle log messages with different 
 * severities. See the README.md for details.
 *
 * Copyright (C) 2020 Julian Friedrich
 * 
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>. 
 *
 * You can file issues at https://github.com/fjulian79/liblogging
 */

#ifndef LOGGING_CONFIG_H_
#define LOGGING_CONFIG_H_

#ifdef __cplusplus
extern "C" {
#endif

#ifndef LOGLEVEL

/**
 * @brief The loglevel to apply to this application. Please note that this 
 * symbol will only be set if not defined previously. This allowes to use 
 * differnet log levels in one binary. All severities up tp the defined value 
 * will be enabled. 
 * 
 *      0 ..... OFF
 *      1 ..... ERROR
 *      2 ..... WARNING
 *      3 ..... INFO
 *      4 ..... DEBUG
 *      5 ..... TRACE
 * 
 * Example: LOGLEVEL 2 will enables error and warning log messages. 
 */
#define LOGLEVEL                1

#endif /* ifndef LOGLEVEL */

/**
 * @brief Used to define the format of log messages
 * 
 * Examples: 
 *  * Without timestamp and severity:
 *    MODULENAME ": " _fmt
 *  
 *  * Function and line number based logging:
 *    "%s(%d) " _fmt, __FUNCTION__, __LINE__
 * 
 *  * With severity and a bsp systick timestamp:
 *    "%010lu " _sev " " MODULENAME ": " _fmt, bspGetSysTick()
 *      
 * @param _sev  The character corresponding to the particular severity.
 *              Optional, might be omitted if not needed.
 * @param _fmt  The format sting og the log message. 
 *              Mandatory, log messages will not be displayed if omitted.
 */
#define LOGFORMAT(_sev, _fmt)                                               \
                                                                            \
        MODULENAME ": " _fmt
   
#ifdef __cplusplus
}
#endif

#endif /* LOGGING_CONFIG_H_ */
