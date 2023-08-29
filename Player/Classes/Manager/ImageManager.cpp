#include "ImageManager.h"
#include "Manager/GameManager.h"
#include "Manager/MovieManager.h"

USING_NS_CC;

NS_AGTK_BEGIN
//-------------------------------------------------------------------------------------------------------------------
Image::Image()
{
	_object = nullptr;
	_imageShowData = nullptr;
	_imageData = nullptr;
	_sprite = nullptr;
	_eventTimer = nullptr;
	_objectBackside = false;
	_waitOneFrame = false;
	_objectActionId = -1;
}

Image::~Image()
{
	CC_SAFE_RELEASE_NULL(_object);
	CC_SAFE_RELEASE_NULL(_imageShowData);
	CC_SAFE_RELEASE_NULL(_imageData);
	CC_SAFE_RELEASE_NULL(_sprite);
	CC_SAFE_RELEASE_NULL(_eventTimer);
}

Image *Image::create(agtk::Object *object, agtk::data::ObjectCommandImageShowData *data)
{
	auto p = new (std::nothrow) Image();
	if (p && p->init(object, data)) {
		p->autorelease();
		return p;
	}
	CC_SAFE_DELETE(p);
	return nullptr;
}

bool Image::init(agtk::Object *object, agtk::data::ObjectCommandImageShowData *data)
{
	this->setObject(object);
	this->setImageShowData(data);

	auto scene = GameManager::getInstance()->getCurrentScene();
	auto sceneLayer = object->getSceneLayer();
	auto projectData = GameManager::getInstance()->getProjectData();
	auto imageData = projectData->getImageData(data->getImageId());
	this->setImageData(imageData);

	//サイズ
	cocos2d::Size size;
	if (data->getDefaultSize()) {
		size.setSize(imageData->getTexWidth(), imageData->getTexHeight());
	}
	else {
		size.setSize(data->getWidth(), data->getHeight());
	}

	//位置
	cocos2d::Vec2 position;
	switch (data->getPositionType()) {
	case agtk::data::ObjectCommandImageShowData::kPositionCenter: {//このオブジェクトの中心
		if (data->getUseConnect() && data->getConnectId() >= 0) {
			//接続点使用
			int connectId = data->getConnectId();
			agtk::Vertex4 vertex4;
			if (object->getTimeline(agtk::data::TimelineInfoData::kTimelineConnection, connectId, vertex4)) {
				position = Scene::getPositionSceneFromCocos2d(vertex4.addr()[0]);
			}
			else {
				//接続点が見つからない。
				return false;
			}
		}
		else {
			//このオブジェクトの中心
			auto centerPos = object->getCenterPosition();
			position = centerPos;
		}
		break; }
	case agtk::data::ObjectCommandImageShowData::kPositionLockObjectCenter: {//このオブジェクトがロックしたオブジェクトの中心
		//※引数のobjectがロックされたオブジェクトである事を前提とする。
		auto centerPos = object->getCenterPosition();
		position = centerPos;
		break; }
	case agtk::data::ObjectCommandImageShowData::kPositionScenePosition: {//シーンを基準にする
		cocos2d::Vec2 sceneSize = scene->getSceneSize();
		if (sceneLayer->getType() == agtk::SceneLayer::kTypeMenu) {
			sceneSize = cocos2d::Vec2(projectData->getScreenSize());
		}
		//x
		switch (data->getHorzAlign()) {
		case agtk::data::ObjectCommandImageShowData::kHorzAlignLeft: {
			position.x += size.width * 0.5f;
			break; }
		case agtk::data::ObjectCommandImageShowData::kHorzAlignCenter: {
			position.x += sceneSize.x * 0.5f;
			break; }
		case agtk::data::ObjectCommandImageShowData::kHorzAlignRight: {
			position.x += (sceneSize.x - size.width * 0.5f);
			break; }
		}
		//y
		switch (data->getVertAlign()) {
		case agtk::data::ObjectCommandImageShowData::kVertAlignTop: {
			position.y += size.height * 0.5f;
			break; }
		case agtk::data::ObjectCommandImageShowData::kVertAlignCenter: {
			position.y += sceneSize.y * 0.5f;
			break; }
		case agtk::data::ObjectCommandImageShowData::kVertAlignBottom: {
			position.y += (sceneSize.y - size.height * 0.5f);
			break; }
		}
		break; }
	}
	position.x += data->getAdjustX();
	position.y += data->getAdjustY();
	if (sceneLayer->getType() == agtk::SceneLayer::kTypeMenu) {
		auto size = projectData->getScreenSize();
		position = cocos2d::Vec2(position.x, size.height - position.y);
	}
	else {
		position = Scene::getPositionCocos2dFromScene(position);
	}

	auto sprite = cocos2d::Sprite::create(imageData->getFilename(), cocos2d::Rect(cocos2d::Vec2::ZERO, cocos2d::Size(imageData->getTexWidth(), imageData->getTexHeight())));
	if (sprite == nullptr) {
		CC_ASSERT(0);
		return false;
	}
	sprite->getTexture()->setAliasTexParameters();
	sprite->setContentSize(size);
	sprite->setPosition(position);

	this->setSprite(sprite);
	this->setStartObjectPosition(this->getObject()->getCenterPosition());//生成時のオブジェクト位置。
	this->setStartPosition(position);//開始位置。

	//表示時間
	if (data->getDurationUnlimited() == false) {
		auto eventTimer = agtk::EventTimer::create();
		eventTimer->start((float)data->getDuration300() / 300.0f);
		this->setEventTimer(eventTimer);
	}

	//オプション
	//表示位置
	this->setLayerId(this->getObject()->getLayerId());
	if (data->getPriority()) {
		switch (data->getPriorityType()) {
		case agtk::data::ObjectCommandActionExecData::kPriorityBackground:
			// 「背景」設定時
			this->setLayerId(agtk::data::SceneData::kBackgroundLayerId);
			break;
		case agtk::data::ObjectCommandActionExecData::kPriorityMostFront: {
			// 「最前面」設定時
			auto layerId = scene->getSceneLayerFront()->getLayerId();
			this->setLayerId(layerId);
			break; }
		case agtk::data::ObjectCommandActionExecData::kPriorityMostFrontWithMenu:
			// 「最前面＋メニューシーン」設定時
			this->setLayerId(agtk::data::SceneData::kHudTopMostLayerId);
			break;
		default: CC_ASSERT(0);
		}
	}
	//決定ボタンが押されたら画像を閉じる（Image::isCloseByOk）
	//画像表示中はすべてのオブジェクト動作を停止
	if (data->getStopObject()) {
// #AGTK-NX
#ifdef USE_SAR_OPTIMIZE_1
		auto objectList = scene->getObjectAllReference(sceneLayer->getType());
#else
		auto objectList = scene->getObjectAll(sceneLayer->getType());
#endif
		cocos2d::Ref *ref;
		CCARRAY_FOREACH(objectList, ref) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
			auto p = static_cast<agtk::Object *>(ref);
#else
			auto p = dynamic_cast<agtk::Object *>(ref);
#endif
			p->setWaitDuration300(-1);//※制限なしに待ち状態(-1)
		}
	}
	//画像表示中はゲームの動作を一時停止
	if (data->getStopGame()) {
		// 待ちフレームをシーンに設定
		auto scene = GameManager::getInstance()->getCurrentScene();
		scene->setWaitDuration300(-1);//※制限なしに待ち状態(-1)
	}

	// 決定ボタン押下で画像を閉じる場合
	if (data->getCloseByOk()) {
		// 1フレーム待機フラグをON
		_waitOneFrame = true;
	}
	//オブジェクトのアクションが切り替わったら表示を終了
	if (data->getHideOnObjectActionChange()) {
		_objectActionId = object->getCurrentObjectAction()->getId();
	}
	return true;
}

void Image::update(float delta)
{
	auto eventTimer = this->getEventTimer();
	if (eventTimer) {
		eventTimer->update(delta);
	}
	auto type = this->getImageShowData()->getPositionType();
	if (type == agtk::data::ObjectCommandImageShowData::kPositionCenter || type == agtk::data::ObjectCommandImageShowData::kPositionLockObjectCenter) {
		auto nowPosition = this->getObject()->getCenterPosition();
		auto move = nowPosition - this->getStartObjectPosition();
		move.y *= -1;//シーンとcocos2dのY座標の向きが逆のため。
		this->getSprite()->setPosition(this->getStartPosition() + move);
	}
	//レイヤーのα値。
	auto object = this->getObject();
	if (object != nullptr && object->getSceneLayer() != nullptr) {
		auto sceneLayer = object->getSceneLayer();
		this->getSprite()->setOpacity(sceneLayer->getAlphaValue()->getValue());
	}
}

bool Image::isStop()
{
	auto eventTimer = this->getEventTimer();
	if (eventTimer == nullptr) {
		return false;
	}
	return (eventTimer->getState() == agtk::EventTimer::kStateEnd) || (eventTimer->getState() == agtk::EventTimer::kStateIdle);
}

bool Image::isClose()
{
	return isCloseByOk() || isCloseByChangeAction();
}

bool Image::isCloseByOk()
{
	if (this->getImageShowData()->getCloseByOk() && !_waitOneFrame) {
		//オプション：決定ボタンが押されたら画像を閉じる
		auto inputManager = InputManager::getInstance();
		return inputManager->isTriggered(InputController::OK);
	}

	if (_waitOneFrame) {
		_waitOneFrame = false;
	}
	return false;
}

bool Image::isCloseByChangeAction()
{
	//オブジェクトのアクションが切り替わったら表示を終了。
	auto data = this->getImageShowData();
	auto object = this->getObject();
	if (data->getHideOnObjectActionChange()) {
		if (_objectActionId != object->getCurrentObjectAction()->getId()) {
			return true;
		}
	}
	return false;
}

NS_AGTK_END

//-------------------------------------------------------------------------------------------------------------------
ImageManager* ImageManager::_imageManager = NULL;
ImageManager::ImageManager()
{
	_imageList = nullptr;
}

ImageManager::~ImageManager()
{
	CC_SAFE_RELEASE_NULL(_imageList);
}

ImageManager* ImageManager::getInstance()
{
	if (!_imageManager) {
		_imageManager = new ImageManager();
		_imageManager->init();
	}
	return _imageManager;
}

void ImageManager::purge()
{
	if (!_imageManager) {
		return;
	}
	ImageManager *p = _imageManager;
	_imageManager = NULL;
	p->release();
}

bool ImageManager::init()
{
	this->setImageList(cocos2d::Array::create());
	return true;
}

/**
　* @brief 「画像を表示」の画像を更新する。「画像を表示」または「動画を表示」でゲームを止める設定のものがあれば、それ以外のものは更新しない。
 */
void ImageManager::update(float delta)
{
	auto movieManager = MovieManager::getInstance();
	auto imageGameStop = this->getImageCheckGameStop();
	bool bGameStop = (imageGameStop || movieManager->checkGameStop()) ? true : false;

	auto imageList = this->getImageList();
	cocos2d::Ref *ref;
	CCARRAY_FOREACH(imageList, ref) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
		auto p = static_cast<agtk::Image *>(ref);
#else
		auto p = dynamic_cast<agtk::Image *>(ref);
#endif
		// @todo 停止させる「画像を表示」に上乗せで、停止させる「動画を表示」があると、「画像の表示」の方も反応してしまうと思われる。
		if (!bGameStop || (bGameStop && imageGameStop == p)) {
			p->update(delta);
		}
	}

	//remove
	auto scene = GameManager::getInstance()->getCurrentScene();
	bool bRemoved;
	// @todo CCARRAY_FOREACH_REVERSEを使えば、シンプルに処理できるかも？
	do {
		auto imageList = this->getImageList();
		bRemoved = false;
		CCARRAY_FOREACH(imageList, ref) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
			auto p = static_cast<agtk::Image *>(ref);
#else
			auto p = dynamic_cast<agtk::Image *>(ref);
#endif
			if (p->isStop() || p->isClose()) {
				this->removeImage(p);
				bRemoved = true;
				break;
			}
		}
	} while (bRemoved);
}

void ImageManager::addImage(agtk::Object *object, agtk::data::ObjectCommandImageShowData *data)
{
	//設定無し。
	if (data->getImageId() < 0) {
		return;
	}
	auto imageList = cocos2d::__Array::create();
	//このオブジェクトがロックしたオブジェクトの中心
	if (data->getPositionType() == agtk::data::ObjectCommandImageShowData::kPositionLockObjectCenter) {
		auto sceneLayer = object->getSceneLayer();
		auto scene = sceneLayer->getScene();
// #AGTK-NX
#ifdef USE_SAR_OPTIMIZE_1
		auto objectAll = scene->getObjectAllReference(sceneLayer->getType());
#else
		auto objectAll = scene->getObjectAll(sceneLayer->getType());
#endif
		cocos2d::Ref *ref = nullptr;

		CCARRAY_FOREACH(objectAll, ref) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
			auto obj = static_cast<agtk::Object *>(ref);
#else
			auto obj = dynamic_cast<agtk::Object *>(ref);
#endif
			auto playObjectData = obj->getPlayObjectData();

			// ロックしているオブジェクトの場合
			if (playObjectData->isLocked(object->getInstanceId())) {
				auto image = agtk::Image::create(obj, data);
				if (image == nullptr) continue;
				imageList->addObject(image);
			}
		}
	}
	else {
		auto image = agtk::Image::create(object, data);
		if (image == nullptr) {
			return;
		}
		imageList->addObject(image);
	}

	cocos2d::Ref *ref;
	CCARRAY_FOREACH(imageList, ref) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
		auto image = static_cast<agtk::Image *>(ref);
#else
		auto image = dynamic_cast<agtk::Image *>(ref);
#endif

		//「画像の裏側に設定」
		if (object->getPlayer()) {
			image->setObjectBackside(object->getPlayer()->getTimelineBackside(data->getConnectId()));
		}

#ifdef USE_REDUCE_RENDER_TEXTURE
		int localZOrder = object->getLocalZOrder();
#else
		image->getSprite()->setLocalZOrder(object->getLocalZOrder());//zオーダー設定（※オブジェクトの手前に表示されるように）
#endif

		auto sceneLayer = object->getSceneLayer();
		if (sceneLayer->getType() == agtk::SceneLayer::kTypeMenu) {//メニューシーンの場合。
#ifdef USE_REDUCE_RENDER_TEXTURE
			localZOrder = image->getObjectBackside() ? agtk::Object::kPartPriorityBackImage : agtk::Object::kPartPriorityFrontImage;
			image->getObject()->addChild(image->getSprite());
#else
			sceneLayer->addChild(image->getSprite());
#endif
		}
		else {//通常シーンの場合。
			auto scene = sceneLayer->getScene();
			int layerId = image->getLayerId();

			// 「背景」設定時
			if (layerId == agtk::data::SceneData::kBackgroundLayerId) {
#ifdef USE_REDUCE_RENDER_TEXTURE
				localZOrder = 0;
#endif
				auto sceneBackground = scene->getSceneBackground();
				sceneBackground->addChild(image->getSprite());
			}
			// 「最前面＋メニューシーン」設定時
			else if (layerId == agtk::data::SceneData::kHudTopMostLayerId) {
#ifdef USE_REDUCE_RENDER_TEXTURE
				localZOrder = agtk::BaseLayer::HUD_MENU_ZORDER;
#endif
				auto menuSceneLayer = scene->getMenuLayer(layerId);
				menuSceneLayer->addChild(image->getSprite());
				if (sceneLayer->getType() == agtk::SceneLayer::kTypeMenu) {
					image->getSprite()->setCameraMask((unsigned short)cocos2d::CameraFlag::USER2);
				}
			}
			// 「最前面」設定時
			else if (layerId > 0) {
#ifdef USE_REDUCE_RENDER_TEXTURE
				localZOrder = image->getObjectBackside() ? agtk::Object::kPartPriorityBackImage : agtk::Object::kPartPriorityFrontImage;
#endif
				auto sceneLayer = scene->getSceneLayer(layerId);
				sceneLayer->addChild(image->getSprite());
				if (sceneLayer->getType() == agtk::SceneLayer::kTypeMenu) {
					image->getSprite()->setCameraMask((unsigned short)cocos2d::CameraFlag::USER1);
				}
			}
			else {
				CC_ASSERT(0);
				return;
			}
		}
#ifdef USE_REDUCE_RENDER_TEXTURE
		image->getSprite()->setLocalZOrder(localZOrder);
#endif
		this->getImageList()->addObject(image);
	}
}

void ImageManager::removeImage(agtk::Object *object)
{
lRetry:
	auto imageList = this->getImageList();
	cocos2d::Ref *ref;
	CCARRAY_FOREACH(imageList, ref) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
		auto p = static_cast<agtk::Image *>(ref);
#else
		auto p = dynamic_cast<agtk::Image *>(ref);
#endif
		if (p->getObject() == object) {
			this->removeImage(p);
			goto lRetry;
		}
	}
}

void ImageManager::removeImage(agtk::Image *image)
{
	auto scene = GameManager::getInstance()->getCurrentScene();
	auto object = image->getObject();
	auto sceneLayer = object->getSceneLayer();
#ifdef USE_REDUCE_RENDER_TEXTURE
	image->getSprite()->removeFromParent();
#else
	int layerId = image->getLayerId();
	if (layerId == agtk::data::SceneData::kBackgroundLayerId) {
		//background
		auto sceneBackground = scene->getSceneBackground();
		if (sceneBackground != nullptr) {
			sceneBackground->removeChild(image->getSprite());
		}
	}
	else if (layerId == agtk::data::SceneData::kTopMostLayerId ||
			 layerId == agtk::data::SceneData::kTopMostWithMenuLayerId) {
		//topMost
		auto sceneTopMost = scene->getSceneTopMost();
		if (sceneTopMost != nullptr) {
			sceneTopMost->removeChild(image->getSprite());
		}
	}
	else if (layerId > 0) {
		agtk::SceneLayer *layer = nullptr;
		//layer
		if (image->getObject()->getSceneData()->isMenuScene()) {
			layer = scene->getMenuLayer(layerId);
		}
		else {
			layer = scene->getSceneLayer(layerId);
		}
		if (layer != nullptr) {
			layer->removeChild(image->getSprite());
		}
	}
	else {
		CC_ASSERT(0);
		return;
	}
#endif

	//画像表示中はすべてのオブジェクト動作を再開する。
	auto data = image->getImageShowData();
	if (data->getStopObject()) {
// #AGTK-NX
#ifdef USE_SAR_OPTIMIZE_1
		auto objectList = scene->getObjectAllReference(sceneLayer->getType());
#else
		auto objectList = scene->getObjectAll(sceneLayer->getType());
#endif
		cocos2d::Ref *ref;
		CCARRAY_FOREACH(objectList, ref) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
			auto p = static_cast<agtk::Object *>(ref);
#else
			auto p = dynamic_cast<agtk::Object *>(ref);
#endif
			p->setWaitDuration300(0);
		}
	}
	//画像表示中はゲームの動作を再開
	if (data->getStopGame()) {
		// 待ちフレームをシーンに設定
		auto scene = GameManager::getInstance()->getCurrentScene();
		scene->setWaitDuration300(0);
	}

	this->getImageList()->removeObject(image);
}

cocos2d::__Array *ImageManager::getImageArray(agtk::Object *object)
{
	auto imageList = cocos2d::__Array::create();
	if (_imageList == nullptr) {
		return imageList;
	}
	cocos2d::Ref *ref;
	CCARRAY_FOREACH(_imageList, ref) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
		auto image = static_cast<agtk::Image *>(ref);
#else
		auto image = dynamic_cast<agtk::Image *>(ref);
#endif
		if (image->getObject() == object) {
			imageList->addObject(image);
		}
	}
	return imageList;
}

bool ImageManager::checkGameStop()
{
	return getImageCheckGameStop() != nullptr ? true : false;
}

agtk::Image *ImageManager::getImageCheckGameStop()
{
	//「ゲームの動作を一時停止」のチェック
	auto imageList = this->getImageList();
	cocos2d::Ref *ref;
	CCARRAY_FOREACH(imageList, ref) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
		auto p = static_cast<agtk::Image *>(ref);
#else
		auto p = dynamic_cast<agtk::Image *>(ref);
#endif
		if (p->getImageShowData()->getStopGame()) {
			return p;
		}
	}
	return nullptr;
}
