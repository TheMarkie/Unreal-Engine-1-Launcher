#pragma once

#define P_GET_FARRAY_REF( var ) FArray var##T; GPropAddr = 0; Stack.Step( Stack.Object, &var##T ); FArray* var = GPropAddr ? ( FArray* )GPropAddr : &var##T;
#define P_GET_TARRAY_REF( type, var ) TArray<type> var##T; GPropAddr = 0; Stack.Step( Stack.Object, &var##T ); TArray<type>* var = GPropAddr ? ( TArray<type>* ) GPropAddr : &var##T;

class DynamicArray {
    enum DynamicArrayFunction {
        ArrayCount = 3200
    };

public:
    DynamicArray() {
        NativeFunctionData data[] = {
            DECLARE_NATIVE_FUNCTION( EX_DynArrayElement, DynamicArray, execDynArrayElement ),
            DECLARE_NATIVE_FUNCTION( ArrayCount, DynamicArray, execArrayCount ),
        };

        AddNativeFunctions( data, 2 );
    }

    // Hanfling's code with my modification
    void execDynArrayElement( FFrame& Stack, RESULT_DECL ) {
        P_GET_INT( Index );
        P_GET_FARRAY_REF( Array );

        checkSlow( GProperty );
        checkSlow( GProperty->IsA( UArrayProperty::StaticClass() ) );
        UArrayProperty* Property = ( UArrayProperty* ) GProperty;

        INT ArrayNum = Array->Num();
        INT ElementSize = Property->Inner->ElementSize;

        // Result is true means we're reading from the array.
        if ( Result ) {
            bool isValidIndex = Array->IsValidIndex( Index );

            // Handle specially reserved indexes.
            switch ( Index ) {
                // -1: Return the topmost value.
                case -1:
                    Index = ArrayNum - 1;
                    isValidIndex = Array->IsValidIndex( Index );

                    break;
                // -2: Return the default value.
                case -2:
                    // Empty beccause this is already the default behaviour, we're just omitting the warning.
                    break;
                default:
                    if ( !isValidIndex ) {
                        Stack.Logf( TEXT( "Dynamic array index out of bounds (%i/%i)" ), Index, ArrayNum );
                    }

                    break;
            }

            if ( isValidIndex ) {
                GPropAddr = ( ( BYTE* ) Array->GetData() ) + ( Index * ElementSize );
                Property->Inner->CopyCompleteValue( Result, GPropAddr );
            }
            else {
                // Han's note: We should create a default value of the respective type and return it otherwise we would leak mem.
                BYTE* Element = new BYTE[ElementSize];
                appMemzero( Element, ElementSize );
                Property->Inner->CopyCompleteValue( Result, Element );
                delete Element;
            }
        }
        // We're assigning to the array.
        else {
            // Handle specially reserved indexes.
            switch ( Index ) {
                // -1: Add a new item on top.
                case -1:
                    Index = ArrayNum;

                    break;
                // -2: Empty the array.
                case -2:
                    Array->Empty( ElementSize );

                    return;
            }

            if ( Index >= 0 ) {
                // Resize the array as needed.
                if ( Index >= ArrayNum ) {
                    Array->AddZeroed( ElementSize, Index + 1 - ArrayNum );
                }

                GPropAddr = ( ( BYTE* ) Array->GetData() ) + ( Index * ElementSize );
            }
            else {
                Stack.Logf( TEXT( "Dynamic array index cannot be negative (%i)" ), Index );

                GPropAddr = NULL;
            }
        }
    }

    void execArrayCount( FFrame& Stack, RESULT_DECL ) {
        P_GET_TARRAY_REF( INT, Array )
        P_FINISH;

        *( INT* ) Result = Array->Num();
    }
};