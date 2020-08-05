#pragma once

#pragma warning( disable : 4595 )

#ifndef HELPER_H
#define HELPER_H

#include <Windows.h>
#include <windowsx.h>

#include <Engine.h>
#include <Extension.h>

extern HWND MainWindow;
extern UViewport* Viewport;

void InitHelper();
void InitRedirects();
void InitNativeFunctions();
void CleanUpHelper();

BOOL ToggleDEP( BOOL );

void SetResolution( const int, const int, bool );
void SetAllResolution( const int, const int, const int, const int );
void SetWindowMode( const bool, int, int );
void ToggleWindowMode( const bool );
void ToggleWindowMode();
void SetFPSCap( const int );
void SetUIScale( const int );

void RegisterRawInput();

int GetFPSCap();
int GetUIScale();
bool IsUsingBorderless();
bool IsUsingFullScreen();
bool IsInFullScreen();

void GetDesktopResolution( int&, int& );
bool StringToResolution( const TCHAR*, int&, int& );

#endif // !HELPER_H
