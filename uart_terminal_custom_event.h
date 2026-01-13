#pragma once

typedef enum {
    UART_TerminalEventRefreshConsoleOutput = 0,
    UART_TerminalEventStartConsole,
    UART_TerminalEventStartKeyboard,
    UART_TerminalEventFlushFirstHalf,
    UART_TerminalEventFlushSecondHalf,
    UART_TerminalEventSyncSettings
} UART_TerminalCustomEvent;
