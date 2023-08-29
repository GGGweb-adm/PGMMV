#include "SSPlayer.h"
#include "InputManager.h"
#include "GameManager.h"

NS_AGTK_BEGIN

bool SSPlayer::_bPlatformInit = false;
SSPlayer::SSPlayer() : ss::SSPlayerControl(), agtk::BasePlayer()
{
	_type = agtk::BasePlayer::SpriteStudio;
	_filename = nullptr;
	_animationOnlyData = nullptr;
	_texture2d = nullptr;
	_buffer = nullptr;
	//※デフォルト中心点(0.0f, 0.0f)
	//　左下中心点(-0.5f, -0.5f): cocos2d::Spriteの場合(0.0f, 0.0f)
	_leftupAnchorPoint = cocos2d::Vec2(0.0f, 1.0f);//左下
	_nodeToWorldTransformFlag = false;
	_originOrg = cocos2d::Vec2::ZERO;
	_flipX = false;
	_flipY = false;
}

SSPlayer::~SSPlayer()
{
	removeTexture2D();
	CC_SAFE_RELEASE_NULL(_filename);
	CC_SAFE_RELEASE_NULL(_animationOnlyData);
#ifdef AGTK_DEBUG
	auto debugNode = this->getDebugNode();
	if (debugNode) {
		auto pm = PrimitiveManager::getInstance();
		pm->remove(debugNode);
		debugNode->removeFromParent();
	}
#endif
}

SSPlayer *SSPlayer::createWithFilename(std::string path, ss::ResourceManager* resman)
{
	if (!SSPlayer::_bPlatformInit) {
		ss::SSPlatformInit();
		SSPlayer::_bPlatformInit = true;
	}
	auto p = new (std::nothrow) SSPlayer();
	if (p && p->initWithFilename(path, resman)) {
		p->autorelease();
		//p->scheduleUpdate();
		return p;
	}
	CC_SAFE_DELETE(p);
	return nullptr;
}

SSPlayer *SSPlayer::createWithAnimationData(agtk::data::AnimationData *animationData, agtk::data::ResourceInfoData *resourceInfoData, ss::ResourceManager* resman)
{
	if (!SSPlayer::_bPlatformInit) {
		ss::SSPlatformInit();
		SSPlayer::_bPlatformInit = true;
	}
	auto p = new (std::nothrow) SSPlayer();
	if (p && p->initWithAnimationData(animationData, resourceInfoData, resman)) {
		p->autorelease();
		//p->scheduleUpdate();
		return p;
	}
	CC_SAFE_DELETE(p);
	return nullptr;
}

bool SSPlayer::initWithFilename(std::string path, ss::ResourceManager* resman)
{
	if (ss::SSPlayerControl::init() == false) {
		return false;
	}
	if (resman == nullptr) {
		resman = ss::ResourceManager::getInstance();
	}
	this->getSSPInstance()->setResourceManager(resman);
	this->initCustomShaderProgram();
	if (!resman->isDataKeyExists(path)) {
		resman->addData(path);
	}
	//ファイル名設定。 
	auto p = extractFilename(path);
	this->setFilename(cocos2d::__String::create(p));

	this->getSSPInstance()->setData(this->getFilename());

	//set funcion position,scale,rotation
	this->onSetPosition = [&](float x, float y) { cocos2d::Sprite::setPosition(x, y); };
	this->onSetScale = [&](float x, float y) { cocos2d::Sprite::setScale(x, y); };
	this->onSetRotation = [&](float rotation) { cocos2d::Sprite::setRotation(rotation); };
	return true;
}

bool SSPlayer::initWithAnimationData(agtk::data::AnimationData *animationData, agtk::data::ResourceInfoData *resourceInfoData, ss::ResourceManager* resman)
{
	int imageId = resourceInfoData->getImageId();
	auto animationOnlyData = GameManager::getInstance()->getProjectData()->getAnimationOnlyData(imageId);

	if (!initWithFilename(animationOnlyData->getBinFilename(), resman)) {
		return false;
	}
	this->setAnimationOnlyData(animationOnlyData);

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
		auto motion = agtk::AnimationMotion::create(data, resourceInfoData, agtk::data::ResourceInfoData::kInvalidImageId);
		motionList->setObject(motion, data->getId());
	}
	this->setAnimationData(animationData);
	this->setAnimationMotionList(motionList);

	//! ACT2-1510 関連の対応によりコメントアウト
	// Player.cpp で登録するようにしました
	//set funcion position,scale,rotation
	//this->onSetPosition = [&](float x, float y) { cocos2d::Sprite::setPosition(x, y); };
	//this->onSetScale = [&](float x, float y) { cocos2d::Sprite::setScale(x, y); };
	//this->onSetRotation = [&](float rotation) { cocos2d::Sprite::setRotation(rotation); };

	return true;
}

bool SSPlayer::createTexture2D(int width, int height)
{
	_texture2d = new cocos2d::Texture2D();
	if (_texture2d == nullptr) {
		return false;
	}
	unsigned int len = sizeof(cocos2d::Color4B) * width * height;
	_buffer = (unsigned char *)malloc(len);
	if (_buffer == nullptr) {
		return false;
	}
	memset(_buffer, 0x00, len);
	_texture2d->initWithData(_buffer, len, Texture2D::PixelFormat::RGBA8888, width, height, cocos2d::Size(width, height));
	return true;
}

void SSPlayer::removeTexture2D()
{
	if (_texture2d) {
		delete _texture2d;
		_texture2d = nullptr;
	}
	if (_buffer) {
		free(_buffer);
		_buffer = nullptr;
	}
}

void SSPlayer::updateTexture2D(int offsetX, int offsetY, int width, int height)
{
	if (_texture2d == nullptr || _buffer == nullptr) {
		return;
	}
	auto size = _texture2d->getContentSize();
	unsigned int len = sizeof(cocos2d::Color4B) * size.width * size.height;
	memset(_buffer, 0x00, len);
	for (int y = offsetY; y < offsetY + height; y++) {
		for (int x = offsetX; x < offsetX + width; x++) {
			auto p = (unsigned int *)_buffer + (y * (int)size.width + x);
			*p = 0xFF000000;
		}
	}
	_texture2d->updateWithData(_buffer, offsetX, offsetY, size.width, size.height);
}

void SSPlayer::update(float dt)
{
	auto animationMotion = this->getCurrentAnimationMotion();
	auto ssplayer = this->getSSPInstance();

	float nowSeconds = animationMotion->_seconds;
	float maxSeconds = (float)animationMotion->getCurrentDirection()->getMaxFrameCount300() / 300.0f;
	float nextSeconds = nowSeconds;
	int sspFreameMax = ssplayer->getFrameMax() - 1;

	animationMotion->update(dt);

	nextSeconds = animationMotion->_seconds;
	if (animationMotion->getMotionData()->getReversePlay()) {
		if (animationMotion->getReachedLastFrame()) {
			// 逆再生を含む最終フレームに到達した
			if (nowSeconds < nextSeconds) {
				if (nowSeconds > 0) {
					animationMotion->_seconds = 0;
				}
			}
		}
		else if (animationMotion->_bReverse) {
			if (nowSeconds < nextSeconds) {
				// 逆再生開始時
				if (nextSeconds < maxSeconds) {
					animationMotion->_seconds = maxSeconds;
				}
				// 最終フレームにキーフレームが設定されているかチェック
				auto direction = animationMotion->getCurrentDirection();
				auto maxAnimationFrame = direction->getAnimationFrame(direction->getAnimationFrameCount() - 1);
				auto freamCount = maxAnimationFrame->getFrameData()->getFrameCount300() / (direction->getMaxFrameCount300() / direction->getMaxFrame());
				if (freamCount == 1) {
					// ssplayerの最終フレームを指定
					sspFreameMax++;
				}
			}
		}
	}
	else {
		if (nowSeconds > nextSeconds) {
			if (nowSeconds < maxSeconds) {
				animationMotion->_seconds = maxSeconds;
			}
		}
	}

	int frame = (sspFreameMax < 0) ? 0 : AGTK_LINEAR_INTERPOLATE(
		0,
		sspFreameMax,
		animationMotion->getCurrentDirection()->getMaxFrameCount300(),
		(int)(animationMotion->_seconds * 300)
	);
	if (frame == 0) {
		//frame=0の時、SSPlayerでフレームを更新しないようにdtを0とする。
		dt = 0.0f;
	}

	ssplayer->setFrameNo(frame);
	if(_nodeToWorldTransformFlag){
		_ssp->setParentMatrix(_nodeToWorldTransform.m, true);
		_ssp->setAlpha(_displayedOpacity);
		_ssp->update(dt);
	}
	else {
		ss::SSPlayerControl::update(dt);
	}

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

void SSPlayer::play(const std::string& animeName, int loop, int startFrameNo)
{
	auto p = this->getSSPInstance();
	if (p->getFrameMax(animeName) < startFrameNo) {
		startFrameNo = p->getFrameMax(animeName);
	}
	p->play(animeName, loop, startFrameNo);
}

void SSPlayer::play(int motionNo, int motionDirectNo, float seconds, bool bIgnoredSound, bool bReverse)
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
	motion->onSetFlipX = [&](bool flag) {
		this->setFlipX(flag);
	};
	motion->onSetFlipY = [&](bool flag) {
		this->setFlipY(flag);
	};
	motion->onSetAlpha = [&](int alpha) { this->setOpacity(alpha); };
	motion->onSetColor = [&](int r, int g, int b) { this->getSSPInstance()->setColor(r, g, b); };
	motion->onSetScale = [&](float sx, float sy) { this->setInnerScale(sx, sy); };
	motion->onSetRotation = [&](float rotation) { this->setInnerRotation(rotation); };
	motion->onSetOffset = [&](float ox, float oy) { this->setOffset(ox, oy); };
	motion->onSetCenter = [&](float cx, float cy) {
		//! ACT2-1510 関連の対応によりコメントアウト
		cocos2d::Size sz = this->getContentSize();
		//this->setAnchorPoint(_leftupAnchorPoint + cocos2d::Vec2(cx / sz.width, -cy / sz.height));
		_refVisibleCtrlNode->setAnchorPoint(cocos2d::Vec2(cx / sz.width, -cy / sz.height));
		this->setCenter(cx, cy);
	};
	motion->setIgnored(ignored);
	motion->setIgnoredSound(bIgnoredSound);
	this->setCurrentAnimationMotion(motion);

	const char *animationName;
	if (this->getAnimationData()->getType() == agtk::data::AnimationData::kEffect) {
		animationName = this->getAnimationData()->getAnimationName();
	}
	else {
		animationName = motion->getAnimationDirection(motionDirectNo)->getDirectionData()->getAnimationName();
	}
	auto animationOnlyData = this->getAnimationOnlyData();
	auto animationInfoData = animationOnlyData->getAnimationInfoData(animationName);

	//※SpriteStudioで原点が中心か、中下の場合のみ対応しています。
	//　それ以外の場合は、計算方法を考えないと。

	//play ss
#ifdef USE_30FPS
	// 30FPSでは2フレームづつアニメーションが進むのでloop設定を厳密にしなければ意図しないアニメーションのループが発生する
	int loop = 0;
	if (!motion->getMotionData()->getInfiniteLoop()) {
		loop = motion->getMotionData()->getLoopCount();
	}
	this->play(animationName, loop, seconds / Director::getInstance()->getAnimationInterval());
#else
	this->play(animationName, 0, seconds / Director::getInstance()->getAnimationInterval());
#endif

	int width = animationInfoData->getImageWidth();
	int height = animationInfoData->getImageHeight();
	int originX = animationInfoData->getOriginX();
	int originY = animationInfoData->getOriginY();
// #AGTK-NX
#if CC_TARGET_PLATFORM != CC_PLATFORM_NX
	auto origin = this->getAnimationData()->calcOriginPosition(cocos2d::Size(width, height));
#else
#endif
	origin.x += originX;
	origin.y += (height - originY);
	this->setOrigin(origin);
	this->setAnchorPoint(_leftupAnchorPoint);
	this->setContentSize(cocos2d::Size(width, height));
	this->setOriginOrg(cocos2d::Vec2(originX, originY));
	this->setOriginOffset(-width * 0.5f, height - originY);

	//canvas size
	this->updateTexture2D(0, 0, width, height);
	this->setAnimationOnlyRect(cocos2d::Rect(originX, originY, width, height));

	//play
	motion->play(motionDirectNo, seconds, bReverse);

#ifdef AGTK_DEBUG
	auto ap = cocos2d::Vec2(0.5f, 1.0f - (float)originY / height);
	if (this->getDebugNode() == nullptr) {
		auto pm = PrimitiveManager::getInstance();
		auto tile = pm->createRectangle(0, 0, width, height, cocos2d::Color4F(1, 1, 1, 0.5f));
		tile->setAnchorPoint(ap);
		this->addChild(tile, 0, agtk::BasePlayer::DebugDisplayName);
	}
#endif

	//size
	_refVisibleCtrlNode->setContentSize(this->getContentSize());
}

const char *SSPlayer::getFilename()
{
	CC_ASSERT(_filename);
	return _filename->getCString();
}

void SSPlayer::setPosition(const cocos2d::Vec2& pos)
{
	SSPlayer::setPosition(pos.x, pos.y);
}

void SSPlayer::setPosition(float x, float y)
{
	_pos = cocos2d::Vec2(x, y);
	updateRealPosition();
}

const cocos2d::Vec2& SSPlayer::getPosition() const
{
	return _pos;
}

void SSPlayer::getPosition(float *x, float *y) const
{
	*x = _pos.x;
	*y = _pos.y;
}

void SSPlayer::setScale(const cocos2d::Vec2& scale)
{
	SSPlayer::setScale(scale.x, scale.y);
}

void SSPlayer::setScale(float scaleX, float scaleY)
{
	_scale = cocos2d::Vec2(scaleX, scaleY);
	updateRealScale();
}

float SSPlayer::getScaleX() const
{
	return _scale.x;
}

float SSPlayer::getScaleY() const
{
	return _scale.y;
}

void SSPlayer::setRotation(float rotation)
{
	_rotation = rotation;
	updateRealRotation();
}

float SSPlayer::getRotation() const
{
	return _rotation;
}

std::string SSPlayer::extractFilename(std::string path)
{
	int path_i = 0;
	if (path.find_last_of("\\") != std::string::npos) {
		path_i = path.find_last_of("\\") + 1;
	}
	else if (path.find_last_of("/") != std::string::npos) {
		path_i = path.find_last_of("/") + 1;
	}
	else {
		CC_ASSERT(0);
	}
	int ext_i = path.find_last_of(".");
	std::string pathname = path.substr(0, path_i + 1);
	std::string extname = path.substr(ext_i, path.size() - ext_i);
	return path.substr(path_i, ext_i - path_i);
}

#ifdef AGTK_DEBUG
void SSPlayer::setVisibleDebugDisplay(bool bVisible)
{
	auto node = this->getDebugNode();
	if (node) {
		node->setVisible(bVisible);
	}
}

PrimitiveNode *SSPlayer::getDebugNode()
{
	return dynamic_cast<PrimitiveNode *>(this->getChildByName(agtk::BasePlayer::DebugDisplayName));
}
#endif

void SSPlayer::draw(cocos2d::Renderer *renderer, const cocos2d::Mat4 &transform, uint32_t flags)
{
	if (_enableRenderingBlendFunc == false)
	{
		//通常描画
		_customCommand.init(_globalZOrder, transform, flags);
		_customCommand.func = CC_CALLBACK_0(SSPlayer::onDraw, this, renderer, transform, flags);
		renderer->addCommand(&_customCommand);
	}
	else
	{
		//レンダリング用描画
		_customCommandRendering.init(_globalZOrder, transform, flags);
		_customCommandRendering.func = CC_CALLBACK_0(SSPlayer::onRenderingDraw, this, renderer, transform, flags);
		renderer->addCommand(&_customCommandRendering);
	}
}

void SSPlayer::onDraw(cocos2d::Renderer *renderer, const cocos2d::Mat4 &transform, uint32_t flags)
{
	//アップデートを行わずにDrawされた場合（RenderTextuer等）もあるので親ノードのマトリクスを設定する
	//cocos2d::Mat4 mat = getNodeToWorldTransform();
	cocos2d::Mat4 mat = transform;
	mat.scale(cocos2d::Vec3(this->getFlipX() ? -1 : 1, this->getFlipY() ? -1 : 1, 1));
	_ssp->setParentMatrix(mat.m, true);

	//プレイヤーの描画
	this->getGLProgram()->use();
	_ssp->draw();
}

void SSPlayer::onRenderingDraw(cocos2d::Renderer *renderer, const cocos2d::Mat4 &transform, uint32_t flags)
{
	//レンダリング描画
	ss::SSRenderingBlendFuncEnable(true);
	onDraw(renderer, transform, flags);
	ss::SSRenderingBlendFuncEnable(false);
}

NS_AGTK_END
