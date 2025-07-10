cd ./../windows/PixVideoCore

@set FF_ARG1=%1

echo "build pixvideocore"
"C:\Program Files (x86)\Microsoft Visual Studio\2019\Professional\MSBuild\Current\Bin\MSBuild.exe" PixVideoCore.sln /p:Configuration=Release /p:Platform=x64 /t:rebuild
"C:\Program Files (x86)\Microsoft Visual Studio\2019\Professional\MSBuild\Current\Bin\MSBuild.exe" PixVideoCore.sln /p:Configuration=Release /p:Platform=x86 /t:rebuild

cd ./../PixVideo

@set FF_ARG1=%1

if "%FF_ARG1%"=="pxauth" (
    echo "build with pxauth"
    "C:\Program Files (x86)\Microsoft Visual Studio\2019\Professional\MSBuild\Current\Bin\MSBuild.exe" PixVideo.sln /p:Configuration=Release /p:Platform=x64 /p:ForceImportBeforeCppTargets="%cd%\PixVideo.props" /t:rebuild
    "C:\Program Files (x86)\Microsoft Visual Studio\2019\Professional\MSBuild\Current\Bin\MSBuild.exe" PixVideo.sln /p:Configuration=Release /p:Platform=x86 /p:ForceImportBeforeCppTargets="%cd%\PixVideo.props" /t:rebuild
) else (
    echo "build without pxauth"
    "C:\Program Files (x86)\Microsoft Visual Studio\2019\Professional\MSBuild\Current\Bin\MSBuild.exe" PixVideo.sln /p:Configuration=Release /p:Platform=x64 /t:rebuild
    "C:\Program Files (x86)\Microsoft Visual Studio\2019\Professional\MSBuild\Current\Bin\MSBuild.exe" PixVideo.sln /p:Configuration=Release /p:Platform=x86 /t:rebuild
)