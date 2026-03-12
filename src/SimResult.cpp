#include "pch.h"
#include "framework.h"
#include "SimResult.h"

CSimResult::CSimResult()
    : m_duration(0)
    , m_iterations(0)
    , m_simTime(0.0)
{
}

CSimResult::~CSimResult()
{
}

void CSimResult::GenerateId()
{
}

CString CSimResult::GetTimestampString() const
{
    return _T("");
}

json CSimResult::ToJson() const
{
    return json();
}

BOOL CSimResult::FromJson(const json& j)
{
    return FALSE;
}

BOOL CSimResult::SaveToFile(const CString& filePath) const
{
    return FALSE;
}

BOOL CSimResult::LoadFromFile(const CString& filePath)
{
    return FALSE;
}

CString CSimResult::CompareWith(const CSimResult& other) const
{
    return _T("");
}

CString CSimResult::GetDpsString() const
{
    return _T("");
}

CString CSimResult::GetHpsString() const
{
    return _T("");
}

CString CSimResult::GenerateUuid()
{
    return _T("");
}
