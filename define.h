#ifndef __DEFINE_H__
#define __DEFINE_H__

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <errno.h>
#include <semaphore.h>
#include <pthread.h>

#define printr(fmt, ...) printf("[\x1b[31m%s\x1b[0m]" fmt "\n", __func__, ##__VA_ARGS__)
#define printg(fmt, ...) printf("[\x1b[32m%s\x1b[0m]" fmt "\n", __func__, ##__VA_ARGS__)
#define printw(fmt, ...) printf("[\x1b[33m%s\x1b[0m]" fmt "\n", __func__, ##__VA_ARGS__)
#define printd(fmt, ...) printf("[\x1b[33mDBG\x1b[0m]" fmt "\n", ##__VA_ARGS__)
#define printd(fmt, ...) printf("[\x1b[33mDBG\x1b[0m]" fmt "\n", ##__VA_ARGS__)
#define print_cordinate(row, column, fmt, ...) printf("\033[" row ";" column "H" "[\x1b[33mDBG\x1b[0m]" fmt "\n", ##__VA_ARGS__)
#define print_clear printf("\033[2J\033[1;1H")

typedef enum
{
    FLAG_OFF,
    FLAG_ON,
}COMMON_FLAGS;

#endif
