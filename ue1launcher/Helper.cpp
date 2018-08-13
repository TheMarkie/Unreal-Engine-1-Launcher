#include "Helper.h"

void GetDesktopResolution( int& width, int& height ) {
	const HWND hDesktop = GetDesktopWindow();
	RECT rect;

	GetWindowRect( hDesktop, &rect );

	width = rect.right;
	height = rect.bottom;
}
