////////////////////////////////////////////////////////////////////////
// printf para a uart (apenas permite enviar pela porta série os printfs)
// não trata de receber nada!
// Config: 38400, 8, N, 1   e uC com cristal de 14.7456 MHz
// Obs: é necessário chamar init_printf_tools()
////////////////////////////////////////////////////////////////////////

#ifndef	_PRINTF_TOOLS_H
#define	_PRINTF_TOOLS_H	1


#include <stdio.h>

static int uart_putchar(char c, FILE *stream);

static FILE mystdout = FDEV_SETUP_STREAM(uart_putchar, NULL, _FDEV_SETUP_WRITE);

static int uart_putchar(char c, FILE *stream)
{
    loop_until_bit_is_set(UCSRA, UDRE);
    UDR = c;
    return 0;
}


static void init_printf_tools(void) 
{
	  // init stdout
    stdout = &mystdout; 
}


#endif