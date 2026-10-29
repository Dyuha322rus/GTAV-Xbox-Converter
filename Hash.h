#pragma once

unsigned int GetHashKey(char *str, unsigned int a2)
{
	unsigned int v2;
	char *temp;
	char i;
	
	v2 = a2;
	temp = str;
	if(*str == '"')
		temp = str + 1;
	for(i = *temp; *temp; i = *temp)
	{
		if(*str == '"' && i == '"')
			break;
		++temp;
		if((unsigned __int8)(i - 65) > 25)
		{
			if(i == '\\')
				i = '/';
		}
		else
			i += 32;
		v2 = (1025 * (v2 + i) >> 6) ^ 1025 * (v2 + i);
	}
	return 32769 * (9 * v2 ^ (9 * v2 >> 11));
}