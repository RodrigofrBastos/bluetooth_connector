#include <msp430.h> // A "lib padrão" que você pediu

/**
 * Pisca o LED Vermelho (P1.0) usando acesso direto 
 * aos registradores (Bare-Metal).
 */
int main(void) {

    // 1. PARAR O WATCHDOG TIMER (WDT)
    // Esta é a primeira coisa a se fazer em 99% dos códigos
    // de MSP430. Se não fizer isso, o chip reinicia sozinho.
    WDTCTL = WDTPW + WDTHOLD;

    // 2. CONFIGURAR O PINO DO LED (P1.0)
    // P1DIR = "Port 1 Direction Register" (Registro de Direção)
    // Queremos que P1.0 seja SAÍDA.
    // Usamos 'ou' (|=) para "ligar" o BIT0 (que é 0x01)
    P1DIR |= BIT0; // Equivalente a pinMode(RED_LED, OUTPUT);

    // 3. INICIAR O LED DESLIGADO
    // P1OUT = "Port 1 Output Register" (Registro de Saída)
    // Usamos 'e não' (&= ~) para "desligar" o BIT0.
    P1OUT &= ~BIT0; // Equivalente a digitalWrite(RED_LED, LOW);


    // 4. LOOP INFINITO
    while (1) {
        
        // 5. INVERTER (TOGGLE) O LED
        // P1OUT ^= BIT0; (Operação XOR) inverte o estado do BIT0
        P1OUT ^= BIT0; // Inverte o LED

        // 6. DELAY (ESPERA)
        // Não temos a função delay()... então criamos uma espera "gasta-CPU"
        // A palavra "volatile" impede o compilador de otimizar
        // e apagar nosso loop.
        volatile unsigned int i;
        for (i = 30000; i > 0; i--);
    }

    // O programa nunca deve chegar aqui, mas é uma boa prática
    // em C ter um 'return' na 'main'.
    return 0;
}