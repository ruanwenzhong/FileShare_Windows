
#include "stdafx.h"
#include "TransformChar.h"

wchar_t * ANSIToUnicode(const char* str)
{
	int textlen ;
	wchar_t * result;
	textlen = MultiByteToWideChar( CP_ACP, 0, str,-1, NULL,0 );
	result = (wchar_t *)malloc((textlen+1)*sizeof(wchar_t));
	memset(result,0,(textlen+1)*sizeof(wchar_t));
	MultiByteToWideChar(CP_ACP, 0,str,-1,(LPWSTR)result,textlen);
	return result; 
}

char * UnicodeToANSI(const wchar_t* str)
{
	char* result;
	int textlen;
	textlen = WideCharToMultiByte( CP_ACP, 0, str, -1, NULL, 0, NULL, NULL );
	result =(char *)malloc((textlen+1)*sizeof(char));
	memset( result, 0, sizeof(char) * ( textlen + 1 ) );
	WideCharToMultiByte( CP_ACP, 0, str, -1, result, textlen, NULL, NULL );
	return result;
}

wchar_t * UTF8ToUnicode( const char* str )
{
	int textlen ;
	wchar_t * result;
	textlen = MultiByteToWideChar( CP_UTF8,0, str,-1, NULL,0 ); 
	result = (wchar_t *)malloc((textlen+1)*sizeof(wchar_t)); 
	memset(result,0,(textlen+1)*sizeof(wchar_t)); 
	MultiByteToWideChar(CP_UTF8, 0,str,-1,(LPWSTR)result,textlen); 
	return result; 
}

char * UnicodeToUTF8( const wchar_t* str )
{
	char* result;
	int textlen;
	textlen = WideCharToMultiByte( CP_UTF8, 0, str, -1, NULL, 0, NULL, NULL );
	result =(char *)malloc((textlen+1)*sizeof(char));
	memset(result, 0, sizeof(char) * ( textlen + 1 ) );
	WideCharToMultiByte( CP_UTF8, 0, str, -1, result, textlen, NULL, NULL );
	return result;
}

char* WideToMulti(const wchar_t* wcs)
{
	int len;
	char* buf;
	size_t NumOfCharConvert  = 0; 
	len =wcstombs_s(&NumOfCharConvert,NULL,0,wcs,0);
	if (len == 0)
		return NULL;
	buf = (char *)malloc(sizeof(char)*(len+1));
	memset(buf, 0, sizeof(char) *(len+1));
	len =wcstombs_s(&NumOfCharConvert,buf,sizeof(buf),wcs,len+1);
	return buf;
}

wchar_t* MultitoWide(const char* mbs)
{
	int len;
	size_t NumOfCharConvert = 0;
	wchar_t* buf;
	len =mbstowcs_s(&NumOfCharConvert,NULL,0,mbs,0);
	if (len == 0)
		return NULL;
	buf = (wchar_t *)malloc(sizeof(wchar_t)*(len+1));
	memset(buf, 0, sizeof(wchar_t) *(len+1));
	len =mbstowcs_s(&NumOfCharConvert,buf,sizeof(buf),mbs,len+1);
	return buf;
}

char* ANSIToUTF8(const char* str)
{
	wchar_t * pString = ANSIToUnicode(str);
	char * pString1 = UnicodeToUTF8(pString);
	free((void*)pString);
	return pString1;
}

char* UTF8ToANSI(const char* str)
{
	wchar_t * pString = UTF8ToUnicode(str);
	char * pString1 = UnicodeToANSI(pString);
	free((void*)pString);
	return pString1;
}