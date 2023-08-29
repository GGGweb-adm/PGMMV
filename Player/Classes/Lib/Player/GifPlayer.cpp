#include "GifPlayer.h"
#include "PrimitiveManager.h"
#include "GameManager.h"

NS_AGTK_BEGIN

//-------------------------------------------------------------------------------------------------
GifFrame::Bitmap::Bitmap()
{
	_buffer = nullptr;
	_width = 0;
	_height = 0;
	_length = 0;
}

GifFrame::Bitmap::~Bitmap()
{
	CC_ASSERT(_buffer);
	free(_buffer);
}

bool GifFrame::Bitmap::init(unsigned int width, unsigned int height)
{
	unsigned int len = sizeof(cocos2d::Color4B) * width * height;
	_buffer = (unsigned char *)malloc(len);
	CC_ASSERT(_buffer);
	memset(_buffer, 0x00, len);
	_width = width;
	_height = height;
	_length = len;
	return true;
}

void GifFrame::Bitmap::copy(GifFrame::Bitmap *bitmap)
{
	unsigned char *buffer = bitmap->getAddr();
	CC_ASSERT(bitmap->getLength() == this->getLength());
	memcpy(this->getAddr(), buffer, this->getLength());
}

unsigned char *GifFrame::Bitmap::getAddr(unsigned int pos)
{
	CC_ASSERT(pos < _length);
	return _buffer + pos;
}

GifFrame::GifFrame()
{
	_frameIndex = -1;
	_duration = 0;
	_durationEnd = 0;
	_bitmap = nullptr;
}

GifFrame::~GifFrame()
{
	CC_SAFE_RELEASE_NULL(_bitmap);
}

bool GifFrame::init(GifFileType *fileType, int frameIndex)
{
	CC_ASSERT(fileType);
	struct SavedImage image = fileType->SavedImages[frameIndex];
	auto bm = Bitmap::create(fileType->SWidth, fileType->SHeight);
	if (bm == nullptr) {
		return false;
	}
	this->setBitmap(bm);
	this->setDuration(this->getDuration(&image));
	this->setDurationEnd(this->getDuration());
	this->setFrameIndex(frameIndex);

	const ColorMapObject *cmap = this->getColorMap(fileType, &image);
	if (cmap == nullptr) {
		return false;
	}
	int trans = this->getTransparent(&image);

	//Background color.
	GifImageDesc desc = image.ImageDesc;
	cocos2d::Color4B *dst = (cocos2d::Color4B *)bm->getAddr();
	auto bgCol = cmap->Colors[fileType->SBackGroundColor];
	for (int i = 0; i < fileType->SWidth * fileType->SHeight; i++) {
		dst[i] = cocos2d::Color4B(bgCol.Red, bgCol.Green, bgCol.Blue, 0xFF);
	}

	//Image color.
	cocos2d::Color4B *p = dst;
	for (int y = 0; y < desc.Height; y++) {
		p = &dst[(y + desc.Top) * fileType->SWidth + desc.Left];
		for (int x = 0; x < desc.Width; x++) {
			int cid = image.RasterBits[y * desc.Width + x] & 0xff;
			if (cid != trans) {
				const GifColorType& col = cmap->Colors[cid];
				*p = cocos2d::Color4B(col.Red, col.Green, col.Blue, 0xFF);
			}
			else if (cid == trans) {
				const GifColorType& col = cmap->Colors[trans];
				*p = cocos2d::Color4B(col.Red, col.Green, col.Blue, 0x00);
			}
			p++;
		}
	}
	return true;
}

bool GifFrame::init(GifFileType *fileType, int frameIndex, GifFrame *prevGifFrame)
{
	CC_ASSERT(fileType);
	struct SavedImage image = fileType->SavedImages[frameIndex];
	auto bm = Bitmap::create(fileType->SWidth, fileType->SHeight);
	if (bm == nullptr) {
		return false;
	}
	//disposal Method
	//UNDEFINED_RECORD_TYPE: 前のフレームを残さず単にフレームを置き換える
	//SCREEN_DESC_RECORD_TYPE: 前のフレームを残しその上に上書きする（透明部分は前のままになる）
	//IMAGE_DESC_RECORD_TYPE: 指定した特定の背景色で塗りつぶしたあとフレームを描画する
	int disposalMethod = this->getDisposalMethod(&image);

	if (disposalMethod == SCREEN_DESC_RECORD_TYPE) {
		bm->copy(prevGifFrame->getBitmap());
	}

	this->setBitmap(bm);
	this->setDuration(this->getDuration(&image));
	this->setFrameIndex(frameIndex);

	const ColorMapObject *cmap = this->getColorMap(fileType, &image);
	if (cmap == nullptr) {
		return false;
	}
	int trans = this->getTransparent(&image);
	GifImageDesc desc = image.ImageDesc;
	cocos2d::Color4B *dst = (cocos2d::Color4B *)bm->getAddr();

	if (disposalMethod == IMAGE_DESC_RECORD_TYPE) {
		auto bgCol = cmap->Colors[fileType->SBackGroundColor];
		for (int i = 0; i < fileType->SWidth * fileType->SHeight; i++) {
			dst[i] = cocos2d::Color4B(bgCol.Red, bgCol.Green, bgCol.Blue, 0xFF);
		}
	}

	//Image color.
	cocos2d::Color4B *p = dst;
	for (int y = 0; y < desc.Height; y++) {
		p = &dst[(y + desc.Top) * fileType->SWidth + desc.Left];
		for (int x = 0; x < desc.Width; x++) {
			int cid = image.RasterBits[y * desc.Width + x] & 0xff;
			if (cid != trans) {
				const GifColorType& col = cmap->Colors[cid];
				*p = cocos2d::Color4B(col.Red, col.Green, col.Blue, 0xFF);
			}
			else if (cid == trans) {
				if (disposalMethod != SCREEN_DESC_RECORD_TYPE) {
					const GifColorType& col = cmap->Colors[trans];
					*p = cocos2d::Color4B(col.Red, col.Green, col.Blue, 0x00);
				}
			}
			p++;
		}
	}
	return true;
}

unsigned int GifFrame::getDuration(const SavedImage *image)
{
	unsigned int duration = 0;
	for (int j = 0; j < image->ExtensionBlockCount; j++)
	{
		if (image->ExtensionBlocks[j].Function == GRAPHICS_EXT_FUNC_CODE)
		{
			int size = image->ExtensionBlocks[j].ByteCount;
			//assert(size >= 4);
			if (size < 4) break;
			const uint8_t* b = (const uint8_t*)image->ExtensionBlocks[j].Bytes;
			duration = ((b[2] << 8) | b[1]) * 10;
			break;
		}
	}
	//duration = duration <= 50 ? 50 : duration;
	return duration;
}

const ColorMapObject *GifFrame::getColorMap(const GifFileType *fileType, const SavedImage *image)
{
	const ColorMapObject* cmap = fileType->SColorMap;
	if (image->ImageDesc.ColorMap != NULL) {
		// use local color table
		cmap = image->ImageDesc.ColorMap;
	}
	if (cmap == NULL || cmap->ColorCount != (1 << cmap->BitsPerPixel)) {
		return nullptr;
	}
	return cmap;
}

int GifFrame::getTransparent(const SavedImage *frame)
{
	int transparent = -1;
	for (int i = 0; i < frame->ExtensionBlockCount; ++i) {
		ExtensionBlock* eb = frame->ExtensionBlocks + i;
		if (eb->Function == GRAPHICS_EXT_FUNC_CODE && eb->ByteCount == 4) {
			bool has_transparency = ((eb->Bytes[0] & 0x01) == 1);
			if (has_transparency) {
				transparent = eb->Bytes[3];
			}
		}
	}
	return transparent;
}

int GifFrame::getDisposalMethod(const SavedImage *frame)
{
	int disposalMethod = 0;
	for (int i = 0; i < frame->ExtensionBlockCount; ++i) {
		ExtensionBlock* eb = frame->ExtensionBlocks + i;
		if (eb->Function == GRAPHICS_EXT_FUNC_CODE &&
			eb->ByteCount == 4) {
			disposalMethod = (eb->Bytes[0] >> 2) & 0x07;
			break;
		}
	}
	return disposalMethod;
}

#if defined(AGTK_DEBUG)
void GifFrame::dump()
{
	CCLOG("frameIndex:%d", this->getFrameIndex());
	CCLOG("duration:%d", this->getDuration());
}
#endif

//-------------------------------------------------------------------------------------------------
GifAnimation::GifAnimation()
{
	_filename = nullptr;
	_width = 0;
	_height = 0;
	_gifFrameList = nullptr;
	_fileType = nullptr;
	_frameNo = -1;
	_frameCount = 0;
	_pause = false;
	_maxDuration = 0;
}

GifAnimation::~GifAnimation()
{
	CC_SAFE_RELEASE_NULL(_filename);
	CC_SAFE_RELEASE_NULL(_gifFrameList);
	DGifCloseFile(_fileType, NULL);
}

bool GifAnimation::init(const char *filename)
{
#ifdef USE_AGTK_MEMORY_FILE
	FILE *fp = nullptr;
	ssize_t nSize;
	auto loadData = cocos2d::FileUtils::getInstance()->getFileData(filename, "rb", &nSize);
	int p[2];
	if (loadData) {
		fp = cocos2d::FileUtils::getInstance()->memFopen((const char *)loadData, nSize, p);
	}
#else
	FILE *fp = fopen(FileUtils::getInstance()->getSuitableFOpen(filename).c_str(), "rb");
#endif
	if (!fp) {
		return false;
	}
	//ファイル読み込み
	int error = 0;
	auto fileType = DGifOpen(fp, GifAnimation::callbackDecode, &error);
	if (NULL == fileType || DGifSlurp(fileType) != GIF_OK) {
		fclose(fp);
		DGifCloseFile(fileType, &error);
		fileType = NULL;
		return false;
	}
#ifdef USE_AGTK_MEMORY_FILE
	cocos2d::FileUtils::getInstance()->memFclose(fp, p);
	free(loadData);
#else
	fclose(fp);
#endif

	//Image
	auto arr = cocos2d::__Array::create();
	int duration = 0;
	agtk::GifFrame *prevGifFrame = nullptr;
	for (int i = 0; i < fileType->ImageCount; i++) {
		agtk::GifFrame *p = nullptr;
		if (i > 0) {
			p = agtk::GifFrame::create(fileType, i, prevGifFrame);
		}
		else {
			p = agtk::GifFrame::create(fileType, i);
		}
		if (p == nullptr) {
			return false;
		}
		duration += p->getDuration();
		p->setDurationEnd(duration);
		arr->addObject(p);
		prevGifFrame = p;
	}
	this->setGifFrameList(arr);
	this->setMaxDuration(duration);

	//Width,Height
	this->setWidth(fileType->SWidth);
	this->setHeight(fileType->SHeight);
	this->setFilename(cocos2d::__String::create(filename));
	_fileType = fileType;
	return true;
}

const char *GifAnimation::getFilename()
{
	CC_ASSERT(_filename);
	return _filename->getCString();
}

GifFrame *GifAnimation::getGifFrame(unsigned int id)
{
	if (id >= getGifFrameCount()) {
		return nullptr;
	}
	return dynamic_cast<GifFrame *>(this->getGifFrameList()->getObjectAtIndex(id));
}

unsigned int GifAnimation::getGifFrameCount()
{
	return this->getGifFrameList()->count();
}

GifFrame::Bitmap *GifAnimation::getBitmap()
{
	int frameNo = this->getFrameNo();
	return getGifFrame(frameNo)->getBitmap();
}

int GifAnimation::callbackDecode(GifFileType *gif, GifByteType *bytes, int size)
{
	FILE* file = (FILE*)gif->UserData;
	return fread(bytes, 1, size, file);
}

void GifAnimation::play(int frames)
{
	float animInterval = Director::getInstance()->getAnimationInterval();
	float seconds = frames * animInterval;
	int milliSeconds = 0;
	int frameNo = 0;
	int frameCount = 0;
	for (unsigned int i = 0; i < this->getGifFrameCount(); i++) {
		auto p = this->getGifFrame(i);
		if (seconds < p->getDurationEnd() * animInterval) {
			frameCount += p->getDurationEnd() / 1000.0f;
			frameNo = i;
			break;
		}
	}
	_frameNo = frameNo;
	_frameCount = frames;
	_pause = false;
}

void GifAnimation::stop()
{
	_frameNo = -1;
	_frameCount = 0;
}

bool GifAnimation::update(float dt)
{
	if (_frameNo < 0) {
		return false;
	}
	if (_pause) {
		return false;
	}

	float frameCount = (float)_frameCount;
	float frameMax = this->getMaxDuration() / 1000.0f;

	auto nowFrame = dynamic_cast<GifFrame *>(this->getGifFrame(_frameNo));
	CC_ASSERT(nowFrame);

	frameCount += dt;
	if (frameCount > frameMax) {
		frameCount -= frameMax;
	}

	//FrameNo
	unsigned int frameNo = _frameNo;
	int frameStartNo = -1;
	if (_frameCount < frameCount) {
		if (frameCount > nowFrame->getDurationEnd() / 1000.0f) {
			frameStartNo = _frameNo + 1;
		}
	}
	else {
		frameStartNo = 0;
	}
	if (frameStartNo >= 0) {
		for (unsigned int n = frameStartNo; n < this->getGifFrameCount(); n++) {
			auto p = this->getGifFrame(n);
			if (frameCount < p->getDurationEnd() / 1000.0f) {
				_frameNo = p->getFrameIndex();
				break;
			}
		}
	}
	bool bChangeFrameNo = (_frameNo != frameNo) ? true : false;
	_frameCount = frameCount;
	return bChangeFrameNo;
}

int GifAnimation::setFrameNo(int frameNo)
{
	if (frameNo < 0) frameNo = 0;
	if (static_cast<unsigned int>(frameNo) >= this->getGifFrameCount()) frameNo = this->getGifFrameCount() - 1;
	int oldFrameNo = _frameNo;
	_frameNo = frameNo;

	float frameCount = 0;
	for (int i = 0; i < frameNo - 1; i++) {
		auto p = this->getGifFrame(i);
		frameCount += p->getDuration();
	}
	_frameCount = frameCount / 1000.0f;
	return oldFrameNo;
}

int GifAnimation::getFrameMax()
{
	return (int)this->getGifFrameCount();
}

#if defined(AGTK_DEBUG)
void GifAnimation::dump()
{
	CCLOG("filename:%s", this->getFilename());
	CCLOG("width:%d", this->getWidth());
	CCLOG("height:%d", this->getHeight());
	cocos2d::Ref *ref = nullptr;
	CCARRAY_FOREACH(this->getGifFrameList(), ref) {
		auto p = dynamic_cast<GifFrame *>(ref);
		p->dump();
	}
}
#endif

//-------------------------------------------------------------------------------------------------
GifPlayer::GifPlayer() : agtk::BasePlayer()
{
	_type = agtk::BasePlayer::Gif;
	_gifAnimation = nullptr;
	_leftupAnchorPoint = cocos2d::Vec2(0.0f, 1.0f);
	_updateGifFirstFlag = false;
}

GifPlayer::~GifPlayer()
{
	CC_SAFE_RELEASE_NULL(_gifAnimation);
#if defined(AGTK_DEBUG)
	auto debugNode = this->getDebugNode();
	if (debugNode) {
		auto pm = PrimitiveManager::getInstance();
		pm->remove(debugNode);
		debugNode->removeFromParent();
	}
#endif
}

GifPlayer *GifPlayer::createWithFilename(std::string path)
{
	auto p = new (std::nothrow) GifPlayer();
	if (p && p->initWithFilename(path)) {
		p->autorelease();
		return p;
	}
	CC_SAFE_DELETE(p);
	return nullptr;
}

GifPlayer *GifPlayer::createWithAnimationData(agtk::data::AnimationData *animationData, agtk::data::ResourceInfoData *resourceInfoData)
{
	auto p = new (std::nothrow) GifPlayer();
	if (p && p->initWithAnimationData(animationData, resourceInfoData)) {
		p->autorelease();
		return p;
	}
	CC_SAFE_DELETE(p);
	return nullptr;
}

bool GifPlayer::initWithFilename(std::string filename)
{
	auto gifAnimation = agtk::GifAnimation::create(filename.c_str());
	if (gifAnimation == nullptr) {
		return false;
	}
	this->setGifAnimation(gifAnimation);

	auto texture = new Texture2D();
	int width = gifAnimation->getWidth();
	int height = gifAnimation->getHeight();
	auto bm = agtk::GifFrame::Bitmap::create(width, height);
	texture->initWithData(
		bm->getAddr(),
		bm->getLength(),
		Texture2D::PixelFormat::RGBA8888,
		bm->getWidth(),
		bm->getHeight(),
		cocos2d::Size(bm->getWidth(), bm->getHeight())
	);
	if (!initWithTexture(texture)) {
		return false;
	}
	//scheduleUpdate();
	return true;
}

bool GifPlayer::initWithAnimationData(agtk::data::AnimationData *animationData, agtk::data::ResourceInfoData *resourceInfoData)
{
	int imageId = resourceInfoData->getImageId();
	auto animationOnlyData = GameManager::getInstance()->getProjectData()->getAnimationOnlyData(imageId);
	if (!initWithFilename(animationOnlyData->getFilename())) {
		return false;
	}

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

void GifPlayer::update(float dt)
{
	auto animationMotion = this->getCurrentAnimationMotion();
	if (animationMotion != nullptr) {
		animationMotion->update(dt);
	}

	this->updateGif(dt);

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

void GifPlayer::updateGif(float dt)
{
	auto animationMotion = this->getCurrentAnimationMotion();
	auto animation = this->getGifAnimation();

	int frame = animationMotion->getFrameDataNo();
	int oldFrame = animation->setFrameNo(frame);

	bool ret = animation->update(dt);
	if (ret || oldFrame != frame || _updateGifFirstFlag) {
		auto bitmap = animation->getBitmap();
		cocos2d::Size contentSize(bitmap->getWidth(), bitmap->getHeight());
		auto texture = new Texture2D();
		texture->initWithData(
			bitmap->getAddr(),
			bitmap->getLength(),
			Texture2D::PixelFormat::RGBA8888,
			bitmap->getWidth(),
			bitmap->getHeight(),
			contentSize
		);
		auto textureOld = this->getTexture();
		this->setTexture(texture);
		if (textureOld) {
			textureOld->release();
		}
		_updateGifFirstFlag = false;
	}
}

void GifPlayer::play(int motionNo, int motionDirectNo, float seconds, bool bIgnoredSound, bool bReverse)
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
	int width = this->getContentSize().width;
	int height = this->getContentSize().height;
// #AGTK-NX
#if CC_TARGET_PLATFORM != CC_PLATFORM_NX
	cocos2d::Vec2 origin = this->getAnimationData()->calcOriginPosition(cocos2d::Size(width, height));
#else
#endif
	this->setOrigin(origin);
	this->setAnchorPoint(_leftupAnchorPoint);

	//play
	motion->play(motionDirectNo, seconds, bReverse);
	this->getGifAnimation()->play(seconds / Director::getInstance()->getAnimationInterval());

#if defined(AGTK_DEBUG)
	if (this->getDebugNode() == nullptr) {
		auto pm = PrimitiveManager::getInstance();
		auto tile = pm->createRectangle(0, 0, 0, 0, cocos2d::Color4F(1, 1, 1, 0.5f));
		this->addChild(tile, 0, agtk::BasePlayer::DebugDisplayName);
	}
#endif
	_updateGifFirstFlag = true;
}

void GifPlayer::stop()
{
	if (this->getCurrentAnimationMotion()) {
		this->getCurrentAnimationMotion()->stop();
	}
	this->setCurrentAnimationMotion(nullptr);
	this->getGifAnimation()->stop();
}

void GifPlayer::setPosition(const cocos2d::Vec2& pos)
{
	GifPlayer::setPosition(pos.x, pos.y);
}

void GifPlayer::setPosition(float x, float y)
{
	_pos = cocos2d::Vec2(x, y);
	updateRealPosition();
}

const cocos2d::Vec2& GifPlayer::getPosition() const
{
	return _pos;
}

void GifPlayer::getPosition(float *x, float *y) const
{
	*x = _pos.x;
	*y = _pos.y;
}

void GifPlayer::setScale(const cocos2d::Vec2& scale)
{
	GifPlayer::setScale(scale.x, scale.y);
}

void GifPlayer::setScale(float scaleX, float scaleY)
{
	_scale = cocos2d::Vec2(scaleX, scaleY);
	updateRealScale();
}

float GifPlayer::getScaleX() const
{
	return _scale.x;
}

float GifPlayer::getScaleY() const
{
	return _scale.y;
}

void GifPlayer::setRotation(float rotation)
{
	_rotation = rotation;
	updateRealRotation();
}

float GifPlayer::getRotation() const
{
	return _rotation;
}

#ifdef AGTK_DEBUG
void GifPlayer::setVisibleDebugDisplay(bool bVisible)
{
	auto node = this->getDebugNode();
	if (node) {
		node->setVisible(bVisible);
	}
}

PrimitiveNode *GifPlayer::getDebugNode()
{
	return dynamic_cast<PrimitiveNode *>(this->getChildByName(agtk::BasePlayer::DebugDisplayName));
}
#endif

NS_AGTK_END
