#ifndef SERIAL_COMM_H
#define SERIAL_COMM_H

#include <Arduino.h>

// Mantemos o limite seguro de memória
#define MAX_RX_BUFFER_SIZE 75000 
// Tamanho do bloco para o protocolo Stop-and-Wait
#define SERIAL_CHUNK_SIZE 64

extern uint8_t rxAudioBuffer[MAX_RX_BUFFER_SIZE];
extern int rxAudioLen;

void setupSerialComms();

// Inicia a transação
void startSerialTransaction(const uint8_t* dataToSend, int totalLen);

// Executa o protocolo Ping-Pong (Envia 64, Espera 64)
void updateSerialTransaction();

// Verificadores
bool isTransactionComplete(); // Substitui isTx/isRx separados
int getBytesProcessed();      // Quantos bytes já foram confirmados (Ida e Volta)
bool hasSerialError();

#endif