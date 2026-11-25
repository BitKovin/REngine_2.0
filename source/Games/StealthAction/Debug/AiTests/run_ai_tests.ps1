# Runs the automated AI scenarios headless and reports. See Debug/AiTests/AiTest.h.
#
#   .\run_ai_tests.ps1                         every built scenario
#   .\run_ai_tests.ps1 -Filter chase,squad     tiers or scenario names
#   .\run_ai_tests.ps1 -Runs 5                 repeat each scenario (random generators are not seedable)
#   .\run_ai_tests.ps1 -Exe path\to\StealthAction.exe
#
# Results: SaveData/ai_tests/summary.log, result.json, trace_<scenario>.csv. Exit code 1 when a scenario failed.

param(
	[string[]]$Filter = @("all"),
	[int]$Runs = 1,
	[int]$TimeoutSeconds = 1800,
	[string]$Exe = ""
)

$gameDir = Resolve-Path (Join-Path $PSScriptRoot "..\..")
$repo = Resolve-Path (Join-Path $gameDir "..\..\..")

if ($Exe -eq "") {
	$candidates = @(
		(Join-Path $repo "source\Debug\StealthAction.exe"),
		(Join-Path $repo "Build\StealthAction\windows\bin\StealthAction.exe")
	)
	$Exe = $candidates | Where-Object { Test-Path $_ } | Sort-Object { (Get-Item $_).LastWriteTime } -Descending | Select-Object -First 1
}

if (-not $Exe -or -not (Test-Path $Exe)) {
	Write-Error "StealthAction.exe not found; build it or pass -Exe"
	exit 2
}

$resultDir = Join-Path $gameDir "SaveData\ai_tests"
if (Test-Path $resultDir) { Remove-Item (Join-Path $resultDir "*") -Force -ErrorAction SilentlyContinue }

# The game runs from its own folder so that GameData is found (staged copies of new data files only appear after a relink).
$arguments = @("-headless", "-no_map", "-ai_test", ($Filter -join ","), "-ai_runs", "$Runs")
Write-Host "$Exe $($arguments -join ' ')"

$process = Start-Process -FilePath $Exe -ArgumentList $arguments -WorkingDirectory $gameDir -PassThru -WindowStyle Hidden
if (-not $process.WaitForExit($TimeoutSeconds * 1000)) {
	Write-Warning "timeout after $TimeoutSeconds s, stopping the game"
	Stop-Process -Id $process.Id -Force
}

$summary = Join-Path $resultDir "summary.log"
$result = Join-Path $resultDir "result.json"

if (-not (Test-Path $result)) {
	Write-Error "no result.json: the run did not finish (see log.txt in $gameDir)"
	exit 2
}

Get-Content $summary
$json = Get-Content $result -Raw | ConvertFrom-Json
Write-Host ""
Write-Host "passed $($json.passed), failed $($json.failed)"

if ($json.failed -gt 0) { exit 1 }
exit 0
