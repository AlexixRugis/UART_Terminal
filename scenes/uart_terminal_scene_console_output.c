#include "../uart_terminal_app_i.h"

static void sync_baud_rate(UART_TerminalApp* app) {
    if(0 == strncmp("75", app->selected_tx_string, strlen("75")) && app->BAUDRATE != 75) {
        app->BAUDRATE = 75;
        app->need_settings_sync = true;
    }
    if(0 == strncmp("110", app->selected_tx_string, strlen("110")) && app->BAUDRATE != 110) {
        app->BAUDRATE = 110;
        app->need_settings_sync = true;
    }
    if(0 == strncmp("150", app->selected_tx_string, strlen("150")) && app->BAUDRATE != 150) {
        app->BAUDRATE = 150;
        app->need_settings_sync = true;
    }
    if(0 == strncmp("300", app->selected_tx_string, strlen("300")) && app->BAUDRATE != 300) {
        app->BAUDRATE = 300;
        app->need_settings_sync = true;
    }
    if(0 == strncmp("600", app->selected_tx_string, strlen("600")) && app->BAUDRATE != 600) {
        app->BAUDRATE = 600;
        app->need_settings_sync = true;
    }
    if(0 == strncmp("1200", app->selected_tx_string, strlen("1200")) && app->BAUDRATE != 1200) {
        app->BAUDRATE = 1200;
        app->need_settings_sync = true;
    }
    if(0 == strncmp("1800", app->selected_tx_string, strlen("1800")) && app->BAUDRATE != 1800) {
        app->BAUDRATE = 1800;
        app->need_settings_sync = true;
    }
    if(0 == strncmp("2400", app->selected_tx_string, strlen("2400")) && app->BAUDRATE != 2400) {
        app->BAUDRATE = 2400;
        app->need_settings_sync = true;
    }
    if(0 == strncmp("4800", app->selected_tx_string, strlen("4800")) && app->BAUDRATE != 4800) {
        app->BAUDRATE = 4800;
        app->need_settings_sync = true;
    }
    if(0 == strncmp("7200", app->selected_tx_string, strlen("7200")) && app->BAUDRATE != 7200) {
        app->BAUDRATE = 7200;
        app->need_settings_sync = true;
    }
    if(0 == strncmp("9600", app->selected_tx_string, strlen("9600")) && app->BAUDRATE != 9600) {
        app->BAUDRATE = 9600;
        app->need_settings_sync = true;
    }
    if(0 == strncmp("14400", app->selected_tx_string, strlen("14400")) && app->BAUDRATE != 14400) {
        app->BAUDRATE = 14400;
        app->need_settings_sync = true;
    }
    if(0 == strncmp("19200", app->selected_tx_string, strlen("19200")) && app->BAUDRATE != 19200) {
        app->BAUDRATE = 19200;
        app->need_settings_sync = true;
    }
    if(0 == strncmp("38400", app->selected_tx_string, strlen("38400")) && app->BAUDRATE != 38400) {
        app->BAUDRATE = 38400;
        app->need_settings_sync = true;
    }
    if(0 == strncmp("56000", app->selected_tx_string, strlen("56000")) && app->BAUDRATE != 56000) {
        app->BAUDRATE = 56000;
        app->need_settings_sync = true;
    }
    if(0 == strncmp("57600", app->selected_tx_string, strlen("57600")) && app->BAUDRATE != 57600) {
        app->BAUDRATE = 57600;
        app->need_settings_sync = true;
    }
    if(0 == strncmp("76800", app->selected_tx_string, strlen("76800")) && app->BAUDRATE != 76800) {
        app->BAUDRATE = 76800;
        app->need_settings_sync = true;
    }
    if(0 == strncmp("115200", app->selected_tx_string, strlen("115200")) &&
       app->BAUDRATE != 115200) {
        app->BAUDRATE = 115200;
        app->need_settings_sync = true;
    }
    if(0 == strncmp("128000", app->selected_tx_string, strlen("128000")) &&
       app->BAUDRATE != 128000) {
        app->BAUDRATE = 128000;
        app->need_settings_sync = true;
    }
    if(0 == strncmp("230400", app->selected_tx_string, strlen("230400")) &&
       app->BAUDRATE != 230400) {
        app->BAUDRATE = 230400;
        app->need_settings_sync = true;
    }
    if(0 == strncmp("250000", app->selected_tx_string, strlen("250000")) &&
       app->BAUDRATE != 250000) {
        app->BAUDRATE = 250000;
        app->need_settings_sync = true;
    }
    if(0 == strncmp("256000", app->selected_tx_string, strlen("256000")) &&
       app->BAUDRATE != 256000) {
        app->BAUDRATE = 256000;
        app->need_settings_sync = true;
    }
    if(0 == strncmp("460800", app->selected_tx_string, strlen("460800")) &&
       app->BAUDRATE != 460800) {
        app->BAUDRATE = 460800;
        app->need_settings_sync = true;
    }
    if(0 == strncmp("921600", app->selected_tx_string, strlen("921600")) &&
       app->BAUDRATE != 921600) {
        app->BAUDRATE = 921600;
        app->need_settings_sync = true;
    }
}

void uart_terminal_console_output_handle_rx_data_cb(uint8_t* buf, size_t len, void* context) {
    furi_assert(context);
    UART_TerminalApp* app = context;

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

            furi_string_cat_printf(
                app->text_box_store,
                "%02u:%02u:%02u - ",
                datetime.hour,
                datetime.minute,
                datetime.second);

            app->console_at_line_start = false;
        }

        furi_string_push_back(app->text_box_store, ch);

        if(ch == '\n') {
            app->console_at_line_start = true;
        }
    }

    view_dispatcher_send_custom_event(
        app->view_dispatcher, UART_TerminalEventRefreshConsoleOutput);
}

void uart_terminal_scene_console_output_on_enter(void* context) {
    UART_TerminalApp* app = context;

    TextBox* text_box = app->text_box;
    text_box_reset(app->text_box);
    text_box_set_font(text_box, TextBoxFontText);
    if(app->focus_console_start) {
        text_box_set_focus(text_box, TextBoxFocusStart);
    } else {
        text_box_set_focus(text_box, TextBoxFocusEnd);
    }

    //Change baudrate ///////////////////////////////////////////////////////////////////////////
    sync_baud_rate(app);
    /////////////////////////////////////////////////////////////////////////////////////////////

    if(app->action_type == ACTION_INFO) {
        const char* help_msg =
            "UART terminal for Flipper\nby AlexixRugis\nBased on: cool4uma\nPress BACK to return\n";
        text_box_set_text(app->text_box, help_msg);
    } else {
        text_box_set_text(app->text_box, furi_string_get_cstr(app->text_box_store));
    }

    // Set starting text - for "View Log", this will just be what was already in the text box store

    scene_manager_set_scene_state(app->scene_manager, UART_TerminalSceneConsoleOutput, 0);
    view_dispatcher_switch_to_view(app->view_dispatcher, UART_TerminalAppViewConsoleOutput);

    // Register callback to receive data
    app->console_at_line_start = true;

    uart_terminal_uart_set_handle_rx_data_cb(
        app->uart, uart_terminal_console_output_handle_rx_data_cb); // setup callback for rx thread

    // Send command with CR+LF or newline '\n'
    if((app->action_type == ACTION_CMD || app->action_type == ACTION_INPUT_CMD) &&
       app->selected_tx_string) {
        if(app->TERMINAL_MODE == 1) {
            uart_terminal_uart_tx(
                app->uart, (uint8_t*)(app->selected_tx_string), strlen(app->selected_tx_string));
            uart_terminal_uart_tx(app->uart, (uint8_t*)("\r\n"), 2);
        } else {
            uart_terminal_uart_tx(
                app->uart, (uint8_t*)(app->selected_tx_string), strlen(app->selected_tx_string));
            uart_terminal_uart_tx(app->uart, (uint8_t*)("\n"), 1);
        }
    }
}

bool uart_terminal_scene_console_output_on_event(void* context, SceneManagerEvent event) {
    UART_TerminalApp* app = context;

    bool consumed = false;

    if(event.type == SceneManagerEventTypeCustom) {
        if(event.event == UART_TerminalEventRefreshConsoleOutput) {
            text_box_set_text(app->text_box, furi_string_get_cstr(app->text_box_store));
        }
        consumed = true;
    } else if(event.type == SceneManagerEventTypeTick) {
        if(app->need_settings_sync) {
            view_dispatcher_send_custom_event(
                app->view_dispatcher, UART_TerminalEventSyncSettings);
            app->need_settings_sync = false;
        }

        consumed = true;
    }

    return consumed;
}

void uart_terminal_scene_console_output_on_exit(void* context) {
    UART_TerminalApp* app = context;

    // Unregister rx callback
    uart_terminal_uart_set_handle_rx_data_cb(app->uart, NULL);
}
