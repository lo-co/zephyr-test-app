/**
 * @file common.h
 * @brief Common utilities used here.
 * 
 * 
 */

#pragma once

#include <zephyr/logging/log.h>

// ANSI Coloring for logging
#define ANSI_COLOR_RED     "\033[1;31m"
#define ANSI_COLOR_GREEN   "\033[1;32m"
#define ANSI_COLOR_YELLOW  "\033[1;33m"
#define ANSI_COLOR_BLUE    "\033[1;34m"
#define ANSI_COLOR_MAGENTA "\033[1;35m"
#define ANSI_COLOR_CYAN    "\033[1;36m"
#define ANSI_COLOR_RESET   "\033[0m"


#define COMMON_LOG_INF(fmt, ...) \
    LOG_INF(ANSI_COLOR_GREEN fmt ANSI_COLOR_RESET, ##__VA_ARGS__)

#define COMMON_LOG_ERR(fmt, ...) \
    LOG_ERR(ANSI_COLOR_RED fmt ANSI_COLOR_RESET, ##__VA_ARGS__)

#define COMMON_LOG_DBG(fmt, ...) \
    LOG_DBG(ANSI_COLOR_BLUE fmt ANSI_COLOR_RESET, ##__VA_ARGS__)

#define COMMON_LOG_WRN(fmt, ...) \
    LOG_DBG(ANSI_COLOR_YELLOW fmt ANSI_COLOR_RESET, ##__VA_ARGS__)
