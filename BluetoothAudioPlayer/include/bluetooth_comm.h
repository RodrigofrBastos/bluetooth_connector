#pragma once

/**
 * @brief Configura e inicia o serviço Bluetooth A2DP
 */
void setupBluetooth();

/**
 * @brief Tarefas de loop do Bluetooth (ex: exibir status)
 * Esta função deve ser chamada repetidamente no loop principal.
 */
void loopBluetooth();