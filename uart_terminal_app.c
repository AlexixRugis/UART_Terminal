#include "uart_terminal_app_i.h"

#include <furi.h>
#include <furi_hal.h>

static void
    uart_terminal_app_make_time_str(char* buf, uint8_t hour, uint8_t minute, uint8_t second) {
    buf[0] = '0' + hour / 10;
    buf[1] = '0' + hour % 10;
    buf[2] = ':';
    buf[3] = '0' + minute / 10;
    buf[4] = '0' + minute % 10;
    buf[5] = ':';
    buf[6] = '0' + second / 10;
    buf[7] = '0' + second % 10;
    buf[8] = ' ';
    buf[9] = '-';
    buf[10] = ' ';
    buf[11] = '\0';
}

static void uart_terminal_app_handle_rx_data_cb(uint8_t* buf, size_t len, void* context) {
    furi_assert(context);
    UART_TerminalApp* app = context;

    if(app->log_to_file && app->file_logger) {
        uart_file_logger_push(app->file_logger, buf, len);
    }

    // If text box store gets too big, then truncate it
    app->text_box_store_strlen += len;
    if(app->text_box_store_strlen >= UART_TERMINAL_TEXT_BOX_STORE_SIZE - 1) {
        furi_string_right(app->text_box_store, app->text_box_store_strlen / 2);
        app->text_box_store_strlen = furi_string_size(app->text_box_store) + len;
    }

    for(size_t i = 0; i < len; i++) {
        char ch = buf[i];

        if(app->console_at_line_start && app->show_time) {
            // If text box store gets too big, then truncate it
            app->text_box_store_strlen += 11;
            if(app->text_box_store_strlen >= UART_TERMINAL_TEXT_BOX_STORE_SIZE - 1) {
                furi_string_right(app->text_box_store, app->text_box_store_strlen / 2);
                app->text_box_store_strlen = furi_string_size(app->text_box_store) + 11;
            }

            DateTime datetime;
            furi_hal_rtc_get_datetime(&datetime);

            char time_buf[12];
            uart_terminal_app_make_time_str(
                time_buf, datetime.hour, datetime.minute, datetime.second);

            furi_string_cat_str(app->text_box_store, time_buf);

            if(app->log_to_file && app->file_logger) {
                uart_file_logger_push(app->file_logger, time_buf, 11);
            }

            app->console_at_line_start = false;
        }

        furi_string_push_back(app->text_box_store, ch);

        if(app->log_to_file && app->file_logger) {
            uart_file_logger_push(app->file_logger, &ch, 1);
        }

        if(ch == '\n') {
            app->console_at_line_start = true;
        }
    }

    if(app->is_in_console_view && app->action_type != ACTION_INFO) {
        view_dispatcher_send_custom_event(
            app->view_dispatcher, UART_TerminalEventRefreshConsoleOutput);
    }
}

static void
    uart_terminal_app_logger_callback(UART_FileLogger* logger, UART_FileLoggerSyncPart part) {
    UART_TerminalApp* app = (UART_TerminalApp*)uart_file_logger_get_context(logger);
    furi_assert(app);

    if(part == BUF_PART_FIRST_HALF) {
        view_dispatcher_send_custom_event(app->view_dispatcher, UART_TerminalEventFlushFirstHalf);
    } else if(part == BUF_PART_SECOND_HALF) {
        view_dispatcher_send_custom_event(app->view_dispatcher, UART_TerminalEventFlushSecondHalf);
    }
}

static void uart_terminal_app_sync_settings(UART_TerminalApp* app) {
    uint32_t cur_baudrate = uart_terminal_uart_get_br(app->uart);
    FuriHalSerialId cur_serial_id = uart_terminal_uart_get_serial_id(app->uart);
    if(app->BAUDRATE != cur_baudrate || app->serial_id != cur_serial_id) {
        uart_terminal_uart_free(app->uart);
        app->uart = uart_terminal_uart_init(app);
        uart_terminal_uart_set_handle_rx_data_cb(app->uart, uart_terminal_app_handle_rx_data_cb);
    }

    if(app->log_to_file && !app->file_logger) {
        app->file_logger = uart_file_logger_create();
        uart_file_logger_set_context(app->file_logger, app);
        uart_file_logger_set_on_filled_callback(
            app->file_logger, uart_terminal_app_logger_callback);
    } else if(!app->log_to_file && app->file_logger) {
        uart_file_logger_flush_pending(app->file_logger);
        uart_file_logger_free(app->file_logger);
        app->file_logger = NULL;
    }
}

static bool uart_terminal_app_custom_event_callback(void* context, uint32_t event) {
    furi_assert(context);
    UART_TerminalApp* app = context;

    if(event == UART_TerminalEventFlushFirstHalf) {
        if(app->file_logger) uart_file_logger_flush(app->file_logger, BUF_PART_FIRST_HALF);
    } else if(event == UART_TerminalEventFlushSecondHalf) {
        if(app->file_logger) uart_file_logger_flush(app->file_logger, BUF_PART_SECOND_HALF);
    }

    return scene_manager_handle_custom_event(app->scene_manager, event);
}

static bool uart_terminal_app_back_event_callback(void* context) {
    furi_assert(context);
    UART_TerminalApp* app = context;
    return scene_manager_handle_back_event(app->scene_manager);
}

static void uart_terminal_app_tick_event_callback(void* context) {
    furi_assert(context);
    UART_TerminalApp* app = context;

    if(app->need_settings_sync) {
        uart_terminal_app_sync_settings(app);
        app->need_settings_sync = false;
    }

    scene_manager_handle_tick_event(app->scene_manager);
}

UART_TerminalApp* uart_terminal_app_alloc() {
    UART_TerminalApp* app = malloc(sizeof(UART_TerminalApp));

    app->gui = furi_record_open(RECORD_GUI);

    app->view_dispatcher = view_dispatcher_alloc();
    app->scene_manager = scene_manager_alloc(&uart_terminal_scene_handlers, app);
    app->need_settings_sync = false;
    view_dispatcher_enable_queue(app->view_dispatcher);
    view_dispatcher_set_event_callback_context(app->view_dispatcher, app);

    view_dispatcher_set_custom_event_callback(
        app->view_dispatcher, uart_terminal_app_custom_event_callback);
    view_dispatcher_set_navigation_event_callback(
        app->view_dispatcher, uart_terminal_app_back_event_callback);
    view_dispatcher_set_tick_event_callback(
        app->view_dispatcher, uart_terminal_app_tick_event_callback, 100);

    view_dispatcher_attach_to_gui(app->view_dispatcher, app->gui, ViewDispatcherTypeFullscreen);

    app->var_item_list = variable_item_list_alloc();
    view_dispatcher_add_view(
        app->view_dispatcher,
        UART_TerminalAppViewVarItemList,
        variable_item_list_get_view(app->var_item_list));

    for(int i = 0; i < NUM_MENU_ITEMS; ++i) {
        app->selected_option_index[i] = 0;
    }

    app->text_box = text_box_alloc();
    view_dispatcher_add_view(
        app->view_dispatcher, UART_TerminalAppViewConsoleOutput, text_box_get_view(app->text_box));
    app->text_box_store = furi_string_alloc();
    app->text_box_store_strlen = 0;
    furi_string_reserve(app->text_box_store, UART_TERMINAL_TEXT_BOX_STORE_SIZE);
    app->is_in_console_view = false;
    app->console_at_line_start = true;

    app->text_input = uart_text_input_alloc();
    view_dispatcher_add_view(
        app->view_dispatcher,
        UART_TerminalAppViewTextInput,
        uart_text_input_get_view(app->text_input));

    scene_manager_next_scene(app->scene_manager, UART_TerminalSceneStart);

    app->BAUDRATE = 115200;
    app->serial_id = FuriHalSerialIdLpuart;
    app->uart = uart_terminal_uart_init(app);
    uart_terminal_uart_set_handle_rx_data_cb(app->uart, uart_terminal_app_handle_rx_data_cb);

    return app;
}

void uart_terminal_app_free(UART_TerminalApp* app) {
    furi_assert(app);

    // Views
    view_dispatcher_remove_view(app->view_dispatcher, UART_TerminalAppViewVarItemList);
    view_dispatcher_remove_view(app->view_dispatcher, UART_TerminalAppViewConsoleOutput);
    view_dispatcher_remove_view(app->view_dispatcher, UART_TerminalAppViewTextInput);
    text_box_free(app->text_box);
    furi_string_free(app->text_box_store);
    uart_text_input_free(app->text_input);

    // View dispatcher
    view_dispatcher_free(app->view_dispatcher);
    scene_manager_free(app->scene_manager);

    uart_terminal_uart_free(app->uart);

    if(app->file_logger) {
        uart_file_logger_flush_pending(app->file_logger);
        uart_file_logger_free(app->file_logger);
        app->file_logger = NULL;
    }

    // Close records
    furi_record_close(RECORD_GUI);

    free(app);
}

int32_t uart_terminal_app(void* p) {
    UNUSED(p);
    UART_TerminalApp* uart_terminal_app = uart_terminal_app_alloc();

    view_dispatcher_run(uart_terminal_app->view_dispatcher);

    uart_terminal_app_free(uart_terminal_app);

    return 0;
}
