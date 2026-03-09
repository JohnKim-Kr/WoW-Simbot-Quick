; WoW Simbot Quick 설치 스크립트
; Inno Setup 6.x용

#define MyAppName "WoW Simbot Quick"
#define MyAppVersion "1.0.0"
#define MyAppPublisher "WoWSimbot Team"
#define MyAppURL "https://github.com/wowsimbot/wow-simbot-quick"
#define MyAppExeName "WoWSimbotQuick.exe"
#define MyAppAssocName MyAppName + " 파일"
#define MyAppAssocExt ".wsq"
#define MyAppAssocKey StringChange(MyAppAssocName, " ", "") + MyAppAssocExt

[Setup]
AppId={{A1B2C3D4-E5F6-7890-ABCD-EF1234567890}
AppName={#MyAppName}
AppVersion={#MyAppVersion}
AppVerName={#MyAppName} {#MyAppVersion}
AppPublisher={#MyAppPublisher}
AppPublisherURL={#MyAppURL}
AppSupportURL={#MyAppURL}
AppUpdatesURL={#MyAppURL}
DefaultDirName={autopf}\{#MyAppName}
ChangesAssociations=yes
DisableProgramGroupPage=yes
LicenseFile=..\LICENSE.txt
InfoBeforeFile=..\README.md
OutputDir=..\installer
OutputBaseFilename=WoWSimbotQuick_Setup_{#MyAppVersion}
Compression=lzma
SolidCompression=yes
WizardStyle=modern
PrivilegesRequiredOverridesAllowed=dialog
UninstallDisplayIcon={app}\{#MyAppExeName}
SetupIconFile=..\resources\app.ico

[Languages]
Name: "english"; MessagesFile: "compiler:Default.isl"
Name: "korean"; MessagesFile: "compiler:Languages\Korean.isl"

[Tasks]
Name: "desktopicon"; Description: "{cm:CreateDesktopIcon}"; GroupDescription: "{cm:AdditionalIcons}"; Flags: unchecked
Name: "quicklaunchicon"; Description: "{cm:CreateQuickLaunchIcon}"; GroupDescription: "{cm:AdditionalIcons}"; Flags: unchecked; OnlyBelowVersion: 6.1; Check: not IsAdminInstallMode

[Files]
; 메인 실행 파일
Source: "..\bin\Release\{#MyAppExeName}"; DestDir: "{app}"; Flags: ignoreversion

; 필수 DLL 및 라이브러리
Source: "..\bin\Release\*.dll"; DestDir: "{app}"; Flags: ignoreversion

; 문서 파일
Source: "..\README.md"; DestDir: "{app}"; Flags: ignoreversion isreadme
Source: "..\LICENSE.txt"; DestDir: "{app}"; Flags: ignoreversion
Source: "..\CHANGELOG.md"; DestDir: "{app}"; Flags: ignoreversion

; 예제 및 템플릿
Source: "..\examples\*.simc"; DestDir: "{app}\Examples"; Flags: ignoreversion

[Registry]
; 파일 연결
Root: HKA; Subkey: "Software\Classes\{#MyAppAssocExt}"; ValueType: string; ValueName: ""; ValueData: "{#MyAppAssocKey}"; Flags: uninsdeletevalue
Root: HKA; Subkey: "Software\Classes\{#MyAppAssocKey}"; ValueType: string; ValueName: ""; ValueData: "{#MyAppAssocName}"; Flags: uninsdeletekey
Root: HKA; Subkey: "Software\Classes\{#MyAppAssocKey}\DefaultIcon"; ValueType: string; ValueName: ""; ValueData: "{app}\{#MyAppExeName},0"
Root: HKA; Subkey: "Software\Classes\{#MyAppAssocKey}\shell\open\command"; ValueType: string; ValueName: ""; ValueData: """{app}\{#MyAppExeName}"" ""%1"""

; 애플리케이션 정보
Root: HKLM; Subkey: "SOFTWARE\{#MyAppPublisher}\{#MyAppName}"; ValueType: string; ValueName: "InstallPath"; ValueData: "{app}"; Flags: uninsdeletekey
Root: HKLM; Subkey: "SOFTWARE\{#MyAppPublisher}\{#MyAppName}"; ValueType: string; ValueName: "Version"; ValueData: "{#MyAppVersion}"

[Icons]
Name: "{autoprograms}\{#MyAppName}"; Filename: "{app}\{#MyAppExeName}"
Name: "{autodesktop}\{#MyAppName}"; Filename: "{app}\{#MyAppExeName}"; Tasks: desktopicon
Name: "{userappdata}\Microsoft\Internet Explorer\Quick Launch\{#MyAppName}"; Filename: "{app}\{#MyAppExeName}"; Tasks: quicklaunchicon

[Run]
Filename: "{app}\{#MyAppExeName}"; Description: "{cm:LaunchProgram,{#StringChange(MyAppName, '&', '&&')}}"; Flags: nowait postinstall skipifsilent

[UninstallDelete]
Type: filesandordirs; Name: "{localappdata}\WoWSimbotQuick\Logs"
Type: filesandordirs; Name: "{localappdata}\WoWSimbotQuick\Temp"

[Code]
var
  SimcPage: TWizardPage;
  SimcPathEdit: TEdit;
  BrowseButton: TButton;

procedure BrowseButtonClick(Sender: TObject);
var
  FileName: string;
begin
  FileName := SimcPathEdit.Text;
  if GetOpenFileName('simc.exe|simc.exe|모든 파일|*.*', FileName, 'exe', 'simc.exe 선택') then
  begin
    SimcPathEdit.Text := FileName;
  end;
end;

procedure InitializeWizard;
begin
  { simc 경로 설정 페이지 생성 }
  SimcPage := CreateCustomPage(wpInfoBefore, 'SimulationCraft 설정', 'simc.exe 경로를 지정하세요.');

  SimcPathEdit := TEdit.Create(SimcPage);
  SimcPathEdit.Parent := SimcPage.Surface;
  SimcPathEdit.Width := SimcPage.SurfaceWidth - 100;
  SimcPathEdit.Top := 16;

  BrowseButton := TButton.Create(SimcPage);
  BrowseButton.Parent := SimcPage.Surface;
  BrowseButton.Caption := '찾아보기...';
  BrowseButton.Left := SimcPathEdit.Width + 10;
  BrowseButton.Top := SimcPathEdit.Top - 2;
  BrowseButton.Width := 90;
  BrowseButton.OnClick := @BrowseButtonClick;

  { 레지스트리에서 기존 설정 읽기 }
  if RegQueryStringValue(HKCU, 'Software\WoWSimbot\WoWSimbotQuick\Settings', 'SimcPath', SimcPathEdit.Text) then
  begin
    if not FileExists(SimcPathEdit.Text) then
      SimcPathEdit.Text := '';
  end;
end;

function NextButtonClick(CurPageID: Integer): Boolean;
begin
  Result := True;

  if CurPageID = SimcPage.ID then
  begin
    { simc 경로 검증 }
    if (SimcPathEdit.Text <> '') and not FileExists(SimcPathEdit.Text) then
    begin
      MsgBox('지정된 simc.exe 파일을 찾을 수 없습니다.', mbError, MB_OK);
      Result := False;
    end;
  end;
end;

procedure CurStepChanged(CurStep: TSetupStep);
begin
  if CurStep = ssPostInstall then
  begin
    { 설치 후 simc 경로 저장 }
    if SimcPathEdit.Text <> '' then
    begin
      RegWriteStringValue(HKCU, 'Software\WoWSimbot\WoWSimbotQuick\Settings', 'SimcPath', SimcPathEdit.Text);
    end;
  end;
end;

function InitializeSetup(): Boolean;
begin
  { Windows 버전 체크 }
  if not IsWindowsVersionOrGreater(6, 1, 0) then
  begin
    MsgBox('이 프로그램은 Windows 7 이상이 필요합니다.', mbError, MB_OK);
    Result := false;
    Exit;
  end;

  { Visual C++ 재배포 가능 패키지 체크 }
  if not IsVCppRedistInstalled then
  begin
    if MsgBox('Visual C++ 재배포 가능 패키지가 설치되어 있지 않을 수 있습니다. 계속하시겠습니까?', mbConfirmation, MB_YESNO) = IDNO then
    begin
      Result := false;
      Exit;
    end;
  end;

  Result := true;
end;

function IsVCppRedistInstalled: Boolean;
begin
  { VC++ 2015-2022 재배포 가능 패키지 (x64) 체크 }
  Result := RegKeyExists(HKLM, 'SOFTWARE\Microsoft\VisualStudio\14.0\VC\Runtimes\x64') or
            RegKeyExists(HKLM64, 'SOFTWARE\Microsoft\VisualStudio\14.0\VC\Runtimes\x64');
end;

function GetUninstallString(): string;
var
  sUnInstPath: string;
  sUnInstallString: string;
begin
  sUnInstPath := ExpandConstant('Software\Microsoft\Windows\CurrentVersion\Uninstall\{#emit SetupSetting("AppId")}_is1');
  sUnInstallString := '';
  if not RegQueryStringValue(HKLM, sUnInstPath, 'UninstallString', sUnInstallString) then
    RegQueryStringValue(HKCU, sUnInstPath, 'UninstallString', sUnInstallString);
  Result := sUnInstallString;
end;

function IsUpgrade(): Boolean;
begin
  Result := (GetUninstallString() <> '');
end;

function InitializeUninstall(): Boolean;
begin
  { 설정 유지 여부 확인 }
  if MsgBox('사용자 설정과 히스토리 데이터를 유지하시겠습니까?' + #13#10 +
            '예: 설정 유지, 아니오: 모든 데이터 삭제', mbConfirmation, MB_YESNO) = IDNO then
  begin
    { 모든 데이터 삭제 }
    RegDeleteKeyIncludingSubkeys(HKCU, 'Software\WoWSimbot\WoWSimbotQuick');
    RegDeleteKeyIncludingSubkeys(HKCU, 'Software\WoWSimbot');
  end;

  Result := true;
end;
