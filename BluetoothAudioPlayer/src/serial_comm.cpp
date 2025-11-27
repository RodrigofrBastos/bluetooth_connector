#include <Arduino.h>
#include "serial_comm.h"

#define RXD2 16 
#define TXD2 17 

// Baud Rate reduzido para estabilidade
#define SERIAL_BAUD_RATE 38400

uint8_t rxAudioBuffer[MAX_RX_BUFFER_SIZE];
int rxAudioLen = 0; // Atua como cursor global de bytes JÁ confirmados

// Variáveis da Transação
const uint8_t* txDataPtr = nullptr;
int totalBytesToProcess = 0;
bool serialError = false;

// Variáveis do Protocolo Stop-and-Wait
bool waitingForEcho = false; // true = Enviou, esperando voltar
int currentChunkSize = 0;    // Tamanho do pacote atual (geralmente 64, mas pode ser menos no fim)
int chunkBytesReceived = 0;  // Quantos bytes deste pacote já voltaram

void setupSerialComms() {
  Serial2.begin(SERIAL_BAUD_RATE, SERIAL_8E1, RXD2, TXD2);
  
  // Buffers de hardware
  Serial2.setRxBufferSize(2048); 
  Serial2.setTxBufferSize(512); 
  
  Serial.println("--- UART Iniciada (Protocolo Stop-and-Wait) ---");
  Serial.printf("Baud Rate: %d | Chunk Size: %d\n", SERIAL_BAUD_RATE, SERIAL_CHUNK_SIZE);
  Serial.printf("Buffer RAM: %d bytes\n", MAX_RX_BUFFER_SIZE);
}

void startSerialTransaction(const uint8_t* dataToSend, int totalLen) {
    // Limpa qualquer lixo anterior
    while(Serial2.available()) Serial2.read();
    
    txDataPtr = dataToSend;
    
    // Clamp de Memória (Segurança)
    if (totalLen > MAX_RX_BUFFER_SIZE) {
        totalBytesToProcess = MAX_RX_BUFFER_SIZE;
        Serial.println("[AVISO] Audio cortado para caber na RAM.");
    } else {
        totalBytesToProcess = totalLen;
    }
    
    // Reseta estado
    rxAudioLen = 0;
    serialError = false;
    
    // Prepara para o primeiro chunk
    waitingForEcho = false;
    chunkBytesReceived = 0;
    
    Serial.printf("[SERIAL] Iniciando. Total: %d bytes.\n", totalBytesToProcess);
}

void updateSerialTransaction() {
    if (serialError || rxAudioLen >= totalBytesToProcess) return;

    // --- FASE 1: ENVIAR O CHUNK ---
    if (!waitingForEcho) {
        // Calcula quanto falta
        int remainingBytes = totalBytesToProcess - rxAudioLen;
        
        // Define o tamanho deste pacote (64 ou o resto que sobrar)
        currentChunkSize = (remainingBytes > SERIAL_CHUNK_SIZE) ? SERIAL_CHUNK_SIZE : remainingBytes;
        
        // Verifica se o hardware aguenta receber esses bytes
        if (Serial2.availableForWrite() >= currentChunkSize) {
            // Envia o bloco exato
            Serial2.write(&txDataPtr[rxAudioLen], currentChunkSize);
            
            // Muda estado para ESPERA
            waitingForEcho = true;
            chunkBytesReceived = 0;
            
            // (Opcional) Debug detalhado
            // Serial.printf("TX Chunk: %d bytes (Offset: %d)\n", currentChunkSize, rxAudioLen);
        }
    }

    // --- FASE 2: ESPERAR O RETORNO (ECHO) ---
    if (waitingForEcho) {
        // Lê tudo que chegar
        while (Serial2.available() > 0) {
            byte b = Serial2.read();
            
            // Grava no buffer global na posição correta
            // Posição = (Tudo que já foi processado antes) + (O que chegou agora deste chunk)
            int writePos = rxAudioLen + chunkBytesReceived;
            
            if (writePos < MAX_RX_BUFFER_SIZE) {
                rxAudioBuffer[writePos] = b;
                chunkBytesReceived++;
            } else {
                serialError = true; // Overflow
            }

            // Verifica se completou este chunk específico
            if (chunkBytesReceived == currentChunkSize) {
                // SUCESSO DO CHUNK!
                // Atualiza o contador global
                rxAudioLen += currentChunkSize;
                
                // Libera para enviar o próximo
                waitingForEcho = false;
                
                // Sai do loop de leitura para dar chance de enviar no próximo ciclo
                return; 
            }
        }
    }
}

bool isTransactionComplete() {
    return rxAudioLen >= totalBytesToProcess;
}

int getBytesProcessed() {
    return rxAudioLen;
}

bool hasSerialError() {
    return serialError;
}