#pragma once

#pragma warning( disable : 4595 )

#ifndef HELPER_H
#define HELPER_H

#include <Windows.h>
#include <windowsx.h>

#include <Engine.h>
#include <Extension.h>

extern UViewport* vp;
extern HWND mainWnd;

void InitHelper();
void InitRedirects();
void InitNativeHooks();
void CleanUpHelper();

void SetResolution( const int, const int, bool );
void SetWindowMode( const bool, int, int );
void ToggleWindowMode( const bool );
void ToggleWindowMode();
void SetFPSCap( const int );
void SetUIScale( const int );

void RegisterRawInput();

int GetFPSCap();
int GetUIScale();
bool UsesBorderless();
bool UsesFullScreen();
bool IsFullScreen();

void GetDesktopResolution( int&, int& );
bool StringToResolution( const TCHAR*, int&, int& );

#endif // !HELPER_H
