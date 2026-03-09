#include "pch.h"
#include "framework.h"
#include "BnetApiClient.h"
#include "CharacterData.h"

#pragma comment(lib, "winhttp.lib")

CBnetApiClient::CBnetApiClient()
    : m_hSession(nullptr)
    , m_hConnect(nullptr)
    , m_dwTokenExpires(0)
    , m_nLastHttpStatus(0)
{
}

CBnetApiClient::~CBnetApiClient()
{
    Cleanup();
}

BOOL CBnetApiClient::Initialize()
{
    // Create WinHTTP session
    m_hSession = WinHttpOpen(USER_AGENT,
        WINHTTP_ACCESS_TYPE_AUTOMATIC_PROXY,
        WINHTTP_NO_PROXY_NAME,
        WINHTTP_NO_PROXY_BYPASS, 0);

    if (!m_hSession)
    {
        m_strLastError = _T("Failed to initialize WinHTTP session");
        return FALSE;
    }

    // Set timeouts
    WinHttpSetTimeouts(m_hSession, HTTP_TIMEOUT_MS, HTTP_TIMEOUT_MS,
                       HTTP_TIMEOUT_MS, HTTP_TIMEOUT_MS);

    return TRUE;
}

void CBnetApiClient::Cleanup()
{
    if (m_hConnect)
    {
        WinHttpCloseHandle(m_hConnect);
        m_hConnect = nullptr;
    }
    if (m_hSession)
    {
        WinHttpCloseHandle(m_hSession);
        m_hSession = nullptr;
    }
}

BOOL CBnetApiClient::Authenticate(const CString& clientId, const CString& clientSecret)
{
    m_strClientId = clientId;
    m_strClientSecret = clientSecret;

    // For client credentials flow, directly request token
    // Build POST data
    CString postData;
    postData.Format(_T("grant_type=client_credentials&client_id=%s&client_secret=%s"),
        clientId, clientSecret);

    CString response;
    BOOL result = HttpPost(_T("https://oauth.battle.net/token"),
                          postData, _T("application/x-www-form-urlencoded"), response);

    if (!result)
    {
        return FALSE;
    }

    try
    {
        auto j = json::parse(CT2A(response));

        if (j.contains("access_token"))
        {
            m_strAccessToken = CA2T(j["access_token"].get<std::string>().c_str());
        }
        if (j.contains("refresh_token"))
        {
            m_strRefreshToken = CA2T(j["refresh_token"].get<std::string>().c_str());
        }
        if (j.contains("expires_in"))
        {
            m_dwTokenExpires = GetTickCount() + (j["expires_in"].get<int>() * 1000);
        }

        return TRUE;
    }
    catch (const std::exception& e)
    {
        m_strLastError.Format(_T("Failed to parse token response: %s"), CA2T(e.what()));
        return FALSE;
    }
}

BOOL CBnetApiClient::RefreshAccessToken()
{
    if (m_strRefreshToken.IsEmpty())
    {
        m_strLastError = _T("No refresh token available");
        return FALSE;
    }

    CString postData;
    postData.Format(_T("grant_type=refresh_token&refresh_token=%s&client_id=%s&client_secret=%s"),
        m_strRefreshToken, m_strClientId, m_strClientSecret);

    CString response;
    return HttpPost(_T("https://oauth.battle.net/token"),
                    postData, _T("application/x-www-form-urlencoded"), response);
}

CString CBnetApiClient::GetAuthorizationUrl() const
{
    // Build authorization URL for implicit flow
    CString url;
    url.Format(_T("https://oauth.battle.net/authorize?client_id=%s&redirect_uri=http://localhost:8080/callback&response_type=code&scope=wow.profile"),
        m_strClientId);
    return url;
}

BOOL CBnetApiClient::FetchCharacter(const std::string& region, const std::string& server,
                                     const std::string& characterName, CCharacterData* pOutData)
{
    if (!m_hSession && !Initialize())
    {
        return FALSE;
    }

    // Build URL
    CString url;
    url.Format(_T("%s/profile/wow/character/%s/%s"),
        BuildApiBaseUrl(region),
        CA2T(server.c_str()),
        CA2T(characterName.c_str()));

    // Add namespace and locale
    url += _T("?namespace=") + BuildNamespace(region);
    url += _T("&locale=en_US");
    url += _T("&access_token=") + m_strAccessToken;

    CString response;
    if (!HttpGet(url, response))
    {
        return FALSE;
    }

    try
    {
        auto j = json::parse(CT2A(response));

        if (j.contains("code") && j.contains("type"))
        {
            // Error response
            m_strLastError.Format(_T("API Error: %s"), CA2T(j["detail"].get<std::string>().c_str()));
            return FALSE;
        }

        pOutData->SetRegion(region);
        return ParseCharacterProfile(j, pOutData);
    }
    catch (const std::exception& e)
    {
        m_strLastError.Format(_T("Failed to parse character data: %s"), CA2T(e.what()));
        return FALSE;
    }
}

BOOL CBnetApiClient::FetchCharacterEquipment(const std::string& region, const std::string& server,
                                              const std::string& characterName, CCharacterData* pOutData)
{
    CString url;
    url.Format(_T("%s/profile/wow/character/%s/%s/equipment"),
        BuildApiBaseUrl(region),
        CA2T(server.c_str()),
        CA2T(characterName.c_str()));

    url += _T("?namespace=") + BuildNamespace(region);
    url += _T("&locale=en_US");
    url += _T("&access_token=") + m_strAccessToken;

    CString response;
    if (!HttpGet(url, response))
    {
        return FALSE;
    }

    try
    {
        auto j = json::parse(CT2A(response));
        return ParseEquipment(j, pOutData);
    }
    catch (const std::exception& e)
    {
        m_strLastError.Format(_T("Failed to parse equipment data: %s"), CA2T(e.what()));
        return FALSE;
    }
}

BOOL CBnetApiClient::FetchCharacterSpecializations(const std::string& region, const std::string& server,
                                                    const std::string& characterName, CCharacterData* pOutData)
{
    CString url;
    url.Format(_T("%s/profile/wow/character/%s/%s/specializations"),
        BuildApiBaseUrl(region),
        CA2T(server.c_str()),
        CA2T(characterName.c_str()));

    url += _T("?namespace=") + BuildNamespace(region);
    url += _T("&locale=en_US");
    url += _T("&access_token=") + m_strAccessToken;

    CString response;
    if (!HttpGet(url, response))
    {
        return FALSE;
    }

    try
    {
        auto j = json::parse(CT2A(response));
        return ParseSpecializations(j, pOutData);
    }
    catch (const std::exception& e)
    {
        m_strLastError.Format(_T("Failed to parse specialization data: %s"), CA2T(e.what()));
        return FALSE;
    }
}

BOOL CBnetApiClient::HttpGet(const CString& url, CString& outResponse)
{
    URL_COMPONENTS urlComp = {};
    urlComp.dwStructSize = sizeof(urlComp);

    TCHAR szHostName[256] = {0};
    TCHAR szUrlPath[2048] = {0};
    urlComp.lpszHostName = szHostName;
    urlComp.dwHostNameLength = ARRAYSIZE(szHostName);
    urlComp.lpszUrlPath = szUrlPath;
    urlComp.dwUrlPathLength = ARRAYSIZE(szUrlPath);

    if (!WinHttpCrackUrl(url, 0, 0, &urlComp))
    {
        m_strLastError = _T("Failed to parse URL");
        return FALSE;
    }

    HINTERNET hConnect = WinHttpConnect(m_hSession, szHostName, urlComp.nPort, 0);
    if (!hConnect)
    {
        m_strLastError = _T("Failed to connect to host");
        return FALSE;
    }

    HINTERNET hRequest = WinHttpOpenRequest(hConnect, L"GET", szUrlPath,
        NULL, WINHTTP_NO_REFERER, WINHTTP_DEFAULT_ACCEPT_TYPES,
        urlComp.nScheme == INTERNET_SCHEME_HTTPS ? WINHTTP_FLAG_SECURE : 0);

    if (!hRequest)
    {
        WinHttpCloseHandle(hConnect);
        m_strLastError = _T("Failed to create request");
        return FALSE;
    }

    // Send request
    BOOL bResults = WinHttpSendRequest(hRequest, WINHTTP_NO_ADDITIONAL_HEADERS, 0,
                                       WINHTTP_NO_REQUEST_DATA, 0, 0, 0);

    if (bResults)
    {
        bResults = WinHttpReceiveResponse(hRequest, NULL);
    }

    // Get status code
    DWORD dwStatusCode = 0;
    DWORD dwSize = sizeof(dwStatusCode);
    WinHttpQueryHeaders(hRequest, WINHTTP_QUERY_STATUS_CODE | WINHTTP_QUERY_FLAG_NUMBER,
                        WINHTTP_HEADER_NAME_BY_INDEX, &dwStatusCode, &dwSize, WINHTTP_NO_HEADER_INDEX);
    m_nLastHttpStatus = dwStatusCode;

    if (dwStatusCode != 200)
    {
        WinHttpCloseHandle(hRequest);
        WinHttpCloseHandle(hConnect);
        m_strLastError.Format(_T("HTTP Error: %d"), dwStatusCode);
        return FALSE;
    }

    // Read response
    std::string response;
    DWORD dwDownloaded = 0;
    do
    {
        DWORD dwAvailable = 0;
        WinHttpQueryDataAvailable(hRequest, &dwAvailable);

        if (dwAvailable == 0)
            break;

        std::vector<char> buffer(dwAvailable + 1);
        WinHttpReadData(hRequest, buffer.data(), dwAvailable, &dwDownloaded);
        buffer[dwDownloaded] = 0;
        response += buffer.data();
    } while (dwDownloaded > 0);

    WinHttpCloseHandle(hRequest);
    WinHttpCloseHandle(hConnect);

    outResponse = CA2T(response.c_str());
    return TRUE;
}

BOOL CBnetApiClient::HttpPost(const CString& url, const CString& postData,
                               const CString& contentType, CString& outResponse)
{
    URL_COMPONENTS urlComp = {};
    urlComp.dwStructSize = sizeof(urlComp);

    TCHAR szHostName[256] = {0};
    TCHAR szUrlPath[2048] = {0};
    urlComp.lpszHostName = szHostName;
    urlComp.dwHostNameLength = ARRAYSIZE(szHostName);
    urlComp.lpszUrlPath = szUrlPath;
    urlComp.dwUrlPathLength = ARRAYSIZE(szUrlPath);

    if (!WinHttpCrackUrl(url, 0, 0, &urlComp))
    {
        m_strLastError = _T("Failed to parse URL");
        return FALSE;
    }

    HINTERNET hConnect = WinHttpConnect(m_hSession, szHostName, urlComp.nPort, 0);
    if (!hConnect)
    {
        m_strLastError = _T("Failed to connect to host");
        return FALSE;
    }

    HINTERNET hRequest = WinHttpOpenRequest(hConnect, L"POST", szUrlPath,
        NULL, WINHTTP_NO_REFERER, WINHTTP_DEFAULT_ACCEPT_TYPES,
        urlComp.nScheme == INTERNET_SCHEME_HTTPS ? WINHTTP_FLAG_SECURE : 0);

    if (!hRequest)
    {
        WinHttpCloseHandle(hConnect);
        m_strLastError = _T("Failed to create request");
        return FALSE;
    }

    // Add Content-Type header
    CString headers;
    headers.Format(_T("Content-Type: %s"), contentType);

    // Convert post data
    CT2A postDataA(postData);
    DWORD postDataLen = strlen(postDataA);

    BOOL bResults = WinHttpSendRequest(hRequest,
                                       CT2W(headers), -1,
                                       (LPVOID)(const char*)postDataA, postDataLen,
                                       postDataLen, 0);

    if (bResults)
    {
        bResults = WinHttpReceiveResponse(hRequest, NULL);
    }

    // Get status code
    DWORD dwStatusCode = 0;
    DWORD dwSize = sizeof(dwStatusCode);
    WinHttpQueryHeaders(hRequest, WINHTTP_QUERY_STATUS_CODE | WINHTTP_QUERY_FLAG_NUMBER,
                        WINHTTP_HEADER_NAME_BY_INDEX, &dwStatusCode, &dwSize, WINHTTP_NO_HEADER_INDEX);
    m_nLastHttpStatus = dwStatusCode;

    // Read response
    std::string response;
    DWORD dwDownloaded = 0;
    do
    {
        DWORD dwAvailable = 0;
        WinHttpQueryDataAvailable(hRequest, &dwAvailable);

        if (dwAvailable == 0)
            break;

        std::vector<char> buffer(dwAvailable + 1);
        WinHttpReadData(hRequest, buffer.data(), dwAvailable, &dwDownloaded);
        buffer[dwDownloaded] = 0;
        response += buffer.data();
    } while (dwDownloaded > 0);

    WinHttpCloseHandle(hRequest);
    WinHttpCloseHandle(hConnect);

    outResponse = CA2T(response.c_str());
    return bResults && dwStatusCode == 200;
}

CString CBnetApiClient::BuildApiBaseUrl(const std::string& region) const
{
    CString baseUrl;
    baseUrl.Format(_T("https://%s.api.blizzard.com"), CA2T(region.c_str()));
    return baseUrl;
}

CString CBnetApiClient::BuildNamespace(const std::string& region) const
{
    // Determine namespace based on region
    // profile-{region} for character data
    CString ns;
    ns.Format(_T("profile-%s"), CA2T(region.c_str()));
    return ns;
}

BOOL CBnetApiClient::ParseCharacterProfile(const json& j, CCharacterData* pOutData)
{
    try
    {
        if (j.contains("name"))
            pOutData->SetName(j["name"].get<std::string>());

        if (j.contains("realm") && j["realm"].contains("name"))
            pOutData->SetRealm(j["realm"]["name"].get<std::string>());

        if (j.contains("level"))
            pOutData->SetLevel(j["level"].get<int>());

        if (j.contains("item_level"))
            pOutData->SetItemLevel(j["item_level"].get<double>());

        if (j.contains("character_class") && j["character_class"].contains("name"))
            pOutData->SetClassName(j["character_class"]["name"].get<std::string>());

        if (j.contains("active_spec") && j["active_spec"].contains("name"))
            pOutData->SetActiveSpecName(j["active_spec"]["name"].get<std::string>());

        return TRUE;
    }
    catch (const std::exception& e)
    {
        m_strLastError.Format(_T("Parse error: %s"), CA2T(e.what()));
        return FALSE;
    }
}

BOOL CBnetApiClient::ParseEquipment(const json& j, CCharacterData* pOutData)
{
    try
    {
        if (j.contains("equipped_items") && j["equipped_items"].is_array())
        {
            for (const auto& item : j["equipped_items"])
            {
                CCharacterData::EquipmentSlot slot;

                if (item.contains("slot") && item["slot"].contains("type"))
                    slot.slotType = item["slot"]["type"].get<std::string>();

                if (item.contains("item"))
                    slot.itemId = std::to_string(item["item"]["id"].get<int>());

                if (item.contains("level") && item["level"].contains("value"))
                    slot.itemLevel = item["level"]["value"].get<int>();

                if (item.contains("quality") && item["quality"].contains("name"))
                    slot.quality = item["quality"]["name"].get<std::string>();

                if (item.contains("name"))
                    slot.name = item["name"].get<std::string>();

                // Parse bonus IDs if available
                if (item.contains("bonus_list"))
                {
                    for (const auto& bonus : item["bonus_list"])
                    {
                        slot.bonusIds.push_back(std::to_string(bonus.get<int>()));
                    }
                }

                // Parse gems
                if (item.contains("sockets"))
                {
                    for (const auto& socket : item["sockets"])
                    {
                        if (socket.contains("item"))
                        {
                            slot.gemIds.push_back(std::to_string(socket["item"]["id"].get<int>()));
                        }
                    }
                }

                pOutData->AddEquipment(slot);
            }
        }

        return TRUE;
    }
    catch (const std::exception& e)
    {
        m_strLastError.Format(_T("Parse equipment error: %s"), CA2T(e.what()));
        return FALSE;
    }
}

BOOL CBnetApiClient::ParseSpecializations(const json& j, CCharacterData* pOutData)
{
    try
    {
        if (j.contains("specializations") && j["specializations"].is_array())
        {
            for (const auto& spec : j["specializations"])
            {
                if (spec.contains("talents") && spec["talents"].is_array())
                {
                    for (const auto& talent : spec["talents"])
                    {
                        if (talent.contains("spell_tooltip") &&
                            talent["spell_tooltip"].contains("spell"))
                        {
                            int spellId = talent["spell_tooltip"]["spell"]["id"].get<int>();
                            pOutData->AddTalent(spellId);
                        }
                    }
                }
            }
        }

        return TRUE;
    }
    catch (const std::exception& e)
    {
        m_strLastError.Format(_T("Parse specs error: %s"), CA2T(e.what()));
        return FALSE;
    }
}
