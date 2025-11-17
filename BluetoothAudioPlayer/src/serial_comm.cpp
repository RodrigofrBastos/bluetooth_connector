
#include "serial_comm.h"
#include <Arduino.h>

// --- Pinos para a UART2 (Hardware Serial 2) ---
// Você pode mudar para quaisquer pinos livres
#define RXD2 16 // Pino RX
#define TXD2 17 // Pino TX

/**
 * @brief Função auxiliar para printar um byte no Monitor Serial
 * em formato binário (bits) com zeros à esquerda.
 * @param val O byte a ser printado.
 */
void printByteAsBinary(byte val) {
  // Itera por todos os 8 bits, do mais significativo (7) 
  // ao menos significativo (0)
  for (int i = 7; i >= 0; i--) {
    // bitRead() lê um bit específico de um byte
    Serial.print(bitRead(val, i));
  }
}


void setupSerialComms() {
  // Inicia a Serial2 nos pinos definidos (RXD2, TXD2)
  // 9600 é a taxa de baudrate (velocidade)
  Serial2.begin(9600, SERIAL_8N1, RXD2, TXD2); 
  
  Serial.println("Comunicação Serial (Serial2) iniciada.");
  Serial.println(">>> Conecte um jumper entre os pinos 16 e 17 para este teste <<<");
  Serial.println();
}


void loopSerialComms() {
  // Vamos definir a "sequência de bits" que queremos enviar.
  // Em C++, 0b... define um número binário.
  // Esta é uma sequência de 8 bits (1 byte).
  byte byte_para_enviar = 0b10110010; // (Hex: 0xB2)

  
  // 1. Enviar a sequencia de bits via TX
  Serial2.write(byte_para_enviar);

  // 2. Printar a sequencia de bits no terminal (Monitor Serial)
  Serial.print("Enviando via TX (Pino 17): ");
  printByteAsBinary(byte_para_enviar);
  Serial.println();

  // 3. Esperar receber uma sequencia de bits vinda de RX
  Serial.println("Aguardando dados de RX (Pino 16)...");

  // O loop 'while' bloqueia o código até que a Serial2 
  // (o pino RX) receba algum dado.
  while (Serial2.available() == 0) {
    delay(10); // Espera ativa
  }

  // Assim que houver dados, leia o byte que chegou
  byte byte_recebido = Serial2.read();

  // 4. Printar os bits recebidos de RX
  Serial.print("Recebido de RX (Pino 16):   ");
  printByteAsBinary(byte_recebido);
  Serial.println();
  Serial.println("---------------------------------");
  
  delay(3000); // Espera 3 segundos antes de repetir o processo
}