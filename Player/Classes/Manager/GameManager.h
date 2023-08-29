#ifndef __GAME_MANAGER_H__
#define	__GAME_MANAGER_H__

#include "Data/AssetData.h"
#include "Data/ProjectData.h"
#include "Data/PlayData.h"
#include "Lib/Scene.h"
#include "Lib/Slope.h"
#if defined(USE_PREVIEW) || defined(AGTK_DEBUG)
#include "Lib/WebSocket.h"
#endif
#ifdef USE_PREVIEW
#include "Lib/SharedMemory.h"
#endif

#define DEFAULT_PHYSICS_ITERATION 10	// 物理演算イテレーション初期値

// map関連で多数するのでwarningを抑制「C4503: 装飾された名前の長さが限界を超えました。名前は切り捨てられます。」
#pragma warning(disable:4503)

USING_NS_CC;
class AGTKPLAYER_API GameManager : public Ref
{
public:
	/* 物理グループ */
	enum EnumPhysicsGroup
	{
		kObject = 1,			// オブジェクト
		kPhysicalObject = 2,	// 物理オブジェクト
		kRope = 3,				// ロープ
		kNoneObject = 4,		// 無し（他の物理オブジェクトと衝突しなくなる）

		kNone = -1,				// 無し(他の物理オブジェクトと衝突しなくなる)

		kTile = -2,				// タイル
		kSlope = -3,			// 坂

		kExplode = -4,			// 爆発
		kForce = -5				// 引力・斥力
	};

	static const int USE_SCENE_SETTING_BGM = -1;//「シーン設定のBGM」

	/* スタートポイント製オブジェクトの再生成回避用データ */
	class IgnoreCreateObjectData : public cocos2d::Ref {
	public:
		IgnoreCreateObjectData() { _playerId = 0; _objectId = 0; };
		virtual ~IgnoreCreateObjectData() { };
		CREATE_FUNC_PARAM2(IgnoreCreateObjectData, int, playerId, int, objectId);
		bool init(int playerId, int objectId) { _playerId = playerId; _objectId = objectId; return true; };

		CC_SYNTHESIZE(int, _playerId, PlayerId);//プレイヤーID
		CC_SYNTHESIZE(int, _objectId, ObjectId);//オブジェクトID
	};

private:
	GameManager();
	static GameManager *_gameManager;

	// 前の時間
	double _prevTime;

	const float AIR_DENSITY = 1.2250f;//空気密度(1気圧20度)
	const float AIR_RESISTANCE_COE = 0.05f;//空気抵抗係数(物体によって異なるが大層な計算が必要になるので固定値を使用する)
public:
	virtual void update(float delta);

//	// 読み込みパース済みJSONデータ
//	typedef std::map<std::string, picojson::value *> JsonMapType;
//	JsonMapType _jsonMap;
//
//	// レジュームデータに埋め込むアプリバージョン
//	char appVersion[4];

	// 保存ファイルパスを取得する。
	std::string getFilePath();

	// セーブフォルダがなければ作成し、セーブフォルダのパスを返す
	std::string createAndGetSaveDir()const;

//	// マスタファイルパスを取得する。
//	std::string getMasterFilePath(MasterFileTypes type);

//	// 日付変更のチェックをする。
//	void checkDateChanged();
	void startCanvas(bool bLoading = false);
	void restartCanvas(bool bLoading = false);
	cocos2d::Scene *createCanvas(int id, bool bLoading);
	void changeCanvas(int id, bool bLoading = false);
	void changeCanvas();
	void createLoadingScene();
	void removeLoadingScene();

#if defined(USE_PREVIEW) || defined(AGTK_DEBUG)
	void createWebsocket(char *hostname, int port);
	void removeWebsocket();
	bool sendMessage(const std::string msg);
	bool sendBinaryMessage(unsigned char *data, unsigned int len);
	static void setWebSocketPort(int port) {
		mPort = port;
	}
#endif
#if defined(USE_PREVIEW)
	void runCommand(const rapidjson::Value &json);
	void runObjectCommand(const rapidjson::Value &json);
	void runSceneCommand(const rapidjson::Value &json);
	void runTilesetCommand(const rapidjson::Value &json);
	void runImageCommand(const rapidjson::Value &json);
	void runTextCommand(const rapidjson::Value &json);
#endif
	static std::string getProjectPathFromProjectFile(const std::string &filePath);
	// 最後にタイトルにログインした時間(tm)を取得する。
	struct tm *getLastLoginTm();
	void initPlugins();
	void finalPlugins();
#ifdef USE_SCRIPT_PRECOMPILE
	void compileObjectScripts(agtk::data::ObjectData *objectData);
	void compileSceneObjectScripts(int sceneId, agtk::data::ScenePartObjectData *scenePartObjectData);
#endif
#if defined(USE_PREVIEW) || defined(AGTK_DEBUG)
	static int *getPortPtr() { return &mPort; }
	static char *getHostnamePtr() { return &mHostname[0]; }
	static int getHostnameSize() { return sizeof(mHostname); }
#endif
	//シーン終了時の状態維持情報を破棄する
	void removeTakeoverStatesObject(int sceneId, int sceneLayerId);
#if defined(USE_PREVIEW)
	static FILE *getScriptLogFp() { return mScriptLogFp; }
#endif
	static void visitScene(cocos2d::Renderer *renderer, agtk::Scene *scene, cocos2d::RenderTexture *renderTexture);
	static void visitSceneOnlyMenu(cocos2d::Renderer *renderer, agtk::Scene *scene, cocos2d::RenderTexture *renderTexture);
#ifdef USE_PREVIEW
	static void dropFileHandler(const char *filename);
#endif
	bool checkPortalTouched(agtk::Object *object);

private:
#if defined(USE_PREVIEW) || defined(AGTK_DEBUG)
	CC_SYNTHESIZE_RETAIN(agtk::WebSocket*, _webSocket, WebSocket);
	//CC_SYNTHESIZE_RETAIN(cocos2d::__Array *, _logList, LogList);
	bool _bTerminate;
	static int mPort;
	static char mHostname[64];
#endif
#if defined(USE_PREVIEW)
	static FILE *mScriptLogFp;
#endif
	static int mLogNumber;
	std::string _saveDataPath;
#ifdef USE_PREVIEW
	CC_SYNTHESIZE(agtk::SharedMemory *, _sharedMemory, SharedMemory);
#endif

public:
	CC_SYNTHESIZE_RETAIN(agtk::data::ProjectData *, _projectData, ProjectData);
	CC_SYNTHESIZE_RETAIN(agtk::data::PlayData *, _playData, PlayData);
	CC_SYNTHESIZE_RETAIN(agtk::Scene *, _currentScene, CurrentScene);
	CC_SYNTHESIZE_RETAIN(agtk::Scene *, _loadingScene, LoadingScene);
	CC_SYNTHESIZE_RETAIN(cocos2d::Layer *, _currentLayer, CurrentLayer);
	CC_SYNTHESIZE_READONLY(int, _prevSceneId, PrevSceneId);
	CC_SYNTHESIZE_READONLY(int, _nextSceneId, NextSceneId);
	int setNextSceneId(int id) {
		_prevSceneId = this->getNextSceneId();
		_nextSceneId = id;
		return this->getPrevSceneId();
	}
	enum {
		kLoadBit_CommonVariable = (1 << 0),// プロジェクト共通編巣をロード 
		kLoadBit_CommonSwitch   = (1 << 1),// プロジェクト共通スイッチをロード
		kLoadBit_Scene          = (1 << 2),// シーンをロード
		kLoadBit_ObjectList     = (1 << 3),// シーン配置されているオブジェクトをロード
		kLoadBit_All            = (1 << 4) - 1,// 全てロード 
	};
	CC_SYNTHESIZE(int, _loadBit, LoadBit); // なにをロードするか
	CC_SYNTHESIZE(int, _loadEffectType, LoadEffectType); // ロード画面演出
	CC_SYNTHESIZE(int, _loadEffectDuration300, LoadEffectDuration300); // ロード画面演出時間

	enum EnumStateLoading {
		kStateLoadingNone,
		kStateLoadingTimerStart,
		kStateLoadingTimerUpdate,
		kStateLoadingTimerEnd,
		kStateLoadingShowStart,
		kStateLoadingShowUpdate,
		kStateLoadingShowEnd,
	};
	CC_SYNTHESIZE(EnumStateLoading, _stateLoading, StateLoading);
	CC_SYNTHESIZE_RETAIN(agtk::Timer *, _loadingTimer, LoadingTimer);
	// シーン破棄フラグ(この値がTrueの場合、シーン更新後にカレントシーンが破棄される)
	CC_SYNTHESIZE(bool, _needTerminateScene, NeedTerminateScene);

	// ポータル遷移要求フラグ
	CC_SYNTHESIZE(bool, _needPortalMove, NeedPortalMove);
	// シーン変更有無フラグ
	CC_SYNTHESIZE(bool, _needSceneChange, NeedSceneChange);
	//ポータルデータ
	CC_SYNTHESIZE_RETAIN(agtk::data::MoveSettingData *, _portalData, PortalData);
	//ポータルに触れたプレイヤーリスト
	CC_SYNTHESIZE_RETAIN(cocos2d::__Array *, _portalTouchedPlayerList, PortalTouchedPlayerList);
#ifdef FIX_ACT2_4774
	//ポータル遷移するポータルのID
	CC_SYNTHESIZE(int, _transitionPortalId, TransitionPortalId);
#endif
#ifdef FIX_ACT2_5233
	CC_SYNTHESIZE(int, _moveToLayerId, MoveToLayerId);
#endif
	// ポータル遷移中フラグ
	CC_SYNTHESIZE(bool, _isPortalMoving, IsPortalMoving);
	CC_SYNTHESIZE(bool, _isSceneMoving, IsSceneMoving);
	bool isSceneChanging() { return _isPortalMoving || _isSceneMoving; }

	// シーン切り替えで復活する消滅済みオブジェクトのリスト
	CC_SYNTHESIZE_RETAIN(cocos2d::__Array *, _sceneChangeReappearObjectList, SceneChangeReappearObjectList);
	// シーン遷移後、復活条件が設定されていないので復活できないオブジェクトのリスト
	CC_SYNTHESIZE_RETAIN(cocos2d::__Array *, _notReappearObjectList, NotReappearObjectList);
	// 実行アクションで復活するオブジェクト
	CC_SYNTHESIZE_RETAIN(cocos2d::__Array *, _commandReappearObjectList, CommandReappearObjectList);
	// シーン終了時の状態を維持のオブジェクトのリスト
	CC_SYNTHESIZE_RETAIN(cocos2d::__Array *, _sceneEndTakeoverStatesObjectList, SceneEndTakeoverStatesObjectList);
	// BGMフェードアウト中フラグ
	CC_SYNTHESIZE(bool, _isBgmFadeOut, IsBgmFadeOut);

	// データロードするシーンID
	CC_SYNTHESIZE(int, _loadSceneId, LoadSceneId);

	CC_SYNTHESIZE(int, _preMoveBgmId, PreMoveBgmId);//遷移前のシーンのBGMID
	CC_SYNTHESIZE(int, _preMoveBgmLoopFlag, PreMoveBgmLoopFlag);//遷移前のシーンのBGMループフラグ

	// 共通情報
	//CC_SYNTHESIZE(int, _resVersion, ResVersion);													// リソースバージョン
	//CC_SYNTHESIZE(int, _appVersion, AppVersion);													// アプリバージョン
	CC_SYNTHESIZE(double, _currentTime, CurrentTime);												// 現在時間
	CC_SYNTHESIZE_RETAIN(agtk::Timer *, _timer, Timer);
	CC_SYNTHESIZE_RETAIN(cocos2d::String *, _projectFilePath, ProjectFilePath);
#ifdef USE_PREVIEW
	CC_SYNTHESIZE_RETAIN(cocos2d::String *, _previewMode, PreviewMode);
#endif
	CC_SYNTHESIZE_RETAIN(cocos2d::String *, _appName, AppName);										// アプリ名文字列
	CC_SYNTHESIZE_RETAIN(cocos2d::String *, _appVersion, AppVersion);								// アプリバージョン文字列
	CC_SYNTHESIZE(std::string, _editorLocale, EditorLocale);	//エディターから指定されたロケール
#ifdef USE_PREVIEW
	CC_SYNTHESIZE(std::string, _autoTestReplayFilePath, AutoTestReplayFilePath);
	CC_SYNTHESIZE(std::string, _autoTestPluginFilePath, AutoTesPluginFilePath);
	CC_SYNTHESIZE(std::string, _autoTestPluginInternalJson, AutoTesPluginInternalJson);
	CC_SYNTHESIZE(std::string, _autoTestPluginParamValueJson, AutoTesPluginParamValueJson);
	CC_SYNTHESIZE(bool, _startSceneFlg, StartSceneFlg);
	CC_SYNTHESIZE(bool, _autoTestRestartFlg, AutoTestRestartFlg);
#endif

	// 生成除外プレイヤーリスト
	CC_SYNTHESIZE_RETAIN(cocos2d::__Array *, _ignoreCreatePlayerList, IgnoreCreatePlayerList);

	struct BgmInfo {
		int _bgmId{ -1 };
		bool _loop{ false };
		int _volume{ 0 };
		int _pan{ 0 };
		int _pitch{ 0 };
		bool compare(const BgmInfo& bgm) {
			//完全一致の場合:false, それ以外はtrue
			return !((bgm._bgmId == this->_bgmId) && (bgm._loop == this->_loop) && (bgm._volume == this->_volume) && (bgm._pan == this->_pan) && (bgm._pitch == this->_pitch));
		}
	};
	std::vector<BgmInfo> _loadedBgm;// セーブデータからロードしたBGM情報
	std::vector<BgmInfo>& getLoadedBgm() { return _loadedBgm; }
	void clearLoadedBgm(const BgmInfo& bgm) { this->clearBgm(bgm, _loadedBgm); };
	std::vector<BgmInfo> _registeringBgm;//
	std::vector<BgmInfo> _registeringStopBgm;//再生中BGM停止用
	void clearRegisteringBgm(const BgmInfo& bgm) { this->clearBgm(bgm, _registeringBgm); };
	void clearRegisteringStopBgm(const BgmInfo& bgm) { this->clearBgm(bgm, _registeringStopBgm); };
protected:
	void clearBgm(const BgmInfo& bgm, std::vector<BgmInfo> &list) {
		for (auto it = list.begin(); it != list.end(); ++it) {
			auto p = &(*it);
			if (p->compare(bgm) == false) {
				list.erase(it);
				break;
			}
		}
	}

	CC_SYNTHESIZE(std::list<int>, _preloadBgmEnd, PreloadBgmEnd);//先読みが終了したBGM
	CC_SYNTHESIZE(int, _frameRate, FrameRate);

	CC_SYNTHESIZE(int, _frame, Frame);	//何フレーム目か
// #AGTK-NX #AGTK-WIN
#ifdef USE_30FPS
	CC_SYNTHESIZE(float, _frameProgressScale, FrameProgressScale); //フレームレート制御による60FPSを基準とした進行倍率
#endif
	// デストラクタ
	virtual ~GameManager();

	// インスタンスを取得する。
	static GameManager* getInstance();

	static void purge();

	// 現在の時間(tm)を取得する。
	struct tm *getCurrentTm();

/*	// jsonファイルを読み込む。
	void loadMasterFile();

	// パース済みJSONがあれば返す。なければ読み込んでパースしたものを返す。
	picojson::value *getJson(std::string jsonFilePath);

	// パース済みのJSONをセットする。
	void setJson(std::string jsonFilePath, picojson::value *json);

	// 一時的な値を削除する。
	void removeTempValue();*/

	void removeCommandReappearObjectList(bool bWithoutScenePartObjectOnly);

	void setCursorVisible(bool visible);

	void loadProjectData(std::string filePath);// プロジェクトデータを読み込む
	void startProjectData(int sceneId = -1);// プロジェクトデータを開始する。
	void restartProjectData();// プロジェクトデータを再開始する。
	void stopProjectData();// プロジェクトデータを停止する。
	void loadAndStartProjectData(std::string filePath, int sceneId = -1);// プロジェクトデータを読み込んで開始する。

	void startScene(int sceneId = -1);//シーンをスタートする。
	void restartScene();//シーンを再スタートする。
	void stopScene();//シーンを停止する（終了する）。
	void loadAndStartScene(std::string filePath, int sceneId = -1);//プロジェクトファイルを読み込みシーンをスタートする。
#ifdef USE_PREVIEW
	void loadProject(std::string filePath);//プロジェクトファイルを読み込む。
#endif

protected:
	void updateSceneState();

	enum SceneState { kSceneStateIdle, kSceneStateStart, kSceneStateRestart, kSceneStateStop, kSceneStateLoadAndStart
#ifdef USE_PREVIEW
		, kSceneStateLoad
#endif
	};
	struct SceneStateInfo {
		SceneState state;
		std::string projectFilePath;
		int sceneId;
		SceneStateInfo() {
			state = kSceneStateIdle;
			sceneId = -1;
		}
	} _sceneStateInfo;

public:
	void loadJsonFile(std::string projectFilePath);

	static void actionLog(bool bPrintTime, const char *fmt, ...);
	static void debugLog(const char *fmt, ...);
	static void setLogNumber(int logNumber);

	void checkRequestVariableAndSwitch();
	void updateObjectVariableAndSwitch();
	void updateSystemVariableAndSwitch();
	void updateByChangingVariableAndSwitch();// スイッチ・変数変更による更新
	void updateFileExistSwitch();// ファイル確認スイッチを更新

	void saveConfig();
	void loadConfig();
	bool isExistConfigData()const;
	std::string getSaveConfigPath()const;

	void saveData();
	void loadData();
	void copyData();
	void deleteData();
	bool isExistsSaveData();
	std::string getSaveFilePath(int slotIdx = 0x80000000) const;

	//! @brief セーブデータファイルを返す
	bool  getSaveDataFile(rapidjson::Document& doc)const;
	//! @brief セーブ
	bool save(rapidjson::Document& doc,int slotIdx)const;

	//! @brief セーブデータファイルに保存されているシーンIDを返す
	int getSceneIdFromSaveDataFile()const;

	// データセーブ要求チェック
	void checkRequestSave();

	// データロード要求チェック
	bool checkRequestLoad();

	// データ削除要求チェック
	void checkRequestDelete();

	// データコピー要求チェック
	void checkRequestCopy();

	// ロードデータを反映
	void attachLoadData();

	// シリアライズする。
	void serialize();

	// デシリアライズする。
	void deserialize();

	//シーンの先読み込み
	void preloadSceneData(agtk::data::SceneData *scene);
	//リソース（画像、サウンド）をロードする。
	void loadResources(agtk::data::SceneData *scene);
	void loadResources(int sceneId);
// #AGTK-NX
#ifdef USE_CLEAR_UNUSED_TEX
	std::map<std::string, int> _markedFilenameList;
	void markLoadResources();
	void unmarkLoadResources();
#endif
	void clearLoadResources(agtk::data::SceneData *scene);
	void clearLoadResources(int sceneId);
	void clearAllLoadResources();
	//リソースロード状態をチェックする。
	enum EnumStateLoadResources { NONE, LOAD, END, };
	EnumStateLoadResources getStateLoadResouces() { return _stateLoadResources; }
	//リソース更新
	void updateLoadResources();
	EnumStateLoadResources _stateLoadResources;
	std::map<std::string, int> _endLoadResources;

	//指定のインスタンスIDのオブジェクト取得
	agtk::Object* getTargetObjectByInstanceId(int instanceId);

	//指定のオブジェクトIDのオブジェクトリスト取得
	cocos2d::__Array *getTargetObjectListByObjectId(int objectId);

	// ロックしたオブジェクトリスト取得
	cocos2d::__Array *getTargetObjectLocked(agtk::Object* _object, int sceneLayerType = -1);

	// スイッチ、変数のデータリスト取得
	void getSwitchVariableDataList(int qualifierId, int objectId, int dataId, bool isSwitch, cocos2d::__Array *out);

	// スイッチの条件チェック
	bool checkSwitchCondition(agtk::data::SwitchVariableConditionData::EnumSwitchValueType type, bool value, agtk::data::PlaySwitchData::EnumState state);

	// 変数の条件チェック
	bool checkVariableCondition(agtk::data::SwitchVariableConditionData::EnumCompareValueType type, double srcValue, agtk::data::SwitchVariableConditionData::EnumCompareOperator op, double constantValue, int qualifierId, int objectId, int variableId);

	// スイッチデータの設定
	void assignSwitchData(agtk::data::SwitchVariableAssignData::EnumSwitchAssignValue assignValue, agtk::data::PlaySwitchData *switchData);

	// 変数データの設定
	void assignVariableData(agtk::data::SwitchVariableAssignData::EnumVariavleAssignValue op, agtk::data::PlayVariableData *variableData, double value);

	// スイッチ・変数の変更処理
	enum EnumPlaceType {
		kPlaceCourse,
		kPlacePortal,
		kPlaceTransitionLink,
		kPlaceMax
	};
	void calcSwichVariableChange(cocos2d::__Array *switchVariableAssginList, EnumPlaceType placeType, int id1, int id2, int id3);

	// スイッチの値を取得する。0 = false, 1 = true, 2 = スイッチが見つからなかった。
	int getSwitchValue(int switchObjectId, int switchQualifierId, int switchId);

	// 変数の値を取得する。取得値はretValueに設定、見つかったかの判定は戻り値に設定。
	bool getVariableValue(int variableObjectId, int variableQualifierId, int variableId, double &retValue, agtk::Object* _object = nullptr);

	// オブジェクトのアクションプログラム実行処理
	void calcObjectActionProgram(cocos2d::__Array *objectActionList);

	// BGM演出処理
	void calcMoveBgmEffect(float duration, agtk::data::EnumBgmChangeType changeType, bool isFadeOut, const std::vector<BgmInfo>& bgmInfo, bool isPreMove);

public:
	// 物理演算イテレーション回数取得
	int getPhysicsIterations();

	// 物理演算イテレーション回数設定
	void setPhysicsIterations(int iterations = 10);

	// 重力設定
	void setGravity(float gravity, float rotation);

	// 更新
	void updatePhysics(float dt);

	// デバッグ表示ON/OFF
	void setPhysicsDebugVisible(bool flg);

	//翻訳データ取得
	static const char *tr(const char *sourceText, const char *disambiguation = "");

protected:
	//翻訳データ
	struct LocaleText {
	public:
		LocaleText(const char *_locale, const char *_text) {
			locale = _locale;
			text = _text;
		}
	public:
		std::string locale;
		std::string text;
	};
	static std::map<std::string, std::map<std::string, std::vector<LocaleText>>> _translationMap;

#ifdef USE_RUNTIME
	CC_SYNTHESIZE(bool, _encryptSaveFile, EncryptSaveFile);//プレイデータのセーブファイルを暗号化するとき真。
	CC_SYNTHESIZE(bool, _projectFolderSave, ProjectFolderSave);//プロジェクトフォルダー内にセーブファイルを作成するとき真。
#endif

private:
	// =============================================================================
	// 物理関係
	// =============================================================================
	// 物理用
	CC_SYNTHESIZE_READONLY(cocos2d::PhysicsWorld *, _physicsWorld, PhysicsWorld);// 物理ワールド
	CC_SYNTHESIZE(float, _airResistance, AirResistance);//空気抵抗率(シーンデータから設定される)
	CC_SYNTHESIZE(bool, _alivePhysicsObj, AlivePhysicsObj);//物理ボディ存在中フラグ

private:
	// 衝突開始イベントコールバック
	bool onContactBeginCallback(cocos2d::PhysicsContact &contact);

	// 衝突時の事前解決コールバック
	bool onContactPreSolveCallback(cocos2d::PhysicsContact &contact, cocos2d::PhysicsContactPreSolve &preSolve);

	// 離れたイベントコールバック
	void onContactSeparateCallback(cocos2d::PhysicsContact &contact);

	// 坂の衝突回避チェック
	bool checkPassSlope(agtk::Slope *slope, cocos2d::Vec2 targetPosition);

	// タイルの衝突回避チェック
	bool checkPassTile(PhysicsShapeEdgeSegment *line, cocos2d::Vec2 targetPosition);
// #AGTK-NX
#ifdef USE_SAR_OPTIMIZE_4
public:
	typedef std::multimap<intptr_t, agtk::Object*> SwitchWatcherObjectDic;
	CC_SYNTHESIZE(SwitchWatcherObjectDic*, _switchWatcherObjectList, SwitchWatcherObjectList);
	void addSwitchWatcher(agtk::Object* object, intptr_t switchPtr);
	void removeSwitchWatcher(agtk::Object* object, intptr_t switchPtr);
#endif
// #AGTK-NX
#if (CC_TARGET_PLATFORM == CC_PLATFORM_NX)
#endif
// #AGTK-NX
#ifdef USE_BG_PROJECT_LOAD
	CC_SYNTHESIZE(bool, _enableProject, EnableProject);
#endif
// #AGTK-NX #AGTK-WIN
#ifdef USE_30FPS_3
	CC_SYNTHESIZE(int, _passIndex, PassIndex);
	CC_SYNTHESIZE(int, _passCount, PassCount);
	CC_SYNTHESIZE(bool, _bLastPass, LastPass);
private:
	int _savePassIndex;
	int _savePassCount;
	bool _saveLastPass;
public:
	void saveWallCollisionPass() {
		_savePassIndex = getPassIndex();
		_savePassCount = getPassCount();
		_saveLastPass = getLastPass();
	}
	void restoreWallCollisionPass() {
		setPassIndex(_savePassIndex);
		setPassCount(_savePassCount);
		setLastPass(_saveLastPass);
	}
#endif
};

#ifdef USE_SAR_OHNISHI_DEBUG
extern int g_debugObjectId;
extern int g_debugCurObjectId;
extern int g_debugIntVal;
extern Node* g_debugNode;
#endif

#endif	//__GAME_MANAGER_H__
