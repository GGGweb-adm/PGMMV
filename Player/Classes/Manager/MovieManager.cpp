#include "MovieManager.h"
#include "Manager/GameManager.h"
#include "Manager/ImageManager.h"

USING_NS_CC;

NS_AGTK_BEGIN
//-------------------------------------------------------------------------------------------------------------------
Movie::Movie()
{
	_object = nullptr;
	_videoSprite = nullptr;
	_movieShowData = nullptr;
	_movieData = nullptr;
	_fillBlackNode = nullptr;
	_objectBackside = false;
	_objectActionId = -1;
}

Movie::~Movie()
{
	CC_SAFE_RELEASE_NULL(_object);
	CC_SAFE_RELEASE_NULL(_videoSprite);
	CC_SAFE_RELEASE_NULL(_movieShowData);
	CC_SAFE_RELEASE_NULL(_movieData);
	CC_SAFE_RELEASE_NULL(_fillBlackNode);
}

bool Movie::init(agtk::Object *object, agtk::data::ObjectCommandMovieShowData *data)
{
	this->setObject(object);
	this->setMovieShowData(data);

	auto scene = GameManager::getInstance()->getCurrentScene();
	auto sceneLayer = object->getSceneLayer();
	auto projectData = GameManager::getInstance()->getProjectData();
	auto movieData = projectData->getMovieData(data->getMovieId());
	this->setMovieData(movieData);

	//サイズ	
	cocos2d::Size size;
	if (data->getDefaultSize()) {
		size.setSize(movieData->getWidth(), movieData->getHeight());
	}
	else {
		size.setSize(data->getWidth(), data->getHeight());
	}

	//位置
	cocos2d::Vec2 position;
	switch (data->getPositionType()) {
	case agtk::data::ObjectCommandMovieShowData::kPositionCenter: {//このオブジェクトの中心
		if (data->getUseConnect() && data->getConnectId() >= 0) {
			//接続点使用
			int connectId = data->getConnectId();
			agtk::Vertex4 vertex4;
			if (object->getTimeline(agtk::data::TimelineInfoData::kTimelineConnection, connectId, vertex4)) {
				position = Scene::getPositionSceneFromCocos2d(vertex4.addr()[0]);
			}
			else {
				//接続点が見つからない場合。
				return false;
			}
		}
		else {
			//このオブジェクトの中心
			auto centerPos = object->getCenterPosition();
			position = centerPos;
		}
		break; }
	case agtk::data::ObjectCommandMovieShowData::kPositionLockObjectCenter: {//このオブジェクトがロックしたオブジェクトの中心
		//※引数のobjectがロックされたオブジェクトである事を前提とする。
		auto centerPos = object->getCenterPosition();
		position = centerPos;
		break; }
	case agtk::data::ObjectCommandMovieShowData::kPositionScenePosition: {//シーンを基準にする
		auto sceneSize = scene->getSceneSize();
		//x
		switch (data->getHorzAlign()) {
		case agtk::data::ObjectCommandMovieShowData::kHorzAlignLeft: {
			position.x += size.width * 0.5f;
			break; }
		case agtk::data::ObjectCommandMovieShowData::kHorzAlignCenter: {
			position.x += sceneSize.x * 0.5f;
			break; }
		case agtk::data::ObjectCommandMovieShowData::kHorzAlignRight: {
			position.x += (sceneSize.x - size.width * 0.5f);
			break; }
		}
		//y
		switch (data->getVertAlign()) {
		case agtk::data::ObjectCommandMovieShowData::kVertAlignTop: {
			position.y += size.height * 0.5f;
			break; }
		case agtk::data::ObjectCommandMovieShowData::kVertAlignCenter: {
			position.y += sceneSize.y * 0.5f;
			break; }
		case agtk::data::ObjectCommandMovieShowData::kVertAlignBottom: {
			position.y += (sceneSize.y - size.height * 0.5f);
			break; }
		}
		break; }
	}
	position.x += data->getAdjustX();
	position.y += data->getAdjustY();
	position = Scene::getPositionCocos2dFromScene(position);

	//動画
	auto videoSprite = agtk::VideoSprite::createWithFilename(movieData->getFilename(), cocos2d::Size(movieData->getWidth(), movieData->getHeight()), data->getLoop());
	if(videoSprite == nullptr){
		CC_ASSERT(0);
		return false;
	}
	videoSprite->play((float)(movieData->getVolume() * data->getVolume()) * 0.01f);
	videoSprite->setPosition(position);
	videoSprite->setContentSize(size);
	this->setVideoSprite(videoSprite);

	this->setStartObjectPosition(this->getObject()->getCenterPosition());//生成時のオブジェクト位置。
	this->setStartPosition(position);//開始位置。

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
	//動画以外のエリアを黒塗りにする
	if (data->getFillBlack()) {
		auto sceneSize = scene->getSceneSize();
		auto fillBlackNode = PrimitiveManager::getInstance()->createPlate(0, 0, sceneSize.x, sceneSize.y, cocos2d::Color4F::BLACK);
		fillBlackNode->setAnchorPoint(cocos2d::Vec2(0, 0));//左下を軸にする。
		this->setFillBlackNode(fillBlackNode);
	}
	//オブジェクトのアクションが切り替わったら表示を終了
	if (data->getHideOnObjectActionChange()) {
		_objectActionId = object->getCurrentObjectAction()->getId();
	}
	return true;
}

void Movie::update(float delta)
{
	auto sprite = this->getVideoSprite();
	sprite->update(delta);

	auto type = this->getMovieShowData()->getPositionType();
	if (type == agtk::data::ObjectCommandMovieShowData::kPositionCenter || type == agtk::data::ObjectCommandMovieShowData::kPositionLockObjectCenter) {
		auto nowPosition = this->getObject()->getCenterPosition();
		auto move = nowPosition - this->getStartObjectPosition();
		move.y *= -1;//シーンとcocos2dのY座標の向きが逆のため。
		sprite->setPosition(this->getStartPosition() + move);
	}
}

void Movie::play()
{
	auto videoSprite = this->getVideoSprite();
	auto movieData = this->getMovieData();
	auto movieShowData = this->getMovieShowData();
	videoSprite->play((float)(movieData->getVolume() * movieShowData->getVolume()) * 0.01f);
}

void Movie::stop()
{
	auto videoSprite = this->getVideoSprite();
	videoSprite->stop();
}

void Movie::pause()
{
	auto videoSprite = this->getVideoSprite();
	videoSprite->pause();
}

void Movie::resume()
{
	auto videoSprite = this->getVideoSprite();
	videoSprite->resume();
}

bool Movie::isEnd()
{
	return (this->getVideoSprite()->getState() == agtk::VideoSprite::kStateStop) && (this->getMovieShowData()->getLoop() == false);
}

bool Movie::isPause()
{
	auto videoSprite = this->getVideoSprite();
	return videoSprite->isPause();
}

bool Movie::isClose()
{
	//オブジェクトのアクションが切り替わったら表示を終了。
	auto data = this->getMovieShowData();
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
MovieManager* MovieManager::_movieManager = NULL;
MovieManager::MovieManager()
{
	_movieList = nullptr;
}

MovieManager::~MovieManager()
{
	CC_SAFE_RELEASE_NULL(_movieList);
}

MovieManager* MovieManager::getInstance()
{
	if (!_movieManager) {
		_movieManager = new MovieManager();
		_movieManager->init();
	}
	return _movieManager;
}

void MovieManager::purge()
{
	if (!_movieManager) {
		return;
	}
	MovieManager *p = _movieManager;
	_movieManager = NULL;
	p->release();
}

bool MovieManager::init()
{
	this->setMovieList(cocos2d::Array::create());
	return true;
}

void MovieManager::update(float delta)
{
	auto imageManager = ImageManager::getInstance();
	auto movieGameStop = this->getMovieCheckGameStop();
	bool bGameStop = (movieGameStop || imageManager->checkGameStop()) ? true : false;

	auto movieList = this->getMovieList();
	cocos2d::Ref *ref;
	CCARRAY_FOREACH(movieList, ref) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
		auto movie = static_cast<agtk::Movie *>(ref);
#else
		auto movie = dynamic_cast<agtk::Movie *>(ref);
#endif
		if (!bGameStop || (bGameStop && movieGameStop == movie)) {
			if (movie->isPause()) movie->resume();//一時停止中は一時停止を解除する。
			movie->update(delta);
		}
		else {
			if (!movie->isPause()) movie->pause();//一時停止する。
		}
	}

	//remove
	bool bRemoved;
	do {
		movieList = this->getMovieList();
		bRemoved = false;
		CCARRAY_FOREACH(movieList, ref) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
			auto movie = static_cast<agtk::Movie *>(ref);
#else
			auto movie = dynamic_cast<agtk::Movie *>(ref);
#endif
			if (movie->isEnd() || movie->isClose()) {
				this->removeMovie(movie);
				bRemoved = true;
				break;
			}
		}
	} while (bRemoved);
}

void MovieManager::addMovie(agtk::Object *object, agtk::data::ObjectCommandMovieShowData *data)
{
	//設定無し。
	if (data->getMovieId() < 0) {
		return;
	}

	auto movieList = cocos2d::__Array::create();
	// このオブジェクトがロックしたオブジェクトの中心
	if (data->getPositionType() == agtk::data::ObjectCommandMovieShowData::kPositionLockObjectCenter) {
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
				auto movie = agtk::Movie::create(obj, data);
				if (movie == nullptr) continue;
				movieList->addObject(movie);
			}
		}
	}
	else {
		auto movie = agtk::Movie::create(object, data);
		if (movie == nullptr) {
			return;
		}
		movieList->addObject(movie);
	}

	cocos2d::Ref *ref;
	CCARRAY_FOREACH(movieList, ref) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
		auto movie = static_cast<agtk::Movie *>(ref);
#else
		auto movie = dynamic_cast<agtk::Movie *>(ref);
#endif
		//「画像の裏側に設定」
		if (object->getPlayer()) {
			movie->setObjectBackside(object->getPlayer()->getTimelineBackside(data->getConnectId()));
		}

#ifdef USE_REDUCE_RENDER_TEXTURE
		int localZOrder = object->getLocalZOrder();
#else
		movie->getVideoSprite()->setLocalZOrder(object->getLocalZOrder());// zオーダー設定（※オブジェクトの手前に表示されるように）
#endif

		auto scene = GameManager::getInstance()->getCurrentScene();
		auto sceneLayer = scene->getSceneLayer(object->getLayerId());

		int layerId = movie->getLayerId();

		// 「背景」設定時
		if (layerId == agtk::data::SceneData::kBackgroundLayerId) {
#ifdef USE_REDUCE_RENDER_TEXTURE
			localZOrder = 0;
#endif
			auto sceneBackground = scene->getSceneBackground();
			if (movie->getFillBlackNode()) {
				sceneBackground->addChild(movie->getFillBlackNode());
			}
			sceneBackground->addChild(movie->getVideoSprite());
		}
		// 「最前面＋メニューシーン」設定時
		else if (layerId == agtk::data::SceneData::kHudTopMostLayerId) {
#ifdef USE_REDUCE_RENDER_TEXTURE
			localZOrder = agtk::BaseLayer::HUD_MENU_ZORDER;
#endif
			auto menuSceneLayer = scene->getMenuLayer(layerId);
			if (movie->getFillBlackNode()) {
				menuSceneLayer->addChild(movie->getFillBlackNode());
			}
			menuSceneLayer->addChild(movie->getVideoSprite());
		}
		// 「最前面」設定時
		else if (layerId > 0) {
#ifdef USE_REDUCE_RENDER_TEXTURE
			localZOrder = movie->getObjectBackside() ? agtk::Object::kPartPriorityBackMovie : agtk::Object::kPartPriorityFrontMovie;
#endif
			auto sceneLayer = scene->getSceneLayer(layerId);
			if (movie->getFillBlackNode()) {
				sceneLayer->addChild(movie->getFillBlackNode());
			}
			sceneLayer->addChild(movie->getVideoSprite());
		}
		else {
			CC_ASSERT(0);
			return;
		}
		this->getMovieList()->addObject(movie);
	}
}

void MovieManager::removeMovie(agtk::Object *object)
{
lRetry:
	auto movieList = this->getMovieList();
	cocos2d::Ref *ref;
	CCARRAY_FOREACH(movieList, ref) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
		auto p = static_cast<agtk::Movie *>(ref);
#else
		auto p = dynamic_cast<agtk::Movie *>(ref);
#endif
		if (p->getObject() == object) {
			this->removeMovie(p);
			goto lRetry;
		}
	}
}

void MovieManager::removeMovie(agtk::Movie *movie)
{
	auto scene = GameManager::getInstance()->getCurrentScene();
	auto object = movie->getObject();
	auto sceneLayer = object->getSceneLayer();
#ifdef USE_REDUCE_RENDER_TEXTURE
	if (movie->getFillBlackNode()) {
		movie->getFillBlackNode()->removeFromParent();
	}
	movie->getVideoSprite()->removeFromParent();
#else
	int layerId = movie->getLayerId();
	if (layerId == agtk::data::SceneData::kBackgroundLayerId) {
		//background
		auto sceneBackground = scene->getSceneBackground();
		if (sceneBackground) {
			if (movie->getFillBlackNode()) {
				sceneBackground->removeChild(movie->getFillBlackNode());
			}
			sceneBackground->removeChild(movie->getVideoSprite());
		}
	}
	else if (layerId == agtk::data::SceneData::kTopMostLayerId ||
			 layerId == agtk::data::SceneData::kTopMostWithMenuLayerId) {
		//topMost
		auto sceneTopMost = scene->getSceneTopMost();
		if (movie->getFillBlackNode()) {
			sceneTopMost->removeChild(movie->getFillBlackNode());
		}
		sceneTopMost->removeChild(movie->getVideoSprite());
	}
	else if (layerId > 0) {
		//layer
		auto sceneLayer = scene->getSceneLayer(layerId);
		if (sceneLayer) {
			if (movie->getFillBlackNode()) {
				sceneLayer->removeChild(movie->getFillBlackNode());
			}
			sceneLayer->removeChild(movie->getVideoSprite());
		}
	}
	else {
		CC_ASSERT(0);
		return;
	}
#endif

	//画像表示中はすべてのオブジェクト動作を再開する。
	auto data = movie->getMovieShowData();
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

	this->getMovieList()->removeObject(movie);
}

cocos2d::__Array *MovieManager::getMovieArray(agtk::Object *object)
{
	auto movieList = cocos2d::__Array::create();
	if (_movieList == nullptr) {
		return movieList;
	}
	cocos2d::Ref *ref;
	CCARRAY_FOREACH(_movieList, ref) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
		auto movie = static_cast<agtk::Movie *>(ref);
#else
		auto movie = dynamic_cast<agtk::Movie *>(ref);
#endif
		if (movie->getObject() == object) {
			movieList->addObject(movie);
		}
	}
	return movieList;
}

bool MovieManager::checkGameStop()
{
	return this->getMovieCheckGameStop() != nullptr ? true : false;
}

agtk::Movie *MovieManager::getMovieCheckGameStop()
{
	//「ゲームの動作を一時停止」のチェック
	auto movieList = this->getMovieList();
	cocos2d::Ref *ref = nullptr;
	CCARRAY_FOREACH(movieList, ref) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
		auto movie = static_cast<agtk::Movie *>(ref);
#else
		auto movie = dynamic_cast<agtk::Movie *>(ref);
#endif
		if (movie->getMovieShowData()->getStopGame()) {
			return movie;
		}
	}
	return nullptr;
}
