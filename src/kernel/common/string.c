#include "string.h"
#include <stddef.h>

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
 * @brief Converts long to string
 * 
 * @param num Long to convert
 * @param str Buffer
 * @param base Base to use when converting the number
 * @return char* Buffer
 */
char* itoa(int num, char* str, int base)
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
		int rem = num % base;
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
 * @brief Converts long to string
 * 
 * @param num Long to convert
 * @param str Buffer
 * @param base Base to use when converting the number
 * @return char* Buffer
 */
char* ltoa(long num, char* str, int base)
{
	long idx = 0;
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