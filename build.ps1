param(
    [string]$NDK = "C:\Users\CamBam\AppData\Local\Android\Sdk\ndk\25.2.9519653",
    [string[]]$Abis = @("arm64-v8a"),
    [string]$AndroidPlatform = "android-21",
    [string]$ProjectRoot = (Get-Location).Path
)

function Find-CMake {
    $cmake = Get-Command cmake -ErrorAction SilentlyContinue
    if ($cmake) { return $cmake.Source }

    $sdk = "$env:LOCALAPPDATA\Android\Sdk"
    if (Test-Path $sdk) {
        $cmakeDirs = Get-ChildItem "$sdk\cmake" -Directory -ErrorAction SilentlyContinue | Sort-Object LastWriteTime -Descending
        if ($cmakeDirs -and $cmakeDirs.Count -gt 0) {
            $exe = Join-Path $cmakeDirs[0].FullName "bin\cmake.exe"
            if (Test-Path $exe) { return $exe }
        }
    }
    return $null
}

$cmakeExe = Find-CMake
if (-not $cmakeExe) {
    Write-Error "CMake not found. Install CMake or specify PATH to cmake.exe. (Use winget: 'winget install --id Kitware.CMake -e')"
    exit 1
}

if (-not (Test-Path $NDK)) {
    Write-Error "NDK not found at '$NDK'. Pass -NDK <path> or install the Android NDK."
    exit 1
}

$ninja = Get-Command ninja -ErrorAction SilentlyContinue
if (-not $ninja) {
    Write-Warning "Ninja not found on PATH. Install ninja or change generator in script."
}

foreach ($abi in $Abis) {
    $BuildDir = Join-Path $ProjectRoot "build\$abi"
    if (Test-Path $BuildDir) { Remove-Item $BuildDir -Recurse -Force }
    New-Item -ItemType Directory -Path $BuildDir | Out-Null

    $toolchain = Join-Path $NDK "build\cmake\android.toolchain.cmake"
    if (-not (Test-Path $toolchain)) {
        Write-Error "Toolchain file not found: $toolchain"
        exit 1
    }

    & "$cmakeExe" -S $ProjectRoot -B $BuildDir -G "Ninja" `
        -DANDROID_ABI=$abi `
        -DANDROID_PLATFORM=$AndroidPlatform `
        -DANDROID_NDK=$NDK `
        -DCMAKE_TOOLCHAIN_FILE="$toolchain" `
        -DCMAKE_BUILD_TYPE=Release

    if ($LASTEXITCODE -ne 0) { Write-Error "CMake configure failed for $abi"; exit 1 }

    & "$cmakeExe" --build $BuildDir --config Release -- -j
    if ($LASTEXITCODE -ne 0) { Write-Error "Build failed for $abi"; exit 1 }

    Write-Host "Built $abi -> $ProjectRoot\src\main\jniLibs\$abi\libshadowclient.so"
}

Write-Host "All done."
