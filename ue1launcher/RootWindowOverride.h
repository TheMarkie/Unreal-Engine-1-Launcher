#pragma once

class RootWindowOverride : public XRootWindow {
public:
    // Lifted from kentie's.
    void SetScale( const int scale, UCanvas* canvas ) {
        float x = canvas->X;
        float y = canvas->Y;
        bool resolutionChanged = ( ( x / hMultiplier ) != width ) || ( ( y / vMultiplier ) != height );

        if ( resolutionChanged || scale != hMultiplier || scale != vMultiplier ) {
            ResizeRoot( canvas );

            const float fScale = static_cast< float >( scale );
            width = width * ( hMultiplier / fScale );
            height = height * ( vMultiplier / fScale );

            clipRect.clipWidth = width;
            clipRect.clipHeight = height;

            winGC->SetClipRect( clipRect );

            hMultiplier = scale;
            vMultiplier = scale;

            for ( XWindow* childWnd = GetBottomChild(); childWnd != nullptr; childWnd = childWnd->GetHigherSibling() ) {
                childWnd->Hide();
                childWnd->Show();
            }
        }
    }
};

class PreRenderWindowsHook {
public:
    PreRenderWindowsHook() {
        AddNativeFunction( EXTENSION_PreRenderWindows, TO_NATIVE_FUNCTION( PreRenderWindowsHook, execPreRenderWindows ) );
    }

    void execPreRenderWindows( FFrame& Stack, RESULT_DECL ) {
        UNREFERENCED_PARAMETER( Result );

        P_GET_OBJECT( UCanvas, canvas )
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
