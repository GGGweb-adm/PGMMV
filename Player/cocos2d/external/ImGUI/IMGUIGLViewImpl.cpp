/****************************************************************************
Copyright (c) 2010-2012 cocos2d-x.org
Copyright (c) 2013-2014 Chukong Technologies Inc.

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

#include "IMGUIGLViewImpl.h"

#include <cmath>
#include <unordered_map>

#include "platform/CCApplication.h"
#include "base/CCDirector.h"
#include "base/CCTouch.h"
#include "base/CCEventDispatcher.h"
#include "base/CCEventKeyboard.h"
#include "base/CCEventMouse.h"
#include "base/CCEventController.h"
#include "base/CCIMEDispatcher.h"
#include "base/ccUtils.h"
#include "base/ccUTF8.h"
#include "2d/CCCamera.h"

#include "imgui/imgui.h"
#include "imgui/imgui_impl_glfw.h"

#ifdef USE_AGTK
#include "json/stringbuffer.h"
#include "json/prettywriter.h"
#include "json/document.h"
#endif

// #AGTK-NX
#if (CC_TARGET_PLATFORM == CC_PLATFORM_NX)
#endif

#ifdef USE_AGTK
static bool sUpdateSwapInterval = false;
static int sSwapIntervalValue = 1;
#endif
NS_CC_BEGIN

#if (CC_TARGET_PLATFORM == CC_PLATFORM_NX) // #AGTK-NX
#endif

// GLFWEventHandler

class GLFWEventHandler
{
public:
    static void onGLFWError(int errorID, const char* errorDesc)
    {
        if (_view)
            _view->onGLFWError(errorID, errorDesc);
    }

    static void onGLFWMouseCallBack(GLFWwindow* window, int button, int action, int modify)
    {
        if (_view)
        {
            _view->onGLFWMouseCallBack(window, button, action, modify);
            ImGui_ImplGlfw_MouseButtonCallback(window, button, action, modify);
        }
    }

    static void onGLFWMouseMoveCallBack(GLFWwindow* window, double x, double y)
    {
        if (_view)
            _view->onGLFWMouseMoveCallBack(window, x, y);
    }

    static void onGLFWMouseScrollCallback(GLFWwindow* window, double x, double y)
    {
        if (_view)
        {
            _view->onGLFWMouseScrollCallback(window, x, y);
            ImGui_ImplGlfw_ScrollCallback(window, x, y);
        }
    }

    static void onGLFWKeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
    {
        if (_view)
        {
            _view->onGLFWKeyCallback(window, key, scancode, action, mods);
            ImGui_ImplGlFw_KeyCallback(window, key, scancode, action, mods);
        }
    }

    static void onGLFWCharCallback(GLFWwindow* window, unsigned int character)
    {
        if (_view)
        {
            _view->onGLFWCharCallback(window, character);
            ImGui_ImplGlfw_CharCallback(window, character);
        }
    }

    static void onGLFWWindowPosCallback(GLFWwindow* windows, int x, int y)
    {
        if (_view)
            _view->onGLFWWindowPosCallback(windows, x, y);
    }

    static void onGLFWframebuffersize(GLFWwindow* window, int w, int h)
    {
        if (_view)
            _view->onGLFWframebuffersize(window, w, h);
    }

    static void onGLFWWindowSizeFunCallback(GLFWwindow *window, int width, int height)
    {
        if (_view)
            _view->onGLFWWindowSizeFunCallback(window, width, height);
    }

    static void setGLViewImpl(IMGUIGLViewImpl* view)
    {
        _view = view;
    }

    static void onGLFWWindowIconifyCallback(GLFWwindow* window, int iconified)
    {
        if (_view)
        {
            _view->onGLFWWindowIconifyCallback(window, iconified);
        }
    }

    static void onWindowResizeCallback(GLFWwindow* window, int w, int h)
    {
        if (_view)
        {
            _view->setFrameSize(w, h);
            _view->setDesignResolutionSize(w, h, ResolutionPolicy::SHOW_ALL);
        }
    }

#if (CC_TARGET_PLATFORM == CC_PLATFORM_NX) // #AGTK-NX
#endif

private:
    static IMGUIGLViewImpl* _view;
};

IMGUIGLViewImpl* GLFWEventHandler::_view = nullptr;

////////////////////////////////////////////////////

struct keyCodeItem
{
    int glfwKeyCode;
    EventKeyboard::KeyCode keyCode;
};

static std::unordered_map<int, EventKeyboard::KeyCode> g_keyCodeMap;
#ifdef USE_AGTK//sakihama-h, 2018.10.11
static std::unordered_map<int, EventKeyboard::KeyCode> g_charCodeMap;
#endif

static keyCodeItem g_keyCodeStructArray[] = {
    /* The unknown key */
    { GLFW_KEY_UNKNOWN         , EventKeyboard::KeyCode::KEY_NONE          },

    /* Printable keys */
    { GLFW_KEY_SPACE           , EventKeyboard::KeyCode::KEY_SPACE         },
    { GLFW_KEY_APOSTROPHE      , EventKeyboard::KeyCode::KEY_APOSTROPHE    },
    { GLFW_KEY_COMMA           , EventKeyboard::KeyCode::KEY_COMMA         },
    { GLFW_KEY_MINUS           , EventKeyboard::KeyCode::KEY_MINUS         },
    { GLFW_KEY_PERIOD          , EventKeyboard::KeyCode::KEY_PERIOD        },
    { GLFW_KEY_SLASH           , EventKeyboard::KeyCode::KEY_SLASH         },
    { GLFW_KEY_0               , EventKeyboard::KeyCode::KEY_0             },
    { GLFW_KEY_1               , EventKeyboard::KeyCode::KEY_1             },
    { GLFW_KEY_2               , EventKeyboard::KeyCode::KEY_2             },
    { GLFW_KEY_3               , EventKeyboard::KeyCode::KEY_3             },
    { GLFW_KEY_4               , EventKeyboard::KeyCode::KEY_4             },
    { GLFW_KEY_5               , EventKeyboard::KeyCode::KEY_5             },
    { GLFW_KEY_6               , EventKeyboard::KeyCode::KEY_6             },
    { GLFW_KEY_7               , EventKeyboard::KeyCode::KEY_7             },
    { GLFW_KEY_8               , EventKeyboard::KeyCode::KEY_8             },
    { GLFW_KEY_9               , EventKeyboard::KeyCode::KEY_9             },
    { GLFW_KEY_SEMICOLON       , EventKeyboard::KeyCode::KEY_SEMICOLON     },
    { GLFW_KEY_EQUAL           , EventKeyboard::KeyCode::KEY_EQUAL         },
    { GLFW_KEY_A               , EventKeyboard::KeyCode::KEY_A             },
    { GLFW_KEY_B               , EventKeyboard::KeyCode::KEY_B             },
    { GLFW_KEY_C               , EventKeyboard::KeyCode::KEY_C             },
    { GLFW_KEY_D               , EventKeyboard::KeyCode::KEY_D             },
    { GLFW_KEY_E               , EventKeyboard::KeyCode::KEY_E             },
    { GLFW_KEY_F               , EventKeyboard::KeyCode::KEY_F             },
    { GLFW_KEY_G               , EventKeyboard::KeyCode::KEY_G             },
    { GLFW_KEY_H               , EventKeyboard::KeyCode::KEY_H             },
    { GLFW_KEY_I               , EventKeyboard::KeyCode::KEY_I             },
    { GLFW_KEY_J               , EventKeyboard::KeyCode::KEY_J             },
    { GLFW_KEY_K               , EventKeyboard::KeyCode::KEY_K             },
    { GLFW_KEY_L               , EventKeyboard::KeyCode::KEY_L             },
    { GLFW_KEY_M               , EventKeyboard::KeyCode::KEY_M             },
    { GLFW_KEY_N               , EventKeyboard::KeyCode::KEY_N             },
    { GLFW_KEY_O               , EventKeyboard::KeyCode::KEY_O             },
    { GLFW_KEY_P               , EventKeyboard::KeyCode::KEY_P             },
    { GLFW_KEY_Q               , EventKeyboard::KeyCode::KEY_Q             },
    { GLFW_KEY_R               , EventKeyboard::KeyCode::KEY_R             },
    { GLFW_KEY_S               , EventKeyboard::KeyCode::KEY_S             },
    { GLFW_KEY_T               , EventKeyboard::KeyCode::KEY_T             },
    { GLFW_KEY_U               , EventKeyboard::KeyCode::KEY_U             },
    { GLFW_KEY_V               , EventKeyboard::KeyCode::KEY_V             },
    { GLFW_KEY_W               , EventKeyboard::KeyCode::KEY_W             },
    { GLFW_KEY_X               , EventKeyboard::KeyCode::KEY_X             },
    { GLFW_KEY_Y               , EventKeyboard::KeyCode::KEY_Y             },
    { GLFW_KEY_Z               , EventKeyboard::KeyCode::KEY_Z             },
    { GLFW_KEY_LEFT_BRACKET    , EventKeyboard::KeyCode::KEY_LEFT_BRACKET  },
    { GLFW_KEY_BACKSLASH       , EventKeyboard::KeyCode::KEY_BACK_SLASH    },
    { GLFW_KEY_RIGHT_BRACKET   , EventKeyboard::KeyCode::KEY_RIGHT_BRACKET },
    { GLFW_KEY_GRAVE_ACCENT    , EventKeyboard::KeyCode::KEY_GRAVE         },
    { GLFW_KEY_WORLD_1         , EventKeyboard::KeyCode::KEY_GRAVE         },
    { GLFW_KEY_WORLD_2         , EventKeyboard::KeyCode::KEY_NONE          },

    /* Function keys */
    { GLFW_KEY_ESCAPE          , EventKeyboard::KeyCode::KEY_ESCAPE        },
    { GLFW_KEY_ENTER           , EventKeyboard::KeyCode::KEY_ENTER      },
    { GLFW_KEY_TAB             , EventKeyboard::KeyCode::KEY_TAB           },
    { GLFW_KEY_BACKSPACE       , EventKeyboard::KeyCode::KEY_BACKSPACE     },
    { GLFW_KEY_INSERT          , EventKeyboard::KeyCode::KEY_INSERT        },
    { GLFW_KEY_DELETE          , EventKeyboard::KeyCode::KEY_DELETE        },
    { GLFW_KEY_RIGHT           , EventKeyboard::KeyCode::KEY_RIGHT_ARROW   },
    { GLFW_KEY_LEFT            , EventKeyboard::KeyCode::KEY_LEFT_ARROW    },
    { GLFW_KEY_DOWN            , EventKeyboard::KeyCode::KEY_DOWN_ARROW    },
    { GLFW_KEY_UP              , EventKeyboard::KeyCode::KEY_UP_ARROW      },
    { GLFW_KEY_PAGE_UP         , EventKeyboard::KeyCode::KEY_PG_UP      },
    { GLFW_KEY_PAGE_DOWN       , EventKeyboard::KeyCode::KEY_PG_DOWN    },
    { GLFW_KEY_HOME            , EventKeyboard::KeyCode::KEY_HOME       },
    { GLFW_KEY_END             , EventKeyboard::KeyCode::KEY_END           },
    { GLFW_KEY_CAPS_LOCK       , EventKeyboard::KeyCode::KEY_CAPS_LOCK     },
    { GLFW_KEY_SCROLL_LOCK     , EventKeyboard::KeyCode::KEY_SCROLL_LOCK   },
    { GLFW_KEY_NUM_LOCK        , EventKeyboard::KeyCode::KEY_NUM_LOCK      },
    { GLFW_KEY_PRINT_SCREEN    , EventKeyboard::KeyCode::KEY_PRINT         },
    { GLFW_KEY_PAUSE           , EventKeyboard::KeyCode::KEY_PAUSE         },
    { GLFW_KEY_F1              , EventKeyboard::KeyCode::KEY_F1            },
    { GLFW_KEY_F2              , EventKeyboard::KeyCode::KEY_F2            },
    { GLFW_KEY_F3              , EventKeyboard::KeyCode::KEY_F3            },
    { GLFW_KEY_F4              , EventKeyboard::KeyCode::KEY_F4            },
    { GLFW_KEY_F5              , EventKeyboard::KeyCode::KEY_F5            },
    { GLFW_KEY_F6              , EventKeyboard::KeyCode::KEY_F6            },
    { GLFW_KEY_F7              , EventKeyboard::KeyCode::KEY_F7            },
    { GLFW_KEY_F8              , EventKeyboard::KeyCode::KEY_F8            },
    { GLFW_KEY_F9              , EventKeyboard::KeyCode::KEY_F9            },
    { GLFW_KEY_F10             , EventKeyboard::KeyCode::KEY_F10           },
    { GLFW_KEY_F11             , EventKeyboard::KeyCode::KEY_F11           },
    { GLFW_KEY_F12             , EventKeyboard::KeyCode::KEY_F12           },
    { GLFW_KEY_F13             , EventKeyboard::KeyCode::KEY_NONE          },
    { GLFW_KEY_F14             , EventKeyboard::KeyCode::KEY_NONE          },
    { GLFW_KEY_F15             , EventKeyboard::KeyCode::KEY_NONE          },
    { GLFW_KEY_F16             , EventKeyboard::KeyCode::KEY_NONE          },
    { GLFW_KEY_F17             , EventKeyboard::KeyCode::KEY_NONE          },
    { GLFW_KEY_F18             , EventKeyboard::KeyCode::KEY_NONE          },
    { GLFW_KEY_F19             , EventKeyboard::KeyCode::KEY_NONE          },
    { GLFW_KEY_F20             , EventKeyboard::KeyCode::KEY_NONE          },
    { GLFW_KEY_F21             , EventKeyboard::KeyCode::KEY_NONE          },
    { GLFW_KEY_F22             , EventKeyboard::KeyCode::KEY_NONE          },
    { GLFW_KEY_F23             , EventKeyboard::KeyCode::KEY_NONE          },
    { GLFW_KEY_F24             , EventKeyboard::KeyCode::KEY_NONE          },
    { GLFW_KEY_F25             , EventKeyboard::KeyCode::KEY_NONE          },
    { GLFW_KEY_KP_0            , EventKeyboard::KeyCode::KEY_0             },
    { GLFW_KEY_KP_1            , EventKeyboard::KeyCode::KEY_1             },
    { GLFW_KEY_KP_2            , EventKeyboard::KeyCode::KEY_2             },
    { GLFW_KEY_KP_3            , EventKeyboard::KeyCode::KEY_3             },
    { GLFW_KEY_KP_4            , EventKeyboard::KeyCode::KEY_4             },
    { GLFW_KEY_KP_5            , EventKeyboard::KeyCode::KEY_5             },
    { GLFW_KEY_KP_6            , EventKeyboard::KeyCode::KEY_6             },
    { GLFW_KEY_KP_7            , EventKeyboard::KeyCode::KEY_7             },
    { GLFW_KEY_KP_8            , EventKeyboard::KeyCode::KEY_8             },
    { GLFW_KEY_KP_9            , EventKeyboard::KeyCode::KEY_9             },
    { GLFW_KEY_KP_DECIMAL      , EventKeyboard::KeyCode::KEY_PERIOD        },
    { GLFW_KEY_KP_DIVIDE       , EventKeyboard::KeyCode::KEY_KP_DIVIDE     },
    { GLFW_KEY_KP_MULTIPLY     , EventKeyboard::KeyCode::KEY_KP_MULTIPLY   },
    { GLFW_KEY_KP_SUBTRACT     , EventKeyboard::KeyCode::KEY_KP_MINUS      },
    { GLFW_KEY_KP_ADD          , EventKeyboard::KeyCode::KEY_KP_PLUS       },
    { GLFW_KEY_KP_ENTER        , EventKeyboard::KeyCode::KEY_KP_ENTER      },
    { GLFW_KEY_KP_EQUAL        , EventKeyboard::KeyCode::KEY_EQUAL         },
    { GLFW_KEY_LEFT_SHIFT      , EventKeyboard::KeyCode::KEY_LEFT_SHIFT         },
    { GLFW_KEY_LEFT_CONTROL    , EventKeyboard::KeyCode::KEY_LEFT_CTRL          },
    { GLFW_KEY_LEFT_ALT        , EventKeyboard::KeyCode::KEY_LEFT_ALT           },
    { GLFW_KEY_LEFT_SUPER      , EventKeyboard::KeyCode::KEY_HYPER         },
    { GLFW_KEY_RIGHT_SHIFT     , EventKeyboard::KeyCode::KEY_RIGHT_SHIFT         },
    { GLFW_KEY_RIGHT_CONTROL   , EventKeyboard::KeyCode::KEY_RIGHT_CTRL          },
    { GLFW_KEY_RIGHT_ALT       , EventKeyboard::KeyCode::KEY_RIGHT_ALT           },
    { GLFW_KEY_RIGHT_SUPER     , EventKeyboard::KeyCode::KEY_HYPER         },
    { GLFW_KEY_MENU            , EventKeyboard::KeyCode::KEY_MENU          },
#ifdef USE_AGTK//sakihama-h, 2018.10.11
#else
    { GLFW_KEY_LAST            , EventKeyboard::KeyCode::KEY_NONE          }
#endif
};

#ifdef USE_AGTK//sakihama-h, 2018.10.11
struct charCodeItem
{
	int charCode;
	EventKeyboard::KeyCode keyCode;
};

static charCodeItem g_charCodeStructArray[] = {
	{ -1         , EventKeyboard::KeyCode::KEY_NONE },
	{ GLFW_KEY_SPACE           , EventKeyboard::KeyCode::KEY_SPACE },
	{ GLFW_KEY_APOSTROPHE      , EventKeyboard::KeyCode::KEY_APOSTROPHE },
	{ 42                       , EventKeyboard::KeyCode::KEY_ASTERISK },
	{ 43                       , EventKeyboard::KeyCode::KEY_PLUS },
	{ GLFW_KEY_COMMA           , EventKeyboard::KeyCode::KEY_COMMA },
	{ GLFW_KEY_MINUS           , EventKeyboard::KeyCode::KEY_MINUS },
	{ GLFW_KEY_PERIOD          , EventKeyboard::KeyCode::KEY_PERIOD },
	{ GLFW_KEY_SLASH           , EventKeyboard::KeyCode::KEY_SLASH },
	{ 48               , EventKeyboard::KeyCode::KEY_0 },
	{ 49               , EventKeyboard::KeyCode::KEY_1 },
	{ 50               , EventKeyboard::KeyCode::KEY_2 },
	{ 51               , EventKeyboard::KeyCode::KEY_3 },
	{ 52               , EventKeyboard::KeyCode::KEY_4 },
	{ 53               , EventKeyboard::KeyCode::KEY_5 },
	{ 54               , EventKeyboard::KeyCode::KEY_6 },
	{ 55               , EventKeyboard::KeyCode::KEY_7 },
	{ 56               , EventKeyboard::KeyCode::KEY_8 },
	{ 57               , EventKeyboard::KeyCode::KEY_9 },
	{ 58               , EventKeyboard::KeyCode::KEY_COLON },
	{ 59               , EventKeyboard::KeyCode::KEY_SEMICOLON },
	{ 64               , EventKeyboard::KeyCode::KEY_AT },
	{ 94               , EventKeyboard::KeyCode::KEY_CIRCUMFLEX },
	{ 97               , EventKeyboard::KeyCode::KEY_A },
	{ 98               , EventKeyboard::KeyCode::KEY_B },
	{ 99               , EventKeyboard::KeyCode::KEY_C },
	{ 100               , EventKeyboard::KeyCode::KEY_D },
	{ 101               , EventKeyboard::KeyCode::KEY_E },
	{ 102               , EventKeyboard::KeyCode::KEY_F },
	{ 103               , EventKeyboard::KeyCode::KEY_G },
	{ 104               , EventKeyboard::KeyCode::KEY_H },
	{ 105               , EventKeyboard::KeyCode::KEY_I },
	{ 106               , EventKeyboard::KeyCode::KEY_J },
	{ 107               , EventKeyboard::KeyCode::KEY_K },
	{ 108               , EventKeyboard::KeyCode::KEY_L },
	{ 109               , EventKeyboard::KeyCode::KEY_M },
	{ 110               , EventKeyboard::KeyCode::KEY_N },
	{ 111               , EventKeyboard::KeyCode::KEY_O },
	{ 112               , EventKeyboard::KeyCode::KEY_P },
	{ 113               , EventKeyboard::KeyCode::KEY_Q },
	{ 114               , EventKeyboard::KeyCode::KEY_R },
	{ 115               , EventKeyboard::KeyCode::KEY_S },
	{ 116               , EventKeyboard::KeyCode::KEY_T },
	{ 117               , EventKeyboard::KeyCode::KEY_U },
	{ 118               , EventKeyboard::KeyCode::KEY_V },
	{ 119               , EventKeyboard::KeyCode::KEY_W },
	{ 120               , EventKeyboard::KeyCode::KEY_X },
	{ 121               , EventKeyboard::KeyCode::KEY_Y },
	{ 122               , EventKeyboard::KeyCode::KEY_Z },
	{ GLFW_KEY_LEFT_BRACKET    , EventKeyboard::KeyCode::KEY_LEFT_BRACKET },
	{ GLFW_KEY_BACKSLASH       , EventKeyboard::KeyCode::KEY_BACK_SLASH },
	{ GLFW_KEY_RIGHT_BRACKET   , EventKeyboard::KeyCode::KEY_RIGHT_BRACKET },
};
#endif

//////////////////////////////////////////////////////////////////////////
// implement IMGUIGLViewImpl
//////////////////////////////////////////////////////////////////////////


IMGUIGLViewImpl::IMGUIGLViewImpl(bool initglfw)
: _captured(false)
, _supportTouch(false)
, _isInRetinaMonitor(false)
, _isRetinaEnabled(false)
, _retinaFactor(1)
, _frameZoomFactor(1.0f)
, _mainWindow(nullptr)
, _monitor(nullptr)
, _mouseX(0.0f)
, _mouseY(0.0f)
{
#ifdef USE_AGTK//sakihama-h, 2017.9.21
	_fullScreen.flag = false;
#endif
#if (CC_TARGET_PLATFORM == CC_PLATFORM_WIN32)
#ifdef USE_AGTK//sakihama-h, 2018.05.08
	_imcReserve = nullptr;
#endif
#endif /* (CC_TARGET_PLATFORM == CC_PLATFORM_WIN32) */
    _viewName = "cocos2dx";
    g_keyCodeMap.clear();
    for (auto& item : g_keyCodeStructArray)
    {
        g_keyCodeMap[item.glfwKeyCode] = item.keyCode;
    }
#ifdef USE_AGTK//sakihama-h, 2018.10.11
	for (auto& item : g_charCodeStructArray)
	{
		g_charCodeMap[item.charCode] = item.keyCode;
	}
#endif
    GLFWEventHandler::setGLViewImpl(this);
    if (initglfw)
    {
        glfwSetErrorCallback(GLFWEventHandler::onGLFWError);
        glfwInit();
    }
}

IMGUIGLViewImpl::~IMGUIGLViewImpl()
{
    CCLOGINFO("deallocing IMGUIGLViewImpl: %p", this);
	GLFWEventHandler::setGLViewImpl(nullptr);
    glfwTerminate();
}

IMGUIGLViewImpl* IMGUIGLViewImpl::create(const std::string& viewName)
{
    return IMGUIGLViewImpl::create(viewName, false);
}

IMGUIGLViewImpl* IMGUIGLViewImpl::create(const std::string& viewName, bool resizable)
{
    auto ret = new (std::nothrow) IMGUIGLViewImpl;
    if(ret && ret->initWithRect(viewName, Rect(0, 0, 960, 640), 1.0f, resizable)) {
        ret->autorelease();
        return ret;
    }
    CC_SAFE_DELETE(ret);
    return nullptr;
}

IMGUIGLViewImpl* IMGUIGLViewImpl::createWithRect(const std::string& viewName, Rect rect, float frameZoomFactor, bool resizable)
{
    auto ret = new (std::nothrow) IMGUIGLViewImpl;
    if(ret && ret->initWithRect(viewName, rect, frameZoomFactor, resizable)) {
        ret->autorelease();
        return ret;
    }
    CC_SAFE_DELETE(ret);
    return nullptr;
}

IMGUIGLViewImpl* IMGUIGLViewImpl::createWithFullScreen(const std::string& viewName)
{
    auto ret = new (std::nothrow) IMGUIGLViewImpl();
    if(ret && ret->initWithFullScreen(viewName)) {
        ret->autorelease();
        return ret;
    }
    CC_SAFE_DELETE(ret);
    return nullptr;
}

IMGUIGLViewImpl* IMGUIGLViewImpl::createWithFullScreen(const std::string& viewName, const GLFWvidmode &videoMode, GLFWmonitor *monitor)
{
    auto ret = new (std::nothrow) IMGUIGLViewImpl();
    if(ret && ret->initWithFullscreen(viewName, videoMode, monitor)) {
        ret->autorelease();
        return ret;
    }
    
    return nullptr;
}

bool IMGUIGLViewImpl::initWithRect(const std::string& viewName, Rect rect, float frameZoomFactor, bool resizable)
{
    setViewName(viewName);

    _frameZoomFactor = frameZoomFactor;

    glfwWindowHint(GLFW_RESIZABLE,resizable?GL_TRUE:GL_FALSE);
    glfwWindowHint(GLFW_RED_BITS,_glContextAttrs.redBits);
    glfwWindowHint(GLFW_GREEN_BITS,_glContextAttrs.greenBits);
    glfwWindowHint(GLFW_BLUE_BITS,_glContextAttrs.blueBits);
    glfwWindowHint(GLFW_ALPHA_BITS,_glContextAttrs.alphaBits);
    glfwWindowHint(GLFW_DEPTH_BITS,_glContextAttrs.depthBits);
    glfwWindowHint(GLFW_STENCIL_BITS,_glContextAttrs.stencilBits);

    int neededWidth = rect.size.width * _frameZoomFactor;
    int neededHeight = rect.size.height * _frameZoomFactor;
#ifdef USE_AGTK	//モニターサイズを超えるウィンドウが作られるのを防止したい。
#if (CC_TARGET_PLATFORM != CC_PLATFORM_NX) // #AGTK-NX ResolutionPolicy::SHOW_ALLにしか対応していないためここでは単に無視
	_frameZoomFactor = adjustWindowSizeAndZoomScale(&neededWidth, &neededHeight);
#else
#endif
#endif

    _mainWindow = glfwCreateWindow(neededWidth, neededHeight, _viewName.c_str(), _monitor, nullptr);

    if (_mainWindow == nullptr)
    {
        std::string message = "Can't create window";
        if (!_glfwError.empty())
        {
            message.append("\nMore info: \n");
            message.append(_glfwError);
        }
// #AGTK-NX
#if (CC_TARGET_PLATFORM == CC_PLATFORM_NX)
#else
        MessageBox(message.c_str(), "Error launch application");
#endif	// (CC_TARGET_PLATFORM == CC_PLATFORM_NX)
        return false;
    }

    /*
    *  Note that the created window and context may differ from what you requested,
    *  as not all parameters and hints are
    *  [hard constraints](@ref window_hints_hard).  This includes the size of the
    *  window, especially for full screen windows.  To retrieve the actual
    *  attributes of the created window and context, use queries like @ref
    *  glfwGetWindowAttrib and @ref glfwGetWindowSize.
    *
    *  see declaration glfwCreateWindow
    */

#ifdef USE_AGTK//sakihama-h, 2019.01.08
#if (CC_TARGET_PLATFORM == CC_PLATFORM_WIN32)
	int xPos = INT_MAX;
	int yPos = INT_MAX;

	//読み込み
	auto iniFilePath = FileUtils::getInstance()->getWritablePath() + std::string("\player.ini");
	auto jsonData = FileUtils::getInstance()->getStringFromFile(iniFilePath);
	rapidjson::Document doc;
	doc.Parse(jsonData.c_str());
	bool error = doc.HasParseError();
	if (error) {
		// JSON化される前の旧iniファイルか確認
		char buf[256] = { 0 };
		DWORD ret = GetPrivateProfileStringA("system", "xpos", "non", buf, sizeof(buf), iniFilePath.c_str());
		if (ret && strcmp(buf, "non") != 0) {
			xPos = atoi(buf);
		}
		ret = GetPrivateProfileStringA("system", "ypos", "non", buf, sizeof(buf), iniFilePath.c_str());
		if (ret && strcmp(buf, "non") != 0) {
			yPos = atoi(buf);
		}
	}
	else {
		xPos = doc["system"]["xpos"].GetInt();
		yPos = doc["system"]["ypos"].GetInt();
	}
	// 座標をセット
	if (xPos != INT_MAX && yPos != INT_MAX) {
		bool bInside = false;
		int count;
		GLFWmonitor **monitorList = glfwGetMonitors(&count);
		for (int i = 0; i < count; i++) {
			auto m = monitorList[i];
			int mx = 0;
			int my = 0;
			glfwGetMonitorPos(m, &mx, &my);
			const GLFWvidmode *videoMode = glfwGetVideoMode(m);
			if ((mx <= xPos && xPos < mx + videoMode->width) && (my <= yPos && yPos < my + videoMode->height)) {
				bInside = true;
				break;
			}
		}
		if (bInside) {
			glfwSetWindowPos(_mainWindow, xPos, yPos);
		}
	}
	else {
		glfwGetWindowPos(_mainWindow, &xPos, &yPos);
	}
#endif
#else
	int realW = 0, realH = 0;
	glfwGetWindowSize(_mainWindow, &realW, &realH);
	if (realW != neededWidth)
	{
		rect.size.width = realW / _frameZoomFactor;
	}
	if (realH != neededHeight)
	{
		rect.size.height = realH / _frameZoomFactor;
	}
#endif

    glfwMakeContextCurrent(_mainWindow);

// #AGTK-NX
#if (CC_TARGET_PLATFORM == CC_PLATFORM_NX)
#endif

    glfwSetMouseButtonCallback(_mainWindow, GLFWEventHandler::onGLFWMouseCallBack);
    glfwSetCursorPosCallback(_mainWindow, GLFWEventHandler::onGLFWMouseMoveCallBack);
    glfwSetScrollCallback(_mainWindow, GLFWEventHandler::onGLFWMouseScrollCallback);
    glfwSetCharCallback(_mainWindow, GLFWEventHandler::onGLFWCharCallback);
    glfwSetKeyCallback(_mainWindow, GLFWEventHandler::onGLFWKeyCallback);
    glfwSetWindowPosCallback(_mainWindow, GLFWEventHandler::onGLFWWindowPosCallback);
    glfwSetFramebufferSizeCallback(_mainWindow, GLFWEventHandler::onGLFWframebuffersize);
    glfwSetWindowSizeCallback(_mainWindow, GLFWEventHandler::onGLFWWindowSizeFunCallback);
    glfwSetWindowIconifyCallback(_mainWindow, GLFWEventHandler::onGLFWWindowIconifyCallback);
#if (CC_TARGET_PLATFORM == CC_PLATFORM_NX) // #AGTK-NX
#endif

#ifdef USE_AGTK	//モニターサイズを超えるウィンドウが作られるのを防止したい。
#if (CC_TARGET_PLATFORM != CC_PLATFORM_NX) // #AGTK-NX
	setFrameSize(neededWidth / _frameZoomFactor, neededHeight / _frameZoomFactor);
#else
#endif
#else
    setFrameSize(rect.size.width, rect.size.height);
#endif

    // check OpenGL version at first
    const GLubyte* glVersion = glGetString(GL_VERSION);

    if ( utils::atof((const char*)glVersion) < 1.5 )
    {
        char strComplain[256] = {0};
        sprintf(strComplain,
                "OpenGL 1.5 or higher is required (your version is %s). Please upgrade the driver of your video card.",
                glVersion);
// #AGTK-NX
#if (CC_TARGET_PLATFORM == CC_PLATFORM_NX)
#else
        MessageBox(strComplain, "OpenGL version too old");
#endif// (CC_TARGET_PLATFORM == CC_PLATFORM_NX)
        return false;
    }

    initGlew();

    // Enable point size by default.
    glEnable(GL_VERTEX_PROGRAM_POINT_SIZE);

    // imgui
    ImGui_ImplGlfw_Init(_mainWindow, false);
    
#if (CC_TARGET_PLATFORM == CC_PLATFORM_WIN32)
#ifdef USE_AGTK//sakihama-h, 2018.05.08
	//日本語入力のフォーカスを外す。
	this->onFocusOutIme();
#endif
#endif /* (CC_TARGET_PLATFORM == CC_PLATFORM_WIN32) */

#ifdef USE_AGTK//sakihama-h, 2018.07.24 ウインドウ背景を黒にする。
	glClearColor(0, 0, 0, 0);

// #AGTK-NX
#if (CC_TARGET_PLATFORM == CC_PLATFORM_NX)
#endif
	this->swapBuffers();
#endif
	return true;
}

bool IMGUIGLViewImpl::initWithFullScreen(const std::string& viewName)
{
    //Create fullscreen window on primary monitor at its current video mode.
#ifdef USE_AGTK
	_monitor = this->getCurrentMonitor();
#else
    _monitor = glfwGetPrimaryMonitor();
#endif
    if (nullptr == _monitor)
        return false;

    const GLFWvidmode* videoMode = glfwGetVideoMode(_monitor);
    return initWithRect(viewName, Rect(0, 0, videoMode->width, videoMode->height), 1.0f, false);
}

bool IMGUIGLViewImpl::initWithFullscreen(const std::string &viewname, const GLFWvidmode &videoMode, GLFWmonitor *monitor)
{
    //Create fullscreen on specified monitor at the specified video mode.
    _monitor = monitor;
    if (nullptr == _monitor)
        return false;
    
    //These are soft constraints. If the video mode is retrieved at runtime, the resulting window and context should match these exactly. If invalid attribs are passed (eg. from an outdated cache), window creation will NOT fail but the actual window/context may differ.
    glfwWindowHint(GLFW_REFRESH_RATE, videoMode.refreshRate);
    glfwWindowHint(GLFW_RED_BITS, videoMode.redBits);
    glfwWindowHint(GLFW_BLUE_BITS, videoMode.blueBits);
    glfwWindowHint(GLFW_GREEN_BITS, videoMode.greenBits);
    
    return initWithRect(viewname, Rect(0, 0, videoMode.width, videoMode.height), 1.0f, false);
}

bool IMGUIGLViewImpl::isOpenGLReady()
{
    return nullptr != _mainWindow;
}

void IMGUIGLViewImpl::end()
{
    if(_mainWindow)
    {

#ifdef USE_AGTK//sakihama-h, 2019.01.08
#if (CC_TARGET_PLATFORM == CC_PLATFORM_WIN32)
		// ウィンドウの座標を取得
		int xPos, yPos;
		glfwGetWindowPos(_mainWindow, &xPos, &yPos);

		auto iniFilePath = FileUtils::getInstance()->getWritablePath() + std::string("\player.ini");
		auto jsonData = FileUtils::getInstance()->getStringFromFile(iniFilePath);
		rapidjson::Document doc;
		rapidjson::Document::AllocatorType& allocator = doc.GetAllocator();
		doc.Parse(jsonData.c_str());
		bool error = doc.HasParseError();
		if (error) {
			//CCASSERT(0, "Error: Json Parse.");
			doc.SetObject();
		}
		else {
			if (doc.HasMember("system")) {
				// 一度削除してから設定を保存
				doc.RemoveMember("system");
			}
		}
		rapidjson::Value saveSystem(rapidjson::kObjectType);
		saveSystem.AddMember("xpos", xPos, allocator);
		saveSystem.AddMember("ypos", yPos, allocator);
		doc.AddMember("system", saveSystem, allocator);
		// 書き込み
		rapidjson::StringBuffer buffer;
		rapidjson::PrettyWriter<rapidjson::StringBuffer> writer(buffer);
		doc.Accept(writer);
		FileUtils::getInstance()->writeStringToFile(buffer.GetString(), iniFilePath);
#endif
#endif

        glfwSetWindowShouldClose(_mainWindow,1);
        _mainWindow = nullptr;
    }
    // Release self. Otherwise, IMGUIGLViewImpl could not be freed.
    release();
}

void IMGUIGLViewImpl::swapBuffers()
{
    if(_mainWindow)
        glfwSwapBuffers(_mainWindow);

#if (CC_TARGET_PLATFORM == CC_PLATFORM_NX) // #AGTK-NX
#endif
#ifdef USE_AGTK
	if (sUpdateSwapInterval) {
		sUpdateSwapInterval = false;
		glfwSwapInterval(sSwapIntervalValue);
	}
#endif
}

#ifdef USE_AGTK
void IMGUIGLViewImpl::setSwapInterval(int interval)
{
	sSwapIntervalValue = interval;
	sUpdateSwapInterval = true;
}

int IMGUIGLViewImpl::getSSwapIntervalValue() const
{
	return sSwapIntervalValue;
}
#endif

bool IMGUIGLViewImpl::windowShouldClose()
{
    if(_mainWindow)
        return glfwWindowShouldClose(_mainWindow) ? true : false;
    else
        return true;
}

#if (CC_TARGET_PLATFORM == CC_PLATFORM_WIN32)
extern "C" {
    void win32_cocos2dx_lib_GameControllerAdapter_Update();
};
#endif

// #AGTK-NX
#if (CC_TARGET_PLATFORM == CC_PLATFORM_NX)
#endif

void IMGUIGLViewImpl::pollEvents()
{
// #AGTK-NX
#if (CC_TARGET_PLATFORM == CC_PLATFORM_NX)
#endif
    glfwPollEvents();
#if (CC_TARGET_PLATFORM == CC_PLATFORM_WIN32)
	if(_mainWindow)
		win32_cocos2dx_lib_GameControllerAdapter_Update();
#endif

// #AGTK-NX
#if (CC_TARGET_PLATFORM == CC_PLATFORM_NX)
#endif
}

void IMGUIGLViewImpl::enableRetina(bool enabled)
{
#if (CC_TARGET_PLATFORM == CC_PLATFORM_MAC)
    _isRetinaEnabled = enabled;
    if (_isRetinaEnabled)
    {
        _retinaFactor = 1;
    }
    else
    {
        _retinaFactor = 2;
    }
    updateFrameSize();
#endif
}

#ifdef USE_AGTK//sakihama-h, 2016.11.09
bool IMGUIGLViewImpl::setFullScreen(bool fullScreen, int width, int height)
{
	bool bFullScreen = _fullScreen.flag;
	if (fullScreen == _fullScreen.flag)
	{
		return bFullScreen;
	}

	auto window = this->getWindow();
	auto monitor = this->getCurrentMonitor();

	if (fullScreen)
	{
		const GLFWvidmode* videoMode = glfwGetVideoMode(monitor);
		_fullScreen.refreshRate = videoMode->refreshRate;
		glfwGetWindowPos(window, &_fullScreen.xpos, &_fullScreen.ypos);
		glfwGetWindowSize(window, &_fullScreen.width, &_fullScreen.height);
		glfwSetWindowMonitor(window, monitor, 0, 0, width, height, _fullScreen.refreshRate);
	}
	else
	{
		glfwSetWindowMonitor(window, nullptr, _fullScreen.xpos, _fullScreen.ypos, _fullScreen.width, _fullScreen.height, _fullScreen.refreshRate);
	}
	_fullScreen.flag = fullScreen;
	return bFullScreen;
}
#endif

#ifdef USE_AGTK
// RestoreScreen if iconified.
void IMGUIGLViewImpl::restoreScreen()
{
	auto window = this->getWindow();
	auto iconified = glfwGetWindowAttrib(window, GLFW_ICONIFIED);
	if (iconified) {
		glfwRestoreWindow(window);
	}
}

void IMGUIGLViewImpl::focusWindow()
{
	auto window = this->getWindow();
	glfwFocusWindow(window);
}
#endif

#ifdef USE_AGTK//sakihama-h, 2018.08.01
GLFWmonitor *IMGUIGLViewImpl::getCurrentMonitor()
{
	GLFWwindow *window = this->getWindow();
	GLFWmonitor *monitor = glfwGetWindowMonitor(window);
	if (monitor == nullptr) {
		int xposWin = 0;
		int yposWin = 0;
		glfwGetWindowPos(window, &xposWin, &yposWin);

		int count;
		GLFWmonitor **monitorList = glfwGetMonitors(&count);
		for (int i = 0; i < count; i++) {
			auto m = monitorList[i];
			int xpos = 0;
			int ypos = 0;
			glfwGetMonitorPos(m, &xpos, &ypos);
			const GLFWvidmode *videoMode = glfwGetVideoMode(m);
			if ((xpos <= xposWin && xposWin < xpos + videoMode->width) && (ypos <= yposWin && yposWin < ypos + videoMode->height)) {
				monitor = m;
				break;
			}
		}
		if (monitor == nullptr) {
			monitor = glfwGetPrimaryMonitor();
		}
	}
	return monitor;
}
#endif

void IMGUIGLViewImpl::setIMEKeyboardState(bool /*bOpen*/)
{

}

void IMGUIGLViewImpl::setCursorVisible( bool isVisible )
{
    if( _mainWindow == NULL )
        return;
    
    if( isVisible )
        glfwSetInputMode(_mainWindow, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
    else
        glfwSetInputMode(_mainWindow, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);
}

void IMGUIGLViewImpl::setFrameZoomFactor(float zoomFactor)
{
    CCASSERT(zoomFactor > 0.0f, "zoomFactor must be larger than 0");

    if (std::abs(_frameZoomFactor - zoomFactor) < FLT_EPSILON)
    {
        return;
    }

#if defined(USE_AGTK) && (CC_TARGET_PLATFORM != CC_PLATFORM_NX) // #AGTK-NX
	int newWidth = _screenSize.width * zoomFactor;
	int newHeight = _screenSize.height * zoomFactor;
	_frameZoomFactor = adjustWindowSizeAndZoomScale(&newWidth, &newHeight);
#else
    _frameZoomFactor = zoomFactor;
#endif
    updateFrameSize();
}

float IMGUIGLViewImpl::getFrameZoomFactor() const
{
    return _frameZoomFactor;
}

void IMGUIGLViewImpl::updateFrameSize()
{
    if (_screenSize.width > 0 && _screenSize.height > 0)
    {
        int w = 0, h = 0;
        glfwGetWindowSize(_mainWindow, &w, &h);

        int frameBufferW = 0, frameBufferH = 0;
        glfwGetFramebufferSize(_mainWindow, &frameBufferW, &frameBufferH);

        if (frameBufferW == 2 * w && frameBufferH == 2 * h)
        {
            if (_isRetinaEnabled)
            {
                _retinaFactor = 1;
            }
            else
            {
                _retinaFactor = 2;
            }
            glfwSetWindowSize(_mainWindow, _screenSize.width/2 * _retinaFactor * _frameZoomFactor, _screenSize.height/2 * _retinaFactor * _frameZoomFactor);

            _isInRetinaMonitor = true;
        }
        else
        {
            if (_isInRetinaMonitor)
            {
                _retinaFactor = 1;
            }
#ifdef USE_AGTK
#if (CC_TARGET_PLATFORM == CC_PLATFORM_NX) // #AGTK-NX
#else
			auto ffw = _screenSize.width * _retinaFactor * _frameZoomFactor;
			auto ffh = _screenSize.height *_retinaFactor * _frameZoomFactor;
#endif
			auto fw = (int)std::roundf(ffw);
			auto fh = (int)std::roundf(ffh);
			glfwSetWindowSize(_mainWindow, fw, fh);
#else
            glfwSetWindowSize(_mainWindow, _screenSize.width * _retinaFactor * _frameZoomFactor, _screenSize.height *_retinaFactor * _frameZoomFactor);
#endif

            _isInRetinaMonitor = false;
        }
    }
}

void IMGUIGLViewImpl::setFrameSize(float width, float height)
{
#if defined(USE_AGTK) && (CC_TARGET_PLATFORM != CC_PLATFORM_NX) // #AGTK-NX
	int newWidth = width * _frameZoomFactor;
	int newHeight = height * _frameZoomFactor;
	adjustWindowSizeAndZoomScale(&newWidth, &newHeight);
	width = newWidth / _frameZoomFactor;
	height = newHeight / _frameZoomFactor;
#endif
    GLView::setFrameSize(width, height);
    updateFrameSize();
}

#ifdef USE_AGTK
void IMGUIGLViewImpl::setFrameSizeAndZoomFactor(float width, float height, float zoomFactor)
{
	_frameZoomFactor = zoomFactor;
#if (CC_TARGET_PLATFORM != CC_PLATFORM_NX) // #AGTK-NX
	int newWidth = width * _frameZoomFactor;
	int newHeight = height * _frameZoomFactor;
	adjustWindowSizeAndZoomScale(&newWidth, &newHeight);
	width = newWidth / _frameZoomFactor;
	height = newHeight / _frameZoomFactor;
#endif
	GLView::setFrameSize(width, height);
	updateFrameSize();
}
#endif

void IMGUIGLViewImpl::setViewPortInPoints(float x , float y , float w , float h)
{
#if (CC_TARGET_PLATFORM != CC_PLATFORM_NX) // #AGTK-NX
    experimental::Viewport vp((float)(x * _scaleX * _retinaFactor * _frameZoomFactor + _viewPortRect.origin.x * _retinaFactor * _frameZoomFactor),
        (float)(y * _scaleY * _retinaFactor  * _frameZoomFactor + _viewPortRect.origin.y * _retinaFactor * _frameZoomFactor),
        (float)(w * _scaleX * _retinaFactor * _frameZoomFactor),
        (float)(h * _scaleY * _retinaFactor * _frameZoomFactor));
#else
#endif
    Camera::setDefaultViewport(vp);
}

void IMGUIGLViewImpl::setScissorInPoints(float x , float y , float w , float h)
{
#ifdef USE_AGTK	//Windowサイズがぴったり等倍になっていないとクリップされる領域がずれてしまうのを修正。
	GLint viewport[4];
	glGetIntegerv(GL_VIEWPORT, viewport);

#if (CC_TARGET_PLATFORM != CC_PLATFORM_NX) // #AGTK-NX
	// レンダーありとレンダーなしではclipping位置が変わる
	if (viewport[2] != _designResolutionSize.width ||
		viewport[3] != _designResolutionSize.height) {
		glScissor((GLint)(x * _scaleX * _retinaFactor * _frameZoomFactor + _viewPortRect.origin.x * _retinaFactor * _frameZoomFactor),
			(GLint)(y * _scaleY * _retinaFactor  * _frameZoomFactor + _viewPortRect.origin.y * _retinaFactor * _frameZoomFactor),
			(GLsizei)(w * _scaleX * _retinaFactor * _frameZoomFactor),
			(GLsizei)(h * _scaleY * _retinaFactor * _frameZoomFactor));
	}
	else {
		glScissor((GLint)(x * _scaleX * _retinaFactor),
			(GLint)(y * _scaleY * _retinaFactor),
			(GLsizei)(w * _scaleX * _retinaFactor),
			(GLsizei)(h * _scaleY * _retinaFactor));
	}
#else
#endif // (CC_TARGET_PLATFORM != CC_PLATFORM_NX)
#else
    glScissor((GLint)(x * _scaleX * _retinaFactor * _frameZoomFactor + _viewPortRect.origin.x * _retinaFactor * _frameZoomFactor),
               (GLint)(y * _scaleY * _retinaFactor  * _frameZoomFactor + _viewPortRect.origin.y * _retinaFactor * _frameZoomFactor),
               (GLsizei)(w * _scaleX * _retinaFactor * _frameZoomFactor),
               (GLsizei)(h * _scaleY * _retinaFactor * _frameZoomFactor));
#endif
}

Rect IMGUIGLViewImpl::getScissorRect() const
{
    GLfloat params[4];
    glGetFloatv(GL_SCISSOR_BOX, params);
// #AGTK-NX
#if (CC_TARGET_PLATFORM != CC_PLATFORM_NX)
    float x = (params[0] - _viewPortRect.origin.x * _retinaFactor * _frameZoomFactor) / (_scaleX * _retinaFactor * _frameZoomFactor);
    float y = (params[1] - _viewPortRect.origin.y * _retinaFactor * _frameZoomFactor) / (_scaleY * _retinaFactor  * _frameZoomFactor);
    float w = params[2] / (_scaleX * _retinaFactor * _frameZoomFactor);
    float h = params[3] / (_scaleY * _retinaFactor  * _frameZoomFactor);
#else
#endif
    return Rect(x, y, w, h);
}

void IMGUIGLViewImpl::onGLFWError(int errorID, const char* errorDesc)
{
    if (_mainWindow)
    {
        _glfwError = StringUtils::format("GLFWError #%d Happen, %s", errorID, errorDesc);
    }
    else
    {
        _glfwError.append(StringUtils::format("GLFWError #%d Happen, %s\n", errorID, errorDesc));
    }
    CCLOGERROR("%s", _glfwError.c_str());
}

void IMGUIGLViewImpl::onGLFWMouseCallBack(GLFWwindow* window, int button, int action, int modify)
{
	//Because OpenGL and cocos2d-x uses different Y axis, we need to convert the coordinate here
// #AGTK-NX
#if (CC_TARGET_PLATFORM != CC_PLATFORM_NX)
	float cursorX = (_mouseX - _viewPortRect.origin.x) / _scaleX;
	float cursorY = (_viewPortRect.origin.y + _viewPortRect.size.height - _mouseY) / _scaleY;
#else
#endif

	if(GLFW_MOUSE_BUTTON_LEFT == button)
    {
// #AGTK-NX
#if (CC_TARGET_PLATFORM == CC_PLATFORM_NX)
#endif

        if(GLFW_PRESS == action)
        {
            _captured = true;
// #AGTK-NX
#if (CC_TARGET_PLATFORM == CC_PLATFORM_NX)
#else
			if (this->getViewPortRect().equals(Rect::ZERO) || this->getViewPortRect().containsPoint(Vec2(_mouseX, _mouseY)))
            {
                intptr_t id = 0;
                this->handleTouchesBegin(1, &id, &_mouseX, &_mouseY);
            }
#endif
		}
        else if(GLFW_RELEASE == action)
        {
            if (_captured)
            {
                _captured = false;
                intptr_t id = 0;
                this->handleTouchesEnd(1, &id, &_mouseX, &_mouseY);
            }
        }
    }
    
    if(GLFW_PRESS == action)
    {
        EventMouse event(EventMouse::MouseEventType::MOUSE_DOWN);
        event.setCursorPosition(cursorX, cursorY);
        event.setMouseButton((cocos2d::EventMouse::MouseButton)button);
        Director::getInstance()->getEventDispatcher()->dispatchEvent(&event);
    }
    else if(GLFW_RELEASE == action)
    {
        EventMouse event(EventMouse::MouseEventType::MOUSE_UP);
        event.setCursorPosition(cursorX, cursorY);
        event.setMouseButton((cocos2d::EventMouse::MouseButton)button);
        Director::getInstance()->getEventDispatcher()->dispatchEvent(&event);
    }
}

void IMGUIGLViewImpl::onGLFWMouseMoveCallBack(GLFWwindow* window, double x, double y)
{
    _mouseX = (float)x;
    _mouseY = (float)y;

// #AGTK-NX
#if (CC_TARGET_PLATFORM != CC_PLATFORM_NX)
    _mouseX /= this->getFrameZoomFactor();
    _mouseY /= this->getFrameZoomFactor();
#endif

    if (_isInRetinaMonitor)
    {
        if (_retinaFactor == 1)
        {
            _mouseX *= 2;
            _mouseY *= 2;
        }
    }

    if (_captured)
    {
        intptr_t id = 0;
        this->handleTouchesMove(1, &id, &_mouseX, &_mouseY);
    }
    
// #AGTK-NX
#if (CC_TARGET_PLATFORM == CC_PLATFORM_NX)
#else
	//Because OpenGL and cocos2d-x uses different Y axis, we need to convert the coordinate here
	float cursorX = (_mouseX - _viewPortRect.origin.x) / _scaleX;
    float cursorY = (_viewPortRect.origin.y + _viewPortRect.size.height - _mouseY) / _scaleY;
#endif

    EventMouse event(EventMouse::MouseEventType::MOUSE_MOVE);
    // Set current button
    if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS)
    {
        event.setMouseButton((cocos2d::EventMouse::MouseButton)GLFW_MOUSE_BUTTON_LEFT);
    }
    else if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS)
    {
        event.setMouseButton((cocos2d::EventMouse::MouseButton)GLFW_MOUSE_BUTTON_RIGHT);
    }
    else if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_MIDDLE) == GLFW_PRESS)
    {
        event.setMouseButton((cocos2d::EventMouse::MouseButton)GLFW_MOUSE_BUTTON_MIDDLE);
    }
    event.setCursorPosition(cursorX, cursorY);
    Director::getInstance()->getEventDispatcher()->dispatchEvent(&event);
}

void IMGUIGLViewImpl::onGLFWMouseScrollCallback(GLFWwindow* window, double x, double y)
{
    EventMouse event(EventMouse::MouseEventType::MOUSE_SCROLL);
    //Because OpenGL and cocos2d-x uses different Y axis, we need to convert the coordinate here
// #AGTK-NX
#if (CC_TARGET_PLATFORM == CC_PLATFORM_NX)
#else
	float cursorX = (_mouseX - _viewPortRect.origin.x) / _scaleX;
    float cursorY = (_viewPortRect.origin.y + _viewPortRect.size.height - _mouseY) / _scaleY;
#endif
    event.setScrollData((float)x, -(float)y);
    event.setCursorPosition(cursorX, cursorY);
    Director::getInstance()->getEventDispatcher()->dispatchEvent(&event);
}

void IMGUIGLViewImpl::onGLFWKeyCallback(GLFWwindow *window, int key, int scancode, int action, int mods)
{
    if (GLFW_REPEAT != action)
    {
#ifdef USE_AGTK//sakihama-h, 2018.10.11
		EventKeyboard event(g_keyCodeMap[key], GLFW_PRESS == action, scancode);
#else
		EventKeyboard event(g_keyCodeMap[key], GLFW_PRESS == action);
#endif
        auto dispatcher = Director::getInstance()->getEventDispatcher();
        dispatcher->dispatchEvent(&event);
    }

    if (GLFW_RELEASE != action)
    {
        switch (g_keyCodeMap[key])
        {
        case EventKeyboard::KeyCode::KEY_BACKSPACE:
            IMEDispatcher::sharedDispatcher()->dispatchDeleteBackward();
            break;
        case EventKeyboard::KeyCode::KEY_HOME:
        case EventKeyboard::KeyCode::KEY_KP_HOME:
        case EventKeyboard::KeyCode::KEY_DELETE:
        case EventKeyboard::KeyCode::KEY_KP_DELETE:
        case EventKeyboard::KeyCode::KEY_END:
        case EventKeyboard::KeyCode::KEY_LEFT_ARROW:
        case EventKeyboard::KeyCode::KEY_RIGHT_ARROW:
        case EventKeyboard::KeyCode::KEY_ESCAPE:
            IMEDispatcher::sharedDispatcher()->dispatchControlKey(g_keyCodeMap[key]);
            break;
        default:
            break;
        }
    }
}

void IMGUIGLViewImpl::onGLFWCharCallback(GLFWwindow *window, unsigned int character)
{
    char16_t wcharString[2] = { (char16_t) character, 0 };
    std::string utf8String;

    StringUtils::UTF16ToUTF8( wcharString, utf8String );
    static std::set<std::string> controlUnicode = {
        "\xEF\x9C\x80", // up
        "\xEF\x9C\x81", // down
        "\xEF\x9C\x82", // left
        "\xEF\x9C\x83", // right
        "\xEF\x9C\xA8", // delete
        "\xEF\x9C\xA9", // home
        "\xEF\x9C\xAB", // end
        "\xEF\x9C\xAC", // pageup
        "\xEF\x9C\xAD", // pagedown
        "\xEF\x9C\xB9"  // clear
    };
    // Check for send control key
    if (controlUnicode.find(utf8String) == controlUnicode.end())
    {
        IMEDispatcher::sharedDispatcher()->dispatchInsertText( utf8String.c_str(), utf8String.size() );
    }
#ifdef USE_AGTK//sakihama-h, 2018.10.11
	EventKeyboard event(g_charCodeMap[character], GLFW_PRESS, true);
	auto dispatcher = Director::getInstance()->getEventDispatcher();
	dispatcher->dispatchEvent(&event);
#endif
}

void IMGUIGLViewImpl::onGLFWWindowPosCallback(GLFWwindow *windows, int x, int y)
{
    Director::getInstance()->setViewport();
}

void IMGUIGLViewImpl::onGLFWframebuffersize(GLFWwindow* window, int w, int h)
{
#if (CC_TARGET_PLATFORM != CC_PLATFORM_NX) // #AGTK-NX
	float frameSizeW = _screenSize.width;
    float frameSizeH = _screenSize.height;
    float factorX = frameSizeW / w * _retinaFactor * _frameZoomFactor;
    float factorY = frameSizeH / h * _retinaFactor * _frameZoomFactor;

    if (std::abs(factorX - 0.5f) < FLT_EPSILON && std::abs(factorY - 0.5f) < FLT_EPSILON)
    {
        _isInRetinaMonitor = true;
        if (_isRetinaEnabled)
        {
            _retinaFactor = 1;
        }
        else
        {
            _retinaFactor = 2;
        }

        glfwSetWindowSize(window, static_cast<int>(frameSizeW * 0.5f * _retinaFactor * _frameZoomFactor) , static_cast<int>(frameSizeH * 0.5f * _retinaFactor * _frameZoomFactor));
    }
    else if (std::abs(factorX - 2.0f) < FLT_EPSILON && std::abs(factorY - 2.0f) < FLT_EPSILON)
    {
        _isInRetinaMonitor = false;
        _retinaFactor = 1;
        glfwSetWindowSize(window, static_cast<int>(frameSizeW * _retinaFactor * _frameZoomFactor), static_cast<int>(frameSizeH * _retinaFactor * _frameZoomFactor));
    }
#endif
}

void IMGUIGLViewImpl::onGLFWWindowSizeFunCallback(GLFWwindow *window, int width, int height)
{
    if (width && height && _resolutionPolicy != ResolutionPolicy::UNKNOWN)
    {
        Size baseDesignSize = _designResolutionSize;
        ResolutionPolicy baseResolutionPolicy = _resolutionPolicy;
#ifdef USE_AGTK	//後でウィンドウの大きさを計算するときに小さなサイズが算出されないように切り捨てない。
#if (CC_TARGET_PLATFORM != CC_PLATFORM_NX) // #AGTK-NK
		auto frameWidth = width / _frameZoomFactor;
		auto frameHeight = height / _frameZoomFactor;
#else
#endif
#else
        int frameWidth = width / _frameZoomFactor;
        int frameHeight = height / _frameZoomFactor;
#endif
        setFrameSize(frameWidth, frameHeight);
        setDesignResolutionSize(baseDesignSize.width, baseDesignSize.height, baseResolutionPolicy);
        Director::getInstance()->setViewport();
    }
}

#ifdef USE_AGTK
//(*pWidth, *pHeight)がモニターサイズを超えないよう調整。必要なら_frameZoomFactorを更新。
float IMGUIGLViewImpl::adjustWindowSizeAndZoomScale(int *pWidth, int *pHeight)
{
	auto width = *pWidth;
	auto height = *pHeight;
	GLFWwindow *window = this->getWindow();
	GLFWmonitor *monitor = nullptr;
	if (window) {
		monitor = glfwGetWindowMonitor(window);
	}
	if (monitor == nullptr) {
		monitor = glfwGetPrimaryMonitor();
	}
	auto videoMode = glfwGetVideoMode(monitor);
	auto newWidth = width;
	auto newHeight = height;
	//_resolutionPolicy == ResolutionPolicy::SHOW_ALL以外には未対応。
	if (width > videoMode->width && height > videoMode->height) {
		if ((float)videoMode->width / width < (float)videoMode->height / height) {
			newWidth = videoMode->width;
			newHeight = height * videoMode->width / width;
		}
		else {
			newHeight = videoMode->height;
			newWidth = width * videoMode->height / height;
		}
	}
	else if (width > videoMode->width) {
		newWidth = videoMode->width;
		newHeight = height * videoMode->width / width;
	}
	else if (height > videoMode->height) {
		newHeight = videoMode->height;
		newWidth = width * videoMode->height / height;
	}
	if (newWidth != width || newHeight != height) {
		*pWidth = newWidth;
		*pHeight = newHeight;
	}
	if (_resolutionPolicy == ResolutionPolicy::SHOW_ALL && _designResolutionSize.width > 0 && _designResolutionSize.height > 0) {
		_scaleX = (float)newWidth / _designResolutionSize.width;
		_scaleY = (float)newHeight / _designResolutionSize.height;
		auto scale = MIN(_scaleX, _scaleY);
		if (_pixelMaxScale >= 0) {
			if (scale > 1 && scale > _pixelMaxScale) {
				scale = std::floorf(scale);
			}
			if (_pixelMaxScale > 0 && scale > _pixelMaxScale) {
				scale = _pixelMaxScale;
			}
		}
		_frameZoomFactor = scale;
	}
	else {
		if (newWidth != width || newHeight != height) {
			//フルスクリーン想定で_resolutionPolicy == ResolutionPolicy::SHOW_ALL設定想定での_frameZoomFactorを返す。
			auto scaleX = (float)newWidth / videoMode->width;
			auto scaleY = (float)newHeight / videoMode->height;
			return MIN(scaleX, scaleY);
		}
	}
	return _frameZoomFactor;
}
#endif

void IMGUIGLViewImpl::onGLFWWindowIconifyCallback(GLFWwindow* window, int iconified)
{
    if (iconified == GL_TRUE)
    {
        Application::getInstance()->applicationDidEnterBackground();
    }
    else
    {
        Application::getInstance()->applicationWillEnterForeground();
    }
}

#if (CC_TARGET_PLATFORM == CC_PLATFORM_WIN32)
static bool glew_dynamic_binding()
{
    const char *gl_extensions = (const char*)glGetString(GL_EXTENSIONS);

    // If the current opengl driver doesn't have framebuffers methods, check if an extension exists
    if (glGenFramebuffers == nullptr)
    {
        log("OpenGL: glGenFramebuffers is nullptr, try to detect an extension");
        if (strstr(gl_extensions, "ARB_framebuffer_object"))
        {
            log("OpenGL: ARB_framebuffer_object is supported");

            glIsRenderbuffer = (PFNGLISRENDERBUFFERPROC) wglGetProcAddress("glIsRenderbuffer");
            glBindRenderbuffer = (PFNGLBINDRENDERBUFFERPROC) wglGetProcAddress("glBindRenderbuffer");
            glDeleteRenderbuffers = (PFNGLDELETERENDERBUFFERSPROC) wglGetProcAddress("glDeleteRenderbuffers");
            glGenRenderbuffers = (PFNGLGENRENDERBUFFERSPROC) wglGetProcAddress("glGenRenderbuffers");
            glRenderbufferStorage = (PFNGLRENDERBUFFERSTORAGEPROC) wglGetProcAddress("glRenderbufferStorage");
            glGetRenderbufferParameteriv = (PFNGLGETRENDERBUFFERPARAMETERIVPROC) wglGetProcAddress("glGetRenderbufferParameteriv");
            glIsFramebuffer = (PFNGLISFRAMEBUFFERPROC) wglGetProcAddress("glIsFramebuffer");
            glBindFramebuffer = (PFNGLBINDFRAMEBUFFERPROC) wglGetProcAddress("glBindFramebuffer");
            glDeleteFramebuffers = (PFNGLDELETEFRAMEBUFFERSPROC) wglGetProcAddress("glDeleteFramebuffers");
            glGenFramebuffers = (PFNGLGENFRAMEBUFFERSPROC) wglGetProcAddress("glGenFramebuffers");
            glCheckFramebufferStatus = (PFNGLCHECKFRAMEBUFFERSTATUSPROC) wglGetProcAddress("glCheckFramebufferStatus");
            glFramebufferTexture1D = (PFNGLFRAMEBUFFERTEXTURE1DPROC) wglGetProcAddress("glFramebufferTexture1D");
            glFramebufferTexture2D = (PFNGLFRAMEBUFFERTEXTURE2DPROC) wglGetProcAddress("glFramebufferTexture2D");
            glFramebufferTexture3D = (PFNGLFRAMEBUFFERTEXTURE3DPROC) wglGetProcAddress("glFramebufferTexture3D");
            glFramebufferRenderbuffer = (PFNGLFRAMEBUFFERRENDERBUFFERPROC) wglGetProcAddress("glFramebufferRenderbuffer");
            glGetFramebufferAttachmentParameteriv = (PFNGLGETFRAMEBUFFERATTACHMENTPARAMETERIVPROC) wglGetProcAddress("glGetFramebufferAttachmentParameteriv");
            glGenerateMipmap = (PFNGLGENERATEMIPMAPPROC) wglGetProcAddress("glGenerateMipmap");
        }
        else
        if (strstr(gl_extensions, "EXT_framebuffer_object"))
        {
            log("OpenGL: EXT_framebuffer_object is supported");
            glIsRenderbuffer = (PFNGLISRENDERBUFFERPROC) wglGetProcAddress("glIsRenderbufferEXT");
            glBindRenderbuffer = (PFNGLBINDRENDERBUFFERPROC) wglGetProcAddress("glBindRenderbufferEXT");
            glDeleteRenderbuffers = (PFNGLDELETERENDERBUFFERSPROC) wglGetProcAddress("glDeleteRenderbuffersEXT");
            glGenRenderbuffers = (PFNGLGENRENDERBUFFERSPROC) wglGetProcAddress("glGenRenderbuffersEXT");
            glRenderbufferStorage = (PFNGLRENDERBUFFERSTORAGEPROC) wglGetProcAddress("glRenderbufferStorageEXT");
            glGetRenderbufferParameteriv = (PFNGLGETRENDERBUFFERPARAMETERIVPROC) wglGetProcAddress("glGetRenderbufferParameterivEXT");
            glIsFramebuffer = (PFNGLISFRAMEBUFFERPROC) wglGetProcAddress("glIsFramebufferEXT");
            glBindFramebuffer = (PFNGLBINDFRAMEBUFFERPROC) wglGetProcAddress("glBindFramebufferEXT");
            glDeleteFramebuffers = (PFNGLDELETEFRAMEBUFFERSPROC) wglGetProcAddress("glDeleteFramebuffersEXT");
            glGenFramebuffers = (PFNGLGENFRAMEBUFFERSPROC) wglGetProcAddress("glGenFramebuffersEXT");
            glCheckFramebufferStatus = (PFNGLCHECKFRAMEBUFFERSTATUSPROC) wglGetProcAddress("glCheckFramebufferStatusEXT");
            glFramebufferTexture1D = (PFNGLFRAMEBUFFERTEXTURE1DPROC) wglGetProcAddress("glFramebufferTexture1DEXT");
            glFramebufferTexture2D = (PFNGLFRAMEBUFFERTEXTURE2DPROC) wglGetProcAddress("glFramebufferTexture2DEXT");
            glFramebufferTexture3D = (PFNGLFRAMEBUFFERTEXTURE3DPROC) wglGetProcAddress("glFramebufferTexture3DEXT");
            glFramebufferRenderbuffer = (PFNGLFRAMEBUFFERRENDERBUFFERPROC) wglGetProcAddress("glFramebufferRenderbufferEXT");
            glGetFramebufferAttachmentParameteriv = (PFNGLGETFRAMEBUFFERATTACHMENTPARAMETERIVPROC) wglGetProcAddress("glGetFramebufferAttachmentParameterivEXT");
            glGenerateMipmap = (PFNGLGENERATEMIPMAPPROC) wglGetProcAddress("glGenerateMipmapEXT");
        }
        else
        {
            log("OpenGL: No framebuffers extension is supported");
            log("OpenGL: Any call to Fbo will crash!");
            return false;
        }
    }
    return true;
}
#endif

// helper
bool IMGUIGLViewImpl::initGlew()
{
// #AGTK-NX
#if (CC_TARGET_PLATFORM != CC_PLATFORM_MAC) && (CC_TARGET_PLATFORM != CC_PLATFORM_NX)
    GLenum GlewInitResult = glewInit();
    if (GLEW_OK != GlewInitResult)
    {
        MessageBox((char *)glewGetErrorString(GlewInitResult), "OpenGL error");
        return false;
    }

    if (GLEW_ARB_vertex_shader && GLEW_ARB_fragment_shader)
    {
        log("Ready for GLSL");
    }
    else
    {
        log("Not totally ready :(");
    }

    if (glewIsSupported("GL_VERSION_2_0"))
    {
        log("Ready for OpenGL 2.0");
    }
    else
    {
        log("OpenGL 2.0 not supported");
    }

#if (CC_TARGET_PLATFORM == CC_PLATFORM_WIN32)
    if(glew_dynamic_binding() == false)
    {
        MessageBox("No OpenGL framebuffer support. Please upgrade the driver of your video card.", "OpenGL error");
        return false;
    }
#endif

#endif // (CC_TARGET_PLATFORM != CC_PLATFORM_MAC) && (CC_TARGET_PLATFORM != CC_PLATFORM_NX)

    return true;
}

#if (CC_TARGET_PLATFORM == CC_PLATFORM_WIN32)
#ifdef USE_AGTK//日本語入力IMEのフォーカス制御する sakihama-h, 2018.05.08
//入力フォーカスを外す。
void IMGUIGLViewImpl::onFocusOutIme()
{
	auto handle = this->getWin32Window();
	if (!handle) {
		CC_ASSERT(0);
		return;
	}
	if (_imcReserve) {
		return;
	}
	auto hImc = ImmGetContext(handle);
	if (ImmGetCompositionString(hImc, GCS_COMPSTR, nullptr, 0) > 0) {
		ImmNotifyIME(hImc, NI_COMPOSITIONSTR, CPS_CANCEL, 0);
	}
	ImmReleaseContext(handle, hImc);

	_imcReserve = ImmAssociateContext(handle, nullptr);
}

//入力フォーカスを受け取る。
void IMGUIGLViewImpl::onFocusInIme()
{
	auto handle = this->getWin32Window();
	if (!handle) {
		CC_ASSERT(0);
		return;
	}
	if (_imcReserve) {
		ImmAssociateContext(handle, _imcReserve);

		auto hImc = ImmGetContext(handle);
		if (ImmGetCompositionString(hImc, GCS_COMPSTR, nullptr, 0) > 0) {
			ImmNotifyIME(hImc, NI_COMPOSITIONSTR, CPS_CANCEL, 0);
		}

		ImmReleaseContext(handle, hImc);
		_imcReserve = nullptr;
	}
}

//入力フォーカスを受け取っている状態のとき真
bool IMGUIGLViewImpl::isFocusInIme()
{
	return !_imcReserve;
}

//IMEのOpen状態を切り替える
void IMGUIGLViewImpl::setImeOpenStatus(bool open)
{
	auto handle = this->getWin32Window();
	if (!handle) {
		CC_ASSERT(0);
		return;
	}
	auto hImc = ImmGetContext(handle);
	if (hImc) {
		ImmSetOpenStatus(hImc, open);
	}
}
#endif
#endif /* (CC_TARGET_PLATFORM == CC_PLATFORM_WIN32) */

NS_CC_END // end of namespace cocos2d;
