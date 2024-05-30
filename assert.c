#include <stdio.h>
#include <stdlib.h>

void custom_assert_handler(const char *file, int line)
{
    (void)file; /*Unused*/
    (void)line; /*Unused*/
    printf("Assertion failed! %s:%d\n", file, line);
    while (1)
    {
    }
}