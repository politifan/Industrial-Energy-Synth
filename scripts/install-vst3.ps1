Param(
  [ValidateSet("Debug", "Release")]
  [string]$Config = "Release",

  # Install destination. Recommended: a user-writable folder you add in Reaper VST paths, e.g. C:\VST3
  [string]$Dest = "C:\\VST3"
)

$ErrorActionPreference = "Stop"

$buildDir = if ($Config -eq "Debug") { "build-win-debug" } else { "build-win-release" }

if (-not (Test-Path $buildDir)) {
  throw "Build dir not found: $buildDir. Run scripts/build-win.ps1 first."
}

$candidates = Get-ChildItem -Path $buildDir -Recurse -Directory -Filter "*.vst3" |
  Where-Object { $_.Name -eq "Industrial Energy Synth.vst3" }

if ($candidates.Count -eq 0) {
  # Fallback: any vst3
  $candidates = Get-ChildItem -Path $buildDir -Recurse -Directory -Filter "*.vst3"
}

if ($candidates.Count -eq 0) {
  throw "No .vst3 plugin folder found under $buildDir."
}

$src = $candidates[0].FullName

New-Item -ItemType Directory -Force -Path $Dest | Out-Null

$dst = Join-Path $Dest (Split-Path $src -Leaf)

Write-Host "Installing:"
Write-Host "  from: $src"
Write-Host "  to:   $dst"

if (Test-Path $dst) {
  Remove-Item -Recurse -Force $dst
}

Copy-Item -Recurse -Force $src $dst

Write-Host "Installed. In Reaper: Preferences -> Plug-ins -> VST -> Re-scan."

