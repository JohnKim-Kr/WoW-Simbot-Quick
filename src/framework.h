#pragma once

#ifndef VC_EXTRALEAN
#define VC_EXTRALEAN            // Exclude rarely-used stuff from Windows headers
#endif

#include "targetver.h"

#define _ATL_CSTRING_EXPLICIT_CONSTRUCTORS      // some CString constructors will be explicit

// turns off MFC's hiding of some common and often safely ignored warning messages
#define _AFX_ALL_WARNINGS

#include <afxwin.h>         // MFC core and standard components
#include <afxext.h>         // MFC extensions
#include <afxcview.h>
#include <afxdisp.h>        // MFC Automation classes
#include <afxbutton.h>
#include <afxcmn.h>         // MFC support for Windows Common Controls
#include <afxcontrolbars.h>     // MFC support for ribbons and control bars
#include <afxdlgs.h>
#include <afxlistctrl.h>
#include <afxpropertygridctrl.h>

// Windows API headers
#ifndef NOMINMAX
#define NOMINMAX  // Prevent Windows from defining min/max macros
#endif
#include <windows.h>
#include <winhttp.h>
#include <shellapi.h>
#include <shlobj.h>
#include <atlbase.h>
#include <atlconv.h>

// C++ Standard Library
#include <string>
#include <vector>
#include <map>
#include <memory>
#include <thread>
#include <mutex>
#include <future>
#include <chrono>
#include <sstream>
#include <fstream>
#include <filesystem>
#include <functional>
#include <algorithm>
#include <regex>

// JSON library
#include "json.hpp"

namespace fs = std::filesystem;
using json = nlohmann::json;
