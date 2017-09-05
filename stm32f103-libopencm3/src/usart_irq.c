/*
 * This file is part of the libopencm3 project.
 *
 * Copyright (C) 2009 Uwe Hermann <uwe@hermann-uwe.de>
 *
 * This library is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this library.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <stdio.h>

#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/gpio.h>
#include <libopencm3/stm32/usart.h>
#include <libopencm3/cm3/nvic.h>

#define BUFFERSZ	256

struct ring_buf {
	unsigned	inptr;
	unsigned	outptr;
	unsigned char	buffer[BUFFERSZ];
} output;

int bufempty(struct ring_buf *buf)
{
	if(buf->inptr == buf->outptr) {
		return -1;
	}
	return 0;
}

int putbuf(struct ring_buf *buf, unsigned char c)
{
	unsigned	ptr;

	if(c == '\n') {
		putbuf(buf, '\r');
	}
	ptr = buf->inptr + 1;
	if(ptr >= BUFFERSZ) {
		ptr = 0;
	}
	if(ptr == buf->outptr) {
		return -1;
	}

	buf->buffer[buf->inptr] = c;
	buf->inptr = ptr;
	return 0;
}

int getbuf(struct ring_buf *buf)
{
	int		c;

	if(buf->inptr == buf->outptr) {
		return -1;
	}
	c = buf->buffer[buf->outptr];
	if(++buf->outptr >= BUFFERSZ) {
		buf->outptr = 0;
	}
	return c;
}

int writebuf(struct ring_buf *buf, unsigned char *b)
{
	while(*b) {
		if(putbuf(buf, *b++) < 0) {
			return -1;
		}
	}
	USART_CR1(USART1) |= USART_CR1_TXEIE;
	return 0;
}

static void clock_setup(void)
{
	rcc_clock_setup_in_hse_8mhz_out_72mhz();

	/* Enable GPIOB clock (for LED GPIOs). */
	rcc_periph_clock_enable(RCC_GPIOB);

	/* Enable clocks for GPIO port A (for GPIO_USART1_TX) and USART1. */
	rcc_periph_clock_enable(RCC_GPIOA);
	rcc_periph_clock_enable(RCC_AFIO);
	rcc_periph_clock_enable(RCC_USART1);
}

static void usart_setup(void)
{
	/* Enable the USART1 interrupt. */
	nvic_enable_irq(NVIC_USART1_IRQ);

	/* Setup GPIO pin GPIO_USART1_RE_TX on GPIO port B for transmit. */
	gpio_set_mode(GPIOA, GPIO_MODE_OUTPUT_50_MHZ,
		      GPIO_CNF_OUTPUT_ALTFN_PUSHPULL, GPIO_USART1_TX);

	/* Setup GPIO pin GPIO_USART1_RE_RX on GPIO port B for receive. */
	gpio_set_mode(GPIOA, GPIO_MODE_INPUT,
		      GPIO_CNF_INPUT_FLOAT, GPIO_USART1_RX);

	/* Setup UART parameters. */
	usart_set_baudrate(USART1, 9600);
	usart_set_databits(USART1, 8);
	usart_set_stopbits(USART1, USART_STOPBITS_1);
	usart_set_parity(USART1, USART_PARITY_NONE);
	usart_set_flow_control(USART1, USART_FLOWCONTROL_NONE);
	usart_set_mode(USART1, USART_MODE_TX_RX);

	/* Enable USART1 Receive interrupt. */
	USART_CR1(USART1) |= USART_CR1_RXNEIE;

	/* Finally enable the USART. */
	usart_enable(USART1);
}

static void gpio_setup(void)
{
	gpio_set(GPIOB, GPIO1);

	/* Setup GPIO1 (in GPIO port B) for LED use. */
	gpio_set_mode(GPIOB, GPIO_MODE_OUTPUT_50_MHZ,
		      GPIO_CNF_OUTPUT_PUSHPULL, GPIO1);
}

void usart1_isr(void)
{
	unsigned char	d;
	int				c;

	/* Check if we were called because of RXNE. */
	if (((USART_CR1(USART1) & USART_CR1_RXNEIE) != 0) &&
	    ((USART_SR(USART1) & USART_SR_RXNE) != 0)) {

		/* Indicate that we got data. */
		gpio_toggle(GPIOB, GPIO1);

		/* Retrieve the data from the peripheral. */
		d = usart_recv(USART1);
		if(d == '\r') {
			d = '\n';
		}
		putbuf(&output, d);

		/* Enable transmit interrupt so it sends back the data. */
		USART_CR1(USART1) |= USART_CR1_TXEIE;
	}

	/* Check if we were called because of TXE. */
	if (((USART_CR1(USART1) & USART_CR1_TXEIE) != 0) &&
	    ((USART_SR(USART1) & USART_SR_TXE) != 0)) {

		/* Indicate that we are sending out data. */
		// gpio_toggle(GPIOA, GPIO7);

		if((c = getbuf(&output)) >= 0) {
			/* Put data into the transmit register. */
			usart_send(USART1, (unsigned char)c);
		}
		if(bufempty(&output) < 0) {
			/* Disable the TXE interrupt as we don't need it anymore. */
			USART_CR1(USART1) &= ~USART_CR1_TXEIE;
		}
	}
}

int main(void)
{
	clock_setup();
	gpio_setup();
	usart_setup();

	writebuf(&output, "Hello\n");
	/* Wait forever and do nothing. */
	while (1)
		__asm__("nop");

	return 0;
}
