#ifndef __BASE_PLAYER_H__
#define	__BASE_PLAYER_H__

#include "Lib/Macros.h"
#include "Lib/Animation.h"

NS_AGTK_BEGIN

/**
 * @brief アニメ再生のフレーム処理を管理するクラス。
 */
class AGTKPLAYER_API BasePlayer
{
public:
#ifdef AGTK_DEBUG
	static const char *DebugDisplayName;
#endif
	enum Type {
		None,
		Image,
		SpriteStudio,
		Gif,
		Spine,
	};
protected:
	BasePlayer();
	virtual ~BasePlayer();
public:
	void setFixedFrameNo(int frameNo, bool bEnableFrameSkip = true);
	int getFixedFrameNo();
	void setFixedFrame(int frameCount, bool bEnableFrameSkip = true);
	int getFixedFrame();
	virtual void play(int actionNo, int actionDirectNo, float seconds = 0.0f, bool bIgnoredSound = false, bool bReverse = false) = 0;
	bool isNowPlaying(int actionNo, int actionDirectNo);
	agtk::data::AnimationData *getAnimationData(void) const { return _animationData; }
	void setAnimationData(agtk::data::AnimationData *animationData);
	virtual void setResourceSetId(int resourceSetId) {}
	virtual void setResourceSetName(string resourceSetName) {}
	virtual int getResourceSetIdByName(string resourceSetName) { return -1; }
public:
	std::function<void(float x, float y)> onSetPosition;
	std::function<void(float x, float y)> onSetScale;
	std::function<void(float rotate)> onSetRotation;
	std::function<void(float x, float y)> onSetOffset;
	//	virtual void setPosition(const cocos2d::Vec2 pos);
//	virtual void setPosition(float x, float y);
//	virtual const cocos2d::Vec2& getPosition() const;
//	virtual void getPosition(float *x, float *y) const;
public:
	virtual void setOffset(const cocos2d::Vec2& offset);
	virtual void setOffset(float x, float y);
	virtual const cocos2d::Vec2& getOffset() const;
public:
	virtual void setOrigin(const cocos2d::Vec2& origin);
	virtual void setOrigin(float x, float y);
	virtual const cocos2d::Vec2& getOrigin() const;
public:
	virtual void setCenter(const cocos2d::Vec2& center);
	virtual void setCenter(float x, float y);
	virtual const cocos2d::Vec2& getCenter() const;
	void setOriginOffset(float x, float y){ setOriginOffset(cocos2d::Vec2(x, y)); }
public:
	virtual void setInnerScale(const cocos2d::Vec2& scale);
	virtual void setInnerScale(float x, float y);
	virtual const cocos2d::Vec2& getInnerScale() const;
	virtual void setInnerRotation(float rotation);
	virtual float getInnerRotation() const;
protected:
	cocos2d::Vec2 getRealPosition();
	void updateRealPosition();
	cocos2d::Vec2 getRealScale();
	void updateRealScale();
	float getRealRotation();
	void updateRealRotation();
protected:
	CC_SYNTHESIZE(int, _frameNo, FrameNo);
	cocos2d::Vec2 _offset;
	cocos2d::Vec2 _origin;
	cocos2d::Vec2 _center;
	cocos2d::Vec2 _pos;
	cocos2d::Vec2 _scale, _innerScale;
	float _rotation, _innerRotation;
	CC_SYNTHESIZE(BasePlayer::Type, _type, Type);
	agtk::data::AnimationData *_animationData;
	CC_SYNTHESIZE_RETAIN(agtk::AnimationMotion *, _currentAnimationMotion, CurrentAnimationMotion);
	CC_SYNTHESIZE_RETAIN(cocos2d::__Dictionary *, _animationMotionList, AnimationMotionList);
	CC_SYNTHESIZE(cocos2d::Vec2, _originOffset, OriginOffset);
	CC_SYNTHESIZE(bool, _hasRotation, HasRotation);	//アニメーションが回転を持っているか。
public:
	cocos2d::Node * _refVisibleCtrlNode; // 回転・移動制御用ノードの参照
	cocos2d::Vec2 _leftupAnchorPoint;

	cocos2d::Node * _objectNode;
	void setObjectNode(cocos2d::Node * objectNode) { 
		_objectNode = objectNode;

		cocos2d::DictElement *el;
		CCDICT_FOREACH(_animationMotionList, el) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
			auto motion = static_cast<agtk::AnimationMotion *>(el->getObject());
#else
			auto motion = dynamic_cast<agtk::AnimationMotion *>(el->getObject());
#endif
			motion->setObjectNode(objectNode);
		}
	};
// #AGTK-NX
#ifdef USE_SAR_OPTIMIZE_6
protected:
	agtk::AnimationFrame* _frameCache;
#endif
};

NS_AGTK_END

#endif	//__BASE_PLAYER_H__
