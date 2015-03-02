#pragma  once

wchar_t * ANSIToUnicode( const char* str); 
char * UnicodeToANSI( const wchar_t* str);
wchar_t * UTF8ToUnicode( const char* str );
char * UnicodeToUTF8( const wchar_t* str );
/*���ַ�ת��Ϊ���ַ�Unicode - ANSI*/
char* WideToMulti(const wchar_t* wcs);
/*���ַ�ת��Ϊ���ַ�ANSI - Unicode*/
wchar_t* MultitoWide(const char* mbs);
char* ANSIToUTF8(const char* str);
char* UTF8ToANSI(const char* str);