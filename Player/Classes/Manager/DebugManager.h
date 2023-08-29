#ifndef __DEBUG_MANAGER_H__
#define	__DEBUG_MANAGER_H__

#include "Lib/Macros.h"
#ifdef USE_PREVIEW
#include "Lib/RenderTexture.h"
#include "Lib/Shader.h"
#endif
#include "Lib/Object.h"
#include "Lib/Portal.h"
#include "Lib/Course.h"
#include "Lib/Slope.h"
#if (CC_TARGET_PLATFORM == CC_PLATFORM_WIN32) || (CC_TARGET_PLATFORM == CC_PLATFORM_MAC)
#include "ImGUI/imgui.h"
#endif
#include "Manager/InputManager.h"

NS_AGTK_BEGIN

//-------------------------------------------------------------------------------------------------------------------
class DebugDeviceController : public cocos2d::Ref
{
private:
	DebugDeviceController();
	virtual ~DebugDeviceController();
public:
	CREATE_FUNC_PARAM(DebugDeviceController, int, deviceId);
	void update(float delta);
	const char *getDeviceName();
	const char *getName();
	bool isConnected();
private:
	virtual bool init(int deviceId);
	void setup();
	void reset();
private:
	CC_SYNTHESIZE(int, _deviceId, DeviceId);
	CC_SYNTHESIZE_RETAIN(cocos2d::__String *, _deviceName, DeviceName);
	CC_SYNTHESIZE_RETAIN(cocos2d::__String *, _name, Name);
	CC_SYNTHESIZE_RETAIN(cocos2d::__Array *, _buttonActionList, ButtonActionList);
};

//-------------------------------------------------------------------------------------------------------------------
class DebugExecuteLogWindow: public cocos2d::Ref
{
private:
	DebugExecuteLogWindow();
	virtual ~DebugExecuteLogWindow();
public:
	CREATE_FUNC(DebugExecuteLogWindow);
	void draw(bool *bOpenFlag);
	void clear();
	void addLog(const char *fmt, ...);
	void addLog(const std::string log);
private:
	virtual bool init();
private:
	CC_SYNTHESIZE_RETAIN(cocos2d::__Array *, _logList, LogList);
	char _filterString[64];
	bool _actionLogConsoleFilterTextInputFocused;
public:
	static int _lineCount;
};

//-------------------------------------------------------------------------------------------------------------------
class DebugShaderData : public cocos2d::Ref
{
private:
	DebugShaderData() {
		_check = false;
		_value = 0.0f;
		_name = nullptr;
	}
	~DebugShaderData() {
		CC_SAFE_RELEASE_NULL(_name);
	}
public:
	CREATE_FUNC_PARAM3(DebugShaderData, int, layerId, agtk::Shader::ShaderKind, kind, const char *, name);
public:
	virtual bool init(int layerId, agtk::Shader::ShaderKind kind, const char *name) {
		this->setLayerId(layerId);
		this->setKind(kind);
		this->setName(cocos2d::__String::create(name));
		switch (this->getKind()) {
		case agtk::Shader::kShaderColorGameboy: this->setId(2); break;
		case agtk::Shader::kShaderBlur: this->setId(1); break;
		case agtk::Shader::kShaderCRTMonitor: this->setId(0); break;
		default: CC_ASSERT(0); break;
		}
		return this->getName() ? true : false;
	}
	const char *getName() { return _name->getCString(); }
private:
	CC_SYNTHESIZE(int, _layerId, LayerId);
	CC_SYNTHESIZE(agtk::Shader::ShaderKind, _kind, Kind);
	CC_SYNTHESIZE_RETAIN(cocos2d::__String *, _name, Name);
	CC_SYNTHESIZE(bool, _check, Check);
	CC_SYNTHESIZE(float, _value, Value);
	CC_SYNTHESIZE(int, _id, Id);
};

//-------------------------------------------------------------------------------------------------------------------
class DebugDisplayData : public cocos2d::Ref
{
private:
	DebugDisplayData();
	virtual ~DebugDisplayData();
public:
	static DebugDisplayData *create(agtk::data::ProjectData *projectData);
#ifdef USE_PREVIEW
	agtk::DebugShaderData *getShaderKind(int layerId, agtk::Shader::ShaderKind kind);
	void setShaderKind(int layerId, agtk::Shader::ShaderKind kind, float value, bool bIgnored);
	agtk::DebugShaderData *getInitShaderKind(int layerId, agtk::Shader::ShaderKind kind);
	void setInitShaderKind(int layerId, agtk::Shader::ShaderKind kind, float value, bool bIgnored);
	agtk::DebugShaderData *getTmpShaderKind(int layerId, agtk::Shader::ShaderKind kind);
	void setTmpShaderKind(int layerId, agtk::Shader::ShaderKind kind, float value, bool bIgnored);
#endif
	const char *getName();
private:
	virtual bool init(agtk::data::ProjectData *projectData);
private:
	CC_SYNTHESIZE_RETAIN(cocos2d::__Dictionary *, _shaderList, ShaderList);
	CC_SYNTHESIZE(bool, _fullScreenFlag, FullScreenFlag);
	CC_SYNTHESIZE(bool, _magnifyWindow, MagnifyWindow);
	CC_SYNTHESIZE(int, _windowMagnification, WindowMagnification);
	CC_SYNTHESIZE(int, _mainLanguageId, MainLanguageId);
	CC_SYNTHESIZE(bool, _disableScreenShake, DisableScreenShake);
// #AGTK-NX
#if (CC_TARGET_PLATFORM == CC_PLATFORM_NX)
#endif
	//初期値
	CC_SYNTHESIZE_RETAIN(cocos2d::__Dictionary *, _initShaderList, InitShaderList);
	CC_SYNTHESIZE(bool, _initFullScreenFlag, InitFullScreenFlag);
	CC_SYNTHESIZE(bool, _initMagnifyWindow, InitMagnifyWindow);
	CC_SYNTHESIZE(int, _initWindowMagnification, InitWindowMagnification);
	CC_SYNTHESIZE(int, _initMainLanguageId, InitMainLanguageId);
	CC_SYNTHESIZE(bool, _initDisableScreenShake, InitDisableScreenShake);
// #AGTK-NX
#if (CC_TARGET_PLATFORM == CC_PLATFORM_NX)
#endif
	//一時保持値
	CC_SYNTHESIZE_RETAIN(cocos2d::__Dictionary *, _tmpShaderList, TmpShaderList);
	CC_SYNTHESIZE(bool, _tmpFullScreenFlag, TmpFullScreenFlag);
	CC_SYNTHESIZE(bool, _tmpMagnifyWindow, TmpMagnifyWindow);
	CC_SYNTHESIZE(int, _tmpWindowMagnification, TmpWindowMagnification);
	CC_SYNTHESIZE(int, _tmpMainLanguageId, TmpMainLanguageId);
	CC_SYNTHESIZE(bool, _tmpDisableScreenShake, TmpDisableScreenShake);
// #AGTK-NX
#if (CC_TARGET_PLATFORM == CC_PLATFORM_NX)
#endif
};

//-------------------------------------------------------------------------------------------------------------------
class DebugSoundData : public cocos2d::Ref
{
private:
	DebugSoundData();
	virtual ~DebugSoundData() {};
public:
	static DebugSoundData *create(float initBgmVolume, float initSeVolume, float initVoiceVolume, float bgmVolume, float seVolume, float voiceVolume);
	void reset(float initBgmVolume, float initSeVolume, float initVoiceVolume);
private:
	virtual bool init(float initBgmVolume, float initSeVolume, float initVoiceVolume, float bgmVolume, float seVolume, float voiceVolume);
public:
	CC_SYNTHESIZE(float, _bgmVolume, BgmVolume);
	CC_SYNTHESIZE(float, _seVolume, SeVolume);
	CC_SYNTHESIZE(float, _voiceVolume, VoiceVolume);
	CC_SYNTHESIZE(float, _initBgmVolume, InitBgmVolume);
	CC_SYNTHESIZE(float, _initSeVolume, InitSeVolume);
	CC_SYNTHESIZE(float, _initVoiceVolume, InitVoiceVolume);
	CC_SYNTHESIZE(float, _tmpBgmVolume, TmpBgmVolume);
	CC_SYNTHESIZE(float, _tmpSeVolume, TmpSeVolume);
	CC_SYNTHESIZE(float, _tmpVoiceVolume, TmpVoiceVolume);
};

//-------------------------------------------------------------------------------------------------------------------
class DebugObjectInfoWindow : public cocos2d::Ref
{
private:
	DebugObjectInfoWindow();
	virtual ~DebugObjectInfoWindow();
public:
	CREATE_FUNC_PARAM2(DebugObjectInfoWindow, agtk::Object *, object, cocos2d::Vec2, pos);
	void draw();
	void update();
public:
	void setSwitchListFromPlaySwitchData();//switchListにplaySwitchDataの情報をセット
	void setPlaySwitchDataFromSwitchList();//playSwitchDataにswitchListの情報をセット
	void setVariableListFromPlayVariableData();//variableListにplayVariableDataの情報をセット
	void setPlayVariableDataFromVariableList();//playVariableDataにvariableListの情報をセット
private:
	virtual bool init(agtk::Object *object, cocos2d::Vec2 pos);
private:
	CC_SYNTHESIZE_RETAIN(agtk::Object *, _object, Object);
	CC_SYNTHESIZE(bool, _displayFlag, DisplayFlag);
	CC_SYNTHESIZE(cocos2d::Vec2, _position, Position);
	bool *_switchList;
	int _switchListCount;
	double *_variableList;
	int _variableListCount;
};

//-------------------------------------------------------------------------------------------------------------------
class DebugResourcesCommonWindow : public cocos2d::Ref
{
private:
	DebugResourcesCommonWindow();
	virtual ~DebugResourcesCommonWindow();
public:
	CREATE_FUNC(DebugResourcesCommonWindow);
	void draw();
public:
	void setSwitchListFromPlaySwitchData();//switchListにplaySwitchDataの情報をセット
	bool setPlaySwitchDataFromSwitchList();//playSwitchDataにswitchListの情報をセット
	void setVariableListFromPlayVariableData();//variableListにplayVariableDataの情報をセット
	bool setPlayVariableDataFromVariableList();//playVariableDataにvariableListの情報をセット
	void updatePlayVariableSwitchDataList();
private:
	virtual bool init();
private:
	CC_SYNTHESIZE(bool, _displayFlag, DisplayFlag);
	bool *_switchList;
	int _switchListCount;
	double *_variableList;
	int _variableListCount;
};

//-------------------------------------------------------------------------------------------------------------------
class DebugPerformanceAndSpeedSettingsWindow : public cocos2d::Ref
{
public:
	enum State {
		kStateScenePlay,//再生
		kStateScenePause,//一時停止
		kStateSceneFrameByFrame,//コマ送り
	};
private:
	DebugPerformanceAndSpeedSettingsWindow() {
		_frameRate = FRAME60_RATE;
		_state = kStateScenePlay;
// #AGTK-NX
#if (CC_TARGET_PLATFORM == CC_PLATFORM_NX)
#endif
	}
	virtual ~DebugPerformanceAndSpeedSettingsWindow() {}
public:
	CREATE_FUNC(DebugPerformanceAndSpeedSettingsWindow);
	void draw();
// #AGTK-NX
#if (CC_TARGET_PLATFORM == CC_PLATFORM_NX)
#endif
private:
	virtual bool init();
private:
	CC_SYNTHESIZE(int, _frameRate, FrameRate);//フレームレート
	CC_SYNTHESIZE(State, _state, State);//状態（再生、一時停止、コマ送り）
	CC_SYNTHESIZE(bool, _displayFlag, DisplayFlag);
// #AGTK-NX
#if (CC_TARGET_PLATFORM == CC_PLATFORM_NX)
	CC_SYNTHESIZE(int, _gameSpeed, GameSpeed);//ゲームスピード
	State _prevState;
	bool _bPause;
#endif
};

// #AGTK-NX
#if (CC_TARGET_PLATFORM == CC_PLATFORM_NX)
#endif
//-------------------------------------------------------------------------------------------------------------------
class DebugManager : public cocos2d::Ref
{
private:
	DebugManager();
	virtual ~DebugManager();
	static DebugManager *_debugManager;
private:
	virtual bool init();
	bool initFont();
	void showMainMenuBar();//メインバー
	void showSoundWindow();//サウンドウインドウ
	void showMethodOfOperationWindow();//操作方法ウインドウ
	bool showEditOperationWindow(int buttonId);//操作編集ウインドウ
	void showGameDisplayWindow();//ゲーム画面ウインドウ
// #AGTK-NX
#if (CC_TARGET_PLATFORM == CC_PLATFORM_NX)
#endif
	void showGameInformationWindow();//ゲーム情報ウインドウ
	void showExecuteLogWindow();//実行ログウインドウ
	void showFramerateWindow();//フレームレートウインドウ
#ifdef USE_PREVIEW
	void showInputState();	//入力状態表示ウィンドウ
#endif
#if !defined(USE_RUNTIME)
	void showChangeSceneWindow();//シーン切り替えウインドウ
#endif
#if defined(USE_PREVIEW) && defined(AGTK_DEBUG)
	void showWebSocketWindow();//Websocketウインドウ（デバッグ用）
	void showMovieWindow();//動画ウインドウ
#endif
	void showDebugObjectInfoWindow();
	void showResourcesCommonWindow();
	void showPerformanceAndSpeedSettingsWindow();
	void draw();
	void pause(cocos2d::Node *node);
	void resume(cocos2d::Node *node);
public:
	static DebugManager *getInstance();
	static void purge();
	void update(float delta);
	void start(cocos2d::Layer *layer);
	void end();
	void reset();
// #AGTK-NX
#if defined(USE_PREVIEW) || ((CC_TARGET_PLATFORM == CC_PLATFORM_NX) && defined(AGTK_DEBUG))
//#ifdef USE_PREVIEW
	void setVisibleCollisionArea(agtk::data::TimelineInfoData::EnumTimelineType type, bool bVisible);
#endif
	int getSceneId(int selectSceneId = -1);
	int findSelectSceneId(std::string name);
	int findSelectSceneId(int sceneId);
	void createObjectInfoWindow(agtk::Object *object, cocos2d::Vec2 pos = cocos2d::Vec2::ZERO);
	void removeObjectInfoWindow(agtk::Object *object);
	DebugObjectInfoWindow *getObjectInfoWindow(agtk::Object *object);
	void showObjectInfoWindows();
	void showPortalView(bool isShow);
	void showTileWallView(bool isShow);
#ifdef USE_PREVIEW
	void showPartOthersView(bool isShow);
	void showLimitArea(bool bShow);
	void showLimitCamera(bool bShow);
#endif
#ifdef USE_PREVIEW
	void resetChangeShader();
	void changeShader();
	void changeShader(int layerId, agtk::DebugDisplayData *displayData);
#endif
	void createDisplayData();
	void removeDisplayData();
	void createGridLine();
	void removeGridLine();
private:
#if defined(AGTK_DEBUG)
public:
	enum MovieState { None, Play, Stop, Pause };
	std::function<void(MovieState)> onMovieState;
#endif
public:
	static void setFontName(const std::string &fontName);
	void saveDebugSetting();
	void loadDebugSetting();
#if defined(USE_RUNTIME)
	void saveDebugSettingForRuntime();
	void loadDebugSettingForRuntime();
#endif
private:
	CC_SYNTHESIZE_RETAIN(cocos2d::Layer *, _layer, Layer);
	CC_SYNTHESIZE_RETAIN(agtk::DebugSoundData *, _soundData, SoundData);
	CC_SYNTHESIZE_RETAIN(agtk::DebugDisplayData *, _displayData, DisplayData);
#ifdef USE_PREVIEW
	CC_SYNTHESIZE_RETAIN(cocos2d::RenderTexture *, _renderTextureDebug, RenderTextureDebug);
#endif
	CC_SYNTHESIZE(bool, _bShowMenuBar, ShowMenuBar);
	bool _bShowSoundWindow;
	bool _bShowMethodOfOperationWindow;
	bool _bShowEditOperationWindow;
	bool _bShowGameDisplayWindow;
	bool _bShowGameInformationWindow;
#if !defined(USE_RUNTIME)
	bool _bShowChangeSceneWindow;
#endif
	bool _bShowDebugFrameRate;//cocos2d用のデバッグ表示（フレームレート）
#if defined(AGTK_DEBUG)
	bool _bShowWebSocketWindow;
	bool _bShowMovieWindow;
#endif
	bool _bForcedClosePopupModal;
	CC_SYNTHESIZE_READONLY(bool, _showDebugObjectInfoWindow, ShowDebugObjectInfoWindow);
	CC_SYNTHESIZE_READONLY(bool, _showDebugNormalSceneObjectInfoFlag, ShowDebugNormalSceneObjectInfoFlag);
	CC_SYNTHESIZE_READONLY(bool, _showDebugMenuSceneObjectInfoFlag, ShowDebugMenuSceneObjectInfoFlag);
	char _showDebugObjectName[64];
	bool _showDebugObjectNameFocused;

	CC_SYNTHESIZE(bool, _bShowDebugDispGrid, ShowDebugDispGrid);
	CC_SYNTHESIZE(bool, _debugDisableScreenShake, DebugDisableScreenShake);
	bool _running;
#if defined(USE_PREVIEW)
	bool _bReloadProjectData;
#endif
	int _selectedDeviceId;//0:選択なし、n>=1:デバイスID
	int _selectedEditOperationType;//0:テンプレートから選択 1:コントローラー入力
	int _selectedDeviceKeyCodeId;
#if (CC_TARGET_PLATFORM == CC_PLATFORM_NX)
#endif
	CC_SYNTHESIZE_RETAIN(cocos2d::__Dictionary *, _initInputMappingList, InitInputMappingList);
	CC_SYNTHESIZE_RETAIN(cocos2d::__Dictionary *, _inputMappingList, InputMappingList);
#if defined(AGTK_DEBUG)
	CC_SYNTHESIZE_RETAIN(cocos2d::__Array *, _logList, LogList);
	CC_SYNTHESIZE_RETAIN(cocos2d::__Array *, _folderList, FolderList);
#endif
	CC_SYNTHESIZE_READONLY(bool, _collisionWallEnabled, CollisionWallEnabled);
	CC_SYNTHESIZE_READONLY(bool, _collisionHitEnabled, CollisionHitEnabled);
	CC_SYNTHESIZE_READONLY(bool, _collisionAttackEnabled, CollisionAttackEnabled);
	CC_SYNTHESIZE_READONLY(bool, _collisionConnectionEnabled, CollisionConnectionEnabled);
	CC_SYNTHESIZE_READONLY(bool, _logConsoleDisplayEnabled, LogConsoleDisplayEnabled);
	CC_SYNTHESIZE_READONLY(bool, _freeMovingEnabled, FreeMovingEnabled);
	CC_SYNTHESIZE_READONLY(bool, _invincibleModeEnabled, InvincibleModeEnabled);//無敵モード
	CC_SYNTHESIZE_READONLY(bool, _frameRateDisplayEnabled, FrameRateDisplayEnabled);
	CC_SYNTHESIZE_READONLY(bool, _skipOneFrameWhenSceneCreatedIgnored, SkipOneFrameWhenSceneCreatedIgnored);//シーン生成時のスキップを無効
	CC_SYNTHESIZE_READONLY(bool, _debugForDevelopment, DebugForDevelopment);//デバッグフラグ
	CC_SYNTHESIZE_READONLY(bool, _showPhysicsBoxEnabled, ShowPhysicsBoxEnabled);//物理用表示
	CC_SYNTHESIZE_READONLY(bool, _showPortalFlag, ShowPortalFlag);//ポータル表示
	CC_SYNTHESIZE_READONLY(bool, _showTileWallFlag, ShowTileWallFlag);//タイル壁表示
	CC_SYNTHESIZE_READONLY(bool, _showPartOthersFlag, ShowPartOthersFlag);//その他パーツ表示
	CC_SYNTHESIZE_READONLY(bool, _showLimitAreaFlag, ShowLimitAreaFlag);//行動範囲制限表示
	CC_SYNTHESIZE_READONLY(bool, _showLimitCameraFlag, ShowLimitCameraFlag);//カメラの表示範囲制限
#if !defined(USE_RUNTIME)
	CC_SYNTHESIZE_READONLY(bool, _fixFramePerSecondFlag, FixFramePerSecondFlag);//FPS固定フラグ
#endif
	CC_SYNTHESIZE_READONLY(bool, _showLoadingScene, ShowLoadingScene);//ロード画面フラグ
	CC_SYNTHESIZE(bool, _pause, Pause);
	CC_SYNTHESIZE_RETAIN(DebugExecuteLogWindow *, _debugExecuteLogWindow, DebugExecuteLogWindow);
	CC_SYNTHESIZE_RETAIN(cocos2d::__Array *, _debugGridLineList, DebugGridLineList);
private:
	CC_SYNTHESIZE_RETAIN(cocos2d::__Array *, _deviceControllerList, DeviceControllerList);
	CC_SYNTHESIZE_RETAIN(cocos2d::__Array *, _objectInfoWindowList, ObjectInfoWindowList);
	CC_SYNTHESIZE_RETAIN(agtk::DebugResourcesCommonWindow *, _debugResourcesCommonWindow, DebugResourcesCommonWindow);
	CC_SYNTHESIZE_RETAIN(agtk::DebugPerformanceAndSpeedSettingsWindow *, _debugPerformanceAndSpeedSettingsWindow, DebugPerformanceAndSpeedSettingsWindow);
	CC_SYNTHESIZE(int, _selectSceneId, SelectSceneId);
	CC_SYNTHESIZE(bool, _selectPhysicsIterationNormal, SelectPhysicsIterationNormal);//物理演算イテレーション回数：通常
	CC_SYNTHESIZE(bool, _selectPhysicsIterationTwice, SelectPhysicsIterationTwice);//物理演算イテレーション回数：二倍
	CC_SYNTHESIZE(bool, _selectPhysicsIterationThreeTimes, SelectPhysicsIterationThreeTimes);//物理演算イテレーション回数：三倍
#ifdef USE_PREVIEW
	CC_SYNTHESIZE_RETAIN(cocos2d::__String *, _appNameVersion, AppNameVersion);
#endif
	static std::string mFontName;
// #AGTK-NX
#if (CC_TARGET_PLATFORM == CC_PLATFORM_NX)
#endif
};

bool IsFullScreen();
void ChangeScreen(bool bFullScreen);
void RestoreScreen();
void FocusWindow();
cocos2d::Size GetScreenResolutionSize();
cocos2d::Size GetFrameSize();
void ChangeScreenResolutionSize(cocos2d::Size designResolutionSize, float magnifyWindow = 1.0f);
void SetCursorVisible(bool bVisible);

NS_AGTK_END

#endif	//__DEBUG_MANAGER_H__
