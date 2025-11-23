@echo off
REM R-TYPE - All-in-One Script (Windows)
REM Build, run, test, and analyze the R-TYPE project
REM Usage: r-type.bat [command] [options]

setlocal enabledelayedexpansion

REM ============================================================================
REM Global Variables
REM ============================================================================
set "PROJECT_ROOT=%~dp0"
set "BUILD_DIR=%PROJECT_ROOT%build"
set "BUILD_TYPE=Release"
set "CLEAN_BUILD=0"
set "VERBOSE=0"
set "COMMAND=build"

REM Detect number of processors
if not defined NUMBER_OF_PROCESSORS set NUMBER_OF_PROCESSORS=4
set "JOBS=%NUMBER_OF_PROCESSORS%"

REM Colors (Windows 10+)
set "RED=[91m"
set "GREEN=[92m"
set "YELLOW=[93m"
set "BLUE=[94m"
set "MAGENTA=[95m"
set "CYAN=[96m"
set "NC=[0m"

REM ============================================================================
REM Functions
REM ============================================================================

:print_banner
echo.
echo %CYAN%╔═══════════════════════════════════════════════════════╗%NC%
echo %CYAN%║                                                       ║%NC%
echo %CYAN%║                   R-TYPE Manager                      ║%NC%
echo %CYAN%║            Build • Run • Test • Analyze               ║%NC%
echo %CYAN%║                                                       ║%NC%
echo %CYAN%╚═══════════════════════════════════════════════════════╝%NC%
echo.
goto :eof

:show_help
call :print_banner
echo %YELLOW%USAGE:%NC%
echo     r-type.bat [COMMAND] [OPTIONS]
echo.
echo %YELLOW%COMMANDS:%NC%
echo     %GREEN%build%NC%               Build the project (default: Release)
echo     %GREEN%client%NC%              Build and run the client
echo     %GREEN%server%NC%              Build and run the server
echo     %GREEN%test%NC%                Build and run tests
echo     %GREEN%coverage%NC%            Generate code coverage report (requires OpenCppCoverage)
echo     %GREEN%clean%NC%               Clean build directory
echo     %GREEN%install%NC%             Install dependencies only
echo     %GREEN%rebuild%NC%             Clean and build from scratch
echo     %GREEN%all%NC%                 Build everything (client + server + tests)
echo.
echo %YELLOW%OPTIONS:%NC%
echo     -d, --debug             Build in Debug mode
echo     -r, --release           Build in Release mode (default)
echo     -c, --clean             Clean before building
echo     -v, --verbose           Verbose output
echo     -j N, --jobs N          Number of parallel jobs (default: auto)
echo     -h, --help              Show this help message
echo.
echo %YELLOW%EXAMPLES:%NC%
echo     %CYAN%# Quick start (build + run server)%NC%
echo     r-type.bat server
echo.
echo     %CYAN%# Build in Debug mode%NC%
echo     r-type.bat build --debug
echo.
echo     %CYAN%# Clean rebuild%NC%
echo     r-type.bat rebuild
echo.
goto :eof

:print_step
echo.
echo %BLUE%▶ %~1%NC%
goto :eof

:print_success
echo %GREEN%✓ %~1%NC%
goto :eof

:print_error
echo %RED%✗ %~1%NC%
goto :eof

:print_warning
echo %YELLOW%⚠ %~1%NC%
goto :eof

:command_exists
where %1 >nul 2>&1
goto :eof

:get_bin_dir
if exist "%BUILD_DIR%\build\%BUILD_TYPE%\bin" (
    set "BIN_DIR=%BUILD_DIR%\build\%BUILD_TYPE%\bin"
) else if exist "%BUILD_DIR%\bin" (
    set "BIN_DIR=%BUILD_DIR%\bin"
) else (
    set "BIN_DIR=%BUILD_DIR%\bin"
)
goto :eof

REM ============================================================================
REM Check Functions
REM ============================================================================

:check_conan
call :print_step "Checking Conan installation..."

call :command_exists conan
if %ERRORLEVEL% neq 0 (
    call :print_warning "Conan not found. Installing..."
    call :command_exists pip
    if %ERRORLEVEL% neq 0 (
        call :print_error "pip not found. Please install Python 3 and pip first."
        echo   Download from: https://www.python.org/downloads/
        exit /b 1
    )
    pip install --user conan
    if %ERRORLEVEL% neq 0 (
        call :print_error "Failed to install Conan"
        exit /b 1
    )
    conan profile detect --force
    call :print_success "Conan installed successfully"
) else (
    call :print_success "Conan is already installed"
)
goto :eof

:check_cmake
call :print_step "Checking CMake..."

call :command_exists cmake
if %ERRORLEVEL% neq 0 (
    call :print_error "CMake not found. Please install CMake 3.20 or higher."
    echo   Download from: https://cmake.org/download/
    exit /b 1
)

for /f "tokens=3" %%i in ('cmake --version ^| findstr /R "[0-9]"') do (
    call :print_success "CMake %%i found"
    goto :cmake_found
)
:cmake_found
goto :eof

:check_compiler
call :print_step "Checking C++ compiler..."

call :command_exists cl
if %ERRORLEVEL% equ 0 (
    for /f "tokens=*" %%i in ('cl 2^>^&1 ^| findstr "Version"') do (
        call :print_success "MSVC found"
        goto :compiler_found
    )
)

call :command_exists g++
if %ERRORLEVEL% equ 0 (
    for /f "tokens=3" %%i in ('g++ --version ^| findstr "g++"') do (
        call :print_success "GCC %%i found"
        goto :compiler_found
    )
)

call :print_error "No C++ compiler found. Please install Visual Studio or MinGW."
exit /b 1

:compiler_found
goto :eof

REM ============================================================================
REM Build Functions
REM ============================================================================

:install_dependencies
cd /d "%PROJECT_ROOT%"

REM Check if dependencies are already installed
if exist "%BUILD_DIR%\conan_toolchain.cmake" goto :deps_found
if exist "%BUILD_DIR%\build\%BUILD_TYPE%\generators\conan_toolchain.cmake" goto :deps_found
if exist "%BUILD_DIR%\generators\conan_toolchain.cmake" goto :deps_found

call :print_step "Installing dependencies with Conan (this may take a few minutes)..."

REM Create build directory if it doesn't exist
if not exist "%BUILD_DIR%" mkdir "%BUILD_DIR%"

if "%VERBOSE%"=="1" (
    conan install . --output-folder="%BUILD_DIR%" --build=missing -s build_type=%BUILD_TYPE% -c tools.system.package_manager:mode=install
) else (
    conan install . --output-folder="%BUILD_DIR%" --build=missing -s build_type=%BUILD_TYPE% -c tools.system.package_manager:mode=install > "%BUILD_DIR%\conan-install.log" 2>&1
    if %ERRORLEVEL% neq 0 (
        call :print_error "Conan installation failed. Check %BUILD_DIR%\conan-install.log for details."
        type "%BUILD_DIR%\conan-install.log" | more +20
        exit /b 1
    )
)

call :print_success "Dependencies installed"
goto :eof

:deps_found
call :print_success "Dependencies already installed (skipping)"
goto :eof

:configure_cmake
call :print_step "Configuring CMake..."

cd /d "%PROJECT_ROOT%"

REM Use CMake preset if available
if exist "CMakeUserPresets.json" (
    if "%BUILD_TYPE%"=="Release" set "PRESET=conan-release"
    if "%BUILD_TYPE%"=="Debug" set "PRESET=conan-debug"
    if "%VERBOSE%"=="1" (
        cmake --preset !PRESET!
        set CMAKE_EXIT=!ERRORLEVEL!
    ) else (
        cmake --preset !PRESET! > "%BUILD_DIR%\cmake-config.log" 2>&1
        set CMAKE_EXIT=!ERRORLEVEL!
    )
    if !CMAKE_EXIT! neq 0 (
        call :print_error "CMake configuration failed"
        echo.
        if "%VERBOSE%"=="0" (
            echo Last 30 lines of %BUILD_DIR%\cmake-config.log:
            echo ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
            type "%BUILD_DIR%\cmake-config.log" | more +30
            echo ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
            echo.
            echo Tip: Run with --verbose for full output
        )
        exit /b 1
    )
) else (
    REM Fallback to manual configuration
    if not exist "%BUILD_DIR%" mkdir "%BUILD_DIR%"
    cd /d "%BUILD_DIR%"
    REM Find toolchain file
    if exist "conan_toolchain.cmake" (
        set "TOOLCHAIN=conan_toolchain.cmake"
    ) else if exist "build\%BUILD_TYPE%\generators\conan_toolchain.cmake" (
        set "TOOLCHAIN=build\%BUILD_TYPE%\generators\conan_toolchain.cmake"
    ) else if exist "generators\conan_toolchain.cmake" (
        set "TOOLCHAIN=generators\conan_toolchain.cmake"
    ) else (
        call :print_error "Could not find conan_toolchain.cmake"
        exit /b 1
    )
    cmake -DCMAKE_BUILD_TYPE=%BUILD_TYPE% -DCMAKE_TOOLCHAIN_FILE=!TOOLCHAIN! ..
    if %ERRORLEVEL% neq 0 (
        call :print_error "CMake configuration failed"
        exit /b 1
    )
)

call :print_success "CMake configured"
goto :eof

:build_project
set "TARGET=%~1"
if "%TARGET%"=="" set "TARGET=all"

call :print_step "Building %TARGET% (%BUILD_TYPE% mode)..."

REM Determine actual build directory
set "ACTUAL_BUILD_DIR=%BUILD_DIR%"
if exist "%BUILD_DIR%\build\%BUILD_TYPE%" (
    set "ACTUAL_BUILD_DIR=%BUILD_DIR%\build\%BUILD_TYPE%"
)

cd /d "!ACTUAL_BUILD_DIR!"

if not "%TARGET%"=="all" (
    set "BUILD_CMD=cmake --build . --config %BUILD_TYPE% -j %JOBS% --target %TARGET%"
) else (
    set "BUILD_CMD=cmake --build . --config %BUILD_TYPE% -j %JOBS%"
)

if "%VERBOSE%"=="1" (
    !BUILD_CMD!
) else (
    !BUILD_CMD! 2>&1 | findstr /R /C:"Building" /C:"Linking" /C:"Error" /C:"error:" /C:"warning:"
)

if %ERRORLEVEL% neq 0 (
    call :print_error "Build failed"
    exit /b 1
)

call :print_success "Build completed"
goto :eof

:clean_build
call :print_step "Cleaning build directory..."

if exist "%BUILD_DIR%" (
    rd /s /q "%BUILD_DIR%"
    call :print_success "Build directory cleaned"
) else (
    call :print_warning "Build directory doesn't exist"
)
goto :eof

REM ============================================================================
REM Run Functions
REM ============================================================================

:run_client
call :print_step "Launching R-TYPE Client..."

call :get_bin_dir

if not exist "%BIN_DIR%\r-type_client.exe" (
    call :print_error "Client not found. Building..."
    call :do_build r-type_client
    call :get_bin_dir
)

cd /d "%BIN_DIR%"
echo %CYAN%━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━%NC%
r-type_client.exe %*
goto :eof

:run_server
call :print_step "Launching R-TYPE Server..."

call :get_bin_dir

if not exist "%BIN_DIR%\r-type_server.exe" (
    call :print_error "Server not found. Building..."
    call :do_build r-type_server
    call :get_bin_dir
)

cd /d "%BIN_DIR%"
echo %CYAN%━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━%NC%
r-type_server.exe %*
goto :eof

:run_tests
call :print_step "Running tests..."

if not exist "%BUILD_DIR%" (
    call :print_error "Build directory not found. Building..."
    call :do_build
)

REM Determine test directory
set "TEST_DIR=%BUILD_DIR%"
if exist "%BUILD_DIR%\build\%BUILD_TYPE%" (
    set "TEST_DIR=%BUILD_DIR%\build\%BUILD_TYPE%"
)

cd /d "!TEST_DIR!"
ctest --output-on-failure -C %BUILD_TYPE%

if %ERRORLEVEL% neq 0 (
    call :print_error "Tests failed"
    exit /b 1
)

call :print_success "All tests passed!"
goto :eof

:generate_coverage
call :print_step "Generating code coverage report..."

call :print_warning "Note: On Windows, coverage requires OpenCppCoverage"
echo   Install from: https://github.com/OpenCppCoverage/OpenCppCoverage/releases
echo   Or with Chocolatey: choco install opencppcoverage
echo.

call :command_exists OpenCppCoverage
if %ERRORLEVEL% neq 0 (
    call :print_error "OpenCppCoverage not found"
    exit /b 1
)

if not "%BUILD_TYPE%"=="Debug" (
    call :print_warning "Coverage requires Debug build. Switching to Debug mode..."
    set "BUILD_TYPE=Debug"
)

call :do_build

REM Determine test directory
set "TEST_DIR=%BUILD_DIR%"
if exist "%BUILD_DIR%\build\%BUILD_TYPE%" (
    set "TEST_DIR=%BUILD_DIR%\build\%BUILD_TYPE%"
)

cd /d "!TEST_DIR!"

call :get_bin_dir
OpenCppCoverage --sources "%PROJECT_ROOT%" --export_type html:coverage_html -- ctest -C Debug

call :print_success "Coverage report generated in !TEST_DIR!\coverage_html\index.html"
echo.
echo Open the report with:
echo   start !TEST_DIR!\coverage_html\index.html
goto :eof

REM ============================================================================
REM Main Build Workflow
REM ============================================================================

:do_build
set "TARGET=%~1"
if "%TARGET%"=="" set "TARGET=all"

if "%CLEAN_BUILD%"=="1" call :clean_build

call :check_conan
if %ERRORLEVEL% neq 0 exit /b 1

call :check_cmake
if %ERRORLEVEL% neq 0 exit /b 1

call :check_compiler
if %ERRORLEVEL% neq 0 exit /b 1

call :install_dependencies
if %ERRORLEVEL% neq 0 exit /b 1

call :configure_cmake
if %ERRORLEVEL% neq 0 exit /b 1

call :build_project %TARGET%
if %ERRORLEVEL% neq 0 exit /b 1

goto :eof

REM ============================================================================
REM Parse Arguments
REM ============================================================================

:parse_args
if "%~1"=="" goto :args_done

REM Commands
if /i "%~1"=="build" (
    set "COMMAND=build"
    shift
    goto :parse_args
)
if /i "%~1"=="client" (
    set "COMMAND=client"
    shift
    goto :parse_args
)
if /i "%~1"=="server" (
    set "COMMAND=server"
    shift
    goto :parse_args
)
if /i "%~1"=="test" (
    set "COMMAND=test"
    shift
    goto :parse_args
)
if /i "%~1"=="coverage" (
    set "COMMAND=coverage"
    shift
    goto :parse_args
)
if /i "%~1"=="clean" (
    set "COMMAND=clean"
    shift
    goto :parse_args
)
if /i "%~1"=="install" (
    set "COMMAND=install"
    shift
    goto :parse_args
)
if /i "%~1"=="rebuild" (
    set "COMMAND=rebuild"
    shift
    goto :parse_args
)
if /i "%~1"=="all" (
    set "COMMAND=all"
    shift
    goto :parse_args
)

REM Options
if /i "%~1"=="-d" (
    set "BUILD_TYPE=Debug"
    shift
    goto :parse_args
)
if /i "%~1"=="--debug" (
    set "BUILD_TYPE=Debug"
    shift
    goto :parse_args
)
if /i "%~1"=="-r" (
    set "BUILD_TYPE=Release"
    shift
    goto :parse_args
)
if /i "%~1"=="--release" (
    set "BUILD_TYPE=Release"
    shift
    goto :parse_args
)
if /i "%~1"=="-c" (
    set "CLEAN_BUILD=1"
    shift
    goto :parse_args
)
if /i "%~1"=="--clean" (
    set "CLEAN_BUILD=1"
    shift
    goto :parse_args
)
if /i "%~1"=="-v" (
    set "VERBOSE=1"
    shift
    goto :parse_args
)
if /i "%~1"=="--verbose" (
    set "VERBOSE=1"
    shift
    goto :parse_args
)
if /i "%~1"=="-j" (
    set "JOBS=%~2"
    shift
    shift
    goto :parse_args
)
if /i "%~1"=="--jobs" (
    set "JOBS=%~2"
    shift
    shift
    goto :parse_args
)
if /i "%~1"=="-h" (
    call :show_help
    exit /b 0
)
if /i "%~1"=="--help" (
    call :show_help
    exit /b 0
)

REM Unknown argument
call :print_warning "Unknown argument: %~1"
shift
goto :parse_args

:args_done
goto :eof

REM ============================================================================
REM Main Entry Point
REM ============================================================================

:main
REM Parse command line arguments
call :parse_args %*

REM Execute command
if /i "%COMMAND%"=="build" (
    call :print_banner
    call :do_build
    goto :done
)

if /i "%COMMAND%"=="client" (
    call :print_banner
    call :do_build r-type_client
    call :run_client
    goto :done
)

if /i "%COMMAND%"=="server" (
    call :print_banner
    call :do_build r-type_server
    call :run_server
    goto :done
)

if /i "%COMMAND%"=="test" (
    call :print_banner
    call :do_build
    call :run_tests
    goto :done
)

if /i "%COMMAND%"=="coverage" (
    call :print_banner
    call :generate_coverage
    goto :done
)

if /i "%COMMAND%"=="clean" (
    call :print_banner
    call :clean_build
    goto :done
)

if /i "%COMMAND%"=="install" (
    call :print_banner
    call :check_conan
    call :install_dependencies
    goto :done
)

if /i "%COMMAND%"=="rebuild" (
    call :print_banner
    set "CLEAN_BUILD=1"
    call :do_build
    goto :done
)

if /i "%COMMAND%"=="all" (
    call :print_banner
    call :do_build all
    goto :done
)

REM Unknown command
call :print_error "Unknown command: %COMMAND%"
call :show_help
exit /b 1

:done
echo.
call :print_success "Done! 🚀"
exit /b 0

REM Call main
call :main %*