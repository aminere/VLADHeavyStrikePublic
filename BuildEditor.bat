
echo Building Editor..
call "C:\Program Files (x86)\MSBuild\14.0\Bin\msbuild.exe" "Game\build\vc11\editor\ShootTestEditorLauncher.sln" /m /p:Configuration="Release OptimOff" /v:normal
