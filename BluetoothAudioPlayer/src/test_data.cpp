#include "test_data.h"

// AQUI ocorre a definição real dos dados (alocação de memória)

const unsigned char test_data[] = {
  0xff, 0xff, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00, 0xfe, 0xff, 0xff, 0xff
  // ... coloque aqui o restante dos seus dados de áudio ...
};
// Calcula o tamanho automaticamente aqui
const unsigned int test_data_len = sizeof(test_data);
