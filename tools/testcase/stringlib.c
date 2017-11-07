#include <linux/kernel.h>
#include <string.h>

#ifdef CONFIG_TESTCODE
/*
 * Test /lib/string.c - strcpy()
 */
void test_strcpy(void)
{
	char buffer[10];
	const char *str = "strcpy";

	strcpy(buffer, str);
	printk("Buffer %s\n", buffer);
}

/*
 * Test /lib/string.c - strncpy()
 */
void test_strncpy(void)
{
	char buffer[10];
	const char *str = "strncp";

	strncpy(buffer, str, 3);
	printk("%s buffer %s\n", __func__, buffer);
}

/*
 * Test /lib/string.c - strcat()
 */
void test_strcat(void)
{
	char buffer[10] = "H";
	const char *str = "elloWorld";

	strcat(buffer, str);
	printk("%s buffer %s\n", __func__, buffer);
}

/*
 * Test /lib/string.c - strncat()
 */
void test_strncat(void)
{
	char buffer[10] = "H";
	const char *str = "elloWorld";

	strncat(buffer, str, 3);
	printk("%s buffer %s\n", __func__, buffer);
}

/*
 * Test /lib/string.c - strcmp()
 */
void test_strcmp(void)
{
	char *str1 = "Hello";
	char *str2 = "HelloA";
	char *str3 = "HelloB";
	char *str4 = "Hello";

	/* equ */
	if ((strcmp(str1, str4) == 0))
		printk("%s == %s\n", str1, str4);

	/* little */
	if ((strcmp(str1, str2) < 0))
		printk("%s < %s\n", str1, str2);

	/* big */
	if ((strcmp(str3, str2) > 0))
		printk("%s > %s\n", str3, str2);
}

/*
 * Test /lib/string.c - strncmp()
 */
void test_strncmp(void)
{
	const char *str1 = "HelAoWorld";
	const char *str2 = "HelBoWorld";
	const char *str3 = "HelCoWorld";
	const char *str4 = "HelB";

	if (strncmp(str2, str1, 4) > 0)
		printk("%s > %s range in 4 words\n", str2, str1);

	if (strncmp(str2, str3, 4) < 0)
		printk("%s < %s range in 4 words\n", str2, str3);

	if (strncmp(str2, str4, 4) == 0)
		printk("%s == %s range in 4 words\n", str2, str4);
}

/*
 * Test /lib/string.c - strchr()
 */
void test_strchr(void)
{
	const char *str = "HelloWorld";
	char c = 'W';
	char *dest;

	dest = strchr(str, c);
	printk("%s Cut %s from %c is %s\n", __func__, str, c, dest);
}

#endif
