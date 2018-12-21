int a = 2;
int func(int b,int c)
{
	int temp = b+(2*c)-b*c;
	return temp;
}
int main()
{
	int result = func(12,23);
	return 0;
}
