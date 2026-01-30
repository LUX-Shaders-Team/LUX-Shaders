::===================== File of the LUX Shader Project =====================::
::  - Initial D.  : 03.04.2025                                              ::
::  - Last Change : 29.01.2026                                              ::
::==========================================================================::
@echo off
setlocal

REM Temporary Includes
if exist fxctmp9_tmp rd /s /q fxctmp9_tmp
if exist vshtmp9_tmp rd /s /q vshtmp9_tmp
if exist pshtmp9_tmp rd /s /q pshtmp9_tmp

REM 360 Temp Includes?
if exist fxctmp9_360_tmp rd /s /q fxctmp9_360_tmp
if exist vshtmp9_360_tmp rd /s /q vshtmp9_360_tmp
if exist pshtmp9_360_tmp rd /s /q pshtmp9_360_tmp

echo "Cleaned temporary Include Files"
pause 