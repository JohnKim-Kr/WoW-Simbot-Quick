#include "pch.h"
#include "framework.h"
#include "SimcDownloader.h"
#include "json.hpp"
#include <fstream>
#include <filesystem>
#include <shlobj.h>
#include <winhttp.h>
#include <algorithm>

using json = nlohmann::json;
namespace fs = std::filesystem;

namespace
{
    CString EscapeCommandLineArgument(const CString& value)
    {
        CString escaped(value);
        escaped.Replace(_T("\""), _T("\\\""));
        return escaped;
    }

    CString EscapePowerShellSingleQuoted(const CString& value)
    {
        CString escaped(value);
        escaped.Replace(_T("'"), _T("''"));
        return escaped;
    }

    bool RunProcess(const CString& applicationName, const CString& arguments, DWORD* outExitCode = nullptr, DWORD* outLaunchError = nullptr)
    {
        CString commandLine;
        commandLine.Format(_T("\"%s\" %s"), static_cast<LPCTSTR>(applicationName), static_cast<LPCTSTR>(arguments));

        STARTUPINFO si = { sizeof(si) };
        PROCESS_INFORMATION pi = {};
        if (outExitCode)
        {
            *outExitCode = static_cast<DWORD>(-1);
        }
        if (outLaunchError)
        {
            *outLaunchError = ERROR_SUCCESS;
        }

        LPTSTR rawCommandLine = commandLine.GetBuffer();
        const BOOL created = CreateProcess(NULL, rawCommandLine, NULL, NULL, FALSE, CREATE_NO_WINDOW, NULL, NULL, &si, &pi);
        const DWORD createError = created ? ERROR_SUCCESS : GetLastError();
        commandLine.ReleaseBuffer();

        if (!created)
        {
            if (outLaunchError)
            {
                *outLaunchError = createError;
            }
            return false;
        }

        WaitForSingleObject(pi.hProcess, 600000);

        DWORD exitCode = 1;
        GetExitCodeProcess(pi.hProcess, &exitCode);
        CloseHandle(pi.hProcess);
        CloseHandle(pi.hThread);

        if (outExitCode)
        {
            *outExitCode = exitCode;
        }
        if (outLaunchError)
        {
            *outLaunchError = ERROR_SUCCESS;
        }

        return exitCode == 0;
    }

    bool DownloadWithPowerShell(const CString& url, const CString& destPath, DWORD* outExitCode = nullptr, DWORD* outLaunchError = nullptr)
    {
        CString arguments;
        arguments.Format(
            _T("-NoProfile -NonInteractive -ExecutionPolicy Bypass -Command \"$ProgressPreference='SilentlyContinue'; try { [Net.ServicePointManager]::SecurityProtocol = [Net.SecurityProtocolType]::Tls12; Invoke-WebRequest -UseBasicParsing -Headers @{ 'User-Agent'='WoWSimbotQuick/1.0' } -Uri '%s' -OutFile '%s'; exit 0 } catch { exit 1 }\""),
            static_cast<LPCTSTR>(EscapePowerShellSingleQuoted(url)),
            static_cast<LPCTSTR>(EscapePowerShellSingleQuoted(destPath)));
        return RunProcess(_T("powershell.exe"), arguments, outExitCode, outLaunchError);
    }

    bool DownloadWithCurl(const CString& url, const CString& destPath, DWORD* outExitCode = nullptr, DWORD* outLaunchError = nullptr)
    {
        CString arguments;
        arguments.Format(
            _T("-L --fail --silent --show-error --globoff -A \"WoWSimbotQuick/1.0\" --output \"%s\" --url \"%s\""),
            static_cast<LPCTSTR>(EscapeCommandLineArgument(destPath)),
            static_cast<LPCTSTR>(EscapeCommandLineArgument(url)));

        DWORD launchError = ERROR_SUCCESS;
        if (RunProcess(_T("curl.exe"), arguments, outExitCode, &launchError))
        {
            if (outLaunchError)
            {
                *outLaunchError = ERROR_SUCCESS;
            }
            return true;
        }

        // Retry once with PowerShell if curl is missing or returns an HTTP/process failure.
        if (launchError == ERROR_FILE_NOT_FOUND ||
            launchError == ERROR_PATH_NOT_FOUND ||
            launchError == ERROR_SUCCESS)
        {
            return DownloadWithPowerShell(url, destPath, outExitCode, outLaunchError);
        }

        if (outLaunchError)
        {
            *outLaunchError = launchError;
        }
        return false;
    }

    bool HttpGetWithWinHttp(const CString& url, std::string& outResponse, DWORD* outStatusCode = nullptr, DWORD* outWinHttpError = nullptr)
    {
        if (outStatusCode)
        {
            *outStatusCode = 0;
        }
        if (outWinHttpError)
        {
            *outWinHttpError = ERROR_SUCCESS;
        }

        HINTERNET hSession = WinHttpOpen(L"WoWSimbotQuick/1.0", WINHTTP_ACCESS_TYPE_DEFAULT_PROXY, WINHTTP_NO_PROXY_NAME, WINHTTP_NO_PROXY_BYPASS, 0);
        if (!hSession)
        {
            if (outWinHttpError)
            {
                *outWinHttpError = GetLastError();
            }
            return false;
        }

        DWORD redirectPolicy = WINHTTP_OPTION_REDIRECT_POLICY_ALWAYS;
        WinHttpSetOption(hSession, WINHTTP_OPTION_REDIRECT_POLICY, &redirectPolicy, sizeof(redirectPolicy));
        DWORD secureProtocols = WINHTTP_FLAG_SECURE_PROTOCOL_TLS1_2;
        WinHttpSetOption(hSession, WINHTTP_OPTION_SECURE_PROTOCOLS, &secureProtocols, sizeof(secureProtocols));

        URL_COMPONENTS components = {};
        components.dwStructSize = sizeof(components);
        components.dwHostNameLength = static_cast<DWORD>(-1);
        components.dwUrlPathLength = static_cast<DWORD>(-1);
        components.dwExtraInfoLength = static_cast<DWORD>(-1);

        const std::wstring urlW(url);
        if (!WinHttpCrackUrl(urlW.c_str(), static_cast<DWORD>(urlW.size()), 0, &components))
        {
            if (outWinHttpError)
            {
                *outWinHttpError = GetLastError();
            }
            WinHttpCloseHandle(hSession);
            return false;
        }

        const std::wstring host(components.lpszHostName, components.dwHostNameLength);
        std::wstring path = (components.dwUrlPathLength > 0 && components.lpszUrlPath != nullptr)
            ? std::wstring(components.lpszUrlPath, components.dwUrlPathLength)
            : std::wstring(L"/");

        if (components.dwExtraInfoLength > 0 && components.lpszExtraInfo != nullptr)
        {
            path.append(components.lpszExtraInfo, components.dwExtraInfoLength);
        }

        HINTERNET hConnect = WinHttpConnect(hSession, host.c_str(), components.nPort, 0);
        if (!hConnect)
        {
            if (outWinHttpError)
            {
                *outWinHttpError = GetLastError();
            }
            WinHttpCloseHandle(hSession);
            return false;
        }

        const DWORD requestFlags = (components.nScheme == INTERNET_SCHEME_HTTPS) ? WINHTTP_FLAG_SECURE : 0;
        HINTERNET hRequest = WinHttpOpenRequest(hConnect, L"GET", path.c_str(), nullptr, WINHTTP_NO_REFERER, WINHTTP_DEFAULT_ACCEPT_TYPES, requestFlags);
        if (!hRequest)
        {
            if (outWinHttpError)
            {
                *outWinHttpError = GetLastError();
            }
            WinHttpCloseHandle(hConnect);
            WinHttpCloseHandle(hSession);
            return false;
        }

        bool ok = false;
        DWORD statusCode = 0;

        if (WinHttpSendRequest(hRequest, WINHTTP_NO_ADDITIONAL_HEADERS, 0, WINHTTP_NO_REQUEST_DATA, 0, 0, 0) &&
            WinHttpReceiveResponse(hRequest, nullptr))
        {
            DWORD statusSize = sizeof(statusCode);
            WinHttpQueryHeaders(hRequest, WINHTTP_QUERY_STATUS_CODE | WINHTTP_QUERY_FLAG_NUMBER, WINHTTP_HEADER_NAME_BY_INDEX, &statusCode, &statusSize, WINHTTP_NO_HEADER_INDEX);

            if (statusCode >= 200 && statusCode < 300)
            {
                outResponse.clear();
                while (true)
                {
                    DWORD available = 0;
                    if (!WinHttpQueryDataAvailable(hRequest, &available))
                    {
                        break;
                    }
                    if (available == 0)
                    {
                        ok = !outResponse.empty();
                        break;
                    }

                    std::vector<char> buffer(available);
                    DWORD bytesRead = 0;
                    if (!WinHttpReadData(hRequest, buffer.data(), available, &bytesRead))
                    {
                        break;
                    }
                    outResponse.append(buffer.data(), bytesRead);
                }
            }
        }

        if (!ok && outWinHttpError && *outWinHttpError == ERROR_SUCCESS)
        {
            *outWinHttpError = GetLastError();
        }
        if (outStatusCode)
        {
            *outStatusCode = statusCode;
        }

        WinHttpCloseHandle(hRequest);
        WinHttpCloseHandle(hConnect);
        WinHttpCloseHandle(hSession);
        return ok;
    }

    bool DownloadWithWinHttpToFile(const CString& url, const CString& destPath, DWORD* outStatusCode = nullptr, DWORD* outWinHttpError = nullptr)
    {
        if (outStatusCode)
        {
            *outStatusCode = 0;
        }
        if (outWinHttpError)
        {
            *outWinHttpError = ERROR_SUCCESS;
        }

        HINTERNET hSession = WinHttpOpen(L"WoWSimbotQuick/1.0", WINHTTP_ACCESS_TYPE_DEFAULT_PROXY, WINHTTP_NO_PROXY_NAME, WINHTTP_NO_PROXY_BYPASS, 0);
        if (!hSession)
        {
            if (outWinHttpError) *outWinHttpError = GetLastError();
            return false;
        }

        DWORD redirectPolicy = WINHTTP_OPTION_REDIRECT_POLICY_ALWAYS;
        WinHttpSetOption(hSession, WINHTTP_OPTION_REDIRECT_POLICY, &redirectPolicy, sizeof(redirectPolicy));
        DWORD secureProtocols = WINHTTP_FLAG_SECURE_PROTOCOL_TLS1_2;
        WinHttpSetOption(hSession, WINHTTP_OPTION_SECURE_PROTOCOLS, &secureProtocols, sizeof(secureProtocols));

        URL_COMPONENTS components = {};
        components.dwStructSize = sizeof(components);
        components.dwHostNameLength = static_cast<DWORD>(-1);
        components.dwUrlPathLength = static_cast<DWORD>(-1);
        components.dwExtraInfoLength = static_cast<DWORD>(-1);

        const std::wstring urlW(url);
        if (!WinHttpCrackUrl(urlW.c_str(), static_cast<DWORD>(urlW.size()), 0, &components))
        {
            if (outWinHttpError) *outWinHttpError = GetLastError();
            WinHttpCloseHandle(hSession);
            return false;
        }

        const std::wstring host(components.lpszHostName, components.dwHostNameLength);
        std::wstring path = (components.dwUrlPathLength > 0 && components.lpszUrlPath != nullptr)
            ? std::wstring(components.lpszUrlPath, components.dwUrlPathLength)
            : std::wstring(L"/");
        if (components.dwExtraInfoLength > 0 && components.lpszExtraInfo != nullptr)
        {
            path.append(components.lpszExtraInfo, components.dwExtraInfoLength);
        }

        HINTERNET hConnect = WinHttpConnect(hSession, host.c_str(), components.nPort, 0);
        if (!hConnect)
        {
            if (outWinHttpError) *outWinHttpError = GetLastError();
            WinHttpCloseHandle(hSession);
            return false;
        }

        const DWORD requestFlags = (components.nScheme == INTERNET_SCHEME_HTTPS) ? WINHTTP_FLAG_SECURE : 0;
        HINTERNET hRequest = WinHttpOpenRequest(hConnect, L"GET", path.c_str(), nullptr, WINHTTP_NO_REFERER, WINHTTP_DEFAULT_ACCEPT_TYPES, requestFlags);
        if (!hRequest)
        {
            if (outWinHttpError) *outWinHttpError = GetLastError();
            WinHttpCloseHandle(hConnect);
            WinHttpCloseHandle(hSession);
            return false;
        }

        bool ok = false;
        DWORD statusCode = 0;

        std::ofstream output(std::wstring(destPath), std::ios::binary | std::ios::trunc);
        if (output &&
            WinHttpSendRequest(hRequest, WINHTTP_NO_ADDITIONAL_HEADERS, 0, WINHTTP_NO_REQUEST_DATA, 0, 0, 0) &&
            WinHttpReceiveResponse(hRequest, nullptr))
        {
            DWORD statusSize = sizeof(statusCode);
            WinHttpQueryHeaders(hRequest, WINHTTP_QUERY_STATUS_CODE | WINHTTP_QUERY_FLAG_NUMBER, WINHTTP_HEADER_NAME_BY_INDEX, &statusCode, &statusSize, WINHTTP_NO_HEADER_INDEX);

            if (statusCode >= 200 && statusCode < 300)
            {
                while (true)
                {
                    DWORD available = 0;
                    if (!WinHttpQueryDataAvailable(hRequest, &available))
                    {
                        break;
                    }
                    if (available == 0)
                    {
                        ok = true;
                        break;
                    }

                    std::vector<char> buffer(available);
                    DWORD bytesRead = 0;
                    if (!WinHttpReadData(hRequest, buffer.data(), available, &bytesRead))
                    {
                        break;
                    }
                    output.write(buffer.data(), static_cast<std::streamsize>(bytesRead));
                    if (!output.good())
                    {
                        break;
                    }
                }
            }
        }

        if (!ok)
        {
            output.close();
            DeleteFile(destPath);
            if (outWinHttpError && *outWinHttpError == ERROR_SUCCESS)
            {
                *outWinHttpError = GetLastError();
            }
        }

        if (outStatusCode)
        {
            *outStatusCode = statusCode;
        }

        WinHttpCloseHandle(hRequest);
        WinHttpCloseHandle(hConnect);
        WinHttpCloseHandle(hSession);
        return ok;
    }

    bool ReadFileToString(const CString& path, std::string& outContent)
    {
        std::ifstream input(std::wstring(path), std::ios::binary);
        if (!input)
        {
            return false;
        }

        outContent.assign(std::istreambuf_iterator<char>(input), std::istreambuf_iterator<char>());
        return !outContent.empty();
    }

    CString FindSimcExecutableUnder(const CString& baseDir)
    {
        try
        {
            for (auto const& entry : fs::recursive_directory_iterator(std::wstring(baseDir)))
            {
                if (entry.path().filename() == L"simc.exe")
                {
                    return entry.path().wstring().c_str();
                }
            }
        }
        catch (...) {}

        return _T("");
    }

    bool ParseNightlyAsset(const std::string& response, CString& outVersion, CString& outDownloadUrl)
    {
        const std::string hrefPrefix = "href=\"simc-";
        const std::string win64Marker = "-win64.";

        std::string bestFileName;
        size_t searchPos = 0;
        while (true)
        {
            const size_t hrefPos = response.find(hrefPrefix, searchPos);
            if (hrefPos == std::string::npos)
            {
                break;
            }

            const size_t filePos = hrefPos + 6; // points to "simc-"
            const size_t quoteEnd = response.find('"', filePos);
            if (quoteEnd == std::string::npos)
            {
                break;
            }

            const std::string fileName = response.substr(filePos, quoteEnd - filePos);
            const size_t markerPos = fileName.find(win64Marker);
            const bool isArchive = (fileName.size() > 3 &&
                (fileName.rfind(".7z") == fileName.size() - 3 ||
                 fileName.rfind(".zip") == fileName.size() - 4));

            if (markerPos != std::string::npos && isArchive)
            {
                if (bestFileName.empty() || fileName > bestFileName)
                {
                    bestFileName = fileName;
                }
            }

            searchPos = quoteEnd + 1;
        }

        if (bestFileName.empty())
        {
            return false;
        }

        const size_t markerPos = bestFileName.find(win64Marker);
        if (markerPos == std::string::npos || markerPos <= 5)
        {
            return false;
        }

        const std::string version = bestFileName.substr(5, markerPos - 5);
        outVersion = version.c_str();
        outDownloadUrl = (std::string("https://downloads.simulationcraft.org/nightly/") + bestFileName).c_str();
        return true;
    }
}

CSimcDownloader::CSimcDownloader()
    : m_bCancelled(FALSE)
{
}

CSimcDownloader::~CSimcDownloader()
{
}

void CSimcDownloader::SetLastErrorMessage(const CString& message)
{
    m_lastErrorMessage = message;
}

void CSimcDownloader::SetLastErrorMessage(const CString& message, DWORD errorCode)
{
    m_lastErrorMessage.Format(_T("%s (error=%lu)"), (LPCTSTR)message, errorCode);
}

BOOL CSimcDownloader::CheckLatestVersion(CString& outVersion, CString& outDownloadUrl)
{
    m_lastErrorMessage.Empty();
    return FetchLatestReleaseInfo(outVersion, outDownloadUrl);
}

BOOL CSimcDownloader::DownloadAndInstall(const CString& installDir, CString& outSimcPath, ProgressCallback callback)
{
    m_bCancelled = FALSE;
    m_lastErrorMessage.Empty();
    CString version = _T("");
    CString downloadUrl = _T("");
    CString baseDir = GetDefaultInstallPath();
    CString targetDir = _T("");
    CString zipPath = _T("");
    CString extractTempDir = _T("");
    std::wstring foundSimcW = L"";
    TCHAR tempPath[MAX_PATH];

    if (callback) callback(5, _T("Checking latest version..."));
    if (!FetchLatestReleaseInfo(version, downloadUrl))
    {
        if (m_lastErrorMessage.IsEmpty())
        {
            SetLastErrorMessage(_T("Failed to resolve version metadata."));
        }
        if (callback) callback(0, _T("Failed to resolve version metadata."));
        return FALSE;
    }

    targetDir = baseDir + _T("\\") + version;
    
    if (PathIsDirectory(targetDir))
    {
        outSimcPath = FindSimcExecutableUnder(targetDir);
        if (!outSimcPath.IsEmpty())
        {
            if (callback) callback(100, _T("Latest version is already installed."));
            return TRUE;
        }
    }

    if (!CreateDirectoryRecursive(baseDir)) return FALSE;

    GetTempPath(MAX_PATH, tempPath);
    CString ext = (downloadUrl.Find(_T(".7z")) != -1) ? _T(".7z") : _T(".zip");
    zipPath.Format(_T("%s\\simc_%s%s"), tempPath, version, ext);

    if (callback) callback(10, _T("Downloading simulator..."));
    if (!DownloadFile(downloadUrl, zipPath, [callback](int p, const CString& s) {
        if (callback) callback(10 + (p * 70 / 100), s);
    }))
    {
        if (callback)
        {
            CString downloadError = m_lastErrorMessage.IsEmpty() ? CString(_T("Download failed.")) : m_lastErrorMessage;
            callback(0, downloadError);
        }
        return FALSE;
    }

    if (m_bCancelled) return FALSE;

    if (callback) callback(85, _T("Extracting archive (tar)..."));
    
    extractTempDir = baseDir + _T("\\temp_extract");
    if (fs::exists(std::wstring(extractTempDir))) {
        try { fs::remove_all(std::wstring(extractTempDir)); } catch(...) {}
    }
    if (!CreateDirectoryRecursive(extractTempDir)) return FALSE;

    if (!ExtractZip(zipPath, extractTempDir))
    {
        CString extractError = m_lastErrorMessage.IsEmpty() ? CString(_T("Archive extraction failed (tar.exe).")) : m_lastErrorMessage;
        if (callback) callback(0, extractError);
        return FALSE;
    }

    // Find the parent folder that actually contains simc.exe after extraction.
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
        SetLastErrorMessage(_T("Failed to finalize install directory."));
        if (callback) callback(0, _T("Failed to finalize install directory."));
        return FALSE;
    }

    // Verify simc.exe exists in final target location.
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
        SetLastErrorMessage(_T("simc.exe was not found after extraction."));
        if (callback) callback(0, _T("simc.exe was not found after extraction."));
        return FALSE;
    }

    outSimcPath = foundSimcW.c_str();
    DeleteFile(zipPath);
    if (fs::exists(std::wstring(extractTempDir))) {
        try { fs::remove_all(std::wstring(extractTempDir)); } catch(...) {}
    }

    if (callback) callback(100, _T("Install complete!"));
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
    auto pickWin64AssetFromRelease = [&outVersion, &outDownloadUrl](const json& release) -> bool
    {
        if (!release.is_object() || !release.contains("assets") || !release["assets"].is_array())
        {
            return false;
        }

        std::string tag;
        if (release.contains("tag_name") && release["tag_name"].is_string())
        {
            tag = release["tag_name"].get<std::string>();
        }

        for (const auto& asset : release["assets"])
        {
            if (!asset.is_object())
            {
                continue;
            }

            if (!asset.contains("name") || !asset["name"].is_string() ||
                !asset.contains("browser_download_url") || !asset["browser_download_url"].is_string())
            {
                continue;
            }

            const std::string name = asset["name"].get<std::string>();
            const std::string url = asset["browser_download_url"].get<std::string>();
            if (name.find("win64") != std::string::npos &&
                (name.find(".7z") != std::string::npos || name.find(".zip") != std::string::npos))
            {
                outVersion = tag.c_str();
                outDownloadUrl = url.c_str();
                return true;
            }
        }

        return false;
    };

    std::string response;
    if (HttpGet(_T("https://downloads.simulationcraft.org/nightly/"), response) &&
        ParseNightlyAsset(response, outVersion, outDownloadUrl))
    {
        return TRUE;
    }

    response.clear();
    if (HttpGet(_T("http://downloads.simulationcraft.org/nightly/"), response) &&
        ParseNightlyAsset(response, outVersion, outDownloadUrl))
    {
        return TRUE;
    }

    response.clear();
    if (HttpGet(_T("https://api.github.com/repos/simulationcraft/simc/releases/latest"), response))
    {
        try
        {
            auto j = json::parse(response);
            if (pickWin64AssetFromRelease(j))
            {
                return TRUE;
            }
        }
        catch (...) {}
    }

    // Some repositories return 404 for /releases/latest. Fall back to releases list.
    response.clear();
    if (HttpGet(_T("https://api.github.com/repos/simulationcraft/simc/releases?per_page=20"), response))
    {
        try
        {
            auto releases = json::parse(response);
            if (releases.is_array())
            {
                for (const auto& release : releases)
                {
                    if (pickWin64AssetFromRelease(release))
                    {
                        return TRUE;
                    }
                }
            }
        }
        catch (...) {}
    }

    if (m_lastErrorMessage.IsEmpty())
    {
        SetLastErrorMessage(_T("No usable version metadata found from nightly or GitHub."));
    }

    return FALSE;
}

BOOL CSimcDownloader::DownloadFile(const CString& url, const CString& destPath, ProgressCallback callback)
{
    auto tryDownload = [this, callback, &destPath](const CString& attemptUrl, DWORD& outCurlExitCode, DWORD& outCurlLaunchError, DWORD& outStatusCode, DWORD& outWinHttpError) -> BOOL
    {
        outCurlExitCode = 1;
        outCurlLaunchError = ERROR_SUCCESS;
        outStatusCode = 0;
        outWinHttpError = ERROR_SUCCESS;

        if (DownloadWithCurl(attemptUrl, destPath, &outCurlExitCode, &outCurlLaunchError))
        {
            if (callback) callback(100, _T("Download complete."));
            return TRUE;
        }

        if (DownloadWithWinHttpToFile(attemptUrl, destPath, &outStatusCode, &outWinHttpError))
        {
            if (callback) callback(100, _T("Download complete."));
            return TRUE;
        }

        DeleteFile(destPath);
        return FALSE;
    };

    DWORD curlExitCode = 1;
    DWORD curlLaunchError = ERROR_SUCCESS;
    DWORD statusCode = 0;
    DWORD winHttpError = ERROR_SUCCESS;
    if (tryDownload(url, curlExitCode, curlLaunchError, statusCode, winHttpError))
    {
        return TRUE;
    }

    CString finalUrl = url;
    CString fallbackUrl(url);
    const CString httpsPrefix = _T("https://downloads.simulationcraft.org/");
    if (fallbackUrl.Left(httpsPrefix.GetLength()).CompareNoCase(httpsPrefix) == 0)
    {
        fallbackUrl = _T("http://") + fallbackUrl.Mid(8);
        if (tryDownload(fallbackUrl, curlExitCode, curlLaunchError, statusCode, winHttpError))
        {
            return TRUE;
        }
        finalUrl = fallbackUrl;
    }

    CString msg;
    if (curlLaunchError != ERROR_SUCCESS)
    {
        msg.Format(_T("download failed: %s (curl launch error=%lu, winhttp error=%lu, status=%lu)"),
            static_cast<LPCTSTR>(finalUrl), curlLaunchError, winHttpError, statusCode);
    }
    else
    {
        msg.Format(_T("download failed: %s (curl exit=%lu, winhttp error=%lu, status=%lu)"),
            static_cast<LPCTSTR>(finalUrl), curlExitCode, winHttpError, statusCode);
    }
    SetLastErrorMessage(msg);
    return FALSE;
}

BOOL CSimcDownloader::ExtractZip(const CString& zipPath, const CString& destDir)
{
    DWORD tarExitCode = 1;
    DWORD tarLaunchError = ERROR_SUCCESS;

    CString tarArgs;
    tarArgs.Format(_T("-xf \"%s\" -C \"%s\""), (LPCTSTR)zipPath, (LPCTSTR)destDir);
    if (RunProcess(_T("tar.exe"), tarArgs, &tarExitCode, &tarLaunchError))
    {
        return TRUE;
    }

    if (tarLaunchError == ERROR_FILE_NOT_FOUND || tarLaunchError == ERROR_PATH_NOT_FOUND)
    {
        TCHAR systemDir[MAX_PATH] = {};
        if (GetSystemDirectory(systemDir, MAX_PATH) > 0)
        {
            CString systemTar = systemDir;
            if (!systemTar.IsEmpty() && systemTar.Right(1) != _T("\\"))
            {
                systemTar += _T("\\");
            }
            systemTar += _T("tar.exe");

            if (RunProcess(systemTar, tarArgs, &tarExitCode, &tarLaunchError))
            {
                return TRUE;
            }
        }
    }

    // For .zip, also try PowerShell Expand-Archive as a fallback.
    if (zipPath.Right(4).CompareNoCase(_T(".zip")) == 0)
    {
        DWORD psExitCode = 1;
        DWORD psLaunchError = ERROR_SUCCESS;
        CString psArgs;
        psArgs.Format(
            _T("-NoProfile -NonInteractive -ExecutionPolicy Bypass -Command \"try { Expand-Archive -LiteralPath '%s' -DestinationPath '%s' -Force; exit 0 } catch { exit 1 }\""),
            static_cast<LPCTSTR>(EscapePowerShellSingleQuoted(zipPath)),
            static_cast<LPCTSTR>(EscapePowerShellSingleQuoted(destDir)));

        if (RunProcess(_T("powershell.exe"), psArgs, &psExitCode, &psLaunchError))
        {
            return TRUE;
        }

        if (psLaunchError != ERROR_SUCCESS)
        {
            SetLastErrorMessage(_T("Archive extraction tool launch failed"), psLaunchError);
        }
        else
        {
            CString msg;
            msg.Format(_T("Archive extraction failed (tar exit=%lu, powershell exit=%lu)"), tarExitCode, psExitCode);
            SetLastErrorMessage(msg);
        }
        return FALSE;
    }

    if (tarLaunchError != ERROR_SUCCESS)
    {
        SetLastErrorMessage(_T("Archive extraction tool launch failed"), tarLaunchError);
    }
    else
    {
        CString msg;
        msg.Format(_T("Archive extraction failed (tar exit=%lu)"), tarExitCode);
        SetLastErrorMessage(msg);
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
    TCHAR tempPath[MAX_PATH];
    TCHAR tempFile[MAX_PATH];
    GetTempPath(MAX_PATH, tempPath);
    GetTempFileName(tempPath, _T("sim"), 0, tempFile);

    DWORD curlExitCode = 1;
    DWORD curlLaunchError = ERROR_SUCCESS;
    if (!DownloadWithCurl(url, tempFile, &curlExitCode, &curlLaunchError))
    {
        DWORD statusCode = 0;
        DWORD winHttpError = ERROR_SUCCESS;
        if (HttpGetWithWinHttp(url, outResponse, &statusCode, &winHttpError))
        {
            DeleteFile(tempFile);
            return TRUE;
        }

        DeleteFile(tempFile);
        CString msg;
        if (curlLaunchError != ERROR_SUCCESS)
        {
            msg.Format(_T("metadata fetch failed: %s (curl launch error=%lu, winhttp error=%lu, status=%lu)"),
                static_cast<LPCTSTR>(url), curlLaunchError, winHttpError, statusCode);
        }
        else
        {
            msg.Format(_T("metadata fetch failed: %s (curl exit=%lu, winhttp error=%lu, status=%lu)"),
                static_cast<LPCTSTR>(url), curlExitCode, winHttpError, statusCode);
        }
        SetLastErrorMessage(msg);
        return FALSE;
    }

    const bool readOk = ReadFileToString(tempFile, outResponse);
    DeleteFile(tempFile);

    if (!readOk || outResponse.empty())
    {
        SetLastErrorMessage(_T("Version response body was empty."));
        return FALSE;
    }

    return TRUE;
}
