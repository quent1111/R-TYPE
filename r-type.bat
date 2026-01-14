@echo off
setlocal enabledelayedexpansion

if exist "C:\msys64\ucrt64\bin" (
    set "PATH=C:\msys64\ucrt64\bin;%PATH%"
)

set "PROJECT_ROOT=%~dp0"
set "BUILD_DIR=%PROJECT_ROOT%build"
set "BUILD_TYPE=Release"
set "CLEAN_BUILD=0"
set "VERBOSE=0"
set "COMMAND="
set "EXTRA_ARGS="

if not defined NUMBER_OF_PROCESSORS set NUMBER_OF_PROCESSORS=4
set "JOBS=%NUMBER_OF_PROCESSORS%"



:parse_loop
if "%~1"=="" goto parse_done

if /i "%~1"=="build" set "COMMAND=build" & shift & goto parse_loop
if /i "%~1"=="client" set "COMMAND=client" & shift & goto collect_extra_args
if /i "%~1"=="server" set "COMMAND=server" & shift & goto collect_extra_args
if /i "%~1"=="test" set "COMMAND=test" & shift & goto parse_loop
if /i "%~1"=="clean" set "COMMAND=clean" & shift & goto parse_loop
if /i "%~1"=="install" set "COMMAND=install" & shift & goto parse_loop
if /i "%~1"=="rebuild" set "COMMAND=rebuild" & shift & goto parse_loop
if /i "%~1"=="all" set "COMMAND=all" & shift & goto parse_loop
if /i "%~1"=="-d" set "BUILD_TYPE=Debug" & shift & goto parse_loop
if /i "%~1"=="--debug" set "BUILD_TYPE=Debug" & shift & goto parse_loop
if /i "%~1"=="-r" set "BUILD_TYPE=Release" & shift & goto parse_loop
if /i "%~1"=="--release" set "BUILD_TYPE=Release" & shift & goto parse_loop
if /i "%~1"=="-c" set "CLEAN_BUILD=1" & shift & goto parse_loop
if /i "%~1"=="--clean" set "CLEAN_BUILD=1" & shift & goto parse_loop
if /i "%~1"=="-v" set "VERBOSE=1" & shift & goto parse_loop
if /i "%~1"=="--verbose" set "VERBOSE=1" & shift & goto parse_loop
if /i "%~1"=="-h" goto show_help
if /i "%~1"=="--help" goto show_help

echo [WARNING] Unknown argument: %~1
shift
goto parse_loop

:collect_extra_args
if "%~1"=="" goto parse_done
set "EXTRA_ARGS=!EXTRA_ARGS! %~1"
shift
goto collect_extra_args

:parse_done
if "%COMMAND%"=="" set "COMMAND=build"



echo.
echo ========================================================
echo                   R-TYPE Manager
echo            Build * Run * Test * Analyze
echo ========================================================
echo.


if /i "%COMMAND%"=="clean" goto cmd_clean
if /i "%COMMAND%"=="install" goto cmd_install
if /i "%COMMAND%"=="build" goto cmd_build
if /i "%COMMAND%"=="client" goto cmd_client
if /i "%COMMAND%"=="server" goto cmd_server
if /i "%COMMAND%"=="test" goto cmd_test
if /i "%COMMAND%"=="rebuild" goto cmd_rebuild
if /i "%COMMAND%"=="all" goto cmd_all

echo [ERROR] Unknown command: %COMMAND%
goto show_help


:cmd_clean
echo.
echo [STEP] Cleaning build directory...
if exist "%BUILD_DIR%" (
    rd /s /q "%BUILD_DIR%"
    echo [OK] Build directory cleaned
) else (
    echo [WARNING] Build directory doesn't exist
)
goto end_success

:cmd_install
call :check_conan || exit /b 1
call :install_deps || exit /b 1
goto end_success

:cmd_build
call :do_full_build "" || exit /b 1
goto end_success

:cmd_client
call :do_full_build "r-type_client" || exit /b 1
call :run_client || exit /b 1
goto end_success

:cmd_server
call :do_full_build "r-type_server" || exit /b 1
call :run_server || exit /b 1
goto end_success

:cmd_test
call :do_full_build "" || exit /b 1
call :run_tests || exit /b 1
goto end_success

:cmd_rebuild
set "CLEAN_BUILD=1"
call :do_full_build "" || exit /b 1
goto end_success

:cmd_all
call :do_full_build "all" || exit /b 1
goto end_success

:check_conan
echo.
echo [STEP] Checking Conan installation...
where conan >nul 2>&1
if errorlevel 1 (
    echo [WARNING] Conan not found in PATH. Checking if installed via pip...
    
    python -c "import conans" >nul 2>&1
    if not errorlevel 1 (
        echo [OK] Conan is installed via pip but not in PATH
        echo [INFO] Will use 'python -m conans.cli' instead of 'conan'
        set "CONAN_CMD=python -m conans.cli"
        
        python -m conans.cli profile detect --force >nul 2>&1
        exit /b 0
    )
    
    echo [WARNING] Conan not found. Installing...
    
    where python >nul 2>&1
    if not errorlevel 1 (
        echo Installing with python -m pip...
        python -m pip install --user conan
        if not errorlevel 1 (
            echo [OK] Conan installed successfully
            set "CONAN_CMD=python -m conans.cli"
            python -m conans.cli profile detect --force >nul 2>&1
            exit /b 0
        )
    )
    
    where py >nul 2>&1
    if not errorlevel 1 (
        echo Installing with py -m pip...
        py -m pip install --user conan
        if not errorlevel 1 (
            echo [OK] Conan installed successfully
            set "CONAN_CMD=py -m conans.cli"
            py -m conans.cli profile detect --force >nul 2>&1
            exit /b 0
        )
    )
    
    where pip >nul 2>&1
    if not errorlevel 1 (
        echo Installing with pip...
        pip install --user conan
        if not errorlevel 1 (
            echo [OK] Conan installed successfully
            set "CONAN_CMD=pip"
            exit /b 0
        )
    )
    
    echo [ERROR] Could not install Conan.
    echo Try manually: python -m pip install conan
    exit /b 1
) else (
    echo [OK] Conan is already installed
    set "CONAN_CMD=conan"
)
exit /b 0

:check_cmake
echo.
echo [STEP] Checking CMake...
where cmake >nul 2>&1
if errorlevel 1 (
    echo [ERROR] CMake not found. Please install CMake 3.20 or higher.
    echo   Download from: https://cmake.org/download/
    exit /b 1
)
for /f "tokens=3" %%i in ('cmake --version 2^>^&1 ^| findstr /R "[0-9]"') do (
    echo [OK] CMake %%i found
    exit /b 0
)
exit /b 0

:check_compiler
echo.
echo [STEP] Checking C++ compiler...
where cl >nul 2>&1
if not errorlevel 1 (
    echo [OK] MSVC found
    exit /b 0
)
where g++ >nul 2>&1
if not errorlevel 1 (
    echo [OK] GCC found
    exit /b 0
)
echo [ERROR] No C++ compiler found. Please install Visual Studio or MinGW.
exit /b 1

:install_deps
cd /d "%PROJECT_ROOT%"

if exist "%BUILD_DIR%\conan_toolchain.cmake" (
    echo [OK] Dependencies already installed (skipping)
    exit /b 0
)
if exist "%BUILD_DIR%\build\%BUILD_TYPE%\generators\conan_toolchain.cmake" (
    echo [OK] Dependencies already installed (skipping)
    exit /b 0
)
if exist "%BUILD_DIR%\generators\conan_toolchain.cmake" (
    echo [OK] Dependencies already installed (skipping)
    exit /b 0
)

echo.
echo [STEP] Installing dependencies with Conan...
if not exist "%BUILD_DIR%" mkdir "%BUILD_DIR%"

if not defined CONAN_CMD set "CONAN_CMD=conan"

if "%VERBOSE%"=="1" (
    %CONAN_CMD% install . --output-folder="%BUILD_DIR%" --build=missing -s build_type=%BUILD_TYPE% -c tools.system.package_manager:mode=install
) else (
    echo This may take a few minutes...
    %CONAN_CMD% install . --output-folder="%BUILD_DIR%" --build=missing -s build_type=%BUILD_TYPE% -c tools.system.package_manager:mode=install > "%BUILD_DIR%\conan-install.log" 2>&1
    if errorlevel 1 (
        echo [ERROR] Conan installation failed. See log:
        type "%BUILD_DIR%\conan-install.log"
        exit /b 1
    )
)
echo [OK] Dependencies installed
exit /b 0

:configure_cmake
echo.
echo [STEP] Configuring CMake...
cd /d "%PROJECT_ROOT%"

if exist "CMakeUserPresets.json" (
    if "%BUILD_TYPE%"=="Release" set "PRESET=conan-release"
    if "%BUILD_TYPE%"=="Debug" set "PRESET=conan-debug"
    
    if "%VERBOSE%"=="1" (
        cmake --preset !PRESET!
    ) else (
        cmake --preset !PRESET! > "%BUILD_DIR%\cmake-config.log" 2>&1
    )
    
    if errorlevel 1 (
        echo [ERROR] CMake configuration failed
        if exist "%BUILD_DIR%\cmake-config.log" type "%BUILD_DIR%\cmake-config.log"
        exit /b 1
    )
) else (
    if not exist "%BUILD_DIR%" mkdir "%BUILD_DIR%"
    cd /d "%BUILD_DIR%"
    
    if exist "conan_toolchain.cmake" (
        set "TOOLCHAIN=conan_toolchain.cmake"
    ) else if exist "build\%BUILD_TYPE%\generators\conan_toolchain.cmake" (
        set "TOOLCHAIN=build\%BUILD_TYPE%\generators\conan_toolchain.cmake"
    ) else if exist "generators\conan_toolchain.cmake" (
        set "TOOLCHAIN=generators\conan_toolchain.cmake"
    ) else (
        echo [ERROR] Could not find conan_toolchain.cmake
        exit /b 1
    )
    
    cmake -DCMAKE_BUILD_TYPE=%BUILD_TYPE% -DCMAKE_TOOLCHAIN_FILE=!TOOLCHAIN! ..
    if errorlevel 1 (
        echo [ERROR] CMake configuration failed
        exit /b 1
    )
)

echo [OK] CMake configured
exit /b 0

:build_project
set "TARGET=%~1"
if "%TARGET%"=="" set "TARGET=all"

echo.
echo [STEP] Building %TARGET% (%BUILD_TYPE% mode)...

set "ACTUAL_BUILD_DIR=%BUILD_DIR%"
if exist "%BUILD_DIR%\build\%BUILD_TYPE%" set "ACTUAL_BUILD_DIR=%BUILD_DIR%\build\%BUILD_TYPE%"

cd /d "!ACTUAL_BUILD_DIR!"

if not "%TARGET%"=="all" (
    cmake --build . --config %BUILD_TYPE% -j %JOBS% --target %TARGET%
) else (
    cmake --build . --config %BUILD_TYPE% -j %JOBS%
)

if errorlevel 1 (
    echo [ERROR] Build failed
    exit /b 1
)

echo [OK] Build completed

if exist "C:\msys64\ucrt64\bin\libgcc_s_seh-1.dll" (
    echo [STEP] Copying MinGW runtime DLLs...
    copy /Y "C:\msys64\ucrt64\bin\libgcc_s_seh-1.dll" "%BUILD_DIR%\build\%BUILD_TYPE%\bin\" >nul 2>&1
    copy /Y "C:\msys64\ucrt64\bin\libwinpthread-1.dll" "%BUILD_DIR%\build\%BUILD_TYPE%\bin\" >nul 2>&1
    copy /Y "C:\msys64\ucrt64\bin\libstdc++-6.dll" "%BUILD_DIR%\build\%BUILD_TYPE%\bin\" >nul 2>&1
    echo [OK] Runtime DLLs copied
)

exit /b 0

:do_full_build
set "BUILD_TARGET=%~1"

if "%CLEAN_BUILD%"=="1" (
    echo.
    echo [STEP] Cleaning build directory...
    if exist "%BUILD_DIR%" rd /s /q "%BUILD_DIR%"
    echo [OK] Build directory cleaned
)

call :check_conan || exit /b 1
call :check_cmake || exit /b 1
call :check_compiler || exit /b 1
call :install_deps || exit /b 1
call :configure_cmake || exit /b 1
call :build_project "%BUILD_TARGET%" || exit /b 1

exit /b 0

:run_client
echo.
echo [STEP] Launching R-TYPE Client...

set "BIN_DIR=%BUILD_DIR%\bin"
if exist "%BUILD_DIR%\build\%BUILD_TYPE%\bin" set "BIN_DIR=%BUILD_DIR%\build\%BUILD_TYPE%\bin"

if not exist "!BIN_DIR!\r-type_client.exe" (
    echo [ERROR] Client not found at: !BIN_DIR!\r-type_client.exe
    exit /b 1
)

cd /d "!BIN_DIR!"
echo ================================================
echo Starting client...%EXTRA_ARGS%
echo ================================================
r-type_client.exe%EXTRA_ARGS%
exit /b 0

:run_server
echo.
echo [STEP] Launching R-TYPE Server...

set "BIN_DIR=%BUILD_DIR%\bin"
if exist "%BUILD_DIR%\build\%BUILD_TYPE%\bin" set "BIN_DIR=%BUILD_DIR%\build\%BUILD_TYPE%\bin"

if not exist "!BIN_DIR!\r-type_server.exe" (
    echo [ERROR] Server not found at: !BIN_DIR!\r-type_server.exe
    exit /b 1
)

cd /d "!BIN_DIR!"
echo ================================================
echo Starting server...%EXTRA_ARGS%
echo ================================================
r-type_server.exe%EXTRA_ARGS%
exit /b 0

:run_tests
echo.
echo [STEP] Running tests...

set "TEST_DIR=%BUILD_DIR%"
if exist "%BUILD_DIR%\build\%BUILD_TYPE%" set "TEST_DIR=%BUILD_DIR%\build\%BUILD_TYPE%"

cd /d "!TEST_DIR!"
ctest --output-on-failure -C %BUILD_TYPE%

if errorlevel 1 (
    echo [ERROR] Tests failed
    exit /b 1
)

echo [OK] All tests passed!
exit /b 0

:end_success
echo.
echo [OK] Done!
exit /b 0

:show_help
echo.
echo ========================================================
echo                   R-TYPE Manager
echo            Build * Run * Test * Analyze
echo ========================================================
echo.
echo USAGE:
echo     r-type.bat [COMMAND] [OPTIONS] [-- ARGS]
echo.
echo COMMANDS:
echo     build               Build the project (default: Release)
echo     client [ARGS]       Build and run the client with optional arguments
echo     server [ARGS]       Build and run the server with optional arguments
echo     test                Build and run tests
echo     clean               Clean build directory
echo     install             Install dependencies only
echo     rebuild             Clean and build from scratch
echo     all                 Build everything
echo.
echo OPTIONS:
echo     -d, --debug         Build in Debug mode
echo     -r, --release       Build in Release mode (default)
echo     -c, --clean         Clean before building
echo     -v, --verbose       Verbose output
echo     -h, --help          Show this help message
echo.
echo EXAMPLES:
echo     r-type.bat server
echo     r-type.bat server -p 5000
echo     r-type.bat client -h 10.84.106.198
echo     r-type.bat client -h 192.168.1.10 -p 4242
echo     r-type.bat build --debug
echo     r-type.bat rebuild
echo.
exit /b 0