#include "serial_comm.h"

/* Variáveis do Buffer Circular */
volatile uint8_t rx_buffer[RX_BUFFER_SIZE];
volatile uint16_t rx_head = 0;
volatile uint16_t rx_tail = 0;

void uart_init(void) {
    // 1. Configurar Clock para 16MHz (Essencial para baud rates altos/estáveis)
    if (CALBC1_16MHZ==0xFF) return; // Se calibração estiver apagada, trava (segurança)
    DCOCTL = 0;
    BCSCTL1 = CALBC1_16MHZ;
    DCOCTL = CALDCO_16MHZ;

    // 2. Configurar Pinos (P1.1 = RX, P1.2 = TX) HW UART
    P1SEL |= BIT1 + BIT2;
    P1SEL2 |= BIT1 + BIT2;

    // 3. Configurar USCI_A0
    UCA0CTL1 |= UCSWRST;                       // Coloca em Reset para configurar
    UCA0CTL1 |= UCSSEL_2;                      // Fonte de Clock: SMCLK (16MHz)
    
    // --- CONFIGURAÇÃO 8E1 (8 bits, Even Parity, 1 Stop) ---
    // UCPEN = Habilita Paridade
    // UCPAR = 1 (Paridade Par/Even)
    // UCSPB = 0 (1 Stop Bit - Padrão, não precisa setar bit)
    // UC7BIT = 0 (8 bits de dados - Padrão)
    UCA0CTL0 |= UCPEN | UCPAR;                 

    // --- CÁLCULO PARA 38400 BAUD @ 16MHz ---
    // 16,000,000 / 38400 = 416.666...
    // Divisão inteira: 416
    // 416 em hex = 0x01A0 -> BR1 = 0x01, BR0 = 0xA0 (160)
    
    UCA0BR0 = 160;                             // Divisor Baixo
    UCA0BR1 = 1;                               // Divisor Alto (256 + 160 = 416)
    
    // Modulação para compensar o 0.666... restante
    // UCBRS_6 é uma boa aproximação para 0.66
    UCA0MCTL = UCBRS_6;                        

    UCA0CTL1 &= ~UCSWRST;                      // Tira do Reset (Inicia UART)
    
    IE2 |= UCA0RXIE;                           // Habilita Interrupção de Recepção
}

void uart_send_char(uint8_t c) {
    // Espera o buffer de TX estar pronto para enviar
    while (!(IFG2 & UCA0TXIFG)); 
    UCA0TXBUF = c;
}

uint16_t uart_available(void) {
    // Retorna quantos bytes tem no buffer circular
    return (rx_head - rx_tail) & (RX_BUFFER_SIZE - 1);
}

int uart_read_char(uint8_t *data) {
    // Se cabeça == rabo, buffer vazio
    if (rx_head == rx_tail) return 0;
    
    *data = rx_buffer[rx_tail];
    rx_tail = (rx_tail + 1) & (RX_BUFFER_SIZE - 1); // Avança rabo com wrap-around
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
        
        // Calcula próxima posição da cabeça
        uint16_t next_head = (rx_head + 1) & (RX_BUFFER_SIZE - 1);
        
        // Se não for atropelar o rabo (buffer cheio), salva
        if (next_head != rx_tail) {
            rx_buffer[rx_head] = received;
            rx_head = next_head;
        }
        // Se estiver cheio, descarta o byte (melhor perder 1 byte do que travar lógica)
    }
}