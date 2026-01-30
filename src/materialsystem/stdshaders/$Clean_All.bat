::===================== File of the LUX Shader Project =====================::
::  - Initial D.  : 03.04.2025                                              ::
::  - Last Change : 29.01.2026                                              ::
::==========================================================================::
@echo off
setlocal

REM Never seen this Folder? Commenting.
REM if exist debug_dx9 rd /s /q debug_dx9

REM Includes
if exist fxctmp9 rd /s /q fxctmp9
if exist vshtmp9 rd /s /q vshtmp9
if exist pshtmp9 rd /s /q pshtmp9

REM 360 Includes?
if exist fxctmp9_360 rd /s /q fxctmp9_360
if exist vshtmp9_360 rd /s /q vshtmp9_360
if exist pshtmp9_360 rd /s /q pshtmp9_360

REM Temporary Includes
if exist fxctmp9_tmp rd /s /q fxctmp9_tmp
if exist vshtmp9_tmp rd /s /q vshtmp9_tmp
if exist pshtmp9_tmp rd /s /q pshtmp9_tmp

REM 360 Temp Includes?
if exist fxctmp9_360_tmp rd /s /q fxctmp9_360_tmp
if exist vshtmp9_360_tmp rd /s /q vshtmp9_360_tmp
if exist pshtmp9_360_tmp rd /s /q pshtmp9_360_tmp

REM vcs Files
if exist shaders rd /s /q shaders

echo "Cleaned All Includes and VCS Files"
pause 