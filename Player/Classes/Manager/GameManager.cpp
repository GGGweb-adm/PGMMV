#include "GameManager.h"
#include "DebugManager.h"

#include "AppMacros.h"
#include "Manager/AudioManager.h"
#include "json/stringbuffer.h"
#include "json/writer.h"
#include "json/prettywriter.h"
#include "json/document.h"

#ifdef USE_PREVIEW
#include "Manager/ParticleManager.h"
#endif
#include "Manager/MovieManager.h"
#include "Manager/ImageManager.h"
#include "Manager/EffectManager.h"
#include "Manager/BulletManager.h"
#include "Manager/GuiManager.h"
#include "Manager/FontManager.h"
#ifdef USE_PREVIEW
#include "Lib/SharedMemory.h"
#include "Manager/DebugManager.h"
#include "Lib/ObjectCommand.h"
#endif
#include "Lib/PhysicsObject.h"
#include "Manager/JavascriptManager.h"
#include "scripting/js-bindings/manual/ScriptingCore.h"
#include "Manager/DllPluginManager.h"
#include "scripting/js-bindings/manual/ScriptingCore.h"

#include "External/collision/CollisionComponent.hpp"
#include "Lib/Collision.h"
#include "Lib/Object.h"

#include "renderer/CCGLProgramCache.h"
#include "renderer/CCGLProgramStateCache.h"
#include "Manager/ThreadManager.h"
#ifdef USE_RUNTIME
#include "Lib/Runtime/FileUtils-runtime.h"
#endif

// #AGTK-NX
#if (CC_TARGET_PLATFORM == CC_PLATFORM_NX)
#endif

// #AGTK-NX #AGTK-WIN
#ifdef USE_PRELOAD_TEX
#include <Manager/ProjectLoadingManager.h>
#endif

#ifdef USE_SAR_OHNISHI_DEBUG
int g_debugObjectId = 43;
int g_debugCurObjectId = 0;
int g_debugIntVal = 0;
Node* g_debugNode = nullptr;
#endif

namespace {
	namespace jsonUtil {
		// 安全に json から int のメンバーを返す
		int getInt(const rapidjson::Value& json, char const * name, int withoutMember = 0) {
			if (json.HasMember(name)) {
				return json[name].GetInt();
			}
			return withoutMember;
		}
	}
}

#if defined(USE_PREVIEW) || defined(AGTK_DEBUG)
	int GameManager::mPort = 1234;
	char GameManager::mHostname[64] = "localhost";
#endif
#if defined(USE_PREVIEW)
	FILE *GameManager::mScriptLogFp = nullptr;
#endif
	int GameManager::mLogNumber = 0;

	static const char* const SAVE_FOLDER = "save/";

	std::map<std::string, std::map<std::string, std::vector<GameManager::LocaleText>>> GameManager::_translationMap;

GameManager* GameManager::_gameManager = NULL;
GameManager::GameManager()
{
	_projectData = nullptr;
	_playData = nullptr;
	_currentScene = nullptr;
	_loadingScene = nullptr;
	_currentLayer = nullptr;
	_prevSceneId = -1;
	_nextSceneId = -1;
	_loadBit = kLoadBit_All;
	_loadEffectType = agtk::data::ObjectCommandFileLoadData::kEffectTypeNone;
	_loadEffectDuration300 = 300;
	_needTerminateScene = false;
	_needPortalMove = false;
	_needSceneChange = false;
	_portalData = nullptr;
	_portalTouchedPlayerList = nullptr;
#ifdef FIX_ACT2_4774
	_transitionPortalId = -1;
#endif
	_isPortalMoving = false;
	_isSceneMoving = false;
	_sceneChangeReappearObjectList = nullptr;
	_notReappearObjectList = nullptr;
	_commandReappearObjectList = nullptr;
	_sceneEndTakeoverStatesObjectList = nullptr;

	_projectFilePath = nullptr;
#ifdef USE_PREVIEW
	_previewMode = nullptr;
#endif
#ifdef USE_PREVIEW
	_webSocket = nullptr;
	_bTerminate = false;
#endif
#ifdef USE_PREVIEW
	_sharedMemory = nullptr;
#endif

	_physicsWorld = nullptr;
	_airResistance = 0.0f;
	_alivePhysicsObj = false;
	_timer = nullptr;

	_isBgmFadeOut = false;

	_preMoveBgmId = -1;
	_preMoveBgmLoopFlag = false;

	_loadSceneId = -1;
	_stateLoadResources = EnumStateLoadResources::NONE;
	_loadingTimer = nullptr;
	_stateLoading = EnumStateLoading::kStateLoadingNone;
	_appName = nullptr;
	_appVersion = nullptr;
#ifdef USE_PREVIEW
	_autoTestReplayFilePath = "";
	_autoTestPluginFilePath = "";
	_autoTestPluginInternalJson = "";
	_autoTestPluginParamValueJson = "";
	_startSceneFlg = false;
	_autoTestRestartFlg = false;
#endif
	_editorLocale = "en_US";

	_ignoreCreatePlayerList = nullptr;
	_preloadBgmEnd.clear();
	_frameRate = FRAME60_RATE;
// #AGTK-NX #AGTK-WIN
#ifdef USE_30FPS
	_frameProgressScale = 1.0f;
#endif
#ifdef USE_PREVIEW
	Application::setDropFileCallback(dropFileHandler);
#endif
#ifdef FIX_ACT2_5233
	_moveToLayerId = -1;
#endif
#ifdef USE_RUNTIME
	_encryptSaveFile = false;
#endif
// #AGTK-NX
#ifdef USE_SAR_OPTIMIZE_4
	_switchWatcherObjectList = nullptr;
#endif
// #AGTK-NX
#if (CC_TARGET_PLATFORM == CC_PLATFORM_NX)
#endif
// #AGTK-NX
#ifdef USE_BG_PROJECT_LOAD
	_enableProject = false;
#endif
// #AGTK-NX #AGTK-WIN
#ifdef USE_30FPS_3
	_passIndex = 1;
	_passCount = 1;
	_bLastPass = true;
#endif
}

GameManager::~GameManager()
{
	CC_SAFE_RELEASE_NULL(_ignoreCreatePlayerList);
	CC_SAFE_RELEASE_NULL(_notReappearObjectList);
	CC_SAFE_RELEASE_NULL(_commandReappearObjectList);
	CC_SAFE_RELEASE_NULL(_sceneChangeReappearObjectList);
	CC_SAFE_RELEASE_NULL(_sceneEndTakeoverStatesObjectList);
	CC_SAFE_RELEASE_NULL(_portalTouchedPlayerList);
	CC_SAFE_RELEASE_NULL(_portalData);
	CC_SAFE_RELEASE_NULL(_playData);
	CC_SAFE_RELEASE_NULL(_projectData);
	CC_SAFE_RELEASE_NULL(_loadingScene);
	CC_SAFE_RELEASE_NULL(_currentScene);
	CC_SAFE_RELEASE_NULL(_currentLayer);
	CC_SAFE_RELEASE_NULL(_projectFilePath);
#ifdef USE_PREVIEW
	CC_SAFE_RELEASE_NULL(_previewMode);
#endif
#ifdef USE_PREVIEW
	CC_SAFE_RELEASE_NULL(_webSocket);
#endif
#ifdef USE_PREVIEW
	CC_SAFE_DELETE(_sharedMemory);
#endif
	CC_SAFE_RELEASE_NULL(_timer);
	CC_SAFE_RELEASE_NULL(_loadingTimer);
// #AGTK-NX
#ifdef USE_SAR_OPTIMIZE_4
	CC_SAFE_DELETE(_switchWatcherObjectList);
#endif
}

GameManager* GameManager::getInstance()
{
	if (!_gameManager) {
		_gameManager = new GameManager();

		// スケジューラーを登録する。
		Scheduler *s = Director::getInstance()->getScheduler();
		s->scheduleUpdate(_gameManager, kSchedulePriorityGameManager, false);

		// 前の時間
		_gameManager->_prevTime = (double)time(NULL);

#ifdef USE_PREVIEW
		_gameManager->createWebsocket(mHostname, mPort);
#endif
#ifdef USE_PREVIEW
		_gameManager->setSharedMemory(new agtk::SharedMemory());
#endif
#if defined(USE_PREVIEW)
		auto fileUtils = FileUtils::getInstance();
		mScriptLogFp = fopen(fileUtils->getSuitableFOpen(fileUtils->getApplicationPath() + "/script.log").c_str(), "wb");
#endif

		// リストを初期化
		_gameManager->setSceneChangeReappearObjectList(cocos2d::Array::create());
		_gameManager->setNotReappearObjectList(cocos2d::Array::create());
		_gameManager->setCommandReappearObjectList(cocos2d::__Array::create());
		_gameManager->setSceneEndTakeoverStatesObjectList(cocos2d::__Array::create());
		_gameManager->setIgnoreCreatePlayerList(cocos2d::__Array::create());

		_gameManager->setTimer(agtk::Timer::create());
		_gameManager->getTimer()->start();
		_gameManager->setLoadingTimer(agtk::Timer::create());
// #AGTK-NX
#ifdef USE_SAR_OPTIMIZE_4
		_gameManager->setSwitchWatcherObjectList(new SwitchWatcherObjectDic());
#endif

		// セーブデータ用パスを生成 ※ゲームをビルドの設定で「ゲームデータと同じフォルダにセーブデータを作成」がチェックされている場合には使われないことに注意。
#ifdef USE_RUNTIME
		// 書き込み可能なディレクトリのパスを取得
		auto writablePath = FileUtils::getInstance()->getWritablePath();
#ifdef WIN32
		// 実行ファイルが存在するディレクトリ名を取得
		auto appPath = FileUtils::getInstance()->getApplicationPath();
		appPath.pop_back();
		appPath.replace(0, appPath.find_last_of('/') + 1, "");
		appPath.push_back('/');

		// <APP_NAME>/ を除去
		writablePath.pop_back();
		auto idx = writablePath.find_last_of('/');
		writablePath.replace(idx, writablePath.length() - idx, "/");
#endif
// #AGTK-NX
#if (CC_TARGET_PLATFORM == CC_PLATFORM_NX)
#else
		_gameManager->_saveDataPath = writablePath + appPath + SAVE_FOLDER;
#endif	// (CC_TARGET_PLATFORM == CC_PLATFORM_NX)
#else
		_gameManager->_saveDataPath = FileUtils::getInstance()->getApplicationPath() + SAVE_FOLDER;
#endif
		THREAD_PRINTF("_gameManager: 0x%x", _gameManager);
	}
	return _gameManager;
}

void GameManager::purge()
{
	_translationMap.clear();
	if (!_gameManager) {
		return;
	}

	if (_gameManager->_currentScene != nullptr) {
		_gameManager->_currentScene->end(false);
	}
	_gameManager->finalPlugins();

	CC_SAFE_RELEASE_NULL(_gameManager->_currentLayer);

	// autoreleaseされているオブジェクトを強制的にリリースする。
	PoolManager::getInstance()->getCurrentPool()->clear();

	// パーティクルマネージャをパージ
	ParticleManager::getInstance()->purge();

	// effectマネージャをパージ
	EffectManager::getInstance()->purge();

	// bulletマネージャをパージ
	BulletManager::getInstance()->purge();

	// imageマネージャをパージ
	ImageManager::getInstance()->purge();

	// movieマネージャをパージ
	MovieManager::getInstance()->purge();

	// debugマネージャをパージ
	agtk::DebugManager::getInstance()->purge();

	// Primitiveマネージャをパージ
	PrimitiveManager::getInstance()->removeAll();
	PrimitiveManager::getInstance()->purge();

	// インプットマネージャをパージ
	InputManager::getInstance()->purge();

	// GUIマネージャをパージ
	GuiManager::getInstance()->purge();

	// Fontマネージャをパージ
	FontManager::getInstance()->purge();

	// DLLプラグインマネージャをパージ
	DllPluginManager::purge();

#if (CC_TARGET_PLATFORM == CC_PLATFORM_NX)
#endif

#ifdef USE_MULTITHREAD_OBJECT_UPDATE
	ThreadManager::purge();
#endif
	if (ScriptingCore::isCreateInstance()) {
		ScriptingCore* sc = ScriptingCore::getInstance();
		sc->cleanup();
	}
#if defined(USE_PREVIEW)
	if (mScriptLogFp) {
		fclose(mScriptLogFp);
		mScriptLogFp = nullptr;
	}
#endif
#ifdef USE_PREVIEW
	Application::setDropFileCallback(nullptr);
#endif

	// スケジューラーを停止する。
	Scheduler *s = Director::getInstance()->getScheduler();
	s->unscheduleUpdate(_gameManager);

	GameManager *gm = _gameManager;
	_gameManager = NULL;
	gm->release();
}

//--------------------------------------------------------------------------------------------------------------------
// スケジュールで呼び出されるメソッド
//--------------------------------------------------------------------------------------------------------------------
void GameManager::update(float delta)
{
#ifdef USE_BG_PROJECT_LOAD
	if (!getEnableProject()) {
		return;
	}
#endif
	//CCLOG("delta:%f", delta);
	// 経過時間を取得する。
	double nowTime = (double)time(NULL);
	double passedTime = nowTime - _prevTime;

	this->getTimer()->laptime();
	this->getLoadingTimer()->laptime();

	this->updateSceneState();

	this->updateLoadResources();
#if 0//ファイルに変更があったかチェックする処理。
	for (int type = 0; type < JsonFileTypes::kJsonFileMAX; type++) {
		auto utime = FileUtils::getInstance()->getLastWriteTime(std::string("data/") + std::string(jsonList[type]) + std::string(".json"));
		if (g_updateTime[type] != utime) {
			CCLOG("%s", jsonList[type]);
			g_updateTime[type] = utime;
		}
	}
#endif

	// 経過時間を加算する。
	_currentTime += passedTime;
	_prevTime = nowTime;

#ifdef USE_PREVIEW
	if (_bTerminate) {
		bool closed = true;
		if (this->getWebSocket()) {
			int state = this->getWebSocket()->getState();
			if (state == 1) {
				//OPEN
				closed = false;
				this->removeWebsocket();
			}
			else if (state != 3) {
				// not CLOSED
				//待つ
				closed = false;
			}
		}
		if (closed) {
			auto director = Director::getInstance();
			director->getOpenGLView()->end();
		}
	}
#endif
#ifdef USE_PREVIEW
	ParticleManager::getInstance()->update(delta, this);
#endif
	DllPluginManager::getInstance()->updatePlugins(delta);
	JavascriptManager::updatePlugins(delta);
#if defined(USE_PREVIEW)
	if (mScriptLogFp) {
		fflush(mScriptLogFp);
	}
#endif
	if (!isSceneChanging()) {
		_frame++;
	}
}

void GameManager::setCursorVisible(bool visible)
{
	auto glview = Director::getInstance()->getOpenGLView();
	glview->setCursorVisible(visible);
}


void GameManager::removeCommandReappearObjectList(bool bWithoutScenePartObjectOnly)
{
lRetry:
	auto commandReappearObjectList = this->getCommandReappearObjectList();
	if (commandReappearObjectList->count() > 0) {
		//シーン配置以外のオブジェクト。
		if (bWithoutScenePartObjectOnly) {
			cocos2d::Ref *ref;
			CCARRAY_FOREACH(commandReappearObjectList, ref) {
				auto reappearData = dynamic_cast<agtk::data::ObjectReappearData*>(ref);
				if (reappearData->getScenePartsId() <= 0) {
					commandReappearObjectList->removeObject(reappearData);
					goto lRetry;
				}
			}
		}
		else {
			//すべて破棄。
			commandReappearObjectList->removeAllObjects();
		}
	}
}

//--------------------------------------------------------------------------------------------------------------------
// 現在の時間(tm)を取得する。
//--------------------------------------------------------------------------------------------------------------------
struct tm *GameManager::getCurrentTm()
{
	return NULL;
	//	return convertEpochToTm(this->_currentTime, true);
}

//--------------------------------------------------------------------------------------------------------------------
// 最後にタイトルにログインした時間(tm)を取得する。
//--------------------------------------------------------------------------------------------------------------------
struct tm *GameManager::getLastLoginTm()
{
	return NULL;
	//	return convertEpochToTm(this->_lastLoginTime, true);
}

#include "GameScene.h"
#include "LoadingScene.h"
#include "ViewerScene.h"

void GameManager::startCanvas(bool bLoading)
{
	int nextSceneId = 0;
	GameManager::getInstance()->setNextSceneId(nextSceneId);
	auto director = Director::getInstance();
	auto scene = createCanvas(nextSceneId, bLoading);

	// run
	director->runWithScene(scene);
}

void GameManager::restartCanvas(bool bLoading)
{
	int nextSceneId = 0;
	GameManager::getInstance()->setNextSceneId(nextSceneId);
	auto director = Director::getInstance();
	auto scene = createCanvas(nextSceneId, bLoading);
	director->stopAnimation();
	//director->pushScene(scene);
	director->replaceScene(scene);
	director->startAnimation();
}

cocos2d::Scene *GameManager::createCanvas(int id, bool bLoading)
{
	//auto scene = Scene::create();
	auto scene = Scene::createWithPhysics();
	CCASSERT(scene, "failed crate scene");

	// 物理演算ワールドの設定
	auto pworld = scene->getPhysicsWorld();
	pworld->setIterations(DEFAULT_PHYSICS_ITERATION * 3);	// イテレーション回数初期値をデフォルトの3倍に設定
	pworld->setGravity(Vec2::ZERO);							// デフォルト重力は0
	pworld->setAutoStep(false);								// 自動更新をOFF
	this->_physicsWorld = pworld;

	cocos2d::Layer *layer = nullptr;
	if (bLoading) {
		layer = LoadingScene::create();
	}
	else {
		layer = GameScene::create(id);
		//		layer = ViewerScene::create(id);
	}

	// 衝突コールバックを設定
	auto contactListener = cocos2d::EventListenerPhysicsContact::create();
	contactListener->onContactBegin = CC_CALLBACK_1(GameManager::onContactBeginCallback, this);
	contactListener->onContactPreSolve = CC_CALLBACK_2(GameManager::onContactPreSolveCallback, this);
	contactListener->onContactSeparate = CC_CALLBACK_1(GameManager::onContactSeparateCallback, this);
	scene->getEventDispatcher()->addEventListenerWithSceneGraphPriority(contactListener, scene);

	CCASSERT(layer, "failed crate layer.");
	scene->addChild(layer);
#ifdef USE_DESIGNATED_RESOLUTION_RENDER
	if (!bLoading) {
		auto gameScene = dynamic_cast<GameScene *>(layer);
		if (gameScene) {
			auto renderTexture = gameScene->getRenderTexture();
			if (renderTexture) {
				scene->addChild(renderTexture);
			}
		}
	}
#endif

	this->setCurrentLayer(layer);
	//ロード画面生成。
	this->createLoadingScene();
	return scene;
}

void GameManager::changeCanvas(int nextSceneId, bool bLoading)
{
	GameManager::getInstance()->setNextSceneId(nextSceneId);
	int prevSceneId = GameManager::getInstance()->getPrevSceneId();

	auto scene = createCanvas(nextSceneId, bLoading);

#if 0
	agtk::Scene::Fade fadeType = GameManager::getInstance()->getSceneList()->getFadeOut(prevSceneId).type;
	if (fadeType == agtk::Scene::Fade::TypeNone) {
		Director::getInstance()->replaceScene(scene);
	}
	else if (fadeType == agtk::Scene::Fade::TypeBlack) {
		TransitionFade* fade = TransitionFade::create(0.5f, scene, Color3B::BLACK);
		Director::getInstance()->replaceScene(fade);
	}
	else if (fadeType == agtk::Scene::Fade::TypeWhite) {
		TransitionFade* fade = TransitionFade::create(0.5f, scene, Color3B::WHITE);
		Director::getInstance()->replaceScene(fade);
	}
	else {
		CC_ASSERT(0);
	}
#endif
	//入力情報をリセットする。
	InputManager::getInstance()->reset();
}

void GameManager::changeCanvas()
{
	int nextSceneId = GameManager::getInstance()->getNextSceneId();
	int prevSceneId = GameManager::getInstance()->getPrevSceneId();

	auto scene = createCanvas(nextSceneId, false);

#if 0
	agtk::Scene::Fade fadeType = GameManager::getInstance()->getSceneList()->getFadeOut(prevSceneId).type;
	if (fadeType == agtk::Scene::Fade::TypeNone) {
		Director::getInstance()->replaceScene(scene);
	}
	else if (fadeType == agtk::Scene::Fade::TypeBlack) {
		TransitionFade* fade = TransitionFade::create(0.5f, scene, Color3B::BLACK);
		Director::getInstance()->replaceScene(fade);
	}
	else if (fadeType == agtk::Scene::Fade::TypeWhite) {
		TransitionFade* fade = TransitionFade::create(0.5f, scene, Color3B::WHITE);
		Director::getInstance()->replaceScene(fade);
	}
	else {
		CC_ASSERT(0);
	}
#endif
	//入力情報をリセットする。
	InputManager::getInstance()->reset();
}

void GameManager::createLoadingScene()
{
	auto projectData = this->getProjectData();
	int loadingSceneId = projectData->getLoadingSceneId();
	if (loadingSceneId < 0) {
		return;
	}
	auto loadingScene = agtk::Scene::create(projectData->getSceneData(loadingSceneId), -1, agtk::Scene::EnumCreateType::kCreateTypeNone);
	this->setLoadingScene(loadingScene);
}

void GameManager::removeLoadingScene()
{
	auto layer = dynamic_cast<agtk::BaseLayer *>(this->getCurrentLayer());
	if (layer) {
		auto loadingScene = this->getLoadingScene();
		layer->detachScene(loadingScene);
	}
	this->setLoadingScene(nullptr);
}

//--------------------------------------------------------------------------------------------------------------------
// jsonファイルを読み込む。
//--------------------------------------------------------------------------------------------------------------------
/*void GameManager::loadMasterFile()
{
	for (int i = 0; i < kMasterMAX;i++)
	{
		MasterFileTypes type = (MasterFileTypes)i;
		std::string jsonFilePath = this->getMasterJsonFilePath(type);
		JsonMapType::iterator ret = this->_jsonMap.find(jsonFilePath);
		if (ret != _jsonMap.end())
		{
			switch(type)
			{
				case kMasterBanner:
				{
					// バナー
					BannerInfos *bannerInfos = BannerInfos::deserialize(jsonFilePath);
					this->setBanners(bannerInfos);
					break;
				}
				case kMasterBond:
				{
					// キズナ
					BondInfos *bondInfos = BondInfos::deserialize(jsonFilePath);
					this->setBonds(bondInfos);
					break;
				}
				case kMasterBondQuestDetail:
				{
					// キズナクエスト
					BondQuestInfos *bondQuestInfos = BondQuestInfos::deserialize(jsonFilePath);
					this->setBondQuests(bondQuestInfos);
					break;
				}
				case kMasterCharacter:
				{
					// キャラクター
					CharacterInfos *characterInfos = CharacterInfos::deserialize(jsonFilePath);
					this->setCharacters(characterInfos);
					break;
				}
				case kMasterCharacterExperience:
				{
					// キャラクター経験値
					CharacterExperienceInfos *characterExperienceInfos = CharacterExperienceInfos::deserialize(jsonFilePath);
					this->setCharacterExperiences(characterExperienceInfos);
					break;
				}
				case kMasterConfig:
				{
					// 設定
					ConfigInfos *configInfos = ConfigInfos::deserialize(jsonFilePath);
					this->setConfigs(configInfos);
					break;
				}
				case kMasterCollaborationGacha:
				{
					// コラボ召喚バナーなど
					CollaborationGachaInfos *collaborationGachaInfos = CollaborationGachaInfos::deserialize(jsonFilePath);
					this->setCollaborationGachaInfos(collaborationGachaInfos);
					break;
				}
				case kMasterEvolutionAdjustment:
				{
					// 進化調整
					EvolutionAdjustmentInfos *evoAdjustmentInfos = EvolutionAdjustmentInfos::deserialize(jsonFilePath);
					this->setEvoAdjustment(evoAdjustmentInfos);
					break;
				}
				case kMasterEvolutionMaterial:
				{
					// 進化素材
					EvolutionMaterialInfos *evoMaterialInfos = EvolutionMaterialInfos::deserialize(jsonFilePath);
					this->setEvoMaterial(evoMaterialInfos);
					break;
				}
				case kMasterLeaderSkill:
				{
					// リーダースキル
					LeaderSkillInfos *leaderskills = LeaderSkillInfos::deserialize(jsonFilePath);
					this->setLeaderSkills(leaderskills);
					break;
				}
				case kMasterMansion:
				{
					// お屋敷
					MansionInfos *mansionInfos = MansionInfos::deserialize(jsonFilePath);
					this->setMansionInfos(mansionInfos);
					break;
				}
				case kMasterMisson:
				{
					// ミッション
					MissionInfos *missionInfos = MissionInfos::deserialize(jsonFilePath);
					this->setMissionInfos(missionInfos);
					break;
				}
				case kMasterNormalQuestMap:
				{
					// 通常クエスト
					NormalQuestInfos *normalQuestInfos = NormalQuestInfos::deserialize(jsonFilePath);
					this->setNormalQuests(normalQuestInfos);
					break;
				}
				case kMasterExperience:
				{
					// プレイヤー経験値
					PlayerExpInfos *playerExpInfos = PlayerExpInfos::deserialize(jsonFilePath);
					this->setPlayerExps(playerExpInfos);
					break;
				}
				case kMasterSkill:
				{
					// スキル
					SkillInfos *skills = SkillInfos::deserialize(jsonFilePath);
					this->setSkills(skills);
					break;
				}
				case kMasterSkillSe:
				{
					// スキルSE
					SkillSEInfos *skillSEs = SkillSEInfos::deserialize(jsonFilePath);
					this->setSkillSEs(skillSEs);
					break;
				}
				case kMasterSkillType:
				{
					// スキルタイプ
					SkillTypeInfos *skillTypes = SkillTypeInfos::deserialize(jsonFilePath);
					this->setSkillTypes(skillTypes);
					break;
				}
				case kMasterSpecialQuest:
				{
					// スペシャルクエスト
					SpecialQuestInfos *specialQuestInfos = SpecialQuestInfos::deserialize(jsonFilePath);
					this->setSpecialQuests(specialQuestInfos);
					break;
				}
				case kMasterSaleAdjustment:
				{
					// 売却調整
					SaleAdjustmentInfos *saleAdjustmentInfos = SaleAdjustmentInfos::deserialize(jsonFilePath);
					this->setSaleAdjustment(saleAdjustmentInfos);
					break;
				}
				case kMasterSerialCode:
				{
					// シリアルコード
					SerialCodeInfos *serialCodeInfos = SerialCodeInfos::deserialize(jsonFilePath);
					this->setSerialCodeInfos(serialCodeInfos);
					break;
				}
				case kMasterSpecies:
				{
					// 種族
					SpeciesInfos *speciesInfos = SpeciesInfos::deserialize(jsonFilePath);
					this->setSpeciesInfos(speciesInfos);
					break;
				}
				case kMasterSpecialPeriod:
				{
					// スペシャル探索期間
					SpecialPeriodInfos *specialPeriodInfos = SpecialPeriodInfos::deserialize(jsonFilePath);
					this->setSpecialPeriodInfos(specialPeriodInfos);
					break;
				}
				case kMasterGemGacha:
				{
					// 召喚イベント
					GemGachaEventInfos *gemGachaEventInfos = GemGachaEventInfos::deserialize(jsonFilePath);
					this->setGemGachaEventInfos(gemGachaEventInfos);
					break;
				}
				case kMasterErrand:
				{
					// お遣い
					ErrandInfos *errandInfos = ErrandInfos::deserialize(jsonFilePath);
					this->setErrandInfos(errandInfos);
					break;
				}
				case kMasterErrandExperience:
				{
					// お遣い経験値
					ErrandExperienceInfos *errandExperienceInfos = ErrandExperienceInfos::deserialize(jsonFilePath);
					this->setErrandExperienceInfos(errandExperienceInfos);
					break;
				}
				case kMasterErrandReward:
				{
					// お遣いリワード
					ErrandRewardInfos *errandRewardInfos = ErrandRewardInfos::deserialize(jsonFilePath);
					this->setErrandRewardInfos(errandRewardInfos);
					break;
				}
				case kMasterErrandMaterialAttribute:
				{
					// お遣い素材属性
					ErrandMaterialAttributeInfos *errandMaterialAttributeInfos = ErrandMaterialAttributeInfos::deserialize(jsonFilePath);
					this->setErrandMaterialAttributeInfos(errandMaterialAttributeInfos);
					break;
				}
				case kMasterErrandMaterialSpecies:
				{
					// お遣い素材種族
					ErrandMaterialSpeciesInfos *errandMaterialSpeciesInfos = ErrandMaterialSpeciesInfos::deserialize(jsonFilePath);
					this->setErrandMaterialSpeciesInfos(errandMaterialSpeciesInfos);
					break;
				}
				case kMasterPrivilegeEffect:
				{
					// 効率アップエフェクトマスタ
					PrivilegeEffectInfos *privilegeEffectInfos = PrivilegeEffectInfos::deserialize(jsonFilePath);
					this->setPrivilegeEffectInfos(privilegeEffectInfos);
					break;
				}
				case kMasterCharacterBonusMaxStatus:
				{
					// キャラクター最大ボーナスマスタ
					CharacterBonusMaxInfos *characterBonusMaxInfos = CharacterBonusMaxInfos::deserialize(jsonFilePath);
					this->setCharacterBonusMaxInfos(characterBonusMaxInfos);
					break;
				}
				case kMasterMarquee:
				{
					// マーキーマスタ
					MarqueeInfos *marqueeInfos = MarqueeInfos::deserialize(jsonFilePath);
					this->setMarqueeInfos(marqueeInfos);
					break;
				}
				case kMasterRarity:
				{
					// レアリティマスタ
					RarityInfos *rarityInfos = RarityInfos::deserialize(jsonFilePath);
					this->setRarityInfos(rarityInfos);
					break;
				}
				case kMasterRanking:
				{
					// ランキングマスタ
					RankingInfos *rankingInfos = RankingInfos::deserialize(jsonFilePath);
					this->setRankingInfos(rankingInfos);
					break;
				}
				case kMasterRankingEnemyConsecutiveDefeat:
				{
					// 撃破回数マスタ
					RankingEnemyConsecutiveDefeatInfos *rankingEnemyConsecutiveDefeatInfos = RankingEnemyConsecutiveDefeatInfos::deserialize(jsonFilePath);
					this->setRankingEnemyConsecutiveDefeatInfos(rankingEnemyConsecutiveDefeatInfos);
					break;
				}
				case kMasterGemStore:
				{
					// 万屋マスタ
					GemStoreInfos *gemStoreInfos = GemStoreInfos::deserialize(jsonFilePath);
					this->setGemStoreInfos(gemStoreInfos);
					break;
				}
				case kMasterHighEvolution:
				{
					// 覚醒昇華マスタ
					HighEvolutionInfos *highEvolutionInfos = HighEvolutionInfos::deserialize(jsonFilePath);
					this->setHighEvolutionInfos(highEvolutionInfos);
					break;
				}
				case kMasterSkillExtension:
				{
					// スキル覚醒マスタ
					SkillExtensionInfos *skillExtensionInfos = SkillExtensionInfos::deserialize(jsonFilePath);
					this->setSkillExtensionInfos(skillExtensionInfos);
					break;
				}
				case kMasterSkillExtensionType:
				{
					// スキル覚醒タイプマスタ
					SkillExtensionTypeInfos *skillExtensionTypeInfos = SkillExtensionTypeInfos::deserialize(jsonFilePath);
					this->setSkillExtensionTypeInfos(skillExtensionTypeInfos);
					break;
				}
				case kMasterSpecialQuestType2:
				{
					// スペシャル探索タイプ2マスタ
					SpecialQuestType2Infos *specialQuestType2Infos = SpecialQuestType2Infos::deserialize(jsonFilePath);
					this->setSpecialQuestType2Infos(specialQuestType2Infos);
					break;
				}
				case kMasterSpecialQuestCondition:
				{
					// スペシャル探索条件複合マスタ
					SpecialQuestConditionInfos *specialQuestConditionInfos = SpecialQuestConditionInfos::deserialize(jsonFilePath);
					this->setSpecialQuestConditionInfos(specialQuestConditionInfos);
					break;
				}
				case kMasterSpecialQuestBaseCondition:
				{
					// スペシャル探索条件マスタ
					SpecialQuestBaseConditionInfos *specialQuestBaseConditionInfos = SpecialQuestBaseConditionInfos::deserialize(jsonFilePath);
					this->setSpecialQuestBaseConditionInfos(specialQuestBaseConditionInfos);
					break;
				}
				case kMasterExchange:
				{
					// 交換所マスタ
					ExchangeInfos *exchangeInfos = ExchangeInfos::deserialize(jsonFilePath);
					this->setExchangeInfos(exchangeInfos);
					break;
				}
				case kMasterIconImage:
				{
					// アイコン画像マスタ
					IconImageInfos *iconImageInfos = IconImageInfos::deserialize(jsonFilePath);
					this->setIconImageInfos(iconImageInfos);
					break;
				}
				case kMasterSpecialVirtue:
				{
					// スペシャル探索特効マスタ
					SpecialVirtueInfos *specialVirtueInfos = SpecialVirtueInfos::deserialize(jsonFilePath);
					this->setSpecialVirtueInfos(specialVirtueInfos);
					break;
				}
				case kMasterGachaCampaign:
				{
					// ガチャキャンペーンマスタ
					GemGachaCampaignInfos *gemGachaCampaignInfos = GemGachaCampaignInfos::deserialize(jsonFilePath);
					this->setGemGachaCampaignInfos(gemGachaCampaignInfos);
					break;
				}
				case kMasterStoreCampaign:
				{
					// ストアキャンペーンマスタ
					GemStoreCampaignInfos *gemStoreCampaignInfos = GemStoreCampaignInfos::deserialize(jsonFilePath);
					this->setGemStoreCampaignInfos(gemStoreCampaignInfos);
					break;
				}
				case kMasterItem:
				{
					// アイテムマスタ
					ItemInfos *itemInfos = ItemInfos::deserialize(jsonFilePath);
					this->setItemInfos(itemInfos);
					break;
				}
				case kMasterSpecialQuestSeals:
				{
					// 封印探索マスタ
					SpecialQuestSealsInfos *specialQuestSealsInfos = SpecialQuestSealsInfos::deserialize(jsonFilePath);
					this->setSpecialQuestSealsInfos(specialQuestSealsInfos);
					break;
				}
				case kMasterSpecialQuestSealsDetail:
				{
					// 封印探索詳細マスタ
					SpecialQuestSealsDetailInfos *specialQuestSealsDetailInfos = SpecialQuestSealsDetailInfos::deserialize(jsonFilePath);
					this->setSpecialQuestSealsDetailInfos(specialQuestSealsDetailInfos);
					break;
				}
				case kMasterStoreMessage:
				{
					// 交換所セリフマスタ
					StoreMessageInfos *storeMessageInfos = StoreMessageInfos::deserialize(jsonFilePath);
					this->setStoreMessageInfos(storeMessageInfos);
					break;
				}
				case kMasterGachaCharm:
				{
					// 召喚チャームマスタ
					GachaCharmInfos *gachaCharmInfos = GachaCharmInfos::deserialize(jsonFilePath);
					this->setGachaCharmInfos(gachaCharmInfos);
					break;
				}
				case kMasterRaidMapping:
				{
					// レイドマッピングマスタ
					RaidMappingInfos *infos = RaidMappingInfos::deserialize(jsonFilePath);
					this->setRaidMappingInfos(infos);
					break;
				}
				case kMasterRaidQuest:
				{
					// レイド探索マスタ
					RaidQuestInfos *infos = RaidQuestInfos::deserialize(jsonFilePath);
					this->setRaidQuestInfos(infos);
					break;
				}
				case kMasterRaidQuestDetail:
				{
					// レイド探索詳細マスタ
					RaidQuestDetailInfos *infos = RaidQuestDetailInfos::deserialize(jsonFilePath);
					this->setRaidQuestDetailInfos(infos);
					break;
				}
				case kMasterRaidScript:
				{
					// レイドスクリプトマスタ
					RaidScriptInfos *infos = RaidScriptInfos::deserialize(jsonFilePath);
					this->setRaidScriptInfos(infos);
					break;
				}
				case kMasterRaidBoss:
				{
					// レイドボスマスタ
					RaidBossInfos *infos = RaidBossInfos::deserialize(jsonFilePath);
					this->setRaidBossInfos(infos);
					break;
				}
				case kMasterGroupingItems:
				{
					// アイテムグループマスタ
					GroupItemsInfos *infos = GroupItemsInfos::deserialize(jsonFilePath);
					this->setGroupItemsInfos(infos);
					break;
				}
				case kMasterTicketGacha:
				{
					// チケット召喚マスタ
					MasterTicketGacha *master = MasterTicketGacha::deserialize(jsonFilePath);
					this->setMasterTicketGacha(master);
					break;
				}
				case kMasterSpecialQuestArea:
				{
					// 成長譚エリアマスタ
					MasterSpecialQuestArea *master = MasterSpecialQuestArea::deserialize(jsonFilePath);
					this->setMasterSpecialQuestArea(master);
					break;
				}
				case kMasterSpecialQuestAreaTop:
				{
					// 成長譚TOPマスタ
					MasterSpecialQuestAreaTop *master = MasterSpecialQuestAreaTop::deserialize(jsonFilePath);
					this->setMasterSpecialQuestAreaTop(master);
					break;
				}
				case kMasterSpecialQuestAreaScript:
				{
					// 成長譚スクリプトマスタ
					MasterSpecialQuestAreaScript *master = MasterSpecialQuestAreaScript::deserialize(jsonFilePath);
					this->setMasterSpecialQuestAreaScript(master);
					break;
				}
				case kMasterSpecialQuestAreaScriptDetail:
				{
					// 成長譚スクリプト詳細マスタ
					MasterSpecialQuestAreaScriptDetail *master = MasterSpecialQuestAreaScriptDetail::deserialize(jsonFilePath);
					this->setMasterSpecialQuestAreaScriptDetail(master);
					break;
				}
				case kMasterHonor:
				{
					// 称号マスタ
					MasterHonor *master = MasterHonor::deserialize(jsonFilePath);
					this->setMasterHonor(master);
					break;
				}
				case kMasterEquipment:
				{
					// 魅玉マスタ
					MasterEquipment *master = MasterEquipment::deserialize(jsonFilePath);
					this->setMasterEquipment(master);
					break;
				}
				default:
					break;
			}
		}
	}
}
*/
/*
//--------------------------------------------------------------------------------------------------------------------
// パース済みJSONがあれば返す。なければ読み込んでパースしたものを返す。
//--------------------------------------------------------------------------------------------------------------------
picojson::value *GameManager::getJson(std::string jsonFilePath)
{
	JsonMapType::iterator it = _jsonMap.find(jsonFilePath);
	if (it == _jsonMap.end()){
		// jsonファイルの復号化
		unsigned long size(0);
		unsigned char* buff = CCFileUtils::sharedFileUtils()->getFileData(jsonFilePath.c_str(), "rb", &size);

		// 【win32環境限定】
		// ファイルが存在しなかった場合でもアプリ落ちさせない、
		// ただしデバッグブレークで一時停止させておく
		#if (CC_TARGET_PLATFORM == CC_PLATFORM_WIN32)
		if ( !buff )
		{
			CCLOG( "****\n****\n****\n Can not find [%s]\n****\n****\n****\n", jsonFilePath.c_str() );
			DebugBreak();
			return 0;
		}
		else
		#endif
		{
			// 復号
			Blowfish BF;
			BF.Set_Passwd( getBlowFishPassword() );
			BF.Decrypt( (void *)buff, size );

			// ↓このコードを入れておくと末尾の詰め物を抜いたサイズが求まります。
			while ( 0 == buff[size - 1] )
			{
				--size;
			}

			// jsonを解析する。
			std::string err;
			picojson::value *json = new picojson::value();
			picojson::parse( *json, buff, buff + size, &err );

			// Dispose data.
			delete[] buff;

			return json;
		}
	}
	picojson::value *json = it->second;
	_jsonMap.erase(it);
	return json;
}

//--------------------------------------------------------------------------------------------------------------------
// パース済みのJSONをセットする。
//--------------------------------------------------------------------------------------------------------------------
void GameManager::setJson(std::string jsonFilePath, picojson::value *json)
{
	JsonMapType::iterator it = _jsonMap.find(jsonFilePath);
	if (it != _jsonMap.end()){
		//既に登録済みのものがあれば消しておく。
		delete it->second;
		_jsonMap.erase(it);
	}
	_jsonMap[jsonFilePath] = json;
}
*/
//--------------------------------------------------------------------------------------------------------------------
// 保存ファイルパスを取得する。
//--------------------------------------------------------------------------------------------------------------------
std::string GameManager::getFilePath()
{
	return FileUtils::getInstance()->getApplicationPath() + "game_data.json";
	//	return CCFileUtils::getInstance()->getWritablePath() + "game_manager_data.json";
}

//--------------------------------------------------------------------------------------------------------------------
// セーブフォルダがなければ作成し、セーブフォルダのパスを返す
//--------------------------------------------------------------------------------------------------------------------
std::string GameManager::createAndGetSaveDir()const
{
	auto savedataPath = this->getSaveFilePath();

	// セーブディレクトリが無い場合
	if (!CCFileUtils::getInstance()->isDirectoryExist(savedataPath)) {
		// 生成
		CCFileUtils::getInstance()->createDirectory(savedataPath);
	}
	return savedataPath;
}
/*
//--------------------------------------------------------------------------------------------------------------------
// 一時的な値を削除する。
//--------------------------------------------------------------------------------------------------------------------
void GameManager::removeTempValue()
{
	this->_prevLayer = MainScene::kLayerTypeNone;
	this->_selectedNormalQuestMapId = "";
	this->_selectedNormalQuestId = "";
	this->_selectedNormalQuestDetailId = "";
	this->_selectedSpecialQuestDetailId = "";
	this->_selectedBondQuestDetailId = "";
	this->_selectedSpecialQuestAreaId = "";
	this->_selectedSpecialQuestAreaDetailId = "";

	this->_isSealsQuestSelected = false;

	this->_isLocalEnemyCompleted = false;
	this->_isChapterCleared = false;
	this->_isBigAreaCleared = false;
	this->_isSmallAreaCleared = false;
	this->_isNewBondQuest = false;
	this->_isNewAreaQuest = false;
	this->_isMaterialOver = false;
	this->_isCardNumOver = false;
	this->_isSpecialQuestBonus = false;
	this->_isExchangeOpen = false;
	this->_prevNewInfoTime = 0;
	this->_questRankingRewards.clear();
	this->_isGrowMaterilOver = false;
	this->_isGrowStoryOpen = false;
}
*/

void GameManager::serialize()
{
#if 0
	rapidjson::Document doc;
	doc.SetObject();
	rapidjson::Document::AllocatorType& allocator = doc.GetAllocator();

	auto project = this->getProjectData();
	//variable
	rapidjson::Value variableList(rapidjson::kArrayType);
	if (project->getVariableList()->count() > 0) {
		auto dic = project->getVariableList();
		cocos2d::DictElement *el = nullptr;
		CCDICT_FOREACH(dic, el) {
			auto v = dynamic_cast<agtk::data::VariableData *>(el->getObject());
			if (v->getToBeSaved() == false) {
				continue;
			}
			rapidjson::Value obj(rapidjson::kObjectType);
			obj.AddMember(rapidjson::Value("id", allocator), v->getId(), allocator);
			obj.AddMember(rapidjson::Value("value", allocator), v->getValue(), allocator);
			variableList.PushBack(obj, allocator);
		}
	}
	doc.AddMember("variableList", variableList, allocator);
	//switch
	rapidjson::Value switchList(rapidjson::kArrayType);
	if (project->getSwitchList()->count() > 0) {
		auto dic = project->getSwitchList();
		cocos2d::DictElement *el = nullptr;
		CCDICT_FOREACH(dic, el) {
			auto v = dynamic_cast<agtk::data::SwitchData *>(el->getObject());
			if (v->getToBeSaved() == false) {
				continue;
			}
			rapidjson::Value obj(rapidjson::kObjectType);
			obj.AddMember(rapidjson::Value("id", allocator), v->getId(), allocator);
			obj.AddMember(rapidjson::Value("value", allocator), v->getValue(), allocator);
			switchList.PushBack(obj, allocator);
		}
	}
	doc.AddMember("switchList", switchList, allocator);

	rapidjson::StringBuffer buffer;
	rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
	doc.Accept(writer);

	const std::string path = this->getFilePath();
	FileUtils::getInstance()->writeStringToFile(buffer.GetString(), path);
#endif
}

void GameManager::deserialize()
{
#if 0
	std::string path = this->getFilePath();
	auto jsonData = FileUtils::getInstance()->getStringFromFile(path);
	if (jsonData.length() == 0) {
		return;
	}

	rapidjson::Document doc;
	doc.Parse(jsonData.c_str());
	bool error = doc.HasParseError();
	if (error) {
		CCASSERT(0, "Error: Json Parse.");
	}

	auto project = this->getProjectData();
	//variableList
	rapidjson::Value& variableList = doc["variableList"];
	for (rapidjson::SizeType i = 0; i < variableList.Size(); i++) {
		int id = variableList[i]["id"].GetInt();
		int value = variableList[i]["value"].GetDouble();
		auto dic = project->getVariableList();
		cocos2d::DictElement *el = nullptr;
		CCDICT_FOREACH(dic, el) {
			auto v = dynamic_cast<agtk::data::VariableData *>(el->getObject());
			if (id == v->getId()) {
				v->setValue(value);
				break;
			}
		}
	}
	//switchList
	rapidjson::Value& switchList = doc["switchList"];
	for (rapidjson::SizeType i = 0; i < switchList.Size(); i++) {
		int id = switchList[i]["id"].GetInt();
		bool value = switchList[i]["value"].GetBool();
		auto dic = project->getSwitchList();
		cocos2d::DictElement *el = nullptr;
		CCDICT_FOREACH(dic, el) {
			auto v = dynamic_cast<agtk::data::SwitchData *>(el->getObject());
			if (id == v->getId()) {
				v->setValue(value);
				break;
			}
		}
	}
#endif
}

void GameManager::checkRequestVariableAndSwitch()
{
	auto scene = this->getCurrentScene();

	// 変数・スイッチ初期化要求スイッチがONの場合
	if(scene->getRequestSwitchInit()) {

		auto projectPlayData = this->getPlayData();
		projectPlayData->reset(true);
// #AGTK-NX
#ifdef USE_SAR_OPTIMIZE_1
		auto objectList = scene->getObjectAllReference();
#else
		auto objectList = scene->getObjectAll();
#endif
		cocos2d::Ref *ref;
		CCARRAY_FOREACH(objectList, ref) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
			auto object = static_cast<agtk::Object *>(ref);
#else
			auto object = dynamic_cast<agtk::Object *>(ref);
#endif
			object->getPlayObjectData()->reset(true);
		}

		// ACT2-6205 オブジェクトの初期化
		this->setSceneChangeReappearObjectList(cocos2d::Array::create());
		this->setNotReappearObjectList(cocos2d::Array::create());
		this->setCommandReappearObjectList(cocos2d::__Array::create());
		this->setSceneEndTakeoverStatesObjectList(cocos2d::__Array::create());
		this->setIgnoreCreatePlayerList(cocos2d::__Array::create());

		auto layerList = scene->getSceneData()->getLayerList();
		int startZOrder = agtk::BaseLayer::ZOrder::Scene + agtk::BaseLayer::ADD_ZORDER;

		for (int layerId = (int)layerList->count(); layerId >= 1; layerId--) {
			auto layerData = scene->getSceneData()->getLayer(layerId);
			if (layerData == nullptr) continue;
			CC_ASSERT(layerData);
			CC_ASSERT(layerData->getLayerId() == layerId);
			auto sceneLayer = agtk::SceneLayer::create(scene, getProjectData()->getSceneData(scene->getSceneData()->getId()), layerData->getLayerId(), false, false);
#ifdef USE_AGTK
			getPhysicsWorld()->setDebugLayer(sceneLayer);
#endif

			scene->getSceneTopMost()->addChild(sceneLayer, startZOrder + agtk::BaseLayer::ADD_ZORDER * ((int)layerList->count() - layerId));
			scene->getSceneLayerList()->setObject(sceneLayer, layerData->getLayerId());
		}

		scene->setRequestSwitchInit(false);//OFF!
	}
}

void GameManager::updateByChangingVariableAndSwitch()
{
	updateObjectVariableAndSwitch();
	updateFileExistSwitch();// ファイルスロットが変更されているかもしれないので、「ファイルを確認」スイッチを更新
}

void GameManager::updateFileExistSwitch()
{
	auto projectPlayData = this->getPlayData();
	auto const slotIdx = projectPlayData->getCommonVariableData(agtk::data::EnumProjectSystemVariable::kProjectSystemVariableFileSlot)->getValue();
	auto const filePath = getSaveFilePath(slotIdx);

	auto sd = projectPlayData->getCommonSwitchData(agtk::data::EnumProjectSystemSwitch::kProjectSystemSwitchFileExists);
	auto const readOnly = sd->getReadOnly();
	sd->setReadOnly(false);
	sd->setValue(FileUtils::getInstance()->isFileExist(filePath));
	sd->setReadOnly(readOnly);
}

void GameManager::updateObjectVariableAndSwitch()
{
	//オブジェクトデータの変数・スイッチ
	auto scene = this->getCurrentScene();
	if (scene) {
// #AGTK-NX
#ifdef USE_SAR_OPTIMIZE_1
		auto objectList = scene->getObjectAllReference();
#else
		auto objectList = scene->getObjectAll();
#endif
		cocos2d::Ref *ref;
		CCARRAY_FOREACH(objectList, ref) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
			auto object = static_cast<agtk::Object *>(ref);
#else
			auto object = dynamic_cast<agtk::Object *>(ref);
#endif

// #AGTK-NX
#ifndef USE_SAR_OPTIMIZE_4
			// スイッチ変更による接続オブジェクトの設定変更処理
			object->checkCreateConnectObject(false, true);
#endif

			// 無効
			if (object->getDisabled()) {
				if (object->isVisible()) {
					//非表示にする。
					object->setVisible(false);
					// パーティクルを削除
					object->removeParticles();
				}
			}
			else {
				if (!object->isVisible()) {
					object->setVisible(true);
				}
			}

			//無敵スイッチ
			auto playObjectData = object->getPlayObjectData();
			if (playObjectData->getSwitchData(agtk::data::kObjectSystemSwitchInvincible)->getValue() == true) {
				auto damageInvincible = object->getObjectVisible()->getObjectDamageInvincible();
				if (!damageInvincible->isInvincible() || damageInvincible->isInvincibleStartAcceptable()) {
					damageInvincible->start(true);
				}
			}
		}
	}
}

void GameManager::updateSystemVariableAndSwitch()
{
	auto projectPlayData = this->getPlayData();

	//1PコントローラーID ～ 4PコントローラーID
	for (int id = agtk::data::kProjectSystemVariable1PController; id <= agtk::data::kProjectSystemVariable4PController; id++) {
		auto variableData = projectPlayData->getCommonVariableData(id);
		if (variableData->isExternalValue()) {
			if (variableData->checkChangeValue()) {
				InputManager::getInstance()->setupController(id);
			}
			variableData->resetExternalValue();
		}
	}
}

//--------------------------------------------------------------------------------------------------------------------
//
// データロード・セーブ関係
//
//--------------------------------------------------------------------------------------------------------------------
/**
* データセーブ
*/
void GameManager::saveData()
{
	auto scene = this->getCurrentScene();
	auto sceneCamera = scene->getCamera();
	auto cameraNode = sceneCamera->getCamera();

	// プロジェクトプレイデータ取得
	auto projectPlayData = this->getPlayData();

	// スロットIDXを取得
	int slotIdx = projectPlayData->getCommonVariableData(agtk::data::EnumProjectSystemVariable::kProjectSystemVariableFileSlot)->getValue();
// #AGTK-NX
#if (CC_TARGET_PLATFORM == CC_PLATFORM_NX)
#endif

	CCLOG("データセーブ開始 [%d]", slotIdx);

	// Jsonシリアライザ
	rapidjson::Document doc;
	doc.SetObject();
	rapidjson::Document::AllocatorType& allocator = doc.GetAllocator();

	// ----------------------------------------------------------------
	// 現在のシーン状態
	// ----------------------------------------------------------------
	rapidjson::Value saveSceneData(rapidjson::kObjectType);
	// シーンID
	saveSceneData.AddMember("sceneId", scene->getSceneData()->getId(), allocator);
	// シーン背景の画面効果
	auto bgRenderTexture = scene->getSceneBackground()->getRenderTexture();
	rapidjson::Value bgShaderKindList(rapidjson::kArrayType);
	if (bgRenderTexture) {
		cocos2d::Ref *ref = nullptr;
		CCARRAY_FOREACH(bgRenderTexture->getShaderList(), ref) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
			auto shader = static_cast<agtk::Shader *>(ref);
#else
			auto shader = dynamic_cast<agtk::Shader *>(ref);
#endif
			rapidjson::Value shaderData(rapidjson::kObjectType);
			shaderData.AddMember("kind", shader->getKind(), allocator);
			shaderData.AddMember("value", shader->getValue()->getValue(), allocator);
			shaderData.AddMember("seconds", shader->getValue()->getSeconds(), allocator);
			shaderData.AddMember("ignored", shader->getIgnored(), allocator);
			bgShaderKindList.PushBack(shaderData, allocator);
			CCLOG("シーン背景の画面効果保存: kind(%d), value(%f), seconds(%f) ignored(%s)", shader->getKind(), shader->getValue()->getValue(), shader->getValue()->getSeconds(), (shader->getIgnored() ? "True" : "False"));
		}
	}
	saveSceneData.AddMember("bgShaderList", bgShaderKindList, allocator);
	// シーン最前面の画面効果
	auto topMostRenderTexture = scene->getSceneTopMost()->getRenderTexture();
	rapidjson::Value topMostShaderKindList(rapidjson::kArrayType);
	if (topMostRenderTexture) {
		cocos2d::Ref *ref = nullptr;
		CCARRAY_FOREACH(topMostRenderTexture->getShaderList(), ref) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
			auto shader = static_cast<agtk::Shader *>(ref);
#else
			auto shader = dynamic_cast<agtk::Shader *>(ref);
#endif
			rapidjson::Value shaderData(rapidjson::kObjectType);
			shaderData.AddMember("kind", shader->getKind(), allocator);
			shaderData.AddMember("value", shader->getValue()->getValue(), allocator);
			shaderData.AddMember("seconds", shader->getValue()->getSeconds(), allocator);
			shaderData.AddMember("ignored", shader->getIgnored(), allocator);
			topMostShaderKindList.PushBack(shaderData, allocator);
			CCLOG("シーン最前面の画面効果保存: kind(%d), value(%f), seconds(%f) ignored(%s)", shader->getKind(), shader->getValue()->getValue(), shader->getValue()->getSeconds(), (shader->getIgnored() ? "True" : "False"));
		}
	}
	saveSceneData.AddMember("topMostShaderList", topMostShaderKindList, allocator);
	// シーン最前面(メニュー含む)の画面効果
	auto topMostWithMenuRenderTexture = scene->getSceneTopMost()->getWithMenuRenderTexture();
	rapidjson::Value topMostWithMenuShaderKindList(rapidjson::kArrayType);
	if (topMostWithMenuRenderTexture) {
		cocos2d::Ref *ref = nullptr;
		CCARRAY_FOREACH(topMostWithMenuRenderTexture->getShaderList(), ref) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
			auto shader = static_cast<agtk::Shader *>(ref);
#else
			auto shader = dynamic_cast<agtk::Shader *>(ref);
#endif
			rapidjson::Value shaderData(rapidjson::kObjectType);
			shaderData.AddMember("kind", shader->getKind(), allocator);
			shaderData.AddMember("value", shader->getValue()->getValue(), allocator);
			shaderData.AddMember("seconds", shader->getValue()->getSeconds(), allocator);
			shaderData.AddMember("ignored", shader->getIgnored(), allocator);
			topMostWithMenuShaderKindList.PushBack(shaderData, allocator);
			CCLOG("シーン最前面(メニュー含む)の画面効果保存: kind(%d), value(%f), seconds(%f) ignored(%s)", shader->getKind(), shader->getValue()->getValue(), shader->getValue()->getSeconds(), (shader->getIgnored() ? "True" : "False"));
		}
	}
	saveSceneData.AddMember("topMostWithMenuShaderList", topMostWithMenuShaderKindList, allocator);
	// シーンレイヤーの画面効果
	rapidjson::Value layerEffList(rapidjson::kObjectType);
	auto sceneLayerList = scene->getSceneLayerList();
	cocos2d::DictElement *el = nullptr;
	CCDICT_FOREACH(sceneLayerList, el) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
		auto sceneLayer = static_cast<agtk::SceneLayer *>(el->getObject());
#else
		auto sceneLayer = dynamic_cast<agtk::SceneLayer *>(el->getObject());
#endif
		auto layerRenderCtr = sceneLayer->getRenderTexture();
		if (layerRenderCtr) {
			rapidjson::Value shaderKindList(rapidjson::kArrayType);
			cocos2d::Ref *ref2 = nullptr;
			CCARRAY_FOREACH(layerRenderCtr->getShaderList(), ref2) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
				auto shader = static_cast<agtk::Shader *>(ref2);
#else
				auto shader = dynamic_cast<agtk::Shader *>(ref2);
#endif
				rapidjson::Value shaderData(rapidjson::kObjectType);
				shaderData.AddMember("kind", shader->getKind(), allocator);
				shaderData.AddMember("value", shader->getValue()->getValue(), allocator);
				shaderData.AddMember("seconds", shader->getValue()->getSeconds(), allocator);
				shaderData.AddMember("ignored", shader->getIgnored(), allocator);
				shaderKindList.PushBack(shaderData, allocator);
				CCLOG("シーンレイヤーの画面効果保存:[%d] kind(%d), value(%f), seconds(%f) ignored(%s)", sceneLayer->getLayerId(), shader->getKind(), shader->getValue()->getValue(), shader->getValue()->getSeconds(), (shader->getIgnored() ? "True" : "False"));
			}
			rapidjson::Value key(StringUtils::format("layer%d", sceneLayer->getLayerId()).c_str(), allocator);
			layerEffList.AddMember(key, shaderKindList, allocator);
		}
	}
	saveSceneData.AddMember("sceneLayerEffectList", layerEffList, allocator);

	// カメラのアンカー座標
	saveSceneData.AddMember("camAnchorX", cameraNode->getAnchorPoint().x, allocator);
	saveSceneData.AddMember("camAnchorY", cameraNode->getAnchorPoint().y, allocator);
	// シーンの回転状態
	saveSceneData.AddMember("rotation", sceneCamera->getCameraRotationZ()->getValue(), allocator);
	// シーンの反転状態
	saveSceneData.AddMember("flipX", sceneCamera->getCameraRotationY()->getValue(), allocator);
	saveSceneData.AddMember("flipY", sceneCamera->getCameraRotationX()->getValue(), allocator);
	// シーンの重力
	saveSceneData.AddMember("gravityX", scene->getGravity()->getGravity().x, allocator);
	saveSceneData.AddMember("gravityY", scene->getGravity()->getGravity().y, allocator);
	saveSceneData.AddMember("duration300", scene->getGravity()->getDuration300(), allocator);

	// シーンで再生中のBGMs
	{
		std::vector<BgmInfo> tmpBgmInfo;
		rapidjson::Value bgmList(rapidjson::kArrayType);

		// 再生中BGM
		auto bgmIdList = AudioManager::getInstance()->getBgmIdList();
		cocos2d::Ref* ref = nullptr;
		CCARRAY_FOREACH(bgmIdList, ref){
			
			BgmInfo bgm;
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
			bgm._bgmId = static_cast<Integer *>(ref)->getValue();
			auto bgmAudioList = static_cast<cocos2d::__Dictionary *>(AudioManager::getInstance()->getBgmList()->objectForKey(bgm._bgmId));
#else
			bgm._bgmId = dynamic_cast<Integer *>(ref)->getValue();
			auto bgmAudioList = dynamic_cast<cocos2d::__Dictionary *>(AudioManager::getInstance()->getBgmList()->objectForKey(bgm._bgmId));
#endif
			if (_registeringStopBgm.size() > 0) {
				bool skipFlag = false;
				for (auto stopBgm : _registeringStopBgm) {
					if (stopBgm._bgmId == bgm._bgmId) {
						skipFlag = true;
						break;
					}
				}
				if (skipFlag) continue;
			}
			cocos2d::DictElement *el;
			CCDICT_FOREACH(bgmAudioList, el) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
				auto p = static_cast<AudioManager::AudioInfo *>(el->getObject());
#else
				auto p = dynamic_cast<AudioManager::AudioInfo *>(el->getObject());
#endif
				bgm._loop = p->getIsLoop() ? 1 : 0;
				bgm._volume = static_cast<int>(p->getInitVolume() * 100);
				bgm._pan = p->getPan();
				bgm._pitch = p->getPitch();
				tmpBgmInfo.push_back(bgm);
			}
		}

		//未再生BGM
		for (auto bgm : _loadedBgm) {
			bool flag = true;
			for (auto bgm2 : tmpBgmInfo) {
				if (!bgm.compare(bgm2)) {
					//再生中のBGMと未再生BGMが同じ。すでに再生されている。
					flag = false;
					break;
				}
			}
			if (flag) {
				tmpBgmInfo.push_back(bgm);
			}
		}
		//登録中BGM
		for (auto bgm : _registeringBgm) {
			bool flag = true;
			for (auto bgm2 : tmpBgmInfo) {
				if (!bgm.compare(bgm2)) {
					//再生中のBGMと未再生BGMが同じ。すでに再生されている。
					flag = false;
					break;
				}
			}
			if (flag) {
				tmpBgmInfo.push_back(bgm);
			}
		}

		//BGMセーブデータを登録。
		for (auto bgm : tmpBgmInfo) {
			rapidjson::Value bgmOne(rapidjson::kObjectType);
			bgmOne.AddMember("bgmId", bgm._bgmId, allocator);
			bgmOne.AddMember("bgmLoop", bgm._loop, allocator);
			bgmOne.AddMember("bgmVolume", bgm._volume, allocator);
			bgmOne.AddMember("bgmPan", bgm._pan, allocator);
			bgmOne.AddMember("bgmPitch", bgm._pitch, allocator);
			CCLOG("データセーブ：BGM情報: [%d], %s, volume:%d , pan:%d , pitch:%d", bgm._bgmId, (bgm._loop ? "Loop" : "Once"), bgm._volume, bgm._pan, bgm._pitch);
			bgmList.PushBack(bgmOne, allocator);
		}
		saveSceneData.AddMember("bgm", bgmList, allocator);
	}
	// シーンセーブデータを登録
	doc.AddMember("sceneData", saveSceneData, allocator);

	auto const & objMemberFunc = [&](agtk::Object* obj){
		// objectID・レイヤーID・座標・最初に生成されたシーンのID・シーンパーツID・プレイデータ
		rapidjson::Value objJson(rapidjson::kObjectType);
// #AGTK-NX
#if CC_TARGET_PLATFORM != CC_PLATFORM_NX
		objJson.AddMember(rapidjson::Value("objectId", allocator), obj->getObjectData()->getId(), allocator);
		objJson.AddMember(rapidjson::Value("instanceId", allocator), obj->getInstanceId(), allocator);
		objJson.AddMember(rapidjson::Value("layerId", allocator), obj->getLayerId(), allocator);

		auto pos = obj->getPosition();
		// 「カメラとの位置関係を固定する」設定時は、カメラ位置からの差分をポジションに設定する。
		if (obj->getObjectData()->getFixedInCamera() && obj->getSceneLayer()->getType() != agtk::SceneLayer::kTypeMenu) {
			pos = obj->getObjectPosInCamera();
		}

		objJson.AddMember(rapidjson::Value("px", allocator), pos.x, allocator);
		objJson.AddMember(rapidjson::Value("py", allocator), pos.y, allocator);
		objJson.AddMember(rapidjson::Value("sx", allocator), obj->getScaleX(), allocator);
		objJson.AddMember(rapidjson::Value("sy", allocator), obj->getScaleY(), allocator);
		objJson.AddMember(rapidjson::Value("sceneId", allocator), obj->getSceneIdOfFirstCreated(), allocator);
		objJson.AddMember(rapidjson::Value("currentSceneId", allocator), obj->getSceneLayer()->getSceneData()->getId(), allocator);
		objJson.AddMember(rapidjson::Value("scenePartsId", allocator), obj->getScenePartsId(), allocator);
		auto currentAction = obj->getCurrentObjectAction();
		objJson.AddMember(rapidjson::Value("initialActionId", allocator), (currentAction ? currentAction->getId() : -1), allocator);
		objJson.AddMember(rapidjson::Value("prevActionId", allocator), obj->getPrevObjectActionId(), allocator);
		objJson.AddMember(rapidjson::Value("directionNo", allocator), obj->getMoveDirection(), allocator);
		{
			rapidjson::Value initArray(rapidjson::kArrayType);
			auto moveDirection = obj->getObjectMovement()->getDirectionDirect();
			initArray.PushBack(moveDirection.x, allocator);
			initArray.PushBack(moveDirection.y, allocator);
			objJson.AddMember(rapidjson::Value("moveDirection", allocator), initArray, allocator);
		}
		objJson.AddMember(rapidjson::Value("courseId", allocator), obj->getObjectReappearData()->getInitialCourseId(), allocator);
		objJson.AddMember(rapidjson::Value("coursePointId", allocator), obj->getObjectReappearData()->getInitialCoursePointId(), allocator);
		objJson.AddMember(rapidjson::Value("takeOverAnimMotionId", allocator), obj->getTakeOverAnimMotionId(), allocator);
#else
#endif
		objJson.AddMember("playData", obj->getPlayObjectData()->json(allocator), allocator);

		// フィルター効果
		if (obj->getPlayer() && obj->getPlayer()->getRenderTexture()) {
			rapidjson::Value shaderKindList(rapidjson::kArrayType);
			cocos2d::Ref* ref = nullptr;
			CCARRAY_FOREACH(obj->getPlayer()->getRenderTexture()->getShaderList(), ref) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
				auto shader = static_cast<agtk::Shader*>(ref);
#else
				auto shader = dynamic_cast<agtk::Shader*>(ref);
#endif
				rapidjson::Value shaderData(rapidjson::kObjectType);
				shaderData.AddMember("kind", shader->getKind(), allocator);
				shaderData.AddMember("ignored", shader->getIgnored(), allocator);
				shader->getValue()->getJsonData(shaderData, allocator);
				if (shader->getKind() == agtk::Shader::kShaderColorRgba) {
					rapidjson::Value colorJson(rapidjson::kObjectType);
					auto color = shader->getShaderRgbaColorBase();
					colorJson.AddMember("r", color.r, allocator);
					colorJson.AddMember("g", color.g, allocator);
					colorJson.AddMember("b", color.b, allocator);
					colorJson.AddMember("a", color.a, allocator);
// #AGTK-NX
#if CC_TARGET_PLATFORM != CC_PLATFORM_NX
					shaderData.AddMember(rapidjson::Value("color", allocator), colorJson, allocator);
#else
#endif
				}
				shaderKindList.PushBack(shaderData, allocator);
				CCLOG("save objShader kind:%d value:%f sec:%f", shader->getKind(), shader->getValue()->getValue(), shader->getValue()->getSeconds());
			}
			objJson.AddMember("shader", shaderKindList, allocator);
		}

		// 接続されているオブジェクト
		{
			rapidjson::Value connectObjectList(rapidjson::kArrayType);
			cocos2d::Ref *ref = nullptr;
			CCARRAY_FOREACH(obj->getConnectObjectList(), ref) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
				auto connectObj = static_cast<agtk::ConnectObject *>(ref);
#else
				auto connectObj = dynamic_cast<agtk::ConnectObject *>(ref);
#endif

				rapidjson::Value connectObjectData(rapidjson::kObjectType);
				connectObjectData.AddMember("instanceId", connectObj->getInstanceId(), allocator);
				connectObjectData.AddMember("settingId", connectObj->getObjectConnectSettingData()->getId(), allocator);
				connectObjectList.PushBack(connectObjectData, allocator);
				CCLOG("save connectObject instanceId:%d settingId:%d", connectObj->getInstanceId(), connectObj->getObjectConnectSettingData()->getId());
			}
			objJson.AddMember("connectObject", connectObjectList, allocator);
		}
		//接続表示優先順位
		{
			rapidjson::Value connectObjectDispPriorityList(rapidjson::kArrayType);
			cocos2d::Ref *ref = nullptr;
			CCARRAY_FOREACH(obj->getConnectObjectDispPriorityList(), ref) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
				auto p = static_cast<agtk::Object::ConnectObjectDispPriority *>(ref);
#else
				auto p = dynamic_cast<agtk::Object::ConnectObjectDispPriority *>(ref);
#endif
				rapidjson::Value connectObjectDispPriority(rapidjson::kObjectType);
				connectObjectDispPriority.AddMember("instanceId", p->getObject()->getInstanceId(), allocator);
				connectObjectDispPriority.AddMember("lowerPriority", p->getLowerPriority(), allocator);
				connectObjectDispPriorityList.PushBack(connectObjectDispPriority, allocator);
			}
			objJson.AddMember("connectObjectDispPriority", connectObjectDispPriorityList, allocator);
		}
		// 子オブジェクト
		{
			rapidjson::Value childObjectList(rapidjson::kArrayType);
			cocos2d::Ref *ref = nullptr;
			CCARRAY_FOREACH(obj->getChildrenObjectList(), ref) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
				auto childObj = static_cast<agtk::Object *>(ref);
#else
				auto childObj = dynamic_cast<agtk::Object *>(ref);
#endif
				rapidjson::Value childObjectData(rapidjson::kObjectType);
				childObjectData.AddMember("instanceId", childObj->getInstanceId(), allocator);
				auto offset = childObj->getParentFollowPosOffset();
				childObjectData.AddMember("offsetX", offset.x, allocator);
				childObjectData.AddMember("offsetY", offset.y, allocator);
				childObjectData.AddMember("connectId", childObj->getParentFollowConnectId(), allocator);
				childObjectList.PushBack(childObjectData, allocator);
				CCLOG("save childObject instanceId:%d", childObj->getInstanceId());
			}
			objJson.AddMember("childObject", childObjectList, allocator);
		}
		// コース移動オブジェクト
		{
			auto courseMove = obj->getObjectCourseMove();
			if (courseMove) {
				rapidjson::Value objectCourseMoveData(rapidjson::kObjectType);

				objectCourseMoveData.AddMember("isFirst", courseMove->getIsFirst(), allocator);
				objectCourseMoveData.AddMember("moveDist", courseMove->getMoveDist(), allocator);
				objectCourseMoveData.AddMember("currentPointId", courseMove->getCurrentPointId(), allocator);
				objectCourseMoveData.AddMember("currentPointIdx", courseMove->getCurrentPointIdx(), allocator);
				objectCourseMoveData.AddMember("loopCount", courseMove->getLoopCount(), allocator);
				objectCourseMoveData.AddMember("reverseMove", courseMove->getReverseMove(), allocator);
				objectCourseMoveData.AddMember("reverseCourse", courseMove->getReverseCourse(), allocator);
				objectCourseMoveData.AddMember("moveX", courseMove->getMove().x, allocator);
				objectCourseMoveData.AddMember("moveY", courseMove->getMove().y, allocator);
				objectCourseMoveData.AddMember("currentPosX", courseMove->getCurrentPos().x, allocator);
				objectCourseMoveData.AddMember("currentPosY", courseMove->getCurrentPos().y, allocator);
				objJson.AddMember("objectCourseMoveData", objectCourseMoveData, allocator);
			}
		}
		//テキスト
		{
			auto guiManager = GuiManager::getInstance();
			auto list = cocos2d::__Array::create();

			//テキストを表示
			rapidjson::Value messageTextList(rapidjson::kArrayType);
			if (guiManager->getActionCommandMessageGui(obj, list)) {
				cocos2d::Ref *ref;
				CCARRAY_FOREACH(list, ref) {
					rapidjson::Value messageTextData(rapidjson::kObjectType);
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
					auto messageText = static_cast<agtk::ActionCommandMessageTextUi *>(ref);
#else
					auto messageText = dynamic_cast<agtk::ActionCommandMessageTextUi *>(ref);
#endif
					auto data = messageText->getData();
					int lockObjectInstanceId = -1;
					auto lockObject = messageText->getLockObject();
					if (lockObject != nullptr) {
						lockObjectInstanceId = lockObject->getInstanceId();
					}
					int actionId = obj->getActionId(data);
					int commandDataId = data->getId();
					messageTextData.AddMember("actionId", actionId, allocator);
					messageTextData.AddMember("commandDataId", commandDataId, allocator);
					messageTextData.AddMember("targetObjectInstanceId", obj->getInstanceId(), allocator);
					messageTextData.AddMember("lockObjectInstanceId", lockObjectInstanceId, allocator);
					messageTextList.PushBack(messageTextData, allocator);
				}
				objJson.AddMember("messageText", messageTextList, allocator);
			}
			//テキストをスクロール表示
			rapidjson::Value messageScrollTextList(rapidjson::kArrayType);
			if (guiManager->getActionCommandScrollMessageGui(obj, list)) {
				cocos2d::Ref *ref;
				CCARRAY_FOREACH(list, ref) {
					rapidjson::Value messageScrollTextData(rapidjson::kObjectType);
					auto messageScrollText = dynamic_cast<agtk::ActionCommandScrollMessageTextUi *>(ref);
					auto data = messageScrollText->getData();
					int lockObjectInstanceId = -1;
					auto lockObject = messageScrollText->getLockObject();
					if (lockObject != nullptr) {
						lockObjectInstanceId = lockObject->getInstanceId();
					}
					int actionId = obj->getActionId(data);
					int commandDataId = data->getId();
					messageScrollTextData.AddMember("actionId", actionId, allocator);
					messageScrollTextData.AddMember("commandDataId", commandDataId, allocator);
					messageScrollTextData.AddMember("targetObjectInstanceId", obj->getInstanceId(), allocator);
					messageScrollTextData.AddMember("lockObjectInstanceId", lockObjectInstanceId, allocator);
					messageScrollTextList.PushBack(messageScrollTextData, allocator);
				}
				objJson.AddMember("messageScrollText", messageScrollTextList, allocator);
			}
		}
// #AGTK-NX #AGTK-WIN
#ifdef USE_30FPS_4
		// 中間フレーム情報
		{
			rapidjson::Value middleFrameStock(rapidjson::kObjectType);

			auto mfs = &obj->_middleFrameStock;
			middleFrameStock.AddMember("middleFrameIndex", mfs->_middleFrameIndex, allocator);

			rapidjson::Value frameData(rapidjson::kArrayType);
			for (int i = 0; i < 2; i++) {
				rapidjson::Value middleFrame(rapidjson::kObjectType);
				auto mf = mfs->_frameData + i;
				middleFrame.AddMember("hasMiddleFrame", mf->_hasMiddleFrame, allocator);
				if (mf->_hasMiddleFrame) {
					middleFrame.AddMember("objectPosX", mf->_objectPos.x, allocator);
					middleFrame.AddMember("objectPosY", mf->_objectPos.y, allocator);
					middleFrame.AddMember("centerRotation", mf->_centerRotation, allocator);
					middleFrame.AddMember("innerScaleX", mf->_innerScale.x, allocator);
					middleFrame.AddMember("innerScaleY", mf->_innerScale.y, allocator);
					middleFrame.AddMember("innerRotation", mf->_innerRotation, allocator);
					middleFrame.AddMember("offsetX", mf->_offset.x, allocator);
					middleFrame.AddMember("offsetY", mf->_offset.y, allocator);
					middleFrame.AddMember("centerX", mf->_center.x, allocator);
					middleFrame.AddMember("centerY", mf->_center.y, allocator);
				}
				frameData.PushBack(middleFrame, allocator);
			}
			middleFrameStock.AddMember("frameData", frameData, allocator);
			objJson.AddMember("middleFrameStock", middleFrameStock, allocator);
		}
#endif

		CCLOG("save obj: objectId:%d, instanceId:%d, layerId:%d, px:%f, py:%f, sceneId:%d, scenePartsId:%d, initialActionId:%d",
			objJson["objectId"].GetInt(), objJson["instanceId"].GetInt(), objJson["layerId"].GetInt(), objJson["px"].GetDouble(), objJson["py"].GetDouble(), objJson["sceneId"].GetInt(), objJson["scenePartsId"].GetInt(), objJson["initialActionId"].GetInt());
		return objJson;
	};
	// ----------------------------------------------------------------
	// プレイヤーオブジェクトの位置情報
	// ----------------------------------------------------------------
	rapidjson::Value playerObjList(rapidjson::kObjectType);
	auto playerObjectList = scene->getObjectAllObjGroup(agtk::data::ObjectData::EnumObjGroup::kObjGroupPlayer, false, agtk::SceneLayer::kTypeScene);
	cocos2d::Ref *ref = nullptr;
	CCARRAY_FOREACH(playerObjectList, ref) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
		auto obj = static_cast<agtk::Object *>(ref);
#else
		auto obj = dynamic_cast<agtk::Object *>(ref);
#endif
		rapidjson::Value key(StringUtils::format("instance%d", obj->getInstanceId()).c_str(), allocator);
		playerObjList.AddMember(key, objMemberFunc(obj), allocator);
	}
	doc.AddMember("playerDataList", playerObjList, allocator);

	// ----------------------------------------------------------------
	// 初期配置されているオブジェクトの情報
	// ----------------------------------------------------------------
	rapidjson::Value initObjObjList(rapidjson::kArrayType);
	cocos2d::DictElement *el2 = nullptr;
	CCDICT_FOREACH(sceneLayerList, el2) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
		auto layer = static_cast<agtk::SceneLayer *>(el2->getObject());
#else
		auto layer = dynamic_cast<agtk::SceneLayer *>(el2->getObject());
#endif
		auto initialObjList = layer->getCreateObjectList();

		cocos2d::Ref *ref2 = nullptr;
		CCARRAY_FOREACH(initialObjList, ref2) {
			auto obj = dynamic_cast<agtk::Object *>(ref2);
			if (obj) {
				initObjObjList.PushBack(objMemberFunc(obj), allocator);
			}
		}
	}
	auto menuLayerList = scene->getMenuLayerList();
	CCDICT_FOREACH(menuLayerList, el2) {
		auto layer = dynamic_cast<agtk::SceneLayer *>(el2->getObject());
		auto initialObjList = layer->getCreateObjectList();

		cocos2d::Ref *ref2 = nullptr;
		CCARRAY_FOREACH(initialObjList, ref2) {
			auto obj = dynamic_cast<agtk::Object *>(ref2);
			if (obj) {
				initObjObjList.PushBack(objMemberFunc(obj), allocator);
			}
		}
	}
	doc.AddMember("initObjDataList", initObjObjList, allocator);

	// ----------------------------------------------------------------
	//「シーン終了時の状態を保持」の情報
	// ----------------------------------------------------------------
	rapidjson::Value initEndTakeoverObjList(rapidjson::kArrayType);
	{
		auto sceneEndTakeoverStatesObjectList = this->getSceneEndTakeoverStatesObjectList();
		cocos2d::Ref *ref;
		CCARRAY_FOREACH(sceneEndTakeoverStatesObjectList, ref) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
			auto data = static_cast<agtk::ObjectTakeoverStatesData *>(ref);
#else
			auto data = dynamic_cast<agtk::ObjectTakeoverStatesData *>(ref);
#endif
			// ID・レイヤーID・座標・プレイデータ
			rapidjson::Value initObj(rapidjson::kObjectType);
// #ATGK-NX
#if CC_TARGET_PLATFORM != CC_PLATFORM_NX
			initObj.AddMember(rapidjson::Value("id", allocator), data->getObjectId(), allocator);
			initObj.AddMember(rapidjson::Value("layerId", allocator), data->getSceneLayerId(), allocator);
			initObj.AddMember(rapidjson::Value("px", allocator), data->getPosition().x, allocator);
			initObj.AddMember(rapidjson::Value("py", allocator), data->getPosition().y, allocator);
			initObj.AddMember(rapidjson::Value("sx", allocator), data->getScale().x, allocator);
			initObj.AddMember(rapidjson::Value("sy", allocator), data->getScale().y, allocator);
#else
#endif
			initObj.AddMember("playData", data->getPlayObjectData()->json(allocator), allocator);

// #AGTK-NX
#if CC_TARGET_PLATFORM != CC_PLATFORM_NX
			initObj.AddMember(rapidjson::Value("sceneId", allocator), data->getSceneId(), allocator);
			initObj.AddMember(rapidjson::Value("scenePartsId", allocator), data->getScenePartsId(), allocator);
			initObj.AddMember(rapidjson::Value("actionNo", allocator), data->getActionNo(), allocator);
			initObj.AddMember(rapidjson::Value("directionNo", allocator), data->getDirectionNo(), allocator);
			initObj.AddMember(rapidjson::Value("dispDirection", allocator), data->getDispDirection(), allocator);
			{
				rapidjson::Value initArray(rapidjson::kArrayType);
				auto moveDirection = data->getMoveDirection();
				initArray.PushBack(moveDirection.x, allocator);
				initArray.PushBack(moveDirection.y, allocator);
				initObj.AddMember(rapidjson::Value("moveDirection", allocator), initArray, allocator);
			}
			initObj.AddMember(rapidjson::Value("takeOverAnimMotionId", allocator), data->getTakeOverAnimMotionId(), allocator);
			initObj.AddMember(rapidjson::Value("sceneIdOfFirstCreated", allocator), data->getSceneIdOfFirstCreated(), allocator);
#else
#endif

			auto objectData = this->getProjectData()->getObjectData(data->getObjectId());
			CCLOG("save initialObj: [%s] id:%d, layerId:%d, px:%f, py:%f, moveDirection:%f,%f", objectData->getName(), initObj["id"].GetInt(), initObj["layerId"].GetInt(), initObj["px"].GetDouble(), initObj["py"].GetDouble(), initObj["moveDirection"][0].GetDouble(), initObj["moveDirection"][1].GetDouble());

			initEndTakeoverObjList.PushBack(initObj, allocator);
		}
	}
	doc.AddMember("initEndTakeoverObjList", initEndTakeoverObjList, allocator);

	// ----------------------------------------------------------------
	//「復活オブジェクト情報リスト」データ
	// ----------------------------------------------------------------
	{
		auto const & reappearDataFunc = [&](agtk::data::ObjectReappearData* data) {
			rapidjson::Value dataJson(rapidjson::kObjectType);
// #AGTK-NX
#if CC_TARGET_PLATFORM != CC_PLATFORM_NX
			dataJson.AddMember(rapidjson::Value("sceneId", allocator), data->getSceneId(), allocator);//sceneId
			dataJson.AddMember(rapidjson::Value("scenePartsId", allocator), data->getScenePartsId(), allocator);//scenePartsId
			dataJson.AddMember(rapidjson::Value("sceneLayerId", allocator), data->getSceneLayerId(), allocator);//sceneLayerId
			dataJson.AddMember(rapidjson::Value("objectId", allocator), data->getObjectId(), allocator);//objectId
			dataJson.AddMember(rapidjson::Value("initialActionId", allocator), data->getInitialActionId(), allocator);//initialActionId
			dataJson.AddMember(rapidjson::Value("initialPositionX", allocator), data->getInitialPosition().x, allocator);//initialPosition.x
			dataJson.AddMember(rapidjson::Value("initialPositionY", allocator), data->getInitialPosition().y, allocator);//initialPosition.y
			dataJson.AddMember(rapidjson::Value("initialScaleX", allocator), data->getInitialScale().y, allocator);//initialScale.x
			dataJson.AddMember(rapidjson::Value("initialScaleY", allocator), data->getInitialScale().y, allocator);//initialScale.y
			dataJson.AddMember(rapidjson::Value("initialRotation", allocator), data->getInitialRotation(), allocator);//initialRotation
			dataJson.AddMember(rapidjson::Value("initialMoveDirectionId", allocator), data->getInitialMoveDirectionId(), allocator);//initialMoveDirectionId
			dataJson.AddMember(rapidjson::Value("reappearFlag", allocator), data->getReappearFlag(), allocator);//reappearFlag
			dataJson.AddMember(rapidjson::Value("initialCourseId", allocator), data->getInitialCourseId(), allocator);//initialCourseId
			dataJson.AddMember(rapidjson::Value("InitialCoursePointId", allocator), data->getInitialCoursePointId(), allocator);//initialCoursePointId
#else
#endif
			return dataJson;
		};

		rapidjson::Value reappearDataList(rapidjson::kArrayType);
		auto sceneLayerList = scene->getSceneLayerList();
		cocos2d::DictElement *el = nullptr;
		CCDICT_FOREACH(sceneLayerList, el) {
			auto sceneLayer = dynamic_cast<agtk::SceneLayer *>(el->getObject());
			auto objectList = sceneLayer->getDeleteObjectList();
			cocos2d::Ref *ref;
			CCARRAY_FOREACH(objectList, ref) {
				auto reappearData = dynamic_cast<agtk::data::ObjectReappearData *>(ref);
				reappearDataList.PushBack(reappearDataFunc(reappearData), allocator);
			}
		}
		doc.AddMember("reappearDataList", reappearDataList, allocator);
	}

	// ----------------------------------------------------------------
	//「復活不能オブジェクト情報リスト」データ
	// ----------------------------------------------------------------
	{
		auto const & notReappearDataFunc = [&](agtk::data::ObjectReappearData* data) {
			rapidjson::Value dataJson(rapidjson::kObjectType);
// #AGTK-NX
#if CC_TARGET_PLATFORM != CC_PLATFORM_NX
			dataJson.AddMember(rapidjson::Value("sceneId", allocator), data->getSceneId(), allocator);//sceneId
			dataJson.AddMember(rapidjson::Value("scenePartsId", allocator), data->getScenePartsId(), allocator);//scenePartsId
			dataJson.AddMember(rapidjson::Value("sceneLayerId", allocator), data->getSceneLayerId(), allocator);//sceneLayerId
			dataJson.AddMember(rapidjson::Value("objectId", allocator), data->getObjectId(), allocator);//objectId
			dataJson.AddMember(rapidjson::Value("initialActionId", allocator), data->getInitialActionId(), allocator);//initialActionId
			dataJson.AddMember(rapidjson::Value("initialPositionX", allocator), data->getInitialPosition().x, allocator);//initialPosition.x
			dataJson.AddMember(rapidjson::Value("initialPositionY", allocator), data->getInitialPosition().y, allocator);//initialPosition.y
			dataJson.AddMember(rapidjson::Value("initialScaleX", allocator), data->getInitialScale().y, allocator);//initialScale.x
			dataJson.AddMember(rapidjson::Value("initialScaleY", allocator), data->getInitialScale().y, allocator);//initialScale.y
			dataJson.AddMember(rapidjson::Value("initialRotation", allocator), data->getInitialRotation(), allocator);//initialRotation
			dataJson.AddMember(rapidjson::Value("initialMoveDirectionId", allocator), data->getInitialMoveDirectionId(), allocator);//initialMoveDirectionId
			dataJson.AddMember(rapidjson::Value("reappearFlag", allocator), data->getReappearFlag(), allocator);//reappearFlag
			dataJson.AddMember(rapidjson::Value("initialCourseId", allocator), data->getInitialCourseId(), allocator);//initialCourseId
			dataJson.AddMember(rapidjson::Value("InitialCoursePointId", allocator), data->getInitialCoursePointId(), allocator);//initialCoursePointId
#else
#endif
			return dataJson;
		};

		rapidjson::Value notReappearDataList(rapidjson::kArrayType);
		
		auto notReappearObjectList = GameManager::getInstance()->getNotReappearObjectList();
		
		cocos2d::Ref* ref = nullptr;

		CCARRAY_FOREACH(notReappearObjectList, ref) {
			auto reappearData = dynamic_cast<agtk::data::ObjectReappearData*>(ref);
			notReappearDataList.PushBack(notReappearDataFunc(reappearData), allocator);
		}
		doc.AddMember("notReappearDataList", notReappearDataList, allocator);
	}

	// ----------------------------------------------------------------
	//「実行アクションで復活するオブジェクト情報リスト」データ
	// ----------------------------------------------------------------
	{
		auto const & commandReappearDataFunc = [&](agtk::data::ObjectReappearData* data) {
			rapidjson::Value dataJson(rapidjson::kObjectType);
			// #AGTK-NX
#if CC_TARGET_PLATFORM != CC_PLATFORM_NX
			dataJson.AddMember(rapidjson::Value("sceneId", allocator), data->getSceneId(), allocator);//sceneId
			dataJson.AddMember(rapidjson::Value("scenePartsId", allocator), data->getScenePartsId(), allocator);//scenePartsId
			dataJson.AddMember(rapidjson::Value("sceneLayerId", allocator), data->getSceneLayerId(), allocator);//sceneLayerId
			dataJson.AddMember(rapidjson::Value("objectId", allocator), data->getObjectId(), allocator);//objectId
			dataJson.AddMember(rapidjson::Value("initialActionId", allocator), data->getInitialActionId(), allocator);//initialActionId
			dataJson.AddMember(rapidjson::Value("initialPositionX", allocator), data->getInitialPosition().x, allocator);//initialPosition.x
			dataJson.AddMember(rapidjson::Value("initialPositionY", allocator), data->getInitialPosition().y, allocator);//initialPosition.y
			dataJson.AddMember(rapidjson::Value("initialScaleX", allocator), data->getInitialScale().y, allocator);//initialScale.x
			dataJson.AddMember(rapidjson::Value("initialScaleY", allocator), data->getInitialScale().y, allocator);//initialScale.y
			dataJson.AddMember(rapidjson::Value("initialRotation", allocator), data->getInitialRotation(), allocator);//initialRotation
			dataJson.AddMember(rapidjson::Value("initialMoveDirectionId", allocator), data->getInitialMoveDirectionId(), allocator);//initialMoveDirectionId
			dataJson.AddMember(rapidjson::Value("reappearFlag", allocator), data->getReappearFlag(), allocator);//reappearFlag
			dataJson.AddMember(rapidjson::Value("initialCourseId", allocator), data->getInitialCourseId(), allocator);//initialCourseId
			dataJson.AddMember(rapidjson::Value("InitialCoursePointId", allocator), data->getInitialCoursePointId(), allocator);//initialCoursePointId
#else
#endif
			return dataJson;
		};

		rapidjson::Value commandReappearDataList(rapidjson::kArrayType);
		auto objectList = GameManager::getInstance()->getCommandReappearObjectList();
		cocos2d::Ref *ref;
		CCARRAY_FOREACH(objectList, ref) {
			auto reappearData = dynamic_cast<agtk::data::ObjectReappearData *>(ref);
			commandReappearDataList.PushBack(commandReappearDataFunc(reappearData), allocator);
		}
		doc.AddMember("commandReappearDataList", commandReappearDataList, allocator);
	}

	// ----------------------------------------------------------------
	// スクリプトプラグインの内部データ
	// ----------------------------------------------------------------
	rapidjson::Value scriptPluginInternalData(rapidjson::kStringType);
	// internals
	auto internals = JavascriptManager::getPluginInternals();
	scriptPluginInternalData = rapidjson::StringRef(internals.c_str());

	// プラグイン内部データを登録
	doc.AddMember("scriptPluginInternals", scriptPluginInternalData, allocator);

	// ----------------------------------------------------------------
	// プロジェクト共通の変数及びスイッチとオブジェクト共通の変数及びスイッチ
	// playData は末尾に追加する。
	// コピーで出来たセーブファイルと、セーブで出来たセーブファイルとで並びに差異が出ないようにするため。
	// コピーでは playData は末尾に追加されるので。（copyData参照）
	// ----------------------------------------------------------------
	auto savePlayData = projectPlayData->saveData(allocator);
	doc.AddMember("playData", savePlayData, allocator);

	// ----------------------------------------------------------------
	// ファイルに書き込み開始
	// ----------------------------------------------------------------
	auto issuccess = save(doc, slotIdx);

	updateFileExistSwitch();

// #AGTK-NX
#if (CC_TARGET_PLATFORM == CC_PLATFORM_NX)
#endif

	CCLOG("** データセーブ: %s", (issuccess ? "成功" : "失敗"));
}

bool GameManager::save(rapidjson::Document& doc,int slotIdx)const
{
	rapidjson::StringBuffer buffer;
	rapidjson::PrettyWriter<rapidjson::StringBuffer> writer(buffer);
	doc.Accept(writer);

	auto const savedataPath = createAndGetSaveDir();

	// セーブデータファイル保存
#ifdef USE_RUNTIME
	if (this->getEncryptSaveFile()){
	}
#endif
	return FileUtils::getInstance()->writeStringToFile(buffer.GetString(), getSaveFilePath(slotIdx));
}

/**
* データロード
*/
void GameManager::loadData()
{
	auto scene = this->getCurrentScene();

	// プロジェクトプレイデータ取得
	auto projectPlayData = this->getPlayData();

	CCLOG("データロード開始 ");

	// ----------------------------------------------------------------
	// セーブデータファイル取得
	// ----------------------------------------------------------------
	rapidjson::Document doc;
	if (!getSaveDataFile(doc)) {
		return;
	}

	// ----------------------------------------------------------------
	// プロジェクト共通の変数及びスイッチをロード
	// ----------------------------------------------------------------
	CCLOG("データロード: プロジェクト共通変数とスイッチ");
	projectPlayData->loadData(doc["playData"], _loadBit & kLoadBit_CommonSwitch, _loadBit & kLoadBit_CommonVariable, _loadBit & kLoadBit_ObjectList);

	//「ファイルを確認」スイッチを更新
	updateFileExistSwitch();

		// シーンID取得
	if (_loadBit & kLoadBit_Scene) {
		this->setLoadSceneId(doc["sceneData"]["sceneId"].GetInt());
	}
	// ----------------------------------------------------------------
	// スクリプトプラグインの内部データをロード
	// ----------------------------------------------------------------
	std::string internals("{}");
	if (doc.HasMember("scriptPluginInternals")) {
		internals = std::string(doc["scriptPluginInternals"].GetString());
	}
	JavascriptManager::setPluginInternals(internals);

	// ---------------------------------------------------------------------------- //
	// ※※ シーンとオブジェクトに対する反映は別途 attachLoadData() で行います ※※ //
	// ---------------------------------------------------------------------------- //
	CCLOG("データロード: 事前処理完了");
}

/**
* セーブデータのコピー
*/
void GameManager::copyData()
{
	rapidjson::Document doc;
	if (getSaveDataFile(doc)) 
	{
		// コピー先ファイルパスを取得
		int const dstSlotIdx = getPlayData()->getCommonVariableData(agtk::data::EnumProjectSystemVariable::kProjectSystemVariableCopyDestinationFileSlot)->getValue();

		// そのままコピーするとファイルスロットが嘘になってしまう。困るのでコピー先ファイルスロットの値で書き換える。
		{
			cocos2d::RefPtr<agtk::data::PlayData> playData = agtk::data::PlayData::create(getProjectData());
			playData->loadData(doc["playData"]);
			playData->getCommonVariableData(agtk::data::EnumProjectSystemVariable::kProjectSystemVariableFileSlot)->setValue(dstSlotIdx);
			doc.EraseMember("playData");
			doc.AddMember("playData", playData->saveData(doc.GetAllocator()), doc.GetAllocator());
		}

		save(doc, dstSlotIdx);
		CCLOG("セーブデータコピー完了");
	}
}

/**
* セーブデータの削除
*/
void GameManager::deleteData()
{
	// プロジェクトプレイデータ取得
	auto projectPlayData = this->getPlayData();

	// スロットIDXを取得
	int slotIdx = projectPlayData->getCommonVariableData(agtk::data::EnumProjectSystemVariable::kProjectSystemVariableFileSlot)->getValue();
// #AGTK-NX
#if (CC_TARGET_PLATFORM == CC_PLATFORM_NX)
#endif

	// ファイルパス取得
	auto filePath = getSaveFilePath(slotIdx);

	// ファイルが存在する場合
	if (FileUtils::getInstance()->isFileExist(filePath)) {
		// セーブデータファイル削除
		FileUtils::getInstance()->removeFile(filePath);
		updateFileExistSwitch();
// #AGTK-NX
#if (CC_TARGET_PLATFORM == CC_PLATFORM_NX)
#endif
		CCLOG("セーブデータ削除完了");
	}
}

/**
* セーブデータ存在チェック
* @return セーブデータの有無
* @note 破損している場合も存在していないと判断する
*/
bool GameManager::isExistsSaveData()
{
	// プロジェクトプレイデータ取得
	auto projectPlayData = this->getPlayData();

	// スロットIDXを取得
	int slotIdx = projectPlayData->getCommonVariableData(agtk::data::EnumProjectSystemVariable::kProjectSystemVariableFileSlot)->getValue();
// #AGTK-NX
#if (CC_TARGET_PLATFORM == CC_PLATFORM_NX)
#endif

	auto path = getSaveFilePath(slotIdx);
	auto filePath = FileUtils::getInstance()->fullPathForFilename(path);
	auto jsonData = FileUtils::getInstance()->getStringFromFile(filePath);
	if (jsonData.length() == 0) {
		CCLOG("データロード: データ無し");
		return false;
	}
	rapidjson::Document doc;
	doc.Parse(jsonData.c_str());
	bool error = doc.HasParseError();
	if (error) {
		CCASSERT(0, "Error: Json Parse.");
		return false;
	}

	return true;
}

/**
* セーブデータのファイルパス取得
* @param	slotIdx	スロットIDX(0x80000000の時はセーブデータフォルダパスを返す)
* @return	セーブデータのファイルパス
*/
std::string GameManager::getSaveFilePath(int slotIdx) const {

	std::string fileName;
	if (slotIdx != 0x80000000) {
		fileName = StringUtils::format("play_%d.json", slotIdx);
	}
#ifdef USE_RUNTIME
#if CC_TARGET_PLATFORM == CC_PLATFORM_NX
#else
	if (this->getProjectFolderSave()) {
		return getProjectPathFromProjectFile(getProjectFilePath()->getCString()) + SAVE_FOLDER + fileName;
	}
	else {
		return _saveDataPath + fileName;
	}
#endif
#else
	return getProjectPathFromProjectFile(getProjectFilePath()->getCString()) + SAVE_FOLDER + fileName;
#endif
}

bool  GameManager::getSaveDataFile(rapidjson::Document& doc)const
{
	// プロジェクトプレイデータ取得
	auto projectPlayData = this->getPlayData();
	// スロットIDXを取得
	int slotIdx = projectPlayData->getCommonVariableData(agtk::data::EnumProjectSystemVariable::kProjectSystemVariableFileSlot)->getValue();
// #AGTK-NX
#if (CC_TARGET_PLATFORM == CC_PLATFORM_NX)
#endif

	auto path = getSaveFilePath(slotIdx);
	auto filePath = FileUtils::getInstance()->fullPathForFilename(path);
	auto jsonData = FileUtils::getInstance()->getStringFromFile(filePath);
	if (jsonData.length() == 0) {
		CCASSERT(0, "データロード: データ無し(ロード作業中にデータが消された可能性があります)");
		return false;
	}

	doc.Parse(jsonData.c_str());
	bool error = doc.HasParseError();
	if (error) {
		CCASSERT(0, "Error: Json Parse.");
		return false;
	}
	CCLOG("セーブデータファイル取得 スロット%d", slotIdx);
	return true;
}

int GameManager::getSceneIdFromSaveDataFile()const
{
	rapidjson::Document doc;
	if (!getSaveDataFile(doc)) {
		return -1;
	}
	return doc["sceneData"]["sceneId"].GetInt();
}

void GameManager::saveConfig()
{
#if defined(USE_RUNTIME)
	rapidjson::StringBuffer buffer;
	rapidjson::PrettyWriter<rapidjson::StringBuffer> writer(buffer);
	getProjectData()->serializeConfig().Accept(writer);// 保存データ作成
	CCFileUtils::getInstance()->writeStringToFile(buffer.GetString(), getSaveConfigPath());
#endif
}

void GameManager::loadConfig()
{
#if defined(USE_RUNTIME)
	if (!isExistConfigData()) {
		return;
	}

	// ファイル読み込み
	auto filePath = FileUtils::getInstance()->fullPathForFilename( getSaveConfigPath() );
	auto jsonData = FileUtils::getInstance()->getStringFromFile(filePath);
	if (jsonData.length() == 0) {
		CCASSERT(0, "Error: data length 0");
		return;
	}
	// json 解析
	rapidjson::Document doc;
	doc.Parse(jsonData.c_str());
	bool error = doc.HasParseError();
	if (error) {
		CCASSERT(0, "Error: Json Parse.");
		return;
	}

	// 復元
	getProjectData()->deserializeConfig(doc);
#endif
}

bool GameManager::isExistConfigData()const
{
	auto filePath = FileUtils::getInstance()->fullPathForFilename(getSaveConfigPath());
	return CCFileUtils::getInstance()->isFileExist(filePath);
}
std::string GameManager::getSaveConfigPath()const
{
	return createAndGetSaveDir() + "config.json";
}

/**
* データセーブ要求チェック
*/
void GameManager::checkRequestSave()
{
	auto scene = this->getCurrentScene();

	// セーブ要求スイッチがONの場合
	if(scene->getRequestSwitchSaveFile()) {

		// セーブ実行
		this->saveData();

		scene->setRequestSwitchSaveFile(false);//OFF!
	}
}

/**
* データロード要求チェック
* @return	True:ロード実行開始 / False:何もなし
*/
bool GameManager::checkRequestLoad()
{
	bool isLoadStartOk = false;

	auto scene = this->getCurrentScene();

	// ロード要求スイッチがONの場合
	if(scene->getRequestSwitchLoadFile()) {

		// ロードするセーブデータがある場合はOK
		isLoadStartOk = isExistsSaveData();

		scene->setRequestSwitchLoadFile(false);//OFF!
	}

	return isLoadStartOk;
}

/**
* データ削除要求チェック
*/
void GameManager::checkRequestDelete()
{
	auto scene = this->getCurrentScene();

	// 削除要求スイッチがONの場合
	if(scene->getRequestSwitchDeleteFile()) {

		// 削除実行
		this->deleteData();

		scene->setRequestSwitchDeleteFile(false);//OFF!
	}
}

/**
* データコピー要求チェック
*/
void GameManager::checkRequestCopy()
{
	auto scene = this->getCurrentScene();

	// コピー要求スイッチがONの場合
	if(scene->getRequestSwitchCopyFile()) {

		// コピー実行
		this->copyData();

		scene->setRequestSwitchCopyFile(false);//OFF!
	}
}

/**
* ロードしたデータの反映
*/
void GameManager::attachLoadData()
{
	auto scene = this->getCurrentScene();
	auto sceneCamera = scene->getCamera();
	sceneCamera->update(0.0f);
	auto cameraNode = sceneCamera->getCamera();
	auto sceneLayerList = scene->getSceneLayerList();
	auto menuLayerList = scene->getMenuLayerList();

	struct ChildInfo {
		int instanceId;
		float offsetX;
		float offsetY;
		int connectId;
		ChildInfo(int instanceId, float offsetX, float offsetY, int connectId) {
			this->instanceId = instanceId;
			this->offsetX = offsetX;
			this->offsetY = offsetY;
			this->connectId = connectId;
		}
	};
	std::map<int, std::list<ChildInfo>> parentInstanceIdChildInfoList;

	struct DispPriorityInfo {
		int instanceId;
		bool lowerPriority;
		DispPriorityInfo(int instanceId, bool lowerPriority) {
			this->instanceId = instanceId;
			this->lowerPriority = lowerPriority;
		}
	};
	std::map<int, std::list<DispPriorityInfo>> parentInstanceIdDispPriorityList;

	struct MessageTextInfo {
		int actionId;
		int commandDataId;
		int instanceId;
		MessageTextInfo(int actionId, int commandDataId, int instanceId) {
			this->actionId = actionId;
			this->commandDataId = commandDataId;
			this->instanceId = instanceId;
		};
	};
	typedef MessageTextInfo MessageScrollTextInfo;
	std::map<int, std::list<MessageTextInfo>> targetInstanceIdMessageTextList;
	std::map<int, std::list<MessageScrollTextInfo>> targetInstanceIdMessageScrollTextList;

	// プロジェクトプレイデータ取得
	auto projectPlayData = this->getPlayData();

	CCLOG("データロード: シーンへの反映開始");

	// ----------------------------------------------------------------
	// セーブデータファイル取得
	// ----------------------------------------------------------------
	rapidjson::Document doc;
	if (!getSaveDataFile(doc)) {
		return;
	}

	// ----------------------------------------------------------------
	// シーンの状態を復帰
	// ----------------------------------------------------------------
	if (_loadBit & kLoadBit_Scene) {
		CCLOG("データロード: シーンへの反映開始: シーンの状態を復帰");
		const auto &savedSceneData = doc["sceneData"];

		// シーンのBGMとループフラグを保持
		_loadedBgm.clear();
		_loadedBgm.reserve(savedSceneData["bgm"].Size());
		for (unsigned int i = 0; i < savedSceneData["bgm"].Size(); ++i) {
			auto const & bgmOne = savedSceneData["bgm"][i];
			BgmInfo bgm;
			bgm._bgmId = bgmOne["bgmId"].GetInt();
			bgm._loop = bgmOne["bgmLoop"].GetBool();
			bgm._volume = bgmOne["bgmVolume"].GetInt();
			bgm._pan = bgmOne["bgmPan"].GetInt();
			bgm._pitch = bgmOne["bgmPitch"].GetInt();
			_loadedBgm.emplace_back(bgm);
			CCLOG("データロード：BGM情報[%d], %s, volume:%d, pan:%d , pitch:%d", bgm._bgmId, (bgm._loop ? "Loop" : "Once"), bgm._volume, bgm._pan, bgm._pitch);
		}
		// シーンの重力
		auto sceneGravity = scene->getGravity();
		sceneGravity->setGravity(Vec2(savedSceneData["gravityX"].GetDouble(), savedSceneData["gravityY"].GetDouble()));
		sceneGravity->setDuration300(savedSceneData["duration300"].GetInt());

		// シーンの回転状態・反転状態
		sceneCamera->getCameraRotationZ()->setValue(savedSceneData["rotation"].GetDouble());
		sceneCamera->getCameraRotationY()->setValue(savedSceneData["flipX"].GetDouble());
		sceneCamera->getCameraRotationX()->setValue(savedSceneData["flipY"].GetDouble());

		// カメラのアンカーポイント
		cameraNode->setAnchorPoint(Vec2(savedSceneData["camAnchorX"].GetDouble(), savedSceneData["camAnchorY"].GetDouble()));

		// 背景の画面効果がある場合
		if (savedSceneData["bgShaderList"].Size() > 0) {
			// シーン背景の画面効果
			auto sceneBg = scene->getSceneBackground();
			const auto &list = savedSceneData["bgShaderList"];
			for (unsigned int i = 0; i < list.Size(); i++) {
				const auto &shaderData = list[i];
				auto kind = (agtk::Shader::ShaderKind)shaderData["kind"].GetInt();
				sceneBg->setShader(kind, shaderData["value"].GetDouble(), shaderData["seconds"].GetDouble());
				auto shader = sceneBg->getShader(kind);
				shader->setIgnored(shaderData["ignored"].GetBool());
				CCLOG("シーン背景の画面効果ロード: kind(%d), value(%f), seconds(%f), ignored(%s)", kind, shaderData["value"].GetDouble(), shaderData["seconds"].GetDouble(), (shaderData["ignored"].GetBool() ? "True" : "False"));
			}
		}

		// 最前面の画面効果がある場合
		if (savedSceneData["topMostShaderList"].Size() > 0) {
			// シーン背景の画面効果
			auto sceneTopMost = scene->getSceneTopMost();
			const auto &list = savedSceneData["topMostShaderList"];
			for (unsigned int i = 0; i < list.Size(); i++) {
				const auto &shaderData = list[i];
				auto kind = (agtk::Shader::ShaderKind)shaderData["kind"].GetInt();
				sceneTopMost->setShader(kind, shaderData["value"].GetDouble(), shaderData["seconds"].GetDouble());
				auto shader = sceneTopMost->getShader(kind);
				if (shader) {
					shader->setIgnored(shaderData["ignored"].GetBool());
				}
				CCLOG("シーン最前面の画面効果ロード: kind(%d), value(%f), seconds(%f), ignored(%s)", kind, shaderData["value"].GetDouble(), shaderData["seconds"].GetDouble(), (shaderData["ignored"].GetBool() ? "True" : "False"));
			}
		}

		// 最前面(メニュー含む)の画面効果がある場合
		if (savedSceneData["topMostWithMenuShaderList"].Size() > 0) {
			// シーン背景の画面効果
			auto sceneTopMost = scene->getSceneTopMost();
			const auto &list = savedSceneData["topMostWithMenuShaderList"];
			for (unsigned int i = 0; i < list.Size(); i++) {
				const auto &shaderData = list[i];
				auto kind = (agtk::Shader::ShaderKind)shaderData["kind"].GetInt();
				sceneTopMost->setWithMenuShader(kind, shaderData["value"].GetDouble(), shaderData["seconds"].GetDouble());
				auto shader = sceneTopMost->getWithMenuShader(kind);
				if (shader) {
					shader->setIgnored(shaderData["ignored"].GetBool());
				}
				CCLOG("シーン最前面(メニュー含む)の画面効果ロード: kind(%d), value(%f), seconds(%f), ignored(%s)", kind, shaderData["value"].GetDouble(), shaderData["seconds"].GetDouble(), (shaderData["ignored"].GetBool() ? "True" : "False"));
			}
		}

		// シーンの画面効果のデータがある場合
		if (savedSceneData["sceneLayerEffectList"].MemberCount() > 0) {
			// シーンレイヤーの画面効果
			const auto &effList = savedSceneData["sceneLayerEffectList"];
			cocos2d::DictElement *el = nullptr;
			CCDICT_FOREACH(sceneLayerList, el) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
				auto layer = static_cast<agtk::SceneLayer *>(el->getObject());
#else
				auto layer = dynamic_cast<agtk::SceneLayer *>(el->getObject());
#endif
				rapidjson::Value key(StringUtils::format("layer%d", layer->getLayerId()).c_str(), doc.GetAllocator());

				if (effList.HasMember(key)) {
					for (unsigned int i = 0; i < effList[key].Size(); i++) {
						const auto &shaderData = effList[key][i];
						auto kind = (agtk::Shader::ShaderKind)shaderData["kind"].GetInt();
						layer->setShader(kind, shaderData["value"].GetDouble(), shaderData["seconds"].GetDouble());
						auto shader = layer->getShader(kind);
						shader->setIgnored(shaderData["ignored"].GetBool());
						CCLOG("シーンレイヤーの画面効果ロード:[%d] kind(%d), value(%f), seconds(%f), ignored(%s)", layer->getLayerId(), kind, shaderData["value"].GetDouble(), shaderData["seconds"].GetDouble(), (shaderData["ignored"].GetBool() ? "True" : "False"));
					}
				}
			}
		}
	}

	// レイヤー変更を行うメソッド
	auto changeLayerFunc = [](agtk::Scene *scene, agtk::Object *obj, int layerId) {
		CCLOG("change layer player obj[%d](%d -> %d)", obj->getInstanceId(), obj->getLayerId(), layerId);
		// レイヤーを移動させる
		auto nowSceneLayer = scene->getSceneLayer(obj->getLayerId());
		nowSceneLayer->getObjectList()->removeObject(obj);
// #AGTK-NX
#ifdef USE_SAR_OPTIMIZE_1
		nowSceneLayer->removeIdMap(obj);
#endif
		nowSceneLayer->removeChild(obj);
		obj->removeAllComponents();

		auto newSceneLayer = scene->getSceneLayer(layerId);
		newSceneLayer->getObjectList()->addObject(obj);
// #AGTK-NX
#ifdef USE_SAR_OPTIMIZE_1
		newSceneLayer->addIdMap(obj);
#endif
		obj->setLayerId(layerId);
		newSceneLayer->addCollisionDetaction(obj);
		newSceneLayer->addChild(obj);
	};

	// シェーダを適用する関数
	auto const & applyShader = [](agtk::Object* obj, const rapidjson::Value* shaderList) {
		auto player = obj->getPlayer();
		if (player != nullptr) {
			player->removeAllShader();
			if (shaderList) {
				for (unsigned int i = 0; i < shaderList->Size(); ++i) {
					const auto &shaderData = (*shaderList)[i];
					auto kind = (agtk::Shader::ShaderKind)shaderData["kind"].GetInt();
					auto shader = player->setShader(kind, shaderData);
					if (kind == agtk::Shader::kShaderColorRgba && shaderData.HasMember("color")) {
						const auto &color = shaderData["color"];
						shader->setShaderRgbaColor(cocos2d::Color4B(
							color["r"].GetInt(),
							color["g"].GetInt(),
							color["b"].GetInt(),
							color["a"].GetInt())
						);
					}
				}
			}
		}
	};

	// オブジェクト接続を適用する関数
	auto const & applyConnectObject = [](agtk::Object* obj, const rapidjson::Value* connectObjectList) {
		if (obj->getPlayer()) {
			if (connectObjectList) {
				std::vector<agtk::Object::ConnectObjectLoadList> connectObjectLoadList;
				for (unsigned int i = 0; i < connectObjectList->Size(); ++i) {
					const auto &connectObjectData = (*connectObjectList)[i];
					auto const instanceId = connectObjectData["instanceId"].GetInt();
					auto const settingId = connectObjectData["settingId"].GetInt();

					agtk::Object::ConnectObjectLoadList data;
					data.instanceId = instanceId;
					data.settingId = settingId;
					connectObjectLoadList.push_back(data);
				}
				obj->setConnectObjectLoadList(connectObjectLoadList);
			}
		}
	};
	// 子オブジェクト情報を適用する関数
	auto const & applyChildObject = [&parentInstanceIdChildInfoList](agtk::Object* obj, const rapidjson::Value* childObjectList) {
		if (childObjectList) {
			std::list<ChildInfo> childInfoList;
			for (unsigned int i = 0; i < childObjectList->Size(); ++i) {
				const auto &childObjectData = (*childObjectList)[i];
				auto const instanceId = childObjectData["instanceId"].GetInt();
				auto const offsetX = childObjectData["offsetX"].GetFloat();
				auto const offsetY = childObjectData["offsetY"].GetFloat();
				auto const connectId = childObjectData["connectId"].GetInt();
				childInfoList.emplace_back(ChildInfo(instanceId, offsetX, offsetY, connectId));
			}
			parentInstanceIdChildInfoList.insert(std::make_pair(obj->getInstanceId(), childInfoList));
		}
	};

	// オブジェクト接続を適用する関数
	auto const & applyObjectCourseMove = [](agtk::Object* obj, const rapidjson::Value* courseMoveList) {
		if (courseMoveList) {
			const auto &courseMoveData = *courseMoveList;

			auto data = agtk::Object::CourseMoveLoadData::create();

			data->setIsFirst(courseMoveData["isFirst"].GetBool());
			data->setMoveDist(courseMoveData["moveDist"].GetFloat());
			data->setCurrentPointId(courseMoveData["currentPointId"].GetInt());
			data->setCurrentPointIdx(courseMoveData["currentPointIdx"].GetInt());
			data->setLoopCount(courseMoveData["loopCount"].GetInt());
			data->setReverseMove(courseMoveData["reverseMove"].GetBool());
			data->setReverseCourse(courseMoveData["reverseCourse"].GetBool());
			data->setMove(cocos2d::Vec2(courseMoveData["moveX"].GetFloat(),
										courseMoveData["moveY"].GetFloat()));
			data->setCurrentPos(cocos2d::Vec2(courseMoveData["currentPosX"].GetFloat(),
												courseMoveData["currentPosY"].GetFloat()));

			obj->setCourseMoveLoadData(data);
		}
	};

	// 接続表示優先順位
	auto const & applyConnectObjectDispPriority = [&parentInstanceIdDispPriorityList](agtk::Object* obj, const rapidjson::Value* connectObjectDispPriorityList) {
		if (connectObjectDispPriorityList) {
			std::list<DispPriorityInfo> dispPriorityInfoList;
			for (unsigned int i = 0; i < connectObjectDispPriorityList->Size(); ++i) {
				const auto &data = (*connectObjectDispPriorityList)[i];
				auto const instanceId = data["instanceId"].GetInt();
				auto const lowerPriority = data["lowerPriority"].GetBool();
				dispPriorityInfoList.emplace_back(DispPriorityInfo(instanceId, lowerPriority));
			}
			parentInstanceIdDispPriorityList.insert(std::make_pair(obj->getInstanceId(), dispPriorityInfoList));
		}
	};

	// テキストを表示
	auto const & applyMessageText = [&targetInstanceIdMessageTextList](agtk::Object* obj, const rapidjson::Value* messageTextList) {
		if (messageTextList != nullptr) {
			std::list<MessageTextInfo> messageTextInfoList;
			for (unsigned int i = 0; i < messageTextList->Size(); ++i) {
				const auto &data = (*messageTextList)[i];
				auto const instanceId = data["lockObjectInstanceId"].GetInt();
				auto const actionId = data["actionId"].GetInt();
				auto const commandDataId = data["commandDataId"].GetInt();
				messageTextInfoList.emplace_back(MessageTextInfo(actionId, commandDataId, instanceId));
			}
			targetInstanceIdMessageTextList.insert(std::make_pair(obj->getInstanceId(), messageTextInfoList));
		}
	};

	// テキストをスクロール表示
	auto const & applyMessageScrollText = [&targetInstanceIdMessageScrollTextList](agtk::Object* obj, const rapidjson::Value* messageScrollTextList) {
		if (messageScrollTextList != nullptr) {
			std::list<MessageScrollTextInfo> messageScrollTextInfoList;
			for (unsigned int i = 0; i < messageScrollTextList->Size(); ++i) {
				const auto &data = (*messageScrollTextList)[i];
				auto const instanceId = data["lockObjectInstanceId"].GetInt();
				auto const actionId = data["actionId"].GetInt();
				auto const commandDataId = data["commandDataId"].GetInt();
				messageScrollTextInfoList.emplace_back(MessageScrollTextInfo(actionId, commandDataId, instanceId));
			}
			targetInstanceIdMessageScrollTextList.insert(std::make_pair(obj->getInstanceId(), messageScrollTextInfoList));
		}
	};

// #AGTK-NX #AGTK-WIN
#ifdef USE_30FPS_4
	// 中間フレーム情報
	auto const & applyMiddleFrameStock = [](agtk::Object* obj, const rapidjson::Value* middleFrameStock) {
		if (middleFrameStock != nullptr) {
			obj->_middleFrameStock._middleFrameIndex = (*middleFrameStock)["middleFrameIndex"].GetInt();
			for (int i = 0; i < 2; i++) {
				const auto &frameData = (*middleFrameStock)["frameData"][i];
				auto mf = obj->_middleFrameStock._frameData + i;
				mf->_hasMiddleFrame = frameData["hasMiddleFrame"].GetBool();
				if (mf->_hasMiddleFrame) {
					mf->_objectPos.x = frameData["objectPosX"].GetFloat();
					mf->_objectPos.y = frameData["objectPosY"].GetFloat();
					mf->_centerRotation = frameData["centerRotation"].GetFloat();
					mf->_innerScale.x = frameData["innerScaleX"].GetFloat();
					mf->_innerScale.y = frameData["innerScaleY"].GetFloat();
					mf->_innerRotation = frameData["innerRotation"].GetFloat();
					mf->_offset.x = frameData["offsetX"].GetFloat();
					mf->_offset.y = frameData["offsetY"].GetFloat();
					mf->_center.x = frameData["centerX"].GetFloat();
					mf->_center.y = frameData["centerY"].GetFloat();
				}
			}
		}
	};
#endif


	// オブジェクトを生成するメソッド
	auto createObjectFunc = [&](agtk::Scene *scene, agtk::SceneLayer *sceneLayer, const rapidjson::Value& objJson )-> agtk::Object* {
		auto const layerId = objJson["layerId"].GetInt();
		auto const objectId = objJson["objectId"].GetInt();
		auto const sceneId = objJson["sceneId"].GetInt();
		auto const scenePartsId = objJson["scenePartsId"].GetInt();
		auto const initialActionId = objJson["initialActionId"].GetInt();
		auto const prevActionId = jsonUtil::getInt(objJson, "prevActionId", initialActionId);
		auto const instanceId = jsonUtil::getInt(objJson, "instanceId");
		auto px = objJson["px"].GetFloat();
		auto py = objJson["py"].GetFloat();
		// 「カメラとの位置関係を固定する」設定時
		auto projectData = GameManager::getInstance()->getProjectData();
		auto objectData = projectData->getObjectData(objectId);
		if (objectData->getFixedInCamera() && sceneLayer->getType() != agtk::SceneLayer::kTypeMenu) {
			auto pos = agtk::Scene::getPositionSceneFromCocos2d(cocos2d::Vec2(px, py) + scene->getCamera()->getPosition());
			px = pos.x;
			py = pos.y;
		}
		auto const sx = (objJson.HasMember("sx")) ? objJson["sx"].GetFloat() : 1.0f;
		auto const sy = (objJson.HasMember("sy")) ? objJson["sy"].GetFloat() : 1.0f;
		auto const dir = objJson["directionNo"].GetInt();
		auto const courseId = (objJson.HasMember("courseId")) ? objJson["courseId"].GetInt() : -1;
		auto const coursePointId = (objJson.HasMember("coursePointId")) ? objJson["coursePointId"].GetInt() : -1;
		auto const &playData = objJson["playData"];
		auto const & shaderList = objJson.HasMember("shader") ? &(objJson["shader"]) : nullptr;
		auto const & connectObjectList = objJson.HasMember("connectObject") ? &(objJson["connectObject"]) : nullptr;
		auto const & childObjectList = objJson.HasMember("childObject") ? &(objJson["childObject"]) : nullptr;
		auto const & objectCourseMoveData = objJson.HasMember("objectCourseMoveData") ? &(objJson["objectCourseMoveData"]) : nullptr;
		auto const takeOverAnimMotionId = (objJson.HasMember("takeOverAnimMotionId")) ? objJson["takeOverAnimMotionId"].GetInt() : -1;
		auto const & connectObjectDispPriorityList = objJson.HasMember("connectObjectDispPriority") ? &(objJson["connectObjectDispPriority"]) : nullptr;
		auto const &messageText = objJson.HasMember("messageText") ? &(objJson["messageText"]) : nullptr;
		auto const &messageScrollText = objJson.HasMember("messageScrollText") ? &(objJson["messageScrollText"]) : nullptr;
		auto scenePartObjectData = scene->getSceneData()->getScenePartObjectData(scenePartsId);
// #AGTK-NX #AGTK-WIN
#ifdef USE_30FPS_4
		auto const & middleFrameStock = objJson.HasMember("middleFrameStock") ? &(objJson["middleFrameStock"]) : nullptr;
#endif

		// プレイヤーオブジェクトを生成
		auto newObject = agtk::Object::createScenePartObjectData(
			sceneLayer,
			objectId,
			initialActionId,
			Vec2(px, py),
			cocos2d::Vec2(sx, sy),//scale
			0,//rotation
			dir,// directionId
			courseId,
			coursePointId,
			takeOverAnimMotionId,
			scenePartObjectData
			);
		if (objJson.HasMember("moveDirection")) {
			auto moveDirection = cocos2d::Vec2(objJson["moveDirection"][0].GetDouble(), objJson["moveDirection"][1].GetDouble());
			newObject->getObjectMovement()->setDirection(moveDirection);
		}
		newObject->setPrevObjectActionId(prevActionId);
		auto currentAction = newObject->getCurrentObjectAction();
		if (currentAction) {
			currentAction->setPreActionID(prevActionId);
		}
		newObject->setTakeOverAnimMotionId(takeOverAnimMotionId);

		// プレイデータ上書き
		auto playObjectData = agtk::data::PlayObjectData::create(playData);
		playObjectData->setup(newObject->getObjectData());
		newObject->setPlayObjectData(playObjectData);
		playObjectData->setObjectId(objectId);//オブジェクトID設定。
		// オブジェクトをロードした際はセーブ時のinstanceIdとなるので
		// ここで設定されたinstanceIdをシーンの_instanceIdへ上書きする
		scene->forceUpdateObjectInstanceId(instanceId, newObject->getSceneData()->isMenuScene());
		// 初回アップデート関数をロード用に変更
		newObject->setUpdateOneshot(true);

		applyShader(newObject, shaderList);

		applyConnectObject(newObject, connectObjectList);
		applyChildObject(newObject, childObjectList);
		applyObjectCourseMove(newObject, objectCourseMoveData);
		applyConnectObjectDispPriority(newObject, connectObjectDispPriorityList);
		applyMessageText(newObject, messageText);
		applyMessageScrollText(newObject, messageScrollText);
// #AGTK-NX #AGTK-WIN
#ifdef USE_30FPS_4
		applyMiddleFrameStock(newObject, middleFrameStock);
#endif

		// 最初に生成されたシーンのIDを上書き
		newObject->setSceneIdOfFirstCreated(sceneId);

		// シーンパーツIDを上書き
		newObject->setScenePartsId(scenePartsId);

		// シーンパーツデータを設定
		if (sceneId > 0 && scenePartsId > 0) {
			auto projectData = GameManager::getInstance()->getProjectData();
			auto sceneData = projectData->getSceneData(sceneId);
			auto scenePartsData = sceneData->getScenePartObjectData(scenePartsId);
			newObject->setScenePartObjectData(scenePartsData);
		}

		//オブジェクトをレイヤーに追加。
		sceneLayer->addCollisionDetaction(newObject);
		newObject->setId(sceneLayer->publishObjectId());
		newObject->getPlayObjectData()->setInstanceCount(scene->incrementObjectInstanceCount(newObject->getObjectData()->getId()));
		scene->updateObjectInstanceCount(newObject->getObjectData()->getId());
		newObject->setLayerId(layerId);
		newObject->setPhysicsBitMask(layerId, sceneLayer->getSceneData()->getId());

		auto viewportLightSceneLayer = scene->getViewportLight()->getViewportLightSceneLayer(layerId);
		auto newObjectData = newObject->getObjectData();
		if (viewportLightSceneLayer && newObjectData->getViewportLightSettingFlag() && newObjectData->getViewportLightSettingList()->count() > 0) {
			auto viewportLightObject = agtk::ViewportLightObject::create(newObject, scene->getViewportLight(), sceneLayer);
			viewportLightSceneLayer->getViewportLightObjectList()->addObject(viewportLightObject);
		}
		sceneLayer->addObject(newObject);

		// オブジェクトに紐付いた物理オブジェクトを生成
		sceneLayer->createPhysicsObjectWithObject(newObject);

		//デバッグ表示
		if (agtk::DebugManager::getInstance()->getDebugForDevelopment()) {
			auto primitiveManager = PrimitiveManager::getInstance();
			//生成オブジェクト
			auto objRect = newObject->getRect();
			objRect = agtk::Scene::getRectSceneFromCocos2d(objRect);
			auto objRectangle = agtk::PolygonShape::createRectangle(objRect, 0.0f);
			auto polyObj = primitiveManager->createPolygon(0, 0, objRectangle->_vertices, objRectangle->_segments, cocos2d::Color4F(1, 1, 1, 1), cocos2d::Color4F(1, 1, 1, 0.5));
			primitiveManager->setTimer(polyObj, 2.0f);
			newObject->addChild(polyObj);
		}

		currentAction = newObject->getCurrentObjectAction();
		if (currentAction) {
			currentAction->setDisableChangingFileSaveSwitchNextExecOtherAction(true);
		}

		CCLOG("Create Obj:[%s] id:%d (%d), layerId:%d (%d), instance:%d , pos: %f, %f", newObject->getObjectData()->getName(), newObject->getObjectData()->getId(), objectId, newObject->getLayerId(), layerId, instanceId, px, py);

		return newObject;
	};

	auto createConnectObjectFunc = [&](agtk::Scene *scene, agtk::SceneLayer *sceneLayer, const rapidjson::Value& objJson, agtk::Object *parentObject)-> agtk::Object* {

		CC_ASSERT(parentObject);

		auto const instanceId = jsonUtil::getInt(objJson, "instanceId");

		auto connectObjectLoadList = parentObject->getConnectObjectLoadList();
		agtk::Object::ConnectObjectLoadList *connectObjectLoadData = nullptr;
// #AGTK-NX, #AGTK-WIN
#ifdef USE_SAR_PROVISIONAL_2
		agtk::Object::ConnectObjectLoadList tempConnectObjectLoadData;
#endif
		int idx = -1;
		for (auto data : connectObjectLoadList) {
			idx += 1;
			if (instanceId == data.instanceId) {
// #AGTK-NX, #AGTK-WIN
#ifdef USE_SAR_PROVISIONAL_2
				tempConnectObjectLoadData = data;
				connectObjectLoadData = &tempConnectObjectLoadData;
#else
				connectObjectLoadData = &data;
#endif
				break;
			}
		}
		if (connectObjectLoadData == nullptr) {
			return nullptr;
		}
		connectObjectLoadList.erase(connectObjectLoadList.begin() + idx);
		parentObject->setConnectObjectLoadList(connectObjectLoadList);
		auto const &playData = objJson["playData"];
		auto const objectId = objJson["objectId"].GetInt();
		auto const initialActionId = objJson["initialActionId"].GetInt();
		auto const prevActionId = jsonUtil::getInt(objJson, "prevActionId", initialActionId);
		auto px = objJson["px"].GetFloat();
		auto py = objJson["py"].GetFloat();
		auto const sx = (objJson.HasMember("sx")) ? objJson["sx"].GetFloat() : 1.0f;
		auto const sy = (objJson.HasMember("sy")) ? objJson["sy"].GetFloat() : 1.0f;
		auto const dir = objJson["directionNo"].GetInt();
		auto const & shaderList = objJson.HasMember("shader") ? &(objJson["shader"]) : nullptr;
		auto const & connectObjectList = objJson.HasMember("connectObject") ? &(objJson["connectObject"]) : nullptr;
		auto const & childObjectList = objJson.HasMember("childObject") ? &(objJson["childObject"]) : nullptr;
		auto const & objectCourseMoveData = objJson.HasMember("objectCourseMoveData") ? &(objJson["objectCourseMoveData"]) : nullptr;
		auto const takeOverAnimMotionId = (objJson.HasMember("takeOverAnimMotionId")) ? objJson["takeOverAnimMotionId"].GetInt() : -1;
		auto const & connectObjectDispPriorityList = objJson.HasMember("connectObjectDispPriority") ? &(objJson["connectObjectDispPriority"]) : nullptr;

		// 「カメラとの位置関係を固定する」設定時
		auto projectData = GameManager::getInstance()->getProjectData();
		auto objectData = projectData->getObjectData(objectId);
		if (objectData->getFixedInCamera() && sceneLayer->getType() != agtk::SceneLayer::kTypeMenu) {
			auto pos = agtk::Scene::getPositionSceneFromCocos2d(cocos2d::Vec2(px, py) + scene->getCamera()->getPosition());
			px = pos.x;
			py = pos.y;
		}
		cocos2d::Vec2 position = cocos2d::Vec2(px, py);

		auto connectObject = agtk::ConnectObject::create2(parentObject, connectObjectLoadData->settingId, cocos2d::Vec2(px, py), cocos2d::Vec2(sx, sy), initialActionId, dir);
		if (objJson.HasMember("moveDirection")) {
			auto moveDirection = cocos2d::Vec2(objJson["moveDirection"][0].GetDouble(), objJson["moveDirection"][1].GetDouble());
			connectObject->getObjectMovement()->setDirection(moveDirection);
		}

		// プレイデータ上書き
		// #AGTK-NX
#ifdef USE_SAR_OPTIMIZE_1
		// Objectのcreate時に設定したIdMap (InstanceId,ObjectId)をSceneLayerから一旦削除
		sceneLayer->removeIdMap(connectObject);
#endif
		auto playObjectData = agtk::data::PlayObjectData::create(playData);
		playObjectData->setup(connectObject->getObjectData());
		connectObject->setPlayObjectData(playObjectData);
		playObjectData->setObjectId(objectId);//オブジェクトID設定。
// #AGTK-NX
#ifdef USE_SAR_OPTIMIZE_1
		// 更新されたプレイデータによるIdMapをSceneLayerに再登録
		sceneLayer->addIdMap(connectObject);
#endif

		connectObject->setPrevObjectActionId(prevActionId);
		connectObject->getCurrentObjectAction()->setPreActionID(prevActionId);
		connectObject->setTakeOverAnimMotionId(takeOverAnimMotionId);

		applyShader(connectObject, shaderList);
		applyConnectObject(connectObject, connectObjectList);
		applyChildObject(connectObject, childObjectList);
		applyObjectCourseMove(connectObject, objectCourseMoveData);

		scene->forceUpdateObjectInstanceId(instanceId, connectObject->getSceneData()->isMenuScene());
		// 初回アップデート関数をロード用に変更
		connectObject->setUpdateOneshot(true);

		// 物理ノードにも座標を反映する
		auto physicsNode = connectObject->getphysicsNode();
		if (physicsNode) {
			auto player = connectObject->getPlayer();
			physicsNode->setPosition(player ? player->getPosition() : agtk::Scene::getPositionSceneFromCocos2d(position, scene));
		}

		// スケールを反映
		auto variableScalingX = playObjectData->getVariableData(agtk::data::kObjectSystemVariableScalingX);
		auto variableScalingY = playObjectData->getVariableData(agtk::data::kObjectSystemVariableScalingY);
		connectObject->setScale(Vec2(variableScalingX->getValue() * 0.01f, variableScalingY->getValue() * 0.01f));

		// 位置を直接変更したので衝突判定を更新する
		connectObject->getObjectCollision()->updateWallHitInfoGroup();
		connectObject->getObjectCollision()->lateUpdateWallHitInfoGroup();

		parentObject->getConnectObjectList()->addObject(connectObject);

		// 接続するオブジェクトに紐付いた物理オブジェクトを生成する
		parentObject->getSceneLayer()->createPhysicsObjectWithObject(connectObject);

		return connectObject;
	};

	// 初期配置オブジェクトデータリストから漏れたオブジェクトのIDXリスト
	auto noInitCreatedObjDataList = cocos2d::__Array::create();

	if (_loadBit & kLoadBit_ObjectList) {
		this->getSceneEndTakeoverStatesObjectList()->removeAllObjects();
		// ----------------------------------------------------------------
		//「シーン終了時の状態を保持」の情報
		// ----------------------------------------------------------------
		if (doc["initEndTakeoverObjList"].Size() > 0) {
			auto sceneEndTakeoverStatesObjectList = this->getSceneEndTakeoverStatesObjectList();
			for (unsigned int idx = 0; idx < doc["initEndTakeoverObjList"].Size(); idx++) {
				const auto &initObjData = doc["initEndTakeoverObjList"][idx];

				auto objectId = initObjData["id"].GetInt();
				auto objectData = this->getProjectData()->getObjectData(objectId);
				auto playObjectData = agtk::data::PlayObjectData::create(initObjData["playData"]);
				playObjectData->setup(objectData);

				auto objectTakeoverStatesData = agtk::ObjectTakeoverStatesData::create(playObjectData);
				objectTakeoverStatesData->setObjectId(objectId);
				objectTakeoverStatesData->setSceneId(initObjData["sceneId"].GetInt());
				objectTakeoverStatesData->setSceneLayerId(initObjData["layerId"].GetInt());
				objectTakeoverStatesData->setScenePartsId(initObjData["scenePartsId"].GetInt());
				objectTakeoverStatesData->setActionNo(initObjData["actionNo"].GetInt());
				objectTakeoverStatesData->setDirectionNo(initObjData["directionNo"].GetInt());
				objectTakeoverStatesData->setDispDirection(initObjData["dispDirection"].GetInt());
				if (initObjData.HasMember("moveDirection")) {
					objectTakeoverStatesData->setMoveDirection(cocos2d::Vec2(initObjData["moveDirection"][0].GetDouble(), initObjData["moveDirection"][1].GetDouble()));
				}
				objectTakeoverStatesData->setPosition(cocos2d::Vec2(initObjData["px"].GetDouble(), initObjData["py"].GetDouble()));
				if (initObjData.HasMember("sx") && initObjData.HasMember("sy")) {
					objectTakeoverStatesData->setScale(cocos2d::Vec2(initObjData["sx"].GetFloat(), initObjData["sy"].GetFloat()));
				}
				objectTakeoverStatesData->setTakeOverAnimMotionId(initObjData["takeOverAnimMotionId"].GetInt());
				if (initObjData.HasMember("sceneIdOfFirstCreated")) {
					objectTakeoverStatesData->setSceneIdOfFirstCreated(initObjData["sceneIdOfFirstCreated"].GetInt());
				}
				else {
					objectTakeoverStatesData->setSceneIdOfFirstCreated(initObjData["sceneId"].GetInt());
				}
				sceneEndTakeoverStatesObjectList->addObject(objectTakeoverStatesData);
			}
		}

		// ----------------------------------------------------------------
		// 初期配置されているオブジェクト情報を復帰
		// ----------------------------------------------------------------
		CCLOG("データロード: シーンへの反映開始: 初期配置オブジェクトを復帰");

		// 座標とスケール設定用メソッド
		auto setPosAndScaleFunc = [](agtk::Scene *scene, agtk::Object *obj, agtk::data::PlayObjectData *playObjectData, Vec2 newPos) {

			auto objectData = obj->getObjectData();
			// 座標
			if (objectData->getFixedInCamera() && obj->getSceneLayer()->getType() != agtk::SceneLayer::kTypeMenu) {
				// 「カメラとの位置関係を固定する」設定時は、カメラ位置からの差分をポジションに設定する。
				obj->setObjectPosInCamera(newPos);
			} else {
				obj->setPosition(newPos);
				obj->setOldPosition(newPos);
				obj->setPremoveObjectPosition(newPos);
				playObjectData->getVariableData(agtk::data::kObjectSystemVariableX)->setExternalValue(newPos.x);
				playObjectData->getVariableData(agtk::data::kObjectSystemVariableY)->setExternalValue(newPos.y);
			}

			auto newPos2 = agtk::Scene::getPositionCocos2dFromScene(newPos, scene);
			// 物理ノードにも座標を反映する
			auto physicsNode = obj->getphysicsNode();
			if (physicsNode) {
				auto player = obj->getPlayer();
				physicsNode->setPosition(player ? player->getPosition() : newPos2);
			}
			auto physicsPartsList = obj->getPhysicsPartsList();
			if (physicsPartsList->count() > 0) {
				cocos2d::Ref *ref;
				CCARRAY_FOREACH(physicsPartsList, ref) {
					auto disk = dynamic_cast<agtk::PhysicsDisk *>(ref);
					auto rectangle = dynamic_cast<agtk::PhysicsRectangle *>(ref);
					if (disk != nullptr) {
						disk->setPosition(Vec2(newPos2.x + disk->getPhysicsData()->getX(), newPos2.y - disk->getPhysicsData()->getY()));
					}
					else if (rectangle != nullptr) {
						rectangle->setPosition(Vec2(newPos2.x + rectangle->getPhysicsData()->getX(), newPos2.y - rectangle->getPhysicsData()->getY()));
					}
				}
			}

			// スケールを反映
			auto variableScalingX = playObjectData->getVariableData(agtk::data::kObjectSystemVariableScalingX);
			auto variableScalingY = playObjectData->getVariableData(agtk::data::kObjectSystemVariableScalingY);
			obj->setScale(Vec2(variableScalingX->getValue() * 0.01f, variableScalingY->getValue() * 0.01f));

			// 位置を直接変更したので衝突判定を更新する
			obj->getObjectCollision()->updateWallHitInfoGroup();
			obj->getObjectCollision()->lateUpdateWallHitInfoGroup();

			//テンプレート終了
			obj->getObjectTemplateMove()->end();
		};

		// 保存された初期配置されているオブジェクト情報がある場合
		if (doc.HasMember("initObjDataList") && doc["initObjDataList"].Size() > 0) {

			auto applyInitObjDataList = [&](agtk::SceneLayer *sceneLayer){
				// シーン初期配置のオブジェクトがセーブ時に消滅していたら削除を行う
				{
					auto removeObjList = cocos2d::__Array::create();
					auto initialObjList = sceneLayer->getCreateObjectList();
					cocos2d::Ref *ref = nullptr;
					CCARRAY_FOREACH(initialObjList, ref) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
						auto obj = static_cast<agtk::Object *>(ref);
#else
						auto obj = dynamic_cast<agtk::Object *>(ref);
#endif
						auto objData = obj->getObjectData();
						bool isRemoveObj = true;
						for (unsigned int idx = 0; idx < doc["initObjDataList"].Size(); idx++) {
							const auto &initObjData = doc["initObjDataList"][idx];
							auto objectId = initObjData["objectId"].GetInt();
							auto scenePartsId = initObjData["scenePartsId"].GetInt();
							auto sceneId = initObjData["currentSceneId"].GetInt();
							if (objData->getId() != objectId) {
								continue;
							}
							if (obj->getSceneData()->getId() != sceneId) {
								continue;
							}
							if (obj->getScenePartsId() > 0 && obj->getScenePartsId() != scenePartsId) {
								continue;
							}
							isRemoveObj = false;
							break;
						}
						if (isRemoveObj) {
							removeObjList->addObject(obj);
						}
					}

					CCARRAY_FOREACH(removeObjList, ref) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
						auto removeObj = static_cast<agtk::Object *>(ref);
#else
						auto removeObj = dynamic_cast<agtk::Object *>(ref);
#endif
						removeObj->removeSelf(false, true);
					}
				}

				// 初期配置オブジェクトの復帰
				auto loadedObjList = cocos2d::__Array::create();
				for (unsigned int idx = 0; idx < doc["initObjDataList"].Size(); idx++) {

					const auto &initObjData = doc["initObjDataList"][idx];
					auto objectId = initObjData["objectId"].GetInt();
					auto instanceId = initObjData["instanceId"].GetInt();
					auto sceneId = initObjData["currentSceneId"].GetInt();
					auto layerId = initObjData["layerId"].GetInt();
					auto scenePartsId = initObjData["scenePartsId"].GetInt();
					bool isLoaded = false;

					if (sceneId != sceneLayer->getSceneData()->getId() || layerId != sceneLayer->getLayerId()) {
						continue;
					}

					// 初期生成されるオブジェクトを走査
					{
						auto initialObjList = sceneLayer->getCreateObjectList();
						cocos2d::Ref *ref = nullptr;
						CCARRAY_FOREACH(initialObjList, ref) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
							auto obj = static_cast<agtk::Object *>(ref);
#else
							auto obj = dynamic_cast<agtk::Object *>(ref);
#endif
							auto objData = obj->getObjectData();

							// ロードするデータのオブジェクトIDと異なる または 既にロード済の場合
							if (objData->getId() != objectId || loadedObjList->getIndexOfObject(obj) != CC_INVALID_INDEX) {
								continue;
							}
							if (obj->getScenePartsId() > 0 && obj->getScenePartsId() != scenePartsId) {
								continue;
							}

							// プレイデータ反映
// #AGTK-NX
#ifdef USE_SAR_OPTIMIZE_1
							// Objectのcreate時に設定したIdMap (InstanceId,ObjectId)をSceneLayerから一旦削除
							auto tmpSceneLayer = obj->getSceneLayer();
							tmpSceneLayer->removeIdMap(obj);
#endif
							auto playObjectData = agtk::data::PlayObjectData::create(initObjData["playData"]);
							playObjectData->setup(obj->getObjectData());
							obj->setPlayObjectData(playObjectData);
							playObjectData->setObjectId(objectId);//オブジェクトID設定。
// #AGTK-NX
#ifdef USE_SAR_OPTIMIZE_1
							// 更新されたプレイデータによるIdMapをSceneLayerに再登録
							tmpSceneLayer->addIdMap(obj);
#endif
							// 初回アップデート関数をロード用に変更
							obj->setUpdateOneshot(true);

							// オブジェクトの接続、子オブジェクト反映
							auto const & connectObjectList = initObjData.HasMember("connectObject") ? &(initObjData["connectObject"]) : nullptr;
							auto const & childObjectList = initObjData.HasMember("childObject") ? &(initObjData["childObject"]) : nullptr;
							auto const & objectCourseMoveData = initObjData.HasMember("objectCourseMoveData") ? &(initObjData["objectCourseMoveData"]) : nullptr;
							auto const &messageText = initObjData.HasMember("messageText") ? &(initObjData["messageText"]) : nullptr;
							auto const &messageScrollText = initObjData.HasMember("messageScrollText") ? &(initObjData["messageScrollText"]) : nullptr;

							applyConnectObject(obj, connectObjectList);
							applyChildObject(obj, childObjectList);
							applyObjectCourseMove(obj, objectCourseMoveData);
							applyMessageText(obj, messageText);
							applyMessageScrollText(obj, messageScrollText);

							// オブジェクトが無効化されている場合
							if (obj->getDisabled()) {
								// 無効化を一旦解除してから非表示後に再度無効化
								obj->setDisabled(false);
								obj->setVisible(false);
								obj->update(0);
								obj->setDisabled(true);
							}

							//ACT2-4366 playActionで壁判定wallCollisionCorrection()を行うので先に座標をセットする
							// 座標とスケールを設定
							auto newPos = Vec2(initObjData["px"].GetDouble(), initObjData["py"].GetDouble());
							setPosAndScaleFunc(scene, obj, playObjectData, newPos);

							// 初期アクションを設定
							int initialActionId = initObjData.HasMember("initialActionId") ? initObjData["initialActionId"].GetInt() : -1;
							if (initialActionId < 0) {
								initialActionId = objData->getInitialActionId();
							}
							auto const & moveDirectionId = initObjData["directionNo"].GetInt();

							// 遷移前のモーションを引き継ぐ場合
							int takeOverAnimMotionId = initObjData.HasMember("takeOverAnimMotionId") ? initObjData["takeOverAnimMotionId"].GetInt() : -1;
							if (takeOverAnimMotionId > -1) {
								obj->setCurrentAnimMotionId(takeOverAnimMotionId);
								// 遷移前のモーションを引き継ぐかつ移動方向が同じ場合は現在のアニメーションを引き続き再生されるため、
								// 再度再生させるために必ず被らない移動方向を設定する
								obj->setMoveDirection(moveDirectionId+1);
							}
							if (initObjData.HasMember("moveDirection")) {
								auto moveDirection = cocos2d::Vec2(initObjData["moveDirection"][0].GetDouble(), initObjData["moveDirection"][1].GetDouble());
								obj->getObjectMovement()->setDirection(moveDirection);
							}

							if (obj->getCurrentObjectAction()) {
								obj->playAction(initialActionId, moveDirectionId);
							}
							auto currentAction = obj->getCurrentObjectAction();
							if (currentAction) {
								currentAction->setDisableChangingFileSaveSwitchNextExecOtherAction(true);
							}
							// シェーダ反映
							// アクションの実行でシェーダが適用される可能性があるので、playAction よりあとにシェーダを適用することで上書き。
							applyShader(obj, initObjData.HasMember("shader") ? &(initObjData["shader"]) : nullptr);

							CCLOG("load initialObj:[%s] id:%d (%d), instanceId:%d (%d), layerId:%d (%d), pos: %f, %f", obj->getObjectData()->getName(), obj->getObjectData()->getId(), objectId, obj->getInstanceId(), instanceId, obj->getLayerId(), layerId, initObjData["px"].GetDouble(), initObjData["py"].GetDouble());

							// 生成直後のレイヤーIDとロードデータのレイヤーIDが異なる場合
							if (obj->getLayerId() != layerId) {
								// レイヤーを移動させる
								changeLayerFunc(scene, obj, layerId);
							}

							isLoaded = true;
							loadedObjList->addObject(obj);

							// オブジェクトをロードした際はセーブ時のinstanceIdとなるので
							// ここで設定されたinstanceIdをシーンの_instanceIdへ上書きする
							scene->forceUpdateObjectInstanceId(instanceId, obj->getSceneData()->isMenuScene());
							break;
						}

						if (isLoaded) {
							continue;
						}
					}
					// 復活不可になっているオブジェクトを走査
					{
						auto tmpReappearObjectList = cocos2d::__Array::create();
						auto notReappearObjectList = GameManager::getInstance()->getNotReappearObjectList();
						cocos2d::Ref *ref = nullptr;
						CCARRAY_FOREACH(notReappearObjectList, ref) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
							auto reappearData = static_cast<agtk::data::ObjectReappearData*>(ref);
#else
							auto reappearData = dynamic_cast<agtk::data::ObjectReappearData*>(ref);
#endif

							// ロードするデータのオブジェクトIDと異なる場合
							if (reappearData->getObjectId() != objectId) {
								tmpReappearObjectList->addObject(reappearData);
								continue;
							}

							// オブジェクト生成
							createObjectFunc(scene, sceneLayer, initObjData);

							isLoaded = true;
							break;
						}

						if (isLoaded) {
							// ロードしたオブジェクトを除いた復活不可リストへと更新
							notReappearObjectList->removeAllObjects();
							GameManager::getInstance()->setNotReappearObjectList(tmpReappearObjectList);
							continue;
						}
					}
					// 出現条件を満たせていないオブジェクトを走査
					{
						auto uncreateObjList = sceneLayer->getUncreateObjectList();
						cocos2d::Ref *ref = nullptr;
						CCARRAY_FOREACH(uncreateObjList, ref) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
							auto scenePartObjectData = static_cast<agtk::data::ScenePartObjectData*>(ref);
#else
							auto scenePartObjectData = dynamic_cast<agtk::data::ScenePartObjectData*>(ref);
#endif

							// ロードするデータのオブジェクトIDと異なる場合
							if (scenePartObjectData->getObjectId() != objectId) {
								continue;
							}
							if (scenePartObjectData->getId() != scenePartsId) {
								continue;
							}

							// オブジェクト生成
							createObjectFunc(scene, sceneLayer, initObjData);

							isLoaded = true;
							uncreateObjList->removeObject(scenePartObjectData);
							break;
						}
					}

					if (isLoaded) {
						continue;
					}

					noInitCreatedObjDataList->addObject(Integer::create(idx));
				}
			};
			cocos2d::DictElement *el2 = nullptr;
			CCDICT_FOREACH(sceneLayerList, el2) {
				auto sceneLayer = dynamic_cast<agtk::SceneLayer *>(el2->getObject());
				applyInitObjDataList(sceneLayer);
			}
			CCDICT_FOREACH(menuLayerList, el2) {
				auto menuLayer = dynamic_cast<agtk::SceneLayer *>(el2->getObject());
				applyInitObjDataList(menuLayer);
			}
		}

		// 接続オブジェクトのインスタンスIDを取得。
		auto connectObjInstanceIdList = cocos2d::__Array::create();
		if (doc.HasMember("initObjDataList") && doc["initObjDataList"].Size() > 0) {

			for (unsigned int idx = 0; idx < doc["initObjDataList"].Size(); idx++) {
				const auto &initObjData = doc["initObjDataList"][idx];
				auto objectId = initObjData["objectId"].GetInt();
				auto const &connectObjectList = initObjData.HasMember("connectObject") ? &(initObjData["connectObject"]) : nullptr;

				for (unsigned int i = 0; i < connectObjectList->Size(); ++i) {
					const auto &connectObjectData = (*connectObjectList)[i];
					auto const instanceId = connectObjectData["instanceId"].GetInt();
					connectObjInstanceIdList->addObject(Integer::create(instanceId));
				}
			}
		}

		// ----------------------------------------------------------------
		// 初期配置オブジェクトとして生成されなかったオブジェクトの復帰
		// ----------------------------------------------------------------
		if (noInitCreatedObjDataList->count() > 0) {
			CCLOG("** [LOAD DATA] 初期オブジェクトとして生成されなかったオブジェクトのロード **");
			cocos2d::Ref *ref = nullptr;

			auto projectData = GameManager::getInstance()->getProjectData();
			CC_ASSERT(projectData);

			//接続オブジェクトではないオブジェクトをシーンに生成する。
			auto createdObjDataList = cocos2d::__Array::create();
			CCARRAY_FOREACH(noInitCreatedObjDataList, ref) {

// #AGTK-NX
#ifdef STATIC_DOWN_CAST
				int dataIdx = static_cast<cocos2d::Integer *>(ref)->getValue();
#else
				int dataIdx = dynamic_cast<cocos2d::Integer *>(ref)->getValue();
#endif
				const auto &initObjData = doc["initObjDataList"][dataIdx];
				auto const instanceId = initObjData["instanceId"].GetInt();
				auto const objectId = initObjData["objectId"].GetInt();

				//接続オブジェクトかチェックする。
				bool bConnectObject = false;
				cocos2d::Ref *ref2;
				CCARRAY_FOREACH(connectObjInstanceIdList, ref2) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
					auto idx = static_cast<cocos2d::Integer *>(ref2);
#else
					auto idx = dynamic_cast<cocos2d::Integer *>(ref2);
#endif
					if (idx->getValue() == instanceId) {
						bConnectObject = true;
						break;
					}
				}
				if (bConnectObject) {
					continue;
				}

				auto objectData = projectData->getObjectData(objectId);
				if (objectData != nullptr) {
					auto sceneLayer = scene->getSceneLayer(initObjData["layerId"].GetInt());
					createObjectFunc(scene, sceneLayer, initObjData);
				}
				createdObjDataList->addObject(ref);
			}
			if (createdObjDataList->count() > 0) {
				CCARRAY_FOREACH(createdObjDataList, ref) {
					noInitCreatedObjDataList->removeObject(ref);
				}
			}
		}

		// ----------------------------------------------------------------
		//「復活オブジェクト情報リスト」をシーンレイヤーに反映する。
		// ----------------------------------------------------------------
		if (doc.HasMember("reappearDataList") && doc["reappearDataList"].Size() > 0) {
			for (unsigned int idx = 0; idx < doc["reappearDataList"].Size(); idx++) {
				const auto &data = doc["reappearDataList"][idx];
				auto reappearData = agtk::data::ObjectReappearData::create();
				reappearData->setSceneId(data["sceneId"].GetInt());
				reappearData->setScenePartsId(data["scenePartsId"].GetInt());
				reappearData->setSceneLayerId(data["sceneLayerId"].GetInt());
				reappearData->setObjectId(data["objectId"].GetInt());
				reappearData->setInitialActionId(data["initialActionId"].GetInt());
				cocos2d::Vec2 initialPosition(data["initialPositionX"].GetFloat(), data["initialPositionY"].GetFloat());
				reappearData->setInitialPosition(initialPosition);
				cocos2d::Vec2 initialScale(data["initialScaleX"].GetFloat(), data["initialScaleY"].GetFloat());
				reappearData->setInitialScale(initialScale);
				reappearData->setInitialRotation(data["initialRotation"].GetFloat());
				reappearData->setInitialMoveDirectionId(data["initialMoveDirectionId"].GetInt());
				reappearData->setReappearFlag(data["reappearFlag"].GetBool());
				reappearData->setInitialCourseId(data["initialCourseId"].GetInt());
				reappearData->setInitialCoursePointId(data["InitialCoursePointId"].GetInt());
				auto sceneLayer = scene->getSceneLayer(reappearData->getSceneLayerId());
				if (sceneLayer != nullptr) {
					auto deleteObjectList = sceneLayer->getDeleteObjectList();
					if (deleteObjectList->containsObject(reappearData) == false) {
						sceneLayer->getDeleteObjectList()->addObject(reappearData);
					}
				}
			}
		}

		// ----------------------------------------------------------------
		//「復活しないオブジェクト情報リスト」をシーンレイヤーに反映する。
		// ----------------------------------------------------------------
		if (doc.HasMember("notReappearDataList") && doc["notReappearDataList"].Size() > 0) {

			auto tmpReappearObjectList = cocos2d::__Array::create();
			for (unsigned int idx = 0; idx < doc["notReappearDataList"].Size(); idx++) {
				const auto &data = doc["notReappearDataList"][idx];
				auto reappearData = agtk::data::ObjectReappearData::create();
				reappearData->setSceneId(data["sceneId"].GetInt());
				reappearData->setScenePartsId(data["scenePartsId"].GetInt());
				reappearData->setSceneLayerId(data["sceneLayerId"].GetInt());
				reappearData->setObjectId(data["objectId"].GetInt());
				reappearData->setInitialActionId(data["initialActionId"].GetInt());
				cocos2d::Vec2 initialPosition(data["initialPositionX"].GetFloat(), data["initialPositionY"].GetFloat());
				reappearData->setInitialPosition(initialPosition);
				cocos2d::Vec2 initialScale(data["initialScaleX"].GetFloat(), data["initialScaleY"].GetFloat());
				reappearData->setInitialScale(initialScale);
				reappearData->setInitialRotation(data["initialRotation"].GetFloat());
				reappearData->setInitialMoveDirectionId(data["initialMoveDirectionId"].GetInt());
				reappearData->setReappearFlag(data["reappearFlag"].GetBool());
				reappearData->setInitialCourseId(data["initialCourseId"].GetInt());
				reappearData->setInitialCoursePointId(data["InitialCoursePointId"].GetInt());

				tmpReappearObjectList->addObject(reappearData);
			}
			GameManager::getInstance()->setNotReappearObjectList(tmpReappearObjectList);

		}

		// ----------------------------------------------------------------
		//「実行アクションで復活するオブジェクト情報リスト」をGameManagerに反映する。
		// ----------------------------------------------------------------
		GameManager::getInstance()->getCommandReappearObjectList()->removeAllObjects(); //重複を避けるために反映前に今あるものをクリア

		if (doc.HasMember("commandReappearDataList") && doc["commandReappearDataList"].Size() > 0) {
			for (unsigned int idx = 0; idx < doc["commandReappearDataList"].Size(); idx++) {
				const auto &data = doc["commandReappearDataList"][idx];
				auto reappearData = agtk::data::ObjectReappearData::create();
				reappearData->setSceneId(data["sceneId"].GetInt());
				reappearData->setScenePartsId(data["scenePartsId"].GetInt());
				reappearData->setSceneLayerId(data["sceneLayerId"].GetInt());
				reappearData->setObjectId(data["objectId"].GetInt());
				reappearData->setInitialActionId(data["initialActionId"].GetInt());
				cocos2d::Vec2 initialPosition(data["initialPositionX"].GetFloat(), data["initialPositionY"].GetFloat());
				reappearData->setInitialPosition(initialPosition);
				cocos2d::Vec2 initialScale(data["initialScaleX"].GetFloat(), data["initialScaleY"].GetFloat());
				reappearData->setInitialScale(initialScale);
				reappearData->setInitialRotation(data["initialRotation"].GetFloat());
				reappearData->setInitialMoveDirectionId(data["initialMoveDirectionId"].GetInt());
				reappearData->setReappearFlag(data["reappearFlag"].GetBool());
				reappearData->setInitialCourseId(data["initialCourseId"].GetInt());
				reappearData->setInitialCoursePointId(data["InitialCoursePointId"].GetInt());
				GameManager::getInstance()->getCommandReappearObjectList()->addObject(reappearData);
			}
		}
	}

	if( _loadBit & kLoadBit_Scene )
	{
		// ----------------------------------------------------------------
		// プレイヤーオブジェクトの情報を復帰
		// ----------------------------------------------------------------
		// シーンからプレイヤーオブジェクトリストを取得
		auto playerObjList = scene->getObjectAllObjGroup(agtk::data::ObjectData::EnumObjGroup::kObjGroupPlayer, false, agtk::SceneLayer::kTypeScene);

		// 一旦すべて削除
		cocos2d::Ref* ref = nullptr;
		CCARRAY_FOREACH(playerObjList, ref) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
			auto p = static_cast<agtk::Object*>(ref);
#else
			auto p = dynamic_cast<agtk::Object*>(ref);
#endif
#if 1
			// ACT2-4369 SceneLayer->removeObject()ではオブジェクトに紐づいた物理パーツが残っているので
			// 通常動作でオブジェクトを削除している関数removeSelf()で削除
			bool bDisappearFlag = false;// オブジェクト消滅時の設定は行わない
			bool bIgnoredReappearCondition = true;//※シーン終了での消滅後の復活条件は無視。
			p->removeSelf(bDisappearFlag, bIgnoredReappearCondition);
#else
			bool bIgnoredReappearCondition = true;//※シーン終了での消滅後の復活条件は無視。
			scene->getSceneLayer(p->getLayerId())->removeObject(p, bIgnoredReappearCondition);
#endif
		}
		
		CCLOG("** [LOAD DATA] シーンにプレイヤーオブジェクトを生成 **");
		// シーンにプレイヤーオブジェクトを生成する
		for (auto itr = doc["playerDataList"].MemberBegin(); itr != doc["playerDataList"].MemberEnd(); ++itr) {
			const auto &playerData = itr->value;
#if defined(USE_RUNTIME)
			auto projectData = GameManager::getInstance()->getProjectData();
			if (projectData->getObjectData(playerData["objectId"].GetInt())->getTestplayOnly()) {
				continue;
			}
#endif
			//接続されているオブジェクトはここでは生成しない
			//ObjectDataList内のオブジェクトがこのインスタンスIDのオブジェクトと接続しているかどうか
			bool isConnection = false;
			for (int i = 0; i < doc["initObjDataList"].Size(); i++) {
				const auto &objectData = doc["initObjDataList"][i];
				if (objectData.HasMember("connectObject") && objectData["connectObject"].Size() > 0) {
					for (int j = 0; j < objectData["connectObject"].Size(); j++) {
						const auto &connectData = objectData["connectObject"][j];
						if (connectData["instanceId"].GetInt() == playerData["instanceId"].GetInt()) {
							isConnection = true;
						}
					}
				}
			}
			if (isConnection) {
				continue;
			}

			auto sceneLayer = scene->getSceneLayer(playerData["layerId"].GetInt());
			createObjectFunc(scene, sceneLayer, playerData);
		}
	}

	// 接続オブジェクトをシーンに生成する。
	if (noInitCreatedObjDataList->count() > 0) {

		auto projectData = GameManager::getInstance()->getProjectData();
		CC_ASSERT(projectData);
		cocos2d::Ref *ref;

		std::function<cocos2d::Integer *(cocos2d::__Array *, agtk::Object* &)> searchConnectObject = [&scene, &doc, &projectData](cocos2d::__Array *objectList, agtk::Object* &pobject)->cocos2d::Integer* {
			auto objectAllList = scene->getObjectAll();
			cocos2d::Ref *ref;
			CCARRAY_FOREACH(objectList, ref) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
				int dataIdx = static_cast<cocos2d::Integer *>(ref)->getValue();
#else
				int dataIdx = dynamic_cast<cocos2d::Integer *>(ref)->getValue();
#endif
				const auto &initObjData = doc["initObjDataList"][dataIdx];
				auto const instanceId = jsonUtil::getInt(initObjData, "instanceId");
				cocos2d::Ref *ref2;
				CCARRAY_FOREACH(objectAllList, ref2) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
					auto object = dynamic_cast<agtk::Object *>(ref2);
#else
					auto object = dynamic_cast<agtk::Object *>(ref2);
#endif
					if (instanceId == object->getInstanceId()) continue;
					auto connectObjectLoadList = object->getConnectObjectLoadList();
					for (auto connectObjectLoad : connectObjectLoadList) {
						if (connectObjectLoad.instanceId == instanceId) {
							pobject = object;
							return dynamic_cast<cocos2d::Integer *>(ref);
						}
					}
				}
			}
			pobject = nullptr;
			return nullptr;
		};

		lRetrySearchConnectObject:
		{
			agtk::Object *parentObject = nullptr;
			auto objectIdx = searchConnectObject(noInitCreatedObjDataList, parentObject);
			if (objectIdx != nullptr) {
				const auto &initObjData = doc["initObjDataList"][objectIdx->getValue()];
				auto const objectId = initObjData["objectId"].GetInt();
				auto objectData = projectData->getObjectData(objectId);
				if (objectData != nullptr) {
					auto sceneLayer = scene->getSceneLayer(initObjData["layerId"].GetInt());
					createConnectObjectFunc(scene, sceneLayer, initObjData, parentObject);
				}
				noInitCreatedObjDataList->removeObject(objectIdx);
				goto lRetrySearchConnectObject;
			}
		}

		// 残りのオブジェクトをシーンに生成する。
		CCARRAY_FOREACH(noInitCreatedObjDataList, ref) {

// #AGTK-NX
#ifdef STATIC_DOWN_CAST
			int dataIdx = static_cast<cocos2d::Integer *>(ref)->getValue();
#else
			int dataIdx = dynamic_cast<cocos2d::Integer *>(ref)->getValue();
#endif
			const auto &initObjData = doc["initObjDataList"][dataIdx];
			auto const objectId = initObjData["objectId"].GetInt();

			auto objectData = projectData->getObjectData(objectId);
			if (objectData) {
				auto sceneLayer = scene->getSceneLayer(initObjData["layerId"].GetInt());
				createObjectFunc(scene, sceneLayer, initObjData);
			}
		}
	}

	// 子オブジェクトリストを再設定する。
	for (auto &pair : parentInstanceIdChildInfoList) {
		auto parentInstanceId = pair.first;
		auto &childInfoList = pair.second;
		auto parentObj = this->getTargetObjectByInstanceId(parentInstanceId);
		for (auto childInfo : childInfoList) {
			auto childObj = this->getTargetObjectByInstanceId(childInfo.instanceId);
			childObj->getPlayObjectData()->setParentObjectInstanceId(parentObj->getInstanceId());
			parentObj->addChildObject(childObj, cocos2d::Vec2(childInfo.offsetX, childInfo.offsetY), childInfo.connectId);
		}
	}

	//接続表示優先順位
	for (auto pair : parentInstanceIdDispPriorityList) {
		auto parentInstanceId = pair.first;
		auto dispPriorityInfoList = pair.second;
		auto parentObj = this->getTargetObjectByInstanceId(parentInstanceId);
		for (auto info : dispPriorityInfoList) {
			auto connectObj = this->getTargetObjectByInstanceId(info.instanceId);
			parentObj->addConnectObjectDispPriority(connectObj, info.lowerPriority);
		}
	}

	auto guiManager = GuiManager::getInstance();
	//テキストを表示
	for (auto pair : targetInstanceIdMessageTextList) {
		auto targetInstanceId = pair.first;
		auto infoList = pair.second;
		auto targetObj = this->getTargetObjectByInstanceId(targetInstanceId);
		if (targetObj == nullptr) {
			//見つからない。
			continue;
		}
		for (auto info : infoList) {
			if (targetObj->getCurrentObjectAction()->getId() == info.actionId) {
				//アクションが同じ場合、２重になるため生成しないようにする。
				continue;
			}
			auto cmd = dynamic_cast<agtk::data::ObjectCommandMessageShowData *>(targetObj->getCommandData(info.actionId, info.commandDataId));
			if (cmd == nullptr) {
				//コマンドが見つからない。
				continue;
			}
			agtk::Object *lockObj = nullptr;
			if (info.instanceId >= 0) {
				lockObj = this->getTargetObjectByInstanceId(info.instanceId);
			}
			guiManager->addActionCommandMessageGui(targetObj, lockObj, cmd);
		}
	}

	//テキストをスクロール表示
	for (auto pair : targetInstanceIdMessageScrollTextList) {
		auto targetInstanceId = pair.first;
		auto infoList = pair.second;
		auto targetObj = this->getTargetObjectByInstanceId(targetInstanceId);
		if (targetObj == nullptr) {
			//見つからない。
			continue;
		}
		for (auto info : infoList) {
			if (targetObj->getCurrentObjectAction()->getId() == info.actionId) {
				//アクションが同じ場合、２重になるため生成しないようにする。
				continue;
			}
			auto cmd = dynamic_cast<agtk::data::ObjectCommandScrollMessageShowData *>(targetObj->getCommandData(info.actionId, info.commandDataId));
			if (cmd == nullptr) {
				//コマンドが見つからない。
				continue;
			}
			agtk::Object *lockObj = nullptr;
			if (info.instanceId >= 0) {
				lockObj = this->getTargetObjectByInstanceId(info.instanceId);
			}
			guiManager->addActionCommandScrollMessageGui(targetObj, lockObj, cmd);
		}
	}

	//ロードして単体インスタンスIDが変わりうるため、再設する。
	if (scene != nullptr) {
		auto projectPlayData = this->getPlayData();
// #AGTK-NX
#ifdef USE_SAR_OPTIMIZE_1
		auto objectList = scene->getObjectAllReference(agtk::SceneLayer::kTypeMax);
#else
		auto objectList = scene->getObjectAll(agtk::SceneLayer::kTypeMax);
#endif
		cocos2d::Ref *ref;
		CCARRAY_FOREACH(objectList, ref) {
			auto object = dynamic_cast<agtk::Object *>(ref);
			if (object) {
				auto playObjectData = object->getPlayObjectData();
				playObjectData->adjustData();
			}
		}
	}

	_loadBit = kLoadBit_All;// リセット
}

//シーン終了時の状態維持情報を破棄する
void GameManager::removeTakeoverStatesObject(int sceneId, int sceneLayerId)
{
	auto sceneEndTakeoverStatesObjectList = this->getSceneEndTakeoverStatesObjectList();
	cocos2d::Ref *ref;
	auto removeList = cocos2d::__Array::create();
	CCARRAY_FOREACH(sceneEndTakeoverStatesObjectList, ref) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
		auto data = static_cast<agtk::ObjectTakeoverStatesData *>(ref);
#else
		auto data = dynamic_cast<agtk::ObjectTakeoverStatesData *>(ref);
#endif
		if (data->getSceneId() == sceneId && data->getSceneLayerId() == sceneLayerId) {
			removeList->addObject(data);
		}
	}
	if (removeList->count()) {
		sceneEndTakeoverStatesObjectList->removeObjectsInArray(removeList);
	}
}

std::string GameManager::getProjectPathFromProjectFile(const std::string &filePath)
{
	auto it = filePath.end();
	int count = 2;
	while (--it >= filePath.begin()) {
		if (*it == '/') {
			count--;
			if (count == 0) {
				//found
				return filePath.substr(0, it - filePath.begin() + 1);
			}
		}
	}
	return std::string();
}

void GameManager::loadJsonFile(std::string projectFilePath)
{
	auto filePath = projectFilePath;
	setProjectFilePath(cocos2d::String::create(filePath));
#if 0
	std::string logStr;
	logStr += filePath + "\r\n";
	FileUtils::getInstance()->writeStringToFile(logStr, "C:\\tmp\\20170626\\player.log");
#endif
	auto projectPath = getProjectPathFromProjectFile(filePath);
#if 0
	logStr += projectPath + "\r\n";
	FileUtils::getInstance()->writeStringToFile(logStr, "C:/tmp/20170626/player.log");
#endif
	auto jsonData = FileUtils::getInstance()->getStringFromFile(filePath);
#if 0
	logStr += jsonData + "\r\n";
	FileUtils::getInstance()->writeStringToFile(logStr, "C:/tmp/20170626/player.log");
#endif
	GameManager::getInstance()->actionLog(1, "projectFilePath:");
	GameManager::getInstance()->actionLog(1, projectFilePath.c_str());
#ifndef USE_RUNTIME
	GameManager::getInstance()->actionLog(1, "project.json: jsonData:");
	GameManager::getInstance()->actionLog(1, jsonData.c_str());
#endif
	rapidjson::Document doc;
	doc.Parse(jsonData.c_str());
	bool error = doc.HasParseError();
	if (error) {
		CCASSERT(0, "Error: Json Parse.");
		return;
	}
	if (!this->getProjectData()) {
#ifdef USE_PRELOAD_TEX
		ProjectLoadingManager::getInstance()->preparePreloadTex(doc, projectPath);
#endif
		auto project = agtk::data::ProjectData::create(doc, projectPath);
		this->setProjectData(project);
	}

	//playData
	auto playData = agtk::data::PlayData::create(this->getProjectData());
	this->setPlayData(playData);
	updateFileExistSwitch();

#if 0//TODO セーブ保留。
	playData->save();
#endif

	// 設定データ読み込み
	loadConfig();

}

void GameManager::loadProjectData(std::string filePath)
{
	FileUtils::getInstance()->clearFileExistCache();	//プロジェクトが切り替わるタイミングでファイル存在キャッシュをクリアしておく。
// #AGTK-NX #AGTK-WIN
#ifdef USE_PRELOAD_TEX
	ProjectLoadingManager::getInstance()->load(filePath);
// #AGTK-WIN
#if defined(USE_PRELOAD_TEX) && !defined(USE_BG_PROJECT_LOAD)
	ImageMtTask::getInstance()->setEnable(false);
	ProjectLoadingManager::getInstance()->postProcessPreloadTex();
	ProjectLoadingManager::getInstance()->postProcessCacheTex();
#endif
#else
	this->loadJsonFile(filePath);
#endif
#ifdef USE_PREVIEW
	auto message = cocos2d::String::createWithFormat("system feedbackInfo { \"projectFilename\": \"%s\" }", filePath.c_str());
	auto ws = this->getWebSocket();
	if (ws) {
		ws->send(message->getCString());
	}
#endif
}

void GameManager::stopProjectData()
{
	if (getProjectData() == nullptr) {
		return;
	}
	auto scene = this->getCurrentScene();
	if (scene != nullptr) {
		scene->end();
	}
	this->setProjectData(nullptr);

	finalPlugins();

	auto inputManager = InputManager::getInstance();
	inputManager->setInputMappingData(nullptr);

	auto debugManager = agtk::DebugManager::getInstance();
	debugManager->removeDisplayData();

	if (this->getPortalTouchedPlayerList()) {
		this->getPortalTouchedPlayerList()->removeAllObjects();
	}
	if (this->getSceneChangeReappearObjectList()) {
		this->getSceneChangeReappearObjectList()->removeAllObjects();
	}
	if (this->getNotReappearObjectList()) {
		this->getNotReappearObjectList()->removeAllObjects();
	}
	if (this->getCommandReappearObjectList()) {
		this->getCommandReappearObjectList()->removeAllObjects();
	}
	if (this->getSceneEndTakeoverStatesObjectList()) {
		this->getSceneEndTakeoverStatesObjectList()->removeAllObjects();
	}
#if 1
#else
	JavascriptManager::getInstance()->term();
#endif

#ifdef USE_DESIGNATED_RESOLUTION_RENDER
	auto layer = this->getCurrentLayer();
	CC_ASSERT(layer);
	auto gameScene = dynamic_cast<GameScene *>(layer);
	if (gameScene != nullptr) {
		auto renderTexture = gameScene->getRenderTexture();
		renderTexture->removeAllCommand();
	}
#endif
// #AGTK-NX #AGTK-WIN
#ifdef USE_PRELOAD_TEX
	ProjectLoadingManager::getInstance()->removePreloadTex();
#endif
	this->setCurrentScene(nullptr);
}

void GameManager::startProjectData(int sceneId)
{
	AGTK_DEBUG_ACTION_LOG("%d,%s", __LINE__, __FUNCTION__);
	auto projectData = this->getProjectData();
	auto inputManager = InputManager::getInstance();
	inputManager->setInputMappingData(projectData->getInputMapping());
	agtk::setTileCollisionThreshold(projectData->getWallDetectionOverlapMargin());
	if (projectData->getMultithreading()) {
		ThreadManager::setUsedThreadCount(MAX(1, MIN(2, AGTK_THREAD_COUNT)));
	} else {
		ThreadManager::setUsedThreadCount(1);
	}

#ifdef USE_PLUGIN
#if 1
#else
	JavascriptManager::getInstance()->init();
	auto jsData = FileUtils::getInstance()->getStringFromFile("plugins/init.js");
	CCLOG("jsData: %s", jsData.c_str());
	JavascriptManager::getInstance()->initScript(jsData.c_str());
#endif
#endif
	//シーン削除。
	if(this->getCurrentScene()) {
		auto scene = this->getCurrentScene();
		if (scene != nullptr) {
			scene->end();
		}
		this->setCurrentScene(nullptr);
	}

	auto layer = this->getCurrentLayer();
	CC_ASSERT(layer);

#ifdef USE_PREVIEW
	// レイヤーの反転と回転をリセット
	auto baseLayer = dynamic_cast<agtk::BaseLayer *>(layer);
	baseLayer->reset();

	{
		auto debugManager = agtk::DebugManager::getInstance();
		debugManager->removeDisplayData();
		debugManager->getDebugExecuteLogWindow()->clear();

		auto primitiveManager = PrimitiveManager::getInstance();
		primitiveManager->removeAll();
	}
#endif

	if (sceneId < 0) {
		//「遷移」で登録されているSTART時のシーンを取得する。
		auto transitionFlow = projectData->getTransitionFlow();
		cocos2d::Ref *ref = nullptr;
		agtk::data::FlowScreenData *startFlowScreen = nullptr;
		CCARRAY_FOREACH(transitionFlow->getFlowScreenList(), ref) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
			auto flowScreen = static_cast<agtk::data::FlowScreenData *>(ref);
#else
			auto flowScreen = dynamic_cast<agtk::data::FlowScreenData *>(ref);
#endif
			if (flowScreen->getSceneMenuId() == agtk::data::FlowScreenData::SCENE_MENU_ID_START) {
				startFlowScreen = flowScreen;
				break;
			}
		}
		CCARRAY_FOREACH(transitionFlow->getFlowLinkList(), ref) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
			auto flowLink = static_cast<agtk::data::FlowLinkData *>(ref);
#else
			auto flowLink = dynamic_cast<agtk::data::FlowLinkData *>(ref);
#endif
			auto screenIdPair = flowLink->getscreenIdPair();
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
			auto flowScreenId = static_cast<cocos2d::Integer *>(screenIdPair->getObjectAtIndex(0));
#else
			auto flowScreenId = dynamic_cast<cocos2d::Integer *>(screenIdPair->getObjectAtIndex(0));
#endif
			if (flowScreenId->getValue() == startFlowScreen->getId()) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
				auto nextFlowScreenId = static_cast<cocos2d::Integer *>(screenIdPair->getObjectAtIndex(1));
#else
				auto nextFlowScreenId = dynamic_cast<cocos2d::Integer *>(screenIdPair->getObjectAtIndex(1));
#endif
				auto nextFlowScreenData = transitionFlow->getFlowScreenData(nextFlowScreenId->getValue());
				sceneId = nextFlowScreenData->getSceneMenuId();
				break;
			}
		}

		//「遷移」に登録されていない場合は、シーン配列の最初にする。
		if (sceneId < 0) {
			cocos2d::DictElement *el = nullptr;
			auto sceneList = projectData->getSceneList();
			CCDICT_FOREACH(sceneList, el) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
				auto sceneData = static_cast<agtk::data::SceneData *>(el->getObject());
#else
				auto sceneData = dynamic_cast<agtk::data::SceneData *>(el->getObject());
#endif
				if (sceneData->getFolder()) {
					continue;
				}
				sceneId = sceneData->getId();
				break;
			}
		}
		//なければ終了
		if (sceneId < 0) {
			CC_ASSERT(0);
			return;
		}
	}
#ifdef USE_PREVIEW
	if (sceneId < 0) {
		cocos2d::DictElement *el = nullptr;
		auto sceneList = projectData->getSceneList();
		CCDICT_FOREACH(sceneList, el) {
			auto sceneData = dynamic_cast<agtk::data::SceneData *>(el->getObject());
			if (sceneData->getFolder()) {
				continue;
			}
			sceneId = sceneData->getId();
			break;
		}
	}
	if (sceneId < 0) {
		CC_ASSERT(0);
		return;
	}
#endif
	AGTK_DEBUG_ACTION_LOG("%d,%s:%d", __LINE__, __FUNCTION__, sceneId);
	initPlugins();

	EffectManager::getInstance()->getDisabledLayerIdList()->removeAllObjects();
	ParticleManager::getInstance()->getDisabledLayerIdList()->removeAllObjects();
	InputManager::getInstance()->reset(true);

	//プロジェクトプレイデータを初期化。
	auto projectPlayData = GameManager::getInstance()->getPlayData();
	projectPlayData->reset();

	//サウンドをすべてストップする。
	auto audioManager = AudioManager::getInstance()->getInstance();
	audioManager->stopAllBgmNonObject();
	audioManager->stopAllSe();
	audioManager->stopAllVoice();
	audioManager->uncacheAll();

	auto debugManager = agtk::DebugManager::getInstance();
	CC_ASSERT(sceneId >= 0);
	auto scene = agtk::Scene::create(projectData->getSceneData(sceneId), 0);
	if (scene) {
		layer->addChild(scene, agtk::BaseLayer::ZOrder::Scene, agtk::BaseLayer::Tag::Scene);
		scene->start(layer);
		layer->onEnter();//collisionで再登録。
		if (debugManager->getSkipOneFrameWhenSceneCreatedIgnored() == false) {
			//生成時に１フレームスキップする。
#if 1	// ACT2-5107 テストプレイ開始時と同じ処理にする。
			auto gm = GameManager::getInstance();
			auto director = Director::getInstance();
			auto camera = scene->getCamera();
			auto duration = camera->getMoveDuration();
			camera->setMoveDuration(0.0f);//※カメラの移動補完の値を0にして、即指定カメラ位置へ移動。
			auto framePerSeconds = 0.0f;// director->getAnimationInterval();
										//※１フレームスキップするために、ここで２回更新する。
										//※更新中にオブジェクトが消滅すると該当のレンダリングコマンドが死亡するのでレンダリングも挟む
			auto alivePhysicsObj = gm->getAlivePhysicsObj();
			scene->update(framePerSeconds);
			director->getRenderer()->render();
			scene->update(framePerSeconds);
			director->getRenderer()->render();
			camera->setMoveDuration(duration);
			gm->setAlivePhysicsObj(alivePhysicsObj);
#else
			auto camera = scene->getCamera();
			auto duration = camera->getMoveDuration();
			camera->setMoveDuration(0.0f);
			scene->update(Director::getInstance()->getAnimationInterval());
			camera->setMoveDuration(duration);
#endif
		}
	}

	//サウンドボリューム設定。
	auto soundSettingData = projectData->getSoundSetting();
	audioManager->setBgmVolume(soundSettingData->getBgmVolume() * 0.01f);
	audioManager->setSeVolume(soundSettingData->getSeVolume() * 0.01f);
	audioManager->setVoiceVolume(soundSettingData->getVoiceVolume() * 0.01f);
	debugManager->getSoundData()->reset(soundSettingData->getBgmVolume(), soundSettingData->getSeVolume(), soundSettingData->getVoiceVolume());

	auto sceneData = scene->getSceneData();
	CCLOG("%s(%d)", sceneData->getName(), sceneData->getId());
	BgmInfo bgm;
	bgm._bgmId = sceneData->getBgmId();
	bgm._loop = sceneData->getLoopBgmFlag();
	this->calcMoveBgmEffect(0.0f, agtk::data::kBgmChangeTypeChange, false, { bgm }, false);

	debugManager->setShowMenuBar(projectData->getDisplayMenuBar());
	debugManager->createDisplayData();

	//cursor
	agtk::SetCursorVisible(projectData->getDisplayMousePointer());

	auto gameScene = dynamic_cast<GameScene *>(layer);
	if (gameScene != nullptr) {
		// カレントシーンリンクデータリスト更新
		gameScene->initCurSceneLinkDataList(sceneId);
	}
	AGTK_DEBUG_ACTION_LOG("%d,%s", __LINE__, __FUNCTION__);
}

void GameManager::restartProjectData()
{
	auto scene = this->getCurrentScene();
	if (scene == nullptr) {
		return;
	}
	auto sceneData = scene->getSceneData();
	int sceneId = sceneData->getId();
	scene->end();

	auto projectData = this->getProjectData();
	{
		//「遷移」で登録されているSTART時のシーンを取得する。
		auto transitionFlow = projectData->getTransitionFlow();
		cocos2d::Ref *ref = nullptr;
		agtk::data::FlowScreenData *startFlowScreen = nullptr;
		CCARRAY_FOREACH(transitionFlow->getFlowScreenList(), ref) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
			auto flowScreen = static_cast<agtk::data::FlowScreenData *>(ref);
#else
			auto flowScreen = dynamic_cast<agtk::data::FlowScreenData *>(ref);
#endif
			if (flowScreen->getSceneMenuId() == agtk::data::FlowScreenData::SCENE_MENU_ID_START) {
				startFlowScreen = flowScreen;
				break;
			}
		}
		CCARRAY_FOREACH(transitionFlow->getFlowLinkList(), ref) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
			auto flowLink = static_cast<agtk::data::FlowLinkData *>(ref);
#else
			auto flowLink = dynamic_cast<agtk::data::FlowLinkData *>(ref);
#endif
			auto screenIdPair = flowLink->getscreenIdPair();
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
			auto flowScreenId = static_cast<cocos2d::Integer *>(screenIdPair->getObjectAtIndex(0));
#else
			auto flowScreenId = dynamic_cast<cocos2d::Integer *>(screenIdPair->getObjectAtIndex(0));
#endif
			if (flowScreenId->getValue() == startFlowScreen->getId()) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
				auto nextFlowScreenId = static_cast<cocos2d::Integer *>(screenIdPair->getObjectAtIndex(1));
#else
				auto nextFlowScreenId = dynamic_cast<cocos2d::Integer *>(screenIdPair->getObjectAtIndex(1));
#endif
				auto nextFlowScreenData = transitionFlow->getFlowScreenData(nextFlowScreenId->getValue());
				sceneId = nextFlowScreenData->getSceneMenuId();
				break;
			}
		}

		//「遷移」に登録されていない場合は、シーン配列の最初にする。
		if (sceneId < 0) {
			cocos2d::DictElement *el = nullptr;
			auto sceneList = projectData->getSceneList();
			CCDICT_FOREACH(sceneList, el) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
				auto sceneData = static_cast<agtk::data::SceneData *>(el->getObject());
#else
				auto sceneData = dynamic_cast<agtk::data::SceneData *>(el->getObject());
#endif
				if (sceneData->getFolder()) {
					continue;
				}
				sceneId = sceneData->getId();
				break;
			}
		}
	}

	finalPlugins();

	auto layer = this->getCurrentLayer();

	// レイヤーの反転と回転を戻す
	auto baseLayer = dynamic_cast<agtk::BaseLayer *>(layer);
	if (baseLayer) {
		baseLayer->reset();
	}
	auto gameScene = dynamic_cast<GameScene *>(layer);
	if (gameScene) {
		gameScene->initCurSceneLinkDataList(sceneId);
	}

	auto debugManager = agtk::DebugManager::getInstance();
	debugManager->removeDisplayData();
	debugManager->getDebugExecuteLogWindow()->clear();

	auto primitiveManager = PrimitiveManager::getInstance();
	primitiveManager->removeAll();

	if (this->getPortalTouchedPlayerList()) {
		this->getPortalTouchedPlayerList()->removeAllObjects();
	}
	if (this->getSceneChangeReappearObjectList()) {
		this->getSceneChangeReappearObjectList()->removeAllObjects();
	}
	if (this->getNotReappearObjectList()) {
		this->getNotReappearObjectList()->removeAllObjects();
	}
	if (this->getCommandReappearObjectList()) {
		this->getCommandReappearObjectList()->removeAllObjects();
	}
	if (this->getSceneEndTakeoverStatesObjectList()) {
		this->getSceneEndTakeoverStatesObjectList()->removeAllObjects();
	}
	initPlugins();
#ifdef USE_PREVIEW
	JavascriptManager::loadAutoTestPlugins(_autoTestPluginFilePath, _autoTestPluginInternalJson, _autoTestPluginParamValueJson);
#endif

	EffectManager::getInstance()->getDisabledLayerIdList()->removeAllObjects();
	ParticleManager::getInstance()->getDisabledLayerIdList()->removeAllObjects();
	InputManager::getInstance()->reset(true);

	AnimationCache::destroyInstance();
	SpriteFrameCache::destroyInstance();
	GLProgramCache::destroyInstance();
	GLProgramStateCache::destroyInstance();

#if (CC_TARGET_PLATFORM == CC_PLATFORM_NX) // #AGTK-NX
#endif

	//プロジェクトプレイデータを初期化。
	auto projectPlayData = GameManager::getInstance()->getPlayData();
	projectPlayData->reset();

	//サウンドをすべて停止する。
	auto audioManager = AudioManager::getInstance()->getInstance();
	audioManager->stopAllBgmNonObject();
	audioManager->stopAllSe();
	audioManager->stopAllVoice();
	audioManager->uncacheAll();

	scene = agtk::Scene::create(projectData->getSceneData(sceneId), 0);
	if (scene) {
		layer->addChild(scene, agtk::BaseLayer::ZOrder::Scene, agtk::BaseLayer::Tag::Scene);
		scene->start(layer, false);
		layer->onEnter();//collisionで再登録。
		if (debugManager->getSkipOneFrameWhenSceneCreatedIgnored() == false) {
			//生成時に１フレームスキップする。
#if 1	// ACT2-5107 テストプレイ開始時と同じ処理にする。
			auto gm = GameManager::getInstance();
			auto director = Director::getInstance();
			auto camera = scene->getCamera();
			auto duration = camera->getMoveDuration();
			camera->setMoveDuration(0.0f);//※カメラの移動補完の値を0にして、即指定カメラ位置へ移動。
			auto framePerSeconds = 0.0f;// director->getAnimationInterval();
										//※１フレームスキップするために、ここで２回更新する。
										//※更新中にオブジェクトが消滅すると該当のレンダリングコマンドが死亡するのでレンダリングも挟む
			auto alivePhysicsObj = gm->getAlivePhysicsObj();
			scene->setSceneCreateSkipFrameFlag(true);
			{
				scene->update(framePerSeconds);
				director->getRenderer()->render();
				scene->update(framePerSeconds);
				director->getRenderer()->render();
			}
			scene->setSceneCreateSkipFrameFlag(false);
			camera->setMoveDuration(duration);
			gm->setAlivePhysicsObj(alivePhysicsObj);
#else
			auto camera = scene->getCamera();
			auto duration = camera->getMoveDuration();
			camera->setMoveDuration(0.0f);
			scene->update(Director::getInstance()->getAnimationInterval());
			camera->setMoveDuration(duration);
#endif
		}
	}

	//サウンドボリューム設定。
	auto soundSettingData = projectData->getSoundSetting();
	audioManager->setBgmVolume(soundSettingData->getBgmVolume() * 0.01f);
	audioManager->setSeVolume(soundSettingData->getSeVolume() * 0.01f);
	audioManager->setVoiceVolume(soundSettingData->getVoiceVolume() * 0.01f);

	sceneData = scene->getSceneData();
	debugManager->getSoundData()->reset(soundSettingData->getBgmVolume(), soundSettingData->getSeVolume(), soundSettingData->getVoiceVolume());
	CCLOG("%s(%d)", sceneData->getName(), sceneData->getId());
	BgmInfo bgm;
	bgm._bgmId = sceneData->getBgmId();
	bgm._loop = sceneData->getLoopBgmFlag();
	this->calcMoveBgmEffect(0.0f, agtk::data::kBgmChangeTypeChange, false, { bgm }, false);

	debugManager->createDisplayData();
}

void GameManager::loadAndStartProjectData(std::string filePath, int sceneId)
{
	this->stopProjectData();
	this->loadProjectData(filePath);
	this->startProjectData(sceneId);
}

void GameManager::startScene(int sceneId)
{
	_sceneStateInfo.state = kSceneStateStart;
	_sceneStateInfo.sceneId = sceneId;
}

void GameManager::restartScene()
{
	_sceneStateInfo.state = kSceneStateRestart;
}

void GameManager::stopScene()
{
	_sceneStateInfo.state = kSceneStateStop;
}

void GameManager::loadAndStartScene(std::string filePath, int sceneId)
{
	_sceneStateInfo.state = kSceneStateLoadAndStart;
	_sceneStateInfo.projectFilePath = filePath;
	_sceneStateInfo.sceneId = sceneId;
}

#ifdef USE_PREVIEW
void GameManager::loadProject(std::string filePath)
{
	_sceneStateInfo.state = kSceneStateLoad;
	_sceneStateInfo.projectFilePath = filePath;
}
#endif

void GameManager::updateSceneState()
{
	switch (_sceneStateInfo.state) {
	case kSceneStateIdle: {
		auto inputManager = InputManager::getInstance();
		//-----------------------------------------------------------------------------------------------------------
		auto resetTriggered = false;
		auto resetPressed = inputManager->isPressed(agtk::data::InputMappingData::kSystemKeyReset1OperationKeyId)
			&& inputManager->isPressed(agtk::data::InputMappingData::kSystemKeyReset2OperationKeyId)
			&& inputManager->isPressed(agtk::data::InputMappingData::kSystemKeyReset3OperationKeyId)
			&& inputManager->isPressed(agtk::data::InputMappingData::kSystemKeyReset4OperationKeyId);
		if (resetPressed) {
			resetTriggered = inputManager->isTriggered(agtk::data::InputMappingData::kSystemKeyReset1OperationKeyId)
				|| inputManager->isTriggered(agtk::data::InputMappingData::kSystemKeyReset2OperationKeyId)
				|| inputManager->isTriggered(agtk::data::InputMappingData::kSystemKeyReset3OperationKeyId)
				|| inputManager->isTriggered(agtk::data::InputMappingData::kSystemKeyReset4OperationKeyId);
		}
		// ACT2-6206 共通スイッチリセット対応
		auto scene = this->getCurrentScene();
		if (nullptr != scene && scene->getRequestSwitchReset()) {
			resetTriggered = true;
		}
#ifdef USE_PREVIEW
		// プロジェクトを再スタートさせてリプレイ記録する。記録終了する場合はF10で行う。
		if (inputManager->isPressedKeyboard((int)EventKeyboard::KeyCode::KEY_SHIFT) &&
			resetTriggered && !this->isSceneChanging()) {	// SHIFT + F5
			// restartCanvas()を済ませてから記録を開始
			this->restartCanvas();
			auto gameScene = dynamic_cast<GameScene *>(this->getCurrentLayer());
			if (gameScene) {
				// restartCanvas()中はgameSceneのupdateを行わない
				gameScene->setIsRestartCanvas(true);
			}

			getCurrentLayer()->runAction(agtk::Sequence::create(
				DelayTime::create(0.01f),
				agtk::IfCallFunc::create([this]() {
					auto inputManager = InputManager::getInstance();
					auto gameScene = dynamic_cast<GameScene *>(this->getCurrentLayer());
					if (gameScene) {
						// restartCanvas()が終わったのでgameSceneのupdateを再開
						gameScene->setIsRestartCanvas(false);
					}

					// すでに記録されていたら一度停止させてから記録
					if (inputManager->isRecording()) {
						inputManager->stopRecording();
					}
					_autoTestRestartFlg = true;
					inputManager->startRecording();
					return true;
				}),
			nullptr));

			break;
		}
#endif
		//プロジェクトを再スタートする。
		//キーボード用
		if (resetTriggered && !this->isSceneChanging()) {
			this->restartProjectData();
			break;
		}
// #AGTK-NX
#if CC_TARGET_PLATFORM == CC_PLATFORM_NX
#endif
#ifdef USE_PREVIEW
		if (inputManager->isTriggeredKeyboard((int)EventKeyboard::KeyCode::KEY_F10) && !this->isSceneChanging()) {
			if (inputManager->isRecording()) {
				inputManager->stopRecording();
			}
			else {
				inputManager->startRecording();
			}
			break;
		}
#endif
#if CC_ENABLE_PROFILERS
		if (inputManager->isTriggeredKeyboard((int)EventKeyboard::KeyCode::KEY_F12)) {
			if (CC_PROFILER_IS_PROFILING_STARTED()) {
				CC_PROFILER_STOP_PROFILING();
			}
			else {
				CC_PROFILER_START_PROFILING();
			}
		}
#endif

// #AGTK-NX
#if (CC_TARGET_PLATFORM == CC_PLATFORM_NX)
#endif
#ifdef USE_MULTITHREAD_OBJECT_UPDATE
#ifdef USE_MULTITHREAD_UPDOWN_THREADS
		if (getProjectData()->getMultithreading()) {
			if (inputManager->isTriggeredKeyboard((int)EventKeyboard::KeyCode::KEY_PLUS)) {
				ThreadManager::changeUsedThreadCount(1);
			}
			if (inputManager->isTriggeredKeyboard((int)EventKeyboard::KeyCode::KEY_MINUS)) {
				ThreadManager::changeUsedThreadCount(-1);
			}
		}
#endif
#endif
// #AGTK-NX
#if (CC_TARGET_PLATFORM == CC_PLATFORM_NX)
#endif
		break; }
	case kSceneStateStart: {
		this->startProjectData(_sceneStateInfo.sceneId);
		_sceneStateInfo.state = kSceneStateIdle;
		break; }
	case kSceneStateRestart: {
		if (this->isSceneChanging()) {
			break;
		}
		this->restartProjectData();
		_sceneStateInfo.state = kSceneStateIdle;
		break; }
	case kSceneStateStop: {
		this->stopProjectData();
		_sceneStateInfo.state = kSceneStateIdle;
		break; }
	case kSceneStateLoadAndStart: {
		this->loadAndStartProjectData(_sceneStateInfo.projectFilePath, _sceneStateInfo.sceneId);
		_sceneStateInfo.state = kSceneStateIdle;
		break; }
#ifdef USE_PREVIEW
	case kSceneStateLoad: {
		this->stopProjectData();
		this->loadProjectData(_sceneStateInfo.projectFilePath);
		auto projectData = GameManager::getInstance()->getProjectData();
		InputManager::getInstance()->setInputMappingData(projectData->getInputMapping());	//遷移のリンクチェックで入力チェックをするため、それより先に設定が必要なため、とりあえず。
		_sceneStateInfo.state = kSceneStateIdle;
		break; }
#endif
	}
}

void GameManager::preloadSceneData(agtk::data::SceneData *scene)
{
	auto projectData = GameManager::getInstance()->getProjectData();
	auto audioManager = AudioManager::getInstance()->getInstance();

	//SE ----------------------------------------------------------------------
	//tile
	auto layerList = scene->getLayerList();
	cocos2d::DictElement *el = nullptr;
	CCDICT_FOREACH(layerList, el) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
		auto layerData = static_cast<agtk::data::LayerData *>(el->getObject());
#else
		auto layerData = dynamic_cast<agtk::data::LayerData *>(el->getObject());
#endif
		cocos2d::Ref *ref = nullptr;
		CCARRAY_FOREACH(layerData->getTileList(), ref) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
			auto tile = static_cast<agtk::data::Tile *>(ref);
#else
			auto tile = dynamic_cast<agtk::data::Tile *>(ref);
#endif
			auto tileset = projectData->getTilesetData(tile->getTilesetId());
			auto tileData = tileset->getTileData(tile->getX() + tile->getY() * tileset->getHorzTileCount());
			if (tileData && tileData->getPlaySe() && tileData->getPlaySeId() > 0) {
				audioManager->preloadSe(tileData->getPlaySeId());
			}
		}
	}
	//object
	auto scenePartObjectList = scene->getScenePartObjectList();
	el = nullptr;
	CCDICT_FOREACH(scenePartObjectList, el) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
		auto scenePartData = static_cast<agtk::data::ScenePartObjectData *>(el->getObject());
#else
		auto scenePartData = dynamic_cast<agtk::data::ScenePartObjectData *>(el->getObject());
#endif
		auto objectData = projectData->getObjectData(scenePartData->getObjectId());
		if (objectData == nullptr) {
			continue;
		}
		auto animationData = projectData->getAnimationData(objectData->getAnimationId());

		// アニメーションデータがある場合
		if (nullptr != animationData) {
			//animation
			auto motionList = animationData->getMotionList();
			cocos2d::DictElement *el2 = nullptr;
			CCDICT_FOREACH(motionList, el2) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
				auto motionData = static_cast<agtk::data::MotionData *>(el2->getObject());
#else
				auto motionData = dynamic_cast<agtk::data::MotionData *>(el2->getObject());
#endif
				auto directionList = motionData->getDirectionList();
				cocos2d::DictElement *el3 = nullptr;
				CCDICT_FOREACH(directionList, el3) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
					auto directionData = static_cast<agtk::data::DirectionData *>(el3->getObject());
#else
					auto directionData = dynamic_cast<agtk::data::DirectionData *>(el3->getObject());
#endif
					auto frameList = directionData->getFrameList();
					cocos2d::DictElement *el4 = nullptr;
					CCDICT_FOREACH(frameList, el4) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
						auto frameData = static_cast<agtk::data::FrameData *>(el4->getObject());
#else
						auto frameData = dynamic_cast<agtk::data::FrameData *>(el4->getObject());
#endif
						if (frameData->getPlaySe() && frameData->getPlaySeId() > 0) {
							audioManager->preloadSe(frameData->getPlaySeId());
						}
					}
				}
			}
		}

		auto objectActionList = objectData->getActionList();
		cocos2d::DictElement *el2 = nullptr;
		CCDICT_FOREACH(objectActionList, el2) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
			auto objectActionData = static_cast<agtk::data::ObjectActionData *>(el2->getObject());
#else
			auto objectActionData = dynamic_cast<agtk::data::ObjectActionData *>(el2->getObject());
#endif
			auto objCommandList = objectActionData->getObjCommandList();
			cocos2d::DictElement *el3 = nullptr;
			CCDICT_FOREACH(objCommandList, el3) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
				auto objCommandData = static_cast<agtk::data::ObjectCommandData *>(el3->getObject());
#else
				auto objCommandData = dynamic_cast<agtk::data::ObjectCommandData *>(el3->getObject());
#endif
				if (objCommandData->getIgnored()) {
					continue;
				}
				if (objCommandData->getCommandType() == agtk::data::ObjectCommandData::EnumObjCommandType::kSoundPlay) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
					auto cmd = static_cast<agtk::data::ObjectCommandSoundPlayData *>(objCommandData);
#else
					auto cmd = dynamic_cast<agtk::data::ObjectCommandSoundPlayData *>(objCommandData);
#endif
					CC_ASSERT(cmd);
					if (cmd->getSoundType() == agtk::data::ObjectCommandSoundPlayData::kSoundSe) {
						if (cmd->getSeId() > 0) {
							audioManager->preloadSe(cmd->getSeId());
						}
					}
				}
			}
		}
	}
}

//リソース（画像、サウンド）をロードする。
void GameManager::loadResources(agtk::data::SceneData *scene)
{
	if (_stateLoadResources == EnumStateLoadResources::LOAD) {
		return;
	}

	auto projectData = GameManager::getInstance()->getProjectData();
	auto textureCache = Director::getInstance()->getTextureCache();

// #AGTK-NX #AGTK-WIN
#ifdef USE_CLEAR_UNUSED_TEX
	_markedFilenameList.clear();

	std::function<void(std::string)> addFilename = [&](std::string filename) {
		auto itr = _markedFilenameList.find(filename);
		if (itr != _markedFilenameList.end()) {
			itr->second += 1;
			return;
		}
		_markedFilenameList.insert(std::make_pair(filename, 0));
	};
#endif

	std::function<void(std::string)> addImageAsync = [&](std::string filename) {
		if (FileUtils::getInstance()->isFileExist(filename) == false) {
			//ファイルチェック。
			CC_ASSERT(0);
			return;
		}
		//GIFファイルは先読み対象外。
		if (strstr(filename.c_str(), ".GIF") || strstr(filename.c_str(), ".gif")) {
			return;
		}
		_endLoadResources.insert(std::make_pair(filename, 0));
		textureCache->addImageAsync(filename, [&](cocos2d::Texture2D *texture) {
			auto value = &_endLoadResources.at(texture->getPath());
			*value = 1;
// #AGTK-NX #AGTK-WIN
#ifdef USE_CLEAR_UNUSED_TEX
			texture->setVolatileCache(true);
#endif
		});
// #AGTK-NX #AGTK-WIN
#ifdef USE_CLEAR_UNUSED_TEX
		addFilename(filename);
#endif
	};
	std::function<bool(std::string)> checkImage = [&](std::string filename) {
		auto itr = _endLoadResources.find(filename);
		if (itr != _endLoadResources.end()) {
			return true;
		}
		return false;
	};

	//menu tile
	auto menuLayerIdList = scene->getPreloadMenuLayerIdList();
	auto menuSceneData = projectData->getMenuSceneData();
	cocos2d::Ref *ref;
	CCARRAY_FOREACH(menuLayerIdList, ref) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
		auto id = static_cast<cocos2d::__Integer *>(ref);
#else
		auto id = dynamic_cast<cocos2d::__Integer *>(ref);
#endif
		auto menuLayerData = menuSceneData->getLayer(id->getValue());
		if (menuLayerData == nullptr) continue;
		auto tileList = menuLayerData->getTileList();
		cocos2d::Ref *ref = nullptr;
		CCARRAY_FOREACH(tileList, ref) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
			auto tile = static_cast<agtk::data::Tile *>(ref);
#else
			auto tile = dynamic_cast<agtk::data::Tile *>(ref);
#endif
			auto tileset = projectData->getTilesetData(tile->getTilesetId());
			auto imageData = projectData->getImageData(tileset->getImageId());
			auto filename = std::string(imageData->getFilename());
			if (checkImage(filename) == false) {
				addImageAsync(filename);
			}
		}
	}

	//tile
	auto layerList = scene->getLayerList();
	cocos2d::DictElement *el = nullptr;
	CCDICT_FOREACH(layerList, el) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
		auto layerData = static_cast<agtk::data::LayerData *>(el->getObject());
#else
		auto layerData = dynamic_cast<agtk::data::LayerData *>(el->getObject());
#endif
		auto tileList = layerData->getTileList();
		cocos2d::Ref *ref = nullptr;
		CCARRAY_FOREACH(tileList, ref) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
			auto tile = static_cast<agtk::data::Tile *>(ref);
#else
			auto tile = dynamic_cast<agtk::data::Tile *>(ref);
#endif
			auto tileset = projectData->getTilesetData(tile->getTilesetId());
			auto imageData = projectData->getImageData(tileset->getImageId());
			auto filename = std::string(imageData->getFilename());
			if (checkImage(filename) == false) {
				addImageAsync(filename);
			}
		}
	}

	std::function<bool(std::vector<int>&, int)> vector_finder = [&](std::vector<int>& vec, int num) {
		auto itr = std::find(vec.begin(), vec.end(), num);
		auto index = std::distance(vec.begin(), itr);
		return index != vec.size();
	};

	std::function<void(agtk::data::ObjectData *, std::vector<int>&)> loadResourceObject = [&](agtk::data::ObjectData *objectData, std::vector<int>& parentIds) {
		if (objectData == nullptr) {
			return;
		}
		parentIds.push_back(objectData->getId());
		auto animationData = projectData->getAnimationData(objectData->getAnimationId());

		// アニメーションデータがある場合
		if (nullptr != animationData) {
			//animation
			auto motionList = animationData->getMotionList();
			cocos2d::DictElement *el = nullptr;
			auto imageDataList = projectData->getImageList();
			CCDICT_FOREACH(motionList, el) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
				auto motionData = static_cast<agtk::data::MotionData *>(el->getObject());
#else
				auto motionData = dynamic_cast<agtk::data::MotionData *>(el->getObject());
#endif
				auto directionList = motionData->getDirectionList();
				cocos2d::DictElement *el2 = nullptr;
				CCDICT_FOREACH(directionList, el2) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
					auto directionData = static_cast<agtk::data::DirectionData *>(el2->getObject());
#else
					auto directionData = dynamic_cast<agtk::data::DirectionData *>(el2->getObject());
#endif
					auto resourceInfoData = animationData->getResourceInfoData(directionData->getResourceInfoId());
					if (resourceInfoData == nullptr || resourceInfoData->getImage() == false) continue;
					for (auto imageId : resourceInfoData->getImageIdList()) {
						auto imageData = projectData->getImageData(imageId);
						if (imageData) {
							auto filename = std::string(imageData->getFilename());
							if (checkImage(filename) == false) {
								addImageAsync(filename);
							}
						}
					}
				}
			}
		}

		auto objectActionList = objectData->getActionList();
		cocos2d::DictElement *el2 = nullptr;
		CCDICT_FOREACH(objectActionList, el2) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
			auto objectActionData = static_cast<agtk::data::ObjectActionData *>(el2->getObject());
#else
			auto objectActionData = dynamic_cast<agtk::data::ObjectActionData *>(el2->getObject());
#endif
			auto objCommandList = objectActionData->getObjCommandList();
			cocos2d::DictElement *el3 = nullptr;
			CCDICT_FOREACH(objCommandList, el3) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
				auto objCommandData = static_cast<agtk::data::ObjectCommandData *>(el3->getObject());
#else
				auto objCommandData = dynamic_cast<agtk::data::ObjectCommandData *>(el3->getObject());
#endif
				if (objCommandData->getIgnored()) {
					continue;
				}
				if (objCommandData->getCommandType() == agtk::data::ObjectCommandData::kObjectCreate) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
					auto cmd = static_cast<agtk::data::ObjectCommandObjectCreateData *>(objCommandData);
#else
					auto cmd = dynamic_cast<agtk::data::ObjectCommandObjectCreateData *>(objCommandData);
#endif
					if (cmd->getObjectId() == objectData->getId()) {
						continue;
					}
					if(vector_finder(parentIds, cmd->getObjectId())) {
						continue;
					}
					loadResourceObject(projectData->getObjectData(cmd->getObjectId()), parentIds);
				}
				else if (objCommandData->getCommandType() == agtk::data::ObjectCommandData::kObjectChange) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
					auto cmd = static_cast<agtk::data::ObjectCommandObjectChangeData *>(objCommandData);
#else
					auto cmd = dynamic_cast<agtk::data::ObjectCommandObjectChangeData *>(objCommandData);
#endif
					if (cmd->getObjectId() == objectData->getId()) {
						continue;
					}
					if (vector_finder(parentIds, cmd->getObjectId())) {
						continue;
					}
					loadResourceObject(projectData->getObjectData(cmd->getObjectId()), parentIds);
				}
				else if (objCommandData->getCommandType() == agtk::data::ObjectCommandData::kBulletFire) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
					auto cmd = static_cast<agtk::data::ObjectCommandBulletFireData *>(objCommandData);
#else
					auto cmd = dynamic_cast<agtk::data::ObjectCommandBulletFireData *>(objCommandData);
#endif
					auto fireBulletSettingData = objectData->getFireBulletSettingData(cmd->getBulletId());
					if (fireBulletSettingData == nullptr) {
						continue;
					}
					if (fireBulletSettingData->getBulletObjectId() == objectData->getId()) {
						continue;
					}
					if (vector_finder(parentIds, fireBulletSettingData->getBulletObjectId())) {
						continue;
					}
					if (fireBulletSettingData) {
						loadResourceObject(projectData->getObjectData(fireBulletSettingData->getBulletObjectId()), parentIds);
					}
				}
			}
		}
	};

	std::vector<int> parentIds;
	//object
	auto scenePartObjectList = scene->getScenePartObjectList();
	el = nullptr;
	CCDICT_FOREACH(scenePartObjectList, el) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
		auto scenePartData = static_cast<agtk::data::ScenePartObjectData *>(el->getObject());
#else
		auto scenePartData = dynamic_cast<agtk::data::ScenePartObjectData *>(el->getObject());
#endif
		auto objectData = projectData->getObjectData(scenePartData->getObjectId());
		loadResourceObject(objectData, parentIds);
	}

	//menu object
	CCARRAY_FOREACH(menuLayerIdList, ref) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
		auto id = static_cast<cocos2d::__Integer *>(ref);
#else
		auto id = dynamic_cast<cocos2d::__Integer *>(ref);
#endif
		scenePartObjectList = menuSceneData->getScenePartObjectList();
		CCDICT_FOREACH(scenePartObjectList, el) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
			auto scenePartData = static_cast<agtk::data::ScenePartObjectData *>(el->getObject());
#else
			auto scenePartData = dynamic_cast<agtk::data::ScenePartObjectData *>(el->getObject());
#endif
			if (scenePartData->getVisible() == false) continue;
			if (scenePartData->getLayerIndex() + 1 == id->getValue()) {
				auto objectData = projectData->getObjectData(scenePartData->getObjectId());
				loadResourceObject(objectData, parentIds);
			}
		}
	}

	_stateLoadResources = EnumStateLoadResources::LOAD;
}

void GameManager::loadResources(int sceneId)
{
	auto projectData = this->getProjectData();
	auto sceneData = projectData->getSceneData(sceneId);
	this->loadResources(sceneData);
}

// #AGTK-NX #AGTK-WIN
#ifdef USE_CLEAR_UNUSED_TEX
//シーンで使用するテクスチャキャッシュの参照カウントインクリメント
void GameManager::markLoadResources()
{
	auto textureCache = Director::getInstance()->getTextureCache();
	for (auto itr = _markedFilenameList.begin(); itr != _markedFilenameList.end(); ++itr) {
		auto texture = textureCache->getTextureForKey(itr->first);
		if (texture) {
			texture->retain();
		}
	}
}

//シーンで使用するテクスチャキャッシュの参照カウントデクリメント
void GameManager::unmarkLoadResources()
{
	auto textureCache = Director::getInstance()->getTextureCache();
	for (auto itr = _markedFilenameList.begin(); itr != _markedFilenameList.end(); ++itr) {
		auto texture = textureCache->getTextureForKey(itr->first);
		if (texture) {
			texture->release();
		}
	}
}
#endif

void GameManager::clearLoadResources(agtk::data::SceneData *scene)
{
	if (_stateLoadResources == EnumStateLoadResources::LOAD) {
		return;
	}
	auto projectData = GameManager::getInstance()->getProjectData();
	auto textureCache = Director::getInstance()->getTextureCache();

	std::map<std::string, int> filenameList;

	std::function<void(std::string)> addFilename = [&](std::string filename) {
		auto itr = filenameList.find(filename);
		if (itr != filenameList.end()) {
			itr->second += 1;
			return;
		}
		filenameList.insert(std::make_pair(filename, 0));
	};

	//tile
	auto layerList = scene->getLayerList();
	cocos2d::DictElement *el = nullptr;
	CCDICT_FOREACH(layerList, el) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
		auto layerData = static_cast<agtk::data::LayerData *>(el->getObject());
#else
		auto layerData = dynamic_cast<agtk::data::LayerData *>(el->getObject());
#endif
		auto tileList = layerData->getTileList();
		cocos2d::Ref *ref = nullptr;
		CCARRAY_FOREACH(tileList, ref) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
			auto tile = static_cast<agtk::data::Tile *>(ref);
#else
			auto tile = dynamic_cast<agtk::data::Tile *>(ref);
#endif
			auto tileset = projectData->getTilesetData(tile->getTilesetId());
			auto imageData = projectData->getImageData(tileset->getImageId());
			auto filename = std::string(imageData->getFilename());
			addFilename(filename);
		}
	}

	std::function<void(agtk::data::ObjectData *)> loadResourceObject = [&](agtk::data::ObjectData *objectData) {
		if (objectData == nullptr) {
			return;
		}
		auto animationData = projectData->getAnimationData(objectData->getAnimationId());

		// アニメーションデータがある場合
		if (nullptr != animationData) {
			//animation
			auto motionList = animationData->getMotionList();
			cocos2d::DictElement *el2 = nullptr;
			CCDICT_FOREACH(motionList, el2) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
				auto motionData = static_cast<agtk::data::MotionData *>(el2->getObject());
#else
				auto motionData = dynamic_cast<agtk::data::MotionData *>(el2->getObject());
#endif
				auto directionList = motionData->getDirectionList();
				cocos2d::DictElement *el3 = nullptr;
				CCDICT_FOREACH(directionList, el3) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
					auto directionData = static_cast<agtk::data::DirectionData *>(el3->getObject());
#else
					auto directionData = dynamic_cast<agtk::data::DirectionData *>(el3->getObject());
#endif
					auto imageData = projectData->getImageData(directionData->getResourceInfoId());
					auto filename = std::string(imageData->getFilename());
					addFilename(filename);
				}
			}
		}

		auto objectActionList = objectData->getActionList();
		cocos2d::DictElement *el2 = nullptr;
		CCDICT_FOREACH(objectActionList, el2) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
			auto objectActionData = static_cast<agtk::data::ObjectActionData *>(el2->getObject());
#else
			auto objectActionData = dynamic_cast<agtk::data::ObjectActionData *>(el2->getObject());
#endif
			auto objCommandList = objectActionData->getObjCommandList();
			cocos2d::DictElement *el3 = nullptr;
			CCDICT_FOREACH(objCommandList, el3) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
				auto objCommandData = static_cast<agtk::data::ObjectCommandData *>(el3->getObject());
#else
				auto objCommandData = dynamic_cast<agtk::data::ObjectCommandData *>(el3->getObject());
#endif
				if (objCommandData->getCommandType() == agtk::data::ObjectCommandData::kObjectCreate) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
					auto cmd = static_cast<agtk::data::ObjectCommandObjectCreateData *>(objCommandData);
#else
					auto cmd = dynamic_cast<agtk::data::ObjectCommandObjectCreateData *>(objCommandData);
#endif
					loadResourceObject(projectData->getObjectData(cmd->getObjectId()));
				}
				else if (objCommandData->getCommandType() == agtk::data::ObjectCommandData::kObjectChange) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
					auto cmd = static_cast<agtk::data::ObjectCommandObjectChangeData *>(objCommandData);
#else
					auto cmd = dynamic_cast<agtk::data::ObjectCommandObjectChangeData *>(objCommandData);
#endif
					loadResourceObject(projectData->getObjectData(cmd->getObjectId()));
				}
				else if (objCommandData->getCommandType() == agtk::data::ObjectCommandData::kBulletFire) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
					auto cmd = static_cast<agtk::data::ObjectCommandBulletFireData *>(objCommandData);
#else
					auto cmd = dynamic_cast<agtk::data::ObjectCommandBulletFireData *>(objCommandData);
#endif
					auto fireBulletSettingData = objectData->getFireBulletSettingData(cmd->getBulletId());
					loadResourceObject(projectData->getObjectData(fireBulletSettingData->getBulletObjectId()));
				}
			}
		}
	};

	//object
	auto scenePartObjectList = scene->getScenePartObjectList();
	el = nullptr;
	CCDICT_FOREACH(scenePartObjectList, el) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
		auto scenePartData = static_cast<agtk::data::ScenePartObjectData *>(el->getObject());
#else
		auto scenePartData = dynamic_cast<agtk::data::ScenePartObjectData *>(el->getObject());
#endif
		auto objectData = projectData->getObjectData(scenePartData->getObjectId());
		loadResourceObject(objectData);
	}

	//キャッシュ破棄
	for (auto itr = filenameList.begin(); itr != filenameList.end(); ++itr) {
		textureCache->removeTextureForKey(itr->first);
	}
}

void GameManager::clearLoadResources(int sceneId)
{
	auto projectData = this->getProjectData();
	auto sceneData = projectData->getSceneData(sceneId);
	this->clearLoadResources(sceneData);
}

void GameManager::clearAllLoadResources()
{
	CC_ASSERT(_stateLoadResources == EnumStateLoadResources::NONE);
	auto textureCache = Director::getInstance()->getTextureCache();
	textureCache->removeAllTextures();
}

//リソース更新
void GameManager::updateLoadResources()
{
	switch (_stateLoadResources) {
	case EnumStateLoadResources::NONE:
		break;
	case EnumStateLoadResources::LOAD: {
		bool bEndFlag = true;
		if (_endLoadResources.size()) {
			for (auto itr = _endLoadResources.begin(); itr != _endLoadResources.end(); ++itr) {
				if (itr->second == 0) {
					bEndFlag = false;
					break;
				}
			}
		}
		if (bEndFlag) {
			_stateLoadResources = EnumStateLoadResources::END;
		}
		break; }
	case EnumStateLoadResources::END: {
		_endLoadResources.clear();
		_stateLoadResources = EnumStateLoadResources::NONE;
		break; }
	}
}

/**
* スイッチ、変数のデータリスト取得
* @param	qualifierId	制限ID (全体 or 単体)
* @param	objectId	オブジェクトID
* @param	dataId		データのID(SwitchId or VariableId)
* @param	isSwitch	True:スイッチデータリスト / False:変数データリスト
* @param	out			出力用配列
*/
void GameManager::getSwitchVariableDataList(int qualifierId, int objectId, int dataId, bool isSwitch, cocos2d::__Array *out)
{
	CCASSERT(out, "out put Array is null.");

	// ワーク変数
	auto projectPlayData = this->getPlayData();

	// プロジェクト共通データの場合
	if (objectId == agtk::data::PlayData::COMMON_PLAY_DATA_ID) {
		if (isSwitch) {
			auto data = projectPlayData->getSwitchData(objectId, dataId);
			CC_ASSERT(data);
			if (data != nullptr) {
				out->addObject(data);
			}
		}
		else {
			auto data = projectPlayData->getVariableData(objectId, dataId);
			CC_ASSERT(data);
			if (data != nullptr) {
				out->addObject(data);
			}
		}
	}
	// オブジェクト固有データの場合
	else if (objectId > agtk::data::PlayData::COMMON_PLAY_DATA_ID) {
		// 単体 or 指定インスタンスの場合
		if (qualifierId == agtk::data::kQualifierSingle || qualifierId > 0) {
			int instanceId = qualifierId;// 指定インスタンスの場合は qualifierId にインスタンスIDが格納されている

			// 単体指定の場合
			if (qualifierId == agtk::data::kQualifierSingle) {
				auto singleInstanceData = projectPlayData->getVariableData(objectId, agtk::data::kObjectSystemVariableSingleInstanceID);
				instanceId = (int)singleInstanceData->getValue();
			}

			// 指定インスタンスのオブジェクトを取得
			auto object = this->getTargetObjectByInstanceId(instanceId);

			// オブジェクトが存在する場合
			if (nullptr != object) {
				auto playObjectData = object->getPlayObjectData();
				if (isSwitch) {
					auto data = playObjectData->getSwitchData(dataId);
					CC_ASSERT(data);
					if (data != nullptr) {
						out->addObject(data);
					}
				}
				else {
					auto data = playObjectData->getVariableData(dataId);
					CC_ASSERT(data);
					if (data != nullptr) {
						out->addObject(data);
					}
				}
			}
		}
		// 全体の場合
		else if (qualifierId == agtk::data::kQualifierWhole) {
			auto objectList = this->getTargetObjectListByObjectId(objectId);
			cocos2d::Ref *ref = nullptr;
			CCARRAY_FOREACH(objectList, ref) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
				auto object = static_cast<agtk::Object *>(ref);
#else
				auto object = dynamic_cast<agtk::Object *>(ref);
#endif
				auto playObjectData = object->getPlayObjectData();

				if (isSwitch) {
					auto data = playObjectData->getSwitchData(dataId);
					CC_ASSERT(data);
					if (data != nullptr) {
						out->addObject(data);
					}
				}
				else {
					auto data = playObjectData->getVariableData(dataId);
					CC_ASSERT(data);
					if (data != nullptr) {
						out->addObject(data);
					}
				}
			}
		}
	}
}

/**
* スイッチの条件チェック
* @param	type	スイッチタイプ
* @param	value	スイッチの値
* @param	state	スイッチの状態
* @return			True:条件を満たした / False:条件を満たしていない
*/
bool GameManager::checkSwitchCondition(agtk::data::SwitchVariableConditionData::EnumSwitchValueType type, bool value, agtk::data::PlaySwitchData::EnumState state)
{
	bool ret = false;

	// スイッチのON/OFFの状態による条件チェック
	switch (type)
	{
		// ----------------------------------------------
		// スイッチがON
		// ----------------------------------------------
	case agtk::data::SwitchVariableConditionData::EnumSwitchValueType::kSwitchConditionOn:
		ret = (value == true);
		break;
		// ----------------------------------------------
		// スイッチがOFF
		// ----------------------------------------------
	case agtk::data::SwitchVariableConditionData::EnumSwitchValueType::kSwitchConditionOff:
		ret = (value == false);
		break;
		// ----------------------------------------------
		// スイッチがOFFからON
		// ----------------------------------------------
	case agtk::data::SwitchVariableConditionData::EnumSwitchValueType::kSwitchConditionOffFromOn:
		ret = (state == agtk::data::PlaySwitchData::kStateOnFromOff);
		break;
		// ----------------------------------------------
		// スイッチがONからOFF
		// ----------------------------------------------
	case agtk::data::SwitchVariableConditionData::EnumSwitchValueType::kSwitchConditionOnFromOff:
		ret = (state == agtk::data::PlaySwitchData::kStateOffFromOn);
		break;
	}

	return ret;
}

/**
* 変数の条件チェック
* @param	type			比較先の値タイプ
* @param	srcValue		比較元の値
* @param	op				比較方法タイプ
* @param	constantValue	比較先の定数値
* @param	qualifierId		比較先の変数の制限ID
* @param	objectId		比較先の変数のオブジェクトID
* @param	variableId		比較先の変数ID
* @return					True:条件を満たした / False:条件を満たしていない
*/
bool GameManager::checkVariableCondition(agtk::data::SwitchVariableConditionData::EnumCompareValueType type, double srcValue, agtk::data::SwitchVariableConditionData::EnumCompareOperator op, double constantValue, int qualifierId, int objectId, int variableId)
{
	bool ret = false;
	double compareValue = NAN;

	// タイプ別に比較先の値を設定
	switch (type) {
		// -----------------------------------------------
		// 定数
		// -----------------------------------------------
	case agtk::data::SwitchVariableConditionData::EnumCompareValueType::kCompareValue:
		compareValue = constantValue;
		break;
		// -----------------------------------------------
		// 変数
		// -----------------------------------------------
	case agtk::data::SwitchVariableConditionData::EnumCompareValueType::kCompareVariable:
	{
		cocos2d::__Array * comparedVarialbeList = cocos2d::__Array::create();
		this->getSwitchVariableDataList(qualifierId, objectId, variableId, false, comparedVarialbeList);
		CCASSERT((comparedVarialbeList->count() > 0), "compared variable is not found.");
		auto comparedVariableData = dynamic_cast<agtk::data::PlayVariableData *>(comparedVarialbeList->getObjectAtIndex(0));
		if (comparedVariableData == nullptr) {
			// 変数がなく比較できないのでfalseで返す
			return false;
		}
		compareValue = comparedVariableData->getValue();
	} break;
	// -----------------------------------------------
	// 非数
	// -----------------------------------------------
	case agtk::data::SwitchVariableConditionData::EnumCompareValueType::kCompareNan:
		compareValue = NAN;
		break;
	}

	bool src_isnan = std::isnan(srcValue);
	bool compare_isnan = std::isnan(compareValue);
	// 比較演算子別処理
	if (src_isnan || compare_isnan) {
		//どちらかが非数なら特別処理
		switch (op) {
			// -----------------------------------------------
			// ＜
			// -----------------------------------------------
		case agtk::data::SwitchVariableConditionData::EnumCompareOperator::kCompareSmaller:
			ret = false;
			break;
			// -----------------------------------------------
			// ＜＝
			// -----------------------------------------------
		case agtk::data::SwitchVariableConditionData::EnumCompareOperator::kCompareSmallerEqual:
			ret = (src_isnan && compare_isnan);
			break;
			// -----------------------------------------------
			// ＝
			// -----------------------------------------------
		case agtk::data::SwitchVariableConditionData::EnumCompareOperator::kCompareEqual:
			ret = (src_isnan && compare_isnan);
			break;
			// -----------------------------------------------
			// ＞＝
			// -----------------------------------------------
		case agtk::data::SwitchVariableConditionData::EnumCompareOperator::kCompareLargerEqual:
			ret = (src_isnan && compare_isnan);
			break;
			// -----------------------------------------------
			// ＞
			// -----------------------------------------------
		case agtk::data::SwitchVariableConditionData::EnumCompareOperator::kCompareLarger:
			ret = false;
			break;
			// -----------------------------------------------
			// ！＝
			// -----------------------------------------------
		case agtk::data::SwitchVariableConditionData::EnumCompareOperator::kCompareNotEqual:
			ret = (src_isnan != compare_isnan);
			break;
		}
	}
	else {
		switch (op) {
			// -----------------------------------------------
			// ＜
			// -----------------------------------------------
		case agtk::data::SwitchVariableConditionData::EnumCompareOperator::kCompareSmaller:
			ret = (srcValue < compareValue);
			break;
			// -----------------------------------------------
			// ＜＝
			// -----------------------------------------------
		case agtk::data::SwitchVariableConditionData::EnumCompareOperator::kCompareSmallerEqual:
			ret = (srcValue <= compareValue);
			break;
			// -----------------------------------------------
			// ＝
			// -----------------------------------------------
		case agtk::data::SwitchVariableConditionData::EnumCompareOperator::kCompareEqual:
			ret = (srcValue == compareValue);
			break;
			// -----------------------------------------------
			// ＞＝
			// -----------------------------------------------
		case agtk::data::SwitchVariableConditionData::EnumCompareOperator::kCompareLargerEqual:
			ret = (srcValue >= compareValue);
			break;
			// -----------------------------------------------
			// ＞
			// -----------------------------------------------
		case agtk::data::SwitchVariableConditionData::EnumCompareOperator::kCompareLarger:
			ret = (srcValue > compareValue);
			break;
			// -----------------------------------------------
			// ！＝
			// -----------------------------------------------
		case agtk::data::SwitchVariableConditionData::EnumCompareOperator::kCompareNotEqual:
			ret = (srcValue != compareValue);
			break;
		}
	}

	return ret;
}

/**
* スイッチ、変数の変更処理
* @param	switchVariableAssginList	変更内容リスト
*/
void GameManager::calcSwichVariableChange(cocos2d::__Array *switchVariableAssginList, EnumPlaceType placeType, int id1, int id2, int id3)
{
	auto projectPlayData = this->getPlayData();

	cocos2d::Ref *ref = nullptr;
	int index = -1;
	CCARRAY_FOREACH(switchVariableAssginList, ref) {
		index++;
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
		auto data = static_cast<agtk::data::SwitchVariableAssignData *>(ref);
#else
		auto data = dynamic_cast<agtk::data::SwitchVariableAssignData *>(ref);
#endif

		// スイッチを変更する場合
		if (data->getSwtch()) {
			auto objectId = data->getSwitchObjectId();
			auto switchId = data->getSwitchId();
			auto switchQualifierId = data->getSwitchQualifierId();

			// プロジェクト共通の場合
			if (objectId == agtk::data::PlayData::COMMON_PLAY_DATA_ID) {
				auto switchData = projectPlayData->getSwitchData(agtk::data::PlayData::COMMON_PLAY_DATA_ID, switchId);
				this->assignSwitchData(data->getSwitchValue(), switchData);
			}
			// オブジェクト固有の場合
			else if (objectId > agtk::data::PlayData::COMMON_PLAY_DATA_ID) {
				// 指定オブジェクトIDのオブジェクトリストを取得する
				auto objectList = this->getTargetObjectListByObjectId(objectId);

				// 単体 or 指定インスタンスの場合
				if (switchQualifierId == agtk::data::EnumQualifierType::kQualifierSingle || switchQualifierId > 0) {
					int instanceId = switchQualifierId;// 指定インスタンスの場合は switchQualifierId にインスタンスIDが入っている

					// 単体の場合
					if (switchQualifierId == agtk::data::EnumQualifierType::kQualifierSingle) {
						auto p = projectPlayData->getVariableData(objectId, agtk::data::kObjectSystemVariableSingleInstanceID);
						if (p) {
							instanceId = (int)p->getValue();
						}
					}

					cocos2d::Ref *ref = nullptr;
					CCARRAY_FOREACH(objectList, ref) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
						auto object = static_cast<agtk::Object *>(ref);
#else
						auto object = dynamic_cast<agtk::Object *>(ref);
#endif
						if (object->getInstanceId() == instanceId) {
							auto switchData = object->getPlayObjectData()->getSwitchData(switchId);
							this->assignSwitchData(data->getSwitchValue(), switchData);
							break;
						}
					}
				}
				// 全体の場合
				else if (switchQualifierId == agtk::data::EnumQualifierType::kQualifierWhole) {
					cocos2d::Ref *ref = nullptr;
					CCARRAY_FOREACH(objectList, ref) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
						auto object = static_cast<agtk::Object *>(ref);
#else
						auto object = dynamic_cast<agtk::Object *>(ref);
#endif
						auto switchData = object->getPlayObjectData()->getSwitchData(switchId);
						this->assignSwitchData(data->getSwitchValue(), switchData);
					}
				}
			}
		}
		// 変数を変更する場合
		else {
			// 変更値を取得
			double value = std::nan("1");
			switch (data->getVariableAssignValueType())
			{
				// ----------------------------------------
				// 定数
				// ----------------------------------------
			case agtk::data::SwitchVariableAssignData::kVariableAssignConstant:
				value = data->getAssignValue();
				break;
				// ----------------------------------------
				// 変数
				// ----------------------------------------
			case agtk::data::SwitchVariableAssignData::kVariableAssignVariable:
			{
				auto assignObjectId = data->getAssignVariableObjectId();
				auto assignVariableId = data->getAssignVariableId();
				auto assignQualifierId = data->getAssignVariableQualifierId();

				// プロジェクト共通の場合
				if (assignObjectId == agtk::data::PlayData::COMMON_PLAY_DATA_ID) {
					auto assignVariableData = projectPlayData->getVariableData(agtk::data::PlayData::COMMON_PLAY_DATA_ID, assignVariableId);
					if (assignVariableData == nullptr) {
						// 変数データがなく変更はできないので終了させる
						return;
					}
					value = assignVariableData->getValue();
				}
				// オブジェクト固有の場合
				else if (assignObjectId > agtk::data::PlayData::COMMON_PLAY_DATA_ID) {
					// 指定オブジェクトIDのオブジェクトリストを取得する
					auto objectList = this->getTargetObjectListByObjectId(assignObjectId);
					CCASSERT(assignQualifierId != agtk::data::EnumQualifierType::kQualifierWhole, "Invalid assginQualifierdId.");

					// 単体 or 指定インスタンスの場合
					if (assignQualifierId == agtk::data::EnumQualifierType::kQualifierSingle || assignQualifierId > 0) {
						int instanceId = assignQualifierId;// 指定インスタンスの場合は assignQualifierId にインスタンスIDが入っている

						 // 単体の場合
						if (assignQualifierId == agtk::data::EnumQualifierType::kQualifierSingle) {
							auto p = projectPlayData->getVariableData(assignObjectId, agtk::data::kObjectSystemVariableSingleInstanceID);
							if (p) {
								instanceId = (int)p->getValue();
							}
						}

						bool findValue = false;
						cocos2d::Ref *ref = nullptr;
						CCARRAY_FOREACH(objectList, ref) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
							auto object = static_cast<agtk::Object *>(ref);
#else
							auto object = dynamic_cast<agtk::Object *>(ref);
#endif
							if (object->getInstanceId() == instanceId) {
								auto assignVariableData = object->getPlayObjectData()->getVariableData(assignVariableId);
								if (assignVariableData == nullptr) {
									continue;
								}
								value = object->getPlayObjectData()->getVariableData(assignVariableId)->getValue();
								findValue = true;
								break;
							}
						}
						if (!findValue) {
							// 変数データがなく変更はできないので終了させる
							return;
						}
					}
				}
			} break;
			// ----------------------------------------
			// 乱数
			// ----------------------------------------
			case agtk::data::SwitchVariableAssignData::kVariableAssignRandom:
				value = (data->getRandomMin() + rand() % (data->getRandomMax() - data->getRandomMin() + 1));
				break;
				// ----------------------------------------
				// スクリプト
				// ----------------------------------------
			case agtk::data::SwitchVariableAssignData::kVariableAssignScript:
				if (strlen(data->getAssignScript()) > 0) {
#ifdef USE_SCRIPT_PRECOMPILE
					//JSB_AUTOCOMPARTMENT_WITH_GLOBAL_OBJCET
					auto sc = ScriptingCore::getInstance();
					auto cx = sc->getGlobalContext();
					auto _global = sc->getGlobalObject();
					JS::RootedObject gobj(cx, _global);
					JSAutoCompartment ac(cx, gobj);

					JS::RootedObject ns(cx);
					JS::MutableHandleObject jsObj = &ns;
					JS::RootedValue nsval(cx);
					JS_GetProperty(cx, gobj, "Agtk", &nsval);
					JS::RootedValue rv(cx);
					bool ret = false;
					if (nsval != JSVAL_VOID) {
						jsObj.set(nsval.toObjectOrNull());
						JS::RootedValue v(cx);
						JS_GetProperty(cx, jsObj, "scriptFunctions", &v);
						if (v.isObject()) {
							JS::RootedObject rscriptFunctions(cx, &v.toObject());
							const char *categoryStr = nullptr;
							int count = 3;
							if (placeType == kPlaceCourse) {
								categoryStr = "execSceneCourseSwitchVariableChange";
								count = 4;
							} else if (placeType == kPlacePortal) {
								if (id3 == 0) {
									categoryStr = "execPortalPreMoveSwitchVariableChange";
								}
								else {
									categoryStr = "execPortalPostMoveSwitchVariableChange";
								}
							} else if (placeType == kPlaceTransitionLink) {
								categoryStr = "execTransitionLinkPostMoveSwitchVariableChange";
								count = 2;
							}
							JS_GetProperty(cx, rscriptFunctions, categoryStr, &v);
							if (v.isObject()) {
								JS::RootedValue rexec(cx, v);
								jsval args[4];
								args[0] = JS::Int32Value(id1);
								if (count == 2) {
									args[1] = JS::Int32Value(index);
								} else
								if (count == 3) {
									args[1] = JS::Int32Value(id2);
									args[2] = JS::Int32Value(index);
								}
								else {
									args[1] = JS::Int32Value(id2);
									args[2] = JS::Int32Value(id3);
									args[3] = JS::Int32Value(index);
								}
								ret = JS_CallFunctionValue(cx, rscriptFunctions, rexec, JS::HandleValueArray::fromMarkedLocation(count, args), &rv);
							}
						}
					}
#else
					auto scriptingCore = ScriptingCore::getInstance();
					auto context = scriptingCore->getGlobalContext();
					JS::RootedValue rv(context);
					JS::MutableHandleValue mhv(&rv);
					//todo course, portal等の情報を埋め込むようにする。
					auto script = String::createWithFormat("(function(){ return (%s\n); })()", data->getAssignScript());
					auto ret = ScriptingCore::getInstance()->evalString(script->getCString(), mhv);
#endif
					if (!ret) {
						//スクリプトエラー
						auto errorStr = String::createWithFormat("Runtime error in switchVariableChange(script: %s).", data->getAssignScript())->getCString();
						agtk::DebugManager::getInstance()->getDebugExecuteLogWindow()->addLog(errorStr);
#if defined(USE_PREVIEW)
						auto fp = GameManager::getScriptLogFp();
						if (fp) {
							fwrite(errorStr, 1, strlen(errorStr), fp);
							fwrite("\n", 1, 1, fp);
						}
#endif
						value = std::nan("1");
					}
					if (rv.isDouble()) {
						value = rv.toDouble();
					}
					else if (rv.isInt32()) {
						value = rv.toInt32();
					}
					else {
						//数値でない
						value = std::nan("1");
					}
				}
			}

			auto objectId = data->getVariableObjectId();
			auto variableId = data->getVariableId();
			auto variableQualifierId = data->getVariableQualifierId();

			// プロジェクト共通の場合
			if (objectId == agtk::data::PlayData::COMMON_PLAY_DATA_ID) {
				auto variableData = projectPlayData->getVariableData(agtk::data::PlayData::COMMON_PLAY_DATA_ID, variableId);
				this->assignVariableData(data->getVariableAssignOperator(), variableData, value);
			}
			// オブジェクト固有の場合
			else if (objectId > agtk::data::PlayData::COMMON_PLAY_DATA_ID) {
				// 指定オブジェクトIDのオブジェクトリストを取得する
				auto objectList = this->getTargetObjectListByObjectId(objectId);

				// 単体 or 指定インスタンスの場合
				if (variableQualifierId == agtk::data::EnumQualifierType::kQualifierSingle || variableQualifierId > 0) {
					int instanceId = variableQualifierId;// 指定インスタンスの場合は assignQualifierId にインスタンスIDが入っている

														 // 単体の場合
					if (variableQualifierId == agtk::data::EnumQualifierType::kQualifierSingle) {
						auto p = projectPlayData->getVariableData(objectId, agtk::data::kObjectSystemVariableSingleInstanceID);
						if (p) {
							instanceId = (int)p->getValue();
						}
					}

					cocos2d::Ref *ref = nullptr;
					CCARRAY_FOREACH(objectList, ref) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
						auto object = static_cast<agtk::Object *>(ref);
#else
						auto object = dynamic_cast<agtk::Object *>(ref);
#endif
						if (object->getInstanceId() == instanceId) {
							auto variableData = object->getPlayObjectData()->getVariableData(variableId);
							this->assignVariableData(data->getVariableAssignOperator(), variableData, value);
							object->getPlayObjectData()->adjustData();
							break;
						}
					}
				}
				// 全体の場合
				else if (variableQualifierId == agtk::data::EnumQualifierType::kQualifierWhole) {
					cocos2d::Ref *ref = nullptr;
					CCARRAY_FOREACH(objectList, ref) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
						auto object = static_cast<agtk::Object *>(ref);
#else
						auto object = dynamic_cast<agtk::Object *>(ref);
#endif
						auto variableData = object->getPlayObjectData()->getVariableData(variableId);
						this->assignVariableData(data->getVariableAssignOperator(), variableData, value);
					}
				}
			}
		}
	}
}

/**
 * @brief スイッチの値を取得する。
 * @ret 0 = false, 1 = true, 2 = オブジェクトが見つからなかった／スイッチが見つからなかった。
 */
int GameManager::getSwitchValue(int switchObjectId, int switchQualifierId, int switchId)
{
	auto projectPlayData = this->getPlayData();
	// プロジェクト共通の場合
	if (switchObjectId == agtk::data::PlayData::COMMON_PLAY_DATA_ID) {
		auto switchData = projectPlayData->getSwitchData(agtk::data::PlayData::COMMON_PLAY_DATA_ID, switchId);
		if (switchData) {
			return switchData->getValue() ? 1 : 0;
		}
	}
	// オブジェクト固有の場合
	else if (switchObjectId > agtk::data::PlayData::COMMON_PLAY_DATA_ID) {
		// 指定オブジェクトIDのオブジェクトリストを取得する
		auto objectList = this->getTargetObjectListByObjectId(switchObjectId);

		// 単体 or 指定インスタンス
		int instanceId = switchQualifierId;// 指定インスタンスの場合は switchQualifierId にインスタンスIDが入っている

										   // 単体の場合
		if (switchQualifierId == agtk::data::EnumQualifierType::kQualifierSingle) {
			auto p = projectPlayData->getVariableData(switchObjectId, agtk::data::kObjectSystemVariableSingleInstanceID);
			if (p) {
				instanceId = (int)p->getValue();
			}
		}

		cocos2d::Ref *ref = nullptr;
		CCARRAY_FOREACH(objectList, ref) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
			auto object = static_cast<agtk::Object *>(ref);
#else
			auto object = dynamic_cast<agtk::Object *>(ref);
#endif
			if (object->getInstanceId() == instanceId) {
				auto switchData = object->getPlayObjectData()->getSwitchData(switchId);
				if (switchData) {
					return switchData->getValue() ? 1 : 0;
				}
			}
		}
	}
	return 2;
}

/**
* @brief 変数の値を取得する。
* @param variableObjectId    オブジェクト共通・オブジェクト固有・自身のオブジェクト・ロックしたオブジェクト・親オブジェクト
* @param variableQualifierId 単体・全体
* @param variableId          変数ID
* @param retValue            変数の取得値
* @param _object             自身のオブジェクト、ロックしたオブジェクト、親オブジェクトの変数取得に必要
* @return                    True:変数が見つかった / False:変数が見つからなかった
*/
bool GameManager::getVariableValue(int variableObjectId, int variableQualifierId, int variableId, double &retValue, agtk::Object* _object)
{
	bool ret = false;
	auto projectPlayData = this->getPlayData();

	// プロジェクト共通の場合
	if (variableObjectId == agtk::data::PlayData::COMMON_PLAY_DATA_ID) {
		auto variableData = projectPlayData->getVariableData(agtk::data::PlayData::COMMON_PLAY_DATA_ID, variableId);
		if (variableData) {
			retValue = variableData->getValue();
			projectPlayData->adjustCommonVariableData(variableData);
			ret = true;
		}
	}
	// オブジェクト固有の場合
	else if (variableObjectId > agtk::data::PlayData::COMMON_PLAY_DATA_ID) {
		auto projectPlayData = GameManager::getInstance()->getPlayData();
		switch (variableQualifierId) {
		//単体
		case agtk::data::ObjectCommandData::kQualifierSingle: {
			auto singleInstanceData = projectPlayData->getVariableData(variableObjectId, agtk::data::kObjectSystemVariableSingleInstanceID);
			auto object = getTargetObjectByInstanceId((int)singleInstanceData->getValue());
			if (object) {
				auto playObjectData = object->getPlayObjectData();
				auto variableData = playObjectData->getVariableData(variableId);
				if (variableData) {
					retValue = variableData->getValue();
					playObjectData->adjustVariableData(variableData);
					ret = true;
				}
			}
			break; }
		//全体
		case agtk::data::ObjectCommandData::kQualifierWhole: {
			auto objectList = getTargetObjectListByObjectId(variableObjectId);
			cocos2d::Ref *ref;
			CCARRAY_FOREACH(objectList, ref) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
				auto object = static_cast<agtk::Object *>(ref);
#else
				auto object = dynamic_cast<agtk::Object *>(ref);
#endif
				auto playObjectData = object->getPlayObjectData();
				auto variableData = playObjectData->getVariableData(variableId);
				if (variableData) {
					retValue = variableData->getValue();
					playObjectData->adjustVariableData(variableData);
					ret = true;
				}
			}
			break; }
		default: {
			if (variableQualifierId >= 0) {
				auto objectList = getTargetObjectListByObjectId(variableObjectId);
				cocos2d::Ref *ref;
				CCARRAY_FOREACH(objectList, ref) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
					auto object = static_cast<agtk::Object *>(ref);
#else
					auto object = dynamic_cast<agtk::Object *>(ref);
#endif
					if (object->getInstanceId() == variableQualifierId) {
						auto playObjectData = object->getPlayObjectData();
						auto variableData = playObjectData->getVariableData(variableId);
						if (variableData) {
							retValue = variableData->getValue();
							playObjectData->adjustVariableData(variableData);
							ret = true;
						}
					}
				}
			}
			else {
				CC_ASSERT(0);
			}
			break; }
		}
	}
	//自身のオブジェクト
	else if (variableObjectId == agtk::data::ObjectCommandData::kSelfObject) {
		if (_object) {
			auto playData = _object->getPlayObjectData();
			auto variableData = playData->getVariableData(variableId);
			if (variableData) {
				retValue = variableData->getValue();
				playData->adjustVariableData(variableData);
				ret = true;
			}
		}
	}
	//ロックしたオブジェクト
	else if (variableObjectId == agtk::data::ObjectCommandData::kLockedObject) {
		if (_object) {
			auto objectList = getTargetObjectLocked(_object, agtk::SceneLayer::kTypeAll);
			cocos2d::Ref *ref = nullptr;
			CCARRAY_FOREACH(objectList, ref) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
				auto object = static_cast<agtk::Object *>(ref);
#else
				auto object = dynamic_cast<agtk::Object *>(ref);
#endif
				auto playObjectData = object->getPlayObjectData();
				auto variableData = playObjectData->getVariableData(variableId);
				if (variableData) {
					retValue = variableData->getValue();
					playObjectData->adjustVariableData(variableData);
					ret = true;
				}
			}
		}
	}
	// 親オブジェクト
	else if (variableObjectId == agtk::data::ObjectCommandData::kParentObject) {
		if (_object) {
			// 親の変数変化判定
			auto parentObjectInstanceId = _object->getPlayObjectData()->getParentObjectInstanceId();
			if (parentObjectInstanceId > -1) {
				auto parentObject = getTargetObjectByInstanceId(parentObjectInstanceId);
				if (parentObject) {
					auto playObjectData = parentObject->getPlayObjectData();
					auto variableData = playObjectData->getVariableData(variableId);
					if (variableData) {
						retValue = variableData->getValue();
						playObjectData->adjustVariableData(variableData);
						ret = true;
					}
				}
			}
		}
	}
	//未設定
	else if (variableObjectId == agtk::data::ObjectCommandData::kUnsetObject) {
	}
	else {
		//エラー
		CC_ASSERT(0);
	}
	return ret;
}

/**
* オブジェクトのアクションプログラム実行処理
* @param	objectActionList	アクションプログラムリスト
*/
void GameManager::calcObjectActionProgram(cocos2d::__Array *objectActionList)
{
	auto gameManager = GameManager::getInstance();
	auto projectPlayData = gameManager->getPlayData();

	cocos2d::Ref *ref = nullptr;
	CCARRAY_FOREACH(objectActionList, ref) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
		auto data = static_cast<agtk::data::ObjectActionExcecData *>(ref);
#else
		auto data = dynamic_cast<agtk::data::ObjectActionExcecData *>(ref);
#endif

		auto actionId = data->getActionBox() ? data->getActionId() : data->getCommonActionId();
		auto objectId = data->getObjectId();
		auto qualifierId = data->getQualifierId();

		// 指定オブジェクトIDのオブジェクトリストを取得する
		auto objectList = gameManager->getTargetObjectListByObjectId(objectId);

		// 単体 or 指定インスタンスの場合
		if (qualifierId == agtk::data::EnumQualifierType::kQualifierSingle || qualifierId > 0) {
			int instanceId = qualifierId;
			if (qualifierId == agtk::data::EnumQualifierType::kQualifierSingle) {
				auto p = projectPlayData->getVariableData(objectId, agtk::data::kObjectSystemVariableSingleInstanceID);
				if (p) {
					instanceId = (int)p->getValue();
				}
			}

			cocos2d::Ref *ref = nullptr;
			CCARRAY_FOREACH(objectList, ref) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
				auto object = static_cast<agtk::Object *>(ref);
#else
				auto object = dynamic_cast<agtk::Object *>(ref);
#endif
				if (object->getInstanceId() == instanceId) {
					int directionId = object->getDispDirection();
					object->playAction(actionId, directionId);
					object->getCurrentObjectAction()->update(0);
					break;
				}
			}
		}
		// 全体の場合
		else if (qualifierId == agtk::data::EnumQualifierType::kQualifierWhole) {
			cocos2d::Ref *ref = nullptr;
			CCARRAY_FOREACH(objectList, ref) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
				auto object = static_cast<agtk::Object *>(ref);
#else
				auto object = dynamic_cast<agtk::Object *>(ref);
#endif
				int directionId = object->getDispDirection();
				object->playAction(actionId, directionId);
				object->getCurrentObjectAction()->update(0);
			}
		}
	}
}

/**
* 画面切り替え時のBGM演出処理
* @param	duration	処理時間
* @param	changeType	BGM変更タイプ
* @param	isFadeOut	BGM停止時のフェードアウトフラグ
* @param	bgmInfo		BGM変更時の変更先BGMの情報
* @param	isPreMove	遷移演出前フラグ
*/
void GameManager::calcMoveBgmEffect(float duration, agtk::data::EnumBgmChangeType changeType, bool isFadeOut, const std::vector<BgmInfo>& bgmInfo, bool isPreMove)
{
	if (!getCurrentScene()) {
		return;
	}

	switch (changeType)
	{
		// ---------------------------------------------
		// BGMを継続する場合
		// ---------------------------------------------
	case agtk::data::kBgmChangeTypeContinue:
		// 何もしない
		break;
		// ---------------------------------------------
		// BGMを停止する場合
		// ---------------------------------------------
	case agtk::data::kBgmChangeTypeStop:
	{
		// フェードアウトする場合
		if (isFadeOut) {
			this->setIsBgmFadeOut(true);
			float preVolume = AudioManager::getInstance()->getBgmVolume();
			getCurrentLayer()->runAction(Sequence::create(
				AudioManager::getInstance()->fadeOutBgmVolume(duration, 0.0f),
				CallFunc::create([preVolume]() {
				AudioManager::getInstance()->stopAllBgm();
				AudioManager::getInstance()->setBgmVolume(preVolume);
				GameManager::getInstance()->setIsBgmFadeOut(false);
			}),
				nullptr
				));
		}
		// フェードアウトしない場合
		else {
			// フェードアウト中でない場合
			if (!getIsBgmFadeOut()) {
				// 停止する
				AudioManager::getInstance()->stopAllBgm();
			}
		}
	} break;
	// ---------------------------------------------
	// BGMを変更する場合
	// ---------------------------------------------
	case agtk::data::kBgmChangeTypeChange:
	{
		auto stopOtherBgm = true;// 他のBGMをとめるか
		_preloadBgmEnd.clear();

		//シーン変更中かつ演出前の場合。
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
		auto gameScene = static_cast<GameScene *>(this->getCurrentLayer());
#else
		auto gameScene = dynamic_cast<GameScene *>(this->getCurrentLayer());
#endif
		if (isPreMove &&  gameScene->getSceneChangeState() != GameScene::SceneChangeState::NONE) {
			_registeringStopBgm.clear();
			auto bgmIdList = AudioManager::getInstance()->getBgmIdList();
			if (bgmIdList->count() > 0) {
				cocos2d::Ref *ref;
				CCARRAY_FOREACH(bgmIdList, ref) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
					auto bgmId = static_cast<cocos2d::Integer *>(ref);
#else
					auto bgmId = dynamic_cast<cocos2d::Integer *>(ref);
#endif
					BgmInfo bgm;
					bgm._bgmId = bgmId->getValue();
					_registeringStopBgm.push_back(bgm);
				}
			}
		}

		for(auto it = bgmInfo.begin(); it != bgmInfo.end(); ++it)
		{
			GameManager::BgmInfo* pbgm = (GameManager::BgmInfo *)(&(*it));
			GameManager::BgmInfo bgm = *pbgm;

			auto const bUseSceneSettingBgm = bgm._bgmId < 0 ? true : false;// シーンに設定されているBGMを指定されているか？

			if (bgm._bgmId == USE_SCENE_SETTING_BGM) 
			{// bgmId が「シーン設定のBGM」の場合

				CCASSERT(bgmInfo.size() == 1, "想定外");// 念のため。

				auto sceneData = this->getCurrentScene()->getSceneData();
				if (sceneData->getPlayBgmType() == agtk::data::SceneData::EnumPlayBgmType::kPlayBgmContinue) 
				{// 設定が「継続」の場合
					break;// なにもしない
				}
				else 
				{// BGMが指定されている場合
					bgm._bgmId = sceneData->getBgmId();
					bgm._loop = sceneData->getLoopBgmFlag();
					if (bgm._bgmId < 0) 
					{//「設定無し」
						getCurrentLayer()->runAction(Sequence::create(
							DelayTime::create(getIsBgmFadeOut() ? duration : 0.01f),
							CallFunc::create([]() {
								auto audioManager = AudioManager::getInstance();
								// AudioManager の update が呼ばれる前にこのアクションが呼ばれる為先にupdateをコールする
								audioManager->update(0);
								audioManager->stopAllBgmNonObject();
							}),
							nullptr));
						break; // 全てのBGMをとめ、何も再生しない。
					}
				}
			}
																					  
			if (bgm._bgmId > -1) {
				if (isPreMove) {
					this->setPreMoveBgmId(bgm._bgmId);
					this->setPreMoveBgmLoopFlag(bgm._loop);
				}
				if (!bUseSceneSettingBgm || bUseSceneSettingBgm && !isPreMove) 
				{
					auto audioManager = AudioManager::getInstance();
					audioManager->preloadBgm(bgm._bgmId, [this, bgm](bool isSuccess) {
						this->_preloadBgmEnd.push_back(bgm._bgmId);
					});
					//BGM再生登録
					getCurrentLayer()->runAction(agtk::Sequence::create(
						DelayTime::create(getIsBgmFadeOut() ? duration : 0.01f),
						agtk::IfCallFunc::create([this, bgm, stopOtherBgm]() {
							auto audioManager = AudioManager::getInstance();
							auto & preLoadBgms = this->_preloadBgmEnd;
							if( preLoadBgms.empty() || (std::find(preLoadBgms.begin(), preLoadBgms.end(), bgm._bgmId) == preLoadBgms.end())) {
								if (!audioManager->isAudioEngine()) {//Engineのインスタンスが無い（デバイスが無い）
									return true;
								}
								return false;
							}
							// AudioManager の update が呼ばれる前にこのアクションが呼ばれる為先にupdateをコールする
							audioManager->update(0);
							if (stopOtherBgm) {
								audioManager->stopAllBgmNonObject();
								_registeringStopBgm.clear();
							}
							// 新しいBGMを再生
							audioManager->playBgm(bgm._bgmId, bgm._loop, bgm._volume, bgm._pan, bgm._pitch);

							// リストから破棄。
							this->clearLoadedBgm(bgm);
							this->clearRegisteringBgm(bgm);

							return true;
						}),
						nullptr)
					);
	
					bool registBgmFlag = true;
					auto bgmIdList = audioManager->getBgmIdList();
					if (bgmIdList->count() > 0) {
						cocos2d::Ref *ref;
						CCARRAY_FOREACH(bgmIdList, ref) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
							auto bgmId = static_cast<cocos2d::Integer *>(ref);
#else
							auto bgmId = dynamic_cast<cocos2d::Integer *>(ref);
#endif
							if (bgmId->getValue() == bgm._bgmId) {
								registBgmFlag = false;
								break;
							}
						}
					}
					if (registBgmFlag) {
						_registeringBgm.push_back(bgm);
					}

					stopOtherBgm = false;// 他のBGMをとめるのは1つ目BGM再生時のみ。2つ目以降のBGM再生時に他のBGMをとめるとそれまでに再生していたものが止められてしまうので。
				}
			}
		}
	} break;
	}
}

void GameManager::actionLog(bool bPrintTime, const char *fmt, ...)
{
// #AGTK-NX
#if (CC_TARGET_PLATFORM == CC_PLATFORM_NX)
#endif
	va_list ap;
	va_start(ap, fmt);
	{
		std::string tmp;

		//ログを出力する。
		auto fileUtils = FileUtils::getInstance();
		auto path = fileUtils->getApplicationPath() + "action_log.txt";
		if (mLogNumber > 0) {
			path = fileUtils->getApplicationPath() + cocos2d::String::createWithFormat("action_log%d.txt", mLogNumber)->getCString();
		}
		auto filePath = fileUtils->fullPathForFilename(path);
		if (bPrintTime) {
			//時間
			time_t timer;
			time(&timer);
			auto t_st = localtime(&timer);
			char timeStr[128];
			std::sprintf(timeStr, "[%d/%02d/%02d %02d:%02d:%02d]", 1900 + t_st->tm_year, t_st->tm_mon + 1, t_st->tm_mday, t_st->tm_hour, t_st->tm_min, t_st->tm_sec);
			tmp = std::string(timeStr) + std::string(" ");
		}
		const int len = vsnprintf(NULL, 0, fmt, ap);
		char *buf = (char *)malloc(len + 1);
		buf[len] = 0;
		vsnprintf(buf, len + 1, fmt, ap);
		tmp += buf;
		tmp += std::string("\n");

		//ログ書き込み。
		static bool bFirst = true;
		if (bFirst) {
			//write BOM at first.
			fileUtils->writeStringToFile("\xEF\xBB\xBF", filePath);
			bFirst = false;
		}
		fileUtils->addStringToFile(tmp, filePath);
		free(buf);
	}
}

void GameManager::debugLog(const char *fmt, ...)
{
	va_list ap;
	va_start(ap, fmt);
	{
		std::string tmp;

// #AGTK-NX
#if (CC_TARGET_PLATFORM == CC_PLATFORM_NX)
#else
		auto fileUtils = FileUtils::getInstance();
		auto path = fileUtils->getApplicationPath() + "agtk_debug.txt";
		auto filePath = fileUtils->fullPathForFilename(path);
#endif

		//時間
		time_t timer;
		time(&timer);
		auto t_st = localtime(&timer);
		char timeStr[128];
		std::sprintf(timeStr, "[%d/%02d/%02d %02d:%02d:%02d] ", 1900 + t_st->tm_year, t_st->tm_mon + 1, t_st->tm_mday, t_st->tm_hour, t_st->tm_min, t_st->tm_sec);

		const int len = vsnprintf(NULL, 0, fmt, ap);
		char *buf = (char *)malloc(len + 1);
		buf[len] = 0;
		vsnprintf(buf, len + 1, fmt, ap);
		tmp += timeStr;
		tmp += buf;
		tmp += std::string("\n");

		std::string s = agtk::UTF8toSjis(tmp);

		static bool bFirst = false;
		if (bFirst == false) {
// #AGTK-NX
#if (CC_TARGET_PLATFORM == CC_PLATFORM_NX)
#else
			fileUtils->writeStringToFile(s, filePath);
#endif
			bFirst = true;
		}
		else {
// #AGTK-NX
#if (CC_TARGET_PLATFORM == CC_PLATFORM_NX)
#else
			fileUtils->addStringToFile(s, filePath);
#endif
		}
		free(buf);
	}
	va_end(ap);
}

void GameManager::setLogNumber(int logNumber)
{
	mLogNumber = logNumber;
}

/**
* 指定のインスタンスIDのオブジェクトを取得する
* @param	instanceId	インスタンスID
* @return				オブジェクト
*/
agtk::Object *GameManager::getTargetObjectByInstanceId(int instanceId)
{
	auto scene = this->getCurrentScene();
	if (nullptr == scene) {
		return nullptr;
	}
// #AGTK-NX
#ifdef USE_SAR_OPTIMIZE_3
	if (instanceId >= 0) {
		return scene->getObjectInstance(-1, instanceId);
	}
#else
// #AGTK-NX
#ifdef USE_SAR_OPTIMIZE_1
	auto allObjects = scene->getObjectAllReference();
#else
	auto allObjects = scene->getObjectAll();
#endif
	cocos2d::Ref *ref = nullptr;
	CCARRAY_FOREACH(allObjects, ref) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
		auto object = static_cast<agtk::Object *>(ref);
#else
		auto object = dynamic_cast<agtk::Object *>(ref);
#endif
		auto playObjectData = object->getPlayObjectData();
		if (playObjectData->getInstanceId() == instanceId) {
			return object;
		}
	}
#endif
	return nullptr;
}

/**
* 指定のオブジェクトIDのオブジェクトリスト取得
* @param	objectId	オブジェクトID
* @return				オブジェクトリスト
*/
cocos2d::__Array *GameManager::getTargetObjectListByObjectId(int objectId)
{
	auto scene = this->getCurrentScene();
	if (nullptr == scene) {
		return nullptr;
	}
// #AGTK-NX
#ifdef USE_SAR_OPTIMIZE_3
	return scene->getObjectAll(objectId);
#else
	auto objectList = cocos2d::__Array::create();
// #AGTK-NX
#ifdef USE_SAR_OPTIMIZE_1
	auto objectAllList = scene->getObjectAllReference();
#else
	auto objectAllList = scene->getObjectAll();
#endif
	cocos2d::Ref *ref = nullptr;
	CCARRAY_FOREACH(objectAllList, ref) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
		auto object = static_cast<agtk::Object *>(ref);
#else
		auto object = dynamic_cast<agtk::Object *>(ref);
#endif
		auto objectData = object->getObjectData();
		if (objectData->getId() == objectId) {
			objectList->addObject(object);
		}
	}
	return objectList;
#endif
}

cocos2d::__Array *GameManager::getTargetObjectLocked(agtk::Object* _object, int sceneLayerType)
{
	auto scene = _object->getSceneLayer()->getScene();
	auto sceneLayer = _object->getSceneLayer();
	if (sceneLayerType < 0) sceneLayerType = sceneLayer->getType();
#ifdef USE_LOCKING_MULTI_TARGET_FIX
	return scene->getObjectAllLocked(_object->getInstanceId(), (agtk::SceneLayer::EnumType)sceneLayerType);
#else
	auto objectId = _object->getObjectData()->getId();
	return scene->getObjectAllLocked(objectId, sceneLayer->getType());
#endif
}

bool GameManager::checkPortalTouched(agtk::Object *object)
{
	auto portalTouchedPlayerList = this->getPortalTouchedPlayerList();
	if (portalTouchedPlayerList != nullptr) {
		cocos2d::Ref *ref;
		CCARRAY_FOREACH(portalTouchedPlayerList, ref) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
			auto data = static_cast<agtk::PortalTouchedData *>(ref);
#else
			auto data = dynamic_cast<agtk::PortalTouchedData *>(ref);
#endif
			if (object == data->getObject()) {
				return true;
			}
			if (data->getObject()->getChildrenObjectList()->count() > 0) {
				if (data->getObject()->getChildrenObjectList()->containsObject(object)) {
					return true;
				}
			}
			if (data->getObject()->getConnectObjectList()->count() > 0) {
				if (data->getObject()->getConnectObjectList()->containsObject(object)) {
					return true;
				}
			}
		}
	}
	return false;
}

/**
 * スイッチデータの設定
 * @param	assignValue	設定するスイッチのタイプ
 * @param	switchData	設定対象のスイッチデータ
 */
void GameManager::assignSwitchData(agtk::data::SwitchVariableAssignData::EnumSwitchAssignValue assignValue, agtk::data::PlaySwitchData *switchData)
{
	if (nullptr == switchData) {
		return;
	}

	switch (assignValue) {
	case agtk::data::SwitchVariableAssignData::kSwitchAssignOn:
		switchData->setValue(true);
		break;
	case agtk::data::SwitchVariableAssignData::kSwitchAssignOff:
		switchData->setValue(false);
		break;
	case agtk::data::SwitchVariableAssignData::kSwitchAssignToggle:
		switchData->setValue(!switchData->getValue());
		break;
	default:CC_ASSERT(0);
	}
}

/**
* 変数データの設定
* @param	op				変数設定オペレーションタイプ
* @param	variableData	設定対象の変数データ
* @param	value			設定値
*/
void GameManager::assignVariableData(agtk::data::SwitchVariableAssignData::EnumVariavleAssignValue op, agtk::data::PlayVariableData *variableData, double value)
{
	if (nullptr == variableData) {
		return;
	}

	double nowValue = variableData->getValue();
	switch (op) {
		// ------------------------------------
		// 代入
		// ------------------------------------
	case agtk::data::SwitchVariableAssignData::kVariavleSubstitute:
		variableData->setValue(value);
		break;
		// ------------------------------------
		// 加算
		// ------------------------------------
	case agtk::data::SwitchVariableAssignData::kVariableAdd:
		variableData->setValue(nowValue + value);
		break;
		// ------------------------------------
		// 減算
		// ------------------------------------
	case agtk::data::SwitchVariableAssignData::kVariableSub:
		variableData->setValue(nowValue - value);
		break;
		// ------------------------------------
		// 乗算
		// ------------------------------------
	case agtk::data::SwitchVariableAssignData::kVariableMul:
		variableData->setValue(nowValue * value);
		break;
		// ------------------------------------
		// 除算
		// ------------------------------------
	case agtk::data::SwitchVariableAssignData::kVariableDiv:
		variableData->setValue(value == 0 ? std::nan("1") : (nowValue / value));
		break;
		// ------------------------------------
		// 余りを代入
		// ------------------------------------
	case agtk::data::SwitchVariableAssignData::kVariableMod:
		variableData->setValue(value == 0 ? std::nan("1") : (double)((int)nowValue % (int)value));
		break;
	}
}

//--------------------------------------------------------------------------------------------------------------------
// !物理空間に関するメソッド群
//--------------------------------------------------------------------------------------------------------------------
/**
* 物理演算イテレーション回数取得
* @return イテレーション回数
*/
int GameManager::getPhysicsIterations()
{
	return this->_physicsWorld->getIterations();
}

/**
* 物理演算イテレーション回数取得
* @param	iterations		イテレーション回数
*/
void GameManager::setPhysicsIterations(int iterations/* = 10*/)
{
	this->_physicsWorld->setIterations(iterations);
}

/**
* 重力の設定
* @param	graviry		重力の量
* @param	rotation	重力のかかる角度
*/
void GameManager::setGravity(float gravity, float rotation)
{
	Vec2 direction = agtk::GetDirectionFromDegrees(rotation) * (gravity * DOT_PER_METER);
	this->_physicsWorld->setGravity(direction);
}

/**
* 物理空間の更新
* @param	dt			前のフレームからの経過時間
*/
void GameManager::updatePhysics(float dt)
{
	// 全ての物理ボディ取得
	auto list = this->_physicsWorld->getAllBodies();

	// タイルと坂以外の物理ボディがあるかチェック
	bool isAlivePhyiscsObj = false;
	for (auto body : list) {
		auto group = body->getGroup();
		if (group != EnumPhysicsGroup::kTile && group != EnumPhysicsGroup::kSlope) {
			isAlivePhyiscsObj = true;
			break;
		}
	}

	// タイルと坂以外の物理ボディが無い場合
	if (!isAlivePhyiscsObj && !this->_alivePhysicsObj) {
		return;
	}

	// 物理ボディの存在フラグが前フレームと異なる場合
	if (isAlivePhyiscsObj != this->_alivePhysicsObj) {
		this->_alivePhysicsObj = isAlivePhyiscsObj;
	}

	this->_physicsWorld->step(dt);

	// 全ての動的物理オブジェクトに空気抵抗を与える
	for (auto body : list) {
		if (body->isDynamic() && body->getFirstShape()) {
			auto vec = body->getVelocity();//速度(dot)
			auto vec2 = vec / DOT_PER_METER;//速度(dot)を速度(m)に単位変換
			auto radius = sqrt(body->getFirstShape()->getArea() / M_PI) / DOT_PER_METER;//半径(矩形も円としてみなす)
			auto airForce = 0.5f * AIR_DENSITY * AIR_RESISTANCE_COE * Vec2(vec2.x * abs(vec2.x), vec2.y * abs(vec2.y)) * radius * _airResistance;//空気抵抗力(F=1/2*ρ*Cd*v^2*S)
			auto afv = airForce / body->getMass();//運動の第二法則から空気抵抗力の速度を算出
			body->setVelocity(vec - afv);//空気抵抗を加味して速度を設定
		}
	}
}

/**
* 物理空間のデバッグ表示設定
* @param	flg			表示のON/OFF
*/
void GameManager::setPhysicsDebugVisible(bool flg)
{
#ifdef USE_PREVIEW
	// 物理ボディが一つもない場合
	if (this->_physicsWorld->getAllBodies().size() <= 0) {
		// なにもしない
		return;
	}

	int mask = flg ? (PhysicsWorld::DEBUGDRAW_ALL | PhysicsWorld::DEBUGDRAW_OUTSIDE_DEBUGDRAW) : PhysicsWorld::DEBUGDRAW_NONE;
	this->_physicsWorld->setDebugDrawMask(mask);

	auto scene = getCurrentScene();
	if (scene == nullptr) {
		return;
	}
	auto sceneLayer = scene->getSceneLayerList();
	cocos2d::DictElement *el = nullptr;
	CCDICT_FOREACH(sceneLayer, el) {
		auto layer = dynamic_cast<agtk::SceneLayer *>(el->getObject());
		CC_ASSERT(layer);
		cocos2d::Ref *ref = nullptr;
		CCARRAY_FOREACH(layer->getPhysicsObjectList(), ref) {
			auto physicsBody = dynamic_cast<agtk::PhysicsBase *>(ref);
			CC_ASSERT(physicsBody);
			physicsBody->showDebugVisible(flg);
		}
	}
#endif//USE_PREVIEW
}

/**
* 物理：衝突開始イベントコールバック
* @param	contact	衝突データ
* @return	衝突処理継続フラグ
*/
bool GameManager::onContactBeginCallback(cocos2d::PhysicsContact &contact)
{
	auto shapeA = contact.getShapeA();
	auto shapeB = contact.getShapeB();
	auto groupA = shapeA->getGroup();
	auto groupB = shapeB->getGroup();

	// オブジェクトとオブジェクトの場合。
	if (groupA == EnumPhysicsGroup::kObject && groupB == EnumPhysicsGroup::kObject) {
		auto bodyA = shapeA->getBody();
		auto bodyB = shapeB->getBody();
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
		auto objectA = static_cast<agtk::Object *>(bodyA->getOwner()->getParent());
		auto objectB = static_cast<agtk::Object *>(bodyB->getOwner()->getParent());
#else
		auto objectA = dynamic_cast<agtk::Object *>(bodyA->getOwner()->getParent());
		auto objectB = dynamic_cast<agtk::Object *>(bodyB->getOwner()->getParent());
#endif
		bool ignoredObjectWallA = objectA->getObjectTemplateMove()->isIgnoredObjectWall();
		bool ignoredObjectWallB = objectB->getObjectTemplateMove()->isIgnoredObjectWall();
		auto physicsSettingA = objectA->getObjectData()->getPhysicsSetting();
		auto physicsSettingB = objectB->getObjectData()->getPhysicsSetting();
		auto playObjectDataA = objectA->getPlayObjectData();
		auto playObjectDataB = objectB->getPlayObjectData();
		// 「他の物理オブジェクトに影響を与える」 または 「物理影響を受ける」または「接続された物理オブジェクトの動作を優先」でない場合
		bool bPhysicsAFlag = ((playObjectDataA->getSwitchData(agtk::data::kObjectSystemSwitchAffectOtherObjects)->getValue() || playObjectDataA->getSwitchData(agtk::data::kObjectSystemSwitchAffectedByOtherObjects)->getValue()) && !playObjectDataA->getSwitchData(agtk::data::kObjectSystemSwitchFollowConnectedPhysics)->getValue());
		bool bPhysicsBFlag = ((playObjectDataB->getSwitchData(agtk::data::kObjectSystemSwitchAffectOtherObjects)->getValue() || playObjectDataB->getSwitchData(agtk::data::kObjectSystemSwitchAffectedByOtherObjects)->getValue()) && !playObjectDataB->getSwitchData(agtk::data::kObjectSystemSwitchFollowConnectedPhysics)->getValue());
		return shapeA->getGroup() > 0 && (ignoredObjectWallA == false && ignoredObjectWallB == false);
	}

	// --------------------------------------------------------------
	// 物理オブジェクトの非衝突グループチェック
	// --------------------------------------------------------------
	// タイル・坂・力系のグループでない場合
	if (groupA > 0 && groupB > 0) {

		auto getLayerIdFromPhysics = [](int groupId, cocos2d::PhysicsBody *body) -> int {
			int layerId = -1;

			if (groupId == EnumPhysicsGroup::kObject || groupId == EnumPhysicsGroup::kNoneObject) {
// #AGTK-NX
#if defined(STATIC_DOWN_CAST) && defined(FORCE_STATIC_DOWN_CAST)
				layerId = static_cast<agtk::Object *>(body->getOwner()->getParent())->getLayerId();
			}
			else {
				layerId = static_cast<agtk::PhysicsBase *>(body->getOwner())->getLayerId();
			}
#else
				layerId = dynamic_cast<agtk::Object *>(body->getOwner()->getParent())->getLayerId();
			}
			else {
				layerId = dynamic_cast<agtk::PhysicsBase *>(body->getOwner())->getLayerId();
			}
#endif

			return layerId;
		};

		auto bodyA = shapeA->getBody();
		auto bodyB = shapeB->getBody();
		auto layerIdOfA = getLayerIdFromPhysics(groupA, bodyA);
		auto layerIdOfB = getLayerIdFromPhysics(groupB, bodyB);

		// レイヤーIDが異なる or 非衝突グループの論理積の結果が 0 でない場合
		if ((layerIdOfA != layerIdOfB) || (bodyA->getTag() & bodyB->getTag()) != 0) {
			// 衝突させない
			return false;
		}
		// 通常レイヤーとメニューレイヤーとでは衝突させない。
		if (!(shapeA->getCategoryBitmask() & shapeB->getCategoryBitmask())) {
			return false;
		}

		// いずれがオブジェクトの場合
		if (groupA == EnumPhysicsGroup::kNoneObject || groupB == EnumPhysicsGroup::kNoneObject) {
			return false;
		}
	}

	// --------------------------------------------------------------
	// 同一IDのロープ同士が衝突した場合は衝突させない
	// --------------------------------------------------------------
	if (groupA == EnumPhysicsGroup::kRope && groupB == EnumPhysicsGroup::kRope) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
		auto ropeA = static_cast<agtk::PhysicsRopeParts *>(shapeA->getBody()->getOwner());
		auto ropeB = static_cast<agtk::PhysicsRopeParts *>(shapeB->getBody()->getOwner());
#else
		auto ropeA = dynamic_cast<agtk::PhysicsRopeParts *>(shapeA->getBody()->getOwner());
		auto ropeB = dynamic_cast<agtk::PhysicsRopeParts *>(shapeB->getBody()->getOwner());
#endif

		return (ropeA->getScenePartsId() != ropeB->getScenePartsId());
	}

	// --------------------------------------------------------------
	// ロープの衝突回避チェック
	// --------------------------------------------------------------
	if (groupA == EnumPhysicsGroup::kRope) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
		auto ropeA = static_cast<agtk::PhysicsRopeParts *>(shapeA->getBody()->getOwner());
#else
		auto ropeA = dynamic_cast<agtk::PhysicsRopeParts *>(shapeA->getBody()->getOwner());
#endif
		ropeA->setIsContacting(true);
		if (ropeA->getIgnoreBody() == shapeB->getBody()) {
			return false;
		}
	}
	else if (groupB == EnumPhysicsGroup::kRope) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
		auto ropeB = static_cast<agtk::PhysicsRopeParts *>(shapeB->getBody()->getOwner());
#else
		auto ropeB = dynamic_cast<agtk::PhysicsRopeParts *>(shapeB->getBody()->getOwner());
#endif
		ropeB->setIsContacting(true);
		if (ropeB->getIgnoreBody() == shapeA->getBody()) {
			return false;
		}
	}

	// --------------------------------------------------------------
	// 坂と物理オブジェクトが衝突した場合の衝突パスチェック
	// --------------------------------------------------------------
	if (groupA == EnumPhysicsGroup::kSlope && groupB > 0) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
		auto slope = static_cast<agtk::Slope *>(shapeA->getBody()->getNode());
#else
		auto slope = dynamic_cast<agtk::Slope *>(shapeA->getBody()->getNode());
#endif
		return checkPassSlope(slope, shapeB->getBody()->getOwner()->getPosition());
	}
	else if (groupB == EnumPhysicsGroup::kSlope && groupA > 0) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
		auto slope = static_cast<agtk::Slope *>(shapeB->getBody()->getNode());
#else
		auto slope = dynamic_cast<agtk::Slope *>(shapeB->getBody()->getNode());
#endif
		return checkPassSlope(slope, shapeA->getBody()->getOwner()->getPosition());
	}

	// --------------------------------------------------------------
	// タイルと物理オブジェクトが衝突した場合の衝突パスチェック
	// --------------------------------------------------------------
	if (groupA == EnumPhysicsGroup::kTile && groupB > 0) {
		auto box = dynamic_cast<PhysicsShapeBox *>(shapeA);
		if (box) return true;
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
		auto segmentA = static_cast<PhysicsShapeEdgeSegment *>(shapeA);
#else
		auto segmentA = dynamic_cast<PhysicsShapeEdgeSegment *>(shapeA);
#endif
		return checkPassTile(segmentA, shapeB->getBody()->getOwner()->getPosition());
	}
	else if (groupB == EnumPhysicsGroup::kTile && groupA > 0) {
		auto box = dynamic_cast<PhysicsShapeBox *>(shapeB);
		if (box) return true;
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
		auto segmentB = static_cast<PhysicsShapeEdgeSegment *>(shapeB);
#else
		auto segmentB = dynamic_cast<PhysicsShapeEdgeSegment *>(shapeB);
#endif
		return checkPassTile(segmentB, shapeA->getBody()->getOwner()->getPosition());
	}

	// --------------------------------------------------------------
	// オブジェクトと物理オブジェクトが衝突した場合
	// --------------------------------------------------------------
	auto createHitData = [](agtk::Object * obj, cocos2d::Vec2 normal, cocos2d::PhysicsBody * body) {
		auto hitData = agtk::Object::HitPhysicsObjData::create();
		hitData->setPhysicsBody(body);
		hitData->setDirectionVec(normal);
		obj->getHitPhysicsObjList()->addObject(hitData);
	};

	if (groupA == EnumPhysicsGroup::kObject && groupB == EnumPhysicsGroup::kPhysicalObject) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
		auto obj = static_cast<agtk::Object *>(shapeA->getBody()->getOwner()->getParent());
#else
		auto obj = dynamic_cast<agtk::Object *>(shapeA->getBody()->getOwner()->getParent());
#endif
		createHitData(obj, contact.getContactData()->normal, shapeB->getBody());
	}
	else if (groupB == EnumPhysicsGroup::kObject && groupA == EnumPhysicsGroup::kPhysicalObject) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
		auto obj = static_cast<agtk::Object *>(shapeB->getBody()->getOwner()->getParent());
#else
		auto obj = dynamic_cast<agtk::Object *>(shapeB->getBody()->getOwner()->getParent());
#endif
		createHitData(obj, contact.getContactData()->normal, shapeA->getBody());
	}

	return true;
}

/**
* 物理：衝突時の事前解決イベントコールバック
* @param	contact		衝突データ
* @param	preSolve	事前解決データ
* @return	衝突処理継続フラグ
*/
bool GameManager::onContactPreSolveCallback(cocos2d::PhysicsContact &contact, cocos2d::PhysicsContactPreSolve &preSolve)
{
	//	auto shapeA = contact.getShapeA();
	//	auto shapeB = contact.getShapeB();
	//	auto groupA = shapeA->getGroup();
	//	auto groupB = shapeB->getGroup();
	//
	//		// オブジェクトAがタイル・坂・力系のグループでない場合
	//		if (groupA > 0) {
	////			preSolve.setFriction(1.0f);
	////			preSolve.setRestitution(1.0f);
	//		}
	//
	//		// オブジェクトBがタイル・坂・力系のグループでない場合
	//		if (groupB > 0) {
	////			preSolve.setFriction(1.0f);
	////			preSolve.setRestitution(1.0f);
	//		}

	return true;
}

/**
* 物理：衝突から離れたイベントコールバック
* @param	contact	衝突データ
*/
void GameManager::onContactSeparateCallback(cocos2d::PhysicsContact &contact)
{
	auto shapeA = contact.getShapeA();
	auto shapeB = contact.getShapeB();
	auto groupA = shapeA->getGroup();
	auto groupB = shapeB->getGroup();

	// --------------------------------------------------------------
	// 衝突していたオブジェクトと物理オブジェクトが離れた場合
	// --------------------------------------------------------------
	auto removeFunc = [](agtk::Object * obj, cocos2d::PhysicsBody * body) {
		for (int i = obj->getHitPhysicsObjList()->count() - 1; i >= 0; i--) {
			auto ref = obj->getHitPhysicsObjList()->getObjectAtIndex(i);
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
			auto hitData = static_cast<agtk::Object::HitPhysicsObjData *>(ref);
#else
			auto hitData = dynamic_cast<agtk::Object::HitPhysicsObjData *>(ref);
#endif
			if (hitData->getPhysicsBody() == body) {
				obj->getHitPhysicsObjList()->removeObjectAtIndex(i);
			}
		}
	};

	if (groupA == EnumPhysicsGroup::kObject && groupB == EnumPhysicsGroup::kPhysicalObject) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
		auto obj = static_cast<agtk::Object *>(shapeA->getBody()->getOwner()->getParent());
#else
		auto obj = dynamic_cast<agtk::Object *>(shapeA->getBody()->getOwner()->getParent());
#endif
		removeFunc(obj, shapeB->getBody());
	}
	else if (groupB == EnumPhysicsGroup::kObject && groupA == EnumPhysicsGroup::kPhysicalObject) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
		auto obj = static_cast<agtk::Object *>(shapeB->getBody()->getOwner()->getParent());
#else
		auto obj = dynamic_cast<agtk::Object *>(shapeB->getBody()->getOwner()->getParent());
#endif
		removeFunc(obj, shapeA->getBody());
	}
}

/**
* 物理：坂との衝突回避チェック
* @param	slope			坂オブジェクト
* @param	targetPosition	衝突したオブジェクトの位置
* @param					True:衝突する / False:衝突回避する
*/
bool GameManager::checkPassSlope(agtk::Slope *slope, cocos2d::Vec2 targetPosition)
{
	auto slopeStart = agtk::Scene::getPositionSceneFromCocos2d(slope->start);
	auto slopeVec = (agtk::Scene::getPositionSceneFromCocos2d(slope->end) - slopeStart);

	// 坂のベクトルと始点から衝突オブジェクトの位置へのベクトルとの外積を算出
	auto cross = slopeVec.cross(targetPosition - slopeStart);

	// 下から通過可能で下から衝突した or 上から通過可能で上から衝突した場合
	if (cross < 0 && slope->getSlopeData()->getPassableFromLower() || cross > 0 && slope->getSlopeData()->getPassableFromUpper()) {
		// 衝突させない
		return false;
	}

	return true;
}

/**
* 物理：タイルの壁の線分との衝突回避チェック
* @param	line			タイルの壁用物理線分
* @param	targetPosition	衝突したオブジェクトの位置
* @param					True:衝突する / False:衝突回避する
*/
bool GameManager::checkPassTile(PhysicsShapeEdgeSegment *line, cocos2d::Vec2 targetPosition)
{
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
	auto tile = static_cast<agtk::Tile *>(line->getBody()->getNode());
#else
	auto tile = dynamic_cast<agtk::Tile *>(line->getBody()->getNode());
#endif
	auto start = tile->getPosition() + line->getPointA();				// 始点
	auto end = tile->getPosition() + line->getPointB();					// 終点
	auto vec = end - start;
	auto cross = vec.cross(targetPosition - start);
	return cross > 0;
}

#ifdef USE_PREVIEW
void GameManager::createWebsocket(char *hostname, int port)
{
	if (this->getWebSocket()) {
		return;
	}
	auto ws = agtk::WebSocket::create();
	this->setWebSocket(ws);
	ws->onConnectionOpened = [&]() {
#if defined(AGTK_DEBUG)
		agtk::DebugManager::getInstance()->getLogList()->addObject(cocos2d::__String::create("CONNECTED"));
#endif
	};
	ws->onConnectionClosed = [&]() {
#if defined(AGTK_DEBUG)
		agtk::DebugManager::getInstance()->getLogList()->addObject(cocos2d::__String::create("DISCONNECTED"));
#endif
	};
	ws->onMessageReceived = [&](std::string msg) {
#if defined(AGTK_DEBUG)
		agtk::DebugManager::getInstance()->getLogList()->addObject(cocos2d::__String::createWithFormat("TEXT: %s", msg.c_str()));
		CCLOG("%s", msg.c_str());
#endif
		if (msg == "terminate") {
			_bTerminate = true;
		}
#ifdef USE_PREVIEW
		else if (msg[0] == '{') {
			rapidjson::Document doc;
			doc.Parse(msg.c_str());
			bool error = doc.HasParseError();
			if (error) {
				CCASSERT(0, "Error: Json Parse.");
			}
			else {
				auto it = doc.MemberBegin();
				while (it != doc.MemberEnd()) {
					auto key = it->name.GetString();
					if (strcmp(key, "system") == 0) {
						runCommand(it->value);
					}
					else if (strcmp(key, "particle") == 0) {
						ParticleManager::getInstance()->runCommand(doc[key]);
					}
					else if (strcmp(key, "object") == 0) {
						runObjectCommand(it->value);
					}
					else if (strcmp(key, "scene") == 0) {
						runSceneCommand(it->value);
					}
					else if (strcmp(key, "tileset") == 0) {
						runTilesetCommand(it->value);
					}
					else if (strcmp(key, "image") == 0) {
						runImageCommand(it->value);
					}
					else if (strcmp(key, "text") == 0) {
						runTextCommand(it->value);
					}
					it++;
				}
			}
		}
#endif
	};
	ws->onBinaryMessageReceived = [&](char *binaryMsg, unsigned int len) {
		CCLOG("binaryLen:%d", len);
#if 0
		int num = 0;
		for (unsigned int i = 0; i < len; i++) {
			num += binaryMsg[i];
		}
		if (num == 0) {
			this->getLogList()->addObject(cocos2d::__String::create("RECIEVED BINARY (ZERO)--------------"));
			this->getLogList()->addObject(cocos2d::__String::createWithFormat("DATA SIZE:%d bytes", len));
			return;
		}
		//画像ファイルを受取、描画する。
		auto image = new (std::nothrow) cocos2d::Image();
		char fname[128];
		sprintf(fname, "binary%d", time(NULL));
		CCLOG("fname:%s", fname);
		image->initWithImageData((unsigned char *)binaryMsg, len);
		auto texture = cocos2d::Director::getInstance()->getTextureCache()->addImage(image, fname);

		auto spriteFrame = cocos2d::SpriteFrame::createWithTexture(texture, cocos2d::Rect(0, 0, texture->getContentSize().width, texture->getContentSize().height));
		cocos2d::SpriteFrameCache::getInstance()->addSpriteFrame(spriteFrame, fname);
		CC_SAFE_RELEASE(image);

		auto winSize = cocos2d::Director::getInstance()->getWinSize();
		auto sprite = cocos2d::Sprite::createWithTexture(texture);
		float x = ((float)rand() / RAND_MAX) * winSize.width;
		float y = ((float)rand() / RAND_MAX) * winSize.height;
		sprite->setPosition(x, y);
		//		this->getCanvas()->addChild(sprite);
		this->getScene()->addChild(sprite);
		this->getLogList()->addObject(cocos2d::__String::create("RECIEVED BINARY ----------------"));
		this->getLogList()->addObject(cocos2d::__String::createWithFormat("DATA SIZE:%d bytes", len));
		this->getLogList()->addObject(cocos2d::__String::createWithFormat(
			"SPRITE(x,y,w,h):%d,%d,%d,%d",
			(int)x, (int)y,
			(int)sprite->getContentSize().width, (int)sprite->getContentSize().height
		));
#endif
	};
	ws->onErrorOccurred = [&](const cocos2d::network::WebSocket::ErrorCode &error) {
#if defined(AGTK_DEBUG)
		agtk::DebugManager::getInstance()->getLogList()->addObject(cocos2d::__String::createWithFormat("Error: %d", error));
#endif
	};
	CC_ASSERT(hostname);
	CC_ASSERT(port > 0);
	this->getWebSocket()->connect(hostname, port);
}

void GameManager::removeWebsocket()
{
	if (!this->getWebSocket()) {
		return;
	}
	this->getWebSocket()->disconnect();
	this->setWebSocket(nullptr);
}

bool GameManager::sendMessage(const std::string msg)
{
	if (!this->getWebSocket()) {
		return false;
	}
	return this->getWebSocket()->send(msg);
}

bool GameManager::sendBinaryMessage(unsigned char *data, unsigned int len)
{
	if (!this->getWebSocket()) {
		return false;
	}
	return this->getWebSocket()->send(data, len);
}
#endif

#ifdef USE_PREVIEW
//--------------------------------------------------------------------------------------------------------------------
// コマンドを実行する
//--------------------------------------------------------------------------------------------------------------------
void GameManager::runCommand(const rapidjson::Value &json)
{
	auto len = json.Size();
	for (rapidjson::SizeType i = 0; i < len; i++) {
		//rapidjson::SizeType memberCount = json[i].MemberCount();
		auto &commandObj = json[i];
		auto it = commandObj.MemberBegin();
		while (it != commandObj.MemberEnd()) {
			auto command = cocos2d::String::create(it->name.GetString());
			auto &params = it->value;
			const char *p = command->getCString();
			if (strcmp(p, "setSharedMemory") == 0) {
				auto name = params.GetString();
				agtk::SharedMemory::instance()->setKey(name);
			}
			else if (strcmp(p, "loadProject") == 0) {
				agtk::RestoreScreen();
				stopProjectData();
				this->setPlayData(nullptr);
				auto projectFilePath = params.GetString();
				loadProject(projectFilePath);
			}
			else if (strcmp(p, "startScene") == 0 && this->getProjectData()) {
				agtk::RestoreScreen();
				auto sceneId = params.GetInt();
				startScene(sceneId);
			}
			else if (strcmp(p, "restartScene") == 0) {
				restartScene();
			}
			else if (strcmp(p, "setPreviewMode") == 0) {
				//プレビューモードを設定する
				auto previewMode = params.GetString();
#if 0
				"<プレビュータイプ>: ""scene""
					汎用的なデバッグ表示・デバッグ機能が使用可能になる。
					キャプチャー機能はOFFになる。

					<プレビュータイプ> : ""particle""
					汎用的なデバッグ表示・デバッグ機能が使用不能になる。
					キャプチャー機能がONになる。

					<プレビュータイプ> : ""object""
					汎用的なデバッグ表示・デバッグ機能が使用不能になる。
					アクションの切り替わり情報がエディターに通知されるようになる。"
#endif
					this->setPreviewMode(cocos2d::String::create(previewMode));
				auto message = cocos2d::String::createWithFormat("system feedbackInfo { \"previewMode\": \"%s\" }", previewMode);
				auto ws = this->getWebSocket();
				if (ws) {
					ws->send(message->getCString());
				}
			}
			else if (strcmp(p, "bye") == 0) {
				Director::getInstance()->end();
			}
			else if (strcmp(p, "setPause") == 0) {
				auto paused = params.GetBool();
				auto scene = GameManager::getInstance()->getCurrentScene();
				if (scene) {
					scene->getGameSpeed()->setPaused(paused);
				}
			}
			else if (strcmp(p, "setFullScreen") == 0) {
				agtk::RestoreScreen();
				auto fullScreen = params.GetBool();
				if (this->getProjectData()) {
					agtk::ChangeScreen(fullScreen);
				}
			}
			else if (strcmp(p, "focusWindow") == 0) {
				agtk::RestoreScreen();
				agtk::FocusWindow();
			}
			else {
				CCLOG("Unsupported command: %s", p);
			}
			it++;
		}
	}
}

//--------------------------------------------------------------------------------------------------------------------
// Objectコマンドを実行する
//--------------------------------------------------------------------------------------------------------------------
void GameManager::runObjectCommand(const rapidjson::Value &json)
{
	auto len = json.Size();
	for (rapidjson::SizeType i = 0; i < len; i++) {
		auto &commandObj = json[i];
		auto it = commandObj.MemberBegin();
		while (it != commandObj.MemberEnd()) {
			auto command = cocos2d::String::create(it->name.GetString());
			auto &params = it->value;
			const char *p = command->getCString();
			if (strcmp(p, "connect") == 0) {
				int objectId = -1;
				if (params.HasMember("objectId")) {
					objectId = params["objectId"].GetInt();
				}
				int actionId = -1;
				if (params.HasMember("actionId")) {
					actionId = params["actionId"].GetInt();
				}
				int layerId = -1;
				auto scene = GameManager::getInstance()->getCurrentScene();
				//オブジェクトを生成するレイヤーを決定。(最初のレイヤー）
				agtk::SceneLayer *sceneLayer = nullptr;
				{
					cocos2d::DictElement *el = nullptr;
					auto sceneLayerList = scene->getSceneLayerList();
					CCDICT_FOREACH(sceneLayerList, el) {
						auto sl = dynamic_cast<agtk::SceneLayer *>(el->getObject());
						if (sl != nullptr) {
							sceneLayer = sl;
						}
					}
				}
				CCASSERT(sceneLayer, "SceneLayer not found");
				layerId = sceneLayer->getLayerId();
				auto camera = scene->getCamera();
				auto pos = camera->getPosition();
				pos = agtk::Scene::getPositionSceneFromCocos2d(pos, scene->getSceneData());
				if (scene->getPreviewObjectId() >= 0 && scene->getPreviewObjectId() != objectId) {
				}
				if (objectId >= 0) {
					//シーンに同じオブジェクトIDでかつカメラターゲットの場合は破棄する。
					auto objectList = scene->getObjectAll(objectId);
					cocos2d::Ref *ref;
					CCARRAY_FOREACH(objectList, ref) {
						auto obj = dynamic_cast<agtk::Object *>(ref);
						if (obj != camera->getTargetObject()) continue;
						if (obj->getObjectData()->getId() == objectId) {
							obj->removeSelf();
						}
					}
					int instanceId = agtk::ObjectAction::callObjectCreate(objectId, actionId, layerId, pos);
					scene->setPreviewObjectId(objectId);
					scene->setPreviewInstanceId(instanceId);

					auto message = cocos2d::String::createWithFormat("object feedbackInstanceInfo { \"instanceId\": %d, \"objectId\": %d }", instanceId, objectId);
					auto ws = this->getWebSocket();
					if (ws) {
						ws->send(message->getCString());
					}
				}
			}
			else if (strcmp(p, "remove") == 0) {
				auto scene = GameManager::getInstance()->getCurrentScene();
				int objectId = scene->getPreviewObjectId();
				int instanceId = scene->getPreviewInstanceId();
				if (params.HasMember("objectId")) {
					objectId = params["objectId"].GetInt();
				}
				if (params.HasMember("instanceId")) {
					instanceId = params["instanceId"].GetInt();
				}
				if (instanceId >= 0) {
					agtk::Object *object = scene->getObjectInstance(objectId, instanceId);
					if (object) {
						object->removeSelf();
					}
				}
			}
			else if (strcmp(p, "setAction") == 0) {
				int objectId = -1;
				int instanceId = -1;
				int actionId = -1;
				if (params.HasMember("objectId")) {
					objectId = params["objectId"].GetInt();
				}
				if (params.HasMember("instanceId")) {
					instanceId = params["instanceId"].GetInt();
				}
				if (params.HasMember("actionId")) {
					actionId = params["actionId"].GetInt();
				}
				if (instanceId >= 0 && actionId >= 0) {
					auto scene = GameManager::getInstance()->getCurrentScene();
					agtk::Object *object = scene->getObjectInstance(objectId, instanceId);
					if (object) {
						int directionId = object->getDispDirection();
						object->playAction(actionId, directionId);
					}
				}
			}
			else if (strcmp(p, "execScript") == 0) {
				if (params.HasMember("script")) {
					auto script = params["script"].GetString();
					int objectId = -1;
					int instanceId = -1;
					if (params.HasMember("objectId")) {
						objectId = params["objectId"].GetInt();
					}
					if (params.HasMember("instanceId")) {
						instanceId = params["instanceId"].GetInt();
					}
					if (strlen(script) > 0) {
						auto scriptingCore = ScriptingCore::getInstance();
						auto context = scriptingCore->getGlobalContext();
						JS::RootedValue rv(context);
						JS::MutableHandleValue mhv(&rv);
						auto js = String::createWithFormat("(function(){ var objectId = %d; var instanceId = %d; %s\n})()", objectId, instanceId, script);
						auto ret = ScriptingCore::getInstance()->evalString(js->getCString(), mhv);
						if (!ret) {
							//スクリプトエラー
							auto errorStr = String::createWithFormat("Runtime error in execScript(objectId: %d, instanceId: %d, script: %s).", objectId, instanceId, script)->getCString();
							agtk::DebugManager::getInstance()->getDebugExecuteLogWindow()->addLog(errorStr);
#if defined(USE_PREVIEW)
							auto fp = GameManager::getScriptLogFp();
							if (fp) {
								fwrite(errorStr, 1, strlen(errorStr), fp);
								fwrite("\n", 1, 1, fp);
							}
#endif
						}
					}
				}
			}
			else if (strcmp(p, "setJson") == 0) {
				int objectId = -1;
				if (params.HasMember("objectId") && params.HasMember("json")) {
					objectId = params["objectId"].GetInt();
					auto &json = params["json"];
					auto projectData = this->getProjectData();
					projectData->setObjectData(objectId, json);
					auto message = cocos2d::String::createWithFormat("object feedbackSetJson { \"objectId\": %d }", objectId);
					auto ws = this->getWebSocket();
					if (ws) {
						ws->send(message->getCString());
					}
				}
			}
			else {
				CCLOG("Unsupported command: %s", p);
			}
			it++;
		}
	}
}

//--------------------------------------------------------------------------------------------------------------------
// Sceneコマンドを実行する
//--------------------------------------------------------------------------------------------------------------------
void GameManager::runSceneCommand(const rapidjson::Value &json)
{
	auto len = json.Size();
	for (rapidjson::SizeType i = 0; i < len; i++) {
		auto &commandObj = json[i];
		auto it = commandObj.MemberBegin();
		while (it != commandObj.MemberEnd()) {
			auto command = cocos2d::String::create(it->name.GetString());
			auto &params = it->value;
			const char *p = command->getCString();
			if (strcmp(p, "setJson") == 0) {
				int sceneId = -1;
				if (params.HasMember("sceneId") && params.HasMember("json")) {
					sceneId = params["sceneId"].GetInt();
					auto &json = params["json"];
					auto projectData = this->getProjectData();
					projectData->setSceneData(sceneId, json);
					auto message = cocos2d::String::createWithFormat("scene feedbackSetJson { \"sceneId\": %d }", sceneId);
					auto ws = this->getWebSocket();
					if (ws) {
						ws->send(message->getCString());
					}
				}
			}
			else if (strcmp(p, "watchPhysics") == 0) {
				auto scene = this->getCurrentScene();
				if (scene) {
					auto idList = cocos2d::Array::create();
					auto &arr = params["scenePartIdList"];
					auto len2 = arr.Size();
					for (rapidjson::SizeType j = 0; j < len2; j++) {
						auto scenePartId = arr[j].GetInt();
						auto data = cocos2d::Integer::create(scenePartId);
						idList->addObject(data);
					}
					scene->setWatchPhysicsPartIdList(idList);
				}
			}
			else if (strcmp(p, "requestPhysics") == 0) {
				auto scene = this->getCurrentScene();
				if (scene) {
					scene->setPhysicsRequested(true);
				}
			}
			else {
				CCLOG("Unsupported command: %s", p);
			}
			it++;
		}
	}
}

//--------------------------------------------------------------------------------------------------------------------
// Tilesetコマンドを実行する
//--------------------------------------------------------------------------------------------------------------------
void GameManager::runTilesetCommand(const rapidjson::Value &json)
{
	auto len = json.Size();
	for (rapidjson::SizeType i = 0; i < len; i++) {
		auto &commandObj = json[i];
		auto it = commandObj.MemberBegin();
		while (it != commandObj.MemberEnd()) {
			auto command = cocos2d::String::create(it->name.GetString());
			auto &params = it->value;
			const char *p = command->getCString();
			if (strcmp(p, "setJson") == 0) {
				int tilesetId = -1;
				if (params.HasMember("tilesetId") && params.HasMember("json")) {
					tilesetId = params["tilesetId"].GetInt();
					auto &json = params["json"];
					auto projectData = this->getProjectData();
					projectData->setTilesetData(tilesetId, json);
					auto message = cocos2d::String::createWithFormat("tileset feedbackSetJson { \"tilesetId\": %d }", tilesetId);
					auto ws = this->getWebSocket();
					if (ws) {
						ws->send(message->getCString());
					}
				}
			}
			else {
				CCLOG("Unsupported command: %s", p);
			}
			it++;
		}
	}
}

//--------------------------------------------------------------------------------------------------------------------
// Imageコマンドを実行する
//--------------------------------------------------------------------------------------------------------------------
void GameManager::runImageCommand(const rapidjson::Value &json)
{
	auto len = json.Size();
	for (rapidjson::SizeType i = 0; i < len; i++) {
		auto &commandObj = json[i];
		auto it = commandObj.MemberBegin();
		while (it != commandObj.MemberEnd()) {
			auto command = cocos2d::String::create(it->name.GetString());
			auto &params = it->value;
			const char *p = command->getCString();
			if (strcmp(p, "setJson") == 0) {
				int imageId = -1;
				if (params.HasMember("imageId") && params.HasMember("json")) {
					imageId = params["imageId"].GetInt();
					auto &json = params["json"];
					auto projectData = this->getProjectData();
					projectData->setImageData(imageId, json);
					auto message = cocos2d::String::createWithFormat("image feedbackSetJson { \"imageId\": %d }", imageId);
					auto ws = this->getWebSocket();
					if (ws) {
						ws->send(message->getCString());
					}
				}
			}
			else {
				CCLOG("Unsupported command: %s", p);
			}
			it++;
		}
	}
}

//--------------------------------------------------------------------------------------------------------------------
// Textコマンドを実行する
//--------------------------------------------------------------------------------------------------------------------
void GameManager::runTextCommand(const rapidjson::Value &json)
{
	auto len = json.Size();
	for (rapidjson::SizeType i = 0; i < len; i++) {
		auto &commandObj = json[i];
		auto it = commandObj.MemberBegin();
		while (it != commandObj.MemberEnd()) {
			auto command = cocos2d::String::create(it->name.GetString());
			auto &params = it->value;
			const char *p = command->getCString();
			if (strcmp(p, "setJson") == 0) {
				int textId = -1;
				if (params.HasMember("textId") && params.HasMember("json")) {
					textId = params["textId"].GetInt();
					auto &json = params["json"];
					auto projectData = this->getProjectData();
					projectData->setTextData(textId, json);
					auto message = cocos2d::String::createWithFormat("text feedbackSetJson { \"textId\": %d }", textId);
					auto ws = this->getWebSocket();
					if (ws) {
						ws->send(message->getCString());
					}
				}
			}
			else {
				CCLOG("Unsupported command: %s", p);
			}
			it++;
		}
	}
}

#endif

const char *GameManager::tr(const char *sourceText, const char *disambiguation)
{
	if (_translationMap.size() == 0) {
		static const char *sTable[] = {
			"",
			"Reset to Default", "ja_JP", "初期設定に戻す", "zh_CN", "重置", nullptr,
			"OK", "ja_JP", "OK", "zh_CN", "确定", nullptr,
			"Cancel", "ja_JP", "キャンセル", "zh_CN", "取消", nullptr,
            "Settings", "ja_JP", "設定", "zh_CN", "设置", nullptr,
			"Debugging", "ja_JP", "デバッグ機能", "zh_CN", "调试功能", nullptr,
            "Contorols", "ja_JP", "操作方法", "zh_CN", "操作键", nullptr,
            "Game Screen", "ja_JP", "ゲーム画面", "zh_CN", "游戏屏幕", nullptr,
            "WebSocket", "ja_JP", "WebSocket", "zh_CN", "WebSocket", nullptr,
            "Pause", "ja_JP", "一時停止", "zh_CN", "暂停", nullptr,
            "Show TileWall", "ja_JP", "タイルの壁判定を表示", "zh_CN", "显示图块墙", nullptr,
            "Show Wall Detection", "ja_JP", "壁判定を表示", "zh_CN", "显示墙面区域", nullptr,
            "Show Collision Detection", "ja_JP", "当たり判定を表示", "zh_CN", "显示击中区域", nullptr,
            "Show Attack Detection", "ja_JP", "攻撃判定を表示", "zh_CN", "显示攻击区域", nullptr,
            "Show Connection Point", "ja_JP", "接続点を表示", "zh_CN", "显示连接点", nullptr,
            "Show Runtime Log Console", "ja_JP", "実行ログコンソール表示", "zh_CN", "显示执行日志", nullptr,
			"Action Log Console", "ja_JP", "実行ログコンソール", "zh_CN", "启动日志控制台", nullptr,
            "Free Movement Mode", "ja_JP", "自由移動モード", "zh_CN", "自由移动模式", nullptr,
            "Invincible Mode", "ja_JP", "無敵モード", "zh_CN", "无敌模式", nullptr,
            "Frame Rate", "ja_JP", "フレームレート", "zh_CN", "帧率", nullptr,
			"Performance/Speed Settings", "ja_JP", "ゲーム動作を変更", "zh_CN", "变更游戏的运算", nullptr,
            "Show Debugging for Development", "ja_JP", "開発用デバッグ表示", "zh_CN", "显示开发调试", nullptr,
            "Show physics debug", "ja_JP", "物理演算用デバッグ表示", "zh_CN", "显示物理调试", nullptr,
            "Show Debugging for Other Parts", "ja_JP", "その他パーツ用デバッグ表示", "zh_CN", "显示其他调试", nullptr,
            "Show portal debug", "ja_JP", "ポータルデバッグ表示", "zh_CN", "显示端口调试", nullptr,
            "Show Debugging for Player Move Range Restriction", "ja_JP", "プレイヤーの行動範囲制限デバッグ表示", "zh_CN", "显示玩家的可移动区域调试", nullptr,
            "Show Debugging for Camera Range Restriction", "ja_JP", "カメラの表示範囲制限デバッグ表示", "zh_CN", "显示针对相机的显示区域调试", nullptr,
            "Disable Skip During Scene Generation", "ja_JP", "シーン生成時のスキップ無効", "zh_CN", "在创建场景中禁用跳帧", nullptr,
            "Fixed FPS", "ja_JP", "FPS固定", "zh_CN", "修复每秒帧速", nullptr,
            "Change Scene", "ja_JP", "シーン切り替え", "zh_CN", "改变场景", nullptr,
            "Loading Scene", "ja_JP", "ロード画面", "zh_CN", "正在加载场景", nullptr,
            "Game Data", "ja_JP", "ゲーム情報", "zh_CN", "游戏信息", nullptr,
            "Show Simple Draw Load", "ja_JP", "簡易描画負荷表示", "zh_CN", "显示简易拉深力", nullptr,
            "Object Data", "ja_JP", "オブジェクト情報", "zh_CN", "对象信息", nullptr,
			"Object Information", "ja_JP", "オブジェクト情報", "zh_CN", "对象情报", nullptr,
			"Input Search Text", "ja_JP", "検索するテキストを入力", "zh_CN", "输入搜索文本", nullptr,
			"Normal Scene", "ja_JP", "通常シーン", "zh_CN", "普通场景", nullptr,
			"Menu Scene", "ja_JP", "メニューシーン", "zh_CN", "菜单场景", nullptr,
			"Object Count: %d", "ja_JP", "オブジェクト数: %d", "zh_CN", "对象数量: %d", nullptr,
			"Name: %s", "ja_JP", "名前: %s", "zh_CN", "名称: %s", nullptr, 
			"Object Name: %s", "ja_JP", "オブジェクト名: %s", "zh_CN", "对象名称: %s", nullptr,
			"Action Name: %s", "ja_JP", "アクション名: %s", "zh_CN", "动作名称: %s", nullptr,
			"Position: (%4.2f, %4.2f)", "ja_JP", "座標: (%4.2f, %4.2f)", "zh_CN", "坐标: (%4.2f, %4.2f)", nullptr,
			"Layer: %s", "ja_JP", "レイヤー: %s", "zh_CN", "层级: %s", nullptr,
			"Instance ID: %d", "ja_JP", "インスタンスID: %d", "zh_CN", "实例ID: %d", nullptr,
			"Switch", "ja_JP", "スイッチ", "zh_CN", "开关", nullptr,
			"Variable", "ja_JP", "変数", "zh_CN", "变量", nullptr,
            "Data of Common Variables and Switches", "ja_JP", "共通変数とスイッチ情報", "zh_CN", "公共资源", nullptr,
            "Show Grid", "ja_JP", "グリッド表示", "zh_CN", "显示网格", nullptr,
            "Iteration Count of Physics Calculation", "ja_JP", "物理演算のイテレーション回数", "zh_CN", "物理计算的重复计数", nullptr,
            "Reload Project Data", "ja_JP", "プロジェクトデータ リロード", "zh_CN", "重新加载项目数据", nullptr,
            "Operation settings", "ja_JP", "操作設定", "zh_CN", "操作设置", nullptr,
            "Control Method Settings", "ja_JP", "操作方法の設定", "zh_CN", "操作输入设置", nullptr,
            "Select Input Device", "ja_JP", "入力を受け付けている機器を選択", "zh_CN", "选择输入设备", nullptr,
            "Keyboard/mouse", "ja_JP", "キーボード・マウス", "zh_CN", "键盘/鼠标", nullptr,
            "Key settings", "ja_JP", "キー設定", "zh_CN", "按键设置", nullptr,
            "Select a template", "ja_JP", "テンプレートから選択", "zh_CN", "选择模版", nullptr,
            "Controller input", "ja_JP", "コントローラー入力", "zh_CN", "控制器输入", nullptr,
            "Key input", "ja_JP", "キー入力", "zh_CN", "按键输入", nullptr,
            "Press a controller button...", "ja_JP", "コントローラーのボタンを押してください...", "zh_CN", "按一个控制器按钮…", nullptr,
            "Press a key...", "ja_JP", "キーを押してください...", "zh_CN", "按一个按键…", nullptr,
            "None", "ja_JP", "設定なし", "zh_CN", "无", nullptr,
            "Click to reassign", "ja_JP", "クリックで再設定できます", "zh_CN", "点击重新分配", nullptr,
            "Press a key to reassign", "ja_JP", "キー入力で再設定できます", "zh_CN", "按一个按键重新分配", nullptr,
			"No input, so input something.", "ja_JP", "入力がありません。\nなにか入力してください。", "zh_CN", "无输入，请输入", nullptr,
            "Game screen settings", "ja_JP", "ゲーム画面の設定", "zh_CN", "游戏屏幕设置", nullptr,
            "Window", "ja_JP", "ウインドウ表示", "zh_CN", "窗口", nullptr,
            "Full screen", "ja_JP", "フルスクリーン表示", "zh_CN", "全屏", nullptr,
            "Magnify", "ja_JP", "拡大表示", "zh_CN", "放大", nullptr,
			"Normal", "ja_JP", "通常", "zh_CN", "常规", nullptr,
			"x2", "ja_JP", "２倍", "zh_CN", "x2", nullptr,
			"x3", "ja_JP", "３倍", "zh_CN", "x3", nullptr,
            "Screen effect", "ja_JP", "画面エフェクト", "zh_CN", "屏幕效果", nullptr,
			"Retro game machine", "ja_JP", "レトロゲーム機", "zh_CN", "Retro游戏机", nullptr,
			"Defocus", "ja_JP", "ぼかし", "zh_CN", "散焦", nullptr,
			"Analog TV", "ja_JP", "アナログTV", "zh_CN", "模拟电视", nullptr,
            "Layer screen:", "ja_JP", "レイヤー画面：", "zh_CN", "屏幕图层：", nullptr,
            "All", "ja_JP", "全て", "zh_CN", "全部", nullptr,
            "Background", "ja_JP", "背景", "zh_CN", "背景", nullptr,
            "Language settings", "ja_JP", "言語設定", "zh_CN", "语言设置", nullptr,
            "Language:", "ja_JP", "言語：", "zh_CN", "语言：", nullptr,
            "Title: %s", "ja_JP", "タイトル：%s", "zh_CN", "标题：%s", nullptr,
            "Creator: %s", "ja_JP", "作成者：%s", "zh_CN", "作者：%s", nullptr,
            "Genre: %s", "ja_JP", "ジャンル：%s", "zh_CN", "类型: %s", nullptr,
            "Content: %s", "ja_JP", "内容：%s", "zh_CN", "描述：%s", nullptr,
            "Scene Selection", "ja_JP", "シーン選択", "zh_CN", "场景选择", nullptr,
            "Movie test", "ja_JP", "ムービーテスト", "zh_CN", "影视测试", nullptr,
			"Play", "ja_JP", "再生", "zh_CN", "播放", nullptr,
			"Stop", "ja_JP", "停止", "zh_CN", "停止", nullptr,
			"Pause", "ja_JP", "一時停止", "zh_CN", "暂停", nullptr,
			"Details", "ja_JP", "詳細", "zh_CN", "详情", nullptr,
			"Sound", "ja_JP", "サウンド", "zh_CN", "声音", nullptr,
			"BGM Volume: ", "ja_JP", "BGMの音量：", "zh_CN", "背景音乐音量：", nullptr,
			"SE Volume: ", "ja_JP", "SEの音量：", "zh_CN", "音效音量", nullptr,
			"Voice Volume: ", "ja_JP", "音声の音量：", "zh_CN", "语音音量", nullptr,
            "A", "ja_JP", "A", "zh_CN", "A", nullptr,
            "B", "ja_JP", "B", "zh_CN", "B", nullptr,
            "X", "ja_JP", "X", "zh_CN", "X", nullptr,
            "Y", "ja_JP", "Y", "zh_CN", "Y", nullptr,
#if (CC_TARGET_PLATFORM == CC_PLATFORM_NX)
#endif
            "R1", "ja_JP", "R1", "zh_CN", "R1", nullptr,
            "R2", "ja_JP", "R2", "zh_CN", "R2", nullptr,
            "L1", "ja_JP", "L1", "zh_CN", "L1", nullptr,
            "L2", "ja_JP", "L2", "zh_CN", "L2", nullptr,
            "Up", "ja_JP", "↑", "zh_CN", "向上", nullptr,
            "Down", "ja_JP", "↓", "zh_CN", "向下", nullptr,
            "Left", "ja_JP", "←", "zh_CN", "向左", nullptr,
            "Right", "ja_JP", "→", "zh_CN", "向右", nullptr,
#if (CC_TARGET_PLATFORM == CC_PLATFORM_NX)
#endif
			"Left stick(Up)", "ja_JP", "左スティック（↑）", "zh_CN", "左摇杆（向上）", nullptr,
			"Left stick(Down)", "ja_JP", "左スティック（↓）", "zh_CN", "左摇杆（向下）", nullptr,
			"Left stick(Left)", "ja_JP", "左スティック（←）", "zh_CN", "左摇杆（向左）", nullptr,
			"Left stick(Right)", "ja_JP", "左スティック（→）", "zh_CN", "左摇杆（向右）", nullptr,
			"Right stick(Up)", "ja_JP", "右スティック（↑）", "zh_CN", "右摇杆（向上）", nullptr,
			"Right stick(Down)", "ja_JP", "右スティック（↓）", "zh_CN", "右摇杆（向下）", nullptr,
			"Right stick(Left)", "ja_JP", "右スティック（←）", "zh_CN", "右摇杆（向左）", nullptr,
			"Right stick(Right)", "ja_JP", "右スティック（→）", "zh_CN", "右摇杆（向右）", nullptr,
            "CANCEL", "ja_JP", "CANCEL", "zh_CN", "取消", nullptr,
            "Left click", "ja_JP", "左クリック", "zh_CN", "左键点击", nullptr,
            "Right click", "ja_JP", "右クリック", "zh_CN", "右键点击", nullptr,
            "Middle click", "ja_JP", "中クリック", "zh_CN", "中键点击", nullptr,
            "Wheel(Up)", "ja_JP", "ホイール（↑）", "zh_CN", "滚轮（向上）", nullptr,
            "Wheel(Down)", "ja_JP", "ホイール（↓）", "zh_CN", "滚轮（向下）", nullptr,
#if (CC_TARGET_PLATFORM == CC_PLATFORM_NX)
#endif
			"Left stick(Press)", "ja_JP", "左スティック（押し込み）", "zh_CN", "左摇杆（按）", nullptr,
            "Right stick(Press)", "ja_JP", "右スティック（押し込み）", "zh_CN", "右摇杆（按）", nullptr,
            "Touch pad(Press)", "ja_JP", "タッチパッド（押し込み）", "zh_CN", "触摸板（按）", nullptr,
#if (CC_TARGET_PLATFORM == CC_PLATFORM_NX)
#endif
			"Left stick(X)", "ja_JP", "左スティック（X）", "zh_CN", "左摇杆（X）", nullptr,
            "Left stick(Y)", "ja_JP", "左スティック（Y）", "zh_CN", "左摇杆（Y）", nullptr,
            "Right stick(X)", "ja_JP", "右スティック（X）", "zh_CN", "右摇杆（X）", nullptr,
            "Right stick(Y)", "ja_JP", "右スティック（Y）", "zh_CN", "右摇杆（Y）", nullptr,
            "Square", "ja_JP", "□", "zh_CN", "正方形", nullptr,
            "Cross", "ja_JP", "×", "zh_CN", "十字形", nullptr,
            "Circle", "ja_JP", "○", "zh_CN", "圆形", nullptr,
            "Triangle", "ja_JP", "△", "zh_CN", "三角形", nullptr,
            "Mouse", "ja_JP", "マウス", "zh_CN", "鼠标", nullptr,
            "", "ja_JP", "", "zh_CN", "", nullptr,
            "", "ja_JP", "", "zh_CN", "", nullptr,
            "", "ja_JP", "", "zh_CN", "", nullptr,
            "", "ja_JP", "", "zh_CN", "", nullptr,
            "", "ja_JP", "", "zh_CN", "", nullptr,
            "Debug", "ja_JP", "デバッグ", "zh_CN", "调试", nullptr,
            "Set Move Direction and Move", "ja_JP", "移動方向を指定して移動", "zh_CN", "设置移动方向并移动", nullptr,
            "Move Towards Display Direction", "ja_JP", "表示方向と同じ方へ移動", "zh_CN", "向前移动显示方向", nullptr,
            "Back and Forth Moving and Turning", "ja_JP", "前後移動と旋回", "zh_CN", "来回移动和转向", nullptr,
            "Template Move Settings", "ja_JP", "テンプレート移動の設定", "zh_CN", "模板移动设置", nullptr,
            "Lock Object", "ja_JP", "オブジェクトをロック", "zh_CN", "锁定对象", nullptr,
            "Generate Object", "ja_JP", "オブジェクトを生成", "zh_CN", "生成对象", nullptr,
            "Change Object", "ja_JP", "オブジェクトを変更", "zh_CN", "改变对象", nullptr,
            "Move Object", "ja_JP", "オブジェクトを移動させる", "zh_CN", "移动对象", nullptr,
            "Push/Pull Object", "ja_JP", "オブジェクトを押す・引く", "zh_CN", "推拉对象", nullptr,
            "Execute Object Action", "ja_JP", "オブジェクトのアクションを実行", "zh_CN", "执行对象动作", nullptr,
            "Move Layer", "ja_JP", "レイヤーを移動", "zh_CN", "移动图层", nullptr,
            "Attack Settings", "ja_JP", "攻撃の設定", "zh_CN", "攻击设置", nullptr,
            "Fire Bullet", "ja_JP", "弾を発射", "zh_CN", "发射子弹", nullptr,
            "Destroy Object", "ja_JP", "オブジェクトを消滅する", "zh_CN", "摧毁对象", nullptr,
            "Restore Destroyed Object", "ja_JP", "消滅状態のオブジェクトを復活させる", "zh_CN", "恢复已摧毁对象", nullptr,
            "Disable Object", "ja_JP", "オブジェクトを無効にする", "zh_CN", "禁用对象", nullptr,
            "Enable Disabled Object", "ja_JP", "無効状態のオブジェクトを有効にする", "zh_CN", "启用已禁用对象", nullptr,
            "Change Switch/Variable", "ja_JP", "スイッチ・変数を変更", "zh_CN", "改变开关/变量", nullptr,
            "Reset Switch/Variable", "ja_JP", "スイッチ・変数を初期値に戻す", "zh_CN", "重置开关/变量", nullptr,
            "Apply Filter Effects on Object", "ja_JP", "オブジェクトにフィルター効果を設定", "zh_CN", "对对象应用过滤效果", nullptr,
            "Delete Filter Effects from Object", "ja_JP", "オブジェクトのフィルター効果を削除", "zh_CN", "删除对象的过滤效果", nullptr,
            "Apply Screen Effects on Scene", "ja_JP", "シーンに画面効果を設定", "zh_CN", "对场景应用筛选效果", nullptr,
            "Delete Screen Effects from Scene", "ja_JP", "シーンの画面効果を削除", "zh_CN", "删除场景的筛选效果", nullptr,
            "Change Scene Gravity Effects", "ja_JP", "シーンの重力効果を変更する", "zh_CN", "改变场景引力效果", nullptr,
            "Rotate/Flip Scene", "ja_JP", "シーンを回転・反転", "zh_CN", "旋转/翻转场景", nullptr,
            "Shake Scene", "ja_JP", "シーンを揺らす", "zh_CN", "晃动场景", nullptr,
            "Show Effect", "ja_JP", "エフェクトを表示", "zh_CN", "显示效果", nullptr,
            "Hide Effects", "ja_JP", "エフェクトを非表示", "zh_CN", "隐藏效果", nullptr,
            "Show Particles", "ja_JP", "パーティクルを表示", "zh_CN", "显示粒子", nullptr,
            "Hide Particles", "ja_JP", "パーティクルを非表示", "zh_CN", "隐藏粒子", nullptr,
            "Disable Layer Display", "ja_JP", "レイヤーの表示をOFF", "zh_CN", "禁用图层显示", nullptr,
            "Enable Layer Display", "ja_JP", "レイヤーの表示をON", "zh_CN", "启用图层显示", nullptr,
            "Disable Layer Motion", "ja_JP", "レイヤーの動作をOFF", "zh_CN", "禁用图层运动", nullptr,
            "Enable Layer Motion", "ja_JP", "レイヤーの動作をON", "zh_CN", "禁用图层运动", nullptr,
            "Change Camera Display Area", "ja_JP", "カメラの表示領域を変更する", "zh_CN", "改变相机显示区域", nullptr,
            "Audio Playback", "ja_JP", "音の再生", "zh_CN", "音频回放", nullptr,
            "Stop Audio Item", "ja_JP", "音の停止", "zh_CN", "停止音频项", nullptr,
            "Play Video", "ja_JP", "動画を再生", "zh_CN", "播放视频", nullptr,
            "Display Image", "ja_JP", "画像を表示", "zh_CN", "显示图像", nullptr,
            "Show Text", "ja_JP", "テキストを表示", "zh_CN", "显示文字", nullptr,
            "Show Scrolling Text", "ja_JP", "テキストをスクロール表示", "zh_CN", "显示滚动文字", nullptr,
            "Change Game Speed", "ja_JP", "ゲームスピードを変更する", "zh_CN", "改变游戏速度", nullptr,
            "Wait", "ja_JP", "ウェイトを入れる", "zh_CN", "等待", nullptr,
            "Timer Function", "ja_JP", "タイマー機能", "zh_CN", "计时器功能", nullptr,
            "End Scene", "ja_JP", "このシーンを終了", "zh_CN", "结束场景", nullptr,
            "Execute Script", "ja_JP", "スクリプトを実行", "zh_CN", "执行脚本", nullptr,
            "Show Menu Screen", "ja_JP", "メニュー画面を表示", "zh_CN", "显示菜单屏幕", nullptr,
            "Hide Menu Screen", "ja_JP", "メニュー画面を非表示", "zh_CN", "隐藏菜单屏幕", nullptr,
			"Load a File", "ja_JP", "ファイルをロード", "zh_CN", "加载文件",nullptr,
			"Save Playhead Start Time", "ja_JP", "音の再生位置を保存", "zh_CN", "保存音频播放位置",nullptr,
            "Release Lock", "ja_JP", "ロックを解除", "zh_CN", "解除锁定", nullptr,
			"Change Animation Set", "ja_JP", "アニメーションの素材セットを変更", "zh_CN", "变更动画素材集",nullptr,
			"Close", "ja_JP", "閉じる", "zh_CN", "关闭", nullptr,
			"Disable Screen Shake", "ja_JP", "画面振動を無効", "zh_CN", "取消画面震动特效", nullptr,
			"Start", "ja_JP", "開始", "zh_CN", "开始" , nullptr,
			"Enlarge the screen", "ja_JP", "拡大表示", "zh_CN", "放大", nullptr,
			"Screen scale", "ja_JP", "表示倍率", "zh_CN", "放大倍率", nullptr,
			"Fullscreen", "ja_JP", "フルスクリーン表示", "zh_CN", "全屏表示", nullptr,
			"Operation Mode: %s", "ja_JP", "操作モード: %s", "zh_CN", "操作模式: %s", nullptr,
			"Save Data Management", "ja_JP", "セーブデータ管理", "zh_CN", "保存数据管理", nullptr,
			"Empty", "ja_JP", "空き", "zh_CN", "虚空", nullptr,
			"Save to Slot %d. Are you sure?", "ja_JP", "Slot %d にセーブします。本当によろしいですか？", "zh_CN", "保存到Slot %d。你确定吗？", nullptr,
			"Load from Slot %d. Are you sure?", "ja_JP", "Slot %d からロードします。本当によろしいですか？", "zh_CN", "从Slot %d 加载。 你确定吗？", nullptr,
			"Delete Slot %d. Are you sure?", "ja_JP", "Slot %d を削除します。本当によろしいですか？", "zh_CN", "删除Slot %d。 你确定吗？", nullptr,
			nullptr,

			"Menu",
			nullptr,

			nullptr
		};
		int head = 0;
		while (true) {
			if (!sTable[head]) {
				break;
			}
			_translationMap.insert(std::make_pair(sTable[head], std::map<std::string, std::vector<LocaleText>>()));
			auto &ambList = _translationMap[sTable[head]];
			head++;
			std::vector<LocaleText> list;
			while (true) {
				if (!sTable[head]) {
					break;
				}
				list.clear();
				auto key = sTable[head++];
				while (true) {
					if (!sTable[head]) {
						break;
					}
					list.push_back(LocaleText(sTable[head], sTable[head + 1]));
					head += 2;
				}
				ambList.insert(std::make_pair(key, list));
				head++;
			}
			head++;
		}
	}
	std::map<std::string, std::vector<LocaleText>> *translation = nullptr;
	if (_translationMap.find(disambiguation) != _translationMap.end()) {
		translation = &_translationMap[disambiguation];
	}
	else {
		translation = &_translationMap[""];
	}
	if (translation->find(sourceText) == translation->end()) {
		return sourceText;
	}
	auto &list = (*translation)[sourceText];
	auto editorLocale = GameManager::getInstance()->getEditorLocale();
	for (auto &localeText : list) {
		if (localeText.locale == editorLocale){
			return localeText.text.c_str();
		}
	}
	editorLocale = editorLocale.substr(0, 2);
	for (auto &localeText : list) {
		if (localeText.locale == editorLocale) {
			return localeText.text.c_str();
		}
	}
	return sourceText;
}

void GameManager::visitScene(cocos2d::Renderer *renderer, agtk::Scene *scene, cocos2d::RenderTexture *renderTexture)
{
	CC_ASSERT(renderer);
	CC_ASSERT(scene);
	CC_ASSERT(renderTexture);

	auto camera = scene->getCamera()->getCamera();
	auto vm = camera->getViewMatrix();
	auto menuCamera = scene->getCamera()->getMenuCamera();
	auto vm_menu = menuCamera->getViewMatrix();

	renderTexture->beginWithClear(0, 0, 0, 0);
	{
		auto withMenuRenderTextureCtrl = scene->getSceneTopMost()->getWithMenuRenderTexture();
		if (withMenuRenderTextureCtrl) {
			auto topMostSprite = withMenuRenderTextureCtrl->getLastRenderTexture()->getSprite();
			topMostSprite->visit(renderer, vm, false);
		}
		else {
			auto renderTextureCtrl = scene->getSceneTopMost()->getRenderTexture();
			if (renderTextureCtrl) {
				auto topMostSprite = renderTextureCtrl->getLastRenderTexture()->getSprite();
				topMostSprite->visit(renderer, vm, false);
			}
			else {
				// 背景
				{
					auto sceneBackground = scene->getSceneBackground();
					auto renderTextureCtrl = sceneBackground->getRenderTexture();
					if (renderTextureCtrl) {
						auto bgSprite = renderTextureCtrl->getLastRenderTexture()->getSprite();
						bgSprite->visit(renderer, vm, false);
					}
					else {
						sceneBackground->visit(renderer, vm, true);
					}
				}

				// シーンレイヤー
				{
					auto dic = scene->getSceneLayerList();
					cocos2d::DictElement *el = nullptr;
					CCDICT_FOREACH(dic, el) {
						bool isRender = false;

// #AGTK-NX
#ifdef STATIC_DOWN_CAST
						auto sceneLayer = static_cast<agtk::SceneLayer *>(el->getObject());
#else
						auto sceneLayer = dynamic_cast<agtk::SceneLayer *>(el->getObject());
#endif
						auto renderTextureCtrl = sceneLayer->getRenderTexture();
						if (renderTextureCtrl) {
							if (renderTextureCtrl->isUseShader()) {
								isRender = true;
							}
						}

						if (isRender) {
							auto sprite = renderTextureCtrl->getLastRenderTexture()->getSprite();
							sprite->visit(renderer, vm, false);
						}
						else {
							sceneLayer->visit(renderer, vm, true);
						}
					}
				}
			}

			// シーンレイヤーのシェーダー使用しているシーンレイヤーを非表示にする。
			// gameManager->getCurrentLayerをvisitすると濃くなるので非表示にしておく
			std::deque<int> sceneLayerVisibleList;//シェーダー使用フラグOFF(=-1),シェーダー使用フラグON(非表示=0,表示=1)
			if (!scene->getSceneTopMost()->getRenderTexture()) {
				auto dic = scene->getSceneLayerList();
				cocos2d::DictElement *el = nullptr;
				CCDICT_FOREACH(dic, el) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
					auto sceneLayer = static_cast<agtk::SceneLayer *>(el->getObject());
#else
					auto sceneLayer = dynamic_cast<agtk::SceneLayer *>(el->getObject());
#endif
					auto renderTextureCtrl = sceneLayer->getRenderTexture();
					if (renderTextureCtrl && renderTextureCtrl->isUseShader()) {
						auto sprite = renderTextureCtrl->getLastRenderTexture()->getSprite();
						sceneLayerVisibleList.emplace_back((int)sprite->isVisible());
						sprite->setVisible(false);
					}
					else {
						sceneLayerVisibleList.emplace_back(-1);
					}
				}
			}

			// メニューシーンを非表示
			// gameManager->getCurrentLayerをvisitすると濃くなるので非表示にしておく
			std::deque<bool> menuVisibleList;
			{
				auto menuLayerList = scene->getMenuLayerList();
				cocos2d::DictElement *el;
				CCDICT_FOREACH(menuLayerList, el) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
					auto menuLayer = static_cast<agtk::SceneLayer *>(el->getObject());
#else
					auto menuLayer = dynamic_cast<agtk::SceneLayer *>(el->getObject());
#endif
					menuVisibleList.emplace_back(menuLayer->isVisible());
					menuLayer->setVisible(false);
				}
			}

			// デバッグ表示
			auto gameManager = GameManager::getInstance();
			auto layer = gameManager->getCurrentLayer();
			layer->setVisible(true);
			auto topMost = scene->getSceneTopMost();
			auto lVisible = topMost->isVisible();
			topMost->setVisible(false);
			layer->visit(renderer, vm, true);
			topMost->setVisible(lVisible);
			layer->setVisible(false);

			// シーンレイヤーのシェーダー使用しているシーンレイヤーを表示に戻す。
			if (!scene->getSceneTopMost()->getRenderTexture()) {
				int sceneLayerVisibleIndex = 0;

				auto dic = scene->getSceneLayerList();
				cocos2d::DictElement *el = nullptr;
				CCDICT_FOREACH(dic, el) {
					auto visible = sceneLayerVisibleList[sceneLayerVisibleIndex++];
					if (visible == -1) continue;
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
					auto sceneLayer = static_cast<agtk::SceneLayer *>(el->getObject());
#else
					auto sceneLayer = dynamic_cast<agtk::SceneLayer *>(el->getObject());
#endif
					auto renderTextureCtrl = sceneLayer->getRenderTexture();
					if (renderTextureCtrl != nullptr) {
						auto sprite = renderTextureCtrl->getLastRenderTexture()->getSprite();
						sprite->setVisible((bool)visible);
					}
				}
			}

			// メニューシーン表示(非表示からの戻し)
			{
				int menuVisibleIndex = 0;

				auto menuLayerList = scene->getMenuLayerList();
				cocos2d::DictElement *el;
				CCDICT_FOREACH(menuLayerList, el) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
					auto menuLayer = static_cast<agtk::SceneLayer *>(el->getObject());
#else
					auto menuLayer = dynamic_cast<agtk::SceneLayer *>(el->getObject());
#endif

					menuLayer->setVisible(menuVisibleList[menuVisibleIndex]);

					++menuVisibleIndex;
				}
			}

			// メニューシーン
			{
				auto menuLayerList = scene->getMenuLayerList();
				cocos2d::DictElement *el;
				CCDICT_FOREACH(menuLayerList, el) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
					auto menuLayer = static_cast<agtk::SceneLayer *>(el->getObject());
#else
					auto menuLayer = dynamic_cast<agtk::SceneLayer *>(el->getObject());
#endif

					if (menuLayer->getLayerId() != agtk::data::SceneData::kHudTopMostLayerId) {
						menuLayer->visit(renderer, vm_menu, true);
					}
					else {
						menuLayer->visit(renderer, vm, true);
					}
				}
			}
		}
	}
	renderTexture->end();
}

void GameManager::visitSceneOnlyMenu(cocos2d::Renderer *renderer, agtk::Scene *scene, cocos2d::RenderTexture *renderTexture)
{
	CC_ASSERT(renderer);
	CC_ASSERT(scene);
	CC_ASSERT(renderTexture);

	auto camera = scene->getCamera()->getCamera();
	auto vm = camera->getViewMatrix();
	auto menuCamera = scene->getCamera()->getMenuCamera();
	auto vm_menu = menuCamera->getViewMatrix();

	renderTexture->beginWithClear(0, 0, 0, 0);
	{
		// メニューシーン
		{
			auto menuLayerList = scene->getMenuLayerList();
			cocos2d::DictElement *el;
			CCDICT_FOREACH(menuLayerList, el) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
				auto menuLayer = static_cast<agtk::SceneLayer *>(el->getObject());
#else
				auto menuLayer = dynamic_cast<agtk::SceneLayer *>(el->getObject());
#endif

				menuLayer->setVisible(true);
				if (menuLayer->getLayerId() != agtk::data::SceneData::kHudTopMostLayerId) {
					menuLayer->visit(renderer, vm_menu, true);
				}
				else {
					menuLayer->visit(renderer, vm, true);
				}
				menuLayer->setVisible(false);
			}
		}
	}

	renderTexture->end();
}

#ifdef USE_PREVIEW
void GameManager::dropFileHandler(const char *filename)
{
	if (strlen(filename) >= 9 && strcmp(filename + strlen(filename) - 9, ".pgminput") == 0) {
		// リプレイデータ内にリセット開始フラグがあるか先に確認
		if (InputManager::getInstance()->getRestartFlg(filename)) {
			// リセット開始フラグがある場合は先にrestartCanvas()を済ませてからリプレイデータを読み込む
			auto gm = GameManager::getInstance();
			gm->restartCanvas();
			auto gameScene = dynamic_cast<GameScene *>(gm->getCurrentLayer());
			if (gameScene) {
				// restartCanvas()中はgameSceneのupdateを行わない
				gameScene->setIsRestartCanvas(true);
			}

			std::string _filename = filename;
			gm->getCurrentLayer()->runAction(agtk::Sequence::create(
				DelayTime::create(0.01f),
				agtk::IfCallFunc::create([gm, _filename]() {
					auto inputManager = InputManager::getInstance();
					auto gameScene = dynamic_cast<GameScene *>(gm->getCurrentLayer());
					if (gameScene) {
						// restartCanvas()が終わったのでgameSceneのupdateを再開
						gameScene->setIsRestartCanvas(false);
					}
					InputManager::getInstance()->startPlaying(_filename.c_str());
					return true;
			}),
				nullptr));
		}
		else {
			InputManager::getInstance()->startPlaying(filename);
		}
	}
}
#endif

void GameManager::initPlugins()
{
	auto scriptingCore = ScriptingCore::getInstance();
	if (!scriptingCore->runScript("plugins/prepare.js")) {
		auto errorStr = "Runtime error in plugins/prepare.js";
		agtk::DebugManager::getInstance()->getDebugExecuteLogWindow()->addLog(errorStr);
#if defined(USE_PREVIEW)
		auto fp = GameManager::getScriptLogFp();
		if (fp) {
			fwrite(errorStr, 1, strlen(errorStr), fp);
			fwrite("\n", 1, 1, fp);
		}
#endif
	}
	{
		auto context = scriptingCore->getGlobalContext();
		auto _global = scriptingCore->getGlobalObject();
		JS::RootedObject global(context, _global);
		JSAutoCompartment ac(context, global);
		if (!JavascriptManager::addObject(context, global)) {
			CCASSERT(0, "failed to addObject");
		}
	}
	AGTK_DEBUG_ACTION_LOG("%d,%s", __LINE__, __FUNCTION__);
	if (!scriptingCore->runScript("plugins/init.js")) {
		auto errorStr = "Runtime error in plugins/init.js";
		agtk::DebugManager::getInstance()->getDebugExecuteLogWindow()->addLog(errorStr);
#if defined(USE_PREVIEW)
		auto fp = GameManager::getScriptLogFp();
		if (fp) {
			fwrite(errorStr, 1, strlen(errorStr), fp);
			fwrite("\n", 1, 1, fp);
		}
#endif
	}
	AGTK_DEBUG_ACTION_LOG("%d,%s", __LINE__, __FUNCTION__);
	DllPluginManager::getInstance()->loadPlugins();
	JavascriptManager::loadPlugins();

#ifdef USE_SCRIPT_PRECOMPILE
	//スクリプトをコンパイルして登録しておく。
	auto context = scriptingCore->getGlobalContext();
	JS::RootedValue rv(context);
	auto projectData = getProjectData();
	//オブジェクト
	{
		std::function<void(agtk::data::ObjectData *)> compileObjectScriptsRecur = [&](agtk::data::ObjectData *objectData) {
			if (objectData->getFolder()) {
				auto dic = objectData->getChildren();
				cocos2d::DictElement *el = nullptr;
				CCDICT_FOREACH(dic, el) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
					auto child = static_cast<agtk::data::ObjectData *>(el->getObject());
#else
					auto child = dynamic_cast<agtk::data::ObjectData *>(el->getObject());
#endif
					compileObjectScriptsRecur(child);
				}
			}
			compileObjectScripts(objectData);
		};
		auto dic = projectData->getObjectList();
		cocos2d::DictElement *el = nullptr;
		CCDICT_FOREACH(dic, el) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
			auto objectData = static_cast<agtk::data::ObjectData *>(el->getObject());
#else
			auto objectData = dynamic_cast<agtk::data::ObjectData *>(el->getObject());
#endif
			compileObjectScriptsRecur(objectData);
		}
	}

	//シーン
	{
		auto dic = projectData->getSceneList();
		cocos2d::DictElement *el = nullptr;
		CCDICT_FOREACH(dic, el) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
			auto sceneData = static_cast<agtk::data::SceneData *>(el->getObject());
#else
			auto sceneData = dynamic_cast<agtk::data::SceneData *>(el->getObject());
#endif
			auto dic2 = sceneData->getScenePartOthersList();
			cocos2d::DictElement *el2 = nullptr;
			CCDICT_FOREACH(dic2, el2) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
				auto othersData = static_cast<agtk::data::ScenePartOthersData *>(el2->getObject());
#else
				auto othersData = dynamic_cast<agtk::data::ScenePartOthersData *>(el2->getObject());
#endif
				auto othersType = othersData->getOthersType();
				if (othersType != agtk::data::ScenePartOthersData::kOthersLineCourse
					&& othersType != agtk::data::ScenePartOthersData::kOthersCurveCourse
					&& othersType != agtk::data::ScenePartOthersData::kOthersCircleCourse) {
					continue;
				}
				auto partData = othersData->getPart();
				cocos2d::__Array *pointList = nullptr;
				if (othersType == agtk::data::ScenePartOthersData::kOthersLineCourse) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
					pointList = static_cast<agtk::data::OthersLineCourseData *>(partData)->getPointList();
#else
					pointList = dynamic_cast<agtk::data::OthersLineCourseData *>(partData)->getPointList();
#endif
				}
				else if (othersType == agtk::data::ScenePartOthersData::kOthersCurveCourse) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
					pointList = static_cast<agtk::data::OthersCurveCourseData *>(partData)->getPointList();
#else
					pointList = dynamic_cast<agtk::data::OthersCurveCourseData *>(partData)->getPointList();
#endif
				}
				else if (othersType == agtk::data::ScenePartOthersData::kOthersCircleCourse) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
					pointList = static_cast<agtk::data::OthersCircleCourseData *>(partData)->getPointList();
#else
					pointList = dynamic_cast<agtk::data::OthersCircleCourseData *>(partData)->getPointList();
#endif
				}
				cocos2d::Ref *ref;
				int pointIndex = -1;
				CCARRAY_FOREACH(pointList, ref) {
					pointIndex++;
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
					auto pointData = static_cast<agtk::data::OthersData::Point *>(ref);
#else
					auto pointData = dynamic_cast<agtk::data::OthersData::Point *>(ref);
#endif
					auto assignList = pointData->getSwitchVariableAssignList();
					cocos2d::Ref *ref2;
					int index = -1;
					CCARRAY_FOREACH(assignList, ref2) {
						index++;
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
						auto assignData = static_cast<agtk::data::SwitchVariableAssignData *>(ref2);
#else
						auto assignData = dynamic_cast<agtk::data::SwitchVariableAssignData *>(ref2);
#endif
						if (assignData->getSwtch()) {
							continue;
						}
						if (assignData->getVariableAssignValueType() != agtk::data::SwitchVariableAssignData::kVariableAssignScript) {
							continue;
						}
						if (strlen(assignData->getAssignScript()) == 0) {
							continue;
						}
						auto script = assignData->getAssignScript();
						auto regScript = String::createWithFormat("Agtk.scriptFunctions.sceneCourseSwitchVariableChange['%d,%d,%d,%d'] = function(sceneId, scenePartId, pointIndex, index){ return (%s\n); }",
							sceneData->getId(), othersData->getId(), pointIndex, index, script);
						bool ret = scriptingCore->evalString(regScript->getCString());
						if (!ret) {
							//スクリプトエラー
							auto errorStr = String::createWithFormat("Failed to compile course script: pointSwitchVariableChange(sceneId: %d, scenePartId: %d, pointIndex, index: %d, script: %s).", sceneData->getId(), othersData->getId(), pointIndex, index, script)->getCString();
							agtk::DebugManager::getInstance()->getDebugExecuteLogWindow()->addLog(errorStr);
#if defined(USE_PREVIEW)
							auto fp = GameManager::getScriptLogFp();
							if (fp) {
								fwrite(errorStr, 1, strlen(errorStr), fp);
								fwrite("\n", 1, 1, fp);
							}
#endif
						}
					}
				}
			}
			{
				auto sceneId = sceneData->getId();
				dic2 = sceneData->getScenePartObjectList();
				CCDICT_FOREACH(dic2, el2) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
					auto scenePartObjectData = static_cast<agtk::data::ScenePartObjectData *>(el2->getObject());
#else
					auto scenePartObjectData = dynamic_cast<agtk::data::ScenePartObjectData *>(el2->getObject());
#endif
					compileSceneObjectScripts(sceneId, scenePartObjectData);
				}
			}
		}
	}

	//タイルセット
	{
		auto tileWidth = projectData->getTileWidth();
		auto tileHeight = projectData->getTileHeight();
		auto dic = projectData->getTilesetList();
		cocos2d::DictElement *el = nullptr;
		CCDICT_FOREACH(dic, el) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
			auto tilesetData = static_cast<agtk::data::TilesetData *>(el->getObject());
#else
			auto tilesetData = dynamic_cast<agtk::data::TilesetData *>(el->getObject());
#endif
			if (tilesetData->getTilesetType() != agtk::data::TilesetData::kGimmick) {
				continue;
			}
			auto imageData = projectData->getImageData(tilesetData->getImageId());
			if (!imageData) {
				continue;
			}
			auto horzTileCount = (imageData->getTexWidth() + tileWidth - 1) / tileWidth;
			//auto vertTileCount = (imageData->getTexHeight() + tileHeight - 1) / tileHeight;
			auto dic2 = tilesetData->getTileDataList();
			cocos2d::DictElement *el2 = nullptr;
			CCDICT_FOREACH(dic2, el2) {
				auto tileIndex = el2->getIntKey();
				auto tileX = tileIndex % horzTileCount;
				auto tileY = tileIndex / horzTileCount;
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
				auto tileData = static_cast<agtk::data::TileData *>(el2->getObject());
#else
				auto tileData = dynamic_cast<agtk::data::TileData *>(el2->getObject());
#endif
				auto assignList = tileData->getSwitchVariableAssignList();
				cocos2d::Ref *ref;
				int index = -1;
				CCARRAY_FOREACH(assignList, ref) {
					index++;
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
					auto assignData = static_cast<agtk::data::TileSwitchVariableAssignData *>(ref);
#else
					auto assignData = dynamic_cast<agtk::data::TileSwitchVariableAssignData *>(ref);
#endif
					if (assignData->getSwtch()) {
						continue;
					}
					if (assignData->getVariableAssignValueType() != agtk::data::SwitchVariableAssignData::kVariableAssignScript) {
						continue;
					}
					if (assignData->getAssignScript()->length() == 0) {
						continue;
					}
					auto script = assignData->getAssignScript()->getCString();
					auto regScript = String::createWithFormat("Agtk.scriptFunctions.tilesetTileGimmickSwitchVariableChange['%d,%d,%d,%d'] = function(tilesetId, tileX, tileY, index){ return (%s\n); }",
						tilesetData->getId(), tileX, tileY, index, script);
					bool ret = scriptingCore->evalString(regScript->getCString());
					if (!ret) {
						//スクリプトエラー
						auto errorStr = String::createWithFormat("Failed to compile tile script: gimmickSwitchVariableChange(tilesetId: %d, tileX: %d, tileY: %d, index: %d, script: %s).", tilesetData->getId(), tileX, tileY, index, script)->getCString();
						agtk::DebugManager::getInstance()->getDebugExecuteLogWindow()->addLog(errorStr);
#if defined(USE_PREVIEW)
						auto fp = GameManager::getScriptLogFp();
						if (fp) {
							fwrite(errorStr, 1, strlen(errorStr), fp);
							fwrite("\n", 1, 1, fp);
						}
#endif
					}
				}
			}
		}
	}

	//遷移リンク
	{
		auto transitionFlow = projectData->getTransitionFlow();
		auto linkList = transitionFlow->getFlowLinkList();
		cocos2d::Ref *ref;
		CCARRAY_FOREACH(linkList, ref) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
			auto linkData = static_cast<agtk::data::FlowLinkData *>(ref);
#else
			auto linkData = dynamic_cast<agtk::data::FlowLinkData *>(ref);
#endif
			if (!linkData->getPostMoveChangeSwitchVariable()) {
				continue;
			}
			auto assignList = linkData->getPostMoveSwitchVariableAssignList();
			int index = -1;
			CCARRAY_FOREACH(assignList, ref) {
				index++;
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
				auto assignData = static_cast<agtk::data::SwitchVariableAssignData *>(ref);
#else
				auto assignData = dynamic_cast<agtk::data::SwitchVariableAssignData *>(ref);
#endif
				if (assignData->getSwtch()) {
					continue;
				}
				if (assignData->getVariableAssignValueType() != agtk::data::SwitchVariableAssignData::kVariableAssignScript) {
					continue;
				}
				if (strlen(assignData->getAssignScript()) == 0) {
					continue;
				}
				auto script = assignData->getAssignScript();
				auto regScript = String::createWithFormat("Agtk.scriptFunctions.transitionLinkPostMoveSwitchVariableChange['%d,%d'] = function(transitionLinkId, index){ return (%s\n); }",
					linkData->getId(), index, script);
				bool ret = scriptingCore->evalString(regScript->getCString());
				if (!ret) {
					//スクリプトエラー
					auto errorStr = String::createWithFormat("Failed to compile transition link script: postMoveSwitchVariableAssign(transitionLinkId: %d, index: %d, script: %s).", linkData->getId(), index, script)->getCString();
					agtk::DebugManager::getInstance()->getDebugExecuteLogWindow()->addLog(errorStr);
#if defined(USE_PREVIEW)
					auto fp = GameManager::getScriptLogFp();
					if (fp) {
						fwrite(errorStr, 1, strlen(errorStr), fp);
						fwrite("\n", 1, 1, fp);
					}
#endif
				}
			}
		}
	}

	//ポータル
	{
		auto portalList = projectData->getTransitionPortalList();
		cocos2d::Ref *ref;
		CCARRAY_FOREACH(portalList, ref) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
			auto portalData = static_cast<agtk::data::TransitionPortalData *>(ref);
#else
			auto portalData = dynamic_cast<agtk::data::TransitionPortalData *>(ref);
#endif
			for (int i = 0; i < agtk::data::TransitionPortalData::MAX; i++) {
				auto moveSettingList = portalData->getMoveSettingList();
				if (!moveSettingList) {
					continue;
				}
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
				auto moveSettingData = static_cast<agtk::data::MoveSettingData *>(moveSettingList->objectAtIndex(i));
#else
				auto moveSettingData = dynamic_cast<agtk::data::MoveSettingData *>(moveSettingList->objectAtIndex(i));
#endif
				if (!moveSettingData->getPreMoveChangeSwitchVariable()) {
					continue;
				}
				const char *portalCategoryStr = nullptr;
				const char *categoryStr = nullptr;
				for (int j = 0; j < 2; j++) {
					cocos2d::__Array *assignList = nullptr;
					if (j == 0) {
						portalCategoryStr = "portalPreMoveSwitchVariableChange";
						categoryStr = "preMoveSwitchVariableChange";
						assignList = moveSettingData->getPreMoveSwitchVariableAssignList();
					}
					else if (j == 1) {
						portalCategoryStr = "portalPostMoveSwitchVariableChange";
						categoryStr = "postMoveSwitchVariableChange";
						assignList = moveSettingData->getPostMoveSwitchVariableAssignList();
					}
					cocos2d::Ref *ref2;
					int index = -1;
					CCARRAY_FOREACH(assignList, ref2) {
						index++;
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
						auto assignData = static_cast<agtk::data::SwitchVariableAssignData *>(ref2);
#else
						auto assignData = dynamic_cast<agtk::data::SwitchVariableAssignData *>(ref2);
#endif
						if (assignData->getSwtch()) {
							continue;
						}
						if (assignData->getVariableAssignValueType() != agtk::data::SwitchVariableAssignData::kVariableAssignScript) {
							continue;
						}
						if (strlen(assignData->getAssignScript()) == 0) {
							continue;
						}
						auto script = assignData->getAssignScript();
						auto regScript = String::createWithFormat("Agtk.scriptFunctions.%s['%d,%d,%d'] = function(portalId, abId, index){ return (%s\n); }",
							portalCategoryStr, portalData->getId(), i, index, script);
						bool ret = scriptingCore->evalString(regScript->getCString());
						if (!ret) {
							//スクリプトエラー
							auto errorStr = String::createWithFormat("Failed to compile portal script: %s(portalId: %d, abId: %d, index: %d, script: %s).", categoryStr, portalData->getId(), i, index, script)->getCString();
							agtk::DebugManager::getInstance()->getDebugExecuteLogWindow()->addLog(errorStr);
#if defined(USE_PREVIEW)
							auto fp = GameManager::getScriptLogFp();
							if (fp) {
								fwrite(errorStr, 1, strlen(errorStr), fp);
								fwrite("\n", 1, 1, fp);
							}
#endif
						}
					}
				}
			}
		}
	}
#endif
}

void GameManager::finalPlugins()
{
	JavascriptManager::unloadPlugins();
	DllPluginManager::getInstance()->unloadPlugins();
	JavascriptManager::removeObject();
}

#ifdef USE_SCRIPT_PRECOMPILE
void GameManager::compileObjectScripts(agtk::data::ObjectData *objectData)
{
	//スクリプトをコンパイルして登録しておく。
	auto scriptingCore = ScriptingCore::getInstance();
	auto context = scriptingCore->getGlobalContext();
	JS::RootedValue rv(context);
	auto projectData = getProjectData();

	if (objectData->getDamagedSettingFlag()) {
		auto damagedSettingList = objectData->getDamagedSettingList();
		cocos2d::DictElement *el;
		CCDICT_FOREACH(damagedSettingList, el) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
			auto damagedSettingData = static_cast<agtk::data::ObjectDamagedSettingData *>(el->getObject());
#else
			auto damagedSettingData = dynamic_cast<agtk::data::ObjectDamagedSettingData *>(el->getObject());
#endif
			if (!damagedSettingData->getDamagedRateFlag()) {
				auto script = damagedSettingData->getDamagedScript();
				String *regScript = nullptr;
				if (strlen(script) > 0) {
					regScript = String::createWithFormat("Agtk.scriptFunctions.objectDamaged['%d,%d'] = function(objectId){ return (%s\n); }",
						objectData->getId(), damagedSettingData->getId(), script);
					bool ret = scriptingCore->evalString(regScript->getCString());
					if (!ret) {
						//スクリプトエラー
						auto errorStr = String::createWithFormat("Failed to compile command script: objectDamaged(objectId: %d, damagedId: %d, script: %s).", objectData->getId(), damagedSettingData->getId(), script)->getCString();
						agtk::DebugManager::getInstance()->getDebugExecuteLogWindow()->addLog(errorStr);
#if defined(USE_PREVIEW)
						auto fp = GameManager::getScriptLogFp();
						if (fp) {
							fwrite(errorStr, 1, strlen(errorStr), fp);
							fwrite("\n", 1, 1, fp);
						}
#endif
					}
				}
			}
		}
	}
	auto dic2 = objectData->getActionList();
	cocos2d::DictElement *el2 = nullptr;
	CCDICT_FOREACH(dic2, el2) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
		auto actionData = static_cast<agtk::data::ObjectActionData *>(el2->getObject());
#else
		auto actionData = dynamic_cast<agtk::data::ObjectActionData *>(el2->getObject());
#endif
		auto dic3 = actionData->getObjCommandList();
		cocos2d::DictElement *el3 = nullptr;
		CCDICT_FOREACH(dic3, el3) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
			auto commandData = static_cast<agtk::data::ObjectCommandData *>(el3->getObject());
#else
			auto commandData = dynamic_cast<agtk::data::ObjectCommandData *>(el3->getObject());
#endif
			auto commandType = commandData->getCommandType();
			const char *commandName = nullptr;
			const char *script = nullptr;
			String *regScript = nullptr;
			if (commandType == agtk::data::ObjectCommandData::kSwitchVariableChange) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
				auto cmd = static_cast<agtk::data::ObjectCommandSwitchVariableChangeData *>(commandData);
#else
				auto cmd = dynamic_cast<agtk::data::ObjectCommandSwitchVariableChangeData *>(commandData);
#endif
				if (!cmd->getSwtch()
					&& cmd->getVariableAssignValueType() == agtk::data::ObjectCommandSwitchVariableChangeData::kVariableAssignScript
					&& strlen(cmd->getAssignScript()) > 0) {
					script = cmd->getAssignScript();
					regScript = String::createWithFormat("Agtk.scriptFunctions.objectActionCommandSwitchVariableChange['-1,%d,0,%d,%d'] = function(objectId, instanceId, actionId, commandId, isCommonAction, sceneId){ return (%s\n); }",
						objectData->getId(), actionData->getId(), commandData->getId(), script);
					commandName = "switchVariableChange";
				}
			}
			else if (commandType == agtk::data::ObjectCommandData::kScriptEvaluate) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
				auto cmd = static_cast<agtk::data::ObjectCommandScriptEvaluate *>(commandData);
#else
				auto cmd = dynamic_cast<agtk::data::ObjectCommandScriptEvaluate *>(commandData);
#endif
				if (strlen(cmd->getScript()) > 0) {
					script = cmd->getScript();
					regScript = String::createWithFormat("Agtk.scriptFunctions.objectActionCommandScriptEvaluate['-1,%d,0,%d,%d'] = function(objectId, instanceId, actionId, commandId, isCommonAction, sceneId){ %s\n}",
						objectData->getId(), actionData->getId(), commandData->getId(), script);
					commandName = "scriptEvaluate";
				}
			}
			if (commandName && script) {
				bool ret = scriptingCore->evalString(regScript->getCString());
				if (!ret) {
					//スクリプトエラー
					auto errorStr = String::createWithFormat("Failed to compile command script: %s(objectId: %d, actionId: %d, commandId: %d, script: %s).", commandName, objectData->getId(), actionData->getId(), commandData->getId(), script)->getCString();
					agtk::DebugManager::getInstance()->getDebugExecuteLogWindow()->addLog(errorStr);
#if defined(USE_PREVIEW)
					auto fp = GameManager::getScriptLogFp();
					if (fp) {
						fwrite(errorStr, 1, strlen(errorStr), fp);
						fwrite("\n", 1, 1, fp);
					}
#endif
				}
			}
		}

	}
	if (objectData->getCommonActionSettingFlag()) {
		dic2 = objectData->getCommonActionSettingList();
		CCDICT_FOREACH(dic2, el2) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
			auto commonActionSettingData = static_cast<agtk::data::ObjectCommonActionSettingData *>(el2->getObject());
#else
			auto commonActionSettingData = dynamic_cast<agtk::data::ObjectCommonActionSettingData *>(el2->getObject());
#endif
			auto commonActionId = commonActionSettingData->getId();
			auto actionData = commonActionSettingData->getObjAction();
			auto dic3 = actionData->getObjCommandList();
			cocos2d::DictElement *el3 = nullptr;
			CCDICT_FOREACH(dic3, el3) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
				auto commandData = static_cast<agtk::data::ObjectCommandData *>(el3->getObject());
#else
				auto commandData = dynamic_cast<agtk::data::ObjectCommandData *>(el3->getObject());
#endif
				auto commandType = commandData->getCommandType();
				const char *commandName = nullptr;
				const char *script = nullptr;
				String *regScript = nullptr;
				if (commandType == agtk::data::ObjectCommandData::kSwitchVariableChange) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
					auto cmd = static_cast<agtk::data::ObjectCommandSwitchVariableChangeData *>(commandData);
#else
					auto cmd = dynamic_cast<agtk::data::ObjectCommandSwitchVariableChangeData *>(commandData);
#endif
					if (!cmd->getSwtch()
						&& cmd->getVariableAssignValueType() == agtk::data::ObjectCommandSwitchVariableChangeData::kVariableAssignScript
						&& strlen(cmd->getAssignScript()) > 0) {
						script = cmd->getAssignScript();
						regScript = String::createWithFormat("Agtk.scriptFunctions.objectActionCommandSwitchVariableChange['-1,%d,1,%d,%d'] = function(objectId, instanceId, actionId, commandId, isCommonAction, sceneId){ return (%s\n); }",
							objectData->getId(), commonActionId, commandData->getId(), script);
						commandName = "switchVariableChange";
					}
				}
				else if (commandType == agtk::data::ObjectCommandData::kScriptEvaluate) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
					auto cmd = static_cast<agtk::data::ObjectCommandScriptEvaluate *>(commandData);
#else
					auto cmd = dynamic_cast<agtk::data::ObjectCommandScriptEvaluate *>(commandData);
#endif
					if (strlen(cmd->getScript()) > 0) {
						script = cmd->getScript();
						regScript = String::createWithFormat("Agtk.scriptFunctions.objectActionCommandScriptEvaluate['-1,%d,1,%d,%d'] = function(objectId, instanceId, actionId, commandId, isCommonAction, sceneId){ %s\n}",
							objectData->getId(), commonActionId, commandData->getId(), script);
						commandName = "scriptEvaluate";
					}
				}
				if (commandName && script) {
					bool ret = scriptingCore->evalString(regScript->getCString());
					if (!ret) {
						//スクリプトエラー
						auto errorStr = String::createWithFormat("Failed to compile command script: %s(objectId: %d, commonActionSettingId: %d, commandId: %d, script: %s).", commandName, objectData->getId(), commonActionId, commandData->getId(), script)->getCString();
						agtk::DebugManager::getInstance()->getDebugExecuteLogWindow()->addLog(errorStr);
#if defined(USE_PREVIEW)
						auto fp = GameManager::getScriptLogFp();
						if (fp) {
							fwrite(errorStr, 1, strlen(errorStr), fp);
							fwrite("\n", 1, 1, fp);
						}
#endif
					}
				}
			}
			////
			auto actionLinkData = commonActionSettingData->getObjActionLink();
			auto commonActionLinkIndex = 0;
			auto linkConditionListDic = actionLinkData->getLinkConditionList();
			//cocos2d::DictElement *el3 = nullptr;
			CCDICT_FOREACH(linkConditionListDic, el3) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
				auto conditionData = static_cast<agtk::data::ObjectActionLinkConditionData *>(el3->getObject());
#else
				auto conditionData = dynamic_cast<agtk::data::ObjectActionLinkConditionData *>(el3->getObject());
#endif
				auto type = conditionData->getType();
				const char *conditionName = nullptr;
				const char *script = nullptr;
				String *regScript = nullptr;
				if (type == agtk::data::ObjectActionLinkConditionData::kConditionScript) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
					auto cnd = static_cast<agtk::data::ObjectActionLinkConditionScriptData *>(conditionData);
#else
					auto cnd = dynamic_cast<agtk::data::ObjectActionLinkConditionScriptData *>(conditionData);
#endif
					if (strlen(cnd->getScript()) > 0) {
						script = cnd->getScript();
						regScript = String::createWithFormat("Agtk.scriptFunctions.objectLinkConditionScript['%d,%d,%d,%d'] = function(objectId, instanceId, linkId, conditionId, commonActionLinkIndex){ return (%s\n); }",
							objectData->getId(), commonActionLinkIndex, commonActionId, conditionData->getId(), script);
						conditionName = "script";
					}
				}
				if (conditionName && regScript && script) {
					bool ret = scriptingCore->evalString(regScript->getCString());
					if (!ret) {
						//スクリプトエラー
						auto errorStr = String::createWithFormat("Failed to compile condition script: %s(objectId: %d, commonActionId: %d, commonActionLinkIndex: %d, conditionId: %d, script: %s).", conditionName, objectData->getId(), commonActionId, commonActionLinkIndex, conditionData->getId(), script)->getCString();
						agtk::DebugManager::getInstance()->getDebugExecuteLogWindow()->addLog(errorStr);
#if defined(USE_PREVIEW)
						auto fp = GameManager::getScriptLogFp();
						if (fp) {
							fwrite(errorStr, 1, strlen(errorStr), fp);
							fwrite("\n", 1, 1, fp);
						}
#endif
					}
				}
			}
			////
			auto actionLinkData2 = commonActionSettingData->getObjActionLink2();
			commonActionLinkIndex = 1;
			auto linkConditionListDic2 = actionLinkData2->getLinkConditionList();
			//cocos2d::DictElement *el3 = nullptr;
			CCDICT_FOREACH(linkConditionListDic2, el3) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
				auto conditionData = static_cast<agtk::data::ObjectActionLinkConditionData *>(el3->getObject());
#else
				auto conditionData = dynamic_cast<agtk::data::ObjectActionLinkConditionData *>(el3->getObject());
#endif
				auto type = conditionData->getType();
				const char *conditionName = nullptr;
				const char *script = nullptr;
				String *regScript = nullptr;
				if (type == agtk::data::ObjectActionLinkConditionData::kConditionScript) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
					auto cnd = static_cast<agtk::data::ObjectActionLinkConditionScriptData *>(conditionData);
#else
					auto cnd = dynamic_cast<agtk::data::ObjectActionLinkConditionScriptData *>(conditionData);
#endif
					if (strlen(cnd->getScript()) > 0) {
						script = cnd->getScript();
						regScript = String::createWithFormat("Agtk.scriptFunctions.objectLinkConditionScript['%d,%d,%d,%d'] = function(objectId, instanceId, linkId, conditionId, commonActionLinkIndex){ return (%s\n); }",
							objectData->getId(), commonActionLinkIndex, commonActionId, conditionData->getId(), script);
						conditionName = "script";
					}
				}
				if (conditionName && regScript && script) {
					bool ret = scriptingCore->evalString(regScript->getCString());
					if (!ret) {
						//スクリプトエラー
						auto errorStr = String::createWithFormat("Failed to compile condition script: %s(objectId: %d, commonActionId: %d, commonActionLinkIndex: %d, conditionId: %d, script: %s).", conditionName, objectData->getId(), commonActionId, commonActionLinkIndex, conditionData->getId(), script)->getCString();
						agtk::DebugManager::getInstance()->getDebugExecuteLogWindow()->addLog(errorStr);
#if defined(USE_PREVIEW)
						auto fp = GameManager::getScriptLogFp();
						if (fp) {
							fwrite(errorStr, 1, strlen(errorStr), fp);
							fwrite("\n", 1, 1, fp);
						}
#endif
					}
				}
			}
		}
	}
	if (objectData->getDisappearSettingFlag()) {
		auto disappearSettingData = objectData->getDisappearSetting();
		if (disappearSettingData != nullptr) {
			auto dic3 = disappearSettingData->getObjCommandList();
			cocos2d::DictElement *el3 = nullptr;
			CCDICT_FOREACH(dic3, el3) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
				auto commandData = static_cast<agtk::data::ObjectCommandData *>(el3->getObject());
#else
				auto commandData = dynamic_cast<agtk::data::ObjectCommandData *>(el3->getObject());
#endif
				auto commandType = commandData->getCommandType();
				const char *commandName = nullptr;
				const char *script = nullptr;
				String *regScript = nullptr;
				if (commandType == agtk::data::ObjectCommandData::kSwitchVariableChange) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
					auto cmd = static_cast<agtk::data::ObjectCommandSwitchVariableChangeData *>(commandData);
#else
					auto cmd = dynamic_cast<agtk::data::ObjectCommandSwitchVariableChangeData *>(commandData);
#endif
					if (!cmd->getSwtch()
						&& cmd->getVariableAssignValueType() == agtk::data::ObjectCommandSwitchVariableChangeData::kVariableAssignScript
						&& strlen(cmd->getAssignScript()) > 0) {
						script = cmd->getAssignScript();
						regScript = String::createWithFormat("Agtk.scriptFunctions.objectActionCommandSwitchVariableChange['-1,%d,0,-1,%d'] = function(objectId, instanceId, actionId, commandId, isCommonAction, sceneId){ return (%s\n); }",
							objectData->getId(), commandData->getId(), script);
						commandName = "switchVariableChange";
					}
				}
				else if (commandType == agtk::data::ObjectCommandData::kScriptEvaluate) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
					auto cmd = static_cast<agtk::data::ObjectCommandScriptEvaluate *>(commandData);
#else
					auto cmd = dynamic_cast<agtk::data::ObjectCommandScriptEvaluate *>(commandData);
#endif
					if (strlen(cmd->getScript()) > 0) {
						script = cmd->getScript();
						regScript = String::createWithFormat("Agtk.scriptFunctions.objectActionCommandScriptEvaluate['-1,%d,0,-1,%d'] = function(objectId, instanceId, actionId, commandId, isCommonAction, sceneId){ %s\n}",
							objectData->getId(), commandData->getId(), script);
						commandName = "scriptEvaluate";
					}
				}
				if (commandName && script) {
					bool ret = scriptingCore->evalString(regScript->getCString());
					if (!ret) {
						//スクリプトエラー
						auto errorStr = String::createWithFormat("Failed to compile command script: %s(objectId: %d, disappearSetting, commandId: %d, script: %s).", commandName, objectData->getId(), commandData->getId(), script)->getCString();
						agtk::DebugManager::getInstance()->getDebugExecuteLogWindow()->addLog(errorStr);
#if defined(USE_PREVIEW)
						auto fp = GameManager::getScriptLogFp();
						if (fp) {
							fwrite(errorStr, 1, strlen(errorStr), fp);
							fwrite("\n", 1, 1, fp);
						}
#endif
					}
				}
			}

		}
	}
	auto linkDic = objectData->getActionLinkList();
	CCDICT_FOREACH(linkDic, el2) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
		auto linkData = static_cast<agtk::data::ObjectActionLinkData *>(el2->getObject());
#else
		auto linkData = dynamic_cast<agtk::data::ObjectActionLinkData *>(el2->getObject());
#endif
		auto conditionGroupList = linkData->getLinkConditionGroupList();
		cocos2d::Ref *ref;
		CCARRAY_FOREACH(conditionGroupList, ref) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
			auto linkConditionList = static_cast<cocos2d::__Array *>(ref);
#else
			auto linkConditionList = dynamic_cast<cocos2d::__Array *>(ref);
#endif
			cocos2d::Ref *ref2;
			CCARRAY_FOREACH(linkConditionList, ref2) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
				auto conditionData = static_cast<agtk::data::ObjectActionLinkConditionData *>(ref2);
#else
				auto conditionData = dynamic_cast<agtk::data::ObjectActionLinkConditionData *>(ref2);
#endif
				auto type = conditionData->getType();
				const char *conditionName = nullptr;
				const char *script = nullptr;
				String *regScript = nullptr;
				if (type == agtk::data::ObjectActionLinkConditionData::kConditionScript) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
					auto cnd = static_cast<agtk::data::ObjectActionLinkConditionScriptData *>(conditionData);
#else
					auto cnd = dynamic_cast<agtk::data::ObjectActionLinkConditionScriptData *>(conditionData);
#endif
					if (strlen(cnd->getScript()) > 0) {
						script = cnd->getScript();
						regScript = String::createWithFormat("Agtk.scriptFunctions.objectLinkConditionScript['%d,-1,%d,%d'] = function(objectId, instanceId, linkId, conditionId, commonActionLinkIndex){ return (%s\n); }",
							objectData->getId(), linkData->getId(), conditionData->getId(), script);
						conditionName = "script";
					}
				}
				if (conditionName && regScript && script) {
					bool ret = scriptingCore->evalString(regScript->getCString());
					if (!ret) {
						//スクリプトエラー
						auto errorStr = String::createWithFormat("Failed to compile condition script: %s(objectId: %d, linkId: %d, conditionId: %d, script: %s).", conditionName, objectData->getId(), linkData->getId(), conditionData->getId(), script)->getCString();
						agtk::DebugManager::getInstance()->getDebugExecuteLogWindow()->addLog(errorStr);
#if defined(USE_PREVIEW)
						auto fp = GameManager::getScriptLogFp();
						if (fp) {
							fwrite(errorStr, 1, strlen(errorStr), fp);
							fwrite("\n", 1, 1, fp);
						}
#endif
					}
				}
			}
		}
	}
}

void GameManager::compileSceneObjectScripts(int sceneId, agtk::data::ScenePartObjectData *scenePartObjectData)
{
	if (scenePartObjectData->isStartPointObject()) {
		//スタートポイントで生成したオブジェクトは対象外。
		return;
	}
	//シーンに配置したオブジェクトのスクリプトをコンパイルして登録しておく。
	auto scriptingCore = ScriptingCore::getInstance();
	auto context = scriptingCore->getGlobalContext();
	JS::RootedValue rv(context);
	auto projectData = getProjectData();

	auto objectId = scenePartObjectData->getObjectId();
	auto objectData = projectData->getObjectData(objectId);
	auto instanceId = scenePartObjectData->getId();
	auto actionCommandListObject = scenePartObjectData->getActionCommandListObject();
	cocos2d::DictElement *el = nullptr;
	CCDICT_FOREACH(actionCommandListObject, el) {
		auto actionId = el->getIntKey();
		auto commandListDic = dynamic_cast<cocos2d::__Dictionary *>(el->getObject());
		cocos2d::DictElement *el4 = nullptr;
		CCDICT_FOREACH(commandListDic, el4) {
			auto commandId = el4->getIntKey();
			auto commandData = dynamic_cast<agtk::data::ObjectCommandData *>(el4->getObject());
			if (!commandData->getInstanceConfigurable()) {
				continue;
			}
			{
				auto commandType = commandData->getCommandType();
				const char *commandName = nullptr;
				const char *script = nullptr;
				String *regScript = nullptr;
				if (commandType == agtk::data::ObjectCommandData::kSwitchVariableChange) {
					auto cmd = dynamic_cast<agtk::data::ObjectCommandSwitchVariableChangeData *>(commandData);
					if (!cmd->getSwtch()
						&& cmd->getVariableAssignValueType() == agtk::data::ObjectCommandSwitchVariableChangeData::kVariableAssignScript
						&& strlen(cmd->getAssignScript()) > 0) {
						script = cmd->getAssignScript();
						regScript = String::createWithFormat("Agtk.scriptFunctions.objectActionCommandSwitchVariableChange['%d,%d,0,%d,%d'] = function(objectId, instanceId, actionId, commandId, isCommonAction, sceneId){ return (%s\n); }",
							sceneId, instanceId, actionId, commandData->getId(), script);
						commandName = "switchVariableChange";
					}
				}
				else if (commandType == agtk::data::ObjectCommandData::kScriptEvaluate) {
					auto cmd = dynamic_cast<agtk::data::ObjectCommandScriptEvaluate *>(commandData);
					if (strlen(cmd->getScript()) > 0) {
						script = cmd->getScript();
						regScript = String::createWithFormat("Agtk.scriptFunctions.objectActionCommandScriptEvaluate['%d,%d,0,%d,%d'] = function(objectId, instanceId, actionId, commandId, isCommonAction, sceneId){ %s\n}",
							sceneId, instanceId, actionId, commandData->getId(), script);
						commandName = "scriptEvaluate";
					}
				}
				if (commandName && script) {
					bool ret = scriptingCore->evalString(regScript->getCString());
					if (!ret) {
						//スクリプトエラー
						auto errorStr = String::createWithFormat("Failed to compile command script: %s(sceneId: %d, objectId: %d, instanceId: %d, actionId: %d, commandId: %d, script: %s).", commandName, sceneId, objectId, actionId, commandData->getId(), script)->getCString();
						agtk::DebugManager::getInstance()->getDebugExecuteLogWindow()->addLog(errorStr);
#if defined(USE_PREVIEW)
						auto fp = GameManager::getScriptLogFp();
						if (fp) {
							fwrite(errorStr, 1, strlen(errorStr), fp);
							fwrite("\n", 1, 1, fp);
						}
#endif
					}
				}
			}
		}
	}
	if (objectData->getCommonActionSettingFlag()) {
		auto dic2 = scenePartObjectData->getCommonActionCommandListObject();
		cocos2d::DictElement *el2 = nullptr;
		CCDICT_FOREACH(dic2, el2) {
			auto commonActionId = el2->getIntKey();
			auto dic3 = dynamic_cast<cocos2d::__Dictionary *>(el2->getObject());
			cocos2d::DictElement *el3 = nullptr;
			CCDICT_FOREACH(dic3, el3) {
				auto commandData = dynamic_cast<agtk::data::ObjectCommandData *>(el3->getObject());
				auto commandType = commandData->getCommandType();
				const char *commandName = nullptr;
				const char *script = nullptr;
				String *regScript = nullptr;
				if (commandType == agtk::data::ObjectCommandData::kSwitchVariableChange) {
					auto cmd = dynamic_cast<agtk::data::ObjectCommandSwitchVariableChangeData *>(commandData);
					if (!cmd->getSwtch()
						&& cmd->getVariableAssignValueType() == agtk::data::ObjectCommandSwitchVariableChangeData::kVariableAssignScript
						&& strlen(cmd->getAssignScript()) > 0) {
						script = cmd->getAssignScript();
						regScript = String::createWithFormat("Agtk.scriptFunctions.objectActionCommandSwitchVariableChange['%d,%d,1,%d,%d'] = function(objectId, instanceId, actionId, commandId, isCommonAction, sceneId){ return (%s\n); }",
							sceneId, instanceId, commonActionId, commandData->getId(), script);
						commandName = "switchVariableChange";
					}
				}
				else if (commandType == agtk::data::ObjectCommandData::kScriptEvaluate) {
					auto cmd = dynamic_cast<agtk::data::ObjectCommandScriptEvaluate *>(commandData);
					if (strlen(cmd->getScript()) > 0) {
						script = cmd->getScript();
						regScript = String::createWithFormat("Agtk.scriptFunctions.objectActionCommandScriptEvaluate['%d,%d,1,%d,%d'] = function(objectId, instanceId, actionId, commandId, isCommonAction, sceneId){ %s\n}",
							sceneId, instanceId, commonActionId, commandData->getId(), script);
						commandName = "scriptEvaluate";
					}
				}
				if (commandName && script) {
					bool ret = scriptingCore->evalString(regScript->getCString());
					if (!ret) {
						//スクリプトエラー
						auto errorStr = String::createWithFormat("Failed to compile command script: %s(sceneId: %d, objectId: %d, instanceId: %d, commonActionSettingId: %d, commandId: %d, script: %s).", commandName, sceneId, objectId, instanceId, commonActionId, commandData->getId(), script)->getCString();
						agtk::DebugManager::getInstance()->getDebugExecuteLogWindow()->addLog(errorStr);
#if defined(USE_PREVIEW)
						auto fp = GameManager::getScriptLogFp();
						if (fp) {
							fwrite(errorStr, 1, strlen(errorStr), fp);
							fwrite("\n", 1, 1, fp);
						}
#endif
					}
				}
			}

		}
	}
	if (objectData->getDisappearSettingFlag()) {
		auto dic3 = scenePartObjectData->getDisappearActionCommandList();
		{
			cocos2d::DictElement *el3 = nullptr;
			CCDICT_FOREACH(dic3, el3) {
				auto commandData = dynamic_cast<agtk::data::ObjectCommandData *>(el3->getObject());
				auto commandType = commandData->getCommandType();
				const char *commandName = nullptr;
				const char *script = nullptr;
				String *regScript = nullptr;
				if (commandType == agtk::data::ObjectCommandData::kSwitchVariableChange) {
					auto cmd = dynamic_cast<agtk::data::ObjectCommandSwitchVariableChangeData *>(commandData);
					if (!cmd->getSwtch()
						&& cmd->getVariableAssignValueType() == agtk::data::ObjectCommandSwitchVariableChangeData::kVariableAssignScript
						&& strlen(cmd->getAssignScript()) > 0) {
						script = cmd->getAssignScript();
						regScript = String::createWithFormat("Agtk.scriptFunctions.objectActionCommandSwitchVariableChange['%d,%d,0,-1,%d'] = function(objectId, instanceId, actionId, commandId, isCommonAction, sceneId){ return (%s\n); }",
							sceneId, instanceId, commandData->getId(), script);
						commandName = "switchVariableChange";
					}
				}
				else if (commandType == agtk::data::ObjectCommandData::kScriptEvaluate) {
					auto cmd = dynamic_cast<agtk::data::ObjectCommandScriptEvaluate *>(commandData);
					if (strlen(cmd->getScript()) > 0) {
						script = cmd->getScript();
						regScript = String::createWithFormat("Agtk.scriptFunctions.objectActionCommandScriptEvaluate['%d,%d,0,-1,%d'] = function(objectId, instanceId, actionId, commandId, isCommonAction, sceneId){ %s\n}",
							sceneId, instanceId, commandData->getId(), script);
						commandName = "scriptEvaluate";
					}
				}
				if (commandName && script) {
					bool ret = scriptingCore->evalString(regScript->getCString());
					if (!ret) {
						//スクリプトエラー
						auto errorStr = String::createWithFormat("Failed to compile command script: %s(sceneId: %d, objectId: %d, instanceId: %d, disappearSetting, commandId: %d, script: %s).", commandName, sceneId, objectId, instanceId, commandData->getId(), script)->getCString();
						agtk::DebugManager::getInstance()->getDebugExecuteLogWindow()->addLog(errorStr);
#if defined(USE_PREVIEW)
						auto fp = GameManager::getScriptLogFp();
						if (fp) {
							fwrite(errorStr, 1, strlen(errorStr), fp);
							fwrite("\n", 1, 1, fp);
						}
#endif
					}
				}
			}

		}
	}
}
#endif

// #AGTK-NX
#ifdef USE_SAR_OPTIMIZE_4
void GameManager::addSwitchWatcher(agtk::Object* object, intptr_t switchPtr)
{
	auto watcherList = getSwitchWatcherObjectList();
	auto r = watcherList->equal_range(switchPtr);
	if (r.first != watcherList->end()) {
		for (auto it = r.first; it != r.second; it++)
		{
			if (it->second == object) {
				return;
			}
		}
		watcherList->insert(r.second, std::make_pair(switchPtr, object));
	}
	else {
		watcherList->insert(std::make_pair(switchPtr, object));
	}
}

void GameManager::removeSwitchWatcher(agtk::Object* object, intptr_t switchPtr)
{
	auto watcherList = getSwitchWatcherObjectList();
	auto r = watcherList->equal_range(switchPtr);
	for (auto it = r.first; it != r.second; it++)
	{
		if (it->second == object) {
			watcherList->erase(it);
			return;
		}
	}
}
#endif
