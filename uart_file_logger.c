#include "uart_file_logger.h"

#include <furi.h>
#include <storage/storage.h>

#define TAG "UART_FILE_LOGGER"

struct UART_FileLogger {
    uint8_t buf[FILE_BUF_SIZE];
    volatile size_t write_position;
    volatile size_t flush_position;
    volatile bool half_locked[2];

    void (*on_filled)(UART_FileLogger*, UART_FileLoggerSyncPart);

    Storage* storage;
    File* file;
    void* context;
};

UART_FileLogger* uart_file_logger_create(void) {
    UART_FileLogger* logger = (UART_FileLogger*)malloc(sizeof(UART_FileLogger));

    logger->write_position = 0;
    logger->flush_position = 0;
    logger->half_locked[0] = false;
    logger->half_locked[1] = false;
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
        int half = ((logger->write_position % FILE_BUF_SIZE) < FILE_BUF_SIZE / 2) ? 0 : 1;
        if(logger->half_locked[half]) {
            break;
        }

        if(logger->write_position == FILE_BUF_SIZE) {
            logger->write_position = 0;
        }

        logger->buf[logger->write_position] = *p;
        logger->write_position++;
        p++;

        if(logger->write_position == FILE_BUF_SIZE / 2) {
            logger->half_locked[0] = true;
            if(logger->on_filled != NULL) {
                logger->on_filled(logger, BUF_PART_FIRST_HALF);
            }
        } else if(logger->write_position == FILE_BUF_SIZE) {
            logger->half_locked[1] = true;
            if(logger->on_filled != NULL) {
                logger->on_filled(logger, BUF_PART_SECOND_HALF);
            }
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
            logger->half_locked[0] = false;
            logger->flush_position = FILE_BUF_SIZE / 2;
        } else if(part == BUF_PART_SECOND_HALF) {
            logger->half_locked[1] = false;
            logger->flush_position = 0;
        }
    }
}

void uart_file_logger_flush_pending(UART_FileLogger* logger) {
    size_t cur_write_pos = logger->write_position;
    if(logger->flush_position == 0 || cur_write_pos > logger->flush_position) {
        storage_file_write(
            logger->file,
            &logger->buf[logger->flush_position],
            cur_write_pos - logger->flush_position);
    } else {
        storage_file_write(
            logger->file, &logger->buf[FILE_BUF_SIZE / 2], FILE_BUF_SIZE - FILE_BUF_SIZE / 2);
        storage_file_write(logger->file, &logger->buf[0], logger->write_position);
    }

    logger->write_position = 0;
    logger->flush_position = 0;
    logger->half_locked[0] = false;
    logger->half_locked[1] = false;
}
