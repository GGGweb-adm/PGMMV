#include "Animation.h"
#include "Lib/Object.h"
#include "External/SplineInterp/SplineInterp.h"
#include "Manager/AudioManager.h"
#include "Manager/GameManager.h"

NS_AGTK_BEGIN

//-------------------------------------------------------------------------------------------------------------------
AnimationFrame::AnimationFrame()
{
	_frameData = nullptr;
	_spriteFrame = nullptr;
	_startFrameCount = 0;
	_endFrameCount = 0;
}

AnimationFrame::~AnimationFrame()
{
	CC_SAFE_RELEASE_NULL(_frameData);
	CC_SAFE_RELEASE_NULL(_spriteFrame);
}

bool AnimationFrame::init(agtk::data::FrameData *frame, agtk::data::ResourceInfoData *resourceInfoData, int imageId)
{
	cocos2d::Rect rect;
	this->setFrameData(frame);
	imageId = (imageId != agtk::data::ResourceInfoData::kInvalidImageId) ? imageId : resourceInfoData->getImageId();
	if (resourceInfoData->getImage() && imageId >= 0) {
		auto imageData = GameManager::getInstance()->getProjectData()->getImageData(imageId);
		int width = imageData->getTexWidth() / resourceInfoData->getHDivCount();
		int height = imageData->getTexHeight() / resourceInfoData->getVDivCount();
		rect.setRect(
			frame->getImageTileX() * width,
			frame->getImageTileY() * height,
			width, height
		);
		auto spriteFrame = cocos2d::SpriteFrame::create(imageData->getFilename(), rect, false, cocos2d::Vec2(0, 0), rect.size);
		spriteFrame->getTexture()->setAliasTexParameters();
		this->setSpriteFrame(spriteFrame);
	}
	this->setRect(rect);
	return true;
}

void AnimationFrame::changeImage(agtk::data::ResourceInfoData *resourceInfoData, int imageId)
{
	cocos2d::Rect rect;
	if (imageId >= 0) {
		auto imageData = GameManager::getInstance()->getProjectData()->getImageData(imageId);
		int width = imageData->getTexWidth() / resourceInfoData->getHDivCount();
		int height = imageData->getTexHeight() / resourceInfoData->getVDivCount();
		rect.setRect(
			_frameData->getImageTileX() * width,
			_frameData->getImageTileY() * height,
			width, height
		);
		auto spriteFrame = cocos2d::SpriteFrame::create(imageData->getFilename(), rect, false, cocos2d::Vec2(0, 0), rect.size);
		spriteFrame->getTexture()->setAliasTexParameters();
		this->setSpriteFrame(spriteFrame);
	}
	this->setRect(rect);
}

//-------------------------------------------------------------------------------------------------------------------
AnimationTimeline::AnimationTimeline()
{
	_timelineInfoData = nullptr;
	_backSide = false;
	_valid = false;
// #AGTK-NX
#ifdef USE_SAR_OPTIMIZE_1
	_cachedIndex = -1;
#endif
// #AGTK-NX
#ifdef USE_SAR_OPTIMIZE_6
	_areaDataCache = nullptr;
#endif
}

AnimationTimeline::~AnimationTimeline()
{
	CC_SAFE_RELEASE_NULL(_timelineInfoData);
}

bool AnimationTimeline::init(agtk::data::TimelineInfoData *timelineInfoData)
{
	this->setTimelineInfoData(timelineInfoData);
	this->setRect(cocos2d::Rect::ZERO);
	return true;
}

void AnimationTimeline::update(int frameCount300)
{
// #AGTK-NX
#ifndef USE_SAR_OPTIMIZE_6
	//clear rect
	this->setRect(cocos2d::Rect::ZERO);
	this->setBackSide(false);
	this->setValid(false);
#endif

	int frame = frameCount300 / 5;
	auto nowAreaData = this->getTimelineAreaData(frame);
	if (nowAreaData == nullptr) {
// #AGTK-NX
#ifdef USE_SAR_OPTIMIZE_6
		_areaDataCache = nullptr;
		//clear rect
		this->setRect(cocos2d::Rect::ZERO);
		this->setBackSide(false);
		this->setValid(false);
#endif
		return;
	}
	int areaDataNo = this->getTimelineAreaDataNo(nowAreaData);
	auto nextAreaData = this->getTimelineInfoData()->getAreaData(areaDataNo + 1);
	if (nextAreaData == nullptr){
		nextAreaData = nowAreaData;
	}

// #AGTK-NX
#ifdef USE_SAR_OPTIMIZE_6
	if(_areaDataCache != nowAreaData || nowAreaData->getInterpType() != agtk::data::TimelineInfoAreaData::EnumInterpType::kInterpNone) {
#endif

	if (nowAreaData->getValid()) {
		switch (nowAreaData->getInterpType()) {
		case agtk::data::TimelineInfoAreaData::EnumInterpType::kInterpNone:
			this->setRect(cocos2d::Rect(nowAreaData->getX(), nowAreaData->getY(), nowAreaData->getWidth(), nowAreaData->getHeight()));
			break;
		case agtk::data::TimelineInfoAreaData::EnumInterpType::kInterpLinear: {
			float range = nextAreaData->getFrame() - nowAreaData->getFrame();
			// 2017/04/17 agusa-k: 0除算を回避する。
			if (range == 0) {
				this->setRect(cocos2d::Rect(nextAreaData->getX(), nextAreaData->getY(), nextAreaData->getWidth(), nextAreaData->getHeight()));
				break;
			}
			float pos = frame - nowAreaData->getFrame();
			cocos2d::Rect rect(
				AGTK_LINEAR_INTERPOLATE(nowAreaData->getX(), nextAreaData->getX(), range, pos),
				AGTK_LINEAR_INTERPOLATE(nowAreaData->getY(), nextAreaData->getY(), range, pos),
				AGTK_LINEAR_INTERPOLATE(nowAreaData->getWidth(), nextAreaData->getWidth(), range, pos),
				AGTK_LINEAR_INTERPOLATE(nowAreaData->getHeight(), nextAreaData->getHeight(), range, pos)
			);
			this->setRect(rect);
			break; }
		case agtk::data::TimelineInfoAreaData::EnumInterpType::kInterpCurve: {
			cocos2d::Rect rect(
				this->getSplineInterpolate(frame, areaDataNo, "x"),
				this->getSplineInterpolate(frame, areaDataNo, "y"),
				this->getSplineInterpolate(frame, areaDataNo, "width"),
				this->getSplineInterpolate(frame, areaDataNo, "height")
			);
			this->setRect(rect);
			break; }
		}
		this->setBackSide(nowAreaData->getBackside());
	}
// #AGTK-NX
#ifdef USE_SAR_OPTIMIZE_6
	else {
		this->setRect(cocos2d::Rect::ZERO);
		this->setBackSide(false);
	}
#endif
	this->setValid(nowAreaData->getValid());
// #AGTK-NX
#ifdef USE_SAR_OPTIMIZE_6
		if (nowAreaData->getInterpType() == agtk::data::TimelineInfoAreaData::EnumInterpType::kInterpNone) {
			_areaDataCache = nowAreaData;
		}
		else {
			_areaDataCache = nullptr;
		}
	}
#endif
}

agtk::data::TimelineInfoAreaData* AnimationTimeline::getTimelineAreaData(int frame)
{
	agtk::data::TimelineInfoAreaData *areaData = nullptr;
	cocos2d::Ref *ref = nullptr;
	auto areaList = this->getTimelineInfoData()->getAreaList();
// #AGTK-NX
#ifdef USE_SAR_OPTIMIZE_1
	if (frame > 0 && getCachedIndex() >= 0) // 高速化の一環として frame == 0 の場合は先頭のエリアデータから取得
	{
		auto curData = static_cast<agtk::data::TimelineInfoAreaData *>(areaList->getObjectAtIndex(getCachedIndex()));
		if (frame >= curData->getFrame())
		{
			if (getCachedIndex() + 1 < areaList->count())
			{
				for (int i = getCachedIndex(); i < areaList->count() - 1; i++)
				{
					auto nextData = static_cast<agtk::data::TimelineInfoAreaData *>(areaList->getObjectAtIndex(i + 1));
					if (frame >= curData->getFrame() && frame < nextData->getFrame())
					{
						setCachedIndex(i);
						return curData;
					}
					curData = nextData;
				}
				setCachedIndex(areaList->count() - 1);
				return curData;
			}
			return curData;
		}
		else {
			if (getCachedIndex() - 1 >= 0)
			{
				for (int i = getCachedIndex() - 1; i >= 0; i--)
				{
					auto nextData = curData;
					auto curData = static_cast<agtk::data::TimelineInfoAreaData *>(areaList->getObjectAtIndex(i));
					if (frame >= curData->getFrame() && frame < nextData->getFrame())
					{
						setCachedIndex(i);
						return curData;
					}
				}
			}
			setCachedIndex(0);
			return nullptr;
		}
	}

	int index = -1;
	CCARRAY_FOREACH(areaList, ref) {
		auto data = static_cast<agtk::data::TimelineInfoAreaData *>(ref);
		if (frame >= data->getFrame()) {
			areaData = data;
			index++;
			continue;
		}
		break;
	}
	setCachedIndex(index);
#else
	CCARRAY_FOREACH(areaList, ref) {
		auto data = dynamic_cast<agtk::data::TimelineInfoAreaData *>(ref);
		if (frame >= data->getFrame()) {
			areaData = data;
			continue;
		}
		break;
	}
#endif
	return areaData;
}

int AnimationTimeline::getTimelineAreaDataNo(agtk::data::TimelineInfoAreaData *areaData)
{
	return (int)this->getTimelineInfoData()->getAreaList()->getIndexOfObject(areaData);
}

float AnimationTimeline::getSplineInterpolate(int frame, int areaDataNo, std::string name)
{
	auto timelineInfoData = this->getTimelineInfoData();
	int keyframeNo0 = -1;
	int keyframeNo1 = areaDataNo;
	int keyframeNo2 = -1;
	int keyframeNo3 = -1;
	agtk::data::TimelineInfoAreaData *areaData0 = nullptr;
	agtk::data::TimelineInfoAreaData *areaData1 = timelineInfoData->getAreaData(keyframeNo1);
	agtk::data::TimelineInfoAreaData *areaData2 = nullptr;
	agtk::data::TimelineInfoAreaData *areaData3 = nullptr;
	int keyFrame0 = -1;
	int keyFrame1 = areaData1->getFrame();
	int keyFrame2 = -1;
	int keyFrame3 = -1;
	if (keyframeNo1 < 0) {
		CC_ASSERT(0);
		return 0.0f;
	}
	if (keyframeNo1 > 0) {
		keyframeNo0 = keyframeNo1 - 1;
		areaData0 = timelineInfoData->getAreaData(keyframeNo0);
		keyFrame0 = areaData0->getFrame();
	}
	if (keyframeNo1 + 1 < timelineInfoData->getAreaList()->count()) {
		keyframeNo2 = keyframeNo1 + 1;
		areaData2 = timelineInfoData->getAreaData(keyframeNo2);
		keyFrame2 = areaData2->getFrame();
	}
	if (keyframeNo2 >= 0 && keyframeNo2 + 1 < timelineInfoData->getAreaList()->count()) {
		keyframeNo3 = keyframeNo2 + 1;
		areaData3 = timelineInfoData->getAreaData(keyframeNo3);
		keyFrame3 = areaData3->getFrame();
	}
	if (keyframeNo2 < 0) {
		//補間に必要な２つ目のキーフレームが見つからない or ステップ補間。１つ目のキーフレームでの値を返す。
		return getFrameKeyframeValue(areaData1, name);
	}
	SplineInterp spline;

	//! ACT2-1332 のエディタ側対応に合わせるよう変更
	//! キーが等間隔に打たれているとみなして補完 
#if 1
	int interval = keyFrame2 - keyFrame1;
	if (keyframeNo0 >= 0) {
		spline.AddKey(0, getFrameKeyframeValue(areaData0, name));
	}
	spline.AddKey(interval, getFrameKeyframeValue(areaData1, name));
	spline.AddKey(interval * 2, getFrameKeyframeValue(areaData2, name));
	if (keyframeNo3 >= 0) {
		spline.AddKey(interval * 3, getFrameKeyframeValue(areaData3, name));
	}
	return spline.GetInterpolated(interval + (frame - keyFrame1) * interval / (keyFrame2 - keyFrame1));
#else
	if (keyframeNo0 >= 0) {
		spline.AddKey(keyFrame0, getFrameKeyframeValue(areaData0, name));
	}
	spline.AddKey(keyFrame1, getFrameKeyframeValue(areaData1, name));
	spline.AddKey(keyFrame2, getFrameKeyframeValue(areaData2, name));
	if (keyframeNo3 >= 0) {
		spline.AddKey(keyFrame3, getFrameKeyframeValue(areaData3, name));
	}
	return spline.GetInterpolated(frame);
#endif
}

float AnimationTimeline::getFrameKeyframeValue(agtk::data::TimelineInfoAreaData *areaData, std::string name)
{
	if (name == "x") {
		return areaData->getX();
	}
	else if (name == "y") {
		return areaData->getY();
	}
	else if (name == "width") {
		return areaData->getWidth();
	}
	else if (name == "height") {
		return areaData->getHeight();
	}
	CC_ASSERT(0);
	return 0.0f;
}

//-------------------------------------------------------------------------------------------------------------------
AnimationDirection::AnimationDirection()
{
	_animationFrameList = nullptr;
	_directionData = nullptr;
	_maxFrame = 0;
	_maxFrameCount300 = 0;
	_animationTimelineList = nullptr;
}

AnimationDirection::~AnimationDirection()
{
	CC_SAFE_RELEASE_NULL(_animationFrameList);
	CC_SAFE_RELEASE_NULL(_directionData);
	CC_SAFE_RELEASE_NULL(_animationTimelineList);
}

bool AnimationDirection::init(agtk::data::DirectionData *directionData, agtk::data::ResourceInfoData *resourceInfoData, int imageId)
{
	auto arr = cocos2d::__Array::create();
	auto keys = directionData->getFrameList()->allKeys();
	cocos2d::Ref *ref = nullptr;
	unsigned int maxFrameCount300 = 0;
	CCARRAY_FOREACH(keys, ref) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
		auto id = static_cast<cocos2d::Integer *>(ref);
		auto frameData = static_cast<agtk::data::FrameData *>(directionData->getFrameList()->objectForKey(id->getValue()));
#else
		auto id = dynamic_cast<cocos2d::Integer *>(ref);
		auto frameData = dynamic_cast<agtk::data::FrameData *>(directionData->getFrameList()->objectForKey(id->getValue()));
		CC_ASSERT(frameData);
#endif
		auto p = AnimationFrame::create(frameData, resourceInfoData, imageId);
		CC_ASSERT(p);
		p->setStartFrameCount(maxFrameCount300);
		maxFrameCount300 += frameData->getFrameCount300();
		p->setEndFrameCount(maxFrameCount300);
		arr->addObject(p);
	}
	this->setAnimationFrameList(arr);
	this->setDirectionData(directionData);
	this->setMaxFrameCount300(maxFrameCount300);
	this->setMaxFrame(maxFrameCount300 / 5);

	//timeline
	auto timelineList = directionData->getTimelineInfoList();
	cocos2d::DictElement *el = nullptr;
	auto list = cocos2d::__Dictionary::create();
	CCDICT_FOREACH(timelineList, el) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
		auto timelineInfoData = static_cast<agtk::data::TimelineInfoData *>(el->getObject());
#else
		auto timelineInfoData = dynamic_cast<agtk::data::TimelineInfoData *>(el->getObject());
#endif
		auto timeline = AnimationTimeline::create(timelineInfoData);
		list->setObject(timeline, timelineInfoData->getId());
	}
	this->setAnimationTimelineList(list);
	return true;
}

const char *AnimationDirection::getName()
{
	return getDirectionData()->getName();
}

AnimationFrame *AnimationDirection::getAnimationFrame(int id)
{
	if (this->getAnimationFrameCount() <= id) {
		return nullptr;
	}
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
	return static_cast<AnimationFrame *>(this->getAnimationFrameList()->getObjectAtIndex(id));
#else
	return dynamic_cast<AnimationFrame *>(this->getAnimationFrameList()->getObjectAtIndex(id));
#endif
}

int AnimationDirection::getAnimationFrameCount()
{
	CC_ASSERT(_animationFrameList);
	return this->getAnimationFrameList()->count();
}

void AnimationDirection::update(int frameCount300)
{
	auto timelineList = this->getAnimationTimelineList();
	cocos2d::DictElement *el = nullptr;
	CCDICT_FOREACH(timelineList, el) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
		auto p = static_cast<AnimationTimeline *>(el->getObject());
#else
		auto p = dynamic_cast<AnimationTimeline *>(el->getObject());
#endif
		p->update(frameCount300);
	}
}

//-------------------------------------------------------------------------------------------------------------------
AnimationMotion::AnimationMotion()
{
	_animationDirectionList = nullptr;
	_motionData = nullptr;
	_currentDirection = nullptr;
	_seconds = 0;
	_loopNum = 0;
	_frameDataNo = 0;
	_bFrameFirst = false;
	_bReverse = false;
	_isAllAnimationFinished = false;
	_reachedLastFrame = false;
	_frame300 = 0;
	_ignored = false;
	_fixedFrameNo = -2;
	_fixedFrame = -1;
	_bRestoreFixFrame = false;

	onSetSpriteFrame = nullptr;
	onSetFlipX = nullptr;
	onSetFlipY = nullptr;
	onSetAlpha = nullptr;
	onSetColor = nullptr;
	onSetScale = nullptr;
	onSetRotation = nullptr;
	onSetOffset = nullptr;
	onSetCenter = nullptr;
	onSetArea = nullptr;

	_ignoredSound = false;//サウンド無効

	_objectNode = nullptr;
}

AnimationMotion::~AnimationMotion()
{
	CC_SAFE_RELEASE_NULL(_animationDirectionList);
	CC_SAFE_RELEASE_NULL(_motionData);
	CC_SAFE_RELEASE_NULL(_currentDirection);
}

bool AnimationMotion::init(agtk::data::MotionData *motionData, agtk::data::ResourceInfoData *resourceInfoData, int imageId)
{
	auto dic = cocos2d::__Dictionary::create();
	if (dic == nullptr) {
		return false;
	}
	auto keys = motionData->getDirectionList()->allKeys();
	cocos2d::Ref *ref = nullptr;
	CCARRAY_FOREACH(keys, ref) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
		auto id = static_cast<cocos2d::Integer *>(ref);
		auto data = static_cast<agtk::data::DirectionData *>(motionData->getDirectionList()->objectForKey(id->getValue()));
#else
		auto id = dynamic_cast<cocos2d::Integer *>(ref);
		auto data = dynamic_cast<agtk::data::DirectionData *>(motionData->getDirectionList()->objectForKey(id->getValue()));
#endif
		auto p = AnimationDirection::create(data, resourceInfoData, imageId);
		if (p == nullptr) {
			return false;
		}
		dic->setObject(p, id->getValue());
	}
	this->setAnimationDirectionList(dic);
	this->setMotionData(motionData);
	return true;
}

void AnimationMotion::play(int directionNo)
{
	this->setCurrentDirection(this->getAnimationDirection(directionNo));
	if (!_bRestoreFixFrame) {
		_seconds = 0;
		_loopNum = 0;
		_frameDataNo = 0;
		_bFrameFirst = true;
		_bReverse = false;
		_isAllAnimationFinished = false;
		_reachedLastFrame = false;
		_frame300 = 0;
	}
	_bRestoreFixFrame = false;

	update(0);
}

void AnimationMotion::play(int directionNo, float seconds, bool reverse)
{
	this->setCurrentDirection(this->getAnimationDirection(directionNo));
	if (!_bRestoreFixFrame) {
		_seconds = 0;
		_loopNum = 0;
		_frameDataNo = 0;
		_bFrameFirst = true;
		_bReverse = false;
		_isAllAnimationFinished = false;
		_reachedLastFrame = false;
		_frame300 = 0;
	}
	_bRestoreFixFrame = false;

	float delta = Director::getInstance()->getAnimationInterval();
// #AGTK-NX
#ifdef USE_SAR_OPTIMIZE_1
	// 最適化前の処理ではスキップするフレーム数の計算は誤差によりseconds + 1フレーム分多くアニメーションフレームを進めることがある。
	// 修正するとゲームの挙動に違いが発生するためそのままとする。
	int frameCount = 0;
	while (seconds > 0) {
		frameCount++;
		seconds -= delta;
	}
	if (frameCount > 0) {
		skipOneFrame(delta * frameCount);
	}
#else
	while (seconds > 0) {
		skipOneFrame(delta);
		seconds -= delta;
	}
#endif
	if (reverse) {
		_bReverse = reverse;
	}
	update(0);
}

void AnimationMotion::stop()
{
	this->setCurrentDirection(nullptr);

	onSetSpriteFrame = nullptr;
	onSetFlipX = nullptr;
	onSetFlipY = nullptr;
	onSetAlpha = nullptr;
	onSetColor = nullptr;
	onSetScale = nullptr;
	onSetRotation = nullptr;
	onSetOffset = nullptr;
	onSetCenter = nullptr;
	onSetArea = nullptr;
}

// #AGTK-NX
#ifdef USE_SAR_OPTIMIZE_6
void AnimationMotion::update(float dt, AnimationFrame** pFrameCache)
#else
void AnimationMotion::update(float dt)
#endif
{
	if (!this->isRunning()) {
		return;
	}
	if (this->getIgnored()) {
		return;
	}
	auto directionData = this->getCurrentDirection();
	auto motionData = this->getMotionData();
	bool infiniteLoop = motionData->getInfiniteLoop();
	bool reversePlay = motionData->getReversePlay();

	float seconds = _seconds;
	float secondsMax = (float)directionData->getMaxFrameCount300() / 300.0f;
// #AGTK-NX #AGTK-WIN
#ifdef USE_30FPS
	// 1フレームのみのアニメーションが30fpsの時に最初のフレーム更新で予期せず終了してしまう問題への対応
	if (secondsMax < 1.0f / GameManager::getInstance()->getFrameRate()) {
		secondsMax = 1.0f / GameManager::getInstance()->getFrameRate();
	}
#endif
	int frameCount = (int)(seconds * 300.0f);

	_reachedLastFrame = false;

	// フレーム固定される場合
	if (_fixedFrameNo > -1) {
		auto animationFrameCountMax = directionData->getAnimationFrameCount() - 1;
		_frameDataNo = _fixedFrameNo > animationFrameCountMax ? animationFrameCountMax : _fixedFrameNo;
	}

	//------------------------------------------------------------------------------------------------
	auto nowFrameData = directionData->getAnimationFrame(_frameDataNo);
	if (nowFrameData == nullptr && (_bFrameFirst || dt == 0.0f)) {
		if (onSetSpriteFrame) {
			onSetSpriteFrame(nullptr, cocos2d::Rect::ZERO);
		}
		return;
	}

	auto nextFrameData = directionData->getAnimationFrame(_frameDataNo + 1);
	if (nextFrameData == nullptr) {
		nextFrameData = nowFrameData;
	}
	auto interpType = nowFrameData->getFrameData()->getInterpType();
	float range = (nowFrameData->getEndFrameCount() - nowFrameData->getStartFrameCount()) / 300.0f;
	float pos = seconds - nowFrameData->getStartFrameCount() / 300.0f;
	if (range < pos) pos = range;

	//SpriteFrame
	if (nowFrameData->getSpriteFrame() && (_bFrameFirst || dt == 0.0f)) {
		if (onSetSpriteFrame) {
			cocos2d::Rect rect = nowFrameData->getRect();
			onSetSpriteFrame(nowFrameData->getSpriteFrame(), rect);
		}
	}

	if (_bFrameFirst && !this->getIgnoredSound())
	{
		//SE
		if (nowFrameData->getFrameData()->getPlaySe()) {
			if (_objectNode != nullptr) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
				auto _object = static_cast<agtk::Object *>(_objectNode);
#else
				auto _object = dynamic_cast<agtk::Object *>(_objectNode);
#endif
				_object->playSeObject(nowFrameData->getFrameData()->getPlaySeId());
			}
			else {
				AudioManager::getInstance()->playSe(nowFrameData->getFrameData()->getPlaySeId());
			}
		}
		//Voice
		if (nowFrameData->getFrameData()->getPlayVoice()) {
			if (_objectNode != nullptr) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
				auto _object = static_cast<agtk::Object *>(_objectNode);
#else
				auto _object = dynamic_cast<agtk::Object *>(_objectNode);
#endif
				_object->playVoiceObject(nowFrameData->getFrameData()->getPlayVoiceId());
			}
			else {
				AudioManager::getInstance()->playVoice(nowFrameData->getFrameData()->getPlayVoiceId());
			}
		}
	}

// #AGTK-NX
#ifdef USE_SAR_OPTIMIZE_6
	if ( pFrameCache == nullptr || *pFrameCache != nowFrameData) {
#endif
	//FlipX
	if (onSetFlipX) {
		onSetFlipX(nowFrameData->getFrameData()->getFlipX());
	}
	//FlipY
	if (onSetFlipY) {
		onSetFlipY(nowFrameData->getFrameData()->getFlipY());
	}
// #AGTK-NX
#ifdef USE_SAR_OPTIMIZE_6
	}
#endif
// #AGTK-NX
#ifdef USE_SAR_OPTIMIZE_6
	if (pFrameCache == nullptr || *pFrameCache != nowFrameData || interpType != agtk::data::FrameData::Interpolate::None) {
#endif
	//Alpha
	if (onSetAlpha) {
		switch (interpType) {
		case agtk::data::FrameData::Interpolate::None:
			onSetAlpha(nowFrameData->getFrameData()->getAlpha());
			break;
		case agtk::data::FrameData::Interpolate::Linear: {
			unsigned char alpha = AGTK_LINEAR_INTERPOLATE(
				nowFrameData->getFrameData()->getAlpha(),
				nextFrameData->getFrameData()->getAlpha(),
				range, pos
			);
			onSetAlpha(alpha);
			break; }
		case agtk::data::FrameData::Interpolate::Curve: {
			unsigned char alpha = (unsigned char)this->getSplineInterpolate(frameCount, _frameDataNo, "alpha");
			onSetAlpha(alpha);
			break; }
		}
	}
	//Color
	if (onSetColor) {
		switch (interpType) {
		case agtk::data::FrameData::Interpolate::None:
			onSetColor(
				nowFrameData->getFrameData()->getR(),
				nowFrameData->getFrameData()->getG(),
				nowFrameData->getFrameData()->getB()
			);
			break;
		case agtk::data::FrameData::Interpolate::Linear: {
			unsigned char r = AGTK_LINEAR_INTERPOLATE(
				nowFrameData->getFrameData()->getR(),
				nextFrameData->getFrameData()->getR(),
				range, pos
			);
			unsigned char g = AGTK_LINEAR_INTERPOLATE(
				nowFrameData->getFrameData()->getG(),
				nextFrameData->getFrameData()->getG(),
				range, pos
			);
			unsigned char b = AGTK_LINEAR_INTERPOLATE(
				nowFrameData->getFrameData()->getB(),
				nextFrameData->getFrameData()->getB(),
				range, pos
			);
			onSetColor(r, g, b);
			break; }
		case agtk::data::FrameData::Interpolate::Curve: {
			unsigned char r = (unsigned char)this->getSplineInterpolate(frameCount, _frameDataNo, "r");
			unsigned char g = (unsigned char)this->getSplineInterpolate(frameCount, _frameDataNo, "g");
			unsigned char b = (unsigned char)this->getSplineInterpolate(frameCount, _frameDataNo, "b");
			onSetColor(r, g, b);
			break; }
		}
	}
	//Scale
	if (onSetScale) {
		switch (interpType) {
		case agtk::data::FrameData::Interpolate::None:
			onSetScale(nowFrameData->getFrameData()->getScalingX() * 0.01f, nowFrameData->getFrameData()->getScalingY() * 0.01f);
			break;
		case agtk::data::FrameData::Interpolate::Linear: {
			float sx = AGTK_LINEAR_INTERPOLATE(
				nowFrameData->getFrameData()->getScalingX(),
				nextFrameData->getFrameData()->getScalingX(),
				range, pos
			);
			float sy = AGTK_LINEAR_INTERPOLATE(
				nowFrameData->getFrameData()->getScalingY(),
				nextFrameData->getFrameData()->getScalingY(),
				range, pos
			);
			onSetScale(sx * 0.01f, sy * 0.01f);
			break; }
		case agtk::data::FrameData::Interpolate::Curve: {
			float sx = this->getSplineInterpolate(frameCount, _frameDataNo, "scalingX");
			float sy = this->getSplineInterpolate(frameCount, _frameDataNo, "scalingY");
			onSetScale(sx * 0.01f, sy * 0.01f);
			break; }
		}
	}
	//Rotation
	if (onSetRotation) {
		switch (interpType) {
		case agtk::data::FrameData::Interpolate::None:
			onSetRotation(nowFrameData->getFrameData()->getRotation());
			break;
		case agtk::data::FrameData::Interpolate::Linear: {
			float rotation = AGTK_LINEAR_INTERPOLATE(
				nowFrameData->getFrameData()->getRotation(),
				nextFrameData->getFrameData()->getRotation(),
				range, pos
			);
			onSetRotation(rotation);
			break; }
		case agtk::data::FrameData::Interpolate::Curve: {
			float rotation = this->getSplineInterpolate(frameCount, _frameDataNo, "rotation");
			onSetRotation(rotation);
			break; }
		}
	}
	//Offset
	if (onSetOffset) {
		switch (interpType) {
		case agtk::data::FrameData::Interpolate::None: {
			onSetOffset(nowFrameData->getFrameData()->getOffsetX(), nowFrameData->getFrameData()->getOffsetY());
			break; }
		case agtk::data::FrameData::Interpolate::Linear: {
#define AGTK_LINEAR_INTERPOLATE(v1, v2, range, pos)		((v1) != (v2) ? (((v1) * ((range) - (pos)) + (v2) * (pos)) / (range)) : (v2))
			if (_bFrameFirst) {
				onSetOffset(nowFrameData->getFrameData()->getOffsetX(), nowFrameData->getFrameData()->getOffsetY());
			}
			else {
				float ofx = AGTK_LINEAR_INTERPOLATE(
					nowFrameData->getFrameData()->getOffsetX(),
					nextFrameData->getFrameData()->getOffsetX(),
					range, pos
				);
				float ofy = AGTK_LINEAR_INTERPOLATE(
					nowFrameData->getFrameData()->getOffsetY(),
					nextFrameData->getFrameData()->getOffsetY(),
					range, pos
				);
				onSetOffset(ofx, ofy);
			}
			break; }
		case agtk::data::FrameData::Interpolate::Curve: {
			float ofx = this->getSplineInterpolate(frameCount, _frameDataNo, "offsetX");
			float ofy = this->getSplineInterpolate(frameCount, _frameDataNo, "offsetY");
			onSetOffset(ofx, ofy);
			break; }
		}
	}
	//Center
	if (onSetCenter) {
		switch (interpType) {
		case agtk::data::FrameData::Interpolate::None: {
			float cx = nowFrameData->getFrameData()->getCenterX();
			float cy = nowFrameData->getFrameData()->getCenterY();
			onSetCenter(cx, cy);
			break; }
		case agtk::data::FrameData::Interpolate::Linear: {
			float cx = AGTK_LINEAR_INTERPOLATE(
				nowFrameData->getFrameData()->getCenterX(),
				nextFrameData->getFrameData()->getCenterX(),
				range, pos
			);
			float cy = AGTK_LINEAR_INTERPOLATE(
				nowFrameData->getFrameData()->getCenterY(),
				nextFrameData->getFrameData()->getCenterY(),
				range, pos
			);
			onSetCenter(cx, cy);
			break; }
		case agtk::data::FrameData::Interpolate::Curve: {
			float cx = this->getSplineInterpolate(frameCount, _frameDataNo, "centerX");
			float cy = this->getSplineInterpolate(frameCount, _frameDataNo, "centerY");
			onSetCenter(cx, cy);
			break; }
		}
	}
// #AGTK-NX
#ifdef USE_SAR_OPTIMIZE_6
	}

	if(pFrameCache) {
		if (interpType == agtk::data::FrameData::Interpolate::None) {
			*pFrameCache = nowFrameData;
		}
		else {
			*pFrameCache = nullptr;
		}
	}
#endif
	//------------------------------------------------------------------------------------------------
	// Collition
	directionData->update(frameCount);

	// フレーム固定されない場合
// #AGTK-NX #AGTK-WIN
#ifdef USE_30FPS
	bool bLocalFrameFirst = false;
	_bFrameFirst = false;
	bool bHalfFrame = false;
	float halfDt = 0.0f;

	// stepDt に dtの更新周期を設定する。
	// サウンド処理のため 1/60sec 以下、30fpsでの中間フレーム壁判定処理のためstepDtの整数倍がdt/2にほぼ等しくなるようにする必要がある。
	int progressScale = (int)GameManager::getInstance()->getFrameProgressScale();
	float stepDt = dt;
	if (stepDt > 1.0f / FRAME60_RATE) {
		stepDt = 1.0f / FRAME60_RATE;
	}
	if (progressScale > 1) {
		halfDt = stepDt = dt * 0.5f;
		while (stepDt > 1.0f / FRAME60_RATE) stepDt *= 0.5f;
	}

	for (float amountDt = dt; amountDt > FRAME_ALLOWABLE_ERROR; amountDt -= stepDt)
	{
		float dt = stepDt;
		if (amountDt <= dt + FRAME_ALLOWABLE_ERROR ) {
			dt = amountDt;
		}

		nowFrameData = directionData->getAnimationFrame(_frameDataNo);

		if (nowFrameData)
		{
			// スキップしないフレーム毎の処理
			// 30FPSでサウンド処理がスキップされないための対応
			if (_bFrameFirst && !this->getIgnoredSound())
			{
				//SE
				if (nowFrameData->getFrameData()->getPlaySe()) {
					if (_objectNode != nullptr) {
						// #AGTK-NX
#ifdef STATIC_DOWN_CAST
						auto _object = static_cast<agtk::Object *>(_objectNode);
#else
						auto _object = dynamic_cast<agtk::Object *>(_objectNode);
#endif
						_object->playSeObject(nowFrameData->getFrameData()->getPlaySeId());
					}
					else {
						AudioManager::getInstance()->playSe(nowFrameData->getFrameData()->getPlaySeId());
					}
				}
				//Voice
				if (nowFrameData->getFrameData()->getPlayVoice()) {
					if (_objectNode != nullptr) {
						// #AGTK-NX
#ifdef STATIC_DOWN_CAST
						auto _object = static_cast<agtk::Object *>(_objectNode);
#else
						auto _object = dynamic_cast<agtk::Object *>(_objectNode);
#endif
						_object->playVoiceObject(nowFrameData->getFrameData()->getPlayVoiceId());
					}
					else {
						AudioManager::getInstance()->playVoice(nowFrameData->getFrameData()->getPlayVoiceId());
					}
				}
			}
#ifdef USE_30FPS_4
			// 30fpsでの中間フレーム壁判定用のTransformデータ保存処理
			if (progressScale > 1 && !bHalfFrame && amountDt < halfDt + FRAME_ALLOWABLE_ERROR) {
				nextFrameData = directionData->getAnimationFrame(_frameDataNo + 1);
				if (nextFrameData == nullptr) {
					nextFrameData = nowFrameData;
				}
				this->updateMiddleFrame(nowFrameData, nextFrameData, seconds);
				bHalfFrame = true;
			}
#endif
		}
#endif
	if (_fixedFrameNo < 0) {
		if (_bReverse) {
			seconds -= dt;
			//無限ループ
			if (infiniteLoop) {
				if (reversePlay) {
					if (seconds < 0.0f) {
						seconds = -seconds;
						_bReverse = false;
						_reachedLastFrame = true;
					}
				}
			}
			else {
				if (reversePlay) {
					if (seconds < 0.0f) {
						_reachedLastFrame = true;
						if (_loopNum < motionData->getLoopCount()) {
							_loopNum++;
						}
						if (_loopNum >= motionData->getLoopCount()) {
							seconds = 0.0f;
							// アニメーションを再生完了
							_isAllAnimationFinished = true;
						} else {
							seconds = -seconds;
							if (seconds > secondsMax) seconds = secondsMax;
							_bReverse = false;
						}
					}
				}
			}
		}
		else {
			seconds += dt;
			//無限ループ
			if (infiniteLoop) {
				if (reversePlay) {
					if (seconds > secondsMax) {
						seconds = seconds - (seconds - secondsMax);
						_bReverse = true;
					}
				}
				else {
					if (seconds > secondsMax) {
						seconds = fmod(seconds, secondsMax);
						_reachedLastFrame = true;
					}
				}
			}
			else {
				if (reversePlay) {
					if (seconds > secondsMax) {
						seconds = seconds - (seconds - secondsMax);
						_bReverse = true;
					}
				}
				else {
					if (seconds > secondsMax) {
						if (_loopNum < motionData->getLoopCount()) {
							_loopNum++;
							if (seconds > secondsMax) {
								seconds = fmod(seconds, secondsMax);
							}
						}
						if (_loopNum >= motionData->getLoopCount()) {
							seconds = secondsMax;
							// アニメーションを再生完了
							_isAllAnimationFinished = true;
						}
						_reachedLastFrame = true;
					}
				}
			}
		}
	}

	//FrameNo
	unsigned int frameDataNo = _frameDataNo;

	// フレーム固定されない場合
	if (_fixedFrameNo < 0) {
// #AGTK-NX
#ifdef USE_SAR_OPTIMIZE_1
		int frameStartNo = -1;
		float seconds300 = seconds * 300.0f;
		if (_seconds < seconds) {
			if (seconds300 >= nowFrameData->getEndFrameCount()) {
				frameStartNo = _frameDataNo + 1;
				for (int n = frameStartNo; n < directionData->getAnimationFrameCount(); n++) {
					auto p = directionData->getAnimationFrame(n);
					if (seconds300 < p->getEndFrameCount()) {
						_frameDataNo = n;// p->getFrameData()->getId();
						break;
					}
				}
			}
		}
		else {
			if (seconds300 < nowFrameData->getStartFrameCount()) {
				auto p = directionData->getAnimationFrame(0);
				if (seconds300 < p->getEndFrameCount()) {
					for (int n = 0; n < directionData->getAnimationFrameCount(); n++) {
						auto p = directionData->getAnimationFrame(n);
						if (seconds300 < p->getEndFrameCount()) {
							_frameDataNo = n;
							break;
						}
					}
				}
				else {
					frameStartNo = _frameDataNo - 1;
					for (int n = frameStartNo; n >= 0; n--) {
						auto p = directionData->getAnimationFrame(n);
						if (seconds300 >= p->getStartFrameCount()) {
							_frameDataNo = n;// p->getFrameData()->getId();
							break;
						}
					}
				}
			}
		}
#else
		int frameStartNo = -1;
		if (_seconds < seconds) {
			if (seconds * 300.0f >= nowFrameData->getEndFrameCount()) {
				frameStartNo = _frameDataNo + 1;
			}
		}
		else {
			frameStartNo = 0;
		}
		if (frameStartNo >= 0) {
			for (int n = frameStartNo; n < directionData->getAnimationFrameCount(); n++) {
				auto p = directionData->getAnimationFrame(n);
				if (seconds * 300.0f < p->getEndFrameCount()) {
					_frameDataNo = n;
					break;
				}
			}
		}
#endif
	}

	_bFrameFirst = (_frameDataNo != frameDataNo) ? true : false;
// #AGTK-NX #AGTK-WIN
#ifdef USE_30FPS
	if (_bFrameFirst) {
		bLocalFrameFirst = true;
	}
#endif
	_seconds = seconds;
	_frame300 += (dt * 300);
// #AGTK-NX #AGTK-WIN
#ifdef USE_30FPS
	}
	_bFrameFirst = bLocalFrameFirst;
#endif
#ifdef USE_SAR_OHNISHI_DEBUG
	if (strcmp(getName(), "whip") == 0 || strcmp(getName(), "whip_end") == 0) {
		CCLOG("# motion name: %s, sec: %8.3f, bFrameFirst: %d, isAllAnimationFinished: %d, reachedLastFrame: %d", getName(), _seconds, (int)_bFrameFirst, (int)_isAllAnimationFinished, (int)_reachedLastFrame);
	}
#endif
}

// #AGTK-NX #AGTK-WIN
#ifdef USE_30FPS_4
// 30FPSでの中間フレームの壁判定を得るためのアニメーションによるTransformパラメータ更新
void AnimationMotion::updateMiddleFrame(agtk::AnimationFrame *now, agtk::AnimationFrame *next, float seconds)
{
	agtk::Object* object = static_cast<agtk::Object*>(_objectNode);
	if (object == nullptr) {
		// Effectは_objectNodeがnull
		return;
	}
	BasePlayer* basePlayer = object->getBasePlayer();
	if (!basePlayer) {
		return;
	}

	MiddleFrameStock* ms = &object->_middleFrameStock;
	ms->updateAnimation();
	MiddleFrame* mf = ms->getUpdatingMiddleFrame();
	agtk::AnimationFrame *nowFrameData = now;
	agtk::AnimationFrame *nextFrameData = next;

	auto interpType = nowFrameData->getFrameData()->getInterpType();
	float range = (nowFrameData->getEndFrameCount() - nowFrameData->getStartFrameCount()) / 300.0f;
	float pos = seconds - nowFrameData->getStartFrameCount() / 300.0f;
	if (range < pos) pos = range;
	int frameCount = (int)(seconds * 300.0f);

	//Scale
	if (onSetScale) {
		switch (interpType) {
			case agtk::data::FrameData::Interpolate::None:
				mf->_innerScale = Vec2(nowFrameData->getFrameData()->getScalingX() * 0.01f, nowFrameData->getFrameData()->getScalingY() * 0.01f);
				break;
			case agtk::data::FrameData::Interpolate::Linear: {
					float sx = AGTK_LINEAR_INTERPOLATE(
						nowFrameData->getFrameData()->getScalingX(),
						nextFrameData->getFrameData()->getScalingX(),
						range, pos
					);
					float sy = AGTK_LINEAR_INTERPOLATE(
						nowFrameData->getFrameData()->getScalingY(),
						nextFrameData->getFrameData()->getScalingY(),
						range, pos
					);
					mf->_innerScale = Vec2(sx * 0.01f, sy * 0.01f);
					break; }
			case agtk::data::FrameData::Interpolate::Curve: {
					float sx = this->getSplineInterpolate(frameCount, _frameDataNo, "scalingX");
					float sy = this->getSplineInterpolate(frameCount, _frameDataNo, "scalingY");
					mf->_innerScale = Vec2(sx * 0.01f, sy * 0.01f);
					break; }
		}
		if(basePlayer->getInnerScale() != mf->_innerScale) {
			mf->_hasMiddleFrame = true;
		}
	}
	//Rotation
	if (onSetRotation) {
		switch (interpType) {
			case agtk::data::FrameData::Interpolate::None:
				mf->_innerRotation = (nowFrameData->getFrameData()->getRotation());
				break;
			case agtk::data::FrameData::Interpolate::Linear: {
					float rotation = AGTK_LINEAR_INTERPOLATE(
						nowFrameData->getFrameData()->getRotation(),
						nextFrameData->getFrameData()->getRotation(),
						range, pos
					);
					mf->_innerRotation = (rotation);
					break; }
			case agtk::data::FrameData::Interpolate::Curve: {
					float rotation = this->getSplineInterpolate(frameCount, _frameDataNo, "rotation");
					mf->_innerRotation = (rotation);
					break; }
		}
		if (basePlayer->getInnerRotation() != mf->_innerRotation) {
			mf->_hasMiddleFrame = true;
		}
	}
	//Offset
	if (onSetOffset) {
		switch (interpType) {
			case agtk::data::FrameData::Interpolate::None: {
					mf->_offset = Vec2(nowFrameData->getFrameData()->getOffsetX(), nowFrameData->getFrameData()->getOffsetY());
					break; }
			case agtk::data::FrameData::Interpolate::Linear: {
#define AGTK_LINEAR_INTERPOLATE(v1, v2, range, pos)		((v1) != (v2) ? (((v1) * ((range) - (pos)) + (v2) * (pos)) / (range)) : (v2))
					float ofx = AGTK_LINEAR_INTERPOLATE(
						nowFrameData->getFrameData()->getOffsetX(),
						nextFrameData->getFrameData()->getOffsetX(),
						range, pos
					);
					float ofy = AGTK_LINEAR_INTERPOLATE(
						nowFrameData->getFrameData()->getOffsetY(),
						nextFrameData->getFrameData()->getOffsetY(),
						range, pos
					);
					mf->_offset = Vec2(ofx, ofy);
					break; }
			case agtk::data::FrameData::Interpolate::Curve: {
					float ofx = this->getSplineInterpolate(frameCount, _frameDataNo, "offsetX");
					float ofy = this->getSplineInterpolate(frameCount, _frameDataNo, "offsetY");
					mf->_offset = Vec2(ofx, ofy);
					break; }
		}
		if (basePlayer->getOffset() != mf->_offset) {
			mf->_hasMiddleFrame = true;
		}
	}
	//Center
	if (onSetCenter) {
		switch (interpType) {
			case agtk::data::FrameData::Interpolate::None: {
					float cx = nowFrameData->getFrameData()->getCenterX();
					float cy = nowFrameData->getFrameData()->getCenterY();
					mf->_center = Vec2(cx, cy);
					break; }
			case agtk::data::FrameData::Interpolate::Linear: {
					float cx = AGTK_LINEAR_INTERPOLATE(
						nowFrameData->getFrameData()->getCenterX(),
						nextFrameData->getFrameData()->getCenterX(),
						range, pos
					);
					float cy = AGTK_LINEAR_INTERPOLATE(
						nowFrameData->getFrameData()->getCenterY(),
						nextFrameData->getFrameData()->getCenterY(),
						range, pos
					);
					mf->_center = Vec2(cx, cy);
					break; }
			case agtk::data::FrameData::Interpolate::Curve: {
					float cx = this->getSplineInterpolate(frameCount, _frameDataNo, "centerX");
					float cy = this->getSplineInterpolate(frameCount, _frameDataNo, "centerY");
					mf->_center = Vec2(cx, cy);
					break; }
		}
		if (basePlayer->getCenter() != mf->_center) {
			mf->_hasMiddleFrame = true;
		}
	}
}
#endif


void AnimationMotion::skipOneFrame(float dt)
{
	auto directionData = this->getCurrentDirection();
	auto motionData = this->getMotionData();
	bool infiniteLoop = motionData->getInfiniteLoop();
	bool reversePlay = motionData->getReversePlay();

	float seconds = _seconds;
	float secondsMax = (float)directionData->getMaxFrameCount300() / 300.0f;
	int frameCount = (int)(seconds * 300.0f);

	//------------------------------------------------------------------------------------------------
	auto nowFrameData = directionData->getAnimationFrame(_frameDataNo);
	if (nowFrameData == nullptr) {
		return;
	}

	// フレーム固定されない場合
// #AGTK-NX
#ifdef USE_SAR_OPTIMIZE_1
	if (_fixedFrameNo < 0) {
		if (_bReverse) {
			seconds -= dt;
			int reachCount = (int)(seconds / secondsMax);
			if (seconds < 0.0f)
			{
				reachCount--;
			}
			if (reachCount < 0)
			{
				reachCount = -reachCount;
			}
			if (!_reachedLastFrame) {
				_reachedLastFrame = (reachCount > 0);
			}
			//無限ループ
			if (infiniteLoop) {
				seconds = fmod(seconds, secondsMax);
				if (reversePlay) {
					if (reachCount & 1)
					{
						if (seconds < 0.0f) {
							seconds = -seconds;
							_bReverse = false;
						}
					}
					else {
						if (seconds < 0.0f) {
							seconds = seconds + secondsMax;
						}
					}
				}
			}
			else {
				seconds = fmod(seconds, secondsMax);
				if (reversePlay) {
					if (motionData->getLoopCount() < _loopNum + reachCount)
					{
						reachCount -= (_loopNum + reachCount - motionData->getLoopCount());
					}
					if (reachCount & 1)
					{
						if (seconds < 0.0f) {
							seconds = -seconds;
							_bReverse = false;
						}
					}
					else {
						if (seconds < 0.0f) {
							seconds = seconds + secondsMax;
						}
					}
					_loopNum += reachCount;
					if (_loopNum >= motionData->getLoopCount()) {
						seconds = 0.0f;
						// アニメーションを再生完了
						_isAllAnimationFinished = true;
					}
				}
			}
		}
		else {
			seconds += dt;
			int reachCount = (int)(seconds / secondsMax);
			if (!_reachedLastFrame) {
				_reachedLastFrame = (reachCount > 0);
			}
			//無限ループ
			if (infiniteLoop) {
				seconds = fmod(seconds, secondsMax);
				if (reversePlay) {
					if (reachCount & 1)
					{
						if (reachCount > 0) {
							//※ SAR ohnishi 順方向の再生と折り返し処理が異なるため最新バージョンで確認
#ifdef USE_SAR_PROVISIONAL
							seconds = secondsMax - seconds;
#else
							seconds = seconds - (seconds - secondsMax);
#endif
							_bReverse = true;
						}
					}
				}
			}
			else {
				seconds = fmod(seconds, secondsMax);
				if (motionData->getLoopCount() < _loopNum + reachCount)
				{
					reachCount -= (_loopNum + reachCount - motionData->getLoopCount());
				}
				if (reversePlay) {
					if (reachCount & 1)
					{
						if (reachCount > 0) {
							//※ SAR ohnishi 順方向の再生と折り返し処理が異なるため最新バージョンで確認
#ifdef USE_SAR_PROVISIONAL
							seconds = secondsMax - seconds;
#else
							seconds = seconds - (seconds - secondsMax);
#endif
							_bReverse = true;
						}
					}
				}
				_loopNum += reachCount;
				if (_loopNum >= motionData->getLoopCount()) {
					seconds = secondsMax;
					// アニメーションを再生完了
					_isAllAnimationFinished = true;
				}
			}
		}
	}
#else
	if (_fixedFrameNo < 0) {
		if (_bReverse) {
			seconds -= dt;
			//無限ループ
			if (infiniteLoop) {
				if (reversePlay) {
					if (seconds < 0.0f) {
						seconds = -seconds;
						_bReverse = false;
						_reachedLastFrame = true;
					}
				}
			}
			else {
				if (reversePlay) {
					if (seconds < 0.0f) {
						seconds = -seconds;
						_bReverse = false;
						_reachedLastFrame = true;
						_loopNum++;
						if (_loopNum >= motionData->getLoopCount()) {
							seconds = 0.0f;
							// アニメーションを再生完了
							_isAllAnimationFinished = true;
						}
					}
				}
			}
		}
		else {
			seconds += dt;
			//無限ループ
			if (infiniteLoop) {
				if (reversePlay) {
					if (seconds > secondsMax) {
						seconds = seconds - (seconds - secondsMax);
						_bReverse = true;
					}
				}
				else {
					if (seconds > secondsMax) {
						seconds = fmod(seconds, secondsMax);
						_reachedLastFrame = true;
					}
				}
			}
			else {
				if (reversePlay) {
					if (seconds > secondsMax) {
						seconds = seconds - (seconds - secondsMax);
						_bReverse = true;
					}
				}
				else {
					if (seconds > secondsMax) {
						_loopNum += 1;
						if (_loopNum < motionData->getLoopCount()) {
							if (seconds > secondsMax) {
								seconds = fmod(seconds, secondsMax);
							}
						}
						else {
							seconds = secondsMax;
							// アニメーションを再生完了
							_isAllAnimationFinished = true;
						}
						_reachedLastFrame = true;
					}
				}
			}
		}
	}
#endif
	//FrameNo
	unsigned int frameDataNo = _frameDataNo;

	// フレーム固定される場合
	if (_fixedFrameNo > -1) {
		auto animationFrameCountMax = directionData->getAnimationFrameCount() - 1;
		_frameDataNo = _fixedFrameNo > animationFrameCountMax ? animationFrameCountMax : _fixedFrameNo;
	}
	// フレーム固定されない場合
	else {
// #AGTK-NX
#ifdef USE_SAR_OPTIMIZE_1
		int frameStartNo = -1;
		float seconds300 = seconds * 300.0f;
		if (_seconds < seconds) {
			if (seconds300 >= nowFrameData->getEndFrameCount()) {
				frameStartNo = _frameDataNo + 1;
				for (int n = frameStartNo; n < directionData->getAnimationFrameCount(); n++) {
					auto p = directionData->getAnimationFrame(n);
					if (seconds300 < p->getEndFrameCount()) {
						_frameDataNo = n;// p->getFrameData()->getId();
						break;
					}
				}
			}
		}
		else {
			if (seconds300 < nowFrameData->getStartFrameCount()) {
				auto p = directionData->getAnimationFrame(0);
				if (seconds300 < p->getEndFrameCount()) {
					for (int n = 0; n < directionData->getAnimationFrameCount(); n++) {
						auto p = directionData->getAnimationFrame(n);
						if (seconds300 < p->getEndFrameCount()) {
							_frameDataNo = n;
							break;
						}
					}
				}
				else {
					frameStartNo = _frameDataNo - 1;
					for (int n = frameStartNo; n >= 0; n--) {
						auto p = directionData->getAnimationFrame(n);
						if (seconds300 >= p->getStartFrameCount()) {
							_frameDataNo = n;// p->getFrameData()->getId();
							break;
						}
					}
				}
			}
		}
#else
		int frameStartNo = -1;
		if (_seconds < seconds) {
			if (seconds * 300.0f > nowFrameData->getEndFrameCount()) {
				frameStartNo = _frameDataNo + 1;
			}
		}
		else {
			frameStartNo = 0;
		}
		if (frameStartNo >= 0) {
			for (int n = frameStartNo; n < directionData->getAnimationFrameCount(); n++) {
				auto p = directionData->getAnimationFrame(n);
				if (seconds * 300.0f < p->getEndFrameCount()) {
					_frameDataNo = n;// p->getFrameData()->getId();
					break;
				}
			}
		}
#endif
	}

	_bFrameFirst = (_frameDataNo != frameDataNo) ? true : false;
	_seconds = seconds;
	_frame300 += (dt * 300);
}

void AnimationMotion::updateCollision(float dt)
{
}

float AnimationMotion::getSplineInterpolate(int frame, int frameDataNo, std::string name)
{
	auto directionData = this->getCurrentDirection();
	int keyframe0 = -1;
	int keyframe1 = frameDataNo;
	int keyframe2 = -1;
	int keyframe3 = -1;
	agtk::data::FrameData *frameData0 = nullptr;
	agtk::data::FrameData *frameData1 = directionData->getAnimationFrame(keyframe1)->getFrameData();
	agtk::data::FrameData *frameData2 = nullptr;
	agtk::data::FrameData *frameData3 = nullptr;
	int keyFrame300_0 = -1;
	int keyFrame300_1 = directionData->getAnimationFrame(keyframe1)->getStartFrameCount();
	int keyFrame300_2 = -1;
	int keyFrame300_3 = -1;
	if (keyframe1 < 0) {
		CC_ASSERT(0);
		return 0.0f;
	}
	if (keyframe1 > 0) {
		keyframe0 = keyframe1 - 1;
		frameData0 = directionData->getAnimationFrame(keyframe0)->getFrameData();
		keyFrame300_0 = directionData->getAnimationFrame(keyframe0)->getStartFrameCount();
	}
	if (keyframe1 + 1 < directionData->getAnimationFrameCount()) {
		keyframe2 = keyframe1 + 1;
		frameData2 = directionData->getAnimationFrame(keyframe2)->getFrameData();
		keyFrame300_2 = directionData->getAnimationFrame(keyframe2)->getStartFrameCount();
	}
	if (keyframe2 >= 0 && keyframe2 + 1 < directionData->getAnimationFrameCount()) {
		keyframe3 = keyframe2 + 1;
		frameData3 = directionData->getAnimationFrame(keyframe3)->getFrameData();
		keyFrame300_3 = directionData->getAnimationFrame(keyframe3)->getStartFrameCount();
	}
	if (keyframe2 < 0) {
		//補間に必要な２つ目のキーフレームが見つからない or ステップ補間。１つ目のキーフレームでの値を返す。
		return getFrameKeyframeValue(frameData1, name);
	}
	SplineInterp spline;
	float value;
	//! ACT2-1332 のエディタ側対応に合わせるよう変更
	//! キーが等間隔に打たれているとみなして補完 
#if 1
	int interval = keyFrame300_2 - keyFrame300_1;
	if (keyframe0 >= 0) {
		spline.AddKey(0, getFrameKeyframeValue(frameData0, name));
	}
	spline.AddKey(interval, getFrameKeyframeValue(frameData1, name));
	spline.AddKey(interval * 2, getFrameKeyframeValue(frameData2, name));
	if (keyframe3 >= 0) {
		spline.AddKey(interval * 3, getFrameKeyframeValue(frameData3, name));
	}
	value = spline.GetInterpolated(interval + (frame - keyFrame300_1) * interval / (keyFrame300_2 - keyFrame300_1));
#else
	if (keyframe0 >= 0) {
		spline.AddKey(keyFrame300_0, getFrameKeyframeValue(frameData0, name));
	}
	spline.AddKey(keyFrame300_1, getFrameKeyframeValue(frameData1, name));
	spline.AddKey(keyFrame300_2, getFrameKeyframeValue(frameData2, name));
	if (keyframe3 >= 0) {
		spline.AddKey(keyFrame300_3, getFrameKeyframeValue(frameData3, name));
	}
	value = spline.GetInterpolated(frame);
#endif

	if (name == "alpha" || name == "r" || name == "g" || name == "b") {
		return max(0, min(255, (int)(value + 0.5f)));
	}
	return value;
}

float AnimationMotion::getFrameKeyframeValue(agtk::data::FrameData *frameData, std::string name)
{
	if (name == "frameCount300") {
		return frameData->getFrameCount300();
	}
	else if (name == "interpType") {
		return frameData->getInterpType();
	}
	else if (name == "playSe") {
		return frameData->getPlaySe();
	}
	else if (name == "playSeId") {
		return frameData->getPlaySeId();
	}
	else if (name == "playVoice") {
		return frameData->getPlayVoice();
	}
	else if (name == "playVoiceId") {
		return frameData->getPlayVoiceId();
	}
	else if (name == "centerOrigin") {
		return frameData->getCenterOrigin();
	}
	else if (name == "originX") {
		return frameData->getOriginX();
	}
	else if (name == "originY") {
		return frameData->getOriginY();
	}
	else if (name == "offsetX") {
		return frameData->getOffsetX();
	}
	else if (name == "offsetY") {
		return frameData->getOffsetY();
	}
	else if (name == "centerX") {
		return frameData->getCenterX();
	}
	else if (name == "centerY") {
		return frameData->getCenterY();
	}
	else if (name == "scalingX") {
		return frameData->getScalingX();
	}
	else if (name == "scalingY") {
		return frameData->getScalingY();
	}
	else if (name == "flipX") {
		return frameData->getFlipX();
	}
	else if (name == "flipY") {
		return frameData->getFlipY();
	}
	else if (name == "rotation") {
		return frameData->getRotation();
	}
	else if (name == "alpha") {
		return frameData->getAlpha();
	}
	else if (name == "r") {
		return frameData->getR();
	}
	else if (name == "g") {
		return frameData->getG();
	}
	else if (name == "b") {
		return frameData->getB();
	}
	CC_ASSERT(0);
	return 0.0f;
}

bool AnimationMotion::isRunning()
{
	return (this->getCurrentDirection() != nullptr);
}

const char *AnimationMotion::getName()
{
	return this->getMotionData()->getName();
}

agtk::AnimationDirection *AnimationMotion::getAnimationDirection(int id)
{
	return dynamic_cast<agtk::AnimationDirection *>(this->getAnimationDirectionList()->objectForKey(id));
}

agtk::AnimationDirection *AnimationMotion::getAnimationDirection(std::string name)
{
	cocos2d::DictElement *el = nullptr;
	auto animationDirectionList = this->getAnimationDirectionList();
	CCDICT_FOREACH(animationDirectionList, el) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
		auto p = static_cast<agtk::AnimationDirection *>(el->getObject());
#else
		auto p = dynamic_cast<agtk::AnimationDirection *>(el->getObject());
#endif
		if (name.compare(p->getName()) == 0) {
			return p;
		}
	}
	return nullptr;
}

int AnimationMotion::getAnimationDirectionCount()
{
	return this->getAnimationDirectionList()->count();
}

bool AnimationMotion::isAnimationFinished()
{
	auto animationDirection = this->getCurrentDirection();
	return _frame300 >= animationDirection->getMaxFrameCount300();
}

bool AnimationMotion::isAllAnimationFinished()
{
	return _isAllAnimationFinished;
}

void AnimationMotion::setFixedFrameNo(int frameNo)
{
	_fixedFrameNo = frameNo;
	auto directionData = this->getCurrentDirection();
	if (directionData) {
		auto frameData = directionData->getAnimationFrame(frameNo);
		if (frameData) {
			int frameCount = frameData->getStartFrameCount();
			_fixedFrame = frameCount / 5;
			_seconds = frameCount / 300.0f;
		}
	}
}

void AnimationMotion::setFixedFrame(int frame)
{
	int oldFixedFrameNo = _fixedFrameNo;
	_fixedFrameNo = -2;
	_fixedFrame = frame;
	if (frame == kAnimPauseFrameValue) {
		// kAnimPauseFrameValueが指定された場合は、現在再生中のフレームで固定する。
		_fixedFrameNo = _frameDataNo;
		_fixedFrame = (int)(_seconds * 300 / 5);
		return;
	}
	if (frame < 0.0f) {
		return;
	}
	auto directionData = this->getCurrentDirection();
	if (directionData) {
		for (int frameNo = 0; frameNo < directionData->getAnimationFrameCount(); frameNo++) {
			auto frameData = directionData->getAnimationFrame(frameNo);
			unsigned int frameCount = frame * 5;
			if (frameData->getStartFrameCount() <= frameCount && frameCount < frameData->getEndFrameCount()) {
				_fixedFrameNo = frameNo;
				_seconds = frameCount / 300.0f;
				break;
			}
		}
		//指定フレームが範囲外の場合は、範囲内に収まるよに調整。
		if (_fixedFrameNo < 0) {
			CC_ASSERT(frame > 0);
			int frameNo = directionData->getAnimationFrameCount() - 1;
			auto frameData = directionData->getAnimationFrame(frameNo);
			_fixedFrameNo = frameNo;
			_seconds = (float)(frameData->getEndFrameCount() - 1) / 300.0f;
		}
		CC_ASSERT(_fixedFrameNo >= 0);
		if (_fixedFrameNo >= 0 && _fixedFrameNo != oldFixedFrameNo) {
			_bFrameFirst = true;
		}
	}
}

NS_AGTK_END
