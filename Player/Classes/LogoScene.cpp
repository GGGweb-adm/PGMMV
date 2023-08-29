#include "AppMacros.h"
#include "Manager/GameManager.h"
#ifdef USE_RUNTIME
#include "Lib/Runtime/FileUtils-runtime.h"
#endif
#include "LogoScene.h"
// #AGTK-NX
#if (CC_TARGET_PLATFORM == CC_PLATFORM_NX)
#endif
#ifdef USE_BG_PROJECT_LOAD
#if (CC_TARGET_PLATFORM == CC_PLATFORM_NX)
#endif
#include "Manager\ProjectLoadingManager.h"
#include "DebugManager.h"
#include "jsapi.h"
#include "scripting/js-bindings/manual/ScriptingCore.h"
#include "scripting/js-bindings/auto/jsb_cocos2dx_auto.hpp"
#include "scripting/js-bindings/manual/js_module_register.h"
#include "scripting/js-bindings/manual/cocos2d_specifics.hpp"
#include "Manager/FontManager.h"
#include "base/CCDirector.h"
#endif
USING_NS_CC;

#ifdef USE_BG_PROJECT_LOAD
static cocos2d::Rect loadingFrame[] = {
	{ 0.0f, 0.0f, 72.0f, 64.0f},
	{ 72.0f, 0.0f, 72.0f, 64.0f },
	{ 144.0f, 0.0f, 72.0f, 64.0f },
	{ 216.0f, 0.0f, 72.0f, 64.0f }
};
#endif

LogoScene::LogoScene()
: mLeaving(false)
{
}

#ifdef USE_BG_PROJECT_LOAD
Scene* LogoScene::createScene(const std::string &locale, const std::string &projectFilePath)
#else
Scene* LogoScene::createScene(const std::string &locale)
#endif
{
    // 'scene' is an autorelease object
    auto scene = Scene::create();
    
    // 'layer' is an autorelease object
    auto layer = LogoScene::create(locale);
	if (!layer) {
		return nullptr;
	}
#ifdef USE_BG_PROJECT_LOAD
	layer->_projectFilePath = projectFilePath;
#endif

    // add layer as a child to scene
    scene->addChild(layer);

    // return the scene
    return scene;
}

bool LogoScene::init(const std::string &locale)
{
    if ( !Layer::init() )
    {
        return false;
    }
    
// #AGTK-NX
#ifdef USE_LOGO_ACT2_6098
	auto glView = Director::getInstance()->getOpenGLView();
#ifndef USE_BG_PROJECT_LOAD
	orgDesignSize = glView->getDesignResolutionSize();
	orgPolicy = glView->getResolutionPolicy();
	orgFrameZoomFactor = glView->getFrameZoomFactor();
#endif
	glView->setFrameZoomFactor(1.0f); // setDesignResolutionSize()でviewportが更新されるためZoomFactorを先に更新
	glView->setDesignResolutionSize(NX_DEFAULT_FRAME_WIDTH, NX_DEFAULT_FRAME_HEIGHT, ResolutionPolicy::NO_ADJUST);

	auto visibleSize = Director::getInstance()->getVisibleSize();
	Vec2 origin = Director::getInstance()->getVisibleOrigin();

	std::vector<string> orgSearchPaths = FileUtils::getInstance()->getSearchPaths();
	FileUtils::getInstance()->addSearchPath("rom:/");
	auto fileUtils = static_cast<FileUtilsRuntime *>(FileUtils::getInstance());
	std::string key = fileUtils->key();
	fileUtils->setKey("");

	auto createSprite = [&](std::string fullpath, Sprite** outSprite, const cocos2d::Rect* rect = nullptr) {
		cocos2d::Sprite* sprite = nullptr;
		if (!rect) {
			sprite = Sprite::create(fullpath);
		}
		else {
			sprite = Sprite::create(fullpath, *rect);
		}
		if (!sprite) {
			return false;
		}
		float scale = 1.0f;
		auto size = sprite->getContentSize();
		if (size.width > visibleSize.width || size.height > visibleSize.height) {
			if (size.width * visibleSize.height >= size.height * visibleSize.width) {
				scale = visibleSize.width / size.width;
			}
			else {
				scale = visibleSize.height / size.height;
			}
		}
		sprite->setScale(scale);
		sprite->setPosition(Vec2(visibleSize.width / 2 + origin.x, visibleSize.height / 2 + origin.y));
		sprite->getTexture()->setAliasTexParameters();
		sprite->setVisible(false);
		this->addChild(sprite, 0);
		*outSprite = sprite;
		return true;
	};

	std::string fileGGG = "playerResources/GGG.png";
	std::string fullpathGGG = FileUtils::getInstance()->fullPathForFilename(fileGGG);
	if (!createSprite(fullpathGGG, &gggSprite)) {
		return false;
	}

	std::string fileLogo = "playerResources/logo_1.png";
	std::string fullpathLogo = FileUtils::getInstance()->fullPathForFilename(fileLogo);
	if (!createSprite(fullpathLogo, &logoSprite)) {
		return false;
	}

#ifdef USE_BG_PROJECT_LOAD
	std::string fileloading1 = "playerResources/loading1.png";
	std::string fullpathLoading1 = FileUtils::getInstance()->fullPathForFilename(fileloading1);
	if (!createSprite(fullpathLoading1, &loadingSprite1)) {
		return false;
	}

	loadingFrameIndex = 0;
	loadingDelta = 0.0f;
	std::string fileloading2 = "playerResources/loading2.png";
	std::string fullpathLoading2 = FileUtils::getInstance()->fullPathForFilename(fileloading2);
	if (!createSprite(fullpathLoading2, &loadingSprite2, &loadingFrame[loadingFrameIndex])) {
		return false;
	}

	loadingSprite1->setPositionX(loadingSprite1->getPositionX() - loadingSprite2->getContentSize().width * 0.5f);
	loadingSprite2->setPositionX(loadingSprite2->getPositionX() + loadingSprite1->getContentSize().width * 0.5f);

	_loadStarted = false;
#endif

	registerInputListener(_eventDispatcher, this);

	fileUtils->setKey(key);
	FileUtils::getInstance()->setSearchPaths(orgSearchPaths);

	phase = 0;
	if (AppDelegate::FastBoot) {
		phase = 4;
	}

#else // USE_LOGO_ACT2_6098
	auto visibleSize = Director::getInstance()->getVisibleSize();
	Vec2 origin = Director::getInstance()->getVisibleOrigin();

	bool isFile = false;
	std::string filename = "img/logo_en_US.png";
	if (locale == "ja_JP") {
		filename = "img/logo_ja_JP.png";
	}
	std::string fullpath = FileUtils::getInstance()->fullPathForFilename(filename);
#ifdef USE_RUNTIME
	if (fullpath.size() > 0) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
		auto fileUtils = static_cast<FileUtilsRuntime *>(FileUtils::getInstance());
#else
		auto fileUtils = dynamic_cast<FileUtilsRuntime *>(FileUtils::getInstance());
#endif
		std::string key = fileUtils->key();
		fileUtils->setKey("");
		Data data = FileUtils::getInstance()->getDataFromFile(fullpath);
		fileUtils->setKey(key);
		if (data.getSize() >= 3 && memcmp(data.getBytes(), "enc", 3) == 0) {
			isFile = true;
		}
	}
	if (!isFile) {
		return false;
	}
#endif
	auto sprite = Sprite::create(fullpath);
	if (!sprite) {
		return false;
	}
	float scale = 1.0f;
	auto size = sprite->getContentSize();
	if (size.width > visibleSize.width || size.height > visibleSize.height) {
		if (size.width * visibleSize.height >= size.height * visibleSize.width) {
			scale = visibleSize.width / size.width;
		}
		else {
			scale = visibleSize.height / size.height;
		}
	}
	sprite->setScale(scale);
    sprite->setPosition(Vec2(visibleSize.width / 2 + origin.x, visibleSize.height / 2 + origin.y));
	sprite->getTexture()->setAliasTexParameters();
    this->addChild(sprite, 0);
    
	registerInputListener(_eventDispatcher, this);
#endif // USE_LOGO_ACT2_6098
	return true;
}

void LogoScene::onEnter()
{
	Layer::onEnter();
// #AGTK-NX
#ifdef USE_LOGO_ACT2_6098
	scheduleUpdateWithPriority(kSchedulePriorityScene);
#else // USE_LOGO_ACT2_6098
	this->runAction(CCSequence::create(DelayTime::create(1), CCFadeTo::create(0.5f, 0), CallFunc::create([this]() {
		GameManager::getInstance()->restartCanvas();
	}), nullptr));
	scheduleUpdateWithPriority(kSchedulePriorityScene);
#endif // USE_LOGO_ACT2_6098
}

void LogoScene::update(float delta)
{
// #AGTK-NX
#ifdef USE_LOGO_ACT2_6098
	(void)delta;
	if (mLeaving) {
		return;
	}

	auto isInput = [] {
		auto inputManager = InputManager::getInstance();
		cocos2d::DictElement *el = nullptr;
		auto inputControllerList = inputManager->getInputControllerList();
		bool isInput = false;
		CCDICT_FOREACH(inputControllerList, el) {
			auto p = static_cast<InputController *>(el->getObject());
			if (p->isTriggeredOr(false)) {
				GameManager::getInstance()->actionLog(1, "pressed");
				isInput = true;
				break;
			}
		}
		if (isInput) {
			GameManager::getInstance()->actionLog(1, "isInput");
			return true;
		}
		return false;
	};
#ifdef USE_BG_PROJECT_LOAD
	auto plm = ProjectLoadingManager::getInstance();
	auto loadingFunc = [&] {
		if (loadingDelta + delta >= 0.5f) {
			loadingDelta = 0.5f;
		}
		else {
			loadingDelta += delta;
		}
		if (loadingDelta >= 0.5f) {
			loadingFrameIndex = (loadingFrameIndex + 1) % 4;
			loadingSprite2->setTextureRect(loadingFrame[loadingFrameIndex]);
			loadingDelta -= 0.5f;
		}

		plm->restart();
		do {
			auto elapse = cocos2d::Director::getInstance()->getTimeFromSwapBuffer();
			if (elapse < 1.0f * 0.8f / 60.0f) {
				if (plm->execOneTask()) {
					continue;
				}
			}
			break;
		} while (true);

		auto elapse = cocos2d::Director::getInstance()->getTimeFromSwapBuffer();
		if (elapse > 1.0f * 0.8f / 60.0f) {
			plm->pause();
			while (!plm->isPaused()) {
				std::this_thread::sleep_for(10us);
			}
		}
	};
#endif
	switch (phase)
	{
		case 0:
			// ggg 表示開始
			gggSprite->setVisible(true);
			gggSprite->runAction(CCSequence::create(DelayTime::create(3), CCFadeTo::create(1.0f, 0), CallFunc::create([this]() {
				if (phase == 1) {
					gggSprite->setVisible(false);
					phase++;
				}
			}), nullptr));
			phase++;
			break;
		case 1:
			// ggg スキップ
			if (isInput()) {
				gggSprite->stopAllActions();
				gggSprite->runAction(CCSequence::create(CCFadeTo::create(0.25f, 0), CallFunc::create([this]() {
					gggSprite->setVisible(false);
					phase++;
				}), nullptr));
			}
			break;
		case 2:
			// logo 表示開始
			logoSprite->setVisible(true);
			logoSprite->runAction(CCSequence::create(DelayTime::create(3), CCFadeTo::create(1.0f, 0), CallFunc::create([this]() {
				if (phase == 3) {
					logoSprite->setVisible(false);
					phase++;
				}
			}), nullptr));
			phase++;
			break;
		case 3:
			// logo スキップ
			if (isInput()) {
				logoSprite->stopAllActions();
				logoSprite->runAction(CCSequence::create(CCFadeTo::create(0.25f, 0), CallFunc::create([this]() {
					logoSprite->setVisible(false);
					phase++;
				}), nullptr));
			}
			break;
#ifdef USE_BG_PROJECT_LOAD
		case 4:
			// wait 表示開始
			loadingSprite1->setVisible(true);
			loadingSprite2->setVisible(true);
			phase++;
			break;
		case 5:
			// プロジェクトデータのロードを開始
			{
				if (!_loadStarted) {
					_loadStarted = true;
// #AGTK-NX
#if (CC_TARGET_PLATFORM == CC_PLATFORM_NX)
#endif
#ifdef USE_PRELOAD_TEX
					GameManager::getInstance()->loadProjectData(_projectFilePath);
#else
					plm->load(_projectFilePath);
#endif
				}
				if(!plm->isDone()) {
					loadingFunc();
				}
				else {
#ifdef USE_PRELOAD_TEX
					plm->postProcessPreloadTex();
#else
					plm->join();
// #AGTK-NX
#if (CC_TARGET_PLATFORM == CC_PLATFORM_NX)
#endif
#endif
					phase++;
				}
			}
			break;
		case 6:
#ifdef USE_PRELOAD_TEX
			{
				auto plm = ProjectLoadingManager::getInstance();
				if (!plm->isDone()) {
					loadingFunc();
				}
				else {
					plm->join();
					plm->postProcessCacheTex();
// #AGTK-NX
#if (CC_TARGET_PLATFORM == CC_PLATFORM_NX)
#endif
					phase++;
				}
			}
			break;
		case 7:
#endif
#else		
		case 4:
#endif
			{
// #AGTK-NX
#ifdef USE_BG_PROJECT_LOAD
				ApplyProjectData();
#endif
				GameManager::getInstance()->restartCanvas();
				mLeaving = true;
// #AGTK-NX
#ifndef USE_BG_PROJECT_LOAD
				auto glView = Director::getInstance()->getOpenGLView();
				glView->setFrameZoomFactor(orgFrameZoomFactor);
				glView->setDesignResolutionSize(orgDesignSize.width, orgDesignSize.height, orgPolicy);
#endif
			}
			break;
	}
#else // USE_LOGO_ACT2_6098
	(void)delta;
	if (mLeaving) {
		return;
	}
	auto inputManager = InputManager::getInstance();
	cocos2d::DictElement *el = nullptr;
	auto inputControllerList = inputManager->getInputControllerList();
	bool isInput = false;
	CCDICT_FOREACH(inputControllerList, el) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
		auto p = static_cast<InputController *>(el->getObject());
#else
		auto p = dynamic_cast<InputController *>(el->getObject());
#endif
		if (p->isPressedOr(false)) {
			GameManager::getInstance()->actionLog(1, "pressed");
			isInput = true;
			break;
		}
	}
	if (isInput) {
		GameManager::getInstance()->actionLog(1, "isInput");
		mLeaving = true;
 		this->stopAllActions();
		this->runAction(CCSequence::create(CCFadeTo::create(0.25f, 0), CallFunc::create([this]() {
			GameManager::getInstance()->restartCanvas();
		}), nullptr));
	}
#endif // USE_LOGO_ACT2_6098
}

// #AGTK-NX
#ifdef USE_BG_PROJECT_LOAD
void LogoScene::ApplyProjectData()
{
	auto gm = GameManager::getInstance();

	auto im = InputManager::getInstance();
	auto tmpNxFrameCount = im->getNxFrameCount();
	auto tmpIsNxStart = im->getIsNxStart();
	auto tmpIsNxForceStart = im->getIsNxForceStart();
	auto tmpIsNxHandheld = im->getIsNxHandheldMode();
	auto tmpIsNxSelectChanged = im->getIsNxSelectChanged();

	AGTK_ACTION_LOG(1, "# InputManager Init");
	im->purge(); // 初期化済みなので一旦破棄
	im = InputManager::getInstance(); // 一度破棄しているのでインスタンスの再取得
	im->init(gm->getProjectData()->getInputMapping());
	im->setIsNxEnable(true); // コントローラーサポートアプレット処理有効化
	im->setNxFrameCount(tmpNxFrameCount);
	im->setIsNxStart(tmpIsNxStart);
	im->setIsNxForceStart(tmpIsNxForceStart);
	im->setIsNxHandheldMode(tmpIsNxHandheld);
	im->setIsNxSelectChanged(tmpIsNxSelectChanged);

	AGTK_ACTION_LOG(1, "# AudioManager Init");
	AudioManager::getInstance()->init(gm->getProjectData()->getSoundSetting());

	auto projectData = gm->getProjectData();
	auto screenSize = projectData->getScreenSize();
	float frameZoomFactor = projectData->getMagnifyWindow() ? projectData->getWindowMagnification() : 1.0f;
#if (CC_TARGET_PLATFORM == CC_PLATFORM_NX) // #AGTK-NX
#endif

	auto glview = Director::getInstance()->getOpenGLView();
#if (CC_TARGET_PLATFORM == CC_PLATFORM_WIN32) || (CC_TARGET_PLATFORM == CC_PLATFORM_MAC)
// #AGTK-NX
#elif (CC_TARGET_PLATFORM == CC_PLATFORM_NX)
#endif
	glview->setFrameZoomFactor(frameZoomFactor);
	glview->setDesignResolutionSize(screenSize.width, screenSize.height, resolutionPolicy);

	//resolution size
	float scaleFactor = 1.0f;
#if (CC_TARGET_PLATFORM != CC_PLATFORM_NX) // #AGTK-NX
	glview->setDesignResolutionSize(screenSize.width, screenSize.height, ResolutionPolicy::SHOW_ALL);
#else
#endif
	auto director = Director::getInstance();
	director->setContentScaleFactor(scaleFactor);

	//cursor
	glview->setCursorVisible(projectData->getDisplayMousePointer());

	if ((bool)projectData->getScreenSettings() != agtk::IsFullScreen()) {
		AGTK_ACTION_LOG(1, "# ChangeScreen:%d", projectData->getScreenSettings());
		agtk::ChangeScreen(projectData->getScreenSettings());
	}

	//register_all_packages();

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

	// disableLogo が true == ロゴを非表示 == ロゴスキップ
	AppDelegate::FastBoot = projectData->getDisableLogo();

	auto inputManager = InputManager::getInstance();
	inputManager->setInputMappingData(projectData->getInputMapping());
	agtk::setTileCollisionThreshold(projectData->getWallDetectionOverlapMargin());
	if (projectData->getMultithreading()) {
		ThreadManager::setUsedThreadCount(MAX(1, MIN(2, AGTK_THREAD_COUNT)));
	}
	else {
		ThreadManager::setUsedThreadCount(1);
	}

// #AGTK-NX
#if (CC_TARGET_PLATFORM == CC_PLATFORM_NX)
#endif

	// GameManagerのprojectDataに依存する処理を有効化
	GameManager::getInstance()->setEnableProject(true);
}
#endif

#if 0
// create a scene. it's an autorelease object
//GameManager::getInstance()->setNextSceneId(0);
//auto scene = GameScene::createScene();
//    auto scene = MainScene::createScene();
GameManager::getInstance()->restartCanvas();
#endif
