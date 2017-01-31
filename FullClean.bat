rmdir /s /q "%~dp0Intermediate"
rmdir /s /q "%~dp0Binaries"
rmdir /s /q "%~dp0Build"

mkdir "%~dp0Intermediate"
mkdir "%~dp0Binaries"
mkdir "%~dp0Build"

del /f /s /q "%~dp0PretendEmpyres.sln"
del /f /s /q "%~dp0PretendEmpyres.sdf"

"%~dp0..\UnrealEngine\GenerateProjectFiles.bat" -2015 -Game "%~dp0PretendEmpyres.uproject" -Engine