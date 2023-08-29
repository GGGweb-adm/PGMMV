#include "ImagePlayer.h"
#include "Manager/PrimitiveManager.h"
#include "Manager/GameManager.h"

NS_AGTK_BEGIN

ImagePlayer *(*ImagePlayer::createWithAnimationData)(agtk::data::AnimationData *animationData, agtk::data::ResourceInfoData *resourceInfoData, int imageId) = ImagePlayer::_createWithAnimationData;

//-------------------------------------------------------------------------------------------------

ImagePlayer::ImagePlayer() : cocos2d::Sprite(), agtk::BasePlayer()
{
	_type = agtk::BasePlayer::Image;
	_leftupAnchorPoint = cocos2d::Vec2(0.0f, 1.0f);
}

ImagePlayer::~ImagePlayer()
{
#if defined(AGTK_DEBUG)
	auto debugNode = this->getDebugNode();
	if (debugNode) {
		auto pm = PrimitiveManager::getInstance();
		pm->remove(debugNode);
		debugNode->removeFromParent();
	}
#endif
}

void ImagePlayer::setCreateWithAnimationData(ImagePlayer *(*createWithAnimationData)(agtk::data::AnimationData *animationData, agtk::data::ResourceInfoData *resourceInfoData, int imageId))
{
	ImagePlayer::createWithAnimationData = createWithAnimationData;
}

ImagePlayer *ImagePlayer::_createWithAnimationData(agtk::data::AnimationData *animationData, agtk::data::ResourceInfoData *resourceInfoData, int imageId)
{
	auto p = new (std::nothrow) ImagePlayer();
	if (p && p->initWithAnimationData(animationData, resourceInfoData, imageId)) {
		p->autorelease();
		return p;
	}
	CC_SAFE_DELETE(p);
	return nullptr;
}

bool ImagePlayer::initWithAnimationData(agtk::data::AnimationData *animationData, agtk::data::ResourceInfoData * resourceInfoData, int imageId)
{
	if (!initWithTexture(nullptr, cocos2d::Rect::ZERO)) {
		return false;
	}
	this->setContentSize(cocos2d::Size(1, 1));
	auto motionList = cocos2d::__Dictionary::create();
	auto keys = animationData->getMotionList()->allKeys();
	cocos2d::Ref *ref = nullptr;
	CCARRAY_FOREACH(keys, ref) {
		auto id = dynamic_cast<cocos2d::Integer *>(ref);
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
		auto data = static_cast<agtk::data::MotionData *>(animationData->getMotionList()->objectForKey(id->getValue()));
#else
		auto data = dynamic_cast<agtk::data::MotionData *>(animationData->getMotionList()->objectForKey(id->getValue()));
#endif
		auto motion = agtk::AnimationMotion::create(data, resourceInfoData, imageId);
		motionList->setObject(motion, data->getId());
	}
	this->setAnimationData(animationData);
	this->setAnimationMotionList(motionList);

	//! ACT2-1510 関連の対応によりコメントアウト
	// Player.cpp で登録するようにしました
	//set funcion position,scale,rotation
	//this->onSetPosition = [&](float x, float y) {
	//	cocos2d::Sprite::setPosition(x, y);
	//};
	//this->onSetScale = [&](float x, float y) {
	//	cocos2d::Sprite::setScale(x, y);
	//};
	//this->onSetRotation = [&](float rotation) {
	//	cocos2d::Sprite::setRotation(rotation);
	//};

	//とりあえず。
//	scheduleUpdate();
	return true;
}

void ImagePlayer::update(float dt)
{
	auto motion = this->getCurrentAnimationMotion();
	if (motion == nullptr) {
		return;
	}
// #AGTK-NX
#ifdef USE_SAR_OPTIMIZE_6
	motion->update(dt, &_frameCache);
#else
	motion->update(dt);
#endif

#ifdef AGTK_DEBUG
	auto node = this->getDebugNode();
	if (node) {
		cocos2d::Rect r(
			0, 0,
			this->getContentSize().width, this->getContentSize().height
		);
		node->setRectangle(r.origin.x, r.origin.y, r.size.width, r.size.height);
	}
#endif
}

void ImagePlayer::play(int motionNo, int motionDirectNo, float seconds, bool bIgnoredSound, bool bReverse)
{
	auto motion = this->getCurrentAnimationMotion();
	bool ignored = false;
	if (motion != nullptr) {
		ignored = motion->getIgnored();
		motion->setIgnored(false);
	}
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
	motion = static_cast<agtk::AnimationMotion *>(this->getAnimationMotionList()->objectForKey(motionNo));
#else
	motion = dynamic_cast<agtk::AnimationMotion *>(this->getAnimationMotionList()->objectForKey(motionNo));
#endif
	//set function
	motion->onSetSpriteFrame = [&](cocos2d::SpriteFrame *spriteFrame, cocos2d::Rect rect) {
		if (spriteFrame) {
			this->setSpriteFrame(spriteFrame);
		}
		this->setTextureRect(rect);
	};
	motion->onSetFlipX = [&](bool flag) { this->setFlippedX(flag); };
	motion->onSetFlipY = [&](bool flag) { this->setFlippedY(flag); };
	motion->onSetAlpha = [&](int alpha) { this->setOpacity(alpha); };
	motion->onSetColor = [&](int r, int g, int b) { this->setColor(cocos2d::Color3B(r, g, b)); };
	motion->onSetScale = [&](float sx, float sy) { this->setInnerScale(sx, sy); };
	motion->onSetRotation = [&](float rotation) { this->setInnerRotation(rotation); };
	motion->onSetOffset = [&](float ox, float oy) { this->setOffset(ox, oy); };
	motion->onSetCenter = [&](float cx, float cy) {
		cocos2d::Size sz = this->getContentSize();
		//! ACT2-1510 関連の対応によりコメントアウト
		//this->setAnchorPoint(_leftupAnchorPoint + cocos2d::Vec2(cx / sz.width, -cy / sz.height));
		_refVisibleCtrlNode->setAnchorPoint(cocos2d::Vec2(cx / sz.width, -cy / sz.height));
		this->setCenter(cx, cy);
	};
	motion->setIgnored(ignored);
	motion->setIgnoredSound(bIgnoredSound);
	this->setCurrentAnimationMotion(motion);

	//原点
	auto motionData = motion->getMotionData();
	auto directionData = motionData->getDirectionData(motionDirectNo);
	int resourceInfoId = -1;
	if (directionData != nullptr) {
		resourceInfoId = directionData->getResourceInfoId();
	}
	auto resourceInfoData = this->getAnimationData()->getResourceInfoData(resourceInfoId);
	auto imageData = GameManager::getInstance()->getProjectData()->getImageData(resourceInfoData->getImageId());
	cocos2d::Size imageSize;
	if (imageData != nullptr) {
		imageSize.width = imageData->getTexWidth() / resourceInfoData->getHDivCount();
		imageSize.height = imageData->getTexHeight() / resourceInfoData->getVDivCount();
	}
	cocos2d::Vec2 origin = this->getAnimationData()->calcOriginPosition(imageSize);
	this->setOrigin(origin);
	this->setAnchorPoint(_leftupAnchorPoint);

	//play
	motion->play(motionDirectNo, seconds, bReverse);

#ifdef AGTK_DEBUG
	if (this->getDebugNode() == nullptr) {
		auto pm = PrimitiveManager::getInstance();
		auto tile = pm->createRectangle(0, 0, 0, 0, cocos2d::Color4F(1, 1, 1, 0.5f));
		this->addChild(tile, 0, agtk::BasePlayer::DebugDisplayName);
	}
#endif
}

void ImagePlayer::setResourceSetId(int resourceSetId)
{
	//全てのモーション、表示方向、フレームリストのimageId変更に対して、テクスチャーを変更する。
	auto animationData = this->getAnimationData();

	cocos2d::DictElement *el = nullptr;
	CCDICT_FOREACH(_animationMotionList, el) {
		auto motion = dynamic_cast<agtk::AnimationMotion *>(el->getObject());
		auto directionList = motion->getAnimationDirectionList();
		cocos2d::DictElement *el2 = nullptr;
		CCDICT_FOREACH(directionList, el2) {
			auto direction = dynamic_cast<agtk::AnimationDirection *>(el2->getObject());
			auto directionData = direction->getDirectionData();
			auto resourceInfoId = directionData->getResourceInfoId();
			auto resourceInfoData = animationData->getResourceInfoData(resourceInfoId);
			if (resourceInfoData->getImage()) {
				//画像素材のみ処理する。
				auto imageId = resourceInfoData->getImageIdByResourceSetId(resourceSetId);
				auto frameList = direction->getAnimationFrameList();
				cocos2d::Ref *ref3 = nullptr;
				CCARRAY_FOREACH(frameList, ref3) {
					auto frame = dynamic_cast<agtk::AnimationFrame *>(ref3);
					frame->changeImage(resourceInfoData, imageId);

				}
			}
		}
	}
}

void ImagePlayer::setPosition(const cocos2d::Vec2& pos)
{
	ImagePlayer::setPosition(pos.x, pos.y);
}

void ImagePlayer::setPosition(float x, float y)
{
	_pos = cocos2d::Vec2(x, y);
	updateRealPosition();
}

const cocos2d::Vec2& ImagePlayer::getPosition() const
{
	return _pos;
}

void ImagePlayer::getPosition(float *x, float *y) const
{
	*x = _pos.x;
	*y = _pos.y;
}

void ImagePlayer::setScale(const cocos2d::Vec2& scale)
{
	ImagePlayer::setScale(scale.x, scale.y);
}

void ImagePlayer::setScale(float scaleX, float scaleY)
{
	_scale = cocos2d::Vec2(scaleX, scaleY);
	updateRealScale();
}

float ImagePlayer::getScaleX() const
{
	return _scale.x;
}

float ImagePlayer::getScaleY() const
{
	return _scale.y;
}

void ImagePlayer::setRotation(float rotation)
{
	_rotation = rotation;
	updateRealRotation();
}

float ImagePlayer::getRotation() const
{
	return _rotation;
}

#ifdef AGTK_DEBUG
void ImagePlayer::setVisibleDebugDisplay(bool bVisible)
{
	auto node = this->getDebugNode();
	if (node) {
		node->setVisible(bVisible);
	}
}

PrimitiveNode *ImagePlayer::getDebugNode()
{
	return dynamic_cast<PrimitiveNode *>(this->getChildByName(agtk::BasePlayer::DebugDisplayName));
}
#endif

NS_AGTK_END
