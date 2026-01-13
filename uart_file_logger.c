#include "uart_file_logger.h"

#include <furi.h>
#include <storage/storage.h>

#define TAG "UART_FILE_LOGGER"

struct UART_FileLogger {
    uint8_t buf[FILE_BUF_SIZE];
    size_t write_position;
    size_t flush_position;

    void (*on_filled)(UART_FileLogger*, UART_FileLoggerSyncPart);

    Storage* storage;
    File* file;
    void* context;
};

UART_FileLogger* uart_file_logger_create(void) {
    UART_FileLogger* logger = (UART_FileLogger*)malloc(sizeof(UART_FileLogger));

    logger->write_position = 0;
    logger->flush_position = 0;
    logger->on_filled = NULL;
    logger->context = NULL;

    logger->storage = furi_record_open(RECORD_STORAGE);
    logger->file = storage_file_alloc(logger->storage);

    if(!storage_file_open(
           logger->file, APP_DATA_PATH(LOG_FILE_NAME), FSAM_WRITE, FSOM_OPEN_APPEND)) {
        FURI_LOG_E(TAG, "Failed to open file");
    }

    return logger;
}

void uart_file_logger_free(UART_FileLogger* ptr) {
    storage_file_close(ptr->file);
    storage_file_free(ptr->file);
    furi_record_close(RECORD_STORAGE);

    free(ptr);
}

void uart_file_logger_set_context(UART_FileLogger* logger, void* context) {
    logger->context = context;
}

void* uart_file_logger_get_context(UART_FileLogger* logger) {
    return logger->context;
}

void uart_file_logger_set_on_filled_callback(
    UART_FileLogger* logger,
    void (*func)(UART_FileLogger*, UART_FileLoggerSyncPart)) {
    logger->on_filled = func;
}

void uart_file_logger_push(UART_FileLogger* logger, const void* buf, size_t sz) {
    const uint8_t* p = buf;
    while(sz--) {
        logger->buf[logger->write_position] = *p;
        logger->write_position++;
        p++;

        if(logger->write_position == FILE_BUF_SIZE / 2) {
            if(logger->on_filled != NULL) {
                logger->on_filled(logger, BUF_PART_FIRST_HALF);
            }
        } else if(logger->write_position == FILE_BUF_SIZE) {
            if(logger->on_filled != NULL) {
                logger->on_filled(logger, BUF_PART_SECOND_HALF);
            }
            logger->write_position = 0;
        }
    }
}

void uart_file_logger_flush(UART_FileLogger* logger, UART_FileLoggerSyncPart part) {
    void* buf = NULL;
    size_t sz = 0;

    if(part == BUF_PART_FIRST_HALF) {
        buf = (void*)&logger->buf[0];
        sz = FILE_BUF_SIZE / 2;
    } else if(part == BUF_PART_SECOND_HALF) {
        buf = (void*)&logger->buf[FILE_BUF_SIZE / 2];
        sz = FILE_BUF_SIZE - FILE_BUF_SIZE / 2;
    }

    furi_assert(buf != NULL);

    if(!storage_file_write(logger->file, buf, sz)) {
        FURI_LOG_E(TAG, "Failed to write to file");
    } else {
        if(part == BUF_PART_FIRST_HALF) {
            logger->flush_position = FILE_BUF_SIZE / 2;
        } else if(part == BUF_PART_SECOND_HALF) {
            logger->flush_position = 0;
        }
    }
}

void uart_file_logger_flush_pending(UART_FileLogger* logger) {
    furi_assert(logger->flush_position <= logger->write_position);

    void* buf = (void*)&logger->buf[logger->flush_position];
    size_t sz = logger->write_position - logger->flush_position;

    if(sz > 0) {
        if(!storage_file_write(logger->file, buf, sz)) {
            FURI_LOG_E(TAG, "Failed to write to file");
        } else {
            logger->flush_position = 0;
            logger->write_position = 0;
        }
    }
}
