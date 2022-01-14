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

IF %TARGET% EQU DEBUG (
  SET CLANG_OPTIONS=/Od /Zi
)

IF %TARGET% EQU RELEASE (
  SET CLANG_OPTIONS=/O2 /DNDEBUG
)

ECHO "BUILING TESTS WITH TARGET %TARGET%"

SET BUILD_DIR=build_win64_%TARGET%
MKDIR %BUILD_DIR%


SET TESTS=gs_mem_alloc_test

FOR %%a in (%TESTS%) do (
  echo clang-cl %INCLUDES% %CLANG_OPTIONS% /o %BUILD_DIR%\%%a %%a.c
  clang-cl %INCLUDES% %CLANG_OPTIONS% /o %BUILD_DIR%\%%a %%a.c
  IF ERRORLEVEL 1 GOTO Failure
) 


GOTO Success

:Failure
set EXITCODE=1
:Success
EXIT /B %EXITCODE%
