#include <msp430.h>
#include <stdint.h>
#include "audio_data.h" // Inclua o seu arquivo de áudio aqui

/* * Definições de Ponto Fixo Q15
 * 32768 representa 1.0. 
 * Qualquer multiplicação resulta em Q30, então fazemos shift de 15 (>> 15) para voltar a Q15.
 */
#define Q15_SHIFT 15

// Estrutura para manter o estado de cada seção do filtro (Biquad)
typedef struct {
    int16_t b0, b1, b2; // Coeficientes do Numerador
    int16_t a1, a2;     // Coeficientes do Denominador (a0 é assumido como 1.0/32768)
    int16_t w1, w2;     // Linhas de atraso (Delay lines / Estados anteriores)
} Biquad;

// Número de seções conforme seu header do MATLAB (MWSPT_NSEC)
#define NUM_SECTIONS 5

// Mapeamento dos seus coeficientes gerados pelo MATLAB para nossa estrutura limpa.
// Nota: Convertemos uint16_T para int16_t porque filtros usam números com sinal.
Biquad filter[NUM_SECTIONS] = {
    // Seção 0 (Ganho)
    { .b0=4824, .b1=0, .b2=0, .a1=0, .a2=0, .w1=0, .w2=0 },
    // Seção 1 (Filtro)
    { .b0=32768, .b1=0, .b2=0, .a1=0, .a2=31511, .w1=0, .w2=0 },
    // Seção 2 (Ganho)
    { .b0=4824, .b1=0, .b2=0, .a1=0, .a2=0, .w1=0, .w2=0 },
    // Seção 3 (Filtro)
    { .b0=32768, .b1=0, .b2=0, .a1=0, .a2=21440, .w1=0, .w2=0 },
    // Seção 4 (Ganho Final Unitário)
    { .b0=32768, .b1=0, .b2=0, .a1=0, .a2=0, .w1=0, .w2=0 }
};

/*
 * Função de Processamento do Filtro IIR (Forma Direta II)
 * Entrada: Amostra atual (int16_t)
 * Saída: Amostra filtrada (int16_t)
 */
int16_t IIR_Filter_Process(int16_t input) {
    int32_t accumulator; // Acumulador de 32 bits é CRUCIAL para evitar overflow
    int16_t x_in = input;
    int16_t y_out = 0;
    int i;

    // Processa cada estágio do filtro em cascata
    for (i = 0; i < NUM_SECTIONS; i++) {
        
        // --- Forma Direta II Transposta ou Padrão ---
        // Equação: w[n] = x[n] - a1*w[n-1] - a2*w[n-2]
        
        accumulator = (int32_t)x_in << Q15_SHIFT; // Coloca entrada em escala para contas
        
        // Parte do Feedback (Denominador)
        // Nota: Subtraímos porque a equação é x - a*w. 
        // O MATLAB exporta 'a' positivo, então subtraímos.
        accumulator -= (int32_t)filter[i].a1 * filter[i].w1;
        accumulator -= (int32_t)filter[i].a2 * filter[i].w2;
        
        // O resultado do acumulador é o novo estado w[n] (em formato Q30 devido à mult)
        // Precisamos trazer de volta para Q15 para salvar nos estados, mas cuidado com precisão.
        // No MSP430, uma implementação robusta simples faz o shift no final da soma.
        
        int16_t w_n = (int16_t)(accumulator >> Q15_SHIFT); 

        // Parte do Feedforward (Numerador)
        // y[n] = b0*w[n] + b1*w[n-1] + b2*w[n-2]
        accumulator =  (int32_t)filter[i].b0 * w_n;
        accumulator += (int32_t)filter[i].b1 * filter[i].w1;
        accumulator += (int32_t)filter[i].b2 * filter[i].w2;

        y_out = (int16_t)(accumulator >> Q15_SHIFT);

        // Atualiza as linhas de atraso (Shift dos estados)
        filter[i].w2 = filter[i].w1;
        filter[i].w1 = w_n;

        // A saída deste estágio é a entrada do próximo
        x_in = y_out;
    }

    return y_out;
}

void main(void) {
    WDTCTL = WDTPW | WDTHOLD;   // Parar Watchdog
    
    // Variáveis para iteração
    int i = 0; 
    volatile int16_t saidaDAC = 0; // Saída do filtro (Saída)
    
    // Mantenha um ponteiro de leitura volátil para rastreamento no debug
    volatile int16_t *audio_ptr = (int16_t *)audio_teste_raw;

    // Loop principal para processar todo o arquivo de áudio
    while(i < audio_teste_raw_len) {
        
        // 1. Ler a amostra de entrada
        int16_t entrada_sample = audio_ptr[i]; 

        // 2. Rodar o filtro para a amostra atual
        saidaDAC = IIR_Filter_Process(entrada_sample);

        // 3. (OPCIONAL) Salvar a saída filtrada em outro array 
        // Se você quiser salvar o resultado, crie um array de saída:
        // filtered_output[i] = saidaDAC;

        // 4. Avance o índice
        i++;
        
        // Em um sistema real, você executaria o filtro em uma rotina de interrupção 
        // disparada pelo timer ou pela taxa de amostragem. No seu teste, a iteração basta.
    }
    
    // Após o loop, o processamento de áudio terminou.
    __no_operation(); 
}