#include <Arduino.h>
#include "serial_comm.h"
#include "test_data.h"

// --- Pinos para a UART2 ---
#define RXD2 16 
#define TXD2 17 

// Mantemos a função auxiliar igual, pois ela calcula a paridade DE UM BYTE
byte calculateEvenParity(byte val) {
  byte count = 0;
  for (int i = 0; i < 8; i++) {
    count ^= bitRead(val, i);
  }
  return count;
}

void printByteAsBinary(byte val) {
  for (int i = 7; i >= 0; i--) {
    Serial.print(bitRead(val, i));
  }
}

void setupSerialComms() {
  // Configuração: 8 bits de dados, Paridade Par (Even), 1 Stop Bit
  Serial2.begin(115200, SERIAL_8E1, RXD2, TXD2);

  Serial.println("--- UART Iniciada (8E1) ---");
  Serial.println("Pinos: RX=16, TX=17");
  Serial.println();
}

void loopSerialComms(bool verbose) {
  // 1. Enviar o ARRAY COMPLETO de uma vez via Hardware
  // O hardware vai pegar byte por byte, calcular a paridade de cada um e enviar.
  size_t bytes_sent = Serial2.write(test_data, test_data_len);

  Serial.print(">>> Pacote enviado! Total de bytes escritos: ");
  Serial.println(bytes_sent);


  if (verbose) {
      Serial.println("--- DETALHAMENTO DOS FRAMES ENVIADOS (Simulação Visual) ---");
    
      // 2. Loop para visualizar o que o Hardware fez com CADA byte
      // A paridade é per-byte, então precisamos iterar o array de dados.
      for (unsigned int i = 0; i < test_data_len; i++) {
        byte currentByte = test_data[i];
        byte parityBit = calculateEvenParity(currentByte);
    
        Serial.print("Byte [");
        Serial.print(i);
        Serial.print("]: ");
        printByteAsBinary(currentByte);
    
        Serial.print(" | Paridade (E): ");
        Serial.print(parityBit);
    
        // Visualização do Frame Físico
        Serial.print("  -> Frame: [S:0] [");
        printByteAsBinary(currentByte);
        Serial.print("] [P:");
        Serial.print(parityBit);
        Serial.println("] [E:1]");
      }
      Serial.println("-----------------------------------------------------------");
  }

  // 3. Recebimento dos Dados (Loop para ler múltiplos bytes)
  Serial.println("Aguardando chegada dos dados em RX...");
  
  // Aguarda até que chegue pelo menos 1 byte
  while (Serial2.available() == 0) {
    delay(10);
  }

  // Pequeno delay para garantir que o buffer encha se os bytes estiverem chegando rápido
  delay(100); 

  Serial.println("--- DADOS RECEBIDOS EM RX ---");
  int count_rx = 0;
  
  // Enquanto houver dados no buffer de recepção...
  while (Serial2.available() > 0) {
    byte byte_recebido = Serial2.read();
    
    Serial.print("RX Byte [");
    Serial.print(count_rx);
    Serial.print("]: ");
    printByteAsBinary(byte_recebido);
    Serial.println();
    
    count_rx++;
  }
  
  Serial.println("===========================================================\n");
  
  delay(500); 
}