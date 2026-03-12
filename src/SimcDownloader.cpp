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
    CString version, downloadUrl;
    std::wstring finalExePath = L"";

    if (callback) callback(5, _T("최신 버전 정보 확인 중..."));
    if (!FetchLatestReleaseInfo(version, downloadUrl))
    {
        return FALSE;
    }

    CString targetDir = GetDefaultInstallPath() + _T("\\") + version;
    if (PathIsDirectory(targetDir))
    {
        CString existingExe = targetDir + _T("\\simc.exe");
        if (PathFileExists(existingExe))
        {
            if (callback) callback(100, _T("이미 최신 버전이 설치되어 있습니다."));
            outSimcPath = existingExe;
            return TRUE;
        }
    }

    if (!CreateDirectoryRecursive(targetDir)) return FALSE;

    TCHAR tempPath[MAX_PATH];
    GetTempPath(MAX_PATH, tempPath);
    CString zipPath;
    zipPath.Format(_T("%s\\simc_%s.7z"), tempPath, version);

    if (callback) callback(10, _T("시뮬레이터 다운로드 중..."));
    if (!DownloadFile(downloadUrl, zipPath, [callback](int p, const CString& s) {
        if (callback) callback(10 + (p * 70 / 100), s);
    }))
    {
        return FALSE;
    }

    if (m_bCancelled) return FALSE;

    if (callback) callback(85, _T("압축 해제 중..."));
    if (!ExtractZip(zipPath, targetDir))
    {
        if (callback) callback(0, _T("압축 해제 실패 (7z/PowerShell 필요)"));
        return FALSE;
    }

    for (auto const& it : fs::recursive_directory_iterator(std::wstring(targetDir)))
    {
        if (it.path().filename() == L"simc.exe")
        {
            finalExePath = it.path().wstring();
            break;
        }
    }

    if (finalExePath.empty())
    {
        if (callback) callback(0, _T("simc.exe를 찾을 수 없습니다."));
        return FALSE;
    }

    outSimcPath = finalExePath.c_str();
    DeleteFile(zipPath);

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
    if (SUCCEEDED(SHGetFolderPath(NULL, CSIDL_MYDOCUMENTS, NULL, 0, szPath)))
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
    if (!loader.FetchLatestReleaseInfo(outLatestVersion, dummyUrl)) return -1;

    CString defaultBase = GetDefaultInstallPath();
    if (PathIsDirectory(defaultBase))
    {
        std::vector<std::wstring> versions;
        for (auto const& it : fs::directory_iterator(std::wstring(defaultBase)))
        {
            if (it.is_directory() && fs::exists(it.path() / L"simc.exe"))
                versions.push_back(it.path().filename().wstring());
        }

        if (!versions.empty())
        {
            std::sort(versions.rbegin(), versions.rend());
            outInstalledVersion = versions[0].c_str();
            return (outInstalledVersion == outLatestVersion) ? 1 : 0;
        }
    }

    return 0;
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

    BYTE buffer[8192];
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
    CString cmd;
    if (zipPath.Right(3).CompareNoCase(_T(".7z")) == 0)
    {
        cmd.Format(_T("powershell.exe -NoProfile -Command \"Expand-Archive -Path '%s' -DestinationPath '%s' -Force\""),
            (LPCTSTR)zipPath, (LPCTSTR)destDir);
    }
    else
    {
        cmd.Format(_T("powershell.exe -NoProfile -Command \"& { Add-Type -AssemblyName System.IO.Compression.FileSystem; [System.IO.Compression.ZipFile]::ExtractToDirectory('%s', '%s') }\""),
            (LPCTSTR)zipPath, (LPCTSTR)destDir);
    }

    STARTUPINFO si = { sizeof(si) };
    PROCESS_INFORMATION pi;
    if (CreateProcess(NULL, (LPTSTR)(LPCTSTR)cmd, NULL, NULL, FALSE, CREATE_NO_WINDOW, NULL, NULL, &si, &pi))
    {
        WaitForSingleObject(pi.hProcess, INFINITE);
        DWORD exitCode;
        GetExitCodeProcess(pi.hProcess, &exitCode);
        CloseHandle(pi.hProcess);
        CloseHandle(pi.hThread);
        return exitCode == 0;
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
    return fs::create_directories(wpath);
}

BOOL CSimcDownloader::HttpGet(const CString& url, std::string& outResponse)
{
    HINTERNET hSession = WinHttpOpen(L"WoWSimbotQuick/1.0", WINHTTP_ACCESS_TYPE_DEFAULT_PROXY, WINHTTP_NO_PROXY_NAME, WINHTTP_NO_PROXY_BYPASS, 0);
    if (!hSession) return FALSE;

    DWORD dwProtocols = WINHTTP_FLAG_SECURE_PROTOCOL_TLS1_2 | WINHTTP_FLAG_SECURE_PROTOCOL_TLS1_3;
    WinHttpSetOption(hSession, WINHTTP_OPTION_SECURE_PROTOCOLS, &dwProtocols, sizeof(dwProtocols));

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
