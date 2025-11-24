#include <msp430.h>
#include "serial_comm.h"

// Variável global de estado
state_t current_state = STATE_IDLE;

// Buffer temporário para processamento
uint8_t process_buffer[PACKET_SIZE];
uint8_t process_index = 0;

// Flag volátil para comunicação entre a Interrupção do Botão e a Main
volatile uint8_t flag_botao_pressionado = 0;

/**
 * Configura o Botão S2 (P1.3) do Launchpad com Interrupção
 */
void setup_button(void) {
    P1DIR &= ~BIT3;  // P1.3 como Entrada
    P1REN |= BIT3;   // Habilita Resistor interno
    P1OUT |= BIT3;   // Configura como Pull-UP (O botão aterra o pino)
    
    P1IES |= BIT3;   // Interrupção na Borda de Descida (High -> Low)
    P1IFG &= ~BIT3;  // Limpa flag de interrupção anterior
    P1IE  |= BIT3;   // Habilita interrupção para P1.3
}

int main(void) {
    WDTCTL = WDTPW | WDTHOLD;   // Stop Watchdog

    uart_init();     // Inicia Serial (16MHz, 115200, 8E1)
    setup_button();  // Inicia Botão com Interrupção

    // Configura LEDs: P1.0 (Vermelho) e P1.6 (Verde)
    P1DIR |= BIT0 + BIT6;
    P1OUT &= ~(BIT0 + BIT6);

    // Habilita interrupções globais (GIE) - Essencial para UART e Botão
    __bis_SR_register(GIE);

    while(1) {

        switch (current_state) {
            
            case STATE_IDLE:
                P1OUT &= ~BIT0; // LED Vermelho OFF
                
                // Se chegou dado, começa a receber
                if (uart_available() > 0) {
                    current_state = STATE_RECEIVING;
                    process_index = 0;
                    
                    // Opcional: Resetar o botão ao iniciar novo ciclo 
                    // para exigir um novo clique para cada pacote
                    flag_botao_pressionado = 0; 
                }
                break;

            // ---------------------------------------------------------
            // ESTADO 2: RECEIVING (Modificado)
            // Agora espera duas condições: Buffer Cheio AND Botão Apertado
            // ---------------------------------------------------------
            case STATE_RECEIVING:
                P1OUT |= BIT0; // LED Vermelho ON (Ocupado recebendo/esperando)

                // 1. Enche o buffer enquanto houver dados na UART
                while (uart_available() > 0 && process_index < PACKET_SIZE) {
                    uint8_t byte_temp;
                    if (uart_read_char(&byte_temp)) {
                        process_buffer[process_index++] = byte_temp;
                    }
                }

                // 2. Lógica de Transição MODIFICADA
                // Só avança SE o pacote estiver completo E o botão foi pressionado
                if (process_index >= PACKET_SIZE) {
                    
                    if (flag_botao_pressionado == 1) {
                        // Condições satisfeitas!
                        flag_botao_pressionado = 0; // Limpa a flag para a próxima vez
                        current_state = STATE_PROCESSING;
                    } 
                    else {
                        // Pacote está cheio, mas o usuário não apertou o botão.
                        // O código fica preso aqui (loopando no while(1) da main)
                        // aguardando a interrupção do botão acontecer.
                        // (O LED Vermelho continua aceso indicando "Pronto, aguardando você")
                    }
                }
                break;

            case STATE_PROCESSING:
                P1OUT |= BIT6; // LED Verde ON
                
                // Simula processamento
                __delay_cycles(16000); // Delay maior para ser visível (1ms @ 16MHz)

                current_state = STATE_SENDING;
                break;

            case STATE_SENDING:
                {
                    int i;
                    for (i = 0; i < PACKET_SIZE; i++) {
                        uart_send_char(process_buffer[i]);
                    }
                }
                current_state = STATE_SENT;
                break;

            case STATE_SENT:
                P1OUT &= ~BIT6; // LED Verde OFF
                process_index = 0;
                current_state = STATE_IDLE;
                break;
        }
    }
}

/**
 * ISR da PORTA 1 (Onde o botão P1.3 está)
 */
#if defined(__TI_COMPILER_VERSION__) || defined(__IAR_SYSTEMS_ICC__)
#pragma vector=PORT1_VECTOR
__interrupt void Port_1(void)
#elif defined(__GNUC__)
void __attribute__ ((interrupt(PORT1_VECTOR))) Port_1 (void)
#else
#error Compiler not supported!
#endif
{
    // Verifica se foi o P1.3 que causou a interrupção
    if (P1IFG & BIT3) {
        
        // Simples Debounce (Delay curto)
        __delay_cycles(5000); 

        // Verifica se o botão ainda está pressionado (Low)
        if ((P1IN & BIT3) == 0) {
            flag_botao_pressionado = 1; // LEVANTA A FLAG
        }

        P1IFG &= ~BIT3; // Limpa a flag de interrupção do hardware
    }
}