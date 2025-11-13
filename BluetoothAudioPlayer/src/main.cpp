#include <Arduino.h>
#include "BluetoothA2DPSource.h"

// Inclua o arquivo que você gerou
#include "audio_data.h"

BluetoothA2DPSource a2dp_source;

const char* NOME_DO_FONE = "QCY H3"; 

// Posição atual da leitura do áudio
// 'volatile' é importante porque essa variável é alterada dentro de uma interrupção
volatile int posicao_audio = 0; 

// ======================================================
// NOVO: Callback para mostrar Status da Conexão
// ======================================================
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

// ======================================================
// CALLBACK: Envia o áudio da memória para o fone
// ======================================================
int32_t get_sound_data(Frame *data, int32_t len) {
    int frames_para_copiar = len;
    
    for (int i = 0; i < frames_para_copiar; i++) {
        // Verifica fim do áudio
        if (posicao_audio >= audio_teste_raw_len) { 
            posicao_audio = 0; 
            // Nota: Evitamos Serial.print aqui para não travar o áudio,
            // mas podemos usar uma flag se quisermos avisar no loop.
        }

        // 1. Lê o byte de 8 bits
        uint8_t sample_8bit = audio_teste_raw[posicao_audio];
        
        // 2. Converte para 16 bits
        int16_t sample_16bit = (sample_8bit - 128) * 256;
        
        // 3. Envia para os canais
        data[i].channel1 = sample_16bit;
        data[i].channel2 = sample_16bit;
        
        posicao_audio++;
    }
    return len;
}

void setup() {
  Serial.begin(115200);
  Serial.println("=======================================");
  Serial.println("   INICIANDO SISTEMA DE ÁUDIO ESP32    ");
  Serial.println("=======================================");
  
  Serial.printf("Tamanho do áudio na memória: %d bytes\n", audio_teste_raw_len);
  Serial.printf("Procurando dispositivo: %s\n", NOME_DO_FONE);

  // Configura o callback de status (para vermos quando conectar)
  a2dp_source.set_on_connection_state_changed(connection_state_changed);

  // Inicia o Bluetooth
  a2dp_source.start(NOME_DO_FONE, get_sound_data); 
  
  // Volume (Cuidado com volumes altos em 8-bit, pode gerar muito ruído)
  a2dp_source.set_volume(70); 
}

void loop() {
//   // Lógica para mostrar o progresso sem travar o áudio
//   // Executa a cada 1 segundo (1000ms)
//   static unsigned long ultima_atualizacao = 0;
  
//   if (millis() - ultima_atualizacao > 1000) {
//       ultima_atualizacao = millis();
      
//       if (a2dp_source.is_connected()) {
//           // Calcula a porcentagem
//           int porcentagem = (posicao_audio * 100) / audio_teste_raw_len;
          
//           Serial.printf("[Tocando] Progresso: %d%% (Byte %d de %d)\n", 
//                         porcentagem, posicao_audio, audio_teste_raw_len);
          
//           if (porcentagem < 2) {
//               Serial.println(">>> Início da Faixa / Loop <<<");
//           }
//       } else {
//           Serial.println("[Status] Aguardando conexão Bluetooth...");
//       }
//   }
  delay(10);
  // Pequeno delay para não saturar a CPU no loop principal
}