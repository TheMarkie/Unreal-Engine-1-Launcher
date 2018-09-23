#include "Helper.h"

#include <sstream>

#include <Core.h>

#define BORDER_STYLE ( WS_CAPTION | WS_THICKFRAME )

typedef signed int UBOOL;

UBOOL usesBorderless;
UBOOL usesFullScreen;

int fpsCap;
int wW, wH, fW, fH;

int dW, dH;

HWND mainWnd;

bool isFullScreen;

void InitHelper() {
	GConfig->GetBool( appPackage(), L"BorderlessWindowed", usesBorderless );
	GConfig->GetBool( L"WinDrv.WindowsClient", L"StartupFullscreen", usesFullScreen );

	GConfig->GetInt( appPackage(), L"FPSCap", fpsCap );

	GConfig->GetInt( L"WinDrv.WindowsClient", L"FullscreenViewportX", fW );
	GConfig->GetInt( L"WinDrv.WindowsClient", L"FullscreenViewportY", fH );
	GConfig->GetInt( L"WinDrv.WindowsClient", L"WindowedViewportX", wW );
	GConfig->GetInt( L"WinDrv.WindowsClient", L"WindowedViewportY", wH );

	isFullScreen = usesFullScreen;

	GetDesktopResolution( dW, dH );
}

void SetMainWindow( const HWND hWnd ) {
	mainWnd = hWnd;
}

void CleanUpHelper() {
	GConfig->SetBool( L"WinDrv.WindowsClient", L"StartupFullscreen", IsFullScreen() );

	if ( UsesBorderless() ) {
		GConfig->SetInt( TEXT( "WinDrv.WindowsClient" ), TEXT( "WindowedViewportX" ), wW );
		GConfig->SetInt( TEXT( "WinDrv.WindowsClient" ), TEXT( "WindowedViewportY" ), wH );
	}
}

void SetResolution( const int w, const int h, bool fullScreen ) {
	if ( fullScreen ) {
		fW = w > 0 ? w : fW;
		fH = h > 0 ? h : fH;

		int offsetX, offsetY;
		offsetX = max( ( dW - w ) / 2, 0 );
		offsetY = max( ( dH - h ) / 2, 0 );

		SetWindowPos( mainWnd, NULL, offsetX, offsetY, fW, fH, 0 );
	}
	else {
		wW = w > 0 ? w : wW;
		wH = h > 0 ? h : wH;
	}
}

void SetWindowMode( const bool borderless, int w, int h ) {
	LONG_PTR style = GetWindowLongPtr( mainWnd, GWL_STYLE );
	if ( borderless ) {
		style = style & ~BORDER_STYLE;
	}
	else {
		style = style | BORDER_STYLE;

		RECT rect = { 0, 0, w, h };
		AdjustWindowRect( &rect, style, TRUE );

		w = rect.right - rect.left;
		h = rect.bottom - rect.top - 20;
	}

	int offsetX, offsetY;
	offsetX = Max( ( dW - w ) / 2, 0 );
	offsetY = Max( ( dH - h ) / 2, 0 );

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

HWND GetMainWindow() {
	return mainWnd;
}

int GetFPSCap() {
	return fpsCap;
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