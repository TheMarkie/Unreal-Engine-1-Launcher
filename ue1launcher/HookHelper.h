#pragma once

#ifndef HOOKHELPER_H
#define HOOKHELPER_H

// ==============================================
// Native hooks
// ==============================================
#define TO_NATIVE_FUNCTION( cls, func ) reinterpret_cast< Native >( &cls##::##func )
#define DECLARE_NATIVE_HOOK( index, cls, func ) { index, TO_NATIVE_FUNCTION( cls, func ) }

struct NativeHookData {
    size_t ID;
    Native Function;

    NativeHookData() {}

    NativeHookData( size_t id, Native function ) {
        ID = id;
        Function = function;
    }
};

void HookNativeFunction( size_t id, Native function ) {
        GNatives[id] = function;
    }

void HookNativeFunctions( NativeHookData data[], size_t count ) {
    for ( size_t i = 0; i < count; i++ ) {
        GNatives[data[i].ID] = data[i].Function;
    }
}

// PreRenderWindows hook
#include "RootWindowOverride.h"

// Dynamic Array support
#include "DynamicArray.h"

// ==============================================
// Redirects
// ==============================================
// Courtesy of Hanfling.
void RedirectCall( void* Address, void* JumpAddress ) {
    guard( RedirectCall );

    struct FRedirectCode {
        BYTE  _Pad[3];
        BYTE  Jump;
        DWORD Offset;
    };

    DWORD OldProtect;

    // Make function start writeable.
    verify( VirtualProtect( ( LPVOID ) Address, 5, PAGE_EXECUTE_READWRITE, &OldProtect ) );

    // Create replacement code.
    FRedirectCode RedirectCode;

    RedirectCode.Jump = 0xE9;
    RedirectCode.Offset = ( DWORD ) JumpAddress - ( DWORD ) Address - 5;

    // Copy in replacement.
    appMemcpy( Address, ( ( BYTE* ) &RedirectCode ) + 3, 5 );

    // Restore old protection.
    verify( VirtualProtect( ( LPVOID ) Address, 5, OldProtect, &OldProtect ) );

    // https://docs.microsoft.com/en-us/windows/desktop/api/processthreadsapi/nf-processthreadsapi-flushinstructioncache
    FlushInstructionCache( GetCurrentProcess(), NULL, 0 );

    unguard;
}

// Courtesy of Hanfling.
#define SUBSTITUTE_FUNCTION( pkg, exp, cls, func, retval, parms ) \
    {UPackage* pkg##Pkg = CastChecked< UPackage >( UObject::CreatePackage( NULL, TEXT( #pkg ) ) ); \
    check( pkg##Pkg ); \
    void* VoidPtr = NULL; \
    typedef retval ( cls::*pfn##cls##func ) parms; \
    * ( pfn##cls##func* ) &VoidPtr = &cls::func; \
    RedirectCall( pkg##Pkg->GetDllExport( exp,1 ), VoidPtr );}

// Courtesy of Hanfling.
#define SUBSTITUTE_NATIVE_FUNCTION( pkg, oldcls, oldfunc, cls, func ) \
    SUBSTITUTE_FUNCTION( pkg, TEXT( "?" ) TEXT( #oldfunc ) TEXT( "@" ) TEXT( #oldcls ) TEXT( "@@QAEXAAUFFrame@@QAX@Z" ), cls, func, void, ( FFrame&, void* const ) )

// Stub to redirect to
class Stub {
public:
    void execStub( FFrame& Stack, RESULT_DECL ) {
        // Do nothing because it's a stub.
    }
};

#endif // !HOOKHELPER_H