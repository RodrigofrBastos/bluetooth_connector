#ifndef BLUETOOTH_COMM_H
#define BLUETOOTH_COMM_H

#include <Arduino.h>

void setupBluetooth();
void loopBluetooth();

// --- FUNÇÃO UNIFICADA DE REPRODUÇÃO ---
// Recebe o array de áudio e o tamanho.
// Reinicia a posição, define o buffer e ativa a flag de reprodução.
void playBuffer(const uint8_t* data, int len);

// Verifica se o áudio ainda está tocando
bool isAudioPlaying();

// Verifica se o dispositivo está conectado
bool isBluetoothConnected();

// Getters para o áudio de teste (Boot)
int getTestDataLen();
const uint8_t* getTestData();

#endif