#include <goofy-os/fbcon.h>
#include <goofy-os/spinlock.h>
#include <stdarg.h>
#include <stdbool.h>

struct spinlock printk_lock;

void reverse(char str[], int length) {
	int start = 0;
	int end = length - 1;
	while (start < end) {
		char temp = str[start];
		str[start] = str[end];
		str[end] = temp;
		end--;
		start++;
	}
}

char *itoa(int num, char *str, int base, int capital) {
	int i = 0;
	bool isNegative = false;
	if (num == 0) {
		str[i++] = '0';
		str[i++] = '\0';
		return str;
	}

	if (num < 0 && base == 10) {
		isNegative = true;
		num = -num;
	} else if (num < 0) {
		num = -num; // Won't have minus sign, because convention or
			    // whatever
	}

	while (num != 0) {
		int rem = num % base;
		str[i++] =
		    (rem > 9) ? (rem - 10) + (capital ? 'A' : 'a') : rem + '0';
		num = num / base;
	}

	if (isNegative)
		str[i++] = '-';

	reverse(str, i);
	str[i] = '\0';
	return str;
}

char *ltoa(uint64_t num, char *str, int base, int capital) {
	int i = 0;
	if (num == 0) {
		str[i++] = '0';
		str[i++] = '\0';
		return str;
	}

	while (num != 0) {
		uint64_t rem = num % base;
		str[i++] =
		    (rem > 9) ? (rem - 10) + (capital ? 'A' : 'a') : rem + '0';
		num = num / base;
	}

	reverse(str, i);
	str[i] = '\0';
	return str;
}

int strnlen(const char *s, int count) {
	const char *sc;

	for (sc = s; count-- && *sc != '\0'; ++sc)
		/* nothing */;
	return sc - s;
}

int _vsprintf(char *buf, const char *fmt, va_list args) {
	char *str;
	char *s;
	char numbuf[16];
	int len;

	for (str = buf; *fmt; ++fmt) {
		if (*fmt != '%') {
			*str++ = *fmt;
			continue;
		}

		++fmt;
		switch (*fmt) {
		case 's':
			s = va_arg(args, char *);
			len = strnlen(s, 1024); // Max length: 1024

			for (int i = 0; i < len; ++i)
				*str++ = *s++;
			continue;
		case 'c':
			*str++ = (char)va_arg(args, int);
			continue;
		case 'd':
			itoa(va_arg(args, int32_t), numbuf, 10, 0);
			for (int i = 0; numbuf[i] != '\0'; i++) {
				*str++ = numbuf[i];
			}
			continue;
		case 'x':
			itoa(va_arg(args, int32_t), numbuf, 16, 0);
			for (int i = 0; numbuf[i] != '\0'; i++) {
				*str++ = numbuf[i];
			}
			continue;
		case 'X':
			itoa(va_arg(args, int32_t), numbuf, 16, 1);
			for (int i = 0; numbuf[i] != '\0'; i++) {
				*str++ = numbuf[i];
			}
			continue;
		case 'p':
			*str++ = '0';
			*str++ = 'x';
			ltoa(va_arg(args, uint64_t), numbuf, 16, 1);
			for (int i = 0; numbuf[i] != '\0'; i++) {
				*str++ = numbuf[i];
			}
			continue;
		}
	}

	*str = '\0';
	return (str - buf);
}

int printk(const char *fmt, ...) {
	char printf_buf[1024];
	va_list args;
	int printed;

	va_start(args, fmt);
	printed = _vsprintf(printf_buf, fmt, args);
	va_end(args);

	acquire(&printk_lock);
	if (fbcon.enabled)
		console_write(printf_buf);

	for (int i = 0; printf_buf[i]; i++)
		serial_write(printf_buf[i]);
	release(&printk_lock);

	return printed;
}

int sprintf(char *buf, const char *fmt, ...) {
	va_list args;
	int printed;

	va_start(args, fmt);
	printed = _vsprintf(buf, fmt, args);
	va_end(args);
	return printed;
}
