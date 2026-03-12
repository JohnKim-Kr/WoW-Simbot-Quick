#pragma once

// SimulationCraft 자동 다운로더
class CSimcDownloader
{
public:
    CSimcDownloader();
    ~CSimcDownloader();

    // 다운로드 진행 상황 콜백
    typedef std::function<void(int percent, const CString& status)> ProgressCallback;

    // 최신 simc 버전 확인
    BOOL CheckLatestVersion(CString& outVersion, CString& outDownloadUrl);

    // simc 다운로드 및 설치
    BOOL DownloadAndInstall(const CString& installDir, CString& outSimcPath, ProgressCallback callback = nullptr);

    // 다운로드 중지
    void Cancel();

    CString GetLastErrorMessage() const { return m_lastErrorMessage; }

    // 설치된 simc 경로 가져오기
    static CString GetDefaultInstallPath();


    // 이미 설치되어 있는지 확인
    static BOOL IsAlreadyInstalled(const CString& installDir);

    // 설치된 버전과 최신 버전 비교
    // return: -1 = 설치 안됨, 0 = 최신, 1 = 구버전
    static int CheckVersionStatus(CString& outInstalledVersion, CString& outLatestVersion);

private:
    BOOL m_bCancelled;
    CString m_tempZipPath;
    CString m_lastErrorMessage;

    void SetLastErrorMessage(const CString& message);
    void SetLastErrorMessage(const CString& message, DWORD errorCode);

    // GitHub API에서 최신 릴리즈 정보 가져오기
    BOOL FetchLatestReleaseInfo(CString& outVersion, CString& outDownloadUrl);

    // 파일 다운로드
    BOOL DownloadFile(const CString& url, const CString& destPath, ProgressCallback callback);

    // ZIP/7z 파일 압축 해제
    BOOL ExtractZip(const CString& zipPath, const CString& destDir);
    BOOL ExtractWithPowerShell(const CString& zipPath, const CString& destDir);
    CString Find7zExecutable();
    CString Download7zCli();
    void CleanupTemp7z(const CString& szPath);
    BOOL CreateDirectoryRecursive(const CString& path);

    // HTTP 요청 수행
    BOOL HttpGet(const CString& url, std::string& outResponse);
};
