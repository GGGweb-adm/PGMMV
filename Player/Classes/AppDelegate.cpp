#include "AppDelegate.h"
#include "GameScene.h"
#include "GameManager.h"
#include "DebugManager.h"
#include "AudioManager.h"
#include "InputManager.h"
#ifdef USE_RUNTIME
#include "Lib/Runtime/FileUtils-runtime.h"
#include "LogoScene.h"
#endif

// #AGTK-NX
#if (CC_TARGET_PLATFORM == CC_PLATFORM_NX)
#endif

#include "jsapi.h"
#include "scripting/js-bindings/manual/ScriptingCore.h"
#include "scripting/js-bindings/auto/jsb_cocos2dx_auto.hpp"
#include "scripting/js-bindings/manual/js_module_register.h"
#include "scripting/js-bindings/manual/cocos2d_specifics.hpp"
#include "Manager/FontManager.h"

// #AGTK-NX
#if (CC_TARGET_PLATFORM == CC_PLATFORM_WIN32) || (CC_TARGET_PLATFORM == CC_PLATFORM_MAC) || (CC_TARGET_PLATFORM == CC_PLATFORM_NX)
#include "ImGUI/IMGUIGLViewImpl.h"
#include "ImGUI/ImGuiLayer.h"
#include "ImGUI/imgui.h"
#endif

#ifdef OPENAL_PLAIN_INCLUDES
#include "alc.h"
#include "alext.h"
#else
#include "AL/alc.h"
#include "AL/alext.h"
#endif

#if (CC_TARGET_PLATFORM == CC_PLATFORM_WIN32)
#include <stdlib.h>
#include <winnls.h>
#endif

#if (CC_TARGET_PLATFORM == CC_PLATFORM_WIN32)
#include <windows.h>
#include <cstdio>
#include <clocale>
#include <typeinfo>
#include <conio.h>
#include <crtdbg.h>
#endif

#include "Manager/JavascriptManager.h"

#if defined(USE_PRELOAD_TEX) && !defined(USE_BG_PROJECT_LOAD)
#include "Manager/ProjectLoadingManager.h"
#endif

// #AGTK-NX
#if (CC_TARGET_PLATFORM == CC_PLATFORM_NX)
#endif // (CC_TARGET_PLATFORM == CC_PLATFORM_NX)

USING_NS_CC;

static cocos2d::Size designResolutionSize = cocos2d::Size(960, 640);
static cocos2d::Size smallResolutionSize = cocos2d::Size(480, 320);
static cocos2d::Size mediumResolutionSize = cocos2d::Size(1024, 768);
static cocos2d::Size largeResolutionSize = cocos2d::Size(2048, 1536);

#if (CC_TARGET_PLATFORM == CC_PLATFORM_NX) // #AGTK-NX
#endif

AppDelegate::AppDelegate()
{
}

AppDelegate::~AppDelegate() 
{
	// ゲームマネージャをパージ
	auto gm = GameManager::getInstance();
	if (gm) {
		gm->purge();
	}

	// オーディオマネージャをパージ
	auto am = AudioManager::getInstance();
	if (am) {
// #AGTK-NX
#if (CC_TARGET_PLATFORM == CC_PLATFORM_NX)
#endif
		am->purge();
	}
#ifdef USE_RUNTIME
	AGTK_ACTION_LOG(1, "normal termination.");
#endif

// #AGTK-NX
#if (CC_TARGET_PLATFORM == CC_PLATFORM_NX)
#endif
}

// if you want a different context, modify the value of glContextAttrs
// it will affect all platforms
void AppDelegate::initGLContextAttrs()
{
// #AGTK-NX
#if (CC_TARGET_PLATFORM == CC_PLATFORM_NX)
#endif

    // set OpenGL context attributes: red,green,blue,alpha,depth,stencil,multisamplesCount
    GLContextAttrs glContextAttrs = {8, 8, 8, 8, 24, 8, 0};

    GLView::setGLContextAttrs(glContextAttrs);
}

// if you want to use the package manager to install more packages,  
// don't modify or remove this function
static int register_all_packages()
{
    return 0; //flag for packages manager
}

#if (CC_TARGET_PLATFORM == CC_PLATFORM_WIN32)
// WCHAR文字列をcharに変換する
static LPSTR wtoa(LPCWSTR src)
{
	LPSTR buf;
	int dst_size, rc;

	rc = WideCharToMultiByte(CP_UTF8, 0, src, -1, NULL, 0, NULL, NULL);
	if (rc == 0) {
		return NULL;
	}

	dst_size = rc + 1;
	buf = (LPSTR)malloc(dst_size);
	if (buf == NULL) {
		return NULL;
	}

	rc = WideCharToMultiByte(CP_UTF8, 0, src, -1, buf, dst_size, NULL, NULL);
	if (rc == 0) {
		free(buf);
		return NULL;
	}
	buf[rc] = '\0';

	return buf;
}
#endif

#if (CC_TARGET_PLATFORM == CC_PLATFORM_WIN32)
static void getWebSocketPortAndProjectPathAndLocale(int *pPort, std::string *pProjectFilePath, std::string *pLocale, int *pLogNumber, std::string *pAutoTestFilePath)
{
	int argc;

	auto argv_w = CommandLineToArgvW(GetCommandLine(), &argc);

	//オプション引数検索
	bool portArg = false;
	bool projectArg = false;
	bool autoTestArg = false;
	bool localArg = false;
	bool logNumberArg = false;
	for (int i = 0; i < argc; i++) {
		auto argv_c = wtoa(argv_w[i]);
		if (argv_c == NULL) {
			break;
		}
		CCLOG("%d: %s", i, argv_c);
		if (i == 0) {
		}
		else {
			if (portArg) {
				*pPort = atoi(argv_c);
				portArg = false;
			}
			else if (projectArg) {
				*pProjectFilePath = argv_c;
				projectArg = false;
			}
			else if (autoTestArg) {
				auto fileUtils = FileUtils::getInstance();
				auto path = fileUtils->getApplicationPath() + argv_c;
				*pAutoTestFilePath = path;
				autoTestArg = false;
			}
			else if (localArg) {
				*pLocale = argv_c;
				localArg = false;
			}
			else if (logNumberArg) {
				*pLogNumber = atoi(argv_c);
				logNumberArg = false;
			}
			else if (strcmp(argv_c, "-port") == 0) {
				portArg = true;
			}
			else if (strcmp(argv_c, "-projectFile") == 0) {
				projectArg = true;
			}
			else if (strcmp(argv_c, "-autotest") == 0) {
				autoTestArg = true;
			}
			else if (strcmp(argv_c, "-locale") == 0) {
				localArg = true;
			}
			else if (strcmp(argv_c, "-log") == 0) {
				logNumberArg = true;
			}
			else {
				// オプション指定なしの場合は、従来のコマンドライン指定
				if (i == 1) {
					*pPort = atoi(argv_c);
				}
				else if (i == 2) {
					*pProjectFilePath = argv_c;
				}
				else if (i == 3) {
					*pLocale = argv_c;
				}
				else if (i == 4) {
					*pLogNumber = atoi(argv_c);
				}
			}
		}
		free(argv_c);
	}
	(void)LocalFree((HLOCAL)argv_w);
}
// #AGTK-NX
#elif (CC_TARGET_PLATFORM == CC_PLATFORM_NX)
#endif

#if CC_TARGET_PLATFORM == CC_PLATFORM_WIN32
static void getAppInfo()
{
	int argc;
	auto argv_w = CommandLineToArgvW(GetCommandLine(), &argc);

	DWORD dwHandle;
	auto len = GetFileVersionInfoSize(argv_w[0], &dwHandle);
	if(len){
		auto buf = new wchar_t[len + 1];
		if(buf){
			if(GetFileVersionInfo(argv_w[0], 0, len, buf)){
				UINT langLen;
				DWORD *lang;

				// lang-codepageを取得
				if (VerQueryValue(buf, L"\\VarFileInfo\\Translation", (void **)&lang, &langLen))
				{
					// 情報を取得する
					LPTSTR lpBuffer;
					// 製品バージョン名
					wchar_t subBlock[128];
					wsprintf(subBlock, L"\\StringFileInfo\\%04X%04X\\%s", LOWORD(*lang), HIWORD(*lang), L"FileVersion");
					VerQueryValue(buf, subBlock, (void **)&lpBuffer, &langLen);
					auto astr = wtoa(lpBuffer);
					if (strcmp(astr + strlen(astr) - 2, ".0") == 0) {
						wsprintf(subBlock, L"\\StringFileInfo\\%04X%04X\\%s", LOWORD(*lang), HIWORD(*lang), L"ProductVersion");
						VerQueryValue(buf, subBlock, (void **)&lpBuffer, &langLen);
						astr = wtoa(lpBuffer);
					}
					GameManager::getInstance()->setAppVersion(cocos2d::String::create(std::string(astr)));
					free(astr);
					wsprintf(subBlock, L"\\StringFileInfo\\%04X%04X\\%s", LOWORD(*lang), HIWORD(*lang), L"ProductName");
					VerQueryValue(buf, subBlock, (void **)&lpBuffer, &langLen);
					astr = wtoa(lpBuffer);
					GameManager::getInstance()->setAppName(cocos2d::String::create(std::string(astr)));
					free(astr);
					// 表示
					CCLOG("%s %s", GameManager::getInstance()->getAppName()->getCString(), GameManager::getInstance()->getAppVersion()->getCString());
				}
			}
			delete[] buf;
		}
	}
	(void)LocalFree((HLOCAL)argv_w);
}
// #AGTK-NX
#elif (CC_TARGET_PLATFORM == CC_PLATFORM_NX)
#endif

#if defined(USE_PREVIEW)
extern "C" {
	static void consoleLogCallback(const char *log)
	{
		auto fp = GameManager::getScriptLogFp();
		if (fp) {
			fprintf(fp, "console: %s\n", log);
		}
	}
}
#endif

bool AppDelegate::applicationDidFinishLaunching() {

// #AGTK-NX
#if (CC_TARGET_PLATFORM == CC_PLATFORM_NX)
#else
	FileUtils::getInstance()->addSearchPath("Resources");
#endif
	//アクツクMVではPNGをストレートアルファで扱う。
	cocos2d::CCImage::setPNGPremultipliedAlphaEnabled(false);

	int port = 1234;
	std::string projectFilePath = FileUtils::getInstance()->fullPathForFilename(std::string("data/project.json"));
	std::string editorLocale = "ja_JP";
	int logNumber = 0;
	std::string autoTestFilePath;
	getWebSocketPortAndProjectPathAndLocale(&port, &projectFilePath, &editorLocale, &logNumber, &autoTestFilePath);
#ifdef USE_RUNTIME
// #AGTK-NX
#if (CC_TARGET_PLATFORM == CC_PLATFORM_NX)
#else
	{
		int argc;
		auto argv_w = CommandLineToArgvW(GetCommandLine(), &argc);
		do if (argc >= 2) {
			auto argv_c = wtoa(argv_w[1]);
			if (argv_c == NULL) {
				break;
			}
			CCLOG("%d: %s", 1, argv_c);
			logNumber = atoi(argv_c);
			free(argv_c);
		} while (0);
		(void)LocalFree((HLOCAL)argv_w);
	}
#endif
#else
	GameManager::setWebSocketPort(port);
#endif
	GameManager::setLogNumber(logNumber);
	getAppInfo();
#if !defined(USE_RUNTIME) && (CC_TARGET_PLATFORM == CC_PLATFORM_WIN32)
	//引数がなければ終了させる。（Windowsではタスクバーから単体起動できるが、起動できても意味が無いためすぐに終了させる。）
	int argc;
	auto argv_w = CommandLineToArgvW(GetCommandLine(), &argc);
	(void)LocalFree((HLOCAL)argv_w);
	if (argc <= 1) {
		exit(2);
	}
#endif
#ifdef USE_RUNTIME
	{
		auto projectJson = FileUtils::getInstance()->getStringFromFile(projectFilePath);
		if (strncmp(projectJson.c_str(), "enc", 3) == 0) {
			GameManager::getInstance()->setEncryptSaveFile(true);
		}
	}
	FileUtilsRuntime::createAndSet();
// #AGTK-NX
#if (CC_TARGET_PLATFORM == CC_PLATFORM_NX)
#else
	FileUtils::getInstance()->addSearchPath("Resources");
#endif
	auto index = projectFilePath.find_last_of('/');
	auto jsonData = FileUtils::getInstance()->getStringFromFile(projectFilePath.substr(0, index) + "/info.json");
	rapidjson::Document doc;
	doc.Parse(jsonData.c_str());
	bool error = doc.HasParseError();
	if (error) {
		CCASSERT(0, "Error: Json Parse.");
	}
	else {
		if (doc.HasMember("projectFolderSave") && doc["projectFolderSave"].GetBool()) {
			GameManager::getInstance()->setProjectFolderSave(true);
		}
	}
#endif
	GameManager *gm = GameManager::getInstance();
#ifdef USE_RUNTIME
	std::string locale = "en_US";
#if (CC_TARGET_PLATFORM == CC_PLATFORM_WIN32)
	{
		auto ja_JP_LangID = MAKELANGID(LANG_JAPANESE, SUBLANG_JAPANESE_JAPAN);
		//auto en_US_LangID = MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US);
		auto userDefUiLang = ::GetUserDefaultUILanguage();
		auto zh_CN_LangID = MAKELANGID(LANG_CHINESE, SUBLANG_CHINESE_SIMPLIFIED);
		CCLOG("GetUserDefaultUILanguage: 0x%04hx", userDefUiLang);
		if (userDefUiLang == ja_JP_LangID) {
			locale = "ja_JP";
		} else if (userDefUiLang == zh_CN_LangID) {
			locale = "zh_CN";
		}
	}
// #AGTK-NX
#elif (CC_TARGET_PLATFORM == CC_PLATFORM_NX)
#endif
	gm->setEditorLocale(locale);
#else
	gm->setEditorLocale(editorLocale);
#endif
	std::string uiFontName = "mplus-1m-regular";
	if (gm->getEditorLocale() == "zh_CN") {
		uiFontName = "SourceHanSansCN-Regular";
	}
	MenuItemFont::setFontName(uiFontName);
	ui::RichText::setFontName(uiFontName);
	agtk::DebugManager::setFontName(uiFontName);
	GameScene::setFontName(uiFontName);
// #AGTK-NX
#ifdef USE_BG_PROJECT_LOAD
	// プロジェクトデータに依存する処理を後で行う
	cocos2d::Size screenSize;
	float frameZoomFactor = 1.0f;
	ResolutionPolicy resolutionPolicy = ResolutionPolicy::NO_ADJUST;

	{
		std::vector<string> orgSearchPaths = FileUtils::getInstance()->getSearchPaths();
		FileUtils::getInstance()->addSearchPath("rom:/");
		auto fileUtils = static_cast<FileUtilsRuntime *>(FileUtils::getInstance());
		std::string key = fileUtils->key();
		fileUtils->setKey("");

		std::string filePath = "playerResources/defaultSettings.json";
		auto jsonData = FileUtils::getInstance()->getStringFromFile(filePath);
		rapidjson::Document doc;
		doc.Parse(jsonData.c_str());
		bool error = doc.HasParseError();
		if (error) {
			CCASSERT(0, "Error: inputMapping Json Parse.");
			return false;
		}

		AGTK_ACTION_LOG(1, "# InputManager Init");
		InputManager::getInstance()->init(agtk::data::InputMappingData::create(doc));

		fileUtils->setKey(key);
		FileUtils::getInstance()->setSearchPaths(orgSearchPaths);
	}

	// GameManagerのprojectDataに依存する処理を無効化
	GameManager::getInstance()->setEnableProject(false);
#else
// #AGTK-NX
#if (CC_TARGET_PLATFORM == CC_PLATFORM_NX)
#endif

// #AGTK-NX #AGTK-WIN
#ifdef USE_PRELOAD_TEX
	ProjectLoadingManager::getInstance()->load(projectFilePath);
	ImageMtTask::getInstance()->setEnable(false);
	ProjectLoadingManager::getInstance()->postProcessPreloadTex();
#else
	gm->loadJsonFile(projectFilePath);
#endif
	gm->deserialize();
	gm->serialize();

// #AGTK-NX
#if (CC_TARGET_PLATFORM == CC_PLATFORM_NX)
#endif

	AGTK_ACTION_LOG(1, "# InputManager Init");
	InputManager::getInstance()->init(gm->getProjectData()->getInputMapping());
	AGTK_ACTION_LOG(1, "# AudioManager Init");
	AudioManager::getInstance()->init(gm->getProjectData()->getSoundSetting());

	auto projectData = gm->getProjectData();
	auto screenSize = projectData->getScreenSize();
	float frameZoomFactor = projectData->getMagnifyWindow() ? projectData->getWindowMagnification() : 1.0f;
#if (CC_TARGET_PLATFORM == CC_PLATFORM_NX) // #AGTK-NX
#endif
#endif // USE_BG_PROJECT_LOAD

    // initialize director
	AGTK_ACTION_LOG(1, "# Director GetInstance");
    auto director = Director::getInstance();

	AGTK_ACTION_LOG(1, "# GetOpenGLView");
	auto glview = director->getOpenGLView();
    if(!glview) {
#if (CC_TARGET_PLATFORM == CC_PLATFORM_WIN32) || (CC_TARGET_PLATFORM == CC_PLATFORM_MAC)
		AGTK_ACTION_LOG(1, "# IMGUIGLViewImpl CreateWithRect");
		glview = IMGUIGLViewImpl::createWithRect(
			projectData->getGameInformation()->getTitle(),//viewName
			Rect(0, 0, screenSize.width, screenSize.height),//size
			frameZoomFactor,
			true//resizable
		);
// #AGTK-NX
#elif (CC_TARGET_PLATFORM == CC_PLATFORM_NX)
#else
		glview = GLViewImpl::create(gm->getSystem()->getAppName()->getCString(), true);
		//GLFWmonitor *monitor = glfwGetPrimaryMonitor();
		//CC_ASSERT(monitor);
		//const GLFWvidmode* videoMode = glfwGetVideoMode(monitor);
		//glview = GLViewImpl::createWithFullScreen(gm->getSystem()->getAppName(), *videoMode, monitor);
		//glview = GLViewImpl::createWithRect(gm->getSystem()->getAppName(), cocos2d::Rect(cocos2d::Vec2(0, 0), designResolutionSize), 1.0f, true);
#endif
// #AGTK-NX
#if (CC_TARGET_PLATFORM == CC_PLATFORM_NX)
#endif
		director->setOpenGLView(glview);
	}
// #AGTK-NX #AGTK-WIN
#if defined(USE_PRELOAD_TEX) && !defined(USE_BG_PROJECT_LOAD)
	ProjectLoadingManager::getInstance()->postProcessCacheTex();
#endif
#if defined(USE_PREVIEW) && (CC_TARGET_PLATFORM == CC_PLATFORM_WIN32)
	AGTK_ACTION_LOG(1, "# DragAcceptFiles");
	DragAcceptFiles(
		Director::getInstance()->getOpenGLView()->getWin32Window(),
		TRUE
	);
#endif

	director->getInstance()->setProjection(Director::Projection::_2D);
	director->getInstance()->setDepthTest(false);

	//アクションログ(action_log.txt)
	AGTK_ACTION_LOG(1, "############################################################################################");
	AGTK_ACTION_LOG(1, "# Action log ");
	AGTK_ACTION_LOG(1, "############################################################################################");
	AGTK_ACTION_LOG(1, "# OpenGL");
	AGTK_ACTION_LOG(1, "Vender : %s", (char *)glGetString(GL_VENDOR));
	AGTK_ACTION_LOG(1, "GPU    : %s", (char *)glGetString(GL_RENDERER));
	AGTK_ACTION_LOG(1, "OpenGL ver.%s", (char *)glGetString(GL_VERSION));
	AGTK_ACTION_LOG(1, "############################################################################################");
	AGTK_ACTION_LOG(1, "# GLFW");
	AGTK_ACTION_LOG(1, glfwGetVersionString());
	AGTK_ACTION_LOG(1, "############################################################################################");
	AGTK_ACTION_LOG(1, "# cocos2d Configuration");
	Configuration *conf = Configuration::getInstance();
	AGTK_ACTION_LOG(1, conf->getInfo().c_str());
	AGTK_ACTION_LOG(1, "############################################################################################");
	AGTK_ACTION_LOG(1, "# Language:%d", (int)CCApplication::getCurrentLanguage());
	AGTK_ACTION_LOG(1, "############################################################################################");

    // turn on display FPS
    // set FPS. the default value is 1.0/60 if you don't call this
// #AGTK-NX #AGTK-WIN
#ifdef USE_30FPS
	director->setAnimationInterval(1.0f/FRAME60_RATE);// gm->getSystem()->getFPS());
#else
	director->setAnimationInterval(FRAME_PER_SECONDS);// gm->getSystem()->getFPS());
#endif

	//randam seed
	chrono::time_point<chrono::system_clock> now = chrono::system_clock::now();
	auto micro = std::chrono::duration_cast<std::chrono::microseconds > (now.time_since_epoch()).count() & 0x00000000ffffffff;
	CCLOG("randam seed : %u", micro);
	srand(micro);// time(NULL));

	//resolution size
	float scaleFactor = 1.0f;
#if (CC_TARGET_PLATFORM != CC_PLATFORM_NX) // #AGTK-NX
	glview->setDesignResolutionSize(screenSize.width, screenSize.height, ResolutionPolicy::SHOW_ALL);
#else
#endif
	director->setContentScaleFactor(scaleFactor);

// #AGTK-NX
#ifdef USE_BG_PROJECT_LOAD
	// プロジェクトデータに依存する処理を後で行う
#else
	//cursor
	glview->setCursorVisible(projectData->getDisplayMousePointer());

	if ((bool)projectData->getScreenSettings() != agtk::IsFullScreen()) {
		AGTK_ACTION_LOG(1, "# ChangeScreen:%d", projectData->getScreenSettings());
		agtk::ChangeScreen(projectData->getScreenSettings());
	}

	/*
    // if the frame's height is larger than the height of medium size.
    if (frameSize.height > mediumResolutionSize.height)
    {        
        director->setContentScaleFactor(MIN(largeResolutionSize.height/designResolutionSize.height, largeResolutionSize.width/designResolutionSize.width));
    }
    // if the frame's height is larger than the height of small size.
    else if (frameSize.height > smallResolutionSize.height)
    {        
        director->setContentScaleFactor(MIN(mediumResolutionSize.height/designResolutionSize.height, mediumResolutionSize.width/designResolutionSize.width));
    }
    // if the frame's height is smaller than the height of medium size.
    else
    {        
        director->setContentScaleFactor(MIN(smallResolutionSize.height/designResolutionSize.height, smallResolutionSize.width/designResolutionSize.width));
    }
*/
    register_all_packages();

	ScriptEngineProtocol *engine = ScriptingCore::getInstance();
	ScriptEngineManager::getInstance()->setScriptEngine(engine);
	ScriptingCore* sc = ScriptingCore::getInstance();
	js_module_register();
	AGTK_ACTION_LOG(1, "# ScriptingCore Start");
	sc->start();
#ifdef USE_PREVIEW
	sc->enableDebugger();
#endif
	sc->runScript("script/jsb_boot.js");
	sc->runScript("script/jsb.js");
#if defined(USE_PREVIEW)
	registerConsoleLogCallback(consoleLogCallback);
#endif
	AGTK_ACTION_LOG(1, "# FontManager GetInstance");
	FontManager::getInstance();

// #AGTK-NX
#if (CC_TARGET_PLATFORM == CC_PLATFORM_NX)
#endif

#endif // USE_BG_PROJECT_LOAD

// #AGTK-NX
#if (CC_TARGET_PLATFORM == CC_PLATFORM_NX)
#endif

#ifdef USE_RUNTIME
#if (CC_TARGET_PLATFORM == CC_PLATFORM_NX) // #AGTK-NX
#endif
	{
#ifdef USE_BG_PROJECT_LOAD
		auto scene = LogoScene::createScene(locale, projectFilePath);
#else
		auto scene = LogoScene::createScene( locale );
#endif
		if (!scene) {
			AGTK_ACTION_LOG(1, "# GameManager StartCanvas");
			GameManager::getInstance()->startCanvas();
		}
		else {
			director->runWithScene(scene);
		}
	}
#else
    // create a scene. it's an autorelease object
	//GameManager::getInstance()->setNextSceneId(0);
	//auto scene = GameScene::createScene();
//    auto scene = MainScene::createScene();
	AGTK_ACTION_LOG(1, "# GameManager StartCanvas");
	GameManager::getInstance()->startCanvas();
#endif

#if (CC_TARGET_PLATFORM == CC_PLATFORM_WIN32) || (CC_TARGET_PLATFORM == CC_PLATFORM_MAC)
# if 1
	AGTK_ACTION_LOG(1, "# DebugManager GetInstance");
	agtk::DebugManager::getInstance();
#else
	cocos2d::Director::getInstance()->getScheduler()->schedule([](float)
	{
		if (!Director::getInstance()->getRunningScene()->getChildByName("ImGUILayer"))
		{
			Director::getInstance()->getRunningScene()->addChild(ImGuiLayer::create(), INT_MAX, "ImGUILayer");
		}
	}, this, 0, false, "checkImGUI");
#endif
#endif

#ifdef USE_PREVIEW
	// 自動テスト設定のjsonファイルから情報を取得
	if (autoTestFilePath.compare("") != 0) {
		auto jsonData = FileUtils::getInstance()->getStringFromFile(autoTestFilePath);
		rapidjson::Document doc;
		doc.Parse(jsonData.c_str());
		bool error = doc.HasParseError();
		if (error) {
			CCASSERT(0, "Error: Json Parse.");
		}
		else {
			std::string replayFile = "";				// リプレイ pgminputファイルパス
			std::string pluginFile = "";				// プラグイン jsファイルパス
			std::string pluginInternalJson = "null";	// プラグイン 内部パラメータJSON
			std::string pluginParamValueJson = "null";	// プラグイン パラメータ設定値JSON
			
			auto it = doc.MemberBegin();
			while (it != doc.MemberEnd()) {
				auto key = it->name.GetString();
				if (strcmp(key, "replayfile") == 0) {
					replayFile = it->value.GetString();
				}
				else if (strcmp(key, "pluginfile") == 0) {
					pluginFile = it->value.GetString();
				}
				else if (strcmp(key, "pluginInternalJson") == 0) {
					pluginInternalJson = it->value.GetString();
				}
				else if (strcmp(key, "pluginParamValueJson") == 0) {
					pluginParamValueJson = it->value.GetString();
				}
				it++;
			}

			auto fileUtils = FileUtils::getInstance();
			// リプレイファイル
			if (replayFile.compare("") != 0) {
				// player.exeからの相対パスを設定
				std::string relativePath = fileUtils->getApplicationPath() + replayFile.c_str();
				// リプレイファイルパスを保存
				gm->setAutoTestReplayFilePath(relativePath);
			}

			// プラグイン
			if (pluginFile.compare("") != 0) {
				std::string relativePath = fileUtils->getApplicationPath() + pluginFile.c_str();
				// プラグインファイルパスを保存
				gm->setAutoTesPluginFilePath(relativePath);
				gm->setAutoTesPluginInternalJson(pluginInternalJson);
				gm->setAutoTesPluginParamValueJson(pluginParamValueJson);
				JavascriptManager::loadAutoTestPlugins(relativePath, pluginInternalJson, pluginParamValueJson);
			}
		}
	}
#endif
#ifdef USE_BG_PROJECT_LOAD
	// プロジェクトデータに依存する処理を後で行う
#else
	auto inputManager = InputManager::getInstance();
	inputManager->setInputMappingData(projectData->getInputMapping());
	agtk::setTileCollisionThreshold(projectData->getWallDetectionOverlapMargin());
	if (projectData->getMultithreading()) {
		ThreadManager::setUsedThreadCount(MAX(1, MIN(2, AGTK_THREAD_COUNT)));
	}
	else {
		ThreadManager::setUsedThreadCount(1);
	}
#endif

    return true;
}

// This function will be called when the app is inactive. Note, when receiving a phone call it is invoked.
void AppDelegate::applicationDidEnterBackground() {
    Director::getInstance()->stopAnimation();

    // if you use SimpleAudioEngine, it must be paused
    // SimpleAudioEngine::getInstance()->pauseBackgroundMusic();
}

// this function will be called when the app is active again
void AppDelegate::applicationWillEnterForeground() {
    Director::getInstance()->startAnimation();

    // if you use SimpleAudioEngine, it must resume here
    // SimpleAudioEngine::getInstance()->resumeBackgroundMusic();
}
