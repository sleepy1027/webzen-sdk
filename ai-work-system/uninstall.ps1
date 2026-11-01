# work-system 제거. work-system이 설치한 것만 지운다 (installed.txt 기준). 기존 스킬·에이전트는 건드리지 않는다.
#   uninstall.cmd           : 플러그인·설치 파일·CLAUDE.md 연결 줄 제거, 개인 설정 보존
#   uninstall.cmd -Purge    : 개인 설정 폴더까지 삭제
param([switch]$Purge)
$ErrorActionPreference = "Continue"
$ClaudeHome = if ($env:CLAUDE_CONFIG_DIR) { $env:CLAUDE_CONFIG_DIR } else { Join-Path $env:USERPROFILE ".claude" }
$HomeDir = Join-Path $ClaudeHome "work-system"
$Manifest = Join-Path $HomeDir "installed.txt"
$Mkt = "work-system-local"
$Utf8 = New-Object System.Text.UTF8Encoding($false)
if (Get-Command claude -ErrorAction SilentlyContinue) {
  & claude plugin uninstall "work-system@$Mkt" *> $null; if ($LASTEXITCODE -eq 0) { Write-Host "  플러그인 제거" }
  & claude plugin marketplace remove $Mkt *> $null
}
if (Test-Path $Manifest) {
  foreach ($rel in (Get-Content $Manifest | Where-Object { $_ -match '^(skills|agents)/' })) {
    $p = Join-Path $ClaudeHome ($rel -replace '/', '\')
    if (Test-Path $p) { Remove-Item -Recurse -Force $p; Write-Host "  제거: $rel" }
  }
  [IO.File]::WriteAllText($Manifest, "", $Utf8)
}
$ClaudeMd = Join-Path $ClaudeHome "CLAUDE.md"
if (Test-Path $ClaudeMd) {
  $lines = @([IO.File]::ReadAllLines($ClaudeMd, [Text.Encoding]::UTF8) | Where-Object { -not $_.Contains("@~/.claude/work-system/AGENTS.md") -and -not $_.StartsWith("# work-system (AI 업무 시스템 지침") })
  [IO.File]::WriteAllLines($ClaudeMd, [string[]]$lines, $Utf8); Write-Host "  CLAUDE.md 연결 줄 제거"
}
if ($Purge) { Remove-Item -Recurse -Force $HomeDir; Write-Host "  개인 설정 삭제: $HomeDir" } else { Write-Host "  개인 설정 보존: $HomeDir" }
Write-Host "✅ 제거 완료"
