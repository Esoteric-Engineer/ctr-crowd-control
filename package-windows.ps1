# Package the current Windows x86 build for release.

param(
    [string]$Version,
    [string]$BuildDir = "build",
    [string]$DistDir = "dist"
)

$ErrorActionPreference = "Stop"

if (-not $Version) {
    $match = Select-String -Path "CMakeLists.txt" -Pattern 'CTR_NATIVE_VERSION\s+"([^"]+)"' | Select-Object -First 1
    if (-not $match) {
        throw "Could not find CTR_NATIVE_VERSION in CMakeLists.txt"
    }
    $Version = $match.Matches[0].Groups[1].Value
}

$packageName = "ctr-native-$Version-windows-x86"
$packageDir = Join-Path $DistDir $packageName
$binaryPath = Join-Path $BuildDir "ctr_native.exe"

if (-not (Test-Path $binaryPath)) {
    Write-Error "Missing executable: $binaryPath`nRun build-msvc.bat first."
    exit 1
}

if (Test-Path $packageDir) {
    Remove-Item -Recurse -Force $packageDir
}
New-Item -ItemType Directory -Force -Path $packageDir | Out-Null
New-Item -ItemType Directory -Force -Path (Join-Path $packageDir "pack") | Out-Null

Copy-Item $binaryPath $packageDir
Copy-Item "LICENSE" $packageDir
Copy-Item "THIRD_PARTY_NOTICES.md" $packageDir
Copy-Item "pack\CrashTeamRacingNative.cs" (Join-Path $packageDir "pack")

$readmeText = @"
CTR Native Windows x86 $Version build

Game assets are not included.

Simple setup:
- Put your own NTSC-U retail CTR disc image at:

assets\
  ctr-u.bin

- Run:

ctr_native.exe

Playing with Crowd Control:
- Install the Crowd Control desktop app and the Crowd Control SDK
  (https://github.com/WarpWorld/CrowdControl.SDK/releases).
- Point the SDK at pack\CrashTeamRacingNative.cs (included in this download) and load it as a local pack.
- Run: ctr_native.exe --crowd-control

The disc image must be the common single-track raw PSX BIN layout:
MODE2/2352 sectors, with the data track starting at byte 0.
A cooked 2048-byte ISO does not preserve the XA/STR sector data needed for
audio and video playback.

Extracted asset override:

Extracted files are optional and mostly useful for development, modding, and
debugging. If present, they override files from ctr-u.bin.

assets\
  BIGFILE.BIG
  SOUNDS\KART.HWL
  TEST.STR
  XA\
    ENG.XNF
    ENG\EXTRA\S00.XA ... S05.XA
    ENG\GAME\S00.XA ... S20.XA
    MUSIC\S00.XA ... S01.XA

XA files must preserve CD-XA sector data. Use 2336-byte Mode2/Form2 sector data
or 2352-byte raw sectors. 2048-byte cooked XA extractions are not suitable.

Windows requirements:
- 64-bit Windows 10 or later, capable of running 32-bit applications
- A GPU/driver supporting OpenGL 3.3

If the game does not launch, run it from a command prompt and include:
- Windows version
- GPU/driver
- console output
"@

Set-Content -Path (Join-Path $packageDir "README.txt") -Value $readmeText

$zipPath = Join-Path $DistDir "$packageName.zip"
if (Test-Path $zipPath) {
    Remove-Item -Force $zipPath
}
Compress-Archive -Path $packageDir -DestinationPath $zipPath

$hash = Get-FileHash -Path $zipPath -Algorithm SHA256
"$($hash.Hash.ToLower())  $packageName.zip" | Set-Content -Path "$zipPath.sha256"

Write-Host "Wrote $zipPath"
Write-Host "Wrote $zipPath.sha256"
