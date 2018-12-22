int a = 2;
float fl = 3.7;
int func(int b,int c)
{
	int temp = b+(2*c)-b*c;
	return temp;
}
int main()
{
	int result = func(a,23);
	return 0;
}
