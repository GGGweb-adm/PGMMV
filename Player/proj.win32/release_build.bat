@echo off

rem set DEVENV_EXE="C:\Program Files (x86)\Microsoft Visual Studio 14.0\Common7\IDE\devenv"
set DEVENV_EXE="C:\Program Files (x86)\MSBuild\14.0\Bin\MSBuild.exe"
set PLAYER_ROOT=C:\Workspace\agtk\Player\proj.win32
set PLAYER_OUTPUT=C:\Workspace\player
set BUILD_OUTPUT_DIR=Release.win32

rem ------------------------------------------------
rem �r���h�f�B���N�g���N���A
rmdir /S /Q %PLAYER_ROOT%\%BUILD_OUTPUT_DIR%

rem ------------------------------------------------
rem �r���h
rem %DEVENV_EXE% /rebuild release %PLAYER_ROOT%\player.sln
%DEVENV_EXE% %PLAYER_ROOT%\player.sln /t:Rebuild /p:Configuration=Release /p:Platform=Win32

rem ------------------------------------------------
rem �v���C���[�̃R�s�[����N���A�ɂ���B
rem �R�s�[��f�B���N�g����j���B
rmdir /S /Q %PLAYER_OUTPUT%
rem �R�s�[��f�B���N�g���쐬�B
mkdir %PLAYER_OUTPUT%

rem �v���W�F�N�g�f�[�^���O���ɂ���ꍇ�Aanimations,audio,data,img,movies�̃R�s�[���Ȃ��Ă��悢�B
xcopy /Y /Q /S /E %PLAYER_ROOT%\%BUILD_OUTPUT_DIR%\animations %PLAYER_OUTPUT%\animations\
xcopy /Y /Q /S /E %PLAYER_ROOT%\%BUILD_OUTPUT_DIR%\audio %PLAYER_OUTPUT%\audio\
xcopy /Y /Q /S /E %PLAYER_ROOT%\%BUILD_OUTPUT_DIR%\data %PLAYER_OUTPUT%\data\
xcopy /Y /Q /S /E %PLAYER_ROOT%\%BUILD_OUTPUT_DIR%\img %PLAYER_OUTPUT%\img\
xcopy /Y /Q /S /E %PLAYER_ROOT%\%BUILD_OUTPUT_DIR%\movies %PLAYER_OUTPUT%\movies\

xcopy /Y /Q /S /E %PLAYER_ROOT%\%BUILD_OUTPUT_DIR%\fonts %PLAYER_OUTPUT%\fonts\
xcopy /Y /Q /S /E %PLAYER_ROOT%\%BUILD_OUTPUT_DIR%\plugins %PLAYER_OUTPUT%\plugins\
copy /Y %PLAYER_ROOT%\%BUILD_OUTPUT_DIR%\*.dll %PLAYER_OUTPUT%\
copy /Y %PLAYER_ROOT%\%BUILD_OUTPUT_DIR%\*.dll %PLAYER_OUTPUT%\
copy /Y %PLAYER_ROOT%\%BUILD_OUTPUT_DIR%\*.exe %PLAYER_OUTPUT%\
