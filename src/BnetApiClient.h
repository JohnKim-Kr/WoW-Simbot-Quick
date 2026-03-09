#pragma once

// Forward declarations
class CCharacterData;

// Battle.net API client using WinHTTP
class CBnetApiClient
{
public:
    CBnetApiClient();
    ~CBnetApiClient();

    // Initialize the HTTP session
    BOOL Initialize();
    void Cleanup();

    // OAuth2 authentication
    BOOL Authenticate(const CString& clientId, const CString& clientSecret);
    BOOL RefreshAccessToken();
    CString GetAuthorizationUrl() const;
    BOOL ExchangeCodeForToken(const CString& authCode);

    // Character API methods
    BOOL FetchCharacter(const std::string& region, const std::string& server,
                        const std::string& characterName, CCharacterData* pOutData);
    BOOL FetchCharacterEquipment(const std::string& region, const std::string& server,
                                  const std::string& characterName, CCharacterData* pOutData);
    BOOL FetchCharacterSpecializations(const std::string& region, const std::string& server,
                                        const std::string& characterName, CCharacterData* pOutData);

    // Error handling
    CString GetLastErrorString() const { return m_strLastError; }
    int GetLastHttpStatus() const { return m_nLastHttpStatus; }

private:
    // HTTP request methods
    BOOL HttpGet(const CString& url, CString& outResponse);
    BOOL HttpPost(const CString& url, const CString& postData,
                  const CString& contentType, CString& outResponse);

    // Build API URLs
    CString BuildApiBaseUrl(const std::string& region) const;
    CString BuildNamespace(const std::string& region) const;

    // Parse responses
    BOOL ParseCharacterProfile(const json& j, CCharacterData* pOutData);
    BOOL ParseEquipment(const json& j, CCharacterData* pOutData);
    BOOL ParseSpecializations(const json& j, CCharacterData* pOutData);

private:
    HINTERNET m_hSession;
    HINTERNET m_hConnect;

    // OAuth tokens
    CString m_strAccessToken;
    CString m_strRefreshToken;
    DWORD   m_dwTokenExpires;

    // Credentials
    CString m_strClientId;
    CString m_strClientSecret;

    // Error tracking
    CString m_strLastError;
    int     m_nLastHttpStatus;

    // Constants
    static constexpr int HTTP_TIMEOUT_MS = 30000;
    static constexpr LPCWSTR USER_AGENT = L"WoWSimbotQuick/1.0";
};
