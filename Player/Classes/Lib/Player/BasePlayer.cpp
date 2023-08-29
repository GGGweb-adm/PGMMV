#include "BasePlayer.h"

NS_AGTK_BEGIN

#ifdef AGTK_DEBUG
const char *BasePlayer::DebugDisplayName = "DebugDisplay";
#endif
BasePlayer::BasePlayer()
{
	_frameNo = 0;
	_type = None;
	_offset = cocos2d::Vec2::ZERO;
	_origin = cocos2d::Vec2::ZERO;
	_center = cocos2d::Vec2::ZERO;
	_pos = cocos2d::Vec2::ZERO;
	_scale = _innerScale = cocos2d::Vec2::ONE;
	_rotation = _innerRotation = 0.0f;
	onSetPosition = nullptr;
	onSetScale = nullptr;
	onSetRotation = nullptr;
	_animationData = nullptr;
	_animationMotionList = nullptr;
	_currentAnimationMotion = nullptr;
	_originOffset = cocos2d::Vec2(0, 0);

	_refVisibleCtrlNode = nullptr;
	_hasRotation = false;

	_objectNode = nullptr;
// #AGTK-NX
#ifdef USE_SAR_OPTIMIZE_6
	_frameCache = nullptr;
#endif
}

BasePlayer::~BasePlayer()
{
	_refVisibleCtrlNode = nullptr;
	onSetPosition = nullptr;
	onSetScale = nullptr;
	onSetRotation = nullptr;
	CC_SAFE_RELEASE_NULL(_animationData);
	CC_SAFE_RELEASE_NULL(_animationMotionList);
	CC_SAFE_RELEASE_NULL(_currentAnimationMotion);
}

/**
* 固定フレーム番号の設定
* @param	frameNo		固定したいフレームの番号
*						0 より小さい値が渡された場合は固定解除とする
*/
void BasePlayer::setFixedFrameNo(int frameNo, bool bEnableFrameSkip)
{
	auto animeMotion = this->getCurrentAnimationMotion();
	if (animeMotion) {
		auto oldFrameNo = animeMotion->getFixedFrameNo();
		animeMotion->setFixedFrameNo(frameNo);

		// 固定解除の場合
		if (bEnableFrameSkip && frameNo < 0) {
			// 固定していたフレームから開始させる為、経過時間を固定フレームに相当する値で設定する
			auto currentDirection = animeMotion->getCurrentDirection();
			if (oldFrameNo >= currentDirection->getAnimationFrameCount()) {
				oldFrameNo = currentDirection->getAnimationFrameCount() - 1;
			}
			if (oldFrameNo >= 0) {
				int frameCount = currentDirection->getAnimationFrame(oldFrameNo)->getEndFrameCount();
				animeMotion->_seconds = frameCount / 300.0f;
				animeMotion->_frame300 = frameCount;
				animeMotion->_bRestoreFixFrame = true;
			}
		}
	}
}

/**
* 固定フレーム番号の取得
* @note					設定されている値は 1 減算されているので返却時は 1 加算しておく
*/
int BasePlayer::getFixedFrameNo()
{
	int ret = -1;
	auto animeMotion = this->getCurrentAnimationMotion();
	if (animeMotion) {
		ret = animeMotion->getFixedFrameNo() + 1;
	}

	return ret;
}

void BasePlayer::setFixedFrame(int frame, bool bEnableFrameSkip)
{
	auto animeMotion = this->getCurrentAnimationMotion();
	if (animeMotion) {
		auto oldFrameNo = animeMotion->getFixedFrameNo();
		auto oldFrame = animeMotion->getFixedFrame();
		animeMotion->setFixedFrame(frame);

		// 固定解除の場合
		if (bEnableFrameSkip && frame < 0) {
			// 固定していたフレームから開始させる為、経過時間を固定フレームに相当する値で設定する
			auto currentDirection = animeMotion->getCurrentDirection();
			if (currentDirection != nullptr) {
				if (oldFrameNo >= currentDirection->getAnimationFrameCount()) {
					oldFrameNo = currentDirection->getAnimationFrameCount() - 1;
				}
				if (oldFrameNo >= 0) {
					int frameCount = currentDirection->getAnimationFrame(oldFrameNo)->getEndFrameCount();
					animeMotion->_seconds = frameCount / 300.0f;
					animeMotion->_frame300 = frameCount;
					animeMotion->_bRestoreFixFrame = true;
				}
			}
			else {
				int frameCount = 0;
				animeMotion->_seconds = frameCount / 300.0f;
				animeMotion->_frame300 = frameCount;
				animeMotion->_bRestoreFixFrame = true;
			}
		}
	}
}

int BasePlayer::getFixedFrame()
{
	int ret = -1;
	auto animeMotion = this->getCurrentAnimationMotion();
	if (animeMotion) {
		ret = animeMotion->getFixedFrame();
	}
	return ret;
}

void BasePlayer::setOffset(const cocos2d::Vec2& offset)
{
	setOffset(offset.x, offset.y);
}

void BasePlayer::setOffset(float x, float y)
{
	if (_offset.x == x && _offset.y == y) {
		return;
	}
	_offset.x = x;
	_offset.y = y;
	updateRealPosition();
}

const cocos2d::Vec2& BasePlayer::getOffset() const
{
	return _offset;
}

void BasePlayer::setOrigin(const cocos2d::Vec2& origin)
{
	setOrigin(origin.x, origin.y);
}

void BasePlayer::setOrigin(float x, float y)
{
	if (_origin.x == x && _origin.y == y) {
		return;
	}
	_origin.x = x;
	_origin.y = y;
	updateRealPosition();
}

const cocos2d::Vec2& BasePlayer::getCenter() const
{
	return _center;
}

void BasePlayer::setCenter(const cocos2d::Vec2& origin)
{
	setCenter(origin.x, origin.y);
}

void BasePlayer::setCenter(float x, float y)
{
	if (_center.x == x && _center.y == y) {
		return;
	}
	_center.x = x;
	_center.y = y;
	updateRealPosition();
}

const cocos2d::Vec2& BasePlayer::getOrigin() const
{
	return _origin;
}

void BasePlayer::setInnerScale(const cocos2d::Vec2& scale)
{
	setInnerScale(scale.x, scale.y);
}

void BasePlayer::setInnerScale(float x, float y)
{
	_innerScale.x = x;
	_innerScale.y = y;
	updateRealScale();
}

const cocos2d::Vec2& BasePlayer::getInnerScale() const
{
	return _innerScale;
}

void BasePlayer::setInnerRotation(float rotation)
{
	_innerRotation = rotation;
	updateRealRotation();
}

float BasePlayer::getInnerRotation() const
{
	return _innerRotation;
}

cocos2d::Vec2 BasePlayer::getRealPosition()
{
	return cocos2d::Vec2(_pos.x + _origin.x + _offset.x + _center.x, _pos.y + _origin.y - _offset.y - _center.y);
}

void BasePlayer::updateRealPosition()
{
	cocos2d::Vec2 pos = getRealPosition();
	CC_ASSERT(onSetPosition);
	onSetPosition(pos.x, pos.y);
}

cocos2d::Vec2 BasePlayer::getRealScale()
{
	return cocos2d::Vec2(_scale.x * _innerScale.x, _scale.y * _innerScale.y);
}

void BasePlayer::updateRealScale()
{
	cocos2d::Vec2 scale = getRealScale();
	CC_ASSERT(onSetScale);
	onSetScale(scale.x, scale.y);
}

float BasePlayer::getRealRotation()
{
	return _rotation + _innerRotation;
}

void BasePlayer::updateRealRotation()
{
	float rotation = getRealRotation();
	CC_ASSERT(onSetRotation);
	onSetRotation(rotation);
}

bool BasePlayer::isNowPlaying(int actionNo, int actionDirectNo)
{
	auto motion = this->getCurrentAnimationMotion();
	if (motion) {
		return (motion->getMotionData()->getId() == actionNo && motion->getCurrentDirection()->getDirectionData()->getId() == actionDirectNo);
	}
	return false;
}

void BasePlayer::setAnimationData(agtk::data::AnimationData *animationData)
{
	if (_animationData != animationData)
	{
		CC_SAFE_RETAIN(animationData);
		CC_SAFE_RELEASE(_animationData);
		_animationData = animationData;
	}
	_hasRotation = false;
	auto motionDic = _animationData->getMotionList();
	if (motionDic != nullptr) {
		cocos2d::DictElement *el = nullptr;
		CCDICT_FOREACH(motionDic, el) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
			auto motionData = static_cast<agtk::data::MotionData *>(el->getObject());
#else
			auto motionData = dynamic_cast<agtk::data::MotionData *>(el->getObject());
#endif
			auto directionDic = motionData->getDirectionList();
			cocos2d::DictElement *el2 = nullptr;
			CCDICT_FOREACH(directionDic, el2) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
				auto directionData = static_cast<agtk::data::DirectionData *>(el2->getObject());
#else
				auto directionData = dynamic_cast<agtk::data::DirectionData *>(el2->getObject());
#endif
				auto frameDic = directionData->getFrameList();
				cocos2d::DictElement *el3 = nullptr;
				CCDICT_FOREACH(frameDic, el3) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
					auto frameData = static_cast<agtk::data::FrameData *>(el3->getObject());
#else
					auto frameData = dynamic_cast<agtk::data::FrameData *>(el3->getObject());
#endif
					if (frameData->getRotation() != 0) {
						_hasRotation = true;
						goto LEnd;
					}
				}
			}
		}
	}
// #AGTK-NX
#if CC_TARGET_PLATFORM == CC_PLATFORM_NX
#endif
	auto frameDic = _animationData->getFrameList();
	if (frameDic != nullptr) {
		cocos2d::DictElement *el3;
		CCDICT_FOREACH(frameDic, el3) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
			auto frameData = static_cast<agtk::data::FrameData *>(el3->getObject());
#else
			auto frameData = dynamic_cast<agtk::data::FrameData *>(el3->getObject());
#endif
			if (frameData->getRotation() != 0) {
				_hasRotation = true;
				goto LEnd;
			}
		}
	}
// #AGTK-NX
#if CC_TARGET_PLATFORM == CC_PLATFORM_NX
#endif
LEnd:;
	//CCLOG("_hasRotation: %d", _hasRotation);
}

NS_AGTK_END
