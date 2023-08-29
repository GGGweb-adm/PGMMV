#ifndef __ANIMATION_H__
#define	__ANIMATION_H__

#include "Macros.h"
#include "Data/AnimationData.h"
#include "json/document.h"
// #AGTK-NX, #AGTK-WIN
#ifdef USE_30FPS_4
#include "Lib/MiddleFrame.h"
#endif


NS_AGTK_BEGIN

//-------------------------------------------------------------------------------------------------------------------
class AGTKPLAYER_API AnimationFrame : public cocos2d::Ref
{
public:
	AnimationFrame();
	virtual ~AnimationFrame();
	CREATE_FUNC_PARAM3(AnimationFrame, agtk::data::FrameData*, frame, agtk::data::ResourceInfoData *, resourceInfoData, int, imageId);
	void changeImage(agtk::data::ResourceInfoData *resourceInfoData, int imageId);
private:
	virtual bool init(agtk::data::FrameData *frame, agtk::data::ResourceInfoData *resourceInfoData, int imageId);
private:
	CC_SYNTHESIZE_RETAIN(agtk::data::FrameData *, _frameData, FrameData);
	CC_SYNTHESIZE_RETAIN(cocos2d::SpriteFrame *, _spriteFrame, SpriteFrame);
	CC_SYNTHESIZE(cocos2d::Rect, _rect, Rect);
	CC_SYNTHESIZE(unsigned int, _startFrameCount, StartFrameCount);
	CC_SYNTHESIZE(unsigned int, _endFrameCount, EndFrameCount);
};

//-------------------------------------------------------------------------------------------------------------------
class AGTKPLAYER_API AnimationTimeline : public cocos2d::Ref
{
private:
	AnimationTimeline();
	virtual ~AnimationTimeline();
public:
	CREATE_FUNC_PARAM(AnimationTimeline, agtk::data::TimelineInfoData *, timelineInfoData);
	void update(int frameCount300);
private:
	virtual bool init(agtk::data::TimelineInfoData *timelineInfoData);
	agtk::data::TimelineInfoAreaData* getTimelineAreaData(int frame);
	int getTimelineAreaDataNo(agtk::data::TimelineInfoAreaData *areaData);
	float getSplineInterpolate(int frame, int areaDataNo, std::string name);
	float getFrameKeyframeValue(agtk::data::TimelineInfoAreaData *areaData, std::string name);
private:
	CC_SYNTHESIZE_RETAIN(agtk::data::TimelineInfoData *, _timelineInfoData, TimelineInfoData);
	CC_SYNTHESIZE(cocos2d::Rect, _rect, Rect);
	CC_SYNTHESIZE(bool, _backSide, BackSide);
	CC_SYNTHESIZE(bool, _valid, Valid);
// #AGTK-NX
#ifdef USE_SAR_OPTIMIZE_1
	CC_SYNTHESIZE(int, _cachedIndex, CachedIndex);
#endif
// #AGTK-NX
#ifdef USE_SAR_OPTIMIZE_6
	agtk::data::TimelineInfoAreaData* _areaDataCache;
#endif
};

//-------------------------------------------------------------------------------------------------------------------
class AGTKPLAYER_API AnimationDirection : public cocos2d::Ref
{
public:
	AnimationDirection();
	virtual ~AnimationDirection();
	CREATE_FUNC_PARAM3(AnimationDirection, agtk::data::DirectionData *, directionData, agtk::data::ResourceInfoData *, resourceInfoData, int, imageId);
public:
	virtual bool init(agtk::data::DirectionData *directionData, agtk::data::ResourceInfoData *resourceInfoData, int imageId);
	const char *getName();
	AnimationFrame *getAnimationFrame(int id);
	int getAnimationFrameCount();
	void update(int frameCount300);
private:
	CC_SYNTHESIZE_RETAIN(cocos2d::__Array*, _animationFrameList, AnimationFrameList);
	CC_SYNTHESIZE_RETAIN(agtk::data::DirectionData *, _directionData, DirectionData);
	CC_SYNTHESIZE(unsigned int, _maxFrameCount300, MaxFrameCount300);
	CC_SYNTHESIZE(unsigned int, _maxFrame, MaxFrame);
	CC_SYNTHESIZE_RETAIN(cocos2d::__Dictionary *, _animationTimelineList, AnimationTimelineList);
};

//-------------------------------------------------------------------------------------------------------------------
/**
 * @brief モーションアニメの再生を管理するクラス
 */
class AGTKPLAYER_API AnimationMotion : public cocos2d::Ref
{
public:
	enum {
		kAnimPauseFrameValue = -2,
	};
	AnimationMotion();
	virtual ~AnimationMotion();
	CREATE_FUNC_PARAM3(AnimationMotion, agtk::data::MotionData*, motionData, agtk::data::ResourceInfoData *, resourceInfoData, int, imageId);
public:
	virtual bool init(agtk::data::MotionData *motionData, agtk::data::ResourceInfoData *resourceInfoData, int imageId);
	void play(int directionNo);
	void play(int directionNo, float seconds, bool reverse);
	void stop();
// #AGTK-NX
#ifdef USE_SAR_OPTIMIZE_6
	void update(float dt, AnimationFrame** pFrameCache = nullptr);
#else
	void update(float dt);
#endif
	bool isRunning();
	const char *getName();
public:
	std::function<void(cocos2d::SpriteFrame *spriteFrame, cocos2d::Rect rect)> onSetSpriteFrame;
	std::function<void(bool flag)> onSetFlipX;
	std::function<void(bool flag)> onSetFlipY;
	std::function<void(int alpha)> onSetAlpha;
	std::function<void(int r, int g, int b)> onSetColor;
	std::function<void(float sx, float sy)> onSetScale;
	std::function<void(float rotation)> onSetRotation;
	std::function<void(float ox, float oy)> onSetOffset;
	std::function<void(float cx, float cy)> onSetCenter;
	std::function<void(int id, int type, float x, float y, float w, float h)> onSetArea;
public:
	agtk::AnimationDirection *getAnimationDirection(int id);
	agtk::AnimationDirection *getAnimationDirection(std::string name);
	int getAnimationDirectionCount();
	bool isAnimationFinished();
	bool isAllAnimationFinished();
	int getFixedFrameNo() { return _fixedFrameNo; }
	int getFixedFrame() { return _fixedFrame; }
	void setFixedFrameNo(int frameNo);
	void setFixedFrame(int frame);
// #AGTK-NX #AGTK-WIN
#ifdef USE_30FPS_4
	void updateMiddleFrame(agtk::AnimationFrame *now, agtk::AnimationFrame *next, float seconds);
#endif
private:
	void updateCollision(float dt);
	float getSplineInterpolate(int frame, int frameDataNo, std::string name);
	float getFrameKeyframeValue(agtk::data::FrameData *frameData, std::string name);
	void skipOneFrame(float dt);
private:
	CC_SYNTHESIZE_RETAIN(cocos2d::__Dictionary *, _animationDirectionList, AnimationDirectionList);
	CC_SYNTHESIZE_RETAIN(agtk::data::MotionData *, _motionData, MotionData);
	CC_SYNTHESIZE_RETAIN(agtk::AnimationDirection *, _currentDirection, CurrentDirection);
	CC_SYNTHESIZE(unsigned int, _frameDataNo, FrameDataNo);
	CC_SYNTHESIZE(bool, _ignored, Ignored);
	CC_SYNTHESIZE(bool, _ignoredSound, IgnoredSound);
	int _fixedFrameNo;//アニメーションフレームNo固定用値
	int _fixedFrame;//アニメーションフレーム固定値用
	bool _isAllAnimationFinished;// ループ等を含んで最後まで再生されたか？
	CC_SYNTHESIZE_READONLY(bool, _reachedLastFrame, ReachedLastFrame);// 逆再生を含む最終フレームに到達したか？
public:
	float _seconds;
	unsigned int _loopNum;
	bool _bFrameFirst;//初回FrameDataNo時:TRUE,それ以外はFALSE
	bool _bReverse;//逆生成フラグ、このフラグが立っているときは逆生成。
	float _frame300;//update関数でframeを加算する
	bool _bRestoreFixFrame;//フレーム固定からの復帰の場合(アクションが変更された際にアニメーションが最初から再生されるのをブロックする)

	cocos2d::Node * _objectNode;
	void setObjectNode(cocos2d::Node * objectNode) { _objectNode = objectNode; };
};

NS_AGTK_END

#endif	//__ANIMATION_H__
