/*=============================================================================
    UnEngineWin.h: Unreal engine windows-specific code.
    Copyright 1997-1999 Epic Games, Inc. All Rights Reserved.

Revision history:
    * Created by Tim Sweeney.
=============================================================================*/

#pragma DISABLE_OPTIMIZATION /* Avoid VC++ code generation bug */

// Markie
#include "Helper.h"

/*-----------------------------------------------------------------------------
    Splash screen.
-----------------------------------------------------------------------------*/

//
// Splash screen, implemented with old-style Windows code so that it
// can be opened super-fast before initialization.
//
BOOL CALLBACK SplashDialogProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
    if( uMsg==WM_DESTROY )
        PostQuitMessage(0);
    return 0;
}
HWND    hWndSplash = NULL;
HBITMAP hBitmap    = NULL;
INT     BitmapX    = 0;
INT     BitmapY    = 0;
DWORD   ThreadId   = 0;
HANDLE  hThread    = 0;
DWORD WINAPI ThreadProc( VOID* Parm )
{
    hWndSplash = TCHAR_CALL_OS(CreateDialogW(hInstance,MAKEINTRESOURCEW(IDDIALOG_Splash), NULL, SplashDialogProc),CreateDialogA(hInstance, MAKEINTRESOURCEA(IDDIALOG_Splash), NULL, SplashDialogProc) );
    if( hWndSplash )
    {
        HWND hWndLogo = GetDlgItem(hWndSplash,IDC_Logo);
        if( hWndLogo )
        {
            SetWindowPos(hWndSplash,HWND_TOPMOST,(GetSystemMetrics(SM_CXSCREEN)-BitmapX)/2,(GetSystemMetrics(SM_CYSCREEN)-BitmapY)/2,BitmapX,BitmapY,SWP_SHOWWINDOW);
            SetWindowPos(hWndSplash,HWND_NOTOPMOST,0,0,0,0,SWP_NOMOVE|SWP_NOSIZE);
            SendMessageX( hWndLogo, STM_SETIMAGE, IMAGE_BITMAP, (LPARAM)hBitmap );
            UpdateWindow( hWndSplash );
            MSG Msg;
            while( TCHAR_CALL_OS(GetMessageW(&Msg,NULL,0,0),GetMessageA(&Msg,NULL,0,0)) )
                DispatchMessageX(&Msg);
        }
    }
    return 0;
}
void InitSplash( const TCHAR* Filename )
{
    FWindowsBitmap Bitmap(1);
    if( Filename )
    {
        verify(Bitmap.LoadFile(Filename) );
        hBitmap = Bitmap.GetBitmapHandle();
        BitmapX = Bitmap.SizeX;
        BitmapY = Bitmap.SizeY;
    }
    hThread=CreateThread(NULL,0,&ThreadProc,NULL,0,&ThreadId);
}
void ExitSplash()
{
    if( ThreadId )
        TCHAR_CALL_OS(PostThreadMessageW(ThreadId,WM_QUIT,0,0),PostThreadMessageA(ThreadId,WM_QUIT,0,0));
}

/*-----------------------------------------------------------------------------
    System Directories.
-----------------------------------------------------------------------------*/

TCHAR SysDir[256]=TEXT(""), WinDir[256]=TEXT(""), ThisFile[256]=TEXT("");
void InitSysDirs()
{
#if UNICODE
    if( !GUnicodeOS )
    {
        ANSICHAR ASysDir[256]="", AWinDir[256]="", AThisFile[256]="";
        GetSystemDirectoryA( ASysDir, ARRAY_COUNT(ASysDir) );
        GetWindowsDirectoryA( AWinDir, ARRAY_COUNT(AWinDir) );
        GetModuleFileNameA( NULL, AThisFile, ARRAY_COUNT(AThisFile) );
        appStrcpy( SysDir, ANSI_TO_TCHAR(ASysDir) );
        appStrcpy( WinDir, ANSI_TO_TCHAR(AWinDir) );
        appStrcpy( ThisFile, ANSI_TO_TCHAR(AThisFile) );
    }
    else
#endif
    {
        GetSystemDirectory( SysDir, ARRAY_COUNT(SysDir) );
        GetWindowsDirectory( WinDir, ARRAY_COUNT(WinDir) );
        GetModuleFileName( NULL, ThisFile, ARRAY_COUNT(ThisFile) );
    }
    if( !appStricmp( &ThisFile[appStrlen(ThisFile) - 4], TEXT(".ICD") ) )
        appStrcpy( &ThisFile[appStrlen(ThisFile) - 4], TEXT(".EXE") );
}

/*-----------------------------------------------------------------------------
    Config wizard.
-----------------------------------------------------------------------------*/

class WConfigWizard : public WWizardDialog
{
    DECLARE_WINDOWCLASS(WConfigWizard,WWizardDialog,Startup)
    WLabel LogoStatic;
    FWindowsBitmap LogoBitmap;
    UBOOL Cancel;
    FString Title;
    WConfigWizard()
    : LogoStatic(this,IDC_Logo)
    , Cancel(0)
    {}
    void OnInitDialog()
    {
        guard(WStartupWizard::OnInitDialog);
        WWizardDialog::OnInitDialog();
        SendMessageX( *this, WM_SETICON, ICON_BIG, (WPARAM)LoadIconIdX(hInstance,IDICON_Mainframe) );
        LogoBitmap.LoadFile( TEXT("ConfigLogo.bmp") );
        SendMessageX( LogoStatic, STM_SETIMAGE, IMAGE_BITMAP, (LPARAM)LogoBitmap.GetBitmapHandle() );
        SetText( *Title );
        SetForegroundWindow( hWnd );
        unguard;
    }
    void OnCancel()
    {
        GIsCriticalError = 1;
        ExitProcess( 0 );
    }
};

class WConfigPageRenderer : public WWizardPage
{
    DECLARE_WINDOWCLASS(WConfigPageRenderer,WWizardPage,Startup);
    WConfigWizard* Owner;
    WListBox RenderList;
    WLabel RenderNote;
    TArray<FRegistryObjectInfo> RenderDevices;
    WConfigPageRenderer( WConfigWizard* InOwner )
    : WWizardPage( TEXT("ConfigPageRenderer"), IDDIALOG_ConfigPageRenderer, InOwner )
    , Owner(InOwner)
    , RenderList(this,IDC_RenderList)
    , RenderNote(this,IDC_RenderNote)
    {}
    void OnInitDialog()
    {
        WWizardPage::OnInitDialog();
        RenderList.SelectionChangeDelegate = FDelegate(this,(TDelegate)&WConfigPageRenderer::CurrentChange);
        RenderList.DoubleClickDelegate = FDelegate(Owner,(TDelegate)&WWizardDialog::OnFinish);
        RefreshList();
    }
    void RefreshList()
    {
        RenderList.Empty();
        RenderDevices.Empty();
        UObject::GetRegistryObjects( RenderDevices, UClass::StaticClass(), URenderDevice::StaticClass(), 0 );
        FString Default;
        for( TArray<FRegistryObjectInfo>::TIterator It(RenderDevices); It; ++It )
        {
            FString Path=It->Object, Left, Right, Temp;
            if( Path.Split(TEXT("."),&Left,&Right) )
            {
                RenderList.AddString( *(Temp=Localize(*Right,TEXT("ClassCaption"),*Left)) );
                if( Default==TEXT("") )
                {
                    Default=Temp;
                }
            }
        }
        if( Default!=TEXT("") )
            RenderList.SetCurrent( RenderList.FindStringChecked(*Default), 1 );
        CurrentChange();
    }
    void CurrentChange()
    {
        if( CurrentDriver()!=TEXT("") )
        {
            GConfig->SetString(TEXT("Engine.Engine"),TEXT("GameRenderDevice"),*CurrentDriver());
        }
        RenderNote.SetText(Localize(TEXT("Descriptions"),*CurrentDriver(),TEXT("Startup"),NULL,1));
    }
    FString CurrentDriver()
    {
        if( RenderList.GetCurrent()>=0 )
        {
            FString Name = RenderList.GetString(RenderList.GetCurrent());
            for( TArray<FRegistryObjectInfo>::TIterator It(RenderDevices); It; ++It )
            {
                FString Path=It->Object, Left, Right, Temp;
                if( Path.Split(TEXT("."),&Left,&Right) )
                    if( Name==Localize(*Right,TEXT("ClassCaption"),*Left) )
                        return Path;
            }
        }
        return TEXT("");
    }
    const TCHAR* GetNextText()
    {
        return NULL;
    }
    const TCHAR* GetFinishText()
    {
        return LocalizeGeneral("FinishButton", TEXT("Window"));
    }
};

/*-----------------------------------------------------------------------------
    Launch mplayer.com.
    - by Jack Porter
    - Based on mp_launch2.c by Rich Rice --rich@mpath.com
-----------------------------------------------------------------------------*/

#define MPI_FILE TEXT("mput.mpi")
#define MPLAYNOW_EXE TEXT("mplaynow.exe")

static int GetMplayerDirectory(TCHAR *mplayer_directory)
{
    HKEY hkey;
    HKEY key = HKEY_LOCAL_MACHINE;
    TCHAR subkey[]=TEXT("software\\mpath\\mplayer\\main");
    TCHAR valuename[]=TEXT("root directory");
    TCHAR buffer[MAX_PATH];
    DWORD dwType, dwSize;

    if( RegOpenKeyExX(key, subkey, 0, KEY_READ, &hkey) == ERROR_SUCCESS )
    {
        dwSize = MAX_PATH;
        if( RegQueryValueExX(hkey, valuename, 0, &dwType, (LPBYTE) buffer, &dwSize) == ERROR_SUCCESS )
        {
            appSprintf(mplayer_directory, TEXT("%s"), buffer);
            return 1;
        }
        RegCloseKey(hkey);
    }

    return 0;
}

static void LaunchMplayer()
{
    TCHAR mplaunch_exe[MAX_PATH], mplayer_directory[MAX_PATH];

    if( GetMplayerDirectory(mplayer_directory) )
    {
        appSprintf( mplaunch_exe, TEXT("%s\\programs\\mplaunch.exe"), mplayer_directory );
        if( GFileManager->FileSize(mplaunch_exe)>0 )
        {
            appLaunchURL( mplaunch_exe, MPI_FILE );
            return;
        }
    }

    appLaunchURL( MPLAYNOW_EXE, TEXT("") );
}

#undef MPI_FILE
#undef MPLAYNOW_EXE

/*-----------------------------------------------------------------------------
    Exec hook.
-----------------------------------------------------------------------------*/

// FExecHook.
class FExecHook : public FExec, public FNotifyHook
{
private:
    WConfigProperties* Preferences;
    void NotifyDestroy( void* Src )
    {
        if( Src==Preferences )
            Preferences = NULL;
    }
    UBOOL Exec( const TCHAR* Cmd, FOutputDevice& Ar )
    {
        guard(FExecHook::Exec);
        if( ParseCommand(&Cmd,TEXT("ShowLog")) )
        {
            if( GLogWindow )
            {
                GLogWindow->Show(1);
                SetFocus( *GLogWindow );
                GLogWindow->Display.ScrollCaret();
            }
            return 1;
        }
        else if( ParseCommand(&Cmd,TEXT("TakeFocus")) )
        {
            TObjectIterator<UEngine> EngineIt;
            if
            (   EngineIt
            &&  EngineIt->Client
            &&  EngineIt->Client->Viewports.Num() )
                SetForegroundWindow( (HWND)EngineIt->Client->Viewports(0)->GetWindow() );
            return 1;
        }
        else if( ParseCommand(&Cmd,TEXT("EditActor")) )
        {
            UClass* Class;
            TObjectIterator<UEngine> EngineIt;
            if( EngineIt && ParseObject<UClass>( Cmd, TEXT("Class="), Class, ANY_PACKAGE ) )
            {
                AActor* Player  = EngineIt->Client ? EngineIt->Client->Viewports(0)->Actor : NULL;
                AActor* Found   = NULL;
                FLOAT   MinDist = 999999.0;
                for( TObjectIterator<AActor> It; It; ++It )
                {
                    FLOAT Dist = Player ? FDist(It->Location,Player->Location) : 0.0;
                    if
                    (   (!Player || It->GetLevel()==Player->GetLevel())
                    &&  (!It->bDeleteMe)
                    &&  (It->IsA( Class) )
                    &&  (Dist<MinDist) )
                    {
                        MinDist = Dist;
                        Found   = *It;
                    }
                }
                if( Found )
                {
                    WObjectProperties* P = new WObjectProperties( TEXT("EditActor"), 0, TEXT(""), NULL, 1 );
                    P->OpenWindow( (HWND)EngineIt->Client->Viewports(0)->GetWindow() );
                    P->Root.SetObjects( (UObject**)&Found, 1 );
                    P->Show(1);
                }
                else Ar.Logf( TEXT("Actor not found") );
            }
            else Ar.Logf( TEXT("Missing class") );
            return 1;
        }
        else if( ParseCommand(&Cmd,TEXT("HideLog")) )
        {
            if( GLogWindow )
                GLogWindow->Show(0);
            return 1;
        }
        else if( ParseCommand(&Cmd,TEXT("Preferences")) && !GIsClient )
        {
            if( !Preferences )
            {
                Preferences = new WConfigProperties( TEXT("Preferences"), LocalizeGeneral("AdvancedOptionsTitle",TEXT("Window")) );
                Preferences->SetNotifyHook( this );
                Preferences->OpenWindow( GLogWindow ? GLogWindow->hWnd : NULL );
                Preferences->ForceRefresh();
            }
            Preferences->Show(1);
            SetFocus( *Preferences );
            return 1;
        }
        else if( ParseCommand(&Cmd,TEXT("MPLAYER")) && GIsClient )
        {
            LaunchMplayer();
            return 1;
        }
        else if( ParseCommand(&Cmd,TEXT("HEAT")) && GIsClient )
        {
            appLaunchURL( TEXT("GotoHEAT.exe"), TEXT("5193") );
            return 1;
        }
        // Intercept ToggleFullScreen and SetRes to make them work with borderless.
        else if ( ParseCommand( &Cmd, TEXT( "ToggleFullScreen" ) ) ) {
            if ( IsUsingBorderless() ) {
                ToggleWindowMode();

                GConfig->SetBool( L"WinDrv.WindowsClient", L"StartupFullscreen", IsInFullScreen() );

                return 1;
            }

            return 0;
        }
        else if ( ParseCommand( &Cmd, TEXT( "SetRes" ) ) ) {
            if ( IsUsingBorderless() ) {
                int w, h;
                if ( StringToResolution( Cmd, w, h ) ) {
                    SetResolution( w, h, IsInFullScreen() );

                    if ( IsInFullScreen() ) {
                        GConfig->SetInt( TEXT( "WinDrv.WindowsClient" ), TEXT( "FullscreenViewportX" ), w );
                        GConfig->SetInt( TEXT( "WinDrv.WindowsClient" ), TEXT( "FullscreenViewportY" ), h );
                    }
                    else {
                        GConfig->SetInt( TEXT( "WinDrv.WindowsClient" ), TEXT( "WindowedViewportX" ), w );
                        GConfig->SetInt( TEXT( "WinDrv.WindowsClient" ), TEXT( "WindowedViewportY" ), h );
                    }
                }

                return 1;
            }

            return 0;
        }
        // We create a new command to fetch hardcoded default resolutions, in case the renderer gives no resolutions to choose from in-game.
        else if ( ParseCommand( &Cmd, TEXT( "GetDefaultRes" ) ) ) {
            Ar.Logf( L"800x600 1024x576 1024x768 1280x720 1280x800 1280x960 1366x768 1440x900 1440x1080 1600x900 1680x1050 1600x1200 1920x1080 1920x1200 1920x1440" );

            return 1;
        }
        else if ( ParseCommand( &Cmd, TEXT( "FlushLighting" ) ) ) {
            GCache.Flush( MakeCacheID( CID_ShadowMap, 0, 0 ), MakeCacheID( CID_MAX, 0, 0, NULL ) );
            GCache.Flush( MakeCacheID( CID_IlluminationMap, 0, 0 ), MakeCacheID( CID_MAX, 0, 0, NULL ) );
            GCache.Flush( MakeCacheID( CID_StaticMap, 0, 0 ), MakeCacheID( CID_MAX, 0, 0, NULL ) );
            GCache.Flush( MakeCacheID( CID_DynamicMap, 0, 0 ), MakeCacheID( CID_MAX, 0, 0, NULL ) );

            return 1;
        }
        else if ( ParseCommand( &Cmd, TEXT( "FPSCap" ) ) ) {
            int cap = appAtoi( Cmd );
            if ( cap >= 0 ) {
                SetFPSCap( cap );
            }

            return 1;
        }
        else if ( ParseCommand( &Cmd, TEXT( "GetFPSCap" ) ) ) {
            Ar.Logf( L"%i", GetFPSCap() );

            return 1;
        }
        else if ( ParseCommand( &Cmd, TEXT( "UIScale" ) ) ) {
            int scale = appAtoi( Cmd );
            if ( scale >= 1 ) {
                SetUIScale( scale );
            }

            return 1;
        }
        else return 0;
        unguard;
    }
public:
    FExecHook()
    : Preferences( NULL )
    {}
};

/*-----------------------------------------------------------------------------
    Startup and shutdown.
-----------------------------------------------------------------------------*/

//
// Initialize.
//
static UEngine* InitEngine()
{
    guard(InitEngine);
    DOUBLE LoadTime = appSeconds();

    // Set exec hook.
    static FExecHook GLocalHook;
    GExec = &GLocalHook;

    // Create mutex so installer knows we're running.
    CreateMutexX( NULL, 0, TEXT("UnrealIsRunning"));
    UBOOL AlreadyRunning = (GetLastError()==ERROR_ALREADY_EXISTS);

    // First-run menu.
    INT FirstRun=0;
    GConfig->GetInt( TEXT("FirstRun"), TEXT("FirstRun"), FirstRun );
    if( ParseParam(appCmdLine(),TEXT("FirstRun")) )
        FirstRun=0;
    if( FirstRun<220 )
    {
        // Migrate savegames.
        TArray<FString> Saves = GFileManager->FindFiles( TEXT("..\\Save\\*.usa"), 1, 0 );
        for( TArray<FString>::TIterator It(Saves); It; ++It )
        {
            INT Pos = appAtoi(**It+4);
            FString Section = TEXT("UnrealShare.UnrealSlotMenu");
            FString Key     = FString::Printf(TEXT("SlotNames[%i]"),Pos);
            if( appStricmp(GConfig->GetStr(*Section,*Key,TEXT("user")),TEXT(""))==0 )
                GConfig->SetString(*Section,*Key,TEXT("Saved game"),TEXT("user"));
        }
    }

    // Commandline (for mplayer/heat)
    FString Command;
    if( Parse(appCmdLine(),TEXT("consolecommand="), Command) )
    {
        debugf(TEXT("Executing console command %s"),*Command);
        GExec->Exec( *Command, *GLog );
        return NULL;
    }

    // Test render device.
    FString Device;
    if( Parse(appCmdLine(),TEXT("testrendev="),Device) )
    {
        debugf(TEXT("Detecting %s"),*Device);
        try
        {
            UClass* Cls = LoadClass<URenderDevice>( NULL, *Device, NULL, 0, NULL );
            GConfig->Flush(0);
            if( Cls )
            {
                URenderDevice* RenDev = ConstructObject<URenderDevice>(Cls);
                if( RenDev )
                {
                    if( RenDev->Init(NULL,0,0,0,0) )
                    {
                        debugf(TEXT("Successfully detected %s"),*Device);
                    }
                    else delete RenDev;
                }
            }
        } catch( ... ) {}
        FArchive* Ar = GFileManager->CreateFileWriter(TEXT("Detected.ini"),0);
        if( Ar )
            delete Ar;
        return NULL;
    }

    // Config UI.
    guard(ConfigUI);
    if( !GIsEditor && GIsClient )
    {
        WConfigWizard D;
        WWizardPage* Page = NULL;
        if( FirstRun<ENGINE_VERSION )
        {
            // Init first run config here.
            // Snap the full screen resolution to native resolution.
            int width = 1280;
            int height = 720;
            GetDesktopResolution( width, height );

            int fullWidth = Max( width, 1280 );
            int fullHeight = Max( height, 720 );
            int windowedWidth = Min( 1280, width );
            int windowedHeight = Min( 720, height );

            GConfig->SetInt( TEXT( "WinDrv.WindowsClient" ), TEXT( "WindowedViewportX" ), windowedWidth );
            GConfig->SetInt( TEXT( "WinDrv.WindowsClient" ), TEXT( "WindowedViewportY" ), windowedHeight );
            GConfig->SetInt( TEXT( "WinDrv.WindowsClient" ), TEXT( "WindowedColorBits" ), 32 );
            GConfig->SetInt( TEXT( "WinDrv.WindowsClient" ), TEXT( "FullscreenViewportX" ), fullWidth );
            GConfig->SetInt( TEXT( "WinDrv.WindowsClient" ), TEXT( "FullscreenViewportY" ), fullHeight );
            GConfig->SetInt( TEXT( "WinDrv.WindowsClient" ), TEXT( "FullscreenColorBits" ), 32 );

            SetAllResolution( fullWidth, fullHeight, windowedWidth, windowedHeight );

            Page = new WConfigPageRenderer(&D); D.Title=LocalizeGeneral(TEXT("FirstTime"),TEXT("Startup"));
        }
        else if( ParseParam(appCmdLine(),TEXT("changevideo")) )
            {Page = new WConfigPageRenderer(&D); D.Title=LocalizeGeneral(TEXT("Video"),TEXT("Startup"));}
        if( Page )
        {
            ExitSplash();
            D.Advance( Page );
            if( !D.DoModal() )
                return NULL;
            InitSplash(NULL);
        }
    }
    unguard;

    // Create is-running semaphore file.
    FArchive* Ar = GFileManager->CreateFileWriter(TEXT("Running.ini"),0);
    if( Ar )
        delete Ar;

    // Update first-run.
    if( FirstRun<ENGINE_VERSION )
        FirstRun = ENGINE_VERSION;
    GConfig->SetInt( TEXT("FirstRun"), TEXT("FirstRun"), FirstRun );

    // Create the global engine object.
    UClass* EngineClass;
    if( !GIsEditor )
    {
        // Create game engine.
        EngineClass = UObject::StaticLoadClass( UGameEngine::StaticClass(), NULL, TEXT("ini:Engine.Engine.GameEngine"), NULL, LOAD_NoFail, NULL );
    }
    else
    {
        // Editor.
        EngineClass = UObject::StaticLoadClass( UEngine::StaticClass(), NULL, TEXT("ini:Engine.Engine.EditorEngine"), NULL, LOAD_NoFail, NULL );
    }
    UEngine* Engine = ConstructObject<UEngine>( EngineClass );
    Engine->Init();
    debugf( TEXT("Startup time: %f seconds"), appSeconds()-LoadTime );

    return Engine;
    unguard;
}

//
// Unreal's main message loop.  All windows in Unreal receive messages
// somewhere below this function on the stack.
//
static void MainLoop( UEngine* Engine )
{
    guard(MainLoop);
    check(Engine);

    // Enter main loop.
    guard(EnterMainLoop);
    if( GLogWindow )
        GLogWindow->SetExec( Engine );
    unguard;

    // Set up all the necessary stuff that we're gonna use during the game.
    if ( Engine->Client->Viewports.Num() > 0 ) {
        Viewport = Engine->Client->Viewports( 0 );
    }
    else {
        Viewport = nullptr;
    }

    // Get the window pointer.
    MainWindow = Viewport ? ( HWND ) Viewport->GetWindow() : GetActiveWindow();

    // Init raw input.
    RegisterRawInput();

    // Apply borderless mode appropriately.
    if ( IsUsingBorderless() ) {
        ToggleWindowMode( IsInFullScreen() );
    }

    // Loop while running.
    GIsRunning = 1;
    DWORD ThreadId = GetCurrentThreadId();
    HANDLE hThread = GetCurrentThread();
    DOUBLE OldTime = appSeconds();
    DOUBLE SecondStartTime = OldTime;
    INT TickCount = 0;
    while( GIsRunning && !GIsRequestingExit )
    {
        // Rewrite the code that updates the game to add FPS capping.
        guard( UpdateWorld );

        DOUBLE newTime = appSeconds();
        FLOAT deltaTime = newTime - OldTime;

        FLOAT cap = Engine->GetMaxTickRate();
        cap = cap > 0 ? cap : GetFPSCap();
        if ( cap <= 0 || deltaTime >= ( 1.0f / cap ) ) {
            Engine->Tick( deltaTime );

            if ( GWindowManager ) {
                GWindowManager->Tick( deltaTime );
            }

            OldTime = newTime;
            TickCount = TickCount + 1;
        }
        unguard;

        // Markie
        if ( Engine->Client->Viewports.Num() <= 0 ) {
            Viewport = nullptr;
        }

        bool focused = GetFocus() == MainWindow && GetForegroundWindow() == MainWindow;

        // Handle all incoming messages.
        guard(MessagePump);
        MSG Msg;
        while( PeekMessageX(&Msg,NULL,0,0,PM_REMOVE) )
        {
            bool processed = false;

            if( Msg.message == WM_QUIT )
                GIsRequestingExit = 1;
            // Blocks vanilla mouse movement.
            else if ( Msg.message == WM_MOUSEMOVE ) {
                if ( focused ) {
                    processed = true;
                }
            }
            // Adds raw input mouse movement.
            else if ( Msg.message == WM_INPUT ) {
                if ( focused ) {
                    UINT dwSize;
                    GetRawInputData( ( HRAWINPUT ) Msg.lParam, RID_INPUT, NULL, &dwSize, sizeof( RAWINPUTHEADER ) );
                    LPBYTE lpb = new BYTE[dwSize];
                    GetRawInputData( ( HRAWINPUT ) Msg.lParam, RID_INPUT, lpb, &dwSize, sizeof( RAWINPUTHEADER ) );

                    if ( lpb != NULL ) {
                        RAWINPUT* raw = ( RAWINPUT* ) lpb;

                        // Camera movement in-game.
                        if ( raw->data.mouse.lLastX != 0 ) {
                            Engine->InputEvent( Viewport, EInputKey::IK_MouseX, EInputAction::IST_Axis, raw->data.mouse.lLastX );
                        }
                        if ( raw->data.mouse.lLastY != 0 ) {
                            Engine->InputEvent( Viewport, EInputKey::IK_MouseY, EInputAction::IST_Axis, - raw->data.mouse.lLastY );
                        }

                        // Add support for more mouse buttons.
                        if ( raw->data.mouse.ulButtons & RI_MOUSE_BUTTON_4_UP ) {
                            Engine->InputEvent( Viewport, EInputKey::IK_Unknown05, EInputAction::IST_Release );
                        }
                        else if ( raw->data.mouse.ulButtons & RI_MOUSE_BUTTON_4_DOWN ) {
                            Engine->InputEvent( Viewport, EInputKey::IK_Unknown05, EInputAction::IST_Press );
                        }

                        if ( raw->data.mouse.ulButtons & RI_MOUSE_BUTTON_5_UP ) {
                            Engine->InputEvent( Viewport, EInputKey::IK_Unknown06, EInputAction::IST_Release );
                        }
                        else if ( raw->data.mouse.ulButtons & RI_MOUSE_BUTTON_5_DOWN ) {
                            Engine->InputEvent( Viewport, EInputKey::IK_Unknown06, EInputAction::IST_Press );
                        }

                        processed = true;
                    }

                    delete[] lpb;
                }
            }
            else if ( Msg.message == WM_SYSKEYDOWN ) {
                if ( focused && IsUsingBorderless() ) {
                    if ( Msg.wParam == VK_RETURN && ( HIWORD( Msg.lParam ) & KF_ALTDOWN ) ) {
                        ToggleWindowMode();

                        processed = true;
                    }
                }
            }

            if ( processed ) {
                continue;
            }

            guard(TranslateMessage);
            TranslateMessage( &Msg );
            unguardf(( TEXT("%08X %i"), (INT)Msg.hwnd, Msg.message ));

            guard(DispatchMessage);
            DispatchMessageX( &Msg );
            unguardf(( TEXT("%08X %i"), (INT)Msg.hwnd, Msg.message ));
        }
        unguard;

        POINT mP;
        GetCursorPos( &mP );

        // When focused, clip cursor and hide it.
        if ( focused ) {
            RECT rect;
            GetClientRect( MainWindow, &rect );

            POINT points[] = { { rect.left, rect.top },{ rect.right, rect.bottom } };
            MapWindowPoints( MainWindow, NULL, points, 2 );

            RECT clip = { points[0].x, points[0].y, points[1].x, points[1].y };

            ClipCursor( &clip );
        }

        // Snap in-game cursor to real cursor.
        if ( focused ) {
            ScreenToClient( MainWindow, &mP );
            Engine->MousePosition( Viewport, 0, mP.x, mP.y );
        }

        // If editor thread doesn't have the focus, don't suck up too much CPU time.
        if( GIsEditor )
        {
            guard(ThrottleEditor);
            static UBOOL HadFocus=1;
            UBOOL HasFocus = (GetWindowThreadProcessId(GetForegroundWindow(),NULL) == ThreadId );
            if( HadFocus && !HasFocus )
            {
                // Drop our priority to speed up whatever is in the foreground.
                SetThreadPriority( hThread, THREAD_PRIORITY_BELOW_NORMAL );
            }
            else if( HasFocus && !HadFocus )
            {
                // Boost our priority back to normal.
                SetThreadPriority( hThread, THREAD_PRIORITY_NORMAL );
            }
            if( !HasFocus )
            {
                // Surrender the rest of this timeslice.
                Sleep(0);
            }
            HadFocus = HasFocus;
            unguard;
        }
    }
    GIsRunning = 0;

    // Exit main loop.
    guard(ExitMainLoop);
    if( GLogWindow )
        GLogWindow->SetExec( NULL );
    GExec = NULL;
    unguard;

    unguard;
}

/*-----------------------------------------------------------------------------
    The End.
-----------------------------------------------------------------------------*/
