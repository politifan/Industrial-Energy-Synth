Param(
  [ValidateSet("Debug", "Release")]
  [string]$Config = "Release",

  [int]$Jobs = 8
)

$ErrorActionPreference = "Stop"

Write-Host "Industrial Energy Synth build (Windows) - $Config"
Write-Host "Note: run from VS Developer PowerShell / Native Tools prompt (cl.exe must be available)."

$preset = if ($Config -eq "Debug") { "win-debug" } else { "win-release" }

cmake --preset $preset
cmake --build --preset $preset -j $Jobs

Write-Host "Build done."
Write-Host "Look for VST3 under build dir, typically:"
Write-Host "  build-win-$($Config.ToLower())*/IndustrialEnergySynth_artefacts/$Config/VST3/*.vst3"

