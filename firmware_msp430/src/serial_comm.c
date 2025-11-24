#include "serial_comm.h"

/* Variáveis do Buffer Circular */
volatile uint8_t rx_buffer[RX_BUFFER_SIZE];
volatile uint16_t rx_head = 0;
volatile uint16_t rx_tail = 0;

/**
 * Inicializa UART @ 115200 Baud, 16MHz Clock, 8E1
 */
void uart_init(void) {
    // 1. Configurar Clock para 16MHz (CRÍTICO PARA 115200 BAUD)
    if (CALBC1_16MHZ==0xFF) return; // Se calibração estiver apagada, trava
    DCOCTL = 0;
    BCSCTL1 = CALBC1_16MHZ;
    DCOCTL = CALDCO_16MHZ;

    // 2. Configurar Pinos (P1.1 = RX, P1.2 = TX)
    P1SEL |= BIT1 + BIT2;
    P1SEL2 |= BIT1 + BIT2;

    // 3. Configurar USCI_A0
    UCA0CTL1 |= UCSWRST;                       // Reset State

    UCA0CTL1 |= UCSSEL_2;                      // SMCLK (16MHz)
    
    // CONFIGURAÇÃO 8E1 (8 bits, Even Parity, 1 Stop)
    UCA0CTL0 |= UCPEN | UCPAR;                 

    // Baud Rate 115200 com 16MHz
    // 16000000 / 115200 = 138.88
    UCA0BR0 = 138;                             
    UCA0BR1 = 0;                               
    UCA0MCTL = UCBRS_7;                        // Modulação 7 (Melhor ajuste para .88)

    UCA0CTL1 &= ~UCSWRST;                      // Start UART
    
    IE2 |= UCA0RXIE;                           // Habilita Interrupção RX
    __bis_SR_register(GIE);                    // Habilita Interrupções Globais
}

void uart_send_char(char c) {
    while (!(IFG2 & UCA0TXIFG)); 
    UCA0TXBUF = c;
}

uint8_t uart_available(void) {
    return (rx_head - rx_tail) & (RX_BUFFER_SIZE - 1);
}

int uart_read_char(uint8_t *data) {
    if (rx_head == rx_tail) return 0;
    *data = rx_buffer[rx_tail];
    rx_tail = (rx_tail + 1) % RX_BUFFER_SIZE;
    return 1;
}

// ISR - Interrupção de Recepção
#if defined(__TI_COMPILER_VERSION__) || defined(__IAR_SYSTEMS_ICC__)
#pragma vector=USCIAB0RX_VECTOR
__interrupt void USCI0RX_ISR(void)
#elif defined(__GNUC__)
void __attribute__ ((interrupt(USCIAB0RX_VECTOR))) USCI0RX_ISR (void)
#else
#error Compiler not supported!
#endif
{
    if (IFG2 & UCA0RXIFG) {
        uint8_t received = UCA0RXBUF;
        uint16_t next_head = (rx_head + 1) % RX_BUFFER_SIZE;
        if (next_head != rx_tail) {
            rx_buffer[rx_head] = received;
            rx_head = next_head;
        }
        // Opcional: Acordar a CPU se estiver em Low Power Mode
        // __bic_SR_register_on_exit(LPM0_bits);
    }
}