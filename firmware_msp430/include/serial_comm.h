#ifndef SERIAL_COMM_H
#define SERIAL_COMM_H

#include <msp430.h>
#include <stdint.h>

// --- Configurações ---
// Mantemos 128 bytes. Potência de 2 facilita o cálculo de "wrap around"
#define RX_BUFFER_SIZE 128      

// --- Declaração de Funções Públicas ---
void uart_init(void);

// Envia um byte (bloqueante se TX estiver ocupado)
void uart_send_char(uint8_t c);

// Retorna quantos bytes estão esperando no buffer
uint16_t uart_available(void);

// Tenta ler 1 byte. Retorna 1 se leu com sucesso, 0 se buffer vazio.
// O byte lido é salvo no ponteiro *data
int uart_read_char(uint8_t *data);

#endif