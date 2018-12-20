#pragma once

// ==============================================
// Markie: Base native hook class.
// ==============================================
class NativeHook {
public:
	NativeHook( size_t id, Native nf ) {
		GNatives[id] = nf;
	}
};

// ==============================================
// Markie: PreRenderWindows hook.
// ==============================================
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