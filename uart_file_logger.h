#pragma once

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

#define FILE_BUF_SIZE 8192
#define LOG_FILE_NAME "log.txt"

typedef enum {
    BUF_PART_FIRST_HALF,
    BUF_PART_SECOND_HALF
} UART_FileLoggerSyncPart;

typedef struct UART_FileLogger UART_FileLogger;

UART_FileLogger* uart_file_logger_create(void);
void uart_file_logger_free(UART_FileLogger* ptr);

void uart_file_logger_set_on_filled_callback(
    UART_FileLogger* logger,
    void (*func)(UART_FileLogger*, UART_FileLoggerSyncPart));

void uart_file_logger_set_context(UART_FileLogger* logger, void* context);
void* uart_file_logger_get_context(UART_FileLogger* logger);

void uart_file_logger_push(UART_FileLogger* logger, const void* buf, size_t sz);
void uart_file_logger_flush(UART_FileLogger* logger, UART_FileLoggerSyncPart part);
void uart_file_logger_flush_pending(UART_FileLogger* logger);

#ifdef __cplusplus
}
#endif
