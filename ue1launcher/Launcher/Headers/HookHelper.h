#pragma once

#ifdef ENABLE_NATIVE_HOOKS

#ifndef HOOKHELPER_H
#define HOOKHELPER_H

// ==============================================
// Native hooks
// ==============================================
#define TO_NATIVE_FUNCTION( cls, func ) reinterpret_cast< Native >( &cls##::##func )
#define DECLARE_NATIVE_FUNCTION( index, cls, func ) { index, TO_NATIVE_FUNCTION( cls, func ) }

struct NativeFunctionData {
    size_t ID;
    Native Function;

    NativeFunctionData( size_t id, Native function ) {
        ID = id;
        Function = function;
    }
};

void AddNativeFunction( size_t id, Native function );
void AddNativeFunctions( NativeFunctionData data[], size_t count );

// ==============================================
// Redirects
// ==============================================
// Courtesy of Hanfling.
void RedirectCall( void* Address, void* JumpAddress );

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

#endif // ENABLE_NATIVE_HOOKS
