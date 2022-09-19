param (
    [string]$arch = 'x64'
)

$buildArch = 'x64'

# only x86 / x64 is supported
if(-NOT (($arch -eq 'x64') -or ($arch -eq 'x86'))) {
	Write-Host "Invalid argument '$arch' - only 'x86' and 'x64' is supported"
	return
}

if($arch -eq 'x86') {
	$buildArch = 'Win32'
}

$buildPath = "./build/win-$arch"

Write-Host "##################################################################"
Write-Host "glfw build script"
Write-Host "##################################################################"
Write-Host ""
Write-Host "-----------------------------"
Write-Host "### configuration"
Write-Host "-----------------------------"

Write-Host "arch: $arch"
Write-Host "path: $buildPath"

Write-Host ""
Write-Host "-----------------------------"
Write-Host "### preparation:"
Write-Host "-----------------------------"
Write-Host ""

if (Test-Path $buildPath) {
	Get-ChildItem -Path $buildPath -Recurse | Remove-Item -force -recurse
	Remove-Item $buildPath -force
	Write-Host "deleted build folder"
	Write-Host ""
}

New-Item -ItemType directory -Path $buildPath | Out-Null
Write-Host "created build folder"
Write-Host ""
Write-Host "-----------------------------"
Write-Host "### cmake - preparing build"
Write-Host "-----------------------------"
Write-Host ""

cmake -S . -B $buildPath -A $buildArch -D BUILD_SHARED_LIBS=ON

Write-Host ""
Write-Host "-----------------------------"
Write-Host "### cmake - building $arch"
Write-Host "-----------------------------"
Write-Host ""

cd $buildPath
cmake --build . --config Release

cd ..\..
Write-Host ""
Write-Host "-----------------------------"

$libOutputDir = "$buildPath/src/Release/*"
$examplesOutputDir = "$buildPath/examples/Release"
$testsOutputDir = "$buildPath/tests/Release"
Copy-item -Force -Recurse -Verbose $libOutputDir -Destination $examplesOutputDir
Copy-item -Force -Recurse -Verbose $libOutputDir -Destination $testsOutputDir
