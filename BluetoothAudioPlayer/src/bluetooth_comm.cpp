#include "bluetooth_comm.h"
#include <Arduino.h>
#include "BluetoothA2DPSource.h"
#include "test_data.h" 

BluetoothA2DPSource a2dp_source;
const char* NOME_DO_FONE = "QCY H3"; 

volatile int posicao_audio = 0;
volatile bool g_is_playing = false; 

// Ponteiros dinâmicos (iniciam apontando para nada ou teste)
const uint8_t* current_audio_data = test_data;
int current_audio_len = test_data_len;

void connection_state_changed(esp_a2d_connection_state_t state, void *ptr){
  if (state == ESP_A2D_CONNECTION_STATE_CONNECTED) {
      Serial.println("[BT] Conectado! (Aguardando comando de play...)");
  } else if (state == ESP_A2D_CONNECTION_STATE_DISCONNECTED) {
      Serial.println("[BT] Desconectado.");
  }
}

// O CORAÇÃO DO SISTEMA
int32_t get_sound_data(Frame *data, int32_t len) {
    int frames_para_copiar = len;
    
    for (int i = 0; i < frames_para_copiar; i++) {
        // MODO ESPERA: Se não foi mandado tocar, envia silêncio
        if (!g_is_playing) {
            data[i].channel1 = 0;
            data[i].channel2 = 0;
            continue; 
        }

        // MODO FIM: Se o áudio acabou, desliga o play e envia silêncio
        if (posicao_audio >= current_audio_len) { 
            posicao_audio = 0; 
            g_is_playing = false; // Desliga automaticamente
            data[i].channel1 = 0;
            data[i].channel2 = 0;
            continue;
        }

        // MODO TOCANDO: Envia o áudio escolhido
        uint8_t sample_8bit = current_audio_data[posicao_audio];
        int16_t sample_16bit = (sample_8bit - 128) * 256;
        
        data[i].channel1 = sample_16bit;
        data[i].channel2 = sample_16bit;
        
        posicao_audio++;
    }
    return len;
}

// --- Implementação da Função Unificada ---
void playBuffer(const uint8_t* data, int len) {
    // 1. Aponta para o novo áudio
    current_audio_data = data;
    current_audio_len = len;
    
    // 2. Reseta a posição do cursor
    posicao_audio = 0;
    
    // 3. Levanta a bandeira para o get_sound_data começar a enviar som real
    g_is_playing = true;
    
    Serial.printf("[BT] ▶️ Iniciando reprodução (%d bytes)\n", len);
}

void setupBluetooth() {
  a2dp_source.set_on_connection_state_changed(connection_state_changed);
  a2dp_source.start(NOME_DO_FONE, get_sound_data); 
  a2dp_source.set_volume(30); 
}

void loopBluetooth() {}

bool isAudioPlaying() { return g_is_playing; }
bool isBluetoothConnected() { return a2dp_source.is_connected(); }
int getTestDataLen() { return test_data_len; }
const uint8_t* getTestData() { return test_data; }