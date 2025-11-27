#include <Arduino.h>
#include "serial_comm.h"
#include "bluetooth_comm.h"

#define PIN_LED_BUILTIN 2   

enum SystemState {
  STATE_IDLE,
  STATE_PROCESSING,
  STATE_SYNC_TRANSFER, // Nome atualizado
  STATE_PLAYER
};

SystemState currentState = STATE_IDLE;
SystemState nextStateAfterPlay = STATE_IDLE; 

// Monitoramento de Timeout
unsigned long lastActivityTime = 0;
int lastProgress = 0;
const unsigned long TIMEOUT_CHUNK = 2000; // 2s para completar um chunk de 64 bytes

bool hasPlayedStartupSound = false; 

void iniciarSequenciaDeAudio(const uint8_t* audioData, int audioLen, SystemState proximoEstado) {
    playBuffer(audioData, audioLen);
    nextStateAfterPlay = proximoEstado;
    currentState = STATE_PLAYER;
    digitalWrite(PIN_LED_BUILTIN, HIGH); 
}

void setup() {
  Serial.begin(115200); // USB continua rapida
  pinMode(PIN_LED_BUILTIN, OUTPUT);
  
  setupSerialComms(); 
  setupBluetooth(); 
  
  Serial.println("\n--- SISTEMA ESP32: STOP-AND-WAIT (38400) ---");
}

void loop() {
  loopBluetooth();

  switch (currentState) {
    
    // --- IDLE ---
    case STATE_IDLE:
      if (isBluetoothConnected()) {
          if (!hasPlayedStartupSound) {
              Serial.println(">>> [IDLE] Conectado! Boot...");
              iniciarSequenciaDeAudio(getTestData(), getTestDataLen(), STATE_PROCESSING);
              hasPlayedStartupSound = true;
          } else {
              currentState = STATE_PROCESSING;
          }
      }
      break;

    // --- PROCESSING ---
    case STATE_PROCESSING:
      Serial.println("\n>>> [SYNC] Iniciando Transferência em Blocos (64b)...");
      
      startSerialTransaction(getTestData(), getTestDataLen());
      
      lastActivityTime = millis();
      lastProgress = 0;
      
      currentState = STATE_SYNC_TRANSFER;
      break;

    // --- TRANSFERÊNCIA SINCRONIZADA ---
    case STATE_SYNC_TRANSFER:
      {// 1. Executa a máquina de estados Ping-Pong
      updateSerialTransaction();

      // 2. Verifica Progresso (Timeout Watchdog)
      int currentProgress = getBytesProcessed();
      
      if (currentProgress > lastProgress) {
          // Se houve progresso, reseta o timer
          lastActivityTime = millis();
          lastProgress = currentProgress;
          
          // Pisca LED rapidinho a cada chunk completado
          digitalWrite(PIN_LED_BUILTIN, !digitalRead(PIN_LED_BUILTIN));
      }

      // 3. Log Periódico (menos frequente para nao poluir)
      static unsigned long debugTimer = 0;
      if (millis() - debugTimer > 1000) {
          debugTimer = millis();
          Serial.printf("[SYNC] Progresso: %d / %d bytes (%d%%)\n", 
                        currentProgress, getTestDataLen(), 
                        (currentProgress * 100) / getTestDataLen());
      }

      // 4. Verifica Conclusão
      if (isTransactionComplete()) {
          Serial.println("\n>>> [SUCESSO] Todos pacotes confirmados!");
          delay(200); 
          iniciarSequenciaDeAudio(rxAudioBuffer, getBytesProcessed(), STATE_IDLE);
      }
      
      // 5. Verifica Erros
      else if (hasSerialError()) {
          Serial.println("\n[ERRO] Falha de Memória.");
          currentState = STATE_IDLE;
      }
      
      // 6. Timeout
      else if (millis() - lastActivityTime > TIMEOUT_CHUNK) {
          Serial.println("\n[ERRO] Timeout: O outro dispositivo parou de responder o echo.");
          Serial.printf("Travou em: %d bytes\n", currentProgress);
          currentState = STATE_IDLE;
      }
      break;
      }
    // --- PLAYER ---
    case STATE_PLAYER:
      if (isAudioPlaying()) {
          break; 
      }
      digitalWrite(PIN_LED_BUILTIN, LOW); 
      Serial.println("[PLAYER] Fim.");
      delay(1000); 
      currentState = nextStateAfterPlay;
      break;
  }
  delay(1); 
}