#include "Helper.h"

void GetDesktopResolution( int& width, int& height ) {
	const HWND hDesktop = GetDesktopWindow();
	RECT rect;

	GetWindowRect( hDesktop, &rect );

	width = rect.right;
	height = rect.bottom;
}

void RegisterRawInput( const HWND hWnd ) {
	RAWINPUTDEVICE Rid[1];

	Rid[0].usUsagePage = 0x01;
	Rid[0].usUsage = 0x02;
	Rid[0].dwFlags = 0;
	Rid[0].hwndTarget = hWnd;

	RegisterRawInputDevices( Rid, 1, sizeof( Rid[0] ) );
}