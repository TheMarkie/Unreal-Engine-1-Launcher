#include "Helper.h"
#include "HookHelper.h"

// ==============================================
// Native hooks
// ==============================================
void AddNativeFunction( size_t id, Native function ) {
    GNatives[id] = function;
}

void AddNativeFunctions( NativeFunctionData data[], size_t count ) {
    for ( size_t i = 0; i < count; i++ ) {
        GNatives[data[i].ID] = data[i].Function;
    }
}

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
