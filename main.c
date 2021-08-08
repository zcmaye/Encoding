#include"Encode.h"
#include<stdio.h>
#include<string.h>
#include<uchar.h>
int main()
{
	//你_好.html
	//%E4%BD%A0_%E5%A5%BD.html
	const char *res = urlEncode("你_好.html", 10);
	printf("Urlencode: %s\n", res);


	res = urlDecode(res, strlen(res));
	printf("Urldecode: %s\n", res);

	return 0;
}