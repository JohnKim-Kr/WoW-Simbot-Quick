#include "pch.h"
#include "framework.h"
#include "SimcDownloader.h"
#include "json.hpp"
#include <winhttp.h>
#include <fstream>
#include <filesystem>
#include <shlobj.h>
#include <algorithm>

#pragma comment(lib, "winhttp.lib")

using json = nlohmann::json;
namespace fs = std::filesystem;

CSimcDownloader::CSimcDownloader()
    : m_bCancelled(FALSE)
{
}

CSimcDownloader::~CSimcDownloader()
{
}

BOOL CSimcDownloader::CheckLatestVersion(CString& outVersion, CString& outDownloadUrl)
{
    return FetchLatestReleaseInfo(outVersion, outDownloadUrl);
}

BOOL CSimcDownloader::DownloadAndInstall(const CString& installDir, CString& outSimcPath, ProgressCallback callback)
{
    m_bCancelled = FALSE;
    CString version = _T("");
    CString downloadUrl = _T("");
    CString baseDir = GetDefaultInstallPath();
    CString targetDir = _T("");
    CString zipPath = _T("");
    CString extractTempDir = _T("");
    std::wstring foundSimcW = L"";
    TCHAR tempPath[MAX_PATH];

    if (callback) callback(5, _T("버전 정보 확인 중..."));
    if (!FetchLatestReleaseInfo(version, downloadUrl))
    {
        if (callback) callback(0, _T("서버 연결 실패 (GitHub API)"));
        return FALSE;
    }

    targetDir = baseDir + _T("\\") + version;
    
    if (PathIsDirectory(targetDir))
    {
        // 폴더가 있으면 내부를 뒤져서 simc.exe가 있는지 확인
        try {
            for (auto const& entry : fs::recursive_directory_iterator(std::wstring(targetDir))) {
                if (entry.path().filename() == L"simc.exe") {
                    if (callback) callback(100, _T("최신 버전이 이미 설치되어 있습니다."));
                    outSimcPath = entry.path().wstring().c_str();
                    return TRUE;
                }
            }
        } catch(...) {}
    }

    if (!CreateDirectoryRecursive(baseDir)) return FALSE;

    GetTempPath(MAX_PATH, tempPath);
    CString ext = (downloadUrl.Find(_T(".7z")) != -1) ? _T(".7z") : _T(".zip");
    zipPath.Format(_T("%s\\simc_%s%s"), tempPath, version, ext);

    if (callback) callback(10, _T("시뮬레이터 다운로드 중..."));
    if (!DownloadFile(downloadUrl, zipPath, [callback](int p, const CString& s) {
        if (callback) callback(10 + (p * 70 / 100), s);
    }))
    {
        return FALSE;
    }

    if (m_bCancelled) return FALSE;

    if (callback) callback(85, _T("압축 해제 중 (tar)..."));
    
    extractTempDir = baseDir + _T("\\temp_extract");
    if (fs::exists(std::wstring(extractTempDir))) {
        try { fs::remove_all(std::wstring(extractTempDir)); } catch(...) {}
    }
    if (!CreateDirectoryRecursive(extractTempDir)) return FALSE;

    if (!ExtractZip(zipPath, extractTempDir))
    {
        if (callback) callback(0, _T("압축 해제 실패 (tar.exe 오류)"));
        return FALSE;
    }

    // 압축 해제된 내용 중 실제 simc.exe가 있는 상위 폴더 찾기
    std::wstring sourcePathW = L"";
    try {
        for (auto const& entry : fs::recursive_directory_iterator(std::wstring(extractTempDir)))
        {
            if (entry.path().filename() == L"simc.exe")
            {
                sourcePathW = entry.path().parent_path().wstring();
                break;
            }
        }
    } catch (...) {}

    if (sourcePathW.empty()) sourcePathW = std::wstring(extractTempDir);

    if (fs::exists(std::wstring(targetDir))) {
        try { fs::remove_all(std::wstring(targetDir)); } catch(...) {}
    }
    
    try {
        fs::rename(sourcePathW, std::wstring(targetDir));
    } catch (...) {
        if (callback) callback(0, _T("설치 폴더 구성 실패 (파일 잠김 등)"));
        return FALSE;
    }

    // 최종 위치에서 simc.exe 확인
    try {
        for (auto const& entry : fs::recursive_directory_iterator(std::wstring(targetDir)))
        {
            if (entry.path().filename() == L"simc.exe")
            {
                foundSimcW = entry.path().wstring();
                break;
            }
        }
    } catch (...) {}

    if (foundSimcW.empty())
    {
        if (callback) callback(0, _T("simc.exe를 찾을 수 없습니다."));
        return FALSE;
    }

    outSimcPath = foundSimcW.c_str();
    DeleteFile(zipPath);
    if (fs::exists(std::wstring(extractTempDir))) {
        try { fs::remove_all(std::wstring(extractTempDir)); } catch(...) {}
    }

    if (callback) callback(100, _T("설치 완료!"));
    return TRUE;
}

void CSimcDownloader::Cancel()
{
    m_bCancelled = TRUE;
}

CString CSimcDownloader::GetDefaultInstallPath()
{
    TCHAR szPath[MAX_PATH];
    if (SUCCEEDED(SHGetFolderPath(NULL, CSIDL_LOCAL_APPDATA, NULL, 0, szPath)))
    {
        PathAppend(szPath, _T("WoWSimbotQuick\\simc"));
        return szPath;
    }
    return _T("");
}

BOOL CSimcDownloader::IsAlreadyInstalled(const CString& installDir)
{
    return PathFileExists(installDir + _T("\\simc.exe"));
}

int CSimcDownloader::CheckVersionStatus(CString& outInstalledVersion, CString& outLatestVersion)
{
    CSimcDownloader loader;
    CString dummyUrl;
    CString defaultBase = GetDefaultInstallPath();

    loader.FetchLatestReleaseInfo(outLatestVersion, dummyUrl);

    if (PathIsDirectory(defaultBase))
    {
        std::vector<std::wstring> versions;
        try {
            for (auto const& entry : fs::directory_iterator(std::wstring(defaultBase)))
            {
                if (entry.is_directory())
                {
                    std::wstring name = entry.path().filename().wstring();
                    if (!name.empty() && name != L"temp_extract")
                    {
                        bool hasSimc = false;
                        for (auto const& sub : fs::recursive_directory_iterator(entry.path())) {
                            if (sub.path().filename() == L"simc.exe") {
                                hasSimc = true;
                                break;
                            }
                        }
                        if (hasSimc) versions.push_back(name);
                    }
                }
            }
        } catch (...) {}

        if (!versions.empty())
        {
            std::sort(versions.rbegin(), versions.rend());
            outInstalledVersion = versions[0].c_str();
            
            if (outLatestVersion.IsEmpty()) return 0;
            return (outInstalledVersion == outLatestVersion) ? 1 : 0;
        }
    }

    return outLatestVersion.IsEmpty() ? -1 : 0;
}

BOOL CSimcDownloader::FetchLatestReleaseInfo(CString& outVersion, CString& outDownloadUrl)
{
    std::string response;
    if (!HttpGet(_T("https://api.github.com/repos/simulationcraft/simc/releases/latest"), response))
        return FALSE;

    try
    {
        auto j = json::parse(response);
        std::string tag = j["tag_name"];
        outVersion = tag.c_str();

        // win64 버전 찾기
        for (auto& asset : j["assets"])
        {
            std::string name = asset["name"];
            std::string url = asset["browser_download_url"];
            
            if (name.find("win64") != std::string::npos && 
                (name.find(".7z") != std::string::npos || name.find(".zip") != std::string::npos))
            {
                outDownloadUrl = url.c_str();
                return TRUE;
            }
        }
    }
    catch (...) {}

    return FALSE;
}

BOOL CSimcDownloader::DownloadFile(const CString& url, const CString& destPath, ProgressCallback callback)
{
    HINTERNET hSession = WinHttpOpen(L"WoWSimbotQuick/1.0", WINHTTP_ACCESS_TYPE_DEFAULT_PROXY, WINHTTP_NO_PROXY_NAME, WINHTTP_NO_PROXY_BYPASS, 0);
    if (!hSession) return FALSE;

    // 리다이렉션 자동 추적 설정 (GitHub 필수)
    DWORD dwRedirectPolicy = WINHTTP_OPTION_REDIRECT_POLICY_ALWAYS;
    WinHttpSetOption(hSession, WINHTTP_OPTION_REDIRECT_POLICY, &dwRedirectPolicy, sizeof(dwRedirectPolicy));

    URL_COMPONENTS urlComp = { sizeof(URL_COMPONENTS) };
    urlComp.dwHostNameLength = (DWORD)-1;
    urlComp.dwUrlPathLength = (DWORD)-1;
    urlComp.dwExtraInfoLength = (DWORD)-1;

    if (!WinHttpCrackUrl(std::wstring(url).c_str(), (DWORD)url.GetLength(), 0, &urlComp))
    {
        WinHttpCloseHandle(hSession);
        return FALSE;
    }

    std::wstring hostName(urlComp.lpszHostName, urlComp.dwHostNameLength);
    HINTERNET hConnect = WinHttpConnect(hSession, hostName.c_str(), urlComp.nPort, 0);
    if (!hConnect) { WinHttpCloseHandle(hSession); return FALSE; }

    DWORD dwFlags = (urlComp.nPort == INTERNET_DEFAULT_HTTPS_PORT) ? WINHTTP_FLAG_SECURE : 0;
    HINTERNET hRequest = WinHttpOpenRequest(hConnect, L"GET", urlComp.lpszUrlPath, NULL, WINHTTP_NO_REFERER, WINHTTP_DEFAULT_ACCEPT_TYPES, dwFlags);
    if (!hRequest) { WinHttpCloseHandle(hConnect); WinHttpCloseHandle(hSession); return FALSE; }

    if (!WinHttpSendRequest(hRequest, WINHTTP_NO_ADDITIONAL_HEADERS, 0, WINHTTP_NO_REQUEST_DATA, 0, 0, 0))
    {
        WinHttpCloseHandle(hRequest); WinHttpCloseHandle(hConnect); WinHttpCloseHandle(hSession); return FALSE;
    }

    if (!WinHttpReceiveResponse(hRequest, NULL))
    {
        WinHttpCloseHandle(hRequest); WinHttpCloseHandle(hConnect); WinHttpCloseHandle(hSession); return FALSE;
    }

    DWORD dwContentLength = 0;
    DWORD dwSize = sizeof(dwContentLength);
    WinHttpQueryHeaders(hRequest, WINHTTP_QUERY_CONTENT_LENGTH | WINHTTP_QUERY_FLAG_NUMBER, WINHTTP_HEADER_NAME_BY_INDEX, &dwContentLength, &dwSize, WINHTTP_NO_HEADER_INDEX);

    std::ofstream outFile(std::wstring(destPath), std::ios::binary);
    if (!outFile) { WinHttpCloseHandle(hRequest); WinHttpCloseHandle(hConnect); WinHttpCloseHandle(hSession); return FALSE; }

    BYTE buffer[16384];
    DWORD dwRead = 0;
    DWORD dwTotalRead = 0;

    while (WinHttpReadData(hRequest, buffer, sizeof(buffer), &dwRead) && dwRead > 0)
    {
        if (m_bCancelled) { outFile.close(); DeleteFile(destPath); break; }
        outFile.write((char*)buffer, dwRead);
        dwTotalRead += dwRead;
        if (callback && dwContentLength > 0)
        {
            callback((int)(dwTotalRead * 100 / dwContentLength), _T("다운로드 중..."));
        }
    }

    outFile.close();
    WinHttpCloseHandle(hRequest);
    WinHttpCloseHandle(hConnect);
    WinHttpCloseHandle(hSession);

    return !m_bCancelled;
}

BOOL CSimcDownloader::ExtractZip(const CString& zipPath, const CString& destDir)
{
    // 윈도우 10/11에 내장된 tar.exe 사용 (libarchive 기반으로 .7z 지원)
    CString cmd;
    cmd.Format(_T("tar.exe -xf \"%s\" -C \"%s\""), (LPCTSTR)zipPath, (LPCTSTR)destDir);

    STARTUPINFO si = { sizeof(si) };
    PROCESS_INFORMATION pi;
    DWORD dwExitCode = 1;
    if (CreateProcess(NULL, (LPTSTR)(LPCTSTR)cmd, NULL, NULL, FALSE, CREATE_NO_WINDOW, NULL, NULL, &si, &pi))
    {
        WaitForSingleObject(pi.hProcess, 600000); // 최대 10분 대기
        GetExitCodeProcess(pi.hProcess, &dwExitCode);
        CloseHandle(pi.hProcess);
        CloseHandle(pi.hThread);
        return dwExitCode == 0;
    }

    return FALSE;
}

BOOL CSimcDownloader::ExtractWithPowerShell(const CString& zipPath, const CString& destDir)
{
    return ExtractZip(zipPath, destDir);
}

CString CSimcDownloader::Find7zExecutable() { return _T(""); }
CString CSimcDownloader::Download7zCli() { return _T(""); }
void CSimcDownloader::CleanupTemp7z(const CString& szPath) {}

BOOL CSimcDownloader::CreateDirectoryRecursive(const CString& path)
{
    std::wstring wpath = std::wstring(path);
    if (fs::exists(wpath)) return TRUE;
    try {
        return fs::create_directories(wpath);
    } catch (...) { return FALSE; }
}

BOOL CSimcDownloader::HttpGet(const CString& url, std::string& outResponse)
{
    HINTERNET hSession = WinHttpOpen(L"WoWSimbotQuick/1.0", WINHTTP_ACCESS_TYPE_DEFAULT_PROXY, WINHTTP_NO_PROXY_NAME, WINHTTP_NO_PROXY_BYPASS, 0);
    if (!hSession) return FALSE;

    // SSL/TLS & 리다이렉션 설정
    DWORD dwProtocols = WINHTTP_FLAG_SECURE_PROTOCOL_TLS1_2 | WINHTTP_FLAG_SECURE_PROTOCOL_TLS1_3;
    WinHttpSetOption(hSession, WINHTTP_OPTION_SECURE_PROTOCOLS, &dwProtocols, sizeof(dwProtocols));
    
    DWORD dwRedirectPolicy = WINHTTP_OPTION_REDIRECT_POLICY_ALWAYS;
    WinHttpSetOption(hSession, WINHTTP_OPTION_REDIRECT_POLICY, &dwRedirectPolicy, sizeof(dwRedirectPolicy));

    URL_COMPONENTS urlComp = { sizeof(URL_COMPONENTS) };
    urlComp.dwHostNameLength = (DWORD)-1;
    urlComp.dwUrlPathLength = (DWORD)-1;
    urlComp.dwExtraInfoLength = (DWORD)-1;

    if (!WinHttpCrackUrl(std::wstring(url).c_str(), (DWORD)url.GetLength(), 0, &urlComp))
    {
        WinHttpCloseHandle(hSession);
        return FALSE;
    }

    std::wstring hostName(urlComp.lpszHostName, urlComp.dwHostNameLength);
    HINTERNET hConnect = WinHttpConnect(hSession, hostName.c_str(), urlComp.nPort, 0);
    if (!hConnect) { WinHttpCloseHandle(hSession); return FALSE; }

    DWORD dwFlags = (urlComp.nPort == INTERNET_DEFAULT_HTTPS_PORT) ? WINHTTP_FLAG_SECURE : 0;
    HINTERNET hRequest = WinHttpOpenRequest(hConnect, L"GET", urlComp.lpszUrlPath, NULL, WINHTTP_NO_REFERER, WINHTTP_DEFAULT_ACCEPT_TYPES, dwFlags);
    if (!hRequest) { WinHttpCloseHandle(hConnect); WinHttpCloseHandle(hSession); return FALSE; }

    if (!WinHttpSendRequest(hRequest, WINHTTP_NO_ADDITIONAL_HEADERS, 0, WINHTTP_NO_REQUEST_DATA, 0, 0, 0))
    {
        WinHttpCloseHandle(hRequest); WinHttpCloseHandle(hConnect); WinHttpCloseHandle(hSession); return FALSE;
    }

    if (!WinHttpReceiveResponse(hRequest, NULL))
    {
        WinHttpCloseHandle(hRequest); WinHttpCloseHandle(hConnect); WinHttpCloseHandle(hSession); return FALSE;
    }

    DWORD dwSize = 0;
    while (WinHttpQueryDataAvailable(hRequest, &dwSize) && dwSize > 0)
    {
        std::vector<char> buffer(dwSize);
        DWORD dwRead = 0;
        if (WinHttpReadData(hRequest, buffer.data(), dwSize, &dwRead))
        {
            outResponse.append(buffer.data(), dwRead);
        }
    }

    WinHttpCloseHandle(hRequest);
    WinHttpCloseHandle(hConnect);
    WinHttpCloseHandle(hSession);

    return !outResponse.empty();
}
