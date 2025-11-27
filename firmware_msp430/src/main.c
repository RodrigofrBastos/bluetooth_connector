#include <msp430.h>
#include <stdint.h>
#include <math.h> // Necessário para sin, cos, sqrt
#include "serial_comm.h"

// --- DEFINIÇÕES DO ÁUDIO ---
// Ajuste conforme a taxa de amostragem real do seu áudio enviado pelo ESP32
#define SAMPLE_RATE 44100.0
#define CUTOFF_FREQ 2000.0   // Frequência de corte (Low Pass)

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

// --- ESTRUTURA DO FILTRO ---
typedef struct {
    float a0, a1, a2;
    float b1, b2;
    float x1, x2;  // Histórico de Entrada
    float y1, y2;  // Histórico de Saída
} ButterworthFilter;

// Variável Global do Filtro
ButterworthFilter myFilter;

// --- INICIALIZAÇÃO DO FILTRO (Roda 1 vez) ---
void init_lowpass_filter(ButterworthFilter* filter, float cutoff_freq, float sample_rate) {
    float omega = 2.0f * M_PI * cutoff_freq / sample_rate;
    float sn = sinf(omega);
    float cs = cosf(omega);
    float alpha = sn / (2.0f * 0.7071f); // Q = 0.7071 para Butterworth
    
    float a0_norm = 1.0f + alpha;
    
    // Coeficientes Brutos
    float b0 = (1.0f - cs) / 2.0f;
    float b1 = 1.0f - cs;
    float b2 = (1.0f - cs) / 2.0f;
    float a1 = -2.0f * cs;
    float a2 = 1.0f - alpha;
    
    // Normalização (Pré-calculada para economizar tempo no loop)
    filter->a0 = b0 / a0_norm;
    filter->a1 = b1 / a0_norm;
    filter->a2 = b2 / a0_norm;
    filter->b1 = a1 / a0_norm;
    filter->b2 = a2 / a0_norm;
    
    // Zera histórico
    filter->x1 = 0.0f;
    filter->x2 = 0.0f;
    filter->y1 = 0.0f;
    filter->y2 = 0.0f;
}

// --- APLICAÇÃO DO FILTRO (Roda a cada byte) ---
float apply_filter_step(ButterworthFilter* filter, float input) {
    // Fórmula Diferença: y[n] = b0*x[n] + b1*x[n-1] + b2*x[n-2] - a1*y[n-1] - a2*y[n-2]
    // Nota: Os sinais de a1 e a2 dependem da convenção da fórmula. 
    // Usando a lógica do seu código original:
    
    float output = filter->a0 * input + 
                   filter->a1 * filter->x1 + 
                   filter->a2 * filter->x2 - 
                   filter->b1 * filter->y1 - 
                   filter->b2 * filter->y2;
    
    // Atualiza histórico (Shift)
    filter->x2 = filter->x1;
    filter->x1 = input;
    
    filter->y2 = filter->y1;
    filter->y1 = output;
    
    return output;
}

int main(void) {
    WDTCTL = WDTPW | WDTHOLD;   // Stop Watchdog

    // Inicia UART e Clock (Certifique-se que serial_comm.c está configurado para 16MHz)
    uart_init();     

    // LED Verde (P1.6) para indicar processamento
    P1DIR |= BIT6;
    P1OUT &= ~BIT6;
    
    // LED Vermelho (P1.0) apenas para indicar energia
    P1DIR |= BIT0;
    P1OUT |= BIT0;

    // Inicializa os coeficientes do filtro
    // Isso usa sin/cos e é lento, mas só roda uma vez no boot
    init_lowpass_filter(&myFilter, CUTOFF_FREQ, SAMPLE_RATE);

    __bis_SR_register(GIE); // Habilita interrupções

    uint8_t raw_byte;
    float input_float;
    float output_float;
    uint8_t filtered_byte;

    // --- LOOP PRINCIPAL ---
    while(1) {
        
        // Verifica se chegou dado do ESP32
        if (uart_read_char(&raw_byte)) {
            P1OUT |= BIT6; // Liga LED (Inicio processamento)

            // 1. Converter uint8 (0-255) para float (-1.0 a 1.0)
            // (raw - 128) / 128.0
            input_float = ((float)raw_byte - 128.0f) * 0.0078125f; // Multiplicar é mais rápido que dividir
            
            // 2. Aplicar Filtro Butterworth
            output_float = apply_filter_step(&myFilter, input_float);
            
            // 3. Clamp (Segurança contra distorção numérica)
            if (output_float > 1.0f) output_float = 1.0f;
            if (output_float < -1.0f) output_float = -1.0f;
            
            // 4. Converter float de volta para uint8 (0-255)
            filtered_byte = (uint8_t)((output_float * 128.0f) + 128.0f);
            
            // 5. Envia de volta para o ESP32 (Echo filtrado)
            uart_send_char(filtered_byte);
            
            P1OUT &= ~BIT6; // Desliga LED
        }
    }
}