#pragma once

class UnrealScriptUtilities {
    enum UnrealScriptUtilitiesFunctionId {
        Id_Sprintf = 2400,
        Id_FormatFloat = 2401
    };

public:
    UnrealScriptUtilities() {
        NativeFunctionData data[] = {
            DECLARE_NATIVE_FUNCTION( Id_Sprintf, UnrealScriptUtilities, execSprintf ),
            DECLARE_NATIVE_FUNCTION( Id_FormatFloat, UnrealScriptUtilities, execFormatFloat )
        };

        AddNativeFunctions( data, 2 );
    }

    void execSprintf( FFrame& Stack, RESULT_DECL );
    void execFormatFloat( FFrame& Stack, RESULT_DECL );

    FString FormatFloat( FLOAT value, INT precision );
};
