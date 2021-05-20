#include <stdio.h>
#include <sstream>
#include <iomanip>

#include "Helper.h"
#include "HookHelper.h"
#include "UnrealScriptUtilities.h"

void UnrealScriptUtilities::execSprintf( FFrame& Stack, RESULT_DECL ) {
    P_GET_STR( format );
    P_GET_STR( s0 );
    P_GET_STR( s1 );
    P_GET_STR( s2 );
    P_GET_STR( s3 );
    P_FINISH;

    int length = format.Len();
    TCHAR* buffer = new TCHAR[length];
    swprintf( buffer, length, *format, *s0, *s1, *s2, *s3 );

    *( FString* ) Result = FString( buffer );
}

void UnrealScriptUtilities::execFormatFloat( FFrame& Stack, RESULT_DECL ) {
    P_GET_FLOAT( value );
    P_GET_INT_OPTX( precision, 1 );
    P_FINISH;

    *( FString* ) Result = FormatFloat( value, precision );
}

FString UnrealScriptUtilities::FormatFloat( FLOAT value, INT precision ) {
    std::wstringstream stream;
    stream << std::fixed << std::setprecision( precision ) << value;
    return FString( stream.str().c_str() );
}
