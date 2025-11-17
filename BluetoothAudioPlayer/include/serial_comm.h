#pragma once

/**
 * @brief Configura a comunicação serial (UART2)
 */
void setupSerialComms();

/**
 * @brief Executa o ciclo de enviar e receber dados
 * Esta função deve ser chamada no loop principal.
 */
void loopSerialComms();