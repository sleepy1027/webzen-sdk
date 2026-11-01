@echo off
chcp 65001 >nul
echo ==============================================
echo  work-system 설치를 시작합니다...
echo ==============================================
if not exist "%~dp0install.ps1" (
  echo.
  echo [오류] install.ps1 을 찾을 수 없습니다.
  echo zip 파일 안에서 바로 실행하면 이렇게 됩니다.
  echo zip 파일을 마우스 오른쪽 클릭 - "압축 풀기" 한 뒤,
  echo 풀린 work-system 폴더 안의 install.cmd 를 실행하세요.
  echo.
  pause
  exit /b 1
)
powershell -NoProfile -ExecutionPolicy Bypass -File "%~dp0install.ps1"
if errorlevel 1 (
  echo.
  echo [오류] 설치 중 문제가 생겼습니다. 위 메시지를 캡처해서 보내주세요.
)
echo.
pause
