::===================== File of the LUX Shader Project =====================::
::  - Initial D.  : 03.04.2025                                              ::
::  - Last Change : 29.01.2026                                              ::
::==========================================================================::
@echo off
setlocal

set TTEXE=..\..\devtools\bin\timeprecise.exe
if not exist %TTEXE% goto no_ttexe
goto no_ttexe_end

:no_ttexe
set TTEXE=time /t
:no_ttexe_end


rem echo.
rem echo ~~~~~~ buildallshaders %* ~~~~~~
%TTEXE% -cur-Q
set tt_all_start=%ERRORLEVEL%
set tt_all_chkpt=%tt_start%

rem LUX has a lot of BatFiles and it's too annoying to change the targetdir for all of them
rem Just go to this File and change it there.
call "%~dp0ShadersBuildDirectories.bat"

set BUILD_SHADER=call ShadersBuild.bat

set ARG_X360=-x360
set ARG_EXTRA=

REM ****************
REM usage: buildallshaders [-pc | -x360]
REM ****************
set ALLSHADERS_CONFIG=pc

REM ****************
REM PC SHADERS
REM ****************

%BUILD_SHADER% ShaderList_Triplanar_Displacement -game %GAME_DIR% -source %SOURCE_DIR% -force30

rem Ask for a .vcs Reload using +lux_vcshotreloads_pulse
call "%~dp0ShadersCallProcess.bat"

pause 

REM ****************
REM END
REM ****************
:end

rem echo.
if not "%dynamic_shaders%" == "1" (
  rem echo Finished full buildallshaders %*
) else (
  rem echo Finished dynamic buildallshaders %*
)

rem %TTEXE% -diff %tt_all_start% -cur
rem echo.
