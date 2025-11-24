#ifndef SERIAL_COMM_H
#define SERIAL_COMM_H

#include <msp430.h>
#include <stdint.h>

// --- Configurações ---
#define RX_BUFFER_SIZE 128      // Tamanho do buffer circular
#define PACKET_SIZE    12       // Tamanho do pacote para disparar o processamento (ex: 12 bytes do seu teste)

// --- Definição da Máquina de Estados ---
typedef enum {
    STATE_IDLE,
    STATE_RECEIVING,
    STATE_PROCESSING,
    STATE_SENDING,
    STATE_SENT
} state_t;

// --- Declaração de Variáveis Globais ---
// Usamos 'extern' para que a main possa ver estas funções/variáveis
void uart_init(void);
void uart_send_char(char c);
uint8_t uart_available(void);
int uart_read_char(uint8_t *data);
void uart_flush(void); // Função para limpar buffer se necessário

#endif