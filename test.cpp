extern int printf(string format);
extern int scanf(string format);
extern int strlen(string s);

int palindrome(char str[],int len){
    int i;
    int l = len / 2;
    len = len - 1;
    for(i = 0; i <= l; i = i + 1){
        if(str[i] != str[len - i]){
            printf("False");
            i = l + 2;
        }
    }
    if (i == l + 1) {
        printf("True");
    }
    return 0;
} 

int main(){
	char str[128];
    printf("【回文检测】请输入128字节以内的字符串：");
    scanf("%s", str);
    int len = strlen(str);
    palindrome(str, len);
    return 0;
}