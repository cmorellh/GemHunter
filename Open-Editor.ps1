# Opens this project in the Unreal Editor.
#
# Why this script exists: double-clicking GemHunter.uproject routes through the
# launcher's UnrealVersionSelector, which resolves the engine by looking up
# HKLM\SOFTWARE\EpicGames\Unreal Engine\<version>\InstalledDirectory. UE 5.8 is
# not registered there on CMH-TechAltz, so that lookup fails. This calls the
# editor directly and skips the registry.
#
# Usage:   .\Open-Editor.ps1
#          .\Open-Editor.ps1 -Wait          # keep the console attached
#          .\Open-Editor.ps1 -Engine 'D:\UE\UE_5.9'
#
# See docs/Editor Setup.md

[CmdletBinding()]
param(
    # Engine root. Override to try the project against another install.
    [string] $Engine,

    # Block until the editor exits, instead of returning the prompt immediately.
    [switch] $Wait
)

$ErrorActionPreference = 'Stop'

$projectPath = Join-Path $PSScriptRoot 'GemHunter.uproject'
if (-not (Test-Path -LiteralPath $projectPath)) {
    throw "GemHunter.uproject not found next to this script (looked in '$PSScriptRoot')."
}

# Candidate engine roots, best first. The registry is deliberately not consulted:
# it lists 5.5 (deleted) and 5.6 (an empty stub) but not the real 5.8.
$candidates = @()
if ($Engine) { $candidates += $Engine }
$candidates += 'E:\UE\UE_5.8'

# Fall back to whatever the launcher says is installed, so a future engine
# upgrade does not require editing this script.
$launcherDat = 'C:\ProgramData\Epic\UnrealEngineLauncher\LauncherInstalled.dat'
if (Test-Path -LiteralPath $launcherDat) {
    try {
        $installed = (Get-Content -LiteralPath $launcherDat -Raw | ConvertFrom-Json).InstallationList
        # Newest first, so a 5.9 install wins over 5.8 once one exists.
        $candidates += $installed |
            Where-Object { $_.AppName -like 'UE_*' } |
            Sort-Object -Property AppName -Descending |
            Select-Object -ExpandProperty InstallLocation
    }
    catch {
        Write-Verbose "Could not parse $launcherDat : $($_.Exception.Message)"
    }
}

$editor = $null
foreach ($root in ($candidates | Where-Object { $_ } | Select-Object -Unique)) {
    $exe = Join-Path $root 'Engine\Binaries\Win64\UnrealEditor.exe'
    if (Test-Path -LiteralPath $exe) { $editor = $exe; break }
    Write-Verbose "No editor at '$exe'"
}

if (-not $editor) {
    throw ("Could not find UnrealEditor.exe. Checked:`n  " +
           (($candidates | Where-Object { $_ } | Select-Object -Unique) -join "`n  ") +
           "`nPass -Engine '<path to engine root>' to point this at your install.")
}

Write-Host "Engine  : $editor"
Write-Host "Project : $projectPath"

if ($Wait) {
    & $editor $projectPath
    exit $LASTEXITCODE
}

# Detached, so the console comes straight back.
Start-Process -FilePath $editor -ArgumentList "`"$projectPath`""
Write-Host 'Editor launching. First open compiles shaders, so give it a few minutes.'
