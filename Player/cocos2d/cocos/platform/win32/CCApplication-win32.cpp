﻿/****************************************************************************
Copyright (c) 2010-2012 cocos2d-x.org
Copyright (c) 2013-2016 Chukong Technologies Inc.
Copyright (c) 2017-2018 Xiamen Yaji Software Co., Ltd.

http://www.cocos2d-x.org

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
****************************************************************************/

#include "platform/CCPlatformConfig.h"
#if CC_TARGET_PLATFORM == CC_PLATFORM_WIN32

#include "platform/CCApplication.h"
#include "base/CCDirector.h"
#include <algorithm>
#include "platform/CCFileUtils.h"
#ifdef USE_AGTK
#include "base/ccUTF8.h"
#endif
#include <shellapi.h>
#include <WinVer.h>
/**
@brief    This function change the PVRFrame show/hide setting in register.
@param  bEnable If true show the PVRFrame window, otherwise hide.
*/
static void PVRFrameEnableControlWindow(bool bEnable);

NS_CC_BEGIN

// sharedApplication pointer
Application * Application::sm_pSharedApplication = nullptr;
#ifdef USE_AGTK
void (*Application::_dropFileCallback)(const char *filename) = nullptr;
#endif

#ifdef USE_AGTK_BLOCK_PROCESS_DURING_WINDOW_RESIZING // 2017.12.6 tada
HHOOK Application::_getMsgHook = nullptr;
#endif
Application::Application()
: _instance(nullptr)
, _accelTable(nullptr)
{
    _instance    = GetModuleHandle(nullptr);
    _animationInterval.QuadPart = 0;
    CC_ASSERT(! sm_pSharedApplication);
    sm_pSharedApplication = this;
#ifdef USE_AGTK_BLOCK_PROCESS_DURING_WINDOW_RESIZING // 2017.12.6 tada
	_getMsgHook = ::SetWindowsHookEx(WH_GETMESSAGE, &Application::GetMsgProc, _instance, ::GetCurrentThreadId());
#endif
}

Application::~Application()
{
    CC_ASSERT(this == sm_pSharedApplication);
    sm_pSharedApplication = nullptr;
#ifdef USE_AGTK_BLOCK_PROCESS_DURING_WINDOW_RESIZING // 2017.12.6 tada
	if (nullptr != _getMsgHook) {
		UnhookWindowsHookEx(_getMsgHook);
		_getMsgHook = nullptr;
	}
#endif
}

int Application::run()
{
    PVRFrameEnableControlWindow(false);

    ///////////////////////////////////////////////////////////////////////////
    /////////////// changing timer resolution
    ///////////////////////////////////////////////////////////////////////////
    UINT TARGET_RESOLUTION = 1; // 1 millisecond target resolution
    TIMECAPS tc;
    UINT wTimerRes = 0;
    if (TIMERR_NOERROR == timeGetDevCaps(&tc, sizeof(TIMECAPS)))
    {
        wTimerRes = std::min(std::max(tc.wPeriodMin, TARGET_RESOLUTION), tc.wPeriodMax);
        timeBeginPeriod(wTimerRes);
    }

    // Main message loop:
    LARGE_INTEGER nLast;
    LARGE_INTEGER nNow;

    QueryPerformanceCounter(&nLast);

    initGLContextAttrs();

    // Initialize instance and cocos2d.
    if (!applicationDidFinishLaunching())
    {
        return 1;
    }

    auto director = Director::getInstance();
    auto glview = director->getOpenGLView();

    // Retain glview to avoid glview being released in the while loop
    glview->retain();

    LONGLONG interval = 0LL;
    LONG waitMS = 0L;

    LARGE_INTEGER freq;
    QueryPerformanceFrequency(&freq);

    while(!glview->windowShouldClose())
    {
        QueryPerformanceCounter(&nNow);
        interval = nNow.QuadPart - nLast.QuadPart;
        if (interval >= _animationInterval.QuadPart)
        {
            nLast.QuadPart = nNow.QuadPart;
            director->mainLoop();
            glview->pollEvents();
        }
        else
        {
            // The precision of timer on Windows is set to highest (1ms) by 'timeBeginPeriod' from above code,
            // but it's still not precise enough. For example, if the precision of timer is 1ms,
            // Sleep(3) may make a sleep of 2ms or 4ms. Therefore, we subtract 1ms here to make Sleep time shorter.
            // If 'waitMS' is equal or less than 1ms, don't sleep and run into next loop to
            // boost CPU to next frame accurately.
            waitMS = (_animationInterval.QuadPart - interval) * 1000LL / freq.QuadPart - 1L;
            if (waitMS > 1L)
                Sleep(waitMS);
        }
    }

    // Director should still do a cleanup if the window was closed manually.
    if (glview->isOpenGLReady())
    {
        director->end();
        director->mainLoop();
        director = nullptr;
    }
    glview->release();

    ///////////////////////////////////////////////////////////////////////////
    /////////////// restoring timer resolution
    ///////////////////////////////////////////////////////////////////////////
    if (wTimerRes != 0)
    {
        timeEndPeriod(wTimerRes);
    }
    return 0;
}

void Application::setAnimationInterval(float interval)
{
    LARGE_INTEGER freq;
    QueryPerformanceFrequency(&freq);
    _animationInterval.QuadPart = (LONGLONG)(interval * freq.QuadPart);
}

void Application::setAnimationInterval(float interval, SetIntervalReason reason)
{
    setAnimationInterval(interval);
}
#ifdef USE_AGTK    
void Application::setDropFileCallback(void(*callback)(const char *filename))
{
	_dropFileCallback = callback;
}
#endif

//////////////////////////////////////////////////////////////////////////
// static member function
//////////////////////////////////////////////////////////////////////////
Application* Application::getInstance()
{
    CC_ASSERT(sm_pSharedApplication);
    return sm_pSharedApplication;
}

// @deprecated Use getInstance() instead
Application* Application::sharedApplication()
{
    return Application::getInstance();
}

LanguageType Application::getCurrentLanguage()
{
    LanguageType ret = LanguageType::ENGLISH;
    
    LCID localeID = GetUserDefaultLCID();
    unsigned short primaryLanguageID = localeID & 0xFF;
    
    switch (primaryLanguageID)
    {
        case LANG_CHINESE:
            ret = LanguageType::CHINESE;
            break;
        case LANG_ENGLISH:
            ret = LanguageType::ENGLISH;
            break;
        case LANG_FRENCH:
            ret = LanguageType::FRENCH;
            break;
        case LANG_ITALIAN:
            ret = LanguageType::ITALIAN;
            break;
        case LANG_GERMAN:
            ret = LanguageType::GERMAN;
            break;
        case LANG_SPANISH:
            ret = LanguageType::SPANISH;
            break;
        case LANG_DUTCH:
            ret = LanguageType::DUTCH;
            break;
        case LANG_RUSSIAN:
            ret = LanguageType::RUSSIAN;
            break;
        case LANG_KOREAN:
            ret = LanguageType::KOREAN;
            break;
        case LANG_JAPANESE:
            ret = LanguageType::JAPANESE;
            break;
        case LANG_HUNGARIAN:
            ret = LanguageType::HUNGARIAN;
            break;
        case LANG_PORTUGUESE:
            ret = LanguageType::PORTUGUESE;
            break;
        case LANG_ARABIC:
            ret = LanguageType::ARABIC;
            break;
        case LANG_NORWEGIAN:
            ret = LanguageType::NORWEGIAN;
            break;
        case LANG_POLISH:
            ret = LanguageType::POLISH;
            break;
        case LANG_TURKISH:
            ret = LanguageType::TURKISH;
            break;
        case LANG_UKRAINIAN:
            ret = LanguageType::UKRAINIAN;
            break;
        case LANG_ROMANIAN:
            ret = LanguageType::ROMANIAN;
            break;
        case LANG_BULGARIAN:
            ret = LanguageType::BULGARIAN;
            break;
        case LANG_BELARUSIAN:
            ret = LanguageType::BELARUSIAN;
            break;
    }
    
    return ret;
}

const char * Application::getCurrentLanguageCode()
{
    LANGID lid = GetUserDefaultUILanguage();
    const LCID locale_id = MAKELCID(lid, SORT_DEFAULT);
    static char code[3] = { 0 };
    GetLocaleInfoA(locale_id, LOCALE_SISO639LANGNAME, code, sizeof(code));
    code[2] = '\0';
    return code;
}

#ifdef USE_AGTK
std::string Application::getCurrentLanguageShortName() const
{
	auto getLocal = [](LCTYPE type) {
		int const len = GetLocaleInfoA(LOCALE_USER_DEFAULT, type, NULL, 0);
		if (len > 0) {
			std::string langName;
			langName.resize(len);
			GetLocaleInfoA(LOCALE_USER_DEFAULT, type, &langName[0], langName.size());
			return langName;
		}
		return std::string();
	};

	std::string lang = getLocal(LOCALE_SNAME).substr(0,5);// jp-JP を取得
	lang.replace(lang.find("-"), 1, "_");// jp_JP に置き換え
	return lang;
}
#endif
Application::Platform Application::getTargetPlatform()
{
    return Platform::OS_WINDOWS;
}

std::string Application::getVersion()
{
    char verString[256] = { 0 };
    TCHAR szVersionFile[MAX_PATH];
    GetModuleFileName(NULL, szVersionFile, MAX_PATH);
    DWORD  verHandle = NULL;
    UINT   size = 0;
    LPBYTE lpBuffer = NULL;
    DWORD  verSize = GetFileVersionInfoSize(szVersionFile, &verHandle);
    
    if (verSize != NULL)
    {
        LPSTR verData = new char[verSize];
        
        if (GetFileVersionInfo(szVersionFile, verHandle, verSize, verData))
        {
            if (VerQueryValue(verData, L"\\", (VOID FAR* FAR*)&lpBuffer, &size))
            {
                if (size)
                {
                    VS_FIXEDFILEINFO *verInfo = (VS_FIXEDFILEINFO *)lpBuffer;
                    if (verInfo->dwSignature == 0xfeef04bd)
                    {
                        
                        // Doesn't matter if you are on 32 bit or 64 bit,
                        // DWORD is always 32 bits, so first two revision numbers
                        // come from dwFileVersionMS, last two come from dwFileVersionLS
                        sprintf(verString, "%d.%d.%d.%d", (verInfo->dwFileVersionMS >> 16) & 0xffff,
                                (verInfo->dwFileVersionMS >> 0) & 0xffff,
                                (verInfo->dwFileVersionLS >> 16) & 0xffff,
                                (verInfo->dwFileVersionLS >> 0) & 0xffff
                                );
                    }
                }
            }
        }
        delete[] verData;
    }
    return verString;
}

bool Application::openURL(const std::string &url)
{
    WCHAR *temp = new WCHAR[url.size() + 1];
    int wchars_num = MultiByteToWideChar(CP_UTF8, 0, url.c_str(), url.size() + 1, temp, url.size() + 1);
    HINSTANCE r = ShellExecuteW(NULL, L"open", temp, NULL, NULL, SW_SHOWNORMAL);
    delete[] temp;
    return (size_t)r>32;
}

void Application::setResourceRootPath(const std::string& rootResDir)
{
    _resourceRootPath = rootResDir;
    std::replace(_resourceRootPath.begin(), _resourceRootPath.end(), '\\', '/');
    if (_resourceRootPath[_resourceRootPath.length() - 1] != '/')
    {
        _resourceRootPath += '/';
    }
    FileUtils* pFileUtils = FileUtils::getInstance();
    std::vector<std::string> searchPaths = pFileUtils->getSearchPaths();
    searchPaths.insert(searchPaths.begin(), _resourceRootPath);
    pFileUtils->setSearchPaths(searchPaths);
}

const std::string& Application::getResourceRootPath(void)
{
    return _resourceRootPath;
}

void Application::setStartupScriptFilename(const std::string& startupScriptFile)
{
    _startupScriptFilename = startupScriptFile;
    std::replace(_startupScriptFilename.begin(), _startupScriptFilename.end(), '\\', '/');
}

#ifdef USE_AGTK_BLOCK_PROCESS_DURING_WINDOW_RESIZING // 2017.12.6 tada
// Hook GetMessage()
LRESULT CALLBACK Application::GetMsgProc(int code, WPARAM wParam, LPARAM lParam)
{
	if (code == HC_ACTION)
	{
		LPMSG lpMsg = (LPMSG)lParam;
		auto msg = lpMsg->message;
		auto director = Director::getInstance();

		switch (msg)	
		{
		case WM_ENTERSIZEMOVE:
		case WM_SIZE:
		{
			if (!director->isPaused()) director->pause();
			break;
		}
		case WM_NCLBUTTONDOWN:
		{
			auto prm = lpMsg->wParam;
			if (prm == HTCAPTION || prm == HTMINBUTTON || prm == HTMAXBUTTON || (prm >= HTSIZEFIRST && prm <= HTSIZELAST)) {
				if (!director->isPaused()) director->pause();
			}
			break;
		}

		case WM_EXITSIZEMOVE:
		case WM_NCLBUTTONUP:
		case WM_LBUTTONUP:
		{
			if (director->isPaused()) director->resume();
			break;
		}
#ifdef USE_AGTK	//プレビュー時のみ有効。
		case WM_DROPFILES:
		{
			auto hDrop = (HDROP)lpMsg->wParam;
			TCHAR wbuf[MAX_PATH];
			//char szFileName[256];
			auto count = DragQueryFile(hDrop, 0xFFFFFFFF, NULL, 0);
			if (count > 0) {
				DragQueryFile(hDrop, 0, wbuf, sizeof(wbuf) / sizeof(*wbuf));
				std::u16string u16str((char16_t *)wbuf);
				std::string u8str;
				cocos2d::StringUtils::UTF16ToUTF8(u16str, u8str);
				CCLOG("Dropped: %s", u8str.c_str());
				if (_dropFileCallback != nullptr) {
					_dropFileCallback(u8str.c_str());
				}
			}
			DragFinish(hDrop);
			break;
		}
#endif
		}

		//CCLOG("-- msg: 0x%04X, prm: 0x%04X --", msg, lpMsg->wParam);
	}
	return ::CallNextHookEx(sm_pSharedApplication->_getMsgHook, code, wParam, lParam);
}
#endif
NS_CC_END

//////////////////////////////////////////////////////////////////////////
// Local function
//////////////////////////////////////////////////////////////////////////
static void PVRFrameEnableControlWindow(bool bEnable)
{
    HKEY hKey = 0;

    // Open PVRFrame control key, if not exist create it.
    if(ERROR_SUCCESS != RegCreateKeyExW(HKEY_CURRENT_USER,
        L"Software\\Imagination Technologies\\PVRVFRame\\STARTUP\\",
        0,
        0,
        REG_OPTION_NON_VOLATILE,
        KEY_ALL_ACCESS,
        0,
        &hKey,
        nullptr))
    {
        return;
    }

    const WCHAR* wszValue = L"hide_gui";
    const WCHAR* wszNewData = (bEnable) ? L"NO" : L"YES";
    WCHAR wszOldData[256] = {0};
    DWORD   dwSize = sizeof(wszOldData);
    LSTATUS status = RegQueryValueExW(hKey, wszValue, 0, nullptr, (LPBYTE)wszOldData, &dwSize);
    if (ERROR_FILE_NOT_FOUND == status              // the key not exist
        || (ERROR_SUCCESS == status                 // or the hide_gui value is exist
        && 0 != wcscmp(wszNewData, wszOldData)))    // but new data and old data not equal
    {
        dwSize = sizeof(WCHAR) * (wcslen(wszNewData) + 1);
        RegSetValueEx(hKey, wszValue, 0, REG_SZ, (const BYTE *)wszNewData, dwSize);
    }

    RegCloseKey(hKey);
}

#endif // CC_TARGET_PLATFORM == CC_PLATFORM_WIN32
