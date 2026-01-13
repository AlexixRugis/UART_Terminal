#include "../uart_terminal_app_i.h"

void uart_terminal_scene_console_output_on_enter(void* context) {
    UART_TerminalApp* app = context;

    app->is_in_console_view = true;

    TextBox* text_box = app->text_box;
    text_box_reset(app->text_box);
    text_box_set_font(text_box, TextBoxFontText);
    if(app->focus_console_start) {
        text_box_set_focus(text_box, TextBoxFocusStart);
    } else {
        text_box_set_focus(text_box, TextBoxFocusEnd);
    }

    if(app->action_type == ACTION_INFO) {
        const char* help_msg =
            "UART terminal for Flipper\nby AlexixRugis\nBased on: cool4uma\nPress BACK to return\n";
        text_box_set_text(app->text_box, help_msg);
    } else {
        text_box_set_text(app->text_box, furi_string_get_cstr(app->text_box_store));
    }

    scene_manager_set_scene_state(app->scene_manager, UART_TerminalSceneConsoleOutput, 0);
    view_dispatcher_switch_to_view(app->view_dispatcher, UART_TerminalAppViewConsoleOutput);

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
        consumed = true;
    }

    return consumed;
}

void uart_terminal_scene_console_output_on_exit(void* context) {
    UART_TerminalApp* app = context;
    app->is_in_console_view = false;
}
