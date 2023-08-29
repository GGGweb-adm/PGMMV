#include "Bullet.h"
#include "Data/ObjectData.h"
#include "Lib/Collision.h"
#include "Manager/GameManager.h"
#include "Manager/PrimitiveManager.h"
#include "Manager/BulletManager.h"

NS_AGTK_BEGIN

Bullet *(*Bullet::create)(agtk::Object *object, agtk::data::ObjectFireBulletSettingData *fireBulletSettingData, int connectId, int count, int maxCount) = Bullet::_create;

//-------------------------------------------------------------------------------------------------------------------
//function

cocos2d::Vec2 GetDirection(cocos2d::Vec2 next, cocos2d::Vec2 prev, float performance)
{
	float dz = prev.x * next.y - prev.y * next.x;
	float angle2 = atan2f(dz + MATH_FLOAT_SMALL, cocos2d::Vec2::dot(prev, next));
	float degree2 = CC_RADIANS_TO_DEGREES(angle2) * performance * 0.01f;
	if (degree2 < 0) degree2 += 360;
	auto vv = prev.rotateByAngle(cocos2d::Vec2::ZERO, CC_DEGREES_TO_RADIANS(degree2));
	return vv;
}

//-------------------------------------------------------------------------------------------------------------------
// 弾の飛び方(初期動作)：基底
//-------------------------------------------------------------------------------------------------------------------
InitialBulletLocus::InitialBulletLocus()
{
	_objectFireBulletSettingData = nullptr;
	_dispDirection = -1;
	_direction = cocos2d::Vec2::ZERO;
	_targetObject = nullptr;
}

InitialBulletLocus::~InitialBulletLocus()
{
	CC_SAFE_RELEASE_NULL(_objectFireBulletSettingData);
	CC_SAFE_RELEASE_NULL(_targetObject);
}

InitialBulletLocus *InitialBulletLocus::create(data::ObjectFireBulletSettingData *objectFireBulletSettingData)
{
	switch (objectFireBulletSettingData->getInitialBulletLocus()) {
	case agtk::data::ObjectFireBulletSettingData::kInitialBulletLocusFree:
		return InitialBulletLocusFree::create(objectFireBulletSettingData);
	case agtk::data::ObjectFireBulletSettingData::kInitialBulletLocusFireObjectDirection:
		return InitialBulletLocusFireObjectDirection::create(objectFireBulletSettingData);
	case agtk::data::ObjectFireBulletSettingData::kInitialBulletLocusTowardObject:
		return InitialBulletLocusTowardObject::create(objectFireBulletSettingData);
	case agtk::data::ObjectFireBulletSettingData::kInitialBulletLocusOneDirection:
		return InitialBulletLocusOneDirection::create(objectFireBulletSettingData);
	default:CC_ASSERT(0);
	}
	return nullptr;
}

bool InitialBulletLocus::init(data::ObjectFireBulletSettingData *objectFireBulletSettingData)
{
	this->setObjectFireBulletSettingData(objectFireBulletSettingData);
	return true;
}

bool InitialBulletLocus::initial(agtk::Object *object, int connectId)
{
	cocos2d::Vec2 position;
	if (connectId < 0) {
		position = object->getCenterPosition();
	}
	else {
		agtk::Vertex4 vertex4;
		if (!object->getTimeline(agtk::data::TimelineInfoData::kTimelineConnection, connectId, vertex4)) {
			return false;
		}
		position = agtk::Scene::getPositionSceneFromCocos2d(vertex4[0]);
	}
	this->setPosition(position);
	return true;
}

//-------------------------------------------------------------------------------------------------------------------
// 弾の飛び方(初期動作)：飛び方を指定しない
//-------------------------------------------------------------------------------------------------------------------
bool InitialBulletLocusFree::initial(agtk::Object *parent, int connectId, agtk::Bullet *bullet, int count, int maxCount)
{
	if (InitialBulletLocus::initial(parent, connectId) == false) {
		return false;
	}
	auto data = this->getObjectFireBulletSettingData();

	//移動方向 & 表示方向
	int dispDirection = -1;
	float angle = 0;

	//! ※弾として設定したオブジェクトのアクションプログラムで動作します

	// 発射元のオブジェクトに表示方向を合わせる場合
	if (data->getSetActionDirectionToFireObjectDirection()) {

		dispDirection = parent->getDispDirection();

		if (dispDirection == 0) {
			dispDirection = parent->calcDispDirection();
		}

		this->setDispDirection(dispDirection);

		// 親オブジェクトの表示方向が回転で自動生成されている場合
		if (parent->isAutoGeneration()) {
			angle = agtk::GetDegreeFromVector(parent->getObjectMovement()->getDirection());
		}
		else {
			angle = agtk::GetDegreeFromVector(agtk::GetDirectionFromMoveDirectionId(dispDirection));
		}
	}
	// 発射元のオブジェクトに表示方向を合わせない場合
	else {
		dispDirection = bullet->calcDispDirection();
		angle = agtk::GetDegreeFromVector(agtk::GetDirectionFromMoveDirectionId(dispDirection));
	}

	//発射範囲を広げる & 動きを指定
	auto objectFireBulletSettingData = this->getObjectFireBulletSettingData();
	cocos2d::Vec2 direction = BulletLocus::calcDirection(objectFireBulletSettingData, angle, count, maxCount);

	this->setDirection(direction);
	this->setDispDirection(dispDirection);

	return true;
}

//-------------------------------------------------------------------------------------------------------------------
// 弾の飛び方(初期動作)：発射元オブジェクトの表示方向に飛ぶ
//-------------------------------------------------------------------------------------------------------------------
bool InitialBulletLocusFireObjectDirection::initial(agtk::Object *parent, int connectId, agtk::Bullet *bullet, int count, int maxCount)
{
	if (InitialBulletLocus::initial(parent, connectId) == false) {
		return false;
	}

	int dispDirection = -1;
	float angle = 0.0f;
	auto objectData = parent->getObjectData();
	if (objectData->getMoveType() == agtk::data::ObjectData::kMoveNormal) {//基本移動
		dispDirection = parent->getDispDirection();
		if (dispDirection == 0) {//表示方向の指定がない場合
			dispDirection = parent->calcDispDirection();
		}
		// 親オブジェクトの表示方向が回転で自動生成されている場合
		if (parent->isAutoGeneration()) {
			angle = agtk::GetDegreeFromVector(parent->getObjectMovement()->getDirection());
		}
		else {
			cocos2d::Vec2 v = agtk::GetDirectionFromMoveDirectionId(dispDirection);
			angle = agtk::GetDegreeFromVector(v);
		}
	}
	else {//戦車移動、車移動
		dispDirection = parent->getDispDirection();
		auto movement = parent->getObjectMovement();
		angle = agtk::GetDegreeFromVector(movement->getDirection());
	}

	//発射範囲を広げる & 動きを指定
	auto objectFireBulletSettingData = this->getObjectFireBulletSettingData();
	cocos2d::Vec2 direction = BulletLocus::calcDirection(objectFireBulletSettingData, angle, count, maxCount);
	this->setDispDirection(dispDirection);
	this->setDirection(direction);

	return true;
}

//-------------------------------------------------------------------------------------------------------------------
// 弾の飛び方(初期動作)：画面内の発射元以外のオブジェクト方向に飛ぶ
//-------------------------------------------------------------------------------------------------------------------
bool InitialBulletLocusTowardObject::initial(agtk::Object *object, int connectId, agtk::Bullet *bullet, int count, int maxCount)
{
	if (InitialBulletLocus::initial(object, connectId) == false) {
		return false;
	}
	int layerId = object->getLayerId();
	auto scene = GameManager::getInstance()->getCurrentScene();
	auto camera = scene->getCamera();
	auto visibleSize = scene->getCamera()->getScreenSize();
	auto sceneLayer = scene->getSceneLayer(layerId);
#ifdef USE_SAR_OPTIMIZE_1
	auto objectList = scene->getObjectAllReference(sceneLayer->getType());
#else
	auto objectList = scene->getObjectAll(sceneLayer->getType());
#endif
	float maxLength = cocos2d::Vec2(cocos2d::Vec2::ZERO).getDistance(cocos2d::Vec2(visibleSize.width, visibleSize.height));
	agtk::Object *selectObject = nullptr;
	auto objectFireBulletSettingData = this->getObjectFireBulletSettingData();
	for (int i = 0; i < objectList->count(); i++) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
		auto obj = static_cast<agtk::Object *>(objectList->getObjectAtIndex(i));
#else
		auto obj = dynamic_cast<agtk::Object *>(objectList->getObjectAtIndex(i));
#endif
		bool bSelected = false;
		CC_ASSERT(obj);
		auto objData = obj->getObjectData();
		if (objData->getGroupBit() & objectFireBulletSettingData->getTowardObjectGroupBit()) {
			bSelected = true;
		}
		if (bSelected == false) {
			continue;
		}
		if (obj->isBullet()){
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
			auto const bul = static_cast<agtk::Bullet*>(obj);
#else
			auto const bul = dynamic_cast<agtk::Bullet*>(obj);
#endif
			if (bul->getBulletLocus()->getParentObject() == object) {
				continue;
			}
		}
		//カメラ内チェック。
		cocos2d::Rect rect(object->getPosition(), object->getContentSize());
		if (camera->isPositionScreenWithinCamera(rect) == false) {
			continue;
		}
		float len = obj->getCenterPosition().getDistance(object->getCenterPosition());
		if (len <= maxLength) {
			selectObject = obj;
			maxLength = len;
		}
	}
	float angle = 0.0f;
	if (selectObject != nullptr) {
		auto selectPos = selectObject->getCenterPosition();
		auto objectPos = this->getPosition();
		selectPos += Object::getSceneLayerScrollDiff(object->getSceneLayer(), selectObject->getSceneLayer());
		cocos2d::Vec2 v = selectPos - objectPos;
		v.y *= -1.0f;//※座標が左上が原点になっていて、ｙ座標はした方向が正になります。座標をcocos2dにしないと。
		angle = agtk::GetDegreeFromVector(v);
		this->setTargetObject(selectObject);
	}

	//発射範囲を広げる & 動きを指定
	cocos2d::Vec2 direction = BulletLocus::calcDirection(objectFireBulletSettingData, angle, count, maxCount);
	this->setDispDirection(agtk::GetMoveDirectionId(agtk::GetDegreeFromVector(direction)));
	this->setDirection(direction);

	return true;
}

//-------------------------------------------------------------------------------------------------------------------
// 弾の飛び方(初期動作)：一定方向に飛ぶ
//-------------------------------------------------------------------------------------------------------------------
bool InitialBulletLocusOneDirection::initial(agtk::Object *object, int connectId, agtk::Bullet *bullet, int count, int maxCount)
{
	if (InitialBulletLocus::initial(object, connectId) == false) {
		return false;
	}
	auto objectFireBulletSettingData = this->getObjectFireBulletSettingData();
	float angle = objectFireBulletSettingData->getOneDirectionAngle();

	//発射範囲を広げる & 動きを指定
	cocos2d::Vec2 direction = BulletLocus::calcDirection(objectFireBulletSettingData, angle, count, maxCount);
	this->setDispDirection(agtk::GetMoveDirectionId(agtk::GetDegreeFromVector(direction)));
	this->setDirection(direction);
	return true;
}

//-------------------------------------------------------------------------------------------------------------------
// 弾の飛び方(変化)：基底
//-------------------------------------------------------------------------------------------------------------------
NextBulletLocus::NextBulletLocus()
{
	_objectFireBulletSettingData = nullptr;
	_parent = nullptr;
	_bullet = nullptr;
	_direction = cocos2d::Vec2::ZERO;
	_directionOld = cocos2d::Vec2::ZERO;
	_state = kStateNone;
	_targetObject = nullptr;
	_connectId = -1;
}

NextBulletLocus::~NextBulletLocus()
{
	CC_SAFE_RELEASE_NULL(_objectFireBulletSettingData);
	CC_SAFE_RELEASE_NULL(_targetObject);
}

NextBulletLocus *NextBulletLocus::create(data::ObjectFireBulletSettingData *objectFireBulletSettingData)
{
	switch (objectFireBulletSettingData->getNextBulletLocus()) {
	case agtk::data::ObjectFireBulletSettingData::kNextBulletLocusFree:
		return NextBulletLocusFree::create(objectFireBulletSettingData);
	case agtk::data::ObjectFireBulletSettingData::kNextBulletLocusFollowLockedObject:
		return NextBulletLocusFollowLockedObject::create(objectFireBulletSettingData);
	case agtk::data::ObjectFireBulletSettingData::kNextBulletLocusFollowObjectInsideCamera:
		return NextBulletLocusFollowObjectInsideCamera::create(objectFireBulletSettingData);
	case agtk::data::ObjectFireBulletSettingData::kNextBulletLocusBoomerang:
		return NextBulletLocusBoomerang::create(objectFireBulletSettingData);
	default:CC_ASSERT(0);
	}
	return nullptr;
}

bool NextBulletLocus::init(data::ObjectFireBulletSettingData *objectFireBulletSettingData)
{
	this->setObjectFireBulletSettingData(objectFireBulletSettingData);
	return true;
}

void NextBulletLocus::initial(agtk::Object *parent, agtk::Bullet *bullet)
{
	_parent = parent;
	_bullet = bullet;
}

void NextBulletLocus::setTargetObject(agtk::Object *object)
{
	if (_targetObject != object) {
		CC_SAFE_RETAIN(object);
		CC_SAFE_RELEASE(_targetObject);
		_targetObject = object;
	}
}

void NextBulletLocus::setDirection(cocos2d::Vec2 vec, bool bInitialize)
{
	_directionOld = bInitialize ? vec : _direction;
	_direction = vec;
}

//-------------------------------------------------------------------------------------------------------------------
// 弾の飛び方(変化)：飛び方を指定しない
//-------------------------------------------------------------------------------------------------------------------
bool NextBulletLocusFree::init(data::ObjectFireBulletSettingData *objectFireBulletSettingData)
{
	if (!NextBulletLocus::init(objectFireBulletSettingData)) {
		return false;
	}
	return true;
}

void NextBulletLocusFree::start(int count, int maxCount)
{
	CC_ASSERT(_parent && _bullet);
	(void)count;
	(void)maxCount;
	this->setState(kStateStart);
}

void NextBulletLocusFree::end()
{
	this->setState(kStateEnd);
}

void NextBulletLocusFree::update(float delta)
{
	CC_ASSERT(_bullet);
	switch (this->getState()) {
	case kStateNone:{
		break; }
	case kStateStart: {
		this->setState(kStateAction);
		break; }
	case kStateAction:
		break;
	case kStateEnd:
		break;
	}

	// 子オブジェクトとして生成されていない場合、もしくは「子オブジェクトとして生成」かつ「親オブジェクトを追従しない」の場合。
	auto bulletObjectData = this->_bullet->getObjectData();
	if (!this->getObjectFireBulletSettingData()->getBulletChild()
	|| (this->getObjectFireBulletSettingData()->getBulletChild() && (this->_bullet->getObjectData()->getFollowType() == agtk::data::ObjectData::kFollowNone))) {
		//そのままの方向へ向けるように。
		auto movement = _bullet->getObjectMovement();
		this->setDirection(movement->getDirection());
	}
}

//-------------------------------------------------------------------------------------------------------------------
// 弾の飛び方(変化)：発射元がロックしているオブジェクトを追尾
//-------------------------------------------------------------------------------------------------------------------
NextBulletLocusFollowLockedObject::NextBulletLocusFollowLockedObject() : NextBulletLocus()
{
	_duration = 0.0f;
	_startDelay300 = 0;
	_startDelayDispersion300 = 0;
	_performance = 0.0f;
	_oldPosition = cocos2d::Vec2::ZERO;
	_count = 0;
	_maxCount = 1;
	_moved = false;
}

bool NextBulletLocusFollowLockedObject::init(data::ObjectFireBulletSettingData *objectFireBulletSettingData)
{
	if (!NextBulletLocus::init(objectFireBulletSettingData)) {
		return false;
	}
	this->setDuration(0.0f);
	this->setState(kStateNone);
	this->setStartDelay300(objectFireBulletSettingData->getFollowLockedObjectStartDelayStart300());
	this->setStartDelayDispersion300(AGTK_RANDOM(0, objectFireBulletSettingData->getFollowLockedObjectStartDelayEnd300() - objectFireBulletSettingData->getFollowLockedObjectStartDelayStart300() ));
	this->setPerformance(objectFireBulletSettingData->getFollowLockedObjectPerformance());
	return true;
}

bool NextBulletLocusFollowLockedObject::innerStart()
{
	int count = this->getCount();
	int maxCount = this->getMaxCount();
	auto parent = _parent;
	auto bullet = _bullet;

	auto scene = GameManager::getInstance()->getCurrentScene();
	auto sceneLayer = bullet->getSceneLayer();
	auto sceneSize = scene->getSceneSize();
// #AGTK-NX
#ifdef USE_SAR_OPTIMIZE_1
	auto objectList = scene->getObjectAllReference(sceneLayer->getType());
#else
	auto objectList = scene->getObjectAll(sceneLayer->getType());
#endif
	float maxLength = cocos2d::Vec2(cocos2d::Vec2::ZERO).getDistance(sceneSize);
	agtk::Object *selectObject = nullptr;
	for (int i = 0; i < objectList->count(); i++) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
		auto object = static_cast<agtk::Object *>(objectList->getObjectAtIndex(i));
#else
		auto object = dynamic_cast<agtk::Object *>(objectList->getObjectAtIndex(i));
#endif
		if (object == parent || object == bullet) {
			continue;
		}

		if (object->getPlayObjectData()->isLocked(parent->getInstanceId())) {
			float length = parent->getCenterPosition().getDistance(object->getCenterPosition());
			if (length < maxLength) {
				selectObject = object;
				maxLength = length;
			}
		}
	}
	float angle = 0.0f;
	if (selectObject) {
		cocos2d::Vec2 v = selectObject->getCenterPosition() - parent->getCenterPosition();
		v.y *= -1.0f;//※座標が左上が原点になっていて、ｙ座標はした方向が正になります。座標をcocos2dにしないと。
		angle = agtk::GetDegreeFromVector(v);
		this->setTargetObject(selectObject);

		//発射範囲を広げる & 動きを指定
		auto objectFireBulletSettingData = this->getObjectFireBulletSettingData();
		cocos2d::Vec2 direction = BulletLocus::calcDirection(objectFireBulletSettingData, angle, count, maxCount);
		this->setDirection(direction, true);
	}

	return this->getTargetObject() ? true : false;
}

void NextBulletLocusFollowLockedObject::start(int count, int maxCount)
{
	CC_ASSERT(_parent && _bullet);
	this->setCount(count);
	this->setMaxCount(maxCount);
	this->setDuration(0.0f);
	this->setState(kStateStart);
	auto bullet = _bullet;
	this->setOldPosition(bullet->getCenterPosition());
}

void NextBulletLocusFollowLockedObject::end()
{
	this->setState(kStateEnd);
}

void NextBulletLocusFollowLockedObject::update(float delta)
{
	auto movement = _bullet->getObjectMovement();
	auto bullet = _bullet;
	auto target = _targetObject;

	auto oldPosition = this->getOldPosition();
	cocos2d::Vec2 p = bullet->getCenterPosition() - this->getOldPosition();
	this->setOldPosition(bullet->getCenterPosition());
#if defined(AGTK_DEBUG)
	cocos2d::Color4F color = cocos2d::Color4F::YELLOW;
// #AGTK-NX
#if CC_TARGET_PLATFORM != CC_PLATFORM_NX
	showDebugLine(
		agtk::Scene::getPositionCocos2dFromScene(oldPosition),
		agtk::Scene::getPositionCocos2dFromScene(this->getOldPosition()),
		color, 0.2f
	);
#else
#endif
#endif
	_duration += delta;
	switch (this->getState()) {
	case kStateNone: {
		//そのままの方向へ向けるように。
		this->setDirection(movement->getDirection());
		break; }
	case kStateStart: {
		//そのままの方向へ向けるように。
		this->setDirection(movement->getDirection());
		if ((_duration * 300) >= this->getStartDelay300() + this->getStartDelayDispersion300()) {
			bool ret = this->innerStart();
			this->setState(ret ? kStateAction : kStateEnd);
		}
		break; }
	case kStateAction: {
		CC_ASSERT(target);
		if (!target->getSceneLayer()->getObjectList()->containsObject(target)) {
			// シーンレイヤー内にターゲットがいなくなった場合は追尾終了させる
			this->setState(kStateEnd);
			break;
		}
		//移動方向
		auto targetMovement = target->getObjectMovement();
		CC_ASSERT(target);
		cocos2d::Vec2 moveTarget(targetMovement->getMoveX(), targetMovement->getMoveY());
		cocos2d::Vec2 moveBullet(movement->getMoveX(), movement->getMoveY());
		cocos2d::Vec2 bulletPos = bullet->getCenterPosition();
		cocos2d::Vec2 targetPos = target->getCenterPosition();
		targetPos += Object::getSceneLayerScrollDiff(bullet->getSceneLayer(), target->getSceneLayer());
		cocos2d::Vec2 v = (targetPos - bulletPos).getNormalized();
		v.y *= -1.0f;
		float performance = AGTK_PARABOLA_INTERPOLATE(0, 100, 100, this->getPerformance());
		float len = (targetPos - bulletPos).getLength();
		cocos2d::Vec2 direction = GetDirection(v, this->getDirectionOld(), performance);
		bool isCorrection = false;
		if (moveTarget.length() == 0) {//ターゲットが移動していない。
			if (len - moveBullet.length() <= 0) {
				isCorrection = true;
			}
		}
		//弾が移動していない。
		if (moveBullet.length() == 0) {
			// 弾が発射されて移動していると判断できない場合は、弾の座標補正は行わない
			if (_moved) {
				isCorrection = true;
			}
		}
		//弾がターゲットより速い。
		if (moveTarget.length() < moveBullet.length()) {
			if (len - moveBullet.length() <= 0) {
				isCorrection = true;
			}
		}
#if 0
		//弾の速度がターゲットと同じか遅い。
#endif

		if (isCorrection && this->getPerformance() > 0.0f) {
			direction = cocos2d::Vec2::ZERO;
			bullet->addPosition(v * len);
		}
		this->setDirection(direction);
		if (direction != cocos2d::Vec2::ZERO && delta > 0) {
			_moved = true;
		}
		break; }
	case kStateEnd: {
		//そのままの方向へ向けるように。
		this->setDirection(movement->getDirection());
		break; }
	}
}

//-------------------------------------------------------------------------------------------------------------------
// 弾の飛び方(変化)：画面内のオブジェクトを自動追尾
//-------------------------------------------------------------------------------------------------------------------
NextBulletLocusFollowObjectInsideCamera::NextBulletLocusFollowObjectInsideCamera() : NextBulletLocus()
{
	_duration = 0.0f;
	_startDelay300 = 0;
	_startDelayDispersion300 = 0;
	_performance = 0.0f;
	_oldPosition = cocos2d::Vec2::ZERO;
	_moved = false;
}

bool NextBulletLocusFollowObjectInsideCamera::init(data::ObjectFireBulletSettingData *objectFireBulletSettingData)
{
	if (!NextBulletLocus::init(objectFireBulletSettingData)) {
		return false;
	}
	this->setDuration(0.0f);
	this->setStartDelay300(objectFireBulletSettingData->getFollowObjectInsideCameraStartDelayStart300());
	this->setStartDelayDispersion300(AGTK_RANDOM(0, objectFireBulletSettingData->getFollowObjectInsideCameraStartDelayEnd300() - objectFireBulletSettingData->getFollowObjectInsideCameraStartDelayStart300() ));
	this->setPerformance(objectFireBulletSettingData->getFollowObjectInsideCameraPerformance());
	return true;
}

bool NextBulletLocusFollowObjectInsideCamera::innerStart()
{
	int count = this->getCount();
	int maxCount = this->getMaxCount();
	auto parent = _parent;
	auto bullet = _bullet;

	auto objectFireBulletSettingData = this->getObjectFireBulletSettingData();
	auto scene = GameManager::getInstance()->getCurrentScene();
	auto sceneLayer = _bullet->getSceneLayer();
	auto camera = scene->getCamera();
	auto sceneSize = scene->getSceneSize();
// #AGTK-NX
#ifdef USE_SAR_OPTIMIZE_1
	auto objectList = scene->getObjectAllReference(sceneLayer->getType());
#else
	auto objectList = scene->getObjectAll(sceneLayer->getType());
#endif
	float maxLength = cocos2d::Vec2(cocos2d::Vec2::ZERO).getDistance(sceneSize);
	agtk::Object *selectObject = nullptr;
	for (int i = 0; i < objectList->count(); i++) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
		auto object = static_cast<agtk::Object *>(objectList->getObjectAtIndex(i));
#else
		auto object = dynamic_cast<agtk::Object *>(objectList->getObjectAtIndex(i));
#endif
		if (object == parent || object == bullet) {
			continue;
		}
		bool bSelected = false;
		auto objectData = object->getObjectData();
		if (objectData->getGroupBit() & objectFireBulletSettingData->getFollowObjectInsideCameraTargetObjectGroupBit()) {
			bSelected = true;
		}
		if (bSelected == false) {
			continue;
		}

		//カメラ内チェック。
		cocos2d::Rect rect(object->getPosition(), object->getContentSize());
		if (camera->isPositionScreenWithinCamera(rect) == false) {
			continue;
		}
		float length = parent->getCenterPosition().getDistance(object->getCenterPosition());
		if (length < maxLength) {
			selectObject = object;
			maxLength = length;
		}
	}
	float angle = 0.0f;
	if (selectObject) {
		auto selectPos = selectObject->getCenterPosition();
		auto parentPos = parent->getCenterPosition();
		selectPos += Object::getSceneLayerScrollDiff(parent->getSceneLayer(), selectObject->getSceneLayer());
		cocos2d::Vec2 v = selectPos - parentPos;
		v.y *= -1.0f;//※座標が左上が原点になっていて、ｙ座標はした方向が正になります。座標をcocos2dにしないと。
		angle = agtk::GetDegreeFromVector(v);
		this->setTargetObject(selectObject);

		//発射範囲を広げる & 動きを指定
		cocos2d::Vec2 direction = BulletLocus::calcDirection(objectFireBulletSettingData, angle, count, maxCount);
		this->setDirection(direction, true);
	}

	return this->getTargetObject() ? true : false;
}

void NextBulletLocusFollowObjectInsideCamera::start(int count, int maxCount)
{
	CC_ASSERT(_parent && _bullet);
	this->setCount(count);
	this->setMaxCount(maxCount);
	this->setDuration(0.0f);
	this->setState(kStateStart);
	auto bullet = _bullet;
	this->setOldPosition(bullet->getCenterPosition());
}

void NextBulletLocusFollowObjectInsideCamera::end()
{
	this->setState(kStateEnd);
}

void NextBulletLocusFollowObjectInsideCamera::update(float delta)
{
	auto movement = _bullet->getObjectMovement();
	auto bullet = _bullet;
	auto target = _targetObject;

	auto oldPosition = this->getOldPosition();
	cocos2d::Vec2 p = bullet->getCenterPosition() - this->getOldPosition();
	this->setOldPosition(bullet->getCenterPosition());
#if defined(AGTK_DEBUG)
	cocos2d::Color4F color = cocos2d::Color4F::YELLOW;
// #AGTK-NX
#if CC_TARGET_PLATFORM != CC_PLATFORM_NX
	showDebugLine(
		agtk::Scene::getPositionCocos2dFromScene(oldPosition),
		agtk::Scene::getPositionCocos2dFromScene(this->getOldPosition()),
		color, 0.2f
	);
#else
#endif
#endif
	_duration += delta;
	switch (this->getState()) {
	case kStateNone: {
		//そのままの方向へ向けるように。
		this->setDirection(movement->getDirection());
		break; }
	case kStateStart: {
		//そのままの方向へ向けるように。
		this->setDirection(movement->getDirection());
		if ((_duration * 300) >= this->getStartDelay300() + this->getStartDelayDispersion300()) {
			bool ret = this->innerStart();
			this->setState(ret ? kStateAction : kStateEnd);
		}
		break; }
	case kStateAction: {
		if (!target->getSceneLayer()->getObjectList()->containsObject(target)) {
			// シーンレイヤー内にターゲットがいなくなった場合は追尾終了させる
			this->setState(kStateEnd);
			break;
		}
		//移動方向
		auto targetMovement = target->getObjectMovement();
		CC_ASSERT(target);
		cocos2d::Vec2 moveTarget(targetMovement->getMoveX(), targetMovement->getMoveY());
		cocos2d::Vec2 moveBullet(movement->getMoveX(), movement->getMoveY());
		cocos2d::Vec2 bulletPos = bullet->getCenterPosition();
		cocos2d::Vec2 targetPos = target->getCenterPosition();
		targetPos += Object::getSceneLayerScrollDiff(bullet->getSceneLayer(), target->getSceneLayer());
		cocos2d::Vec2 v = (targetPos - bulletPos).getNormalized();
		v.y *= -1.0f;

		float performance = AGTK_PARABOLA_INTERPOLATE(0, 100, 100, this->getPerformance());
		float len = (targetPos - bulletPos).getLength();
		cocos2d::Vec2 direction = GetDirection(v, this->getDirectionOld(), performance);
		bool isCorrection = false;
		if (moveTarget.length() == 0) {//ターゲットが移動していない。
			if (len - moveBullet.length() <= 0) {
				isCorrection = true;
			}
		}
		//弾が移動していない。
		if (moveBullet.length() == 0) {
			// 弾が発射されて移動していると判断できない場合は、弾の座標補正は行わない
			if(_moved){
				isCorrection = true;
			}
		} 
		//弾がターゲットより速い。
		if (moveTarget.length() < moveBullet.length()) {
			if (len - moveBullet.length() <= 0) {
				isCorrection = true;
			}
		}
#if 0
		//弾の速度がターゲットと同じか遅い。
#endif

		if (isCorrection && this->getPerformance() > 0.0f) {
			direction = cocos2d::Vec2::ZERO;
			bullet->addPosition(v * len);
		}
		this->setDirection(direction);
		if (direction != cocos2d::Vec2::ZERO && delta > 0) {
			_moved = true;
		}
		break; }
	case kStateEnd: {
		//そのままの方向へ向けるように。
		auto movement = _bullet->getObjectMovement();
		this->setDirection(movement->getDirection());
		break; }
	}
}

//-------------------------------------------------------------------------------------------------------------------
// 弾の飛び方(変化)：ブーメラン軌道
//-------------------------------------------------------------------------------------------------------------------
NextBulletLocusBoomerang::NextBulletLocusBoomerang()
{
	_duration = 0.0f;
	_performance = 0.0f;
	_oldPosition = cocos2d::Vec2::ZERO;
	_decelMoveSpeed = 1.0f;
}

bool NextBulletLocusBoomerang::init(data::ObjectFireBulletSettingData *objectFireBulletSettingData)
{
	if (!NextBulletLocus::init(objectFireBulletSettingData)) {
		return false;
	}
	this->setDuration(0.0f);
	this->setPerformance(objectFireBulletSettingData->getBoomerangComebackPerformance());
	this->setTurnDuration300(objectFireBulletSettingData->getBoomerangTurnDuration300());
	this->setDecelBeforeTurn(objectFireBulletSettingData->getBoomerangDecelBeforeTurn());
	this->setTurnWhenTouchingWall(objectFireBulletSettingData->getBoomerangTurnWhenTouchingWall());
	return true;
}

void NextBulletLocusBoomerang::start(int count, int maxCount)
{
	this->setDuration(0.0f);
	auto bullet = _bullet;
	this->setOldPosition(bullet->getPosition());
	this->setState(kStateStart);
}

void NextBulletLocusBoomerang::end()
{
	this->setState(kStateEnd);
}

void NextBulletLocusBoomerang::update(float delta)
{
	auto bullet = _bullet;
	auto parent = _parent;
	auto target = _targetObject;

	auto oldPosition = this->getOldPosition();
	cocos2d::Vec2 p = bullet->getCenterPosition() - this->getOldPosition();
	this->setOldPosition(bullet->getCenterPosition());
#if defined(AGTK_DEBUG)
	cocos2d::Color4F color = cocos2d::Color4F::YELLOW;
// #AGTK-NX
#if CC_TARGET_PLATFORM != CC_PLATFORM_NX
	showDebugLine(
		agtk::Scene::getPositionCocos2dFromScene(oldPosition),
		agtk::Scene::getPositionCocos2dFromScene(this->getOldPosition()),
		color, 0.2f
	);
#else
#endif
#endif
	_duration += delta;
	switch (this->getState()) {
	case kStateNone: {
		//そのままの方向へ向けるように。
		auto movement = _bullet->getObjectMovement();
		this->setDirection(movement->getDirection());
		break; }
	case kStateStart: {
		//そのままの方向へ向けるように。
		auto movement = _bullet->getObjectMovement();
		this->setDirection(movement->getDirection());
		this->setState(kStateToTarget);
		break; }
	case kStateToTarget: {
		//移動方向（ターゲットオブジェクトへ）
		if (target) {
			auto bulletPos = bullet->getCenterPosition();
			auto targetPos = target->getCenterPosition();
			auto v = (targetPos - bulletPos).getNormalized();
			v.y *= -1.0f;
			this->setDirection(v, true);
		}
		//折り返すまでの減速
		if (this->getDecelBeforeTurn()) {
			int duration300 = (int)(_duration * 300);
			if (duration300 > this->getTurnDuration300()) duration300 = this->getTurnDuration300();
			this->setDecelMoveSpeed(AGTK_PARABOLA_INTERPOLATE2(1.0f, 0.0f, this->getTurnDuration300(), duration300));
		}

		//移動方向（親オブジェクトへ）
		std::function<cocos2d::Vec2()> getDirectionToParent = [&]() {
			auto bulletPos = bullet->getCenterPosition();
			auto parentPos = parent->getCenterPosition();
			if (this->getConnectId() > 0) {
				//接続点がある場合。
				int connectId = this->getConnectId();
				agtk::Vertex4 vertex4;
				parent->getTimeline(agtk::data::TimelineInfoData::kTimelineConnection, connectId, vertex4);
				parentPos = agtk::Scene::getPositionSceneFromCocos2d(vertex4[0]);
			}
			auto v = (parentPos - bulletPos).getNormalized();
			if (v == cocos2d::Vec2::ZERO) {
				//親と弾が同位置にある場合は、移動方向を加算して求める。
				v = (parentPos - (bulletPos + this->getDirection())).getNormalized();
			}
			v.y *= -1.0f;
			return v;
		};

		//折り返しまでの時間
		if ((_duration * 300) >= this->getTurnDuration300()) {
			this->setState(kStateToParent);//親オブジェクトへ
			_duration = 0.0f;//初期化。
			//移動方向（親オブジェクトへ）
			this->setDirection(getDirectionToParent(), true);
		}
		//壁や当たり判定に接触
		if (this->getTurnWhenTouchingWall()) {
			//当たり判定チェック。
			bool bHit = false;
			if (bullet->getCollisionAttackHitList()->count() > 0) {
				bHit = true;
			}
			if (bHit || bullet->getTileWallBit() || bullet->getAheadTileWallBit() || bullet->getObjectWallBit() || bullet->getSlopeBit()) {
				this->setState(kStateToParent);//親オブジェクトへ
				_duration = 0.0f;//初期化。
				//移動方向（親オブジェクトへ）
				this->setDirection(getDirectionToParent(), true);
			}
		}
		break; }
	case kStateToParent: {
		//折り返すまでの減速
		if (this->getDecelBeforeTurn()) {
			int duration300 = (int)(_duration * 300);
			if (duration300 > this->getTurnDuration300()) duration300 = this->getTurnDuration300();
			this->setDecelMoveSpeed(AGTK_PARABOLA_INTERPOLATE(0.0f, 1.0f, this->getTurnDuration300(), duration300));
		}
		//移動方向（親オブジェクトへ）
		auto bulletPos = bullet->getCenterPosition();
		auto parentPos = parent->getCenterPosition();
		if (this->getConnectId() > 0) {
			//接続点がある場合。
			int connectId = this->getConnectId();
			agtk::Vertex4 vertex4;
			parent->getTimeline(agtk::data::TimelineInfoData::kTimelineConnection, connectId, vertex4);
			parentPos = agtk::Scene::getPositionSceneFromCocos2d(vertex4[0]);
		}
		auto v = (parentPos - bulletPos).getNormalized();
		v.y *= -1.0f;

		float performance = AGTK_PARABOLA_INTERPOLATE(0, 100, 100, this->getPerformance());
		this->setDirection(GetDirection(v, this->getDirectionOld(), performance));

		break; }
	case kStateEnd: {
		//そのままの方向へ向けるように。
		auto movement = _bullet->getObjectMovement();
		this->setDirection(movement->getDirection());
		break; }
	default:CC_ASSERT(0);
	}
}

//-------------------------------------------------------------------------------------------------------------------
BulletLocus::BulletLocus()
{
	_objectFireBulletSettingData = nullptr;
	_initialBulletLocus = nullptr;
	_nextBulletLocus = nullptr;
	_direction = cocos2d::Vec2::ZERO;
	_parentObject = nullptr;
	_bullet = nullptr;
	_connectId = -1;
}

BulletLocus::~BulletLocus()
{
	CC_SAFE_RELEASE_NULL(_objectFireBulletSettingData);
	CC_SAFE_RELEASE_NULL(_initialBulletLocus);
	CC_SAFE_RELEASE_NULL(_nextBulletLocus);
	CC_SAFE_RELEASE_NULL(_parentObject);
}

BulletLocus *BulletLocus::create(agtk::Bullet *bullet, agtk::Object *object, agtk::data::ObjectFireBulletSettingData *objectFireBulletSettingData, int connectId)
{
	auto p = new (std::nothrow) BulletLocus();
	if (p && p->init(bullet, object, objectFireBulletSettingData, connectId)) {
		p->autorelease();
		return p;
	}
	CC_SAFE_DELETE(p);
	return nullptr;
}

bool BulletLocus::init(agtk::Bullet *bullet, agtk::Object *object, agtk::data::ObjectFireBulletSettingData *objectFireBulletSettingData, int connectId)
{
	CC_ASSERT(bullet && object);
	this->setBullet(bullet);
	this->setParentObject(object);
	this->setObjectFireBulletSettingData(objectFireBulletSettingData);
	this->setConnectId(connectId);

	auto initialBulletLocus = agtk::InitialBulletLocus::create(objectFireBulletSettingData);
	if (initialBulletLocus == nullptr) {
		return false;
	}
	this->setInitialBulletLocus(initialBulletLocus);

	auto nextBulletLocus = agtk::NextBulletLocus::create(objectFireBulletSettingData);
	if (nextBulletLocus == nullptr) {
		return false;
	}
	this->setNextBulletLocus(nextBulletLocus);
	return true;
}

bool BulletLocus::initial(int count, int maxCount)
{
	auto initBulletLocus = this->getInitialBulletLocus();
	if (!initBulletLocus->initial(this->getParentObject(), this->getConnectId(), this->getBullet(), count, maxCount)) {
		return false;
	}
	auto nextBulletLocus = this->getNextBulletLocus();
	nextBulletLocus->initial(this->getParentObject(), this->getBullet());
	nextBulletLocus->setConnectId(this->getConnectId());
	return true;
}

void BulletLocus::start(int count, int maxCount)
{
	auto initBulletLocus = this->getInitialBulletLocus();
	auto nextBulletLocus = this->getNextBulletLocus();

	//初期動作を設定。
	auto bullet = _bullet;
	auto objectMovement = bullet->getObjectMovement();
	auto direction = initBulletLocus->getDirection();
	direction = _bullet->directionCorrection(direction);
	objectMovement->setDirection(direction);

	//ブーメラン
	auto objectFireBulletSettingData = this->getObjectFireBulletSettingData();
	if (objectFireBulletSettingData->getNextBulletLocus() == agtk::data::ObjectFireBulletSettingData::kNextBulletLocusBoomerang) {
		nextBulletLocus->setDirection(initBulletLocus->getDirection(), true);
		nextBulletLocus->setTargetObject(initBulletLocus->getTargetObject());
	}

	//スタート時の親オブジェクトの位置
	auto parent = _parentObject;
	int connectId = this->getConnectId();
	agtk::Vertex4 vertex4;
	auto position = parent->getCenterPosition();
	if (connectId >= 0 && parent->getTimeline(agtk::data::TimelineInfoData::kTimelineConnection, connectId, vertex4)) {
		position = agtk::Scene::getPositionSceneFromCocos2d(vertex4[0]);
	}
	bullet->setPosition(position);

	return nextBulletLocus->start(count, maxCount);
}

void BulletLocus::end()
{
	auto nextBulletLocus = this->getNextBulletLocus();
	return nextBulletLocus->end();
}

void BulletLocus::update(float delta)
{
	auto nextBulletLocus = this->getNextBulletLocus();
	nextBulletLocus->update(delta);
	auto direction = nextBulletLocus->getDirection();
	this->setDirection(direction);
}

agtk::Bullet *BulletLocus::getBullet()
{
	CC_ASSERT(_bullet == nullptr || (_bullet && _bullet->getReferenceCount() > 0));
	return _bullet;
}

void BulletLocus::setBullet(agtk::Bullet *bullet)
{
	CC_ASSERT(bullet);
	CC_ASSERT(bullet->getReferenceCount() > 0);
	_bullet = bullet;
}

float BulletLocus::getMoveSpeed()
{
	float moveSpeed = 1.0f;
	auto data = this->getObjectFireBulletSettingData();
	if (data->getFreeBulletMoveSpeedFlag()) {
		moveSpeed = data->getFreeBulletMoveSpeed() * 0.01f;
	}
	// 戻り値 0.0f ～ 1.0f
	auto objectFireBulletSettingData = this->getObjectFireBulletSettingData();
	if (objectFireBulletSettingData->getNextBulletLocus() == agtk::data::ObjectFireBulletSettingData::kNextBulletLocusBoomerang) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
		auto boomerang = static_cast<agtk::NextBulletLocusBoomerang *>(this->getNextBulletLocus());
#else
		auto boomerang = dynamic_cast<agtk::NextBulletLocusBoomerang *>(this->getNextBulletLocus());
#endif
		moveSpeed *= boomerang->getDecelMoveSpeed();
	}
	return moveSpeed;
}

cocos2d::Vec2 BulletLocus::calcDirection(data::ObjectFireBulletSettingData *objectFireBulletSettingData, float degree, int count, int maxCount)
{
	cocos2d::Vec2 direction = cocos2d::Vec2::ZERO;
	switch (objectFireBulletSettingData->getOneDirectionSpreadType()) {
	case agtk::data::ObjectFireBulletSettingData::kSpreadFixed:
		direction = BulletLocus::getFixDirection(degree, objectFireBulletSettingData->getOneDirectionSpreadRange(), count, maxCount);
		break;
	case agtk::data::ObjectFireBulletSettingData::kSpreadWiper:
		direction = BulletLocus::getWaiperDirection(degree, objectFireBulletSettingData->getOneDirectionSpreadRange(), count, maxCount);
		break;
	case agtk::data::ObjectFireBulletSettingData::kSpreadRandom:
		direction = BulletLocus::getRandomDirection(degree, objectFireBulletSettingData->getOneDirectionSpreadRange());
		break;
	default:CC_ASSERT(0);
	}
	return direction;
}

cocos2d::Vec2 BulletLocus::getFixDirection(float degrees, float range, int count, int max)
{
	cocos2d::Vec2 v;
	if (max % 2 == 0) {//偶数
		float r = range / (float)max;
		if (max == 2) {
			if (count % 2 == 0) {//偶数
				v = agtk::GetDirectionFromDegrees(degrees - r);
			}
			else {//奇数
				v = agtk::GetDirectionFromDegrees(degrees + r);
			}
		} else {
			float r1 = r * 0.5f;
			float r2 = 0.0f;
			if (count % 2 == 0) {//偶数
				r2 = r * (count / 2);
				v = agtk::GetDirectionFromDegrees(degrees - (r1 + r2));
			}
			else {//奇数
				r2 = r * ((count - 1) / 2);
				v = agtk::GetDirectionFromDegrees(degrees + (r1 + r2));
			}
		}
	}
	else {//奇数
		if (count == 0) {
			v = agtk::GetDirectionFromDegrees(degrees);
		}
		else {
			CC_ASSERT(max - 1 > 0);
			float r = range / (float)(max - 1);
			if (count % 2 == 0) {//偶数
				v = agtk::GetDirectionFromDegrees(degrees - r * (count / 2));
			}
			else {//奇数
				v = agtk::GetDirectionFromDegrees(degrees + r * ((count + 1) / 2));
			}
		}
	}
	return v;
}

cocos2d::Vec2 BulletLocus::getRandomDirection(float degrees, float range)
{
	float r = ((float)rand() / RAND_MAX) * (range * 2);
	cocos2d::Vec2 v = agtk::GetDirectionFromDegrees(degrees - range + r);
	return v;
}

cocos2d::Vec2 BulletLocus::getWaiperDirection(float degrees, float range, int count, int max)
{
	float r = (range * 4.0f) / (float)max;
	bool reverse = false;
	float a = 0.0f;
	for (int i = 1; i < max; i++) {
		if (i > count) break;
		if (reverse) {
			if (a + r < range) {
				a += r;
			}
			else {
				auto sub = range - (a + r);
				a = range + sub;
				reverse = !reverse;
			}
		}
		else {
			if (a - r >= -range) {
				a -= r;
			}
			else {
				auto sub = -range - (a - r);
				a = -range + sub;
				reverse = !reverse;
			}
		}
	}
	//CCLOG("waiper:%f,%f,%f:%d,%d", degrees, degrees + a, range, count, max);
	return agtk::GetDirectionFromDegrees(degrees + a);
}

//-------------------------------------------------------------------------------------------------------------------
// 弾
//-------------------------------------------------------------------------------------------------------------------
Bullet::Bullet()
{
	_objectFireBulletSettingData = nullptr;
	_bulletLocus = nullptr;
	_parentObject = nullptr;
	_bulletIgnored = false;
}

Bullet::~Bullet()
{
	CC_SAFE_RELEASE_NULL(_objectFireBulletSettingData);
	CC_SAFE_RELEASE_NULL(_bulletLocus);
	CC_SAFE_RELEASE_NULL(_parentObject);
}

void Bullet::setCreate(Bullet *(*create)(agtk::Object *object, agtk::data::ObjectFireBulletSettingData *fireBulletSettingData, int connectId, int count, int maxCount))
{
	Bullet::create = create;
}

Bullet *Bullet::_create(agtk::Object *object, agtk::data::ObjectFireBulletSettingData *fireBulletSettingData, int connectId, int count, int maxCount)
{
	auto p = new (std::nothrow) Bullet();
	if (p && p->init(object, fireBulletSettingData, connectId, count, maxCount)) {
		p->autorelease();
		return p;
	}
	CC_SAFE_DELETE(p);
	return nullptr;
}

bool Bullet::init(agtk::Object *object, agtk::data::ObjectFireBulletSettingData * fireBulletSettingData, int connectId, int count, int maxCount)
{
	auto layerId = object->getLayerId();
	auto scene = GameManager::getInstance()->getCurrentScene();
	auto sceneLayer = scene->getSceneLayer(layerId);

	auto bulletLocus = agtk::BulletLocus::create(this, object, fireBulletSettingData, connectId);
	if (bulletLocus == nullptr) {
		return false;
	}
	this->setBulletLocus(bulletLocus);

	//接続点があるかチェック。
	auto player = object->getPlayer();
	bool valid = (player) ? player->getTimelineValid(connectId) : true;
	if (valid == false && connectId >= 0) {
		return false;
	}

	// initialization.
	bool ret = bulletLocus->initial(count, maxCount);
	if (ret == false) {
		return false;
	}
	auto pos = bulletLocus->getInitialBulletLocus()->getPosition();
	auto dispDirection = bulletLocus->getInitialBulletLocus()->getDispDirection();

	auto objectId = fireBulletSettingData->getBulletObjectId();
	if (objectId < 0) {
		return false;
	}
	// create object.
	ret = Object::init(
		sceneLayer,
		objectId,
		-1,//initialActionId
		pos,
		cocos2d::Vec2(1, 1),//scale
		0.0f,//rotation
		dispDirection,//向き,
		-1,//コースID
		-1//コース開始ポイント
	);
	if (ret == false) {
		return false;
	}

	this->setLayerId(layerId);
	this->setParentObject(object);
	this->setObjectFireBulletSettingData(fireBulletSettingData);
	this->setBulletIgnored(false);
	scene->getSceneLayer(layerId)->createPhysicsObjectWithObject((agtk::Object *)this);
	
	//「回転で自動生成」
	if (this->isAutoGeneration() && this->getPlayer()) {

		//弾の飛び方（飛び方を指定しない場合）
		auto initialBulletLocus = bulletLocus->getInitialBulletLocus();
		if (fireBulletSettingData->getInitialBulletLocus() == agtk::data::ObjectFireBulletSettingData::kInitialBulletLocusFree) {
			// 発射元のオブジェクトに表示方向を合わせない場合。
			auto objectFireBulletSettingData = initialBulletLocus->getObjectFireBulletSettingData();
			if (objectFireBulletSettingData->getSetActionDirectionToFireObjectDirection() == false) {
				dispDirection = this->getDispDirection();
				if (dispDirection <= 0) {
					dispDirection = this->calcDispDirection();
				}
				auto angle = agtk::GetDegreeFromVector(agtk::GetDirectionFromMoveDirectionId(dispDirection));

				//発射範囲を広げる & 動きを指定
				cocos2d::Vec2 direction = BulletLocus::calcDirection(objectFireBulletSettingData, angle, count, maxCount);

				initialBulletLocus->setDirection(direction);
				initialBulletLocus->setDispDirection(dispDirection);
			}
		}
		auto moveDirection = initialBulletLocus->getDirection();

		auto objectMovement = this->getObjectMovement();
		float centerRotation = agtk::GetDegreeFromVector(moveDirection);
		this->getPlayer()->setCenterRotation(centerRotation);
	}

	//スケール
	if (object && fireBulletSettingData->getBulletChild() && this->getObjectData()->getTakeoverScaling()) {
		float scaleX = object->getScaleX();
		float scaleY = object->getScaleY();
		this->setScale(cocos2d::Vec2(scaleX, scaleY));
	}

	// 弾を子として生成する場合
	if (fireBulletSettingData->getBulletChild()) {
		// 親オブジェクトのインスタンスIDを保存
		this->getPlayObjectData()->setParentObjectInstanceId(object->getInstanceId());
		object->addChildObject(this, Vec2::ZERO, connectId);
	}

	return true;
}

void Bullet::start(int count, int maxCount)
{
	if (this->getBulletIgnored()) {
		//無効。
		return;
	}
	auto bulletLocus = this->getBulletLocus();
	bulletLocus->start(count, maxCount);
}

void Bullet::end()
{
	if (this->getBulletIgnored()) {
		//無効。
		return;
	}
	auto bulletLocus = this->getBulletLocus();
	bulletLocus->end();
}

void Bullet::update(float delta)
{
	if (this->getBulletIgnored() == false) {
		//弾処理
		auto bulletLocus = this->getBulletLocus();
		bulletLocus->update(delta);
		//弾の飛び方の「飛び方を指定しない」以外の場合。
		if (this->getObjectFireBulletSettingData()->getNextBulletLocus() != agtk::data::ObjectFireBulletSettingData::kNextBulletLocusFree) {
			auto movement = this->getObjectMovement();
			cocos2d::Vec2 direction = bulletLocus->getDirection();
			direction = this->directionCorrection(direction);
			movement->setDirectionForce(direction);
		}
	}
	Object::update(delta);
}

#ifdef USE_MULTITHREAD_OBJECT_UPDATE
void Bullet::objectUpdateBefore(float delta)
{
	if (this->getBulletIgnored() == false) {
		//弾処理
		auto bulletLocus = this->getBulletLocus();
		bulletLocus->update(delta);
		//弾の飛び方の「飛び方を指定しない」以外の場合。
		if (this->getObjectFireBulletSettingData()->getNextBulletLocus() != agtk::data::ObjectFireBulletSettingData::kNextBulletLocusFree) {
			auto movement = this->getObjectMovement();
			cocos2d::Vec2 direction = bulletLocus->getDirection();
			direction = this->directionCorrection(direction);
			movement->setDirectionForce(direction);
		}
	}
	Object::objectUpdateBefore(delta);
}
#endif

NS_AGTK_END
