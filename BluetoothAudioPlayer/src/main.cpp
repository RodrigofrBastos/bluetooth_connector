// #include <Arduino.h>
// #include "bluetooth_comm.h" // <-- Inclui nosso novo módulo
// // #include "serial_comm.h" // <-- Você poderia incluir outros módulos aqui

// void setup() {
//   Serial.begin(115200);
//   Serial.println("=======================================");
//   Serial.println("   INICIANDO SISTEMA DE ÁUDIO ESP32    ");
//   Serial.println("=======================================");

//   // Delega a configuração do Bluetooth
//   setupBluetooth();
  
//   // setupSerial(); // Você chamaria outras inicializações aqui
// }

// void loop() {
//   // Delega as tarefas de loop do Bluetooth
//   loopBluetooth(); 
  
//   // loopSerial(); // Você chamaria outras tarefas de loop aqui

//   // Pequeno delay para não saturar a CPU no loop principal
//   delay(10); 
// }

#include <Arduino.h>
#include "serial_comm.h" // Inclui nosso novo módulo
// Você pode incluir outros módulos aqui, como o bluetooth_comm

void setup() {
  // Inicia o Monitor Serial (USB) para vermos os prints
  Serial.begin(115200); 
  Serial.println();
  Serial.println("======= TESTE DE LOOPBACK SERIAL =======");
  
  // Chama a configuração do nosso módulo serial
  setupSerialComms(); 
}

void loop() {
  // Chama o loop do nosso módulo serial
  loopSerialComms();
}