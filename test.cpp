extern int printf(string format);
extern int puts(string s);

int func(int a, int b[3]){
	printf("%d", b[a]);
    return 0;
}

int main(){
	int a[3] = {1, 2, 3};
    int i;
    for( i = 1 ; i<3; i=i+1){
        func(i, a);
    }
    return 0;
}