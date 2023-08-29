#ifndef __CAMERA_H__
#define	__CAMERA_H__

#include "Lib/Macros.h"
#include "Lib/Object.h"
#include "json/document.h"
#include "Data/SceneData.h"

NS_AGTK_BEGIN

//-------------------------------------------------------------------------------------------------------------------
class AGTKPLAYER_API CameraVec2 : public agtk::EventTimer
{
private:
	CameraVec2();
	virtual ~CameraVec2();
public:
	CREATE_FUNC_PARAM(CameraVec2, cocos2d::Vec2, value);
private:
	virtual bool init(cocos2d::Vec2 value);
public:
	cocos2d::Vec2 setValue(cocos2d::Vec2 pos, float seconds = 0.0f);
	cocos2d::Vec2 addValue(cocos2d::Vec2 pos, float seconds = 0.0f);
	cocos2d::Vec2 getValue() const { return _value; }
	cocos2d::Vec2 getNextValue() { return _nextValue; }
	bool isChanged() { return _value != _oldValue ? true : false; }
private:
	cocos2d::Vec2 _value;
	cocos2d::Vec2 _nextValue;
	cocos2d::Vec2 _prevValue;
	cocos2d::Vec2 _oldValue;
};

//-------------------------------------------------------------------------------------------------------------------
// カメラの回転用クラス
class AGTKPLAYER_API CameraRotation : public agtk::EventTimer
{
private:
	CameraRotation();
	virtual ~CameraRotation();
public:
	CREATE_FUNC_PARAM(CameraRotation, float, value);
private:
	virtual bool init(float value);
public:
	float setValue(float value, float seconds = 0.0f);
	float addValue(float value, float seconds = 0.0f);
	float getValue() { return _value; }
	float getPreValue() { return _prevValue; }
	bool isChanged() { return _value != _oldValue ? true : false; }
private:
	float _value;
	float _nextValue;
	float _prevValue;
	float _oldValue;
};

//-------------------------------------------------------------------------------------------------------------------
class AGTKPLAYER_API Camera : public cocos2d::Ref
{
public:
	enum CameraFollowType{
		kCameraFollowNone,
		kCameraFollowXonly,
		kCameraFollowYonly,
	};

	// カメラの追従する対象の種別
	enum CameraTargetType {
		kCameraTargetNone,		// 追従しない
		kCameraTargetObject,	// オブジェクトを追従
	};

public:
	Camera();
	virtual ~Camera();
public:
	CREATE_FUNC_PARAM(Camera, agtk::Scene *, scene);
	void start();
	void end();
	virtual void update(float dt);
	cocos2d::Vec2 addPosition(cocos2d::Vec2 pos, float duration = 0.0f);
	cocos2d::Vec2 addPosition(float x, float y, float duration = 0.0f);
	cocos2d::Vec2 setPosition(cocos2d::Vec2 pos, float duration = 0.0f);
	cocos2d::Vec2 setPosition(float x, float y, float duration = 0.0);
	cocos2d::Vec2 getPosition() const;
	cocos2d::Vec2 getPosition2() const;
	cocos2d::Vec2 getPositionForZoom();
	cocos2d::Vec2 getNodePosition() { return _position; }
	void setZoom(cocos2d::Vec2 target, cocos2d::Vec2 scale, float duration = 0.0f);
#ifdef FIX_ACT2_4471
	void setCommandZoom(cocos2d::Vec2 scale, float duration = 0.0f);
#endif
	cocos2d::Vec2 getZoom();
	cocos2d::Vec2 getScale();

	void setDisplaySize(float width, float height); // カメラの表示サイズを設定する

public:
	cocos2d::Vec2 getPositionCameraFromScene(cocos2d::Vec2 pos);
	cocos2d::Vec2 getPositionSceneFromCamera(cocos2d::Vec2 pos);
	bool isPositionScreenWithinCamera(cocos2d::Vec2 pos);
	bool isPositionScreenWithinCamera(cocos2d::Vec2 pos, cocos2d::Vec2 screenSizeOffset);
	bool isPositionScreenWithinCamera(cocos2d::Rect rect);
	bool isPositionScreenWithinCamera(cocos2d::Rect rect, cocos2d::Vec2 screenSizeOffset);
public:
	void setTargetObject(agtk::Object *object);
	void setTargetObject(agtk::Object *object, bool isChangeCamera);
	agtk::Object *getTargetObject() { return _targetObject; };
	void resetTargetObject(agtk::Object *object);

	void setTargetPosition(cocos2d::Vec2 position, bool isInit);
	void setTargetPosition(cocos2d::Vec2 position, bool isChangeCamera, bool isInit);
	cocos2d::Vec2 getTargetPosition() { return _targetPosition; }

	void setTargetFixedPosition(cocos2d::Vec2 position);
	void setTargetFixedPosition(cocos2d::Vec2 position, bool isChangeCamera);

	cocos2d::Vec2 getCameraCenterPos();
	void showDebugMoveArea(bool bShow);
	bool isUseClipping();
	bool isAutoScroll();
	// カメラの表示範囲制限の無効チェックする。
	bool isDisableLimitCamera();
private:
	virtual bool init(agtk::Scene *scene);
	cocos2d::Rect calcMoveArea(cocos2d::Vec2 pos, cocos2d::Vec2 scale);
	cocos2d::Rect updateMoveArea(cocos2d::Vec2 pos, cocos2d::Vec2 scale);
	cocos2d::Vec2 calcCameraPoint(cocos2d::Vec2 pos, cocos2d::Vec2 scale);
	cocos2d::Vec2 updateCameraPoint(cocos2d::Vec2 pos, cocos2d::Vec2 scale, bool& bUpdate);
	cocos2d::Vec2 updateCameraPointAutoScroll(cocos2d::Vec2 pos, cocos2d::Vec2 scale, bool& bUpdate);

	// カメラの拡大率、表示比率を考慮してレイヤーの位置を更新する
	void updateLayerPosition(cocos2d::Vec2 cameraPoint, float duration);
	void updateCameraScale(float dt);
#ifdef FIX_ACT2_4471
	void updateCommandScale(float dt);
#endif

private:
	CameraFollowType _type;
	CC_SYNTHESIZE(float, _moveDuration, MoveDuration); // カメラの座標、レイヤーの座標が移動する際の座標を補間する秒数

	CC_SYNTHESIZE(CameraTargetType, _targetType, TargetType);
	CC_SYNTHESIZE_RETAIN(cocos2d::Camera *, _camera, Camera);
	CC_SYNTHESIZE(cocos2d::Rect, _limitAreaRect, LimitAreaRect);
	CC_SYNTHESIZE(cocos2d::Size, _limitMoveArea, LimitMoveArea);
	CC_SYNTHESIZE(cocos2d::Rect, _moveArea, MoveArea);
	CC_SYNTHESIZE_RETAIN(cocos2d::Layer *, _layer, Layer);
	CC_SYNTHESIZE_RETAIN(cocos2d::__Dictionary *, _layerList, LayerList);
	CC_SYNTHESIZE_RETAIN(cocos2d::ActionInterval *, _autoAction, AutoAction);
	CC_SYNTHESIZE_RETAIN(agtk::data::SceneData *, _sceneData, SceneData);
	CC_SYNTHESIZE_RETAIN(cocos2d::Node *, _followNode, FollowNode);
	CC_SYNTHESIZE(bool, _ignoreCorrectionPos, IgnoreCorrectionPos);
	CC_SYNTHESIZE(cocos2d::Vec2, _halfScreenSize, HalfScreenSize);
	CC_SYNTHESIZE(cocos2d::Vec2, _fullScreenSize, FullScreenSize);
	agtk::Scene *_scene;
	cocos2d::Vec2 _position;
	cocos2d::Vec2 _oldPosition;

	agtk::Object* _targetObject;
	cocos2d::Vec2 _targetObjectPosition;
	cocos2d::Vec2 _targetPosition;
	bool _setTargetObjectFlag;

	CC_SYNTHESIZE_RETAIN(agtk::CameraVec2 *, _layerPosition, LayerPosition);
	CC_SYNTHESIZE_RETAIN(agtk::CameraVec2 *, _cameraPosition, CameraPosition);
	CC_SYNTHESIZE_RETAIN(agtk::CameraVec2 *, _cameraPosition2, CameraPosition2);//スケール値を考慮しないポジション
	CC_SYNTHESIZE_RETAIN(agtk::CameraVec2 *, _cameraScale, CameraScale);
#ifdef FIX_ACT2_4471
	CC_SYNTHESIZE_RETAIN(agtk::CameraVec2 *, _commandScale, CommandScale);
#endif
	CC_SYNTHESIZE_RETAIN(agtk::CameraRotation *, _cameraRotationZ, CameraRotationZ);
	CC_SYNTHESIZE_RETAIN(agtk::CameraRotation *, _cameraRotationX, CameraRotationX);
	CC_SYNTHESIZE_RETAIN(agtk::CameraRotation *, _cameraRotationY, CameraRotationY);
	CC_SYNTHESIZE(cocos2d::Size, _screenSize, ScreenSize);
	CC_SYNTHESIZE(cocos2d::Size, _sceneSize, SceneSize);
	CC_SYNTHESIZE(cocos2d::Size, _displaySize, DisplaySize);
	CC_SYNTHESIZE(cocos2d::Vec2, _displaySizeRatio, DisplaySizeRatio); // スクリーンのサイズとカメラの表示サイズとの比率

	CC_SYNTHESIZE_RETAIN(cocos2d::Camera *, _menuCamera, MenuCamera);//メニューカメラ(cocos2d::CameraFlag::USER1)
	CC_SYNTHESIZE_RETAIN(cocos2d::Camera *, _topMostCamera, TopMostCamera);//最前面カメラ(cocos2d::CameraFlag::USER2)
	CC_SYNTHESIZE_RETAIN(cocos2d::Sprite *, _debugMoveAreaSprite, DebugMoveAreaSprite);

	CC_SYNTHESIZE(cocos2d::Vec2, _initialOffsetPosition, InitialOffsetPosition);//初期位置オフセット
	unsigned int _counter;

	bool _isZooming;//ズーム中フラグ
	std::map<int, cocos2d::Vec2> _sceneLayerPositionList;
public:
	static float _defaultMoveDuration;
};

NS_AGTK_END

#endif	//__CAMERA_H__
