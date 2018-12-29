extern int printf(string format);
extern int scanf(string format);
extern int strlen(string s);

int priority(char s) {
    char opt[7] = {'(', '*', '/', '+', '-', ')', '#'};
    char value[7] = {3, 2, 2, 1, 1, 0, 0};
    int i = 0;
    int result = 0;
    for (i = 0; i < 7; i = i + 1) {
        if (opt[i] == s) {
            result = value[i];
            i = 8;
        }
    }
    return result;
}

int logicalAnd(int a, int b) {
    int flag = 0;
    if (a) {
        flag = flag + 1;
    }
    if (b) {
        flag = flag + 1;
    }
    if (flag == 1) {
        flag = 0;
    } else if (flag == 2) {
        flag = 1;
    }
    return flag;
}

int logicalOr(int a, int b) {
    int flag = 0;
    if (a) {
        flag = 1;
    }
    if (b) {
        flag = 1;
    }
    return flag;
}

int arithmeticOperation(char str[], int len) {
    char RPNStack[20];
    int RPNStackTop = 0;
    char operationStack[20];
    int operationStackTop = 0;
    int i = 0;
    int tmp = 0;
    int subnum = 0;
    char tempOpt;
    int lnum = 0;
    int rnum = 0;
    while(logicalOr(i < len, operationStackTop != 0))
	{
		if (logicalAnd(str[i] >= '0', str[i] <= '9'))
		{
            subnum = str[i] - '0';
            tmp = tmp * 10 + subnum;   
			i = i + 1;
			if (logicalOr(str[i] < '0', str[i] > '9'))
			{
                printf("push %d to RPNStack\r\n", tmp);
				RPNStack[RPNStackTop] = tmp;
                RPNStackTop = RPNStackTop + 1;
				tmp = 0;
			}
		}
		else
		{   
			if (logicalOr(logicalOr(operationStackTop == 0, priority(operationStack[operationStackTop - 1]) < priority(str[i])), logicalAnd(str[i] != ')', operationStack[operationStackTop - 1] == '(')))
			{
                printf("push %c to operationStackTop\r\n", str[i]);
				operationStack[operationStackTop] = str[i];
                operationStackTop = operationStackTop + 1;
				i = i + 1;
			} else {
                if (logicalAnd(operationStack[operationStackTop - 1] == '(', str[i] == ')'))
                {
                    printf("pop ( from operationStackTop\r\n");
                    operationStackTop = operationStackTop - 1;
                    i = i + 1;
                } else {
                    if (logicalOr(logicalOr(str[i] == '\0', logicalAnd(str[i] == ')', operationStack[operationStackTop - 1] != '(')), priority(str[i]) <= priority(operationStack[operationStackTop - 1])))
                    {
                        tempOpt = operationStack[operationStackTop - 1];
                        operationStackTop = operationStackTop - 1;
                        printf("pop %c from operationStackTop\r\n", tempOpt);
                        rnum = RPNStack[RPNStackTop - 1];
                        RPNStackTop = RPNStackTop - 1;
                        printf("pop %d from RPNStackTop\r\n", rnum);
                        lnum = RPNStack[RPNStackTop - 1];
                        RPNStackTop = RPNStackTop - 1;
                        printf("pop %d from RPNStackTop\r\n", rnum);
                        if (tempOpt == '+'){
                            RPNStack[RPNStackTop] = lnum + rnum;
                        } else if (tempOpt == '-') {
                            RPNStack[RPNStackTop] = lnum - rnum;
                        } else if (tempOpt == '*') {
                            RPNStack[RPNStackTop] = lnum * rnum;
                        } else if (tempOpt == '/') {
                            RPNStack[RPNStackTop] = lnum / rnum;
                        } else {
                            printf("ERROR: unkown operation!\r\n");
                        }     
                        RPNStackTop = RPNStackTop + 1;  
                        printf("push %d to RPNStackTop\r\n", RPNStack[RPNStackTop - 1]);       
                    }
                }
            }
		}
	}    
    return RPNStack[RPNStackTop - 1];
}

int main(){
	char str[40];
    printf("【四则运算】请输入50字节以内的字符串：");
    scanf("%s", str);
    int len = strlen(str);
    printf("answer:%d\r\n", arithmeticOperation(str, len));
    return 0;
}