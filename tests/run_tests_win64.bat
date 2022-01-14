@echo off

REM This script is bound to VIM F9 key to execute the compilation into the
REM build directory

SET EXITCODE=0
SET TARGET=""
SET CLANG_OPTIONS=""
SET INCLUDES=/I ..\

REM "Processing script parameters"
:loop
IF NOT "%1" EQU "" (
    IF "%1" EQU "/t" (
        SET TARGET=%2
        SHIFT
    )
    SHIFT
    GOTO :loop
)

IF %TARGET%=="" (
   ECHO "Target not defined"
   GOTO Failure
)

ECHO "BUILING TESTS WITH TARGET %TARGET%"

SET BUILD_DIR=build_win64_%TARGET%
MKDIR %BUILD_DIR%

SET TESTS=gs_mem_alloc_test

FOR %%a in (%TESTS%) do (
  ECHO Executing %%a test
  %BUILD_DIR%\%%a.exe 
  IF ERRORLEVEL 1 (
    ECHO %%a test failed
    GOTO Failure
  ) ELSE (
    ECHO %%a run successful 
  )
) 


GOTO Success

:Failure
set EXITCODE=1
:Success
EXIT /B %EXITCODE%
