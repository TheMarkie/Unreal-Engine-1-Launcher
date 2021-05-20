#include "Helper.h"

// Native hooks
#ifdef ENABLE_NATIVE_HOOKS
#include "HookHelper.h"
#include "DynamicArray.h"
#include "UnrealScriptUtilities.h"
#endif // ENABLE_NATIVE_HOOKS

#define BORDER_STYLE ( WS_CAPTION | WS_THICKFRAME )

typedef signed int UBOOL;

HWND MainWindow;
UViewport* Viewport;

// ==============================================
// Global settings
// ==============================================
TCHAR SettingsPackage[72];

UBOOL UsesBorderless;
UBOOL UsesFullScreen;

int WindowedWidth, WindowedHeight, FullScreenWidth, FullScreenHeight;

int FpsCap;
int UIScale;

bool IsFullScreen;

// ==============================================
// Init
// ==============================================
void InitHelper() {
    GLog->Logf( L"Disabled DEP: %d", ToggleDEP( 0 ) );

    FString package( appPackage() );
    package = package + L"Launcher";
    appStrcpy( SettingsPackage, *package );

    if ( !GConfig->GetBool( SettingsPackage, L"BorderlessWindowed", UsesBorderless ) ) {
        UsesBorderless = true;
        GConfig->SetBool( SettingsPackage, L"BorderlessWindowed", UsesBorderless );
    }

    if ( !GConfig->GetInt( SettingsPackage, L"FPSCap", FpsCap ) ) {
        FpsCap = 120;
        GConfig->SetInt( SettingsPackage, L"FPSCap", FpsCap );
    }
    FpsCap = Max( FpsCap, 0 );

    if ( !GConfig->GetInt( SettingsPackage, L"UIScale", UIScale ) ) {
        UIScale = 1;
        GConfig->SetInt( SettingsPackage, L"UIScale", UIScale );
    }
    UIScale = Max( UIScale, 1 );

    if ( !GConfig->GetBool( L"WinDrv.WindowsClient", L"StartupFullscreen", UsesFullScreen ) ) {
        UsesFullScreen = false;
        GConfig->SetBool( L"WinDrv.WindowsClient", L"StartupFullscreen", UsesFullScreen );
    }
    IsFullScreen = UsesFullScreen;

    int desktopWidth, desktopHeight;
    GetDesktopResolution( desktopWidth, desktopHeight );

    if ( !GConfig->GetInt( L"WinDrv.WindowsClient", L"FullscreenViewportX", FullScreenWidth ) ) {
        FullScreenWidth = desktopWidth;
        GConfig->SetInt( L"WinDrv.WindowsClient", L"FullscreenViewportX", FullScreenWidth );
    }
    if ( !GConfig->GetInt( L"WinDrv.WindowsClient", L"FullscreenViewportY", FullScreenHeight ) ) {
        FullScreenHeight = desktopHeight;
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

void InitNativeFunctions() {
#ifdef ENABLE_NATIVE_HOOKS
#ifdef DYNAMIC_ARRAY
    DynamicArray dynamicArray;
#endif // DYNAMIC_ARRAY
    UnrealScriptUtilities unrealScriptUtilities;
#endif // ENABLE_NATIVE_HOOKS
}

void CleanUpHelper() {
    GConfig->SetBool( L"WinDrv.WindowsClient", L"StartupFullscreen", IsInFullScreen() );

    if ( IsUsingBorderless() ) {
        GConfig->SetInt( TEXT( "WinDrv.WindowsClient" ), TEXT( "WindowedViewportX" ), WindowedWidth );
        GConfig->SetInt( TEXT( "WinDrv.WindowsClient" ), TEXT( "WindowedViewportY" ), WindowedHeight );
    }
}

// ==============================================
// Display Management
// ==============================================
void SetResolution( const int width, const int height, bool fullScreen ) {
    HMONITOR monitor = MonitorFromWindow( MainWindow, MONITOR_DEFAULTTONEAREST );
    MONITORINFO monitorInfo = { sizeof( MONITORINFO ) };
    GetMonitorInfoW( monitor, &monitorInfo );
    RECT monitorRect = monitorInfo.rcMonitor;

    int x = monitorRect.left;
    int y = monitorRect.top;
    int w, h;

    if ( fullScreen ) {
        FullScreenWidth = width > 0 ? width : FullScreenWidth;
        FullScreenHeight = height > 0 ? height : FullScreenHeight;

        w = FullScreenWidth;
        h = FullScreenHeight;
    }
    else {
        WindowedWidth = width > 0 ? width : WindowedWidth;
        WindowedHeight = width > 0 ? height : WindowedHeight;

        RECT rect = { 0, 0, width, height };
        LONG_PTR style = GetWindowLongPtr( MainWindow, GWL_STYLE );
        AdjustWindowRect( &rect, style, TRUE );
        int actualWidth = rect.right - rect.left;
        int actualHeight = rect.bottom - rect.top - 20;

        x += monitorRect.right - actualWidth;
        x /= 2;
        y += monitorRect.bottom - actualHeight - 22;
        y /= 2;
        w = WindowedWidth;
        h = WindowedHeight;
    }

    SetWindowPos( MainWindow, NULL, x, y, w, h, 0 );
}

void SetAllResolution( const int fullWidth, const int fullHeight, const int windowedWidth, const int windowedHeight ) {
    FullScreenWidth = fullWidth > 0 ? fullWidth : FullScreenWidth;
    FullScreenHeight = fullHeight > 0 ? fullHeight : FullScreenHeight;
    WindowedWidth = windowedWidth > 0 ? windowedWidth : WindowedWidth;
    WindowedHeight = windowedHeight > 0 ? windowedHeight : WindowedHeight;
}

void SetWindowMode( const bool borderless, int w, int h ) {
    HMONITOR monitor = MonitorFromWindow( MainWindow, MONITOR_DEFAULTTONEAREST );
    MONITORINFO monitorInfo = { sizeof( MONITORINFO ) };
    GetMonitorInfoW( monitor, &monitorInfo );
    RECT monitorRect = monitorInfo.rcMonitor;

    int x = monitorRect.left;
    int y = monitorRect.top;

    LONG_PTR style = GetWindowLongPtr( MainWindow, GWL_STYLE );
    if ( borderless ) {
        style = style & ~BORDER_STYLE;
        y += monitorRect.bottom - h;
    }
    else {
        RECT rect = { 0, 0, w, h };
        style = style | BORDER_STYLE;
        AdjustWindowRect( &rect, style, TRUE );
        w = rect.right - rect.left;
        h = rect.bottom - rect.top - 20;

        y += monitorRect.bottom - h - 22;
    }

    x += monitorRect.right - w;
    x /= 2;
    y /= 2;

    SetWindowLongPtr( MainWindow, GWL_STYLE, style );
    SetWindowPos( MainWindow, NULL, x, y, w, h, SWP_FRAMECHANGED );
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

bool IsUsingBorderless() {
    return UsesBorderless;
}
bool IsUsingFullScreen() {
    return UsesFullScreen;
}
bool IsInFullScreen() {
    return IsFullScreen;
}

// ==============================================
// Misc
// ==============================================
BOOL ToggleDEP( BOOL enable ) {
    const HMODULE module = GetModuleHandleW( L"Kernel32.dll" );
    if ( module ) {
        const auto procSet = reinterpret_cast< BOOL( WINAPI* const )( DWORD ) >( GetProcAddress( module, "SetProcessDEPPolicy" ) );
        if ( procSet ) {
            return procSet( enable );
        }
    }

    return 0;
}

void RegisterRawInput() {
    if ( !MainWindow ) {
        return;
    }

    RAWINPUTDEVICE Rid[1];

    Rid[0].usUsagePage = 0x01;
    Rid[0].usUsage = 0x02;
    Rid[0].dwFlags = 0;
    Rid[0].hwndTarget = MainWindow;

    RegisterRawInputDevices( Rid, 1, sizeof( Rid[0] ) );
}

int GetFPSCap() {
    return FpsCap;
}
void SetFPSCap( const int cap ) {
    if ( cap >= 0 ) {
        FpsCap = cap;

        GConfig->SetInt( SettingsPackage, L"FPSCap", FpsCap );
    }
}

int GetUIScale() {
    return UIScale;
}
void SetUIScale( const int n ) {
    if ( n >= 1 ) {
        UIScale = n;

        GConfig->SetInt( SettingsPackage, L"UIScale", UIScale );
    }
}
