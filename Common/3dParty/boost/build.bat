SET SCRIPTPATH=%~dp0
CD /D %~dp0

SET outputdir=%SCRIPTPATH%boost_1_68_0\build\%platform%
echo "%outputdir%"

SET TOOLSET=msvc-14.0

CD "%SCRIPTPATH%\boost_1_68_0"
call .\bootstrap.bat

if exist "stage" (
	RMDIR "stage" /S /Q
)

md build

SET folder=build/win_32

if not exist "%folder%" (
  echo "x86..."
  
  md  %folder%
  md  %folder%\static
  md  %folder%\static_fpic
  md  %folder%\shared

  .\b2.exe --clean
  .\bjam.exe link=static --with-filesystem --with-system --with-date_time --with-regex --toolset=%TOOLSET%
  XCOPY /Y stage\lib\* "%folder%\static\"

  .\b2.exe --clean
  .\bjam.exe link=static cxxflags=-fPIC --with-filesystem --with-system --with-date_time --with-regex --toolset=%TOOLSET%
  XCOPY /Y stage\lib\* "%folder%\static_fpic\"

  .\b2.exe --clean
  .\bjam.exe link=shared --with-filesystem --with-system --with-date_time --with-regex --toolset=%TOOLSET%
  XCOPY /Y stage\lib\* "%folder%\shared\"
)

SET folder=build/win_64


if not exist "%folder%" (
  echo "x64..."
  
  if exist "stage" (
    RMDIR "stage" /S /Q
  )

  md  %folder%
  md  %folder%\static
  md  %folder%\static_fpic
  md  %folder%\shared

  .\b2.exe --clean
  .\bjam.exe link=static --with-filesystem --with-system --with-date_time --with-regex address-model=64 --toolset=%TOOLSET%
  XCOPY /Y stage\lib\* "%folder%\static\"

  .\b2.exe --clean
  .\bjam.exe link=static cxxflags=-fPIC --with-filesystem --with-system --with-date_time --with-regex address-model=64 --toolset=%TOOLSET%
  XCOPY /Y stage\lib\* "%folder%\static_fpic\"

  .\b2.exe --clean
  .\bjam.exe link=shared --with-filesystem --with-system --with-date_time --with-regex address-model=64 --toolset=%TOOLSET%
  XCOPY /Y stage\lib\* "%folder%\shared\"
)
