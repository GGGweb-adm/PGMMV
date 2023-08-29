#ifndef __GAME_SCENE_H__
#define __GAME_SCENE_H__

#include "Lib/Macros.h"
#include "Lib/BaseLayer.h"
#include "Lib/Scene.h"
#if (CC_TARGET_PLATFORM == CC_PLATFORM_WIN32) || (CC_TARGET_PLATFORM == CC_PLATFORM_MAC)
#include "ImGUI/imgui.h"
#endif
// #AGTK-NX
#if (CC_TARGET_PLATFORM == CC_PLATFORM_NX)
#endif

class AGTKPLAYER_API GameScene : public agtk::BaseLayer
{
public:
	enum SceneChangeState {
		NONE,//未演出
		PREMOVE,//切り替え前演出中
		POSTMOVE,//切り替え後演出中
		LOADING//ロード演出中
	};
public:
	GameScene();
	virtual ~GameScene();
	virtual bool init(int id);
	virtual void onEnter();
	virtual void onEnterTranslationDidFinish();
	virtual void onExit();
	void initCurSceneLinkDataList(int sceneId);
	static void setFontName(const std::string &fontName);
public:
	CREATE_FUNC_PARAM(GameScene, int, id);

	CC_SYNTHESIZE(bool, _isRestartCanvas, IsRestartCanvas);

// #AGTK-NX
#if (CC_TARGET_PLATFORM == CC_PLATFORM_NX)
#endif
protected:
	virtual void update(float delta);
	void debugGUI();
	agtk::Scene *getScene();
	// シーン遷移条件チェック
	bool checkSceneLinkCondition();
	// カレントシーンのリンクデータリスト更新
	void updateCurSceneLinkDataList();
	// 切り替え演出処理
	void calcMoveEffect(bool isPreMove, bool isPortal, bool needSceneChange);
	// データロード演出処理
	void calcLoadDataEffect();
	// クローズ
	void close();
private:
#ifdef FIX_ACT2_5233
	void RenderSceneToFadeSprite(cocos2d::Camera *camera, cocos2d::RenderTexture *targetFadeSprite, cocos2d::RenderTexture *targetFadeFrontSprite, int objectLayerId, agtk::Scene *scene, cocos2d::Renderer *renderer, agtk::data::EnumMoveEffect effectType = agtk::data::EnumMoveEffect::kMoveEffectNone);
#else
	void RenderSceneToFadeSprite(cocos2d::Camera *camera, cocos2d::RenderTexture *targetFadeSprite, agtk::Scene *scene, cocos2d::Renderer *renderer, agtk::data::EnumMoveEffect effectType = agtk::data::EnumMoveEffect::kMoveEffectNone);
#endif
private:
	CC_SYNTHESIZE(bool, _debugFlag, DebugFlag);
	void debugInit();
	void debugDraw();
	void debugDrawMainMenuBar();
	bool _bDebugDisplay;
	bool _bShowFrameRate;
	CC_SYNTHESIZE_RETAIN(cocos2d::__Array *, _logList, LogList);
	int _selectSceneId;
private:
	unsigned int _counter;
	float _seconds;

	// カレントシーンのリンクデータリスト
	CC_SYNTHESIZE_RETAIN(cocos2d::__Array *, _curSceneLinkDataList, CurSceneLinkDataList);

	// フェード用スプライト
	CC_SYNTHESIZE_RETAIN(cocos2d::RenderTexture *, _fadeSpriteBefore, FadeSpriteBefore);	// 遷移前のスプライト
	CC_SYNTHESIZE_RETAIN(cocos2d::RenderTexture *, _fadeSpriteAfter, FadeSpriteAfter);		// 遷移後のスプライト
#ifdef FIX_ACT2_5233
	CC_SYNTHESIZE_RETAIN(cocos2d::RenderTexture *, _fadeFrontSpriteBefore, FadeFrontSpriteBefore);	// 遷移前の手前側スプライト
	CC_SYNTHESIZE_RETAIN(cocos2d::RenderTexture *, _fadeFrontSpriteAfter, FadeFrontSpriteAfter);		// 遷移後の手前側スプライト
#endif

	// シーン変更ステート
	CC_SYNTHESIZE_READONLY(SceneChangeState, _sceneChangeState, SceneChangeState);

	// 対象となるカレントシーンリンクデータのIDX
	CC_SYNTHESIZE(int, _targetSceneLinkDataIdx, TargetSceneLinkDataIdx);

	// 切り替え後の初期位置のグループIDX
	CC_SYNTHESIZE(int, _startPointGroupIdx, StartPointGroupIdx);

	// レイヤーカラー
	CC_SYNTHESIZE_RETAIN(cocos2d::LayerColor *, _layerColor, LayerColor);

	// 遷移前のカメラの座標
	CC_SYNTHESIZE(cocos2d::Vec2, _preSceneCameraPos, PreSceneCameraPos);

	// 遷移前のカメラのスケール
	CC_SYNTHESIZE(cocos2d::Vec2, _preSceneCameraScale, PreSceneCameraScale);

	// 遷移前のカメラのクオータニオン
	CC_SYNTHESIZE(cocos2d::Quaternion, _preSceneCameraRotationQuat, PreSceneCameraRotationQuat);

	// 移動用オブジェクトリスト
	CC_SYNTHESIZE_RETAIN(cocos2d::__Array *, _moveObjectList, MoveObjectList);

#ifdef USE_PREVIEW
	CC_SYNTHESIZE(bool, _paused, Paused);
#endif

	// 移動用オブジェクト移動時間
	CC_SYNTHESIZE(float, _moveObjectDuration, MoveObjectDuration);

	// 移動オブジェクト用の親ノード
	CC_SYNTHESIZE_RETAIN(cocos2d::Node *, _moveObjectParent, MoveObjectParent);
	CC_SYNTHESIZE_RETAIN(agtk::Timer *, _timer, Timer);

	// ロード時のフェード用カメラ位置
	CC_SYNTHESIZE(cocos2d::Vec2, _loadDataFadingCameraPos, LoadDataFadingCameraPos);
#ifdef USE_DESIGNATED_RESOLUTION_RENDER
	CC_SYNTHESIZE_RETAIN(cocos2d::RenderTexture *, _renderTexture, RenderTexture);
#endif
	CC_SYNTHESIZE(bool, _initialCalcMoveEffect, InitialCalcMoveEffect);

	static std::string mFontName;

// #AGTK-NX
#ifdef USE_AGTK
	std::chrono::steady_clock::time_point _lastFrameRateDownTime;
	std::chrono::steady_clock::time_point _lastFrameRateDown20Time;
#endif

private:
	/* スライド連結演出用データクラス */
	class SlideLinkData : public cocos2d::Ref
	{
	private:
		SlideLinkData()
		{
			_moveTarget = nullptr;
			_moveStartPos = Vec2::ZERO;
			_moveEndPos = Vec2::ZERO;
			_moveDurationMax = 0.0f;
			_guiList = nullptr;
		}
		virtual ~SlideLinkData() {
			CC_SAFE_RELEASE_NULL(_moveTarget);
			CC_SAFE_RELEASE_NULL(_guiList);
		}
		virtual bool init(agtk::Player * target, cocos2d::Vec2 moveStartPos, cocos2d::Vec2 moveEndPos)
		{
			this->setMoveTarget(target);
			this->setMoveStartPos(moveStartPos);
			this->setMoveEndPos(moveEndPos);
			this->setGuiList(cocos2d::__Array::create());
			return true;
		}
	public:
		CREATE_FUNC_PARAM3(SlideLinkData, agtk::Player *, target, cocos2d::Vec2, moveStartPos, cocos2d::Vec2, moveEndPos);
	private:
		CC_SYNTHESIZE_RETAIN(agtk::Player *, _moveTarget, MoveTarget);	// 演出用オブジェクト
		CC_SYNTHESIZE(cocos2d::Vec2, _moveStartPos, MoveStartPos);		// 演出開始時の座標
		CC_SYNTHESIZE(cocos2d::Vec2, _moveEndPos, MoveEndPos);			// 演出終了時の座標
		CC_SYNTHESIZE(cocos2d::Quaternion, _startRotQuat, StartRotQuat);// 演出開始時のクオータニオン

		CC_SYNTHESIZE(cocos2d::Vec2, _deltaPos, DeltaPos);				// 演出終了までの座標差分
		CC_SYNTHESIZE(cocos2d::Vec2, _deltaScale, DeltaScale);			// 演出終了までのスケール差分
		CC_SYNTHESIZE(float, _moveDurationMax, MoveDurationMax);		// 演出時間最大値]
	private:
		CC_SYNTHESIZE_RETAIN(cocos2d::__Array *, _guiList, GuiList);
	};
};

#endif // __GAME_SCENE_H__
