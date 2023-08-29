#include "BulletManager.h"
#include "External/collision/CollisionComponent.hpp"
#include "External/collision/CollisionUtils.hpp"
#include "Manager/GameManager.h"

USING_NS_CC;

int BulletManager::_fireId = 0;

NS_AGTK_BEGIN
//-------------------------------------------------------------------------------------------------------------------
FireBullet::FireBullet()
{
	_bullet = nullptr;
	_object = nullptr;
	_fireBulletSettingData = nullptr;
	_duration = 0.0f;
	_duration300 = 0;
	_count = 0;
	_maxCount = 1;
	_state = kStateNone;
	_removeParentObjectFlag = false;
	_isConnect = false;
	_beforeWaitState = kStateNone;
	_fireId = -1;
}

FireBullet::~FireBullet()
{
	CC_SAFE_RELEASE_NULL(_bullet);
	CC_SAFE_RELEASE_NULL(_object);
	CC_SAFE_RELEASE_NULL(_fireBulletSettingData);
}

FireBullet *FireBullet::create(agtk::Object *object, agtk::data::ObjectFireBulletSettingData *fireBulletSettingData, int connectId, int duration300)
{
	auto p = new (std::nothrow) FireBullet();
	if (p && p->init(object, fireBulletSettingData, connectId, duration300)) {
		p->autorelease();
		return p;
	}
	CC_SAFE_DELETE(p);
	return nullptr;
}

bool FireBullet::init(agtk::Object *object, agtk::data::ObjectFireBulletSettingData *fireBulletSettingData, int connectId, int duration300)
{
	CC_ASSERT(object);
	this->setState(kStateStart);
	this->setObject(object);
	this->setFireBulletSettingData(fireBulletSettingData);
	this->setConnectId(connectId);
	this->setDuration300(duration300);
	this->setLayerId(object->getLayerId());
	_fireId = 0;
	return true;
}

void FireBullet::update(float delta)
{
	auto object = this->getObject();
	if (object == nullptr) {
		return;
	}
	//接続点があるかチェック。
	auto player = object->getPlayer();
	auto connectId = this->getConnectId();
	bool valid = (player) ? player->getTimelineValid(connectId) : true;
	if (valid == false && connectId >= 0) {
		this->setIsConnect(false);
	}
	else {
		this->setIsConnect(true);
	}

	auto sceneLayer = object->getSceneLayer();
	auto scene = sceneLayer->getScene();
	float speed = scene->getGameSpeed()->getTimeScale(object);

	auto state = this->getState();
	if (state != kStateWait) {
		this->setDuration(this->getDuration() + (delta * speed));
	}
	switch (state) {
	case kStateNone: break;
	case kStateStart: {
		if (this->getDuration300() < (int)(this->getDuration() * 300)) {
			if (!this->getIsConnect()) {
				break;
			}

			//弾生成。
			bool ret = this->start();
			this->setState(ret ? kStateFire : kStateEnd);
		}
		break; }
	case kStateFire: {
//		auto bullet = this->getBullet();
//		auto scene = GameManager::getInstance()->getCurrentScene();
//		auto camera = scene->getCamera();
//		cocos2d::Rect rect(bullet->getPosition(), bullet->getContentSize());
//		if (camera->isPositionScreenWithinCamera(rect) == false) {
//			//画面外になった。
//			this->setState(kStateRemove);
//		}
		break; }
	case kStateRemove: {
		auto bullet = this->getBullet();
		if (bullet) {
			bullet->removeSelf();
		}
		this->setState(kStateEnd);
		break; }
	case kStateEnd: break;
	case kStateWait: break;
	default:CC_ASSERT(0);
	}

	//親オブジェクト破棄フラグが立っている場合は、ここでnullにする。
	if (this->getRemoveParentObjectFlag()) {
		if (this->getState() != kStateNone && this->getObject()) {
			this->setObject(nullptr);
			object = this->getObject();
			if (this->getState() == kStateStart || this->getState() == kStateWait) {
				//_object==nullptrになると、以降更新が行われなくなり、リークが発生するため、終了状態に変える。
				this->setState(kStateEnd);
			}
		}
	}

	//update
	if(object) {
		this->setPosition(object->getPosition());
		this->setDispDirection(object->getDispDirection());
	}
}

bool FireBullet::start()
{
	auto object = this->getObject();
	if (object == nullptr) {
		return false;
	}
	if (this->getRemoveParentObjectFlag()) {
		return false;
	}
	auto sceneLayer = object->getSceneLayer();
	auto scene = sceneLayer->getScene();
	auto fireBulletSettingData = this->getFireBulletSettingData();
	auto visibleSize = Director::getInstance()->getVisibleSize();
	auto bullet = agtk::Bullet::create(object, fireBulletSettingData, this->getConnectId(), this->getCount(), this->getMaxCount());
	if (bullet == nullptr) {
		return false;
	}
	this->setBullet(bullet);

	//start
	bullet->start(this->getCount(), this->getMaxCount());
	if (fireBulletSettingData->getInitialBulletLocus() != agtk::data::ObjectFireBulletSettingData::kInitialBulletLocusFree) {
		//「飛び方を指定しない」場合のみ、移動方向が変更されないフラグを立てる。
		bullet->getObjectMovement()->_directionForceIgnoredChangeActionFlag = true;
	}
	//照明効果
	auto viewportLightSceneLayer = scene->getViewportLight()->getViewportLightSceneLayer(this->getLayerId());
	auto objectData = bullet->getObjectData();
	if (objectData->getViewportLightSettingFlag() && objectData->getViewportLightSettingList()->count() > 0) {
		auto viewportLightObject = ViewportLightObject::create(bullet, scene->getViewportLight(), sceneLayer);
		viewportLightSceneLayer->getViewportLightObjectList()->addObject(viewportLightObject);
	}
	sceneLayer->addCollisionDetaction(bullet);
	bullet->setId(sceneLayer->publishObjectId());
	bullet->getPlayObjectData()->setInstanceId(scene->getObjectInstanceId(bullet));
	bullet->getPlayObjectData()->setInstanceCount(scene->incrementObjectInstanceCount(bullet->getObjectData()->getId()));
	sceneLayer->addObject(bullet);
	return true;
}

void FireBullet::end()
{
	auto bullet = this->getBullet();
	this->setState(kStateRemove);
}
NS_AGTK_END

//-------------------------------------------------------------------------------------------------------------------
BulletManager* BulletManager::_bulletManager = NULL;
BulletManager::BulletManager()
{
	_fireBulletList = nullptr;
	_disabledLayerIdList = nullptr;
}

BulletManager::~BulletManager()
{
	CC_SAFE_RELEASE_NULL(_fireBulletList);
	CC_SAFE_RELEASE_NULL(_disabledLayerIdList);
}

BulletManager* BulletManager::getInstance()
{
	if (!_bulletManager) {
		_bulletManager = new BulletManager();
		_bulletManager->init();
	}
	return _bulletManager;
}

void BulletManager::purge()
{
	if (!_bulletManager) {
		return;
	}
	BulletManager *p = _bulletManager;
	_bulletManager = NULL;
	p->release();
}

bool BulletManager::init()
{
	setFireBulletList(cocos2d::Array::create());
	setDisabledLayerIdList(cocos2d::__Array::create());
	return true;
}

void BulletManager::update(float delta)
{
	//update
	std::vector<int> noConnectFireIdList;		// 接続点がなく生成できない弾リスト
	auto fireBulletList = this->getFireBulletList();
	cocos2d::Ref *ref = nullptr;
	CCARRAY_FOREACH(fireBulletList, ref) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
		auto fireBullet = static_cast<agtk::FireBullet *>(ref);
#else
		auto fireBullet = dynamic_cast<agtk::FireBullet *>(ref);
#endif

		bool isDisabled = false;
		if(_disabledLayerIdList->count() > 0) {
			auto burret = fireBullet->getBullet();
			int sceneId = -1;
			if (burret) {
				sceneId = burret->getSceneData()->getId();
			}
			else {
				// 弾がまだ生成されていない場合は、親で判定
				auto obj = fireBullet->getObject();
				if (obj) {
					sceneId = obj->getSceneData()->getId();
				}
			}
			cocos2d::Ref *ref2;
			CCARRAY_FOREACH(_disabledLayerIdList, ref2) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
				auto p = static_cast<cocos2d::Integer *>(ref2);
#else
				auto p = dynamic_cast<cocos2d::Integer *>(ref2);
#endif
				if (sceneId != agtk::data::SceneData::kMenuSceneId && fireBullet->getLayerId() == p->getValue()) {
					isDisabled = true;
					break;
				}
			}
		}
		if (isDisabled) {
			continue;
		}

		bool isStateWait = false;
		auto nowState = fireBullet->getState();
		int fireBulletSettingId = fireBullet->getFireBulletSettingData()->getId();
		auto fireId = fireBullet->getFireId();
		for (auto noFireId : noConnectFireIdList) {
			if (fireId == noFireId) {
				// 待ち状態に変更
				isStateWait = true;
				if (nowState != agtk::FireBullet::kStateWait) {
					fireBullet->setBeforeWaitState(nowState);
					fireBullet->setState(agtk::FireBullet::kStateWait);
				}
				break;
			}
		}
		if (!isStateWait && nowState == agtk::FireBullet::kStateWait) {
			// 待ち状態から戻す
			fireBullet->setState(fireBullet->getBeforeWaitState());
			fireBullet->setBeforeWaitState(agtk::FireBullet::kStateNone);
		}

		fireBullet->update(delta);
		if (!fireBullet->getIsConnect() && nowState == agtk::FireBullet::kStateStart) {
			// 接続点がなかった場合は、同じ設定の弾を待ち状態にするためリストに追加
			if (std::find(noConnectFireIdList.begin(), noConnectFireIdList.end(), fireId) == noConnectFireIdList.end()) {
				noConnectFireIdList.push_back(fireId);
			}
		}
	}
	//remove
	this->updateRemoveFireBullet();
}

void BulletManager::updateRemoveFireBullet()
{
	bool bRemove;
	do {
		bRemove = false;
		cocos2d::Ref *ref = nullptr;
		auto fireBulletList = this->getFireBulletList();
		CCARRAY_FOREACH(fireBulletList, ref) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
			auto fireBullet = static_cast<agtk::FireBullet *>(ref);
#else
			auto fireBullet = dynamic_cast<agtk::FireBullet *>(ref);
#endif
			if (fireBullet->getState() == agtk::FireBullet::kStateEnd) {
				fireBulletList->removeObject(fireBullet);
				bRemove = true;
				break;
			}
		}
	} while (bRemove);
}

int BulletManager::getBulletCount(agtk::Object *object, int fireBulletSettingId)
{
	auto fireBulletList = this->getFireBulletList();
	int count = 0;
	cocos2d::Ref *ref;
	CCARRAY_FOREACH(fireBulletList, ref) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
		auto fireBullet = static_cast<agtk::FireBullet *>(ref);
#else
		auto fireBullet = dynamic_cast<agtk::FireBullet *>(ref);
#endif
		if (fireBullet->getObject() == object && fireBullet->getFireBulletSettingData()->getId() == fireBulletSettingId) {
			count++;
		}
	}
	return count;
}

void BulletManager::createBullet(agtk::Object *object, int fireBulletSettingId, int connectId)
{
	auto scene = GameManager::getInstance()->getCurrentScene();
	auto sceneLayer = scene->getSceneLayer(object->getLayerId());
	auto objectData = object->getObjectData();
	auto fireBulletSettingData = objectData->getFireBulletSettingData(fireBulletSettingId);

	int nowBulletNum = this->getBulletCount(object, fireBulletSettingId);
	int bulletMax = fireBulletSettingData->getFireBulletCount();
	if (fireBulletSettingData->getDispBulletUnlimited() == false) {//制限有り
		if (bulletMax + nowBulletNum > fireBulletSettingData->getDispBulletCount()) {//画面内に表示される弾数
			bulletMax = fireBulletSettingData->getDispBulletCount() - nowBulletNum;
			if (bulletMax > fireBulletSettingData->getFireBulletCount()) {
				bulletMax = fireBulletSettingData->getFireBulletCount();
			}
		}
	}

	int duration300 = 0;
	auto fireId = _fireId++;
	for (int i = 0; i < bulletMax; i++) {
		auto fireBullet = agtk::FireBullet::create(object, fireBulletSettingData, connectId, duration300);//生成。
		this->getFireBulletList()->addObject(fireBullet);
		duration300 += fireBulletSettingData->getBulletInterval300();
		fireBullet->setCount(i);
		fireBullet->setMaxCount(bulletMax);
		fireBullet->setFireId(fireId);
	}
}

void BulletManager::removeParentObject(agtk::Object *object, bool bStartStateOnly)
{
	cocos2d::Ref *ref = nullptr;
	auto fireBulletList = this->getFireBulletList();
	CCARRAY_FOREACH(fireBulletList, ref) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
		auto fireBullet = static_cast<agtk::FireBullet *>(ref);
#else
		auto fireBullet = dynamic_cast<agtk::FireBullet *>(ref);
#endif
		if (fireBullet->getObject() == object && (!bStartStateOnly || fireBullet->getState() == agtk::FireBullet::kStateStart)) {
			if (!fireBullet->getRemoveParentObjectFlag()) {
				fireBullet->setRemoveParentObjectFlag(true);
			}
		}
	}
}

void BulletManager::removeBullet(agtk::Bullet *bullet)
{
	auto fireBulletList = this->getFireBulletList();
	cocos2d::Ref *ref;
	CCARRAY_FOREACH(fireBulletList, ref) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
		auto fireBullet = static_cast<agtk::FireBullet *>(ref);
#else
		auto fireBullet = dynamic_cast<agtk::FireBullet *>(ref);
#endif
		if (fireBullet->getBullet() == bullet && fireBullet->getState() != agtk::FireBullet::kStateRemove) {
			fireBullet->end();
			fireBulletList->removeObject(fireBullet);
		}
	}
}

void BulletManager::removeAllBullet()
{
	bool bRemove;
	do {
		bRemove = false;
		auto fireBulletList = this->getFireBulletList();
		cocos2d::Ref *ref;
		CCARRAY_FOREACH(fireBulletList, ref) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
			auto fireBullet = static_cast<agtk::FireBullet *>(ref);
#else
			auto fireBullet = dynamic_cast<agtk::FireBullet *>(ref);
#endif
			if (fireBullet->getState() != agtk::FireBullet::kStateRemove) {
				fireBullet->end();
				fireBulletList->removeObject(fireBullet);
				bRemove = true;
				break;
			}
		}
	} while (bRemove);

	_disabledLayerIdList->removeAllObjects();
}
