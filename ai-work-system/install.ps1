# work-system 한 번에 설치 (Windows PowerShell)
#   powershell -ExecutionPolicy Bypass -File install.ps1
$ErrorActionPreference = "Stop"
$Src = Split-Path -Parent $MyInvocation.MyCommand.Path
$ClaudeHome = if ($env:CLAUDE_CONFIG_DIR) { $env:CLAUDE_CONFIG_DIR } else { Join-Path $env:USERPROFILE ".claude" }
$HomeDir = Join-Path $ClaudeHome "work-system"
$Pkg = Join-Path $HomeDir "package"
$Ver = if (Test-Path (Join-Path $Src "VERSION")) { (Get-Content (Join-Path $Src "VERSION") -Raw).Trim() } else { "unknown" }
$Mkt = "work-system-local"
$Utf8 = New-Object System.Text.UTF8Encoding($false)

Write-Host "▶ work-system v$Ver 설치"
New-Item -ItemType Directory -Force -Path $HomeDir | Out-Null

# 1) 패키지 복사
if ((Resolve-Path $Src).Path -ne $Pkg) {
  if (Test-Path $Pkg) { Remove-Item -Recurse -Force $Pkg }
  New-Item -ItemType Directory -Force -Path $Pkg | Out-Null
  Get-ChildItem -Force $Src | Where-Object { $_.Name -notin @("dist", ".git") } | ForEach-Object { Copy-Item -Recurse -Force $_.FullName $Pkg }
}
Write-Host "  패키지: $Pkg"

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
  Write-Host "  플러그인 설치 완료: work-system@$Mkt"
  $SetupCmd = "/work-system:work-setup"
} else {
  Write-Host "  claude CLI 플러그인 설치를 쓸 수 없어 직접 복사합니다."
  $Skills = Join-Path $ClaudeHome "skills"; $Agents = Join-Path $ClaudeHome "agents"
  New-Item -ItemType Directory -Force -Path $Skills, $Agents | Out-Null
  Get-ChildItem -Directory (Join-Path $Pkg "skills") | ForEach-Object {
    $dst = Join-Path $Skills $_.Name
    if (Test-Path $dst) { Remove-Item -Recurse -Force $dst }
    Copy-Item -Recurse $_.FullName $dst
  }
  Copy-Item -Force (Join-Path $Pkg "agents\*.md") $Agents
  Write-Host "  스킬·에이전트 복사 완료: $Skills, $Agents"
  $SetupCmd = "/work-setup"
}

# 3) 작업 시스템 홈 파일 (없을 때만)
foreach ($f in @("AGENTS.md", "INSTRUCTIONS.md", "LESSONS.md", "CONFIG.md")) {
  $dst = Join-Path $HomeDir $f
  if (-not (Test-Path $dst)) { Copy-Item (Join-Path $Pkg "skills\work-setup\templates\$f") $dst; Write-Host "  생성: $dst" } else { Write-Host "  유지: $dst" }
}
$Log = Join-Path $HomeDir "CHANGELOG.md"
if (-not (Test-Path $Log)) { [IO.File]::WriteAllText($Log, "# 개인 설정 변경 기록`n`n", $Utf8) }
$m = if ($Mode) { $Mode } else { "copy" }
[IO.File]::AppendAllText($Log, "- $(Get-Date -Format yyyy-MM-dd) work-system v$Ver 설치 ($m)`n", $Utf8)

# 4) 모든 세션에 지침 연결
$Line = "@~/.claude/work-system/AGENTS.md"
$ClaudeMd = Join-Path $ClaudeHome "CLAUDE.md"
if (-not (Test-Path $ClaudeMd)) { [IO.File]::WriteAllText($ClaudeMd, "", $Utf8) }
if ((Get-Content $ClaudeMd -Raw -ErrorAction SilentlyContinue) -match [regex]::Escape($Line)) {
  Write-Host "  CLAUDE.md 연결: 이미 있음"
} else {
  [IO.File]::AppendAllText($ClaudeMd, "`n# work-system (AI 업무 시스템 지침)`n$Line`n", $Utf8)
  Write-Host "  CLAUDE.md 연결: 추가함"
}

Write-Host ""
Write-Host "✅ 설치 완료. 다음 단계:"
Write-Host "  1. 자료가 들어 있는 작업 폴더에서 Claude Code를 새로 시작"
Write-Host "  2. 이렇게 입력:  $SetupCmd"
Write-Host "     → 세션 자료를 읽어 내 지침·팀원 목록을 채우고, 회사 계정에 팀 업무 보드를 만들어 줍니다."
