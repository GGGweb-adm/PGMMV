#include "Camera.h"
#include "Lib/Common.h"
#include "Manager/PrimitiveManager.h"
#include "Manager/GameManager.h"
#include "Manager/DebugManager.h"

NS_AGTK_BEGIN

//-------------------------------------------------------------------------------------------------------------------
CameraVec2::CameraVec2() : agtk::EventTimer()
{
	_value = cocos2d::Vec2::ZERO;
	_prevValue = cocos2d::Vec2::ZERO;
	_nextValue = cocos2d::Vec2::ZERO;
	_oldValue = cocos2d::Vec2::ZERO;
}

CameraVec2::~CameraVec2()
{
}

bool CameraVec2::init(cocos2d::Vec2 value)
{
	if (agtk::EventTimer::init() == false) {
		return false;
	}
	_value = value;
	_prevValue = value;
	_nextValue = value;
	_oldValue = value;

	this->setProcessingFunc([&](float dt) {
		_oldValue = _value;
		_value = AGTK_PARABOLA_INTERPOLATE2(_prevValue, _nextValue, _seconds, _timer);
	});
	this->setEndFunc([&]() {
		_oldValue = _value;
		_value = _nextValue;
	});

	return true;
}

cocos2d::Vec2 CameraVec2::setValue(cocos2d::Vec2 value, float seconds)
{
	auto state = this->getState();
	if (state == kStateIdle) {
		if (value == _value) {
		//	return _value;
			seconds = 0.0f;
		}
	}
	else {
		if (value == _nextValue) {
			return _value;
		}
	}

	_nextValue = value;
	_prevValue = _value;
	//if (seconds == 0.0f) {
	//	_value = value;
	//}
	this->start(seconds);
	return _value;
}

cocos2d::Vec2 CameraVec2::addValue(cocos2d::Vec2 value, float seconds)
{
	auto v = this->getValue();
	this->setValue(v + value, seconds);
	return v;
}

//-------------------------------------------------------------------------------------------------------------------
// CameraRotation class
/**
* コンストラクタ
*/
CameraRotation::CameraRotation()
{
	_value = 0.0f;
	_nextValue = 0.0f;
	_prevValue = 0.0f;
}

/**
* デストラクタ
*/
CameraRotation::~CameraRotation()
{
	// nothing to do.
}

/**
* 初期化
* @param	value	初期値
* @return			初期化の成否
*/
bool CameraRotation::init(float value)
{
	if (agtk::EventTimer::init() == false) {
		return false;
	}
	_value = value;
	_prevValue = value;
	_nextValue = value;
	_oldValue = value;

	this->setProcessingFunc([&](float dt) {
		_oldValue = _value;
		_value = AGTK_PARABOLA_INTERPOLATE2(_prevValue, _nextValue, _seconds, _timer);
	});
	this->setEndFunc([&]() {
		_oldValue = _value;
		_value = _nextValue;
	});

	return true;
}

/**
* 値の設定
* @param	value	設定値
* @param	seconds	変動時間(初期値:0.0f)
* @return			設定された値
*/
float CameraRotation::setValue(float value, float seconds)
{
	auto state = this->getState();
	if (state == kStateIdle) {
		if (value == _value) {
			return _value;
		}
	}
	else {
		if (value == _nextValue) {
			return _value;
		}
	}

	_nextValue = value;
	_prevValue = _value;
	this->start(seconds);
	return _value;
}

/**
* 値の加算
* @param	value	加算値
* @param	seconds	変動時間(初期値:0.0f)
* @return			加算前の値
*/
float CameraRotation::addValue(float value, float seconds)
{
	auto nowValue = this->getValue();
	this->setValue(nowValue + value, seconds);
	return nowValue;
}

//-------------------------------------------------------------------------------------------------------------------
/**
 * @bref class Camera
 */
float Camera::_defaultMoveDuration = 0.0f;
Camera::Camera()
{
	_moveDuration = _defaultMoveDuration;

	_type = CameraFollowType::kCameraFollowNone;
	_targetType = CameraTargetType::kCameraTargetNone;
	_limitAreaRect = cocos2d::Rect::ZERO;
	_limitMoveArea = cocos2d::Size::ZERO;
	_targetPosition = cocos2d::Vec2::ZERO;
	_targetObjectPosition = cocos2d::Vec2::ZERO;
	_displaySizeRatio = cocos2d::Vec2::ONE;
	_initialOffsetPosition = cocos2d::Vec2::ZERO;
	_camera = nullptr;
	_layer = nullptr;
	_layerList = nullptr;
	_autoAction = nullptr;
	_sceneData = nullptr;
	_followNode = nullptr;
	_targetObject = nullptr;
	_layerPosition = nullptr;
	_cameraPosition = nullptr;
	_cameraPosition2 = nullptr;
	_cameraScale = nullptr;
#ifdef FIX_ACT2_4471
	_commandScale = nullptr;
#endif
	_cameraRotationX = nullptr;
	_cameraRotationY = nullptr;
	_cameraRotationZ = nullptr;
	_ignoreCorrectionPos = false;
	_menuCamera = nullptr;
	_topMostCamera = nullptr;
#ifdef USE_PREVIEW
	_debugMoveAreaSprite = nullptr;
#endif
	_counter = 0;

	_isZooming = false;
	_scene = nullptr;
	_setTargetObjectFlag = false;
}

Camera::~Camera()
{
	CC_SAFE_RELEASE_NULL(_layer);
	CC_SAFE_RELEASE_NULL(_layerList);
	CC_SAFE_RELEASE_NULL(_autoAction);
	CC_SAFE_RELEASE_NULL(_sceneData);
	CC_SAFE_RELEASE_NULL(_followNode);
	CC_SAFE_RELEASE_NULL(_targetObject);
	CC_SAFE_RELEASE_NULL(_layerPosition);
	CC_SAFE_RELEASE_NULL(_cameraPosition);
	CC_SAFE_RELEASE_NULL(_cameraPosition2);
	CC_SAFE_RELEASE_NULL(_cameraScale);
	CC_SAFE_RELEASE_NULL(_cameraRotationZ);
	CC_SAFE_RELEASE_NULL(_cameraRotationY);
	CC_SAFE_RELEASE_NULL(_cameraRotationX);
	CC_SAFE_RELEASE_NULL(_camera);
	CC_SAFE_RELEASE_NULL(_menuCamera);
	CC_SAFE_RELEASE_NULL(_topMostCamera);
#ifdef FIX_ACT2_4471
	CC_SAFE_RELEASE_NULL(_commandScale);
#endif
#ifdef USE_PREVIEW
	if (_debugMoveAreaSprite) {
		_debugMoveAreaSprite->removeFromParentAndCleanup(true);
	}
	CC_SAFE_RELEASE_NULL(_debugMoveAreaSprite);
#endif
}

bool Camera::init(agtk::Scene *scene)
{
	_scene = scene;
	auto sceneData = scene->getSceneData();
	auto projectData = GameManager::getInstance()->getProjectData();
	CC_ASSERT(sceneData);
	float screenWidth = projectData->getScreenSize().width;
	float screenHeight = projectData->getScreenSize().height;

	_limitAreaRect = sceneData->getLimitCameraRect();
	{
		float moveAreaWidth = 0;
		float moveAreaHeight = 0;
		float tileWidth = projectData->getTileWidth();//タイル幅
		float tileHeight = projectData->getTileHeight();//タイル高さ
		float blockWidthNum = 25.5;//タイル横個数
		float blockHeightNum = 13;//タイル縦個数
		if (projectData->getScreenSize().width - tileWidth * blockWidthNum > 0) {
			moveAreaWidth = projectData->getScreenSize().width - tileWidth * blockWidthNum;
		}
		else {
			auto variable = 0.95625;//tileWidth * blockWidthNum / projectData->getScreenSize().width;// 48 * 25.5 / 1280
			blockWidthNum = projectData->getScreenSize().width * variable / tileWidth;//※0.75はtileWith=48、blockWidthNum=20に対する比率です。
			moveAreaWidth = projectData->getScreenSize().width - tileWidth * blockWidthNum;
		}
		if (projectData->getScreenSize().height - tileHeight * blockHeightNum > 0) {
			moveAreaHeight = projectData->getScreenSize().height - tileHeight * blockHeightNum;
		}
		else {
			auto variable = 0.8667;// tileHeight * blockHeightNum / projectData->getScreenSize().height;//48 * 13 / 720
			blockHeightNum = projectData->getScreenSize().height * variable / tileHeight;//※0.4はtileHeight=48、blockHeightNum=6に対する比率です。
			moveAreaHeight = projectData->getScreenSize().height - tileHeight * blockHeightNum;
		}
		CC_ASSERT(moveAreaWidth > 0 && moveAreaHeight > 0);
		_limitMoveArea = cocos2d::Size((int)moveAreaWidth, (int)moveAreaHeight);
		//プレイヤーの中心にカメラが追従するように。
		_limitMoveArea = cocos2d::Size(0, 0);
	}
	_halfScreenSize.x = projectData->getScreenSize().width * 0.5f;
	_halfScreenSize.y = projectData->getScreenSize().height * 0.5f;

	this->setSceneData(sceneData);

	auto layerPosition = agtk::CameraVec2::create(cocos2d::Vec2::ZERO);
	if (layerPosition == nullptr) {
		CC_ASSERT(0);
		return false;
	}
	this->setLayerPosition(layerPosition);

	this->setCameraPosition(agtk::CameraVec2::create(cocos2d::Vec2(screenWidth * 0.5f, screenHeight * 0.5f)));
	this->setCameraPosition2(agtk::CameraVec2::create(cocos2d::Vec2(screenWidth * 0.5f, screenHeight * 0.5f)));

	auto cameraScale = agtk::CameraVec2::create(cocos2d::Vec2::ONE);
	if (cameraScale == nullptr) {
		CC_ASSERT(0);
		return false;
	}
	this->setCameraScale(cameraScale);
#ifdef FIX_ACT2_4471
	this->setCommandScale(agtk::CameraVec2::create(cocos2d::Vec2::ONE));
#endif

	this->setCameraRotationX(agtk::CameraRotation::create(0));
	if (this->getCameraRotationX() == nullptr) {
		CC_ASSERT(0);
		return false;
	}
	this->setCameraRotationY(agtk::CameraRotation::create(0));
	if (this->getCameraRotationY() == nullptr) {
		CC_ASSERT(0);
		return false;
	}
	this->setCameraRotationZ(agtk::CameraRotation::create(0));
	if (this->getCameraRotationZ() == nullptr) {
		CC_ASSERT(0);
		return false;
	}

	this->setScreenSize(projectData->getScreenSize());
	this->setSceneSize(cocos2d::Size(
		projectData->getScreenWidth() * sceneData->getHorzScreenCount(),
		projectData->getScreenHeight() * sceneData->getVertScreenCount()
	));

	this->setMoveArea(calcMoveArea(cocos2d::Vec2::ZERO, this->getScale()));
	auto cameraPoint = this->calcCameraPoint(cocos2d::Vec2::ZERO, this->getScale());
	auto cameraPosition = this->getCameraPosition();
	cameraPosition->setValue(cameraPoint);
	this->getCameraPosition2()->setValue(cameraPoint);
	cocos2d::Vec2 cpos = cameraPosition->getValue();
	cocos2d::Vec2 lpos = -cpos + _halfScreenSize;
	layerPosition->setValue(lpos);

	//menu camera
	auto menuCamera = cocos2d::Camera::create();
	menuCamera->setCameraFlag(cocos2d::CameraFlag::USER1);
	this->setMenuCamera(menuCamera);

	//topMost camera
	auto topMostCamera = cocos2d::Camera::create();
	topMostCamera->setCameraFlag(cocos2d::CameraFlag::USER2);
	this->setTopMostCamera(topMostCamera);

	//sceneLayer position.
	auto keys = this->getSceneData()->getLayerList()->allKeys();
	cocos2d::Ref *ref;
	CCARRAY_FOREACH(keys, ref) {
		auto layerId = dynamic_cast<cocos2d::Integer *>(ref)->getValue();
		_sceneLayerPositionList[layerId] = cocos2d::Vec2::ZERO;
	}

	return true;
}

void Camera::setTargetObject(agtk::Object *object)
{
	// カメラ切り替え扱いにして座標移動の補間を行わないで注視対象を設定する
	setTargetObject(object, true);
}

void Camera::setTargetObject(agtk::Object *object, bool isChangeCamera)
{
	float duration = isChangeCamera ? 0 : _moveDuration;

	if (_targetObject == object && _targetType == CameraTargetType::kCameraTargetObject) {
		return;
	}
	if (_targetObject) {
		_targetObject->release();
		_targetObject = nullptr;
	}
	_targetObject = object;
	_targetObject->retain();

	// オブジェクトの追従を設定
	_targetType = CameraTargetType::kCameraTargetObject;

	auto gameManager = GameManager::getInstance();
	auto projectData = gameManager->getProjectData();
	auto sceneData = this->getSceneData();
	float sceneHeight = projectData->getScreenHeight() * sceneData->getVertScreenCount();

	cocos2d::Vec2 pos = this->getTargetObject()->getDispPosition();
	_targetObjectPosition = pos;
	pos.y = sceneHeight - pos.y;

	this->setIgnoreCorrectionPos(false);

	this->setMoveArea(calcMoveArea(pos, this->getScale()));
	auto cameraPoint = this->calcCameraPoint(pos, this->getScale());

	auto cameraPosition = this->getCameraPosition();
	cameraPosition->setValue(cameraPoint, duration);
	// レイヤーの位置を更新
	updateLayerPosition(cameraPoint, duration);

	_setTargetObjectFlag = true;
}

void Camera::resetTargetObject(agtk::Object *object)
{
	if (_targetObject == object) {
		_targetObject->release();
		_targetObject = nullptr;
	}
}

/**
* カメラ座標を指定座標に設定
* @param	position	座標
* @param	isInit		初期設定フラグ
* @note		position は cocos2d 座標系
*/
void Camera::setTargetPosition(cocos2d::Vec2 position, bool isInit)
{
	// カメラ切り替え扱いにして座標移動の補間を行わないで注視座標を設定する
	setTargetPosition(position, true, isInit);
}

/**
* カメラ座標を指定座標に設定
* @param	position		座標
* @param	isChangeCamera	カメラ変更フラグ
* @param	isInit			初期設定フラグ
* @note		position は cocos2d 座標系
*/
void Camera::setTargetPosition(cocos2d::Vec2 position, bool isChangeCamera, bool isInit)
{
	float duration = isChangeCamera ? 0 : _moveDuration;

	//小数点以下切り捨て。
	position.x = (int)position.x;
	position.y = (int)position.y;

	// 追従しないよう設定
	_targetType = CameraTargetType::kCameraTargetNone;

	_targetPosition = position;

	auto gameManager = GameManager::getInstance();
	auto projectData = gameManager->getProjectData();
	auto sceneData = this->getSceneData();
	float sceneHeight = projectData->getScreenHeight() * sceneData->getVertScreenCount();

	cocos2d::Vec2 pos = _targetPosition;
	pos.y = sceneHeight - pos.y;

	this->setIgnoreCorrectionPos(false);

	this->setMoveArea(calcMoveArea(pos, this->getScale()));
	auto cameraPoint = this->calcCameraPoint(pos, this->getScale());

	auto layerPosition = this->getLayerPosition();
	auto cameraPosition = this->getCameraPosition();
	cameraPosition->setValue(cameraPoint, duration);

	// レイヤーの位置を更新
	updateLayerPosition(cameraPoint, duration);

	// 初期設定の場合
	if (isInit) {
		// レイヤー位置の逆位置を初期値として保持
		this->setInitialOffsetPosition(layerPosition->getNextValue() * -1);
	}
}

void Camera::setTargetFixedPosition(cocos2d::Vec2 position)
{
	// カメラ切り替え扱いにして座標移動の補間を行わないで注視座標を設定する
	setTargetFixedPosition(position, true);
}

void Camera::setTargetFixedPosition(cocos2d::Vec2 position, bool isChangeCamera)
{
	float duration = isChangeCamera ? 0 : _moveDuration;

	// 追従しないよう設定
	_targetType = CameraTargetType::kCameraTargetNone;

	_targetPosition = position;

	auto gameManager = GameManager::getInstance();
	auto projectData = gameManager->getProjectData();
	auto sceneData = this->getSceneData();
	float sceneHeight = projectData->getScreenHeight() * sceneData->getVertScreenCount();

	cocos2d::Vec2 pos = _targetPosition;
	pos.y = sceneHeight - pos.y;

	// 固定位置に設定するために位置補正を切る
	this->setIgnoreCorrectionPos(true);

	this->setMoveArea(calcMoveArea(pos, this->getScale()));

	auto cameraPoint = pos;
	auto layerPosition = this->getLayerPosition();
	auto cameraPosition = this->getCameraPosition();
	cameraPosition->setValue(cameraPoint, duration);
	if (duration == 0.0f) {
		cameraPosition->update(0.0f);
	}
	cocos2d::Vec2 lpos = -cameraPoint;

	lpos.x += _halfScreenSize.x * getDisplaySizeRatio().x;
	lpos.y += _halfScreenSize.y * getDisplaySizeRatio().y;

	lpos.x *= projectData->getScreenWidth() / _displaySize.width;
	lpos.y *= projectData->getScreenHeight() / _displaySize.height;

	layerPosition->setValue(lpos, duration);
}

bool Camera::isDisableLimitCamera()
{
	auto currentScene = _scene;
	if (currentScene == nullptr) {
		return false;
	}
	return currentScene->getSceneData()->getDisableLimitCamera();
}

void Camera::start()
{
	_counter = 0;
	this->update(_defaultMoveDuration);
}

void Camera::end()
{
	auto layer = this->getLayer();
	if (layer) {
		layer->setPosition(cocos2d::Vec2::ZERO);
	}
	_scene = nullptr;
	_setTargetObjectFlag = false;
	_counter = 0;
}

cocos2d::Rect Camera::calcMoveArea(cocos2d::Vec2 pos, cocos2d::Vec2 scale)
{
	auto limitAreaRect = this->getLimitAreaRect();
	float sceneWidth = this->getSceneSize().width;
	float sceneHeight = this->getSceneSize().height;
	float screenWidth = this->getScreenSize().width * scale.x;
	float screenHeight = this->getScreenSize().height * scale.y;
	float limitMoveAreaWidth = this->getLimitMoveArea().width * scale.x;
	float limitMoveAreaHeight = this->getLimitMoveArea().height * scale.y;
	limitAreaRect.origin.y = sceneHeight - limitAreaRect.getMaxY();

	//１．ターゲットポイントから、シーン上に移動制限領域を求める。この領域はターゲットポイントを中心になるように求める。
	//	シーンの領域外の場合は補正する。
	cocos2d::Rect moveArea = cocos2d::Rect(
		//moveArea.origin.x = pos.x - limitMoveAreaWidth * 0.5f,
		//moveArea.origin.y = pos.y - limitMoveAreaHeight * 0.5f,
		pos.x - limitMoveAreaWidth * 0.5f,
		pos.y - limitMoveAreaHeight * 0.5f,
		limitMoveAreaWidth,
		limitMoveAreaHeight
	);

	// 座標補正回避フラグがOFF かつ ズーム動作中でない場合
	//if (!_ignoreCorrectionPos && !_isZooming) {
	if (!_ignoreCorrectionPos) {

		// カメラの表示範囲制限 無効の場合は制限調整をしない
		if (this->isDisableLimitCamera()) {
			return moveArea;
		}

		// カメラの表示制限矩形作成
		cocos2d::Rect areaRect(
			moveArea.getMidX() - screenWidth * 0.5f,
			moveArea.getMidY() - screenHeight * 0.5f,
			screenWidth, screenHeight
		);

		if (areaRect.size.width >= limitAreaRect.size.width) {//表示範囲がスクリーン幅以下
			moveArea.origin.x = limitAreaRect.getMidX();
		}
		else if (areaRect.getMinX() < limitAreaRect.getMinX()) {
			moveArea.origin.x = limitAreaRect.getMinX() + areaRect.size.width * 0.5f;
		}
		else if (areaRect.getMaxX() > limitAreaRect.getMaxX()) {
			moveArea.origin.x = limitAreaRect.getMaxX() - areaRect.size.width * 0.5f;
		}
		if (areaRect.size.height >= limitAreaRect.size.height) {//表示範囲がスクリーン高さ以下
			moveArea.origin.y = limitAreaRect.getMidY();
		}
		else if (areaRect.getMinY() < limitAreaRect.getMinY()) {
			moveArea.origin.y = limitAreaRect.getMinY() + areaRect.size.height * 0.5f;
		}
		else if (areaRect.getMaxY() > limitAreaRect.getMaxY()) {
			moveArea.origin.y = limitAreaRect.getMaxY() - areaRect.size.height * 0.5f;
		}
	}

	return moveArea;
}

cocos2d::Rect Camera::updateMoveArea(cocos2d::Vec2 pos, cocos2d::Vec2 scale)
{
	auto limitAreaRect = this->getLimitAreaRect();
	float sceneWidth = this->getSceneSize().width;
	float sceneHeight = this->getSceneSize().height;
	float screenWidth = this->getScreenSize().width * scale.x;
	float screenHeight = this->getScreenSize().height * scale.y;
	limitAreaRect.origin.y = sceneHeight - limitAreaRect.getMaxY();
	auto moveArea = this->getMoveArea();

	// 座標補正回避フラグがOFF かつ ズーム動作中でない場合
	//if (!_ignoreCorrectionPos && !_isZooming) {
	if (!_ignoreCorrectionPos) {

		if (pos.x < moveArea.origin.x) {
			moveArea.origin.x = pos.x;
		}
		else if (moveArea.origin.x + moveArea.size.width < pos.x) {
			moveArea.origin.x = pos.x - moveArea.size.width;
		}

		if (pos.y < moveArea.origin.y) {
			moveArea.origin.y = pos.y;
		}
		else if (moveArea.origin.y + moveArea.size.height < pos.y) {
			moveArea.origin.y = pos.y - moveArea.size.height;
		}

		// カメラの表示範囲制限 無効の場合は制限調整をしない
		if (this->isDisableLimitCamera()) {
			return moveArea;
		}

		// カメラの表示制限矩形作成
		cocos2d::Rect areaRect(
			moveArea.getMidX() - screenWidth * 0.5f,
			moveArea.getMidY() - screenHeight * 0.5f,
			screenWidth, screenHeight
		);

		if (areaRect.size.width >= limitAreaRect.size.width) {//表示範囲がスクリーン幅以下
			moveArea.origin.x = limitAreaRect.getMidX();
		}
		else if (areaRect.getMinX() < limitAreaRect.getMinX()) {
			moveArea.origin.x = limitAreaRect.getMinX() + areaRect.size.width * 0.5f;
		}
		else if (areaRect.getMaxX() > limitAreaRect.getMaxX()) {
			moveArea.origin.x = limitAreaRect.getMaxX() - areaRect.size.width * 0.5f;
		}
		if (areaRect.size.height >= limitAreaRect.size.height) {//表示範囲がスクリーン高さ以下
			moveArea.origin.y = limitAreaRect.getMidY();
		}
		else if (areaRect.getMinY() < limitAreaRect.getMinY()) {
			moveArea.origin.y = limitAreaRect.getMinY() + areaRect.size.height * 0.5f;
		}
		else if (areaRect.getMaxY() > limitAreaRect.getMaxY()) {
			moveArea.origin.y = limitAreaRect.getMaxY() - areaRect.size.height * 0.5f;
		}
	}

	return moveArea;
}

cocos2d::Vec2 Camera::calcCameraPoint(cocos2d::Vec2 pos, cocos2d::Vec2 scale)
{
	auto moveArea = this->calcMoveArea(pos, scale);
	if (moveArea.containsPoint(pos) == false) {
		//移動領域外
		//CCLOG("outside area");
	}
	return cocos2d::Vec2(moveArea.origin.x + moveArea.size.width * 0.5f, moveArea.origin.y + moveArea.size.height * 0.5f);
}

cocos2d::Vec2 Camera::updateCameraPoint(cocos2d::Vec2 pos, cocos2d::Vec2 scale, bool& bUpdate)
{
	auto moveArea = this->getMoveArea();
	float sceneWidth = this->getSceneSize().width;
	float sceneHeight = this->getSceneSize().height;
	float screenWidth = this->getScreenSize().width * scale.x;
	float screenHeight = this->getScreenSize().height * scale.y;
	float limitMoveAreaWidth = this->getLimitMoveArea().width * scale.x;
	float limitMoveAreaHeight = this->getLimitMoveArea().height * scale.y;
	if (moveArea.size.width != limitMoveAreaWidth || moveArea.size.height != limitMoveAreaHeight) {
		float x = moveArea.origin.x + (moveArea.size.width - limitMoveAreaWidth) * 0.5f;
		float y = moveArea.origin.y + (moveArea.size.height - limitMoveAreaHeight) * 0.5f;
		this->setMoveArea(cocos2d::Rect(x, y, limitMoveAreaWidth, limitMoveAreaHeight));
		moveArea = this->getMoveArea();
	}
	std::function<bool(cocos2d::Vec2)> isInsideSceneArea = [&](cocos2d::Vec2 pos) {
		if (pos.x < 0) {
			return false;
		}
		else if (pos.x > sceneWidth) {
			return false;
		}
		if (pos.y < 0) {
			return false;
		}
		else if (pos.y > sceneHeight) {
			return false;
		}
		return true;
	};

	moveArea = this->updateMoveArea(pos, scale);
	bUpdate = true;
	this->setMoveArea(moveArea);
	auto cameraPoint = cocos2d::Vec2(moveArea.origin.x + moveArea.size.width * 0.5f, moveArea.origin.y + moveArea.size.height * 0.5f);

	//オブジェクトを追従。
	if(_targetType == CameraTargetType::kCameraTargetObject && this->getTargetObject()) {
		auto targetObject = this->getTargetObject();
		int layerId = targetObject->getLayerId();
		float moveSpeedX = this->getSceneData()->getLayerMoveSpeedX(layerId);
		float moveSpeedY = this->getSceneData()->getLayerMoveSpeedY(layerId);
		if (moveSpeedX != 100.0f || moveSpeedY != 100.0f) {
			auto layerPosition = _sceneLayerPositionList[layerId];
			cameraPoint += layerPosition;
			moveArea.origin += layerPosition; 

			auto limitAreaRect = this->getLimitAreaRect();
			float sceneWidth = this->getSceneSize().width;
			float sceneHeight = this->getSceneSize().height;
			float screenWidth = this->getScreenSize().width * scale.x;
			float screenHeight = this->getScreenSize().height * scale.y;
			float limitMoveAreaWidth = this->getLimitMoveArea().width * scale.x;
			float limitMoveAreaHeight = this->getLimitMoveArea().height * scale.y;
			limitAreaRect.origin.y = sceneHeight - limitAreaRect.getMaxY();

			auto _moveArea = moveArea;

			// 座標補正回避フラグがOFF かつ ズーム動作中でない場合
			bool update = false;
			//if (!_ignoreCorrectionPos && !_isZooming) {
			if (!_ignoreCorrectionPos) {

				// カメラの表示範囲制限 無効の場合は制限調整をしない
				if (this->isDisableLimitCamera()) {
					return cameraPoint;
				}

				// カメラの表示制限矩形作成
				cocos2d::Rect areaRect(
					moveArea.getMidX() - screenWidth * 0.5f,
					moveArea.getMidY() - screenHeight * 0.5f,
					screenWidth, screenHeight
				);

				if (areaRect.size.width >= limitAreaRect.size.width) {//表示範囲がスクリーン幅以下
					_moveArea.origin.x = limitAreaRect.getMidX();
					update = true;
				}
				else if (areaRect.getMinX() < limitAreaRect.getMinX()) {
					_moveArea.origin.x = limitAreaRect.getMinX() + areaRect.size.width * 0.5f;
					update = true;
				}
				else if (areaRect.getMaxX() > limitAreaRect.getMaxX()) {
					_moveArea.origin.x = limitAreaRect.getMaxX() - areaRect.size.width * 0.5f;
					update = true;
				}
				if (areaRect.size.height >= limitAreaRect.size.height) {//表示範囲がスクリーン高さ以下
					_moveArea.origin.y = limitAreaRect.getMidY();
					update = true;
				}
				else if (areaRect.getMinY() < limitAreaRect.getMinY()) {
					_moveArea.origin.y = limitAreaRect.getMinY() + areaRect.size.height * 0.5f;
					update = true;
				}
				else if (areaRect.getMaxY() > limitAreaRect.getMaxY()) {
					_moveArea.origin.y = limitAreaRect.getMaxY() - areaRect.size.height * 0.5f;
					update = true;
				}
			}
			if (update) {
				moveArea = _moveArea;
				this->setMoveArea(moveArea);
				cameraPoint = cocos2d::Vec2(moveArea.origin.x + moveArea.size.width * 0.5f, moveArea.origin.y + moveArea.size.height * 0.5f);
			}
		}
	}

	return cameraPoint;
}

cocos2d::Vec2 Camera::updateCameraPointAutoScroll(cocos2d::Vec2 pos, cocos2d::Vec2 scale, bool& bUpdate)
{
	auto sceneData = this->getSceneData();
	auto direction = agtk::GetDirectionFromDegrees(sceneData->getScreenAutoScrollDirection());
	auto speed = sceneData->getScreenAutoScrollSpeed();
	auto cameraSpeed = (direction * speed);

	float sceneWidth = this->getSceneSize().width;
	float sceneHeight = this->getSceneSize().height;
	float screenWidth = this->getScreenSize().width * scale.x;
	float screenHeight = this->getScreenSize().height * scale.y;
	auto limitAreaRect = this->getLimitAreaRect();
	auto halfScreenSize = cocos2d::Vec2(_halfScreenSize.x * scale.x, _halfScreenSize.y * scale.y);

	auto keys = this->getSceneData()->getLayerList()->allKeys();
	cocos2d::Ref *ref;
	float camSpeedX = this->getSceneData()->getBgMoveSpeedX();
	float camSpeedY = this->getSceneData()->getBgMoveSpeedY();
	CCARRAY_FOREACH(keys, ref) {
		auto layerId = dynamic_cast<cocos2d::Integer *>(ref)->getValue();
		float moveSpeedX = this->getSceneData()->getLayerMoveSpeedX(layerId);
		if (camSpeedX < moveSpeedX) {
			camSpeedX = moveSpeedX;
		}
		float moveSpeedY = this->getSceneData()->getLayerMoveSpeedY(layerId);
		if (camSpeedY < moveSpeedY) {
			camSpeedY = moveSpeedY;
		}
	}
	//スピードは100基準。
	if (camSpeedX > 100.0f) camSpeedX = 100.0f;
	if (camSpeedY > 100.0f) camSpeedY = 100.0f;

	std::function<float(float, float, float, float)> calc = [&](float speed, float velocity, float value, float half) {
		float m = (velocity == 0.0f) ? 0.0f : (value - half) / velocity;
		return value + m * (speed - velocity);
	};

	sceneWidth = calc(cameraSpeed.x, cameraSpeed.x * camSpeedX * 0.01f, sceneWidth, halfScreenSize.x);
	sceneHeight = calc(cameraSpeed.y, cameraSpeed.y * camSpeedY * 0.01f, sceneHeight, halfScreenSize.y);

	// カメラの表示制限矩形作成
	// note: moveArea は左下から右上への矩形なのでカメラ表示制限矩形も合わせる
	//カメラ表示領域
	cocos2d::Rect areaRect(0, 0, sceneWidth, sceneHeight);
	if (!this->isDisableLimitCamera()) {//カメラの表示範囲制限の無効チェック
		float limitAreaRectX = calc(cameraSpeed.x, cameraSpeed.x * camSpeedX * 0.01f, limitAreaRect.origin.x, 0);
		float limitAreaRectY = calc(cameraSpeed.y, cameraSpeed.y * camSpeedY * 0.01f, sceneHeight - limitAreaRect.getMaxY(), 0);
		areaRect.origin = cocos2d::Vec2(limitAreaRectX, limitAreaRectY);
		float limitAreaRectWidth = calc(cameraSpeed.x, cameraSpeed.x * camSpeedX * 0.01f, limitAreaRect.size.width, halfScreenSize.x);
		float limitAreaRectHeight = calc(cameraSpeed.y, cameraSpeed.y * camSpeedY * 0.01f, limitAreaRect.size.height, halfScreenSize.y);
		areaRect.size = cocos2d::Size(
			limitAreaRectWidth < sceneWidth ? limitAreaRectWidth : sceneWidth,
			limitAreaRectHeight < sceneHeight ? limitAreaRectHeight : sceneHeight
		);
	}

	float cameraSpeedX = (camSpeedX == 0.0f) ? 0.0f : 100.0f / camSpeedX;
	float cameraSpeedY = (camSpeedY == 0.0f) ? 0.0f : 100.0f / camSpeedY;
	float halfScreenSizeX = halfScreenSize.x * cameraSpeedX;
	float halfScreenSizeY = halfScreenSize.y * cameraSpeedY;
	pos += cameraSpeed;
	//x
	if (pos.x - halfScreenSizeX < areaRect.origin.x) {
		pos.x = halfScreenSizeX;
	}
	if (pos.x + halfScreenSizeX > areaRect.origin.x + areaRect.size.width) {
		pos.x = (areaRect.origin.x + areaRect.size.width) - halfScreenSizeX;
	}
	//y
	if (pos.y - halfScreenSizeY < areaRect.origin.y) {
		pos.y = halfScreenSizeY;
	}
	if (pos.y + halfScreenSizeY >= areaRect.origin.y + areaRect.size.height) {
		pos.y = (areaRect.origin.y + areaRect.size.height) - halfScreenSizeY;
	}

	bUpdate = true;
	return pos;
}

void Camera::updateLayerPosition(cocos2d::Vec2 cameraPoint, float duration)
{
	auto layerPosition = getLayerPosition();
	auto scale = this->getScale();

	cocos2d::Vec2 lpos(
		-cameraPoint.x + _halfScreenSize.x * scale.x,
		-cameraPoint.y + _halfScreenSize.y * scale.y
	);

	layerPosition->setValue(lpos, duration);
}

void Camera::updateCameraScale(float dt)
{
	auto cameraScale = this->getCameraScale();
	if (cameraScale->getState() == agtk::EventTimer::kStateIdle) {
		_isZooming = false;
	}
	cameraScale->update(dt);
}

#ifdef FIX_ACT2_4471
void Camera::updateCommandScale(float dt)
{
	auto commandScale = this->getCommandScale();
	if (commandScale->getState() == agtk::EventTimer::kStateIdle) {
		//_isZooming = false;
		//todo need _isCommandZooming???
	}
	commandScale->update(dt);
}
#endif

void Camera::update(float dt)
{
	auto layer = this->getLayer();
	if (layer == nullptr) {
		return;
	}

	auto scene = Director::getInstance()->getRunningScene();
	auto defaultCamera = scene->getDefaultCamera();
	if (defaultCamera && this->getCamera() == nullptr) {
		// カメラにアンカーポイントを有効化させるため contentSize に screenSize を設定する
		// 初期アンカーポイントは左下
		defaultCamera->setContentSize(getScreenSize());
		defaultCamera->setAnchorPoint(Vec2::ANCHOR_BOTTOM_LEFT);
		this->setCamera(defaultCamera);
	}
	if (this->getCamera() == nullptr) {
		return;
	}
	auto camera = this->getCamera();

	cocos2d::Size screenSize = this->getScreenSize();

	float sceneWidth = this->getSceneSize().width;
	float sceneHeight = this->getSceneSize().height;

	_oldPosition = _position;

	auto layerPosition = this->getLayerPosition();
	auto cameraPosition = this->getCameraPosition();
	auto cameraPosition2 = this->getCameraPosition2();
	auto cameraRotationX = this->getCameraRotationX();
	auto cameraRotationY = this->getCameraRotationY();
	auto cameraRotationZ = this->getCameraRotationZ();

	cocos2d::Vec2 pos = Vec2::ZERO;

	// 追従対象に応じて処理を変える
	switch (_targetType) {
		// 追従なし
		case CameraTargetType::kCameraTargetNone: {
			pos = _targetPosition;
		} break;

		// オブジェクトを追従
		case CameraTargetType::kCameraTargetObject: {

			auto targetObject = this->getTargetObject();

			// オブジェクトが存在する場合
			if (targetObject != nullptr) {
				pos = targetObject->getDispPosition();
				_targetObjectPosition = pos;
			}
			// オブジェクトが存在しない場合
			else {
				// これ以上の処理を行わない
				//return;//sakihama-h, 2018.2.16 前回までのオブジェクト位置を継続する。
				pos = _targetObjectPosition;
			}
		} break;
	}
	pos.y = sceneHeight - pos.y;

	//camera scale
	this->updateCameraScale(dt);
#ifdef FIX_ACT2_4471
	this->updateCommandScale(dt);
#endif

	auto sceneData = this->getSceneData();
	auto scale = this->getScale();
	bool bUpdate = false;
	bool bAutoScroll = isAutoScroll();
	cocos2d::Vec2 cameraPoint;
	cocos2d::Vec2 cameraPoint2;

	// オートスクロール判定
	if (bAutoScroll) {
		//画面の自動スクロール
		cameraPoint2 = updateCameraPointAutoScroll(this->getPosition2(), cocos2d::Vec2::ONE, bUpdate);
		cameraPoint = updateCameraPointAutoScroll(this->getPosition(), scale, bUpdate);

		// 追従なし
		if (_targetType == CameraTargetType::kCameraTargetNone) {
			_targetPosition = cameraPoint;
			_targetPosition.y = sceneHeight - _targetPosition.y;
			this->setIgnoreCorrectionPos(false);
		}
	}
	else {
		//キャラ移動追従。　
		cameraPoint2 = updateCameraPoint(pos, cocos2d::Vec2::ONE, bUpdate);
		cameraPoint = updateCameraPoint(pos, scale, bUpdate);
	}

	//ターゲットを設定した時に、カメラ位置の内部更新は無しにする。
	if (_setTargetObjectFlag) {
		if (bAutoScroll) {
			bUpdate = false;
		}
		_setTargetObjectFlag = false;
	}

	if (bUpdate) {
		// カメラズーム動作中はカメラ移動の補完を行わないようにする
		float moveDuration = _isZooming ? 0.0f : _moveDuration;
		//移動範囲
		updateLayerPosition(cameraPoint, moveDuration);
		cameraPosition->setValue(cameraPoint, moveDuration);
		cameraPosition2->setValue(cameraPoint2, moveDuration);
	}

	//layer position
	layerPosition->update(dt);
	auto layerPos = layerPosition->getValue();
	//scene shake
	if (_scene) {
		layerPos += _scene->getShake()->getMoveXY();
	}

	// オブジェクトを追従
	float moveSpeedTargetObjectRateX = 1.0f;
	float moveSpeedTargetObjectRateY = 1.0f;
	if (_targetType == CameraTargetType::kCameraTargetObject && this->getTargetObject()) {
		auto targetObject = this->getTargetObject();
		moveSpeedTargetObjectRateX = this->getSceneData()->getLayerMoveSpeedX(targetObject->getLayerId()) * 0.01f;
		moveSpeedTargetObjectRateY = this->getSceneData()->getLayerMoveSpeedY(targetObject->getLayerId()) * 0.01f;
	}

	// レイヤー毎に移動量を設定
	float moveSpeedX = this->getSceneData()->getBgMoveSpeedX();
	float moveSpeedY = this->getSceneData()->getBgMoveSpeedY();
	if(moveSpeedX != 100 || moveSpeedY != 100){
		auto ratioX = (moveSpeedTargetObjectRateX != 0) ? (moveSpeedX / moveSpeedTargetObjectRateX) : (moveSpeedX == 0) ? 100 : (moveSpeedX / 0.0000001f);
		auto ratioY = (moveSpeedTargetObjectRateY != 0) ? (moveSpeedY / moveSpeedTargetObjectRateY) : (moveSpeedY == 0) ? 100 : (moveSpeedY / 0.0000001f);
		float moveSpeedRateX = ratioX * 0.01f - 1.0f;
		float moveSpeedRateY = ratioY * 0.01f - 1.0f;

		// ACT2-5207 シーンレイヤーのスクロール速度が100%でないときに、描画座標に小数点が出てきて、テクスチャーのサンプリングに影響が出るため、整数化。
		auto layerPosition = Vec2(
			std::floorf((layerPos.x + _initialOffsetPosition.x) * moveSpeedRateX + 0.5f),
			std::floorf((layerPos.y + _initialOffsetPosition.y) * moveSpeedRateY + 0.5f)
		);
		cocos2d::Vec2 sceneLayerPos;
		if (moveSpeedRateX != 0.0f) {
			sceneLayerPos.x = layerPosition.x;
		}
		else {
			//浮動小数点切り捨て
			sceneLayerPos.x = (int)layerPosition.x;
		}
		if (moveSpeedRateY != 0.0f) {
			sceneLayerPos.y = layerPosition.y;
		}
		else {
			//浮動小数点切り捨て
			sceneLayerPos.y = (int)layerPosition.y;
		}

		auto sceneBackground = _scene->getSceneBackground();
		sceneBackground->setMovePosition(sceneLayerPos);
	}
	auto keys = this->getSceneData()->getLayerList()->allKeys();
	cocos2d::Ref *ref;
	CCARRAY_FOREACH(keys, ref) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
		auto layerId = static_cast<cocos2d::Integer *>(ref)->getValue();
#else
		auto layerId = dynamic_cast<cocos2d::Integer *>(ref)->getValue();
#endif
		// moveSpeed は割合指定に変更
		float moveSpeedX = this->getSceneData()->getLayerMoveSpeedX(layerId);
		float moveSpeedY = this->getSceneData()->getLayerMoveSpeedY(layerId);
		auto ratioX = (moveSpeedTargetObjectRateX != 0) ? (moveSpeedX / moveSpeedTargetObjectRateX) : (moveSpeedX == 0) ? 100 : (moveSpeedX / 0.0000001f);
		auto ratioY = (moveSpeedTargetObjectRateY != 0) ? (moveSpeedY / moveSpeedTargetObjectRateY) : (moveSpeedY == 0) ? 100 : (moveSpeedY / 0.0000001f);
		float moveSpeedRateX = ratioX * 0.01f - 1.0f;
		float moveSpeedRateY = ratioY * 0.01f - 1.0f;

#if 1	// ACT2-5207 シーンレイヤーのスクロール速度が100%でないときに、描画座標に小数点が出てきて、テクスチャーのサンプリングに影響が出るため、整数化。
		auto layerPosition = Vec2(
			std::floorf((layerPos.x + _initialOffsetPosition.x) * moveSpeedRateX + 0.5f),
			std::floorf((layerPos.y + _initialOffsetPosition.y) * moveSpeedRateY + 0.5f)
		);
#else
		auto layerPosition = Vec2(((layerPos.x + _initialOffsetPosition.x) * moveSpeedRate), ((layerPos.y + _initialOffsetPosition.y) * moveSpeedRate));
#endif
		_sceneLayerPositionList[layerId] = layerPosition;
		cocos2d::Vec2 sceneLayerPos;
		if(moveSpeedRateX != 0.0f) {
			sceneLayerPos.x = layerPosition.x;
		}
		else {
			//浮動小数点切り捨て
			sceneLayerPos.x = (int)layerPosition.x;
		}
		if (moveSpeedRateY != 0.0f) {
			sceneLayerPos.y = layerPosition.y;
		}
		else {
			//浮動小数点切り捨て
			sceneLayerPos.y = (int)layerPosition.y;
		}

		auto layer = dynamic_cast<agtk::SceneLayer *>(this->getLayerList()->objectForKey(layerId));
		if (layer == nullptr) continue;
		layer->setPosition(sceneLayerPos.x, sceneLayerPos.y);
	}

	//camera position
	cameraPosition->update(dt);
	cameraPosition2->update(dt);

	// camera scale
	camera->setScale(scale.x, scale.y);

	// ※カメラのアンカーポイントが有効になったのでアンカーポイントに合わせて位置を調整する。浮動小数点切り捨て。
	auto camx = (float)(layerPos.x *-1.0f + camera->getAnchorPoint().x * camera->getContentSize().width * scale.x);
	auto camy = (float)(layerPos.y *-1.0f + camera->getAnchorPoint().y * camera->getContentSize().height * scale.y);
	camera->setPositionX((int)camx);
	camera->setPositionY((int)camy);

	_position = cocos2d::Vec2(camx, camy);

	// camera rotation
	cameraRotationX->update(dt);
	cameraRotationY->update(dt);
	cameraRotationZ->update(dt);
	auto cameraRotX = cameraRotationX->getValue();
	auto cameraRotY = cameraRotationY->getValue();
	auto cameraRotZ = cameraRotationZ->getValue();
	camera->setRotation3D(Vec3(cameraRotX, cameraRotY, cameraRotZ));

	auto topMostCamera = this->getTopMostCamera();
	if (topMostCamera) {
		topMostCamera->setScale(camera->getScaleX(), camera->getScaleY());
		topMostCamera->setPositionX(camera->getPositionX());
		topMostCamera->setPositionY(camera->getPositionY());
		topMostCamera->setRotation3D(camera->getRotation3D());
	}
#ifdef USE_PREVIEW
//	this->showDebugMoveArea(true);
#endif
	_counter++;
}

cocos2d::Vec2 Camera::getCameraCenterPos()
{
	auto centerPos = _position;
	auto camera = getCamera();

	if (nullptr != camera) {
		centerPos += Vec2((0.5f - camera->getAnchorPoint().x) * camera->getContentSize().width * camera->getScaleX(), (0.5f - camera->getAnchorPoint().y) * camera->getContentSize().height * camera->getScaleY());
	}

	return centerPos;
}

cocos2d::Vec2 Camera::addPosition(cocos2d::Vec2 pos, float duration)
{
	return Camera::addPosition(pos.x, pos.y, duration);
}

cocos2d::Vec2 Camera::addPosition(float x, float y, float duration)
{
	auto value = this->getPosition();
	this->setPosition(value.x + x, value.y + y, duration);
	return value;
}

cocos2d::Vec2 Camera::setPosition(cocos2d::Vec2 pos, float duration)
{
	return Camera::setPosition(pos.x, pos.y, duration);
}

cocos2d::Vec2 Camera::setPosition(float x, float y, float duration)
{
	auto projectData = GameManager::getInstance()->getProjectData();
	auto sceneData = this->getSceneData();

	cocos2d::Vec2 cameraPoint = cocos2d::Vec2(x, y);
	this->setMoveArea(this->calcMoveArea(cameraPoint, this->getScale()));

	auto cpos = this->getCameraPosition()->setValue(cameraPoint, duration);
	cocos2d::Vec2 lpos = -cameraPoint + _halfScreenSize;
	this->getLayerPosition()->setValue(lpos, duration);

	return cpos;
}

cocos2d::Vec2 Camera::getPosition() const
{
	return this->getCameraPosition()->getValue();
}

cocos2d::Vec2 Camera::getPosition2() const
{
	return this->getCameraPosition2()->getValue();
}

cocos2d::Vec2 Camera::getPositionForZoom()
{
	auto scale = this->getScale();
	if (scale.x < 1.0f && scale.y < 1.0f) {
		return this->getCameraPosition2()->getValue();
	}
	return this->getCameraPosition()->getValue();
}

void Camera::setZoom(cocos2d::Vec2 target, cocos2d::Vec2 scale, float duration)
{
	// ズーム中フラグON
	this->_isZooming = true;

	cocos2d::Vec2 lpos;
	lpos.x = -target.x + _halfScreenSize.x * getDisplaySizeRatio().x;
	lpos.y = -target.y + _halfScreenSize.y * getDisplaySizeRatio().y;

	this->getLayerPosition()->setValue(lpos, duration);

	// カメラの最終スケール値を設定
	auto cameraScale = this->getCameraScale();
	cameraScale->setValue(cocos2d::Vec2(scale.x, scale.y), duration);
}

#ifdef FIX_ACT2_4471
void Camera::setCommandZoom(cocos2d::Vec2 scale, float duration)
{
	// 実行アクションでカメラの表示領域（の拡縮）を設定。
	auto commandScale = this->getCommandScale();
	commandScale->setValue(cocos2d::Vec2(scale.x, scale.y), duration);
}
#endif

cocos2d::Vec2 Camera::getZoom()
{
#ifdef FIX_ACT2_4471
	auto cameraScale = this->getCameraScale()->getValue();
	auto commandScale = this->getCommandScale()->getValue();
	return cocos2d::Vec2(cameraScale.x * commandScale.x, cameraScale.y * commandScale.y);
#else
	return this->getCameraScale()->getValue();
#endif
}

cocos2d::Vec2 Camera::getScale()
{
#ifdef FIX_ACT2_4471
	auto cameraScale = this->getCameraScale()->getValue();
	auto commandScale = this->getCommandScale()->getValue();
	auto scale = cocos2d::Vec2(cameraScale.x * commandScale.x, cameraScale.y * commandScale.y);
#else
	auto scale =  this->getCameraScale()->getValue();
#endif
	scale.x *= getDisplaySizeRatio().x;
	scale.y *= getDisplaySizeRatio().y;
	return scale;
}

void Camera::setDisplaySize(float width, float height)
{
	_displaySize.width = width;
	_displaySize.height = height;
	_displaySizeRatio.x = width / getScreenSize().width;
	_displaySizeRatio.y = height / getScreenSize().height;
}

cocos2d::Vec2 Camera::getPositionCameraFromScene(cocos2d::Vec2 pos)
{
	auto cameraCenterPosition = this->getCameraCenterPos();
	return cocos2d::Vec2(pos.x, _sceneSize.height - pos.y) - cameraCenterPosition;
}

cocos2d::Vec2 Camera::getPositionSceneFromCamera(cocos2d::Vec2 pos)
{
	auto cameraCenterPosition = this->getCameraCenterPos();
	return cocos2d::Vec2(pos.x + cameraCenterPosition.x, -1.0f * ((pos.y - _sceneSize.height) + cameraCenterPosition.y));
}

bool Camera::isPositionScreenWithinCamera(cocos2d::Vec2 pos)
{
	return isPositionScreenWithinCamera(pos, cocos2d::Vec2(0, 0));
}

bool Camera::isPositionScreenWithinCamera(cocos2d::Vec2 pos, cocos2d::Vec2 screenSizeOffset)
{
	auto camera = getCamera();
	cocos2d::Vec2 cpos = this->getPositionCameraFromScene(pos);
	cocos2d::Vec2 halfScreenSize = cocos2d::Vec2(_halfScreenSize.x * camera->getScaleX(), _halfScreenSize.y * camera->getScaleY());
	if ((-(halfScreenSize.x + screenSizeOffset.x) <= cpos.x && cpos.x < (halfScreenSize.x + screenSizeOffset.x))
		&& (-(halfScreenSize.y + screenSizeOffset.y) <= cpos.y && cpos.y < (halfScreenSize.y + screenSizeOffset.y))) {
		return true;
	}
	return false;
}

bool Camera::isPositionScreenWithinCamera(cocos2d::Rect rect)
{
	return isPositionScreenWithinCamera(rect, cocos2d::Vec2(0, 0));
}

bool Camera::isPositionScreenWithinCamera(cocos2d::Rect rect, cocos2d::Vec2 screenSizeOffset)
{
	if (this->isPositionScreenWithinCamera(cocos2d::Vec2(rect.getMinX(), rect.getMinY()), screenSizeOffset)) {
		return true;
	}
	if (this->isPositionScreenWithinCamera(cocos2d::Vec2(rect.getMaxX(), rect.getMinY()), screenSizeOffset)) {
		return true;
	}
	if (this->isPositionScreenWithinCamera(cocos2d::Vec2(rect.getMinX(), rect.getMaxY()), screenSizeOffset)) {
		return true;
	}
	if (this->isPositionScreenWithinCamera(cocos2d::Vec2(rect.getMaxX(), rect.getMaxY()), screenSizeOffset)) {
		return true;
	}
	return false;
}

void Camera::showDebugMoveArea(bool bShow)
{
	auto layer = this->getLayer();
	if (layer == nullptr) {
		return;
	}
#ifdef USE_PREVIEW
	auto sceneData = this->getSceneData();
	auto rect = this->getMoveArea();
	auto sp = this->getDebugMoveAreaSprite();
	if (sp == nullptr) {
		sp = Sprite::create();
		sp->setAnchorPoint(Vec2::ANCHOR_TOP_LEFT);
		sp->setTextureRect(cocos2d::Rect(cocos2d::Vec2::ZERO, rect.size));
		sp->setColor(Color3B::YELLOW);
		sp->setOpacity(100);
		layer->addChild(sp, BaseLayer::ZOrder::Debug);
		this->setDebugMoveAreaSprite(sp);
	}
	if (sp) {
		cocos2d::Vec2 p = agtk::Scene::getPositionCocos2dFromScene(rect.origin);
		sp->setPosition(p);
	}
	sp->setVisible(bShow);
#endif
}

/**
* クリッピング使用判定(表示領域・判定/回転が設定されている場合はクリッピングは使わない)
*/
bool Camera::isUseClipping()
{
	// 表示領域
	auto scale = getScale();
	if (scale.x != 1.0f || scale.y != 1.0f) {
		return false;
	}

	// 判定/回転
	auto rotationX = getCameraRotationX();
	auto rotationY = getCameraRotationY();
	auto rotationZ = getCameraRotationZ();
	if ((int)rotationX->getValue() % 360 != 0 ||
		(int)rotationY->getValue() % 360 != 0 ||
		(int)rotationZ->getValue() % 360 != 0) {
		return false;
	}

	return true;
}

/**
* 自動スクロール判定
*/
bool Camera::isAutoScroll()
{
	auto sceneData = this->getSceneData();
	bool bAutoScroll = false;

	// オートスクロール判定
	if (sceneData->getScreenAutoScroll() && sceneData->getScreenAutoScrollSpeed() > 0.0f) {
		auto projectPlayData = GameManager::getInstance()->getPlayData();

		auto objectId = sceneData->getScreenAutoScrollSwitchObjectId();
		auto switchId = sceneData->getScreenAutoScrollSwitchId();
		auto switchQualifierId = sceneData->getScreenAutoScrollSwitchQualifierId();

		// 未設定の場合
		if (objectId < agtk::data::PlayData::COMMON_PLAY_DATA_ID) {
			// 常に有効化
			bAutoScroll = true;
		}
		// プロジェクト共通の場合共通の場合
		else if (objectId == agtk::data::PlayData::COMMON_PLAY_DATA_ID) {
			auto switchData = projectPlayData->getSwitchData(agtk::data::PlayData::COMMON_PLAY_DATA_ID, switchId);
			if (switchData) {
				bAutoScroll = switchData->getValue();
			}
		}
		// オブジェクト固有の場合
		else if (objectId > agtk::data::PlayData::COMMON_PLAY_DATA_ID) {
			// 指定オブジェクトリストIDのオブジェクトリストを取得する取得する
			auto objectList = GameManager::getInstance()->getTargetObjectListByObjectId(objectId);

			// 単体 or 指定インスタンスの場合
			if (switchQualifierId == agtk::data::EnumQualifierType::kQualifierSingle || switchQualifierId > 0) {
				int instanceId = switchQualifierId;// 指定インスタンスの場合は switchQualifierId にインスタンスIDが入っている

												   // 単体の場合
				if (switchQualifierId == agtk::data::EnumQualifierType::kQualifierSingle) {
					auto p = projectPlayData->getVariableData(objectId, agtk::data::kObjectSystemVariableSingleInstanceID);
					if (p) {
						instanceId = (int)p->getValue();
					}
				}

				cocos2d::Ref *ref2 = nullptr;
				CCARRAY_FOREACH(objectList, ref2) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
					auto object = static_cast<agtk::Object*>(ref2);
#else
					auto object = dynamic_cast<agtk::Object*>(ref2);
#endif
					if (object->getInstanceId() == instanceId) {
						auto switchData = object->getPlayObjectData()->getSwitchData(switchId);
						if (switchData) {
							bAutoScroll = switchData->getValue();
						}
						break;
					}
				}
			}
		}
	}

	return bAutoScroll;
}

NS_AGTK_END
