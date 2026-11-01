# work-system 한 번에 설치 (Windows PowerShell 5.1+)
#   install.cmd 더블클릭, 또는: powershell -ExecutionPolicy Bypass -File install.ps1
# 기존 스킬·에이전트·지침은 덮어쓰지 않는다 (겹치면 ws-<이름>으로 설치, CLAUDE.md는 백업 후 한 줄 추가).
$ErrorActionPreference = "Stop"
$Src = Split-Path -Parent $MyInvocation.MyCommand.Path
$ClaudeHome = if ($env:CLAUDE_CONFIG_DIR) { $env:CLAUDE_CONFIG_DIR } else { Join-Path $env:USERPROFILE ".claude" }
$HomeDir = Join-Path $ClaudeHome "work-system"
$Pkg = Join-Path $HomeDir "package"
$Manifest = Join-Path $HomeDir "installed.txt"
$Ver = if (Test-Path (Join-Path $Src "VERSION")) { (Get-Content (Join-Path $Src "VERSION") -Raw).Trim() } else { "unknown" }
$Mkt = "work-system-local"
$Stamp = Get-Date -Format "yyyyMMdd-HHmmss"
$Utf8 = New-Object System.Text.UTF8Encoding($false)

function Read-Manifest { if (Test-Path $Manifest) { @(Get-Content $Manifest | Where-Object { $_ }) } else { @() } }
function Add-Manifest($rel) { if ((Read-Manifest) -notcontains $rel) { [IO.File]::AppendAllText($Manifest, "$rel`n", $Utf8) } }
function Rename-Frontmatter($file, $newName) {
  $lines = [IO.File]::ReadAllLines($file, [Text.Encoding]::UTF8); $done = $false
  for ($i = 0; $i -lt $lines.Length; $i++) { if (-not $done -and $lines[$i] -match '^name: ') { $lines[$i] = "name: $newName"; $done = $true } }
  [IO.File]::WriteAllLines($file, $lines, $Utf8)
}

Write-Host "▶ work-system v$Ver 설치"
New-Item -ItemType Directory -Force -Path $HomeDir | Out-Null
if (-not (Test-Path $Manifest)) { [IO.File]::WriteAllText($Manifest, "", $Utf8) }

# 0) 기존 환경 요약 (읽기만)
$nSk = @(Get-ChildItem -Path (Join-Path $ClaudeHome "skills") -Directory -ErrorAction SilentlyContinue | Where-Object { Test-Path (Join-Path $_.FullName "SKILL.md") }).Count
$nAg = @(Get-ChildItem -Path (Join-Path $ClaudeHome "agents") -Filter *.md -ErrorAction SilentlyContinue).Count
Write-Host "  기존 환경: 사용자 스킬 ${nSk}개, 에이전트 ${nAg}개 — 그대로 유지합니다."

# 1) 패키지 복사
if ((Resolve-Path $Src).Path -ne $Pkg) {
  if (Test-Path $Pkg) { Remove-Item -Recurse -Force $Pkg }
  New-Item -ItemType Directory -Force -Path $Pkg | Out-Null
  Get-ChildItem -Force $Src | Where-Object { $_.Name -notin @("dist", ".git") } | ForEach-Object { Copy-Item -Recurse -Force $_.FullName $Pkg }
}

# 2) 플러그인 설치
$Mode = ""
if (Get-Command claude -ErrorAction SilentlyContinue) {
  $ErrorActionPreference = "Continue"
  $list = (& claude plugin marketplace list 2>$null) -join "`n"
  if ($list -match $Mkt) {
    & claude plugin marketplace update $Mkt *> $null
    & claude plugin update "work-system@$Mkt" *> $null
    if ($LASTEXITCODE -ne 0) { & claude plugin install "work-system@$Mkt" *> $null }
    if ($LASTEXITCODE -eq 0) { $Mode = "plugin" }
  } else {
    & claude plugin marketplace add $Pkg *> $null
    if ($LASTEXITCODE -eq 0) { & claude plugin install "work-system@$Mkt" *> $null; if ($LASTEXITCODE -eq 0) { $Mode = "plugin" } }
  }
  $ErrorActionPreference = "Stop"
}

if ($Mode -eq "plugin") {
  Write-Host "  플러그인 설치: work-system@$Mkt (스킬 이름은 work-system:<이름>, 기존 스킬과 충돌 없음)"
  $SetupCmd = "/work-system:work-setup"
  $keep = @()
  foreach ($rel in (Read-Manifest)) {
    if ($rel -match '^(skills|agents)/') {
      $p = Join-Path $ClaudeHome ($rel -replace '/', '\')
      if (Test-Path $p) { Remove-Item -Recurse -Force $p; Write-Host "  이전 복사본 정리: $rel" }
    } else { $keep += $rel }
  }
  [IO.File]::WriteAllText($Manifest, (($keep | ForEach-Object { "$_`n" }) -join ""), $Utf8)
} else {
  Write-Host "  플러그인 설치를 쓸 수 없어 직접 복사합니다 (기존 파일은 덮어쓰지 않음)."
  $SetupCmd = "/work-setup"
  New-Item -ItemType Directory -Force -Path (Join-Path $ClaudeHome "skills"), (Join-Path $ClaudeHome "agents") | Out-Null
  $owned = Read-Manifest
  Get-ChildItem -Directory (Join-Path $Pkg "skills") | ForEach-Object {
    $n = $_.Name; $rel = "skills/$n"
    if ((Test-Path (Join-Path $ClaudeHome "skills\$n")) -and ($owned -notcontains $rel)) {
      $rel = "skills/ws-$n"; Write-Host "  이름 겹침: 기존 스킬 '$n' 유지 → work-system 것은 'ws-$n'으로 설치"
      if ($n -eq "work-setup") { $SetupCmd = "/ws-work-setup" }
    }
    $dst = Join-Path $ClaudeHome ($rel -replace '/', '\')
    if (Test-Path $dst) { Remove-Item -Recurse -Force $dst }
    Copy-Item -Recurse $_.FullName $dst
    if ($rel -ne "skills/$n") { Rename-Frontmatter (Join-Path $dst "SKILL.md") "ws-$n" }
    Add-Manifest $rel
  }
  Get-ChildItem (Join-Path $Pkg "agents") -Filter *.md | ForEach-Object {
    $n = $_.BaseName; $rel = "agents/$n.md"
    if ((Test-Path (Join-Path $ClaudeHome "agents\$n.md")) -and ($owned -notcontains $rel)) {
      $rel = "agents/ws-$n.md"; Write-Host "  이름 겹침: 기존 에이전트 '$n' 유지 → work-system 것은 'ws-$n'으로 설치"
    }
    $dst = Join-Path $ClaudeHome ($rel -replace '/', '\')
    Copy-Item -Force $_.FullName $dst
    if ($rel -ne "agents/$n.md") { Rename-Frontmatter $dst "ws-$n" }
    Add-Manifest $rel
  }
  Write-Host "  스킬·에이전트 설치 완료 (목록: $Manifest)"
}

# 3) 작업 시스템 홈 파일 (없을 때만)
foreach ($f in @("AGENTS.md", "INSTRUCTIONS.md", "LESSONS.md", "CONFIG.md", "INTEGRATION.md")) {
  $dst = Join-Path $HomeDir $f
  if (-not (Test-Path $dst)) { Copy-Item (Join-Path $Pkg "skills\work-setup\templates\$f") $dst; Write-Host "  생성: work-system\$f" }
}
$Log = Join-Path $HomeDir "CHANGELOG.md"
if (-not (Test-Path $Log)) { [IO.File]::WriteAllText($Log, "# 개인 설정 변경 기록`n`n", $Utf8) }
$m = if ($Mode) { $Mode } else { "copy" }
[IO.File]::AppendAllText($Log, "- $(Get-Date -Format yyyy-MM-dd) work-system v$Ver 설치 ($m)`n", $Utf8)

# 4) 모든 세션에 지침 연결 (백업 후 한 줄 추가)
$Line = "@~/.claude/work-system/AGENTS.md"
$ClaudeMd = Join-Path $ClaudeHome "CLAUDE.md"
$cur = if (Test-Path $ClaudeMd) { [IO.File]::ReadAllText($ClaudeMd, [Text.Encoding]::UTF8) } else { "" }
if ($cur.Contains($Line)) {
  Write-Host "  CLAUDE.md 연결: 이미 있음"
} else {
  if ($cur.Trim().Length -gt 0) {
    $bk = Join-Path $HomeDir "backup\$Stamp"; New-Item -ItemType Directory -Force -Path $bk | Out-Null
    Copy-Item $ClaudeMd (Join-Path $bk "CLAUDE.md"); Write-Host "  CLAUDE.md 백업: work-system\backup\$Stamp\CLAUDE.md"
  }
  [IO.File]::AppendAllText($ClaudeMd, "`n# work-system (AI 업무 시스템 지침 — 기존 지침이 우선)`n$Line`n", $Utf8)
  Write-Host "  CLAUDE.md 연결: 맨 끝에 한 줄 추가 (기존 내용 유지)"
}

Write-Host ""
Write-Host "✅ 설치 완료. 다음 단계:"
Write-Host "  1. 자료가 들어 있는 작업 폴더에서 Claude Code를 새로 시작"
Write-Host "  2. 이렇게 입력:  $SetupCmd"
Write-Host "     → 기존 스킬·지침을 조사해 겹치는 부분의 사용 규칙을 정하고(기존 것 우선),"
Write-Host "       세션 자료로 내 지침·팀원을 채운 뒤 회사 계정에 팀 업무 보드를 만듭니다."
Write-Host "  제거:  $Pkg\uninstall.cmd"
