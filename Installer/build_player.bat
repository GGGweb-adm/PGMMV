rem build players of Pixel Game Maker MV

cd %~dp0
set INSTALLERDIR=%cd%
cd ..
set WORKSPACE=%cd%
cd %INSTALLERDIR%

cd %WORKSPACE%

cd Installer

set VC_DIR=C:\Program Files (x86)\Microsoft Visual Studio 17.0\VC
set VCINSTALLDIR=C:\Program Files (x86)\Microsoft Visual Studio 17.0\VC

call "C:\Program Files\Microsoft Visual Studio\2022\Community\Common7\Tools\VsDevCmd.bat"

rem ====================== Player Build ==============================
rem ---------------------------------------------------------------------
set DEVENV_EXE="C:\Program Files\Microsoft Visual Studio\2022\Community\MSBuild\Current\Bin\MSBuild.exe"
set PLAYER_ROOT="%WORKSPACE%\Player\proj.win32"
set PLAYER_OUTPUT="%WORKSPACE%\Installer\player"
set BUILD_OUTPUT_DIR=Release.win32

rem ------------------------------------------------
rem Remove a build directory.
rmdir /S /Q %PLAYER_ROOT%\%BUILD_OUTPUT_DIR%

rem ------------------------------------------------
rem Build
%DEVENV_EXE% %PLAYER_ROOT%\player.sln /t:Clean /p:Configuration=Release /p:Platform=Win32
%DEVENV_EXE% %PLAYER_ROOT%\player.sln /t:Build /p:Configuration=Release /p:Platform=Win32

rem Error if an execution file is not created.
if exist %PLAYER_ROOT%\%BUILD_OUTPUT_DIR%\player.exe goto LPlayerCreated
echo "Building \"Player(Release)\" was failed."
pause
exit 1

:LPlayerCreated
echo "Player Created."

rem ------------------------------------------------
rem Remake a player output directory.
rmdir /S /Q %PLAYER_OUTPUT%
dir %PLAYER_OUTPUT%
mkdir %PLAYER_OUTPUT%

xcopy /Y /Q /S /E %PLAYER_ROOT%\%BUILD_OUTPUT_DIR%\Resources %PLAYER_OUTPUT%\Resources\
xcopy /Y /Q /S /E %PLAYER_ROOT%\%BUILD_OUTPUT_DIR%\plugins %PLAYER_OUTPUT%\plugins\
copy /Y %PLAYER_ROOT%\%BUILD_OUTPUT_DIR%\*.dll %PLAYER_OUTPUT%\
copy /Y %PLAYER_ROOT%\%BUILD_OUTPUT_DIR%\*.exe %PLAYER_OUTPUT%\
copy /Y %PLAYER_ROOT%\%BUILD_OUTPUT_DIR%\player.lib %PLAYER_OUTPUT%\
copy /Y %PLAYER_ROOT%\%BUILD_OUTPUT_DIR%\credits.html %PLAYER_OUTPUT%\


rem ====================== Runtime Build ==============================
rem ---------------------------------------------------------------------
set RUNTIME_OUTPUT="%WORKSPACE%\Installer\runtime"
set BUILD_OUTPUT_DIR=Runtime.win32

rem ------------------------------------------------
rem Remove a build directory.
rmdir /S /Q %PLAYER_ROOT%\%BUILD_OUTPUT_DIR%

rem ------------------------------------------------
rem Build
cscript //nologo rc_change.js player runtime < game.rc > game.tmp
copy /y game.tmp game.rc
%DEVENV_EXE% %PLAYER_ROOT%\player.sln /t:Clean /p:Configuration=Runtime /p:Platform=Win32
%DEVENV_EXE% %PLAYER_ROOT%\player.sln /t:Build /p:Configuration=Runtime /p:Platform=Win32
cscript //nologo rc_change.js runtime player < game.tmp > game.rc

rem Error if an execution file is not created.
if exist %PLAYER_ROOT%\%BUILD_OUTPUT_DIR%\player.exe goto LRuntimeCreated
echo "Building \"Player(Runtime)\" was failed."
pause
exit 1

:LRuntimeCreated
echo "Runtime Created."
rem ------------------------------------------------
rem Remake a runtime output directory.
rmdir /S /Q %RUNTIME_OUTPUT%
dir %RUNTIME_OUTPUT%
mkdir %RUNTIME_OUTPUT%

xcopy /Y /Q /S /E %PLAYER_ROOT%\%BUILD_OUTPUT_DIR%\Resources %RUNTIME_OUTPUT%\Resources\
xcopy /Y /Q /S /E %PLAYER_ROOT%\%BUILD_OUTPUT_DIR%\plugins %RUNTIME_OUTPUT%\plugins\
copy /Y %PLAYER_ROOT%\%BUILD_OUTPUT_DIR%\*.dll %RUNTIME_OUTPUT%\
copy /Y %PLAYER_ROOT%\%BUILD_OUTPUT_DIR%\*.exe %RUNTIME_OUTPUT%\
copy /Y %PLAYER_ROOT%\%BUILD_OUTPUT_DIR%\player.lib %RUNTIME_OUTPUT%\
copy /Y %PLAYER_ROOT%\%BUILD_OUTPUT_DIR%\Resources\credits.html %RUNTIME_OUTPUT%\
del %RUNTIME_OUTPUT%\Resources\credits.html

rem ====================== DebugMenuRuntime Build ==============================
rem ---------------------------------------------------------------------
set DEBUG_MENU_RUNTIME_OUTPUT="%WORKSPACE%\Installer\debugMenuRuntime\main"
set BUILD_OUTPUT_DIR=DebugMenuRuntime.win32

rem ------------------------------------------------
rem Remove a build directory.
rmdir /S /Q %PLAYER_ROOT%\%BUILD_OUTPUT_DIR%

rem ------------------------------------------------
rem Build
cscript //nologo rc_change.js player debugMenuRuntime < game.rc > game.tmp
copy /y game.tmp game.rc
%DEVENV_EXE% %PLAYER_ROOT%\player.sln /t:Clean /p:Configuration=DebugMenuRuntime /p:Platform=Win32
%DEVENV_EXE% %PLAYER_ROOT%\player.sln /t:Build /p:Configuration=DebugMenuRuntime /p:Platform=Win32
cscript //nologo rc_change.js debugMenuRuntime player < game.tmp > game.rc

rem Error if an execution file is not created.
if exist %PLAYER_ROOT%\%BUILD_OUTPUT_DIR%\player.exe goto LDebugMenuRuntimeCreated
echo "Building \"Player(DebugMenuRuntime)\" was failed."
pause
exit 1

:LDebugMenuRuntimeCreated
echo "DebugMenuRuntime Created."
rem ------------------------------------------------
rem Remake a debugMenuRuntime output directory.
rmdir /S /Q %DEBUG_MENU_RUNTIME_OUTPUT%
dir %DEBUG_MENU_RUNTIME_OUTPUT%
mkdir %DEBUG_MENU_RUNTIME_OUTPUT%

xcopy /Y /Q /S /E %PLAYER_ROOT%\%BUILD_OUTPUT_DIR%\Resources %DEBUG_MENU_RUNTIME_OUTPUT%\Resources\
xcopy /Y /Q /S /E %PLAYER_ROOT%\%BUILD_OUTPUT_DIR%\plugins %DEBUG_MENU_RUNTIME_OUTPUT%\plugins\
copy /Y %PLAYER_ROOT%\%BUILD_OUTPUT_DIR%\*.dll %DEBUG_MENU_RUNTIME_OUTPUT%\
copy /Y %PLAYER_ROOT%\%BUILD_OUTPUT_DIR%\*.exe %DEBUG_MENU_RUNTIME_OUTPUT%\
copy /Y %PLAYER_ROOT%\%BUILD_OUTPUT_DIR%\player.lib %DEBUG_MENU_RUNTIME_OUTPUT%\
copy /Y %PLAYER_ROOT%\%BUILD_OUTPUT_DIR%\Resources\credits.html %DEBUG_MENU_RUNTIME_OUTPUT%\..\
del %DEBUG_MENU_RUNTIME_OUTPUT%\Resources\credits.html

rem =================================================================================================================

echo "Building finished."
pause

