extern int scanf(string format);
extern int printf(string format);
extern int strlen(string text_str);
extern int putchar(int ch);

int KmpSearch(char text_str[], char pattern_str[], int nextval[]) {
	int i = 0;
	int j = 0;
	int result = 0;
    int n = strlen(text_str);
	int m = strlen(pattern_str);
	while (i < n)
	{
        int flag = 0;
		if (text_str[i] == pattern_str[j]) {
            flag = 1;
        }
        if(j == 0-1) {
            flag = 1;
        }
        if(flag == 1)
		{
			i = i+1;
			j = j+1;
		}
		else
		{
			j = nextval[j];
		}
		if (j == m)
		{
			result = result+1;
			printf("%d,", i - j);
			j = nextval[j];
		}
	}
	return result;
}

int main() {
    int n;
    int m;
    
    int nextval[1005];
    int result;
    int i = 0-1;
	int j = 0;
    
    char text_str[1024];
	char pattern_str[1024];
    printf("Please enter text string:");
    scanf("%s", text_str);
    n = strlen(text_str);
	while(n == 0) {
		printf("Text string cannot be empty! Please enter text string:");
    	scanf("%s", text_str);
    	n = strlen(text_str);
	}
    printf("Please enter pattern string:");
    scanf("%s", pattern_str);
    m = strlen(pattern_str);
	while(m == 0) {
		printf("Pattern string cannot be empty! Please enter pattern string:");
    	scanf("%s", pattern_str);
    	m = strlen(pattern_str);
	}
	
	nextval[0] = 0-1;
	while (j < m)
	{
        int flag = 0;
        if (i == 0-1) {
            flag = 1;
        }
        if (pattern_str[i] == pattern_str[j]) {
            flag = 1;
        }
		if (flag == 1)
		{
			i=i+1;
			j=j+1;
			if (pattern_str[i] != pattern_str[j])
			{
				nextval[j] = i;
			}
			else
			{
				nextval[j] = nextval[i];
			}
		}
		else {
            i = nextval[i];
        }	
	}
	result = KmpSearch(text_str, pattern_str, nextval);
    if(result == 0) {
        printf("False\n");
    }
    else {
        printf("\nTotal: %d matches!\n", result);
    }
	return 0;
}