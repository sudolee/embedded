
int strcmp(const char *str1, const char *str2)
{
	signed char res;

	while(1) {
		if((res = *str1 - *str2++) != 0 || !*str1++)
			break;
	}
	return (int)res;
}
