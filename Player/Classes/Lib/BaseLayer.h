/**
 * @class BaseLayer
 * @brief レイヤー基クラス
 * @note 継承先では、必ずupdateメソッド内でBaseLayer::updateメソッドを呼ぶようにする。
        回転・反転アニメーション処理を行うため。
 */
#ifndef __BASE_LAYER_H__
#define	__BASE_LAYER_H__

#include "Lib/Macros.h"
#include "Lib/Common.h"
#include "Manager/InputManager.h"

NS_AGTK_BEGIN

//-------------------------------------------------------------------------------------------------------------------
class AGTKPLAYER_API TimerFloat : public agtk::EventTimer {
private:
	TimerFloat();
public:
	CREATE_FUNC_PARAM(TimerFloat, float, value);
private:
	virtual bool init(float value);
public:
	float setValue(float value, float seconds = 0.0f);
	float addValue(float value, float seconds = 0.0f);
	float getValue() { return _value; }
	float getPreValue() { return _prevValue; }
	float getNextValue() { return _nextValue; }
	bool isChanged() { return _value != _oldValue ? true : false; }
	void reset();
private:
	float _initValue;
	float _value;
	float _nextValue;
	float _prevValue;
	float _oldValue;
};

//-------------------------------------------------------------------------------------------------------------------
class AGTKPLAYER_API TimerVec2 : public agtk::EventTimer
{
private:
	TimerVec2();
public:
	CREATE_FUNC_PARAM(TimerVec2, cocos2d::Vec2, value);
private:
	virtual bool init(cocos2d::Vec2 value);
public:
	cocos2d::Vec2 setValue(cocos2d::Vec2 pos, float seconds = 0.0f);
	cocos2d::Vec2 addValue(cocos2d::Vec2 pos, float seconds = 0.0f);
	cocos2d::Vec2 getValue() { return _value; }
	cocos2d::Vec2 getPrevValue() { return _prevValue; }
	cocos2d::Vec2 getNextValue() { return _nextValue; }
	bool isChanged() { return _value != _oldValue ? true : false; }
	void reset();
private:
	cocos2d::Vec2 _initValue;
	cocos2d::Vec2 _value;
	cocos2d::Vec2 _nextValue;
	cocos2d::Vec2 _prevValue;
	cocos2d::Vec2 _oldValue;
};

//-------------------------------------------------------------------------------------------------------------------
class AGTKPLAYER_API TimerFlip : public agtk::EventTimer {
private:
	TimerFlip() : EventTimer() {};
	virtual ~TimerFlip() {};
public:
	CREATE_FUNC(TimerFlip);
	void start(bool startFlip, bool endFlip, float seconds) {
		if (startFlip == endFlip) {
			return;
		}
		EventTimer::start(seconds, nullptr);
		_startScale = startFlip ? -1.0f : 1.0f;
		_endScale = endFlip ? -1.0f : 1.0f;
	}
	virtual void end() {
		EventTimer::end();
		_scale = _endScale;
	}
	virtual void update(float delta) {
		EventTimer::update(delta);
		if (this->getState() == kStateStart || this->getState() == kStateProcessing) {
			_scale = AGTK_LINEAR_INTERPOLATE(_startScale, _endScale, _seconds, _timer);
		}
		else if (this->getState() == kStateEnd) {
			end();
		}
	}
	bool getFlip() {
		return TimerFlip::getFlip(_scale);
	}
	void reset() {
		_scale = 1.0f;
		_startScale = 1.0f;
		_endScale = 1.0f;
	}
private:
	virtual bool init() {
		if (EventTimer::init() == false) {
			return false;
		}
		_scale = 1.0f;
		_startScale = 1.0f;
		_endScale = 1.0f;
		return true;
	}
private:
	float _startScale;
	float _endScale;
	CC_SYNTHESIZE_READONLY(float, _scale, Scale);
public:
	static bool getFlip(float scale) {
		return scale < 0.0f ? true : false;
	}
};

//-------------------------------------------------------------------------------------------------------------------
class Scene;
class AGTKPLAYER_API BaseLayer: public cocos2d::Layer, public InputEventListener
{
public:
	struct Tag {
		static const int BG = 0;
		static const int Scene = 1000;
		static const int Movie = 1001;
		static const int Debug = 1090;
		static const int Fade = 1100;
	};
	struct ZOrder {
		static const int BG = 0;
		static const int Scene = 1000;
		static const int Movie = 1001;
		static const int Debug = 1090;
		static const int Fade = 1100;
		static const int Menu = 9000;
		// SceneData.hの値と揃えておく
		static const int HudMenu = 9999;
		static const int HudTopMost = 99999;
		static const int TopMost = 999998;
		static const int TopMostWithMenu = 999999;
	};

	static const int ADD_ZORDER = 10;			// レイヤー間でのzOrder
	static const int HUD_MENU_ZORDER = 1000;	// HUDMENUへのzOrder

public:
	BaseLayer();
	virtual ~BaseLayer();
protected:
	virtual void update(float delta);
	enum Tags {
		kTagMain,
	};
	enum ZOrders {
		kZOrderMain,
	};
public:
	void setRotation(float rotate);
	void setRotation(float rotate, cocos2d::Vec2 AnchorPoint, float duration = 0.0f);
	void setFlipX(bool bFlipX, cocos2d::Vec2 AnchorPoint = Vec2::ANCHOR_MIDDLE, float duration = 0.0f);
	void setFlipY(bool bFlipY, cocos2d::Vec2 AnchorPoint = Vec2::ANCHOR_MIDDLE, float duration = 0.0f);
	bool getFlipX() { return this->getTimerFlipX()->getFlip(); };
	bool getFlipY() { return this->getTimerFlipY()->getFlip(); };
	void setScale(cocos2d::Vec2 scale, float duration = 0.0f);
	cocos2d::Vec2 getScale();
	cocos2d::Vec2 setAnchorPoint(cocos2d::Vec2 value, float duration = 0.0f);
	cocos2d::Vec2 getAnchorPoint();
	void reset();
	virtual void addChild(cocos2d::Node *child) { this->addChild(child, child->getLocalZOrder(), child->getName()); }
	virtual void addChild(cocos2d::Node *child, int localZOrder) { this->addChild(child, localZOrder, child->getName()); }
	virtual void addChild(cocos2d::Node *child, int localZOrder, int tag);
	virtual void addChild(cocos2d::Node *child, int localZOrder, const std::string &name);
	virtual void removeChild(cocos2d::Node *child, bool cleanup = true);
public:
	void attachScene(agtk::Scene *scene);
	void detachScene(agtk::Scene *scene);
	int getTopPrioritySceneLayer();
private:
	CC_SYNTHESIZE_RETAIN(cocos2d::Ref *, _args, Args);
	CC_SYNTHESIZE_RETAIN(agtk::TimerFloat *, _timerRotation, TimerRotation);
	CC_SYNTHESIZE_RETAIN(agtk::TimerFlip *, _timerFlipX, TimerFlipX);
	CC_SYNTHESIZE_RETAIN(agtk::TimerFlip *, _timerFlipY, TimerFlipY);
	CC_SYNTHESIZE_RETAIN(agtk::TimerVec2 *, _timerScale, TimerScale);
	CC_SYNTHESIZE_RETAIN(agtk::TimerVec2 *, _timerAnchorPoint, TimerAnchorPoint);
};

NS_AGTK_END

#endif	//__BASE_LAYER_H__
