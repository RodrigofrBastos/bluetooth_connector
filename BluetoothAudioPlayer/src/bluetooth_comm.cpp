#include "bluetooth_comm.h" // Inclui o "menu"
#include <Arduino.h>
#include "BluetoothA2DPSource.h"
#include "audio_data.h"       // Inclui os dados do áudio

// --- Variáveis Globais Privadas deste Módulo ---
BluetoothA2DPSource a2dp_source;
const char* NOME_DO_FONE = "QCY H3"; 
volatile int posicao_audio = 0; 

// --- Callbacks Internos (Não precisam estar no .h) ---

void connection_state_changed(esp_a2d_connection_state_t state, void *ptr){
  Serial.print("[Bluetooth] Estado mudou para: ");
  if (state == ESP_A2D_CONNECTION_STATE_DISCONNECTED) {
      Serial.println("Desconectado ❌");
  } else if (state == ESP_A2D_CONNECTION_STATE_CONNECTING) {
      Serial.println("Conectando... ⏳");
  } else if (state == ESP_A2D_CONNECTION_STATE_CONNECTED) {
      Serial.println("CONECTADO! ✅ Tocando áudio...");
  } else if (state == ESP_A2D_CONNECTION_STATE_DISCONNECTING) {
      Serial.println("Desconectando...");
  }
}

int32_t get_sound_data(Frame *data, int32_t len) {
    int frames_para_copiar = len;
    
    for (int i = 0; i < frames_para_copiar; i++) {
        if (posicao_audio >= audio_teste_raw_len) { 
            posicao_audio = 0; 
        }

        uint8_t sample_8bit = audio_teste_raw[posicao_audio];
        int16_t sample_16bit = (sample_8bit - 128) * 256;
        
        data[i].channel1 = sample_16bit;
        data[i].channel2 = sample_16bit;
        
        posicao_audio++;
    }
    return len;
}


// --- Funções Públicas (Definidas no .h) ---

void setupBluetooth() {
  Serial.printf("Tamanho do áudio na memória: %d bytes\n", audio_teste_raw_len);
  Serial.printf("Procurando dispositivo: %s\n", NOME_DO_FONE);

  // Configura o callback de status
  a2dp_source.set_on_connection_state_changed(connection_state_changed);

  // Inicia o Bluetooth
  a2dp_source.start(NOME_DO_FONE, get_sound_data); 
  
  // Volume
  a2dp_source.set_volume(20); 
}

void loopBluetooth() {
  // Lógica para mostrar o progresso sem travar o áudio
  // (Este era o código comentado no seu loop original)
  static unsigned long ultima_atualizacao = 0;
  
  if (millis() - ultima_atualizacao > 1000) {
      ultima_atualizacao = millis();
      
      if (a2dp_source.is_connected()) {
          // Calcula a porcentagem
          int porcentagem = (posicao_audio * 100) / audio_teste_raw_len;
          
          Serial.printf("[Tocando] Progresso: %d%% (Byte %d de %d)\n", 
                        porcentagem, posicao_audio, audio_teste_raw_len);
          
          if (porcentagem < 2) { // Ajuste o 2% se o buffer for grande
              Serial.println(">>> Início da Faixa / Loop <<<");
          }
      } else {
          Serial.println("[Status] Aguardando conexão Bluetooth...");
      }
  }
}