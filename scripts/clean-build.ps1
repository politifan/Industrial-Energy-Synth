Param(
  [ValidateSet("win-debug", "win-release", "linux", "tests-linux", "all")]
  [string]$Target = "all"
)

$ErrorActionPreference = "Stop"

$dirs = @()
switch ($Target) {
  "win-debug"   { $dirs = @("build-win-debug") }
  "win-release" { $dirs = @("build-win-release") }
  "linux"       { $dirs = @("build-linux") }
  "tests-linux" { $dirs = @("build-tests-linux") }
  "all"         { $dirs = @("build-win-debug", "build-win-release", "build-linux", "build-tests-linux", "build-win") }
}

foreach ($d in $dirs) {
  if (Test-Path $d) {
    Write-Host "Removing: $d"
    Remove-Item -Recurse -Force $d
  }
}

Write-Host "Done."

