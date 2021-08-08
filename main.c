#include"Encode.h"
#include<stdio.h>
#include<string.h>
#include<uchar.h>
#include<wchar.h>
int main()
{
    //你_好.html
    //%E4%BD%A0_%E5%A5%BD.html
    const char* str = u8"你_好";
    const char* res = urlEncode(str, strlen(str));
    printf("Urlencode: %s\n", res);


    char *res1 = urlDecode(res, strlen(res));
    printf("Urldecode: %s\n", res1);

    char uf[] = { 0xE4 , 0xBD , 0xA0 ,'_', 0xE5 , 0xA5 , 0xBD,0};
    printf("%s", uf);
    //utf8 1110 0100 1011 1101 1010 0000
    //gbk            1100 0100 1110 0011
    const char* hh = "你_好";
    printf("%s", hh);

    return 0;
}
