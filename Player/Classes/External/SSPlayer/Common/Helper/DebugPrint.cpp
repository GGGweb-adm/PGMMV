#include "DebugPrint.h"
#include <stdio.h>  
#include <string>
#include <iostream>


#if _WIN32
	#include <Windows.h>
#endif

namespace ss
{


void DEBUG_PRINTF( const char* strFormat, ...   )
{
	char strBuffer[1024];

	va_list arglist;
	va_start( arglist, strFormat);
#if _WIN32
	vsprintf_s(strBuffer, 1024, strFormat, arglist);
	va_end(arglist);
	OutputDebugStringA(strBuffer);
#else
	vsnprintf(strBuffer, 1024, strFormat, arglist);
	va_end(arglist);
#endif // win32

	std::cerr << strBuffer << "\n";


}

void	THROW_ERROR_MESSAGE_MAIN( std::string str , char* fname , size_t line )
{
	char	___str__buffer[1024];
#if _WIN32
	sprintf_s( ___str__buffer , 1024,  "%s(%d) : %s \n" , fname , (int)line , str.c_str() );
#else
    sprintf( ___str__buffer , "%s(%d) : %s \n" , fname , (int)line , str.c_str() );
#endif // win32
	std::string ___err_message = ___str__buffer;

	DEBUG_PRINTF( ___str__buffer );
	throw ThrowErrorMessage( 0 , ___err_message );
}
};
