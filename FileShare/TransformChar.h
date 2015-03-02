#pragma  once

wchar_t * ANSIToUnicode( const char* str); 
char * UnicodeToANSI( const wchar_t* str);
wchar_t * UTF8ToUnicode( const char* str );
char * UnicodeToUTF8( const wchar_t* str );
/*¿í×Ö·û×ª»»Îª¶à×Ö·ûUnicode - ANSI*/
char* WideToMulti(const wchar_t* wcs);
/*¶à×Ö·û×ª»»Îª¿í×Ö·ûANSI - Unicode*/
wchar_t* MultitoWide(const char* mbs);
char* ANSIToUTF8(const char* str);
char* UTF8ToANSI(const char* str);