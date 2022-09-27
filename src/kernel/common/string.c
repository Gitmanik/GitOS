#include "string.h"
#include <stddef.h>
#include <stdarg.h>
#include "../memory/memory.h"
/**
 * @brief Calculates length of specified string
 * 
 * @param str String
 * @return size_t Length
 */
size_t strlen(const char* str)
{
    size_t len = 0;
    while(str[len] != 0)
    {
        len++;
    }
    return len;
}

/**
 * @brief Reverses string
 * 
 * @param str String to reverse
 * @return char* str
 */
char *strrev(char *str)
{
    if (!str || ! *str)
        return str;

    int i = strlen(str) - 1, j = 0;

    char ch;
    while (i > j)
    {
        ch = str[i];
        str[i] = str[j];
        str[j] = ch;
        i--;
        j++;
    }
    return str;
}

/**
 * @brief Converts number to string
 * 
 * @param num Long to convert
 * @param str Buffer
 * @param base Base to use when converting the number
 * @return char* Buffer
 */
char* itoa(long num, char* str, int base)
{
	long long idx = 0;
	int negative = 0;

	if (num == 0)
	{
		str[idx++] = '0';
		str[idx] = '\0';
		return str;
	}
	if (num < 0 && base == 10)
	{
		negative = 1;
		num = -num;
	}

	while (num != 0)
	{
		long rem = num % base;
		str[idx++] = (rem > 9) ? (rem-10) + 'a' : rem + '0';
		num = num / base;
	}

	if (negative)
		str[idx++] = '-';

	str[idx] = '\0';

	strrev(str);

	return str;
}

/**
 * @brief Kernel sprintf
 * 
 * @param buf Buffer
 * @param fmt Text to format
 * @param ... Args
 * @return char* Buffer
 */
char* ksprintf(char* buf, char* fmt, ...)
{
	va_list args;
	va_start(args, fmt);

	char* org_buf = buf;

	char internal_buffer[512];

	while (*fmt != 0)
	{
		if (*fmt != '%')
		{
			*buf = *fmt;
			buf++;
			fmt++;
			continue;
		}

		fmt++;
		switch (*fmt)
		{
			int sz;
			case '%':
				*buf = '%';
				buf++;
				fmt++;
				break;

			case 'c':
				*buf = (char) va_arg(args, int);
				buf++;
				fmt++;
				break;

			case 's':
				;
				char* s = va_arg(args, char*);
				sz = strlen(s);
				memcpy(buf, s, sz);
				buf +=sz;
				fmt++;
				break;

			case 'p':
				memset(internal_buffer, 0, 512);
				itoa(va_arg(args, long), internal_buffer, 16);
				sz = strlen(internal_buffer);
				memcpy(buf, internal_buffer, sz);
				buf += sz;
				fmt++;
				break;

			case 'l': //Longer
				fmt++;
				switch (*fmt)
				{
					case 'i':
					case 'd':
						memset(internal_buffer, 0, 512);
						itoa(va_arg(args, long), internal_buffer, 10);
						sz = strlen(internal_buffer);
						memcpy(buf, internal_buffer, sz);
						buf += sz;
						fmt++;
						break;

					case 'x':
						memset(internal_buffer, 0, 512);
						itoa(va_arg(args, long), internal_buffer, 16);
						sz = strlen(internal_buffer);
						memcpy(buf, internal_buffer, sz);
						buf += sz;
						fmt++;
						break;

					default:
						fmt++;
						break;
				}
				break;

			case 'x':
				memset(internal_buffer, 0, 512);
				itoa(va_arg(args, int), internal_buffer, 16);
				sz = strlen(internal_buffer);
				memcpy(buf, internal_buffer, sz);
				buf += sz;
				fmt++;
				break;
			case 'i':
			case 'd':
				memset(internal_buffer, 0, 512);
				itoa(va_arg(args, int), internal_buffer, 10);
				sz = strlen(internal_buffer);
				memcpy(buf, internal_buffer, sz);
				buf += sz;
				fmt++;
				break;

			default:
				fmt++;
				break;
		}

	}
	va_end(args);
	return org_buf;
}