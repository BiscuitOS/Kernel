#include <linux/kernel.h>

#ifdef CONFIG_TESTCODE
/*
 * Test /lib/string.c - strcpy()
 */
extern inline char *strcpy(char *dest, const char *src);

void test_strcpy(void)
{
	char buffer[30];
	const char *str = "strcpy";

	strcpy(buffer, str);

	printk("Buffer %s\n", buffer);
}

#endif
