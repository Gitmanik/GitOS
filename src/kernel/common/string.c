#include "string.h"
#include <stddef.h>
#include <stdarg.h>
#include "../memory/memory.h"
/**
 * @brief Determine the length of a string
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
 * @brief Determine the length of a fixed-size string
 * 
 * @param str 
 * @param max_len 
 * @return size_t 
 */
size_t strnlen(const char* str, size_t max_len)
{
	for (size_t len = 0; len < max_len; len++)
	{
		if (str[len] == 0)
			return len;
	}
	return max_len;
}

/**
 * @brief Reverses string
 * 
 * @param str String to reverse
 * @return char* str
 */
char* strrev(char *str)
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
 * @brief Copies string
 * 
 * @param dest Destination buffer
 * @param src Input buffer
 * @return char* Pointer to destination buffer
 */
char* strcpy(char* dest, const char* src)
{
	char* dest_org = dest;
	while (*src != 0)
	{
		*dest = *src;
		src++;
		dest++;
	}
	*dest = '\0';
	return dest_org;
}

/**
 * @brief Copies string with maximum length
 * 
 * @param dest Destination buffer
 * @param src Input buffer
 * @param n Max length 
 * @return char* Pointer to destination buffer
 */
char* strncpy(char* dest, const char* src, int n)
{
	char* dest_org = dest;
	
	int i;
	for (i = 0; i < n-1; i++)
	{
		if (src[i] == 0)
			break;
		dest[i] = src[i];
	}
	dest[i] = 0;
	return dest_org;
}

/**
 * @brief Compares two strings with maximum length
 * 
 * @param str1 String
 * @param str2 String
 * @param n Max length 
 * @return int 0 -> str1==str2, <0 -> str1<str2, >0 ->str1>str2
 */
int strncmp(const char* str1, const char* str2, int n)
{
    unsigned char u1, u2;

    while(n-- > 0)
    {
        u1 = (unsigned char)*str1++;
        u2 = (unsigned char)*str2++;
        if (u1 != u2)
            return u1 - u2;
        if (u1 == '\0')
            return 0;
    }

    return 0;
}

/**
 * @brief Converts ASCII letter to lowercase
 * 
 * @param c Input letter
 * @return char Lowercase letter
 */
char tolower(char c)
{
	if (c >= 65 && c <= 90)
		c += 32;
	return c;
}

int istrncmp(const char* str1, const char* str2, int n)
{
    unsigned char u1, u2;

    while(n-- > 0)
    {
        u1 = (unsigned char)*str1++;
        u2 = (unsigned char)*str2++;
        if (u1 != u2 && tolower(u1) != tolower(u2))
            return u1 - u2;
        if (u1 == '\0')
            return 0;
    }

    return 0;
}

/**
 * @brief Calculates string length with custom terminator
 * 
 * @param str String to measure length
 * @param max Max length of string
 * @param terminator Custom terminator
 * @return int Length
 */
int strnlen_terminator(const char* str, int max, char terminator)
{
	int i;
	for (i = 0; i < max; i++)
	{
		if (str[i] == 0 || str[i] == terminator)
			break;
	}
	return i;
}

/**
 * @brief Converts signed number to string
 * 
 * @param num Long to convert
 * @param str Buffer
 * @param base Base to use when converting the number
 * @return char* Buffer
 */
char* itoa(long num, char* str, int base)
{
	int idx = 0;
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
 * @brief Converts unsigned number to string
 *
 * @param num Unsigned Long to convert
 * @param str Buffer
 * @param base Base to use when converting the number
 * @return char* Buffer
 */
char* uitoa(unsigned long num, char* str, int base)
{
	int idx = 0;

	if (num == 0)
	{
		str[idx++] = '0';
		str[idx] = '\0';
		return str;
	}

	while (num != 0)
	{
		long rem = num % base;
		str[idx++] = (rem > 9) ? (rem-10) + 'a' : rem + '0';
		num = num / base;
	}

	str[idx] = '\0';

	strrev(str);

	return str;
}


/**
 * @brief Kernel vsprintf
 * params: %% %c %s %p %l(mod) %d %i %x
 * @param buf Buffer
 * @param fmt Text to format
 * @param va_list arguments
 * @return char* Buffer
 */
char* kvsprintf(char* buf, const char* fmt, va_list args)
{
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

					case 'b':
						memset(internal_buffer, 0, 512);
						itoa(va_arg(args, long), internal_buffer, 2);
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
			
			case 'b':
				memset(internal_buffer, 0, 512);
				itoa(va_arg(args, int), internal_buffer, 2);
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

/**
 * @brief Kernel sprintf
 * params: %% %c %s %p %l(mod) %d %i %x
 * @param buf Buffer
 * @param fmt Text to format
 * @param ... Args
 * @return char* Buffer
 */
char* ksprintf(char* buf, const char* fmt, ...)
{
	va_list args;
	va_start(args, fmt);

	kvsprintf(buf, fmt, args);
	return buf;
}

/**
 * @brief Determines if given char is an ASCII digit
 * 
 * @param c Character to test
 * @return int 1 if given char is digit
 */
int is_digit(char c)
{
	return c >= 48 && c <= 57;
}

/**
 * @brief Converts ASCII digit to numeric
 * 
 * @param c ASCII digit
 * @return int Numeric digit
 */
int to_numeric_digit(char c)
{
	return c - 48;
}