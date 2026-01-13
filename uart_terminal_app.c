#include "uart_terminal_app_i.h"

#include <furi.h>
#include <furi_hal.h>

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
    // TODO: reopen port/create or delete logger

    uint32_t cur_baudrate = uart_terminal_uart_get_br(app->uart);
    if(app->BAUDRATE != cur_baudrate) {
        uart_terminal_uart_free(app->uart);
        app->uart = uart_terminal_uart_init(app);
    }

    if(app->log_to_file && !app->file_logger) {
        app->file_logger = uart_file_logger_create();
        uart_file_logger_set_context(app->file_logger, app);
        uart_file_logger_set_on_filled_callback(
            app->file_logger, uart_terminal_app_logger_callback);
    } else if(!app->log_to_file && !app->file_logger) {
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
    } else if(event == UART_TerminalEventSyncSettings) {
        uart_terminal_app_sync_settings(app);
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
    scene_manager_handle_tick_event(app->scene_manager);
}

UART_TerminalApp* uart_terminal_app_alloc() {
    UART_TerminalApp* app = malloc(sizeof(UART_TerminalApp));

    app->gui = furi_record_open(RECORD_GUI);

    app->view_dispatcher = view_dispatcher_alloc();
    app->scene_manager = scene_manager_alloc(&uart_terminal_scene_handlers, app);
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
    furi_string_reserve(app->text_box_store, UART_TERMINAL_TEXT_BOX_STORE_SIZE);

    app->text_input = uart_text_input_alloc();
    view_dispatcher_add_view(
        app->view_dispatcher,
        UART_TerminalAppViewTextInput,
        uart_text_input_get_view(app->text_input));

    scene_manager_next_scene(app->scene_manager, UART_TerminalSceneStart);

    app->uart = uart_terminal_uart_init(app);

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
