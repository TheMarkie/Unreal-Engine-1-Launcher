#include "Helper.h"
#include "HookHelper.h"

#define BORDER_STYLE ( WS_CAPTION | WS_THICKFRAME )

typedef signed int UBOOL;

HWND mainWnd;
UViewport* vp;

// ==============================================
// Global settings
// ==============================================
TCHAR settingsPackage[72];

UBOOL usesBorderless;
UBOOL usesFullScreen;

int fpsCap;
int wW, wH, fW, fH;

int dW, dH;

int scale;

bool isFullScreen;

bool resolutionChanged;

// ==============================================
// Functions to set stuff
// ==============================================
void InitHelper() {
    // Redirect all the necessary functions before we do anything.
    InitRedirects();

    FString package( appPackage() );
    package = package + L"Launcher";
    appStrcpy( settingsPackage, *package );

    if ( !GConfig->GetBool( settingsPackage, L"BorderlessWindowed", usesBorderless ) ) {
        usesBorderless = true;

        GConfig->SetBool( settingsPackage, L"BorderlessWindowed", usesBorderless );
    }
    GConfig->GetBool( L"WinDrv.WindowsClient", L"StartupFullscreen", usesFullScreen );

    if ( !GConfig->GetInt( settingsPackage, L"FPSCap", fpsCap ) ) {
        fpsCap = 120;

        GConfig->SetInt( settingsPackage, L"FPSCap", fpsCap );
    }
    fpsCap = Max( fpsCap, 0 );

    if ( !GConfig->GetInt( settingsPackage, L"UIScale", scale ) ) {
        scale = 1;

        GConfig->SetInt( settingsPackage, L"UIScale", scale );
    }
    scale = Max( scale, 1 );

    GConfig->GetInt( L"WinDrv.WindowsClient", L"FullscreenViewportX", fW );
    GConfig->GetInt( L"WinDrv.WindowsClient", L"FullscreenViewportY", fH );
    GConfig->GetInt( L"WinDrv.WindowsClient", L"WindowedViewportX", wW );
    GConfig->GetInt( L"WinDrv.WindowsClient", L"WindowedViewportY", wH );

    isFullScreen = usesFullScreen;

    GetDesktopResolution( dW, dH );

    // Override native functions.
    InitNativeFunctions();
}

void InitRedirects() {
    // UCommandlet
    SUBSTITUTE_NATIVE_FUNCTION( Core, UCommandlet, execMain, Stub, execStub );

    // AStatLog
    SUBSTITUTE_NATIVE_FUNCTION( Engine, AStatLog, execGetMapFileName, Stub, execStub );
    SUBSTITUTE_NATIVE_FUNCTION( Engine, AStatLog, execGetGMTRef, Stub, execStub );
    SUBSTITUTE_NATIVE_FUNCTION( Engine, AStatLog, execGetPlayerChecksum, Stub, execStub );
    SUBSTITUTE_NATIVE_FUNCTION( Engine, AStatLog, execLogMutator, Stub, execStub );
    SUBSTITUTE_NATIVE_FUNCTION( Engine, AStatLog, execInitialCheck, Stub, execStub );
    SUBSTITUTE_NATIVE_FUNCTION( Engine, AStatLog, execBrowseRelativeLocalURL, Stub, execStub );
    SUBSTITUTE_NATIVE_FUNCTION( Engine, AStatLog, execExecuteWorldLogBatcher, Stub, execStub );
    SUBSTITUTE_NATIVE_FUNCTION( Engine, AStatLog, execBatchLocal, Stub, execStub );
    SUBSTITUTE_NATIVE_FUNCTION( Engine, AStatLog, execExecuteSilentLogBatcher, Stub, execStub );
    SUBSTITUTE_NATIVE_FUNCTION( Engine, AStatLog, execExecuteLocalLogBatcher, Stub, execStub );

    // AStatLogFile
    SUBSTITUTE_NATIVE_FUNCTION( Engine, AStatLogFile, execFileLog, Stub, execStub );
    SUBSTITUTE_NATIVE_FUNCTION( Engine, AStatLogFile, execFileFlush, Stub, execStub );
    SUBSTITUTE_NATIVE_FUNCTION( Engine, AStatLogFile, execGetChecksum, Stub, execStub );
    SUBSTITUTE_NATIVE_FUNCTION( Engine, AStatLogFile, execWatermark, Stub, execStub );
    SUBSTITUTE_NATIVE_FUNCTION( Engine, AStatLogFile, execCloseLog, Stub, execStub );
    SUBSTITUTE_NATIVE_FUNCTION( Engine, AStatLogFile, execOpenLog, Stub, execStub );
}

void InitNativeFunctions() {
    PreRenderWindowsHook preRenderWindowsHook;
    DynamicArray dynamicArray;
}

void CleanUpHelper() {
    GConfig->SetBool( L"WinDrv.WindowsClient", L"StartupFullscreen", IsFullScreen() );

    if ( UsesBorderless() ) {
        GConfig->SetInt( TEXT( "WinDrv.WindowsClient" ), TEXT( "WindowedViewportX" ), wW );
        GConfig->SetInt( TEXT( "WinDrv.WindowsClient" ), TEXT( "WindowedViewportY" ), wH );
    }
}

void SetResolution( const int w, const int h, bool fullScreen ) {
    int offsetX, offsetY;

    if ( fullScreen ) {
        fW = w > 0 ? w : fW;
        fH = h > 0 ? h : fH;

        offsetX = max( ( dW - w ) / 2, 0 );
        offsetY = max( ( dH - h ) / 2, 0 );

        SetWindowPos( mainWnd, NULL, offsetX, offsetY, fW, fH, 0 );
    }
    else {
        wW = w > 0 ? w : wW;
        wH = h > 0 ? h : wH;

        RECT rect = { 0, 0, w, h };
        LONG_PTR style = GetWindowLongPtr( mainWnd, GWL_STYLE );

        AdjustWindowRect( &rect, style, TRUE );

        int aW = rect.right - rect.left;
        int aH = rect.bottom - rect.top - 20;

        offsetX = max( ( dW - aW ) / 2, 0 );
        offsetY = max( ( dH - aH - 22 ) / 2, 0 );

        SetWindowPos( mainWnd, NULL, offsetX, offsetY, wW, wH, 0 );
    }
}

void SetWindowMode( const bool borderless, int w, int h ) {
    int offsetX, offsetY;

    LONG_PTR style = GetWindowLongPtr( mainWnd, GWL_STYLE );
    if ( borderless ) {
        style = style & ~BORDER_STYLE;

        offsetY = Max( ( dH - h ) / 2, 0 );
    }
    else {
        style = style | BORDER_STYLE;

        RECT rect = { 0, 0, w, h };
        AdjustWindowRect( &rect, style, TRUE );

        w = rect.right - rect.left;
        h = rect.bottom - rect.top - 20;

        offsetY = Max( ( dH - h - 22 ) / 2, 0 );
    }

    offsetX = Max( ( dW - w ) / 2, 0 );

    SetWindowLongPtr( mainWnd, GWL_STYLE, style );
    SetWindowPos( mainWnd, NULL, offsetX, offsetY, w, h, SWP_FRAMECHANGED );
}

void ToggleWindowMode( const bool fullScreen ) {
    if ( fullScreen ) {
        SetWindowMode( true, fW, fH );
    }
    else {
        SetWindowMode( false, wW, wH );
    }

    isFullScreen = fullScreen;
}

void ToggleWindowMode() {
    ToggleWindowMode( !IsFullScreen() );
}

void SetFPSCap( const int cap ) {
    if ( cap >= 0 ) {
        fpsCap = cap;

        GConfig->SetInt( settingsPackage, L"FPSCap", fpsCap );
    }
}

void SetUIScale( const int n ) {
    if ( n >= 1 ) {
        scale = n;

        GConfig->SetInt( settingsPackage, L"UIScale", scale );
    }
}

// ==============================================
// Miscellaneous functions.
// ==============================================
void RegisterRawInput() {
    if ( !mainWnd ) {
        return;
    }

    RAWINPUTDEVICE Rid[1];

    Rid[0].usUsagePage = 0x01;
    Rid[0].usUsage = 0x02;
    Rid[0].dwFlags = 0;
    Rid[0].hwndTarget = mainWnd;

    RegisterRawInputDevices( Rid, 1, sizeof( Rid[0] ) );
}

// ==============================================
// Functions to get stuff
// ==============================================
int GetFPSCap() {
    return fpsCap;
}

int GetUIScale() {
    return scale;
}

bool UsesBorderless() {
    return usesBorderless;
}

bool UsesFullScreen() {
    return usesFullScreen;
}

bool IsFullScreen() {
    return isFullScreen;
}

void GetDesktopResolution( int& width, int& height ) {
    const HWND hDesktop = GetDesktopWindow();
    RECT rect;

    GetWindowRect( hDesktop, &rect );

    width = rect.right;
    height = rect.bottom;
}

bool StringToResolution( const TCHAR* str, int& w, int& h ) {
    FString fstr( str );
    FString fstrW = fstr.Left( fstr.InStr( L"x" ) );
    FString fstrH = fstr.Right( fstr.Len() - fstr.InStr( L"x" ) - 1 );

    w = -1;
    h = -1;

    w = appAtoi( *fstrW );
    h = appAtoi( *fstrH );

    return ( w > 0 && h > 0 );
}
