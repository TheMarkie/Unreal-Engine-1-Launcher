#pragma once

#include <Windows.h>
#include <windowsx.h>

void InitHelper();
void SetMainWindow( const HWND );
void CleanUpHelper();

void SetResolution( const int, const int, bool );
void SetWindowMode( const bool, int, int );
void ToggleWindowMode( const bool );
void ToggleWindowMode();

void RegisterRawInput();

HWND GetMainWindow();
int GetFPSCap();
bool UsesBorderless();
bool UsesFullScreen();
bool IsFullScreen();

void GetDesktopResolution( int&, int& );
bool StringToResolution( const TCHAR*, int&, int& );
