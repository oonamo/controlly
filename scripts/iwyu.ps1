param(
  [string]$FixIncludesScript = "fix_includes.py",
  [string]$BuildDir = "build"
)

if (-Not (Test-Path $FixIncludesScript)) {
  $MsysBinFallback = "C:\msys64\mingw64\bin\fix_includes.py"

  if (-Not (Test-Path $MsysBinFallback)) {
    Write-Host "Error: scripts '$FixIncludesScript' and '$MsysBinFallback' were not found. Install the script before using this script" -ForegroundColor Red
    exit 1
  }

  Write-Host "Note: Using msys64 fallback '$MsysBinFallback'" -ForegroundColor Cyan
  $FixIncludesScript = $MsysBinFallback
}

$LogFile = "$BuildDir/iwyu.log"

Write-Host "Starting IWYU automated" -ForegroundColor Cyan

if (-Not (Test-Path $BuildDir)) {
  Write-Host "Error: Build directory '$BuildDir' not found. Please run CMake configuration first." -ForegroundColor Red
  exit 1
}

Write-Host "Cleaning build directory..." -ForegroundColor Yellow
cmake --build $BuildDir --target clean

Write-Host "Building and Capturing output to $LogFile" -ForegroundColor Yellow
cmake --build $BuildDir *> "$LogFile"

Write-Host "Forcing angle brackets for <controlly/*> prefix" -ForegroundColor Yellow
(Get-Content $LogFile) -Replace '"controlly/(.*?)"', '<controlly/$1>' | Set-Content $LogFile

Write-Host "Applying fixes" -ForegroundColor Yellow
Get-Content $LogFile | python $FixIncludesScript --noblank_lines --reorder

Write-Host "Done!" -ForegroundColor Green
