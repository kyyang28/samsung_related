
/*
 *  Embedded Fans Club (EFC)
 *  string.c 
 */
unsigned int efc_strlen(const char *str)
{
    const char *s;

    for (s = str; *s != '\0'; ++s);
    return s - str;
}

char * efc_strcpy(char *dest, const char *src)
{
    char *res = dest;

    while ((*dest++ = *src++) != '\0');
    return res;
}

