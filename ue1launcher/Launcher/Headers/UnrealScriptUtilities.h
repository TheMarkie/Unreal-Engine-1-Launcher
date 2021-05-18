#pragma once

#include <stdio.h>

class UnrealScriptUtilities {
    enum UnrealScriptUtilitiesFunction {
        Sprintf = 2300
    };

public:
    UnrealScriptUtilities() {
        NativeFunctionData data[] = {
            DECLARE_NATIVE_FUNCTION( Sprintf, UnrealScriptUtilities, execSprintf )
        };

        AddNativeFunctions( data, 1 );
    }

    void execSprintf( FFrame& Stack, RESULT_DECL ) {
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
};
