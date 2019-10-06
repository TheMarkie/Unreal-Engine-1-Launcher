#pragma once

#ifndef HOOKHELPER_H
#define HOOKHELPER_H

#include "RootWindowOverride.h"

// ==============================================
// Markie: Native hooks
// ==============================================
// Markie: Base native hook class
class NativeHook {
public:
	NativeHook( size_t id, Native nf ) {
		GNatives[id] = nf;
	}
};

// Markie: PreRenderWindows hook
class PRWHook : NativeHook {
public:
	PRWHook() : NativeHook( EXTENSION_PreRenderWindows, reinterpret_cast< Native >( &PRWHook::execPreRenderWindows ) ) {};

	void execPreRenderWindows( FFrame& Stack, RESULT_DECL ) {
		UNREFERENCED_PARAMETER( Result );

		P_GET_OBJECT( UCanvas, canvas );
		P_FINISH;

		PreRenderWindows( canvas );
	}
private:
	void PreRenderWindows( UCanvas* canvas ) {
		APlayerPawnExt* pp = static_cast< APlayerPawnExt* >( canvas->Viewport->Actor );
		RootWindowOverride* rwo = static_cast< RootWindowOverride* >( pp->rootWindow );

		rwo->SetScale( GetUIScale(), canvas );
	}
};

// ==============================================
// Markie: Redirects
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