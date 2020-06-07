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

UBOOL UsesBorderless;
UBOOL UsesFullScreen;

int FpsCap;
int WindowedWidth, WindowedHeight, FullScreenWidth, FullScreenHeight;

int DesktopWidth, DesktopHeight;

int UIScale;

bool IsFullScreen;

bool ResolutionChanged;

// ==============================================
// Functions to set stuff
// ==============================================
void InitHelper() {
    // Redirect all the necessary functions before we do anything.
    InitRedirects();

    FString package( appPackage() );
    package = package + L"Launcher";
    appStrcpy( settingsPackage, *package );

    if ( !GConfig->GetBool( settingsPackage, L"BorderlessWindowed", UsesBorderless ) ) {
        UsesBorderless = true;
        GConfig->SetBool( settingsPackage, L"BorderlessWindowed", UsesBorderless );
    }

    if ( !GConfig->GetInt( settingsPackage, L"FPSCap", FpsCap ) ) {
        FpsCap = 120;
        GConfig->SetInt( settingsPackage, L"FPSCap", FpsCap );
    }
    FpsCap = Max( FpsCap, 0 );

    if ( !GConfig->GetInt( settingsPackage, L"UIScale", UIScale ) ) {
        UIScale = 1;
        GConfig->SetInt( settingsPackage, L"UIScale", UIScale );
    }
    UIScale = Max( UIScale, 1 );

    if ( !GConfig->GetBool( L"WinDrv.WindowsClient", L"StartupFullscreen", UsesFullScreen ) ) {
        UsesFullScreen = false;
        GConfig->SetBool( L"WinDrv.WindowsClient", L"StartupFullscreen", UsesFullScreen );
    }
    IsFullScreen = UsesFullScreen;

    GetDesktopResolution( DesktopWidth, DesktopHeight );

    if ( !GConfig->GetInt( L"WinDrv.WindowsClient", L"FullscreenViewportX", FullScreenWidth ) ) {
        FullScreenWidth = DesktopWidth;
        GConfig->SetInt( L"WinDrv.WindowsClient", L"FullscreenViewportX", FullScreenWidth );
    }
    if ( !GConfig->GetInt( L"WinDrv.WindowsClient", L"FullscreenViewportY", FullScreenHeight ) ) {
        FullScreenHeight = DesktopHeight;
        GConfig->SetInt( L"WinDrv.WindowsClient", L"FullscreenViewportY", FullScreenHeight );
    }
    if ( !GConfig->GetInt( L"WinDrv.WindowsClient", L"WindowedViewportX", WindowedWidth ) ) {
        WindowedWidth = 1280;
        GConfig->GetInt( L"WinDrv.WindowsClient", L"WindowedViewportX", WindowedWidth );
    }
    if ( !GConfig->GetInt( L"WinDrv.WindowsClient", L"WindowedViewportY", WindowedHeight ) ) {
        WindowedHeight = 720;
        GConfig->GetInt( L"WinDrv.WindowsClient", L"WindowedViewportY", WindowedHeight );
    }

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
    GConfig->SetBool( L"WinDrv.WindowsClient", L"StartupFullscreen", IsInFullScreen() );

    if ( IsUsingBorderless() ) {
        GConfig->SetInt( TEXT( "WinDrv.WindowsClient" ), TEXT( "WindowedViewportX" ), WindowedWidth );
        GConfig->SetInt( TEXT( "WinDrv.WindowsClient" ), TEXT( "WindowedViewportY" ), WindowedHeight );
    }
}

void SetResolution( const int width, const int height, bool fullScreen ) {
    int offsetX, offsetY;

    if ( fullScreen ) {
        FullScreenWidth = width > 0 ? width : FullScreenWidth;
        FullScreenHeight = height > 0 ? height : FullScreenHeight;

        offsetX = Max( ( DesktopWidth - width ) / 2, 0 );
        offsetY = Max( ( DesktopHeight - height ) / 2, 0 );

        SetWindowPos( mainWnd, NULL, offsetX, offsetY, FullScreenWidth, FullScreenHeight, 0 );
    }
    else {
        WindowedWidth = width > 0 ? width : WindowedWidth;
        WindowedHeight = width > 0 ? height : WindowedHeight;

        RECT rect = { 0, 0, width, height };
        LONG_PTR style = GetWindowLongPtr( mainWnd, GWL_STYLE );

        AdjustWindowRect( &rect, style, TRUE );

        int actualWidth = rect.right - rect.left;
        int actualHeight = rect.bottom - rect.top - 20;

        offsetX = Max( ( DesktopWidth - actualWidth ) / 2, 0 );
        offsetY = Max( ( DesktopHeight - actualHeight - 22 ) / 2, 0 );

        SetWindowPos( mainWnd, NULL, offsetX, offsetY, WindowedWidth, WindowedHeight, 0 );
    }
}

void SetWindowMode( const bool borderless, int w, int h ) {
    int offsetX, offsetY;

    LONG_PTR style = GetWindowLongPtr( mainWnd, GWL_STYLE );
    if ( borderless ) {
        style = style & ~BORDER_STYLE;

        offsetY = Max( ( DesktopHeight - h ) / 2, 0 );
    }
    else {
        style = style | BORDER_STYLE;

        RECT rect = { 0, 0, w, h };
        AdjustWindowRect( &rect, style, TRUE );

        w = rect.right - rect.left;
        h = rect.bottom - rect.top - 20;

        offsetY = Max( ( DesktopHeight - h - 22 ) / 2, 0 );
    }

    offsetX = Max( ( DesktopWidth - w ) / 2, 0 );

    SetWindowLongPtr( mainWnd, GWL_STYLE, style );
    SetWindowPos( mainWnd, NULL, offsetX, offsetY, w, h, SWP_FRAMECHANGED );
}

void ToggleWindowMode( const bool fullScreen ) {
    if ( fullScreen ) {
        SetWindowMode( true, FullScreenWidth, FullScreenHeight );
    }
    else {
        SetWindowMode( false, WindowedWidth, WindowedHeight );
    }

    IsFullScreen = fullScreen;
}

void ToggleWindowMode() {
    ToggleWindowMode( !IsInFullScreen() );
}

void SetFPSCap( const int cap ) {
    if ( cap >= 0 ) {
        FpsCap = cap;

        GConfig->SetInt( settingsPackage, L"FPSCap", FpsCap );
    }
}

void SetUIScale( const int n ) {
    if ( n >= 1 ) {
        UIScale = n;

        GConfig->SetInt( settingsPackage, L"UIScale", UIScale );
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
    return FpsCap;
}

int GetUIScale() {
    return UIScale;
}

bool IsUsingBorderless() {
    return UsesBorderless;
}

bool IsUsingFullScreen() {
    return UsesFullScreen;
}

bool IsInFullScreen() {
    return IsFullScreen;
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
