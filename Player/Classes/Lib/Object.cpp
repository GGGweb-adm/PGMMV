/**
 * @brief オブジェクト
 */
#include "Object.h"
#include "Portal.h"
#include "Lib/Collision.h"
#include "Lib/Slope.h"
#include "Lib/Bullet.h"
#include "Lib/Gui.h"
#include "Lib/CameraObject.h"
#include "Lib/PhysicsObject.h"
#include "External/collision/CollisionComponent.hpp"
#include "External/collision/CollisionUtils.hpp"
#include "Manager/GameManager.h"
#include "Manager/InputManager.h"
#include "Manager/AudioManager.h"
#include "Manager/PrimitiveManager.h"
#include "Manager/EffectManager.h"
#include "Manager/ParticleManager.h"
#include "Manager/GuiManager.h"
#include "Manager/DebugManager.h"
#include "Manager/BulletManager.h"
#include "Manager/MovieManager.h"
#include "Manager/ImageManager.h"
#include "Lib/Scene.h"
#include "deprecated/CCDictionary.h"
#include "Manager/ThreadManager.h"
#include "Lib/Tile.h"
#include "Manager/JavascriptManager.h"
#include "scripting/js-bindings/manual/ScriptingCore.h"
#include "Manager/DebugManager.h"
#if (CC_TARGET_PLATFORM == CC_PLATFORM_NX)
#endif

#define TANK_OR_CAR_DIRECTION_MARGINE 22.5f	// 戦車 or 車タイプの移動の8方向決定用マージン(degree)
#define MIN_TANK_AND_CAR_MOVE_VELOCITY 0.0001f	// 速度方向の最小値
#ifdef USE_COLLISION_MEASURE
extern int wallCollisionCount;
extern int hitCollisionCount;
extern int attackCollisionCount;
extern int connectionCollisionCount;
extern int woConnectionCollisionCount;
extern int noInfoCount;
extern int callCount;
extern int cachedCount;
extern int roughWallCollisionCount;
extern int roughHitCollisionCount;
#endif
NS_AGTK_BEGIN

Object *(*Object::create)(agtk::SceneLayer *sceneLayer, int objectId, int initialActionId, cocos2d::Vec2 position, cocos2d::Vec2 scale, float rotation, int moveDirectionId, int courseId, int coursePointId, int forceAnimMotionId) = Object::_create;
Object *(*Object::createWithSceneDataAndScenePartObjectData)(agtk::SceneLayer *sceneLayer, agtk::data::ScenePartObjectData *scenePartObjectData, int forceObjectId) = Object::_createWithSceneDataAndScenePartObjectData;
ConnectObject* (*ConnectObject::create)(agtk::Object * object, int connectObjectSettingId, int actionId) = ConnectObject::_create;

#ifdef USE_MULTITHREAD_OBJECT_UPDATE
std::vector<agtk::Tile *>  getCollisionTileList(int type, int layerId, cocos2d::Point boundMin, cocos2d::Point boundMax, cocos2d::Vec2 moveVec=Vec2::ZERO,agtk::Object* obj=nullptr, int collideWithTileGroupBit = -1);
#else
cocos2d::__Array* getCollisionTileList(int type, int layerId, cocos2d::Point boundMin, cocos2d::Point boundMax, cocos2d::Vec2 moveVec = Vec2::ZERO, agtk::Object* obj = nullptr, int collideWithTileGroupBit = -1);
#endif

namespace {
	// 「基本設定パラメータ」の「被ダメージ率」の設定から、被ダメージ率を算出
	double getBaseParamDamageRate(agtk::data::PlayObjectData *playObjectData) {
		auto const baseDamageRate = playObjectData->getVariableData(agtk::data::EnumObjectSystemVariable::kObjectSystemVariableDamageRatio)->getValue();
		auto const damageRateValue = playObjectData->getVariableData(agtk::data::EnumObjectSystemVariable::kObjectSystemVariableDamageVariationValue)->getValue();
		double const damagedRate = AGTK_RANDOM(std::max(0.0, baseDamageRate - damageRateValue), baseDamageRate + damageRateValue);
		return damagedRate;
	}

	// 指定のタイルとオブジェクトは衝突判定が必要か？
	bool isNeedCheck(agtk::Object const * obj, agtk::Tile const * tile, bool checkActionCondition = false)
	{
		if (tile->getType() == agtk::Tile::kTypeLimitTile )
		{
			if (!dynamic_cast<LimitTile const *>(tile)->isNeedCheck(obj)) 
			{
				return false;
			}
		}
		else if (!checkActionCondition && !(obj->getObjectData()->getCollideWithTileGroupBit() & tile->getGroupBit()))
		{
			return false;
		}
		return true;
	}

	// 指定方向で当たっているタイルを格納する
	void addHitTileFunc(ObjectCollision::HitTileWalls *pLinkConditionTileWall, int tileWallBit, const WallHitInfo &wallHitInfo, int hitDir)
	{
		if (tileWallBit & hitDir && pLinkConditionTileWall) {
			for (auto & hitTile : wallHitInfo.hitTiles) {
				if (hitTile.first & hitDir) {
					// hitTile.first ではなく hitDir を記録しているのは、
					// 例えば、左にあるタイルにぶつかった場合(hitDirがleft)に
					// hitTile.first では、「左」「上」「下」が当たったという情報になってしまったりする。
					// この場合「左」だけを記録したい。（これまでの動作がそうだったのでそれに倣っている）
					ObjectCollision::HitTileWall htw;
					htw.bit = hitDir;
					htw.tile = hitTile.second;
					pLinkConditionTileWall->emplace_back(htw);
				}
			}
		}
	}

}

//-------------------------------------------------------------------------------------------------------------------
ObjectDamageInvincible::ObjectDamageInvincible()
{
	_object = nullptr;
	_objectData = nullptr;
	_frame300 = 0.0f;
	_bInvincible = false;
#ifdef FIX_ACT2_5401
	_bBlink = true;
#else
	_bBlink = false;
#endif
	_bUpdate = false;
#ifdef FIX_ACT2_5401
#else
	_startCount = 0;
	_updateCount = 0;
#endif
	_switchDamageInvincible = false;
#ifdef FIX_ACT2_5401
	_objectInvincible = nullptr;
#endif
	_isInvincibleStartAcceptable = false;
	_bUpdateFuncFlag = false;
}

ObjectDamageInvincible::~ObjectDamageInvincible()
{
	CC_SAFE_RELEASE_NULL(_objectData);
#ifdef FIX_ACT2_5401
	CC_SAFE_RELEASE_NULL(_objectInvincible);
#endif
	auto playObjectData = _object->getPlayObjectData();
	for (auto idKey : _switchChangedCallbackIdKeyList) {
		auto playSwitchData = idKey.first;
		playSwitchData->unregisterChangeCallback(idKey.second);
		playSwitchData->autorelease();
	}
}

bool ObjectDamageInvincible::init(agtk::Object *object)
{
	_object = object;
	this->setObjectData(object->getObjectData());
	return true;
}

void ObjectDamageInvincible::initCallback(agtk::ObjectVisible *objectVisible)
{
	if (_switchChangedCallbackIdKeyList.size() > 0) {
		return;
	}
	auto playObjectData = _object->getPlayObjectData();
	auto playSwitchData = playObjectData->getSwitchData(agtk::data::kObjectSystemSwitchInvincible);
	playSwitchData->retain();
	_switchChangedCallbackIdKeyList.emplace_back(std::make_pair(playSwitchData, playSwitchData->registerChangeCallback([](int id, bool value, void *arg) {
		((ObjectDamageInvincible *)arg)->switchChanged(true, id, value);
	}, this)));
	agtk::ObjectInvincible *objectInvincible = nullptr;
	auto objectInvincibleList = objectVisible->getObjectInvincibleList();
	cocos2d::Ref *ref;
	CCARRAY_FOREACH(objectInvincibleList, ref) {
		auto p = dynamic_cast<agtk::ObjectInvincible *>(ref);
		//check switch
		auto data = p->getInvincibleSettingData();
		if (data->getObjectSwitch()) {
			auto switchId = data->getObjectSwitchId();
			if (switchId == -1) {//無し
			}
			else {
				auto switchData = playObjectData->getSwitchData(switchId);
				if (switchData != nullptr) {
					switchData->retain();
					_switchChangedCallbackIdKeyList.emplace_back(std::make_pair(switchData, switchData->registerChangeCallback([](int id, bool value, void *arg) {
						((ObjectDamageInvincible *)arg)->switchChanged(false, id, value);
					}, this)));
				}
			}
		}
		else {
			auto projectPlayData = GameManager::getInstance()->getPlayData();
			auto switchId = data->getSystemSwitchId();
			if (switchId == -1) {//無し
			}
			else {
				auto switchData = projectPlayData->getSwitchData(agtk::data::PlayData::COMMON_PLAY_DATA_ID, switchId);
				if (switchData != nullptr) {
					switchData->retain();
					_switchChangedCallbackIdKeyList.emplace_back(std::make_pair(switchData, switchData->registerChangeCallback([](int id, bool value, void *arg) {
						((ObjectDamageInvincible *)arg)->switchChanged(true, id, value);
					}, this)));
				}
			}
		}
	}
}

void ObjectDamageInvincible::switchChanged(bool projectCommon, int id, bool value)
{
	cocos2d::log("%s, %s, %d: %d, %d, %d", __FILE__, __FUNCTION__, __LINE__, projectCommon, id, value);
	if (!_bInvincible) {
		//無敵中でなければチェック不要。
		return;
	}
	if (projectCommon && id == agtk::data::kObjectSystemSwitchInvincible) {
		if (!value) {
			//無敵中に、オブジェクトの「無敵」スイッチがOFFになったら、無敵開始受け入れ可能をONにする。
			_isInvincibleStartAcceptable = true;
		}
	}
	else if (_objectInvincible && !value) {
		//無敵中に、無敵関連のスイッチがOFFになった。もしアクティブな無敵関連のスイッチであれば、無敵開始受け入れ可能をONにする。オブジェクトの「無敵」スイッチをOFFにする。
		if (_objectInvincible) {
			auto invincibleSettingData = _objectInvincible->getInvincibleSettingData();
			bool hit = false;
			if (invincibleSettingData->getObjectSwitch()) {
				if (!projectCommon && invincibleSettingData->getObjectSwitchId() == id) {
					hit = true;
				}
			}
			else {
				if (projectCommon && invincibleSettingData->getSystemSwitchId() == id) {
					hit = true;
				}
			}
			if (hit) {
				_isInvincibleStartAcceptable = true;
				//オブジェクトの「無敵」スイッチをOFFにする。
				auto invincibleSwitch = _object->getPlayObjectData()->getSwitchData(agtk::data::kObjectSystemSwitchInvincible);
				if (invincibleSwitch->getValue()) {
					invincibleSwitch->setValue(false);
				}
			}
		}
	}
}

bool ObjectDamageInvincible::isInvincibleStartAcceptable()
{
	return _isInvincibleStartAcceptable;
}

bool ObjectDamageInvincible::start(bool bSwitchDamageInvincible)
{
	if (_isInvincibleStartAcceptable) {
		if (_bInvincible) {
			//無敵開始受け入れ可能なので、現在の無敵を終了させる。
			if (_objectInvincible) {
				_objectInvincible->end();
			}
			this->end();
		}
		_isInvincibleStartAcceptable = false;
	}
#ifdef FIX_ACT2_5401
	if (this->isInvincible()) {
		// 無敵中の場合は新たな無敵の更新等を行わない。
		return false;
	}
	//有効な無敵設定を探す。
	agtk::ObjectInvincible *objectInvincible = nullptr;
	auto objectInvincibleList = _object->getObjectVisible()->getObjectInvincibleList();
	cocos2d::Ref *ref;
	auto playObjectData = _object->getPlayObjectData();
	CCARRAY_FOREACH(objectInvincibleList, ref) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
		auto p = static_cast<agtk::ObjectInvincible *>(ref);
#else
		auto p = dynamic_cast<agtk::ObjectInvincible *>(ref);
#endif
		//check switch
		auto data = p->getInvincibleSettingData();
		bool bSwitch = false;
		if (data->getObjectSwitch()) {
			if (data->getObjectSwitchId() == -1) {//無し
				bSwitch = true;
			}
			else {
				auto switchData = playObjectData->getSwitchData(data->getObjectSwitchId());
				if (switchData != nullptr) {
					bSwitch = switchData->getValue();
				}
			}
		}
		else {
			auto projectPlayData = GameManager::getInstance()->getPlayData();
			if (data->getSystemSwitchId() == -1) {//無し
				bSwitch = true;
			}
			else {
				auto switchData = projectPlayData->getSwitchData(agtk::data::PlayData::COMMON_PLAY_DATA_ID, data->getSystemSwitchId());
				if (switchData != nullptr) {
					bSwitch = switchData->getValue();
				}
			}
		}
		if (bSwitch) {
			objectInvincible = p;
			break;
		}
	}
	this->setObjectInvincible(objectInvincible);
	auto objectData = this->getObjectData();
	auto invincibleSwitch = _object->getPlayObjectData()->getSwitchData(agtk::data::kObjectSystemSwitchInvincible);
	if (!_objectInvincible && !objectData->getInvincibleOnDamaged()) {
		// 無敵設定が無い場合は、無敵スイッチをOFFにし、何もせずに終わる。
		if (invincibleSwitch->getValue()) {
			invincibleSwitch->setValue(false);
		}
		return true;
	}
	_frame300 = 0.0f;
	_bBlink = true;
	_bInvincible = true;
	// 無敵スイッチをONにする。
	if (!invincibleSwitch->getValue()) {
		invincibleSwitch->setValue(true);
	}
	if (_objectInvincible) {
		_objectInvincible->start();
	}
	_bUpdate = true;
	_bUpdateFuncFlag = false;
#else
	if (_bUpdate == true && !bSwitchDamageInvincible) {
		_startCount++;
		if (_updateCount == _startCount) {
			return false;
		}
	}
	auto objectData = this->getObjectData();
	_bInvincible = bSwitchDamageInvincible ? true : objectData->getInvincibleOnDamaged();
	_frame300 = 0.0f;
	_bBlink = (objectData->getWinkWhenInvincible() == false);
	_bUpdate = true;
	_startCount = 0;
	_updateCount = 0;
	_switchDamageInvincible = bSwitchDamageInvincible;

	// 無敵状態なら「無敵」スイッチもONにする
	if (objectData->getInvincibleOnDamaged()) {
		_object->getPlayObjectData()->setSystemSwitchData(agtk::data::kObjectSystemSwitchInvincible, true);
		_switchDamageInvincible = true;
	}
#endif

	return true;
}

void ObjectDamageInvincible::end()
{
	_bInvincible = false;
	_bUpdate = false;
	_bUpdateFuncFlag = false;
#ifdef FIX_ACT2_5401
	this->setObjectInvincible(nullptr);
	auto invincibleSwitch = _object->getPlayObjectData()->getSwitchData(agtk::data::kObjectSystemSwitchInvincible);
	if (invincibleSwitch->getValue()) {
		invincibleSwitch->setValue(false);
	}
#else
	if (_switchDamageInvincible) {
		_object->getPlayObjectData()->setSystemSwitchData(agtk::data::kObjectSystemSwitchInvincible, false);
		_switchDamageInvincible = false;
	}
#endif
	_isInvincibleStartAcceptable = false;
}

void ObjectDamageInvincible::update(float dt)
{
	if (_bUpdate == false) {
		return;
	}
	if (_isInvincibleStartAcceptable) {
		//このタイミングで、無敵開始受け入れ可能がONの場合、無敵を終了させる。ただし、オブジェクトの無敵スイッチがONだった場合は新たな無敵を開始させる。
		auto invincibleSwitch = _object->getPlayObjectData()->getSwitchData(agtk::data::kObjectSystemSwitchInvincible);
		auto invincibleOn = invincibleSwitch->getValue();
		if (_objectInvincible) {
			_objectInvincible->end();
		}
		this->end();
		if (invincibleOn) {
			this->start(true);
		}
		return;
	}
#ifdef FIX_ACT2_5401
	auto invincibleSwitch = _object->getPlayObjectData()->getSwitchData(agtk::data::kObjectSystemSwitchInvincible);
	if (!invincibleSwitch->getValue()) {
		//無敵スイッチがOFFになっていたら無敵を終了させる。
		if (_objectInvincible) {
			_objectInvincible->end();
		}
		this->end();
		return;
	}
	auto playObjectData = _object->getPlayObjectData();
	if (_objectInvincible) {
		auto invincibleSettingData = _objectInvincible->getInvincibleSettingData();
		if (!invincibleSettingData->getInfinite()) {
			if (_frame300 > invincibleSettingData->getDuration300()) {
				_objectInvincible->end();
				this->end();
				return;
			}
		}
		//check switch
		bool bSwitch = false;
		if (invincibleSettingData->getObjectSwitch()) {
			if (invincibleSettingData->getObjectSwitchId() == -1) {//無し
				bSwitch = true;
			}
			else {
				auto switchData = playObjectData->getSwitchData(invincibleSettingData->getObjectSwitchId());
				if (switchData != nullptr) {
					bSwitch = switchData->getValue();
				}
			}
		}
		else {
			auto projectPlayData = GameManager::getInstance()->getPlayData();
			if (invincibleSettingData->getSystemSwitchId() == -1) {//無し
				bSwitch = true;
			}
			else {
				auto switchData = projectPlayData->getSwitchData(agtk::data::PlayData::COMMON_PLAY_DATA_ID, invincibleSettingData->getSystemSwitchId());
				if (switchData != nullptr) {
					bSwitch = switchData->getValue();
				}
			}
		}
		if (!bSwitch) {
			_objectInvincible->end();
			this->end();
			return;
		}
	}
	else {
		auto objectData = this->getObjectData();
		if (_frame300 > playObjectData->getVariableData(data::kObjectSystemVariableInvincibleDuration)->getValue() * 300.0) {//無敵時間を設定（秒）
			this->end();
			return;
		}
		//無敵中オブジェクトが点滅
		if (objectData->getWinkWhenInvincible()) {
			int winkInterval = objectData->getWinkInterval300();
			int v = (winkInterval) ? (int)_frame300 % winkInterval : 0;
			_bBlink = (v < winkInterval / 2) ? true : false;
		}
	}
	_frame300 += dt * 300.0;
	_bUpdateFuncFlag = true;//更新フラグON!
#else
	//ダメージを受けた時一定時間無敵にする
	auto objectData = this->getObjectData();
	if (objectData->getInvincibleOnDamaged()) {
		auto playObjectData = _object->getPlayObjectData();
		if (_frame300 > playObjectData->getVariableData(data::kObjectSystemVariableInvincibleDuration)->getValue() * 300.0) {//無敵時間を設定（秒）
			this->end();
		}
	}
	else {
		this->end();
	}
	//無敵中オブジェクトが点滅
	if (objectData->getWinkWhenInvincible() && _bInvincible) {
		int winkInterval = objectData->getWinkInterval300();
		int v = (winkInterval) ? (int)_frame300 % winkInterval : 0;
		_bBlink = (v < winkInterval / 2) ? true : false;
	}
	_frame300 += dt * 300.0;
	_updateCount++;
#endif
}

bool ObjectDamageInvincible::isInvincible()
{
	if (_bUpdate == false) {
		return false;
	}
	return (_bInvincible && _bUpdateFuncFlag);
}

bool ObjectDamageInvincible::isBlink()
{
	return _bUpdate ? _bBlink : true;
}

bool ObjectDamageInvincible::isExecuting()
{
	return _bUpdate;
}

//-------------------------------------------------------------------------------------------------------------------
ObjectInvincible::ObjectInvincible()
{
	_object = nullptr;
	_invincibleSettingData = nullptr;
	_bgmIdList = nullptr;
	_executing = false;
	_visible = true;
}

ObjectInvincible::~ObjectInvincible()
{
	CC_SAFE_RELEASE_NULL(_invincibleSettingData);
	CC_SAFE_RELEASE_NULL(_bgmIdList);
}

bool ObjectInvincible::init(agtk::Object *object, agtk::data::ObjectInvincibleSettingData *invincibleSettingData)
{
	_object = object;
	this->setInvincibleSettingData(invincibleSettingData);
	return true;
}

void ObjectInvincible::start()
{
	if (_executing) {
		return;
	}
	_executing = true;
	_frame = 0;
	//effect
	this->setFilterEffect();

	//etc.
	auto data = this->getInvincibleSettingData();
	//無敵中BGMを変更する
	if (data->getPlayBgm() && data->getBgmId() >= 0) {
		auto audioManager = AudioManager::getInstance();
		//pause
		auto bgmIdList = audioManager->getBgmIdList();
		cocos2d::Ref *ref = nullptr;
		CCARRAY_FOREACH(bgmIdList, ref) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
			auto bgmId = static_cast<cocos2d::Integer *>(ref);
#else
			auto bgmId = dynamic_cast<cocos2d::Integer *>(ref);
#endif
			audioManager->pauseBgm(bgmId->getValue());
		}
		this->setBgmIdList(bgmIdList);
		//play
		audioManager->playBgm(data->getBgmId());
	}
}

void ObjectInvincible::end()
{
	if (!_executing) {
		return;
	}
	_executing = false;
	_visible = true;
	//effect
	this->resetFilterEffect();

	auto audioManager = AudioManager::getInstance();
	auto data = this->getInvincibleSettingData();
	//無敵中BGMを変更する
	if (this->getBgmIdList()) {
		//resume
		auto bgmIdList = this->getBgmIdList();
		cocos2d::Ref *ref = nullptr;
		CCARRAY_FOREACH(bgmIdList, ref) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
			auto bgmId = static_cast<cocos2d::Integer *>(ref);
#else
			auto bgmId = dynamic_cast<cocos2d::Integer *>(ref);
#endif
			audioManager->resumeBgm(bgmId->getValue());
		}
		this->setBgmIdList(nullptr);
	}
	//stop
	if (data->getPlayBgm() && data->getBgmId() >= 0) {
		if (audioManager->isPlayingBgm(data->getBgmId())) {
			audioManager->stopBgm(data->getBgmId());
		}
	}

#ifdef FIX_ACT2_5401
#else
	//switch off
	auto playObjectData = _object->getPlayObjectData();
	if (data->getObjectSwitch()) {
		auto switchData = playObjectData->getSwitchData(data->getObjectSwitchId());
		if (switchData != nullptr) {
			switchData->setValue(false);
		}
	}
	else {
		auto projectPlayData = GameManager::getInstance()->getPlayData();
		auto switchData = projectPlayData->getSwitchData(agtk::data::PlayData::COMMON_PLAY_DATA_ID, data->getSystemSwitchId());
		if (switchData != nullptr) {
			switchData->setValue(false);
		}
	}
#endif
}

void ObjectInvincible::update(float delta)
{
	//check switch
	auto playObjectData = _object->getPlayObjectData();
	auto data = this->getInvincibleSettingData();
#ifdef FIX_ACT2_5401
#else
	bool bSwitch = false;
	if (data->getObjectSwitch()) {
		if (data->getObjectSwitchId() == -1) {//無し
			bSwitch = true;
		}
		else {
			auto switchData = playObjectData->getSwitchData(data->getObjectSwitchId());
			if (switchData != nullptr) {
				bSwitch = switchData->getValue();
			}
		}
	}
	else {
		auto projectPlayData = GameManager::getInstance()->getPlayData();
		if (data->getSystemSwitchId() == -1) {//無し
			bSwitch = true;
		}
		else {
			auto switchData = projectPlayData->getSwitchData(agtk::data::PlayData::COMMON_PLAY_DATA_ID, data->getSystemSwitchId());
			if (switchData != nullptr) {
				bSwitch = switchData->getValue();
			}
		}
	}

	if (bSwitch == true) {
		this->start();
	}
	else {
		this->end();
	}
#endif

	if (_executing == false) {
		return;
	}
#ifdef FIX_ACT2_5401
#else
	int durationMax300 = data->getDuration300();
	if (data->getFinishWink()) {
		durationMax300 += data->getFinishWinkDuration300();
	}
	if (_frame * 300 > durationMax300 && data->getInfinite() == false) {
		this->end();
		return;
	}
#endif
	//無敵中オブジェクトが点滅
	bool blink = true;
	int frame300 = _frame * 300;
	if (data->getWink() && (frame300 < data->getDuration300() || data->getInfinite())) {
		int winkInterval = data->getWinkInterval300();
		int v = frame300 % winkInterval;
		blink = (v < winkInterval / 2) ? true : false;
	}
	//終了時にオブジェクトが点滅
#ifdef FIX_ACT2_5401
	if (data->getInfinite() == false && data->getFinishWink() && frame300 >= data->getDuration300() - data->getFinishWinkDuration300()) {
#else
	if (data->getInfinite() == false && data->getFinishWink() && (data->getDuration300() <= frame300 && frame300 < durationMax300)) {
#endif
		data->getFinishWinkDuration300();
		int winkInterval = data->getFinishWinkInterval300();
		int v = frame300 % winkInterval;
		blink = (v < winkInterval / 2) ? true : false;
	}
	this->setVisible(blink);
	_frame += delta;
}

void ObjectInvincible::setFilterEffect()
{
	auto data = this->getInvincibleSettingData();
	if (data->getFilterEffectFlag() == false) {
		return;
	}
	auto player = _object->getPlayer();
	if (player == nullptr) {
		return;
	}
	auto filterEffect = data->getFilterEffect();
	float seconds = (float)filterEffect->getDuration300() / 300.0f;
	player->setStockShaderInfoFlag(true);//シェーダー中の場合は情報を保持する。
	switch (filterEffect->getEffectType()) {
	case agtk::data::FilterEffect::kEffectNoise://ノイズ
		player->setShader(agtk::Shader::kShaderNoisy, (float)filterEffect->getNoise() / 100.0f, seconds);
		break;
	case agtk::data::FilterEffect::kEffectMosaic://モザイク
		player->setShader(agtk::Shader::kShaderMosaic, (float)filterEffect->getMosaic() / 100.0f, seconds);
		break;
	case agtk::data::FilterEffect::kEffectMonochrome://モノクロ
		player->setShader(agtk::Shader::kShaderColorGray, (float)filterEffect->getMonochrome() / 100.0f, seconds);
		break;
	case agtk::data::FilterEffect::kEffectSepia://セピア
		player->setShader(agtk::Shader::kShaderColorSepia, (float)filterEffect->getSepia() / 100.0f, seconds);
		break;
	case agtk::data::FilterEffect::kEffectNegaPosiReverse://ネガ反転
		player->setShader(agtk::Shader::kShaderColorNega, (float)filterEffect->getNegaPosiReverse() / 100.0f, seconds);
		break;
	case agtk::data::FilterEffect::kEffectDefocus://ぼかし
		player->setShader(agtk::Shader::kShaderBlur, (float)filterEffect->getDefocus() / 100.0f, seconds);
		break;
	case agtk::data::FilterEffect::kEffectChromaticAberration://色収差
		player->setShader(agtk::Shader::kShaderColorChromaticAberration, (float)filterEffect->getChromaticAberration() / 100.0f, seconds);
		break;
	case agtk::data::FilterEffect::kEffectDarkness://暗闇
		player->setShader(agtk::Shader::kShaderColorDark, (float)filterEffect->getDarkness() / 100.0f, seconds);
		break;
	case agtk::data::FilterEffect::kEffectTransparency://透明
		player->setShader(agtk::Shader::kShaderTransparency, (float)filterEffect->getTransparency() / 100.0f, seconds);
		break;
	case agtk::data::FilterEffect::kEffectBlink://点滅
		_object->getObjectVisible()->startBlink(filterEffect->getBlinkInterval300() / 300.0f, seconds);
		break;
	case agtk::data::FilterEffect::kEffectDispImage://画像表示
		player->setExecActionSprite(data->getFilterEffect()->getImageId(), 255 * data->getFilterEffect()->getImageTransparency() / 100, seconds);
		break;
	case agtk::data::FilterEffect::kEffectFillColor: {//指定色で塗る
		player->setShader(agtk::Shader::kShaderColorRgba, 1.0f, seconds);
		player->getShader(agtk::Shader::kShaderColorRgba)->setShaderRgbaColor(cocos2d::Color4B(
			filterEffect->getFillR(),
			filterEffect->getFillG(),
			filterEffect->getFillB(),
			filterEffect->getFillA()
		));
		break; }
	default:CC_ASSERT(0);
	}
	player->setStockShaderInfoFlag(false);
}

void ObjectInvincible::resetFilterEffect()
{
	auto data = this->getInvincibleSettingData();
	if (data->getFilterEffectFlag() == false) {
		return;
	}
	auto filterEffect = data->getFilterEffect();
	auto player = _object->getPlayer();
	if (player == nullptr) {
		return;
	}
	float seconds = 0.0f;

	switch (filterEffect->getEffectType()) {
	case agtk::data::FilterEffect::kEffectNoise: {//ノイズ
		player->removeShader(agtk::Shader::kShaderNoisy, seconds);
		break; }
	case agtk::data::FilterEffect::kEffectMosaic: {//モザイク
		player->removeShader(agtk::Shader::kShaderMosaic, seconds);
		break; }
	case agtk::data::FilterEffect::kEffectMonochrome: {//モノクロ
		player->removeShader(agtk::Shader::kShaderColorGray, seconds);
		break; }
	case agtk::data::FilterEffect::kEffectSepia: {//セピア
		player->removeShader(agtk::Shader::kShaderColorSepia, seconds);
		break; }
	case agtk::data::FilterEffect::kEffectNegaPosiReverse: {//ネガ反転
		player->removeShader(agtk::Shader::kShaderColorNega, seconds);
		break; }
	case agtk::data::FilterEffect::kEffectDefocus: {//ぼかし
		player->removeShader(agtk::Shader::kShaderBlur, seconds);
		break; }
	case agtk::data::FilterEffect::kEffectChromaticAberration: {//色収差
		player->removeShader(agtk::Shader::kShaderColorChromaticAberration, seconds);
		break; }
	case agtk::data::FilterEffect::kEffectDarkness: {//暗闇
		player->removeShader(agtk::Shader::kShaderColorDark, seconds);
		break; }
	case agtk::data::FilterEffect::kEffectTransparency: {//透明
		player->removeShader(agtk::Shader::kShaderTransparency, seconds);
		break; }
	case agtk::data::FilterEffect::kEffectBlink: {//点滅
		_object->getObjectVisible()->endBlink(seconds);
		break; }
	case agtk::data::FilterEffect::kEffectDispImage: {//画像表示
		player->removeExecActionSprite(seconds);
		break; }
	case agtk::data::FilterEffect::kEffectFillColor: {//指定色で塗る
		player->removeShader(agtk::Shader::kShaderColorRgba, seconds);
		break; }
	}
}

bool ObjectInvincible::isWallAreaAttack()
{
	if (_executing) {
		//当たり判定を攻撃判定にする
		auto data = this->getInvincibleSettingData();
		return data->getWallAreaAttack();
	}
	return false;
}

//-------------------------------------------------------------------------------------------------------------------
ObjectVisible::ObjectVisible()
{
	_visible = true;
	_visibleTimer = nullptr;
	_objectDamageInvincible = nullptr;
	_objectInvincibleList = nullptr;

	_blinking = false;
	_blinkVisible = true;
	_blinkForceVisible = false;
	_blinkInterval = 1;
	_blinkTime = 0;
	_blinkTargetInterval = 1;
	_blinkTargetDuration = -1;
	_blinkTargetTime = 0;
	_blinkEndOnTargetReached = false;
	_blinkForceVisibleOnTargetReached = false;
}

ObjectVisible::~ObjectVisible()
{
	CC_SAFE_RELEASE_NULL(_visibleTimer);
	CC_SAFE_RELEASE_NULL(_objectDamageInvincible);
	CC_SAFE_RELEASE_NULL(_objectInvincibleList);
}

bool ObjectVisible::init(bool visible, agtk::Object *object)
{
	this->setVisible(visible);
	//タイマー
	auto visibleTimer = VisibleTimer::create();
	if (visibleTimer == nullptr) {
		return false;
	}
	this->setVisibleTimer(visibleTimer);
	//ダメージ点滅
	auto objectDamageInvincible = agtk::ObjectDamageInvincible::create(object);
	if (objectDamageInvincible == nullptr) {
		return false;
	}
	this->setObjectDamageInvincible(objectDamageInvincible);
	//無敵関連
	auto objectData = object->getObjectData();
	this->setObjectInvincibleList(cocos2d::__Array::create());
	if (objectData->getInvincibleSettingFlag()) {
		auto invincibleSettingList = objectData->getInvincibleSettingList();
		cocos2d::DictElement *el;
		CCDICT_FOREACH(invincibleSettingList, el) {
			auto data = dynamic_cast<agtk::data::ObjectInvincibleSettingData *>(el->getObject());
			auto invincible = agtk::ObjectInvincible::create(object, data);
			if (invincible == nullptr) {
				return false;
			}
			this->getObjectInvincibleList()->addObject(invincible);
		}
	}
	objectDamageInvincible->initCallback(this);
	return true;
}

void ObjectVisible::start(bool visible, float seconds)
{
	auto visibleTimer = this->getVisibleTimer();
	visibleTimer->start(visible, this->getVisible(), seconds);
}

void ObjectVisible::update(float dt)
{
	bool bVisible = true;
	//タイマー処理。
	auto visibleTimer = this->getVisibleTimer();
	visibleTimer->update(dt);
	bVisible = visibleTimer->getVisible();

	//ダメージ無敵
	bool bDamageVisible = true;
	auto damageInvincible = this->getObjectDamageInvincible();
	damageInvincible->update(dt);
	if (damageInvincible->isExecuting()) {
		bDamageVisible = damageInvincible->isBlink();
	}

	//点滅
	bool bBlinkVisible = true;
	if (_blinking) {
		do {
			auto newBlinkTime = _blinkTime + dt;
			if (_blinkTargetDuration > 0) {
				auto newBlinkTargetTime = _blinkTargetTime + dt;
				if (newBlinkTargetTime >= _blinkTargetDuration) {
					_blinkInterval = _blinkTargetInterval;
					_blinkTargetDuration = -1;
					if (_blinkEndOnTargetReached) {
						_blinking = false;
						_blinkVisible = true;
						_blinkInterval = 1;
						break;
					}
					if (_blinkForceVisibleOnTargetReached) {
						_blinkForceVisibleOnTargetReached = false;
						_blinkForceVisible = true;
						_blinkInterval = 1;
					}
				}
				else {
					bool isUp = (_blinkTime < _blinkInterval && newBlinkTime >= _blinkInterval);
					bool isDown = (_blinkTime < _blinkInterval * 2 && newBlinkTime >= _blinkInterval * 2);
					if (isUp || isDown) {
						//点滅の切り替わりタイミングにのみ_blinkIntervalを更新させる。
						float weight = newBlinkTargetTime / _blinkTargetDuration;
						_blinkInterval = _blinkInterval * (1 - weight) + _blinkTargetInterval * weight;
						_blinkTargetDuration -= newBlinkTargetTime;
						newBlinkTargetTime = 0;
						if (isUp) {
							newBlinkTime = _blinkInterval;
						}
						else if (isDown) {
							newBlinkTime = 0;
						}
					}
					_blinkTargetTime = newBlinkTargetTime;
				}
			}
			if (_blinkForceVisible) {
				_blinkVisible = true;
				break;
			}
			_blinkTime = newBlinkTime - (int)(newBlinkTime / (_blinkInterval * 2)) * (_blinkInterval * 2);
			_blinkVisible = (_blinkTime >= _blinkInterval);
		} while (0);
		bBlinkVisible = _blinkVisible;
	}

#ifdef FIX_ACT2_5401
	//無敵関連
	bool bInvincibleVisible = true;
	auto objectInvincible = damageInvincible->getObjectInvincible();
	if (objectInvincible) {
		objectInvincible->update(dt);
		bInvincibleVisible = objectInvincible->getVisible();
	}
#else
	//無敵
	bool bInvincibleVisible = true;
	auto objectInvincibleList = this->getObjectInvincibleList();
	cocos2d::Ref *ref;
	CCARRAY_FOREACH(objectInvincibleList, ref) {
		auto p = dynamic_cast<agtk::ObjectInvincible *>(ref);
		p->update(dt);
		bInvincibleVisible &= p->getVisible();
	}
#endif
	this->setVisible(bVisible & bDamageVisible & bBlinkVisible & bInvincibleVisible);
}

bool ObjectVisible::isWallAreaAttackWhenInvincible()
{
	bool bWallAreaAttack = false;
#ifdef FIX_ACT2_5401
	auto objectInvincible = this->getObjectDamageInvincible()->getObjectInvincible();
	if (objectInvincible) {
		bWallAreaAttack = objectInvincible->isWallAreaAttack();
	}
#else
	auto objectInvincibleList = this->getObjectInvincibleList();
	cocos2d::Ref *ref;
	CCARRAY_FOREACH(objectInvincibleList, ref) {
		auto p = dynamic_cast<agtk::ObjectInvincible *>(ref);
		bWallAreaAttack |= p->isWallAreaAttack();
	}
#endif
	return bWallAreaAttack;
}

bool ObjectVisible::isInvincible()
{
	auto objectInvincibleList = this->getObjectInvincibleList();
	cocos2d::Ref *ref;
	CCARRAY_FOREACH(objectInvincibleList, ref) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
		auto p = static_cast<agtk::ObjectInvincible *>(ref);
#else
		auto p = dynamic_cast<agtk::ObjectInvincible *>(ref);
#endif
		if (p->isExecuting()) {
			return true;
		}
	}
	return this->getObjectDamageInvincible()->isInvincible();
}

void ObjectVisible::end()
{
	//タイマー処理
	auto visibleTimer = this->getVisibleTimer();
	visibleTimer->end();
	//ダメージ
	auto damageInvincible = this->getObjectDamageInvincible();
	damageInvincible->end();
	//無敵
	bool bInvincibleVisible = true;
	auto objectInvincibleList = this->getObjectInvincibleList();
	cocos2d::Ref *ref;
	CCARRAY_FOREACH(objectInvincibleList, ref) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
		auto p = static_cast<agtk::ObjectInvincible *>(ref);
#else
		auto p = dynamic_cast<agtk::ObjectInvincible *>(ref);
#endif
		p->end();
	}
}

// interval == 0は、非点滅（常に表示）を表す。
void ObjectVisible::startBlink(float interval, float seconds)
{
	_blinkForceVisibleOnTargetReached = false;
	if (seconds == 0) {
		if (interval == 0) {
			_blinkVisible = true;
			_blinkForceVisible = true;
		}
		else {
			_blinkForceVisible = false;
			if (_blinking) {
			}
			else {
				//新たに点滅開始になったら、非表示から始める。
				_blinkVisible = false;
				_blinkTime = 0;
			}
			_blinkInterval = interval;
		}
		_blinking = true;
		_blinkTargetDuration = -1;
		return;
	}
	if (_blinking) {
		if (interval == 0) {
			if (_blinkForceVisible) {
				//変更前が非点滅だったため、変更の必要なし。
			}
			else {
				_blinkForceVisible = false;
				_blinkTargetInterval = std::max(1.0f, _blinkInterval * 4);
				_blinkForceVisibleOnTargetReached = true;
			}
		}
		else {
			_blinkForceVisible = false;
			_blinkTargetInterval = interval;
		}
		_blinkTargetDuration = seconds;
		_blinkTargetTime = 0;
	}
	else {
		_blinking = true;
		_blinkTime = 0;
		if (interval == 0) {
			_blinkVisible = true;
			_blinkForceVisible = true;
			_blinkInterval = 1;
			_blinkTargetDuration = -1;
		}
		else {
			//新たに点滅開始になったら、非表示から始める。
			_blinkVisible = false;
			_blinkForceVisible = false;
			_blinkInterval = std::max(1.0f, interval * 4);
			_blinkTargetInterval = interval;
			_blinkTargetDuration = seconds;
			_blinkTargetTime = 0;
		}
	}
	_blinkEndOnTargetReached = false;
}

void ObjectVisible::endBlink(float seconds)
{
	if (!_blinking) {
		return;
	}
	if (seconds == 0 || _blinkForceVisible) {
		_blinking = false;
		_blinkVisible = true;
		_blinkForceVisible = true;
	}
	else if (_blinkTargetDuration > 0 && _blinkEndOnTargetReached) {
		//点滅効果の削除中に、新たな削除がリクエストされた場合は、無視する。
	}
	else {
		_blinkTargetInterval = std::max(1.0f, _blinkInterval * 4);
		_blinkTargetDuration = seconds;
		_blinkTargetTime = 0;
		_blinkEndOnTargetReached = true;
	}
}

bool ObjectVisible::isBlinking()
{
	return _blinking;
}

void ObjectVisible::takeOverBlink(ObjectVisible *objectVisible)
{
	_blinking = objectVisible->_blinking;
	_blinkVisible = objectVisible->_blinkVisible;
	_blinkForceVisible = objectVisible->_blinkForceVisible;
	_blinkInterval = objectVisible->_blinkInterval;
	_blinkTime = objectVisible->_blinkTime;
	_blinkTargetInterval = objectVisible->_blinkTargetInterval;
	_blinkTargetDuration = objectVisible->_blinkTargetDuration;
	_blinkTargetTime = objectVisible->_blinkTargetTime;
	_blinkEndOnTargetReached = objectVisible->_blinkEndOnTargetReached;
}

//-------------------------------------------------------------------------------------------------------------------
ObjectDamaged::ObjectDamaged()
{
	_object = nullptr;
	_damagedSettingData = nullptr;
	_visible = true;
	_executing = false;
	_frame = 0.0f;
}

ObjectDamaged::~ObjectDamaged()
{
	CC_SAFE_RELEASE_NULL(_damagedSettingData);
}

bool ObjectDamaged::init(agtk::Object *object, agtk::data::ObjectDamagedSettingData *damagedSettingData)
{
	_object = object;
	this->setDamagedSettingData(damagedSettingData);

	return true;
}

void ObjectDamaged::update(float dt)
{
	if (_executing) {
		auto data = this->getDamagedSettingData();
		if (_frame * 300 > data->getDuration300()) {
			//終了
			this->end();
			return;
		}
		//演出中オブジェクトが点滅
		bool blink = true;
		int frame300 = _frame * 300;
		if (data->getWink() && frame300 < data->getDuration300()) {
			int winkInterval = data->getWinkInterval300();
			if (winkInterval == 0) {
				blink = true;//点滅無し。
			}
			else {
				int v = frame300 % winkInterval;
				blink = (v < winkInterval / 2) ? true : false;
			}
		}
		this->setVisible(blink);
		_frame += dt;
	}
}

void ObjectDamaged::start(agtk::Object *attackObject)
{
	if (_executing == true) {
		this->end();
		this->update(0.0f);
	}
	if (this->checkIgnored(attackObject)) {//無効の場合
		return;
	}
	_executing = true;
	_frame = 0.0f;
	this->setFilterEffect();

	//SE再生
	auto data = this->getDamagedSettingData();
	if (data->getPlaySe()) {
		auto audioManager = AudioManager::getInstance();
		audioManager->playSe(data->getSeId());
	}
}

void ObjectDamaged::end()
{
	if (_executing == false) {
		return;
	}
	_executing = false;
	_visible = true;
	this->resetFilterEffect();
}

void ObjectDamaged::dioStart(agtk::Object *attackObject)
{
	auto data = this->getDamagedSettingData();
	auto dioGameSpeed = data->getDioGameSpeed() / 100;
	auto dioEffectDuration = data->getDioEffectDuration();

	std::function<void(agtk::Object*, bool, bool)> setParentObjectDioGameSpeed = [&](agtk::Object *dioObject, bool _parentFlg, bool _childFlg) {
		if (dioObject == nullptr) {
			return;
		}
		if (dioObject->getDioExecuting()) {
			return;
		}

		// このオブジェクトのヒットストップを設定
		dioObject->setDioExecuting(true);
		dioObject->setDioFrame(0.0f);
		dioObject->setDioGameSpeed(dioGameSpeed);
		dioObject->setDioEffectDuration(dioEffectDuration);
		dioObject->setDioParent(_parentFlg);
		dioObject->setDioChild(_childFlg);

		auto parentObject = dioObject->getOwnParentObject();
		if (parentObject != nullptr) {
			setParentObjectDioGameSpeed(parentObject, _parentFlg, _childFlg);
		}
	};
	std::function<void(agtk::Object*, bool, bool)> setChildObjectDioGameSpeed = [&](agtk::Object *dioObject, bool _parentFlg, bool _childFlg) {
		if (dioObject == nullptr) {
			return;
		}
		if (dioObject->getDioExecuting()) {
			return;
		}

		// このオブジェクトのヒットストップを設定
		dioObject->setDioExecuting(true);
		dioObject->setDioFrame(0.0f);
		dioObject->setDioGameSpeed(dioGameSpeed);
		dioObject->setDioEffectDuration(dioEffectDuration);
		dioObject->setDioParent(_parentFlg);
		dioObject->setDioChild(_childFlg);

		if (_childFlg) {
			auto childrenList = dioObject->getChildrenObjectList();
			cocos2d::Ref *ref;
			CCARRAY_FOREACH(childrenList, ref) {
				auto obj = dynamic_cast<agtk::Object *>(ref);
				setChildObjectDioGameSpeed(obj, _parentFlg, _childFlg);
			}
		}
	};

	agtk::Object* chkObject = nullptr;
	bool dioParent = false;
	bool diochild = false;
	if (data->getDioReceiving()) {
		// 受けた側にヒットストップを行う
		chkObject = _object;
		dioParent = data->getDioRecvParent();
		diochild = data->getDioRecvChild();
		if (dioParent) {
			auto ownParentObject = chkObject->getOwnParentObject();
			if (ownParentObject != nullptr) {
				// 親の設定
				setParentObjectDioGameSpeed(ownParentObject, dioParent, diochild);
			}
		}
		// 自身と子の設定
		setChildObjectDioGameSpeed(chkObject, dioParent, diochild);
	}

	if (data->getDioDealing() && attackObject) {
		// 与えた側にヒットストップを行う
		chkObject = attackObject;
		dioParent = data->getDioDealParent();
		diochild = data->getDioDealChild();
		if (dioParent) {
			auto ownParentObject = chkObject->getOwnParentObject();
			if (ownParentObject != nullptr) {
				// 親の設定
				setParentObjectDioGameSpeed(ownParentObject, dioParent, diochild);
			}
		}
		// 自身と子の設定
		setChildObjectDioGameSpeed(chkObject, dioParent, diochild);
	}
}

void ObjectDamaged::setFilterEffect()
{
	auto data = this->getDamagedSettingData();
	if (data->getFilterEffectFlag() == false) {
		return;
	}
	auto player = _object->getPlayer();
	if (player == nullptr) {
		return;
	}
	auto filterEffect = data->getFilterEffect();
	float seconds = (float)filterEffect->getDuration300() / 300.0f;
	switch (filterEffect->getEffectType()) {
	case agtk::data::FilterEffect::kEffectNoise://ノイズ
		player->setShader(agtk::Shader::kShaderNoisy, (float)filterEffect->getNoise() / 100.0f, seconds);
		break;
	case agtk::data::FilterEffect::kEffectMosaic://モザイク
		player->setShader(agtk::Shader::kShaderMosaic, (float)filterEffect->getMosaic() / 100.0f, seconds);
		break;
	case agtk::data::FilterEffect::kEffectMonochrome://モノクロ
		player->setShader(agtk::Shader::kShaderColorGray, (float)filterEffect->getMonochrome() / 100.0f, seconds);
		break;
	case agtk::data::FilterEffect::kEffectSepia://セピア
		player->setShader(agtk::Shader::kShaderColorSepia, (float)filterEffect->getSepia() / 100.0f, seconds);
		break;
	case agtk::data::FilterEffect::kEffectNegaPosiReverse://ネガ反転
		player->setShader(agtk::Shader::kShaderColorNega, (float)filterEffect->getNegaPosiReverse() / 100.0f, seconds);
		break;
	case agtk::data::FilterEffect::kEffectDefocus://ぼかし
		player->setShader(agtk::Shader::kShaderBlur, (float)filterEffect->getDefocus() / 100.0f, seconds);
		break;
	case agtk::data::FilterEffect::kEffectChromaticAberration://色収差
		player->setShader(agtk::Shader::kShaderColorChromaticAberration, (float)filterEffect->getChromaticAberration() / 100.0f, seconds);
		break;
	case agtk::data::FilterEffect::kEffectDarkness://暗闇
		player->setShader(agtk::Shader::kShaderColorDark, (float)filterEffect->getDarkness() / 100.0f, seconds);
		break;
	case agtk::data::FilterEffect::kEffectTransparency://透明
		player->setShader(agtk::Shader::kShaderTransparency, (float)filterEffect->getTransparency() / 100.0f, seconds);
		break;
	case agtk::data::FilterEffect::kEffectBlink://点滅
		_object->getObjectVisible()->startBlink(filterEffect->getBlinkInterval300() / 300.0f, seconds);
		break;
	case agtk::data::FilterEffect::kEffectDispImage://画像表示
		player->setExecActionSprite(data->getFilterEffect()->getImageId(), 255 * data->getFilterEffect()->getImageTransparency() / 100, seconds);
		break;
	case agtk::data::FilterEffect::kEffectFillColor: {//指定色で塗る
		player->setShader(agtk::Shader::kShaderColorRgba, 1.0f, seconds);
		player->getShader(agtk::Shader::kShaderColorRgba)->setShaderRgbaColor(cocos2d::Color4B(
			filterEffect->getFillR(),
			filterEffect->getFillG(),
			filterEffect->getFillB(),
			filterEffect->getFillA()
		));
		break; }
	default:CC_ASSERT(0);
	}
}

void ObjectDamaged::resetFilterEffect()
{
	auto data = this->getDamagedSettingData();
	if (data->getFilterEffectFlag() == false) {
		return;
	}
	auto filterEffect = data->getFilterEffect();
	auto player = _object->getPlayer();
	if (player == nullptr) {
		return;
	}
	float seconds = 0.0f;

	switch (filterEffect->getEffectType()) {
	case agtk::data::FilterEffect::kEffectNoise: {//ノイズ
		player->removeShader(agtk::Shader::kShaderNoisy, seconds);
		break; }
	case agtk::data::FilterEffect::kEffectMosaic: {//モザイク
		player->removeShader(agtk::Shader::kShaderMosaic, seconds);
		break; }
	case agtk::data::FilterEffect::kEffectMonochrome: {//モノクロ
		player->removeShader(agtk::Shader::kShaderColorGray, seconds);
		break; }
	case agtk::data::FilterEffect::kEffectSepia: {//セピア
		player->removeShader(agtk::Shader::kShaderColorSepia, seconds);
		break; }
	case agtk::data::FilterEffect::kEffectNegaPosiReverse: {//ネガ反転
		player->removeShader(agtk::Shader::kShaderColorNega, seconds);
		break; }
	case agtk::data::FilterEffect::kEffectDefocus: {//ぼかし
		player->removeShader(agtk::Shader::kShaderBlur, seconds);
		break; }
	case agtk::data::FilterEffect::kEffectChromaticAberration: {//色収差
		player->removeShader(agtk::Shader::kShaderColorChromaticAberration, seconds);
		break; }
	case agtk::data::FilterEffect::kEffectDarkness: {//暗闇
		player->removeShader(agtk::Shader::kShaderColorDark, seconds);
		break; }
	case agtk::data::FilterEffect::kEffectTransparency: {//透明
		player->removeShader(agtk::Shader::kShaderTransparency, seconds);
		break; }
	case agtk::data::FilterEffect::kEffectBlink: {//点滅
		_object->getObjectVisible()->endBlink(seconds);
		break; }
	case agtk::data::FilterEffect::kEffectDispImage: {//画像表示
		player->removeExecActionSprite(seconds);
		break; }
	case agtk::data::FilterEffect::kEffectFillColor: {//指定色で塗る
		player->removeShader(agtk::Shader::kShaderColorRgba, seconds);
		break; }
	}
}

bool ObjectDamaged::checkAttribute(int attribute)
{
	auto data = this->getDamagedSettingData();
	switch (data->getAttributeType()) {
	case 0://属性設定無し
		return true;
	case 1://プリセットの属性
		if (data->getAttributePresetId() == attribute) {
			return true;
		}
		break;
	case 2://属性値で指定
		if (data->getAttributeEqual()) {// "="
			if (data->getAttributeValue() == attribute) {
				return true;
			}
		}
		else {// "!="
			if (data->getAttributeValue() != attribute) {
				return true;
			}
		}
	}
	return false;
}

bool ObjectDamaged::getDamage(agtk::Object *attackObject, double& damage, bool& bCriticalDamaged, agtk::data::PlayObjectData *damageRatePlayObjectData, bool takeOverDamageRate)
{
	if (checkIgnored(attackObject)) {
		//無効
		return false;
	}
	damage = 0.0;
	auto attackObjectData = attackObject->getObjectData();
	auto attackPlayObjectData = attackObject->getPlayObjectData();
	auto playObjectData = _object->getPlayObjectData();
	auto data = this->getDamagedSettingData();
	double damagedRate = 0;
	if (data->getDamagedRateFlag()) {
		damagedRate = data->getDamagedRate();
	}
	else {
		//スクリプト
		if (strlen(data->getDamagedScript()) > 0) {
#ifdef USE_SCRIPT_PRECOMPILE
			//JSB_AUTOCOMPARTMENT_WITH_GLOBAL_OBJCET
			auto sc = ScriptingCore::getInstance();
			auto cx = sc->getGlobalContext();
			auto _global = sc->getGlobalObject();
			JS::RootedObject gobj(cx, _global);
			JSAutoCompartment ac(cx, gobj);

			JS::RootedObject ns(cx);
			JS::MutableHandleObject jsObj = &ns;
			JS::RootedValue nsval(cx);
			JS_GetProperty(cx, gobj, "Agtk", &nsval);
			JS::RootedValue rv(cx);
			bool ret = false;
			if (nsval != JSVAL_VOID) {
				jsObj.set(nsval.toObjectOrNull());
				JS::RootedValue v(cx);
				JS_GetProperty(cx, jsObj, "scriptFunctions", &v);
				if (v.isObject()) {
					JS::RootedObject rscriptFunctions(cx, &v.toObject());
					JS_GetProperty(cx, rscriptFunctions, "execObjectDamaged", &v);
					if (v.isObject()) {
						JS::RootedValue rexec(cx, v);
						jsval args[2];
						args[0] = JS::Int32Value(_object->getObjectData()->getId());
						args[1] = JS::Int32Value(data->getId());
						ret = JS_CallFunctionValue(cx, rscriptFunctions, rexec, JS::HandleValueArray::fromMarkedLocation(2, args), &rv);
					}
				}
			}
#else
			auto scriptingCore = ScriptingCore::getInstance();
			auto context = scriptingCore->getGlobalContext();
			JS::RootedValue rv(context);
			JS::MutableHandleValue mhv(&rv);
			auto script = String::createWithFormat("(function(){ var objectId = %d, damagedId = %d; return (%s\n); })()", _object->getObjectData()->getId(), data->getId(), data->getDamagedScript());
			auto ret = ScriptingCore::getInstance()->evalString(script->getCString(), mhv);
#endif
			if (!ret) {
				//スクリプトエラー
				auto errorStr = String::createWithFormat("Runtime error in objectDamaged(objectId: %d, damagedId: %d, script: %s).", _object->getObjectData()->getId(), data->getId(), data->getDamagedScript())->getCString();
				agtk::DebugManager::getInstance()->getDebugExecuteLogWindow()->addLog(errorStr);
#if defined(USE_PREVIEW)
				auto fp = GameManager::getScriptLogFp();
				if (fp) {
					fwrite(errorStr, 1, strlen(errorStr), fp);
					fwrite("\n", 1, 1, fp);
				}
#endif
				damagedRate = std::nan("1");
			}
			if (rv.isDouble()) {
				damagedRate = rv.toDouble();
			}
			else if (rv.isInt32()) {
				damagedRate = rv.toInt32();
			}
			else {
				//数値でない
				damagedRate = std::nan("1");
			}
		}
	}
	//クリティカル発生率
	double criticalRate = 0.0;
	if (attackObjectData->getCritical()) {
		auto variableCriticalIncidence = attackObject->getPlayObjectData()->getVariableData(agtk::data::kObjectSystemVariableCriticalIncidence);
		criticalRate = variableCriticalIncidence->getValue();
	}
	//被ダメージのクリティカル発生率の変更。
	if (data->getCritical() && attackObjectData->getCritical()) {
		criticalRate = data->getCriticalRate();
	}
	if (criticalRate > 100.0) criticalRate = 100.0;
	auto attackRate = attackObject->getVariableAttackRate();
	auto minAttackValue = (int)attackPlayObjectData->getVariableData(agtk::data::EnumObjectSystemVariable::kObjectSystemVariableMinimumAttack)->getValue();
	auto maxAttackValue = (int)attackPlayObjectData->getVariableData(agtk::data::EnumObjectSystemVariable::kObjectSystemVariableMaximumAttack)->getValue();
// #AGTK-NX
#if (CC_TARGET_PLATFORM == CC_PLATFORM_NX)
#else
	double attack = (minAttackValue + (maxAttackValue - minAttackValue + 1) * rand() / (1 + RAND_MAX)) * attackRate / 100.0;
#endif
	//クリティカルが発生する場合。
	if (criticalRate > 0.0 && AGTK_RANDOM(0.0, 100.0) <= criticalRate) {
		auto variableCriticalRatio = attackObject->getPlayObjectData()->getVariableData(agtk::data::kObjectSystemVariableCriticalRatio);
		attack *= variableCriticalRatio->getValue() * 0.01;
		bCriticalDamaged = true;
	}
	//被ダメージ
	if (takeOverDamageRate) {
		damage = attack * (getBaseParamDamageRate(damageRatePlayObjectData) * 0.01);
	}
	else {
		damage = attack * (damagedRate * 0.01) * (getBaseParamDamageRate(damageRatePlayObjectData) * 0.01);
	}
	return true;
}

bool ObjectDamaged::checkIgnored(agtk::Object *attackObject)
{
	//スイッチチェック
	auto playObjectData = _object->getPlayObjectData();
	auto data = this->getDamagedSettingData();
	bool bSwitch = false;
	if (data->getObjectSwitch()) {
		if (data->getObjectSwitchId() == -1) {//無し
			bSwitch = true;
		}
		else {
			auto switchData = playObjectData->getSwitchData(data->getObjectSwitchId());
			if (switchData != nullptr) {
				bSwitch = switchData->getValue();
			}
		}
	}
	else {
		auto projectPlayData = GameManager::getInstance()->getPlayData();
		if (data->getSystemSwitchId() == -1) {//無し
			bSwitch = true;
		}
		else {
			auto switchData = projectPlayData->getSwitchData(agtk::data::PlayData::COMMON_PLAY_DATA_ID, data->getSystemSwitchId());
			if (switchData != nullptr) {
				bSwitch = switchData->getValue();
			}
		}
	}
	if (bSwitch == false) {
		return true;
	}
	//属性チェック
	auto attackPlayObjectData = attackObject->getPlayObjectData();
	if (this->checkAttribute(attackPlayObjectData->getAttackAttribute()) == false) {
		return true;
	}
	return false;
}

//-------------------------------------------------------------------------------------------------------------------
ObjectTemplateMove::ObjectTemplateMove()
{
	_object = nullptr;
	_objCommand = nullptr;
	_state = kStateIdle;
	_moveFlip = false;
	_degree = 0.0f;
	_degreeOld = 0.0f;
	_boundColliedWallBit = 0;
	_fallFromStepFlag = false;
	_actionFrames = 0;
	_direction = cocos2d::Vec2::ZERO;
	_directionOld = cocos2d::Vec2::ZERO;
	_frameCount = 0.0;
	_frameCountMax = 0.0;
}

ObjectTemplateMove::~ObjectTemplateMove()
{
	CC_SAFE_RELEASE_NULL(_objCommand);
}

bool ObjectTemplateMove::init(agtk::Object *object)
{
	if (object == nullptr) {
		CC_ASSERT(0);
		return false;
	}
	this->_object = object;
	return true;
}

void ObjectTemplateMove::update(float delta)
{
	auto cmd = this->getObjCommand();
	if (cmd == nullptr) {
		return;
	}
	//if(delta > 0)	// ACT2-5101 delta == 0のときは状態を変化させない。（表示方向の変更等が反映されてしまわないように。）
	if((delta > 0 && this->getState() != kStateEnd)	// ACT2-5101 delta == 0のときは状態を変化させない。（表示方向の変更等が反映されてしまわないように。）
	|| this->getState() == kStateEnd)	// ACT2-5294 state=kStateEndの時の条件を追加。
	switch (cmd->getMoveType()) {
	case agtk::data::ObjectCommandTemplateMoveData::kMoveHorizontal: this->updateMoveHorizontal(delta); break;
	case agtk::data::ObjectCommandTemplateMoveData::kMoveVertical: this->updateMoveVertical(delta); break;
	case agtk::data::ObjectCommandTemplateMoveData::kMoveBound: this->updateMoveBound(delta); break;
	case agtk::data::ObjectCommandTemplateMoveData::kMoveRandom: this->updateMoveRandom(delta); break;
	case agtk::data::ObjectCommandTemplateMoveData::kMoveNearObject: this->updateMoveNearObject(delta); break;
	case agtk::data::ObjectCommandTemplateMoveData::kMoveApartNearObject: this->updateMoveApartNearObject(delta); break;
	case agtk::data::ObjectCommandTemplateMoveData::kMoveStop: this->updateMoveStop(delta); break;
	default:CC_ASSERT(0);
	}
	// 段差から落ちないフラグOFF
	this->setFallFromStepFlag(false);
	_actionFrames++;
}

void ObjectTemplateMove::start(agtk::data::ObjectCommandTemplateMoveData *objCommand)
{
	this->setObjCommand(objCommand);
	this->setState(kStateStart);
	this->resetMoveInfo();
	//バウンド時
	this->setBoundColliedWallBit(0);
	_actionFrames = 0;//アクションフレーム
	_direction = cocos2d::Vec2::ZERO;
	_directionOld = cocos2d::Vec2::ZERO;
	_frameCount = 0;
	_frameCountMax = 0;

	// テンプレート移動開始時、各動作別に設定が必要な場合
	switch (this->getObjCommand()->getMoveType()) {
	case agtk::data::ObjectCommandTemplateMoveData::kMoveHorizontal: {
		break; }
	case agtk::data::ObjectCommandTemplateMoveData::kMoveVertical: {
		break; }
	case agtk::data::ObjectCommandTemplateMoveData::kMoveBound: {
		// directionがリセットされていた場合は、再度設定を行う
		cocos2d::Vec2 direction = _object->getObjectMovement()->getDirection();
		if (direction == cocos2d::Vec2::ZERO) {
			// directionOldから取得
			direction = _object->getObjectMovement()->getDirectionOld();
		}
		// directionOldもない場合はInitialBulletLocusから再設定
		if (direction == cocos2d::Vec2::ZERO) {
			auto bullet = dynamic_cast<agtk::Bullet *>(_object);
			if (bullet != nullptr) {
				direction = bullet->getBulletLocus()->getInitialBulletLocus()->getDirection();
				_object->getObjectMovement()->setDirection(direction);
			}
		}
		break; }
	case agtk::data::ObjectCommandTemplateMoveData::kMoveRandom: {
		break; }
	case agtk::data::ObjectCommandTemplateMoveData::kMoveNearObject: {
		break; }
	case agtk::data::ObjectCommandTemplateMoveData::kMoveApartNearObject: {
		break; }
	case agtk::data::ObjectCommandTemplateMoveData::kMoveStop: {
		break; }
	}
	
	//強制移動中の場合はテンプレート移動へ切り替えるために処理を終了する。
	auto forceMove = _object->getObjectMovement()->getForceMove();
	if (forceMove && forceMove->isMoving()) {
		forceMove->end();
	}
}

void ObjectTemplateMove::end(bool bImmediate)
{
	if (this->getObjCommand() == nullptr) {
		//コマンドデータが無い。
		return;
	}
	this->setState(kStateEnd);
	if (bImmediate) {
		this->update(0.0);
	}
}

void ObjectTemplateMove::setMoveInfo(bool locked, cocos2d::Vec2 direction)
{
	_moveInfo.locked = locked;
	_moveInfo.direction = direction;
}

void ObjectTemplateMove::resetMoveInfo()
{
	_moveInfo.locked = false;
	_moveInfo.direction = cocos2d::Vec2::ZERO;
}

bool& ObjectTemplateMove::locked()
{
	return _moveInfo.locked;
}

cocos2d::Vec2& ObjectTemplateMove::direction()
{
	return _moveInfo.direction;
}

void ObjectTemplateMove::updateMoveHorizontal(float dt)
{
	auto cmd = this->getObjCommand();
// #AGTK-NX #AGTK-WIN
#if defined(USE_30FPS) && 0 // ObjectTemplateMove::getTimeScale()がdtを考慮するようになったため無効化
	float _dt = 5.0f * this->getTimeScale(dt) * dt * FRAME60_RATE;
#else
	float _dt = 5.0f * this->getTimeScale(dt);
#endif

	//_moveFlip: false:左, true:右

	// 時間が無限で設定されている場合タイマーを進めない
	if (cmd->getHorizontalInfinite()) {
		_dt = 0;
	}
	if (_frameCountMax > 0 || this->getState() == kStateIdle) {
		_frameCount += _dt;
	}

	switch (this->getState()) {
	case kStateIdle:
		break;
	case kStateStart: {
		_moveFlip = cmd->getHorizontalMoveStartRight();
		this->playDirectionMove(_moveFlip ? 90 : 270);
		this->setState(kStateExecute);
		_frameCount = 0;
		_frameCountMax = cmd->getHorizontalMoveDuration300();
		// 時間が無限で設定されている場合は固定値を入れる。
		if (cmd->getHorizontalInfinite()) {
			_frameCountMax = 1;
		}
		break; }
	case kStateExecute: {
		if(_frameCount + (FLT_EPSILON * _frameCount) >= _frameCountMax) {
			bool tmpMoveFlip = _moveFlip;
			_moveFlip = !_moveFlip;
			if (!this->playDirectionMove(_moveFlip ? 90 : 270)) {
				// 方向転換しなかった場合は_moveFlipを元に戻す
				_moveFlip = tmpMoveFlip;
			}
			_frameCount = 0;
		}
		break; }
	case kStateEnd: {
		_object->getObjectMovement()->resetDirectionForce();//移動強制OFF
		this->setObjCommand(nullptr);
		this->setState(kStateIdle);
		_frameCount = 0;
		_frameCountMax = 0;
		break; }
	}
}

void ObjectTemplateMove::updateMoveVertical(float dt)
{
	auto cmd = this->getObjCommand();
	// #AGTK-NX #AGTK-WIN
#if defined(USE_30FPS) && 0 // ObjectTemplateMove::getTimeScale()がdtを考慮するようになったため無効化
	dt = 5.0f * this->getTimeScale(dt) * dt * FRAME60_RATE;
#else
	dt = 5.0f * this->getTimeScale(dt);
#endif

	//_moveFlip: false:上, true:下

	// 時間が無限で設定されている場合タイマーを進めない
	if (cmd->getVerticalInfinite()) {
		dt *= 0;
	}
	if (_frameCountMax > 0 || this->getState() != kStateIdle) {
		_frameCount += dt;
	}

	switch (this->getState()) {
	case kStateIdle:
		break;
	case kStateStart: {
		_moveFlip = cmd->getVerticalMoveStartDown();
		this->playDirectionMove(_moveFlip ? 180 : 0);
		this->setState(kStateExecute);
		_frameCount = 0;
		_frameCountMax = (double)cmd->getVerticalMoveDuration300();
		_oldCollied.wall.up = _object->getTileWallBit() & agtk::data::ObjectActionLinkConditionData::kWallBitUp;
		_oldCollied.wall.down = _object->getTileWallBit() & agtk::data::ObjectActionLinkConditionData::kWallBitDown;
		// 時間が無限で設定されている場合は固定値を入れる。
		if (cmd->getVerticalInfinite()) {
			_frameCountMax = 1;
		}
		break; }
	case kStateExecute: {
		auto objectMovement = _object->getObjectMovement();
		bool bUpWallCollied = _object->getTileWallBit() & agtk::data::ObjectActionLinkConditionData::kWallBitUp;
		bool bDownWallCollied = _object->getTileWallBit() & agtk::data::ObjectActionLinkConditionData::kWallBitDown;
		bool bUpObjectCollied = (_object->getUpWallObjectList()->count() > 0) ? true : false;
		bool bDownObjectCollied = (_object->getDownWallObjectList()->count() > 0) ? true : false;
		//押し戻されない場合。
		if (!_object->getPushedbackByObject() || !objectMovement->isMoveLift()) {
			bDownObjectCollied = false;
			bUpObjectCollied = false;
		}
		if (_frameCount + (FLT_EPSILON * _frameCount) >= _frameCountMax) {
			bool tmpMoveFlip = _moveFlip;
			if ((bUpWallCollied && !_oldCollied.wall.up) || (bUpObjectCollied && !objectMovement->isMoveLift(_object->getUpWallObjectList()))) {
				_moveFlip = true;
			}
			else if ((bDownWallCollied && !_oldCollied.wall.down) || bDownObjectCollied) {
				_moveFlip = false;
			}
			else {
				_moveFlip = !_moveFlip;
			}
			float degree = _moveFlip ? 180.0f : 0.0f;
			if (!this->playDirectionMove(degree)) {
				// 方向転換しなかった場合は_moveFlipを元に戻す
				_moveFlip = tmpMoveFlip;
			}
			_frameCount = 0;
		}
		_oldCollied.wall.up = bUpWallCollied;
		_oldCollied.wall.down = bDownWallCollied;
		break; }
	case kStateEnd: {
		_object->getObjectMovement()->resetDirectionForce();//移動強制OFF
		this->setObjCommand(nullptr);
		this->setState(kStateIdle);
		_frameCount = 0;
		_frameCountMax = 0;
		break; }
	}
}

cocos2d::Vec2 ObjectTemplateMove::getDirection()
{
	return _direction;
}

cocos2d::Vec2 ObjectTemplateMove::setDirection(cocos2d::Vec2 v)
{
	_directionOld = _direction;
	_direction = v;
	return _directionOld;
}

float ObjectTemplateMove::getDegree()
{
	return _degree;
}

float ObjectTemplateMove::setDegree(float degree)
{
	_degreeOld = _degree;
	_degree = degree;
	return _degreeOld;
}

void ObjectTemplateMove::updateMoveBound(float dt)
{
	auto cmd = this->getObjCommand();

	std::function<float(agtk::Object *)> getDegree = [&](agtk::Object *object) {
		float degree = 0.0f;
		cocos2d::Vec2 direction = object->getObjectMovement()->getDirection();
		if (direction == cocos2d::Vec2::ZERO) {
			direction = object->getObjectMovement()->getDirectionOld();
		}
		if (direction != cocos2d::Vec2::ZERO) {
			//方向がある場合、開始方向とする。
			degree = agtk::GetDegreeFromVector(direction);
		}
		//移動方向
		else if (object->getMoveDirectionBit()) {
			auto v = agtk::GetDirectionFromMoveDirectionId(object->getMoveDirection());
			degree = agtk::GetDegreeFromVector(v);
		}
		else {
			//方向がない場合、ランダムに開始方向を決める。
			static float degreeList[] = { 0, 45, 90, 135, 180, 225, 270, 315, };
			int degreeId = AGTK_RANDOM(0, CC_ARRAYSIZE(degreeList) - 1);
			CC_ASSERT(degreeId < CC_ARRAYSIZE(degreeList));
			degree = degreeList[degreeId];
		}
		return degree;
	};

	switch (this->getState()) {
	case kStateIdle:
		break;
	case kStateStart: {
		auto degree = getDegree(_object);
		this->setDegree(degree);
		this->setDirection(agtk::GetDirectionFromDegrees(this->getDegree()));
		this->playDirectionMove(degree, true);
		this->setBoundColliedWallBit(_object->getTileWallBit());
		this->setState(kStateExecute);
		break; }
	case kStateExecute: {
#ifdef USE_MULTITHREAD_OBJECT_UPDATE
		bool bCollied = (_object->getLeftWallObjectList()->size() || _object->getRightWallObjectList()->size() || _object->getUpWallObjectList()->size() || _object->getDownWallObjectList()->size() || _object->getTileWallBit());
#else
		bool bCollied = (_object->getLeftWallObjectList()->count() || _object->getRightWallObjectList()->count() || _object->getUpWallObjectList()->count() || _object->getDownWallObjectList()->count() || _object->getTileWallBit());
#endif

		auto bit = _object->getTileWallBit();
		auto movement = _object->getObjectMovement();
		if (bCollied && (movement->getMoveX() == 0.0f || movement->getMoveY() == 0.0f)) {
			//移動しない状態の場合。
			auto degree = getDegree(_object);
			this->setDegree(degree);
			this->setDirection(agtk::GetDirectionFromDegrees(this->getDegree()));
			this->setBoundColliedWallBit(0);
		}
		else if (bit > 0 && this->getBoundColliedWallBit() > 0) {
			// 前のフレームでバウンドし、現フレームでもバウンドする場合
			if (bit != this->getBoundColliedWallBit()) {
				// 前回のフレームで接触した壁方向とは違う場合はバウンドを行う
				this->setBoundColliedWallBit(0);
			}
		}

		if (bCollied && this->getBoundColliedWallBit() == 0) {
			bool bHorizontal = false;
			bool bVertical = false;
			auto v = this->getDirection();
			auto vOld = v;
			//左
			if (v.x < 0 && (_object->getLeftWallObjectList()->count() || (bit & agtk::data::ObjectActionLinkConditionData::kWallBitLeft))) {
				v.x = -v.x;
			}
			//右
			else if (v.x > 0 && (_object->getRightWallObjectList()->count() || (bit & agtk::data::ObjectActionLinkConditionData::kWallBitRight))) {
				v.x = -v.x;
			}
			//下
			if (v.y < 0 && (_object->getDownWallObjectList()->count() || (bit & agtk::data::ObjectActionLinkConditionData::kWallBitDown))) {
				v.y = -v.y;
			}
			//上
			else if (v.y > 0 && (_object->getUpWallObjectList()->count() || (bit & agtk::data::ObjectActionLinkConditionData::kWallBitUp))) {
				v.y = -v.y;
			}
			//段差から落ちないフラグONの場合。
			if (this->getFallFromStepFlag() && v == vOld) {
				v.x = -v.x;
				v.y = -v.y;
			}
			auto degree = agtk::GetDegreeFromVector(v);
			this->setDegree(degree);
			this->setDirection(v);
			this->playDirectionMove(this->getDegree(), true);
			this->setBoundColliedWallBit(bit);
		}
		else {
			this->setBoundColliedWallBit(0);
		}
		break; }
	case kStateEnd: {
		this->resetDirection();
		this->setDegree(0.0f);
		this->setDirection(cocos2d::Vec2::ZERO);
		this->setObjCommand(nullptr);
		this->setState(kStateIdle);
		break; }
	}
}

void ObjectTemplateMove::updateMoveRandom(float dt)
{
	static float degreeList[] = { 0, 45, 90, 135, 180, 225, 270, 315, };
	//ランダム移動（移動を停止を繰り返す）
	auto cmd = this->getObjCommand();
// #AGTK-NX #AGTK-WIN
#if defined(USE_30FPS) && 0 // ObjectTemplateMove::getTimeScale()がdtを考慮するようになったため無効化
	dt = 5.0f * this->getTimeScale(dt) * dt * FRAME60_RATE;
#else
	dt = 5.0f * this->getTimeScale(dt);
#endif

	//_moveFlip: false:停止, true:移動

	if (_frameCountMax > 0 || this->getState() != kStateIdle) {
		_frameCount += dt;
	}

	switch (this->getState()) {
	case kStateIdle:
		break;
	case kStateStart: {
		int degreeId = AGTK_RANDOM(0, CC_ARRAYSIZE(degreeList) - 1);
		CC_ASSERT(degreeId < CC_ARRAYSIZE(degreeList));
		this->playDirectionMove(degreeList[degreeId]);
		_moveFlip = false;
		this->setState(kStateExecute);
		_frameCount = 0;
		_frameCountMax = (double)cmd->getRandomMoveDuration300();
		break; }
	case kStateExecute: {
		if (_frameCount + (FLT_EPSILON * _frameCount) >= _frameCountMax) {
			if (_moveFlip) {//移動
				int degreeId = AGTK_RANDOM(0, CC_ARRAYSIZE(degreeList) - 1);
				CC_ASSERT(degreeId < CC_ARRAYSIZE(degreeList));
				this->playDirectionMove(degreeList[degreeId]);
				_frameCountMax = (double)cmd->getRandomMoveDuration300();
			}
			else {//停止
				this->resetDirection();
				_frameCountMax = (double)cmd->getRandomMoveStop300();
			}
			_moveFlip = !_moveFlip;
			_frameCount = 0;
		}
		break; }
	case kStateEnd: {
		this->resetDirection();
		this->setObjCommand(nullptr);
		this->setState(kStateIdle);
		_frameCount = 0;
		_frameCountMax = 0;
		break; }
	}
}

void ObjectTemplateMove::updateMoveNearObject(float dt)
{
	//近くのオブジェクトへ移動
	auto cmd = this->getObjCommand();

	switch (this->getState()) {
	case kStateIdle:
		break;
	case kStateStart: {
		this->setState(kStateExecute);
		break; }
	case kStateExecute: {
		//近くのオブジェクト
		auto targetObject = this->getNearObject(cmd->getNearObjectGroup(), cmd->getNearObjectLockedObjectPrior());
		if (targetObject == nullptr) {
			this->setState(kStateEnd);
			break;
		}
		auto targetMvPos = targetObject->getCenterPosition();
		auto myselfMvPos = _object->getCenterPosition();
		targetMvPos += Object::getSceneLayerScrollDiff(_object->getSceneLayer(), targetObject->getSceneLayer());
		auto targetPos = agtk::Scene::getPositionCocos2dFromScene(targetMvPos);
		auto myselfPos = agtk::Scene::getPositionCocos2dFromScene(myselfMvPos);
		float degree = GetDegreeFromVector(targetPos - myselfPos);
		this->playDirectionMove(degree);
		break; }
	case kStateEnd: {
		this->resetDirection();
		this->setObjCommand(nullptr);
		this->setState(kStateIdle);
		break; }
	}
}

void ObjectTemplateMove::updateMoveApartNearObject(float dt)
{
	//近くのオブジェクトから離れる
	auto cmd = this->getObjCommand();

	switch (this->getState()) {
	case kStateIdle:
		break;
	case kStateStart: {
		this->setState(kStateExecute);
		break; }
	case kStateExecute: {
		auto targetObject = this->getNearObject(cmd->getApartNearObjectGroup(), cmd->getApartNearObjectLockedObjectPrior());
		if (targetObject == nullptr) {
			this->setState(kStateEnd);
			break;
		}
		auto targetMvPos = targetObject->getCenterPosition();
		auto myselfMvPos = _object->getCenterPosition();
		targetMvPos += Object::getSceneLayerScrollDiff(_object->getSceneLayer(), targetObject->getSceneLayer());
		auto targetPos = agtk::Scene::getPositionCocos2dFromScene(targetMvPos);
		auto myselfPos = agtk::Scene::getPositionCocos2dFromScene(myselfMvPos);
		float degree = GetDegreeFromVector(myselfPos - targetPos);
		this->playDirectionMove(degree);
		break; }
	case kStateEnd: {
		this->resetDirection();
		this->setObjCommand(nullptr);
		this->setState(kStateIdle);
		break; }
	}
}

void ObjectTemplateMove::updateMoveStop(float dt)
{
	auto cmd = this->getObjCommand();

	switch (this->getState()) {
	case kStateIdle:
		break;
	case kStateStart: {
		this->setState(kStateEnd);
		}
	//case kStateExecute: {
	//	break; }
	case kStateEnd: {
		this->resetDirection(true);
		this->setObjCommand(nullptr);
		this->setState(kStateIdle);
		break; }
	}
}

bool ObjectTemplateMove::playDirectionMove(double degree, bool bIgnoredRequestPlayAction)
{
	auto direction = agtk::GetDirectionFromDegrees(degree);
	int moveDirectionId = agtk::GetMoveDirectionId(degree);
	if (moveDirectionId != _object->getMoveDirection()) {
		if (_actionFrames == 1) {
			//※playActionが実行して１フレーム後にリンク参照が行われます。すぐに切り替わるとリンク参照が行われないため、
			//１フレームは必ず実行して、リンク参照を行えるように処理を施す。
			return false;
		}
		int actionId = _object->getCurrentObjectAction()->getId();
		int preActionId = _object->getPrevObjectActionId();
		_object->playActionTemplateMove(actionId, moveDirectionId);
		if (bIgnoredRequestPlayAction) {
			_object->_bRequestPlayActionTemplateMove = 0;
		}
		_actionFrames = 0;
	}
	auto objectMovement = _object->getObjectMovement();
	objectMovement->setDirectionForce(direction);//強制移動設定。
	auto moveType = _object->getObjectData()->getMoveType();
	if (moveType == agtk::data::ObjectData::kMoveCar || moveType == agtk::data::ObjectData::kMoveTank) {
		objectMovement->setInputDirectionForce(cocos2d::Vec2(0, 1));
	}
	return true;
}

void ObjectTemplateMove::resetDirection(bool bResetDirection)
{
	auto objectMovement = _object->getObjectMovement();
	objectMovement->resetDirectionForce();//移動強制OFF
	auto moveType = _object->getObjectData()->getMoveType();
	if (moveType == agtk::data::ObjectData::kMoveCar || moveType == agtk::data::ObjectData::kMoveTank) {
		objectMovement->resetInputDirectionForce();
	}
	if (bResetDirection) {
		//移動方向をリセットする。
		objectMovement->setDirection(cocos2d::Vec2::ZERO);
	}
}

bool ObjectTemplateMove::isIgnoredObjectWall()
{
	auto objCommand = this->getObjCommand();
	if (objCommand == nullptr) {
		return false;
	}
	return objCommand->getIgnoreOtherObjectWallArea();
}

bool ObjectTemplateMove::isIgnoredTileWall()
{
	auto objCommand = this->getObjCommand();	
	if (objCommand == nullptr) {
		return false;
	}
	return objCommand->getIgnoreWall();
}

/**
* テンプレート移動のバウンドタイプか？
* @return	テンプレート移動：バウンドタイプであれば True
*/
bool ObjectTemplateMove::isBoundMove()
{
	auto cmd = this->getObjCommand();

	return (cmd && cmd->getMoveType() == agtk::data::ObjectCommandTemplateMoveData::kMoveBound);
}

agtk::data::ObjectCommandTemplateMoveData::EnumMoveType ObjectTemplateMove::getMoveType()
{
	auto cmd = this->getObjCommand();
	if (cmd) {
		return cmd->getMoveType();
	}
	return agtk::data::ObjectCommandTemplateMoveData::EnumMoveType::kMoveMax;
}

agtk::SceneLayer *ObjectTemplateMove::getSceneLayer()
{
	CC_ASSERT(_object);
	return _object->getSceneLayer();
}

agtk::Object *ObjectTemplateMove::getNearObject(int objectGroup, bool lock)
{
	//近くのオブジェクトへ移動
	struct ObjectSortInfo {
		agtk::Object *_object;
		bool _lock;
		float _distance;
		ObjectSortInfo(agtk::Object *targetObject, agtk::Object *object) {
			_object = targetObject;
			_distance = object->getCenterPosition().getDistance(targetObject->getCenterPosition());
			auto playObjectData = targetObject->getPlayObjectData();
			_lock = playObjectData->isLocked(object->getInstanceId());
		}
	};
	//近くのオブジェクト
	auto scene = GameManager::getInstance()->getCurrentScene();
	cocos2d::__Array *objectList;
	if (objectGroup == agtk::data::ObjectData::kObjGroupAll) {
// #AGTK-NX
#ifdef USE_SAR_OPTIMIZE_1
		objectList = scene->getObjectAllReference(this->getSceneLayer()->getType());
#else
		objectList = scene->getObjectAll(this->getSceneLayer()->getType());
#endif
	}
	else {
		objectList = scene->getObjectAllObjGroup((agtk::data::ObjectData::EnumObjGroup)objectGroup, false, this->getSceneLayer()->getType());
	}
	if (objectList->count() == 0) {
		return nullptr;
	}
	if (objectList->count() == 1) {
		return dynamic_cast<agtk::Object *>(objectList->getObjectAtIndex(0));
	}
	cocos2d::Ref *ref;
	std::vector<ObjectSortInfo *> sortList;
	CCARRAY_FOREACH(objectList, ref) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
		auto object = static_cast<agtk::Object *>(ref);
#else
		auto object = dynamic_cast<agtk::Object *>(ref);
#endif
		if (object == _object) continue;
		auto p = new ObjectSortInfo(object, _object);
		sortList.push_back(p);
	}

	std::function<bool(ObjectSortInfo*, ObjectSortInfo*)> compare = [lock](ObjectSortInfo *p1, ObjectSortInfo *p2) {
		if (lock && p1->_lock != p2->_lock) {
			if (p1->_lock == lock) {
				return true;
			}
			else {
				return false;
			}
		}
		if (p1->_distance < p2->_distance) {
			return true;
		}
		return false;
	};
	std::sort(sortList.begin(), sortList.end(), compare);

	auto targetObject = sortList.front()->_object;

	//メモリ破棄
	for (auto p : sortList) {
		delete p;
	}
	return targetObject;
}

float ObjectTemplateMove::getTimeScale(float dt)
{
	float timeScale = _object->getTimeScale();
	if (timeScale > 0.0f) {
		timeScale *= (dt * FRAME60_RATE) / timeScale;
	}
	return timeScale;
}

//-------------------------------------------------------------------------------------------------------------------
ObjectCourseMove::ObjectCourseMove()
{
	_object = nullptr;
	_course = nullptr;

	_moveDist = 0;
	_currentPointId = -1;
	_currentPointIdx = -1;
	_loopCount = 0;
	_reverseMove = false;
	_reverseCourse = false;
	_isFirst = true;
	_moving = false;
	_move = Vec2::ZERO;
}

ObjectCourseMove::~ObjectCourseMove()
{
	CC_SAFE_RELEASE_NULL(_course);
}

bool ObjectCourseMove::init(agtk::Object *object, int courseId, int coursePointId, agtk::Scene *scene)
{
	this->_object = object;

	// コースの取得を行う
	//auto scene = GameManager::getInstance()->getCurrentScene();
	if (scene == nullptr) {
		CC_ASSERT(0);
		return false;
	}
	auto course = scene->getOthersCourse(courseId);

	if (course != nullptr) {
		// コースの情報を設定
		setCourse(course);
		// コースの開始ポイントを設定
		setCurrentPointId(coursePointId);
	}

	return true;
}

bool ObjectCourseMove::init(agtk::CameraObject *object, int courseId, int coursePointId, agtk::Scene *scene)
{
	this->_object = nullptr;
	
	//auto scene = GameManager::getInstance()->getCurrentScene();
	if (scene == nullptr) {
		CC_ASSERT(0);
		return false;
	}

	// コースの取得を行う
	auto course = scene->getOthersCourse(courseId);

	if (course != nullptr) {
		// コースの情報を設定
		setCourse(course);
		// コースの開始ポイントを設定
		setCurrentPointId(coursePointId);
	}

	return true;
}


void ObjectCourseMove::update(float dt)
{
	// GameScene.cppの起動時に１フレームスキップでdtが0でくる場合、
	// moveValが0となる&初回起動フラグが変わるので処理しないようにする
	if (_course != nullptr && dt != 0.0f) {
		_moving = true;
		float timeScale = 1.0f;

		if (_object != nullptr) {
			timeScale = _object->getTimeScale();
		}
		timeScale *= (FRAME60_RATE * dt);

		// 初回移動フラグを保持
		bool isFirst = _isFirst;
		bool isReset = false;

		// コース上を移動する
		_currentPos = _course->moveCourse(this, timeScale, isReset);

		if (_object != nullptr) {
			// 初回移動時あるいは座標のリセットが発生した場合
			if (isFirst || isReset) {
				// 座標をそのまま設定
				_object->setPosition(agtk::Scene::getPositionCocos2dFromScene(_currentPos));
				_object->setOldPosition(agtk::Scene::getPositionCocos2dFromScene(_currentPos));
			}

			_move = agtk::Scene::getPositionSceneFromCocos2d(_currentPos) - _object->getPosition();
			_move.y *= -1;
		}
	}
	else {
		_moving = false;
	}
}

void ObjectCourseMove::reset()
{
	_moveDist = 0;
	_currentPointIdx = -1;
	_loopCount = 0;
	_reverseMove = false;
	_reverseCourse = false;
	_isFirst = true;
	if (_course) {
		_course->setPausedBySwitch(true);
	}
}

/**
* コース移動設定(ロードデータがあれば上書き)
*/
void ObjectCourseMove::setup(const agtk::Object *object)
{
	auto courseMoveLoadData = object->getCourseMoveLoadData();

	_isFirst = courseMoveLoadData->getIsFirst();
	_moveDist = courseMoveLoadData->getMoveDist();
	_currentPointId = courseMoveLoadData->getCurrentPointId();
	_currentPointIdx = courseMoveLoadData->getCurrentPointIdx();
	_loopCount = courseMoveLoadData->getLoopCount();
	_reverseMove = courseMoveLoadData->getReverseMove();
	_reverseCourse = courseMoveLoadData->getReverseCourse();
	_move = courseMoveLoadData->getMove();
	_currentPos = courseMoveLoadData->getCurrentPos();
}

/**
* 開始ポイント座標の取得
*/
Vec2 ObjectCourseMove::getStartPointPos()
{
	Vec2 pos = Vec2::ZERO;

	if (_course != nullptr) {
		pos = _course->getStartPointPos(this);
	}
	return pos;
}

//-------------------------------------------------------------------------------------------------------------------
ObjectLoopMove::ObjectLoopMove()
{
	_moveDist = 0.0f;
	_currentPointIdx = 0;
	_reverseMove = false;
	_rotation = 0.0f;
	_moveSpeed = 0.0f;
	_moveVelocity = Vec2::ZERO;

	_object = nullptr;
	_course = nullptr;
}

ObjectLoopMove::~ObjectLoopMove()
{
}

bool ObjectLoopMove::init(agtk::Object* object)
{
	_object = object;

	return true;
}

void ObjectLoopMove::update(float delta)
{
	if (_course != nullptr) {
		float timeScale = 1.0f;
		ObjectMovement* objectMovement = nullptr;
		bool normalAccelMove = false;

		if (_object != nullptr) {
			timeScale = _object->getTimeScale();
			objectMovement = _object->getObjectMovement();
			normalAccelMove = _object->getObjectData()->getNormalAccelMove();
		}

// #AGTK-NX #AGTK-WIN
#if defined(USE_30FPS)
		timeScale *= delta * FRAME60_RATE;
#endif

		float courseRot = 0.0f;
		bool isFinish = false;
		float moveSpeed = _moveSpeed;

		// 自動移動出ない場合
		if (!_course->getLoopData()->getMoveToEnd()) {
			// 加速移動を行う場合
			if (_object != nullptr && normalAccelMove) {
				// 
				float x = abs(_moveVelocity.x);
				if (x > 0) {
					moveSpeed *= abs(objectMovement->getMoveVelocity().x) / x;
				}
			}
		}

		// コース上を移動する
		_currentPos = _course->moveCourse(this, timeScale, moveSpeed, courseRot, isFinish);

		if (_object != nullptr) {
			// コースの向きに応じてオブジェクトの向きを変える
			_object->setRotation(_rotation + courseRot);

			// 座標を更新
			_object->setPosition(agtk::Scene::getPositionCocos2dFromScene(_currentPos));
		}

		// 自動移動でない場合
		if (!_course->getLoopData()->getMoveToEnd()) {

			//         上
			//    +---+---+---+
			//    | 7 | 8 | 9 |
			//    +---+---+---+
			// 左 | 4 |   | 6 | 右
			//    +---+---+---+
			//    | 1 | 2 | 3 |
			//    +---+---+---+
			//         下

			int directionId = _object->getInputDirectionId();

			// 右移動の場合
			if (_isMoveRight) {
				// 右方向への入力がない場合はループ終了
				if (!(directionId == 3 || directionId == 6 || directionId == 9)) {
					isFinish = true;

					// 加速移動を行う場合
					if (normalAccelMove && objectMovement != nullptr) {
						objectMovement->setMoveVelocity(Vec2::ZERO);
					}
				}
			}
			// 左移動の場合
			else {
				// 左方向への入力がない場合はループ終了
				if (!(directionId == 1 || directionId == 4 || directionId == 7)) {
					isFinish = true;

					// 加速移動を行う場合
					if (normalAccelMove && objectMovement != nullptr) {
						objectMovement->setMoveVelocity(Vec2::ZERO);
					}
				}
			}
		}
		// 自動移動の場合
		else {
			// 慣性移動を行うために値を設定する
			if (_object != nullptr && objectMovement != nullptr) {
				objectMovement->setMoveVelocity(_moveVelocity);
			}
		}

		// コース移動を終了した場合
		if (isFinish) {
			finishMove();
		}
	}
}

void ObjectLoopMove::startMove(agtk::OthersLoopCourse* course, bool isReverse)
{
	_course = course;
	_reverseMove = isReverse;
	_rotation = _object->getRotation();

	auto vec = _object->getOldPosition() - _object->getPosition();
	auto objectMovement = _object->getObjectMovement();

	_moveSpeed = vec.getLength();// objectData->getHorizontalMove() + _object->getObjectMovement()->getWallMoveSpeed()->get();

	_moveDist = !isReverse ? 0 : course->getLength();

	_isMoveRight = !isReverse ? _course->getMoveToRightEnter() : _course->getMoveToRightExit();

	// 自動移動の場合
	if (_course->getLoopData()->getMoveToEnd()) {
		// 現在再生中のアニメを再生し続ける
		auto player = _object->getPlayer();
		if (player != nullptr) {
			player->continuePlayCurrentAnime();
		}
	}

	// 移動完了後に慣性移動をさせるために移動量を取得
	if (objectMovement != nullptr) {
		_moveVelocity = _object->getObjectMovement()->getMoveVelocity();
	}
}

void ObjectLoopMove::finishMove()
{
	// オブジェクトの向きを戻す
	_object->setRotation(_rotation);

	// 自動移動の場合
	if (_course->getLoopData()->getMoveToEnd()) {
		// 再生し続けているアニメを停止させる
		auto player = _object->getPlayer();
		if (player != nullptr) {
			player->stopAnimeContinuePlaying();
		}
	}

	_course = nullptr;
}

bool ObjectLoopMove::getMoving()
{
	return _course != nullptr;
}

bool ObjectLoopMove::getMovingAuto()
{
	// 移動中の場合
	if (getMoving()) {
		// 「ループ終了まで自動移動」にチェックがついていれば自動移動中である
		return _course->getLoopData()->getMoveToEnd();
	}

	return false;
}

//-------------------------------------------------------------------------------------------------------------------
ObjectWallIntersect::ObjectWallIntersect()
{
	_wallType = kWallTypeMax;
}

bool ObjectWallIntersect::init(EnumWallType wallType, cocos2d::Vec2 point)
{
	_wallType = wallType;
	_point = point;
	return true;
}

int ObjectWallIntersect::getWallBit()
{
	return 1 << _wallType;
}

bool ObjectWallIntersect::getWallIntersect(cocos2d::Rect r1, cocos2d::Rect r2, cocos2d::__Array *wallIntersectList1, cocos2d::__Array *wallIntersectList2)
{
	if (r1.intersectsRect(r2) || r2.intersectsRect(r1)) {
		//r1
		agtk::Line up1(cocos2d::Vec2(r1.origin.x, r1.origin.y + r1.size.height), cocos2d::Vec2(r1.origin.x + r1.size.width, r1.origin.y + r1.size.height));
		agtk::Line left1(cocos2d::Vec2(r1.origin.x, r1.origin.y), cocos2d::Vec2(r1.origin.x, r1.origin.y + r1.size.height));
		agtk::Line right1(cocos2d::Vec2(r1.origin.x + r1.size.width, r1.origin.y), cocos2d::Vec2(r1.origin.x + r1.size.width, r1.origin.y + r1.size.height));
		agtk::Line down1(cocos2d::Vec2(r1.origin.x, r1.origin.y), cocos2d::Vec2(r1.origin.x + r1.size.width, r1.origin.y));
		//r2
		agtk::Line up2(cocos2d::Vec2(r2.origin.x, r2.origin.y + r2.size.height), cocos2d::Vec2(r2.origin.x + r2.size.width, r2.origin.y + r2.size.height));
		agtk::Line left2(cocos2d::Vec2(r2.origin.x, r2.origin.y), cocos2d::Vec2(r2.origin.x, r2.origin.y + r2.size.height));
		agtk::Line right2(cocos2d::Vec2(r2.origin.x + r2.size.width, r2.origin.y), cocos2d::Vec2(r2.origin.x + r2.size.width, r2.origin.y + r2.size.height));
		agtk::Line down2(cocos2d::Vec2(r2.origin.x, r2.origin.y), cocos2d::Vec2(r2.origin.x + r2.size.width, r2.origin.y));

		if (wallIntersectList1) {
			bool bUp = false;
			bool bDown = false;
			bool bLeft = false;
			bool bRight = false;
			//up ------------------------------------------
			cocos2d::Vec2 p;
			p = agtk::GetIntersectPoint(up1, left2);
			if (p != cocos2d::Vec2::ZERO) {
				wallIntersectList1->addObject(agtk::ObjectWallIntersect::create(kWallTypeUp, p));
				bUp = true;
			}
			p = agtk::GetIntersectPoint(up1, right2);
			if (p != cocos2d::Vec2::ZERO) {
				wallIntersectList1->addObject(agtk::ObjectWallIntersect::create(kWallTypeUp, p));
				bUp = true;
			}
			//left ----------------------------------------
			p = agtk::GetIntersectPoint(left1, up2);
			if (p != cocos2d::Vec2::ZERO) {
				wallIntersectList1->addObject(agtk::ObjectWallIntersect::create(kWallTypeLeft, p));
				bLeft = true;
			}
			p = agtk::GetIntersectPoint(left1, down2);
			if (p != cocos2d::Vec2::ZERO) {
				wallIntersectList1->addObject(agtk::ObjectWallIntersect::create(kWallTypeLeft, p));
				bLeft = true;
			}
			//right ---------------------------------------
			p = agtk::GetIntersectPoint(right1, up2);
			if (p != cocos2d::Vec2::ZERO) {
				wallIntersectList1->addObject(agtk::ObjectWallIntersect::create(kWallTypeRight, p));
				bRight = true;
			}
			p = agtk::GetIntersectPoint(right1, down2);
			if (p != cocos2d::Vec2::ZERO) {
				wallIntersectList1->addObject(agtk::ObjectWallIntersect::create(kWallTypeRight, p));
				bRight = true;
			}
			//down ----------------------------------------
			p = agtk::GetIntersectPoint(down1, left2);
			if (p != cocos2d::Vec2::ZERO) {
				wallIntersectList1->addObject(agtk::ObjectWallIntersect::create(kWallTypeDown, p));
				bDown = true;
			}
			p = agtk::GetIntersectPoint(down1, right2);
			if (p != cocos2d::Vec2::ZERO) {
				wallIntersectList1->addObject(agtk::ObjectWallIntersect::create(kWallTypeDown, p));
				bDown = true;
			}
			if (bLeft && bRight) {
				//上、下で埋まっている位置を調べる。
				if (r2.containsPoint(left1.v) && r2.containsPoint(right1.v)) {//上
					wallIntersectList1->addObject(agtk::ObjectWallIntersect::create(kWallTypeUp, p));
				}
				if (r2.containsPoint(left1.p) && r2.containsPoint(right1.p)) {//下
					wallIntersectList1->addObject(agtk::ObjectWallIntersect::create(kWallTypeDown, p));
				}
			}
			if (bUp && bDown) {
				//左、右で埋まっている位置を調べる。
				if (r2.containsPoint(up1.p) && r2.containsPoint(down1.p)) {//左
					wallIntersectList1->addObject(agtk::ObjectWallIntersect::create(kWallTypeLeft, p));
				}
				if (r2.containsPoint(up1.v) && r2.containsPoint(up1.v)) {//右
					wallIntersectList1->addObject(agtk::ObjectWallIntersect::create(kWallTypeRight, p));
				}
			}
		}
		if (wallIntersectList2) {
			bool bUp = false;
			bool bDown = false;
			bool bLeft = false;
			bool bRight = false;
			//up ------------------------------------------
			cocos2d::Vec2 p;
			p = agtk::GetIntersectPoint(up2, left1);
			if (p != cocos2d::Vec2::ZERO) {
				wallIntersectList2->addObject(agtk::ObjectWallIntersect::create(kWallTypeUp, p));
				bUp = true;
			}
			p = agtk::GetIntersectPoint(up2, right1);
			if (p != cocos2d::Vec2::ZERO) {
				wallIntersectList2->addObject(agtk::ObjectWallIntersect::create(kWallTypeUp, p));
				bUp = true;
			}
			//left ----------------------------------------
			p = agtk::GetIntersectPoint(left2, up1);
			if (p != cocos2d::Vec2::ZERO) {
				wallIntersectList2->addObject(agtk::ObjectWallIntersect::create(kWallTypeLeft, p));
				bLeft = true;
			}
			p = agtk::GetIntersectPoint(left2, down1);
			if (p != cocos2d::Vec2::ZERO) {
				wallIntersectList2->addObject(agtk::ObjectWallIntersect::create(kWallTypeLeft, p));
				bLeft = true;
			}
			//right ---------------------------------------
			p = agtk::GetIntersectPoint(right2, up1);
			if (p != cocos2d::Vec2::ZERO) {
				wallIntersectList2->addObject(agtk::ObjectWallIntersect::create(kWallTypeRight, p));
				bRight = true;
			}
			p = agtk::GetIntersectPoint(right2, down1);
			if (p != cocos2d::Vec2::ZERO) {
				wallIntersectList2->addObject(agtk::ObjectWallIntersect::create(kWallTypeRight, p));
				bRight = true;
			}
			//down ----------------------------------------
			p = agtk::GetIntersectPoint(down2, left1);
			if (p != cocos2d::Vec2::ZERO) {
				wallIntersectList2->addObject(agtk::ObjectWallIntersect::create(kWallTypeDown, p));
				bDown = true;
			}
			p = agtk::GetIntersectPoint(down2, right1);
			if (p != cocos2d::Vec2::ZERO) {
				wallIntersectList2->addObject(agtk::ObjectWallIntersect::create(kWallTypeDown, p));
				bDown = true;
			}
			if (bLeft && bRight) {
				//上、下で埋まっている位置を調べる。
				if (r1.containsPoint(left2.v) && r1.containsPoint(right2.v)) {//上
					wallIntersectList2->addObject(agtk::ObjectWallIntersect::create(kWallTypeUp, p));
				}
				if (r1.containsPoint(left2.p) && r1.containsPoint(right2.p)) {//下
					wallIntersectList2->addObject(agtk::ObjectWallIntersect::create(kWallTypeDown, p));
				}
			}
			if (bUp && bDown) {
				//左、右で埋まっている位置を調べる。
				if (r1.containsPoint(up2.p) && r1.containsPoint(down2.p)) {//左
					wallIntersectList2->addObject(agtk::ObjectWallIntersect::create(kWallTypeLeft, p));
				}
				if (r1.containsPoint(up2.v) && r1.containsPoint(up2.v)) {//右
					wallIntersectList2->addObject(agtk::ObjectWallIntersect::create(kWallTypeRight, p));
				}
			}
		}
		return true;
	}
	return false;
}

bool ObjectWallIntersect::getWallIntersect(agtk::AreaData *area1, agtk::AreaData *area2, cocos2d::__Array *wallIntersectList1, cocos2d::__Array * wallIntersectList2)
{
	//r1
	agtk::Line up1(area1->getVertice2(), area1->getVertice3());
	agtk::Line left1(area1->getVertice1(), area1->getVertice2());
	agtk::Line right1(area1->getVertice3(), area1->getVertice4());
	agtk::Line down1(area1->getVertice4(), area1->getVertice1());
	//r2
	agtk::Line up2(area2->getVertice2(), area2->getVertice3());
	agtk::Line left2(area2->getVertice1(), area2->getVertice2());
	agtk::Line right2(area2->getVertice3(), area2->getVertice4());
	agtk::Line down2(area2->getVertice4(), area2->getVertice1());

	if (wallIntersectList1) {
		bool bUp = false;
		bool bDown = false;
		bool bLeft = false;
		bool bRight = false;
		//up ------------------------------------------
		cocos2d::Vec2 p;
		p = agtk::GetIntersectPoint(up1, left2);
		if (p != cocos2d::Vec2::ZERO) {
			wallIntersectList1->addObject(agtk::ObjectWallIntersect::create(kWallTypeUp, p));
			bUp = true;
		}
		p = agtk::GetIntersectPoint(up1, right2);
		if (p != cocos2d::Vec2::ZERO) {
			wallIntersectList1->addObject(agtk::ObjectWallIntersect::create(kWallTypeUp, p));
			bUp = true;
		}
		//left ----------------------------------------
		p = agtk::GetIntersectPoint(left1, up2);
		if (p != cocos2d::Vec2::ZERO) {
			wallIntersectList1->addObject(agtk::ObjectWallIntersect::create(kWallTypeLeft, p));
			bLeft = true;
		}
		p = agtk::GetIntersectPoint(left1, down2);
		if (p != cocos2d::Vec2::ZERO) {
			wallIntersectList1->addObject(agtk::ObjectWallIntersect::create(kWallTypeLeft, p));
			bLeft = true;
		}
		//right ---------------------------------------
		p = agtk::GetIntersectPoint(right1, up2);
		if (p != cocos2d::Vec2::ZERO) {
			wallIntersectList1->addObject(agtk::ObjectWallIntersect::create(kWallTypeRight, p));
			bRight = true;
		}
		p = agtk::GetIntersectPoint(right1, down2);
		if (p != cocos2d::Vec2::ZERO) {
			wallIntersectList1->addObject(agtk::ObjectWallIntersect::create(kWallTypeRight, p));
			bRight = true;
		}
		//down ----------------------------------------
		p = agtk::GetIntersectPoint(down1, left2);
		if (p != cocos2d::Vec2::ZERO) {
			wallIntersectList1->addObject(agtk::ObjectWallIntersect::create(kWallTypeDown, p));
			bDown = true;
		}
		p = agtk::GetIntersectPoint(down1, right2);
		if (p != cocos2d::Vec2::ZERO) {
			wallIntersectList1->addObject(agtk::ObjectWallIntersect::create(kWallTypeDown, p));
			bDown = true;
		}
		if (bLeft && bRight) {
			//上、下で埋まっている位置を調べる。
			if (agtk::AreaData::intersectsPoint(area2, left1.v) && agtk::AreaData::intersectsPoint(area2, right1.v)) {//上
				wallIntersectList1->addObject(agtk::ObjectWallIntersect::create(kWallTypeUp, p));
			}
			if (agtk::AreaData::intersectsPoint(area2, left1.p) && agtk::AreaData::intersectsPoint(area2, right1.p)) {//下
				wallIntersectList1->addObject(agtk::ObjectWallIntersect::create(kWallTypeDown, p));
			}
		}
		if (bUp && bDown) {
			//左、右で埋まっている位置を調べる。
			if (agtk::AreaData::intersectsPoint(area2, up1.p) && agtk::AreaData::intersectsPoint(area2, down1.p)) {//左
				wallIntersectList1->addObject(agtk::ObjectWallIntersect::create(kWallTypeLeft, p));
			}
			if(agtk::AreaData::intersectsPoint(area2, up1.v) && agtk::AreaData::intersectsPoint(area2, down1.v)){//右
				wallIntersectList1->addObject(agtk::ObjectWallIntersect::create(kWallTypeRight, p));
			}
		}
		if (!bUp && !bDown && !bLeft && !bRight) {
			if (agtk::AreaData::intersectsArea(area2, area1)) {
				//内側に領域がある場合。
				wallIntersectList1->addObject(agtk::ObjectWallIntersect::create(kWallTypeUp, cocos2d::Vec2::ZERO));
				wallIntersectList1->addObject(agtk::ObjectWallIntersect::create(kWallTypeDown, cocos2d::Vec2::ZERO));
				wallIntersectList1->addObject(agtk::ObjectWallIntersect::create(kWallTypeLeft, cocos2d::Vec2::ZERO));
				wallIntersectList1->addObject(agtk::ObjectWallIntersect::create(kWallTypeRight, cocos2d::Vec2::ZERO));
			}
		}
	}
	if (wallIntersectList2) {
		bool bUp = false;
		bool bDown = false;
		bool bLeft = false;
		bool bRight = false;
		//up ------------------------------------------
		cocos2d::Vec2 p;
		p = agtk::GetIntersectPoint(up2, left1);
		if (p != cocos2d::Vec2::ZERO) {
			wallIntersectList2->addObject(agtk::ObjectWallIntersect::create(kWallTypeUp, p));
			bUp = true;
		}
		p = agtk::GetIntersectPoint(up2, right1);
		if (p != cocos2d::Vec2::ZERO) {
			wallIntersectList2->addObject(agtk::ObjectWallIntersect::create(kWallTypeUp, p));
			bUp = true;
		}
		//left ----------------------------------------
		p = agtk::GetIntersectPoint(left2, up1);
		if (p != cocos2d::Vec2::ZERO) {
			wallIntersectList2->addObject(agtk::ObjectWallIntersect::create(kWallTypeLeft, p));
			bLeft = true;
		}
		p = agtk::GetIntersectPoint(left2, down1);
		if (p != cocos2d::Vec2::ZERO) {
			wallIntersectList2->addObject(agtk::ObjectWallIntersect::create(kWallTypeLeft, p));
			bLeft = true;
		}
		//right ---------------------------------------
		p = agtk::GetIntersectPoint(right2, up1);
		if (p != cocos2d::Vec2::ZERO) {
			wallIntersectList2->addObject(agtk::ObjectWallIntersect::create(kWallTypeRight, p));
			bRight = true;
		}
		p = agtk::GetIntersectPoint(right2, down1);
		if (p != cocos2d::Vec2::ZERO) {
			wallIntersectList2->addObject(agtk::ObjectWallIntersect::create(kWallTypeRight, p));
			bRight = true;
		}
		//down ----------------------------------------
		p = agtk::GetIntersectPoint(down2, left1);
		if (p != cocos2d::Vec2::ZERO) {
			wallIntersectList2->addObject(agtk::ObjectWallIntersect::create(kWallTypeDown, p));
			bDown = true;
		}
		p = agtk::GetIntersectPoint(down2, right1);
		if (p != cocos2d::Vec2::ZERO) {
			wallIntersectList2->addObject(agtk::ObjectWallIntersect::create(kWallTypeDown, p));
			bDown = true;
		}
		if (bLeft && bRight) {
			//上、下で埋まっている位置を調べる。
			if (agtk::AreaData::intersectsPoint(area1, left2.v) && agtk::AreaData::intersectsPoint(area1, right2.v)) {//上
				wallIntersectList2->addObject(agtk::ObjectWallIntersect::create(kWallTypeUp, p));
			}
			if (agtk::AreaData::intersectsPoint(area1, left2.p) && agtk::AreaData::intersectsPoint(area1, right2.p)) {//上
				wallIntersectList2->addObject(agtk::ObjectWallIntersect::create(kWallTypeDown, p));
			}
		}
		if (bUp && bDown) {
			//左、右で埋まっている位置を調べる。
			if (agtk::AreaData::intersectsPoint(area1, up2.p) && agtk::AreaData::intersectsPoint(area1, down2.p)) {//左
				wallIntersectList2->addObject(agtk::ObjectWallIntersect::create(kWallTypeLeft, p));
			}
			if (agtk::AreaData::intersectsPoint(area1, up2.v) && agtk::AreaData::intersectsPoint(area1, down2.v)) {//右
				wallIntersectList2->addObject(agtk::ObjectWallIntersect::create(kWallTypeRight, p));
			}
		}
		if (!bUp && !bDown && !bLeft && !bRight) {
			if (agtk::AreaData::intersectsArea(area1, area2)) {
				//内側に領域がある場合。
				wallIntersectList2->addObject(agtk::ObjectWallIntersect::create(kWallTypeUp, cocos2d::Vec2::ZERO));
				wallIntersectList2->addObject(agtk::ObjectWallIntersect::create(kWallTypeDown, cocos2d::Vec2::ZERO));
				wallIntersectList2->addObject(agtk::ObjectWallIntersect::create(kWallTypeLeft, cocos2d::Vec2::ZERO));
				wallIntersectList2->addObject(agtk::ObjectWallIntersect::create(kWallTypeRight, cocos2d::Vec2::ZERO));
			}
		}
	}
	return true;
}

bool ObjectWallIntersect::getWallIntersect(agtk::Vertex4 &v1, agtk::Vertex4 &v2, cocos2d::__Array *wallIntersectList1, cocos2d::__Array *wallIntersectList2)
{
	//r1
	agtk::Line line1[4]{ { v1[0], v1[1] }, { v1[3], v1[0] }, { v1[1], v1[2] }, { v1[2], v1[3] } };
	agtk::Line up1 = &line1[0];
	agtk::Line left1 = &line1[1];
	agtk::Line right1 = &line1[2];
	agtk::Line down1 = &line1[3];
	//r2
	agtk::Line line2[4]{ { v2[0], v2[1] }, { v2[3], v2[0] }, { v2[1], v2[2] }, { v2[2], v2[3] } };
	agtk::Line up2 = &line2[0];
	agtk::Line left2 = &line2[1];
	agtk::Line right2 = &line2[2];
	agtk::Line down2 = &line2[3];

	enum EnumWallBit {
		kWallBitUp = 0x01,//0x0001,
		kWallBitLeft = 0x02,//0x0010,
		kWallBitRight = 0x04,//0x0100,
		kWallBitDown = 0x08,//0x1000,
	};
	unsigned int wallBit1 = 0x00;
	unsigned int wallBit2 = 0x00;

	if (wallIntersectList1) {
		//up ------------------------------------------
		for (auto v : line2) {
			auto p = agtk::GetIntersectPoint(up1, v);
			if (p != cocos2d::Vec2::ZERO) {
				wallIntersectList1->addObject(agtk::ObjectWallIntersect::create(kWallTypeUp, p));
				wallBit1 |= kWallBitUp;
				break;
			}
		}
		//left ----------------------------------------
		for (auto v : line2) {
			auto p = agtk::GetIntersectPoint(left1, v);
			if (p != cocos2d::Vec2::ZERO) {
				wallIntersectList1->addObject(agtk::ObjectWallIntersect::create(kWallTypeLeft, p));
				wallBit1 |= kWallBitLeft;
				break;
			}
		}
		//right ---------------------------------------
		for (auto v : line2) {
			auto p = agtk::GetIntersectPoint(right1, v);
			if (p != cocos2d::Vec2::ZERO) {
				wallIntersectList1->addObject(agtk::ObjectWallIntersect::create(kWallTypeRight, p));
				wallBit1 |= kWallBitRight;
				break;
			}
		}
		//down ----------------------------------------
		for (auto v : line2) {
			auto p = agtk::GetIntersectPoint(down1, v);
			if (p != cocos2d::Vec2::ZERO) {
				wallIntersectList1->addObject(agtk::ObjectWallIntersect::create(kWallTypeDown, p));
				wallBit1 |= kWallBitDown;
				break;
			}
		}

		if ((wallBit1 & kWallBitLeft) && (wallBit1 & kWallBitRight)) {
			//上、下で埋まっている位置を調べる。
			if (agtk::Vertex4::intersectsPoint(v2, left1.v) && agtk::Vertex4::intersectsPoint(v2, right1.p)) {//上
				wallIntersectList1->addObject(agtk::ObjectWallIntersect::create(kWallTypeUp, cocos2d::Vec2::ZERO));
				wallBit1 |= kWallBitUp;
			}
			if (agtk::Vertex4::intersectsPoint(v2, left1.p) && agtk::Vertex4::intersectsPoint(v2, right1.v)) {//下
				wallIntersectList1->addObject(agtk::ObjectWallIntersect::create(kWallTypeDown, cocos2d::Vec2::ZERO));
				wallBit1 |= kWallBitDown;
			}
		}
		if ((wallBit1 & kWallBitUp) && (wallBit1 & kWallBitDown)) {
			//左、右で埋まっている位置を調べる。
			if (agtk::Vertex4::intersectsPoint(v2, up1.p) && agtk::Vertex4::intersectsPoint(v2, down1.v)) {//左
				wallIntersectList1->addObject(agtk::ObjectWallIntersect::create(kWallTypeLeft, cocos2d::Vec2::ZERO));
				wallBit1 |= kWallBitLeft;
			}
			if (agtk::Vertex4::intersectsPoint(v2, up1.v) && agtk::Vertex4::intersectsPoint(v2, down1.p)) {//右
				wallIntersectList1->addObject(agtk::ObjectWallIntersect::create(kWallTypeRight, cocos2d::Vec2::ZERO));
				wallBit1 |= kWallBitRight;
			}
		}
		if (wallBit1 == 0x00) {
			if (agtk::Vertex4::intersectsVertex4(v2, v1)) {
				//内側に領域がある場合。
				wallIntersectList1->addObject(agtk::ObjectWallIntersect::create(kWallTypeUp, cocos2d::Vec2::ZERO));
				wallIntersectList1->addObject(agtk::ObjectWallIntersect::create(kWallTypeDown, cocos2d::Vec2::ZERO));
				wallIntersectList1->addObject(agtk::ObjectWallIntersect::create(kWallTypeLeft, cocos2d::Vec2::ZERO));
				wallIntersectList1->addObject(agtk::ObjectWallIntersect::create(kWallTypeRight, cocos2d::Vec2::ZERO));
			}
		}
	}
	if (wallIntersectList2) {
		//up ------------------------------------------
		for (auto v : line1) {
			auto p = agtk::GetIntersectPoint(up2, v);
			if (p != cocos2d::Vec2::ZERO) {
				wallIntersectList2->addObject(agtk::ObjectWallIntersect::create(kWallTypeUp, p));
				wallBit2 |= kWallBitUp;
				break;
			}
		}
		//left ----------------------------------------
		for (auto v : line1) {
			auto p = agtk::GetIntersectPoint(left2, v);
			if (p != cocos2d::Vec2::ZERO) {
				wallIntersectList2->addObject(agtk::ObjectWallIntersect::create(kWallTypeLeft, p));
				wallBit2 |= kWallBitLeft;
				break;
			}
		}
		//right ---------------------------------------
		for (auto v : line1) {
			auto p = agtk::GetIntersectPoint(right2, v);
			if (p != cocos2d::Vec2::ZERO) {
				wallIntersectList2->addObject(agtk::ObjectWallIntersect::create(kWallTypeRight, p));
				wallBit2 |= kWallBitRight;
				break;
			}
		}
		//down ----------------------------------------
		for (auto v : line1) {
			auto p = agtk::GetIntersectPoint(down2, v);
			if (p != cocos2d::Vec2::ZERO) {
				wallIntersectList2->addObject(agtk::ObjectWallIntersect::create(kWallTypeDown, p));
				wallBit2 |= kWallBitDown;
				break;
			}
		}

		if ((wallBit2 & kWallBitLeft) && (wallBit2 & kWallBitRight)) {
			//上、下で埋まっている位置を調べる。
			if (agtk::Vertex4::intersectsPoint(v1, left2.v) && agtk::Vertex4::intersectsPoint(v1, right2.p)) {//上
				wallIntersectList2->addObject(agtk::ObjectWallIntersect::create(kWallTypeUp, cocos2d::Vec2::ZERO));
				wallBit2 |= kWallBitUp;
			}
			if (agtk::Vertex4::intersectsPoint(v1, left2.p) && agtk::Vertex4::intersectsPoint(v1, right2.v)) {//下
				wallIntersectList2->addObject(agtk::ObjectWallIntersect::create(kWallTypeDown, cocos2d::Vec2::ZERO));
				wallBit2 |= kWallBitDown;
			}
		}
		if ((wallBit2 & kWallBitUp) && (wallBit2 & kWallBitDown)) {
			//左、右で埋まっている位置を調べる。
			if (agtk::Vertex4::intersectsPoint(v1, up2.p) && agtk::Vertex4::intersectsPoint(v1, down2.v)) {//左
				wallIntersectList2->addObject(agtk::ObjectWallIntersect::create(kWallTypeLeft, cocos2d::Vec2::ZERO));
				wallBit2 |= kWallBitLeft;
			}
			if (agtk::Vertex4::intersectsPoint(v1, up2.v) && agtk::Vertex4::intersectsPoint(v1, down2.p)) {//右
				wallIntersectList2->addObject(agtk::ObjectWallIntersect::create(kWallTypeRight, cocos2d::Vec2::ZERO));
				wallBit2 |= kWallBitRight;
			}
		}
		if (wallBit2 == 0x00) {
			if (agtk::Vertex4::intersectsVertex4(v1, v2)) {
				//内側に領域がある場合。
				wallIntersectList2->addObject(agtk::ObjectWallIntersect::create(kWallTypeUp, cocos2d::Vec2::ZERO));
				wallIntersectList2->addObject(agtk::ObjectWallIntersect::create(kWallTypeDown, cocos2d::Vec2::ZERO));
				wallIntersectList2->addObject(agtk::ObjectWallIntersect::create(kWallTypeLeft, cocos2d::Vec2::ZERO));
				wallIntersectList2->addObject(agtk::ObjectWallIntersect::create(kWallTypeRight, cocos2d::Vec2::ZERO));
			}
		}
	}

	//２つの矩形から側面で不必要な壁面を削除する。
	if (wallBit1 && wallBit2) {
		std::function<void(cocos2d::__Array *, unsigned int)> removeList = [](cocos2d::__Array *list, unsigned int bit) {
		lRetry:;
			cocos2d::Ref *ref;
			CCARRAY_FOREACH(list, ref) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
				auto p = static_cast<agtk::ObjectWallIntersect *>(ref);
#else
				auto p = dynamic_cast<agtk::ObjectWallIntersect *>(ref);
#endif
				if (p->getWallBit() & bit) {
					list->removeObject(p);
					goto lRetry;
				}
			}
		};
		std::function<int(unsigned int)> getBits = [](unsigned int bits) {
			int num = (bits >> 1) & 03333333333;
			num = bits - num - ((num >> 1) & 03333333333);
			num = ((num + (num >> 3)) & 0707070707) % 077;
			return num;
		};
		if (wallIntersectList1 && getBits(wallBit1) == 3) {
			//up
			if (wallBit1 == (kWallBitLeft | kWallBitUp | kWallBitRight)) {
				removeList(wallIntersectList1, kWallBitLeft);
				removeList(wallIntersectList1, kWallBitRight);
				wallBit1 = wallBit1 & ~(kWallBitLeft | kWallBitRight);
			}
			//rigth
			if (wallBit1 == (kWallBitUp | kWallBitRight | kWallBitDown)) {
				removeList(wallIntersectList1, kWallBitUp);
				removeList(wallIntersectList1, kWallBitDown);
				wallBit1 = wallBit1 & ~(kWallBitUp | kWallBitDown);
			}
			//down
			if (wallBit1 == (kWallBitRight | kWallBitDown | kWallBitLeft)) {
				removeList(wallIntersectList1, kWallBitRight);
				removeList(wallIntersectList1, kWallBitLeft);
				wallBit1 = wallBit1 & ~(kWallBitRight | kWallBitLeft);
			}
			//left
			if (wallBit1 == (kWallBitDown | kWallBitLeft | kWallBitUp)) {
				removeList(wallIntersectList1, kWallBitDown);
				removeList(wallIntersectList1, kWallBitUp);
				wallBit1 = wallBit1 & ~(kWallBitDown | kWallBitUp);
			}
		}
		if (wallIntersectList2 && getBits(wallBit2) == 3) {
			//up
			if (wallBit2 == (kWallBitLeft | kWallBitUp | kWallBitRight)) {
				removeList(wallIntersectList2, kWallBitLeft);
				removeList(wallIntersectList2, kWallBitRight);
				wallBit2 = wallBit2 & ~(kWallBitLeft | kWallBitRight);
			}
			//rigth
			if (wallBit2 == (kWallBitUp | kWallBitRight | kWallBitDown)) {
				removeList(wallIntersectList2, kWallBitUp);
				removeList(wallIntersectList2, kWallBitDown);
				wallBit2 = wallBit2 & ~(kWallBitUp | kWallBitDown);
			}
			//down
			if (wallBit2 == (kWallBitRight | kWallBitDown | kWallBitLeft)) {
				removeList(wallIntersectList2, kWallBitRight);
				removeList(wallIntersectList2, kWallBitLeft);
				wallBit2 = wallBit2 & ~(kWallBitRight | kWallBitLeft);
			}
			//left
			if (wallBit2 == (kWallBitDown | kWallBitLeft | kWallBitUp)) {
				removeList(wallIntersectList2, kWallBitDown);
				removeList(wallIntersectList2, kWallBitUp);
				wallBit2 = wallBit2 & ~(kWallBitDown | kWallBitUp);
			}
		}
	}
	return true;
}


int ObjectWallIntersect::getWallBit(cocos2d::__Array *wallIntersectList)
{
	int wallBit = 0;
	cocos2d::Ref *ref = nullptr;
	CCARRAY_FOREACH(wallIntersectList, ref) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
		auto p = static_cast<agtk::ObjectWallIntersect *>(ref);
#else
		auto p = dynamic_cast<agtk::ObjectWallIntersect *>(ref);
#endif
		wallBit |= p->getWallBit();
	}
	return wallBit;
}

//-------------------------------------------------------------------------------------------------------------------
ObjectCollision::ObjectWallIntersectTemp::ObjectWallIntersectTemp()
{
	_object = nullptr;
	_wallList = nullptr;
}

ObjectCollision::ObjectWallIntersectTemp::~ObjectWallIntersectTemp()
{
	CC_SAFE_RELEASE_NULL(_object);
	CC_SAFE_RELEASE_NULL(_wallList);
}

bool ObjectCollision::ObjectWallIntersectTemp::init(agtk::Object *object, cocos2d::__Array *wallList)
{
	CC_ASSERT(object);
	_object = object;
	_object->retain();
	this->setWallList(wallList);
	return true;
}

ObjectCollision::SlopeCheckRect::SlopeCheckRect()
{
	_rect = Rect::ZERO;
}

ObjectCollision::SlopeCheckRect::~SlopeCheckRect()
{
}

bool ObjectCollision::SlopeCheckRect::init()
{
	reset();
	return true;
}

void ObjectCollision::SlopeCheckRect::reset()
{
	_rect = Rect::ZERO;
	_wallRectDifferenceList.clear();
}

void ObjectCollision::SlopeCheckRect::updateRect(cocos2d::Rect& rect, agtk::WallHitInfoGroup* wallHitInfoGroup, cocos2d::Vec2& moveVec)
{
	// パラメータを一度リセット
	reset();

	// 検索して取得した矩形以外がタイル等に接触して移動できなくなる可能性があるので
	// 判定矩形の調整を行う
	for (unsigned int i = 0; i < wallHitInfoGroup->getWallHitInfoListCount(); i++) {
		auto info = wallHitInfoGroup->getWallHitInfo(i);

		auto r = Rect(info.boundMin, Size(info.boundMax - info.boundMin));
		r.origin += moveVec;

		// 矩形より高い位置の矩形がある場合
		if (rect.getMaxY() < r.getMaxY()) {
			// 高い位置にある矩形がタイルに接触する可能性があるので
			// 判定矩形の高さを高い位置の矩形に合わせる
			float addH = r.getMaxY() - rect.getMaxY();
			rect.size.height += addH;
		}
	}

	// 判定矩形を更新
	_rect = rect;

	// 各壁当たり矩形左下と判定矩形左下との差を格納
	for (unsigned int i = 0; i < wallHitInfoGroup->getWallHitInfoListCount(); i++) {
		auto info = wallHitInfoGroup->getWallHitInfo(i);
		auto diff = info.boundMin - rect.origin + moveVec;
		_wallRectDifferenceList.push_back(diff);
	}
}

ObjectCollision::ObjectCollision()
{
	_object = nullptr;
	_objectList = nullptr;
	_hitObjectList = nullptr;
	_wallObjectList = nullptr;
	_portalList = nullptr;
	_slopeCheckRect = nullptr;
	_buriedInWallFlag = false;
	_leftWallObjectList = nullptr;
	_rightWallObjectList = nullptr;
	_upWallObjectList = nullptr;
	_downWallObjectList = nullptr;
	_wallHitInfoGroup = nullptr;
	_oldWallHitInfoGroup = nullptr;
	_prevWallHitInfoGroup = nullptr;
}

ObjectCollision::~ObjectCollision()
{
	CC_SAFE_RELEASE_NULL(_portalList);
	CC_SAFE_RELEASE_NULL(_objectList);
	CC_SAFE_RELEASE_NULL(_hitObjectList);
	CC_SAFE_RELEASE_NULL(_wallObjectList);
	CC_SAFE_RELEASE_NULL(_slopeCheckRect);
#ifdef USE_MULTITHREAD_OBJECT_UPDATE
	CC_SAFE_DELETE(_leftWallObjectList);
	CC_SAFE_DELETE(_rightWallObjectList);
	CC_SAFE_DELETE(_upWallObjectList);
	CC_SAFE_DELETE(_downWallObjectList);
#else
	CC_SAFE_RELEASE_NULL(_leftWallObjectList);
	CC_SAFE_RELEASE_NULL(_rightWallObjectList);
	CC_SAFE_RELEASE_NULL(_upWallObjectList);
	CC_SAFE_RELEASE_NULL(_downWallObjectList);
#endif
#ifdef USE_MULTITHREAD_OBJECT_UPDATE
	CC_SAFE_DELETE(_wallHitInfoGroup);
	CC_SAFE_DELETE(_oldWallHitInfoGroup);
	CC_SAFE_DELETE(_prevWallHitInfoGroup);
#else
	CC_SAFE_RELEASE_NULL(_wallHitInfoGroup);
	CC_SAFE_RELEASE_NULL(_oldWallHitInfoGroup);
	CC_SAFE_RELEASE_NULL(_prevWallHitInfoGroup);
#endif
}

bool ObjectCollision::init(agtk::Object *object)
{
	CC_ASSERT(object);
	_object = object;
#ifdef USE_MULTITHREAD_OBJECT_UPDATE
	auto objectList = agtk::NrArray::create();
#else
	auto objectList = cocos2d::__Array::create();
#endif
	if (objectList == nullptr) {
		CC_ASSERT(0);
		return false;
	}
	this->setObjectList(objectList);
	this->setHitObjectList(cocos2d::__Array::create());
	this->setWallObjectList(cocos2d::__Array::create());

	auto portalList = cocos2d::__Array::create();
	if (nullptr == portalList) {
		CC_ASSERT(0);
		return false;
	}
	this->setPortalList(portalList);

	this->setSlopeCheckRect(SlopeCheckRect::create());

#ifdef USE_MULTITHREAD_OBJECT_UPDATE
	this->setLeftWallObjectList(new agtk::MtVector<agtk::Object *>());
	this->setRightWallObjectList(new agtk::MtVector<agtk::Object *>());
	this->setUpWallObjectList(new agtk::MtVector<agtk::Object *>());
	this->setDownWallObjectList(new agtk::MtVector<agtk::Object *>());
#else
	this->setLeftWallObjectList(cocos2d::__Array::create());
	this->setRightWallObjectList(cocos2d::__Array::create());
	this->setUpWallObjectList(cocos2d::__Array::create());
	this->setDownWallObjectList(cocos2d::__Array::create());
#endif
	this->setWallHitInfoGroup(agtk::WallHitInfoGroup::create(object));
	this->setOldWallHitInfoGroup(agtk::WallHitInfoGroup::create(object));
	this->setPrevWallHitInfoGroup(agtk::WallHitInfoGroup::create(object));
	return true;
}

void ObjectCollision::reset()
{
	this->getWallHitInfoGroup()->remove();
	this->setReturnedPos(cocos2d::Vec2::ZERO);
	this->setBuriedInWallFlag(false);
#ifdef USE_MULTITHREAD_OBJECT_UPDATE
	this->getLeftWallObjectList()->clear();
	this->getRightWallObjectList()->clear();
	this->getUpWallObjectList()->clear();
	this->getDownWallObjectList()->clear();
#else
	this->getLeftWallObjectList()->removeAllObjects();
	this->getRightWallObjectList()->removeAllObjects();
	this->getUpWallObjectList()->removeAllObjects();
	this->getDownWallObjectList()->removeAllObjects();
#endif
	this->getObjectList()->removeAllObjects();
	this->getHitObjectList()->removeAllObjects();
	this->getWallObjectList()->removeAllObjects();
}

void ObjectCollision::update()
{
	CC_ASSERT(_object);
	auto objectData = _object->getObjectData();
	auto playObjectData = _object->getPlayObjectData();
	bool bCollision = false;
	cocos2d::__Array *damagedList = cocos2d::__Array::create();

	std::vector<agtk::Vertex4> attackCollisionList;

	_object->getTimelineList(agtk::data::TimelineInfoData::kTimelineAttack, attackCollisionList);

	auto portalList = this->getPortalList();
	if (_object->getIsPortalWarped()) {
		//遷移してきたポータルに触れていなければ、ポータルワープフラグを落とす。
		bool touched = false;
		cocos2d::Ref *ref;
		CCARRAY_FOREACH(portalList, ref) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
			auto portal = static_cast<agtk::Portal *>(ref);
#else
			auto portal = dynamic_cast<agtk::Portal *>(ref);
#endif
			if (portal->getId() == _object->getWarpedTransitionPortalId()) {
				touched = true;
				portalList->removeObject(ref, false);
				break;
			}
		}
		if (!touched) {
			// ポータルにワープ直後フラグをOFF
			_object->setIsPortalWarped(false);
			_object->setWarpedTransitionPortalId(-1);
		}
	}
	auto touchedPortalNum = portalList->count();
// #AGTK-NX #AGTK-WIN
#ifdef USE_30FPS_1
	std::vector<agtk::Vertex4> attackCollisionList2[2];
	attackCollisionList2[0].resize(attackCollisionList.size());
	attackCollisionList2[1].resize(attackCollisionList.size());
	cocos2d::Vec2 passPoint[2];
	int passCount = 1;
	if (_object->getPassedFrameCount() == _object->getFrameCount())
	{
		passCount = 2;
		passPoint[0] = _object->getPassedFramePosition();
		passPoint[1] = _object->getPosition();
	}
	else {
		passPoint[0] = _object->getPosition();
	}

	for (int passIndex = 0; passIndex < passCount; passIndex++) {
		auto difPos = passPoint[passIndex] - _object->getPosition();
		for (int i = 0; i < attackCollisionList.size(); i++)
		{
			for (int j = 0; j < 4; j++) {
				attackCollisionList2[passIndex][i][j] = difPos + attackCollisionList[i][j];
			}
		}
	}
#endif
	//攻撃判定
	cocos2d::Ref *ref;
	auto objectList = this->getHitObjectList();
	CCARRAY_FOREACH(objectList, ref) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
		auto object2 = static_cast<agtk::Object *>(ref);
#else
		auto object2 = dynamic_cast<agtk::Object *>(ref);
#endif
		auto objectData2 = object2->getObjectData();
		auto playObjectData2 = object2->getPlayObjectData();

		std::vector<agtk::Vertex4> hitCollisionList2;
		object2->getTimelineList(agtk::data::TimelineInfoData::kTimelineHit, hitCollisionList2);

		//objectからobject2への攻撃(object.attack -> object2.hit)
		CC_ASSERT(object2->getLayerId() == _object->getLayerId());

		//攻撃判定チェック
		bool bCheckAttack = false;
		if (playObjectData->getHitObjectGroupBit() & (1 << objectData2->getGroup())) {
			bCheckAttack = true;
		}

		auto damageInvincible = object2->getObjectVisible()->getObjectDamageInvincible();
		// _objectからobject2へ攻撃可能か判定する
		if (_object->checkAttackableObject(object2) && bCheckAttack) {
// #AGTK-NX #AGTK-WIN
#ifdef USE_30FPS_1
			bool bHitAttack = false;
			for (int passIndex = 0; passIndex < passCount; passIndex++) {
#endif
// #AGTK-NX #AGTK-WIN
#ifdef USE_30FPS_1
				for (auto attack : attackCollisionList2[passIndex]) {
#else
				for (auto attack : attackCollisionList) {
#endif
				// 壁当たり判定の補正により壁判定と攻撃判定のサイズが同じでも
				// 壁判定は接触しているが攻撃判定は接触していない、というような事象が発生しているので
				// 壁判定同様に攻撃判定を少し補正
#if 1
				// ACT2-5838 攻撃判定が、左右反転、上下反転、回転している場合に、補正する箇所が変化するため、最小値・最大値を見て補正するように。
				auto minX = std::min({ attack[0].x, attack[1].x, attack[2].x, attack[3].x });
				auto maxX = std::max({ attack[0].x, attack[1].x, attack[2].x, attack[3].x });
				auto minY = std::min({ attack[0].y, attack[1].y, attack[2].y, attack[3].y });
				auto maxY = std::max({ attack[0].y, attack[1].y, attack[2].y, attack[3].y });
				for (int i = 0; i < 4; i++) {
					if (attack[i].x == minX) {
						attack[i].x -= TILE_COLLISION_THRESHOLD;
					}
					else if (attack[i].x == maxX) {
						attack[i].x += TILE_COLLISION_THRESHOLD;
					}
					if (attack[i].y == minY) {
						attack[i].y -= TILE_COLLISION_THRESHOLD;
					}
					else if (attack[i].y == maxY) {
						attack[i].y += TILE_COLLISION_THRESHOLD;
					}
				}
#else
				attack[0].x -= TILE_COLLISION_THRESHOLD;
				attack[0].y += TILE_COLLISION_THRESHOLD;
				attack[1].x += TILE_COLLISION_THRESHOLD;
				attack[1].y += TILE_COLLISION_THRESHOLD;
				attack[2].x += TILE_COLLISION_THRESHOLD;
				attack[2].y -= TILE_COLLISION_THRESHOLD;
				attack[3].x -= TILE_COLLISION_THRESHOLD;
				attack[3].y -= TILE_COLLISION_THRESHOLD;
#endif

				for (auto hit : hitCollisionList2) {
					if (agtk::Vertex4::intersectsVertex4(attack, hit) == false) {
						continue;
					}
					cocos2d::__Array *wallAttackList = cocos2d::__Array::create();
					cocos2d::__Array *wallHitList = cocos2d::__Array::create();
					ObjectWallIntersect::getWallIntersect(attack, hit, wallAttackList, wallHitList);
					int wallBit = ObjectWallIntersect::getWallBit(wallAttackList);
					if (wallBit && (object2->isInvincible() == false || damageInvincible->isInvincibleStartAcceptable())) {
						bCollision = true;
						if (damageInvincible->start() && !damagedList->containsObject(object2)) {//無敵開始
							//偶数:agtk::Object, 奇数:Array(agtk::ObjectWallIntersect)
							object2->getCollisionHitAttackList()->addObject(ObjectWallIntersectTemp::create(_object, wallHitList));
							object2->damaged(_object, nullptr);//ダメージ処理
							damagedList->addObject(object2);
							_object->getCollisionAttackHitList()->addObject(ObjectWallIntersectTemp::create(object2, wallAttackList));
							object2->getAttackerObjectInstanceIdList()->addObject(cocos2d::Integer::create(_object->getInstanceId()));
						}
					}
					_object->getAttackObjectList()->addObject(object2);
// #AGTK-NX #AGTK-WIN
#ifdef USE_30FPS_1
					bHitAttack = true;
#endif
				}
			}
// #AGTK-NX #AGTK-WIN
#ifdef USE_30FPS_1
				if (bHitAttack)
					break;
			}
#endif
		}

		//攻撃判定チェック
		bCheckAttack = false;
		if (playObjectData2->getHitObjectGroupBit() & (1 << objectData->getGroup()) ) {
			bCheckAttack = true;
		}

	}
	// ポータルワープ直後以外のポータルに触れている場合。
	if (touchedPortalNum > 0) {

		// 壁判定を持っていて触れたポータルが存在する場合
		std::vector<agtk::Vertex4> wallCollisionList;
		if (objectData->getGroup() == agtk::data::ObjectData::kObjGroupPlayer) {
			_object->getTimelineList(agtk::data::TimelineInfoData::kTimelineWall, wallCollisionList);
		}
		if(wallCollisionList.size() > 0 && GameManager::getInstance()->getIsPortalMoving() == false) {
			// 最初に衝突したポータルをリストに追加
			_object->getCollisionPortalHitList()->addObjectsFromArray(portalList);

#ifdef FIX_ACT2_4774
#else
			_object->_firstTouchedPortalName = nullptr;
#endif
			bCollision = true;
		}
	}

	this->getPortalList()->removeAllObjects();

	_object->_collision = bCollision;
}

static bool getWallCenterAndBound(cocos2d::Array *wallList, Point *center, Point *boundMin, Point *boundMax);
bool getWallCenterAndBound(cocos2d::Array *wallList, Point *center, Point *boundMin, Point *boundMax)
{
	float minX = -FLT_MAX, maxX = FLT_MAX, minY = -FLT_MAX, maxY = FLT_MAX;
	bool first = true;
	cocos2d::Ref *ref = nullptr;
	CCARRAY_FOREACH(wallList, ref) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
		auto wall = static_cast<cocos2d::Node *>(ref);
#else
		auto wall = dynamic_cast<cocos2d::Node *>(ref);
#endif
		auto pos = wall->getPosition();
		auto size = wall->getContentSize();
		if (size.width <= 0.0f || size.height <= 0.0f) {
			continue;
		}
		float x0, y0, x1, y1;

		// 浮動小数点切り捨て
		x0 = (int)pos.x;
		y0 = (int)pos.y;
		x1 = (int)(pos.x + size.width - 1);
		y1 = (int)(pos.y + size.height - 1);

		if (first) {
			minX = x0;
			minY = y0;
			maxX = x1;
			maxY = y1;
			first = false;
			continue;
		}
		if (minX > x0) minX = x0;
		if (minY > y0) minY = y0;
		if (maxX < x0) maxX = x0;
		if (maxY > y0) maxY = y0;
	}
	if (first) {
		return false;
	}
	*center = Point(floorf((minX + maxX) / 2), floorf((minY + maxY) / 2));
	*boundMin = Point(minX, minY);
	*boundMax = Point(maxX, maxY);
	return true;
}

static void getWallCenterAndBound(std::vector<Vertex4>& wallList, Point *center, Point *boundMin, Point *boundMax);
void getWallCenterAndBound(std::vector<Vertex4>& wallList, Point *center, Point *boundMin, Point *boundMax)
{
	float minX = -FLT_MAX, maxX = FLT_MAX, minY = -FLT_MAX, maxY = FLT_MAX;
	bool first = true;
	for (auto wall : wallList) {

		cocos2d::Vec2 pos = cocos2d::Vec2(wall.p0);
		cocos2d::Size size = cocos2d::Size(wall.p2 - wall.p0);
		float x0, y0, x1, y1;

		// 浮動小数点切り捨て
		x0 = (int)pos.x;
		y0 = (int)pos.y;
		x1 = (int)(pos.x + size.width - 1);
		y1 = (int)(pos.y + size.height - 1);

		if (first) {
			minX = x0;
			minY = y0;
			maxX = x1;
			maxY = y1;
			first = false;
			continue;
		}
		if (minX > x0) minX = x0;
		if (minY > y0) minY = y0;
		if (maxX < x0) maxX = x0;
		if (maxY > y0) maxY = y0;
	}
	*center = Point(floorf((minX + maxX) / 2), floorf((minY + maxY) / 2));
	*boundMin = Point(minX, minY);
	*boundMax = Point(maxX, maxY);
}

static void getWallCenterAndBound2(std::vector<Vertex4>& wallList, Point *center, Point *boundMin, Point *boundMax);
void getWallCenterAndBound2(std::vector<Vertex4>& wallList, Point *center, Point *boundMin, Point *boundMax)
{
	float minX = -FLT_MAX, maxX = FLT_MAX, minY = -FLT_MAX, maxY = FLT_MAX;
	bool first = true;
	for (auto wall : wallList) {

		cocos2d::Vec2 pos = cocos2d::Vec2(wall.p0);
		cocos2d::Size size = cocos2d::Size(wall.p2 - wall.p0);
		float x0, y0, x1, y1;

		// 浮動小数点切り捨て
		x0 = (int)pos.x;
		y0 = (int)pos.y;
		x1 = (int)(pos.x + size.width);
		y1 = (int)(pos.y + size.height);

		if (first) {
			minX = x0;
			minY = y0;
			maxX = x1;
			maxY = y1;
			first = false;
			continue;
		}
		if (minX > x0) minX = x0;
		if (minY > y0) minY = y0;
		if (maxX < x0) maxX = x0;
		if (maxY > y0) maxY = y0;
	}
	*center = Point(floorf((minX + maxX) / 2), floorf((minY + maxY) / 2));
	*boundMin = Point(minX, minY);
	*boundMax = Point(maxX, maxY);
}

// 坂に使用する壁当たり情報取得処理
static void getWallCenterAndBoundUseSlope(std::vector<Vertex4>& wallList, Point *center, Point *boundMin, Point *boundMax);
void getWallCenterAndBoundUseSlope(std::vector<Vertex4>& wallList, Point *center, Point *boundMin, Point *boundMax)
{
	float minX = -FLT_MAX, maxX = FLT_MAX, minY = -FLT_MAX, maxY = FLT_MAX;
	bool first = true;
	for (auto wall : wallList) {
		cocos2d::Vec2 pos = cocos2d::Vec2(wall.p0);
		cocos2d::Size size = cocos2d::Size(wall.p2 - wall.p0);
		float x0, y0, x1, y1;
		x0 = pos.x;
		y0 = pos.y;
		x1 = (pos.x + size.width);
		y1 = (pos.y + size.height);

		if (first) {
			minX = x0;
			minY = y0;
			maxX = x1;
			maxY = y1;

			first = false;

			continue;
		}
		if (minX > x0) minX = x0;
		if (minY > y0) minY = y0;
		if (maxX < x1) maxX = x1;
		if (maxY < y1) maxY = y1;
	}
	*center = Point(floorf((minX + maxX) / 2), floorf((minY + maxY) / 2));
	*boundMin = Point(minX, minY);
	*boundMax = Point(maxX, maxY);
}

static bool output_tile_info = false;
#ifdef USE_MULTITHREAD_OBJECT_UPDATE
static void calcWallHitInfo(agtk::Object* object, WallHitInfo &wallHitInfo, cocos2d::Vec2 &velocity, agtk::NrArray *objectList, std::vector<agtk::Tile *> &tileList, bool checkActionCondition, ObjectWallDebugDisplay *debug, cocos2d::Rect thresholdRect);
void calcWallHitInfo(agtk::Object* object, WallHitInfo &wallHitInfo, cocos2d::Vec2 &velocity, agtk::NrArray *objectList, std::vector<agtk::Tile *> &tileList, bool checkActionCondition, ObjectWallDebugDisplay *debug, cocos2d::Rect thresholdRect)
#else
static void calcWallHitInfo(agtk::Object* object, WallHitInfo &wallHitInfo, cocos2d::Vec2 &velocity, cocos2d::Array *objectList, cocos2d::Array *tileList, bool checkActionCondition, ObjectWallDebugDisplay *debug, cocos2d::Rect thresholdRect);
void calcWallHitInfo(agtk::Object* object, WallHitInfo &wallHitInfo, cocos2d::Vec2 &velocity, cocos2d::Array *objectList, cocos2d::Array *tileList, bool checkActionCondition, ObjectWallDebugDisplay *debug, cocos2d::Rect thresholdRect)
#endif
{
	cocos2d::Ref *ref = nullptr;

	//object
	auto objectData = object->getObjectData();
	auto objectTemplateMove = object->getObjectTemplateMove();

	if (objectList != nullptr && objectTemplateMove->isIgnoredObjectWall() == false) {
#ifdef USE_MULTITHREAD_OBJECT_UPDATE
		for(auto ref: *objectList) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
			auto object2 = static_cast<agtk::Object *>(ref);
#else
			auto object2 = dynamic_cast<agtk::Object *>(ref);
#endif
#else
		CCARRAY_FOREACH(objectList, ref) {
			auto object2 = dynamic_cast<agtk::Object *>(ref);
#endif
			auto objectData2 = object2->getObjectData();

			//壁判定の設定
			bool bCheckCollide = false;//判定チェック
			if (objectData->isCollideWith(objectData2)){
				bCheckCollide = true;
			}
			if (bCheckCollide == false) {
				continue;
			}

			bool bCollided = false;

			// 自オブジェクトが他オブジェクトから押され、他オブジェクトが押し戻されない場合
			// 自オブジェクト、他オブジェクトの両方ともが押し戻される場合
			// 自オブジェクト、他オブジェクトの両方ともが押し戻されない場合
			if ((object->getPushedbackByObject() && (!object2->getPushedbackByObject() || object2->getPushedbackByObject())) ||
				!object->getPushedbackByObject() && !object2->getPushedbackByObject()) {
				std::vector<agtk::Vertex4> wallCollisionList2;
// #AGTK-NX #AGTK-WIN
#ifdef USE_30FPS_4
				auto mf = object2->_middleFrameStock.getUpdatedMiddleFrame();
				if (object2->_bUseMiddleFrame && mf->_hasMiddleFrame) {
					// 中間フレームの壁判定をチェック対象とする
					wallCollisionList2 = mf->_wallList;
					Vec2 diff = object2->getPosition() - mf->_objectPos;
					diff.y = -diff.y;
					for (auto it = wallCollisionList2.begin(); it != wallCollisionList2.end(); it++) {
						for (int i = 0; i < 4; i++) {
							Vec2& p = (*it)[i];
							p += diff;
						}
					}
				}
				else {
					object2->getTimelineList(agtk::data::TimelineInfoData::kTimelineWall, wallCollisionList2);
				}
#else
				object2->getTimelineList(agtk::data::TimelineInfoData::kTimelineWall, wallCollisionList2);
#endif
				for (auto wall2 : wallCollisionList2) {
					auto rect = wall2.getRect();
					bCollided |= checkWallHit(wallHitInfo, rect.origin, rect.size, false, debug);
				}
			}

			//衝突した。
			if (bCollided && object2->getPushedbackByObject() == true) {//「他のオブジェクトから押し戻されない」が設定されていない場合。
				//お互いの移動量から量が大きいほうへ作用するようにするためのx,y方向を計算。
				auto objectMovement = object->getObjectMovement();
				auto objectMovement2 = object2->getObjectMovement();

				//X座標
				float mx = objectMovement->getMoveVelocity().x;
				float mx2 = objectMovement2->getMoveVelocity().x;
				if ((mx >= 0 && mx2 >= 0 && mx > mx2) || (mx <= 0 && mx2 <= 0 && mx < mx2)) {
					mx -= mx2;
				}
				else if (mx <= 0 && mx2 >= 0 || mx >= 0 && mx2 <= 0) {
					mx += mx2;
				}
				//Y座標
				float my = objectMovement->getMoveVelocity().y;
				float my2 = objectMovement2->getMoveVelocity().y;
				if ((my >= 0 && my2 >= 0 && my > my2) || (my <= 0 && my2 <= 0 && my < my2)) {
					my -= my2;
				}
				else if (my <= 0 && my2 >= 0 || my >= 0 && my2 <= 0) {
					my += my2;
				}
				wallHitInfo.move.x += mx;
				wallHitInfo.move.y += my;
			}
		}
	}

	//tile
	auto projectData = GameManager::getInstance()->getProjectData();
#ifdef USE_MULTITHREAD_OBJECT_UPDATE
	for (auto tile : tileList) {
#else
	CCARRAY_FOREACH(tileList, ref) {
		auto tile = dynamic_cast<agtk::Tile *>(ref);
#endif

		// アクション切り替えの条件確認用にチェックしていない場合
		if (tile->getType() != agtk::Tile::kTypeLimitTile && !checkActionCondition) {
			//壁判定の設定（タイルとぶつかる）
			if (objectData->getCollideWithTileGroupBit() == 0) {
				wallHitInfo.adjustHit();
				continue;
			}

			//テンプレート移動で「タイル判定を無視する」の場合
			if (objectTemplateMove->isIgnoredTileWall()) {
				wallHitInfo.adjustHit();
				continue;
			}
		}

		if (!isNeedCheck(object, tile, checkActionCondition)) {
			continue;
		}
		
		auto rect = tile->convertToLayerSpaceRect();
		Point center;
		Point boundMin = rect.origin;
		Point boundMax = Point(rect.getMaxX(), rect.getMaxY());
		center = boundMin;
		center.x += rect.size.width * 0.5f;
		center.y += rect.size.height * 0.5f;
		int wallBit = tile->getWallBit();
		// 衝突した時の処理
		auto hitFunc = [&tile,&wallHitInfo,&wallBit]() {
			int bit = 0;
			bit |= (wallHitInfo.hitUp <= wallHitInfo.boundMax.y && (wallBit & (1 << agtk::data::TilesetData::Down)) ? agtk::data::ObjectActionLinkConditionData::kWallBitUp : 0);
			bit |= (wallHitInfo.hitLeft >= wallHitInfo.boundMin.x && (wallBit & (1 << agtk::data::TilesetData::Right)) ? agtk::data::ObjectActionLinkConditionData::kWallBitLeft : 0);
			bit |= (wallHitInfo.hitRight <= wallHitInfo.boundMax.x && (wallBit & (1 << agtk::data::TilesetData::Left)) ? agtk::data::ObjectActionLinkConditionData::kWallBitRight : 0);
			bit |= (wallHitInfo.hitDown >= wallHitInfo.boundMin.y && (wallBit & (1 << agtk::data::TilesetData::Up)) ? agtk::data::ObjectActionLinkConditionData::kWallBitDown : 0);
			//CCLOG("hit!! tile %p group:%d bit:0x%x", tile, tile->getTileData() ? tile->getTileData()->getGroup() : -1, bit);
			wallHitInfo.hitTiles.emplace_back(std::make_pair(bit,tile));
		};
		if (wallBit == 0x0f) {
			if (checkWallHit(wallHitInfo, rect.origin, rect.size, true, debug)) {
				hitFunc();
			}
		}
		else {

// #AGTK-NX #AGTK-WIN
#ifdef USE_SAR_PROVISIONAL_4
			// PrevWallHitInfoGroupに対象のwallHitInfo.idが無い場合の対応。
			auto prevWall = object->getObjectCollision()->getPrevWallHitInfoGroup()->findWallHitInfo(wallHitInfo.id);
			auto nowRect = object->getObjectCollision()->getWallHitInfoGroup()->findWallHitInfo(wallHitInfo.id).getRect();
			auto prevRect = nowRect;
			if (prevWall.object != nullptr) {
				prevRect = prevWall.getRect();
			}
#else
			auto prevRect = object->getObjectCollision()->getPrevWallHitInfoGroup()->findWallHitInfo(wallHitInfo.id).getRect();
			auto nowRect = object->getObjectCollision()->getWallHitInfoGroup()->findWallHitInfo(wallHitInfo.id).getRect();
#endif
			auto nextRect = cocos2d::Rect(wallHitInfo.boundMin + thresholdRect.origin, cocos2d::Size(wallHitInfo.boundMax - wallHitInfo.boundMin + thresholdRect.size));

			// 場所によっては矩形に極微小の誤差が発生している場合があるので
			// 閾値を設けてチェックを行う
			const float threshold = TILE_COLLISION_THRESHOLD;
			const float lsize = (1.0f + TILE_COLLISION_THRESHOLD * 2.0f);
			//ACT2-4606 過去と現在の矩形から壁判定チェックする。

			if (wallBit & (1 << agtk::data::TilesetData::Up)) {
				if (-velocity.y <= 0.0f && ((boundMax.y <= prevRect.getMinY() + threshold) || (boundMax.y <= nowRect.getMinY() + threshold) || (boundMax.y <= nextRect.getMinY() + threshold))) {
					float posy = (1.0f + TILE_COLLISION_THRESHOLD * 2.0f);
					if (checkWallHit(wallHitInfo, Point(boundMin.x, boundMax.y - posy), Size(boundMax.x - boundMin.x, lsize), true, debug)) {
						hitFunc();
					}
				}
			}
			if (wallBit & (1 << agtk::data::TilesetData::Left)) {
				if (velocity.x >= 0.0f && ((prevRect.getMaxX() - threshold <= boundMin.x) || (nowRect.getMaxX() - threshold <= boundMin.x) || (nextRect.getMaxX() - threshold <= boundMin.x))) {
					if (checkWallHit(wallHitInfo, boundMin, Size(lsize, boundMax.y - boundMin.y), true, debug)) {
						hitFunc();
					}
				}
			}
			if (wallBit & (1 << agtk::data::TilesetData::Right)) {
				if (velocity.x <= 0.0f && ((boundMax.x <= prevRect.getMinX() + threshold) || (boundMax.x <= nowRect.getMinX() + threshold) || (boundMax.x <= nextRect.getMinX() + threshold))) {
					float posx = (1.0f + TILE_COLLISION_THRESHOLD * 2.0f);
					if (checkWallHit(wallHitInfo, Point(boundMax.x - posx, boundMin.y), Size(lsize, boundMax.y - boundMin.y), true, debug)) {
						hitFunc();
					}
				}
			}
			if (wallBit & (1 << agtk::data::TilesetData::Down)) {
				if (-velocity.y >= 0.0f && ((prevRect.getMaxY() - threshold <= boundMin.y) || (nowRect.getMaxY() - threshold <= boundMin.y) || (nextRect.getMaxY() - threshold <= boundMin.y))) {
					float posy = 0.0f;
					if (checkWallHit(wallHitInfo, Point(boundMin.x, boundMin.y - posy), Size(boundMax.x - boundMin.x, lsize), true, debug)) {
						hitFunc();
					}
				}
			}
		}
	}
	wallHitInfo.adjustHit();
}

static bool intersectsRectWithLiftMargin(const Rect &rect1, const Rect& rect2, float margin)
{
	return !(rect1.getMaxX() < rect2.getMinX() + margin ||
		rect2.getMaxX() <      rect1.getMinX() + margin);
}

// #AGTK-NX #AGTK-WIN
#ifdef USE_SAR_FIX_AND_OPTIMIZE_1
int checkObjectWallHitDivIndex(LockObjList &lockObjList, cocos2d::Rect &crntRect, cocos2d::Vec2 move, int crntIndex, int div, MtVector<Object *>* hitObjectList)
{
	// 移動しない場合はヒットしない
	if (move == Vec2::ZERO) {
		return div + 1;
	}

	// 移動方向により8方向に分類(テンキー方向)
	int checkRectType = 5;
	if (move.x >= 0.0f)
	{
		if (move.x == 0.0f)
		{
			if (move.y > 0.0f) {
				checkRectType = 8;
			}
			else {
				checkRectType = 2;
			}
		}
		else {
			if (move.y == 0.0f) {
				checkRectType = 6;
			}
			else if (move.y > 0.0f) {
				checkRectType = 9;
			}
			else {
				checkRectType = 3;
			}
		}
	}
	else {
		if (move.y == 0.0f) {
			checkRectType = 4;
		}
		else if (move.y > 0.0f) {
			checkRectType = 7;
		}
		else {
			checkRectType = 1;
		}
	}

	cocos2d::Point sp1[3];
	cocos2d::Point ep1[3];
	CollisionLine lines1[3] = { { &sp1[0], &ep1[0] },{ &sp1[1], &ep1[1] },{ &sp1[2], &ep1[2] } };
	cocos2d::Point originOfs1[3];
	int lines1Num = 0;

	// カレント矩形の移動方向に伸びるレイを設定
	std::function<void(cocos2d::Rect)> setupRectLay = [&](cocos2d::Rect rect1) {
		switch (checkRectType)
		{
			case 1:
				lines1Num = 3;
				sp1[0] = cocos2d::Point(rect1.getMinX(), rect1.getMaxY());
				ep1[0] = sp1[0] + move;
				sp1[1] = cocos2d::Point(rect1.getMinX(), rect1.getMinY());
				ep1[1] = sp1[1] + move;
				sp1[2] = cocos2d::Point(rect1.getMaxX(), rect1.getMinY());
				ep1[2] = sp1[2] + move;
				originOfs1[0] = sp1[0] - cocos2d::Point(rect1.getMinX(), rect1.getMinY());
				originOfs1[1] = sp1[1] - cocos2d::Point(rect1.getMinX(), rect1.getMinY());
				originOfs1[2] = sp1[2] - cocos2d::Point(rect1.getMinX(), rect1.getMinY());
				break;
			case 3:
				lines1Num = 3;
				sp1[0] = cocos2d::Point(rect1.getMinX(), rect1.getMinY());
				ep1[0] = sp1[0] + move;
				sp1[1] = cocos2d::Point(rect1.getMaxX(), rect1.getMinY());
				ep1[1] = sp1[1] + move;
				sp1[2] = cocos2d::Point(rect1.getMaxX(), rect1.getMaxY());
				ep1[2] = sp1[2] + move;
				originOfs1[0] = sp1[0] - cocos2d::Point(rect1.getMinX(), rect1.getMinY());
				originOfs1[1] = sp1[1] - cocos2d::Point(rect1.getMinX(), rect1.getMinY());
				originOfs1[2] = sp1[2] - cocos2d::Point(rect1.getMinX(), rect1.getMinY());
				break;
			case 7:
				lines1Num = 3;
				sp1[0] = cocos2d::Point(rect1.getMaxX(), rect1.getMaxY());
				ep1[0] = sp1[0] + move;
				sp1[1] = cocos2d::Point(rect1.getMinX(), rect1.getMaxY());
				ep1[1] = sp1[1] + move;
				sp1[2] = cocos2d::Point(rect1.getMinX(), rect1.getMinY());
				ep1[2] = sp1[2] + move;
				originOfs1[0] = sp1[0] - cocos2d::Point(rect1.getMinX(), rect1.getMinY());
				originOfs1[1] = sp1[1] - cocos2d::Point(rect1.getMinX(), rect1.getMinY());
				originOfs1[2] = sp1[2] - cocos2d::Point(rect1.getMinX(), rect1.getMinY());
				break;
			case 9:
				lines1Num = 3;
				sp1[0] = cocos2d::Point(rect1.getMaxX(), rect1.getMinY());
				ep1[0] = sp1[0] + move;
				sp1[1] = cocos2d::Point(rect1.getMaxX(), rect1.getMaxY());
				ep1[1] = sp1[1] + move;
				sp1[2] = cocos2d::Point(rect1.getMinX(), rect1.getMaxY());
				ep1[2] = sp1[2] + move;
				originOfs1[0] = sp1[0] - cocos2d::Point(rect1.getMinX(), rect1.getMinY());
				originOfs1[1] = sp1[1] - cocos2d::Point(rect1.getMinX(), rect1.getMinY());
				originOfs1[2] = sp1[2] - cocos2d::Point(rect1.getMinX(), rect1.getMinY());
				break;
			case 2:
				lines1Num = 2;
				sp1[0] = cocos2d::Point(rect1.getMinX(), rect1.getMinY());
				ep1[0] = sp1[0] + move;
				sp1[1] = cocos2d::Point(rect1.getMaxX(), rect1.getMinY());
				ep1[1] = sp1[1] + move;
				originOfs1[0] = sp1[0] - cocos2d::Point(rect1.getMinX(), rect1.getMinY());
				originOfs1[1] = sp1[1] - cocos2d::Point(rect1.getMinX(), rect1.getMinY());
				break;
			case 4:
				lines1Num = 2;
				sp1[0] = cocos2d::Point(rect1.getMinX(), rect1.getMaxY());
				ep1[0] = sp1[0] + move;
				sp1[1] = cocos2d::Point(rect1.getMinX(), rect1.getMinY());
				ep1[1] = sp1[1] + move;
				originOfs1[0] = sp1[0] - cocos2d::Point(rect1.getMinX(), rect1.getMinY());
				originOfs1[1] = sp1[1] - cocos2d::Point(rect1.getMinX(), rect1.getMinY());
				break;
			case 6:
				lines1Num = 2;
				sp1[0] = cocos2d::Point(rect1.getMaxX(), rect1.getMinY());
				ep1[0] = sp1[0] + move;
				sp1[1] = cocos2d::Point(rect1.getMaxX(), rect1.getMaxY());
				ep1[1] = sp1[1] + move;
				originOfs1[0] = sp1[0] - cocos2d::Point(rect1.getMinX(), rect1.getMinY());
				originOfs1[1] = sp1[1] - cocos2d::Point(rect1.getMinX(), rect1.getMinY());
				break;
			case 8:
				lines1Num = 2;
				sp1[0] = cocos2d::Point(rect1.getMinX(), rect1.getMaxY());
				ep1[0] = sp1[0] + move;
				sp1[1] = cocos2d::Point(rect1.getMaxX(), rect1.getMaxY());
				ep1[1] = sp1[1] + move;
				originOfs1[0] = sp1[0] - cocos2d::Point(rect1.getMinX(), rect1.getMinY());
				originOfs1[1] = sp1[1] - cocos2d::Point(rect1.getMinX(), rect1.getMinY());
				break;
		}
	};

	int minHitIndex = div + 1;

	// rect1がmove分移動するときのrect2との接触インデックスを取得 ヒットしなければfalseを返す
	std::function<void(cocos2d::Rect, cocos2d::Rect, cocos2d::Vec2, int)> checkHit = [&](cocos2d::Rect rect1, cocos2d::Rect rect2, cocos2d::Vec2 moveVec, int checkType) {

		cocos2d::Point sp2[2];
		cocos2d::Point ep2[2];
		CollisionLine lines2[2] = { { &sp2[0], &ep2[0] },{ &sp2[1], &ep2[1] } };
		int lines2Num = 0;

		switch (checkType)
		{
			case 1:
				lines2Num = 2;
				sp2[0] = cocos2d::Point(rect2.getMaxX(), rect2.getMinY() - rect1.size.height);
				ep2[0] = cocos2d::Point(rect2.getMaxX(), rect2.getMaxY() + rect1.size.height);
				sp2[1] = cocos2d::Point(rect2.getMaxX() + rect1.size.width, rect2.getMaxY());
				ep2[1] = cocos2d::Point(rect2.getMinX() - rect1.size.width, rect2.getMaxY());
				break;
			case 3:
				lines2Num = 2;
				sp2[0] = cocos2d::Point(rect2.getMaxX() + rect1.size.width, rect2.getMaxY());
				ep2[0] = cocos2d::Point(rect2.getMinX() - rect1.size.width, rect2.getMaxY());
				sp2[1] = cocos2d::Point(rect2.getMinX(), rect2.getMaxY() + rect1.size.height);
				ep2[1] = cocos2d::Point(rect2.getMinX(), rect2.getMinY() - rect1.size.height);
				break;
			case 7:
				lines2Num = 2;
				sp2[0] = cocos2d::Point(rect2.getMinX() - rect1.size.width, rect2.getMinY());
				ep2[0] = cocos2d::Point(rect2.getMaxX() + rect1.size.width, rect2.getMinY());
				sp2[1] = cocos2d::Point(rect2.getMaxX(), rect2.getMinY() - rect1.size.height);
				ep2[1] = cocos2d::Point(rect2.getMaxX(), rect2.getMaxY() + rect1.size.height);
				break;
			case 9:
				lines2Num = 2;
				sp2[0] = cocos2d::Point(rect2.getMinX(), rect2.getMaxY() + rect1.size.height);
				ep2[0] = cocos2d::Point(rect2.getMinX(), rect2.getMinY() - rect1.size.height);
				sp2[1] = cocos2d::Point(rect2.getMinX() - rect1.size.width, rect2.getMinY());
				ep2[1] = cocos2d::Point(rect2.getMaxX() + rect1.size.width, rect2.getMinY());
				break;
			case 2:
				lines2Num = 1;
				sp2[0] = cocos2d::Point(rect2.getMaxX() + rect1.size.width, rect2.getMaxY());
				ep2[0] = cocos2d::Point(rect2.getMinX() - rect1.size.width, rect2.getMaxY());
				break;
			case 4:
				lines2Num = 1;
				sp2[0] = cocos2d::Point(rect2.getMaxX(), rect2.getMinY() - rect1.size.height);
				ep2[0] = cocos2d::Point(rect2.getMaxX(), rect2.getMaxY() + rect1.size.height);
				break;
			case 6:
				lines2Num = 1;
				sp2[0] = cocos2d::Point(rect2.getMinX(), rect2.getMaxY() + rect1.size.height);
				ep2[0] = cocos2d::Point(rect2.getMinX(), rect2.getMinY() - rect1.size.height);
				break;
			case 8:
				lines2Num = 1;
				sp2[0] = cocos2d::Point(rect2.getMinX() - rect1.size.width, rect2.getMinY());
				ep2[0] = cocos2d::Point(rect2.getMaxX() + rect1.size.width, rect2.getMinY());
				break;
		}

		for (int i = 0; i < lines1Num; i++) {
			for (int j = 0; j < lines2Num; j++) {
				cocos2d::Point cross;
				float t;
				if (CollisionUtils::checkPushCross2(&lines2[j], &lines1[i], &cross)) {
					// 実際に当たるか判定
					cocos2d::Point origin = cross + originOfs1[i];
					if (lines2[j].start->y == lines2[j].end->y) {
						// horizontal line
						if (origin.x > rect2.getMaxX() || origin.x + rect1.size.width < rect2.getMinX()) {
							// ヒットしない
							continue;
						}
					}
					else {
						// vertical line
						if (origin.y > rect2.getMaxY() || origin.y + rect1.size.height < rect2.getMinY()) {
							// ヒットしない
							continue;
						}
					}

					// 衝突する分割インデックスを求める
					if (abs(moveVec.x) > abs(moveVec.y)) {
						t = (cross.x - sp1[i].x) / moveVec.x;
					}
					else {
						t = (cross.y - sp1[i].y) / moveVec.y;
					}
					int index = (int)(t * (div - crntIndex)) + crntIndex;
					if (index < minHitIndex) {
						minHitIndex = index;
					}
				}
			}
		}
	};

	if (hitObjectList != nullptr && hitObjectList->count() > 0) {
		bool objectRemoved = false;
		for (int i = 0; i < (int)hitObjectList->size(); i++) {
			auto object2 = (*hitObjectList)[i];
#ifdef USE_MULTITHREAD_OBJECT_UPDATE
			LockObjList::AutoLocker locker(&lockObjList, object2);
#endif
			// 相手グループのWallHitInfoを取得
			std::vector<agtk::Vertex4> wallCollisionList2;
// #AGTK-NX #AGTK-WIN
#ifdef USE_30FPS_4
			// 中間フレーム情報対応のタイムライン壁判定取得
			object2->getWallTimelineList(wallCollisionList2);
#else
			object2->getTimelineList(agtk::data::TimelineInfoData::kTimelineWall, wallCollisionList2);
#endif
#ifdef USE_MULTITHREAD_OBJECT_UPDATE
			auto wallGroup2 = agtk::WallHitInfoGroup::create(object2);
			AutoDeleter<agtk::WallHitInfoGroup> deleter(wallGroup2);
			wallGroup2->addWallHitInfo(wallCollisionList2);
#else
			auto wallGroup2 = object2->getObjectCollision()->getWallHitInfoGroup();
			wallGroup2->remove();
			wallGroup2->addWallHitInfo(wallCollisionList2);
#endif
			checkHit(crntRect, wallGroup2->getRect(), move, checkRectType);
		}
	}

	// この時点でminHitIndexは minHitIndex > crntIndex && minHitIndex <= divとなっている
	// 誤差でオーバーしているインデックス値をdiv以下に制限
	if (minHitIndex < (float)div + 0.5f && minHitIndex > div)
	{
		minHitIndex = div;
	}
	// 実際に判定をとるタイミングは衝突インデックス値を切り上げたインデックスとなる
	minHitIndex = std::ceilf(minHitIndex);

	if (minHitIndex < crntIndex + 1) {
		minHitIndex = crntIndex + 1;
	}
	return minHitIndex;
}
#endif

/**
* 壁判定の衝突解決処理
* @param	object		衝突解決するオブジェクト
* @param	wallHitInfo	衝突解決するオブジェクトの壁判定情報(入力/出力)
* @param	velocity	移動量
* @param	objectList	衝突した他オブジェクトリスト
* @param	tileList	衝突したタイルリスト
* @param	slopeBit	衝突した坂の方向ビット値(出力)
* @param	slopeList	衝突した坂リスト
* @param	checkSlope	坂との衝突チェックを行うかフラグ
*/
#ifdef USE_MULTITHREAD_OBJECT_UPDATE
static void newCalcWallHitInfo(LockObjList &lockObjList, agtk::Object* object, WallHitInfo &wallHitInfo, cocos2d::Vec2& velocity, agtk::NrArray* objectList, std::vector<agtk::Tile *> &tileList, int* slopeBit, agtk::MtVector<agtk::Slope *> *slopeList, bool checkSlope, int *tileWallBit, ObjectCollision::HitTileWalls *pLinkConditionTileWall);
void newCalcWallHitInfo(LockObjList &lockObjList, agtk::Object* object, WallHitInfo &wallHitInfo, cocos2d::Vec2& velocity, agtk::NrArray* objectList, std::vector<agtk::Tile *> &tileList, int* slopeBit, agtk::MtVector<agtk::Slope *> *slopeList, bool checkSlope, int *tileWallBit, ObjectCollision::HitTileWalls *pLinkConditionTileWall)
#else
static void newCalcWallHitInfo(agtk::Object* object, WallHitInfo &wallHitInfo, cocos2d::Vec2& velocity, cocos2d::Array* objectList, cocos2d::Array *tileList, int* slopeBit, cocos2d::Array* slopeList, bool checkSlope);
void newCalcWallHitInfo(agtk::Object* object, WallHitInfo &wallHitInfo, cocos2d::Vec2& velocity, cocos2d::Array* objectList, cocos2d::Array *tileList, int* slopeBit, cocos2d::Array* slopeList, bool checkSlope)
#endif
{
	//THREAD_PRINTF("newCalcWallHitInfo(0x%x)", &lockObjList);
	cocos2d::Ref *ref = nullptr;
	auto objectCourseMove = object->getObjectCourseMove();
	auto objectMovement = object->getObjectMovement();
	auto objectTemplateMove = object->getObjectTemplateMove();
	auto objectData = object->getObjectData();

#ifdef USE_30FPS_3
	auto gm = GameManager::getInstance();
	auto passCount = gm->getPassCount();
	auto bLastPass = gm->getLastPass();
#endif

#ifdef USE_MULTITHREAD_OBJECT_UPDATE
	/*// 接触判定オブジェクトリスト
	MtVector<agtk::Object *> hitObjectList;
	if (objectList != nullptr && objectList->count() > 0 && objectTemplateMove->isIgnoredObjectWall() == false) {
		for (auto ref : *objectList) {
			auto object = dynamic_cast<agtk::Object *>(ref);
			hitObjectList.push_back(object);
		}
	}
	// 接触判定オブジェクトリスト 判定不要オブジェクト削除用
	MtVector<agtk::Object *> tmpHitObjectList;*/
	// 接触判定オブジェクトリスト 判定不要オブジェクト削除用
	auto tmpHitObjectList = new MtVector<Object *>();
	AutoDeleter<MtVector<Object *>> deleter(tmpHitObjectList);
	tmpHitObjectList->addObjectsFromArray(objectList);
	// 接触判定オブジェクトリスト
	auto hitObjectList = new MtVector<Object *>();
	AutoDeleter<MtVector<Object *>> deleter2(hitObjectList);
	hitObjectList->addObjectsFromArray(tmpHitObjectList);
#else
	// 接触判定オブジェクトリスト 判定不要オブジェクト削除用
	cocos2d::Array* tmpHitObjectList = cocos2d::__Array::create();
	tmpHitObjectList->addObjectsFromArray(objectList);
	// 接触判定オブジェクトリスト
	cocos2d::Array* hitObjectList = cocos2d::__Array::create();
	hitObjectList->addObjectsFromArray(tmpHitObjectList);
#endif

	// 現在の位置の矩形を取得
	Rect crntRect = Rect(wallHitInfo.boundMin, Size(wallHitInfo.boundMax - wallHitInfo.boundMin));
	// 過去の位置の矩形を取得
	Rect prevRect = Rect(Point(crntRect.getMinX() - velocity.x, crntRect.getMinY() + velocity.y), crntRect.size);
	//過去の位置保持用（※prevRectは壁に当たると変化するため、そのままの値が必要）
	const Rect _prevRect = prevRect;

	// タイルとの接触を確認するか？
	bool checkCollideTile = true;
	// 坂との接触を確認するか？
	bool checkCollideSlope = true;

	// tile
	//壁判定の設定（タイルとぶつかる）
	//テンプレート移動で「タイル判定を無視する」の場合
	if (objectData->getCollideWithTileGroupBit() == 0 ||
		objectTemplateMove->isIgnoredTileWall()) {
		// タイルとの接触を確認しない
		checkCollideTile = false;
	}

	// コース移動中
	if (objectCourseMove && objectCourseMove->isMoving()) {
		// タイルとの接触を確認しない
		checkCollideTile = false;
		// 坂との接触を確認しない
		checkCollideSlope = false;
	}

	// タイルの引っかかり調整用の閾値(ドット)
	int tileThreshold = objectData->getDotsToMoveAround();

	// 引っかかりの自動調整を行わない場合は閾値を0にする
	if (!objectData->getMoveAroundWithinDots()) {
		tileThreshold = 0;
	}

	bool isPushBack = false;
	bool isSlopeHit = false;

	// タイルとの接触を確認する場合、オブジェクトの過去の位置が埋まっているかをチェック
	if (checkCollideTile && checkBuriedWall(prevRect, tileList))
	{
		// 押し戻し前の過去の位置を保持
		auto oldPreRect = prevRect;

		// 埋まっている場合は押し戻しを試みる
// #AGTK-NX
#if (CC_TARGET_PLATFORM == CC_PLATFORM_NX)
#else
		isPushBack = pushBackBuriedWall(object, prevRect, Vec2(velocity.x, -velocity.y), tileList);
#endif

		// 押し戻しが行われなかった場合
		if (!isPushBack) {

			// 前フレーム時に壁に埋まっているか？
			bool buriedInWallFlag = object->getBuriedInWallFlag();

			// 埋まった直後の場合
			if (!buriedInWallFlag) {

				// 押し戻しを算出
				float moveX = pushBackBuriedWallX(prevRect, velocity.x, tileList);
				float moveY = pushBackBuriedWallY(prevRect, -velocity.y, tileList);

				wallHitInfo.boundMin = Point(prevRect.getMinX() + moveX, prevRect.getMinY() + moveY);
			}
			else {
				// 埋まっている場合は動かさない
				wallHitInfo.boundMin = Point(prevRect.getMinX(), prevRect.getMinY());
			}

			// 埋まっている
			object->setBuriedInWallFlag(true);

			return;
		}
		// 押し戻しが行われた場合
		else {
			// 押し戻された分を現在の判定用矩形にも反映する
			crntRect.origin += (prevRect.origin - oldPreRect.origin);
		}
	}

	// 坂との接触を確認
	if (checkCollideSlope && checkSlope) {
		if (slopeList->count() > 0) {
			bool hitUp, hitDown;
			Vec2 vec = crntRect.origin - prevRect.origin;

			auto playObjectData = object->getPlayObjectData();
			bool isSlip = playObjectData->getSwitchData(agtk::data::kObjectSystemSwitchSlipOnSlope)->getValue();
			if (newCheckSlopeHit(object, hitUp, hitDown, slopeList, vec, isSlip, tileList)) {
				// 足元が坂に接触している
				if (hitDown) {
					*slopeBit |= agtk::data::ObjectActionLinkConditionData::kSlopeBitDown;
				}
				if (hitUp) {
					*slopeBit |= agtk::data::ObjectActionLinkConditionData::kSlopeBitUp;
				}

				if (!hitDown && !hitUp) {
					objectMovement->resetX();
				}
				else {
					isSlopeHit = true;
				}

				// 現在の位置を変更する
				crntRect.origin += vec;
			}
		}
		else if (object->getPassableSlopeTouchedList()->count() > 0) {
			object->getPassableSlopeTouchedList()->removeAllObjects();
		}
	}

	// 現在の位置の矩形と過去の位置の矩形をマージして移動範囲矩形を作成
	Rect mergeRect = crntRect;
	mergeRect.merge(prevRect);

	// 押し戻し分を考慮した移動範囲矩形を作成
	Rect chkMoveRect = mergeRect;
	// 左移動分
	float pushBackX = (chkMoveRect.getMaxX() - wallHitInfo.boundMin.x) + TILE_COLLISION_THRESHOLD;
	chkMoveRect.origin.x -= pushBackX;
	chkMoveRect.size.width += pushBackX;
	// 右移動分
	chkMoveRect.size.width += pushBackX;
	// 下移動分
	float pushBackY = (chkMoveRect.getMaxY() - wallHitInfo.boundMin.y) + TILE_COLLISION_THRESHOLD;
	chkMoveRect.origin.y -= pushBackY;
	chkMoveRect.size.height += pushBackY;
	// 上移動分
	chkMoveRect.size.height += pushBackY;

	// 移動範囲の矩形がタイルと接触が発生しているかをチェック
	bool isHitSlopeTileList = false;
	if (checkCollideTile)
	{
#ifdef USE_MULTITHREAD_OBJECT_UPDATE
		for (auto tile : tileList) {
#else
		CCARRAY_FOREACH(tileList, ref) {
			auto tile = dynamic_cast<agtk::Tile*>(ref);
#endif
			auto rect = tile->convertToLayerSpaceRect();
			auto pos = rect.origin;
			auto size = rect.size;
			pos.x += TILE_COLLISION_THRESHOLD;
			pos.y += TILE_COLLISION_THRESHOLD;
			size.width -= TILE_COLLISION_THRESHOLD * 2;
			size.height -= TILE_COLLISION_THRESHOLD * 2;
			Rect wallRect = Rect(pos, size);

			if (wallRect.intersectsRect(chkMoveRect)) {
				isHitSlopeTileList = true;
				break;
			}
		}
	}

	// 移動範囲の矩形が坂と接触が発生しているかをチェック
	if (!isHitSlopeTileList && checkSlope) {
#ifdef USE_MULTITHREAD_OBJECT_UPDATE
		for (int i = 0; i < (int)slopeList->size(); i++) {
			auto slope = (*slopeList)[i];
#else
		CCARRAY_FOREACH(slopeList, ref) {
			auto slope = dynamic_cast<agtk::Slope*>(ref);
#endif
			// 移動先の接触チェック
			if (slope->checkHitRect(crntRect)) {
				isHitSlopeTileList = true;
				break;
			}
			// 接触チェック
			if (slope->checkHitRect(chkMoveRect)) {
				isHitSlopeTileList = true;
				break;
			}
			auto slopeRect = slope->getWorldSpaceRect();
			auto slopeLeftDown = cocos2d::Vec2(slopeRect.getMinX(), slopeRect.getMinY());
			auto slopeLeftUp = cocos2d::Vec2(slopeRect.getMinX(), slopeRect.getMaxY());
			auto slopeRightDown = cocos2d::Vec2(slopeRect.getMaxX(), slopeRect.getMinY());
			auto slopeRightUp = cocos2d::Vec2(slopeRect.getMaxX(), slopeRect.getMaxY());
			if (chkMoveRect.containsPoint(slopeLeftDown) && chkMoveRect.containsPoint(slopeLeftUp) && chkMoveRect.containsPoint(slopeRightDown) && chkMoveRect.containsPoint(slopeRightUp)) {
				isHitSlopeTileList = true;
				break;
			}
		}
	}

	// 移動量分割数を算出
	float rectLengthSq = Vec2(crntRect.size.width, crntRect.size.height).getLengthSq();
	float moveRectLengthSq = Vec2(mergeRect.size.width, mergeRect.size.height).getLengthSq();
	int div = 2;
	if (!(velocity.x == 0 && velocity.y == 0) && rectLengthSq > 0.001f) {//rectLengthSqの値を小数点第３位までにする。
		div = (int)roundf(moveRectLengthSq / rectLengthSq);
	}

	if (div <= 1) {
		div = 2;
	}

// #AGTK-NX #AGTK-WIN
#ifndef USE_30FPS_3
#ifdef USE_30FPS_2
	// 30FPSの時は分割判定の中間地点でのオブジェクト位置を記録するために分割数を偶数にする
	int passCount = (int)GameManager::getInstance()->getFrameProgressScale();
	if (passCount > 1 && div % 2 != 0) {
		div++;
	}

	// 最も中間地点に近い壁判定結果の保持用
	cocos2d::Vec2 iniCrntWallPos = cocos2d::Vec2(crntRect.getMinX(), crntRect.getMinY());
	cocos2d::Vec2 iniPrevWallPos = cocos2d::Vec2(prevRect.getMinX(), prevRect.getMinY());
	float maxHalfDist = velocity.length() * 0.5f;
	float closestHalfDist = -1.0f;
	cocos2d::Vec2 closestHalfWallPos;
#endif
#endif

	auto projectData = GameManager::getInstance()->getProjectData();
	auto sceneSize = projectData->getSceneSize(object->getSceneData());
	if (div > (int)Vec2(sceneSize.width, sceneSize.height).getLength()) {
		//スクリーンの対角線より大きい分割数の場合は、移動量矩形の対角線分を分割数とする。
		//div = (int)Vec2(mergeRect.size.width, mergeRect.size.height).getLength();
		//ACT2-6410: スクリーンの対角線より大きい分割数の場合は、移動量矩形の対角線分を分割数とする。ただし、スクリーンの対角線の2倍を上限とする。
		div = std::min((int)(Vec2(sceneSize.width, sceneSize.height).getLength() * 2), (int)Vec2(mergeRect.size.width, mergeRect.size.height).getLength());
	}

	Vec2 pos, newPos, moveVec;
	// 移動量を分割して接触判定を行う
// #AGTK-NX #AGTK-WIN
#ifdef USE_SAR_FIX_AND_OPTIMIZE_1
	// 移動範囲のタイルリストを保存
	std::vector<agtk::Tile *> orgTileList = tileList;

	// 事前に判定不要のオブジェクトを判定対象から削除
#ifdef USE_MULTITHREAD_OBJECT_UPDATE
	if (hitObjectList != nullptr && hitObjectList->count() > 0 && objectTemplateMove->isIgnoredObjectWall() == false) {
		bool objectRemoved = false;
		for (int i = 0; i < (int)hitObjectList->size(); i++) {
			auto object2 = (*hitObjectList)[i];
#else
	if (hitObjectList != nullptr && hitObjectList->count() > 0 && objectTemplateMove->isIgnoredObjectWall() == false) {
		CCARRAY_FOREACH(hitObjectList, ref) {
			auto object2 = dynamic_cast<agtk::Object *>(ref);
#endif
			auto objectData2 = object2->getObjectData();

			// object2 が Disabled の場合
			if (object2->getDisabled()) {
				tmpHitObjectList->removeObject(object2);
#ifdef USE_MULTITHREAD_OBJECT_UPDATE
				objectRemoved = true;
#endif
				continue;
			}

			// レイヤーが違うオブジェクトの場合
			if (object->getLayerId() != object2->getLayerId()) {
				tmpHitObjectList->removeObject(object2);
#ifdef USE_MULTITHREAD_OBJECT_UPDATE
				objectRemoved = true;
#endif
				continue;
			}

			// 相手グループとは接触しない場合
			if (!(objectData->isCollideWith(objectData2))) {
#ifdef USE_MULTITHREAD_OBJECT_UPDATE
				objectRemoved = true;
#endif
				tmpHitObjectList->removeObject(object2);
				continue;
			}

			auto objectTemplateMove2 = object2->getObjectTemplateMove();
			// 接触したオブジェクトがテンプレート移動中で他オブジェクトの壁判定を無視する設定の場合
			if (objectTemplateMove2 != nullptr && objectTemplateMove2->isIgnoredObjectWall()) {
#ifdef USE_MULTITHREAD_OBJECT_UPDATE
				objectRemoved = true;
#endif
				tmpHitObjectList->removeObject(object2);
				continue;
			}

#ifdef USE_MULTITHREAD_OBJECT_UPDATE
			LockObjList::AutoLocker locker(&lockObjList, object2);
#endif
			// 相手グループのWallHitInfoを取得
			std::vector<agtk::Vertex4> wallCollisionList2;
// #AGTK-NX #AGTK-WIN
#ifdef USE_30FPS_4
			// 中間フレーム情報対応のタイムライン壁判定取得
			object2->getWallTimelineList(wallCollisionList2);
#else
			object2->getTimelineList(agtk::data::TimelineInfoData::kTimelineWall, wallCollisionList2);
#endif
#ifdef USE_MULTITHREAD_OBJECT_UPDATE
			auto wallGroup2 = agtk::WallHitInfoGroup::create(object2);
			AutoDeleter<agtk::WallHitInfoGroup> deleter(wallGroup2);
			wallGroup2->addWallHitInfo(wallCollisionList2);
#else
			auto wallGroup2 = object2->getObjectCollision()->getWallHitInfoGroup();
			wallGroup2->remove();
			wallGroup2->addWallHitInfo(wallCollisionList2);
#endif
			// 移動範囲の矩形内に相手オブジェクトの矩形がないかチェックする
			if (!chkMoveRect.intersectsRect(wallGroup2->getRect())) {
				// なければ接触チェックが必要ないのでリストから削除しておく
#ifdef USE_MULTITHREAD_OBJECT_UPDATE
				objectRemoved = true;
#endif
				tmpHitObjectList->removeObject(object2);
				continue;
			}
		}
		// 削除したオブジェクト分を反映
#ifdef USE_MULTITHREAD_OBJECT_UPDATE
		if (objectRemoved) {
#else
		if (hitObjectList->count() != tmpHitObjectList->count()) {
#endif
			hitObjectList->removeAllObjects();
			hitObjectList->addObjectsFromArray(tmpHitObjectList);
		}
	}

	//object_start
	float cmpCrntX;
	float cmpCrntY;
	float cmpPrevX;
	float cmpPrevY;
	bool bExit = false;
	bool bSkipHitCheck = false;
	int recoveryDevIndex;
	int i = 0;
	while(i <= div) {
// #AGTK-NX #AGTK-WIN
#ifndef USE_30FPS_3
#ifdef USE_30FPS_2
		// 中間地点の壁判定結果を記録
		if (i > 0 && passCount > 1) {
			float checkDist = (wallHitInfo.boundMin - iniPrevWallPos).length();
			if(checkDist > 0 && checkDist <= maxHalfDist && closestHalfDist < checkDist) {
				closestHalfDist = checkDist;
				closestHalfWallPos = wallHitInfo.boundMin;
				// SAR ohnishi test
				//CCLOG("attk half : %d, dist %d, wall (%d, %d)", i, (int)closestHalfDist, (int)closestHalfWallPos.x, (int)closestHalfWallPos.y);
			}
		}
#endif
#endif
		// 前回の分割毎判定で現在の矩形の移動が無かった場合は次に接触する分割インデックスを予測
		if (!bSkipHitCheck &&
			i > 0 && div - i > 1 &&
			crntRect.getMinX() == cmpCrntX && crntRect.getMinY() == cmpCrntY &&
			prevRect.getMinX() == cmpPrevX && prevRect.getMinY() == cmpPrevY ) {
			cocos2d::Rect checkRect = Rect(newPos, wallHitInfo.size);
			cocos2d::Vec2 moveVec = Vec2(crntRect.getMinX() - newPos.x, crntRect.getMinY() - newPos.y);
			int minHitIndex = div + 1;
			if (objectTemplateMove->isIgnoredObjectWall() == false) {
				int objHitIndex = checkObjectWallHitDivIndex(lockObjList, checkRect, moveVec, i, div, hitObjectList);
				if (objHitIndex < minHitIndex) {
					minHitIndex = objHitIndex;
				}
				// SAR ohnishi test
				//CCLOG("divide collision objHitIndex : %d->%d %d", i, objHitIndex, div);
			}
			int wallHitIndex = checkWallHitDivIndex(object, checkRect, moveVec, wallHitInfo.id, i, div, orgTileList, slopeList, object->getPassableSlopeTouchedList());
			if (wallHitIndex < minHitIndex) {
				minHitIndex = wallHitIndex;
			}
			// SAR ohnishi test
			//CCLOG("divide collision wallHitIndex : %d->%d %d", i, wallHitIndex, div);
			int oldDevIndex = i;
			if (minHitIndex > div) {
				// ループを終了するときはWaiiHitInfoを更新してから抜けたい
				bExit = true;
				i = div;
			} else {
				i = minHitIndex;
			}
// #AGTK-NX #AGTK-WIN
#ifndef USE_30FPS_3
#ifdef USE_30FPS_2
			// 30FPSで分割判定がスキップされる時、中間地点のwallHitInfoを更新できるようにする
			if (passCount > 1 && oldDevIndex < div / 2 && i > div / 2) {
				bSkipHitCheck = true;
				recoveryDevIndex = i;
				i = div / 2;
			}
#endif
#endif
		}
		else {
// #AGTK-NX #AGTK-WIN
#ifndef USE_30FPS_3
			if (bSkipHitCheck) {
				bSkipHitCheck = false;
				i = recoveryDevIndex;
			}
			else {
#endif
				cmpCrntX = crntRect.getMinX();
				cmpCrntY = crntRect.getMinY();
				cmpPrevX = prevRect.getMinX();
				cmpPrevY = prevRect.getMinY();
				i++;
				if (i > div) {
					break;
				}
// #AGTK-NX #AGTK-WIN
#ifndef USE_30FPS_3
			}
#endif
		}

		// 分割後の過去の矩形
		pos.x = (prevRect.getMinX() == crntRect.getMinX()) ? crntRect.getMinX() : MathUtil::lerp(prevRect.getMinX(), crntRect.getMinX(), (float)(i - 1) / div);
		pos.y = (prevRect.getMinY() == crntRect.getMinY()) ? crntRect.getMinY() : MathUtil::lerp(prevRect.getMinY(), crntRect.getMinY(), (float)(i - 1) / div);

		// 分割後の現在の矩形
		newPos.x = (prevRect.getMinX() == crntRect.getMinX()) ? crntRect.getMinX() : MathUtil::lerp(prevRect.getMinX(), crntRect.getMinX(), (float)i / div);
		newPos.y = (prevRect.getMinY() == crntRect.getMinY()) ? crntRect.getMinY() : MathUtil::lerp(prevRect.getMinY(), crntRect.getMinY(), (float)i / div);

		// 移動量
		moveVec = newPos - pos;

		// 分割後の現在座標をwallHitInfoに保持
		wallHitInfo.boundMin = newPos;
		wallHitInfo.boundMax.x = wallHitInfo.boundMin.x + crntRect.size.width;
		wallHitInfo.boundMax.y = wallHitInfo.boundMin.y + crntRect.size.height;
		wallHitInfo.center.x = wallHitInfo.boundMin.x + crntRect.size.width * 0.5f;
		wallHitInfo.center.y = wallHitInfo.boundMin.y + crntRect.size.height * 0.5f;

#ifndef USE_30FPS_3
		// 30FPSの時、ヒットしないことが事前に判明しているが中間地点のwallHitInfoを更新する場合の判定
		if (bSkipHitCheck) {
			continue;
		}
#endif

		// ヒットしないことが事前に判明しているがwallHitInfoを更新して終了する場合の判定
		if (bExit) {
			break;
		}

		bool hitX = false;
		bool hitY = false;

		//object_start
#ifdef USE_MULTITHREAD_OBJECT_UPDATE
		if (hitObjectList != nullptr && hitObjectList->count() > 0 && objectTemplateMove->isIgnoredObjectWall() == false) {
			for (int i = 0; i < (int)hitObjectList->size(); i++) {
				auto object2 = (*hitObjectList)[i];
#else
		if (hitObjectList != nullptr && hitObjectList->count() > 0 && objectTemplateMove->isIgnoredObjectWall() == false) {
			CCARRAY_FOREACH(hitObjectList, ref) {
				auto object2 = dynamic_cast<agtk::Object *>(ref);
#endif
				auto objectData2 = object2->getObjectData();

#ifdef USE_MULTITHREAD_OBJECT_UPDATE
				LockObjList::AutoLocker locker(&lockObjList, object2);
#endif
				// 相手グループのWallHitInfoを取得
				std::vector<agtk::Vertex4> wallCollisionList2;
// #AGTK-NX, #AGTK-WIN
#ifdef USE_30FPS_4
				// 中間フレーム情報対応のタイムライン壁判定取得
				object2->getWallTimelineList(wallCollisionList2);
#else
				object2->getTimelineList(agtk::data::TimelineInfoData::kTimelineWall, wallCollisionList2);
#endif
#ifdef USE_MULTITHREAD_OBJECT_UPDATE
				auto wallGroup2 = agtk::WallHitInfoGroup::create(object2);
				AutoDeleter<agtk::WallHitInfoGroup> deleter(wallGroup2);
				wallGroup2->addWallHitInfo(wallCollisionList2);
#else
				auto wallGroup2 = object2->getObjectCollision()->getWallHitInfoGroup();
				wallGroup2->remove();
				wallGroup2->addWallHitInfo(wallCollisionList2);
#endif
				// 相手オブジェクトと接触しているかをチェックする
				auto objRect = Rect(wallHitInfo.boundMin, Size(wallHitInfo.boundMax - wallHitInfo.boundMin));
				if (!objRect.intersectsRect(wallGroup2->getRect())) {
					continue;
				}

				// 接触が発生している場合
				auto objectCouseMove2 = object2->getObjectCourseMove();
				// 自身が押し戻される設定時
				if (object->getPushedbackByObject()) {
					// 相手は押し戻されない場合
					if (!object2->getPushedbackByObject()) {

						// 自身がコース移動中の場合、押し戻されるとコースから外れるので処理しない
						if (objectCourseMove && objectCourseMove->isMoving()) {
							continue;
						}

						Rect pr = prevRect;
						pushBackBuriedObjectWall(pr, crntRect, moveVec, wallGroup2);

						Vec2 mv = newPos - pr.origin;

						//移動せずに壁に食い込んだ場合。
						if (moveVec == cocos2d::Vec2::ZERO) {
							mv = cocos2d::Vec2::ZERO;
						}

						// 自身を押し戻す
						newCheckWallHit(wallHitInfo, hitX, hitY, wallGroup2, mv, moveVec);
					}
					// 相手も押し戻される場合
					else {
						// 相手がコース移動中でない場合
						if (objectCouseMove2 == nullptr || !objectCouseMove2->isMoving()) {
							// オブジェクトの押し出しを試みる
#ifdef USE_MULTITHREAD_OBJECT_UPDATE
							object2->getObjectWallCollision()->pushObject(lockObjList, wallGroup2, moveVec, crntRect, object, true);
#else
							object2->getObjectWallCollision()->pushObject(moveVec, crntRect, object);
#endif

							// 押し出す壁当たり判定を取得しなおす
							wallCollisionList2.clear();
// #AGTK-NX #AGTK-WIN
#ifdef USE_30FPS_4
							// 中間フレーム情報対応のタイムライン壁判定取得
							object2->getWallTimelineList(wallCollisionList2);
#else
							object2->getTimelineList(agtk::data::TimelineInfoData::kTimelineWall, wallCollisionList2);
#endif
#ifdef USE_MULTITHREAD_OBJECT_UPDATE
							wallGroup2->remove();
							wallGroup2->addWallHitInfo(wallCollisionList2);
#else
							wallGroup2 = object2->getObjectCollision()->getWallHitInfoGroup();
							wallGroup2->remove();
							wallGroup2->addWallHitInfo(wallCollisionList2);
#endif
						}

						// 自身がコース移動中の場合、押し戻されるとコースから外れるので処理しない
						if (objectCourseMove && objectCourseMove->isMoving()) {
							continue;
						}

						// 押し出したオブジェクトがタイルに接触等で押し戻しできない場合もあるので
						// 自身の押し戻し処理を行う
						Rect pr = prevRect;
						pushBackBuriedObjectWall(pr, crntRect, moveVec, wallGroup2);

						Vec2 mv = newPos - pr.origin;

						//移動せずに壁に食い込んだ場合。
						if (moveVec == cocos2d::Vec2::ZERO) {
							mv = cocos2d::Vec2::ZERO;
						}
						// 自身を押し戻す
						newCheckWallHit(wallHitInfo, hitX, hitY, wallGroup2, mv, moveVec);
					}
				}
				// 自身が押し戻されない場合
				else {
					// 相手も押し戻されない場合
					if (!object2->getPushedbackByObject()) {

						// 自身がコース移動中の場合、押し戻されるとコースから外れるので処理しない
						if (objectCourseMove && objectCourseMove->isMoving()) {
							continue;
						}

						Rect pr = prevRect;

						//坂の上に立っている場合かつX方向への移動が無い場合は、閾値以下のY方向移動量の場合は０とする。
						if (*slopeBit & agtk::data::ObjectActionLinkConditionData::kSlopeBitDown) {
							const float threshold = 0.0001f;
							if (moveVec.x == 0 && moveVec.y < 0 && abs(moveVec.y) < threshold) {
								moveVec = cocos2d::Vec2::ZERO;
							}
						}

						pushBackBuriedObjectWall(pr, crntRect, moveVec, wallGroup2);
						Vec2 mv = newPos - pr.origin;

						//移動せずに壁に食い込んだ場合。
						if (moveVec == cocos2d::Vec2::ZERO) {
							mv = cocos2d::Vec2::ZERO;
						}

						//「カメラとの位置関係を固定する」設定しているオブジェクトで、相対するオブジェクトは設定無しの場合。
						auto sceneLayer = object->getSceneLayer();
						if ((objectData->getFixedInCamera() == true && objectData2->getFixedInCamera() == false) && (sceneLayer && sceneLayer->getType() != agtk::SceneLayer::kTypeMenu)) {
							// オブジェクトの押し出しを試みる
#ifdef USE_MULTITHREAD_OBJECT_UPDATE
							object2->getObjectWallCollision()->pushObject(lockObjList, wallGroup2, mv, crntRect, object);
#else
							object2->getObjectWallCollision()->pushObject(mv, crntRect, object);
#endif
						} else {
							// 自身を押し戻す
							if (mv != cocos2d::Vec2::ZERO || moveVec != cocos2d::Vec2::ZERO) {// 移動量がゼロ以外の場合
								newCheckWallHit(wallHitInfo, hitX, hitY, wallGroup2, mv, moveVec);
							}
						}
					}
					// 相手は押し戻される場合
					else {
						Vec2 mv = moveVec;

						// 相手がコース移動中でない場合
						if (objectCouseMove2 == nullptr || !objectCouseMove2->isMoving()) {

							// オブジェクトに移動が発生しなかった場合、前フレームのオブジェクト矩形を基に押し出し方向の算出を行う
							if (mv.x == 0 && mv.y == 0) {
								auto w = object->getObjectCollision()->getOldWallHitInfoGroup()->findWallHitInfo(wallHitInfo.id);
// #AGTK-NX #AGTK-WIN
#ifdef USE_SAR_PROVISIONAL_4
								// PrevWallHitInfoGroupに対象のwallHitInfo.idが無い場合の対応
								if (w.object != nullptr && w.id >= 0) {
#else
								if (w.id >= 0) {
#endif
									mv.x = prevRect.getMidX() - w.getRect().getMidX();//Xは中心基準
									mv.y = prevRect.getMinY() - w.getRect().getMinY();//Yは足元基準
								}
								//mv.x = prevRect.getMidX() - object->getObjectCollision()->getOldWallHitInfoGroup()->getRect().getMidX();//Xは中心基準
								//mv.y = prevRect.getMinY() - object->getObjectCollision()->getOldWallHitInfoGroup()->getRect().getMinY();//Yは足元基準
							}

							// オブジェクトの押し出しを試みる
#ifdef USE_MULTITHREAD_OBJECT_UPDATE
							object2->getObjectWallCollision()->pushObject(lockObjList, wallGroup2, mv, crntRect, object);
#else
							object2->getObjectWallCollision()->pushObject(mv, crntRect, object);
#endif

							// 押し出す壁当たり判定を取得しなおす
							wallCollisionList2.clear();
// #AGTK-NX #AGTK-WIN
#ifdef USE_30FPS_4
							// 中間フレーム情報対応のタイムライン壁判定取得
							object2->getWallTimelineList(wallCollisionList2);
#else
							object2->getTimelineList(agtk::data::TimelineInfoData::kTimelineWall, wallCollisionList2);
#endif
#ifdef USE_MULTITHREAD_OBJECT_UPDATE
							wallGroup2->remove();
							wallGroup2->addWallHitInfo(wallCollisionList2);
#else
							wallGroup2 = object2->getObjectCollision()->getWallHitInfoGroup();
							wallGroup2->remove();
							wallGroup2->addWallHitInfo(wallCollisionList2);
#endif
						}

						// 自身がコース移動中の場合、押し戻されるとコースから外れるので処理しない
						if (objectCourseMove && objectCourseMove->isMoving()) {
							continue;
						}

						if (mv.x == 0 && mv.y == 0) {
							//移動してなければ、自身の押し戻しは不要。
							continue;
						}
						// 押し出したオブジェクトがタイルに接触等で押し戻しできない場合もあるので
						// 自身の押し戻し処理を行う
						Rect pr = prevRect;
						pushBackBuriedObjectWall(pr, crntRect, mv, wallGroup2);

						mv = newPos - pr.origin;
						//移動せずに壁に食い込んだ場合。
						if (moveVec == cocos2d::Vec2::ZERO) {
							mv = cocos2d::Vec2::ZERO;
						}

						// 自身を押し戻す
						newCheckWallHit(wallHitInfo, hitX, hitY, wallGroup2, mv, moveVec);
					}
				}

				// 接触が発生している場合
				if (hitX) {
					crntRect.origin.x = wallHitInfo.boundMin.x;
				}

				if (hitY) {
					crntRect.origin.y = wallHitInfo.boundMin.y;
				}

				if (hitX || hitY) {
					moveVec = wallHitInfo.boundMin - prevRect.origin;
				}
			}
		}
		//object_end

		// tile_start
		// タイルとの接触確認を行わない場合はスキップする
		if (!checkCollideTile) {
			wallHitInfo.adjustHit();
			continue;
		}

		// タイルリストを取得
// #AGTK-NX #AGTK-WIN
#ifdef USE_SAR_PROVISIONAL_4
		// OldWallHitInfoGroupに対象のwallHitInfo.idが存在しない場合に不要なタイルを取得して誤った位置補正を行ってしまう事がある問題に対応
		auto oldWall = object->getObjectCollision()->getOldWallHitInfoGroup()->findWallHitInfo(wallHitInfo.id);
		auto oldWallHitRect = wallHitInfo.getRect();
		Vec2 tileThresholdMoveVec = Vec2::ZERO;
		if (oldWall.object != nullptr) {
			oldWallHitRect = oldWall.getRect();
			tileThresholdMoveVec = oldWallHitRect.origin - crntRect.origin;
		}
#else
		auto oldWallHitRect = object->getObjectCollision()->getOldWallHitInfoGroup()->findWallHitInfo(wallHitInfo.id).getRect();
		Vec2 tileThresholdMoveVec = oldWallHitRect.origin - crntRect.origin;
#endif
		// ジャンプ中＆落下中でない場合。
		if (object->_jumping == false && object->_falling == false && moveVec != cocos2d::Vec2::ZERO && !object->isExternalValueXy()) {
			// ACT2-2902 ジャンプ中＆落下中でない場合は、新しくタイル情報を得るようにする（※この条件以外でタイル情報を得るとジャンプして床に張り付くバグが出ます。
			// ACT2-2847 移動により物理オブジェクトの位置設定により強制的に物理タイルを押すと座標が異常値になることがあるため、強制移動しないようにここで食い止める。
			// ACT2-3575 壁判定がタイルの壁判定に埋まった後の挙動不具合（XY座標変数変更した場合はこの処理をしないようにする。
			Vec2 _moveVec = newPos - cocos2d::Vec2(prevRect.getMinX(), prevRect.getMinY());
			tileList = object->getObjectCollision()->getWallCollisionTileList(_moveVec + tileThresholdMoveVec, object->getObjectData()->getCollideWithTileGroupBit());
		}

		// 移動範囲の矩形とタイルが接触しない場合はスキップする
		if (!isHitSlopeTileList) {
#if defined(USE_30FPS_2) && !defined(USE_30FPS_3)
			// 中間地点を記録するためにここではループを抜けない
#else
			// 接触するオブジェクトもない場合はwallHitInfoを更新して判定を終了する
			if (hitObjectList->count() == 0) {
				wallHitInfo.boundMin = crntRect.origin;
				wallHitInfo.boundMax.x = wallHitInfo.boundMin.x + crntRect.size.width;
				wallHitInfo.boundMax.y = wallHitInfo.boundMin.y + crntRect.size.height;
				wallHitInfo.center.x = wallHitInfo.boundMin.x + crntRect.size.width * 0.5f;
				wallHitInfo.center.y = wallHitInfo.boundMin.y + crntRect.size.height * 0.5f;
				wallHitInfo.adjustHit();
				break;
			}
#endif
			wallHitInfo.adjustHit();
			continue;
		}

		hitX = false;
		hitY = false;
		agtk::HitWallTiles hitTiles;
		// 分割後の現在座標がタイルと接触発生しているかをチェック
		newCheckWallHit(wallHitInfo, hitX, hitY, tileThreshold, tileList, slopeList, object->getPassableSlopeTouchedList(), moveVec, tileThresholdMoveVec, oldWallHitRect, object->_falling, isSlopeHit, hitTiles);

		// 接触が発生している場合
		if (hitX) {
			crntRect.origin.x = wallHitInfo.boundMin.x;
			prevRect.origin.x = wallHitInfo.boundMin.x;

			if (!isSlopeHit) {
				objectMovement->resetX();
			}
		}
		if (hitY) {
			crntRect.origin.y = wallHitInfo.boundMin.y;
			prevRect.origin.y = wallHitInfo.boundMin.y;

			if (!isSlopeHit) {
				objectMovement->resetY();
			}
		}

		// 壁の当たり判定ビットを取得。
		if ((hitX || hitY) && hitTiles.size() > 0) {
			int bit = 0;
			for (auto hitTile : hitTiles) {
				bit |= hitTile.first;

				ObjectCollision::HitTileWall htw;
				htw.bit = hitTile.first;
				htw.tile = hitTile.second;
				pLinkConditionTileWall->emplace_back(htw);
			}

			if (tileWallBit) {
				*tileWallBit |= bit;
			}
		}
		// tile_end
#else
	for (int i = 1; i <= div; i++) {
// #AGTK-NX #AGTK-WIN
#ifdef USE_30FPS_2
		// 中間地点の壁判定結果を記録
		if (passCount > 1) {
			float checkDist = (wallHitInfo.boundMin - iniPrevWallPos).length();
			if (checkDist > 0 && checkDist <= maxHalfDist && closestHalfDist < checkDist) {
				closestHalfDist = checkDist;
				closestHalfWallPos = wallHitInfo.boundMin;
			}
		}
#endif
		// 分割後の過去の矩形
		pos.x = (prevRect.getMinX() == crntRect.getMinX()) ? crntRect.getMinX() : MathUtil::lerp(prevRect.getMinX(), crntRect.getMinX(), (float)(i - 1) / div);
		pos.y = (prevRect.getMinY() == crntRect.getMinY()) ? crntRect.getMinY() : MathUtil::lerp(prevRect.getMinY(), crntRect.getMinY(), (float)(i - 1) / div);

		// 分割後の現在の矩形
		newPos.x = (prevRect.getMinX() == crntRect.getMinX()) ? crntRect.getMinX() : MathUtil::lerp(prevRect.getMinX(), crntRect.getMinX(), (float)i / div);
		newPos.y = (prevRect.getMinY() == crntRect.getMinY()) ? crntRect.getMinY() : MathUtil::lerp(prevRect.getMinY(), crntRect.getMinY(), (float)i / div);

		// 移動量
		moveVec = newPos - pos;

		// 分割後の現在座標をwallHitInfoに保持
		wallHitInfo.boundMin = newPos;
		wallHitInfo.boundMax.x = wallHitInfo.boundMin.x + crntRect.size.width;
		wallHitInfo.boundMax.y = wallHitInfo.boundMin.y + crntRect.size.height;
		wallHitInfo.center.x = wallHitInfo.boundMin.x + crntRect.size.width * 0.5f;
		wallHitInfo.center.y = wallHitInfo.boundMin.y + crntRect.size.height * 0.5f;

		bool hitX = false;
		bool hitY = false;

		//object_start
#ifdef USE_MULTITHREAD_OBJECT_UPDATE
		if (hitObjectList != nullptr && hitObjectList->count() > 0 && objectTemplateMove->isIgnoredObjectWall() == false) {
			bool objectRemoved = false;
			for (int i = 0; i < (int)hitObjectList->size(); i++) {
				auto object2 = (*hitObjectList)[i];
#else
		if (hitObjectList != nullptr && hitObjectList->count() > 0 && objectTemplateMove->isIgnoredObjectWall() == false) {
			CCARRAY_FOREACH(hitObjectList, ref) {
				auto object2 = dynamic_cast<agtk::Object *>(ref);
#endif
				auto objectData2 = object2->getObjectData();

				// object2 が Disabled の場合
				if (object2->getDisabled()) {
					tmpHitObjectList->removeObject(object2);
#ifdef USE_MULTITHREAD_OBJECT_UPDATE
					objectRemoved = true;
#endif
					continue;
				}

				// レイヤーが違うオブジェクトの場合
				if (object->getLayerId() != object2->getLayerId()) {
					tmpHitObjectList->removeObject(object2);
#ifdef USE_MULTITHREAD_OBJECT_UPDATE
					objectRemoved = true;
#endif
					continue;
				}

				// 相手グループとは接触しない場合
				if (!(objectData->isCollideWith(objectData2))) {
#ifdef USE_MULTITHREAD_OBJECT_UPDATE
					objectRemoved = true;
#endif
					tmpHitObjectList->removeObject(object2);
					continue;
				}

				auto objectTemplateMove2 = object2->getObjectTemplateMove();
				// 接触したオブジェクトがテンプレート移動中で他オブジェクトの壁判定を無視する設定の場合
				if (objectTemplateMove2 != nullptr && objectTemplateMove2->isIgnoredObjectWall()) {
#ifdef USE_MULTITHREAD_OBJECT_UPDATE
					objectRemoved = true;
#endif
					tmpHitObjectList->removeObject(object2);
					continue;
				}

#ifdef USE_MULTITHREAD_OBJECT_UPDATE
				LockObjList::AutoLocker locker(&lockObjList, object2);
#endif
				// 相手グループのWallHitInfoを取得
				std::vector<agtk::Vertex4> wallCollisionList2;
				object2->getTimelineList(agtk::data::TimelineInfoData::kTimelineWall, wallCollisionList2);
#ifdef USE_MULTITHREAD_OBJECT_UPDATE
				auto wallGroup2 = agtk::WallHitInfoGroup::create(object2);
				AutoDeleter<agtk::WallHitInfoGroup> deleter(wallGroup2);
				wallGroup2->addWallHitInfo(wallCollisionList2);
#else
				auto wallGroup2 = object2->getObjectCollision()->getWallHitInfoGroup();
				wallGroup2->remove();
				wallGroup2->addWallHitInfo(wallCollisionList2);
#endif
				// 移動範囲の矩形内に相手オブジェクトの矩形がないかチェックする
				if (!chkMoveRect.intersectsRect(wallGroup2->getRect())) {
					// なければ接触チェックが必要ないのでリストから削除しておく
#ifdef USE_MULTITHREAD_OBJECT_UPDATE
					objectRemoved = true;
#endif
					tmpHitObjectList->removeObject(object2);
					continue;
				}

				// 相手オブジェクトと接触しているかをチェックする
				auto objRect = Rect(wallHitInfo.boundMin, Size(wallHitInfo.boundMax - wallHitInfo.boundMin));
				if (!objRect.intersectsRect(wallGroup2->getRect())) {
					continue;
				}

				// 接触が発生している場合
				auto objectCouseMove2 = object2->getObjectCourseMove();
				// 自身が押し戻される設定時
				if (object->getPushedbackByObject()) {
					// 相手は押し戻されない場合
					if (!object2->getPushedbackByObject()) {

						// 自身がコース移動中の場合、押し戻されるとコースから外れるので処理しない
						if (objectCourseMove && objectCourseMove->isMoving()) {
							continue;
						}

						Rect pr = prevRect;
						pushBackBuriedObjectWall(pr, crntRect, moveVec, wallGroup2);

						Vec2 mv = newPos - pr.origin;

						//移動せずに壁に食い込んだ場合。
						if (moveVec == cocos2d::Vec2::ZERO) {
							mv = cocos2d::Vec2::ZERO;
						}

						// 自身を押し戻す
						newCheckWallHit(wallHitInfo, hitX, hitY, wallGroup2, mv, moveVec);
					}
					// 相手も押し戻される場合
					else {
						// 相手がコース移動中でない場合
						if (objectCouseMove2 == nullptr || !objectCouseMove2->isMoving()) {
							// オブジェクトの押し出しを試みる
#ifdef USE_MULTITHREAD_OBJECT_UPDATE
							object2->getObjectWallCollision()->pushObject(lockObjList, wallGroup2, moveVec, crntRect, object, true);
#else
							object2->getObjectWallCollision()->pushObject(moveVec, crntRect, object);
#endif

							// 押し出す壁当たり判定を取得しなおす
							wallCollisionList2.clear();
							object2->getTimelineList(agtk::data::TimelineInfoData::kTimelineWall, wallCollisionList2);
#ifdef USE_MULTITHREAD_OBJECT_UPDATE
							wallGroup2->remove();
							wallGroup2->addWallHitInfo(wallCollisionList2);
#else
							wallGroup2 = object2->getObjectCollision()->getWallHitInfoGroup();
							wallGroup2->remove();
							wallGroup2->addWallHitInfo(wallCollisionList2);
#endif
						}

						// 自身がコース移動中の場合、押し戻されるとコースから外れるので処理しない
						if (objectCourseMove && objectCourseMove->isMoving()) {
							continue;
						}

						// 押し出したオブジェクトがタイルに接触等で押し戻しできない場合もあるので
						// 自身の押し戻し処理を行う
						Rect pr = prevRect;
						pushBackBuriedObjectWall(pr, crntRect, moveVec, wallGroup2);

						Vec2 mv = newPos - pr.origin;

						//移動せずに壁に食い込んだ場合。
						if (moveVec == cocos2d::Vec2::ZERO) {
							mv = cocos2d::Vec2::ZERO;
						}
						// 自身を押し戻す
						newCheckWallHit(wallHitInfo, hitX, hitY, wallGroup2, mv, moveVec);
					}
				}
				// 自身が押し戻されない場合
				else {
					// 相手も押し戻されない場合
					if (!object2->getPushedbackByObject()) {

						// 自身がコース移動中の場合、押し戻されるとコースから外れるので処理しない
						if (objectCourseMove && objectCourseMove->isMoving()) {
							continue;
						}

						Rect pr = prevRect;

						//坂の上に立っている場合かつX方向への移動が無い場合は、閾値以下のY方向移動量の場合は０とする。
						if (*slopeBit & agtk::data::ObjectActionLinkConditionData::kSlopeBitDown) {
							const float threshold = 0.0001f;
							if (moveVec.x == 0 && moveVec.y < 0 && abs(moveVec.y) < threshold) {
								moveVec = cocos2d::Vec2::ZERO;
							}
						}

						pushBackBuriedObjectWall(pr, crntRect, moveVec, wallGroup2);
						Vec2 mv = newPos - pr.origin;

						//移動せずに壁に食い込んだ場合。
						if (moveVec == cocos2d::Vec2::ZERO) {
							mv = cocos2d::Vec2::ZERO;
						}

						//「カメラとの位置関係を固定する」設定しているオブジェクトで、相対するオブジェクトは設定無しの場合。
						auto sceneLayer = object->getSceneLayer();
						if ((objectData->getFixedInCamera() == true && objectData2->getFixedInCamera() == false) && (sceneLayer && sceneLayer->getType() != agtk::SceneLayer::kTypeMenu)) {
							// オブジェクトの押し出しを試みる
#ifdef USE_MULTITHREAD_OBJECT_UPDATE
							object2->getObjectWallCollision()->pushObject(lockObjList, wallGroup2, mv, crntRect, object);
#else
							object2->getObjectWallCollision()->pushObject(mv, crntRect, object);
#endif
						} else {
							// 自身を押し戻す
							if (mv != cocos2d::Vec2::ZERO || moveVec != cocos2d::Vec2::ZERO) {// 移動量がゼロ以外の場合
								newCheckWallHit(wallHitInfo, hitX, hitY, wallGroup2, mv, moveVec);
							}
						}
					}
					// 相手は押し戻される場合
					else {
						Vec2 mv = moveVec;

						// 相手がコース移動中でない場合
						if (objectCouseMove2 == nullptr || !objectCouseMove2->isMoving()) {

							// オブジェクトに移動が発生しなかった場合、前フレームのオブジェクト矩形を基に押し出し方向の算出を行う
							if (mv.x == 0 && mv.y == 0) {
								auto w = object->getObjectCollision()->getOldWallHitInfoGroup()->findWallHitInfo(wallHitInfo.id);
								if (w.id >= 0) {
									mv.x = prevRect.getMidX() - w.getRect().getMidX();//Xは中心基準
									mv.y = prevRect.getMinY() - w.getRect().getMinY();//Yは足元基準
								}
								//mv.x = prevRect.getMidX() - object->getObjectCollision()->getOldWallHitInfoGroup()->getRect().getMidX();//Xは中心基準
								//mv.y = prevRect.getMinY() - object->getObjectCollision()->getOldWallHitInfoGroup()->getRect().getMinY();//Yは足元基準
							}

							// オブジェクトの押し出しを試みる
#ifdef USE_MULTITHREAD_OBJECT_UPDATE
							object2->getObjectWallCollision()->pushObject(lockObjList, wallGroup2, mv, crntRect, object);
#else
							object2->getObjectWallCollision()->pushObject(mv, crntRect, object);
#endif

							// 押し出す壁当たり判定を取得しなおす
							wallCollisionList2.clear();
							object2->getTimelineList(agtk::data::TimelineInfoData::kTimelineWall, wallCollisionList2);
#ifdef USE_MULTITHREAD_OBJECT_UPDATE
							wallGroup2->remove();
							wallGroup2->addWallHitInfo(wallCollisionList2);
#else
							wallGroup2 = object2->getObjectCollision()->getWallHitInfoGroup();
							wallGroup2->remove();
							wallGroup2->addWallHitInfo(wallCollisionList2);
#endif
						}

						// 自身がコース移動中の場合、押し戻されるとコースから外れるので処理しない
						if (objectCourseMove && objectCourseMove->isMoving()) {
							continue;
						}

						if (mv.x == 0 && mv.y == 0) {
							//移動してなければ、自身の押し戻しは不要。
							continue;
						}
						// 押し出したオブジェクトがタイルに接触等で押し戻しできない場合もあるので
						// 自身の押し戻し処理を行う
						Rect pr = prevRect;
						pushBackBuriedObjectWall(pr, crntRect, mv, wallGroup2);

						mv = newPos - pr.origin;
						//移動せずに壁に食い込んだ場合。
						if (moveVec == cocos2d::Vec2::ZERO) {
							mv = cocos2d::Vec2::ZERO;
						}

						// 自身を押し戻す
						newCheckWallHit(wallHitInfo, hitX, hitY, wallGroup2, mv, moveVec);
					}
				}

				// 接触が発生している場合
				if (hitX) {
					crntRect.origin.x = wallHitInfo.boundMin.x;
				}

				if (hitY) {
					crntRect.origin.y = wallHitInfo.boundMin.y;
				}

				if (hitX || hitY) {
					moveVec = wallHitInfo.boundMin - prevRect.origin;
				}
			}
			// 削除したオブジェクト分を反映
#ifdef USE_MULTITHREAD_OBJECT_UPDATE
			if (objectRemoved) {
#else
			if (hitObjectList->count() != tmpHitObjectList->count()) {
#endif
				hitObjectList->removeAllObjects();
				hitObjectList->addObjectsFromArray(tmpHitObjectList);
			}
		}
		//object_end

		// tile_start
		// タイルとの接触確認を行わない場合はスキップする
		if (!checkCollideTile) {
			wallHitInfo.adjustHit();
			continue;
		}

		// タイルリストを取得
		auto oldWallHitRect = object->getObjectCollision()->getOldWallHitInfoGroup()->findWallHitInfo(wallHitInfo.id).getRect();
		Vec2 tileThresholdMoveVec = oldWallHitRect.origin - crntRect.origin;
		// ジャンプ中＆落下中でない場合。
		if (object->_jumping == false && object->_falling == false && moveVec != cocos2d::Vec2::ZERO && !object->isExternalValueXy()) {
			// ACT2-2902 ジャンプ中＆落下中でない場合は、新しくタイル情報を得るようにする（※この条件以外でタイル情報を得るとジャンプして床に張り付くバグが出ます。
			// ACT2-2847 移動により物理オブジェクトの位置設定により強制的に物理タイルを押すと座標が異常値になることがあるため、強制移動しないようにここで食い止める。
			// ACT2-3575 壁判定がタイルの壁判定に埋まった後の挙動不具合（XY座標変数変更した場合はこの処理をしないようにする。
			Vec2 _moveVec = newPos - cocos2d::Vec2(prevRect.getMinX(), prevRect.getMinY());
			tileList = object->getObjectCollision()->getWallCollisionTileList(_moveVec + tileThresholdMoveVec, object->getObjectData()->getCollideWithTileGroupBit());
		}

		// 移動範囲の矩形とタイルが接触しない場合はスキップする
		if (!isHitSlopeTileList) {
			// 接触するオブジェクトもない場合はwallHitInfoを更新して判定を終了する
			if (hitObjectList->count() == 0) {
				wallHitInfo.boundMin = crntRect.origin;
				wallHitInfo.boundMax.x = wallHitInfo.boundMin.x + crntRect.size.width;
				wallHitInfo.boundMax.y = wallHitInfo.boundMin.y + crntRect.size.height;
				wallHitInfo.center.x = wallHitInfo.boundMin.x + crntRect.size.width * 0.5f;
				wallHitInfo.center.y = wallHitInfo.boundMin.y + crntRect.size.height * 0.5f;
				wallHitInfo.adjustHit();
				break;
			}
			wallHitInfo.adjustHit();
			continue;
		}

		hitX = false;
		hitY = false;
		agtk::HitWallTiles hitTiles;
		// 分割後の現在座標がタイルと接触発生しているかをチェック
// #AGTK-NX
#if CC_TARGET_PLATFORM != CC_PLATFORM_NX
		newCheckWallHit(wallHitInfo, hitX, hitY, tileThreshold, tileList, slopeList, object->getPassableSlopeTouchedList(), moveVec, tileThresholdMoveVec, oldWallHitRect, object->_falling, isSlopeHit, hitTiles);
#else
#endif

		// 接触が発生している場合
		if (hitX) {
			crntRect.origin.x = wallHitInfo.boundMin.x;
			prevRect.origin.x = wallHitInfo.boundMin.x;

			if (!isSlopeHit) {
				objectMovement->resetX();
			}
		}
		if (hitY) {
			crntRect.origin.y = wallHitInfo.boundMin.y;
			prevRect.origin.y = wallHitInfo.boundMin.y;

			if (!isSlopeHit) {
				objectMovement->resetY();
			}
		}
		// tile_end
#endif // USE_SAR_FIX_AND_OPTIMIZE_1
	}
// #AGTK-NX #AGTK-WIN
#ifndef USE_30FPS_3
#ifdef USE_30FPS_2
	// 中間地点の壁判定結果をオブジェクトに反映
	if (passCount > 1) {
		object->setPassedFrameCount(-1);
		if (closestHalfDist >= 0.0f) {
			// 中間地点を反映
			Vec2 diff = closestHalfWallPos - iniCrntWallPos;
			if (diff != Vec2::ZERO) {
				object->setPassedFrameCount(object->getFrameCount());
				object->setPassedFramePosition(object->getPosition() + Vec2(diff.x, -diff.y));
			}
		}
	}
#endif	
#endif

	// オブジェクトが正しい位置に配置されているかをチェック
	if (!objectCourseMove || !objectCourseMove->isMoving()) {
		//	CCLOG("----------");
		//	CCLOG("地面設置前 : %f", crntRect.getMinY());

		// タイル接触が許可されている場合のみ床、壁との接触確認を行う
		if (checkCollideTile) {
			Vec2 dist;
			auto object = wallHitInfo.object;
			auto objectTemplateMove = object->getObjectTemplateMove();
			bool bJumping = (object ? object->_jumping : false);
			bool bTemplateMoveVertical = (objectTemplateMove->getMoveType() == agtk::data::ObjectCommandTemplateMoveData::kMoveVertical);
			bool bMoveUp = (object->getObjectMovement()->getMoveSideBit() & agtk::data::ObjectActionLinkConditionData::kWallBitUp) ? true : false;
// #AGTK-NX #AGTK0WIN
#ifndef USE_SAR_PROVISIONAL_4
#if CC_TARGET_PLATFORM != CC_PLATFORM_NX
			if (checkGroundAndWallHit(crntRect, dist, tileList, object->getObjectCollision()->getOldWallHitInfoGroup()->getRect(), object, true, bJumping | bTemplateMoveVertical | bMoveUp)) {
#else
#endif
#else
			auto oldRect = object->getObjectCollision()->getOldWallHitInfoGroup()->getRect();
			if (object->getObjectCollision()->getOldWallHitInfoGroup()->getWallHitInfoListCount() == 0) {
				oldRect = _prevRect;
			}
			if (checkGroundAndWallHit(crntRect, dist, tileList, oldRect, object, true, bJumping | bTemplateMoveVertical | bMoveUp)) {
#endif
				crntRect.origin -= dist;
				wallHitInfo.boundMin -= dist;
				wallHitInfo.boundMax -= dist;
				wallHitInfo.center -= dist;

				// 床への設置を行ったので坂との接触は行わないようにする
				checkSlope = false;
			}
		}
		// 坂が足元に接触している場合
		else if (checkSlope && *slopeBit & agtk::data::ObjectActionLinkConditionData::kSlopeBitDown) {

			float dist;
			if (checkGroundSlope(crntRect, dist, slopeList)) {
				crntRect.origin.y -= dist;
				wallHitInfo.boundMin.y -= dist;

//				CCLOG("坂修正：%f", dist);
			}
		}

		//	CCLOG("地面設置後 : %f", wallHitInfo.boundMin.y);
	}

	// 現在の位置と過去の位置の移動量
	moveVec = object->getPosition() - object->getOldPosition2();
	moveVec.y = -moveVec.y;

	//アニメーションで壁判定移動している場合
	cocos2d::Vec2 animMoveVec;

	//アニメーションの壁判定移動量をmoveVecへ加算する。
	auto player = object->getPlayer();
	if (player != nullptr) {
		animMoveVec = player->getCenterNode()->getPosition() - player->getCenterNode()->getOldPosition();
// #AGTK-NX #AGTK-WIN
#ifdef USE_30FPS_3
		if (passCount > 1) {
			animMoveVec = animMoveVec / passCount;
		}
#endif
	}
	moveVec += animMoveVec;

	// 自オブジェクトの上に乗っているオブジェクトの移動を行う
#ifdef USE_MULTITHREAD_OBJECT_UPDATE
	//// 自オブジェクトの上に乗っているオブジェクトがない かつ 相手オブジェクトの下にオブジェクトがない場合はチェックを行わない
	if (hitObjectList != nullptr && objectTemplateMove->isIgnoredObjectWall() == false) {
		for(int i = 0; i < (int)hitObjectList->size(); i++){
			auto object2 = (*hitObjectList)[i];
#else
	if (hitObjectList != nullptr && objectTemplateMove->isIgnoredObjectWall() == false) {
		CCARRAY_FOREACH(hitObjectList, ref) {
			// 自オブジェクトの上に乗っているオブジェクトがない かつ 相手オブジェクトの下にオブジェクトがない場合はチェックを行わない
			auto object2 = dynamic_cast<agtk::Object*>(ref);
#endif
			if (!object2->getDownWallObjectList()->containsObject(object) &&
				!object->getUpWallObjectList()->containsObject(object2)) {
				continue;
			}

			// 押し戻されないオブジェクトは動かせない。
			if (!object2->getPushedbackByObject()) {
				continue;
			}

			// コース移動しているものは動かせない
			auto objectCourseMove2 = object2->getObjectCourseMove();
			if (objectCourseMove2 && objectCourseMove2->isMoving()) {
				continue;
			}

			// テンプレート移動中で他オブジェクトの壁判定を無視する設定のものは動かせない
			auto objectTemplateMove2 = object2->getObjectTemplateMove();
			if (objectTemplateMove2 != nullptr && objectTemplateMove2->isIgnoredObjectWall()) {
				continue;
			}

			// リフト移動不可能の場合はチェックをしない
			auto objectMovement2 = object2->getObjectMovement();
			bool bJumping = (object2 ? object2->_jumping : false);
			if (!objectMovement2->getCanMoveLift() && !bJumping) {
				continue;
			}

#ifdef USE_MULTITHREAD_OBJECT_UPDATE
			LockObjList::AutoLocker locker(&lockObjList, object2);
#endif
			// 動かすオブジェクトの矩形を取得
			std::vector<agtk::Vertex4> wallCollisionList2;
// #AGTK-NX #AGTK-WIN
#ifdef USE_30FPS_4
			// 中間フレーム情報対応のタイムライン壁判定取得
			object2->getWallTimelineList(wallCollisionList2);
#else
			object2->getTimelineList(agtk::data::TimelineInfoData::kTimelineWall, wallCollisionList2);
#endif

#ifdef USE_MULTITHREAD_OBJECT_UPDATE
			auto wallGroup2 = agtk::WallHitInfoGroup::create(object2);
			AutoDeleter<agtk::WallHitInfoGroup> deleter(wallGroup2);
			wallGroup2->addWallHitInfo(wallCollisionList2);
#else
			auto wallGroup2 = object2->getObjectCollision()->getWallHitInfoGroup();
			wallGroup2->remove();
			wallGroup2->addWallHitInfo(wallCollisionList2);
#endif

#if 1	// ACT2-5132 自オブジェクトの押し戻しにより、自オブジェクトの上に乗っているオブジェクトと誤判定されないよう、現在の位置関係で接触していないものをhitObjectListから除外する。
			// 押し戻し後の矩形内に相手オブジェクトの矩形がないかチェックする。左右は若干ゆるく、上下は吸い付きを許容したいため無視する判定。
			if (!intersectsRectWithLiftMargin(crntRect, wallGroup2->getRect(), 0.0001f)) {
				// なければ除外する。
				continue;
			}
#endif
			// 座標を更新
			auto crntPos = agtk::Scene::getPositionCocos2dFromScene(object2->getPosition());

			// 壁判定矩形の補正込みで接触しているオブジェクのみ移動を行う
			float val = 0.0f;
			if (moveVec.y < 0) {
				auto wallGroup = object->getObjectCollision()->getWallHitInfoGroup();
				auto wall = wallGroup->getWallHitInfo(wallHitInfo.id);
				auto rectWall = cocos2d::Rect(wall.boundMin, wall.size);
				rectWall.origin.y -= moveVec.y;
				for (unsigned int i = 0; i < wallGroup2->getWallHitInfoListCount(); i++) {
					auto wall2 = wallGroup2->getWallHitInfo(i);
					auto rectWall2 = cocos2d::Rect(wall2.boundMin, wall2.size);
					rectWall2.origin.x += TILE_COLLISION_THRESHOLD;
					rectWall2.size.width -= TILE_COLLISION_THRESHOLD * 2;
					if (rectWall2.intersectsRect(rectWall)) {
						auto v = wall2.boundMin.y - wall.boundMax.y;
						if (v > 0.0f && (val == 0.0f || val > v)) {
							val = v;
						}
					}
				}
			}

			//乗っているオブジェクトがジャンプする時は、壁判定の補正内で処理を行う。
			if ((!bJumping || (bJumping && (-TILE_COLLISION_THRESHOLD <= val && val <= TILE_COLLISION_THRESHOLD))) && moveVec != cocos2d::Vec2::ZERO) {
				auto moveVecOld = moveVec;
				//ジャンプしていなくて、アニメーションY軸が下方の場合は、valに重力量を加算する（上に乗っているオブジェクトが離れてしまうのを補正）
				if (!bJumping && (moveVec.y == animMoveVec.y &&  animMoveVec.y < 0) && objectMovement2) {
					val += -objectMovement2->getGravity().y;
				}
				moveVec.y -= val;

				WallHitInfo obj2Info;
				obj2Info.object = object2;
#ifdef USE_MULTITHREAD_OBJECT_UPDATE
				obj2Info.size = wallGroup2->getRect().size;
#else
				obj2Info.size = object2->getObjectCollision()->getWallHitInfoGroup()->getRect().size;
#endif
				obj2Info.boundMin = wallGroup2->getBoundMin() + moveVec;
				obj2Info.boundMax = wallGroup2->getBoundMax() + moveVec;
				obj2Info.center.x = obj2Info.boundMin.x + obj2Info.size.width * 0.5f;
				obj2Info.center.y = obj2Info.boundMin.y + obj2Info.size.height * 0.5f;

				// タイル壁接触を考慮して移動を行う
				bool hitX, hitY;
				agtk::HitWallTiles hitTiles;
// #AGTK-NX #AGTK-WIN
#ifndef USE_SAR_PROVISIONAL_4
#if CC_TARGET_PLATFORM != CC_PLATFORM_NX
#ifdef USE_MULTITHREAD_OBJECT_UPDATE
				agtk::MtVector<agtk::Slope *> slope;
				newCheckWallHit(obj2Info, hitX, hitY, 0, tileList, &slope, nullptr, moveVec, Vec2(0, 0), object2->getObjectCollision()->getOldWallHitInfoGroup()->getRect(), false, false, hitTiles);
#else
				newCheckWallHit(obj2Info, hitX, hitY, 0, tileList, nullptr, nullptr, moveVec, Vec2(0, 0), object2->getObjectCollision()->getOldWallHitInfoGroup()->getRect(), false, false, hitTiles);
#endif
#else
#endif
#else
				auto oldRect = object2->getObjectCollision()->getOldWallHitInfoGroup()->getRect();
				if (object2->getObjectCollision()->getOldWallHitInfoGroup()->getWallHitInfoListCount() == 0) {
					oldRect = obj2Info.getRect();
				}
				auto tileThresholdMoveVec = Vec2(0, 0);
				newCheckWallHit(obj2Info, hitX, hitY, 0, tileList, nullptr, nullptr, moveVec, tileThresholdMoveVec, oldRect, false, false, hitTiles);
#endif

				crntPos += (obj2Info.boundMin - wallGroup2->getBoundMin());

				//幅・高さが０より大きい場合。
				if ((wallHitInfo.size.width > 0 && wallHitInfo.size.height > 0) && (obj2Info.size.width > 0 && obj2Info.size.height > 0)) {
					//ACT2-2704 [player]コースに合わせてカメラスクロールを行うとキャラがブレる
					//リフト中の場合、乗っているオブジェクトとリフトオブジェクト間で座標のブレが生じる。
					//描画時に小数点切り捨てている事で起こっています。
					//ここで、描画時の小数点切り捨てを考慮して座標を調整します。
					auto objPos = object->getPosition();
					auto objPosOld = object->getOldPosition2();
					auto obj2Pos = agtk::Scene::getPositionSceneFromCocos2d(crntPos);
					auto obj2PosOld = object2->getPosition();

					int lenX = (int)objPos.x - (int)obj2Pos.x;
					int lenOldX = (int)objPosOld.x - (int)obj2PosOld.x;
					int lenY = (int)objPos.y - (int)obj2Pos.y;
					int lenOldY = (int)objPosOld.y - (int)obj2PosOld.y;

					if (objPos.x != objPosOld.x) {
						if (lenX != lenOldX) {
							if (lenX > lenOldX) {
								obj2Pos.x = (int)(obj2Pos.x + 1.0f);
							}
							else {
								obj2Pos.x = (int)(obj2Pos.x - 1.0f) + 0.99f;
							}
#ifdef USE_MULTITHREAD_OBJECT_UPDATE
							//押し合いをすると、ここのチェックに引っかかってしまうため、とりあえず他の動作確認が終わるまで回避。
#else
							CC_ASSERT(lenOldX == (int)objPos.x - (int)obj2Pos.x);
#endif
						}
					}
					if (objPos.y != objPosOld.y) {
						if (lenY != lenOldY) {
							if (lenY > lenOldY) {
								obj2Pos.y = (int)(obj2Pos.y + 1.0f);
							}
							else {
								obj2Pos.y = (int)(obj2Pos.y - 1.0f) + 0.99f;
							}
#ifdef USE_MULTITHREAD_OBJECT_UPDATE
							//押し合いをすると、ここのチェックに引っかかってしまうため、とりあえず他の動作確認が終わるまで回避。
#else
							CC_ASSERT(lenOldY == (int)objPos.y - (int)obj2Pos.y);
#endif
						}
					}
					auto pos2 = object2->getPosition();
					if (moveVecOld.y > 0.0f) {
						//自オブジェクトが上に移動している場合は、上に乗っているオブジェクトのY座標の調整をしないようにする。
						obj2Pos.y = pos2.y;
					}
					object2->setPosition(obj2Pos);
				}
			}
			//リフト移動中のオブジェクトを追加。
// #AGTK-NX #AGTK-WIN
#if defined(USE_30FPS_3) && defined(USE_30FPS_3_2)
			if(bLastPass) {
#endif
			objectMovement->getObjectMoveLiftList()->addObject(object2);
// #AGTK-NX #AGTK-WIN
#if defined(USE_30FPS_3) && defined(USE_30FPS_3_2)
			} // if (bLastPass)
#endif
		}
	}

	//オブジェクトが乗っているフラグOn/Off
	object->_objectMoveLift = (objectMovement->getObjectMoveLiftList()->count() > 0) ? true : false;

	// 埋まっていない
	object->setBuriedInWallFlag(false);
}

#ifdef USE_MULTITHREAD_OBJECT_UPDATE
static void getTouchingObjectWallHitInfo(WallHitInfo &wallHitInfo, agtk::Object *object, agtk::NrArray *objectList, agtk::MtVector<agtk::Object *> *pLeftWallObjectList, agtk::MtVector<agtk::Object *> *pRightWallObjectList, agtk::MtVector<agtk::Object *> *pUpWallObjectList, agtk::MtVector<agtk::Object *> *pDownWallObjectList);
void getTouchingObjectWallHitInfo(WallHitInfo &wallHitInfo, agtk::Object *object, agtk::NrArray *objectList, agtk::MtVector<agtk::Object *> *pLeftWallObjectList, agtk::MtVector<agtk::Object *> *pRightWallObjectList, agtk::MtVector<agtk::Object *> *pUpWallObjectList, agtk::MtVector<agtk::Object *> *pDownWallObjectList)
#else
static void getTouchingObjectWallHitInfo(WallHitInfo &wallHitInfo, agtk::Object *object, cocos2d::Array *objectList, cocos2d::Array *pLeftWallObjectList, cocos2d::Array *pRightWallObjectList, cocos2d::Array *pUpWallObjectList, cocos2d::Array *pDownWallObjectList);
void getTouchingObjectWallHitInfo(WallHitInfo &wallHitInfo, agtk::Object *object, cocos2d::Array *objectList, cocos2d::Array *pLeftWallObjectList, cocos2d::Array *pRightWallObjectList, cocos2d::Array *pUpWallObjectList, cocos2d::Array *pDownWallObjectList)
#endif
{
	cocos2d::Ref *ref = nullptr;
	//object
	auto objectData = object->getObjectData();
	if (objectList != nullptr){
#ifdef USE_MULTITHREAD_OBJECT_UPDATE
		for (auto ref : *objectList) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
			auto object2 = static_cast<agtk::Object *>(ref);
#else
			auto object2 = dynamic_cast<agtk::Object *>(ref);
#endif
#else
		CCARRAY_FOREACH(objectList, ref) {
			auto object2 = dynamic_cast<agtk::Object *>(ref);
#endif
			auto objectData2 = object2->getObjectData();

			//テンプレート移動の設定「他オブジェクトの壁判定を無視する」getTouchingObjectWallHitInfo
			if (object2->getObjectTemplateMove()->isIgnoredObjectWall()) {
				continue;
			}

			//壁判定の設定
			bool bCheckCollide = false;//判定チェック
			if (objectData->isCollideWith(objectData2)){
				bCheckCollide = true;
			}

			if (bCheckCollide == false) {
				continue;
			}

			std::vector<agtk::Vertex4> wallCollisionList2;
// #AGTK-NX #AGTK-WIN
#ifdef USE_30FPS_4
			// 中間フレーム情報対応のタイムライン壁判定取得
			object2->getWallTimelineList(wallCollisionList2);
#else
			object2->getTimelineList(agtk::data::TimelineInfoData::kTimelineWall, wallCollisionList2);
#endif
			auto pLeftWallObjectList2 = object2->getObjectWallCollision()->getLeftWallObjectList();
			auto pRightWallObjectList2 = object2->getObjectWallCollision()->getRightWallObjectList();
			auto pUpWallObjectList2 = object2->getObjectWallCollision()->getUpWallObjectList();
			auto pDownWallObjectList2 = object2->getObjectWallCollision()->getDownWallObjectList();
			for (auto wall2 : wallCollisionList2) {
				auto rect = wall2.getRect();
				auto pos = rect.origin;
				auto size = rect.size;
				float x0 = pos.x;
				float y0 = pos.y;
				float x1 = pos.x + size.width;
				float y1 = pos.y + size.height;

				if ((y0 + TILE_COLLISION_THRESHOLD) <= wallHitInfo.boundMax.y && wallHitInfo.boundMin.y <= (y1 - TILE_COLLISION_THRESHOLD * 2)) {
					if (x0 <= wallHitInfo.center.x && x1 >= wallHitInfo.boundMin.x - TILE_COLLISION_THRESHOLD) {
						if (!pLeftWallObjectList->containsObject(object2)) {
							pLeftWallObjectList->addObject(object2);
						}
						if (!pRightWallObjectList2->containsObject(object)) {
							pRightWallObjectList2->addObject(object);
						}
					}
					if (x1 >= wallHitInfo.center.x && x0 <= wallHitInfo.boundMax.x + TILE_COLLISION_THRESHOLD) {
						if (!pRightWallObjectList->containsObject(object2)) {
							pRightWallObjectList->addObject(object2);
						}
						if (!pLeftWallObjectList2->containsObject(object)) {
							pLeftWallObjectList2->addObject(object);
						}
					}
				}

				if ((x0 + TILE_COLLISION_THRESHOLD) <= wallHitInfo.boundMax.x && wallHitInfo.boundMin.x <= (x1 - TILE_COLLISION_THRESHOLD * 2)) {
					if (y0 <= wallHitInfo.center.y && y1 >= wallHitInfo.boundMin.y - TILE_COLLISION_THRESHOLD) {
						if (!pDownWallObjectList->containsObject(object2)) {
							pDownWallObjectList->addObject(object2);
						}
						if (!pUpWallObjectList2->containsObject(object)) {
							pUpWallObjectList2->addObject(object);
						}
					}
					if (y1 >= wallHitInfo.center.y && y0 <= wallHitInfo.boundMax.y + TILE_COLLISION_THRESHOLD) {
						if (!pUpWallObjectList->containsObject(object2)) {
							pUpWallObjectList->addObject(object2);
						}
						if (!pDownWallObjectList2->containsObject(object)) {
							pDownWallObjectList2->addObject(object);
						}
					}
				}
			}
		}
	}
}

//objectの壁判定とobjectListの壁判定を無条件で触れているものがあれば真を返す。
static void getTouchingObjectWallHitInfoAnyway(WallHitInfo &wallHitInfo, agtk::Object *object, cocos2d::Array *objectList, cocos2d::Array *touchedObjectList);
void getTouchingObjectWallHitInfoAnyway(WallHitInfo &wallHitInfo, agtk::Object *object, cocos2d::Array *objectList, cocos2d::Array *touchedObjectList)
{
	cocos2d::Ref *ref = nullptr;
	//object
	CCARRAY_FOREACH(objectList, ref) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
		auto object2 = static_cast<agtk::Object *>(ref);
#else
		auto object2 = dynamic_cast<agtk::Object *>(ref);
#endif

		std::vector<agtk::Vertex4> wallCollisionList2;
// #AGTK-NX #AGTK-WIN
#ifdef USE_30FPS_4
		// 中間フレーム情報対応のタイムライン壁判定取得
		object2->getWallTimelineList(wallCollisionList2);
#else
		object2->getTimelineList(agtk::data::TimelineInfoData::kTimelineWall, wallCollisionList2);
#endif
		bool touched = false;
		for (auto wall2 : wallCollisionList2) {
			//if (Vertex4::intersectsVertex4(wall2, wallHitInfo.wallVertex4)) {
				auto rect = wall2.getRect();
				auto pos = rect.origin;
				auto size = rect.size;
				float x0 = pos.x;
				float y0 = pos.y;
				float x1 = pos.x + size.width;
				float y1 = pos.y + size.height;

				if ((y0 + TILE_COLLISION_THRESHOLD) <= wallHitInfo.boundMax.y && wallHitInfo.boundMin.y <= (y1 - TILE_COLLISION_THRESHOLD * 2)) {
					if (x0 <= wallHitInfo.center.x && x1 >= wallHitInfo.boundMin.x - TILE_COLLISION_THRESHOLD) {
                        touched = true;
						break;
					}
					if (x1 >= wallHitInfo.center.x && x0 <= wallHitInfo.boundMax.x + TILE_COLLISION_THRESHOLD) {
						touched = true;
						break;
					}
				}
				if ((x0 + TILE_COLLISION_THRESHOLD) <= wallHitInfo.boundMax.x && wallHitInfo.boundMin.x <= (x1 - TILE_COLLISION_THRESHOLD * 2)) {
					if (y0 <= wallHitInfo.center.y && y1 >= wallHitInfo.boundMin.y - TILE_COLLISION_THRESHOLD) {
						touched = true;
						break;
					}
					if (y1 >= wallHitInfo.center.y && y0 <= wallHitInfo.boundMax.y + TILE_COLLISION_THRESHOLD) {
						touched = true;
						break;
					}
				}
			//}
		}
		if (touched) {
			if (!touchedObjectList->containsObject(object2)) {
				touchedObjectList->addObject(object2);
			}
		}
	}
}

#ifdef USE_MULTITHREAD_OBJECT_UPDATE
std::vector<agtk::Tile *> getFilteredListByTileGroup(std::vector<agtk::Tile *> &list, int collideWithTileGroupBit)
{
	std::vector<agtk::Tile *> newList;
	for (auto tile : list) {
		if ((tile->getGroupBit() & collideWithTileGroupBit)) {
			newList.push_back(tile);
}
	}
	return newList;
}
#else
cocos2d::__Array *getFilteredListByTileGroup(cocos2d::__Array *list, collideWithTileGroupBit)
{
	auto newList = cocos2d::__Array::create();
	CCARRAY_FOREACH(list, ref) {
		auto tile = dynamic_cast<agtk::Tile *>(ref);
		if ((tile->getGroupBit() & collideWithTileGroupBit)) {
			newList->addObject(tile);
		}
	}
	return newList;
}
tiles = nullptr;
#endif
// 衝突判定を行うタイルのリストを返す
// @todo 対象のグループを絞ることで高速化できる。
#ifdef USE_MULTITHREAD_OBJECT_UPDATE
std::vector<agtk::Tile *>  getCollisionTileList(int type, int layerId, cocos2d::Point boundMin, cocos2d::Point boundMax, cocos2d::Vec2 moveVec, agtk::Object* obj, int collideWithTileGroupBit)
#else
cocos2d::__Array* getCollisionTileList(int type, int layerId, cocos2d::Point boundMin, cocos2d::Point boundMax, cocos2d::Vec2 moveVec, agtk::Object* obj, int collideWithTileGroupBit)
#endif
{
#ifdef USE_MULTITHREAD_OBJECT_UPDATE
	std::vector<agtk::Tile *> tiles;
#else
	cocos2d::__Array *tiles = nullptr;
#endif

	cocos2d::Rect crntRect = Rect(boundMin, cocos2d::Size(boundMax - boundMin));
	cocos2d::Rect prevRect = Rect(boundMin - moveVec, crntRect.size);
	cocos2d::Rect rect = crntRect;
	rect.merge(prevRect);

	// 衝突するタイルリストを取得
	auto scene = GameManager::getInstance()->getCurrentScene();
#ifdef USE_MULTITHREAD_OBJECT_UPDATE
	auto getObjLimitTilesFunc = [&](std::vector<agtk::Tile *> &tiles,agtk::SceneLayer* layer) {
#else
	auto getObjLimitTilesFunc = [&](cocos2d::__Array* tiles,agtk::SceneLayer* layer) {
#endif
		// オブジェクト毎の行動範囲制限
		if (obj->getLimitTileSetList()) {
			auto lts = obj->getLimitTileSetList()->getEnableLimitTileSet();
			if (lts){
				lts->UpdatePosByCamera(scene->getCamera());
				auto limitTileList = lts->getLimitTileList().get();
				cocos2d::Ref* ref = nullptr;
				CCARRAY_FOREACH(limitTileList, ref) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
					LimitTile* lt = static_cast<LimitTile*>(ref);
#else
					LimitTile* lt = dynamic_cast<LimitTile*>(ref);
#endif
					if (lt->convertToWorldSpaceRect().intersectsRect(rect)) {
#ifdef USE_MULTITHREAD_OBJECT_UPDATE
						tiles.push_back(lt);
#else
						tiles->addObject(lt);
#endif
					}
				}
			}
		}
	};
	if (layerId > 0) {
		agtk::SceneLayer *sceneLayer = nullptr;
		if (type == agtk::SceneLayer::kTypeScene) {
			sceneLayer = scene->getSceneLayer(layerId);
		}
		else if (type == agtk::SceneLayer::kTypeMenu) {
			sceneLayer = scene->getMenuLayer(layerId);
		}
		if (sceneLayer) {
// #AGTK-NX
#if (CC_TARGET_PLATFORM == CC_PLATFORM_NX)
#else
			tiles = (collideWithTileGroupBit >= 0)
				? getFilteredListByTileGroup(sceneLayer->getCollisionTileList(boundMin, boundMax, moveVec), collideWithTileGroupBit)
				: sceneLayer->getCollisionTileList(boundMin, boundMax, moveVec);
#endif
			

			getObjLimitTilesFunc(tiles,sceneLayer);
		}
	}
	else {
#ifdef USE_MULTITHREAD_OBJECT_UPDATE
#else
		tiles = cocos2d::__Array::create();
#endif
		//全てのレイヤーのtail情報を取得
		cocos2d::__Dictionary *sceneLayerList;
		if (type == agtk::SceneLayer::kTypeScene) {
			sceneLayerList = scene->getSceneLayerList();
		}
		else if (type == agtk::SceneLayer::kTypeMenu) {
			sceneLayerList = scene->getMenuLayerList();
		}
		if (sceneLayerList) {
			cocos2d::DictElement *el = nullptr;
			CCDICT_FOREACH(sceneLayerList, el) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
				auto sceneLayer = static_cast<agtk::SceneLayer *>(el->getObject());
#else
				auto sceneLayer = dynamic_cast<agtk::SceneLayer *>(el->getObject());
#endif
				if (sceneLayer->getIsVisible() == false) continue;//非表示の場合はタイル情報を得ないように。
// #AGTK-NX
#if (CC_TARGET_PLATFORM == CC_PLATFORM_NX)
#else
				auto tileList = (collideWithTileGroupBit >= 0)
					? getFilteredListByTileGroup(sceneLayer->getCollisionTileOverlapList(boundMin, boundMax), collideWithTileGroupBit)
					: sceneLayer->getCollisionTileOverlapList(boundMin, boundMax);
#endif
#ifdef USE_MULTITHREAD_OBJECT_UPDATE
				for (auto tile : tileList) {
					tiles.push_back(tile);
				}
#else
				tiles->addObjectsFromArray(tileList);
#endif

				getObjLimitTilesFunc(tiles,sceneLayer);
			}
		}
	}

	//ソートオブジェクトに近いタイル順に並べる。
	std::sort(tiles.begin(), tiles.end(), [&](agtk::Tile *a, agtk::Tile *b) {
		auto x = rect.getMidX();
		auto y = rect.getMidY();
		auto rectA = a->convertToLayerSpaceRect();
		auto rectB = b->convertToLayerSpaceRect();

		float lenA = cocos2d::Vec2(x, y).getDistance(cocos2d::Vec2(rectA.getMidX(), rectA.getMidY()));
		float lenB = cocos2d::Vec2(x, y).getDistance(cocos2d::Vec2(rectB.getMidX(), rectB.getMidY()));
		return lenA < lenB;
	});

	return tiles;
}

/**
* 衝突判定を行うタイルのリストを返す(重なり効果用)
*/
#ifdef USE_MULTITHREAD_OBJECT_UPDATE
std::vector<agtk::Tile *>  getCollisionTileOverlapMaskList(int type, cocos2d::Point boundMin, cocos2d::Point boundMax)
#else
cocos2d::__Array* getCollisionTileOverlapMaskList(int type, cocos2d::Point boundMin, cocos2d::Point boundMax)
#endif
{
#ifdef USE_MULTITHREAD_OBJECT_UPDATE
	std::vector<agtk::Tile *> tiles;
#else
	cocos2d::__Array *tiles = nullptr;
#endif

	// 衝突するタイルリストを取得
	auto scene = GameManager::getInstance()->getCurrentScene();

#ifdef USE_MULTITHREAD_OBJECT_UPDATE
#else
	tiles = cocos2d::__Array::create();
#endif
	//全てのレイヤーのtail情報を取得
	cocos2d::__Dictionary *sceneLayerList;
	if (type == agtk::SceneLayer::kTypeScene) {
		sceneLayerList = scene->getSceneLayerList();
	}
	else if (type == agtk::SceneLayer::kTypeMenu) {
		sceneLayerList = scene->getMenuLayerList();
	}
	if (sceneLayerList) {
		cocos2d::DictElement *el = nullptr;
		CCDICT_FOREACH(sceneLayerList, el) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
			auto sceneLayer = static_cast<agtk::SceneLayer *>(el->getObject());
#else
			auto sceneLayer = dynamic_cast<agtk::SceneLayer *>(el->getObject());
#endif
			auto tileList = sceneLayer->getCollisionTileOverlapMaskList(boundMin, boundMax);
#ifdef USE_MULTITHREAD_OBJECT_UPDATE
			for (auto tile : tileList) {
				tiles.push_back(tile);
			}
#else
			tiles->addObjectsFromArray(tileList);
#endif
		}
	}

	return tiles;
}

// 衝突判定を行うタイルのリストを返す（回転対応）
#ifdef USE_MULTITHREAD_OBJECT_UPDATE
std::vector<agtk::Tile *> getCollisionTileList2(int type, int layerId, agtk::Vertex4 &v, agtk::Object* obj = nullptr, int collideWithTileGroupBit = -1);
std::vector<agtk::Tile *> getCollisionTileList2(int type, int layerId, agtk::Vertex4 &v, agtk::Object* obj, int collideWithTileGroupBit)
#else
cocos2d::__Array* getCollisionTileList2(int type, int layerId, agtk::Vertex4 &v, agtk::Object* obj = nullptr, int collideWithTileGroupBit = -1);
cocos2d::__Array* getCollisionTileList2(int type, int layerId, agtk::Vertex4 &v, agtk::Object* obj, int collideWithTileGroupBit)
#endif
{
#ifdef USE_MULTITHREAD_OBJECT_UPDATE
	std::vector<agtk::Tile *> tiles;
#else
	cocos2d::__Array *tiles = nullptr;
#endif
	cocos2d::Rect rect = v.getRect();

	// 衝突するタイルリストを取得
	auto scene = GameManager::getInstance()->getCurrentScene();
#ifdef USE_MULTITHREAD_OBJECT_UPDATE
	auto getObjLimitTilesFunc = [&](std::vector<agtk::Tile *> &tiles, agtk::SceneLayer* layer) {
#else
	auto getObjLimitTilesFunc = [&](cocos2d::__Array* tiles, agtk::SceneLayer* layer) {
#endif
		// オブジェクト毎の行動範囲制限
		if (obj->getLimitTileSetList()) {
			auto lts = obj->getLimitTileSetList()->getEnableLimitTileSet();
			if (lts) {
				lts->UpdatePosByCamera(scene->getCamera());
				auto limitTileList = lts->getLimitTileList().get();
				cocos2d::Ref* ref = nullptr;
				CCARRAY_FOREACH(limitTileList, ref) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
					LimitTile* lt = static_cast<LimitTile*>(ref);
#else
					LimitTile* lt = dynamic_cast<LimitTile*>(ref);
#endif
					if (lt->convertToWorldSpaceRect().intersectsRect(rect)) {
#ifdef USE_MULTITHREAD_OBJECT_UPDATE
						tiles.push_back(lt);
#else
						tiles->addObject(lt);
#endif
					}
				}
			}
		}
	};
	if (layerId > 0) {
		agtk::SceneLayer *sceneLayer = nullptr;
		if (type == agtk::SceneLayer::kTypeScene) {
			sceneLayer = scene->getSceneLayer(layerId);
		}
		else if (type == agtk::SceneLayer::kTypeMenu) {
			sceneLayer = scene->getMenuLayer(layerId);
		}
		if (sceneLayer) {
// #AGTK-NX
#if (CC_TARGET_PLATFORM == CC_PLATFORM_NX)
#else
			tiles = (collideWithTileGroupBit >= 0)
				? getFilteredListByTileGroup(sceneLayer->getCollisionTileList(v), collideWithTileGroupBit)
				: sceneLayer->getCollisionTileList(v);
#endif

			getObjLimitTilesFunc(tiles, sceneLayer);
		}
	}
	else {
#ifdef USE_MULTITHREAD_OBJECT_UPDATE
#else
		tiles = cocos2d::__Array::create();
#endif
		//全てのレイヤーのtail情報を取得
		cocos2d::__Dictionary *sceneLayerList;
		if (type == agtk::SceneLayer::kTypeScene) {
			sceneLayerList = scene->getSceneLayerList();
		}
		else if (type == agtk::SceneLayer::kTypeMenu) {
			sceneLayerList = scene->getMenuLayerList();
		}
		if (sceneLayerList) {
			cocos2d::DictElement *el = nullptr;
			CCDICT_FOREACH(sceneLayerList, el) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
				auto sceneLayer = static_cast<agtk::SceneLayer *>(el->getObject());
#else
				auto sceneLayer = dynamic_cast<agtk::SceneLayer *>(el->getObject());
#endif
// #AGTK-NX
#if (CC_TARGET_PLATFORM == CC_PLATFORM_NX)
#else
				auto tileList = (collideWithTileGroupBit >= 0)
					? getFilteredListByTileGroup(sceneLayer->getCollisionTile2List(v), collideWithTileGroupBit)
					: sceneLayer->getCollisionTile2List(v);
#endif
#ifdef USE_MULTITHREAD_OBJECT_UPDATE
				for (auto tile : tileList) {
					tiles.push_back(tile);
				}
#else
				tiles->addObjectsFromArray(tileList);
#endif

				getObjLimitTilesFunc(tiles, sceneLayer);
			}
		}
	}

	return tiles;
}

#ifdef USE_MULTITHREAD_OBJECT_UPDATE
agtk::MtVector<agtk::Slope *> *getCollisionSlopeList(agtk::SceneLayer::EnumType type, int layerId, cocos2d::Point boundMin, cocos2d::Point boundMax, cocos2d::Vec2 moveVec);
agtk::MtVector<agtk::Slope *> *getCollisionSlopeList(agtk::SceneLayer::EnumType type, int layerId, cocos2d::Point boundMin, cocos2d::Point boundMax, cocos2d::Vec2 moveVec)
#else
static cocos2d::__Array *getCollisionSlopeList(agtk::SceneLayer::EnumType type, int layerId, cocos2d::Point boundMin, cocos2d::Point boundMax, cocos2d::Vec2 moveVec);
cocos2d::__Array *getCollisionSlopeList(agtk::SceneLayer::EnumType type, int layerId, cocos2d::Point boundMin, cocos2d::Point boundMax, cocos2d::Vec2 moveVec)
#endif
{
#ifdef USE_MULTITHREAD_OBJECT_UPDATE
	agtk::MtVector<agtk::Slope *> *slopes = nullptr;
#else
	cocos2d::__Array *slopes = nullptr;
#endif
	// 衝突する坂リストを取得
	if (layerId > 0) {
		auto scene = GameManager::getInstance()->getCurrentScene();
		agtk::SceneLayer *sceneLayer = nullptr;
		if (type == agtk::SceneLayer::kTypeScene) {
			sceneLayer = scene->getSceneLayer(layerId);
		}
		else if (type == agtk::SceneLayer::kTypeMenu) {
			sceneLayer = scene->getMenuLayer(layerId);
		}
		slopes = sceneLayer->getCollisionSlopeList(boundMin, boundMax, moveVec);
	}

	return slopes;
}

static cocos2d::__Array *getCollisionLoopCourseList(agtk::SceneLayer::EnumType type, int layerId, cocos2d::Point boundMin, cocos2d::Point boundMax);
cocos2d::__Array *getCollisionLoopCourseList(agtk::SceneLayer::EnumType type, int layerId, cocos2d::Point boundMin, cocos2d::Point boundMax)
{
	cocos2d::__Array *courses = nullptr;

	// 衝突する360度ループリストを取得取得
	if (layerId > 0) {
		auto scene = GameManager::getInstance()->getCurrentScene();
		agtk::SceneLayer *sceneLayer = nullptr;
		if (type == agtk::SceneLayer::kTypeScene) {
			sceneLayer = scene->getSceneLayer(layerId);
		}
		else if (type == agtk::SceneLayer::kTypeMenu) {
			sceneLayer = scene->getMenuLayer(layerId);
		}
		courses = sceneLayer->getCollisionLoopCourseList(boundMin, boundMax);
	}

	return courses;
}

#ifdef USE_MULTITHREAD_OBJECT_UPDATE
static void updateObjectWallEffect(agtk::Object *object, std::vector<agtk::Tile *> &tileList, agtk::data::TileData::EnumConditionType conditionType, cocos2d::Point *tilePos = nullptr);
void updateObjectWallEffect(agtk::Object *object, std::vector<agtk::Tile *> &tileList, agtk::data::TileData::EnumConditionType conditionType, cocos2d::Point *tilePos)
#else
static void updateObjectWallEffect(agtk::Object *object, cocos2d::__Array *tileList, agtk::data::TileData::EnumConditionType conditionType, cocos2d::Point *tilePos = nullptr);
void updateObjectWallEffect(agtk::Object *object, cocos2d::__Array *tileList, agtk::data::TileData::EnumConditionType conditionType, cocos2d::Point *tilePos)
#endif
{
	auto projectData = GameManager::getInstance()->getProjectData();
#ifdef USE_MULTITHREAD_OBJECT_UPDATE
#else
	auto tileDataList = cocos2d::__Array::create();
#endif
	auto objectMovement = object->getObjectMovement();
#ifdef USE_MULTITHREAD_OBJECT_UPDATE
#else
	cocos2d::Ref *ref = nullptr;
#endif

#ifdef USE_MULTITHREAD_OBJECT_UPDATE
	for (auto tile : tileList) {
#else
	CCARRAY_FOREACH(tileList, ref) {
		auto tile = dynamic_cast<agtk::Tile *>(ref);
#endif
		auto tileData = tile->getTileData();
		if (tileData == nullptr) {
			continue;
		}
		auto tilesetData = projectData->getTilesetData(tile->getTilesetId());
		auto group = object->getObjectData()->getGroupBit();

		//ギミック設定タイルへのタッチ
		if (conditionType == agtk::data::TileData::kConditionTouched && tilesetData->getTilesetType() == agtk::data::TilesetData::kGimmick) {
			if (tileData->getGimmickTargetObjectGroupBit() & group){
				tile->setTouchTrigger(true);
				if (!object->_bTouchGimmickCounted) {
					tile->setTouchCount(tile->getTouchCount() + 1);
					object->_bTouchGimmickCounted = true;
				}
			}
		}

		//オブジェクトチェック。
		if (!(tileData->getTargetObjectGroupBit() & group )) {
			continue;
		}

		if (tileData->getConditionType() != conditionType) {
			continue;
		}
#ifdef USE_MULTITHREAD_OBJECT_UPDATE
#else
		if (tileDataList->containsObject(tileData)) {
			continue;
		}
#endif
		if (tileData->getMoveSpeedChanged()) {//移動速度
			objectMovement->getWallMoveSpeed()->set(tileData->getMoveSpeedChange());
		}
		if (tileData->getJumpChanged()) {//ジャンプ力
			objectMovement->getWallJump()->set(tileData->getJumpChange());
		}
		if (tileData->getMoveXFlag()) {//オブジェクトをX方向に移動（±）
			objectMovement->setWallMoveX(tileData->getMoveX());
		}
		if (tileData->getMoveYFlag()) {//オブジェクトをY方向に移動（±）
			objectMovement->setWallMoveY(tileData->getMoveY());
		}
		if (tileData->getSlipChanged()) {//移動が滑る
			// 加速移動設定時は50～100の間でないと減速に補正がかからない処理になっているので
			// getWallSlip()の初期値を考慮して設定する値を変化させる
			float initVal = objectMovement->getWallSlip()->getInitialValue();
			float rate = (100.0f - initVal) * 0.01f;
			objectMovement->getWallSlip()->set(initVal + (tileData->getSlipChange() * rate));
		}
		if (tileData->getGravityEffectChanged()) {//重力効果
			objectMovement->getWallGravityEffect()->set(tileData->getGravityEffectChange());
		}
		// 物理演算設定がある　かつ　物理演算環境の影響を受ける場合
		if (object->getObjectData()->getPhysicsSettingFlag() && object->getPlayObjectData()->getSwitchData(agtk::data::kObjectSystemSwitchAffectedByOtherObjects)->getValue()) {
			// 物理用摩擦係数
			objectMovement->getWallFriction()->set(tileData->getPhysicsFriction());
		}

		if (tileData->getGetDead()) {//死亡する
			object->setLateRemove(true);
			break;
		}
		//
		if (tileData->getHpChanged()) {//体力の増減
			bool bHpChange = false;
			auto objectData = object->getObjectData();
#ifdef USE_PREVIEW
			bool bInvincibleMode = false;
			if (objectData->getGroup() == agtk::data::ObjectData::kObjGroupPlayer) {
				//無敵モード
				//プレイヤーがダメージを受けない状態で進める事ができる。
				auto debugManager = DebugManager::getInstance();
				if (debugManager->getInvincibleModeEnabled()) {
					bInvincibleMode = true;
				}
			}
			if (!bInvincibleMode) {
				//トリガーチェック。
				bHpChange = object->isHpChangeTrigger(tile, tileData, nullptr, nullptr);
			}
#else
			bHpChange = object->isHpChangeTrigger(tile, tileData, nullptr, nullptr);
#endif
			if (bHpChange) {
				auto playData = object->getPlayObjectData();
				auto hp = playData->getHp();
				if (hp + tileData->getHpChange() > 0) {
					hp += tileData->getHpChange();

					if (hp > objectData->getMaxHp()) {
						hp = objectData->getMaxHp();
					}
				}
				else {
					hp = 0;
				}
				playData->setHp(hp);
				playData->adjustData();
			}
		}
// #AGTK-NX #AGTK-WIN
#ifdef USE_30FPS_3
		if(GameManager::getInstance()->getLastPass()) {
#endif
		if (tileData->getAreaAttributeFlag()) {//オブジェクトのエリア判定変数に数字を代入
			if (tilePos != nullptr && (tilePos->x != tile->getTileX() || tilePos->y != tile->getTileY())){
				//重なっていないタイル。
			}
			else {
				auto playData = object->getPlayObjectData();
				playData->setSystemVariableData(agtk::data::kObjectSystemVariableAreaAttribute, tileData->getAreaAttribute());
			}
		}
// #AGTK-NX #AGTK-WIN
#ifdef USE_30FPS_3
		} // if (GameManager::getInstance()->getLastPass())
#endif
	}
}

/**
* 重なり時のマスク効果の更新
*/
#ifdef USE_MULTITHREAD_OBJECT_UPDATE
void updateObjectOverlapMaskEffect(agtk::Object *object, std::vector<agtk::Tile *> &tileList, agtk::data::TileData::EnumConditionType conditionType)
#else
void updateObjectOverlapMaskEffect(agtk::Object *object, cocos2d::__Array *tileList, agtk::data::TileData::EnumConditionType conditionType)
#endif
{
#ifdef USE_MULTITHREAD_OBJECT_UPDATE
#else
	cocos2d::Ref *ref = nullptr;
#endif

#ifdef USE_MULTITHREAD_OBJECT_UPDATE
	for (auto tile : tileList) {
#else
	CCARRAY_FOREACH(tileList, ref) {
		auto tile = dynamic_cast<agtk::Tile *>(ref);
#endif
		if (conditionType == agtk::data::TileData::kConditionOverlapped && object->getLayerId() > tile->getLayerId()) {//タイルに重なった（オブジェクトより手前のレイヤーのみ）
			object->setOverlapFlag(true, Object::OVERLAP_EFFECT_TARGET_TILE);
			break;
		}
	}
}

static void resetObjectWallEffect(agtk::Object *object);
void resetObjectWallEffect(agtk::Object *object)
{
	auto objectMovement = object->getObjectMovement();
	objectMovement->getWallMoveSpeed()->reset();
	objectMovement->setWallMoveX(0.0f);
	objectMovement->setWallMoveY(0.0f);
	objectMovement->getWallJump()->reset();
	objectMovement->getWallSlip()->reset();
	objectMovement->getWallGravityEffect()->reset();
	objectMovement->getWallFriction()->reset();
}

#ifdef USE_MULTITHREAD_OBJECT_UPDATE
static void updateObjectSlopeEffect(agtk::Object *object, agtk::MtVector<agtk::Slope *> *slopeList);
void updateObjectSlopeEffect(agtk::Object *object, agtk::MtVector<agtk::Slope *> *slopeList)
#else
static void updateObjectSlopeEffect(agtk::Object *object, cocos2d::__Array *slopeList);
void updateObjectSlopeEffect(agtk::Object *object, cocos2d::__Array *slopeList)
#endif
{
	auto objectMovement = object->getObjectMovement();
	cocos2d::Ref *ref = nullptr;

#ifdef USE_MULTITHREAD_OBJECT_UPDATE
	for(int i = 0; i < (int)slopeList->count(); i++){
		auto slope = (*slopeList)[i];
#else
	CCARRAY_FOREACH(slopeList, ref) {

		auto slope = dynamic_cast<agtk::Slope*>(ref);
#endif
		auto slopeData = slope->getSlopeData();

		if (slopeData == nullptr) {
			continue;
		}

		// オブジェクトの種類をチェック
		bool checkType = false;
		if (slopeData->getObjectGroupBit() & object->getObjectData()->getGroupBit() ){
			checkType = true;
		}

		if (!checkType) {
			continue;
		}

		// オブジェクトの移動速度を増減
		if (slopeData->getMoveSpeedChanged()) {
			objectMovement->getWallMoveSpeed()->set(slopeData->getMoveSpeedChange());
		}

		// オブジェクトのジャンプ力を増減
		if (slopeData->getJumpChanged()) {
			objectMovement->getWallJump()->set(slopeData->getJumpChange());
		}

		// オブジェクトをX方向に移動
		if (slopeData->getMoveXFlag()) {
			objectMovement->setWallMoveX(slopeData->getMoveX());
		}

		// オブジェクトをY方向に移動
		if (slopeData->getMoveYFlag()) {
			objectMovement->setWallMoveY(slopeData->getMoveY());
		}

		// オブジェクトの移動が滑るようになる
		if (slopeData->getSlipChanged()) {
			// 加速移動設定時は50～100の間でないと減速に補正がかからない処理になっているので
			// getWallSlip()の初期値を考慮して設定する値を変化させる
			float initVal = objectMovement->getWallSlip()->getInitialValue();
			float rate = (100.0f - initVal) * 0.01f;
			objectMovement->getWallSlip()->set(initVal + (slopeData->getSlipChange() * rate));
		}

		// 重力効果を増減
		if (slopeData->getGravityEffectChanged()) {
			objectMovement->getWallGravityEffect()->set(slopeData->getGravityEffectChange());
		}

		// 物理演算設定がある　かつ　物理演算環境の影響を受ける場合
		if (object->getObjectData()->getPhysicsSettingFlag() && object->getPlayObjectData()->getSwitchData(agtk::data::kObjectSystemSwitchAffectedByOtherObjects)->getValue()) {
			objectMovement->getWallFriction()->set(slopeData->getPhysicsFriction());
		}

		// オブジェクトが消滅する
		if (slopeData->getGetDead()) {
			object->setLateRemove(true);
			break;
		}

		// オブジェクトの体力を増減
		if (slopeData->getHpChanged()) {
			bool bHpChange = false;
			auto objectData = object->getObjectData();
#ifdef USE_PREVIEW
			bool bInvincibleMode = false;
			if (objectData->getGroup() == agtk::data::ObjectData::kObjGroupPlayer) {
				//無敵モード
				//プレイヤーがダメージを受けない状態で進める事ができる。
				auto debugManager = DebugManager::getInstance();
				if (debugManager->getInvincibleModeEnabled()) {
					bInvincibleMode = true;
				}
			}
			if (!bInvincibleMode) {
				//トリガーチェック。
				bHpChange = object->isHpChangeTrigger(nullptr, nullptr, slope, slopeData);
			}
#else
			//トリガーチェック。
			bHpChange = object->isHpChangeTrigger(nullptr, nullptr, slope, slopeData);
#endif
			if (bHpChange) {
				auto playData = object->getPlayObjectData();
				auto hp = playData->getHp();
				if (hp + slopeData->getHpChange() > 0) {
					hp += slopeData->getHpChange();

					if (hp > objectData->getMaxHp()) {
						hp = objectData->getMaxHp();
					}
				}
				else {
					hp = 0;
				}
				playData->setHp(hp);
				playData->adjustData();
			}
		}
	}
}

#ifdef USE_MULTITHREAD_OBJECT_UPDATE
cocos2d::Vec2 ObjectCollision::updateWallHitInfo(float tryX, float tryY, WallHitInfo &wallHitInfo, int *pTileWallBit, int *pAheadTileWallBit, cocos2d::Vec2 move, std::vector<agtk::Tile *> tileList)
#else
cocos2d::Vec2 ObjectCollision::updateWallHitInfo(float tryX, float tryY, WallHitInfo &wallHitInfo, int *pTileWallBit, int *pAheadTileWallBit, cocos2d::Vec2 move, cocos2d::__Array *tileList)
#endif
{
	Vec2 moveVec = Vec2(move.x, -move.y); // シーン上での移動ベクトルなのでy座標を反転
	float moveX = 0;
	float moveY = 0;
	auto object = _object;

	CC_ASSERT(_object->getObjectWallCollision() == this);	//仮説
	calcWallHitInfo(object, wallHitInfo, move, this->getObjectList(), tileList, false, nullptr, cocos2d::Rect::ZERO);

	auto scene = GameManager::getInstance()->getCurrentScene();
	auto sceneLayer = object->getSceneLayer();
	bool hitLeft = wallHitInfo.hitLeft >= wallHitInfo.boundMin.x;
	bool hitRight = wallHitInfo.hitRight <= wallHitInfo.boundMax.x;
	bool hitDown = wallHitInfo.hitDown >= wallHitInfo.boundMin.y;
	bool hitUp = wallHitInfo.hitUp <= wallHitInfo.boundMax.y;

	if (hitLeft || hitRight || hitDown || hitUp) {
		//壁にぶつかった。
		auto oldPosition = object->getOldWallPosition();
		auto moveVec = object->getPosition() - oldPosition;

		bool moveLeft = (tryX < oldPosition.x);
		bool moveRight = (tryX > oldPosition.x);
		bool moveDown = (tryY < oldPosition.y);
		bool moveUp = (tryY > oldPosition.y);
		float horzMove = tryX - oldPosition.x;
		float vertMove = tryY - oldPosition.y;

		auto projectData = GameManager::getInstance()->getProjectData();
		WallHitInfo wallHitInfo2 = wallHitInfo;
		enum {
			kHintUp,
			kHintDown,
			kHintLeft,
			kHintRight,
			kHintMax
		};
		int horzHint = -1;
		int vertHint = -1;
		if (!hitLeft
			|| !hitRight
			|| !hitDown
			|| !hitUp) {
			//いずれかの方向に空きがある。
			//ぶつかっていない方向に壁判定を広げて再計算。
			do {
				break;
			} while (0);
		}
		else if (wallHitInfo.hitLeft == wallHitInfo.center.x
			&& wallHitInfo.hitRight == wallHitInfo.center.x
			&& wallHitInfo.hitDown == wallHitInfo.center.y
			&& wallHitInfo.hitUp == wallHitInfo.center.y) {
			//Bounding box的には囲まれている。
			//空きがありそうな方向に動くことを検討すべき？
		}
		if (horzHint < 0 && vertHint < 0) {
			do {
				if (wallHitInfo.openLeftMax > wallHitInfo.boundMax.y && wallHitInfo.openRightMax > wallHitInfo.boundMax.y
					&& wallHitInfo.openLeftMin >= wallHitInfo.boundMin.y && wallHitInfo.openRightMin >= wallHitInfo.boundMin.y
					&& wallHitInfo.openLeftMin == wallHitInfo.openRightMin) {
					//上に広げる。
					wallHitInfo2.boundMax.y += (wallHitInfo.openRightMin - wallHitInfo.boundMin.y + 1);
					vertHint = kHintUp;
					break;
				}
				if (wallHitInfo.openLeftMin < wallHitInfo.boundMin.y && wallHitInfo.openRightMin < wallHitInfo.boundMin.y
					&& wallHitInfo.openLeftMax <= wallHitInfo.boundMax.y && wallHitInfo.openRightMax <= wallHitInfo.boundMax.y
					&& wallHitInfo.openLeftMax == wallHitInfo.openRightMax) {
					//下に広げる。
					wallHitInfo2.boundMin.y -= (wallHitInfo.boundMax.y - wallHitInfo.openRightMax + 1);
					vertHint = kHintDown;
					break;
				}
				if (wallHitInfo.openDownMax > wallHitInfo.boundMax.x && wallHitInfo.openUpMax > wallHitInfo.boundMax.x
					&& wallHitInfo.openDownMin >= wallHitInfo.boundMin.x && wallHitInfo.openUpMin >= wallHitInfo.boundMin.x
					&& wallHitInfo.openDownMin == wallHitInfo.openUpMin) {
					//右に広げる。
					wallHitInfo2.boundMax.x += (wallHitInfo.openUpMin - wallHitInfo.boundMin.x + 1);
					horzHint = kHintRight;
					break;
				}
				if (wallHitInfo.openDownMin < wallHitInfo.boundMin.x && wallHitInfo.openUpMin < wallHitInfo.boundMin.x
					&& wallHitInfo.openDownMax <= wallHitInfo.boundMax.x && wallHitInfo.openUpMax <= wallHitInfo.boundMax.x
					&& wallHitInfo.openDownMax == wallHitInfo.openUpMax) {
					//左に広げる。
					wallHitInfo2.boundMin.x -= (wallHitInfo.boundMax.x - wallHitInfo.openUpMax + 1);
					horzHint = kHintLeft;
					break;
				}
				//移動した方向を加味して戻す方向を決める。
				if (!hitUp && hitLeft && !hitRight && hitDown) {
					//上か右か右上か、広げる方向を調整。
					if (moveLeft && moveDown) {
						//if (-horzMove >= -vertMove)
						if (wallHitInfo.openDownMin - wallHitInfo.boundMin.x <= wallHitInfo.openLeftMin - wallHitInfo.boundMin.y)
						{
							//右に広げる。
							horzHint = kHintRight;
						}
						else {
							//上に広げる。
							vertHint = kHintUp;
						}
					}
					else if (moveRight && moveDown) {
						//上に広げる。
						vertHint = kHintUp;
					}
					else if (moveLeft && moveUp) {
						//右に広げる。
						horzHint = kHintRight;
					}
					else if (moveRight && moveUp) {
						//右上に広げる。
						horzHint = kHintRight;
						vertHint = kHintUp;
					}
					else if (moveLeft || moveRight) {
						//右に広げる。
						horzHint = kHintRight;
					}
					else if (moveDown || moveUp) {
						//上に広げる。
						vertHint = kHintUp;
					}
					else {
						if (wallHitInfo.openDownMin - wallHitInfo.boundMin.x <= wallHitInfo.openLeftMin - wallHitInfo.boundMin.y)
						{
							//右に広げる。
							horzHint = kHintRight;
						}
						else {
							//上に広げる。
							vertHint = kHintUp;
						}
					}
					if (horzHint == kHintRight) {
						wallHitInfo2.boundMax.x += (wallHitInfo.hitLeft - wallHitInfo.boundMin.x + 1);
					}
					if (vertHint == kHintUp) {
						wallHitInfo2.boundMax.y += (wallHitInfo.hitDown - wallHitInfo.boundMin.y + 1);
					}
					break;
				}
				if (!hitUp && !hitLeft && hitRight && hitDown) {
					//上か左か左上か、広げる方向を調整。
					if (moveRight && moveDown) {
						//if (horzMove >= -vertMove)
						if (wallHitInfo.boundMax.x - wallHitInfo.openDownMax <= wallHitInfo.openRightMin - wallHitInfo.boundMin.y)
						{
							//左に広げる。
							horzHint = kHintLeft;
						}
						else {
							//上に広げる。
							vertHint = kHintUp;
						}
					}
					else if (moveLeft && moveDown) {
						//上に広げる。
						vertHint = kHintUp;
					}
					else if (moveRight && moveUp) {
						//左に広げる。
						horzHint = kHintLeft;
					}
					else if (moveLeft && moveUp) {
						//左上に広げる。
						horzHint = kHintLeft;
						vertHint = kHintUp;
					}
					else if (moveLeft || moveRight) {
						//左に広げる。
						horzHint = kHintLeft;
					}
					else if (moveDown || moveUp) {
						//上に広げる。
						vertHint = kHintUp;
					}
					else {
						if (wallHitInfo.boundMax.x - wallHitInfo.openDownMax <= wallHitInfo.openRightMin - wallHitInfo.boundMin.y)
						{
							//左に広げる。
							horzHint = kHintLeft;
						}
						else {
							//上に広げる。
							vertHint = kHintUp;
						}
					}
					if (horzHint == kHintLeft) {
						wallHitInfo2.boundMin.x -= (wallHitInfo.boundMax.x - wallHitInfo.hitRight + 1);
					}
					if (vertHint == kHintUp) {
						wallHitInfo2.boundMax.y += (wallHitInfo.hitDown - wallHitInfo.boundMin.y + 1);
					}
					break;
				}
				if (hitUp && hitLeft && !hitRight && !hitDown) {
					//下か右か右下か、広げる方向を調整。
					if (moveLeft && moveUp) {
						//if (-horzMove >= vertMove)
						if (wallHitInfo.openUpMin - wallHitInfo.boundMin.x <= wallHitInfo.boundMax.y - wallHitInfo.openLeftMax)
						{
							//右に広げる。
							horzHint = kHintRight;
						}
						else {
							//下に広げる。
							vertHint = kHintDown;
						}
					}
					else if (moveRight && moveUp) {
						//下に広げる。
						vertHint = kHintDown;
					}
					else if (moveLeft && moveDown) {
						//右に広げる。
						horzHint = kHintRight;
					}
					else if (moveRight && moveDown) {
						//右下に広げる。
						horzHint = kHintRight;
						vertHint = kHintDown;
					}
					else if (moveLeft || moveRight) {
						//右に広げる。
						horzHint = kHintRight;
					}
					else if (moveDown || moveUp) {
						//下に広げる。
						vertHint = kHintDown;
					}
					else {
						if (wallHitInfo.openUpMin - wallHitInfo.boundMin.x <= wallHitInfo.boundMax.y - wallHitInfo.openLeftMax)
						{
							//右に広げる。
							horzHint = kHintRight;
						}
						else {
							//下に広げる。
							vertHint = kHintDown;
						}
					}
					if (horzHint == kHintRight) {
						wallHitInfo2.boundMax.x += (wallHitInfo.hitLeft - wallHitInfo.boundMin.x + 1);
					}
					if (vertHint == kHintUp) {
						wallHitInfo2.boundMax.y += (wallHitInfo.hitDown - wallHitInfo.boundMin.y + 1);
					}
					break;
				}
				if (hitUp && !hitLeft && hitRight && !hitDown) {
					//下か左か左下か、広げる方向を調整。
					if (moveRight && moveUp) {
						//if (horzMove >= vertMove)
						if (wallHitInfo.boundMax.x - wallHitInfo.openUpMax <= wallHitInfo.boundMax.y - wallHitInfo.openRightMax)
						{
							//左に広げる。
							horzHint = kHintLeft;
						}
						else {
							//下に広げる。
							vertHint = kHintDown;
						}
					}
					else if (moveLeft && moveUp) {
						//下に広げる。
						vertHint = kHintDown;
					}
					else if (moveRight && moveDown) {
						//左に広げる。
						horzHint = kHintLeft;
					}
					else if (moveLeft && moveDown) {
						//左下に広げる。
						horzHint = kHintLeft;
						vertHint = kHintDown;
					}
					else if (moveLeft || moveRight) {
						//左に広げる。
						horzHint = kHintLeft;
					}
					else if (moveDown || moveUp) {
						//下に広げる。
						vertHint = kHintDown;
					}
					else {
						if (wallHitInfo.boundMax.x - wallHitInfo.openUpMax <= wallHitInfo.boundMax.y - wallHitInfo.openRightMax)
						{
							//左に広げる。
							horzHint = kHintLeft;
						}
						else {
							//下に広げる。
							vertHint = kHintDown;
						}
					}
					if (horzHint == kHintLeft) {
						wallHitInfo2.boundMin.x -= (wallHitInfo.boundMax.x - wallHitInfo.hitRight + 1);
					}
					if (vertHint == kHintDown) {
						wallHitInfo2.boundMin.y -= (wallHitInfo.boundMax.y - wallHitInfo.hitUp + 1);
					}
					break;
				}
				if (moveLeft && moveDown && wallHitInfo.openRightMax > wallHitInfo.boundMax.y && wallHitInfo.openUpMax > wallHitInfo.boundMax.x) {
					//右上に広げる。
					if (wallHitInfo.openUpMin >= wallHitInfo.boundMin.x) {
						wallHitInfo2.boundMax.x += (wallHitInfo.openUpMin - wallHitInfo.boundMin.x + 1);
					}
					else {
						wallHitInfo2.boundMax.x += -horzMove;
					}
					if (wallHitInfo.openRightMin >= wallHitInfo.boundMin.y) {
						wallHitInfo2.boundMax.y += (wallHitInfo.openRightMin - wallHitInfo.boundMin.y + 1);
					}
					else {
						wallHitInfo2.boundMax.y += -vertMove;
					}
					horzHint = kHintRight;
					vertHint = kHintUp;
					break;
				}
				if (moveRight && moveDown && wallHitInfo.openLeftMax > wallHitInfo.boundMax.y && wallHitInfo.openUpMin < wallHitInfo.boundMin.x) {
					//左上に広げる。
					if (wallHitInfo.openUpMax <= wallHitInfo.boundMax.x) {
						wallHitInfo2.boundMin.x -= (wallHitInfo.boundMax.x - wallHitInfo.openUpMax + 1);
					}
					else {
						wallHitInfo2.boundMin.x += -horzMove;
					}
					if (wallHitInfo.openLeftMin >= wallHitInfo.boundMin.y) {
						wallHitInfo2.boundMax.y += (wallHitInfo.openLeftMin - wallHitInfo.boundMin.y + 1);
					}
					else {
						wallHitInfo2.boundMax.y += -vertMove;
					}
					horzHint = kHintLeft;
					vertHint = kHintUp;
					break;
				}
				if (moveLeft && moveUp && wallHitInfo.openRightMin < wallHitInfo.boundMin.y && wallHitInfo.openDownMax > wallHitInfo.boundMax.x) {
					//右下に広げる。
					if (wallHitInfo.openDownMin >= wallHitInfo.boundMin.x) {
						wallHitInfo2.boundMax.x += (wallHitInfo.openDownMin - wallHitInfo.boundMin.x + 1);
					}
					else {
						wallHitInfo2.boundMax.x += -horzMove;
					}
					if (wallHitInfo.openRightMax <= wallHitInfo.boundMax.y) {
						wallHitInfo2.boundMin.y -= (wallHitInfo.boundMax.y - wallHitInfo.openRightMax + 1);
					}
					else {
						wallHitInfo2.boundMin.y += -vertMove;
					}
					horzHint = kHintRight;
					vertHint = kHintDown;
					break;
				}
				if (moveRight && moveUp && wallHitInfo.openLeftMin < wallHitInfo.boundMin.y && wallHitInfo.openDownMin < wallHitInfo.boundMin.x) {
					//左下に広げる。
					if (wallHitInfo.openDownMax <= wallHitInfo.boundMax.x) {
						wallHitInfo2.boundMin.x -= (wallHitInfo.boundMax.x - wallHitInfo.openDownMax + 1);
					}
					else {
						wallHitInfo2.boundMin.x += -horzMove;
					}
					if (wallHitInfo.openLeftMax <= wallHitInfo.boundMax.y) {
						wallHitInfo2.boundMin.y -= (wallHitInfo.boundMax.y - wallHitInfo.openLeftMax + 1);
					}
					else {
						wallHitInfo2.boundMin.y += -vertMove;
					}
					horzHint = kHintLeft;
					vertHint = kHintDown;
					break;
				}
			} while (0);
		}
		wallHitInfo2.initHit();
		//範囲を広げたのでオブジェクトを取り直す。
		this->getObjectList()->removeAllObjects();
		if (sceneLayer) {
			sceneLayer->updateWallCollision(object, CC_CALLBACK_1(Object::callbackDetactionWallCollision, object));
		}
		auto tileList2 = getCollisionTileList(object->getSceneLayer()->getType(), object->getLayerId(), wallHitInfo2.boundMin, wallHitInfo2.boundMax, moveVec,object);
		calcWallHitInfo(object, wallHitInfo2, move, this->getObjectList(), tileList2, false, nullptr, cocos2d::Rect::ZERO);
		bool hitLeft2 = wallHitInfo2.hitLeft >= wallHitInfo2.boundMin.x;
		bool hitRight2 = wallHitInfo2.hitRight <= wallHitInfo2.boundMax.x;
		bool hitDown2 = wallHitInfo2.hitDown >= wallHitInfo2.boundMin.y;
		bool hitUp2 = wallHitInfo2.hitUp <= wallHitInfo2.boundMax.y;
		do {
			if (horzHint >= 0 && vertHint < 0) {
				if (hitLeft2 && hitRight2) {
					//左右の両方でぶつかっている。中間になるように移動量を算出。
					float x = (wallHitInfo2.hitLeft + wallHitInfo2.hitRight) / 2;
					moveX = x - wallHitInfo.center.x;
				}
				else if (horzHint == kHintLeft) {
					moveX = -(wallHitInfo2.boundMax.x - wallHitInfo2.hitRight + 1);
				}
				else {
					moveX = wallHitInfo2.hitLeft - wallHitInfo2.boundMin.x + 1;
				}
				break;
			}
			else if (horzHint < 0 && vertHint >= 0) {
				if (hitDown2 && hitUp2) {
					//上下の両方でぶつかっている。中間になるように移動量を算出。
					float y = (wallHitInfo2.hitDown + wallHitInfo2.hitUp) / 2;
					moveY = y - wallHitInfo.center.y;
				}
				else if (vertHint == kHintDown) {
					moveY = -(wallHitInfo2.boundMax.y - wallHitInfo2.hitUp + 1);
				}
				else {
					moveY = wallHitInfo2.hitDown - wallHitInfo2.boundMin.y + 1;
				}
				break;
			}
			else {
				bool moveOk = false;
				if (horzHint == kHintLeft && vertHint == kHintDown) {
					bool openLeftDown = (wallHitInfo2.openLeftMin < wallHitInfo2.boundMin.y && wallHitInfo2.openDownMin < wallHitInfo2.boundMin.x);
					if (openLeftDown) {
						//左下が空いている。
						if (wallHitInfo2.openDownMax <= wallHitInfo2.boundMax.x) {
							moveX = -(wallHitInfo2.boundMax.x - wallHitInfo2.openDownMax + 1);
						}
						else {
							moveX = -horzMove;
						}
						if (wallHitInfo2.openLeftMax <= wallHitInfo2.boundMax.y) {
							moveY = -(wallHitInfo2.boundMax.y - wallHitInfo2.openLeftMax + 1);
						}
						else {
							moveY = -vertMove;
						}
						moveOk = true;
					}
				}
				else if (horzHint == kHintRight && vertHint == kHintDown) {
					bool openRightDown = (wallHitInfo2.openRightMin < wallHitInfo2.boundMin.y && wallHitInfo2.openDownMax > wallHitInfo2.boundMax.x);
					if (openRightDown) {
						//右下が空いている。
						if (wallHitInfo2.openDownMin >= wallHitInfo2.boundMin.x) {
							moveX = wallHitInfo2.openDownMin - wallHitInfo2.boundMin.x + 1;
						}
						else {
							moveX = -horzMove;
						}
						if (wallHitInfo2.openRightMax <= wallHitInfo2.boundMax.y) {
							moveY = -(wallHitInfo2.boundMax.y - wallHitInfo2.openRightMax + 1);
						}
						else {
							moveY = -vertMove;
						}
						moveOk = true;
					}

				}
				else if (horzHint == kHintLeft && vertHint == kHintUp) {
					bool openLeftUp = (wallHitInfo2.openLeftMax > wallHitInfo2.boundMax.y && wallHitInfo2.openUpMin < wallHitInfo2.boundMin.x);
					if (openLeftUp) {
						//左上が空いている。
						if (wallHitInfo2.openUpMax <= wallHitInfo2.boundMax.x) {
							moveX = -(wallHitInfo2.boundMax.x - wallHitInfo2.openUpMax + 1);
						}
						else {
							moveX = -horzMove;
						}
						if (wallHitInfo2.openLeftMin >= wallHitInfo2.boundMin.y) {
							moveY = wallHitInfo2.openLeftMin - wallHitInfo2.boundMin.y + 1;
						}
						else {
							moveY = -vertMove;
						}
						moveOk = true;
					}
				}
				else if (horzHint == kHintRight && vertHint == kHintUp) {
					bool openRightUp = (wallHitInfo2.openRightMax > wallHitInfo2.boundMax.y && wallHitInfo2.openUpMax > wallHitInfo2.boundMax.x);
					if (openRightUp) {
						//右上が空いている。
						if (wallHitInfo2.openUpMin >= wallHitInfo2.boundMin.x) {
							moveX = wallHitInfo2.openUpMin - wallHitInfo2.boundMin.x + 1;
						}
						else {
							moveX = -horzMove;
						}
						if (wallHitInfo2.openRightMin >= wallHitInfo2.boundMin.y) {
							moveY = wallHitInfo2.openRightMin - wallHitInfo2.boundMin.y + 1;
						}
						else {
							moveY = -vertMove;
						}
						moveOk = true;
					}
				}
				if (!moveOk) {
					float x = (std::max(wallHitInfo2.openUpMin, wallHitInfo2.openDownMin) + std::min(wallHitInfo2.openUpMax, wallHitInfo2.openDownMax)) / 2;
					float y = (std::max(wallHitInfo2.openLeftMin, wallHitInfo2.openRightMin) + std::min(wallHitInfo2.openLeftMax, wallHitInfo2.openRightMax)) / 2;

					bool pushX = false;
					bool pushY = false;

					// 接触している壁の横幅、縦幅と横移動量、縦移動量で
					// X、Yどちらに押し戻すか判断する
					if (move.x != 0 || move.y != 0) {
						// X、Yともに移動している場合
						// 移動方向をもとに対象の頂点が縦の壁、横の壁のどちらに接触するかを検索し、
						// X、Yどちらに戻し処理を行うかを検索
						cocos2d::Point boundPoint, boundPoint2;
						CollisionLine line = { &boundPoint, &boundPoint2 };

						Vec2 cross, cross2;
						cocos2d::Point wallXPoint, wallXPoint2, wallYPoint, wallYPoint2;
						CollisionLine wallXLine = { &wallXPoint, &wallXPoint2 };
						CollisionLine wallYLine = { &wallYPoint, &wallYPoint2 };

						// 左移動時
						if (move.x < 0) {
							wallXPoint = Point(wallHitInfo2.wallRight, wallHitInfo2.wallUp);
							wallXPoint2 = Point(wallHitInfo2.wallRight, wallHitInfo2.wallDown);
							boundPoint.x = wallHitInfo2.boundMin.x;
						}
						// 右移動時
						else if (move.x > 0){
							wallXPoint = Point(wallHitInfo2.wallLeft, wallHitInfo2.wallUp);
							wallXPoint2 = Point(wallHitInfo2.wallLeft, wallHitInfo2.wallDown);
							boundPoint.x = wallHitInfo2.boundMax.x;
						}
						// 横移動なし時
						else {
							float wallWidth = wallHitInfo2.wallRight - wallHitInfo2.wallLeft;
							float objctWidth = wallHitInfo2.boundMax.x - wallHitInfo2.boundMin.x;
							
							// オブジェクトより壁の方が大きい場合
							if (objctWidth < wallWidth) {
								if (wallHitInfo2.wallRight < wallHitInfo2.boundMax.x) {
									wallXPoint = Point(wallHitInfo2.wallRight, wallHitInfo2.wallUp);
									wallXPoint2 = Point(wallHitInfo2.wallRight, wallHitInfo2.wallDown);
									boundPoint.x = wallHitInfo2.boundMin.x;
								}
								else {
									wallXPoint = Point(wallHitInfo2.wallLeft, wallHitInfo2.wallUp);
									wallXPoint2 = Point(wallHitInfo2.wallLeft, wallHitInfo2.wallDown);
									boundPoint.x = wallHitInfo2.boundMax.x;
								}
							}
							else {
								wallXPoint = Point(wallHitInfo2.wallLeft, wallHitInfo2.wallUp);
								wallXPoint2 = Point(wallHitInfo2.wallLeft, wallHitInfo2.wallDown);
								boundPoint.x = wallHitInfo2.boundMax.x;
							}
						}

						// 上移動時
						if (-move.y > 0) {
							wallYPoint = Point(wallHitInfo2.wallLeft, wallHitInfo2.wallDown);
							wallYPoint2 = Point(wallHitInfo2.wallRight, wallHitInfo2.wallDown);
							boundPoint.y = wallHitInfo2.boundMax.y;
						}
						// 下移動時
						else {
							wallYPoint = Point(wallHitInfo2.wallLeft, wallHitInfo2.wallUp);
							wallYPoint2 = Point(wallHitInfo2.wallRight, wallHitInfo2.wallUp);
							boundPoint.y = wallHitInfo2.boundMin.y;
						}

						boundPoint2 = Point(boundPoint.x - move.x, boundPoint.y + move.y);

						pushX = CollisionUtils::checkPushCross2(&wallXLine, &line, &cross);
						pushY = CollisionUtils::checkPushCross2(&wallYLine, &line, &cross2);

						// 両方接触した場合、対象の頂点から近いほうをもとに押し戻しを行う
						if (pushX && pushY) {
							float xLengthSq = (cross - boundPoint2).getLengthSq();
							float yLengthSq = (cross2 - boundPoint2).getLengthSq();

							if (xLengthSq < yLengthSq) {
								pushX = true;
							}
							else {
								pushY = true;
							}
						}
						// 接触できなかった
						else if (!pushX && !pushY) {
#ifdef USE_MULTITHREAD_OBJECT_UPDATE
							if (tileList2.size() <= 1) {
#else
							if (tileList2->count() <= 1) {
#endif
								// 対象の頂点が接触できない位置にある可能性があるので
								// 対象の頂点を変更し再度検索する
								// 左移動
								if (move.x < 0 && boundPoint2.x < wallHitInfo2.wallRight) {
									boundPoint.x = wallHitInfo2.boundMax.x;
									boundPoint2.x = boundPoint.x - move.x;
								}
								// 右移動
								else if (move.x > 0 && boundPoint2.x > wallHitInfo2.wallLeft) {
									boundPoint.x = wallHitInfo2.boundMin.x;
									boundPoint2.x = boundPoint.x - move.x;
								}
								// 横移動なし時
								else{
									float wallWidth = wallHitInfo2.wallRight - wallHitInfo2.wallLeft;
									float objctWidth = wallHitInfo2.boundMax.x - wallHitInfo2.boundMin.x;

									// オブジェクトより壁の方が大きい場合
									if (objctWidth < wallWidth) {
										if (wallHitInfo2.wallRight < wallHitInfo2.boundMax.x) {
											wallXPoint = Point(wallHitInfo2.wallRight, wallHitInfo2.wallUp);
											wallXPoint2 = Point(wallHitInfo2.wallRight, wallHitInfo2.wallDown);
											boundPoint.x = wallHitInfo2.boundMin.x;
										}
										else {
											wallXPoint = Point(wallHitInfo2.wallLeft, wallHitInfo2.wallUp);
											wallXPoint2 = Point(wallHitInfo2.wallLeft, wallHitInfo2.wallDown);
											boundPoint.x = wallHitInfo2.boundMax.x;
										}
									}
									else {
										wallXPoint = Point(wallHitInfo2.wallLeft, wallHitInfo2.wallUp);
										wallXPoint2 = Point(wallHitInfo2.wallLeft, wallHitInfo2.wallDown);
										boundPoint.x = wallHitInfo2.boundMax.x;
									}
								}

								// 上移動
								if (-move.y > 0 && boundPoint2.y > wallHitInfo2.wallDown) {
									boundPoint.y = wallHitInfo2.boundMin.y;
									boundPoint2.y = boundPoint.y + move.y;
								}
								// 下移動
								else if (-move.y < 0 && boundPoint2.y < wallHitInfo2.wallUp){
									boundPoint.y = wallHitInfo2.boundMax.y;
									boundPoint2.y = boundPoint.y + move.y;
								}

								pushX = CollisionUtils::checkPushCross2(&wallXLine, &line, &cross);
								pushY = CollisionUtils::checkPushCross2(&wallYLine, &line, &cross2);

								if (pushX && pushY) {
									float xLengthSq = (cross - boundPoint2).getLengthSq();
									float yLengthSq = (cross2 - boundPoint2).getLengthSq();

									if (xLengthSq < yLengthSq) {
										pushX = true;
									}
									else {
										pushY = true;
									}
								}
							}
							// L字に埋まっている可能性あり
#ifdef USE_MULTITHREAD_OBJECT_UPDATE
							else if (tileList2.size() > 1) {
#else
							else if (tileList2->count() > 1) {
#endif

								pushX = true;
								pushY = true;

								// X、Yともにどこの壁まで戻すかを検索し、壁情報を上書きする
#ifdef USE_MULTITHREAD_OBJECT_UPDATE
								for (auto tile : tileList2) {
#else
								cocos2d::Ref *ref = nullptr;
								CCARRAY_FOREACH(tileList2, ref) {
									auto tile = dynamic_cast<agtk::Tile *>(ref);
#endif
									auto rect = tile->convertToLayerSpaceRect();

									// 左移動
									if (move.x < 0) {
										if (rect.getMinX() < boundPoint.x) {
											wallHitInfo.wallRight = rect.getMaxX();
										}
									}
									// 右移動
									else {
										if (boundPoint.x < rect.getMaxX()) {
											wallHitInfo.wallLeft = rect.getMinX();
										}
									}

									// 上移動
									if (-move.y > 0) {
										if (boundPoint.y < rect.getMaxY()) {
											wallHitInfo.wallDown = rect.getMinY();
										}
									}
									// 下移動
									else {
										if (rect.getMinY() < boundPoint.y) {
											wallHitInfo.wallUp = rect.getMaxY();
										}
									}
								}
							}
						}
					}

#define THRESHOLD	0.00001f
					//if (fabsf(x - wallHitInfo.center.x) <= THRESHOLD) {
					if (fabsf(x - wallHitInfo.center.x) <= THRESHOLD && pushX) {
						////埋まっている場合は元の位置に戻す。
						//moveX = oldPosition.x - tryX;
						//埋まっている場合は側面まで戻す。
						if (move.x > 0.0f) {
							//moveX = wallHitInfo.wallLeft - tryX;
							if (wallHitInfo.wallLeft > (wallHitInfo.center.x - move.x)) {
								moveX = wallHitInfo.wallLeft - wallHitInfo.boundMax.x - 1;
							}
							else {
								moveX = wallHitInfo.wallLeft - wallHitInfo.boundMin.x - 1;
							}
						}
						else if (move.x < 0.0f) {
							//moveX = wallHitInfo.wallRight - tryX;
							if (wallHitInfo.wallRight < (wallHitInfo.center.x - move.x)) {
								moveX = wallHitInfo.wallRight - wallHitInfo.boundMin.x + 1;
							}
							else {
								moveX = wallHitInfo.wallRight - wallHitInfo.boundMax.x + 1;
							}
						}
						else {
							//埋まっている。
							this->setBuriedInWallFlag(true);
						}
						_returnedPos.x = 1.0f;//戻されたフラグ。
						object->getObjectMovement()->reset();//※勢いよくぶつかると「元に戻る」⇒「衝突」を繰り返すので、移動処理をリセットする。
					}
					else {
						moveX = x - wallHitInfo.center.x;
					}
					//if (fabsf(y - wallHitInfo.center.y) <= THRESHOLD) {
					if (fabsf(y - wallHitInfo.center.y) <= THRESHOLD && pushY) {
						////埋まっている場合は元の位置に戻す。
						//moveY = oldPosition.y - tryY;
						//埋まっている場合は、タイル側面まで戻す。
						if (move.y > 0.0f) {
							//moveY = wallHitInfo.wallUp - tryY;
							moveY = wallHitInfo.wallUp - wallHitInfo.boundMin.y;
						}
						else if (move.y < 0.0f) {
							//moveY = wallHitInfo.wallDown - tryY;
							moveY = wallHitInfo.wallDown - wallHitInfo.boundMax.y;
						}
						else {
							//埋まっている。
							this->setBuriedInWallFlag(true);
						}
						_returnedPos.y = 1.0f;//戻されたフラグ。
						object->getObjectMovement()->reset();//※勢いよくぶつかると「元に戻る」⇒「衝突」を繰り返すので、移動処理をリセットする。
					}
					else {
						moveY = y - wallHitInfo.center.y;
					}
				}
				break;
			}
			CCLOG("Unsupposed case2!");
		} while (0);
	}

	wallHitInfo.center += cocos2d::Vec2(moveX, moveY);
	wallHitInfo.boundMin += cocos2d::Vec2(moveX, moveY);
	wallHitInfo.boundMax += cocos2d::Vec2(moveX, moveY);
	wallHitInfo.boundMin.x -= 1;
	wallHitInfo.boundMin.y -= 1;
	wallHitInfo.boundMax.x += 1;
	wallHitInfo.boundMax.y += 1;
	//他オブジェクトの壁判定に接触
	wallHitInfo.initHit();
	//オブジェクトを取り直す。
	this->getObjectList()->removeAllObjects();
	if (sceneLayer) {
		sceneLayer->updateWallCollision(object, CC_CALLBACK_1(Object::callbackDetactionWallCollision, object));
	}
	wallHitInfo.boundMin.x += 1;
	wallHitInfo.boundMin.y += 1;
	wallHitInfo.boundMax.x -= 1;
	wallHitInfo.boundMax.y -= 1;

	auto objectTemplateMove = object->getObjectTemplateMove();
	if (objectTemplateMove->isIgnoredObjectWall() == false) {//テンプレート移動で「他オブジェクトの壁判定を無視する」
		getTouchingObjectWallHitInfo(wallHitInfo, _object, this->getObjectList(), this->getLeftWallObjectList(), this->getRightWallObjectList(), this->getUpWallObjectList(), this->getDownWallObjectList());
	}
	//タイルの壁判定に接触
	wallHitInfo.boundMin.x -= 1;
	wallHitInfo.boundMin.y -= 1;
	wallHitInfo.boundMax.x += 1;
	wallHitInfo.boundMax.y += 1;
	wallHitInfo.initHit();
#ifdef USE_MULTITHREAD_OBJECT_UPDATE
#else
	tileList = NULL;
#endif
	tileList = getCollisionTileList(object->getSceneLayer()->getType(), object->getLayerId(), wallHitInfo.boundMin, wallHitInfo.boundMax, Vec2::ZERO, object);
	wallHitInfo.boundMin.x += 1;
	wallHitInfo.boundMin.y += 1;
	wallHitInfo.boundMax.x -= 1;
	wallHitInfo.boundMax.y -= 1;
	auto tmpWallHitInfoRect = cocos2d::Rect(wallHitInfo.boundMin, cocos2d::Size(wallHitInfo.boundMax - wallHitInfo.boundMin));
	cocos2d::Rect thresholdRect;
	int tileWallBit = 0;
	wallHitInfo.boundMin.x -= 1;
	wallHitInfo.initHit();
	thresholdRect = cocos2d::Rect(tmpWallHitInfoRect.origin - wallHitInfo.boundMin, tmpWallHitInfoRect.size - wallHitInfo.getRect().size);
	calcWallHitInfo(object, wallHitInfo, move, nullptr, tileList, true, nullptr, thresholdRect);
	tileWallBit |= (wallHitInfo.hitLeft >= wallHitInfo.boundMin.x ? agtk::data::ObjectActionLinkConditionData::kWallBitLeft : 0);
	wallHitInfo.boundMin.x += 1;

	wallHitInfo.boundMin.y -= 1;
	wallHitInfo.initHit();
	thresholdRect = cocos2d::Rect(tmpWallHitInfoRect.origin - wallHitInfo.boundMin, tmpWallHitInfoRect.size - wallHitInfo.getRect().size);
	calcWallHitInfo(object, wallHitInfo, move, nullptr, tileList, true, nullptr, thresholdRect);
	tileWallBit |= (wallHitInfo.hitDown >= wallHitInfo.boundMin.y ? agtk::data::ObjectActionLinkConditionData::kWallBitDown : 0);
	wallHitInfo.boundMin.y += 1;

	wallHitInfo.boundMax.x += 1;
	wallHitInfo.initHit();
	thresholdRect = cocos2d::Rect(tmpWallHitInfoRect.origin - wallHitInfo.boundMin, tmpWallHitInfoRect.size - wallHitInfo.getRect().size);
	calcWallHitInfo(object, wallHitInfo, move, nullptr, tileList, true, nullptr, thresholdRect);
	tileWallBit |= (wallHitInfo.hitRight <= wallHitInfo.boundMax.x ? agtk::data::ObjectActionLinkConditionData::kWallBitRight : 0);
	wallHitInfo.boundMax.x -= 1;

	wallHitInfo.boundMax.y += 1;
	wallHitInfo.initHit();
	thresholdRect = cocos2d::Rect(tmpWallHitInfoRect.origin - wallHitInfo.boundMin, tmpWallHitInfoRect.size - wallHitInfo.getRect().size);
	calcWallHitInfo(object, wallHitInfo, move, nullptr, tileList, true, nullptr, thresholdRect);
	tileWallBit |= (wallHitInfo.hitUp <= wallHitInfo.boundMax.y ? agtk::data::ObjectActionLinkConditionData::kWallBitUp : 0);
	wallHitInfo.boundMax.y -= 1;
	*pTileWallBit = tileWallBit;
	//ここでtileWallBitのビットが立っている場合
	if (tileWallBit) {
		updateObjectWallEffect(object, tileList, agtk::data::TileData::kConditionTouched);//オブジェクトに効果を設定（プレイヤーがタイルに触れた場合）
	}

	//進んだ先でタイルの壁判定に接触
	wallHitInfo.boundMin.x -= 2;
	wallHitInfo.boundMin.y -= 2;
	wallHitInfo.boundMax.x += 2;
	wallHitInfo.boundMax.y += 2;
	wallHitInfo.initHit();
#ifdef USE_MULTITHREAD_OBJECT_UPDATE
#else
	tileList = NULL;
#endif
	tileList = getCollisionTileList(object->getSceneLayer()->getType(), object->getLayerId(), wallHitInfo.boundMin, wallHitInfo.boundMax,Vec2::ZERO, object);
	wallHitInfo.boundMin.x += 2;
	wallHitInfo.boundMin.y += 2;
	wallHitInfo.boundMax.x -= 2;
	wallHitInfo.boundMax.y -= 2;
	tileWallBit = 0;
	wallHitInfo.boundMin.x -= 2;
	wallHitInfo.initHit();
	thresholdRect = cocos2d::Rect(tmpWallHitInfoRect.origin - wallHitInfo.boundMin, tmpWallHitInfoRect.size - wallHitInfo.getRect().size);
	calcWallHitInfo(object, wallHitInfo, move, nullptr, tileList, true, nullptr, thresholdRect);
	tileWallBit |= (wallHitInfo.hitLeft >= wallHitInfo.boundMin.x ? agtk::data::ObjectActionLinkConditionData::kWallBitLeft : 0);
	wallHitInfo.boundMin.x += 2;

	wallHitInfo.boundMin.y -= 2;
	wallHitInfo.initHit();
	thresholdRect = cocos2d::Rect(tmpWallHitInfoRect.origin - wallHitInfo.boundMin, tmpWallHitInfoRect.size - wallHitInfo.getRect().size);
	calcWallHitInfo(object, wallHitInfo, move, nullptr, tileList, true, nullptr, thresholdRect);
	tileWallBit |= (wallHitInfo.hitDown >= wallHitInfo.boundMin.y ? agtk::data::ObjectActionLinkConditionData::kWallBitDown : 0);
	wallHitInfo.boundMin.y += 2;

	wallHitInfo.boundMax.x += 2;
	wallHitInfo.initHit();
	thresholdRect = cocos2d::Rect(tmpWallHitInfoRect.origin - wallHitInfo.boundMin, tmpWallHitInfoRect.size - wallHitInfo.getRect().size);
	calcWallHitInfo(object, wallHitInfo, move, nullptr, tileList, true, nullptr, thresholdRect);
	tileWallBit |= (wallHitInfo.hitRight <= wallHitInfo.boundMax.x ? agtk::data::ObjectActionLinkConditionData::kWallBitRight : 0);
	wallHitInfo.boundMax.x -= 2;

	wallHitInfo.boundMax.y += 2;
	wallHitInfo.initHit();
	thresholdRect = cocos2d::Rect(tmpWallHitInfoRect.origin - wallHitInfo.boundMin, tmpWallHitInfoRect.size - wallHitInfo.getRect().size);
	calcWallHitInfo(object, wallHitInfo, move, nullptr, tileList, true, nullptr, thresholdRect);
	tileWallBit |= (wallHitInfo.hitUp <= wallHitInfo.boundMax.y ? agtk::data::ObjectActionLinkConditionData::kWallBitUp : 0);
	wallHitInfo.boundMax.y -= 2;
	*pAheadTileWallBit = tileWallBit;

	//「オブジェクト」-「基本設定」-「他のオブジェクトから押し戻されない」 チェック無し の場合、移動する。
	moveX += wallHitInfo.move.x;
	moveY += wallHitInfo.move.y;
	return cocos2d::Vec2(moveX, moveY);
}

/**
* 壁判定を基にタイルと坂との衝突を解決する
* @param	wallHitInfo					壁判定情報
* @apram	pTileWallBit				衝突したタイルの衝突方向ビット値(出力)
* @apram	pLinkConditionTileWallBit	衝突したタイルの衝突方向ビット値(出力)[アクションリンク判定用]
* @param	pAheadTileWallBit			1dot進んだ先で衝突したタイルの衝突方向ビット値(出力)
* @param	pSlopeBit					衝突した坂の衝突方向ビット値(出力)
* @param	move						前フレームからの移動量
* @param	upObjectList				上側壁判定に衝突したオブジェクトリスト(出力)
* @param	downObjectList				下側壁判定に衝突したオブジェクトリスト(出力)
* @param	tileList					衝突したタイルリスト
* @param	slopeList					衝突した坂リスト
* @param	checkSlope					坂との衝突チェックを行うか？
*/
#ifdef USE_MULTITHREAD_OBJECT_UPDATE
cocos2d::Vec2 ObjectCollision::newUpdateWallHitInfo(LockObjList &lockObjList, WallHitInfo &wallHitInfo, int *pTileWallBit, HitTileWalls* pLinkConditionTileWall, HitTileWalls* pAheadTileWall, int* pSlopeBit, cocos2d::Vec2 move, agtk::MtVector<agtk::Object *>* upObjectList, agtk::MtVector<agtk::Object *>* downObjectList, std::vector<agtk::Tile *> tileList, agtk::MtVector<agtk::Slope *> *slopeList, bool checkSlope)
#else
cocos2d::Vec2 ObjectCollision::newUpdateWallHitInfo(WallHitInfo &wallHitInfo, int *pTileWallBit, HitTileWalls* pLinkConditionTileWall, HitTileWalls* pAheadTileWall, int* pSlopeBit, cocos2d::Vec2 move, cocos2d::Array* upObjectList, cocos2d::Array* downObjectList, cocos2d::__Array *tileList, cocos2d::__Array *slopeList, bool checkSlope)
#endif
{
	//THREAD_PRINTF("newUpdateWallHitInfo(0x%x)", &lockObjList);
	CC_ASSERT(_object->getObjectWallCollision() == this);	//仮説
	float moveX = 0;
	float moveY = 0;
	auto object = _object;

	// 初期の壁情報を保持
	WallHitInfo initInfo = wallHitInfo;
	auto size = wallHitInfo.boundMax - wallHitInfo.boundMin;

#ifdef USE_MULTITHREAD_OBJECT_UPDATE
	newCalcWallHitInfo(lockObjList, object, wallHitInfo, move, this->getObjectList(), tileList, pSlopeBit, slopeList, checkSlope, pTileWallBit, pLinkConditionTileWall);
#else
	newCalcWallHitInfo(object, wallHitInfo, move, this->getObjectList(), tileList, pSlopeBit, slopeList, checkSlope);
#endif

	moveX = wallHitInfo.boundMin.x - initInfo.boundMin.x;
	moveY = wallHitInfo.boundMin.y - initInfo.boundMin.y;

	wallHitInfo.boundMax = wallHitInfo.boundMin + size;
	wallHitInfo.center = wallHitInfo.boundMin;
	wallHitInfo.center.x += size.x * 0.5f;
	wallHitInfo.center.y += size.y * 0.5f;

	wallHitInfo.boundMin.x -= 1;
	wallHitInfo.boundMin.y -= 1;
	wallHitInfo.boundMax.x += 1;
	wallHitInfo.boundMax.y += 1;
	//他オブジェクトの壁判定に接触
	wallHitInfo.initHit();
	//オブジェクトを取り直す。
	this->getObjectList()->removeAllObjects();

	auto sceneLayer = _object->getSceneLayer();
	if(sceneLayer)
	{
		// 物理演算がOFF または 接続されている物理オブジェクトの動作を優先が OFF
		if (!object->getObjectData()->getPhysicsSettingFlag() || !object->getPlayObjectData()->getSwitchData(agtk::data::kObjectSystemSwitchFollowConnectedPhysics)->getValue()) {
			// 移動発生を考慮した位置にオブジェクトを設定
			Vec2 crntPos = object->getPosition();
			bool bChangePosition = (cocos2d::Vec2(moveX, -moveY) != cocos2d::Vec2::ZERO);
			if (bChangePosition) {
				object->setPosition(crntPos + Vec2(moveX, -moveY));
			}
			// 移動後の座標でオブジェクトリストの取得を試みる
#ifdef USE_MULTITHREAD_OBJECT_UPDATE
			sceneLayer->updateThreadWallCollision(object, CC_CALLBACK_1(Object::callbackDetactionWallCollision, object));
#else
			sceneLayer->updateWallCollision(object, CC_CALLBACK_1(Object::callbackDetactionWallCollision, object));
#endif
			// 座標を戻す
			if (bChangePosition) {
				object->setPosition(crntPos);
			}
		}
	}
	wallHitInfo.boundMin.x += 1;
	wallHitInfo.boundMin.y += 1;
	wallHitInfo.boundMax.x -= 1;
	wallHitInfo.boundMax.y -= 1;

	auto objectTemplateMove = object->getObjectTemplateMove();
	if (objectTemplateMove->isIgnoredObjectWall() == false) {//テンプレート移動で「他オブジェクトの壁判定を無視する」
// #AGTK-NX #AGTK-WIN
#if defined(USE_30FPS_3) && defined(USE_30FPS_3_2)
		if(GameManager::getInstance()->getLastPass()) {
#endif
		getTouchingObjectWallHitInfo(wallHitInfo, _object, this->getObjectList(), this->getLeftWallObjectList(), this->getRightWallObjectList(), this->getUpWallObjectList(), this->getDownWallObjectList());
// #AGTK-NX #AGTK-WIN
#if defined(USE_30FPS_3) && defined(USE_30FPS_3_2)
		} //if (GameManager::getInstance()->getLastPass())
#endif
	}

	//タイルの壁判定に接触（タイルと接触しないオブジェクトはタイルとの接触判定を無視する）
	{
		wallHitInfo.boundMin.x -= 1;
		wallHitInfo.boundMin.y -= 1;
		wallHitInfo.boundMax.x += 1;
		wallHitInfo.boundMax.y += 1;
		wallHitInfo.initHit();
		tileList = getCollisionTileList(object->getSceneLayer()->getType(), object->getLayerId(), wallHitInfo.boundMin, wallHitInfo.boundMax, Vec2::ZERO, object);
		wallHitInfo.boundMin.x += 1;
		wallHitInfo.boundMin.y += 1;
		wallHitInfo.boundMax.x -= 1;
		wallHitInfo.boundMax.y -= 1;

#ifdef USE_MULTITHREAD_OBJECT_UPDATE
		std::vector<agtk::Tile *> hitTileList;
		std::function<void()> addHitTileList = [&]() {
			for (auto & hitTile : wallHitInfo.hitTiles) {
				auto tmptile = const_cast<agtk::Tile*>(hitTile.second);			
				if (std::find(hitTileList.begin(), hitTileList.end(), tmptile) == hitTileList.end()) {
					hitTileList.push_back(tmptile);
				}
			}
		};
#else
		auto hitTileList = cocos2d::__Array::create();
		std::function<void()> addHitTileList = [&]() {
			for (auto & hitTile : wallHitInfo.hitTiles) {
				auto tmptile = const_cast<agtk::Tile*>(hitTile.second);
				if (!hitTileList->containsObject(tmptile)) {
					hitTileList->addObject(tmptile);
				}
			}
		};
#endif

		auto tmpWallHitInfoRect = cocos2d::Rect(wallHitInfo.boundMin, cocos2d::Size(wallHitInfo.boundMax - wallHitInfo.boundMin));
		cocos2d::Rect thresholdRect;
		int tileWallBit = 0;
		wallHitInfo.boundMin.x -= 1;
		wallHitInfo.initHit();
		thresholdRect = cocos2d::Rect(tmpWallHitInfoRect.origin - wallHitInfo.boundMin, tmpWallHitInfoRect.size - wallHitInfo.getRect().size);
		calcWallHitInfo(object, wallHitInfo, move, nullptr, tileList, false, nullptr, thresholdRect);
		tileWallBit |= (wallHitInfo.hitLeft >= wallHitInfo.boundMin.x ? agtk::data::ObjectActionLinkConditionData::kWallBitLeft : 0);
		addHitTileList();
		wallHitInfo.boundMin.x += 1;

		wallHitInfo.boundMin.y -= 1;
		wallHitInfo.initHit();
		thresholdRect = cocos2d::Rect(tmpWallHitInfoRect.origin - wallHitInfo.boundMin, tmpWallHitInfoRect.size - wallHitInfo.getRect().size);
		calcWallHitInfo(object, wallHitInfo, move, nullptr, tileList, false, nullptr, thresholdRect);
		tileWallBit |= (wallHitInfo.hitDown >= wallHitInfo.boundMin.y ? agtk::data::ObjectActionLinkConditionData::kWallBitDown : 0);
		addHitTileList();
		wallHitInfo.boundMin.y += 1;

		wallHitInfo.boundMax.x += 1;
		wallHitInfo.initHit();
		thresholdRect = cocos2d::Rect(tmpWallHitInfoRect.origin - wallHitInfo.boundMin, tmpWallHitInfoRect.size - wallHitInfo.getRect().size);
		calcWallHitInfo(object, wallHitInfo, move, nullptr, tileList, false, nullptr, thresholdRect);
		tileWallBit |= (wallHitInfo.hitRight <= wallHitInfo.boundMax.x ? agtk::data::ObjectActionLinkConditionData::kWallBitRight : 0);
		addHitTileList();
		wallHitInfo.boundMax.x -= 1;

		wallHitInfo.boundMax.y += 1;
		wallHitInfo.initHit();
		thresholdRect = cocos2d::Rect(tmpWallHitInfoRect.origin - wallHitInfo.boundMin, tmpWallHitInfoRect.size - wallHitInfo.getRect().size);
		calcWallHitInfo(object, wallHitInfo, move, nullptr, tileList, false, nullptr, thresholdRect);
		tileWallBit |= (wallHitInfo.hitUp <= wallHitInfo.boundMax.y ? agtk::data::ObjectActionLinkConditionData::kWallBitUp : 0);
		addHitTileList();
		wallHitInfo.boundMax.y -= 1;
		*pTileWallBit |= tileWallBit;
		//ここでtileWallBitのビットが立っている場合
		if (tileWallBit) {
			updateObjectWallEffect(object, hitTileList, agtk::data::TileData::kConditionTouched);//オブジェクトに効果を設定（プレイヤーがタイルに触れた場合）
		}
	}
	//タイルの壁判定に接触（タイルと接触しないオブジェクトでもタイルとの接触を確認する）
	{
		int tileWallBit = 0;

		// タイル取得
		wallHitInfo.boundMin.x -= 1;
		wallHitInfo.boundMin.y -= 1;
		wallHitInfo.boundMax.x += 1;
		wallHitInfo.boundMax.y += 1;
		wallHitInfo.initHit();
#ifdef USE_MULTITHREAD_OBJECT_UPDATE
#else
		tileList = NULL;
#endif
		tileList = getCollisionTileList(object->getSceneLayer()->getType(), object->getLayerId(), wallHitInfo.boundMin, wallHitInfo.boundMax, Vec2::ZERO, object);
		wallHitInfo.boundMin.x += 1;
		wallHitInfo.boundMin.y += 1;
		wallHitInfo.boundMax.x -= 1;
		wallHitInfo.boundMax.y -= 1;

		auto tmpWallHitInfoRect = cocos2d::Rect(wallHitInfo.boundMin, cocos2d::Size(wallHitInfo.boundMax - wallHitInfo.boundMin));
		cocos2d::Rect thresholdRect;
		//左
		wallHitInfo.boundMin.x -= 1;
		wallHitInfo.initHit();
		thresholdRect = cocos2d::Rect(tmpWallHitInfoRect.origin - wallHitInfo.boundMin, tmpWallHitInfoRect.size - wallHitInfo.getRect().size);
		calcWallHitInfo(object, wallHitInfo, move, nullptr, tileList, true, nullptr, thresholdRect);
// #AGTK-NX #AGTK-WIN
#if defined(USE_30FPS_3) && defined(USE_30FPS_3_2)
		if (GameManager::getInstance()->getLastPass()) {
#endif
		tileWallBit |= (wallHitInfo.hitLeft >= wallHitInfo.boundMin.x ? agtk::data::ObjectActionLinkConditionData::kWallBitLeft : 0);
		addHitTileFunc(pLinkConditionTileWall, tileWallBit, wallHitInfo, agtk::data::ObjectActionLinkConditionData::kWallBitLeft);
// #AGTK-NX #AGTK-WIN
#if defined(USE_30FPS_3) && defined(USE_30FPS_3_2)
		} // if (GameManager::getInstance()->getLastPass())
#endif
		wallHitInfo.boundMin.x += 1;

		//下
		wallHitInfo.boundMin.y -= 1;
		wallHitInfo.initHit();
		thresholdRect = cocos2d::Rect(tmpWallHitInfoRect.origin - wallHitInfo.boundMin, tmpWallHitInfoRect.size - wallHitInfo.getRect().size);
		calcWallHitInfo(object, wallHitInfo, move, nullptr, tileList, true, nullptr, thresholdRect);
// #AGTK-NX #AGTK-WIN
#if defined(USE_30FPS_3) && defined(USE_30FPS_3_2)
		if (GameManager::getInstance()->getLastPass()) {
#endif
		tileWallBit |= (wallHitInfo.hitDown >= wallHitInfo.boundMin.y ? agtk::data::ObjectActionLinkConditionData::kWallBitDown : 0);
		addHitTileFunc(pLinkConditionTileWall, tileWallBit, wallHitInfo, agtk::data::ObjectActionLinkConditionData::kWallBitDown);
// #AGTK-NX #AGTK-WIN
#if defined(USE_30FPS_3) && defined(USE_30FPS_3_2)
		} // if (GameManager::getInstance()->getLastPass())
#endif
		wallHitInfo.boundMin.y += 1;

		//右
		wallHitInfo.boundMax.x += 1;
		wallHitInfo.initHit();
		thresholdRect = cocos2d::Rect(tmpWallHitInfoRect.origin - wallHitInfo.boundMin, tmpWallHitInfoRect.size - wallHitInfo.getRect().size);
		calcWallHitInfo(object, wallHitInfo, move, nullptr, tileList, true, nullptr, thresholdRect);
// #AGTK-NX #AGTK-WIN
#if defined(USE_30FPS_3) && defined(USE_30FPS_3_2)
		if (GameManager::getInstance()->getLastPass()) {
#endif
		tileWallBit |= (wallHitInfo.hitRight <= wallHitInfo.boundMax.x ? agtk::data::ObjectActionLinkConditionData::kWallBitRight : 0);
		addHitTileFunc(pLinkConditionTileWall, tileWallBit, wallHitInfo, agtk::data::ObjectActionLinkConditionData::kWallBitRight);
// #AGTK-NX #AGTK-WIN
#if defined(USE_30FPS_3) && defined(USE_30FPS_3_2)
		} // if (GameManager::getInstance()->getLastPass())
#endif
		wallHitInfo.boundMax.x -= 1;

		//上
		wallHitInfo.boundMax.y += 1;
		wallHitInfo.initHit();
		thresholdRect = cocos2d::Rect(tmpWallHitInfoRect.origin - wallHitInfo.boundMin, tmpWallHitInfoRect.size - wallHitInfo.getRect().size);
		calcWallHitInfo(object, wallHitInfo, move, nullptr, tileList, true, nullptr, thresholdRect);
// #AGTK-NX #AGTK-WIN
#if defined(USE_30FPS_3) && defined(USE_30FPS_3_2)
		if (GameManager::getInstance()->getLastPass()) {
#endif
		tileWallBit |= (wallHitInfo.hitUp <= wallHitInfo.boundMax.y ? agtk::data::ObjectActionLinkConditionData::kWallBitUp : 0);
		addHitTileFunc(pLinkConditionTileWall, tileWallBit, wallHitInfo, agtk::data::ObjectActionLinkConditionData::kWallBitUp);
// #AGTK-NX #AGTK-WIN
#if defined(USE_30FPS_3) && defined(USE_30FPS_3_2)
		} // if (GameManager::getInstance()->getLastPass())
#endif
		wallHitInfo.boundMax.y -= 1;
	}
	//進んだ先でタイルの壁判定に接触
#if 1//ACT2-1682 「１タイル進んだ先でタイル判定に接触」の実装。
// #AGTK-NX #AGTK-WIN
#if defined(USE_30FPS_3) && defined(USE_30FPS_3_2)
	if (GameManager::getInstance()->getLastPass()) {
#else
	{
#endif
		std::function<void(cocos2d::Vec2, cocos2d::Size, int)> calcWallHit = [&,pAheadTileWall](cocos2d::Vec2 pos, cocos2d::Size size, int directionId) {

			bool const left = (directionId == 1 || directionId == 4 || directionId == 7);
			bool const right = (directionId == 3 || directionId == 6 || directionId == 9);
			bool const up = (directionId == 7 || directionId == 8 || directionId == 9);
			bool const down = (directionId == 1 || directionId == 2 || directionId == 3);

			float const minX = pos.x;
			float const minY = pos.y;
			float const maxX = minX + size.width;
			float const maxY = minY + size.height;

			WallHitInfo wallHitInfo;
			wallHitInfo.center = cocos2d::Point(floorf((minX + maxX) / 2), floorf((minY + maxY) / 2));
			wallHitInfo.boundMin = cocos2d::Point(minX, minY);
			wallHitInfo.boundMax = cocos2d::Point(maxX, maxY);

			wallHitInfo.initHit();
#ifdef USE_MULTITHREAD_OBJECT_UPDATE
			auto tileList = getCollisionTileList(object->getSceneLayer()->getType(), object->getLayerId(), wallHitInfo.boundMin, wallHitInfo.boundMax, Vec2::ZERO, object);
#else
			auto const tileList = getCollisionTileList(object->getSceneLayer()->getType(), object->getLayerId(), wallHitInfo.boundMin, wallHitInfo.boundMax, Vec2::ZERO, object);
#endif
			calcWallHitInfo(object, wallHitInfo, move, nullptr, tileList, true, nullptr, cocos2d::Rect::ZERO);

			int tileWallBit = 0;
			tileWallBit |= ((wallHitInfo.hitLeft >= wallHitInfo.boundMin.x && left) ? agtk::data::ObjectActionLinkConditionData::kWallBitLeft : 0);
			tileWallBit |= ((wallHitInfo.hitDown >= wallHitInfo.boundMin.y && down) ? agtk::data::ObjectActionLinkConditionData::kWallBitDown : 0);
			tileWallBit |= ((wallHitInfo.hitRight <= wallHitInfo.boundMax.x && right) ? agtk::data::ObjectActionLinkConditionData::kWallBitRight : 0);
			tileWallBit |= ((wallHitInfo.hitUp <= wallHitInfo.boundMax.y && up) ? agtk::data::ObjectActionLinkConditionData::kWallBitUp : 0);

			if (pAheadTileWall) {
				for (auto & hitTile : wallHitInfo.hitTiles) {
					if (hitTile.first & tileWallBit) {
						ObjectCollision::HitTileWall htw;
						htw.bit = tileWallBit;
						htw.tile = hitTile.second;
						pAheadTileWall->emplace_back(htw);
					}
				}
			}
		};
		auto projectData = GameManager::getInstance()->getProjectData();

		//地面に立っている&重力影響有り。
		bool bGravity = false;
		auto gravity = _object->getObjectMovement()->getGravity();
		if (gravity != cocos2d::Vec2::ZERO) {
			if (_object->_floor || _object->_floorOld) {
				gravity = gravity.getNormalized();
				bGravity = true;
			}
		}
		else {
			auto scene = GameManager::getInstance()->getCurrentScene();
			gravity = scene->getGravity()->getGravity();
			auto gn = gravity.getNormalized();
			int wallBitDown = 0;
			if (gn != cocos2d::Vec2::ZERO) {
				//重力上方向
				if (gn.y > 0) {
					if (gn.x > 0) {
						wallBitDown = (abs(gn.y) >= abs(gn.x)) ? agtk::data::ObjectActionLinkConditionData::kWallBitUp : agtk::data::ObjectActionLinkConditionData::kWallBitRight;
					}
					else if (gn.x < 0) {
						wallBitDown = (abs(gn.y) >= abs(gn.x)) ? agtk::data::ObjectActionLinkConditionData::kWallBitUp : agtk::data::ObjectActionLinkConditionData::kWallBitLeft;
					}
					else {
						CC_ASSERT(gn.x == 0.0f);
						wallBitDown = agtk::data::ObjectActionLinkConditionData::kWallBitUp;
					}
				}
				//重力下方向
				else if (gn.y < 0) {
					if (gn.x > 0) {
						wallBitDown = (abs(gn.y) >= abs(gn.x)) ? agtk::data::ObjectActionLinkConditionData::kWallBitDown : agtk::data::ObjectActionLinkConditionData::kWallBitRight;
					}
					if (gn.x < 0) {
						wallBitDown = (abs(gn.y) >= abs(gn.x)) ? agtk::data::ObjectActionLinkConditionData::kWallBitDown : agtk::data::ObjectActionLinkConditionData::kWallBitLeft;
					}
					else {
						//				CC_ASSERT(gn.x == 0.0f);
						wallBitDown = agtk::data::ObjectActionLinkConditionData::kWallBitDown;
					}
				}
				else {
					CC_ASSERT(gn.y == 0.0f);
					wallBitDown = gn.x > 0 ? agtk::data::ObjectActionLinkConditionData::kWallBitRight : agtk::data::ObjectActionLinkConditionData::kWallBitLeft;
				}
			}
			if (_object->getTileWallBit() & wallBitDown) {
			gravity = gravity.getNormalized();
			bGravity = true;
		}
		}
		float gravityDegree = agtk::GetDegreeFromVector(gravity);
		int gravityDirectionId = agtk::GetMoveDirectionId(gravityDegree);

		cocos2d::Vec2 basePos = wallHitInfo.center - cocos2d::Vec2(wallHitInfo.size) * 0.5;
		auto moveNormalize = move.getNormalized();
		if (moveNormalize == cocos2d::Vec2::ZERO) {
			moveNormalize = agtk::GetDirectionFromMoveDirectionId(object->getDispDirection()).getNormalized();
		}
		moveNormalize.y *= -1.0f;
		if(moveNormalize != cocos2d::Vec2::ZERO) {
			float degree = agtk::GetDegreeFromVector(moveNormalize);
			int directionId = agtk::GetMoveDirectionId(degree);
			switch (directionId) {
			case 1: {//左下
				auto pos = basePos + cocos2d::Vec2(-projectData->getTileWidth(), -projectData->getTileHeight());
				calcWallHit(pos, wallHitInfo.size, directionId); 
				if (bGravity) {
					calcWallHit(pos + gravity, wallHitInfo.size, gravityDirectionId);
				}
				break; }
			case 2: {//下
				auto pos = basePos + cocos2d::Vec2(0, -projectData->getTileHeight());
				calcWallHit(pos, wallHitInfo.size, directionId);
				if (bGravity) {
					calcWallHit(pos + gravity, wallHitInfo.size, gravityDirectionId);
				}
				break; }
			case 3: {//右下
				auto pos = basePos + cocos2d::Vec2(projectData->getTileWidth(), -projectData->getTileHeight());
				calcWallHit(pos, wallHitInfo.size, directionId);
				if (bGravity) {
					calcWallHit(pos + gravity, wallHitInfo.size, gravityDirectionId);
				}
				break; }
			case 4: {//左
				auto pos = basePos + cocos2d::Vec2(-projectData->getTileWidth(), 0);
				calcWallHit(pos, wallHitInfo.size, directionId);
				if (bGravity) {
					calcWallHit(pos + gravity, wallHitInfo.size, gravityDirectionId);
				}
				break; }
			case 6: {//右
				auto pos = basePos + cocos2d::Vec2(projectData->getTileWidth(), 0);
				calcWallHit(pos, wallHitInfo.size, directionId);
				if (bGravity) {
					calcWallHit(pos + gravity, wallHitInfo.size, gravityDirectionId);
				}
				break; }
			case 7: {//左上
				auto pos = basePos + cocos2d::Vec2(-projectData->getTileWidth(), projectData->getTileHeight());
				calcWallHit(pos, wallHitInfo.size, directionId);
				if (bGravity) {
					calcWallHit(pos + gravity, wallHitInfo.size, gravityDirectionId);
				}
				break; }
			case 8: {//上
				auto pos = basePos + cocos2d::Vec2(0, projectData->getTileHeight());
				calcWallHit(pos, wallHitInfo.size, directionId);
				if (bGravity) {
					calcWallHit(pos + gravity, wallHitInfo.size, gravityDirectionId);
				}
				break; }
			case 9: {//右上
				auto pos = basePos + cocos2d::Vec2(projectData->getTileWidth(), projectData->getTileHeight());
				calcWallHit(pos, wallHitInfo.size, directionId);
				if (bGravity) {
					calcWallHit(pos + gravity, wallHitInfo.size, gravityDirectionId);
				}
				break; }
			}
		}
	}
#else
	wallHitInfo.boundMin.x -= 2;
	wallHitInfo.boundMin.y -= 2;
	wallHitInfo.boundMax.x += 2;
	wallHitInfo.boundMax.y += 2;
	wallHitInfo.initHit();
	tileList = NULL;
	tileList = getCollisionTileList(object->getSceneLayer()->getType(), object->getLayerId(), wallHitInfo.boundMin, wallHitInfo.boundMax);
	wallHitInfo.boundMin.x += 2;
	wallHitInfo.boundMin.y += 2;
	wallHitInfo.boundMax.x -= 2;
	wallHitInfo.boundMax.y -= 2;
	tileWallBit = 0;
	wallHitInfo.boundMin.x -= 2;
	wallHitInfo.initHit();
	calcWallHitInfo(object, wallHitInfo, move, nullptr, tileList, true, nullptr);
	tileWallBit |= (wallHitInfo.hitLeft >= wallHitInfo.boundMin.x ? agtk::data::ObjectActionLinkConditionData::kWallBitLeft : 0);
	wallHitInfo.boundMin.x += 2;

	wallHitInfo.boundMin.y -= 2;
	wallHitInfo.initHit();
	calcWallHitInfo(object, wallHitInfo, move, nullptr, tileList, true, nullptr);
	tileWallBit |= (wallHitInfo.hitDown >= wallHitInfo.boundMin.y ? agtk::data::ObjectActionLinkConditionData::kWallBitDown : 0);
	wallHitInfo.boundMin.y += 2;

	wallHitInfo.boundMax.x += 2;
	wallHitInfo.initHit();
	calcWallHitInfo(object, wallHitInfo, move, nullptr, tileList, true, nullptr);
	tileWallBit |= (wallHitInfo.hitRight <= wallHitInfo.boundMax.x ? agtk::data::ObjectActionLinkConditionData::kWallBitRight : 0);
	wallHitInfo.boundMax.x -= 2;

	wallHitInfo.boundMax.y += 2;
	wallHitInfo.initHit();
	calcWallHitInfo(object, wallHitInfo, move, nullptr, tileList, true, nullptr);
	tileWallBit |= (wallHitInfo.hitUp <= wallHitInfo.boundMax.y ? agtk::data::ObjectActionLinkConditionData::kWallBitUp : 0);
	wallHitInfo.boundMax.y -= 2;
#endif

#ifdef USE_MULTITHREAD_OBJECT_UPDATE
	if (slopeList->size() > 0) {
#else
	if (slopeList->count() > 0) {
#endif
		// 現在の矩形で接触している坂を取得
		newCheckSlopeHit(wallHitInfo, slopeList, object->getSlopeTouchedList());

		// 接触している坂の簡易効果を設定
		updateObjectSlopeEffect(object, object->getSlopeTouchedList());
	}

	return cocos2d::Vec2(moveX, moveY);
}

/**
* 衝突の解決処理
* @param	tryX						チェック時のX座標(未使用)
* @param	tryY						チェック時のY座標(未使用)
* @param	moveX						X軸移動量(出力)
* @param	moveX						Y軸移動量(出力)
* @param	pTileWallBit				タイルとの衝突方向ビット値(出力)
* @param	pLinkConditionTileWall		衝突したタイル(出力)[アクションリンク用]
* @param	pAheadTileWallBit			タイルとの衝突方向ビット値(出力)[1dot先に進んだ場合用]
* @param	pSlopeBit					坂との衝突方向ビット値(出力)
* @param	pLeftWallObjectList			左側壁判定に衝突したオブジェクトリスト(出力)
* @param	pRightWallObjectList		右側壁判定に衝突したオブジェクトリスト(出力)
* @param	pUpWallObjectList			上側壁判定に衝突したオブジェクトリスト(出力)
* @param	pDownWallObjectList			下側壁判定に衝突したオブジェクトリスト(出力)
* @param	buriedInWallFlag			埋まっているフラグ(出力)
* @param	returnedPos					各軸方向で押し戻されたかのフラグ
*/
#ifdef USE_MULTITHREAD_OBJECT_UPDATE
void ObjectCollision::updateWall(LockObjList &lockObjList, float tryX, float tryY, float *moveX, float *moveY, int *pTileWallBit, HitTileWalls* pLinkConditionTileWall, HitTileWalls* pAheadTileWall, int *pSlopeBit, agtk::MtVector<agtk::Object *> *pLeftWallObjectList, agtk::MtVector<agtk::Object *> *pRightWallObjectList, agtk::MtVector<agtk::Object *> *pUpWallObjectList, agtk::MtVector<agtk::Object *> *pDownWallObjectList, bool *buriedInWallFlag, cocos2d::Vec2 &returnedPos)
#else
void ObjectCollision::updateWall(float tryX, float tryY, float *moveX, float *moveY, int *pTileWallBit, HitTileWalls* pLinkConditionTileWall, HitTileWalls* pAheadTileWall, int *pSlopeBit, cocos2d::Array *pLeftWallObjectList, cocos2d::Array *pRightWallObjectList, cocos2d::Array *pUpWallObjectList, cocos2d::Array *pDownWallObjectList, bool *buriedInWallFlag, cocos2d::Vec2 &returnedPos)
#endif
{
	//THREAD_PRINTF("updateWall(0x%x)", &lockObjList);
	auto object = _object;
	*moveX = 0;
	*moveY = 0;
	*pTileWallBit = 0;
	*pSlopeBit = 0;
	*buriedInWallFlag = false;
	auto move = object->getPosition() - object->getOldPosition();

// #AGTK-NX #AGTK-WIN
#ifdef USE_30FPS_3
	auto gm = GameManager::getInstance();
	//auto passIndex = gm->getPassIndex();
	//auto passCount = gm->getPassCount();
	auto bLastPass = gm->getLastPass();
#endif

	std::vector<Vertex4> wallCollisionList;
	auto objectData = object->getObjectData();
	if (this->getObjectList()->count() > 0 || objectData->getGroup() == agtk::data::ObjectData::kObjGroupPlayer || objectData->getCollideWithTileGroupBit() > 0 ) {
// #AGTK-NX #AGTK-WIN
#ifdef USE_30FPS_4
		// 中間フレーム情報対応のタイムライン壁判定取得
		object->getWallTimelineList(wallCollisionList);
#else
		object->getTimelineList(agtk::data::TimelineInfoData::kTimelineWall, wallCollisionList);
#endif
	}

	// 壁当たり判定が設定されていない場合
	if (wallCollisionList.size() == 0) {
		// ------------------------------------------------------------------------
		// ギミックタイルとの重なり判定はオブジェクトの中心点
		//! ※アニメーションが存在しない場合は画像の中心
		cocos2d::Vec2 objPos = Vec2::ZERO;
		cocos2d::Size objSize = Size::ZERO;
		auto player = object->getPlayer();
		if (player) {
			auto objScale = player->getPlayerScale();
			objPos = player->getCenterNodePosition();
			objSize = player->getContentSize();
		}
		else {
			objPos = agtk::Scene::getPositionCocos2dFromScene(object->getCenterPosition());
			objSize = object->getContentSize();
		}

// #AGTK-NX #AGTK-WIN
#if defined(USE_30FPS_1) && !defined(USE_30FPS_3)
		// 30FPSの時、本来60FPSでは行われる当たり判定が省かれるため60FPSに近いタイミングで判定を取るための対応

		// コリジョン判定の通過点の数
		int passCount = (int)GameManager::getInstance()->getFrameProgressScale();
		if (move == Vec2::ZERO) {
			passCount = 1;
		}

		objPos.x -= move.x;
		objPos.y += move.y;

		if (passCount > 1) {
			move = move * 0.5f;
		}

		// 各通過点毎のループ処理
		for (int passIndex = 0; passIndex < passCount; passIndex++)
		{
			objPos.x += move.x;
			objPos.y -= move.y;
#endif

		float x = std::roundf((objPos.x + *moveX) * 10000.0f) * 0.0001f;
		float y = std::roundf((objPos.y + *moveY) * 10000.0f) * 0.0001f;

		cocos2d::Point bminOfCenter = Point(x, y);
		cocos2d::Point bmaxOfCenter = Point(x, y);
		cocos2d::Point bminOfOverlap = Point(x, y);
		cocos2d::Point bmaxOfOverlap = Point(x + objSize.width, y - objSize.height);

		//重なり演出（キャラクターの周辺を透過）
		auto viewSettingData = objectData->getAroundCharacterViewSetting();
		if (viewSettingData && viewSettingData->getViewType() != agtk::data::ObjectAroundCharacterViewSettingData::kDrawSOLID) {
			float halfWidth = viewSettingData->getWidth() * 0.5f;
			float halfHeight = viewSettingData->getHeight() * 0.5f;
			bminOfOverlap = Point(x - halfWidth, y + halfHeight);
			bmaxOfOverlap = Point(x + halfWidth, y - halfHeight);
		}

		// 重なり効果更新
// #AGTK-NX #AGTK-WIN
#ifdef USE_30FPS_3
		if (bLastPass) {
#endif
// #AGTK-NX
#if (CC_TARGET_PLATFORM == CC_PLATFORM_NX)
#else
		updateObjectOverlapMaskEffect(object, getCollisionTileOverlapMaskList(object->getSceneLayer()->getType(), bminOfOverlap, bmaxOfOverlap), agtk::data::TileData::kConditionOverlapped);
#endif
// #AGTK-NX #AGTK-WIN
#ifdef USE_30FPS_3
		} // if (bLastPass)
#endif

		//プレイヤーがタイルに重なった場合の効果を実行
#ifdef USE_MULTITHREAD_OBJECT_UPDATE
		std::vector<agtk::Tile *> tileList = getCollisionTileList(object->getSceneLayer()->getType(), 0, bminOfCenter, bmaxOfCenter, Vec2::ZERO, object);
		updateObjectWallEffect(object, tileList, agtk::data::TileData::kConditionOverlapped);
#else
		updateObjectWallEffect(object, getCollisionTileList(object->getSceneLayer()->getType(), 0, bminOfCenter, bmaxOfCenter, Vec2::ZERO, object), agtk::data::TileData::kConditionOverlapped);
#endif

// #AGTK-NX #AGTK-WIN
#if defined(USE_30FPS_1) && !defined(USE_30FPS_3)
			// 次フレームの攻撃判定等で使用する通過点情報を記録
			if (passIndex != passCount - 1) {
				object->setPassedFrameCount(object->getFrameCount());
				object->setPassedFramePosition(objPos);

				// 消滅した場合は次の判定処理を行わない
				if (object->getDisappearFlag())
					break;
			}
		}
#endif

// #AGTK-NX #AGTK-WIN
#ifdef USE_30FPS_3
		if (gm->getLastPass()) {
#endif
		// 他のオブジェクトとの壁当たり情報を初期化する
		pLeftWallObjectList->removeAllObjects();
		pRightWallObjectList->removeAllObjects();
		pUpWallObjectList->removeAllObjects();
		pDownWallObjectList->removeAllObjects();
		this->reset();
// #AGTK-NX #AGTK-WIN
#ifdef USE_30FPS_3
		} // if (gm->getLastPass()) {}
#endif
		return;
	}

	auto wallGroup = this->getWallHitInfoGroup();
	wallGroup->remove();
	wallGroup->addWallHitInfo(wallCollisionList);
#if 0
	auto wallGroupRect = wallGroup->getRect();
	showDebugRect(cocos2d::Rect(wallGroupRect.origin + wallGroupRect.size * 0.5f, wallGroupRect.size), cocos2d::Color4F(0, 1, 1, 1), 0.2f);
	for (int i = 0; i < wallCollisionList.size(); i++) {
		auto v = wallCollisionList.at(i);
		showDebugPolygon(v.addr(), v.length(), cocos2d::Color4F(1, 1, 0, 1), cocos2d::Color4F(1, 1, 0, 0.3f), 0.2f);
	}
#endif

	cocos2d::Point bmax = wallGroup->getBoundMax();
	cocos2d::Point bmin = wallGroup->getBoundMin();

	bmin.y -= 1;
	bmax.x += 1;
	bmax.y += 1;

	// 衝突しているタイルリストを取得
#ifdef USE_MULTITHREAD_OBJECT_UPDATE
	std::vector<agtk::Tile *> tileList = getCollisionTileList(object->getSceneLayer()->getType(), object->getLayerId(), bmin, bmax, Vec2(move.x, -move.y), object, object->getObjectData()->getCollideWithTileGroupBit());
#else
	cocos2d::__Array *tileList = getCollisionTileList(object->getSceneLayer()->getType(), object->getLayerId(), bmin, bmax, Vec2(move.x, -move.y), object);
#endif
#if 0
	{
		cocos2d::Ref *ref;
		CCARRAY_FOREACH(tileList, ref) {
			auto tile = dynamic_cast<agtk::Tile *>(ref);
			agtk::Vertex4 v;
			tile->convertToLayerSpaceVertex4(v);
			showDebugPolygon(v.addr(), v.length(), cocos2d::Color4F(1, 0, 0, 1), cocos2d::Color4F(1, 0, 0, 0.3f), 0.2f);
		}
	}
#endif

	// 衝突している坂リストを取得
#ifdef USE_MULTITHREAD_OBJECT_UPDATE
	auto slopeList = getCollisionSlopeList(object->getSceneLayer()->getType(), object->getLayerId(), wallGroup->getBoundMin(), wallGroup->getBoundMax(), Vec2(move.x, -move.y));
	AutoDeleter<MtVector<Slope *>> deleter(slopeList);
#else
	cocos2d::__Array *slopeList = getCollisionSlopeList(object->getSceneLayer()->getType(), object->getLayerId(), wallGroup->getBoundMin(), wallGroup->getBoundMax(), Vec2(move.x, -move.y));
#endif
	bool checkSlope = true;

	// 坂当たり判定矩形をリセット
	object->getObjectCollision()->getSlopeCheckRect()->reset();

	// 設定されている壁判定毎に衝突を解決する
	for (unsigned int i = 0; i < wallGroup->getWallHitInfoListCount(); i++) {

		auto wallHitInfo = wallGroup->getWallHitInfo(i);
		
		// 一つ前の衝突判定で確定した移動量を反映する
		wallHitInfo.boundMin.x += *moveX;
		wallHitInfo.boundMin.y += *moveY;
		wallHitInfo.boundMax.x += *moveX;
		wallHitInfo.boundMax.y += *moveY;

		// 衝突の解決を実行
		int tileWallBit = 0;
		int slopeBit = 0;
#ifdef USE_MULTITHREAD_OBJECT_UPDATE
		auto padding = newUpdateWallHitInfo(lockObjList, wallHitInfo, &tileWallBit, pLinkConditionTileWall, pAheadTileWall, &slopeBit, move, pUpWallObjectList, pDownWallObjectList, tileList, slopeList, checkSlope);
#else
		auto padding = newUpdateWallHitInfo(wallHitInfo, &tileWallBit, pLinkConditionTileWall, pAheadTileWall, &slopeBit, move, pUpWallObjectList, pDownWallObjectList, tileList, slopeList, checkSlope);
#endif
		
		// タイルと坂の衝突方向ビット値を更新
		*pTileWallBit |= tileWallBit;
		*pSlopeBit |= slopeBit;
		
		// 確定した移動量を更新
		*moveX += padding.x;
		*moveY += padding.y;

		// 坂のチェックを複数回行うと、行った回数だけ位置がずれていってしまうので
		// フラグを折って処理1回のみにする
		checkSlope = false;
	}

// #AGTK-NX #AGTK-WIN
#ifndef USE_30FPS_3
#ifdef USE_30FPS_1
	// 30FPSの時、本来60FPSでは行われる当たり判定が省かれるため60FPSに近いタイミングで判定を取るための対応

	// コリジョン判定の通過点の数
	int passCount = (int)GameManager::getInstance()->getFrameProgressScale();
	if (move == Vec2::ZERO) {
		passCount = 1;
	}
	if (object->getPassedFrameCount() != object->getFrameCount()) {
		passCount = 1;
	}

	// 各通過点毎のループ処理
	for (int passIndex = 0; passIndex < passCount; passIndex++)	{
#endif
#endif
	// ------------------------------------------------------------------------
	// ギミックタイルとの重なり判定はオブジェクトの中心点
	//! ※アニメーションが存在しない場合は画像の中心
	cocos2d::Vec2 objPos = Vec2::ZERO;
	cocos2d::Size objSize = Size::ZERO;
	auto player = object->getPlayer();
	if (player) {
		auto objScale = player->getPlayerScale();
		objPos = player->getCenterNodePosition();
		objSize = player->getContentSize();
	}
	else {
		objPos = agtk::Scene::getPositionCocos2dFromScene(object->getCenterPosition());
		objSize = object->getContentSize();
	}
// #AGTK-NX #AGTK-WIN
#if defined(USE_30FPS_1) && !defined(USE_30FPS_3)
	float x;
	float y;
	if (passIndex != passCount - 1) {
		// 通過点では位置を補正
		Vec2 diffCenter = objPos - agtk::Scene::getPositionCocos2dFromScene(object->getPosition());
		objPos = diffCenter + agtk::Scene::getPositionCocos2dFromScene(object->getPassedFramePosition());
		x = std::roundf((objPos.x) * 10000.0f) * 0.0001f;
		y = std::roundf((objPos.y) * 10000.0f) * 0.0001f;
	}
	else {
		x = std::roundf((objPos.x + *moveX) * 10000.0f) * 0.0001f;
		y = std::roundf((objPos.y + *moveY) * 10000.0f) * 0.0001f;
	}
#else
	float x = std::roundf((objPos.x + *moveX) * 10000.0f) * 0.0001f;
	float y = std::roundf((objPos.y + *moveY) * 10000.0f) * 0.0001f;
#endif

	cocos2d::Point bminOfCenter = Point(x, y);
	cocos2d::Point bmaxOfCenter = Point(x, y);
	cocos2d::Point bminOfOverlap = Point(x, y);
	cocos2d::Point bmaxOfOverlap = Point(x + objSize.width, y - objSize.height);

	//重なり演出（キャラクターの周辺を透過）
	auto viewSettingData = objectData->getAroundCharacterViewSetting();
	if (viewSettingData && viewSettingData->getViewType() != agtk::data::ObjectAroundCharacterViewSettingData::kDrawSOLID) {
		float halfWidth = viewSettingData->getWidth() * 0.5f;
		float halfHeight = viewSettingData->getHeight() * 0.5f;
		bminOfOverlap = Point(x - halfWidth, y + halfHeight);
		bmaxOfOverlap = Point(x + halfWidth, y - halfHeight);
	}

	// 重なり効果更新
// #AGTK-NX #AGTK-WIN
#ifdef USE_30FPS_3
	if (bLastPass) {
#endif
// #AGTK-NX
#if (CC_TARGET_PLATFORM == CC_PLATFORM_NX)
#else
	updateObjectOverlapMaskEffect(object, getCollisionTileOverlapMaskList(object->getSceneLayer()->getType(), bminOfOverlap, bmaxOfOverlap), agtk::data::TileData::kConditionOverlapped);
#endif
// #AGTK-NX #AGTK-WIN
#ifdef USE_30FPS_3
	} // if (bLastPass)
#endif

	//プレイヤーがタイルに重なった場合の効果を実行
	auto projectData = GameManager::getInstance()->getProjectData();
	auto sceneLayer = object->getSceneLayer();
	auto scene = sceneLayer->getScene();
	auto sceneSize = scene->getSceneSize();
	cocos2d::Point tilePos = Point((int)(x / projectData->getTileWidth()), (int)((sceneSize.y - y) / projectData->getTileHeight()));
#ifdef USE_MULTITHREAD_OBJECT_UPDATE
	std::vector<agtk::Tile *> tileList2 = getCollisionTileList(object->getSceneLayer()->getType(), 0, bminOfCenter, bmaxOfCenter, Vec2::ZERO, object);
	updateObjectWallEffect(object, tileList2, agtk::data::TileData::kConditionOverlapped, &tilePos);
#else
	updateObjectWallEffect(object, getCollisionTileList(object->getSceneLayer()->getType(), 0, bminOfCenter, bmaxOfCenter, Vec2::ZERO, object), agtk::data::TileData::kConditionOverlapped, &tilePos);
#endif

// #AGTK-NX #AGTK-WIN
#if defined(USE_30FPS_1) && !defined(USE_30FPS_3)
		// 次フレームの攻撃判定等で使用する通過点情報を記録
		if (passIndex != passCount - 1) {
			// 消滅した場合は次の判定処理を行わない
			if (object->getDisappearFlag())
				break;
		}
	}
#endif

	// ------------------------------------------------------------------------

	// ------------------------------------------------------------------------
// #AGTK-NX #AGTK-WIN
#ifdef USE_30FPS_3
	if(gm->getLastPass()) {
#endif
	// 他オブジェクトとの衝突情報を再設定
#ifdef USE_MULTITHREAD_OBJECT_UPDATE
	if (_object->getLeftWallObjectList() == pLeftWallObjectList) {
		_object->autoReleaseRetainWallObjectList(this->getLeftWallObjectList(), this->getRightWallObjectList(), this->getUpWallObjectList(), this->getDownWallObjectList());
	}
	else {
		pLeftWallObjectList->removeAllObjects();
		pRightWallObjectList->removeAllObjects();
		pUpWallObjectList->removeAllObjects();
		pDownWallObjectList->removeAllObjects();

		pLeftWallObjectList->addObjectsFromArray(this->getLeftWallObjectList());
		pRightWallObjectList->addObjectsFromArray(this->getRightWallObjectList());
		pUpWallObjectList->addObjectsFromArray(this->getUpWallObjectList());
		pDownWallObjectList->addObjectsFromArray(this->getDownWallObjectList());
	}
#else
	pLeftWallObjectList->removeAllObjects();
	pRightWallObjectList->removeAllObjects();
	pUpWallObjectList->removeAllObjects();
	pDownWallObjectList->removeAllObjects();

	pLeftWallObjectList->addObjectsFromArray(this->getLeftWallObjectList());
	pRightWallObjectList->addObjectsFromArray(this->getRightWallObjectList());
	pUpWallObjectList->addObjectsFromArray(this->getUpWallObjectList());
	pDownWallObjectList->addObjectsFromArray(this->getDownWallObjectList());
#endif
// #AGTK-NX #AGTK-WIN
#ifdef USE_30FPS_3
	}
#endif
	// ------------------------------------------------------------------------

	// 埋まっているフラグと押し戻されたフラグを設定
	*buriedInWallFlag = this->getBuriedInWallFlag();
	returnedPos = this->getReturnedPos();

// #AGTK-NX #AGTK-WIN
#ifdef USE_30FPS_3
	if (gm->getLastPass()) {
#endif
	this->reset();
// #AGTK-NX #AGTK-WIN
#ifdef USE_30FPS_3
	}
#endif
}

void ObjectCollision::updateWallHitInfoGroup()
{
	std::vector<Vertex4> wallCollisionList;
	_object->getTimelineList(agtk::data::TimelineInfoData::kTimelineWall, wallCollisionList);
	auto infoGroup = this->getWallHitInfoGroup();
	infoGroup->remove();
	infoGroup->addWallHitInfo(wallCollisionList);
}

void ObjectCollision::initWallHitInfoGroup()
{
	std::vector<Vertex4> wallCollisionList;
#ifdef USE_COLLISION_OPTIMIZATION
	auto objectData = _object->getObjectData();
	if (objectData->getCollideWithObjectGroupBit() || objectData->getCollideWithTileGroupBit()) {
		_object->getTimelineList(agtk::data::TimelineInfoData::kTimelineWall, wallCollisionList);
	}
#else
	_object->getTimelineList(agtk::data::TimelineInfoData::kTimelineWall, wallCollisionList);
#endif
	auto infoGroup = this->getWallHitInfoGroup();
	infoGroup->remove();
	infoGroup->addWallHitInfo(wallCollisionList);
	auto oldInfoGroup = this->getOldWallHitInfoGroup();
	oldInfoGroup->remove();
	oldInfoGroup->addWallHitInfo(wallCollisionList);
	auto prevInfoGroup = this->getPrevWallHitInfoGroup();
	prevInfoGroup->remove();
	prevInfoGroup->addWallHitInfo(wallCollisionList);
}

void ObjectCollision::lateUpdateWallHitInfoGroup()
{
	std::vector<Vertex4> wallCollisionList;
	auto objectData = _object->getObjectData();
	if (objectData->getCollideWithObjectGroupBit() || objectData->getCollideWithTileGroupBit()) {
		_object->getTimelineList(agtk::data::TimelineInfoData::kTimelineWall, wallCollisionList);
	}

	auto infoGroup = this->getWallHitInfoGroup();
	auto prevInfoGroup = this->getPrevWallHitInfoGroup();
	prevInfoGroup->remove();
	prevInfoGroup->addWallHitInfo(infoGroup->getWallHitInfoList());
	infoGroup->remove();
	infoGroup->addWallHitInfo(wallCollisionList);
	auto oldInfoGroup = this->getOldWallHitInfoGroup();
	oldInfoGroup->remove();
	oldInfoGroup->addWallHitInfo(wallCollisionList);
}

// #AGTK-NX #AGTK-WIN
#ifdef USE_30FPS_3
void ObjectCollision::updateMiddleFrameWallHitInfoGroup()
{
	std::vector<Vertex4> wallCollisionList;
	auto objectData = _object->getObjectData();
	if (objectData->getCollideWithObjectGroupBit() || objectData->getCollideWithTileGroupBit()) {
#ifdef USE_30FPS_4
		// 中間フレーム情報対応のタイムライン壁判定取得
		_object->getWallTimelineList(wallCollisionList);
#else
		_object->getTimelineList(agtk::data::TimelineInfoData::kTimelineWall, wallCollisionList);
#endif
	}

	auto infoGroup = this->getWallHitInfoGroup();
	auto prevInfoGroup = this->getPrevWallHitInfoGroup();
	prevInfoGroup->remove();
	prevInfoGroup->addWallHitInfo(infoGroup->getWallHitInfoList());
	infoGroup->remove();
	infoGroup->addWallHitInfo(wallCollisionList);
	auto oldInfoGroup = this->getOldWallHitInfoGroup();
	oldInfoGroup->remove();
	oldInfoGroup->addWallHitInfo(wallCollisionList);
}
#endif

#ifdef USE_MULTITHREAD_OBJECT_UPDATE
void ObjectCollision::pushObject(LockObjList &lockObjList, agtk::WallHitInfoGroup *infoGroup, cocos2d::Vec2& pushVec, cocos2d::Rect& pushRect, agtk::Object* pushObject, bool bPushedMove)
#else
void ObjectCollision::pushObject(cocos2d::Vec2& pushVec, cocos2d::Rect& pushRect, agtk::Object* pushObject)
#endif
{
#ifdef USE_MULTITHREAD_OBJECT_UPDATE
	//THREAD_PRINTF("pushObject(0x%x)", &lockObjList);
#endif
	auto object = _object;
	auto objectData = object->getObjectData();
	bool buriedInWallFlag = false;

	// 自オブジェクトの壁当たり判定を取得
#ifdef USE_MULTITHREAD_OBJECT_UPDATE
#else
	updateWallHitInfoGroup();
	auto infoGroup = this->getWallHitInfoGroup();
#endif
#ifdef USE_MULTITHREAD_OBJECT_UPDATE
	auto wallHitInfoListCount = infoGroup->getWallHitInfoListCount();
	//THREAD_PRINTF("pushObject(0x%x): %d, %d", &lockObjList, __LINE__, wallHitInfoListCount);
#endif

	// 押し出し方向を取得
	Vec2 moveVec = getPushObjectVec(pushVec, pushRect, infoGroup, buriedInWallFlag, bPushedMove);

	WallHitInfo hitInfo;

	// slope
#ifdef USE_MULTITHREAD_OBJECT_UPDATE
	auto slopeList = getCollisionSlopeList(object->getSceneLayer()->getType(), object->getLayerId(), infoGroup->getBoundMin() + moveVec, infoGroup->getBoundMax() + moveVec, moveVec);
	AutoDeleter<MtVector<Slope *>> deleter(slopeList);
#else
	cocos2d::Array* slopeList = getCollisionSlopeList(object->getSceneLayer()->getType(), object->getLayerId(), infoGroup->getBoundMin() + moveVec, infoGroup->getBoundMax() + moveVec, moveVec);
#endif

	// 坂との接触がある場合
#ifdef USE_MULTITHREAD_OBJECT_UPDATE
	if (slopeList->size() > 0) {
#else
	if (slopeList->count() > 0) {
#endif

		for (unsigned int i = 0; i < infoGroup->getWallHitInfoListCount(); i++) {
			auto wallInfo = infoGroup->getWallHitInfo(i);
			hitInfo.boundMin = wallInfo.boundMin + moveVec;
			hitInfo.boundMax = wallInfo.boundMax + moveVec;
			bool hitUp, hitDown;
			bool isSlip = object->getPlayObjectData()->getSwitchData(agtk::data::kObjectSystemSwitchSlipOnSlope)->getValue();

			Rect prevObjRect = Rect(wallInfo.boundMin, Size(wallInfo.boundMax - wallInfo.boundMin));
			Rect crntObjRect = prevObjRect;
			crntObjRect.origin += moveVec;

			Vec2 vec = crntObjRect.origin - prevObjRect.origin;

// #AGTK-NX #AGTK-WIN
#ifndef USE_SAR_PROVISIONAL_4
#if CC_TARGET_PLATFORM != CC_PLATFORM_NX
			if (newCheckSlopeHit(hitInfo, object->getObjectCollision()->getOldWallHitInfoGroup()->getRect(), false, hitUp, hitDown, slopeList, object->getPassableSlopeTouchedList(), moveVec, isSlip, object->getSlopeTouchedFrame())) {
#else
#endif
#else
			auto oldRect = object->getObjectCollision()->getOldWallHitInfoGroup()->getRect();
			if (object->getObjectCollision()->getOldWallHitInfoGroup()->getWallHitInfoListCount() == 0) {
				oldRect = infoGroup->getRect();
			}
			if (newCheckSlopeHit(hitInfo, oldRect, false, hitUp, hitDown, slopeList, object->getPassableSlopeTouchedList(), moveVec, isSlip, object->getSlopeTouchedFrame())) {
#endif
				crntObjRect.origin += moveVec;
				moveVec = crntObjRect.origin - prevObjRect.origin;
			}
		}
	}
	
	// tile
	bool checkTile = true;
	//壁判定の設定（タイルとぶつかる）
	if (object->getObjectData()->getCollideWithTileGroupBit() == 0 ) {
		checkTile = false;
	}

	//テンプレート移動で「タイル判定を無視する」の場合
	if (object->getObjectTemplateMove()->isIgnoredTileWall()) {
		checkTile = false;
	}

	// タイルとの当たりをチェック
	if (checkTile) {
		// 押し出しを考慮したタイルリストを取得
#ifdef USE_MULTITHREAD_OBJECT_UPDATE
		std::vector<agtk::Tile *> tileList = getCollisionTileList(object->getSceneLayer()->getType(), object->getLayerId(), infoGroup->getBoundMin() + moveVec, infoGroup->getBoundMax() + moveVec, moveVec, object);
#else
		cocos2d::Array* tileList = getCollisionTileList(object->getSceneLayer()->getType(), object->getLayerId(), infoGroup->getBoundMin() + moveVec, infoGroup->getBoundMax() + moveVec, moveVec, object);
#endif

		for (unsigned int i = 0; i < infoGroup->getWallHitInfoListCount(); i++) {

			bool hitX, hitY;
			agtk::HitWallTiles hitTiles;

			auto wallInfo = infoGroup->getWallHitInfo(i);
			hitInfo = wallInfo;
			getWallCenterAndBound(wallInfo.boundMin + moveVec, wallInfo.size, &hitInfo.center, &hitInfo.boundMin, &hitInfo.boundMax);
// #AGTK-NX #AGTK-WIN
#ifndef USE_SAR_PROVISIONAL_4
#if CC_TARGET_PLATFORM != CC_PLATFORM_NX
			newCheckWallHit(hitInfo, hitX, hitY, 0, tileList, slopeList, object->getPassableSlopeTouchedList(), moveVec, Vec2(0, 0), object->getObjectCollision()->getOldWallHitInfoGroup()->getRect(), false, false, hitTiles);
#else
#endif
#else
			auto oldRect = object->getObjectCollision()->getOldWallHitInfoGroup()->getRect();
			if (object->getObjectCollision()->getOldWallHitInfoGroup()->getWallHitInfoListCount() == 0) {
				oldRect = infoGroup->getRect();
			}
			auto tileThresholdMoveVec = Vec2(0, 0);
			newCheckWallHit(hitInfo, hitX, hitY, 0, tileList, slopeList, object->getPassableSlopeTouchedList(), moveVec, tileThresholdMoveVec, oldRect, false, false, hitTiles);
#endif

			if (hitX) {
				moveVec.x = hitInfo.boundMin.x - wallInfo.boundMin.x;
			}

			if (hitY) {
				moveVec.y = hitInfo.boundMin.y - wallInfo.boundMin.y;
			}
		}
	}

	if (moveVec.x == 0 && moveVec.y == 0) {
		return;
	}

	auto scene = GameManager::getInstance()->getCurrentScene();
	auto sceneLayer = scene->getSceneLayer(object->getLayerId());

#ifdef USE_MULTITHREAD_OBJECT_UPDATE
	auto skipObjList = new MtVector<Object *>();
	AutoDeleter<MtVector<Object *>> deleter2(skipObjList);
#else
	cocos2d::Array* skipObjList = cocos2d::Array::create();
#endif

	std::function<void()> resetSkipObjList = [&]() {
		skipObjList->removeAllObjects();
		skipObjList->addObject(pushObject);
		skipObjList->addObject(object);
	};

	// オブジェクトとの当たりをチェック
	cocos2d::Ref *ref = nullptr;
	for (unsigned int i = 0; i < infoGroup->getWallHitInfoListCount(); i++) {
		//THREAD_PRINTF("%s: %s: %d: %d, %d", __FILE__, __FUNCTION__, __LINE__, i, infoGroup->getWallHitInfoListCount());
		CCARRAY_FOREACH(sceneLayer->getObjectList(), ref) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
			auto object2 = static_cast<agtk::Object*>(ref);
#else
			auto object2 = dynamic_cast<agtk::Object*>(ref);
#endif
			//THREAD_PRINTF("%s: %s: %d: %d, %s, %d", __FILE__, __FUNCTION__, __LINE__, i, object2->getObjectData()->getName(), infoGroup->getWallHitInfoListCount());

			// object2 が Disabled の場合
			if (object2->getDisabled()) {
				continue;
			}

			auto objectData2 = object2->getObjectData();

			resetSkipObjList();

			// スキップリストに入っているオブジェクトはチェックしない
			if (skipObjList->containsObject(object2)) {
				continue;
			}

			// レイヤーが違うオブジェクトの場合
			if (object->getLayerId() != object2->getLayerId()) {
				continue;
			}

			if (!(objectData->isCollideWith(objectData2))) {
				continue;
			}

			//THREAD_PRINTF("%s: %s: %d: %d, %d", __FILE__, __FUNCTION__, __LINE__, i, infoGroup->getWallHitInfoListCount());
			auto wallInfo = infoGroup->getWallHitInfo(i);

			// オブジェクトの壁当たり判定を取得
#ifdef USE_MULTITHREAD_OBJECT_UPDATE
			std::vector<Vertex4> wallCollisionList;
// #AGTK-NX #AGTK-WIN
#ifdef USE_30FPS_4
			// 中間フレーム情報対応のタイムライン壁判定取得
			object2->getWallTimelineList(wallCollisionList);
#else
			object2->getTimelineList(agtk::data::TimelineInfoData::kTimelineWall, wallCollisionList);
#endif
			auto infoGroup2 = agtk::WallHitInfoGroup::create(object2);
			AutoDeleter<agtk::WallHitInfoGroup> deleter(infoGroup2);
			infoGroup2->addWallHitInfo(wallCollisionList);
#else
			object2->getObjectCollision()->updateWallHitInfoGroup();
			auto infoGroup2 = object2->getObjectCollision()->getWallHitInfoGroup();
#endif
			Vec2 obj2Pos = object2->getPosition();

			for (unsigned int j = 0; j < infoGroup2->getWallHitInfoListCount(); j++) {
				//THREAD_PRINTF("%s: %s: %d: %d, %d, %d", __FILE__, __FUNCTION__, __LINE__, i, j, infoGroup->getWallHitInfoListCount());

				resetSkipObjList();

				auto wallInfo2 = infoGroup2->getWallHitInfo(j);

				Rect obj2Rect = Rect(wallInfo2.boundMin, Size(wallInfo2.boundMax - wallInfo2.boundMin));

				Rect prevObjRect = Rect(wallInfo.boundMin, Size(wallInfo.boundMax - wallInfo.boundMin));
				Rect crntObjRect = prevObjRect;

				// Xの押し出しチェック
				crntObjRect.origin.x += moveVec.x;

				Rect mergeRect = prevObjRect;
				mergeRect.merge(crntObjRect);

				// X方向への矩形の押し出しを行う
#ifdef USE_MULTITHREAD_OBJECT_UPDATE
				moveVec.x += checkPushRect(lockObjList, moveVec.x, true, object2->getPushedbackByObject(), crntObjRect, obj2Rect, mergeRect, object2, skipObjList);
#else
				moveVec.x += checkPushRect(moveVec.x, true, object2->getPushedbackByObject(), crntObjRect, obj2Rect, mergeRect, object2, skipObjList);
#endif

				// object2の押し出しが発生しているので、矩形を再設定
				Vec2 obj2MoveVec = object2->getPosition() - obj2Pos;
				obj2MoveVec.y *= -1;
				obj2Rect.origin += obj2MoveVec;

				resetSkipObjList();

				// Yの押し出しチェック
				crntObjRect = prevObjRect;
				crntObjRect.origin += moveVec;

				mergeRect = prevObjRect;
				mergeRect.merge(crntObjRect);

				// Y方向への矩形の押し出しを行う
#ifdef USE_MULTITHREAD_OBJECT_UPDATE
				moveVec.y += checkPushRect(lockObjList, moveVec.y, false, object2->getPushedbackByObject(), crntObjRect, obj2Rect, mergeRect, object2, skipObjList);
#else
				moveVec.y += checkPushRect(moveVec.y, false, object2->getPushedbackByObject(), crntObjRect, obj2Rect, mergeRect, object2, skipObjList);
#endif
			}
			//THREAD_PRINTF("%s: %s: %d: %d, %d", __FILE__, __FUNCTION__, __LINE__, i, infoGroup->getWallHitInfoListCount());
		}
	}

	if (moveVec != cocos2d::Vec2::ZERO) {
		if (buriedInWallFlag) {
			object->setBuriedInWallFlag(true, 3);
		}
		object->addPosition(moveVec);
		object->setOldPosition(object->getPosition());
		object->setOldPosition2(object->getPosition());//強制移動した事で再設定。
	}
}

#ifdef USE_MULTITHREAD_OBJECT_UPDATE
void ObjectCollision::pushRect(LockObjList &lockObjList, cocos2d::Vec2& pushVec, cocos2d::Rect& rect, MtVector<Object *> *skipObjList)
#else
void ObjectCollision::pushRect(cocos2d::Vec2& pushVec, cocos2d::Rect& rect, cocos2d::Array* skipObjList)
#endif
{
	//THREAD_PRINTF("pushRect(0x%x)", &lockObjList);
	CC_ASSERT(_object->getObjectCollision() == this);	//仮説
	auto object = _object;
	auto objectData = object->getObjectData();

	skipObjList->addObject(object);

	// slope
#ifdef USE_MULTITHREAD_OBJECT_UPDATE
	auto slopeList = getCollisionSlopeList(object->getSceneLayer()->getType(), object->getLayerId(), Point(rect.getMinX(), rect.getMinY()), Point(rect.getMaxX(), rect.getMaxY()), pushVec);
	AutoDeleter<MtVector<Slope *>> deleter(slopeList);
#else
	cocos2d::Array* slopeList = getCollisionSlopeList(object->getSceneLayer()->getType(), object->getLayerId(), Point(rect.getMinX(), rect.getMinY()), Point(rect.getMaxX(), rect.getMaxY()), pushVec);
#endif

	// 坂
	if (slopeList->count() > 0) {

		WallHitInfo hitInfo;
		Rect crntRect = rect;
		crntRect.origin += pushVec;

		hitInfo.boundMin = crntRect.origin;
		hitInfo.boundMax = crntRect.origin + crntRect.size;

		bool hitUp, hitDown;
		bool isSlip = object->getPlayObjectData()->getSwitchData(agtk::data::kObjectSystemSwitchSlipOnSlope)->getValue();

// #AGTK-NX #AGTK-WIN
#ifndef USE_SAR_PROVISIONAL_4
#if CC_TARGET_PLATFORM != CC_PLATFORM_NX
		if (newCheckSlopeHit(hitInfo, object->getObjectCollision()->getOldWallHitInfoGroup()->getRect(), false, hitUp, hitDown, slopeList, object->getPassableSlopeTouchedList(), pushVec, isSlip, object->getSlopeTouchedFrame())) {
#else
#endif
#else
		auto oldRect = object->getObjectCollision()->getOldWallHitInfoGroup()->getRect();
		if (object->getObjectCollision()->getOldWallHitInfoGroup()->getWallHitInfoListCount() == 0) {
			oldRect = rect;
		}
		if (newCheckSlopeHit(hitInfo, oldRect, false, hitUp, hitDown, slopeList, object->getPassableSlopeTouchedList(), pushVec, isSlip, object->getSlopeTouchedFrame())) {
#endif

			crntRect.origin += pushVec;
			pushVec = crntRect.origin - rect.origin;
		}
	}

	bool checkTile = true;
	//壁判定の設定（タイルとぶつかる）
	if (object->getObjectData()->getCollideWithTileGroupBit() == 0) {
		checkTile = false;
	}

	//テンプレート移動で「タイル判定を無視する」の場合
	if (object->getObjectTemplateMove()->isIgnoredTileWall()) {
		checkTile = false;
	}

	// タイルとの当たりをチェック
	if (checkTile) {

		Rect crntRect = rect;
		crntRect.origin += pushVec;

		// 押し出しを考慮したタイルリストを取得
#ifdef USE_MULTITHREAD_OBJECT_UPDATE
		std::vector<agtk::Tile *> tileList = getCollisionTileList(object->getSceneLayer()->getType(), object->getLayerId(), Point(crntRect.getMinX(), crntRect.getMinY()), Point(crntRect.getMaxX(), crntRect.getMaxY()), pushVec, object);
#else
		cocos2d::Array* tileList = getCollisionTileList(object->getSceneLayer()->getType(), object->getLayerId(), Point(crntRect.getMinX(), crntRect.getMinY()), Point(crntRect.getMaxX(), crntRect.getMaxY()), pushVec, object);
#endif

		bool isHitX, isHitY;
		agtk::HitWallTiles hitTiles;
// #AGTK-NX #AGTK-WIN
#ifndef USE_SAR_PROVISIONAL_4
#if CC_TARGET_PLATFORM != CC_PLATFORM_NX
		newCheckWallHit(crntRect, crntRect, isHitX, isHitY, 0, tileList, slopeList, object->getPassableSlopeTouchedList(), pushVec, Vec2(0, 0), object->getObjectCollision()->getOldWallHitInfoGroup()->getRect(), object, false, false, hitTiles);
#else
#endif
#else
		auto oldRect = object->getObjectCollision()->getOldWallHitInfoGroup()->getRect();
		if (object->getObjectCollision()->getOldWallHitInfoGroup()->getWallHitInfoListCount() == 0) {
			oldRect = rect;
		}
		auto tileThresholdMoveVec = Vec2(0, 0);
		newCheckWallHit(crntRect, crntRect, isHitX, isHitY, 0, tileList, slopeList, object->getPassableSlopeTouchedList(), pushVec, tileThresholdMoveVec, oldRect, object, false, false, hitTiles);
#endif

		if (isHitX) {
			pushVec.x = crntRect.origin.x - rect.origin.x;
			rect.origin.x += pushVec.x;
		}

		if (isHitY) {
			pushVec.y = crntRect.origin.y - rect.origin.y;
			rect.origin.y += pushVec.y;
		}
	}

	auto scene = GameManager::getInstance()->getCurrentScene();
	auto sceneLayer = scene->getSceneLayer(object->getLayerId());

	// オブジェクトとの当たりをチェック
	cocos2d::Ref *ref = nullptr;
	CCARRAY_FOREACH(sceneLayer->getObjectList(), ref) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
		auto object2 = static_cast<agtk::Object*>(ref);
#else
		auto object2 = dynamic_cast<agtk::Object*>(ref);
#endif

		// object2 が Disabled の場合
		if (object2->getDisabled()) {
			continue;
		}

		auto objectData2 = object2->getObjectData();

		// スキップリストに入っているオブジェクトはチェックしない
		if (skipObjList->containsObject(object2)) {
			continue;
		}

		// レイヤーが違うオブジェクトの場合
		if (object->getLayerId() != object2->getLayerId()) {
			continue;
		}

		if (!(objectData->isCollideWith(objectData2))){
			continue;
		}

#ifdef USE_MULTITHREAD_OBJECT_UPDATE
		LockObjList::AutoLocker locker(&lockObjList, object2);
#endif
		// オブジェクトの壁当たり判定を取得
#ifdef USE_MULTITHREAD_OBJECT_UPDATE
		std::vector<Vertex4> wallCollisionList;
// #AGTK-NX #AGTK-WIN
#ifdef USE_30FPS_4
		// 中間フレーム情報対応のタイムライン壁判定取得
		object2->getWallTimelineList(wallCollisionList);
#else
		object2->getTimelineList(agtk::data::TimelineInfoData::kTimelineWall, wallCollisionList);
#endif
		auto infoGroup2 = agtk::WallHitInfoGroup::create(object2);
		AutoDeleter<agtk::WallHitInfoGroup> deleter(infoGroup2);
		infoGroup2->addWallHitInfo(wallCollisionList);
#else
		object2->getObjectCollision()->updateWallHitInfoGroup();
		auto infoGroup2 = object2->getObjectCollision()->getWallHitInfoGroup();
#endif
		Vec2 obj2Pos = object2->getPosition();

		for (unsigned int j = 0; j < infoGroup2->getWallHitInfoListCount(); j++) {

			auto wallInfo2 = infoGroup2->getWallHitInfo(j);

			Rect obj2Rect = Rect(wallInfo2.boundMin, Size(wallInfo2.boundMax - wallInfo2.boundMin));

			Rect prevObjRect = rect;
			Rect crntObjRect = prevObjRect;

			// Xの押し出しチェック
			crntObjRect.origin.x += pushVec.x;

			Rect mergeRect = prevObjRect;
			mergeRect.merge(crntObjRect);

			// X方向への矩形の押し出しを行う
#ifdef USE_MULTITHREAD_OBJECT_UPDATE
			pushVec.x += checkPushRect(lockObjList, pushVec.x, true, object2->getPushedbackByObject(), crntObjRect, obj2Rect, mergeRect, object2, skipObjList);
#else
			pushVec.x += checkPushRect(pushVec.x, true, object2->getPushedbackByObject(), crntObjRect, obj2Rect, mergeRect, object2, skipObjList);
#endif

			// Yの押し出しチェック
			crntObjRect = prevObjRect;
			crntObjRect.origin += pushVec;

			mergeRect = prevObjRect;
			mergeRect.merge(crntObjRect);

			// object2の押し出しが発生しているので、矩形を再設定
			Vec2 obj2MoveVec = object2->getPosition() - obj2Pos;
			obj2MoveVec.y *= -1;
			obj2Rect.origin += obj2MoveVec;

			// Y方向への矩形の押し出しを行う
#ifdef USE_MULTITHREAD_OBJECT_UPDATE
			pushVec.y += checkPushRect(lockObjList, pushVec.y, false, objectData2->getPushedbackByObject(), crntObjRect, obj2Rect, mergeRect, object2, skipObjList);
#else
			pushVec.y += checkPushRect(pushVec.y, false, objectData2->getPushedbackByObject(), crntObjRect, obj2Rect, mergeRect, object2, skipObjList);
#endif
		}
	}

	if (pushVec != cocos2d::Vec2::ZERO) {
		std::function<bool(cocos2d::Vec2, agtk::WallHitInfoGroup *)> checkBuriedInWall = [&](cocos2d::Vec2 vec, agtk::WallHitInfoGroup *infoGroup) {
			for (unsigned int i = 0; i < infoGroup->getWallHitInfoListCount(); i++) {
				auto wallInfo = infoGroup->getWallHitInfo(i);
				auto size = cocos2d::Size(wallInfo.boundMax - wallInfo.boundMin);
				if (abs(vec.x) >= size.width * 0.5f || abs(vec.y) >= size.height * 0.5f) {
					return true;
				}
			}
			return false;
		};
#ifdef USE_MULTITHREAD_OBJECT_UPDATE
		std::vector<Vertex4> wallCollisionList;
// #AGTK-NX #AGTK-WIN
#ifdef USE_30FPS_4
		// 中間フレーム情報対応のタイムライン壁判定取得
		_object->getWallTimelineList(wallCollisionList);
#else
		_object->getTimelineList(agtk::data::TimelineInfoData::kTimelineWall, wallCollisionList);
#endif
		auto wallHitInfoGroup = agtk::WallHitInfoGroup::create(_object);
		AutoDeleter<agtk::WallHitInfoGroup> deleter(wallHitInfoGroup);
		wallHitInfoGroup->addWallHitInfo(wallCollisionList);
#else
		updateWallHitInfoGroup();
		auto wallHitInfoGroup = this->getWallHitInfoGroup();
#endif
		bool buriedInWallFlag = checkBuriedInWall(pushVec, wallHitInfoGroup);
		if (buriedInWallFlag) {
			object->setBuriedInWallFlag(true, 3);
		}
		object->addPosition(pushVec);
		object->setOldPosition(object->getPosition());
		object->setOldPosition2(object->getPosition());//強制移動した事で再設定。
	}
}

static bool intersectsRectWithMargin(const Rect &rect1, const Rect& rect2, float margin)
{
	return !(rect1.getMaxX() < rect2.getMinX() + margin ||
		rect2.getMaxX() <      rect1.getMinX() + margin ||
		rect1.getMaxY() <      rect2.getMinY() + margin ||
		rect2.getMaxY() <      rect1.getMinY() + margin);
}

#ifdef USE_MULTITHREAD_OBJECT_UPDATE
float ObjectCollision::checkPushRect(LockObjList &lockObjList, float move, bool checkX, bool canPushed, cocos2d::Rect& objectRect1, cocos2d::Rect& objectRect2, cocos2d::Rect& checkRect, agtk::Object* pushObject, MtVector<Object *> *skipObjList)
#else
float ObjectCollision::checkPushRect(float move, bool checkX, bool canPushed, cocos2d::Rect& objectRect1, cocos2d::Rect& objectRect2, cocos2d::Rect& checkRect, agtk::Object* pushObject, cocos2d::Array* skipObjList)
#endif
{
	//THREAD_PRINTF("checkPushRect(0x%x)", &lockObjList);
	float val = 0;

	//if (objectRect2.intersectsRect(checkRect) && move != 0) {
	if (intersectsRectWithMargin(objectRect2, checkRect, 0.0001f) && move != 0) {	// ACT2-4650 誤差により衝突が誤検知されることがあるため、マージンを設定。
		if (checkX) {
			//上もしくは下でオブジェクトが乗っている場合のチェック。
			auto tmpRect = checkRect;
			tmpRect.origin += Vec2(0, TILE_COLLISION_THRESHOLD);
			tmpRect.size.height -= TILE_COLLISION_THRESHOLD * 2;
			if (objectRect2.intersectsRect(tmpRect) == false) {
				//押し戻し無し。
				return val;
			}
		}
		if (move > 0) {
			if (checkX) {
				float w1 = std::abs(objectRect1.getMaxX() - objectRect2.getMinX());
				float w2 = std::abs(objectRect1.getMinX() - objectRect2.getMaxX());
				if (w1 > w2) {
					val = objectRect1.getMinX() - objectRect2.getMaxX() - TILE_COLLISION_THRESHOLD;
				}
				else {
					val = objectRect1.getMaxX() - objectRect2.getMinX() + TILE_COLLISION_THRESHOLD;
				}
			}
			else {
				float h1 = std::abs(objectRect1.getMaxY() - objectRect2.getMinY());
				float h2 = std::abs(objectRect1.getMinY() - objectRect2.getMaxY());
				if (h1 <= h2) {
					val = objectRect1.getMaxY() - objectRect2.getMinY() + TILE_COLLISION_THRESHOLD;
				}
				else if (h1 > h2) {
					val = objectRect1.getMinY() - objectRect2.getMaxY() - TILE_COLLISION_THRESHOLD;
				}
			}
		}
		else if (move < 0) {
			if (checkX) {
				float w1 = std::abs(objectRect1.getMaxX() - objectRect2.getMinX());
				float w2 = std::abs(objectRect1.getMinX() - objectRect2.getMaxX());
				if (w1 > w2) {
					val = objectRect1.getMinX() - objectRect2.getMaxX() - TILE_COLLISION_THRESHOLD;
				}
				else {
					val = objectRect1.getMaxX() - objectRect2.getMinX() + TILE_COLLISION_THRESHOLD;
				}
			}
			else {
				//ACT2-4062 オブジェクト同士、上に重なると１ドット程度浮くのを修正。閾値（TILE_COLLISION_THRESHOLD）分を計算から外す。
				////val = objectRect1.getMinY() - objectRect2.getMaxY() - TILE_COLLISION_THRESHOLD;
				//val = objectRect1.getMinY() - objectRect2.getMaxY();

				float h1 = std::abs(objectRect1.getMaxY() - objectRect2.getMinY());
				float h2 = std::abs(objectRect1.getMinY() - objectRect2.getMaxY());
				if (h1 < h2) {
					val = objectRect1.getMaxY() - objectRect2.getMinY();// +TILE_COLLISION_THRESHOLD;
				}
				else if (h1 >= h2) {
					val = objectRect1.getMinY() - objectRect2.getMaxY();// -TILE_COLLISION_THRESHOLD;
				}
			}
		}

		// 押し戻せないオブジェクトの場合
		if (!canPushed) {

			// 押し出されるオブジェクトを小さくする
			auto tmpRect = checkRect;
			tmpRect.origin += Vec2(TILE_COLLISION_THRESHOLD, TILE_COLLISION_THRESHOLD);
			tmpRect.size.width -= TILE_COLLISION_THRESHOLD;
			tmpRect.size.height -= TILE_COLLISION_THRESHOLD;

			// 少し小さくした押し出されるオブジェクトと押し出そうとするオブジェクトが衝突する場合
			if (objectRect2.intersectsRect(tmpRect)) {
				val *= -1;
			}
			else {
				// ピッタリくっついているか上に乗っているかの状態と判断して押し出しを行わない
				val = 0;
			}
		}
		// 押し戻せるオブジェクトの場合
		else {
			// 押し出す量を設定し、object2を押し出してみる
			Vec2 vec = checkX ? Vec2(val, 0) : Vec2(0, val);
#ifdef USE_MULTITHREAD_OBJECT_UPDATE
			pushObject->getObjectCollision()->pushRect(lockObjList, vec, objectRect2, skipObjList);
#else
			pushObject->getObjectCollision()->pushRect(vec, objectRect2, skipObjList);
#endif

			if (checkX) {
				// 左移動時
				if (move < 0) {
					// 予定より大きくobject2を押し戻せた場合
					if (vec.x < move) {
						// 押し戻し不要なので0を設定
						val = 0;
					}
					else {
						val = vec.x - val;
					}
				}
				// 右移動時
				else if (0 < move) {
					// 予定より大きくobject2を押し戻せた場合
					if (move < vec.x) {
						// 押し戻し不要なので0を設定
						val = 0;
					}
					else {
						val = vec.x - val;
					}
				}
				// 停止時
				else {
					val = vec.x - val;
				}
			}
			else {
				// 下移動時
				if (move < 0) {
					// 予定より大きくobject2を押し戻せた場合
					if (vec.y < move) {
						// 押し戻し不要なので0を設定
						val = 0;
					}
					else {
						val = vec.y - val;
					}
				}
				// 上移動時
				else if (0 < move) {
					// 予定より大きくobject2を押し戻せた場合
					if (move < vec.y) {
						// 押し戻し不要なので0を設定
						val = 0;
					}
					else {
						val = vec.y - val;
					}
				}
				// 停止時
				else {
					val = vec.y - val;
				}
			}
		}
	}

	// object1の押し戻し量を返す
	return val;
}

void ObjectCollision::updateLoopCourse()
{
	auto object = _object;

	// 360度ループを移動中の場合は処理しない
	if (object->getObjectLoopMove()->getMoving()) {
		return;
	}

	// 壁当たり判定がない場合は処理しない
	auto wallCollisionList = object->getAreaArray(agtk::data::TimelineInfoData::kTimelineWall);
	if (wallCollisionList->count() == 0) {
		return;
	}

	int slopeBit = 0;
	int aheadSlopeBit = 0;

	WallHitInfo wallHitInfo;
	getWallCenterAndBound(wallCollisionList, &wallHitInfo.center, &wallHitInfo.boundMin, &wallHitInfo.boundMax);
	wallHitInfo.initHit();

	cocos2d::__Array *courseList = getCollisionLoopCourseList(object->getSceneLayer()->getType(), object->getLayerId(), wallHitInfo.boundMin, wallHitInfo.boundMax);

	if (courseList->count() > 0) {
		for (int i = 0; i < courseList->count(); i++) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
			auto course = static_cast<agtk::OthersLoopCourse*>(courseList->getObjectAtIndex(i));
#else
			auto course = dynamic_cast<agtk::OthersLoopCourse*>(courseList->getObjectAtIndex(i));
#endif

			// 入り口に接触した場合
			if (course->checkHitEnter(object, wallHitInfo.boundMin, wallHitInfo.boundMax)) {

				// コース上の移動開始開始
				object->getObjectLoopMove()->startMove(course, false);

				break;
			}

			// 出口に接触した場合した場合
			if (course->checkHitExit(object, wallHitInfo.boundMin, wallHitInfo.boundMax)) {
				
				// コース上の移動開始
				object->getObjectLoopMove()->startMove(course, true);

				break;
			}

			Vec2 cross;
			bool isUpSide;
//			bool isAheadUpSide;

			// オブジェクトがコースに接触するかチェック
			if (course->checkHit(object, cross, wallHitInfo.boundMin, wallHitInfo.boundMax, isUpSide)) {

				// 坂の上にいる場合
				if (isUpSide)
				{
					// 足が坂に接触している
					slopeBit |= agtk::data::ObjectActionLinkConditionData::kSlopeBitDown;
				}
				// 坂の下にいる場合
				else
				{
					// 頭が坂に接触している
					slopeBit |= agtk::data::ObjectActionLinkConditionData::kSlopeBitUp;
					// 適当に大きくY座標を戻す
					cross.y -= 7;
				}
				object->setPosition(agtk::Scene::getPositionCocos2dFromScene(cross));

				// 坂の当たりフラグを設定
				object->setSlopeBit(slopeBit);

				break;
			}
		}
	}
}

void ObjectCollision::addObject(agtk::Object *object)
{
	CC_ASSERT(_object->getObjectWallCollision() == this);	//仮説
	if (!_objectList->containsObject(object)) {
// #AGTK-NX #AGTK-WIN
#if 0
		if (this == _object->getObjectCollision()) {
			if (strcmp(object->getObjectData()->getName(), "chicken_fly") == 0) {
				CCLOG("%s: %s", _object->getObjectData()->getName(), object->getObjectData()->getName());
			}
		}
#endif
		_objectList->addObject(object);
	}
}

void ObjectCollision::addHitObject(agtk::Object *object)
{
	if (!_hitObjectList->containsObject(object)) {
		_hitObjectList->addObject(object);
	}
}

void ObjectCollision::addWallObject(agtk::Object *object)
{
	if (!_wallObjectList->containsObject(object)) {
		_wallObjectList->addObject(object);
	}
}

void ObjectCollision::removeObject(agtk::Object *object)
{
	CC_ASSERT(_object->getObjectCollision() == this);	//仮説
	if (_objectList->containsObject(object)) {
		_objectList->removeObject(object);
	}
}

void ObjectCollision::removeHitObject(agtk::Object *object)
{
	if (_hitObjectList->containsObject(object)) {
		_hitObjectList->removeObject(object);
	}
}

void ObjectCollision::removeWallObject(agtk::Object *object)
{
	if (_wallObjectList->containsObject(object)) {
		_wallObjectList->removeObject(object);
	}
}

/**
 * 攻撃判定が壁に触れているかチェック
 * @param	x	オブジェクトのX座標
 * @param	y	オブジェクトのY座標
 */
void ObjectCollision::checkAttackHitWall(float x, float y,int tileGroupBit)
{
	std::vector<agtk::Vertex4> attackCollisionList;
	if(_object->getTimelineAreaList(agtk::data::TimelineInfoData::kTimelineAttack, attackCollisionList) == false) {
		return;
	}

	for(auto attack : attackCollisionList) {
		auto size = attack.getRect().size;
		if (size.width <= 0.0f || size.height <= 0.0f) {
			continue;
		}
		// 衝突するタイルリスト取得
#ifdef USE_MULTITHREAD_OBJECT_UPDATE
		auto tileList = getCollisionTileList2(_object->getSceneLayer()->getType(), _object->getLayerId(), attack, _object);

		for (auto tile : tileList) {
#else
		auto tileList = getCollisionTileList2(_object->getSceneLayer()->getType(), _object->getLayerId(), attack, _object);
		cocos2d::Ref *ref2;
		CCARRAY_FOREACH(tileList, ref2) {
			auto tile = dynamic_cast<agtk::Tile *>(ref2);
#endif
			auto tileData = tile->getTileData();
			if (tileData == nullptr) {
				continue;
			}
			if (!(tileData->getGroupBit() & tileGroupBit)) {
				continue;
			}
			// タイルに攻撃判定が当たったフラグをON
			tile->setTouchAttackBox(true);
		}
	}
}

#ifdef USE_MULTITHREAD_OBJECT_UPDATE
std::vector<agtk::Tile *> ObjectCollision::getWallCollisionTileList(cocos2d::Vec2 move, int collideWithTileGroupBit)
#else
cocos2d::__Array *ObjectCollision::getWallCollisionTileList(cocos2d::Vec2 move)
#endif
{
	auto object = _object;
	std::vector<Vertex4> wallCollisionList;

	auto objectData = object->getObjectData();
	auto playObjectData = object->getPlayObjectData();
	bool isPhysicsOn = objectData->getPhysicsSettingFlag();
	bool isRelayPhysics = isPhysicsOn && playObjectData->getSwitchData(agtk::data::kObjectSystemSwitchFollowConnectedPhysics)->getValue();

	auto player = object->getPlayer();
	auto pos = player->getPosition();
	if (move != cocos2d::Vec2::ZERO) player->setPosition(pos + move, isRelayPhysics);
// #AGTK-NX #AGTK-WIN
#ifdef USE_30FPS_4
	// 中間フレーム情報対応のタイムライン壁判定取得
	object->getWallTimelineList(wallCollisionList);
#else
	object->getTimelineList(agtk::data::TimelineInfoData::kTimelineWall, wallCollisionList);
#endif
	if (move != cocos2d::Vec2::ZERO) player->setPosition(pos, isRelayPhysics);

	auto rect = agtk::Vertex4::getRectMerge(wallCollisionList);
	cocos2d::Point bmax = cocos2d::Point(rect.getMaxX(), rect.getMaxY());
	cocos2d::Point bmin = cocos2d::Point(rect.getMinX(), rect.getMinY());

	bmin.y -= 1;
	bmax.x += 1;
	bmax.y += 1;

	// 衝突しているタイルリストを取得
	return getCollisionTileList(object->getSceneLayer()->getType(), object->getLayerId(), bmin, bmax, move, object, collideWithTileGroupBit);
}

//-------------------------------------------------------------------------------------------------------------------
bool ObjectMovement::ForceMove::init(agtk::Object *object)
{
	_startPosition = cocos2d::Vec2::ZERO;
	_endPosition = cocos2d::Vec2::ZERO;
	_direction = cocos2d::Vec2::ZERO;
	_inertia = cocos2d::Vec2::ZERO;
	_speed = 0.0f;
	_seconds = 0.0f;
	_continueActionFrame = 0.0f;
	_maxSeconds = 0.0f;
	_range = 0.0f;
	_length = 0.0f;
	_moving = false;
	_object = object;
	_changeMoveSpeed = 100.0f;
	_finalGridMagnet = false;
	_ignoredEndReset = false;
	return true;
#ifdef FIX_ACT2_5237
	_followCameraMoving = false;
	_targetPosInCamera = cocos2d::Vec2::ZERO;
#endif
// #AGTK-NX #AGTK-WIN
#ifdef USE_30FPS
	_warpMoved = false;
#endif
}

void ObjectMovement::ForceMove::update(float dt)
{
	if (_moving == false) {
		if (_continueActionFrame != 0.0f) {
			_continueActionFrame += dt;
			if (isContinueAction()) {
				_continueActionFrame = 0.0f;
			}
		}
		return;
	}

	_seconds += dt;
	_continueActionFrame += dt;

#ifdef FIX_ACT2_5237
	if (_followCameraMoving) {
		auto scene = GameManager::getInstance()->getCurrentScene();
		auto newEndPos = agtk::Scene::getPositionCocos2dFromScene(cocos2d::Vec2(_targetPosInCamera.x, _targetPosInCamera.y));
		auto gameManager = GameManager::getInstance();
		auto projectData = gameManager->getProjectData();
		auto screenWidth = projectData->getScreenWidth();
		auto screenHeight = projectData->getScreenHeight();
		newEndPos += scene->getCamera()->getPosition() - cocos2d::Vec2(screenWidth / 2, screenHeight / 2);
		if (this->getType() == kTypeParam) {
			auto curPos = agtk::Scene::getPositionCocos2dFromScene(_object->getPosition());
			auto mv = (newEndPos - curPos).getNormalized();
			auto direction = _object->directionCorrection(mv);
			auto objectMovement = _object->getObjectMovement();
			objectMovement->setDirectionForce(direction);
		}
		this->setEndPosition(newEndPos);
	}
#endif
	switch(this->getType()) {
	case kTypeTime: {//指定位置までの移動が完了する時間。
		if (_length == _range || _object->getInputDirectionId() >= 0) {
			// 移動中に移動操作キー入力があった場合は移動をキャンセルする。
			this->end();
			break;
		}
		auto sp = this->getStartPosition();
		auto ep = this->getEndPosition();
		float lx = AGTK_LINEAR_INTERPOLATE(sp.x, ep.x, _range, _length);
		float ly = AGTK_LINEAR_INTERPOLATE(sp.y, ep.y, _range, _length);
		if (dt != 0) {
			_length += this->getSpeed() * (dt * FRAME60_RATE);
		}
		if (_length > _range) {
			_length = _range;
		}
#ifdef FIX_ACT2_5237
		if (_followCameraMoving) {
			auto curPos = agtk::Scene::getPositionCocos2dFromScene(_object->getPosition());
			this->setOldPosition(curPos);
		}
		else {
			this->setOldPosition(this->getPosition());
		}
#else
		this->setOldPosition(this->getPosition());
#endif
		float x = AGTK_LINEAR_INTERPOLATE(sp.x, ep.x, _range, _length);
		float y = AGTK_LINEAR_INTERPOLATE(sp.y, ep.y, _range, _length);
		auto objectMovement = _object->getObjectMovement();
		float moveX = objectMovement->getMoveX() + (x - lx);
		float moveY = objectMovement->getMoveY() + (y - ly);
		if (_length == _range) {
			// ACT2-5475 移動がフレームで刻んで行われることで誤差が発生してしまうため、移動完了時に誤差の範囲内に収まっていると思われる場合は、正確な到着座標となるよう補正する。
			auto curPos = agtk::Scene::getPositionCocos2dFromScene(_object->getPosition());
			auto newX = curPos.x + moveX;
			auto newY = curPos.y + moveY;
			if (fabsf(newX - ep.x) < 0.005f) {
				moveX = ep.x - curPos.x;
			}
			if (fabsf(newY - ep.y) < 0.005f) {
				moveY = ep.y - curPos.y;
			}
		}
		objectMovement->setMoveX(moveX);
		objectMovement->setMoveY(moveY);
		break; }
	case kTypeParam: {//基本移動パラメータを使用
		if (_object->getInputDirectionId() >= 0) {
			// 移動中に移動操作キー入力があった場合は移動をキャンセルする。
			this->end();
			return;
		}
		auto object = _object;
		auto objectMovement = object->getObjectMovement();
		float moveX = objectMovement->getMoveX() * (_changeMoveSpeed / 100.0f);
		float moveY = objectMovement->getMoveY() * (_changeMoveSpeed / 100.0f);
		objectMovement->setMoveX(moveX);
		objectMovement->setMoveY(moveY);

		auto nowPos = agtk::Scene::getPositionCocos2dFromScene(_object->getPosition());
		auto nextPos = nowPos + cocos2d::Vec2(moveX, moveY);
		auto endPos = this->getEndPosition();
		auto restPos = (endPos - nowPos);
		auto spdVec = Vec2(moveX, moveY);

		auto startPos = this->getStartPosition();

#if 0
		auto normal = (endPos - nowPos).getNormalized();
		objectMovement->setDirectionForce(normal);
#endif

		if (restPos.getLength() < spdVec.getLength()) {
			objectMovement->setMoveX(restPos.x);
			objectMovement->setMoveY(restPos.y);
		}
		auto en = (endPos - nextPos).getNormalized();
		if (en == cocos2d::Vec2::ZERO) {
			if (this->getIgnoredEndReset()) {
				auto direction = cocos2d::Vec2(moveX, moveY).getNormalized();
				objectMovement->setDirectionForce(direction);//強制移動設定。
			} else {
				objectMovement->resetDirectionForce();
				//※ACT2-3770
				objectMovement->setDirection(cocos2d::Vec2::ZERO);
			}
			this->end();
			break;
		}
		auto sn = (nextPos - nowPos).getNormalized();
		float angle = CC_RADIANS_TO_DEGREES(cocos2d::Vec2::angle(sn, en));
		if (angle == 180.0f) {
			if (this->getIgnoredEndReset()) {
				auto direction = cocos2d::Vec2(moveX, moveY).getNormalized();
				objectMovement->setDirectionForce(direction);//強制移動設定。
			} else {
				auto player = object->getPlayer();
				objectMovement->setMoveX(endPos.x - nowPos.x);
				objectMovement->setMoveY(endPos.y - nowPos.y);
				objectMovement->setMoveVelocity(cocos2d::Vec2::ZERO);
				objectMovement->resetDirectionForce();
				objectMovement->resetInputDirectionForce();
				//※ACT2-3770
				objectMovement->setDirection(cocos2d::Vec2::ZERO);
			}
			this->end();
			break;
		}
		if ((endPos - startPos).getLength() <= (nowPos - startPos).getLength()) {
			if (this->getIgnoredEndReset()) {
				auto direction = cocos2d::Vec2(moveX, moveY).getNormalized();
				objectMovement->setDirectionForce(direction);//強制移動設定。
			}
			else {
				objectMovement->resetDirectionForce();
				objectMovement->resetInputDirectionForce();
				//※ACT2-3770
				objectMovement->setDirection(cocos2d::Vec2::ZERO);
			}
			this->end();
			break;
		}
		if (_seconds >= _maxSeconds) {
			if (this->getIgnoredEndReset()) {
				auto direction = cocos2d::Vec2(moveX, moveY).getNormalized();
				objectMovement->setDirectionForce(direction);//強制移動設定。
			}
			else {
				objectMovement->resetDirectionForce();
				objectMovement->resetInputDirectionForce();
				//※ACT2-3770
				objectMovement->setDirection(cocos2d::Vec2::ZERO);
			}
			this->end();
		}
		break; }
	case kTypePushPull: {
		if (_length == _range) {
			this->end();
			break;
		}
		_length += this->getSpeed();
		if (_length > _range) {
			_length = _range;
		}
		this->setOldPosition(this->getStartPosition());
		this->setPosition(this->getEndPosition());
		break; }
	default:CC_ASSERT(0);
	}
}

void ObjectMovement::ForceMove::startTime(cocos2d::Vec2 startPosition, cocos2d::Vec2 endPosition, float speed, bool finalGridMagnet)
{
	this->setStartPosition(startPosition);
	this->setEndPosition(endPosition);
	this->setSpeed(speed);
	auto normal = (endPosition - startPosition).getNormalized();
	float range = (endPosition - startPosition).getLength();
	this->setDirection(normal);
	_moving = true;
	_seconds = 0.0f;
	_continueActionFrame = 0.0f;
	_length = 0.0f;
	_range = range;
	_finalGridMagnet = finalGridMagnet;
	_changeMoveSpeed = 100.0f;
	this->setPosition(startPosition);
	this->setOldPosition(startPosition);
	this->setType(kTypeTime);
#ifdef FIX_ACT2_5237
	_followCameraMoving = false;
#endif
}

void ObjectMovement::ForceMove::startParam(cocos2d::Vec2 startPosition, cocos2d::Vec2 endPosition, float changeMoveSpeed, bool finalGridMagnet, bool movePosition, cocos2d::Vec2 direction)
{
	this->setStartPosition(startPosition);
	this->setEndPosition(endPosition);

	auto dt = Director::getInstance()->getAnimationInterval();
	auto move = _object->getObjectMovement()->move(dt);
	cocos2d::Vec2 normal;
	if (direction == cocos2d::Vec2::ZERO) {
		normal = (endPosition - startPosition).getNormalized();
	}
	else {
		normal = direction.getNormalized();
	}
	this->setDirection(normal);
	auto strength = cocos2d::Vec2(move.x * normal.x, move.y * normal.y).getLength();
	float length = (endPosition - startPosition).getLength();
	_seconds = 0;
	_continueActionFrame = 0.0f;
	_maxSeconds = length * dt / strength;
	if (movePosition) {
		//距離をｘ成分、ｙ成分に分けて足した長さを移動にかかる最大時間とする。
		float width = std::abs(endPosition.x - startPosition.x);
		float height = std::abs(endPosition.y - startPosition.y);
		_maxSeconds = (width + height) * dt / strength;
	}
	_moving = true;
	this->setPosition(startPosition);
	this->setOldPosition(startPosition);
	this->setType(kTypeParam);
	_changeMoveSpeed = changeMoveSpeed;
	_finalGridMagnet = finalGridMagnet;
#ifdef FIX_ACT2_5237
	_followCameraMoving = false;
#endif
}

void ObjectMovement::ForceMove::startPushPull(cocos2d::Vec2 startPosition, cocos2d::Vec2 endPosition)
{
	this->setStartPosition(startPosition);
	this->setEndPosition(endPosition);
	auto normal = (endPosition - startPosition).getNormalized();
	float range = (endPosition - startPosition).getLength();
	this->setDirection(normal);
	_moving = true;
	_seconds = 0.0f;
	_continueActionFrame = 0.0f;
	_length = 0.0f;
	_range = range;
	_changeMoveSpeed = 100.0f;
	this->setSpeed(range);
	this->setPosition(startPosition);
	this->setOldPosition(startPosition);
	this->setType(kTypePushPull);
#ifdef FIX_ACT2_5237
	_followCameraMoving = false;
#endif
}

void ObjectMovement::ForceMove::end()
{
	auto timerPauseAction = _object->getTimerPauseAction();
	if (timerPauseAction->isProcessing()) {
		timerPauseAction->end();
	}
	auto timerPauseAnimation = _object->getTimerPauseAnimation();
	if (timerPauseAnimation->isProcessing()) {
		timerPauseAnimation->end();
	}

#if 0 // ACT2-4769 無敵状態が解除されるため無効
	auto objectVisible = _object->getObjectVisible();
	if (objectVisible->isInvincible()) {
		objectVisible->end();
	}
#endif

	//移動完了後のオブジェクトがグリッドに吸着する
	if (_finalGridMagnet) {
		_object->setNeedAbsorbToTileCorners(true);
	}
	_moving = false;
}

cocos2d::Vec2 ObjectMovement::ForceMove::move()
{
	auto type = this->getType();
	if (type == kTypePushPull) {
		return this->getPosition() - this->getOldPosition();
	}
	auto objectMovement = _object->getObjectMovement();
	return cocos2d::Vec2(objectMovement->getMoveX(), objectMovement->getMoveY());
}

bool ObjectMovement::ForceMove::isContinueAction()
{
	if (_continueActionFrame != 0.0f && _continueActionFrame < FRAME_PER_SECONDS) {
		return false;
	}
	return true;
}

//-------------------------------------------------------------------------------------------------------------------
ObjectMovement::TimerFloat::TimerFloat() : agtk::EventTimer()
{
	_value = 0.0f;
	_prevValue = 0.0f;
	_nextValue = 0.0f;
}

ObjectMovement::TimerFloat::~TimerFloat()
{
}

bool ObjectMovement::TimerFloat::init(float value)
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
		if (_seconds == 0.0f) {
			_value = _nextValue;
		}
		else {
			_value = AGTK_LINEAR_INTERPOLATE(_prevValue, _nextValue, _seconds, _timer);
		}
	});
	this->setEndFunc([&]() {
		_oldValue = _value;
		_value = _nextValue;
	});

	return true;
}

float ObjectMovement::TimerFloat::setValue(float value, float seconds)
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

	_prevValue = _nextValue;
	_value = _prevValue;
	_nextValue = value;
	//_prevValue = _value;
	if (seconds == 0.0f) {
		_value = value;
	}
	this->start(seconds);
	return _value;
}
 
float ObjectMovement::TimerFloat::addValue(float value, float seconds)
{
	auto v = this->getValue();
	this->setValue(v + value, seconds);
	return v;
}

//-------------------------------------------------------------------------------------------------------------------
ObjectMovement::ObjectMovement()
{
	_object = nullptr;
	_ignoredJump = false;
	_ignoredGravity = false;
	_wallMoveSpeed = nullptr;
	_wallJump = nullptr;
	_wallSlip = nullptr;
	_wallGravityEffect = nullptr;
	_inputDirection = cocos2d::Vec2(0, 0);
	_inputDirectionForce = cocos2d::Vec2(0, 0);
	_inputDirectionForceFlag = false;
	_moveVelocityList = nullptr;
	_forceMove = nullptr;
	_direction = cocos2d::Vec2::ZERO;
	_directionForce = cocos2d::Vec2::ZERO;
	_directionForceFlag = false;
	_resetDirectionXFlag = false;
	_resetDirectionYFlag = false;
	_directionForceIgnoredChangeActionFlag = false;
	_directionOld = cocos2d::Vec2::ZERO;
	_wallFriction = nullptr;
	_moveSpeed = nullptr;
	_upDownMoveSpeed = nullptr;
	_turnSpeed = nullptr;
	_wallMoveX = 0.0f;
	_wallMoveY = 0.0f;
	_moveX = 0.0f;
	_moveY = 0.0f;
	_canMoveLift = false;
	_objectMoveLiftList = nullptr;
	_gravity = cocos2d::Vec2::ZERO;

	_fixedJumpDirectionId = 0;

	_preTimeScale = 1.0f;
	_timeScaleZeroFlag = false;
	_zeroPreTimeScale = 1.0f;
	_jumpDuration = 0.0f;
	_jumpStartFloor = false;

	_keepJumping = false;
	_moveSideBit = 0;
	_distanceMax = -1;
	_movedDistance = 0;
}

ObjectMovement::~ObjectMovement()
{
	CC_SAFE_RELEASE_NULL(_wallMoveSpeed);
	CC_SAFE_RELEASE_NULL(_wallJump);
	CC_SAFE_RELEASE_NULL(_wallSlip);
	CC_SAFE_RELEASE_NULL(_wallGravityEffect);
	CC_SAFE_RELEASE_NULL(_moveVelocityList);
	CC_SAFE_RELEASE_NULL(_forceMove);
	CC_SAFE_RELEASE_NULL(_wallFriction);
	CC_SAFE_RELEASE_NULL(_moveSpeed);
	CC_SAFE_RELEASE_NULL(_upDownMoveSpeed);
	CC_SAFE_RELEASE_NULL(_turnSpeed);
#ifdef USE_MULTITHREAD_OBJECT_UPDATE
	CC_SAFE_DELETE(_objectMoveLiftList);
#else
	CC_SAFE_RELEASE_NULL(_objectMoveLiftList);
#endif
}

bool ObjectMovement::init(agtk::Object *object)
{
	//object
	if (object == nullptr) {
		CC_ASSERT(0);
		return false;
	}
	_object = object;
	auto objectData = _object->getObjectData();

	auto wallMoveSpeed = agtk::ObjectMovement::Value::create(0.0f);
	if (wallMoveSpeed == nullptr) {
		CC_ASSERT(0);
		return false;
	}
	this->setWallMoveSpeed(wallMoveSpeed);
	auto wallJump = agtk::ObjectMovement::Value::create(0.0f);
	if (wallJump == nullptr) {
		CC_ASSERT(0);
		return false;
	}
	this->setWallJump(wallJump);
	//タイル「プレイヤーキャラの移動が滑るようになる」
	//<処理> 減速時に動作する
	//[加速移動の場合]
	//0は減速量が速度と同じ（すぐに止まる）
	//50は設定している減速と同じ。
	//100は減速が0。
	//[基本移動の場合]
	//0は速度と同じ（すぐ止まる）
	//100は減速なし。
	//0～100を移動量で割合を出す。
	float slipRate = this->isAcceleration() ? 50.0f : 0.0f;
	auto wallSlip = agtk::ObjectMovement::Value::create(slipRate);
	if (wallSlip == nullptr) {
		CC_ASSERT(0);
		return false;
	}
	this->setWallSlip(wallSlip);
	auto wallGravityEffect = agtk::ObjectMovement::Value::create(100.0f);
	if (wallGravityEffect == nullptr) {
		CC_ASSERT(0);
		return false;
	}
	this->setWallGravityEffect(wallGravityEffect);
	_direction = cocos2d::Vec2::ZERO;
	_directionForce = cocos2d::Vec2::ZERO;
	auto moveVelocityList = cocos2d::__Array::create();
	if (moveVelocityList == nullptr) {
		CC_ASSERT(0);
		return false;
	}
	this->setMoveVelocityList(moveVelocityList);
	auto forceMove = agtk::ObjectMovement::ForceMove::create(object);
	if (forceMove == nullptr) {
		CC_ASSERT(0);
		return false;
	}
	this->setForceMove(forceMove);
	auto wallFriction = agtk::ObjectMovement::Value::create(0.0f);
	if (wallFriction == nullptr) {
		CC_ASSERT(0);
		return false;
	}
	this->setWallFriction(wallFriction);
	auto moveSpeed = agtk::ObjectMovement::TimerFloat::create(0.0f);
	if (moveSpeed == nullptr) {
		CC_ASSERT(0);
		return false;
	}
	this->setMoveSpeed(moveSpeed);
	auto upDownMoveSpeed = agtk::ObjectMovement::TimerFloat::create(0.0f);
	if (upDownMoveSpeed == nullptr) {
		CC_ASSERT(0);
		return false;
	}
	this->setUpDownMoveSpeed(upDownMoveSpeed);
	auto turnSpeed = agtk::ObjectMovement::TimerFloat::create(0.0f);
	if (turnSpeed == nullptr) {
		CC_ASSERT(0);
		return false;
	}
	this->setTurnSpeed(turnSpeed);
#ifdef USE_MULTITHREAD_OBJECT_UPDATE
	this->setObjectMoveLiftList(new MtVector<Object *>());
#else
	this->setObjectMoveLiftList(cocos2d::__Array::create());
#endif
	return true;
}

void ObjectMovement::update(float dt)
{
	_moveSideBit = 0;
	_canMoveLift = false;
	auto vertVelocityOld = _timeScaleZeroFlag ? _vertVelocityTemp : _vertVelocity;
	if (_keepJumping) {
		_keepJumping = false;
		vertVelocityOld = _keepVertVelocity;
	}
	auto jumping = _object->_jumping;
	auto floor = _object->_floor;
// #AGTK-NX #AGTK-WIN
#ifdef USE_30FPS
	// 移動量計算は60fps毎の計算で行う。_vertVerocityの更新も60fpsでの移動量をセット。
	float progScale = GameManager::getInstance()->getFrameProgressScale();
	cocos2d::Vec2 _move = this->move(dt / progScale);
	cocos2d::Vec2 _jump = this->jump(dt / progScale);
	cocos2d::Vec2 gravity = this->gravity(dt / progScale);
#else
	cocos2d::Vec2 _move = this->move(dt);
	cocos2d::Vec2 _jump = this->jump(dt);
	cocos2d::Vec2 gravity = this->gravity(dt);
#endif
	float water = this->water();
	float timeScale = _object->getTimeScale();
	bool changeJumping = (jumping == false) && (_object->_jumping == true);//ジャンプフラグがOffからOnになる。
	auto vertMove = (_jump + gravity) * water;

	// 前フレームからタイムスケールが変化した場合
	if (this->getPreTimeScale() != timeScale) {
		if (timeScale == 0.0f) {
			_vertVelocityTemp = _vertVelocity;
			_moveVelocityTemp = _moveVelocity;
			_zeroPreTimeScale = this->getPreTimeScale();
			_timeScaleZeroFlag = true;
		} else
		if(_timeScaleZeroFlag) {
			_vertVelocity = _vertVelocityTemp;
			_moveVelocity = _moveVelocityTemp;
			_timeScaleZeroFlag = false;
		}
		// ジャンプ直後でなく、ジャンプ中で、ジャンプベクトルが 0 になった場合、もしくは落下中の場合。
		if ((!changeJumping && jumping && _jump.isZero()) || _object->_falling) {
			// _vertVelocity は前フレームのタイムスケールで保存されているので
			// 変化後のタイムスケール値に近づくような値を設定する
			auto rate = (this->getPreTimeScale() != 0) ? timeScale / this->getPreTimeScale() : timeScale / this->getZeroPreTimeScale();
			_vertVelocity *= rate;
			this->setPreTimeScale(timeScale);
		}
		else {
			this->setPreTimeScale(timeScale);
		}
	}
// #AGTK-NX #AGTK-WIN
#if defined(USE_30FPS)
	timeScale *= dt * FRAME60_RATE;
#endif

	auto vertVelocity = (_vertVelocity + vertMove);
// #AGTK-NX #AGTK-WIN
#if defined(USE_30FPS)
	auto vertVelocity2 = cocos2d::Vec2::ZERO;
	if (GameManager::getInstance()->getFrameProgressScale() > 1.0f) {
		// 30fpsでの追加対応。vertVelocityは30fpsでの今フレームの移動量 、vertVelocity2は次フレームに持ち越す60fpsでの移動量
		this->setVertVelocity(vertVelocity);
		cocos2d::Vec2 _move2 = this->move(dt / progScale);
		_move += _move2;
		cocos2d::Vec2 _jump2 = this->jump(dt / progScale);
		cocos2d::Vec2 gravity2 = this->gravity(dt / progScale);
		auto vertMove2 = (_jump2 + gravity2) * water;
		vertVelocity2 = (_vertVelocity + vertMove2);
		vertVelocity += vertVelocity2;
	}
#endif

	this->setGravity(gravity);

	//落下の最大移動量を計算。
	auto objectData = _object->getObjectData();
	if (objectData->getLimitFalldownAmount()) {
		//重力と同じ方向を落下の移動方向として、XY成分に分けて最大移動量を計算する。
		auto ng = gravity.getNormalized();
		cocos2d::Vec2 v;
		v.x = ((ng.x > 0.0f && vertVelocity.x > 0.0f) || (ng.x < 0.0f && vertVelocity.x < 0.0f)) ? (abs(ng.x) * vertVelocity.x) : 0.0f;
		v.y = ((ng.y > 0.0f && vertVelocity.y > 0.0f) || (ng.y < 0.0f && vertVelocity.y < 0.0f)) ? (abs(ng.y) * vertVelocity.y) : 0.0f;
		if (v.length() > objectData->getMaxFalldownAmount() * timeScale) {
			vertVelocity -= v;
			vertVelocity += (ng * objectData->getMaxFalldownAmount()) * timeScale;
		}
// #AGTK-NX #AGTK-WIN
#ifdef USE_30FPS
		if (GameManager::getInstance()->getFrameProgressScale() > 1.0f) {
			cocos2d::Vec2 v;
			v.x = ((ng.x > 0.0f && vertVelocity2.x > 0.0f) || (ng.x < 0.0f && vertVelocity2.x < 0.0f)) ? (abs(ng.x) * vertVelocity2.x) : 0.0f;
			v.y = ((ng.y > 0.0f && vertVelocity2.y > 0.0f) || (ng.y < 0.0f && vertVelocity2.y < 0.0f)) ? (abs(ng.y) * vertVelocity2.y) : 0.0f;
			if (v.length() > objectData->getMaxFalldownAmount() * timeScale / GameManager::getInstance()->getFrameProgressScale()) {
				vertVelocity2 -= v;
				vertVelocity2 += (ng * objectData->getMaxFalldownAmount()) * timeScale / GameManager::getInstance()->getFrameProgressScale();
			}
		}
#endif
	}

	auto gn = gravity.getNormalized();
	int wallBitUp = 0;
	int slopeBitUp = 0;
	if (gn != cocos2d::Vec2::ZERO) {
		//重力下方向
		if (gn.y > 0) {
			if (gn.x > 0) {
				wallBitUp = (abs(gn.y) >= abs(gn.x)) ? agtk::data::ObjectActionLinkConditionData::kWallBitDown : agtk::data::ObjectActionLinkConditionData::kWallBitLeft;
			}
			else if (gn.x < 0) {
				wallBitUp = (abs(gn.y) >= abs(gn.x)) ? agtk::data::ObjectActionLinkConditionData::kWallBitDown : agtk::data::ObjectActionLinkConditionData::kWallBitRight;
			}
			else {
				CC_ASSERT(gn.x == 0.0f);
				wallBitUp = agtk::data::ObjectActionLinkConditionData::kWallBitDown;
			}

			slopeBitUp = agtk::data::ObjectActionLinkConditionData::kSlopeBitDown;
		}
		//重力上方向
		else if (gn.y < 0) {
			if (gn.x > 0) {
				wallBitUp = (abs(gn.y) >= abs(gn.x)) ? agtk::data::ObjectActionLinkConditionData::kWallBitUp : agtk::data::ObjectActionLinkConditionData::kWallBitLeft;
			}
			else if (gn.x < 0) {
				wallBitUp = (abs(gn.y) >= abs(gn.x)) ? agtk::data::ObjectActionLinkConditionData::kWallBitUp : agtk::data::ObjectActionLinkConditionData::kWallBitRight;
			}
			else {
				CC_ASSERT(gn.x == 0.0f);
				wallBitUp = agtk::data::ObjectActionLinkConditionData::kWallBitUp;
			}

			slopeBitUp = agtk::data::ObjectActionLinkConditionData::kSlopeBitUp;
		}
		else {
			CC_ASSERT(gn.y == 0.0f);
			wallBitUp = gn.x > 0 ? agtk::data::ObjectActionLinkConditionData::kWallBitLeft : agtk::data::ObjectActionLinkConditionData::kWallBitRight;
		}
	}
	
	//タイルの壁判定およびオブジェクトの壁判定の下から衝突した場合。
	if ((objectData->getFallOnCollideWithWall() && ((_object->getTileWallBit() & wallBitUp) || (_object->getSlopeBit() & slopeBitUp)))
	|| (objectData->getFallOnCollideWithHitbox() && _object->getUpWallObjectList()->count() > 0)) {
		//天井に当たったので速度には重力のみ設定する。
		if (_object->_jumping == true && ((_jump.y <= 0.0f && gravity.y > 0.0f) || (_jump.y >= 0.0f && gravity.y < 0.0f))) {
			if (changeJumping) {
				//※天井に当たった場合、重力値が小さいと壁判定に当たって、天井にくっついてしまう場合があるため、
				//重力値が、壁判定で閾値分(TILECOLLISION_THRESHOLD）より小さい場合は、閾値分の大きさにして壁判定から遠ざけるようにする。
				if (gravity.length() < TILE_COLLISION_THRESHOLD) {
					gravity = gravity.getNormalized() * TILE_COLLISION_THRESHOLD;
				}
			}
// #AGTK-NX #AGTK-WIN
#ifdef USE_30FPS
			if (GameManager::getInstance()->getFrameProgressScale() > 1.0f) {
				// 60fps
				vertVelocity = gravity;
			}
			else {
				// 30fps
				vertVelocity = gravity * 2.0f;
				vertVelocity2 = gravity;
			}
#else
			vertVelocity = gravity;
#endif
		}
	}
	//落ちている
	if (_object->_falling && gn != cocos2d::Vec2::ZERO) {
		//壁にぶつかっている場合。
		if ((_object->getTileWallBit() & agtk::data::ObjectActionLinkConditionData::kWallBitRight && _move.x > 0)
		|| (_object->getTileWallBit() & agtk::data::ObjectActionLinkConditionData::kWallBitLeft && _move.x < 0)
		|| _object->getReturnedPos().x != 0.0f) {
			_move.x = 0;
		}
	}

	int wallBitDown = 0;
	int slopeBitDown = 0;
	if (gn != cocos2d::Vec2::ZERO) {
		//重力上方向
		if (gn.y > 0) {
			if (gn.x > 0) {
				wallBitDown = (abs(gn.y) >= abs(gn.x)) ? agtk::data::ObjectActionLinkConditionData::kWallBitUp : agtk::data::ObjectActionLinkConditionData::kWallBitRight;
			}
			else if (gn.x < 0) {
				wallBitDown = (abs(gn.y) >= abs(gn.x)) ? agtk::data::ObjectActionLinkConditionData::kWallBitUp : agtk::data::ObjectActionLinkConditionData::kWallBitLeft;
			}
			else {
				CC_ASSERT(gn.x == 0.0f);
				wallBitDown = agtk::data::ObjectActionLinkConditionData::kWallBitUp;
			}

			slopeBitDown = agtk::data::ObjectActionLinkConditionData::kSlopeBitUp;
		}
		//重力下方向
		else if (gn.y < 0) {
			_canMoveLift = true;
			if (gn.x > 0) {
				wallBitDown = (abs(gn.y) >= abs(gn.x)) ? agtk::data::ObjectActionLinkConditionData::kWallBitDown : agtk::data::ObjectActionLinkConditionData::kWallBitRight;
			}
			if (gn.x < 0) {
				wallBitDown = (abs(gn.y) >= abs(gn.x)) ? agtk::data::ObjectActionLinkConditionData::kWallBitDown : agtk::data::ObjectActionLinkConditionData::kWallBitLeft;
			}
			else {
//				CC_ASSERT(gn.x == 0.0f);
				wallBitDown = agtk::data::ObjectActionLinkConditionData::kWallBitDown;
			}

			slopeBitDown = agtk::data::ObjectActionLinkConditionData::kSlopeBitDown;
		}
		else {
			CC_ASSERT(gn.y == 0.0f);
			wallBitDown = gn.x > 0 ? agtk::data::ObjectActionLinkConditionData::kWallBitRight : agtk::data::ObjectActionLinkConditionData::kWallBitLeft;
		}
	}

	bool bFalling = false;
	bool bFallVelocity = false;
	{
		auto scene = GameManager::getInstance()->getCurrentScene();
		auto gravity = scene->getGravity()->getGravity();
		if (gravity != cocos2d::Vec2::ZERO && vertVelocity != cocos2d::Vec2::ZERO) {
			cocos2d::Vec2 defGravity(0, -1);
			float angle = defGravity.getAngle(gravity.getNormalized());
			auto nv = vertVelocity.getNormalized();
			auto g = nv.rotateByAngle(defGravity, -angle);
			bFallVelocity = g.y < 0.0f ? true : false;
		}
	}
	bFalling = bFallVelocity;

	//! ジャンプ頂点に到達したかチェック(経過時間が0の場合は状態を変えない。)
	// ジャンプ直後にも頂点に達したと判定されていたのでジャンプ直後を判定するフラグも頂点到達判定に追加
	//_object->_jumpTop = !changeJumping && _object->_jumping && ((vertVelocityOld.y >= 0 && vertVelocity.y <= 0) || (vertVelocityOld.y <= 0 && vertVelocity.y >= 0));//ジャンプトップ
	if (timeScale != 0.0f && _object->_jumping) {
// #AGTK-NX #AGTK-WIN
#if defined(USE_30FPS)
		if (GameManager::getInstance()->getFrameProgressScale() > 1.0f) {
			_object->_jumpTop = (!changeJumping || (changeJumping && bFalling)) && ((vertVelocityOld.y >= 0 && vertVelocity2.y <= 0) || (vertVelocityOld.y <= 0 && vertVelocity2.y >= 0));//ジャンプトップ
		} 
		else {
			_object->_jumpTop = (!changeJumping || (changeJumping && bFalling)) && ((vertVelocityOld.y >= 0 && vertVelocity.y <= 0) || (vertVelocityOld.y <= 0 && vertVelocity.y >= 0));//ジャンプトップ
		}
#else
		_object->_jumpTop = (!changeJumping || (changeJumping && bFalling)) && ((vertVelocityOld.y >= 0 && vertVelocity.y <= 0) || (vertVelocityOld.y <= 0 && vertVelocity.y >= 0));//ジャンプトップ
#endif
		if (_object->_jumping && _object->_jumpTop) {
			_object->_jumping = false;
			_jumpStartFloor = false;
			changeJumping = false;
		}
	}

	//地面接触
	_object->_floorOld = _object->_floor;
	_object->_floor = false;
	_object->_floorTile = false;
	_object->_floorObject = false;
	_object->_floorSlope = false;
	if ((_object->getTileWallBit() & wallBitDown) || (_object->getObjectSameLayerWallBit() & wallBitDown) || (_object->getSlopeBit() & wallBitDown)) {
		if (changeJumping == false && _jumpStartFloor == false) {
			//地面に着いている。
			bFalling = false;
			_object->_floor = true;
			_object->_floorTile = (_object->getTileWallBit() & wallBitDown);
			_object->_floorObject = (_object->getObjectSameLayerWallBit() & wallBitDown);
			_object->_floorSlope = (_object->getSlopeBit() & wallBitDown);
			//１フレーム前と現在で地面に接している場合に、垂直方向の速度をクリアする。
			if (_object->_floor && _object->_floorOld) {
				vertVelocity = cocos2d::Vec2::ZERO;
// #AGTK-NX #AGTK-WIN
#ifdef USE_30FPS
				vertVelocity2 = cocos2d::Vec2::ZERO;
#endif
			}
		}
	}
	else {
		// ジャンプ開始時の地面接触フラグをfalse。
		_jumpStartFloor = false;
	}

	//移動量
	_moveX = _move.x + vertVelocity.x;
	_moveY = _move.y + vertVelocity.y;

#if 0
	if (_object->getObjectData()->isGroupPlayer()) {
		CCLOG("move:%f,%f", _move.x, _move.y);
//		CCLOG("jump:%f,gravity:%f,vart:%f,falling:%d,jumping:%d,jumpTop:%d", _jump.y, _gravity.y, _vertVelocity.y, _object->_falling, _object->_jumping, _object->_jumpTop);
//		CCLOG("jump:%f,gravity:%f,vart:%f", _jump.y, _gravity.y, _vertVelocity.y);
		auto scene = GameManager::getInstance()->getCurrentScene();
		auto gravity = scene->getGravity()->getGravity();

		cocos2d::Vec2 defGravity(0, -1);
		cocos2d::Vec2 gn = gravity.getNormalized();
		float angle = defGravity.getAngle(gn);
		CCLOG("angle:%f,%f", angle, CC_RADIANS_TO_DEGREES(angle));
		auto gc = gravity.rotateByAngle(defGravity, -angle);
		CCLOG("gravity:%f,%f:%f,%f:%f,%f", _gravity.x, _gravity.y, gravity.x, gravity.y, gc.x, gc.y);
		CCLOG("falling:%d,jumping:%d,jumpTop:%d", _object->_falling, _object->_jumping, _object->_jumpTop);
		CCLOG("vertVelocity:%f,%f", _vertVelocity.x, _vertVelocity.y);
		//CCLOG("falling:%d:%d,jumping:%d,jumpTop:%d,jumpAction:%d", _object->_falling, falling, _object->_jumping, _object->_jumpTop, _object->getJumpActionFlag());		
	}
#endif
	//強制移動。
	auto forceMove = this->getForceMove();
	forceMove->update(dt);
	if (forceMove->isMoving()) {
		auto move = forceMove->move();
		_moveX = move.x;
		_moveY = move.y;
		vertVelocity = cocos2d::Vec2::ZERO;//強制移動中は0に。
// #AGTK-NX #AGTK-WIN
#ifdef USE_30FPS
		vertVelocity2 = cocos2d::Vec2::ZERO;
#endif
	}

	// コース移動
	auto courseMove = _object->getObjectCourseMove();
	if (courseMove != nullptr) {
		courseMove->update(dt);
		if (courseMove->isMoving()) {
			auto move = courseMove->getMove();
			_moveX = move.x;
			_moveY = move.y;
			vertVelocity = cocos2d::Vec2::ZERO;//コース移動中は0に。
// #AGTK-NX #AGTK-WIN
#ifdef USE_30FPS
			vertVelocity2 = cocos2d::Vec2::ZERO;
#endif
		}
	}

	// ループ移動
	auto loopMove = _object->getObjectLoopMove();
	if (loopMove != nullptr && loopMove->getMovingAuto()) {
		// 自動移動中は値を0にする
		_moveX = 0;
		_moveY = 0;
		vertVelocity = cocos2d::Vec2::ZERO;
// #AGTK-NX #AGTK-WIN
#ifdef USE_30FPS
		vertVelocity2 = cocos2d::Vec2::ZERO;
#endif
	}

	//[その他アクション] テンプレート移動の実行
	this->updateObjectTemplateMove(cocos2d::Vec2(_moveX, _moveY), gravity);

	// 物理オブジェクトと衝突しているかチェック
	auto physicsNode = _object->getphysicsNode();
	bool bHitUpPhysics = false;
	bool bHitDownPhysics = false;
	bool bHitLeftPhysics = false;
	bool bHitRightPhysics = false;
	if (physicsNode) {
		auto physicsBody = physicsNode->getPhysicsBody();
		if (physicsBody) {
			cocos2d::Ref * ref = nullptr;
			CCARRAY_FOREACH(_object->getHitPhysicsObjList(), ref) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
				auto hitData = static_cast<agtk::Object::HitPhysicsObjData *>(ref);
#else
				auto hitData = dynamic_cast<agtk::Object::HitPhysicsObjData *>(ref);
#endif
				auto hitDir = hitData->getDirectionVec();
				auto objVelocity = _move + vertVelocity;
				bool bNeedChangeVelocity = false;

				if (hitDir.y > 0 && objVelocity.y > 0) {
					bHitUpPhysics = true;
					bNeedChangeVelocity = true;
				}
				if (hitDir.y < 0 && objVelocity.y < 0) {
					bHitDownPhysics = true;
					bNeedChangeVelocity = true;

					// 物理オブジェクトの上に乗っているので落下フラグOFF
					bFalling = false;
					_object->_floor = true;
				}
				if (hitDir.x > 0 && objVelocity.x > 0) {
					bHitRightPhysics = true;
					bNeedChangeVelocity = true;
				}
				if (hitDir.x < 0 && objVelocity.x < 0) {
					bHitLeftPhysics = true;
					bNeedChangeVelocity = true;
				}

				if (bNeedChangeVelocity) {
					auto body = hitData->getPhysicsBody();
					auto physicsVelocity = body->getVelocity();

					// 質量計算
					auto calcMass = (physicsBody->getMass() * body->getMass()) / (physicsBody->getMass() + body->getMass());

					// 衝突した物理オブジェクトへの撃力を計算
					auto force = (body->getFirstShape()->getMaterial().restitution) * calcMass * (objVelocity - physicsVelocity);
					body->applyImpulse(force);

					// 物理影響を受ける場合
					//if (objectData->getPhysicsSetting()->getPhysicsAffected()) {
					//	// 衝突したこのオブジェクトへの撃力を計算
					//	force = (physicsBody->getFirstShape()->getMaterial().restitution) * calcMass * (physicsVelocity - objVelocity);

					//	// 左右のどちらかが衝突している場合
					//	if (bHitLeftPhysics || bHitRightPhysics) {
					//		_move.x = force.x;
					//	}

					//	// 上下のどちらかが衝突している場合
					//	if (bHitUpPhysics || bHitDownPhysics) {
					//		_vertVelocity.y = force.y;
					//	}
					//}
				}
			}
		}
	}
	_object->setFalling(bFalling);

// #AGTK-NX #AGTK-WIN
#ifdef USE_30FPS
	if (GameManager::getInstance()->getFrameProgressScale() <= 1.0f) {
		// 60fps
		this->setVertVelocity(vertVelocity);
	}
	else {
		// 30fps
		this->setVertVelocity(vertVelocity2);
	}
#else
	this->setVertVelocity(vertVelocity);
#endif

	this->getWallMoveSpeed()->update();
	this->getWallJump()->update();
	this->getWallSlip()->update();
	this->getWallGravityEffect()->update();
	this->getWallFriction()->update();
	this->getMoveSpeed()->update(dt);
	this->getUpDownMoveSpeed()->update(dt);
	this->getTurnSpeed()->update(dt);
}

void ObjectMovement::updateObjectTemplateMove(cocos2d::Vec2 move, cocos2d::Vec2 gravity)
{
	auto templateMove = _object->getObjectTemplateMove();
	if (templateMove->getState() == agtk::ObjectTemplateMove::kStateIdle) {
		return;
	}

	auto objCommand = templateMove->getObjCommand();
	auto player = _object->getPlayer();
	if (player == nullptr) {
		return;
	}

	auto nowWallHitInfoGroup = _object->getObjectCollision()->getWallHitInfoGroup();
	auto prevWallHitInfoGroup = _object->getObjectCollision()->getPrevWallHitInfoGroup();
	auto oldMove = (nowWallHitInfoGroup->getBoundMin() - prevWallHitInfoGroup->getBoundMin());

	int length = (int)move.getLength();
	if (length < 1) length = 1;

	bool bUpWallCollied = _object->getTileWallBit() & agtk::data::ObjectActionLinkConditionData::kWallBitUp;
	bool bDownWallCollied = _object->getTileWallBit() & agtk::data::ObjectActionLinkConditionData::kWallBitDown;
	bool bLeftWallCollied = _object->getTileWallBit() & agtk::data::ObjectActionLinkConditionData::kWallBitLeft;
	bool bRightWallCollied = _object->getTileWallBit() & agtk::data::ObjectActionLinkConditionData::kWallBitRight;
	if (bLeftWallCollied && move.x > 0.0f) bLeftWallCollied = false;
	if (bRightWallCollied && move.x < 0.0f) bRightWallCollied = false;

	bool bForceLock = false;
	cocos2d::Vec2 mv;
	cocos2d::Vec2 mv_old;
	for (int i = 1; i <= length; i++) {
		auto moveNormalized = move.getNormalized();
		auto len = move.getLength();

		if (i == 1) {
			mv = moveNormalized * len * ((float)i / (float)length);
			mv_old = mv;
		} else {
			mv_old = mv;
			mv = moveNormalized * len * ((float)i / (float)length);
		}

		//移動先で接触する壁判定情報を取得する。
		std::vector<Vertex4> wallCollisionList;
		auto pos = player->getPosition();
		player->setPosition(pos + mv);
		_object->getTimelineList(agtk::data::TimelineInfoData::kTimelineWall, wallCollisionList);
		player->setPosition(pos);

		//坂にいる場合は坂に沿った移動先で接触する壁判定情報を取得する。
		std::vector<Vertex4> oldWallCollisionList;
		cocos2d::Vec2 omv;
		if (_object->_floorSlope == true) {
			auto oldMoveNormalized = oldMove.getNormalized();
			auto oldLen = oldMove.getLength();
			omv = oldMoveNormalized * (oldLen * ((float)i / (float)length));
			player->setPosition(pos + omv);
			_object->getTimelineList(agtk::data::TimelineInfoData::kTimelineWall, oldWallCollisionList);
			player->setPosition(pos);
		}

		//移動先で接触する坂情報を取得する。
		auto infoGroup = _object->getObjectCollision()->getWallHitInfoGroup();
		cocos2d::Point bmin = infoGroup->getBoundMin() + mv;
		cocos2d::Point bmax = infoGroup->getBoundMax() + mv;
		bmin -= Point::ONE;
		bmax += Point::ONE;
		auto slopeList = getCollisionSlopeList(_object->getSceneLayer()->getType(), _object->getLayerId(), bmin, bmax, mv);
#ifdef USE_MULTITHREAD_OBJECT_UPDATE
		AutoDeleter<MtVector<Slope *>> deleter(slopeList);
#endif

		//タイルおよび坂情報が無い場合
		if (slopeList->count() == 0 && wallCollisionList.size() == 0) {
			continue;
		}

		// 移動先の壁と床をチェック
		bool hitWall, hitFloor;
		this->checkWallAndFloor(hitWall, hitFloor, wallCollisionList, oldWallCollisionList, slopeList, mv, omv, bLeftWallCollied, bRightWallCollied);

		// 強制停止を行うか？
		bool forceStop = false;

		// 壁に接触時
		if (hitWall) {
			//　ACT2-4863 テンプレート移動の左右移動で床にぶつかったときに反転しないように条件をつける。
			if (objCommand) {
				if (objCommand->getMoveType() != agtk::data::ObjectCommandTemplateMoveData::kMoveHorizontal
				&&  objCommand->getMoveType() != agtk::data::ObjectCommandTemplateMoveData::kMoveVertical) {
					if (objCommand->getMoveType() == agtk::data::ObjectCommandTemplateMoveData::kMoveRandom) {
						// ACT2-5101 壁にぶつかったと判定された場合に、ぶつかった方向の壁の方向に向かっていく場合にのみ、移動を停止させるように。
						auto v = this->getDirection();
						auto bit = _object->getTileWallBit();
						bool bDirectionCollided = false;
						//左
						if (v.x < 0 && (_object->getLeftWallObjectList()->count() || (bit & agtk::data::ObjectActionLinkConditionData::kWallBitLeft))) {
							bDirectionCollided = true;
						}
						//右
						else if (v.x > 0 && (_object->getRightWallObjectList()->count() || (bit & agtk::data::ObjectActionLinkConditionData::kWallBitRight))) {
							bDirectionCollided = true;
						}
						//下
						if (v.y < 0 && (_object->getDownWallObjectList()->count() || (bit & agtk::data::ObjectActionLinkConditionData::kWallBitDown))) {
							bDirectionCollided = true;
						}
						//上
						else if (v.y > 0 && (_object->getUpWallObjectList()->count() || (bit & agtk::data::ObjectActionLinkConditionData::kWallBitUp))) {
							bDirectionCollided = true;
						}
						if (bDirectionCollided) {
							cocos2d::log("bDirectionCollided");
							forceStop = true;
						}
					}
					else {
						forceStop = true;
					}
				}
			}
		}

		if (objCommand) {
			// 左右移動設定時
			if (objCommand->getMoveType() == agtk::data::ObjectCommandTemplateMoveData::kMoveHorizontal) {
				if (bLeftWallCollied || bRightWallCollied) {
					forceStop = true;
				}
				// 左右移動設定時、「段差から落ちない」設定時、床に接触できなかった場合
				if (!this->getGravity().isZero() && !objCommand->getFallFromStep() && !hitFloor) {
					forceStop = true;
				}
			}

			//左右移動設定時。
			if (objCommand->getMoveType() == agtk::data::ObjectCommandTemplateMoveData::kMoveVertical) {
				if((bUpWallCollied == true && _oldCollied.wall.up == false)
				|| (bDownWallCollied == true && _oldCollied.wall.down == false)) {
					forceStop = true;
				}
			}

			// 近くのオブジェクトへ移動、近くのオブジェクトから離れる、ランダム移動場合
			if (objCommand->getMoveType() == agtk::data::ObjectCommandTemplateMoveData::kMoveNearObject ||
				objCommand->getMoveType() == agtk::data::ObjectCommandTemplateMoveData::kMoveApartNearObject ||
				objCommand->getMoveType() == agtk::data::ObjectCommandTemplateMoveData::kMoveRandom) {

				// 床にいるとき、移動先が床に接触できない場合
				if (_object->_floor && gravity != Vec2::ZERO && !objCommand->getFallFromStep() && !hitFloor) {
					// 移動を止める
					_moveX = 0;
					_moveY = 0;
					_vertVelocity = cocos2d::Vec2::ZERO;
					forceStop = true;
				}
			}

			//バウンドの場合
			if (objCommand->getMoveType() == agtk::data::ObjectCommandTemplateMoveData::kMoveBound) {
				if (_object->_floor && gravity != Vec2::ZERO && !objCommand->getFallFromStep() && !hitFloor) {
					templateMove->setFallFromStepFlag(true);
					forceStop = true;
				}
			}
		}

		if (forceStop) {
			templateMove->endFrameCount();
			bForceLock = true;
			break;
		}
	}

	_oldCollied.wall.up = bUpWallCollied;
	_oldCollied.wall.down = bDownWallCollied;

	int tileWallBit = 0;
	cocos2d::__Array *tileList = cocos2d::__Array::create();

	cocos2d::Vec2 gn = gravity.getNormalized();
	if (gn == cocos2d::Vec2::ZERO) {
		return;
	}

	//CCLOG("move:%f,%f:%f,%f:%f,%f:%d,%d:%d:%d", move.x, move.y, _moveX, _moveY, player->getPosition().x, player->getPosition().y, _object->_falling, _object->_floor, tileList->count(), templateMove->locked());
	if (templateMove->locked()) {
		float angle = gn.getAngle();
		auto gv = agtk::GetRotateByAngle(gn, -angle);
		auto mv = agtk::GetRotateByAngle(move, -angle);
		//CCLOG("gv,mv:%f,%f:%f,%f", gv.x, gv.y, mv.x, mv.y);
		if (bForceLock) {
			_moveX = 0;
			_vertVelocity = cocos2d::Vec2::ZERO;
			templateMove->locked() = false;
		}
		else if (tileList->count() == 0 && (gv.x > 0 && mv.x > 0)) {
			//移動を強制的に止める。
			_moveX = 0;
			_vertVelocity = cocos2d::Vec2::ZERO;
			templateMove->locked() = false;
		}
		else if ((gv.x > 0 && mv.x > 0) && !(tileWallBit & agtk::data::ObjectActionLinkConditionData::kWallBitDown) && (_object->getTileWallBit() & agtk::data::ObjectActionLinkConditionData::kWallBitDown)) {
			_moveX = 0;
			_vertVelocity = cocos2d::Vec2::ZERO;
			templateMove->locked() = false;
		}
	}

	//落下中
	if (_object->_falling) {
		return;
	}

	auto projectData = GameManager::getInstance()->getProjectData();
	cocos2d::Ref *ref = nullptr;
	CCARRAY_FOREACH(tileList, ref) {
		Point center, boundMin, boundMax;
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
		auto tile = static_cast<agtk::Tile *>(ref);
#else
		auto tile = dynamic_cast<agtk::Tile *>(ref);
#endif
		getWallCenterAndBound(tile, &center, &boundMin, &boundMax);
		auto tileset = projectData->getTilesetData(tile->getTilesetId());
		int tileId = tile->getY() * tileset->getHorzTileCount() + tile->getX();
		int wallBit = tileset->getWallSetting(tileId);
		if (wallBit == 0x0f) {
			templateMove->setMoveInfo(true, this->getDirection());
		}
		else {
			if (wallBit & (1 << agtk::data::TilesetData::Up) && gn.y < 0) {
				templateMove->setMoveInfo(true, this->getDirection());
			}
			if (wallBit & (1 << agtk::data::TilesetData::Left) && gn.x < 0) {
				templateMove->setMoveInfo(true, this->getDirection());
			}
			if (wallBit & (1 << agtk::data::TilesetData::Right) && gn.x > 0) {
				templateMove->setMoveInfo(true, this->getDirection());
			}
			if (wallBit & (1 << agtk::data::TilesetData::Down) && gn.y > 0) {
				templateMove->setMoveInfo(true, this->getDirection());
			}
		}
	}
}

bool ObjectMovement::checkWallCollision(std::vector<Vertex4>& wallCollisionList, cocos2d::Vec2 move, cocos2d::__Array *collisionTileList, int *wallBit, WallHitInfo &wallHitInfo)
{
	auto sceneLayer = _object->getSceneLayer();
	Point center;
	Point boundMin;
	Point boundMax;
	getWallCenterAndBound(wallCollisionList, &center, &boundMin, &boundMax);

	auto tmpWallHitInfoRect = cocos2d::Rect(wallHitInfo.boundMin, cocos2d::Size(wallHitInfo.boundMax - wallHitInfo.boundMin));
	int tileWallBit = 0;
	cocos2d::Rect thresholdRect;

	//移動先のタイル壁判定情報を取得する。
	auto velocity = move;
	//タイルの壁判定に接触
	//WallHitInfo wallHitInfo;
	wallHitInfo.initHit();
	wallHitInfo.center = center;
	wallHitInfo.boundMin = boundMin;
	wallHitInfo.boundMax = boundMax;
	wallHitInfo.boundMin.x -= 1;
	wallHitInfo.boundMin.y -= 1;
	wallHitInfo.boundMax.x += 1;
	wallHitInfo.boundMax.y += 1;
	auto tileList = sceneLayer->getCollisionTileList(wallHitInfo.boundMin, wallHitInfo.boundMax);
	wallHitInfo.boundMin.x -= 1;
	wallHitInfo.initHit();
	thresholdRect = cocos2d::Rect(wallHitInfo.boundMin - tmpWallHitInfoRect.origin, wallHitInfo.getRect().size - tmpWallHitInfoRect.size);
	calcWallHitInfo(_object, wallHitInfo, velocity, nullptr, tileList, true, nullptr, thresholdRect);
	tileWallBit |= (wallHitInfo.hitLeft >= wallHitInfo.boundMin.x ? agtk::data::ObjectActionLinkConditionData::kWallBitLeft : 0);
	wallHitInfo.boundMin.x += 1;
	wallHitInfo.boundMin.y -= 1;
	wallHitInfo.initHit();
	thresholdRect = cocos2d::Rect(wallHitInfo.boundMin - tmpWallHitInfoRect.origin, wallHitInfo.getRect().size - tmpWallHitInfoRect.size);
	calcWallHitInfo(_object, wallHitInfo, velocity, nullptr, tileList, true, nullptr, thresholdRect);
	tileWallBit |= (wallHitInfo.hitDown >= wallHitInfo.boundMin.y ? agtk::data::ObjectActionLinkConditionData::kWallBitDown : 0);
	wallHitInfo.boundMin.y += 1;
	wallHitInfo.boundMax.x += 1;
	wallHitInfo.initHit();
	thresholdRect = cocos2d::Rect(wallHitInfo.boundMin - tmpWallHitInfoRect.origin, wallHitInfo.getRect().size - tmpWallHitInfoRect.size);
	calcWallHitInfo(_object, wallHitInfo, velocity, nullptr, tileList, true, nullptr, thresholdRect);
	tileWallBit |= (wallHitInfo.hitRight <= wallHitInfo.boundMax.x ? agtk::data::ObjectActionLinkConditionData::kWallBitRight : 0);
	wallHitInfo.boundMax.x -= 1;
	wallHitInfo.boundMax.y += 1;
	wallHitInfo.initHit();
	thresholdRect = cocos2d::Rect(wallHitInfo.boundMin - tmpWallHitInfoRect.origin, wallHitInfo.getRect().size - tmpWallHitInfoRect.size);
	calcWallHitInfo(_object, wallHitInfo, velocity, nullptr, tileList, true, nullptr, thresholdRect);
	tileWallBit |= (wallHitInfo.hitUp <= wallHitInfo.boundMax.y ? agtk::data::ObjectActionLinkConditionData::kWallBitUp : 0);

	if (wallBit) {
		*wallBit = tileWallBit;
	}
	if (collisionTileList) {
#ifdef USE_MULTITHREAD_OBJECT_UPDATE
		for (auto tile : tileList) {
			collisionTileList->addObject(tile);
		}
#else
		collisionTileList->addObjectsFromArray(tileList);
#endif
	}
	return tileWallBit != 0 ? true : false;
}

#ifdef USE_MULTITHREAD_OBJECT_UPDATE
void ObjectMovement::checkWallAndFloor(bool& hitWall, bool& hitFloor, std::vector<Vertex4> &wallCollisionList, std::vector<Vertex4> &oldWallCollisionList, agtk::MtVector<agtk::Slope *> *slopeList, cocos2d::Vec2& move, cocos2d::Vec2 oldMove, bool& bLeftWallCollied, bool& bRightWallCollied)
#else
void ObjectMovement::checkWallAndFloor(bool& hitWall, bool& hitFloor, std::vector<Vertex4> &wallCollisionList, std::vector<Vertex4> &oldWallCollisionList, cocos2d::__Array *slopeList, cocos2d::Vec2& move, cocos2d::Vec2& oldMove, bool& bLeftWallCollied, bool& bRightWallCollied)
#endif
{
	WallHitInfo wallHitInfo;
	hitWall = false;
	hitFloor = false;

	auto sceneLayer = _object->getSceneLayer();
	Point center;
	Point boundMin;
	Point boundMax;
	getWallCenterAndBound2(wallCollisionList, &center, &boundMin, &boundMax);

	wallHitInfo.initHit();
	wallHitInfo.center = center;
	wallHitInfo.boundMin = boundMin;
	wallHitInfo.boundMax = boundMax;

	//下りの坂の上にいる場合、タイル情報を得るためにY軸方向の移動量を加算する。
	if (_object->_floorSlope == true && oldMove.y < 0) {
		wallHitInfo.boundMin.y += oldMove.y;
		wallHitInfo.boundMax.y += oldMove.y;
	}

	//重力が無い場合。
	if (this->getGravity().isZero()) {
		wallHitInfo.boundMin += Point(TILE_COLLISION_THRESHOLD, TILE_COLLISION_THRESHOLD);
		wallHitInfo.boundMax -= Point(TILE_COLLISION_THRESHOLD, TILE_COLLISION_THRESHOLD);
	}

	auto objectData = _object->getObjectData();
	auto objectTemplateMove = _object->getObjectTemplateMove();
// #AGTK-NX
#if (CC_TARGET_PLATFORM == CC_PLATFORM_NX)
#else
	auto tileList = sceneLayer->getCollisionTileList(wallHitInfo.boundMin, wallHitInfo.boundMax);
#endif

	auto objRect = Rect(wallHitInfo.boundMin, Size(wallHitInfo.boundMax - wallHitInfo.boundMin));
	cocos2d::Ref* ref = nullptr;
	Rect wallRect;

	// 壁接触チェック
#ifdef USE_MULTITHREAD_OBJECT_UPDATE
	for(auto tile: tileList){
#else
	CCARRAY_FOREACH(tileList, ref) {
		auto tile = dynamic_cast<agtk::Tile*>(ref);
#endif

		if (!isNeedCheck(_object, tile)) {
			continue;
		}

		//テンプレート移動で「タイル判定を無視する」の場合
		if (objectTemplateMove->isIgnoredTileWall()) {
			continue;
		}

		auto rect = tile->convertToLayerSpaceRect();
		auto pos = rect.origin;
		auto size = rect.size;

		bool bSlopeConnect = false;
		if (slopeList && slopeList->count() > 0) {
#ifdef USE_MULTITHREAD_OBJECT_UPDATE
			bool bSkip = false;
			for (int i = 0; i < (int)slopeList->count(); i++) {
				auto slope = (*slopeList)[i];
#else
				cocos2d::Ref *ref2;
				bool bSkip = false;
				CCARRAY_FOREACH(slopeList, ref2) {
				auto slope = dynamic_cast<agtk::Slope *>(ref2);
#endif
				if (slope->checkConnectTileUp(rect, objRect, move, true)) {
					bSlopeConnect = true;
					break;
				}
			}
		}

		if (move.x != 0) {
			pos.y += TILE_COLLISION_THRESHOLD;
			size.height -= TILE_COLLISION_THRESHOLD * 2;
		}
		if (move.y != 0) {
			pos.x += TILE_COLLISION_THRESHOLD;
			size.width -= TILE_COLLISION_THRESHOLD * 2;
		}
		wallRect = Rect(pos, size);

		if (wallRect.intersectsRect(objRect)) {
			if (bSlopeConnect && move.x != 0.0f) {
				if (move.x < 0) bLeftWallCollied = false;//左
				if (move.x > 0) bRightWallCollied = false;//右
				continue;
			}
			hitWall = true;
			break;
		}
	}

	// 左移動の場合
	if (move.x < 0) {
		// 床チェックを行う位置をオブジェクトの矩形左端にする
		objRect.size.width = 1;
	}
	// 右移動の場合
	else if (move.x > 0) {
		// 床チェックを行う位置をオブジェクトの矩形右端にする
		objRect.origin.x = objRect.getMaxX();
		objRect.size.width = 1;
	}
	// 横移動がない場合
	else {
		// 何もしない
	}

	// 床接触チェック
	if (!this->getGravity().isZero()) {
		auto slopeRect = objRect;
		slopeRect.origin.y -= oldMove.y;

		objRect.origin.y -= (TILE_COLLISION_THRESHOLD + 1.0f);

#ifdef USE_MULTITHREAD_OBJECT_UPDATE
		for(auto tile: tileList){
#else
		CCARRAY_FOREACH(tileList, ref) {
			auto tile = dynamic_cast<agtk::Tile*>(ref);
#endif

			if (!isNeedCheck(_object, tile)) {
				continue;
			}

			auto rect = tile->convertToLayerSpaceRect();
			auto pos = rect.origin;
			auto size = rect.size;
			pos.x += TILE_COLLISION_THRESHOLD;
			pos.y += TILE_COLLISION_THRESHOLD;
			size.width -= TILE_COLLISION_THRESHOLD * 2;
			size.height -= TILE_COLLISION_THRESHOLD * 2;

			wallRect = Rect(pos, size);

			if (wallRect.intersectsRect(objRect)) {
				hitFloor = true;
				break;
			}
			//坂の上にいる場合。移動先のタイル情報を得るようにする。
			if (_object->_floorSlope == true) {
				if (wallRect.intersectsRect(slopeRect)) {
					hitFloor = true;
					break;
				}
			}
		}
	}

	//坂
	if (slopeList && slopeList->count() > 0) {
		getWallCenterAndBoundUseSlope(wallCollisionList, &center, &boundMin, &boundMax);
		objRect = Rect(boundMin, Size(boundMax - boundMin));
		cocos2d::Rect objRectOld;
		if (_object->_floorSlope == true) {
			getWallCenterAndBoundUseSlope(oldWallCollisionList, &center, &boundMin, &boundMax);
			objRectOld = Rect(boundMin, Size(boundMax - boundMin));
		}

		//前後方向の移動量
		std::function<bool(agtk::Slope *)> checkHitSlope = [&](agtk::Slope *slope) {
			if (slope->checkHitRect(objRect, true)) {
				return true;
			}
			else {
				if (slope->getWorldSpaceRect().intersectsRect(objRect)) {
					return true;
				}
			}
			if (_object->_floorSlope == true) {
				if (slope->checkHitRect(objRectOld, true)) {
					return true;
				}
				if (move.x > 0) {
					if (slope->getType() == agtk::Slope::kTypeUp && objRect.getMinX() < slope->getWorldEndPoint().x && objRect.getMinY() < slope->getWorldEndPoint().y) {
						return true;
					}
					if (slope->getType() == agtk::Slope::kTypeDown && objRect.getMinX() < slope->getWorldEndPoint().x && objRect.getMinY() > slope->getWorldEndPoint().y) {
						return true;
					}
				}
				if (move.x < 0) {
					if (slope->getType() == agtk::Slope::kTypeUp && slope->getWorldStartPoint().x < objRect.getMaxX() && slope->getWorldStartPoint().y < objRect.getMinY()) {
						return true;
					}
					if (slope->getType() == agtk::Slope::kTypeDown && slope->getWorldStartPoint().x < objRect.getMaxX() && slope->getWorldStartPoint().y > objRect.getMinY()) {
						return true;
					}
				}
				if (oldMove.x > 0) {
					if (slope->getType() == agtk::Slope::kTypeUp && objRectOld.getMinX() < slope->getWorldEndPoint().x && objRectOld.getMinY() < slope->getWorldEndPoint().y) {
						return true;
					}
					if (slope->getType() == agtk::Slope::kTypeDown && objRectOld.getMinX() < slope->getWorldEndPoint().x && objRectOld.getMinY() > slope->getWorldEndPoint().y) {
						return true;
					}
				}
				if (oldMove.x < 0) {
					if (slope->getType() == agtk::Slope::kTypeUp && slope->getWorldStartPoint().x < objRectOld.getMaxX() && slope->getWorldStartPoint().y < objRectOld.getMinY()) {
						return true;
					}
					if (slope->getType() == agtk::Slope::kTypeDown && slope->getWorldStartPoint().x < objRectOld.getMaxX() && slope->getWorldStartPoint().y > objRectOld.getMinY()) {
						return true;
					}
				}
			}
			return false;
		};

#ifdef USE_MULTITHREAD_OBJECT_UPDATE
		for (int i = 0; i < (int)slopeList->count(); i++) {
			auto slope = (*slopeList)[i];
#else
		CCARRAY_FOREACH(slopeList, ref) {
			auto slope = dynamic_cast<agtk::Slope *>(ref);
#endif
			if (checkHitSlope(slope)) {
				hitFloor = true;
				break;
			}
			else {
				if (slope->getConnectStartSlope()) {
					if (checkHitSlope(slope->getConnectStartSlope())) {
						hitFloor = true;
						break;
					}
				}
				if (slope->getCanMoveEndSlope()) {
					if (checkHitSlope(slope->getConnectEndSlope())) {
						hitFloor = true;
						break;
					}
				}
			}
		}
	}
}

void ObjectMovement::reset()
{
	this->setMoveX(0.0f);
	this->setMoveY(0.0f);
	this->setMoveVelocity(cocos2d::Vec2::ZERO);
	this->setVertVelocity(cocos2d::Vec2::ZERO);
	_direction = cocos2d::Vec2::ZERO;
	_directionForce = cocos2d::Vec2::ZERO;
	_directionForceFlag = false;
	_resetDirectionXFlag = false;
	_resetDirectionYFlag = false;
	_directionForceIgnoredChangeActionFlag = false;
	_ignoredJump = false;
	_ignoredGravity = false;
	this->getWallMoveSpeed()->reset();
	this->getWallJump()->reset();
	this->getWallSlip()->reset();
	this->getWallGravityEffect()->reset();
	this->getMoveVelocityList()->removeAllObjects();
//	this->getMoveSpeed()->setValue(0.0f);
//	this->getUpDownMoveSpeed()->setValue(0.0f);
//	this->getTurnSpeed()->setValue(0.0f);
	this->setWallMoveX(0.0f);
	this->setWallMoveY(0.0f);
}

void ObjectMovement::resetX()
{
	auto objectData = _object->getObjectData();
	auto moveType = objectData->getMoveType();
	if (moveType == agtk::data::ObjectData::kMoveCar || moveType == agtk::data::ObjectData::kMoveTank) {
		//ひとまず、戦車・車の場合は無視。ACT2-1021のため。
		return;
	}
	auto objectAction = _object->getCurrentObjectAction();
	if (objectAction->getObjectActionData()->getIgnoreMoveInput()) {
		//入力無効。ACT2-1290によるバグ修正。
		return;
	}
	if (!_resetDirectionXFlag) {
		float x = this->getDirection().x;
		_directionOld.x = x;
		_resetDirectionXFlag = true;
	}
	this->setMoveX(0.0f);
	_moveVelocity.x = 0;
	_vertVelocity.x = 0;

	// テンプレート移動の「バウンド」以外の場合
	if (!_object->getObjectTemplateMove()->isBoundMove() && !_directionForceIgnoredChangeActionFlag) {
		_direction.x = 0;
		_directionForce.x = 0;
	}

// #AGTK-NX #AGTK-WIN
#ifdef USE_30FPS_3
	// 中間フレーム以降の移動量をリセット
	_object->_halfMove.x = 0.0f;
#endif
}

void ObjectMovement::resetY()
{
	auto objectData = _object->getObjectData();
	auto moveType = objectData->getMoveType();
	if (moveType == agtk::data::ObjectData::kMoveCar || moveType == agtk::data::ObjectData::kMoveTank) {
		//ひとまず、戦車・車の場合は無視。ACT2-1021のため。
		return;
	}
	auto objectAction = _object->getCurrentObjectAction();
	if (objectAction->getObjectActionData()->getIgnoreMoveInput()) {
		//入力無効。ACT2-1290によるバグ修正。
		return;
	}

	//ジャンプ中かつ、タイルの壁判定にY軸方向からぶつかった場合。
	if (_object->_jumping && !objectData->getFallOnCollideWithWall() && this->getMoveY() > 0) {
		return;
	}

	if (!_resetDirectionYFlag) {
		float y = this->getDirection().y;
		_directionOld.y = y;
		_resetDirectionYFlag = true;
	}
	this->setMoveY(0.0f);
	_moveVelocity.y = 0;
	if (!_directionForceIgnoredChangeActionFlag) {
		_vertVelocity.y = 0;
	}

	// テンプレート移動の「バウンド」以外の場合
	if (!_object->getObjectTemplateMove()->isBoundMove() && !_directionForceIgnoredChangeActionFlag) {
		_direction.y = 0;
		_directionForce.y = 0;
	}

// #AGTK-NX #AGTK-WIN
#ifdef USE_30FPS_3
	// 中間フレーム以降の移動量をリセット
	_object->_halfMove.y = 0.0f;
#endif
}

cocos2d::Vec2 ObjectMovement::move(float dt)
{
	CC_ASSERT(_object);
	auto objectData = _object->getObjectData();
	auto objectAction = _object->getCurrentObjectAction();
	if (!objectAction) {
		return cocos2d::Vec2::ZERO;
	}
	auto objectActionData = objectAction->getObjectActionData();
	auto playObjectData = _object->getPlayObjectData();
// #AGTK-NX #AGTK-WIN
#if defined(USE_30FPS) && 0 // ObjectMovement::getTimeScale()がdtを考慮するようになったため無効化
	auto timeScale = this->getTimeScale(dt) * dt * FRAME60_RATE;
#else
	auto timeScale = this->getTimeScale(dt);
#endif

	//弾情報
	float bulletMoveSpeed = 1.0f;
	auto bullet = dynamic_cast<agtk::Bullet *>(_object);
	if (bullet != nullptr) {
		bulletMoveSpeed = bullet->getBulletLocus()->getMoveSpeed();
	}
	bool bFreeMove = this->isFreeMovingEnabled();//自由移動モード

	if ((objectData->getMoveType() == agtk::data::ObjectData::kMoveNormal && objectData->getNormalAccelMove())
		|| (objectData->getMoveType() == agtk::data::ObjectData::kMoveTank && objectData->getTankAccelMove())
		|| (objectData->getMoveType() == agtk::data::ObjectData::kMoveCar && objectData->getCarAccelMove())) {
		//「加速移動切り替え時間」変更。
		auto playData = _object->getPlayObjectData();
		auto variableData = playData->getVariableData(agtk::data::kObjectSystemVariableDurationToTakeOverAccelerationMoveSpeed);
		auto seconds = variableData->getValue();

		auto moveSpeed = this->getMoveSpeed();
		if (moveSpeed->isProcessing() && moveSpeed->getSeconds() != seconds) {
			moveSpeed->setSeconds(seconds);
		}
		auto upDownMoveSpeed = this->getUpDownMoveSpeed();
		if (upDownMoveSpeed->isProcessing() && upDownMoveSpeed->getSeconds() != seconds) {
			upDownMoveSpeed->setSeconds(seconds);
		}
		auto turnSpeed = this->getTurnSpeed();
		if (turnSpeed->isProcessing() && turnSpeed->getSeconds() != seconds) {
			turnSpeed->setSeconds(seconds);
		}
	}

	//前後方向の移動量
	std::function<cocos2d::Vec2(cocos2d::Vec2, bool)> calcForwardBackwardMove = [&](cocos2d::Vec2 inputDirection, bool bAccelMove) {
		cocos2d::Vec2 move = inputDirection.getNormalized();
		if (move != cocos2d::Vec2::ZERO) {
			if (move.y > 0) {//前方への移動量
				move.y *= bAccelMove ? objectData->getForwardAccel() : (objectData->getForwardMove() + this->getWallMoveSpeed()->get());
			}
			else if (move.y < 0) {//後方への移動量
				move.y *= bAccelMove ? objectData->getBackwardAccel() : (objectData->getBackwardMove() + this->getWallMoveSpeed()->get());
			}
		}
		return move;
	};
	//旋回処理(tank)
	std::function<cocos2d::Vec2(cocos2d::Vec2, cocos2d::Vec2)> calcTurnDirectionByTank = [&](cocos2d::Vec2 move, cocos2d::Vec2 direction) {
		auto turnSpeedRate = this->getTurnSpeedRate();
		auto moveSpeedRate = this->getMoveSpeedRate();
		if (move != cocos2d::Vec2::ZERO) {
			float degree = 0.0f;
			if (move.x > 0) {//右旋回の回転量
				degree = (objectData->getRightTurn() * turnSpeedRate) * moveSpeedRate * timeScale;
			}
			else if (move.x < 0) {//左旋回の回転量
				degree = -(objectData->getLeftTurn() * turnSpeedRate) * moveSpeedRate * timeScale;
			}
			if (degree != 0.0f) {
				auto dir = cocos2d::Vec2::forAngle(-CC_DEGREES_TO_RADIANS(degree));
				direction = direction.rotate(dir);
			}
		}
		return direction;
	};
	//旋回処理(car)
	std::function<cocos2d::Vec2(cocos2d::Vec2, cocos2d::Vec2)> calcTurnDirectionByCar = [&](cocos2d::Vec2 move, cocos2d::Vec2 direction) {
		auto turnSpeedRate = this->getTurnSpeedRate();
		if (move != cocos2d::Vec2::ZERO) {
			float degree = 0.0f;
			if (move.x > 0 && _moveVelocity != cocos2d::Vec2::ZERO) {//右旋回の回転量
				degree = (objectData->getRightTurn() * turnSpeedRate) * timeScale;
			}
			else if (move.x < 0 && _moveVelocity != cocos2d::Vec2::ZERO) {//左旋回の回転量
				degree = -(objectData->getLeftTurn() * turnSpeedRate) * timeScale;
			}
			if (degree != 0.0f) {
				auto dir = cocos2d::Vec2::forAngle(-CC_DEGREES_TO_RADIANS(degree));
				direction = direction.rotate(dir);
			}
		}
		return direction;
	};

	cocos2d::Vec2 vec = cocos2d::Vec2::ZERO;
	switch (objectData->getMoveType()) {
	case agtk::data::ObjectData::kMoveNormal: {//基本移動
		
		auto wallFriction = this->getWallFriction()->get() * 0.1f;
		auto wallMoveSpeed = this->getWallMoveSpeed()->get();
		if (objectData->getNormalAccelMove()) {//加速移動

			vec = _moveVelocity;
			cocos2d::Vec2 move = cocos2d::Vec2::ZERO;
			auto moveSpeedRate = this->getMoveSpeedRate();
			auto upDownMoveSpeedRate = this->getUpDownMoveSpeedRate();
			auto slipRate = this->getWallSlip()->get() * 0.01f;
			int wallMoveFlipX = (playObjectData->getVariableData(data::kObjectSystemVariableHorizontalMaxMove)->getValue() + wallMoveSpeed) >= 0 ? 1 : -1;
			int wallMoveFlipY = (playObjectData->getVariableData(data::kObjectSystemVariableVerticalMaxMove)->getValue() + wallMoveSpeed) >= 0 ? 1 : -1;

			auto direction = this->getDirection();
			bool accel_x = direction.x != 0.0f;// x軸加速するか？
			bool accel_y = direction.y != 0.0f;// y軸加速するか？
			if (objectData->isGroupPlayer()) 
			{// プレイヤーの場合は入力もチェックする
				accel_x &= (_object->getInputDirectionId() > 0 || this->_directionForceFlag || bullet);
				accel_y &= (_object->getInputDirectionId() > 0 || this->_directionForceFlag || bullet);
			}
			auto const accelFunc = [&playObjectData,&wallMoveSpeed,&bFreeMove,&bulletMoveSpeed,&timeScale,&slipRate]
									(float& vec, float& move, bool isAccel, float direction, float moveSpeedRate, int wallMoveFlip, int accelVariable, int maxMoveVarible, int decelVariable) 
			{
				if (isAccel) 
				{//加速
					move = playObjectData->getVariableData(accelVariable)->getValue() + wallMoveSpeed;//加速（入力あり）
					move = std::abs(move) * wallMoveFlip;//※移動方向と移動量を決める。
					move *= direction * moveSpeedRate;
					move *= bulletMoveSpeed;
					move *= timeScale;
					vec += move;
				}
				else 
				{//減速
					if (0.0f <= slipRate && slipRate <= 0.5f) {
						move = AGTK_LINEAR_INTERPOLATE(
							(playObjectData->getVariableData(maxMoveVarible)->getValue() + wallMoveSpeed) * moveSpeedRate,
							playObjectData->getVariableData(decelVariable)->getValue(),
							0.5f, slipRate
							);
					}
					else {
						CC_ASSERT(slipRate <= 1.0f);
						move = AGTK_LINEAR_INTERPOLATE(playObjectData->getVariableData(decelVariable)->getValue(), 0.0f, 0.5f, slipRate - 0.5f);
					}
					move *= bulletMoveSpeed;
					move *= timeScale;
					if (vec > 0) {
						vec -= move;
						if (vec < 0) vec = 0;
					}
					else if (vec < 0) {
						vec += move;
						if (vec > 0) vec = 0;
					}
				}
			};

			//自由移動モード
			if (bFreeMove) {
				moveSpeedRate = 1.0f;
				upDownMoveSpeedRate = 1.0f;
			}

			accelFunc(vec.x, move.x, accel_x, direction.x, moveSpeedRate, wallMoveFlipX, data::kObjectSystemVariableHorizontalAccel, data::kObjectSystemVariableHorizontalMaxMove, data::kObjectSystemVariableHorizontalDecel);
			accelFunc(vec.y, move.y, accel_y, direction.y, upDownMoveSpeedRate, wallMoveFlipY, data::kObjectSystemVariableVerticalAccel, data::kObjectSystemVariableVerticalMaxMove, data::kObjectSystemVariableVerticalDecel);


			//最大移動量
			float xMax = (playObjectData->getVariableData(data::kObjectSystemVariableHorizontalMaxMove)->getValue() + wallMoveSpeed) * moveSpeedRate;// *timeScale;
			float yMax = (playObjectData->getVariableData(data::kObjectSystemVariableVerticalMaxMove)->getValue() + wallMoveSpeed) * upDownMoveSpeedRate;// *timeScale;
			if (xMax >= 0) {
				if (vec.x < -xMax) {
					vec.x = -xMax;
				}
				else if (vec.x > xMax) {
					vec.x = xMax;
				}
			}
			else {
				if (vec.x > -xMax) {
					vec.x = -xMax;
				}
				else if (vec.x < xMax) {
					vec.x = xMax;
				}
			}
			if (yMax >= 0) {
				if (vec.y < -yMax) {
					vec.y = -yMax;
				}
				else if (vec.y > yMax) {
					vec.y = yMax;
				}
			}
			else {
				if (vec.y > -yMax) {
					vec.y = -yMax;
				}
				else if (vec.y < yMax) {
					vec.y = yMax;
				}
			}

			//自由移動モード(false)
			if (bFreeMove == false) {
				// 物理演算設定がある　かつ　物理演算環境の影響を受ける場合
				if (_object->getObjectData()->getPhysicsSettingFlag() && _object->getPlayObjectData()->getSwitchData(agtk::data::kObjectSystemSwitchAffectedByOtherObjects)->getValue()) {
					// 摩擦係数を最終速度に乗算する
					vec *= (1.0f - (wallFriction + _object->getObjectData()->getPhysicsSetting()->getFriction()) * 0.5f);
				}
			}
		}
		else {
			//x方向
			vec.x = playObjectData->getVariableData(data::kObjectSystemVariableHorizontalMove)->getValue() + wallMoveSpeed;
			//y方向
			vec.y = playObjectData->getVariableData(data::kObjectSystemVariableVerticalMove)->getValue() + wallMoveSpeed;

			//移動方向
			auto slipRate = this->getWallSlip()->get() * 0.01f;
			auto direction = this->getDirection();
			vec.x *= direction.x;
			vec.y *= direction.y;
			vec = this->slip(vec, (direction != cocos2d::Vec2::ZERO));

			if (!bFreeMove) {
				//自由移動モードでない。
				vec.x *= this->getMoveSpeedRate();
				vec.y *= this->getUpDownMoveSpeedRate();

				// 物理演算設定がある　かつ　物理演算環境の影響を受ける場合
				if (_object->getObjectData()->getPhysicsSettingFlag() && _object->getPlayObjectData()->getSwitchData(agtk::data::kObjectSystemSwitchAffectedByOtherObjects)->getValue()) {
					// 摩擦係数を最終速度に乗算する
					vec *= (1.0f - (wallFriction + _object->getObjectData()->getPhysicsSetting()->getFriction()) * 0.5f);
				}
			}
			vec *= bulletMoveSpeed;
			vec *= timeScale;
		}
		break; }
	case agtk::data::ObjectData::kMoveTank: {
		auto turnSpeedRate = this->getTurnSpeedRate();
		auto inputDirection = this->getInputDirection();

		//旋回処理。
		auto move = calcForwardBackwardMove(inputDirection, objectData->getTankAccelMove());
		_direction = calcTurnDirectionByTank(move, _direction);
		_directionForce = calcTurnDirectionByTank(move, _directionForce);

		if (objectData->getTankAccelMove()) {//加速移動
			move *= bulletMoveSpeed;
			if (move.y != 0.0f) {
				//加速
				cocos2d::Vec2 direction = this->getDirection();
				direction.normalize();
				float rotate = direction.getAngle(cocos2d::Vec2(0, 1));
				this->accelTankAndCarMoveVelocity(cocos2d::Vec2(0, move.y), rotate, objectData->getTankAccelMove());
			}
			else {
				//減速
				this->decelTankAndCarMoveVelocity(objectData->getTankAccelMove());
			}
			//移動量算出。
			this->updateMoveVelocity();
			vec = this->calcTankAndCarMoveVelocity(objectData->getTankAccelMove());
		}
		else {
			move = this->slip(move, (inputDirection != cocos2d::Vec2::ZERO));
			move *= bulletMoveSpeed;
			if (move.y != 0.0f) {
				//加速
				cocos2d::Vec2 direction = this->getDirection();
				direction.normalize();
				float rotate = direction.getAngle(cocos2d::Vec2(0, 1));
				this->accelTankAndCarMoveVelocity(cocos2d::Vec2(0, move.y), rotate, objectData->getTankAccelMove());
			}
			else {
				//減速
				this->decelTankAndCarMoveVelocity(objectData->getTankAccelMove());
			}
			//移動量算出。
			this->updateMoveVelocity();
			vec = this->calcTankAndCarMoveVelocity(objectData->getTankAccelMove());
		}
		vec *= timeScale;
		vec = vec.rotateByAngle(cocos2d::Vec2::ZERO, -CC_DEGREES_TO_RADIANS(_object->getRotation()));
		break; }
	case agtk::data::ObjectData::kMoveCar: {
		auto turnSpeedRate = this->getTurnSpeedRate();
		auto inputDirection = this->getInputDirection();

		//旋回処理。
		auto move = calcForwardBackwardMove(inputDirection, objectData->getCarAccelMove());
		_direction = calcTurnDirectionByCar(move, _direction);
		_directionForce = calcTurnDirectionByCar(move, _directionForce);

		if (objectData->getCarAccelMove()) {//加速移動
			move *= bulletMoveSpeed;
			if (move.y != 0.0f) {
				//加速
				cocos2d::Vec2 direction = this->getDirection();
				float rotate = direction.getAngle(cocos2d::Vec2(0, 1));
				this->accelTankAndCarMoveVelocity(cocos2d::Vec2(0, move.y), rotate, objectData->getCarAccelMove());
			}
			else {
				//減速
				this->decelTankAndCarMoveVelocity(objectData->getCarAccelMove());
			}
			//移動量算出。
			this->updateMoveVelocity();
			vec = this->calcTankAndCarMoveVelocity(objectData->getCarAccelMove());
		}
		else {
			move = this->slip(move, (inputDirection != cocos2d::Vec2::ZERO));
			move *= bulletMoveSpeed;
			if (move.y != 0.0f) {
				//加速
				cocos2d::Vec2 direction = this->getDirection();
				float rotate = direction.getAngle(cocos2d::Vec2(0, 1));
				this->accelTankAndCarMoveVelocity(cocos2d::Vec2(0, move.y), rotate, objectData->getCarAccelMove());
			}
			else {
				//減速
				this->decelTankAndCarMoveVelocity(objectData->getCarAccelMove());
			}
			//移動量算出。
			this->updateMoveVelocity();
			vec = this->calcTankAndCarMoveVelocity(objectData->getCarAccelMove());
		}
		vec *= timeScale;
		vec = vec.rotateByAngle(cocos2d::Vec2::ZERO, -CC_DEGREES_TO_RADIANS(_object->getRotation()));
		break; }
	}

	// タイル設定（オブジェクトをX、Y方向に移動（±））
	vec.x += this->getWallMoveX();
	vec.y += this->getWallMoveY();

	//「その他の実行アクション」の「表示方向と同じ方へ移動」
	if (_displayDistanceMove.isExecuting()) {
		vec = _displayDistanceMove.add(vec);
		if (_displayDistanceMove.isEnd()) {
			_displayDistanceMove.end();
			resetDirectionForce();
			resetInputDirectionForce();
		}
	}

	_moveVelocity = vec;

	//極座標計算
	if (objectData->getMoveType() == agtk::data::ObjectData::kMoveNormal) {//基本移動
		if (objectData->getDiagonalMoveWithPolar() && this->_direction != cocos2d::Vec2::ZERO) {
			vec = this->calcDiagonalMoveWithPolar(vec, this->getDirection());
		}
	}

	//加速移動
	if ((objectData->getMoveType() == agtk::data::ObjectData::kMoveNormal && objectData->getNormalAccelMove())
	|| (objectData->getMoveType() == agtk::data::ObjectData::kMoveTank && objectData->getTankAccelMove())
	|| (objectData->getMoveType() == agtk::data::ObjectData::kMoveCar && objectData->getCarAccelMove())) {
		vec *= timeScale;
	}
	if (_distanceMax >= 0) {
		//移動距離制限。
		auto move = vec.getLength();
		if (_movedDistance + move >= _distanceMax && move > 0.0f) {
			auto scaling = (_distanceMax - _movedDistance) / move;
			vec *= scaling;
			resetDirectionForce();
			resetInputDirectionForce();
		}
		else {
			_movedDistance += move;
		}
	}

	_moveSideBit = (vec.x < 0 ? agtk::data::ObjectActionLinkConditionData::kWallBitLeft : 0)
		| (vec.x > 0 ? agtk::data::ObjectActionLinkConditionData::kWallBitRight : 0)
		| (vec.y > 0 ? agtk::data::ObjectActionLinkConditionData::kWallBitUp : 0)
		| (vec.y < 0 ? agtk::data::ObjectActionLinkConditionData::kWallBitDown : 0);
	return vec;
}

cocos2d::Vec2 ObjectMovement::jump(float dt)
{
	if (this->getIgnoredJump()) {
		//無効
		return cocos2d::Vec2::ZERO;
	}
	CC_ASSERT(_object);
	cocos2d::Vec2 vec = cocos2d::Vec2::ZERO;
	auto objectData = _object->getObjectData();
	auto objectAction = _object->getCurrentObjectAction();
	if (!objectAction) {
		return cocos2d::Vec2::ZERO;
	}
	auto objectActionData = objectAction->getObjectActionData();
	auto timeScale = this->getTimeScale(dt); 
	//auto timeScale = _object->getTimeScale();
	auto playObjectData = _object->getPlayObjectData();

	//自由移動モード
	if (this->isFreeMovingEnabled()) {
		return vec;
	}

	auto wallJump = this->getWallJump();
	if (objectActionData->getJumpable() || _object->getJumpActionFlag()) {
		if (objectActionData->getEnableCustomizedJump()) {
			int duration300 = _jumpDuration * 300;
			int inputDuration300 = objectData->getAffectInputDuration300();
			if (duration300 <= inputDuration300 && _object->getJumpActionFlag()) {
				wallJump->lock();
				bool ret = false;
				auto button = objectData->getJumpInputOperationKeyId();
				ret = InputManager::getInstance()->isPressed(button, _object->getControllerId());//※とりあずAボタンです。ボタン指定のカラムがあるので作成されたら変更。
				cocos2d::Vec2 jump = cocos2d::Vec2(0, playObjectData->getVariableData(data::kObjectSystemVariableInitialJumpSpeed)->getValue() + wallJump->get());
				if (inputDuration300 > 0) {
					int dt300 = (int)(dt * 300);
					if (dt300 > inputDuration300) {
						dt300 = inputDuration300;
						ret = false;
					}
				}
				jump *= objectActionData->getJumpSpeed() * 0.01f;
				vec = jump * timeScale;
				if (ret == false || inputDuration300 == 0) {
					_object->setJumpActionFlag(false);
					wallJump->unlock();
				}
				_vertVelocity = cocos2d::Vec2::ZERO;
				_object->_jumping = true;//ジャンプON!
				_object->_jumpInputFlag = true;//ジャンプ入力ON!
				_jumpStartFloor = true;
				
				this->setFixedJumpDirectionId(_object->getInputDirectionId());
			}
			else {
				_object->setJumpActionFlag(false);
				wallJump->unlock();
			}
		}
		else {
			if (_object->getJumpActionFlag()) {
				cocos2d::Vec2 jump = cocos2d::Vec2(0, playObjectData->getVariableData(data::kObjectSystemVariableInitialJumpSpeed)->getValue() + wallJump->get());
				jump *= objectActionData->getJumpSpeed() * 0.01f;
				vec = jump * timeScale;
				_object->setJumpActionFlag(false);
				_vertVelocity = cocos2d::Vec2::ZERO;
				wallJump->unlock();
				_object->_jumping = true;//ジャンプON!
				_object->_jumpInputFlag = true;//ジャンプ入力ON!
				_jumpStartFloor = true;

				this->setFixedJumpDirectionId(_object->getInputDirectionId());
			}
		}
	}
	if (_object->getJumpActionFlag()) {
		_jumpDuration += dt;
	}
	return vec;
}

cocos2d::Vec2 ObjectMovement::gravity(float dt)
{
	if (this->getIgnoredGravity()) {
		//無効
		return cocos2d::Vec2::ZERO;
	}
	CC_ASSERT(_object);
	auto objectAction = _object->getCurrentObjectAction();
	if (!objectAction) {
		return cocos2d::Vec2::ZERO;
	}
	auto objectActionData = objectAction->getObjectActionData();
	auto timeScale = this->getTimeScale(dt);
	cocos2d::Vec2 gravity = cocos2d::Vec2::ZERO;

	//自由移動モード
	if (this->isFreeMovingEnabled()) {
		return gravity;
	}
	if ((_object->getJumpActionFlag() == false) || (_object->_jumping == false && _object->_collision == false)) {
		//地面に接している。
		auto objectData = _object->getObjectData();
		auto scene = GameManager::getInstance()->getCurrentScene();
		gravity = scene->getGravity()->getGravity() * (objectData->getGravity() * 0.01f) * (this->getWallGravityEffect()->get() * 0.01f);
		if (objectActionData->getIgnoreGravity()) {
			if (objectActionData->getGravity() == 100.0f) {
				gravity *= 0.0f;
			}
			else if (objectActionData->getGravity() != 0.0f) {
				gravity = gravity * (100.0f - objectActionData->getGravity()) * 0.01f;
			}
		}
		//弾の設定（重力の影響を受けない）
		auto bullet = dynamic_cast<agtk::Bullet *>(_object);
		if (bullet) {
			auto data = bullet->getObjectFireBulletSettingData();
			if (data->getFreeBulletGravityFlag()) {
				if (data->getFreeBulletGravity() == 100.0f) {
					gravity *= 0.0f;
				}
				else if (data->getFreeBulletGravity() != 0.0f) {
					gravity = gravity * (100.0f - data->getFreeBulletGravity()) * 0.01f;
				}
			}
		}
	}
	// 移動量にタイムスケール値をかけておく
	//! (重力は加速度なのでタイムスケールを2回かける必要がある)
	gravity *= (timeScale * timeScale);
	return gravity;
}

float ObjectMovement::water()
{
	//自由移動モード
	if (this->isFreeMovingEnabled()) {
		return 1.0f;
	}

	auto scene = GameManager::getInstance()->getCurrentScene();
	float water = scene->getWater()->getWater();
	if (water > 0.0f) {
		water = (100.0f - water) * 0.01f;
		if (water < 0) water = 0;
	}
	else {
		water = 1.0f;
	}
	return water;
}

bool ObjectMovement::isFreeMovingEnabled()
{
	if (DebugManager::getInstance()->getFreeMovingEnabled() || 
		_object->getPlayObjectData()->getSwitchData(agtk::data::kObjectSystemSwitchFreeMove)->getValue()) 
	{
		auto objectData = _object->getObjectData();
		if (objectData->isGroupPlayer()) {
			return true;
		}
	}
	return false;
}

bool ObjectMovement::isAcceleration()
{
	CC_ASSERT(_object);
	auto objectData = _object->getObjectData();
	//normal
	if (objectData->getMoveType() == agtk::data::ObjectData::kMoveNormal && objectData->getNormalAccelMove() == true) {
		return true;
	}
	//car
	if (objectData->getMoveType() == agtk::data::ObjectData::kMoveCar && objectData->getCarAccelMove() == true) {
		return true;
	}
	//tank
	if (objectData->getMoveType() == agtk::data::ObjectData::kMoveTank && objectData->getTankAccelMove() == true) {
		return true;
	}
	return false;
}

cocos2d::Vec2 ObjectMovement::slip(cocos2d::Vec2 v, bool bDirection)
{
	//スリップ処理
	auto wallSlip = this->getWallSlip();
	if (wallSlip->isValue()) {
		if (bDirection == false) {
			if (_slipVelocity != cocos2d::Vec2::ZERO) {
				auto old = _slipVelocity;
				float wallSlipRate = wallSlip->get() * 0.01f;
				cocos2d::Vec2 _v;
				_v.x = AGTK_LINEAR_INTERPOLATE(_slipVelocityMax.x, 0.0f, 1.0f, wallSlipRate);
				_v.y = AGTK_LINEAR_INTERPOLATE(_slipVelocityMax.y, 0.0f, 1.0f, wallSlipRate);
				_slipVelocity.x += (_slipVelocity.x > 0.0f) ? -_v.x : _v.x;
				_slipVelocity.y += (_slipVelocity.y > 0.0f) ? -_v.y : _v.y;
				if (_slipVelocity.x > 0.0f && old.x < 0.0f || old.x > 0.0f && _slipVelocity.x < 0.0f) {
					_slipVelocity.x = 0.0f;
				}
				if (_slipVelocity.y > 0.0f && old.y < 0.0f || old.y > 0.0f && _slipVelocity.y < 0.0f) {
					_slipVelocity.y = 0.0f;
				}
			}
		}
		else {
			_slipVelocity = v;
			_slipVelocityMax = cocos2d::Vec2(std::abs(v.x), std::abs(v.y));
		}
	}
	else {
		_slipVelocity = v;
		_slipVelocityMax = cocos2d::Vec2(std::abs(v.x), std::abs(v.y));
	}
	return _slipVelocity;
}

void ObjectMovement::setDirection(cocos2d::Vec2 v)
{
	_directionOld = this->getDirection();
	_direction = v;
	_resetDirectionXFlag = _resetDirectionYFlag = false;
}

void ObjectMovement::setDirectionForce(cocos2d::Vec2 v, bool bIgnoredChangeAction)
{
	_directionOld = this->getDirection();
	_directionForce = v;
	_directionForceFlag = true;
	_resetDirectionXFlag = _resetDirectionYFlag = false;
	_directionForceIgnoredChangeActionFlag = bIgnoredChangeAction;
}

void ObjectMovement::resetDirectionForce()
{
	if (!_resetDirectionXFlag) {
		float x = this->getDirection().x;
		_directionOld.x = x;
		_resetDirectionXFlag = true;
	}
	if (!_resetDirectionYFlag) {
		float y = this->getDirection().y;
		_directionOld.y = y;
		_resetDirectionYFlag = true;
	}
	_directionForce = cocos2d::Vec2::ZERO;
	_directionForceFlag = false;
	_directionForceIgnoredChangeActionFlag = false;
	//※表示移動距離をここでクリアする。
	_displayDistanceMove.clear();
	//移動距離制限をここでクリアする。
	_distanceMax = -1;
}

cocos2d::Vec2 ObjectMovement::getDirection()
{
	return _directionForceFlag ? _directionForce : _direction;
}

void ObjectMovement::setInputDirection(cocos2d::Vec2 v)
{
	_inputDirection = v;
	_inputDirectionForceFlag = false;
}

void ObjectMovement::setInputDirectionForce(cocos2d::Vec2 v)
{
	_inputDirectionForce = v;
	_inputDirectionForceFlag = true;
}

void ObjectMovement::resetInputDirectionForce()
{
	_inputDirectionForce = cocos2d::Vec2::ZERO;
	_inputDirectionForceFlag = false;
}

cocos2d::Vec2 ObjectMovement::getInputDirection()
{
	return _inputDirectionForceFlag ? _inputDirectionForce : _inputDirection;
}

void ObjectMovement::startForceMoveTime(cocos2d::Vec2 startPosition, cocos2d::Vec2 endPosition, float speed, bool finalGridMagnet)
{
	this->getForceMove()->startTime(startPosition, endPosition, speed, finalGridMagnet);
}

void ObjectMovement::startForceMoveParam(cocos2d::Vec2 startPosition, cocos2d::Vec2 endPosition, float changeMoveSpeed, bool finalGridMagnet, bool moveDirection, cocos2d::Vec2 direction)
{
	if (direction == cocos2d::Vec2::ZERO) {
		this->setDirectionForce((endPosition - startPosition).getNormalized());
	}
	else {
		this->setDirectionForce(direction);
	}
	this->setVertVelocity(cocos2d::Vec2::ZERO);
	auto forceMove = this->getForceMove();
	forceMove->startParam(startPosition, endPosition, changeMoveSpeed, finalGridMagnet, moveDirection, direction);
}

void ObjectMovement::startForceMovePushPull(cocos2d::Vec2 startPosition, cocos2d::Vec2 endPosition)
{
	this->getForceMove()->startPushPull(startPosition, endPosition);
}

void ObjectMovement::setForceMoveChangeMoveSpeed(float changeMoveSpeed)
{
	this->getForceMove()->setChangeMoveSpeed(changeMoveSpeed);
}

void ObjectMovement::accelTankAndCarMoveVelocity(cocos2d::Vec2 move, float rotate, bool bAccelMove)
{
	//move:移動量（前方後方の移動量）
	//rotate::移動方向
	auto moveVelocityList = this->getMoveVelocityList();
	MoveElement *new_me = MoveElement::create(move, rotate);

	float forwardCurrentMove = 0.0f;
	float backwardCurrentMove = 0.0f;
	calcForwardBackwardMoveLength(forwardCurrentMove, backwardCurrentMove);

	cocos2d::Vec2 new_ve = new_me->getMove();
	float new_length = new_me->getMove().length();

	//向きが反対方向の打消し。
	cocos2d::Ref *ref;
	CCARRAY_FOREACH(moveVelocityList, ref) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
		auto ve = static_cast<MoveElement *>(ref);
#else
		auto ve = dynamic_cast<MoveElement *>(ref);
#endif
		cocos2d::Vec2 vn = ve->getMove().getNormalized();
		float length = ve->getMove().length();
		float tmpLen = length;
		if ((vn.y > 0 && new_ve.y < 0) || (vn.y < 0 && new_ve.y > 0)) {
			if (new_length > 0) {
				length -= new_length;
				if (length < 0) length = 0;
				new_length -= tmpLen;
				if (new_length < 0) {
					new_length = 0;
				}
			}
		}
		ve->setMove(vn * length);
	}

	//順行方向の場合は、最大サイズを超えている場合は方向の更新。
	new_ve = new_me->getMove();
	new_length = new_me->getMove().length();
	if (new_length < MIN_TANK_AND_CAR_MOVE_VELOCITY) {
		return;
	}
	calcForwardBackwardMoveLength(forwardCurrentMove, backwardCurrentMove);

	auto objectData = _object->getObjectData();
	auto objectActionData = _object->getCurrentObjectAction()->getObjectActionData();
	float forwardMove = 0.0f;
	float backwardMove = 0.0f;
	float wallMoveSpeed = this->getWallMoveSpeed()->get();
	auto upDownMoveSpeedRate = objectActionData->getUpDownMoveSpeed() * 0.01f;
	float forwardMaxMove = bAccelMove ? (objectData->getForwardMaxMove() + wallMoveSpeed) : (objectData->getForwardMove() + wallMoveSpeed);
	float backwardMaxMove = bAccelMove ? (objectData->getBackwardMaxMove() + wallMoveSpeed) : (objectData->getBackwardMove() + wallMoveSpeed);
	forwardMaxMove *= upDownMoveSpeedRate;
	backwardMaxMove *= upDownMoveSpeedRate;

	if (forwardCurrentMove + new_length >= forwardMaxMove) {
		CCARRAY_FOREACH(moveVelocityList, ref) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
			auto ve = static_cast<MoveElement *>(ref);
#else
			auto ve = dynamic_cast<MoveElement *>(ref);
#endif
			cocos2d::Vec2 vn = ve->getMove().getNormalized();
			float length = ve->getMove().length();
			float tmpLen = length;
			if (new_length > 0) {
				length -= new_length;
				if (length < 0) length = 0;
				new_length -= tmpLen;
				if (new_length < 0) {
					new_length = 0;
				}
			}
			ve->setMove(vn * length);
		}
	}
	if (new_me->getMove().length() >= MIN_TANK_AND_CAR_MOVE_VELOCITY) {
		moveVelocityList->addObject(new_me);
	}
}

void ObjectMovement::decelTankAndCarMoveVelocity(bool bAccelMove)
{
	auto objectData = _object->getObjectData();
	auto objectActionData = _object->getCurrentObjectAction()->getObjectActionData();

	float wallMoveSpeed = this->getWallMoveSpeed()->get();
	auto moveSpeedRate = objectActionData->getMoveSpeed() * 0.01f;
	float slipRate = this->getWallSlip()->get() * 0.01f;
	float forwardLength = 0.0f;
	float backwardLength = 0.0f;
	if (bAccelMove) {
		if (0.0f <= slipRate && slipRate <= 0.5f) {
			forwardLength = AGTK_LINEAR_INTERPOLATE((objectData->getForwardMaxMove() + wallMoveSpeed) * moveSpeedRate, objectData->getForwardDecel(), 0.5f, slipRate);
			backwardLength = AGTK_LINEAR_INTERPOLATE((objectData->getBackwardMaxMove() + wallMoveSpeed) * moveSpeedRate, objectData->getBackwardDecel(), 0.5f, slipRate);
		}
		else {
			CC_ASSERT(slipRate <= 1.0f);
			forwardLength = AGTK_LINEAR_INTERPOLATE(objectData->getForwardDecel(), 0, 0.5f, slipRate - 0.5f);
			backwardLength = AGTK_LINEAR_INTERPOLATE(objectData->getBackwardDecel(), 0, 0.5f, slipRate - 0.5f);
		}
	}
	else {
		forwardLength = (objectData->getForwardMove() + wallMoveSpeed) * moveSpeedRate;
		backwardLength = (objectData->getBackwardMove() + wallMoveSpeed) * moveSpeedRate;
	}

	auto moveVelocityList = this->getMoveVelocityList();
	cocos2d::Ref *ref = nullptr;
	CCARRAY_FOREACH(moveVelocityList, ref) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
		auto ve = static_cast<MoveElement *>(ref);
#else
		auto ve = dynamic_cast<MoveElement *>(ref);
#endif
		cocos2d::Vec2 vn = ve->getMove().getNormalized();
		float length = ve->getMove().length();
		float tmpLen = length;
		if (vn.y > 0 && forwardLength > 0) {//前方
			length -= forwardLength;
			if (length < 0) length = 0;
			forwardLength -= tmpLen;
			if (forwardLength < 0) forwardLength = 0;
		}
		if (vn.y < 0 && backwardLength > 0) {//後方
			length -= backwardLength;
			if (length < 0) length = 0;
			backwardLength -= tmpLen;
			if (backwardLength < 0) backwardLength = 0;
		}
		ve->setMove(vn * length);
	}
}

cocos2d::Vec2 ObjectMovement::calcTankAndCarMoveVelocity(bool bAccelMove)
{
	cocos2d::Vec2 move = cocos2d::Vec2::ZERO;
	cocos2d::Ref *ref = nullptr;

	auto objectData = _object->getObjectData();
	auto objectActionData = _object->getCurrentObjectAction()->getObjectActionData();
	float forwardMove = 0.0f;
	float backwardMove = 0.0f;
	float wallMoveSpeed = this->getWallMoveSpeed()->get();
	auto upDownMoveSpeedRate = objectActionData->getUpDownMoveSpeed() * 0.01f;
	float forwardMaxMove = bAccelMove ? (objectData->getForwardMaxMove() + wallMoveSpeed) : (objectData->getForwardMove() + wallMoveSpeed);
	float backwardMaxMove = bAccelMove ? (objectData->getBackwardMaxMove() + wallMoveSpeed) : (objectData->getBackwardMove() + wallMoveSpeed);
	forwardMaxMove *= upDownMoveSpeedRate;
	backwardMaxMove *= upDownMoveSpeedRate;

	float forwardCurrentMove = 0.0f;
	float backwardCurrentMove = 0.0f;
	calcForwardBackwardMoveLength(forwardCurrentMove, backwardCurrentMove);

	auto moveVelocityList = this->getMoveVelocityList();
	CCARRAY_FOREACH(moveVelocityList, ref) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
		auto e = static_cast<MoveElement *>(ref);
#else
		auto e = dynamic_cast<MoveElement *>(ref);
#endif
		auto mv = e->getMove();
		auto len = e->getMove().length();
		if (len < MIN_TANK_AND_CAR_MOVE_VELOCITY) {
			continue;
		}
		//前方
		if (mv.y > 0) {
			if (forwardCurrentMove > forwardMaxMove) {
				auto n = mv.getNormalized();
				auto subMove = (forwardCurrentMove - forwardMaxMove);
				forwardCurrentMove -= len;
				len -= subMove;
				if (len < 0) len = 0;
				e->setMove(n * len);
			}
		}
		//後方
		if (mv.y < 0) {
			if (backwardCurrentMove > backwardMaxMove) {
				auto n = mv.getNormalized();
				auto subMove = (backwardCurrentMove - backwardMaxMove);
				backwardCurrentMove -= len;
				len -= subMove;
				if (len < 0) len = 0;
				e->setMove(n * len);
			}
		}
		if (len > 0) {
			move += e->calcMove();
		}
	}

	return move;
}

void ObjectMovement::calcForwardBackwardMoveLength(float &forwardMoveLength, float &backwardMoveLength)
{
	auto moveVelocityList = this->getMoveVelocityList();
	cocos2d::Ref *ref;
	forwardMoveLength = 0.0f;
	backwardMoveLength = 0.0f;
	CCARRAY_FOREACH(moveVelocityList, ref) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
		auto e = static_cast<MoveElement *>(ref);
#else
		auto e = dynamic_cast<MoveElement *>(ref);
#endif
		auto mv = e->getMove();
		auto len = e->getMove().length();
		if (mv.y > 0) forwardMoveLength += len;
		if (mv.y < 0) backwardMoveLength += len;
	}
}

void ObjectMovement::updateMoveVelocity()
{
	while (1) {
		cocos2d::Ref *ref = nullptr;
		cocos2d::__Array *list = this->getMoveVelocityList();
		bool remove = false;
		CCARRAY_FOREACH(list, ref) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
			auto ve = static_cast<MoveElement *>(ref);
#else
			auto ve = dynamic_cast<MoveElement *>(ref);
#endif
			float length = ve->getMove().length();
			if (length < MIN_TANK_AND_CAR_MOVE_VELOCITY) {
				list->removeObject(ve);
				remove = true;
				break;
			}
		}
		if (!remove) {
			break;
		}
	}
}

cocos2d::Vec2 ObjectMovement::calcDiagonalMoveWithPolar(cocos2d::Vec2 velocity, cocos2d::Vec2 direction)
{
	std::function<cocos2d::Vec2(float, float, float)> calcEllipsePoint = [&](float radiusX, float radiusY, float degrees) {
		cocos2d::Vec2 v;
		float rads = CC_DEGREES_TO_RADIANS(degrees);
		float distance = sqrt(pow(sinf(rads) * radiusY, 2) + pow(cosf(rads) * radiusX, 2));
		float a = atan2(sinf(rads) * radiusY, cosf(rads) * radiusX);
		v.x = distance * cosf(a);
		v.y = distance * sinf(a);
		return v;
	};
	auto newVelocity = velocity;
	if (direction.x == 1.0f && direction.y == 1.0f) {//45
		newVelocity = calcEllipsePoint(abs(velocity.x), abs(velocity.y), 45);
	}
	else if (direction.x == -1.0f && direction.y == 1.0f) {//135
		newVelocity = calcEllipsePoint(abs(velocity.x), abs(velocity.y), 135);
	}
	else if (direction.x == -1.0f && direction.y == -1.0f) {//225
		newVelocity = calcEllipsePoint(abs(velocity.x), abs(velocity.y), 225);
	}
	else if (direction.x == 1.0f && direction.y == -1.0f) {//315
		newVelocity = calcEllipsePoint(abs(velocity.x), abs(velocity.y), 315);
	}
	return newVelocity;
}

void ObjectMovement::setDisplayDistanceMove(cocos2d::Vec2 direction, float distance)
{
	this->setDirectionForce(direction, true);
	_displayDistanceMove.start(distance);
}

#ifdef USE_MULTITHREAD_OBJECT_UPDATE
bool ObjectMovement::isMoveLift(agtk::MtVector<agtk::Object *> *list)
#else
bool ObjectMovement::isMoveLift(cocos2d::__Array *list)
#endif
{
	auto objectMoveLiftList = this->getObjectMoveLiftList();
	if (nullptr != list) {
#ifdef USE_MULTITHREAD_OBJECT_UPDATE
		auto size = list->size();
		for(int i = 0; i < (int)size; i++){
			auto p = (*list)[i];
#else
		cocos2d::Ref *ref;
		CCARRAY_FOREACH(list, ref) {
			auto p = dynamic_cast<agtk::Object *>(ref);
#endif
			if (objectMoveLiftList->containsObject(p)) {
				return true;
			}
		}
		return false;
	}
	return objectMoveLiftList->count() > 0 ? true : false;
}

float ObjectMovement::getTimeScale(float dt)
{
	float timeScale = _object->getTimeScale();
	if (timeScale > 0.0f) {
		timeScale *= (dt * FRAME60_RATE) / timeScale;
	}
	return timeScale;
}

//-------------------------------------------------------------------------------------------------------------------
int Object::_objectCount = 0;
Object::Object()
{
#ifdef USE_MULTITHREAD_OBJECT_UPDATE
#ifdef USE_MULTITHREAD_MEASURE
	_destructed = false;
#endif
	_updating = false;
#endif
	_objectCount++;
	_sceneData = nullptr;
	_scenePartObjectData = nullptr;
	_sceneLayer = nullptr;
	_objectData = nullptr;
	_objectReappearData = nullptr;
	_currentObjectAction = nullptr;
	_objectActionList = nullptr;
	_basePlayer = nullptr;
	_player = nullptr;
	_jumping = false;
	_jumpTop = false;
	_falling = false;
	_fallingOld = false;
	_floor = false;
	_floorOld = false;
	_reJumpFlag = false;
	_collision = false;
	_objectCollision = nullptr;
	_objectWallCollision = nullptr;
	_tileWallBit = 0;
	_objectWallBit = 0;
	_objectSameLayerWallBit = 0;
#ifdef USE_MULTITHREAD_OBJECT_UPDATE
	//_prevObjectSameLayerWallBit = 0;
	_changeActionFlag = false;
#endif
	_slopeBit = 0;
	_aheadSlopeBit = 0;
	_layerId = -1;
	_returnedPos = cocos2d::Vec2::ZERO;
	_leftWallObjectList = nullptr;
	_rightWallObjectList = nullptr;
	_upWallObjectList = nullptr;
	_downWallObjectList = nullptr;
	_slopeTouchedList = nullptr;
	_passableSlopeTouchedList = nullptr;
	_slopeTouchedFrame = 0;
	_initialActionId = -1;
	_initialMoveDirection = -1;
	_moveDirection = -1;
	_moveDirectionOld = -1;
	_dispDirection = -1;
	_dispDirectionOld = -1;
	_dispFlipY = false;
#if defined(USE_WALL_DEBUG_DISPLAY)
	_wallDebugDisplay = nullptr;
#endif
	_collisionAttackHitList = nullptr;
	_collisionHitAttackList = nullptr;
	_collisionWallWallChecked = false;
	_collisionWallWallList = nullptr;
	_collisionPortalHitList = nullptr;
	_playObjectData = nullptr;
	_inputDirectionId = -1;
	_inputDirectionIdOld = -1;
	_scenePartsId = 0;
	_prevObjectActionId = -1;

	_objectMovement = nullptr;
	_jumpActionFlag = false;
	_dispDirection = 0;
	_timeScale = 1.0f;
	_timeScaleTmp = -1.0f;
	_ownParentObject = nullptr;
	_childrenObjectList = nullptr;
	_connectObjectList = nullptr;
	_duration = 0.0;
	_objectPosition = cocos2d::Vec2::ZERO;
#ifdef FIX_ACT2_4774
	_premoveObjectPosition = cocos2d::Vec2::ZERO;
#endif
	_objectScale = cocos2d::Vec2::ONE;
	_objectRotation = 0.0f;
	_objectVisible = nullptr;
	_timerPauseAction = nullptr;
	_timerPauseAnimation = nullptr;
	_waitDuration300 = 0;
	_objectTemplateMove = nullptr;
	_objectCourseMove = nullptr;
	_objectLoopMove = nullptr;
	_objectAfterImage = nullptr;
#ifdef FIX_ACT2_5335
#define LOCUS_LENGTH_DIV_LEN	0.5
	_parentFollowPosHead = -1;
	_parentFollowPosTail = -1;
	_parentFollowPosWeight = 0;
	_parentFollowPosList.clear();
#else
	_parentFollowDuration300 = 0;
	_parentFollowPos = cocos2d::Vec2::ZERO;
#endif

	_objectEffectList = nullptr;

	_objectSceneLoop = nullptr;

	_updateOneshotFunction = nullptr;

#ifdef FIX_ACT2_4774
	_warpedTransitionPortalId = -1;
#else
	_firstTouchedPortalName = nullptr;
#endif

	_hitPhysicsObjList = nullptr;
	_damagedList = nullptr;

	_physicsPartsList = nullptr;

	_transparentMaskNode = nullptr;
	_silhouetteNode = nullptr;
	_parentFollowConnectId = -1;

#ifdef USE_REDUCE_RENDER_TEXTURE
#else
	_drawBackPhysicsPartsList = nullptr;
	_drawFrontPhysicsPartsList = nullptr;
#endif

	_physicsNode = nullptr;
	_disappearFlag = false;
	_lateRemove = false;

	_collisionPushBackVec = Vec2::ZERO;
	_connectObjectDispPriorityList = nullptr;

	_sceneIdOfFirstCreated = -1;
	_removeLayerMoveFlag = false;

	_needAbsorbToTileCorners = false;

	_bFirstCollisionCheck = true;
	_bRequestPlayAction = 0;
	_bRequestPlayActionTemplateMove = 0;
	_overlapFlag = 0;
	_bNoClearCollision = false;
	_isExternalValueXyFlag = false;
	_attackObjectList = nullptr;
	_attackerObjectInstanceIdList = nullptr;
#ifdef USE_REDUCE_RENDER_TEXTURE
	_frontPhysicsNode = nullptr;
	_backPhysicsNode = nullptr;
#endif
	_pushedbackByObject = true;
	_variableAttackRate = 100.0;
	_forceDisabled = false;
	_execActionObjectCreateList = nullptr;
	_innerObjectVisible = true;
	_currentAnimMotionId = -1;
	_frameCount = 0;
	_bgmList = nullptr;
	_seList = nullptr;
	_voiceList = nullptr;
	_playerVisible = true;
	_switchInfoCreateConnectObjectList = nullptr;
	_dioGameSpeed = 0.0f;
	_dioExecuting = false;
	_dioFrame = 0.0f;
	_dioEffectDuration = 0.0f;
	_dioParent = false;
	_dioChild = false;
	_switchInfoCreateConnectObjectList = nullptr;
#ifdef FIX_ACT2_4774
	_portalMoveDispBit = -1;
#endif
	_courseMoveLoadData = nullptr;
	_takeOverAnimMotionId = -1;
	_bTouchGimmickCounted = false;
	_objectMoveLift = false;
	_resourceSetId = -1;
	_forceVisibleState = ForceVisibleState::kIgnore;
// #AGTK-NX
#ifdef USE_SAR_OPTIMIZE_4
	_watchSwitchList = nullptr;
#endif
// #AGTK-NX #AGTK-WIN
#ifdef USE_30FPS_1
	_passedFrameCount = -1;
#endif
}

Object::~Object()
{
#ifdef USE_MULTITHREAD_OBJECT_UPDATE
#ifdef USE_MULTITHREAD_MEASURE
	if (_destructed) {
		__debugbreak();
	}
	_destructed = true;
#endif
#endif
#ifdef USE_MULTITHREAD_OBJECT_UPDATE
	autoReleaseWallObjectList();
#endif
	CC_SAFE_RELEASE_NULL(_sceneData);
	CC_SAFE_RELEASE_NULL(_scenePartObjectData);
	CC_SAFE_RELEASE_NULL(_objectData);
	CC_SAFE_RELEASE_NULL(_objectReappearData);
	CC_SAFE_RELEASE_NULL(_currentObjectAction);
	CC_SAFE_RELEASE_NULL(_objectActionList);
	CC_SAFE_RELEASE_NULL(_objectCollision);
//	CC_SAFE_RELEASE_NULL(_basePlayer);
	if (_player) {
		auto basePlayer = _player->getBasePlayer();
		if (basePlayer) {
			basePlayer->setObjectNode(nullptr);
		}
	}
	CC_SAFE_RELEASE_NULL(_player);
	CC_SAFE_RELEASE_NULL(_objectWallCollision);
#ifdef USE_MULTITHREAD_OBJECT_UPDATE
	CC_SAFE_DELETE(_leftWallObjectList);
	CC_SAFE_DELETE(_rightWallObjectList);
	CC_SAFE_DELETE(_upWallObjectList);
	CC_SAFE_DELETE(_downWallObjectList);
	CC_SAFE_DELETE(_slopeTouchedList);
	CC_SAFE_DELETE(_passableSlopeTouchedList);
#else
	CC_SAFE_RELEASE_NULL(_leftWallObjectList);
	CC_SAFE_RELEASE_NULL(_rightWallObjectList);
	CC_SAFE_RELEASE_NULL(_upWallObjectList);
	CC_SAFE_RELEASE_NULL(_downWallObjectList);
	CC_SAFE_RELEASE_NULL(_slopeTouchedList);
	CC_SAFE_RELEASE_NULL(_passableSlopeTouchedList);
#endif
#if defined(USE_WALL_DEBUG_DISPLAY)
	this->removeWallDebugDisplay();
	CC_SAFE_RELEASE_NULL(_wallDebugDisplay);
#endif
	CC_SAFE_RELEASE_NULL(_collisionAttackHitList);
	CC_SAFE_RELEASE_NULL(_collisionHitAttackList);
	CC_SAFE_RELEASE_NULL(_collisionWallWallList);
	CC_SAFE_RELEASE_NULL(_collisionPortalHitList);
	CC_SAFE_RELEASE_NULL(_objectVisible);
	CC_SAFE_RELEASE_NULL(_playObjectData);
	CC_SAFE_RELEASE_NULL(_objectMovement);
	CC_SAFE_RELEASE_NULL(_ownParentObject);
	CC_SAFE_RELEASE_NULL(_childrenObjectList);
	if (_connectObjectList != nullptr) {
		this->unconnectAllConnectObject();
		CC_SAFE_RELEASE_NULL(_connectObjectList);
	}
	CC_SAFE_RELEASE_NULL(_timerPauseAction);
	CC_SAFE_RELEASE_NULL(_timerPauseAnimation);
	CC_SAFE_RELEASE_NULL(_objectAfterImage);
	CC_SAFE_RELEASE_NULL(_objectCourseMove);
	CC_SAFE_RELEASE_NULL(_objectLoopMove);
	CC_SAFE_RELEASE_NULL(_objectTemplateMove);

	CC_SAFE_RELEASE_NULL(_objectEffectList);

	CC_SAFE_RELEASE_NULL(_objectSceneLoop);

	CC_SAFE_RELEASE_NULL(_hitPhysicsObjList);
	CC_SAFE_RELEASE_NULL(_damagedList);
	CC_SAFE_RELEASE_NULL(_physicsPartsList);

	CC_SAFE_RELEASE_NULL(_transparentMaskNode);
	CC_SAFE_RELEASE_NULL(_silhouetteNode);

#ifdef USE_REDUCE_RENDER_TEXTURE
#else
	CC_SAFE_RELEASE_NULL(_drawBackPhysicsPartsList);
	CC_SAFE_RELEASE_NULL(_drawFrontPhysicsPartsList);
#endif

	CC_SAFE_RELEASE_NULL(_physicsNode);
	CC_SAFE_RELEASE_NULL(_connectObjectDispPriorityList);
	CC_SAFE_RELEASE_NULL(_attackObjectList);
	CC_SAFE_RELEASE_NULL(_attackerObjectInstanceIdList);
	CC_SAFE_RELEASE_NULL(_execActionObjectCreateList);
#ifdef USE_REDUCE_RENDER_TEXTURE
	CC_SAFE_RELEASE_NULL(_frontPhysicsNode);
	CC_SAFE_RELEASE_NULL(_backPhysicsNode);
#endif

	// サウンド再生終了のコールバックを削除
	cocos2d::DictElement *el;
	CCDICT_FOREACH(_bgmList, el) {
		auto bgmAudioList = dynamic_cast<cocos2d::__Dictionary *>(el->getObject());
		AudioManager::AudioInfo * audioInfo;
		cocos2d::DictElement *el2;
		CCDICT_FOREACH(bgmAudioList, el2) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
			audioInfo = static_cast<AudioManager::AudioInfo *>(el2->getObject());
#else
			audioInfo = dynamic_cast<AudioManager::AudioInfo *>(el2->getObject());
#endif
			audioInfo->setPlayEndCallback(nullptr);
			audioInfo->setPlayEndCallbackObject(nullptr);
		}
	}
	CC_SAFE_RELEASE_NULL(_bgmList);
	CCDICT_FOREACH(_seList, el) {
		auto seAudioList = dynamic_cast<cocos2d::__Dictionary *>(el->getObject());
		AudioManager::AudioInfo * audioInfo;
		cocos2d::DictElement *el2;
		CCDICT_FOREACH(seAudioList, el2) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
			audioInfo = static_cast<AudioManager::AudioInfo *>(el2->getObject());
#else
			audioInfo = dynamic_cast<AudioManager::AudioInfo *>(el2->getObject());
#endif
			audioInfo->setPlayEndCallback(nullptr);
			audioInfo->setPlayEndCallbackObject(nullptr);
		}
	}
	CC_SAFE_RELEASE_NULL(_seList);
	CCDICT_FOREACH(_voiceList, el) {
		auto voiceAudioList = dynamic_cast<cocos2d::__Dictionary *>(el->getObject());
		AudioManager::AudioInfo * audioInfo;
		cocos2d::DictElement *el2;
		CCDICT_FOREACH(voiceAudioList, el2) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
			audioInfo = static_cast<AudioManager::AudioInfo *>(el2->getObject());
#else
			audioInfo = dynamic_cast<AudioManager::AudioInfo *>(el2->getObject());
#endif
			audioInfo->setPlayEndCallback(nullptr);
			audioInfo->setPlayEndCallbackObject(nullptr);
		}
	}
	CC_SAFE_RELEASE_NULL(_voiceList);
	CC_SAFE_RELEASE_NULL(_switchInfoCreateConnectObjectList);
	CC_SAFE_RELEASE_NULL(_courseMoveLoadData);

#ifdef USE_SAR_OPTIMIZE_4
	CC_SAFE_DELETE(_watchSwitchList);
#endif

	_objectCount--;
}

void Object::initialize(agtk::Scene *scene, agtk::SceneLayer *sceneLayer)
{
	CC_ASSERT(scene && sceneLayer);
	//set wall collision
	sceneLayer->addCollisionDetaction(this);

	//set etc.
	int layerId = sceneLayer->getLayerId();//layer
	this->setId(sceneLayer->publishObjectId());
	//set instanceId
	if (this->getScenePartObjectData()) {
		//ScenePartDataのインスタンスIDを設定する。
		this->getPlayObjectData()->setInstanceId(this->getScenePartObjectData()->getId());
	}
	else {
		this->getPlayObjectData()->setInstanceId(scene->getObjectInstanceId(this));
	}
	this->getPlayObjectData()->setInstanceCount(scene->incrementObjectInstanceCount(this->getObjectData()->getId()));
	scene->updateObjectInstanceCount(this->getObjectData()->getId());
	this->setLayerId(layerId);
	this->setPhysicsBitMask(layerId, scene->getSceneData()->getId());
}

void Object::finalize(unsigned int removeOption)
{
	// 自身に追従しているUIを削除
	if (this->getRemoveLayerMoveFlag() == false) {
		GuiManager::getInstance()->removeGui(this);
	}
	// 自身に追従しているエフェクトを削除
	EffectManager::getInstance()->removeEffectAll(this, true);
	// ムービー削除
	MovieManager::getInstance()->removeMovie(this);
	// イメージ削除
	ImageManager::getInstance()->removeImage(this);
	// デバッグ
	DebugManager::getInstance()->removeObjectInfoWindow(this);
	if (!this->getRemoveLayerMoveFlag()) {
		// パーティクルをオブジェクトから切り離す
		ParticleManager::getInstance()->detachParticlesOfFollowed(this, agtk::data::ObjectCommandParticleRemoveData::ALL_PARTICLE);
	}
	// 弾を削除
	BulletManager::getInstance()->removeParentObject(this);

	this->clearCollision();
	this->getObjectCollision()->getObjectList()->removeAllObjects();
	this->getObjectCollision()->getPortalList()->removeAllObjects();
	this->getObjectCollision()->getHitObjectList()->removeAllObjects();
	this->getObjectCollision()->getWallObjectList()->removeAllObjects();
	this->getObjectWallCollision()->getObjectList()->removeAllObjects();
	this->getLeftWallObjectList()->removeAllObjects();
	this->getRightWallObjectList()->removeAllObjects();
	this->getUpWallObjectList()->removeAllObjects();
	this->getDownWallObjectList()->removeAllObjects();
	this->getCollisionAttackHitList()->removeAllObjects();
	this->getCollisionHitAttackList()->removeAllObjects();
	this->setCollisionWallWallChecked(false);
	this->getCollisionWallWallList()->removeAllObjects();
	auto ownParentObject = this->getOwnParentObject();
	if (!(removeOption & kRemoveOptionKeepOwnParentObjectBit)) {
		this->setOwnParentObject(nullptr);
	}
	agtk::Object *connectBaseObject = nullptr;
	if (this->isConnectObject() && GameManager::getInstance()->checkPortalTouched(this)) {
		auto connectObject = dynamic_cast<agtk::ConnectObject *>(this);
		connectBaseObject = connectObject->getConnectBaseObject();
		connectObject->unconnect();
	}	

	auto sceneLayer = this->getSceneLayer();
	auto scene = sceneLayer->getScene();
// #AGTK-NX
#ifdef USE_SAR_OPTIMIZE_1
	auto objectList = scene->getObjectAllReference(sceneLayer->getType());
#else
	auto objectList = scene->getObjectAll(sceneLayer->getType());
#endif
	auto removeObjectInstanceId = this->getInstanceId();
	cocos2d::Ref *ref;
	CCARRAY_FOREACH(objectList, ref) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
		auto obj = static_cast<agtk::Object *>(ref);
#else
		auto obj = dynamic_cast<agtk::Object *>(ref);
#endif
		if (obj == this) continue;
		//ACT2-3180 他オブジェクトの壁判定に接触で情報が消えてしまうバグの修正。
		//obj->getLeftWallObjectList()->removeObject(this);
		//obj->getRightWallObjectList()->removeObject(this);
		//obj->getUpWallObjectList()->removeObject(this);
		//obj->getDownWallObjectList()->removeObject(this);
		obj->removeCollisionAttackHit(this);
		obj->removeCollisionHitAttack(this);
		obj->removeCollisionWallWall(this);
		obj->getObjectCollision()->getObjectList()->removeObject(this);
		obj->getObjectCollision()->getPortalList()->removeObject(this);
		obj->getObjectCollision()->getHitObjectList()->removeObject(this);
		obj->getObjectCollision()->getWallObjectList()->removeObject(this);
		obj->getObjectWallCollision()->getObjectList()->removeObject(this);
		obj->removeConnectObjectDispPriority(this);

		auto bullet = dynamic_cast<agtk::Bullet *>(this);
		if (bullet == nullptr) {
			BulletManager::getInstance()->removeParentObject(this);
		}
		if (this->getRemoveLayerMoveFlag() == false) {
			// 削除するオブジェクトが親の場合、子オブジェクトに保存していた親インスタンスIDを削除
			if (obj->getPlayObjectData()->getParentObjectInstanceId() == removeObjectInstanceId) {
				obj->getPlayObjectData()->setParentObjectInstanceId(-1);
			}
		}
		obj->getAttackObjectList()->removeObject(this);

		//ポータル移動対象オブジェクトのチェック。
		bool bPortalObject = GameManager::getInstance()->checkPortalTouched(obj);
		if (!(removeOption & kRemoveOptionKeepChildObjectBit)) {
			// ポータル移動対象オブジェクトではない場合。
			if ((bPortalObject == false) || (bPortalObject == true && obj != ownParentObject)) {
				auto childObjectList = obj->getChildrenObjectList();
				if (childObjectList->containsObject(this) == true) {
					obj->getChildrenObjectList()->removeObject(this);
				}
			}
		}
		if (!(removeOption & kRemoveOptionKeepConnectObjectBit)) {
			if(this->isConnectObject()) {
				auto connectObject = dynamic_cast<agtk::ConnectObject *>(this);
				CC_ASSERT(connectObject);
				//ポータル移動対象オブジェクトではない場合。
				if ((bPortalObject == false) || (bPortalObject == true && obj != connectBaseObject)) {
					auto connectObjectList = obj->getConnectObjectList();
					if (connectObjectList->containsObject(this) == true) {
						obj->getConnectObjectList()->removeObject(this);
					}
				}
			}
		}

		obj->getObjectWallCollision()->getLeftWallObjectList()->removeObject(this);
		obj->getObjectWallCollision()->getRightWallObjectList()->removeObject(this);
		obj->getObjectWallCollision()->getUpWallObjectList()->removeObject(this);
		obj->getObjectWallCollision()->getDownWallObjectList()->removeObject(this);
	}
	//viewport&light
	auto viewportLightSceneLayer = scene->getViewportLight()->getViewportLightSceneLayer(this->getLayerId());
	if (viewportLightSceneLayer) {
		viewportLightSceneLayer->removeObject(this);
	}
	//弾
	if (isBullet()) {
		auto bulletManager = BulletManager::getInstance();
		auto bullet = dynamic_cast<agtk::Bullet *>(this);
		bulletManager->removeBullet(bullet);
	}
	//PlayDataからこのオブジェクトの参照をクリアする。
	this->getPlayObjectData()->clearObjectReference();

	this->removeFromParentAndCleanup(true);
}

Object *Object::createReappearData(agtk::SceneLayer *sceneLayer, agtk::data::ObjectReappearData *reappearData)
{
	auto sceneData = sceneLayer->getSceneData();
	auto scenePartObjectData = sceneData->getScenePartObjectData(reappearData->getScenePartsId());
	auto p = new (std::nothrow) Object();
	if (p) {
		if (scenePartObjectData != nullptr) {
			p->setScenePartObjectData(scenePartObjectData);
		}
		p->init(
			sceneLayer,
			reappearData->getObjectId(),
			reappearData->getInitialActionId(),
			reappearData->getInitialPosition(),
			reappearData->getInitialScale(),
			reappearData->getInitialRotation(),
			reappearData->getInitialMoveDirectionId(),
			reappearData->getInitialCourseId(),
			reappearData->getInitialCoursePointId(),
			-1
		);
		p->autorelease();
		return p;
	}
	CC_SAFE_DELETE(p);
	return nullptr;
}

void Object::setCreate(Object *(*create)(agtk::SceneLayer *sceneLayer, int objectId, int initialActionId, cocos2d::Vec2 position, cocos2d::Vec2 scale, float rotation, int moveDirectionId, int courseId, int coursePointId, int forceAnimMotionId))
{
	Object::create = create;
}

Object *Object::_create(agtk::SceneLayer *sceneLayer, int objectId, int initialActionId, cocos2d::Vec2 position, cocos2d::Vec2 scale, float rotation, int moveDirectionId, int courseId, int coursePointId, int forceMotionId)
{
	auto p = new (std::nothrow) Object();
	if (p && p->init(sceneLayer, objectId, initialActionId, position, scale, rotation, moveDirectionId, courseId, coursePointId, forceMotionId)) {
		p->autorelease();
		return p;
	}
	CC_SAFE_DELETE(p);
	return nullptr;
}

void Object::setCreateWithSceneDataAndScenePartObjectData(Object *(*createWithSceneDataAndScenePartObjectData)(agtk::SceneLayer *sceneLayer, agtk::data::ScenePartObjectData *scenePartObjectData, int forceObjectId))
{
	Object::createWithSceneDataAndScenePartObjectData = createWithSceneDataAndScenePartObjectData;
}

Object *Object::_createWithSceneDataAndScenePartObjectData(agtk::SceneLayer *sceneLayer, agtk::data::ScenePartObjectData *scenePartObjectData, int forceObjectId)
{
	auto p = new (std::nothrow) Object();
	if (p && p->initWithSceneDataAndScenePartObjectData(sceneLayer, scenePartObjectData, forceObjectId)) {
		p->autorelease();
		return p;
	}
	CC_SAFE_DELETE(p);
	return nullptr;
}

Object *Object::createScenePartObjectData(agtk::SceneLayer *sceneLayer, int objectId, int initialActionId, cocos2d::Vec2 position, cocos2d::Vec2 scale, float rotation, int moveDirectionId, int courseId, int coursePointId, int forceMotionId, agtk::data::ScenePartObjectData *scenePartObjectData)
{
	auto p = new (std::nothrow) Object();
	p->setScenePartObjectData(scenePartObjectData);
	if (p && p->init(sceneLayer, objectId, initialActionId, position, scale, rotation, moveDirectionId, courseId, coursePointId, forceMotionId)) {
		p->autorelease();
		return p;
	}
	CC_SAFE_DELETE(p);
	return nullptr;
}

/**
 * @brief dtにゲームスピードの影響を加味して、_duration, _timeScaleを更新する。
 *		オブジェクトがメニューレイヤーに置かれていて、メニューレイヤーが見えない場合は、時間を進めない。
 *		プレビュー時に停止の場合は、時間を進めない。
 *		オブジェクトが置かれているのがメニューレイヤーならそのメニューレイヤーのゲームスピードの影響を受けさせる。
 *		オブジェクトが置かれているのがシーンレイヤーならそのシーンのゲームスピードの影響を受けさせる。
 * @param dt	更新させたいゲームスピードを加味しない時間間隔
 * @ret 
 */
float Object::updateDuration(float dt)
{
	auto sceneLayer = this->getSceneLayer();
	auto scene = sceneLayer->getScene();
	/* メニューが停止しても2フレーム分動かすために、dt=0にはしないようにする。
	if (sceneLayer->getType() == agtk::SceneLayer::kTypeMenu) {
		if (sceneLayer->isDisplay() == false) {
			dt *= 0;
		}
	}*/
	float timeScale = this->getTimeScale();
	this->setTimeScaleTmp(timeScale);

	// ゲームスピードが反映されたタイムスケールを設定すると、ゲーム全体が影響を受けてしまったので
	// ゲームスピードが反映されていないタイムスケールを設定する
	// @todo スケジューラーを使っている？　どこで？　必要ないなら処理を無くした方が良い。
	this->getScheduler()->setTimeScale(timeScale);

	// ゲームスピードを反映したタイムスケールを設定
	auto sceneLayerType = sceneLayer->getType();
	float speed = scene->getGameSpeed()->getTimeScale(this);
	if (sceneLayerType == agtk::SceneLayer::kTypeMenu) {
		speed = scene->getGameSpeed()->getTimeScale(SceneGameSpeed::eTYPE_MENU);
	}
	// ヒットストップ中か
	if (_dioExecuting) {
		this->setTimeScale(timeScale * speed *  this->getDioGameSpeed());
		auto dioFream = this->getDioFrame();
		if (dioFream > this->getDioEffectDuration()) {
			//ヒットストップ終了
			this->setDioGameSpeed(0.0f);
			this->setDioExecuting(false);
			this->setDioFrame(0.0f);
			this->setDioEffectDuration(0.0f);
			this->setDioParent(false);
			this->setDioChild(false);
		}
		else {
			this->setDioFrame(dioFream + dt);
		}
	}
	else {
		this->setTimeScale(timeScale * speed);
	}
	dt *= this->getTimeScale();

	this->setDuration(dt);
	return this->getDuration();
}

void Object::updateExecActionObjectCreate()
{
lSkip:;
	auto execActionObjectCreateList = this->getExecActionObjectCreateList();
	if (execActionObjectCreateList->count() > 0) {
		auto player = this->getPlayer();
		cocos2d::Ref *ref;
		CCARRAY_FOREACH(execActionObjectCreateList, ref) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
			auto cmd = static_cast<agtk::data::ObjectCommandObjectCreateData *>(ref);
#else
			auto cmd = dynamic_cast<agtk::data::ObjectCommandObjectCreateData *>(ref);
#endif
			bool valid = (player) ? player->getTimelineValid(cmd->getConnectId()) : true;
			if (valid == false && cmd->getConnectId() >= 0) {
				break;
			}
			this->execActionObjectCreate(cmd);
			execActionObjectCreateList->removeObject(cmd);
			goto lSkip;
		}
	}
}

#ifdef FIX_ACT2_4879
#ifdef USE_PREVIEW
enum {
	kPhysicsSimulationSceneId = 99999997,
};
#endif
#endif
#ifdef USE_MULTITHREAD_OBJECT_UPDATE
void Object::objectUpdateBefore(float dt)
#else
void Object::update(float dt)
#endif
{
#ifdef USE_SAR_OHNISHI_DEBUG
	if (getObjectData()->getId() == g_debugObjectId) {
		//CCLOG("# object name: %s", getObjectData()->getName());
		g_debugIntVal = 0;
	}
#endif
	_bTouchGimmickCounted = false;

#ifdef USE_MULTITHREAD_OBJECT_UPDATE
	setReturnFlag(false);
#endif
#ifdef USE_MULTITHREAD_OBJECT_UPDATE
	_updating = true;
	autoReleaseWallObjectList();
#endif

	//フレームカウンター
	struct FrameCount {
		FrameCount(int* frame) { _frameCount = frame; }
		~FrameCount() { (*_frameCount)++; }
	private:
		int* _frameCount;
	} frameCount(&_frameCount);
	if (_frameCount == 0) {
		// UI表示の初期化
		GuiManager::getInstance()->addObjectParameterGui(this);
	}

	if (this->getDisabled()) {
#ifdef USE_MULTITHREAD_OBJECT_UPDATE
		setReturnFlag(true);
#endif
		this->getObjectCollision()->reset();
		return;
	}

	auto sceneLayer = this->getSceneLayer();
	auto scene = sceneLayer->getScene();

	if (_waitDuration300 < 0 && !scene->getSceneCreateSkipFrameFlag()) {
		//※_waitDuration300にマイナス値を設定した場合は時間制限なしに待ちになる。
#ifdef USE_MULTITHREAD_OBJECT_UPDATE
		setReturnFlag(true);
#endif
		return;
	}
	_waitDuration300 -= (int)(dt * 300);
	if (_waitDuration300 <= 0) {
		_waitDuration300 = 0;
	}
	else {
#ifdef USE_MULTITHREAD_OBJECT_UPDATE
		setReturnFlag(true);
#endif
		return;
	}

#ifdef USE_MULTITHREAD_OBJECT_UPDATE
	setOldDispPos(this->getDispPosition());
#else
	auto oldDispPos = this->getDispPosition();
#endif
	int oldMoveDirection = this->getMoveDirection();
	int oldDispDirection = this->getDispDirection();

	//時間更新。
	if (this->getTimeScaleTmp() < 0.0f) {
		dt = this->updateDuration(dt);
	} else {
		dt = this->getDuration();
	}
#ifdef USE_MULTITHREAD_OBJECT_UPDATE
	setDt(dt);
#endif

	// ポータル移動時の接続オブジェクト設定。
	if (_connectObjectPortalLoadList.size() > 0) {
		auto objectData = this->getObjectData();
		auto switchInfoList = this->getSwitchInfoCreateConnectObjectList();
		for (auto data : _connectObjectPortalLoadList) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
			auto setting = static_cast<agtk::data::ObjectConnectSettingData *>(objectData->getConnectSettingList()->objectForKey(data.settingId));
#else
			auto setting = dynamic_cast<agtk::data::ObjectConnectSettingData *>(objectData->getConnectSettingList()->objectForKey(data.settingId));
#endif

			bool isActive = false;

			// 「このオブジェクトのスイッチ」にチェックが入っている場合
			if (setting->getObjectSwitch()) {
				if (setting->getObjectSwitchId() == -1) {
					// 「無し」設定時は常にアクティブ
					isActive = true;
				}
				else {
					// オブジェクト固有のスイッチを取得
					auto switchData = getPlayObjectData()->getSwitchData(setting->getObjectSwitchId());
					if (switchData != nullptr) {
						auto boolian = dynamic_cast<cocos2d::__Bool *>(switchInfoList->objectForKey((intptr_t)switchData));
						if (boolian != nullptr) {
							if (boolian->getValue() != switchData->getValue()) {
								isActive = switchData->getValue();
							}
							else {
								//変更がない場合。
								continue;
							}
						}
						else {
							isActive = switchData->getValue();
						}
					}
				}
			}
			// 「システム共通」にチェックが入っている場合
			else {
				if (setting->getSystemSwitchId() == -1) {
					// 「無し」設定時は常にアクティブ
					isActive = true;
				}
				else {
					// システム共通のスイッチを取得
					auto switchData = GameManager::getInstance()->getPlayData()->getCommonSwitchData(setting->getSystemSwitchId());
					if (switchData != nullptr) {
						auto boolian = dynamic_cast<cocos2d::__Bool *>(switchInfoList->objectForKey((intptr_t)switchData));
						if (boolian != nullptr) {
							if (boolian->getValue() != switchData->getValue()) {
								isActive = switchData->getValue();
							}
							else {
								//変更がない場合。
								continue;
							}
						}
						else {
							isActive = switchData->getValue();
						}
					}
				}
			}

			if (isActive) {
				bool isConnected = true;
				// 接続するオブジェクトが既に接続済みの場合は処理しない
				auto connectObjectList = this->getConnectObjectList();
				cocos2d::Ref *ref;
				CCARRAY_FOREACH(connectObjectList, ref) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
					auto connectObj = static_cast<agtk::ConnectObject *>(ref);
#else
					auto connectObj = dynamic_cast<agtk::ConnectObject *>(ref);
#endif
					if (connectObj->getObjectConnectSettingData()->getId() == setting->getId()) {
						isConnected = false;
						break;
					}
				}
				if (isConnected) {
					auto connectObject = agtk::ConnectObject::create(this, data.settingId, data.actionId);
					if (connectObject != nullptr) {
						this->getConnectObjectList()->addObject(connectObject);
						//親オブジェクトが強制表示・非表示状態の場合、接続オブジェクトも同じステータスにする。
						if (_forceVisibleState != ForceVisibleState::kIgnore) {
							connectObject->setForceVisibleState(_forceVisibleState);
							connectObject->setWaitDuration300(-1);
						}
						// 接続するオブジェクトに紐付いた物理オブジェクトを生成する
						this->getSceneLayer()->createPhysicsObjectWithObject(connectObject);
					}
				}
			}
		}
		_connectObjectPortalLoadList.clear();
	}

	// 接続するインスタンスID(ロード時に追加される)
	if (_connectObjectLoadList.size()) {
		for (auto connectObjectLoadData : _connectObjectLoadList) {
			// 指定インスタンスのオブジェクトを取得
			auto object = GameManager::getInstance()->getTargetObjectByInstanceId(connectObjectLoadData.instanceId);

			// オブジェクトが存在する場合
			if (nullptr != object) {
				// 対象のオブジェクトを生成する
				agtk::ConnectObject * connectObject = agtk::ConnectObject::create(this, connectObjectLoadData.settingId, connectObjectLoadData.actionId);
				if (connectObject) {
					// プレイデータ上書き
					auto playObjectData = object->getPlayObjectData();
					playObjectData->setup(connectObject->getObjectData());
					connectObject->setPlayObjectData(playObjectData);

					// 座標
					connectObject->setPosition(object->getPosition());
					connectObject->setOldPosition(object->getOldPosition());
					connectObject->setPremoveObjectPosition(object->getPremoveObjectPosition());

					// 物理ノードにも座標を反映する
					auto physicsNode = connectObject->getphysicsNode();
					if (physicsNode) {
						auto player = connectObject->getPlayer();
						physicsNode->setPosition(player ? player->getPosition() : agtk::Scene::getPositionSceneFromCocos2d(object->getPosition(), scene));
					}

					// スケールを反映
					auto variableScalingX = playObjectData->getVariableData(agtk::data::kObjectSystemVariableScalingX);
					auto variableScalingY = playObjectData->getVariableData(agtk::data::kObjectSystemVariableScalingY);
					connectObject->setScale(Vec2(variableScalingX->getValue() * 0.01f, variableScalingY->getValue() * 0.01f));

					// 位置を直接変更したので衝突判定を更新する
					connectObject->getObjectCollision()->updateWallHitInfoGroup();
					connectObject->getObjectCollision()->lateUpdateWallHitInfoGroup();

					// 初期アクションを設定
					connectObject->playAction(object->getCurrentObjectAction()->getId(), object->getMoveDirection());
					connectObject->getCurrentObjectAction()->setDisableChangingFileSaveSwitchNextExecOtherAction(true);

					// シェーダを適用
					auto connectObjPlayer = connectObject->getPlayer();
					if (connectObjPlayer) {
						connectObjPlayer->removeAllShader();
						auto objPlayer = object->getPlayer();
						if (objPlayer && objPlayer->getRenderTexture()) {
							cocos2d::Ref* ref = nullptr;
							CCARRAY_FOREACH(objPlayer->getRenderTexture()->getShaderList(), ref) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
								auto shader = static_cast<agtk::Shader*>(ref);
#else
								auto shader = dynamic_cast<agtk::Shader*>(ref);
#endif
								auto kind = shader->getKind();
								connectObjPlayer->setShader(kind, shader->getValue()->getValue(), shader->getValue()->getSeconds());
								connectObjPlayer->getShader(kind)->setIgnored(shader->getIgnored());
							}
						}
					}

					this->getConnectObjectList()->addObject(connectObject);

					// 接続するオブジェクトに紐付いた物理オブジェクトを生成する
					this->getSceneLayer()->createPhysicsObjectWithObject(connectObject);
				}
				object->getSceneLayer()->removeObject(object);
				object->getCurrentObjectAction()->getObjCommandList()->removeAllObjects();
			}
		}
		_connectObjectLoadList.clear();
	}

	//更新時に一度のみ実行する関数。
	bool bFirstUpdate = false;
	if (_updateOneshotFunction) {
		_updateOneshotFunction();
		_updateOneshotFunction = nullptr;
		bFirstUpdate = true;
	}

	//※updateまでに、layerIdの設定をする。
	CC_ASSERT(this->getLayerId() > 0);

#ifdef FIX_ACT2_4774
	_premoveObjectPosition = getPosition();
	_portalMoveDispBit = -1;
	auto switchData = this->getPlayObjectData()->getSwitchData(agtk::data::kObjectSystemSwitchPortalTouched);
	bool bReadOnly = switchData->getReadOnly();
	switchData->setReadOnly(false);
	switchData->setValue(false);
	switchData->setReadOnly(bReadOnly);
#endif
	if (dt > 0 || (dt == 0 && scene->getSceneCreateSkipFrameFlag())) {
		// 入力更新
		this->updateInputPlayer();

		// 入力による方向変更更新
		this->updateInputPlayerDirection();
	}

	this->refreshHpChangeTrigger();
	// 子オブジェクト更新（前）
	this->updateAsChildBefore(dt);

	// カレントのオブジェクトアクションを取得
	auto objectAction = this->getCurrentObjectAction();

	// カレントのオブジェクトアクションが存在しない場合
	if (objectAction == nullptr) {
		// ここで終了
#ifdef USE_MULTITHREAD_OBJECT_UPDATE
		setReturnFlag(true);
#endif
		return;
	}
	
	// 初回時のみアクション実行前の衝突チェックを行う
	if (_bFirstCollisionCheck)
	{
		_bFirstCollisionCheck = false;
		auto player = this->getPlayer();
		if (player) {
			auto pos = player->getPosition();
			this->wallCollisionCorrection(pos.x, pos.y);
		}
	}

	//表示方向変数の変更を反映する。
	this->retrieveDisplayDirectionVariable();
	
	// アクション用のデルタタイム
	auto actionDt = dt;
#ifdef USE_MULTITHREAD_OBJECT_UPDATE
	this->setChangeActionFlag(false);
#else
	auto bChangeAction = false;
#endif

	// ACT2-5259 時間経過がある場合のみ、リンクチェック（アクション切り替え）や方向変更を行わせるように。
	if (bFirstUpdate == false &&
		_bRequestPlayAction == 0 &&
		!scene->getIgnoredUpdateActionFlag() &&
		!_disappearFlag &&
		((dt > 0) || (dt == 0 && scene->getSceneCreateSkipFrameFlag())))
	{
		int nextObjectActionId = -1;

		// 保留となったアクションがない場合
		if (!objectAction->getHasHoldCommandList()) {
			// アクションリンクの条件チェック
			nextObjectActionId = objectAction->checkActionLinkCondition();
		}

		// 最新の移動方向を取得
		auto nowMoveDirection = this->getMoveDirection();
		auto nowDispDirection = this->getDispDirection();

		// リンク条件が満たされている もしくは 方向が変更された場合
		if (nextObjectActionId >= 0 || (nowMoveDirection > 0 && oldMoveDirection != nowMoveDirection) || (nowDispDirection > 0 && oldDispDirection != nowDispDirection)) {
			// アクションを変更する
			auto oldObjectAction = objectAction;
			objectAction = this->setup(nextObjectActionId, nowMoveDirection);
#ifdef USE_SAR_OHNISHI_DEBUG
			if (getObjectData()->getId() == g_debugObjectId) {
				CCLOG("# action name: %s", objectAction->getObjectActionData()->getName());
			}
#endif
			// アクションが変更された直後はアクション内の時間は経過させない
			if (oldObjectAction != objectAction) {
				actionDt = 0;
#ifdef USE_MULTITHREAD_OBJECT_UPDATE
				this->setChangeActionFlag(true);
#else
				bChangeAction = true;
#endif
			}
		}
	}

	// アクションを更新
	if (_bRequestPlayAction || _bRequestPlayActionTemplateMove) {
		//playActionメソッドを実行する時に、アクションを更新するために _bRequestPlayActionフラグを使用する。
		if (_bRequestPlayAction == 2) {//アクション変化あり。
			actionDt = 0;
#ifdef USE_MULTITHREAD_OBJECT_UPDATE
			this->setChangeActionFlag(true);
#else
			bChangeAction = true;
#endif
		}
		_bRequestPlayAction = 0;
		if (_bRequestPlayActionTemplateMove == 2) {
			actionDt = 0;
#ifdef USE_MULTITHREAD_OBJECT_UPDATE
			this->setChangeActionFlag(true);
#else
			bChangeAction = true;
#endif
		}
		_bRequestPlayActionTemplateMove = 0;
		//当たり判定情報のチェック。
		if (this->isCollision()) {
			//※playActionでアクションを更新した際、実行コマンドが１フレーム飛ばされるため、当たり判定情報を次のフレームで得るために、１度クリアしないようにする。
			_bNoClearCollision = true;
		}
	}
	objectAction->update(actionDt);

	//オブジェクトを生成するアクション。
	this->updateExecActionObjectCreate();

	//アクションコマンド更新後、消滅。
	if (objectAction->getObjCommandList()->count() == 0 && _disappearFlag) {
		this->removeSelf(false);
#ifdef USE_MULTITHREAD_OBJECT_UPDATE
		setReturnFlag(true);
#endif
		return;
	}

	// オブジェクトアニメーション
	auto objectData = this->getObjectData();
	auto playObjectData = this->getPlayObjectData();
	bool isPhysicsOn = objectData->getPhysicsSettingFlag();
	bool isRelayPhysics = isPhysicsOn && playObjectData->getSwitchData(agtk::data::kObjectSystemSwitchFollowConnectedPhysics)->getValue();
	bool isPhysicsAffected = isPhysicsOn && playObjectData->getSwitchData(agtk::data::kObjectSystemSwitchAffectedByOtherObjects)->getValue();
	auto player = this->getPlayer();
#ifdef FIX_ACT2_4879
#ifdef USE_PREVIEW
	if (this->getSceneData()->getId() == kPhysicsSimulationSceneId && (!scene->getWatchPhysicsPartIdList() || scene->getWatchPhysicsPartIdList()->count() == 1 && this->getScenePartObjectData()->getId() == 1)) {
		//オブジェクトの物理シミュレーション中のオブジェクトは座標変更等を行わない。
	} else
#endif
#endif
	if (player) {
		auto objectMovement = this->getObjectMovement();

		// 物理に依存しない場合
		auto nowDirection = objectMovement->getDirection();
		if (isRelayPhysics == false) {
			if (isPhysicsAffected == false) {
				//移動処理
				objectMovement->update(dt);
			}
		}

		//戦車・車タイプの表示方向を更新
		auto objectActionData = this->getCurrentObjectAction()->getObjectActionData();
		auto moveType = objectData->getMoveType();
		if (moveType == agtk::data::ObjectData::kMoveTank || moveType == agtk::data::ObjectData::kMoveCar) {
			auto direction = objectMovement->getDirection();
			auto degree = agtk::GetDegreeFromVector(direction);
			auto directionId = agtk::GetMoveDirectionId(degree);
			if (direction != cocos2d::Vec2::ZERO) {
				this->setDispDirection(directionId);
			}

			bool bAutoGeneration = this->isAutoGeneration();
			auto inputDirection = objectMovement->getInputDirection();
			//y軸（前進、後退）
			if (bAutoGeneration) {
				this->setMoveDirection(0);
			}
			if (inputDirection.y != 0.0f) {
				this->setMoveDirection(inputDirection.y > 0.0f ? directionId : 10 - directionId);
			}
			//旋回（回転で自動生成なし）
			if (direction != nowDirection && !bAutoGeneration) {
				int actionId = this->getCurrentObjectAction()->getId();
				int moveDirectionId = agtk::GetMoveDirectionId(GetDegreeFromVector(direction));
				auto currentObjectAction = this->getCurrentObjectAction();
				this->playAction(actionId, moveDirectionId);
				//アクション再生リクエストフラグ。
				if (currentObjectAction == this->getCurrentObjectAction() && _bRequestPlayAction == 1) {
					//オブジェクトアクションに変化が無くかつ、_bRequestPlayAction=1のアクション変化の場合は、リクエスト無し状態にする。
					_bRequestPlayAction = 0;
				}
			}
		}
		//重力の影響を表示方向へ反映。
		else if (objectActionData->getReflectGravityToDisplayDirection() && this->isAutoGeneration() == false) {
			auto direction = objectMovement->getMoveXy().getNormalized();
			auto degree = agtk::GetDegreeFromVector(direction);
			auto directionId = agtk::GetMoveDirectionId(degree);
			if (direction != cocos2d::Vec2::ZERO) {
				this->setDispDirection(directionId);
			}
			//回転で自動生成なし
			if (direction != nowDirection) {
				int actionId = this->getCurrentObjectAction()->getId();
				int moveDirectionId = agtk::GetMoveDirectionId(GetDegreeFromVector(direction));
				auto currentObjectAction = this->getCurrentObjectAction();
				this->playAction(actionId, moveDirectionId);
				//アクション再生リクエストフラグ。
				if (currentObjectAction == this->getCurrentObjectAction() && _bRequestPlayAction == 1) {
					//オブジェクトアクションに変化が無くかつ、_bRequestPlayAction=1のアクション変化の場合は、リクエスト無し状態にする。
					_bRequestPlayAction = 0;
				}
			}
		}

		// 毎フレームアニメーションのフレームが固定されるかチェック
		auto basePlayer = player->getBasePlayer();
		if (basePlayer != nullptr) {
			auto variableData = getPlayObjectData()->getVariableData(agtk::data::kObjectSystemVariableFixedAnimationFrame);
			auto fixedValue = variableData->getValue();
			int curFrame = basePlayer->getFixedFrame();
			if (curFrame != (int)fixedValue) {
				basePlayer->setFixedFrame((int)fixedValue, false);
				if (fixedValue == agtk::AnimationMotion::kAnimPauseFrameValue) {
					// agtk::AnimationMotion::kAnimPauseFrameValueが指定されたら、現在再生中のフレームを変数に書き戻す。
					variableData->setValue(basePlayer->getFixedFrame());
				}
			}
		}

		// 物理に依存しない場合
		if (!isRelayPhysics) {
			//回転
			float rotation = player->getRotation();

			// 物理影響を受けない場合
			if (!isPhysicsAffected) {
				player->setRotation(rotation);

				//回転の自動生成。
				if (this->isAutoGeneration()) {
					auto direction = objectMovement->getDirection();
					//重力の影響を表示方向へ反映。
					if (objectActionData->getReflectGravityToDisplayDirection()) {
						direction = objectMovement->getMoveXy().getNormalized();
					}
					if (direction == cocos2d::Vec2::ZERO) {
						if (GameManager::getInstance()->getProjectData()->getGameType() == agtk::data::ProjectData::kGameTypeSideView) {
							direction.x = 1;
						}
						else {
							direction = agtk::GetDirectionFromMoveDirectionId(this->getDispDirection());
						}
					}
					player->setCenterRotation(agtk::GetDegreeFromVector(direction));
				}
				else {
					//プレイヤータイプ以外。
					if (!objectData->isGroupPlayer()) {
						player->setCenterRotation(0.0f);
					}
				}
			}
			else {
				auto physicsNode = this->getphysicsNode();
				if (physicsNode) {
					physicsNode->setRotation(rotation);
				}
			}
		}

		//check change attack area
		player->setWallAreaAttackFlag(this->getObjectVisible()->isWallAreaAttackWhenInvincible());

		//scale
		float scaleX = player->getScaleX();
		float scaleY = player->getScaleY();
		player->setScale(scaleX, scaleY);

		auto pos = player->getPosition();
		auto oldPos = pos;

		// 物理に依存しない場合
		if (!isRelayPhysics) {
			//pos.x += objectMovement->getMoveX();
			//pos.y += objectMovement->getMoveY();
			pos.x = std::roundf((pos.x + objectMovement->getMoveX()) * 10000.0f) * 0.0001f;
			pos.y = std::roundf((pos.y + objectMovement->getMoveY()) * 10000.0f) * 0.0001f;
			// 物理影響を受けない場合
			if (!isPhysicsAffected) {
				player->setPosition(pos);
			}
			// 物理影響を受ける場合
			else {
				// 物理ノードに対して速度を設定する
				auto physicsNode = this->getphysicsNode();
				if (physicsNode) {
					//physicsNode->getPhysicsBody()->setVelocity(Vec2(objectMovement->getMoveX(), objectMovement->getMoveY()) * DOT_PER_METER);
					auto move = physicsNode->getPosition() - this->getPhysicsNodeOldPosition();
					pos.x = std::roundf((pos.x + move.x) * 10000.0f) * 0.0001f;
					pos.y = std::roundf((pos.y + move.y) * 10000.0f) * 0.0001f;
					player->setPosition(pos);
				}
			}
		}
		else {
			auto physicsNode = this->getphysicsNode();
			if (physicsNode) {
				pos = physicsNode->getPosition();
				player->setPosition(pos, true);

				// プレイヤータイプでない または 表示方向を操作で変更しない場合
				if (!objectData->isGroupPlayer() || !objectData->getMoveDispDirectionSettingFlag()) {
					auto rotate = physicsNode->getRotation();
					//回転の自動生成。
					if (this->isAutoGeneration()) {
						auto direction = objectMovement->getDirection();
						if (direction == cocos2d::Vec2::ZERO) {
							if (GameManager::getInstance()->getProjectData()->getGameType() == agtk::data::ProjectData::kGameTypeSideView) {
								direction.x = 1;
							}
							else {
								direction = agtk::GetDirectionFromMoveDirectionId(this->getDispDirection());
							}
						}
						float degree = agtk::GetDegreeFromVector(direction);
						player->setCenterRotation(rotate + degree);
					}
					else {
						player->setCenterRotation(rotate);
					}
				}
			}
		}

		player->update(dt);

#ifdef USE_MULTITHREAD_OBJECT_UPDATE
		//this->setPrevObjectSameLayerWallBit(this->getObjectSameLayerWallBit());
#else
		{
			auto nowPos = this->getPosition();
			this->wallCollisionCorrection(pos.x, pos.y, bChangeAction ? kChangeActionUpdate : kChangeActionNone);
			this->setOldPosition2(nowPos);
		}
		//debug
#if defined(USE_WALL_DEBUG_DISPLAY)
		this->updateWallDebugDisplay(dt);
#endif
		// physics
		this->setupPhysicsBody(false);
#endif
#ifdef FIX_ACT2_4879
	}
 	else if(isPhysicsOn){
		auto objectMovement = this->getObjectMovement();

		// 物理に依存しない場合
		auto nowDirection = objectMovement->getDirection();
		if (isRelayPhysics == false) {
			if (isPhysicsAffected == false) {
				//移動処理
				objectMovement->update(dt);
			}
		}

		//戦車・車タイプの表示方向を更新
		auto moveType = objectData->getMoveType();
		if (moveType == agtk::data::ObjectData::kMoveTank || moveType == agtk::data::ObjectData::kMoveCar) {
			auto direction = objectMovement->getDirection();
			auto degree = agtk::GetDegreeFromVector(direction);
			auto directionId = agtk::GetMoveDirectionId(degree);
			if (direction != cocos2d::Vec2::ZERO) {
				this->setDispDirection(directionId);
			}

			bool bAutoGeneration = this->isAutoGeneration();
			auto inputDirection = objectMovement->getInputDirection();
			//y軸（前進、後退）
			if (bAutoGeneration) {
				this->setMoveDirection(0);
			}
			if (inputDirection.y != 0.0f) {
				this->setMoveDirection(inputDirection.y > 0.0f ? directionId : 10 - directionId);
			}
			//旋回（回転で自動生成なし）
			if (direction != nowDirection && !bAutoGeneration) {
				int actionId = this->getCurrentObjectAction()->getId();
				int moveDirectionId = agtk::GetMoveDirectionId(GetDegreeFromVector(direction));
				auto currentObjectAction = this->getCurrentObjectAction();
				this->playAction(actionId, moveDirectionId);
				//アクション再生リクエストフラグ。
				if (currentObjectAction == this->getCurrentObjectAction() && _bRequestPlayAction == 1) {
					//オブジェクトアクションに変化が無くかつ、_bRequestPlayAction=1のアクション変化の場合は、リクエスト無し状態にする。
					_bRequestPlayAction = 0;
				}
			}
		}

		// 物理に依存しない場合
		if (!isRelayPhysics) {
			//回転
			float rotation = 0;

			// 物理影響を受けない場合
			if (!isPhysicsAffected) {

				//回転の自動生成。
				if (this->isAutoGeneration()) {
					auto direction = objectMovement->getDirection();
					if (direction == cocos2d::Vec2::ZERO) {
						if (GameManager::getInstance()->getProjectData()->getGameType() == agtk::data::ProjectData::kGameTypeSideView) {
							direction.x = 1;
						}
						else {
							direction = agtk::GetDirectionFromMoveDirectionId(this->getDispDirection());
						}
					}
				}
				else {
				}
			}
			else {
				auto physicsNode = this->getphysicsNode();
				if (physicsNode) {
					physicsNode->setRotation(rotation);
				}
			}
		}

		//auto pos = player->getPosition();
		auto pos = Scene::getPositionCocos2dFromScene(_objectPosition, this->getSceneData());
		auto oldPos = pos;

		// 物理に依存しない場合
		if (!isRelayPhysics) {
			//pos.x += objectMovement->getMoveX();
			//pos.y += objectMovement->getMoveY();
			pos.x = std::roundf((pos.x + objectMovement->getMoveX()) * 10000.0f) * 0.0001f;
			pos.y = std::roundf((pos.y + objectMovement->getMoveY()) * 10000.0f) * 0.0001f;
			// 物理影響を受けない場合
			if (!isPhysicsAffected) {
				//player->setPosition(pos);
				this->setPosition(agtk::Scene::getPositionSceneFromCocos2d(pos));
			}
			// 物理影響を受ける場合
			else {
				// 物理ノードに対して速度を設定する
				auto physicsNode = this->getphysicsNode();
				if (physicsNode) {
					//physicsNode->getPhysicsBody()->setVelocity(Vec2(objectMovement->getMoveX(), objectMovement->getMoveY()) * DOT_PER_METER);
					auto move = physicsNode->getPosition() - this->getPhysicsNodeOldPosition();
					pos.x = std::roundf((pos.x + move.x) * 10000.0f) * 0.0001f;
					pos.y = std::roundf((pos.y + move.y) * 10000.0f) * 0.0001f;
					//player->setPosition(pos);
					this->setPosition(agtk::Scene::getPositionSceneFromCocos2d(pos));
				}
			}
		}
		else {
			auto physicsNode = this->getphysicsNode();
			if (physicsNode) {
				pos = physicsNode->getPosition();
				//player->setPosition(pos, true);
				this->setPosition(agtk::Scene::getPositionSceneFromCocos2d(pos));

				// プレイヤータイプでない または 表示方向を操作で変更しない場合
				if (!objectData->isGroupPlayer() || !objectData->getMoveDispDirectionSettingFlag()) {
					auto rotate = physicsNode->getRotation();
					//player->setCenterRotation(rotate);
				}
			}
		}

		//player->update(dt);

#ifdef USE_MULTITHREAD_OBJECT_UPDATE
#else
#if defined(USE_WALL_COLLISION)	// 2017/04/20 agusa-k: 壁判定修正
		{
			auto nowPos = this->getPosition();
			this->wallCollisionCorrection(pos.x, pos.y, bChangeAction ? kChangeActionUpdate : kChangeActionNone);
			this->setOldPosition2(nowPos);
		}
#endif
		//debug
#if defined(USE_WALL_DEBUG_DISPLAY)
		this->updateWallDebugDisplay(dt);
#endif
		// physics
		this->setupPhysicsBody(false);
#endif
#endif
	} else {
		auto movement = this->getObjectMovement();
		movement->update(dt);

		auto pos = this->getPosition();
		this->setPosition(pos + cocos2d::Vec2(movement->getMoveX(), -movement->getMoveY()));
	}
// #AGTK-NX #AGTK-WIN
#ifdef USE_30FPS
	auto forceMove = this->getObjectMovement()->getForceMove();
	if (forceMove->getWarpMoved()) {
		this->setOldPosition(this->getPosition());
		forceMove->setWarpMoved(false);
	}
#endif
#ifdef USE_SAR_OHNISHI_DEBUG
	if (getObjectData()->getId() == g_debugObjectId) {
		auto movement = this->getObjectMovement();
		auto pos = this->getPosition();
		CCLOG("# frameCount = %d move(%8.3f, %8.3f) pos(%8.3f, %8.3f)",
			getFrameCount(), movement->getMoveX(), movement->getMoveY(), pos.x, pos.y);
	}
#endif
#ifdef USE_MULTITHREAD_OBJECT_UPDATE
}
void Object::objectUpdateWallCollision(float dt)
{	
	if (getReturnFlag()) return;
	// 壁判定更新
	auto player = this->getPlayer();
	if (player) {
		auto nowPos = this->getPosition();
		auto pos = player->getPosition();
		this->wallCollisionCorrection(pos.x, pos.y, this->getChangeActionFlag() ? kChangeActionUpdate : kChangeActionNone);
		this->setOldPosition2(nowPos);
	}
	else {
		auto objectData = this->getObjectData();
		bool isPhysicsOn = objectData->getPhysicsSettingFlag();
		if (isPhysicsOn) {
			auto nowPos = this->getPosition();
			auto pos = Scene::getPositionCocos2dFromScene(nowPos, this->getSceneData());
			this->wallCollisionCorrection(pos.x, pos.y, this->getChangeActionFlag() ? kChangeActionUpdate : kChangeActionNone);
			this->setOldPosition2(nowPos);
		}
	}
	
}
void Object::objectUpdateAfter(float dt)
{
	if (getReturnFlag()) {
		retainWallObjectList();
		_updating = false;
		return;
	}
	dt = getDt();
	auto sceneLayer = this->getSceneLayer();
	auto scene = sceneLayer->getScene();

	auto oldDispPos = this->getOldDispPos();

	// オブジェクトアニメーション
	auto objectData = this->getObjectData();
	auto playObjectData = this->getPlayObjectData();
	bool isPhysicsOn = objectData->getPhysicsSettingFlag();
	bool isRelayPhysics = isPhysicsOn && playObjectData->getSwitchData(agtk::data::kObjectSystemSwitchFollowConnectedPhysics)->getValue();
	bool isPhysicsAffected = isPhysicsOn && playObjectData->getSwitchData(agtk::data::kObjectSystemSwitchAffectedByOtherObjects)->getValue();
	auto player = this->getPlayer();

#ifdef FIX_ACT2_4879
#ifdef USE_PREVIEW
	if (this->getSceneData()->getId() == kPhysicsSimulationSceneId && (!scene->getWatchPhysicsPartIdList() || scene->getWatchPhysicsPartIdList()->count() == 1 && this->getScenePartObjectData()->getId() == 1)) {
		//オブジェクトの物理シミュレーション中のオブジェクトは座標変更等を行わない。
	} else
#endif
#endif
	if (player) {
		/*if (this->getChangeActionFlag() && this->getObjectSameLayerWallBit() == 0) {
			//※アクションが切り替わったタイミングで、壁判定Bitが0になった場合は、wallCollisionCorrectionで更新前の値を設定する。
			this->setObjectSameLayerWallBit(this->getPrevObjectSameLayerWallBit());
		}*/
		//debug
#if defined(USE_WALL_DEBUG_DISPLAY)
		this->updateWallDebugDisplay(dt);
#endif
		// physics
		this->setupPhysicsBody(false);
	}
#ifdef FIX_ACT2_4879
	else if(isPhysicsOn){
		//debug
#if defined(USE_WALL_DEBUG_DISPLAY)
		this->updateWallDebugDisplay(dt);
#endif
		// physics
		this->setupPhysicsBody(false);
	}
#endif
	else {
	}
#endif

	//壁に埋まった処理。
	this->updateBurieInWall();

	// 自身が子オブジェクトだった場合の更新を行う
	this->updateAsChild(dt);

	// オブジェクトの明滅更新
	this->updateVisible(dt);

	auto timerPauseAction = this->getTimerPauseAction();
	timerPauseAction->update(dt);

	auto timerPauseAnimation = this->getTimerPauseAnimation();
	timerPauseAnimation->update(dt);

	// 残像の更新
	auto afterImage = this->getObjectAfterImage();
	afterImage->update(dt);

	// シルエットの更新
	auto silhouette = this->getSilhouetteNode();
	if (silhouette) {
		silhouette->update(dt);
	}

	//テンプレート移動更新
	auto templateMove = this->getObjectTemplateMove();
	templateMove->update(dt);

	// 360度ループ移動更新
	auto loopMove = this->getObjectLoopMove();
	loopMove->update(dt);

	// タイムスケールを戻す
	CC_ASSERT(this->getTimeScaleTmp() >= 0.0f);
	this->setTimeScale(this->getTimeScaleTmp());
	this->setTimeScaleTmp(-1.0f);

	//毎フレームのオブジェクト変数を更新する
	auto dispPos = this->getDispPosition();
	auto pos = this->getPosition();

	//毎フレームこのオブジェクトのシーンに対するX,Y座標位置が代入されます。
	auto variableX = playObjectData->getVariableData(agtk::data::kObjectSystemVariableX);
	auto variableY = playObjectData->getVariableData(agtk::data::kObjectSystemVariableY);
	bool bExternalValueX = variableX->isExternalValue();
	variableX->setExternalValue(dispPos.x);
	float x = bExternalValueX ? variableX->getValue() : pos.x;
	bool bExternalValueY = variableY->isExternalValue();
	variableY->setExternalValue(dispPos.y);
	float y = bExternalValueY ? variableY->getValue() : pos.y;
	_isExternalValueXyFlag = (bExternalValueX | bExternalValueY);

	// 物理に依存しない場合
	if (!isRelayPhysics && !isPhysicsAffected) {
		this->setPosition(cocos2d::Vec2(x, y));
// #AGTK-NX #AGTK-WIN
#ifdef USE_30FPS_3
		// 変数で位置を変更した場合、2pass壁判定処理で過去の位置を元に中間フレーム位置を算出すると問題があるため過去の位置も更新
		if (_isExternalValueXyFlag) {
			this->setOldPosition(cocos2d::Vec2(x, y));
		}
#endif
	}

	// 「カメラとの位置関係を固定する」設定時
	if (this->getObjectData()->getFixedInCamera() && this->getSceneLayer()->getType() != agtk::SceneLayer::kTypeMenu) {
		_objectPosInCamera = agtk::Scene::getPositionCocos2dFromScene(this->getPosition()) - scene->getCamera()->getPosition();
	}

	//毎フレームこのオブジェクトのスケール値が代入されます。
	float scaleX = this->getScaleX() * 100.0f;
	float scaleY = this->getScaleY() * 100.0f;
	auto variableScalingX = playObjectData->getVariableData(agtk::data::kObjectSystemVariableScalingX);
	auto variableScalingY = playObjectData->getVariableData(agtk::data::kObjectSystemVariableScalingY);
	bool bExternalValue = variableScalingX->isExternalValue();
	variableScalingX->setExternalValue(scaleX);
	float sx = (bExternalValue ? variableScalingX->getValue() : scaleX) * 0.01f;
	bExternalValue = variableScalingY->isExternalValue();
	variableScalingY->setExternalValue(scaleY);
	float sy = (bExternalValue ? variableScalingY->getValue() : scaleY) * 0.01f;

	if (!isRelayPhysics && !isPhysicsAffected) {
		this->setScale(cocos2d::Vec2(sx, sy));
	}

	playObjectData->getVariableData(agtk::data::kObjectSystemVariableVelocityX)->setValue(dispPos.x - oldDispPos.x);//毎フレームこのオブジェクトのX方向移動量が代入されます。
	playObjectData->getVariableData(agtk::data::kObjectSystemVariableVelocityY)->setValue(dispPos.y - oldDispPos.y);//毎フレームこのオブジェクトのY方向移動量が代入されます。
	if (this->getScenePartObjectData()) {
		if (this->getScenePartObjectData()->isStartPointObjectData()) {
			auto objectVariableData = playObjectData->getVariableData(agtk::data::kObjectSystemVariableControllerID);
			// プロジェクト変数「kProjectSystemVariable1PController + プレイヤーID - 1」に格納されたコントローラIDを設定する 
			int idOffset = this->getPlayerId() - 1;
			if (idOffset > -1) {
				auto systemVariableData = GameManager::getInstance()->getPlayData()->getVariableData(agtk::data::PlayData::COMMON_PLAY_DATA_ID, agtk::data::kProjectSystemVariable1PController + idOffset);
				if (objectVariableData->isExternalValue()) {
					auto controllerId = objectVariableData->getValue();
					if (systemVariableData->getValue() < 0.0 && controllerId >= 0) {
						systemVariableData->setValue(controllerId);
						GameManager::getInstance()->updateSystemVariableAndSwitch();
					}
					else if (systemVariableData->getValue() >= 0.0f && controllerId == -1) {
						systemVariableData->setValue(-1);
						GameManager::getInstance()->updateSystemVariableAndSwitch();
					}
					objectVariableData->resetExternalValue();
				} else {
					objectVariableData->setValue(systemVariableData->getValue());
					objectVariableData->resetExternalValue();
				}
			}
			else {
				objectVariableData->setValue(-1);
				objectVariableData->resetExternalValue();
			}
		}
	}

	//表示方向変数の変更を反映する。
	this->retrieveDisplayDirectionVariable();
	//表示方向変数を更新。
	this->updateDisplayDirectionVariable();

#ifdef USE_MULTITHREAD_OBJECT_UPDATE
	retainWallObjectList();
	_updating = false;
#endif
}

#ifdef USE_MULTITHREAD_OBJECT_UPDATE
void Object::update(float dt)
{
// #AGTK-NX #AGTK-WIN
#ifdef USE_30FPS_3
	auto gm = GameManager::getInstance();
	gm->setPassIndex(0);
	gm->setPassCount(1);
	gm->setLastPass(true);
#endif
	objectUpdateBefore(dt);
// #AGTK-NX #AGTK-WIN
#ifdef USE_30FPS_3
	gm->restoreWallCollisionPass();
	auto orgDelta = dt;
	for (int passIndex = 0; passIndex < gm->getPassCount(); passIndex++) {
		gm->setPassIndex(passIndex);
		bool bLastPass = false;
		if (passIndex == gm->getPassCount() - 1) {
			bLastPass = true;
		}
		gm->setLastPass(bLastPass);
		if (gm->getPassCount() > 1) {
			if (!bLastPass) {
				// pass1
				// 2pass処理向けに位置と移動量の補正
				this->_iniPos = this->getPosition();
				Vec2 mv = this->getPosition() - this->getOldPosition();
				this->setPosition(this->getOldPosition() + mv * 0.5f);
				this->_halfMove = mv - mv * 0.5f;
				// delta補正
				dt = dt * 0.5f;
			}
			else {
				// pass2
				// 中間フレームの位置を記録
				Vec2 pass1Pos = this->getPosition();
				Vec2 diff = pass1Pos - this->_iniPos;
				if (diff != Vec2::ZERO) {
					// 中間地点を反映
					this->setPassedFrameCount(this->getFrameCount());
					this->setPassedFramePosition(pass1Pos);
				}
				// pass2での過去位置更新
				this->setOldPosition(pass1Pos);
				this->setOldPosition2(pass1Pos);
				auto player = this->getPlayer();
				if (player) {
					this->setOldWallPosition(player->getPosition());
				}
				// pass2用のWallHitInfoGroupに更新 
				this->_bUseMiddleFrame = true;
				this->getObjectCollision()->updateMiddleFrameWallHitInfoGroup();
				// 2pass処理向けに位置と移動量の補正
				// 2pass移動により本来の目標位置への移動に誤差が発生するため誤差が僅かならば補正
				Vec2 pass2Pos = this->getPosition() + this->_halfMove;
				diff = pass2Pos - this->_iniPos;
				diff.x = std::abs(diff.x);
				diff.y = std::abs(diff.y);
				if (diff.x > 0.0f && diff.x < 0.001f) {
					pass2Pos.x = this->_iniPos.x;
				}
				if (diff.y > 0.0f && diff.y < 0.001f) {
					pass2Pos.y = this->_iniPos.y;
				}
				this->setPosition(pass2Pos);
				// delta補正
				dt = orgDelta - dt;
			}
		}
#endif
	objectUpdateWallCollision(dt);
// #AGTK-NX #AGTK-WIN
#ifdef USE_30FPS_3
	} // for (int passIndex = 0; passIndex < gm->getPassCount(); passIndex++);
	gm->restoreWallCollisionPass();
	dt = orgDelta;
#endif
	objectUpdateAfter(dt);
}

void Object::retainWallObjectList()
{
	auto size = (int)_leftWallObjectList->size();
	for (int i = 0; i < size; i++) {
		auto obj = (*_leftWallObjectList)[i];
		obj->retain();
	}
	size = (int)_rightWallObjectList->size();
	for (int i = 0; i < size; i++) {
		auto obj = (*_rightWallObjectList)[i];
		obj->retain();
	}
	size = (int)_upWallObjectList->size();
	for (int i = 0; i < size; i++) {
		auto obj = (*_upWallObjectList)[i];
		obj->retain();
	}
	size = (int)_downWallObjectList->size();
	for (int i = 0; i < size; i++) {
		auto obj = (*_downWallObjectList)[i];
		obj->retain();
	}
	for(auto &tileWall: _linkConditionTileWall){
		tileWall.tile->retain();
	}
	for (auto &tileWall : _aheadTileWall) {
		tileWall.tile->retain();
	}
}

void Object::autoReleaseWallObjectList()
{
	int size;
	if (_leftWallObjectList) {
		size = (int)_leftWallObjectList->size();
		for (int i = 0; i < size; i++) {
			auto obj = (*_leftWallObjectList)[i];
			obj->autorelease();
		}
	}
	if (_rightWallObjectList) {
		size = (int)_rightWallObjectList->size();
		for (int i = 0; i < size; i++) {
			auto obj = (*_rightWallObjectList)[i];
			obj->autorelease();
		}
	}
	if (_upWallObjectList) {
		size = (int)_upWallObjectList->size();
		for (int i = 0; i < size; i++) {
			auto obj = (*_upWallObjectList)[i];
			obj->autorelease();
		}
	}
	if (_downWallObjectList) {
		size = (int)_downWallObjectList->size();
		for (int i = 0; i < size; i++) {
			auto obj = (*_downWallObjectList)[i];
			obj->autorelease();
		}
	}
	for (auto &tileWall : _linkConditionTileWall) {
		tileWall.tile->autorelease();
	}
	for (auto &tileWall : _aheadTileWall) {
		tileWall.tile->autorelease();
	}
}

void Object::autoReleaseRetainWallObjectList(agtk::MtVector<agtk::Object *> *leftWallObjectList, agtk::MtVector<agtk::Object *> *rightWallObjectList, agtk::MtVector<agtk::Object *> *upWallObjectList, agtk::MtVector<agtk::Object *> *downWallObjectList)
{
	if (_updating) {
		_leftWallObjectList->removeAllObjects();
		_rightWallObjectList->removeAllObjects();
		_upWallObjectList->removeAllObjects();
		_downWallObjectList->removeAllObjects();

		_leftWallObjectList->addObjectsFromArray(leftWallObjectList);
		_rightWallObjectList->addObjectsFromArray(rightWallObjectList);
		_upWallObjectList->addObjectsFromArray(upWallObjectList);
		_downWallObjectList->addObjectsFromArray(downWallObjectList);
		return;
	}
	int size;
	if (_leftWallObjectList) {
		size = (int)_leftWallObjectList->size();
		for (int i = 0; i < size; i++) {
			auto obj = (*_leftWallObjectList)[i];
			obj->autorelease();
		}
	}
	if (_rightWallObjectList) {
		size = (int)_rightWallObjectList->size();
		for (int i = 0; i < size; i++) {
			auto obj = (*_rightWallObjectList)[i];
			obj->autorelease();
		}
	}
	if (_upWallObjectList) {
		size = (int)_upWallObjectList->size();
		for (int i = 0; i < size; i++) {
			auto obj = (*_upWallObjectList)[i];
			obj->autorelease();
		}
	}
	if (_downWallObjectList) {
		size = (int)_downWallObjectList->size();
		for (int i = 0; i < size; i++) {
			auto obj = (*_downWallObjectList)[i];
			obj->autorelease();
		}
	}
	_leftWallObjectList->removeAllObjects();
	_rightWallObjectList->removeAllObjects();
	_upWallObjectList->removeAllObjects();
	_downWallObjectList->removeAllObjects();

	_leftWallObjectList->addObjectsFromArray(leftWallObjectList);
	_rightWallObjectList->addObjectsFromArray(rightWallObjectList);
	_upWallObjectList->addObjectsFromArray(upWallObjectList);
	_downWallObjectList->addObjectsFromArray(downWallObjectList);
	size = (int)_leftWallObjectList->size();
	for (int i = 0; i < size; i++) {
		auto obj = (*_leftWallObjectList)[i];
		obj->retain();
	}
	size = (int)_rightWallObjectList->size();
	for (int i = 0; i < size; i++) {
		auto obj = (*_rightWallObjectList)[i];
		obj->retain();
	}
	size = (int)_upWallObjectList->size();
	for (int i = 0; i < size; i++) {
		auto obj = (*_upWallObjectList)[i];
		obj->retain();
	}
	size = (int)_downWallObjectList->size();
	for (int i = 0; i < size; i++) {
		auto obj = (*_downWallObjectList)[i];
		obj->retain();
	}
}

void Object::autoReleaseRetainTileWallList(ObjectCollision::HitTileWalls *linkConditionTileWallList, ObjectCollision::HitTileWalls *aheadTileWallList)
{
	if (_updating) {
		if (linkConditionTileWallList) {
			this->setLinkConditionTileWall(*linkConditionTileWallList);
		}
		if (aheadTileWallList) {
			this->setAheadTileWall(*aheadTileWallList);
		}
		return;
	}
	if (linkConditionTileWallList) {
		for (auto &tileWall : _linkConditionTileWall) {
			tileWall.tile->autorelease();
		}
	}
	if (aheadTileWallList) {
		for (auto &tileWall : _aheadTileWall) {
			tileWall.tile->autorelease();
		}
	}
	if (linkConditionTileWallList) {
		this->setLinkConditionTileWall(*linkConditionTileWallList);
	}
	if (aheadTileWallList) {
		this->setAheadTileWall(*aheadTileWallList);
	}
	if (linkConditionTileWallList) {
		for (auto &tileWall : _linkConditionTileWall) {
			tileWall.tile->retain();
		}
	}
	if (aheadTileWallList) {
		for (auto &tileWall : _aheadTileWall) {
			tileWall.tile->retain();
		}
	}
}

#endif

void Object::earlyUpdate(float dt)
{
	auto playObjectData = this->getPlayObjectData();
	auto objectData = this->getObjectData();
	bool isPhysicsOn = objectData->getPhysicsSettingFlag();
	bool isRelayPhysics = isPhysicsOn && playObjectData->getSwitchData(agtk::data::kObjectSystemSwitchFollowConnectedPhysics)->getValue();
	bool isPhysicsAffected = isPhysicsOn && playObjectData->getSwitchData(agtk::data::kObjectSystemSwitchAffectedByOtherObjects)->getValue();

	// 物理に依存しない場合かつ、物理影響を受ける場合
	if (isRelayPhysics == false && isPhysicsAffected == true) {
		// 時間更新。
		CC_ASSERT(this->getTimeScaleTmp() < 0.0f);
		dt = this->updateDuration(dt);

		auto objectMovement = this->getObjectMovement();
		objectMovement->update(dt);

		// 物理ノードに対して速度を設定する
		auto physicsNode = this->getphysicsNode();
		if (physicsNode) {
			physicsNode->getPhysicsBody()->setVelocity(Vec2(objectMovement->getMoveX(), objectMovement->getMoveY()) * DOT_PER_METER);
			this->setPhysicsNodeOldPosition(physicsNode->getPosition());
		}
	}
}

void Object::lateUpdate(float dt)
{
	// 消滅設定が行われている場合、消滅させる
	if (this->getLateRemove()) {
		this->removeSelf();
		return;
	}

	//変数・スイッチ更新
	this->getPlayObjectData()->update(dt);

	// 坂に接触している時間を更新
	if (this->getSlopeTouchedList()->count() > 0) {
		_slopeTouchedFrame += (dt * 300);
	}
	else {
		_slopeTouchedFrame = 0;
	}

	// タイルへ吸着要請がある場合
	if (this->getNeedAbsorbToTileCorners()) {
		this->setNeedAbsorbToTileCorners(false);
		this->absorbToTileCorners();
	}

	// 「オブジェクトにエフェクトを表示」設定時
	if (this->getObjectData()->getEffectSettingFlag()) {

		cocos2d::Ref *ref = nullptr;
		CCARRAY_FOREACH(this->getObjectEffectList(), ref) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
			auto objectEffect = static_cast<agtk::ObjectEffect*>(ref);
#else
			auto objectEffect = dynamic_cast<agtk::ObjectEffect*>(ref);
#endif

			// 前のフレームのスイッチの値を更新
			objectEffect->setSwitchValueOld(objectEffect->getSwitchValue());
			
			// エフェクトの設定データを取得
			auto settingData = objectEffect->getEffectSettingData();

			// このオブジェクトのスイッチを使用する場合
			if (settingData->getObjectSwitch()) {

				// スイッチ「無し」の場合は常時表示を許可する
				if (settingData->getObjectSwitchId() == -1) {
					objectEffect->setSwitchValue(true);
				}
				else {
					auto switchData = this->getPlayObjectData()->getSwitchData(settingData->getObjectSwitchId());
					if (switchData != nullptr) {
						objectEffect->setSwitchValue(switchData->getValue());
					}
				}
			}
			// システム共通のスイッチを使用する場合
			else {
				// スイッチ「無し」の場合は常時表示を許可する
				if (settingData->getSystemSwitchId() == -1) {
					objectEffect->setSwitchValue(true);
				}
				else {
					auto switchData = GameManager::getInstance()->getPlayData()->getCommonSwitchData(settingData->getSystemSwitchId());
					if (switchData != nullptr) {
						objectEffect->setSwitchValue(switchData->getValue());
					}
				}
			}

			// スイッチがONになった直後の場合
			if (!objectEffect->getSwitchValueOld() && objectEffect->getSwitchValue()) {

				auto projectData = GameManager::getInstance()->getProjectData();
				CC_ASSERT(projectData);

				// 接続点ID
				int connectId = -1;


				// アニメーションの種類に応じて処理を変化
				switch (settingData->getAnimationType()) {
					// エフェクト
					case agtk::data::ObjectEffectSettingData::EnumAnimationType::kAnimationTypeEffect: 
					{
						auto effectId = settingData->getEffectId();

						// 「設定無し」でない場合
						if (effectId > agtk::data::ObjectEffectSettingData::NO_SETTING) {

							// 生成対象のアニメーションデータを取得
							auto animationData = projectData->getAnimationData(effectId);

							// 表示時間
							int duration300 = (!settingData->getDispDurationUnlimited()) ? settingData->getDispDuration300() : -1;

							// オフセット
							Vec2 offset = Vec2::ZERO;

							switch (settingData->getPositionType()) {
								// このオブジェクトの中心
							case agtk::data::ObjectEffectSettingData::EnumPositionType::kPositionCenter:
							{
							} break;

							// このオブジェクトの足元
							case agtk::data::ObjectEffectSettingData::EnumPositionType::kPositionFoot:
							{
								offset = this->getFootPosition() - this->getCenterPosition();
							} break;

							// 接続点を使用
							case agtk::data::ObjectEffectSettingData::EnumPositionType::kPositionUseConnection:
							{
								connectId = settingData->getConnectionId();
							} break;
							}

							// 位置調整値を設定
							offset.x += settingData->getAdjustX();
							offset.y += settingData->getAdjustY();

							// エフェクトを生成する
#ifdef USE_REDUCE_RENDER_TEXTURE
							auto effect = EffectManager::getInstance()->addEffectAnimation(this, offset, connectId, duration300, animationData, false);
#else
							auto effect = EffectManager::getInstance()->addEffectAnimation(this, offset, connectId, duration300, animationData);
#endif

							// エフェクトを保持
							objectEffect->setEffectAnimation(effect);
						}
					} break;

					// パーティクル
					case agtk::data::ObjectEffectSettingData::EnumAnimationType::kAnimationTypeParticle: 
					{
						auto particleId = settingData->getParticleId();

						// 「設定無し」でない場合
						if (particleId > agtk::data::ObjectEffectSettingData::NO_SETTING) {

							// 生成対象のアニメーションデータを取得
							auto animationData = projectData->getAnimationData(particleId);

							Vec2 pos = Vec2::ZERO;

							switch (settingData->getPositionType()) {
								// このオブジェクトの中心
							case agtk::data::ObjectEffectSettingData::EnumPositionType::kPositionCenter:
							{
								pos = this->getCenterPosition() - this->getPosition();
							} break;

							// このオブジェクトの足元
							case agtk::data::ObjectEffectSettingData::EnumPositionType::kPositionFoot:
							{
								pos = this->getFootPosition() - this->getPosition();
							} break;

							// 接続点を使用
							case agtk::data::ObjectEffectSettingData::EnumPositionType::kPositionUseConnection:
							{
								connectId = settingData->getConnectionId();
							} break;
							}

							// 位置調整値を設定
							pos.x += settingData->getAdjustX();
							pos.y += settingData->getAdjustY();

							// パーティクルを生成
							auto particleList = animationData->getParticleList();

							// パーティクルグループ生成
							auto particleGroup = ParticleManager::getInstance()->addParticle(
								this,
								this->getSceneIdOfFirstCreated(),
								this->getLayerId(),
								animationData->getId(),
								pos,
								connectId,
								settingData->getDispDuration300(),
								settingData->getDispDurationUnlimited(),
								particleList
#ifdef USE_REDUCE_RENDER_TEXTURE
								, false
#endif
							);

							// パーティクルを保持
							objectEffect->setParticleGroup(particleGroup);
						}
					} break;
				}
			}
			// スイッチがOFFになった直後の場合
			else if (objectEffect->getSwitchValueOld() && !objectEffect->getSwitchValue()) {

				// アニメーションの種類に応じて処理を変化
				switch (settingData->getAnimationType()) {
					// エフェクト
					case agtk::data::ObjectEffectSettingData::EnumAnimationType::kAnimationTypeEffect:
					{
						auto effect = objectEffect->getEffectAnimation();

						if (EffectManager::getInstance()->existsEffect(effect)) {
							// エフェクトを削除
//							effect->deleteEffect();
							// エフェクトを停止
							effect->stopEffect();
						}
					} break;

					// パーティクル
					case agtk::data::ObjectEffectSettingData::EnumAnimationType::kAnimationTypeParticle:
					{
						auto particleGroup = objectEffect->getParticleGroup();

						if (ParticleManager::getInstance()->existsParticleGroup(particleGroup)) {
							// パーティクルを停止させる
							particleGroup->changeProccess(agtk::ParticleGroup::PARTICLE_PROC_TYPE::STOP, false);
						}
					} break;
				}
			}
		}
	}

	// オブジェクトの矩形情報を保存する
	auto objectCollision = this->getObjectCollision();
	if (objectCollision) {
		objectCollision->lateUpdateWallHitInfoGroup();
	}

	// 「カメラとの位置関係を固定する」設定時
	auto scene = this->getSceneLayer()->getScene();
	if (this->getObjectData()->getFixedInCamera() && this->getSceneLayer()->getType() != agtk::SceneLayer::kTypeMenu && _updateOneshotFunction == nullptr) {
		cocos2d::Vec2 pos = agtk::Scene::getPositionSceneFromCocos2d(_objectPosInCamera + scene->getCamera()->getPosition());
		this->setPosition(pos);
	}

	// オブジェクト同士の重なり判定（重なり演出用）
	if (_effectTargetType & OVERLAP_EFFECT_TARGET_OBJ) 
	{	
		auto const & objectList = GameManager::getInstance()->getCurrentScene()->getObjectAllFront(this->getSceneLayer()->getType(), getLayerId());
		cocos2d::Vec2 objPos = Vec2::ZERO;
		cocos2d::Size objSize = Size::ZERO;
		auto const player = getPlayer();
		if (player) {
			auto objScale = player->getPlayerScale();
			objPos = player->getCenterNodePosition2();
			objSize = player->getContentSize();
			objSize.width *= objScale.x;
			objSize.height *= objScale.y;
		}
		else {
			objPos = agtk::Scene::getPositionCocos2dFromScene(getCenterPosition());
		}
		cocos2d::Rect rect(objPos - objSize * 0.5f, objSize);

		cocos2d::Ref *ref;
		CCARRAY_FOREACH(objectList, ref) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
			auto const obj2 = static_cast<agtk::Object*>(ref);
#else
			auto const obj2 = dynamic_cast<agtk::Object*>(ref);
#endif
			CC_ASSERT(obj2);
			cocos2d::Vec2 obj2Pos = Vec2::ZERO;
			cocos2d::Size obj2Size = Size::ZERO;
			auto const player = obj2->getPlayer();
			if (player) {
				auto obj2Scale = player->getPlayerScale();
				obj2Pos = player->getCenterNodePosition2();
				obj2Size = player->getContentSize();
				obj2Size.width *= obj2Scale.x;
				obj2Size.height *= obj2Scale.y;
			}
			else {
				obj2Pos = agtk::Scene::getPositionCocos2dFromScene(obj2->getCenterPosition());
			}
			auto rect2 = cocos2d::Rect(obj2Pos - obj2Size * 0.5f, obj2Size);
			if (rect.intersectsRect(rect2)) {
				setOverlapFlag(true, OVERLAP_EFFECT_TARGET_OBJ);
			}
		}
	}
}

void Object::loopVertical(bool fixedCamera)
{
	auto scene = GameManager::getInstance()->getCurrentScene();
	auto sceneSize = scene->getSceneSize();

	auto pos = this->getPosition();

	if (pos.y < 0 || sceneSize.y < pos.y) {

		float moveVecY = this->getPosition().y - this->getOldPosition().y;

		// 座標を再設定する
		pos.y = pos.y - floor(pos.y / sceneSize.y) * sceneSize.y;
		this->setPosition(pos);

		// 過去の座標も再設定する
		pos.y -= moveVecY;
		this->setOldPosition(pos);
	}
}

void Object::loopHorizontal(bool fixedCamera)
{
	auto scene = GameManager::getInstance()->getCurrentScene();
	auto sceneSize = scene->getSceneSize();

	auto pos = this->getPosition();

	// 自座標がシーンサイズからはみ出た場合
	if (pos.x < 0 || sceneSize.x < pos.x) {

		float moveVecX = this->getPosition().x - this->getOldPosition().x;

		// 座標を再設定する
		pos.x = pos.x - floor(pos.x / sceneSize.x) * sceneSize.x;
		this->setPosition(pos);

		// 過去の座標も再設定する
		pos.x -= moveVecX;
		this->setOldPosition(pos);
	}

	// 固定カメラの場合
	if (fixedCamera) {
		auto rect = this->getRect();

		bool isLeftOut = (rect.getMinX() < 0);
		bool isRightOut = (sceneSize.x < rect.getMaxX());

		auto objectLoopScene = this->getObjectSceneLoop();

		// 右か左かにオブジェクトがはみ出ている場合
		if (isLeftOut || isRightOut) {
			// 反対側に表示するオブジェクトを生成
			if (objectLoopScene == nullptr) {
				objectLoopScene = agtk::ObjectSceneLoop::create(this);
				this->setObjectSceneLoop(objectLoopScene);
			}

			// 反対側にオブジェクトを表示する
			if (objectLoopScene != nullptr) {
				objectLoopScene->setVisible(true);
				objectLoopScene->update(0);

				// 反対側に表示するオブジェクトの座標を更新
				pos = Scene::getPositionCocos2dFromScene(pos);
				if (isLeftOut) {
					pos.x += sceneSize.x;
				}
				else if (isRightOut) {
					pos.x -= sceneSize.x;
				}

				objectLoopScene->setPosition(pos.x, pos.y);
			}
		}
		else {
			if (objectLoopScene != nullptr) {
				objectLoopScene->setVisible(false);
			}
		}
	}
}

/**
* ゲーム速度取得
*/
float Object::getGameSpeed()
{
	auto sceneLayer = this->getSceneLayer();
	auto scene = sceneLayer->getScene();

	auto sceneLayerType = sceneLayer->getType();
	float speed = scene->getGameSpeed()->getTimeScale(this);
	if (sceneLayerType == agtk::SceneLayer::kTypeMenu) {
		speed = scene->getGameSpeed()->getTimeScale(SceneGameSpeed::eTYPE_MENU);
	}

	return speed;
}

/**
 * シーンレイヤーのスクロール速度の違いでオブジェクト間の座標の基準が異なってしまうのに対し、シーンレイヤー間の座標のズレを計算して返す。
 */
cocos2d::Vec2 Object::getSceneLayerScrollDiff(agtk::SceneLayer *mySceneLayer, agtk::SceneLayer *targetSceneLayer)
{
	if (mySceneLayer == targetSceneLayer) {
		return cocos2d::Vec2::ZERO;
	}
	auto myLayerPos = mySceneLayer->getPosition();
	auto targetLayerPos = targetSceneLayer->getPosition();
	return cocos2d::Vec2(targetLayerPos.x - myLayerPos.x, myLayerPos.y - targetLayerPos.y);	// cocos2d-x座標系とアクツクシーン座標系の違いも考慮。
}

#ifdef FIX_ACT2_4732
void Object::setMoveAnimDispDirection(int directionId)
{
	auto objectData = this->getObjectData();
	auto moveType = objectData->getMoveType();
	if (moveType == agtk::data::ObjectData::kMoveTank || moveType == agtk::data::ObjectData::kMoveCar) {
		//移動方向も変更。
		auto objectMovement = this->getObjectMovement();
		objectMovement->setDirection(agtk::GetDirectionFromMoveDirectionId(directionId));
	}
	else {
		//基本移動
		auto player = this->getPlayer();
		if (player) {
			agtk::data::MotionData *motionData = nullptr;
			agtk::data::DirectionData *curDirectionData = nullptr;
			agtk::data::DirectionData *newDirectionData = nullptr;
			do {
				auto basePlayer = player->getBasePlayer();
				if (!basePlayer) break;
				auto motion = basePlayer->getCurrentAnimationMotion();
				if (!motion) break;
				motionData = motion->getMotionData();
				if (!motionData) break;
				auto direction = motion->getCurrentDirection();
				if (!direction) break;
				curDirectionData = direction->getDirectionData();
				if (!curDirectionData) break;
				auto directionData = this->getDirectionData(1 << directionId, this->getDispDirectionBit(), motionData, curDirectionData);
				if (directionData && (directionData->getAutoGeneration() || directionData->getDirectionBit() & (1 << directionId))) {
					newDirectionData = directionData;
					break;
				}
			} while (0);
			if (newDirectionData) {
				if (newDirectionData != curDirectionData) {
					cocos2d::log("%d", directionId);
					player->play(motionData->getId(), newDirectionData->getId());
				}
				setMoveDirection(directionId);
			}
		}
		else {
			setMoveDirection(directionId);
		}

		this->setDispDirection(directionId);
	}
}
#endif

void Object::updateAsChildBefore(float dt)
{
	auto parentObject = getOwnParentObject();
	if (parentObject == nullptr) {
		this->setPushedbackByObject(this->getObjectData()->getPushedbackByObject());
		return;
	}
	auto objectData = this->getObjectData();
	auto playObjectData = this->getPlayObjectData();

	// ACT2-5081 「他のオブジェクトから押し戻されない」または子オブジェクトで「親オブジェクトから離れず追従する」場合は、このオブジェクトを「他のオブジェクトから押し戻されない」扱いにする。
	// → 「他のオブジェクトから押し戻される」かつ （子オブジェクトでないか「親オブジェクトから離れず追従する」以外）の場合は、このオブジェクトを「他のオブジェクトから押し戻される」扱いにする。
	this->setPushedbackByObject(objectData->getPushedbackByObject() && objectData->getFollowType() != agtk::data::ObjectData::EnumFollowType::kFollowClose);
	//親オブジェクトから表示方向を引き継ぐ場合
	if (objectData->getTakeoverDispDirection()) {
		//表示方向
		bool bChangeDirectionId = false;
		auto directionId = parentObject->getDispDirection();
		if (directionId != this->getDispDirection()) {
			this->setDispDirection(directionId);
			bChangeDirectionId = true;
		}

		// 自動生成の場合
		if (this->isAutoGeneration()) {

			Vec2 direction = Vec2::ZERO;

			// 親オブジェクトも自動生成の場合
			if (parentObject->isAutoGeneration()) {
				direction = parentObject->getObjectMovement()->getDirection();
			}
			else {
				direction = agtk::GetDirectionFromMoveDirectionId(directionId);
			}

			// 表示方向を基にdirectionを設定
			auto objectMovement = this->getObjectMovement();
			if (objectMovement != nullptr) {
				objectMovement->setDirection(direction);
			}
		}
		else {
			if (bChangeDirectionId) {
#if 0//def FIX_ACT2_4732
				setMoveAnimDispDirection(directionId);
#else
				int actionId = this->getCurrentObjectAction()->getId();
				this->playAction(actionId, directionId);
				this->setDispDirection(directionId);
#endif
			}
		}
	}
	// 「親オブジェクトを追従しない」または
	// 「物理演算」が ON かつ「接続された物理オブジェクトの動作を優先」がON
	// の場合は特に処理しない
	if (objectData->getFollowType() == agtk::data::ObjectData::EnumFollowType::kFollowNone ||
		(objectData->getPhysicsSettingFlag() && playObjectData->getSwitchData(agtk::data::kObjectSystemSwitchFollowConnectedPhysics)->getValue())) {
		return;
	}

	//１フレーム前の位置を保持。
	auto pos = this->getPosition();
	this->setOldPosition(pos);
	auto thisPlayer = this->getPlayer();
	if (thisPlayer) {
		this->setOldWallPosition(thisPlayer->getPosition());
	}

}

void Object::updateAsChild(float dt) 
{
	// 親オブジェクトがいる場合のみ処理を行う
	if (getOwnParentObject()) {

		auto parentObject = getOwnParentObject();
		auto objectData = this->getObjectData();
		auto playObjectData = this->getPlayObjectData();

		// 親オブジェクトからスケールを引き継ぐ場合
		if (objectData->getTakeoverScaling()) {

			float scaleX = parentObject->getScaleX();
			float scaleY = parentObject->getScaleY();
			this->setScale(cocos2d::Vec2(scaleX, scaleY));
			if (parentObject->getPlayer() && this->getPlayer()) {
				//アニメーションのスケールをプレイヤーのスケールに設定。
				scaleX = parentObject->getPlayer()->getVisibleCtrlNode()->getScaleX();
				scaleY = parentObject->getPlayer()->getVisibleCtrlNode()->getScaleY();
				this->getPlayer()->getNodePlayer()->setScale(scaleX, scaleY);
			}
		}

		// 親オブジェクトから角度を引き継ぐ場合
		if (objectData->getTakeoverAngle()) {
			float rot = 0;
			// 下記の2点を全て満たす場合
			// ・親オブジェクトが回転で自動生成されている
			// ・親オブジェクトのプレイヤーが存在する
			if (parentObject->getPlayer() && this->getPlayer()) {
				// 親オブジェクトが回転で自動生成 かつ 自身が回転で自動生成
				if (parentObject->isAutoGeneration() && this->isAutoGeneration()) {
					// 親オブジェクトの自動生成用で用いられた角度を設定
					rot = parentObject->getPlayer()->getVisibleCtrlNode()->getRotation();
					this->getPlayer()->getVisibleCtrlNode()->setRotation(rot);
					rot = parentObject->getPlayer()->getCenterRotation();
					this->getPlayer()->setCenterRotation(rot);
				}
				else {
					//アニメーションの回転角度をオブジェクトの中心に設定する。
					rot += parentObject->getPlayer()->getVisibleCtrlNode()->getRotation();
					rot += parentObject->getPlayer()->getCenterRotation();
					this->getPlayer()->setCenterRotation(rot);
				}
			}
			rot = parentObject->getRotation();
			this->setRotation(rot);
		}

		// 親オブジェクトからフィルター効果を引き継ぐ場合
		if (objectData->getTakeoverIntensity()) {

			auto parentPlayer = parentObject->getPlayer();
			auto thisPlayer = this->getPlayer();

			if (parentPlayer && thisPlayer) {
				// プレイヤーに「ノイズ」設定時は、 シェーダに設定されている値を反映
				auto shader = parentPlayer->getShader(agtk::Shader::kShaderNoisy);
				if (shader) {
					thisPlayer->setShader(agtk::Shader::kShaderNoisy, shader->getValue()->getValue());
				}
				else {
					thisPlayer->removeShader(agtk::Shader::kShaderNoisy);
				}

				// プレイヤーに「モザイク」設定時は、シェーダに設定されている値を反映
				shader = parentPlayer->getShader(agtk::Shader::kShaderMosaic);
				if (shader) {
					thisPlayer->setShader(agtk::Shader::kShaderMosaic, shader->getValue()->getValue());
				}
				else {
					thisPlayer->removeShader(agtk::Shader::kShaderMosaic);
				}

				// プレイヤーに「モノクロ」設定時は、シェーダに設定されている値を反映
				shader = parentPlayer->getShader(agtk::Shader::kShaderColorGray);
				if (shader) {
					thisPlayer->setShader(agtk::Shader::kShaderColorGray, shader->getValue()->getValue());
				}
				else {
					thisPlayer->removeShader(agtk::Shader::kShaderColorGray);
				}

				// プレイヤーに「セピア」設定時は、シェーダに設定されている値を反映
				shader = parentPlayer->getShader(agtk::Shader::kShaderColorSepia);
				if (shader) {
					thisPlayer->setShader(agtk::Shader::kShaderColorSepia, shader->getValue()->getValue());
				}
				else {
					thisPlayer->removeShader(agtk::Shader::kShaderColorSepia);
				}

				// プレイヤーに「ネガ反転」設定時は、シェーダに設定されている値を反映
				shader = parentPlayer->getShader(agtk::Shader::kShaderColorNega);
				if (shader) {
					thisPlayer->setShader(agtk::Shader::kShaderColorNega, shader->getValue()->getValue());
				}
				else {
					thisPlayer->removeShader(agtk::Shader::kShaderColorNega);
				}

				// プレイヤーに「ぼかし」設定時は、シェーダに設定されている値を反映
				shader = parentPlayer->getShader(agtk::Shader::kShaderBlur);
				if (shader) {
					thisPlayer->setShader(agtk::Shader::kShaderBlur, shader->getValue()->getValue());
				}
				else {
					thisPlayer->removeShader(agtk::Shader::kShaderBlur);
				}

				// プレイヤーに「色収差」設定時は、シェーダに設定されている値を反映
				shader = parentPlayer->getShader(agtk::Shader::kShaderColorChromaticAberration);
				if (shader) {
					thisPlayer->setShader(agtk::Shader::kShaderColorChromaticAberration, shader->getValue()->getValue());
				}
				else {
					thisPlayer->removeShader(agtk::Shader::kShaderColorChromaticAberration);
				}

				// プレイヤーに「暗闇」設定時は、シェーダに設定されている値を反映
				shader = parentPlayer->getShader(agtk::Shader::kShaderColorDark);
				if (shader) {
					thisPlayer->setShader(agtk::Shader::kShaderColorDark, shader->getValue()->getValue());
				}
				else {
					thisPlayer->removeShader(agtk::Shader::kShaderColorDark);
				}

				// プレイヤーに「透明」設定時は、シェーダに設定されている値を反映
				shader = parentPlayer->getShader(agtk::Shader::kShaderTransparency);
				if (shader) {
					thisPlayer->setShader(agtk::Shader::kShaderTransparency, shader->getValue()->getValue());
				}
				else {
					thisPlayer->removeShader(agtk::Shader::kShaderTransparency);
				}

				// 親オブジェクトの「点滅」状態を引き継ぐ。
				if (parentObject->getObjectVisible()->isBlinking()) {
					this->getObjectVisible()->takeOverBlink(parentObject->getObjectVisible());
				}
				else {
					this->getObjectVisible()->endBlink(0.0f);
				}

				// プレイヤーに「画像表示」設定時は、スプライトを設定する
				auto execActionSpriteList = parentPlayer->getExecActionSpriteList();
				if(execActionSpriteList->count() > 0) {
					cocos2d::Ref *ref;
					CCARRAY_FOREACH(execActionSpriteList, ref) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
						auto sprite = static_cast<agtk::PlayerSprite *>(ref);
#else
						auto sprite = dynamic_cast<agtk::PlayerSprite *>(ref);
#endif
						thisPlayer->setExecActionSprite(sprite->getImageId(), sprite->getOpacity());
					}
				}
				else {
					thisPlayer->removeExecActionSprite(0.0f);
				}

				// プレイヤーに「指定色で塗る」設定時は、シェーダに設定されている値を反映
				shader = parentPlayer->getShader(agtk::Shader::kShaderColorRgba);
				if (shader) {

					auto color = shader->getShaderRgbaColor();
					thisPlayer->setShader(agtk::Shader::kShaderColorRgba, 1);
					thisPlayer->getShader(agtk::Shader::kShaderColorRgba)->setShaderRgbaColor(
						shader->getShaderRgbaColor()
					);
				}
				else {
					thisPlayer->removeShader(agtk::Shader::kShaderColorRgba);
				}
			}
		}

		// 「親オブジェクトを追従しない」または
		// 「物理演算」が ON かつ「接続された物理オブジェクトの動作を優先」がON
		// の場合は特に処理しない
		if (objectData->getFollowType() == agtk::data::ObjectData::EnumFollowType::kFollowNone ||
			(objectData->getPhysicsSettingFlag() && playObjectData->getSwitchData(agtk::data::kObjectSystemSwitchFollowConnectedPhysics)->getValue())) {
			return;
		}

		// 移動開始位置を自身の座標に設定
		cocos2d::Vec2 startPos = agtk::Scene::getPositionCocos2dFromScene(this->getPosition());

		// 目標位置を親オブジェクトの中心座標に設定
		std::function<cocos2d::Vec2(void)> calcTargetPosition = [&]() {
			cocos2d::Vec2 pos;
			auto connectObj = dynamic_cast<agtk::ConnectObject *>(this);
			if(connectObj) {
				auto settingData = connectObj->getObjectConnectSettingData();
				switch (settingData->getPositionType()) {
				// このオブジェクトの中心
				case agtk::data::ObjectConnectSettingData::EnumPositionType::kPositionCenter: {
					pos = parentObject->getCenterPosition();
				} break;
				// このオブジェクトの足元
				case agtk::data::ObjectConnectSettingData::EnumPositionType::kPositionFoot: {
					pos = parentObject->getFootPosition();
				} break;
				// 接続点を使用
				case agtk::data::ObjectConnectSettingData::EnumPositionType::kPositionUseConnection: {
					int connectId = settingData->getConnectionId();
					pos = parentObject->getCenterPosition();
					if (connectId > 0) {
						agtk::Vertex4 vertex4;
						if (parentObject->getTimeline(agtk::data::TimelineInfoData::kTimelineConnection, connectId, vertex4)) {
							pos = Scene::getPositionSceneFromCocos2d(vertex4.addr()[0], parentObject->getSceneData());
						}
					}
				} break;
				}
			}
			else {
				pos = parentObject->getCenterPosition();
				if (_parentFollowConnectId > 0) {
					agtk::Vertex4 vertex4;
					if (parentObject->getTimeline(agtk::data::TimelineInfoData::kTimelineConnection, _parentFollowConnectId, vertex4)) {
						pos = Scene::getPositionSceneFromCocos2d(vertex4.addr()[0], parentObject->getSceneData());
					}
				}
			}
			return pos + _parentFollowPosOffset;
		};
		cocos2d::Vec2 targetPos = calcTargetPosition();

		switch (objectData->getFollowType()) {

			// 「親オブジェクトから離れず追従する」
		case agtk::data::ObjectData::EnumFollowType::kFollowClose: {

			// 目標位置に固定化する
			this->setPosition(targetPos);
		} break;

			// 「親オブジェクトと一定間隔をあけて追従する」「時間で設定（秒）」
		case agtk::data::ObjectData::EnumFollowType::kFollowNearTime: {

#ifdef FIX_ACT2_5335
			targetPos = agtk::Scene::getPositionCocos2dFromScene(targetPos);
			if (_parentFollowPosHead < 0) {
				_parentFollowPosHead = 0;
				_parentFollowPosTail = 0;
				_parentFollowPosWeight = -objectData->getFollowIntervalByTime300() / 300.0f + FRAME_PER_SECONDS;
				_parentFollowPosList[0]._pos = targetPos;
				_parentFollowPosList[0]._dt = 0;
			}
			else {
                auto addedDt = _parentFollowPosList[_parentFollowPosHead]._dt + dt;
				if (addedDt >= FRAME_PER_SECONDS) {
                    while (addedDt >= FRAME_PER_SECONDS) {
                        // (1/60)経過したら記録する。
                        auto remaining = addedDt - FRAME_PER_SECONDS;
                        _parentFollowPosList[_parentFollowPosHead]._dt = FRAME_PER_SECONDS;
                        auto next = (_parentFollowPosHead + 1) % _parentFollowPosList.size();
                        if(next == _parentFollowPosTail){
                            // dtが一気に進んだためにoverflow発生。
                            CCLOG("overflow1");
                            break;
                        }
                        _parentFollowPosHead = next;
                        _parentFollowPosList[_parentFollowPosHead]._pos = targetPos;
                        _parentFollowPosList[_parentFollowPosHead]._dt = remaining;
                        addedDt = remaining;
                    }
                } else {
					_parentFollowPosList[_parentFollowPosHead]._dt = addedDt;
				}
			}
			_parentFollowPosWeight += dt;
			//cocos2d::log("_parentFollowPosWeight: %f, head: %d, tail: %d", _parentFollowPosWeight, _parentFollowPosHead, _parentFollowPosTail);
			cocos2d::Vec2 pos;
			if (_parentFollowPosWeight < 0) {
                // 初回から一定時間がまだ経過していない。
				pos = _parentFollowPosList[_parentFollowPosTail]._pos;
			}
			else {
				if (_parentFollowPosTail == _parentFollowPosHead) {
                    // 次の記録が無い。
					pos = _parentFollowPosList[_parentFollowPosTail]._pos;
				} else {
                    while (_parentFollowPosWeight >= _parentFollowPosList[_parentFollowPosTail]._dt) {
                        // 記録時間が経過したので次に進める。
                        _parentFollowPosWeight -= _parentFollowPosList[_parentFollowPosTail]._dt;
                        auto next = (_parentFollowPosTail + 1) % _parentFollowPosList.size();
                        if(next == _parentFollowPosHead){
                            // dtが一気に進んだためにoverflow発生。
							CCLOG("overflow2");
                            _parentFollowPosWeight = 0;
                            break;
                        }
						_parentFollowPosTail = next;
                    }
					auto weight = _parentFollowPosWeight / _parentFollowPosList[_parentFollowPosTail]._dt;
					if (weight > 1) {
						CCLOG("weight is over: %f", weight);
						weight = 1;
					}
					pos = _parentFollowPosList[_parentFollowPosTail]._pos * (1 - weight) + _parentFollowPosList[(_parentFollowPosTail + 1) % _parentFollowPosList.size()]._pos * weight;
				}
			}
			auto lpos = agtk::Scene::getPositionCocos2dFromScene(this->getPosition());
			float length = (pos - lpos).getLength();
			float speed = length * objectData->getFollowPrecision() / 100;
			this->getObjectMovement()->startForceMoveTime(lpos, pos, speed);
#else
			// 待ちフレーム更新
			_parentFollowDuration300 -= dt * 300;
			if (_parentFollowDuration300 <= 0) {
				// 待ちフレームを再設定
				_parentFollowDuration300 = objectData->getFollowIntervalByTime300();
				// 座標を更新
				_parentFollowPos = calcTargetPosition();
			}

			// 移動スピード設定処理
			targetPos = agtk::Scene::getPositionCocos2dFromScene(_parentFollowPos);
			float length = (targetPos - startPos).getLength();
			float speed = length * 0.1f;
			this->getObjectMovement()->startForceMoveTime(startPos, targetPos, speed);
#endif

		} break;

			// 「親オブジェクトと一定間隔をあけて追従する」「移動量で設定（ドット）」
		case agtk::data::ObjectData::EnumFollowType::kFollowNearDot: {

			targetPos = agtk::Scene::getPositionCocos2dFromScene(targetPos);
#ifdef FIX_ACT2_5335
			if (_parentFollowPosHead < 0) {
				_parentFollowPosHead = 0;
				_parentFollowPosTail = 0;
				//_parentFollowPosWeight = 0;
				_parentFollowPosList[0]._pos = targetPos;
				_parentFollowPosList[0]._dt = 0;
			}
			else {
				auto len = _parentFollowPosList[_parentFollowPosHead]._pos.getDistance(targetPos);
				if (len >= LOCUS_LENGTH_DIV_LEN) {
					//auto remaining = _parentFollowPosList[_parentFollowPosHead]._dt + dt - FRAME_PER_SECONDS;
					_parentFollowPosList[_parentFollowPosHead]._dt = len;
					auto next = (_parentFollowPosHead + 1) % _parentFollowPosList.size();
                    if(next == _parentFollowPosTail){
						CCLOG("overflow3");
                        break;
                    }
                    _parentFollowPosHead = next;
					_parentFollowPosList[_parentFollowPosHead]._pos = targetPos;
					_parentFollowPosList[_parentFollowPosHead]._dt = 0;
					//_parentFollowPosWeight += len;
				}
			}
			auto locusLength = objectData->getFollowIntervalByLocusLength();
			float sum = 0;
			auto head = _parentFollowPosHead;
            while(head != _parentFollowPosTail){
				head = (head - 1 + _parentFollowPosList.size()) % _parentFollowPosList.size();
                sum += _parentFollowPosList[head]._dt;
                if(sum >= locusLength){
                    break;
                }
            }
			//cocos2d::log("sum: %f, locusLength: %f, head: %d, tail: %d, newTail: %d", sum, locusLength, _parentFollowPosHead, _parentFollowPosTail, head);
			cocos2d::Vec2 pos;
            if(_parentFollowPosTail == _parentFollowPosHead || sum < locusLength){
				pos = _parentFollowPosList[_parentFollowPosTail]._pos;
            } else {
				if (_parentFollowPosTail != head) {
					_parentFollowPosTail = head;
				}
				CC_ASSERT(sum - locusLength <= _parentFollowPosList[head]._dt);
                auto weight = (sum - locusLength) / _parentFollowPosList[head]._dt;
                auto next = (head + 1) % _parentFollowPosList.size();
				pos = _parentFollowPosList[head]._pos * (1 - weight) + _parentFollowPosList[next]._pos * weight;
            }
			auto lpos = agtk::Scene::getPositionCocos2dFromScene(this->getPosition());
			float length = (pos - lpos).getLength();
			float speed = length * objectData->getFollowPrecision() / 100;
			this->getObjectMovement()->startForceMoveTime(lpos, pos, speed);
#else
			float length = (targetPos - startPos).getLength();

			// 一定距離内にいる場合は処理しない
			if (length <= objectData->getFollowIntervalByLocusLength()) {
				return;
			}

			// 一定距離をあける処理を行う
			cocos2d::Vec2 dir = targetPos - startPos;
			dir.normalize();
			dir.scale(length - objectData->getFollowIntervalByLocusLength());
			targetPos = startPos;
			targetPos += dir;

			length = (targetPos - startPos).getLength();
			float speed =  length * 0.1f;
			this->getObjectMovement()->startForceMoveTime(startPos, targetPos, speed);
#endif
		} break;
		}
	}
}

void Object::clearCollision()
{
	if (_bNoClearCollision) {
		_bNoClearCollision = false;
		return;
	}
	//clear collision
	this->getCollisionAttackHitList()->removeAllObjects();
	this->getCollisionHitAttackList()->removeAllObjects();
	this->setCollisionWallWallChecked(false);
	this->getCollisionWallWallList()->removeAllObjects();
	this->getCollisionPortalHitList()->removeAllObjects();
	this->getAttackObjectList()->removeAllObjects();
	this->getAttackerObjectInstanceIdList()->removeAllObjects();
}

bool Object::isCollision()
{
	if (this->getCollisionAttackHitList()->count() > 0
	|| this->getCollisionHitAttackList()->count() > 0
	|| this->getCollisionWallWallChecked()
	|| this->getCollisionWallWallList()->count() > 0
	|| this->getCollisionPortalHitList()->count() > 0) {
		return true;
	}
	return false;
}

void Object::updateCollision()
{
	this->getObjectCollision()->update();
}

void Object::visit(Renderer *renderer, const Mat4& parentTransform, uint32_t parentFlags)
{
	agtk::Player *player = this->getPlayer();
	if (player) {
		player->setVisible(_playerVisible);
	}
	Node::visit(renderer, parentTransform, parentFlags);
	if (player) {
		player->setVisible(false);
	}
}

void Object::updateFixedFrame(int value)
{
	auto player = this->getPlayer();
	if (player) {
		auto basePlayer = player->getBasePlayer();
		if (basePlayer != nullptr) {
			int curFrame = basePlayer->getFixedFrame();
			if (curFrame != value) {
				basePlayer->setFixedFrame(value, false);
				if (value == agtk::AnimationMotion::kAnimPauseFrameValue) {
					// agtk::AnimationMotion::kAnimPauseFrameValueが指定されたら、現在再生中のフレームを変数に書き戻す。
					auto variableData = getPlayObjectData()->getVariableData(agtk::data::kObjectSystemVariableFixedAnimationFrame);
					variableData->setValue(basePlayer->getFixedFrame());
				}
			}
		}
	}
}

#ifdef USE_MULTITHREAD_OBJECT_UPDATE
LockObjList::~LockObjList()
{
	if (_lockedObjList.size() > 0) {
		//THREAD_PRINTF("LockObjList(0x%x)::~LockObjList: unlock: 0, 0x%x", this, _lockedObjList[0]);
		_lockedObjList[0]->unlock();
	}
}

void LockObjList::Add(Object *obj)
{
	//THREAD_PRINTF("LockObjList(0x%x)::Add: object: 0x%x", this, obj);
	auto wallCollisionUpdateOrder = obj->getWallCollisionUpdateOrder();
	if (_lockedObjList.size() == 0 || wallCollisionUpdateOrder < _lockedObjList[0]->getWallCollisionUpdateOrder()) {
		if(_lockedObjList.size() > 0) {
			//THREAD_PRINTF("LockObjList(0x%x)::Add: unlock: 0, 0x%x", this, _lockedObjList[0]);
			_lockedObjList[0]->unlock();
		}
		//THREAD_PRINTF("LockObjList(0x%x)::Add: lock: 0, 0x%x", this, obj);
		obj->lock();
		_lockedObjList.insert(_lockedObjList.begin(), obj);
		//THREAD_PRINTF("LockObjList(0x%x)::Add: leave", this);
		return;
	}
	auto it = _lockedObjList.begin();
	while (it != _lockedObjList.end()) {
		if ((*it)->getWallCollisionUpdateOrder() > wallCollisionUpdateOrder) {
			break;
		}
		it++;
	}
	_lockedObjList.insert(it, obj);
	//THREAD_PRINTF("LockObjList(0x%x)::Add: leave", this);
}

void LockObjList::Remove(Object *obj)
{
	//THREAD_PRINTF("LockObjList(0x%x)::Remove: object: 0x%x", this, obj);
	if (_lockedObjList.size() > 0 && _lockedObjList[0] == obj) {
		if (_lockedObjList.size() >= 2 && _lockedObjList[1] == obj) {
			_lockedObjList.erase(_lockedObjList.begin() + 1);
		}
		else {
			if (_lockedObjList.size() >= 2) {
				//THREAD_PRINTF("LockObjList(0x%x)::Remove: lock: 1, 0x%x", this, _lockedObjList[1]);
				_lockedObjList[1]->lock();
			}
			//THREAD_PRINTF("LockObjList(0x%x)::Remove: unlock: 0, 0x%x", this, obj);
			obj->unlock();
			_lockedObjList.erase(_lockedObjList.begin());
		}
		//THREAD_PRINTF("LockObjList(0x%x)::Remove: leave", this);
		return;
	}
	auto it = _lockedObjList.begin();
	while (it != _lockedObjList.end()) {
		if ((*it) == obj) {
			break;
		}
		it++;
	}
	if (it == _lockedObjList.end()) {
		char buf[16];
		sprintf(buf, "");
		return;
	}
	_lockedObjList.erase(it);
	//THREAD_PRINTF("LockObjList(0x%x)::Remove: leave", this);
}

#endif
/**
* 壁判定
* @param	x			判定時のX座標
* @param	y			判定時のY座標
*/
void Object::wallCollisionCorrection(float x, float y, EnumChangeAction changeAction)
{
#ifdef USE_MULTITHREAD_OBJECT_UPDATE
	LockObjList lockObjList;
	lockObjList.Add(this);
#endif
	if (this->getBasePlayer() == nullptr) {
		return;
	}
	//自由移動モード
	auto objectData = this->getObjectData();
	auto playObjectData = this->getPlayObjectData();
	if (DebugManager::getInstance()->getFreeMovingEnabled() || 
		this->getPlayObjectData()->getSwitchData(agtk::data::kObjectSystemSwitchFreeMove)->getValue()) 
	{
		if (objectData->isGroupPlayer()) {
			return;
		}
	}

// #AGTK-NX #AGTK-WIN
#ifdef USE_30FPS_3
	auto gm = GameManager::getInstance();
#endif

	int objectSameLayerWallBit = this->getObjectSameLayerWallBit();
	bool buriedInWallFlag = this->getBuriedInWallFlag();
	auto tileWallBit = this->getTileWallBit();
	ObjectCollision::HitTileWalls aheadTileWall = this->getAheadTileWall();
	{

		//「オブジェクトのエリア判定変数に数字を代入」クリア。
		auto playData = this->getPlayObjectData();

// #AGTK-NX #AGTK-WIN
#ifdef USE_30FPS_3
		if (gm->getPassIndex() == 0) {
#endif
		playData->setSystemVariableData(agtk::data::kObjectSystemVariableAreaAttribute, -1);

		this->getObjectMovement()->getObjectMoveLiftList()->removeAllObjects();

		/*
		_leftWallObjectList->removeAllObjects();
		_rightWallObjectList->removeAllObjects();
		_upWallObjectList->removeAllObjects();
		_downWallObjectList->removeAllObjects();
		*/
		_slopeTouchedList->removeAllObjects();
		_overlapFlag = 0;

		//reset wall effect
		resetObjectWallEffect(this);

// #AGTK-NX #AGTK-WIN
#ifdef USE_30FPS_3
			_tempTileWallBit = 0;
			_tempLinkConditionTileWall.clear();
			_tempSlopeBit = 0;
			_tempAheadTileWall.clear();
		} // if (gm->getPassIndex() == 0) {}
#endif
		// 四分木衝突判定による衝突チェックを実施
		auto sceneLayer = this->getSceneLayer();
		if (sceneLayer) {
#ifdef USE_MULTITHREAD_OBJECT_UPDATE
			sceneLayer->updateThreadWallCollision(this, CC_CALLBACK_1(Object::callbackDetactionWallCollision, this));
#else
			sceneLayer->updateWallCollision(this, CC_CALLBACK_1(Object::callbackDetactionWallCollision, this));
#endif
		}

		// 現時点の座標を基にタイルと他オブジェクトとの衝突を解決
		float moveX = 0;
		float moveY = 0;
		int tileWallBit = 0;
		ObjectCollision::HitTileWalls linkConditionTileWall;
		int slopeBit = 0;
		ObjectCollision::HitTileWalls aheadTileWall;
		bool buriedInWallFlag = false;
		cocos2d::Vec2 returnedPos;
#ifdef USE_MULTITHREAD_OBJECT_UPDATE
		this->getObjectWallCollision()->updateWall(lockObjList, x, y, &moveX, &moveY, &tileWallBit, &linkConditionTileWall, &aheadTileWall, &slopeBit, _leftWallObjectList, _rightWallObjectList, _upWallObjectList, _downWallObjectList, &buriedInWallFlag, returnedPos);
#else
		this->getObjectWallCollision()->updateWall(x, y, &moveX, &moveY, &tileWallBit, &linkConditionTileWall, &aheadTileWall, &slopeBit, _leftWallObjectList, _rightWallObjectList, _upWallObjectList, _downWallObjectList, &buriedInWallFlag, returnedPos);
#endif

// #AGTK-NX #AGTK-WIN
#if defined(USE_30FPS_3) && !defined(USE_30FPS_3_2)
		if (gm->getPassCount() > 1) {
			// 2パス分の壁判定結果のマージ
			if (!gm->getLastPass()) {
				_tempTileWallBit = tileWallBit;
				_tempLinkConditionTileWall = linkConditionTileWall;
				_tempSlopeBit = slopeBit;
				_tempAheadTileWall = aheadTileWall;
			}
			else {
				tileWallBit |= _tempTileWallBit;
				if (_tempLinkConditionTileWall.size() > 0) {
					ObjectCollision::HitTileWalls tempWall = _tempLinkConditionTileWall;
					tempWall.reserve(linkConditionTileWall.size() + _tempLinkConditionTileWall.size());
					std::copy(linkConditionTileWall.begin(), linkConditionTileWall.end(), std::back_inserter(tempWall));
					linkConditionTileWall = std::move(tempWall);
				}
				slopeBit |= _tempSlopeBit;
				if (_tempAheadTileWall.size() > 0) {
					ObjectCollision::HitTileWalls tempWall = _tempAheadTileWall;
					tempWall.reserve(aheadTileWall.size() + _tempAheadTileWall.size());
					std::copy(aheadTileWall.begin(), aheadTileWall.end(), std::back_inserter(tempWall));
					aheadTileWall = std::move(tempWall);
				}
			}
		}
#endif
// #AGTK-NX #AGTK-WIN
#ifdef USE_30FPS_3
		if(gm->getLastPass()) {
#endif
		// 衝突している「タイル」と「坂」の接触情報を設定
		this->setTileWallBit(tileWallBit);
#ifdef USE_MULTITHREAD_OBJECT_UPDATE
		this->autoReleaseRetainTileWallList(&linkConditionTileWall, &aheadTileWall);
#else
		this->setLinkConditionTileWall(linkConditionTileWall);
		this->setAheadTileWall(aheadTileWall);
#endif
		this->setSlopeBit(slopeBit);
		//CCLOG("tileWallBit: 0x%x, aheadTileWallBit: 0x%x", tileWallBit, aheadTileWallBit);

		//! ---------------------------------------------------------------------------
		//! オブジェクトの上下左右の壁判定に衝突した他オブジェクトの衝突ビット値を設定
		//! ---------------------------------------------------------------------------
		int objectWallBit = 0;
		int objectSameLayerWallBit = 0;
		cocos2d::Ref *ref = nullptr;

		if (_leftWallObjectList->count()) {
			objectWallBit |= agtk::data::ObjectActionLinkConditionData::kWallBitLeft;

#ifdef USE_MULTITHREAD_OBJECT_UPDATE
			auto size = _leftWallObjectList->size();
			for(int i = 0; i < (int)size; i++){
				auto obj = (*_leftWallObjectList)[i];
#else
			CCARRAY_FOREACH(_leftWallObjectList, ref) {
				auto obj = dynamic_cast<agtk::Object*>(ref);
#endif
				if (obj->getLayerId() == this->getLayerId()) {
					objectSameLayerWallBit |= agtk::data::ObjectActionLinkConditionData::kWallBitLeft;
					break;
				}
			}
		}
		if (_rightWallObjectList->count()) {
			objectWallBit |= agtk::data::ObjectActionLinkConditionData::kWallBitRight;

#ifdef USE_MULTITHREAD_OBJECT_UPDATE
			auto size = _rightWallObjectList->size();
			for (int i = 0; i < (int)size; i++) {
				auto obj = (*_rightWallObjectList)[i];
#else
			CCARRAY_FOREACH(_rightWallObjectList, ref) {
				auto obj = dynamic_cast<agtk::Object*>(ref);
#endif
				if (obj->getLayerId() == this->getLayerId()) {
					objectSameLayerWallBit |= agtk::data::ObjectActionLinkConditionData::kWallBitRight;
					break;
				}
			}
		}
		if (_upWallObjectList->count()) {
			objectWallBit |= agtk::data::ObjectActionLinkConditionData::kWallBitUp;

#ifdef USE_MULTITHREAD_OBJECT_UPDATE
			auto size = _upWallObjectList->size();
			for (int i = 0; i < (int)size; i++) {
				auto obj = (*_upWallObjectList)[i];
#else
			CCARRAY_FOREACH(_upWallObjectList, ref) {
				auto obj = dynamic_cast<agtk::Object*>(ref);
#endif
				if (obj->getLayerId() == this->getLayerId()) {
					objectSameLayerWallBit |= agtk::data::ObjectActionLinkConditionData::kWallBitUp;
					break;
				}
			}
		}
		if (_downWallObjectList->count()) {
			objectWallBit |= agtk::data::ObjectActionLinkConditionData::kWallBitDown;

#ifdef USE_MULTITHREAD_OBJECT_UPDATE
			auto size = _downWallObjectList->size();
			for (int i = 0; i < (int)size; i++) {
				auto obj = (*_downWallObjectList)[i];
#else
			CCARRAY_FOREACH(_downWallObjectList, ref) {
				auto obj = dynamic_cast<agtk::Object*>(ref);
#endif
				if (obj->getLayerId() == this->getLayerId()) {
					objectSameLayerWallBit |= agtk::data::ObjectActionLinkConditionData::kWallBitDown;
					break;
				}
			}
		}
		this->setObjectWallBit(objectWallBit);
		this->setObjectSameLayerWallBit(objectSameLayerWallBit);
// #AGTK-NX #AGTK-WIN
#ifdef USE_30FPS_3
		} // if (gm->getLastPass()) {}
#endif
 // #AGTK-NX #AGTK-WIN
#ifdef USE_30FPS_3
		if (gm->getPassCount() > 1) {
#ifdef USE_30FPS_3_2
			if (gm->getLastPass()) {
				// pass2
				this->setReturnedPos(returnedPos);
			}
#else
			if (!gm->getLastPass()) {
				// pass1
				this->_tempBuriedInWallFlag = buriedInWallFlag;
				this->_tempReturnedPos = returnedPos;
			}
			else {
				// pass2
				buriedInWallFlag |= this->_tempBuriedInWallFlag;
				if (this->_tempReturnedPos.x != 0.0f) {
					returnedPos.x = this->_tempReturnedPos.x;
				}
				if (this->_tempReturnedPos.y != 0.0f) {
					returnedPos.y = this->_tempReturnedPos.y;
				}
				this->setReturnedPos(returnedPos);
			}
#endif
		}
		else {
			this->setReturnedPos(returnedPos);
		}
#else
		// this->setBuriedInWallFlag(buriedInWallFlag);
		this->setReturnedPos(returnedPos);
#endif

		// 物理演算設定が OFF の場合
		if (!objectData->getPhysicsSettingFlag()) {
			//x += moveX;
			//y += moveY;
			x = std::roundf((x + moveX) * 10000.0f) * 0.0001f;
			y = std::roundf((y + moveY) * 10000.0f) * 0.0001f;
			auto player = this->getPlayer();
			if (player) {
				player->setPosition(x, y);
			}
		}
		// 物理演算設定が ON かつ 接続した物理オブジェクトの動作を優先が OFF の場合
		else if (!playObjectData->getSwitchData(agtk::data::kObjectSystemSwitchFollowConnectedPhysics)->getValue()) {
			//x += moveX;
			//y += moveY;
			x = std::roundf((x + moveX) * 10000.0f) * 0.0001f;
			y = std::roundf((y + moveY) * 10000.0f) * 0.0001f;
			// 「物理環境、他の物理演算オブジェクトから影響を受ける」が ON の場合
			if (playObjectData->getSwitchData(agtk::data::kObjectSystemSwitchAffectedByOtherObjects)->getValue()) {
				// 衝突判定による押し戻しベクトルを保持
				// ※ここで座標の変更を反映すると物理演算時の衝突判定が行えなく無くなるので、
				// 押し戻し量を保持して物理演算後に反映する
// #AGTK-NX #AGTK-WIN
#ifdef USE_30FPS_3
				if (gm->getPassCount() > 1) {
					if (!gm->getLastPass()) {
						this->setCollisionPushBackVec(Vec2(moveX, moveY));
					}
					else {
						this->setCollisionPushBackVec(this->getCollisionPushBackVec() + Vec2(moveX, moveY));
					}
				}
				else {
					this->setCollisionPushBackVec(Vec2(moveX, moveY));
				}
#else
				this->setCollisionPushBackVec(Vec2(moveX, moveY));
#endif
			}
			else {
				auto player = this->getPlayer();
				if (player) {
					player->setPosition(x, y);
				}
			}
		}

		// 「オブジェクトがタイルに重なったら」で発動するギミックの更新を行う
		updateTileGimmickHitCenterPos();

		/*
		// 360度ループの処理
		this->getObjectWallCollision()->updateLoopCourse();
		*/

		this->setOldPosition(this->getPosition());
		this->setOldWallPosition(cocos2d::Vec2(x, y));
		this->setOldWallSize(this->getContentSize());

		// タイルに攻撃判定が当たる場合
		if (playData->getHitTileGroupBit()) {
			// タイルに攻撃判定が当たっているかチェック
			this->getObjectWallCollision()->checkAttackHitWall(x, y, playData->getHitTileGroupBit());
		}
	}

// #AGTK-NX #AGTK-WIN
#ifdef USE_30FPS_3
	if(gm->getLastPass()) {
#endif
	switch (changeAction) {
	case kChangeActionSetup: {
		this->setBuriedInWallFlag(buriedInWallFlag);
		if (this->getTileWallBit() == 0) {
			this->setTileWallBit(tileWallBit);
		}
#ifdef USE_MULTITHREAD_OBJECT_UPDATE
		this->autoReleaseRetainTileWallList(nullptr, &aheadTileWall);
#else
		this->setAheadTileWall(aheadTileWall);
#endif
		break; }
	case kChangeActionUpdate: {
		if (this->getObjectSameLayerWallBit() == 0) {
			//※アクションが切り替わったタイミングで、壁判定Bitが0になった場合は、wallCollisionCorrectionで更新前の値を設定する。
			this->setObjectSameLayerWallBit(objectSameLayerWallBit);
		}
		this->setTileWallBit(tileWallBit);
#ifdef USE_MULTITHREAD_OBJECT_UPDATE
		this->autoReleaseRetainTileWallList(nullptr, &aheadTileWall);
#else
		this->setAheadTileWall(aheadTileWall);
#endif
		break; }
	}
// #AGTK-NX #AGTK-WIN
#ifdef USE_30FPS_3
	} // if (gm->getLastPass()) {}
#endif
}

void Object::updateTileGimmickHitCenterPos()
{
	// ギミックタイルとの重なり判定はオブジェクトの中心点
	//! ※アニメーションが存在しない場合は画像の中心
	cocos2d::Vec2 objPos = Vec2::ZERO;
	auto player = this->getPlayer();
	if (player) {
		objPos = player->getCenterNodePosition();
	}
	else {
		objPos = agtk::Scene::getPositionCocos2dFromScene(this->getCenterPosition());
	}

// #AGTK-NX #AGTK-WIN
#if defined(USE_30FPS_1) && !defined(USE_30FPS_3)
	auto difObjPos = objPos - this->getPosition();
	cocos2d::Vec2 passPoint[2];
	int passCount = 1;
	if (getPassedFrameCount() == getFrameCount())
	{
		passCount = 2;
		passPoint[0] = getPassedFramePosition();
		passPoint[1] = this->getPosition();
	}
	else {
		passPoint[0] = this->getPosition();
	}

	for (int passIndex = 0; passIndex < passCount; passIndex++) {
		objPos = difObjPos + passPoint[passIndex];
#endif

	// 取得した1dotがタイルに重なってギミックが発生するかをチェックする
#ifdef USE_MULTITHREAD_OBJECT_UPDATE
	std::vector<agtk::Tile *> tileList = getCollisionTileList(this->getSceneLayer()->getType(), 0, objPos, objPos, Vec2::ZERO, this);
#else
	cocos2d::__Array *tileList = getCollisionTileList(this->getSceneLayer()->getType(), 0, objPos, objPos, Vec2::ZERO, this);
#endif

	auto projectData = GameManager::getInstance()->getProjectData();
#ifdef USE_MULTITHREAD_OBJECT_UPDATE
	for (auto tile : tileList) {
#else
	auto tileDataList = cocos2d::__Array::create();
	cocos2d::Ref *ref = nullptr;
	CCARRAY_FOREACH(tileList, ref) {
		auto tile = dynamic_cast<agtk::Tile *>(ref);
#endif
		auto tileData = tile->getTileData();
		if (tileData == nullptr) {
			continue;
		}
		if (!tile->getSceneLayer()->getActiveFlg()) {
			//レイヤーの動作がOFFのときは処理をスキップ。
			continue;
		}
		auto tilesetData = projectData->getTilesetData(tile->getTilesetId());
		auto group = this->getObjectData()->getGroupBit();

		//ギミック設定タイルへのタッチ
		if (tilesetData->getTilesetType() == agtk::data::TilesetData::kGimmick) {

			// オブジェクトの種類チェック
			if(tileData->getGimmickTargetObjectGroupBit() & group ){
				tile->setObjectOverlapped(true, this);
			}
		}
	}

// #AGTK-NX #AGTK-WIN
#if defined(USE_30FPS_1) && !defined(USE_30FPS_3)
	}
#endif
}

void Object::updateInputPlayer()
{
	auto objectData = this->getObjectData();
	if (!objectData->isGroupPlayer()) {
		return;
	}
	if (objectData->getOperatable() == false) {
		return;
	}
	if (this->getOwnParentObject() && objectData->getTakeoverDispDirection()) {
		return;
	}
	switch(objectData->getMoveType()){
	case agtk::data::ObjectData::kMoveNormal:
		this->updateInputPlayerMoveNormal();
		break;
	case agtk::data::ObjectData::kMoveTank:
	case agtk::data::ObjectData::kMoveCar:
		this->updateInputPlayerMoveTankCar();
		break;
	}
}

/**
* プレイヤー入力更新：戦車タイプ or 車タイプ移動
* @return	入力方向ID
*/
int Object::updateInputPlayerMoveTankCar()
{
	int inputDirectionId = -1;
	auto inputManager = InputManager::getInstance();
	auto objectData = this->getObjectData();
	auto objectAction = this->getCurrentObjectAction();

	//動作中の移動の入力を受け付けない
	bool bIgnoredMoveInput = objectAction->getObjectActionData()->getIgnoreMoveInput();
	//動作中の表示方向の変更を受け付けない
	bool bKeepDirection = objectAction->getObjectActionData()->getKeepDirection();

	int forwardMoveKeyId = objectData->getForwardMoveKeyId();//前方向への移動（操作入力キーID）
	int backwardMoveKeyId = objectData->getBackwardMoveKeyId();//後方向への移動（操作入力キーID）
	int leftTurnKeyId = objectData->getLeftTurnKeyId();//左旋回（操作入力キーID）
	int rightTurnKeyId = objectData->getRightTurnKeyId();//右旋回（操作入力キーID）

	bool bUpdateInputDirection = true;
	if (objectData->getMovableWhenJumping() & _jumping) {//ジャンプ中の軌道修正不可
		bUpdateInputDirection = false;//不可
		inputDirectionId = this->getInputDirectionId();
	}
	if (objectData->getMovableWhenFalling() & _falling) {//落下中の軌道修正不可
		bUpdateInputDirection = false;//不可
		inputDirectionId = this->getInputDirectionId();
	}
	if (this->getObjectLoopMove()->getMovingAuto()) {//360度ループ自動移動中
		bUpdateInputDirection = false;//不可
	}
	if (GuiManager::getInstance()->getObjectStop()) {// UI表示によるオブジェクト停止
		bUpdateInputDirection = false;//不可
	}

	if (bUpdateInputDirection && bIgnoredMoveInput == false) {
		auto controllerId = this->getControllerId();
		auto centerPosition = this->getCenterPosition();
		//up+left
		if (inputManager->isPressed(InputController::kMoveUpLeft, centerPosition, forwardMoveKeyId, controllerId) && inputManager->isPressed(InputController::kMoveUpLeft, centerPosition, leftTurnKeyId, controllerId)) {
			inputDirectionId = 7;
		}
		//up+right
		else if (inputManager->isPressed(InputController::kMoveUpRight, centerPosition, forwardMoveKeyId, controllerId) && inputManager->isPressed(InputController::kMoveUpRight, centerPosition, rightTurnKeyId, controllerId)) {
			inputDirectionId = 9;
		}
		//down+left
		else if (inputManager->isPressed(InputController::kMoveDownLeft, centerPosition, backwardMoveKeyId, controllerId) && inputManager->isPressed(InputController::kMoveDownLeft, centerPosition, leftTurnKeyId, controllerId)) {
			inputDirectionId = 1;
		}
		//down+right
		else if (inputManager->isPressed(InputController::kMoveDownRight, centerPosition, backwardMoveKeyId, controllerId) && inputManager->isPressed(InputController::kMoveDownRight, centerPosition, rightTurnKeyId, controllerId)) {
			inputDirectionId = 3;
		}
		//up
		else if (inputManager->isPressed(InputController::kMoveUp, centerPosition, forwardMoveKeyId, controllerId)) {
			inputDirectionId = 8;
		}
		//down
		else if (inputManager->isPressed(InputController::kMoveDown, centerPosition, backwardMoveKeyId, controllerId)) {
			inputDirectionId = 2;
		}
		//left
		else if (inputManager->isPressed(InputController::kMoveLeft, centerPosition, leftTurnKeyId, controllerId)) {
			inputDirectionId = 4;
		}
		//right
		else if (inputManager->isPressed(InputController::kMoveRight, centerPosition, rightTurnKeyId, controllerId)) {
			inputDirectionId = 6;
		}
		//etc.
		else {
			inputDirectionId = -1;
		}
	}
	auto objectMovement = this->getObjectMovement();
	this->setInputDirectionId(inputDirectionId);

	auto inputMoveDirection = agtk::GetDirectionFromMoveDirectionId(inputDirectionId);
	if (inputMoveDirection != cocos2d::Vec2::ZERO) {
		//入力がある場合は、InputDirectionForceをクリアする。
		objectMovement->resetInputDirectionForce();
	}
	if (!objectMovement->isInputDirectionForceFlag()) {
		objectMovement->setInputDirection(inputMoveDirection);
	}

	return inputDirectionId;
}

/**
* プレイヤー入力更新：基本移動
* @return	入力方向ID
*/
int Object::updateInputPlayerMoveNormal()
{
	int inputDirectionId = -1;
	auto inputManager = InputManager::getInstance();
	auto objectData = this->getObjectData();
	auto objectAction = this->getCurrentObjectAction();
	if (objectAction == nullptr) {
		return inputDirectionId;
	}

	//動作中の移動の入力を受け付けない
	bool bIgnoredMoveInput = objectAction->getObjectActionData()->getIgnoreMoveInput();
	//動作中の表示方向の変更を受け付けない
	bool bKeepDirection = objectAction->getObjectActionData()->getKeepDirection();

	int upMoveKeyId = objectData->getUpMoveKeyId();//up
	int downMoveKeyId = objectData->getDownMoveKeyId();//down
	int leftMoveKeyId = objectData->getLeftMoveKeyId();//left
	int rightMoveKeyId = objectData->getRightMoveKeyId();//right

	bool bUpdateInputDirection = true;//入力方向の更新可否
	bool bMovableWhenJumping = (objectData->getMovableWhenJumping() & _jumping);//ジャンプ中に軌道修正が不可か？
	bool bMovableWhenFalling = (objectData->getMovableWhenFalling() & _falling);//落下中に軌道修正が不可か？

	//360度ループ自動移動中
	if (this->getObjectLoopMove()->getMovingAuto()) {
		//不可
		bUpdateInputDirection = false;
	}
	
	// UI表示によるオブジェクト停止
	if (GuiManager::getInstance()->getObjectStop()) {
		//不可
		bUpdateInputDirection = false;
	}

	int inputDirectionIdOld = this->getInputDirectionId();

	// 入力方向の更新が可能 かつ 動作中の移動入力受け付けが可能な場合
	if (bUpdateInputDirection && !bIgnoredMoveInput) {
		//         上
		//    +---+---+---+
		//    | 7 | 8 | 9 |
		//    +---+---+---+
		// 左 | 4 |   | 6 | 右
		//    +---+---+---+
		//    | 1 | 2 | 3 |
		//    +---+---+---+
		//         下

		//自由移動モード
		bool bForceMoveKey = false;
		if (DebugManager::getInstance()->getFreeMovingEnabled() ||
			this->getPlayObjectData()->getSwitchData(agtk::data::kObjectSystemSwitchFreeMove)->getValue()) 
		{
			auto objectData = this->getObjectData();
			if (objectData->isGroupPlayer()) {
				bForceMoveKey = true;
			}
		}

		static struct {
			int keyCode;
			int deviceKeyCode;
		} g_xbox360Input[] = {
			{ agtk::data::InputMapping::kInputButton9, 11 },
			{ agtk::data::InputMapping::kInputButton10, 10 },
			{ agtk::data::InputMapping::kInputButton11, 12 },
			{ agtk::data::InputMapping::kInputButton12, 13 },
		};
		int defaultDeviceKeyCode[CC_ARRAYSIZE(g_xbox360Input)];
		auto inputMapping = GameManager::getInstance()->getProjectData()->getInputMapping();
		auto typeInput = InputManager::getInstance()->getTypeInput();
		if (bForceMoveKey) {
			switch (typeInput) {
			case InputManager::kTypeCustom1Input: {
				for (int i = 0; i < CC_ARRAYSIZE(g_xbox360Input); i++) {
					auto inputMappingData = inputMapping->getInputMapping(g_xbox360Input[i].keyCode);
					int deviceKeyCode = inputMappingData->getCustom1Input();
					defaultDeviceKeyCode[i] = deviceKeyCode;
					inputMappingData->setCustom1Input(g_xbox360Input[i].deviceKeyCode);
				}
				break; }
			case InputManager::kTypeCustom2Input:
				CC_ASSERT(0);
				break;
			case InputManager::kTypeDirectInput:
//				CC_ASSERT(0);
				break;
			case InputManager::kTypeDiInput:
				CC_ASSERT(0);
				break;
			default:CC_ASSERT(0);
			}
		}
		auto controllerId = this->getControllerId();
		auto centerPosition = this->getCenterPosition();
		//up+left
		if (inputManager->isPressed(InputController::kMoveUpLeft, centerPosition, upMoveKeyId, controllerId) && inputManager->isPressed(InputController::kMoveUpLeft, centerPosition, leftMoveKeyId, controllerId)) {
			inputDirectionId = 7;
		}
		//up+right
		else if (inputManager->isPressed(InputController::kMoveUpRight, centerPosition, upMoveKeyId, controllerId) && inputManager->isPressed(InputController::kMoveUpRight, centerPosition, rightMoveKeyId, controllerId)) {
			inputDirectionId = 9;
		}
		//down+left
		else if (inputManager->isPressed(InputController::kMoveDownLeft, centerPosition, downMoveKeyId, controllerId) && inputManager->isPressed(InputController::kMoveDownLeft, centerPosition, leftMoveKeyId, controllerId)) {
			inputDirectionId = 1;
		}
		//down+right
		else if (inputManager->isPressed(InputController::kMoveDownRight, centerPosition, downMoveKeyId, controllerId) && inputManager->isPressed(InputController::kMoveDownRight, centerPosition, rightMoveKeyId, controllerId)) {
			inputDirectionId = 3;
		}
		//up
		else if (inputManager->isPressed(InputController::kMoveUp, centerPosition, upMoveKeyId, controllerId)) {
			inputDirectionId = 8;
		}
		//down
		else if (inputManager->isPressed(InputController::kMoveDown, centerPosition, downMoveKeyId, controllerId)) {
			inputDirectionId = 2;
		}
		//left
		else if (inputManager->isPressed(InputController::kMoveLeft, centerPosition, leftMoveKeyId, controllerId)) {
			inputDirectionId = 4;
		}
		//right
		else if (inputManager->isPressed(InputController::kMoveRight, centerPosition, rightMoveKeyId, controllerId)) {
			inputDirectionId = 6;
		}
		//etc.
		else {
			inputDirectionId = -1;
		}

		// 再ジャンプフラグがONの場合
		if (_reJumpFlag && (bMovableWhenJumping || bMovableWhenFalling)) {
			_reJumpFlag = false;
			this->getObjectMovement()->setFixedJumpDirectionId(inputDirectionId);
		}

		if (bForceMoveKey) {
			//戻す。
			switch (typeInput) {
			case InputManager::kTypeCustom1Input: {
				for (int i = 0; i < CC_ARRAYSIZE(g_xbox360Input); i++) {
					auto inputMappingData = inputMapping->getInputMapping(g_xbox360Input[i].keyCode);
					int deviceKeyCode = inputMappingData->getCustom1Input();
					inputMappingData->setCustom1Input(defaultDeviceKeyCode[i]);
				}
				break; }
			case InputManager::kTypeCustom2Input:
				CC_ASSERT(0);
				break;
			case InputManager::kTypeDirectInput:
//				CC_ASSERT(0);
				break;
			case InputManager::kTypeDiInput:
				CC_ASSERT(0);
				break;
			default:CC_ASSERT(0);
			}
		}
	}

	if (bKeepDirection == false && inputDirectionId != -1 && _bRequestPlayAction == 0) {
		this->setMoveDirection(inputDirectionId);
	}

	// ジャンプ中でジャンプ中の軌道修正不可の場合
	if (bMovableWhenJumping) {
		// ジャンプ時の入力方向を維持
		inputDirectionId = this->getObjectMovement()->getFixedJumpDirectionId();
	}

	// 落下中で落下中の軌道修正不可の場合
	if (bMovableWhenFalling) {
		// 落下直前の入力方向を維持
		inputDirectionId = this->getInputDirectionId();
	}

	this->setInputDirectionId(inputDirectionId);

	auto inputMoveDirection = agtk::GetDirectionFromMoveDirectionId(inputDirectionId);
	
	this->getObjectMovement()->setDirection(inputMoveDirection);
	
	if (inputMoveDirection != cocos2d::Vec2::ZERO) {
		//入力がある場合は、DirectionForceをクリアする。
		this->getObjectMovement()->resetDirectionForce();
	}
	return inputDirectionId;
}

/**
 * 入力によるプレイヤーの進行方向更新
 */
void Object::updateInputPlayerDirection()
{
	// オブジェクトデータ取得
	auto objectData = this->getObjectData();

	// プレイヤータイプ以外 or 「移動方向と表示方向を別操作で指定する」が OFF の場合
	if (!objectData->isGroupPlayer() ||
		!objectData->getMoveDispDirectionSettingFlag()) {
		return;
	}

	// 対応するアクションに含まれていない場合
	if (!objectData->getMoveDispDirectionSetting()->checkExistsActionInfo(getCurrentObjectAction()->getId())) {
		return;
	}

	// 弾の場合
	auto bullet = dynamic_cast<agtk::Bullet *>(this);
	if (bullet) {
		return;
	}

	int oldDirectionId				= this->getDispDirection();
	int inputDirectionId			= oldDirectionId;								// 表示方向の方向ID
	auto moveDispDirectionSetting	= objectData->getMoveDispDirectionSetting();	// 表示方向入力設定
	auto inputManager				= InputManager::getInstance();					// インプットマネージャ

	// --------------------------------------
	// 各表示方向の入力マッピングIDを取得
	// --------------------------------------
	int upDispDirectionKeyId		= moveDispDirectionSetting->getUpOperationKeyId();		// 上方向
	int downDispDirectionKeyId		= moveDispDirectionSetting->getDownOperationKeyId();	// 下方向
	int leftDispDirectionKeyId		= moveDispDirectionSetting->getLeftOperationKeyId();	// 左方向
	int rightDispDirectionKeyId		= moveDispDirectionSetting->getRightOperationKeyId();	// 右方向

	auto controllerId = this->getControllerId();
	auto centerPosition = this->getCenterPosition();
	// --------------------------------------
	// 各表示方向の入力状態から方向を確定
	// --------------------------------------
	// 左方向と上方向のキー設定があり、左上の入力がある場合
	if ((upDispDirectionKeyId >= 0 && leftDispDirectionKeyId >= 0) &&
		inputManager->isPressed(InputController::kMoveUpLeft, centerPosition, upDispDirectionKeyId, controllerId) && inputManager->isPressed(InputController::kMoveUpLeft, centerPosition, leftDispDirectionKeyId, controllerId)) {
		inputDirectionId = InputManager::EnumInputDirection::kDirectionLeftUp;
	}
	// 右方向と上方向のキー設定があり、右上の入力がある場合
	else if ((upDispDirectionKeyId >= 0 && rightDispDirectionKeyId >= 0) &&
		inputManager->isPressed(InputController::kMoveUpRight, centerPosition, upDispDirectionKeyId, controllerId) && inputManager->isPressed(InputController::kMoveUpRight, centerPosition, rightDispDirectionKeyId, controllerId)) {
		inputDirectionId = InputManager::EnumInputDirection::kDirectionRightUp;
	}
	// 左方向と下方向のキー設定があり、左上の入力がある場合
	else if ((downDispDirectionKeyId >= 0 && leftDispDirectionKeyId >= 0) &&
		inputManager->isPressed(InputController::kMoveDownLeft, centerPosition, downDispDirectionKeyId, controllerId) && inputManager->isPressed(InputController::kMoveDownLeft, centerPosition, leftDispDirectionKeyId, controllerId)) {
		inputDirectionId = InputManager::EnumInputDirection::kDirectionLeftDown;
	}
	// 右方向と下方向のキー設定があり、左上の入力がある場合
	else if ((downDispDirectionKeyId >= 0 && rightDispDirectionKeyId >= 0) &&
		inputManager->isPressed(InputController::kMoveDownRight, centerPosition, downDispDirectionKeyId, controllerId) && inputManager->isPressed(InputController::kMoveDownRight, centerPosition, rightDispDirectionKeyId, controllerId)) {
		inputDirectionId = InputManager::EnumInputDirection::kDirectionRightDown;
	}
	// 上方向のキー設定があり、上の入力がある場合
	else if (upDispDirectionKeyId >= 0 && inputManager->isPressed(InputController::kMoveUp, centerPosition, upDispDirectionKeyId, controllerId)) {
		inputDirectionId = InputManager::EnumInputDirection::kDirectionUp;
	}
	// 下方向のキー設定があり、下の入力がある場合
	else if (downDispDirectionKeyId >= 0 && inputManager->isPressed(InputController::kMoveDown, centerPosition, downDispDirectionKeyId, controllerId)) {
		inputDirectionId = InputManager::EnumInputDirection::kDirectionDown;
	}
	// 左方向のキー設定があり、左の入力がある場合
	else if (leftDispDirectionKeyId >= 0 && inputManager->isPressed(InputController::kMoveLeft, centerPosition, leftDispDirectionKeyId, controllerId)) {
		inputDirectionId = InputManager::EnumInputDirection::kDirectionLeft;
	}
	// 右方向のキー設定があり、右の入力がある場合
	else if (rightDispDirectionKeyId >= 0 && inputManager->isPressed(InputController::kMoveRight, centerPosition, rightDispDirectionKeyId, controllerId)) {
		inputDirectionId = InputManager::EnumInputDirection::kDirectionRight;
	}

	// 方向が変更された場合
	if (oldDirectionId != inputDirectionId) {
		// 「回転で自動生成」されている場合
		if (isAutoGeneration()) {
			this->setDispDirection(inputDirectionId);
		}
	}
}

/**
* ポータルアクティベーション処理
*/
void Object::updatePortalActivation()
{
	cocos2d::Ref *ref = nullptr;
	CCARRAY_FOREACH(this->getCollisionPortalHitList(), ref) {
		auto portal = dynamic_cast<agtk::Portal *>(ref);
		if (portal) {
			// ポータルに触れたオブジェクトを通知
			bool ret = portal->addTouchObject(this);
			if (ret == true) {
				break;
			}
		}
	}
}

void Object::callbackDetactionWallCollision(CollisionNode* collisionObject)
{
#ifdef USE_COLLISION_MEASURE
	roughWallCollisionCount++;
#endif
	auto collision = this->getObjectWallCollision();
	auto object = dynamic_cast<agtk::Object *>(collisionObject->getNode());

	if (object) {
		if (object->getLayerId() != this->getLayerId()) {
			//レイヤーが違う。
			return;
		}
		agtk::data::ObjectData *objectData1 = this->getObjectData();
		unsigned int collideGroupBit1;
		agtk::data::ObjectData *objectData2 = object->getObjectData();
		unsigned int groupBit2;
		collideGroupBit1 = objectData1->getCollideWithObjectGroupBit();

		groupBit2 = objectData2->getGroupBit();
		if (!(collideGroupBit1 & groupBit2)) {
			//objectの壁判定と衝突しない。
			return;
		}
		collision->addObject(object);
	}
}

void Object::removeDetactionWallCollision()
{
	auto scene = GameManager::getInstance()->getCurrentScene();
// #AGTK-NX
#ifdef USE_SAR_OPTIMIZE_1
	auto objectList = scene->getObjectAllReference(this->getSceneLayer()->getType());
#else
	auto objectList = scene->getObjectAll(this->getSceneLayer()->getType());
#endif
	cocos2d::Ref *ref = nullptr;
	CCARRAY_FOREACH(objectList, ref) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
		auto p = static_cast<agtk::Object *>(ref);
#else
		auto p = dynamic_cast<agtk::Object *>(ref);
#endif
		auto collision = p->getObjectWallCollision();
		collision->removeObject(this);
		collision->removeHitObject(this);
		collision->removeWallObject(this);
	}
}

/**
 * @brief オブジェクトのシーン座標系の位置を返す。アニメプレイヤーがある場合はその位置情報が参照される。
 */
const cocos2d::Vec2& Object::getPosition() const
{
	cocos2d::Vec2 *p = const_cast<cocos2d::Vec2 *>(&_objectPosition);
	auto player = this->getPlayer();
	if (player) {
		auto pos = player->getPosition();
		*p = Scene::getPositionSceneFromCocos2d(pos, this->getSceneData());
	}
	return _objectPosition;
}

cocos2d::Vec2 Object::getDispPosition()
{
	auto player = this->getPlayer();
	if (player) {
		auto pos = player->getDispPosition();
		return Scene::getPositionSceneFromCocos2d(pos, this->getSceneData());
	}

	auto pos = Scene::getPositionCocos2dFromScene(_objectPosition, this->getSceneData());

	//浮動小数点切り捨て
	pos.x = (int)pos.x;
	pos.y = (int)pos.y;

	return Scene::getPositionSceneFromCocos2d(pos, this->getSceneData());
}

cocos2d::Vec2 Object::getCenterDispPosition()
{
	cocos2d::Vec2 pos = this->getDispPosition();
	cocos2d::Size size = Object::getContentSize();
	cocos2d::Vec2 origin = cocos2d::Vec2::ZERO;
	cocos2d::Vec2 scale = _objectScale;

	auto player = this->getBasePlayer();
	if (nullptr != player) {
		scale.x *= player->getInnerScale().x;
		scale.y *= player->getInnerScale().y;

		origin = player->getOrigin();
		origin.x *= scale.x;
		origin.y *= scale.y;
	}
	size.width *= scale.x;
	size.height *= scale.y;

	return pos + cocos2d::Vec2(origin.x, -origin.y) + size * 0.5f;
}

const cocos2d::Size& Object::getContentSize() const
{
	cocos2d::Size sz;
	if (this->getPlayer()) {
		auto player = this->getPlayer()->getNodePlayer();
		if (player) {
			sz = player->getContentSize();
		}
	}
	cocos2d::Size *p = const_cast<cocos2d::Size *>(&_objectSize);
	p->width = sz.width;
	p->height = sz.height;
	return _objectSize;
}

float Object::getScaleX() const
{
	auto player = this->getPlayer();
	if (player) {
		return player->getScaleX();
	}
	return _objectScale.x;
}

float Object::getScaleY() const
{
	auto player = this->getPlayer();
	if (player) {
		return player->getScaleY();
	}
	return _objectScale.y;
}

float Object::getRotation() const
{
	auto player = this->getPlayer();
	if (player) {
		return player->getRotation();
	}
	return 0.0f;
}

/**
 * @brief オブジェクトのシーン座標系の位置を更新する。_objectPositionが更新される。アニメプレイヤーが設定されていればその位置を更新する。
 */
void Object::setPosition(const Vec2 &position)
{
	Object::setPosition(position.x, position.y);
}

/**
 * @brief オブジェクトのシーン座標系の位置を更新する。_objectPositionが更新される。アニメプレイヤーが設定されていればその位置を更新する。
 */
void Object::setPosition(float x, float y)
{
	_objectPosition = cocos2d::Vec2(x, y);
	auto player = this->getPlayer();
	if (player) {
		auto pos = Scene::getPositionCocos2dFromScene(_objectPosition, this->getSceneData());
		float x = std::roundf((pos.x) * 10000.0f) * 0.0001f;
		float y = std::roundf((pos.y) * 10000.0f) * 0.0001f;
		player->setPosition(x, y);
	}
}

/**
 * @brief オブジェクトの位置をずらす。(x, y)は、cocos2d-x座標系を想定している。
 */
void Object::addPosition(const cocos2d::Vec2 &v)
{
	cocos2d::Vec2 pos = Object::getPosition();
	pos += cocos2d::Vec2(v.x, -v.y);
	Object::setPosition(pos);
}

cocos2d::Vec2 Object::getCenterPosition()
{
	cocos2d::Vec2 pos = Object::getPosition();
	cocos2d::Size size = Object::getContentSize();
	cocos2d::Vec2 origin = cocos2d::Vec2::ZERO;
	cocos2d::Vec2 scale = _objectScale;
	cocos2d::Vec2 newPos;

	auto basePlayer = this->getBasePlayer();
	auto player = this->getPlayer();
	if (nullptr != basePlayer && nullptr != player) {
		//scale.x *= basePlayer->getInnerScale().x;
		//scale.y *= basePlayer->getInnerScale().y;
		//size.width *= scale.x;
		//size.height *= scale.y;
		auto offset = basePlayer->getOffset();
		auto contentCenter = cocos2d::Vec2(size.width * 0.5f, size.height * 0.5f);
		auto pos = player->convertToLayerSpacePosition(contentCenter);
		newPos = agtk::Scene::getPositionCocos2dFromScene(pos, this->getSceneData());
	}
	else {
		if (nullptr != basePlayer) {
			scale.x *= basePlayer->getInnerScale().x;
			scale.y *= basePlayer->getInnerScale().y;
			origin = basePlayer->getOrigin();
			origin.x *= scale.x;
			origin.y *= scale.y;
		}
		size.width *= scale.x;
		size.height *= scale.y;
		newPos = pos + cocos2d::Vec2(origin.x, -origin.y) + size * 0.5f;
	}
	return newPos;
}

cocos2d::Vec2 Object::getLeftDownPosition()
{
	auto pos = this->getPosition();
	cocos2d::Size size = this->getContentSize();
	cocos2d::Vec2 origin = cocos2d::Vec2::ZERO;

	auto player = this->getBasePlayer();
	if (nullptr != player) {
		origin = player->getOrigin();
	}

	return pos + cocos2d::Vec2(origin.x, -origin.y);
}

cocos2d::Vec2 Object::getFootPosition()
{
	cocos2d::Vec2 pos = Object::getCenterPosition();
	pos.y += getContentSize().height * 0.5f;

	return pos;
}

cocos2d::Rect Object::getRect()
{
	cocos2d::Vec2 pos = Object::getPosition();
	cocos2d::Size size = Object::getContentSize();
	cocos2d::Vec2 origin = cocos2d::Vec2::ZERO;
	cocos2d::Vec2 scale = _objectScale;

	auto player = this->getBasePlayer();
	if (nullptr != player) {
		scale.x *= player->getInnerScale().x;
		scale.y *= player->getInnerScale().y;

		origin = player->getOrigin();
		origin.x *= scale.x;
		origin.y *= scale.y;
	}
	size.width *= scale.x;
	size.height *= scale.y;

	pos += cocos2d::Vec2(origin.x, -origin.y);
	return cocos2d::Rect(pos, size);
}

void Object::setScale(cocos2d::Vec2 scale)
{
	_objectScale = scale;
	auto player = this->getPlayer();
	if (player) {
		player->setScale(scale.x, scale.y);
	}
}

void Object::setRotation(float rotation)
{
	_objectRotation = rotation;
	auto player = this->getPlayer();
	if (player) {
		player->setRotation(rotation);
	}
}

void Object::setInputDirectionId(int inputId)
{
	if (_inputDirectionId != inputId) {
		_inputDirectionIdOld = _inputDirectionId;
		_inputDirectionId = inputId;
	}
}

int Object::getInputDirectionId()
{
	return _inputDirectionId;
}

int Object::getInputDirectionIdOld()
{
	return _inputDirectionIdOld;
}

int Object::getInstanceId()
{
	CC_ASSERT(this->getPlayObjectData());
	return this->getPlayObjectData()->getInstanceId();
}

int Object::getControllerId()
{
	int controllerId = -1;

	//入力デバイス操作オブジェクトではない場合（-1)
	if (this->getObjectData()->getOperatable() == false) {
		return controllerId;
	}

	if (this->getScenePartObjectData() && this->getScenePartObjectData()->isStartPointObjectData()) {
		controllerId = getPlayerId();
	}
	else {
		CC_ASSERT(this->getPlayObjectData());
		auto cid = this->getPlayObjectData()->getControllerId();
		controllerId = InputManager::getInstance()->getPlayControllerId(cid);
	}
	return controllerId;
}

int Object::getPlayerId()
{
	CC_ASSERT(this->getPlayObjectData());
	return this->getPlayObjectData()->getPlayerId();
}

int Object::getAttackAttribute()
{
	CC_ASSERT(this->getPlayObjectData());
	return this->getPlayObjectData()->getAttackAttribute();
}

/**
 * @brief このオブジェクトのスイッチ「無効」の現在値を返す。
 */
bool Object::getDisabled()
{
	CC_ASSERT(this->getPlayObjectData());
	return this->getPlayObjectData()->getSwitchData(agtk::data::kObjectSystemSwitchDisabled)->getValue() || this->getForceDisabled();
}

bool Object::setDisabled(bool bIgnored)
{
	auto playObjectData = this->getPlayObjectData();
	CC_ASSERT(playObjectData);
	bool bOldIgnored = playObjectData->getSwitchData(agtk::data::kObjectSystemSwitchDisabled)->getValue();
	playObjectData->setSystemSwitchData(agtk::data::kObjectSystemSwitchDisabled, bIgnored);
	return bOldIgnored;
}

bool Object::isBullet()
{
	return dynamic_cast<agtk::Bullet *>(this) ? true : false;
}

void Object::addConnectObjectDispPriority(agtk::Object *object, bool bLowerPriority)
{
	auto list = this->getConnectObjectDispPriorityList();
	// 同じ登録が無いかチェックする。
	cocos2d::Ref *ref;
	CCARRAY_FOREACH(list, ref) {
		auto p = dynamic_cast<ConnectObjectDispPriority *>(ref);
		if (p->getObject() == object && p->getLowerPriority() == bLowerPriority) {
			return;
		}
	}
	list->addObject(ConnectObjectDispPriority::create(object, bLowerPriority));
}

void Object::removeConnectObjectDispPriority(agtk::Object *object)
{
	auto list = this->getConnectObjectDispPriorityList();
	cocos2d::Ref *ref;
	CCARRAY_FOREACH(list, ref) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
		auto p = static_cast<ConnectObjectDispPriority *>(ref);
#else
		auto p = dynamic_cast<ConnectObjectDispPriority *>(ref);
#endif
		if (p->getObject() == object) {
			list->removeObject(p);
			break;
		}
	}
}

agtk::SceneLayer *Object::getSceneLayer()
{
	CC_ASSERT(_sceneLayer == nullptr || (_sceneLayer && _sceneLayer->getReferenceCount() > 0));
	return _sceneLayer;
}

void Object::setSceneLayer(agtk::SceneLayer *sceneLayer)
{
	CC_ASSERT(sceneLayer);
	CC_ASSERT(sceneLayer->getReferenceCount() > 0);
	_sceneLayer = sceneLayer;
}

bool Object::isBuriedInWall()
{
	return _buriedInWall.flag || _buriedInWall.retainFlag;
}

bool Object::getBuriedInWallFlag()
{
	return _buriedInWall.flag;
}

void Object::setBuriedInWallFlag(bool flag)
{
	_buriedInWall.flag = flag;
}

void Object::setBuriedInWallFlag(bool flag, int framesMax)
{
	if (_buriedInWall.retainFlag) {
		return;
	}
	_buriedInWall.retainFlag = flag;
	_buriedInWall.frames = 0;
	_buriedInWall.framesMax = framesMax;
}

void Object::updateBurieInWall()
{
	if (_buriedInWall.retainFlag == false) {
		return;
	}
	if (_buriedInWall.frames < _buriedInWall.framesMax) {
		_buriedInWall.frames++;
		if (_buriedInWall.frames == _buriedInWall.framesMax) {
			_buriedInWall.retainFlag = false;
		}
	}
}

bool Object::isInvincible()
{
	return this->getObjectVisible()->isInvincible();
}

void Object::setJumpAction(bool flag)
{
	this->setJumpActionFlag(flag);
	this->getObjectMovement()->setJumpDuration(0.0f);
}

int Object::getDispDirectionByCurrentDirectionData()
{
	int dispDirection = 0;
	auto player = this->getPlayer();
	if (player != nullptr) {
		auto directionData = player->getCurrentDirectionData();
		dispDirection = this->calcDispDirection(directionData);
	}
	return dispDirection;
}

bool Object::isConnectObject()
{
	return (dynamic_cast<agtk::ConnectObject *>(this) != nullptr) ? true : false;
}

bool Object::isAutoGeneration()
{
	auto player = this->getPlayer();
	if (player == nullptr) {
		return false;
	}
	return player->isAutoGeneration();
}

void Object::setUpdateOneshot(bool loadflg)
{
	auto objectReappearData = this->getObjectReappearData();
	int courseId = objectReappearData->getInitialCourseId();
	int coursePointId = objectReappearData->getInitialCoursePointId();

	_updateOneshotFunction = [&, courseId, coursePointId, loadflg](void) {
		if (!loadflg) {
			this->checkCreateConnectObject(true, false);
		}

		auto sceneLayer = this->getSceneLayer();
		auto scene = this->getSceneLayer()->getScene();
		auto sceneData = this->getSceneData();

		// 制限範囲内に座標を変更する
		changePositionInLimitArea(sceneData);

		// 「カメラとの位置関係を固定する」設定時
		if (this->getObjectData()->getFixedInCamera() && this->getSceneLayer()->getType() != agtk::SceneLayer::kTypeMenu) {
			if (getObjectPosInCamera() != cocos2d::Vec2::ZERO) {
				//※objectPosInCameraに設定がある場合、ロードやシーン遷移で生成されたオブジェクトに対して再度位置設定をする。
				auto pos = agtk::Scene::getPositionSceneFromCocos2d(getObjectPosInCamera() + scene->getCamera()->getPosition());
				this->setPosition(pos);
			}
			else {
				_objectPosInCamera = agtk::Scene::getPositionCocos2dFromScene(this->getPosition()) - scene->getCamera()->getPosition();
			}
		}

		// コース移動の初期化
		if (courseId >= 0) {
			auto courseMove = agtk::ObjectCourseMove::create(this, courseId, coursePointId, scene);

			// コース移動設定(ロードデータがあれば上書き)
			if (this->getCourseMoveLoadData() != nullptr) {
				courseMove->setup(this);
				this->setCourseMoveLoadData(nullptr);
			}

			this->setObjectCourseMove(courseMove);
		}
	};
}

bool Object::init(agtk::SceneLayer *sceneLayer, int objectId, int initialActionId, cocos2d::Vec2 position, cocos2d::Vec2 scale, float rotation, int moveDirectionId, int courseId, int coursePointId, int changeAnimMotionId)
{
	// コース移動の設定があるなら座標のみ更新する(※コースはここでは作成しない)
	if (courseId >= 0) {
		auto scene = sceneLayer->getScene();
		auto courseMove = agtk::ObjectCourseMove::create(this, courseId, coursePointId, scene);
		position = Scene::getPositionSceneFromCocos2d(courseMove->getStartPointPos(), scene);
	}
	this->setBgmList(cocos2d::__Dictionary::create());
	this->setSeList(cocos2d::__Dictionary::create());
	this->setVoiceList(cocos2d::__Dictionary::create());
	this->setSwitchInfoCreateConnectObjectList(cocos2d::__Dictionary::create());

	auto projectData = GameManager::getInstance()->getProjectData();
	CC_ASSERT(projectData);
	auto objectData = projectData->getObjectData(objectId);
	CC_ASSERT(objectData);
	auto moveType = objectData->getMoveType();
	auto sceneData = sceneLayer->getSceneData();
	this->setSceneLayer(sceneLayer);
	this->setLayerId(sceneLayer->getLayerId());
	auto playObjectData = agtk::data::PlayObjectData::create(objectData);
	this->setPlayObjectData(playObjectData);
	playObjectData->getVariableData(agtk::data::kObjectSystemVariableFixedAnimationFrame)->setObject(this);

	this->setObjectData(objectData);
	this->setSceneData(sceneData);

	//setting playObjectData
// #AGTK-NX #AGTK-WIN
#ifdef USE_SAR_PROVISIONAL_1
	playObjectData->setInitControllerId(objectData->getOperatable() ? InputManager::kControllerAll : InputManager::kControllerNone);//コントローラID設定。
#else
	playObjectData->setControllerId(objectData->getOperatable() ? InputManager::kControllerAll : InputManager::kControllerNone);//コントローラID設定。
#endif
	playObjectData->setObjectId(objectData->getId());//オブジェクトID設定。
	if (!objectData->getPhysicsSettingFlag()) {
		//物理演算がOFFの場合は、オブジェクトスイッチ「他の物理演算オブジェクトに影響を与える」「他の物理演算オブジェクトから影響を受ける」「接続されている物理オブジェクトの動作を優先」の初期値をOFFにし読み込み専用にする。
		auto playSwitchData = playObjectData->getSwitchData(agtk::data::kObjectSystemSwitchAffectOtherObjects);
		playSwitchData->setValue(false);
		playSwitchData->setReadOnly(true);
		playSwitchData = playObjectData->getSwitchData(agtk::data::kObjectSystemSwitchAffectedByOtherObjects);
		playSwitchData->setValue(false);
		playSwitchData->setReadOnly(true);
		playSwitchData = playObjectData->getSwitchData(agtk::data::kObjectSystemSwitchFollowConnectedPhysics);
		playSwitchData->setValue(false);
		playSwitchData->setReadOnly(true);
	}

	//initObjectActionList
	if (this->initObjectActionList(objectData) == false) {
		return false;
	}
	_initialActionId = initialActionId;
	_initialMoveDirection = moveDirectionId;
	_pushedbackByObject = objectData->getPushedbackByObject();

	int dispDirection = 0;
	///初期表示方向を設定
	if (objectData->getInitialDisplayDirectionFlag()) {
		float degree = objectData->getInitialDisplayDirection();
		_moveDirection = agtk::GetMoveDirectionId(degree);
		_moveDirectionOld = _moveDirection;
		dispDirection = _moveDirection;
	}
	if (this->getScenePartObjectData()) {
		auto scenePartObjectData = this->getScenePartObjectData();
		if (scenePartObjectData->getInitialDisplayDirectionFlag()) {
			float degree = scenePartObjectData->getInitialDisplayDirection();
			_moveDirection = agtk::GetMoveDirectionId(degree);
			_moveDirectionOld = _moveDirection;
			dispDirection = _moveDirection;
		}
	}

	//指定(moveDirectionId)がある場合
	if (moveDirectionId > 0) {
		_moveDirection = moveDirectionId;
		_moveDirectionOld = _moveDirection;
	}
	//初期アクション
	if (initialActionId < 0 && objectData->getInitialActionId() >= 0) {
		initialActionId = objectData->getInitialActionId();
	}
#ifdef FIX_ACT2_5335
	//親の位置情報保持ワーク
	_parentFollowPosHead = -1;
	_parentFollowPosTail = -1;
	_parentFollowPosWeight = 0;
	int parentFollowPosSize = 0;
	auto followType = objectData->getFollowType();
	if (followType == agtk::data::ObjectData::EnumFollowType::kFollowNearTime) {
		parentFollowPosSize = (objectData->getFollowIntervalByTime300() + 5 - 1) / 5 + 3;	// (1/60)刻みでの親の位置を記録。
	}
	else if (followType == agtk::data::ObjectData::EnumFollowType::kFollowNearDot) {
		parentFollowPosSize = (int)std::ceilf(objectData->getFollowIntervalByLocusLength() / LOCUS_LENGTH_DIV_LEN) + 3;		// 0.5ドット以上の親の移動位置を記録。
	}
	if (parentFollowPosSize > 0) {
		_parentFollowPosList.resize(parentFollowPosSize);
	}
#endif

	// アニメーションデータ取得
	auto animationData = projectData->getAnimationData(objectData->getAnimationId());

	auto objectAction = this->getObjectAction(initialActionId);
	this->setCurrentObjectAction(objectAction);
	this->setPrevObjectActionId(initialActionId);

	CCLOG("object(%p,%d):%s", this, objectData->getId(), objectData->getName());
	// アニメーションデータがある場合
	if (nullptr != animationData && nullptr != objectAction) {

		objectAction->setPreActionID(initialActionId);

		CCLOG("initAnimation:%s", animationData->getName());
		auto objectActionData = objectAction->getObjectActionData();
		CCLOG("initAction:%s", objectActionData->getName());

		int animMotionId = changeAnimMotionId >= 0 ? changeAnimMotionId : objectActionData->getAnimMotionId();
		this->setCurrentAnimMotionId(animMotionId);
		auto motionData = animationData->getMotionData(animMotionId);

		// モーション設定がある場合
		if (motionData != nullptr) {
			int directionId = this->getAnimDirectionId(objectData, objectActionData, animMotionId);
			auto directionData = animationData->getDirectionData(animMotionId, directionId);
			if (getMoveDirection() < 0) {
				auto dispDirection = this->calcDispDirection(directionData);
				this->setDispDirection(dispDirection);
			}

			//player
			auto player = this->createPlayer(animationData);
			this->setPosition(position.x, position.y);
			this->setScale(scale);
			this->setRotation(rotation);
			player->play(animMotionId, directionId);
			this->setBasePlayer(player->getBasePlayer());
		}
		else {
			this->setPosition(position.x, position.y);
			this->setScale(scale);
			this->setRotation(rotation);
		}
	}
	// アニメーションデータが無い場合
	else {
		this->setPosition(position.x, position.y);
		this->setScale(scale);
		this->setRotation(rotation);
	}

	// 過去の位置を初期化
	this->setOldPosition(this->getPosition());
	this->setOldPosition2(this->getPosition());
	_premoveObjectPosition = _objectPosition;

	// 再出現用のデータを設定
	auto reappearData = agtk::data::ObjectReappearData::create();
	reappearData->setObjectId(objectId);
	reappearData->setSceneId(sceneLayer->getSceneData()->getId());
	reappearData->setSceneLayerId(getLayerId());
	reappearData->setInitialActionId(initialActionId);
	reappearData->setInitialPosition(position);
	reappearData->setInitialScale(scale);
	reappearData->setInitialRotation(rotation);
	reappearData->setInitialMoveDirectionId(moveDirectionId);
	reappearData->setInitialCourseId(courseId);
	reappearData->setInitialCoursePointId(coursePointId);
	this->setObjectReappearData(reappearData);

	//collision
	this->setObjectCollision(agtk::ObjectCollision::create(this));
	this->setCollisionAttackHitList(cocos2d::__Array::create());
	this->setCollisionHitAttackList(cocos2d::__Array::create());
	this->setCollisionWallWallChecked(false);
	this->setCollisionWallWallList(cocos2d::__Array::create());
	this->setCollisionPortalHitList(cocos2d::__Array::create());
	this->setObjectWallCollision(agtk::ObjectCollision::create(this));
#ifdef USE_MULTITHREAD_OBJECT_UPDATE
	this->setLeftWallObjectList(new agtk::MtVector<agtk::Object *>());
	this->setRightWallObjectList(new agtk::MtVector<agtk::Object *>());
	this->setUpWallObjectList(new agtk::MtVector<agtk::Object *>());
	this->setDownWallObjectList(new agtk::MtVector<agtk::Object *>());
	this->setSlopeTouchedList(new agtk::MtVector<agtk::Slope *>());
	this->setPassableSlopeTouchedList(new agtk::MtVector<agtk::Slope *>());
#else
	this->setLeftWallObjectList(cocos2d::Array::create());
	this->setRightWallObjectList(cocos2d::Array::create());
	this->setUpWallObjectList(cocos2d::Array::create());
	this->setDownWallObjectList(cocos2d::Array::create());
	this->setSlopeTouchedList(cocos2d::Array::create());
	this->setPassableSlopeTouchedList(cocos2d::Array::create());
#endif

	//todo: シーンに配置直後は壁判定情報を計算する前に、ActionLinkのupdate()が呼ばれるため、事前に計算が必要。
	setTileWallBit(0);
	this->setAttackObjectList(cocos2d::__Array::create());
	this->setAttackerObjectInstanceIdList(cocos2d::__Array::create());
	this->setExecActionObjectCreateList(cocos2d::__Array::create());

	//movement
	auto movement = agtk::ObjectMovement::create(this);
	cocos2d::Vec2 direction;
//※ ACT2-3771 回転で自動生成しているオブジェクトは停止しない
//	if (isAutoGeneration()) {//回転で自動生成。
//		int dispDirectionId = this->getDispDirection();
//		direction = agtk::GetDirectionFromMoveDirectionId(moveDirectionId < 0 ? dispDirectionId : moveDirectionId);
//	} else
	{
		auto moveType = objectData->getMoveType();
		if (moveType == agtk::data::ObjectData::kMoveTank || moveType == agtk::data::ObjectData::kMoveCar) {
			int dispDirectionId = this->getDispDirection();
			if (objectData->isGroupPlayer()) {
				direction = agtk::GetDirectionFromMoveDirectionId(moveDirectionId < 0 ? dispDirectionId : moveDirectionId);
			}
		}
		else {
			direction = agtk::GetDirectionFromMoveDirectionId(moveDirectionId);
		}
	}
	movement->setDirection(direction);

	this->setObjectMovement(movement);
	//「回転で自動生成」
	if (isAutoGeneration() && this->getPlayer()) {
		float centerRotation = agtk::GetDegreeFromVector(movement->getDirection());
		this->getPlayer()->setCenterRotation(centerRotation);
	}

	if (objectAction) {
		auto objectActionData = objectAction->getObjectActionData();
		movement->getMoveSpeed()->setValue(objectActionData->getMoveSpeed());
		movement->getUpDownMoveSpeed()->setValue(objectActionData->getUpDownMoveSpeed());
		movement->getTurnSpeed()->setValue(objectActionData->getTurnSpeed());

		//ジャンプ動作を行う。
		if (objectActionData->getJumpable()) {
			this->setJumpAction();
		}
	}

	//表示方向設定。
	if (dispDirection > 0 && this->getDispDirection() <= 0) {
		this->setDispDirection(dispDirection);
	}

	//debug
#ifdef USE_PREVIEW
	this->setupDebugDisplayArea();
#endif
#if defined(USE_WALL_DEBUG_DISPLAY)
	this->createWallDebugDisplay();
#endif

	auto player = this->getPlayer();
	cocos2d::Vec2 pos;
	if (player) {
		pos = this->getPlayer()->getPosition();
	}
	this->setOldWallPosition(pos);
	this->setOldWallSize(this->getContentSize());

	// 事前にオブジェクトの矩形情報を保持しておく
	auto objectCollision = this->getObjectCollision();
	if (objectCollision) {
		objectCollision->initWallHitInfoGroup();
	}

	auto objectVisible = agtk::ObjectVisible::create(_visible, this);
	this->setObjectVisible(objectVisible);

	auto timerPauseAction = agtk::EventTimer::create();
	this->setTimerPauseAction(timerPauseAction);

	auto timerPauseAnimation = agtk::EventTimer::create();
	this->setTimerPauseAnimation(timerPauseAnimation);

	auto childrenObjList = cocos2d::__Array::create();
	this->setChildrenObjectList(childrenObjList);

	auto parameterUiList = cocos2d::__Array::create();

	// 
	auto connectObjList = cocos2d::__Array::create();
	this->setConnectObjectList(connectObjList);

	// Zオーダーを設定
#ifdef USE_REDUCE_RENDER_TEXTURE	//agusa-k: cocos2d-xのZ-Orderと逆の設定をしているっぽい。
	this->setLocalZOrder(-getLayerId() * 1000 - (sceneLayer->getObjectId() + 1));
#else
	this->setLocalZOrder(getLayerId() * 1000 + (sceneLayer->getObjectId() + 1));
#endif

	// 残像の初期化
	auto afterImage = agtk::ObjectAfterImage::create(this);
	this->setObjectAfterImage(afterImage);

	//テンプレート移動
	auto templateMove = agtk::ObjectTemplateMove::create(this);
	this->setObjectTemplateMove(templateMove);

	// 360度ループ移動の初期化
	auto loopMove = agtk::ObjectLoopMove::create(this);
	this->setObjectLoopMove(loopMove);

	this->setConnectObjectDispPriorityList(cocos2d::__Array::create());

	// 「オブジェクトにエフェクトを表示」で設定されたエフェクトの初期化
	auto objectEffectList = cocos2d::__Array::create();
	if (this->getObjectData()->getEffectSettingFlag()) {

		auto settingList = this->getObjectData()->getEffectSettingList();

		cocos2d::DictElement *el = nullptr;
		CCDICT_FOREACH(settingList, el) {
			auto setting = dynamic_cast<agtk::data::ObjectEffectSettingData*>(el->getObject());
			auto objectEffect = agtk::ObjectEffect::create(setting);

			objectEffectList->addObject(objectEffect);
		}
	}
	this->setObjectEffectList(objectEffectList);


	// スイッチ変更による接続オブジェクトの設定変更処理
	this->setUpdateOneshot();

	this->setIsPortalWarped(false);
#ifdef FIX_ACT2_4774
	this->_warpedTransitionPortalId = -1;
#endif

	// 物理演算設定
	this->setupPhysicsBody(true);
	this->setPhysicsPartsList(cocos2d::__Array::create());
#ifdef USE_REDUCE_RENDER_TEXTURE
#else
	this->setDrawBackPhysicsPartsList(cocos2d::__Array::create());
	this->setDrawFrontPhysicsPartsList(cocos2d::__Array::create());
#endif

	this->setHitPhysicsObjList(cocos2d::__Array::create());

	//被ダメージ関連
	this->setDamagedList(cocos2d::__Array::create());
	if (objectData->getDamagedSettingFlag()) {
		auto damagedSettingList = objectData->getDamagedSettingList();
		cocos2d::DictElement *el;
		CCDICT_FOREACH(damagedSettingList, el) {
			auto damagedSettingData = dynamic_cast<agtk::data::ObjectDamagedSettingData *>(el->getObject());
			auto p = ObjectDamaged::create(this, damagedSettingData);
			this->getDamagedList()->addObject(p);
		}
	}

	_jumpInputFlag = false;

	//「カメラとの位置関係を固定する」で、メニューシーンの場合
	if (this->getObjectData()->getFixedInCamera() && sceneLayer->getType() == agtk::SceneLayer::kTypeMenu) {
		auto objectMovement = this->getObjectMovement();
		objectMovement->setIgnoredGravity(true);
		objectMovement->setIgnoredJump(true);
	}

	// 重なり演出関連
	setUpArroundCharacterView();

	{// 行動範囲制限
		if (getObjectData()->getMoveRestrictionSettingFlag())
		{	
			_limitTileSetList = LimitTileSetList::create();
			auto const screen = GameManager::getInstance()->getProjectData()->getScreenSize();
			auto const screenSizeHalf = screen / 2;
			auto const tileSize = cocos2d::Size( projectData->getTileWidth(), projectData->getTileHeight() );

			auto mrsl = getObjectData()->getMoveRestrictionSettingList();
			cocos2d::DictElement* de = nullptr;
			CCDICT_FOREACH(mrsl, de) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
				auto const data = static_cast<agtk::data::ObjectMoveRestrictionSettingData*>(de->getObject());
#else
				auto const data = dynamic_cast<agtk::data::ObjectMoveRestrictionSettingData*>(de->getObject());
#endif
				cocos2d::Rect const rc(cocos2d::Vec2(data->getLeft(), data->getTop()), 
					                   cocos2d::Size(screen.width - data->getRight() - data->getLeft(),
										             screen.height - data->getBottom() - data->getTop()));
				auto lts = std::move(agtk::LimitTileSet::create(getLayerId(), rc, tileSize, sceneData));
				lts->setSettingData(data);
				lts->setScreenSize(screen);
				auto const tiles = lts->getLimitTileList();
				cocos2d::Ref* ref = nullptr;
				CCARRAY_FOREACH(tiles, ref) {
					dynamic_cast<LimitTile*>(ref)->setObject(this);
				}
				_limitTileSetList->add(lts);
			}
		}
	}

	//プレイオブジェクトデータに座標とスケール値を設定。
	{
		auto playObjectData = this->getPlayObjectData();
		auto dispPos = this->getDispPosition();

		auto variableX = playObjectData->getVariableData(agtk::data::kObjectSystemVariableX);
		auto variableY = playObjectData->getVariableData(agtk::data::kObjectSystemVariableY);
		variableX->setExternalValue(dispPos.x);
		variableY->setExternalValue(dispPos.y);

		//毎フレームこのオブジェクトのスケール値が代入されます。
		auto variableScalingX = playObjectData->getVariableData(agtk::data::kObjectSystemVariableScalingX);
		auto variableScalingY = playObjectData->getVariableData(agtk::data::kObjectSystemVariableScalingY);
		variableScalingX->setExternalValue(this->getScaleX() * 100.0f);
		variableScalingY->setExternalValue(this->getScaleY() * 100.0f);
	}
	//表示方向変数を更新。
	this->updateDisplayDirectionVariable();

// #AGTK-NX
#ifdef USE_SAR_OPTIMIZE_4
	// スイッチ更新時に即座に自オブジェクトが影響を受けるスイッチリストを作成
	this->setupWatchSwitchList();
#endif
// #AGTK-NX, #AGTK-WIN
#ifdef USE_30FPS_4
	this->_middleFrameStock.init(this);
	this->_bUseMiddleFrame = false;
#endif

	_frameCount = 0;
	return true;
}

bool Object::initWithSceneDataAndScenePartObjectData(agtk::SceneLayer *sceneLayer, agtk::data::ScenePartObjectData *scenePartObjectData, int forceObjectId)
{
	CC_ASSERT(scenePartObjectData);
	this->setScenePartObjectData(scenePartObjectData);
	CC_ASSERT(!(forceObjectId < 0 && scenePartObjectData->isStartPointObjectData()));//※スタートポイントオブジェクトですが、ObjectID指定がない
	bool ret = this->init(
		sceneLayer,
		forceObjectId > 0 ? forceObjectId : scenePartObjectData->getObjectId(),
		scenePartObjectData->getInitialActionId(),
		cocos2d::Vec2(scenePartObjectData->getX(), scenePartObjectData->getY()),
		cocos2d::Vec2(scenePartObjectData->getScalingX() * 0.01f, scenePartObjectData->getScalingY() * 0.01f),
		scenePartObjectData->getRotation(),
		-1,
		scenePartObjectData->getCourseScenePartId(),
		scenePartObjectData->getCourseStartPointId()
	);

	// 生成されたシーンのIDを保持
	this->setSceneIdOfFirstCreated(sceneLayer->getSceneData()->getId());

	//ScenePartDataのインスタンスIDを設定する。
	int instanceId = this->getScenePartObjectData()->getId();
	instanceId += (this->getSceneData()->getId() == (int)agtk::data::SceneData::kMenuSceneId) ? agtk::data::SceneData::kMenuSceneId : 0;
	this->getPlayObjectData()->setInstanceId(instanceId);

	//復活データにScenePartDataのIDを設定する。
	this->getObjectReappearData()->setScenePartsId(this->getScenePartObjectData()->getId());
	if (ret == false) {
		return false;
	}
	return true;
}

/**
 * オブジェクトアクションリストの初期化
 * @param	objectData	オブジェクト設定データ
 * @return				初期化の成否
 */
bool Object::initObjectActionList(agtk::data::ObjectData *objectData)
{
	// ワーク変数
	auto objectActionList = cocos2d::__Dictionary::create();// オブジェクトアクションリスト
	auto commonActionLinkList = cocos2d::__Array::create();	// コモンアクションへのリンクリスト
	cocos2d::DictElement *el = nullptr;

	// コモンアクションが存在する場合
	if (objectData->getCommonActionSettingFlag()) {
		// コモンアクションを登録
		auto commonActionList = objectData->getCommonActionSettingList();
		CCDICT_FOREACH(commonActionList, el) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
			auto p = static_cast<agtk::data::ObjectCommonActionSettingData*>(el->getObject());
#else
			auto p = dynamic_cast<agtk::data::ObjectCommonActionSettingData*>(el->getObject());
#endif
			CC_ASSERT(p);
			auto objectAction = agtk::ObjectAction::create(p->getObjAction(), this, true, p->getId());
			objectActionList->setObject(objectAction, objectAction->getId());

			// コモンアクションへのリンクリストへ登録
			auto linkData = p->getObjActionLink();
			cocos2d::__Array *arr = cocos2d::__Array::create(cocos2d::Integer::create(-1), cocos2d::Integer::create(objectAction->getId()), nullptr);
			linkData->setActionIdPair(arr);
			commonActionLinkList->addObject(linkData);
		}
	}

	// 通常のアクションを登録
	auto actionList = objectData->getActionList();
	CCDICT_FOREACH(actionList, el) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
		auto p = static_cast<agtk::data::ObjectActionData *>(el->getObject());
#else
		auto p = dynamic_cast<agtk::data::ObjectActionData *>(el->getObject());
#endif
		CC_ASSERT(p);
		auto objectAction = agtk::ObjectAction::create(p, this, false, -1);
		objectActionList->setObject(objectAction, objectAction->getId());
	}
	
	this->setObjectActionList(objectActionList);

	//リンク情報をセットする。
	CCDICT_FOREACH(_objectActionList, el) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
		auto p = static_cast<agtk::ObjectAction *>(el->getObject());
#else
		auto p = dynamic_cast<agtk::ObjectAction *>(el->getObject());
#endif
		p->registActionLink(commonActionLinkList);
	}

	return true;
}

agtk::Player *Object::createPlayer(agtk::data::AnimationData *animationData)
{
	if (this->getPlayer()) {
		this->removeChild(this->getPlayer());
	}
	auto player = agtk::Player::create(animationData);
	this->addChild(player, 0, "player");
	this->setPlayer(player);
	return player;
}

void Object::updateTimelineListCache()
{
	auto player = this->getPlayer();
	if (player != nullptr) {
		player->VertListCacheClear(agtk::data::TimelineInfoData::kTimelineWall);
		player->VertListCacheClear(agtk::data::TimelineInfoData::kTimelineHit);
		player->VertListCacheClear(agtk::data::TimelineInfoData::kTimelineAttack);
	}
	auto objectData = this->getObjectData();
	if (objectData->getCollideWithObjectGroupBit()) {
		this->updateTimelineListCacheSingle(agtk::data::TimelineInfoData::kTimelineWall);
	}
	this->updateTimelineListCacheSingle(agtk::data::TimelineInfoData::kTimelineHit);
	this->updateTimelineListCacheSingle(agtk::data::TimelineInfoData::kTimelineAttack);
}

void Object::updateTimelineListCache(cocos2d::Array *objectList)
{
#if 0//def USE_COLLISION_MULTI_THREAD
#define COLLISION_MULTI_THREAD_COUNT	4
	if (!objectList) {
		return;
	}
	std::thread *threadList[COLLISION_MULTI_THREAD_COUNT];
	memset(threadList, 0, sizeof(threadList));
	int head = 0;
	for (int i = 0; i < COLLISION_MULTI_THREAD_COUNT; i++) {
		int size = (i < COLLISION_MULTI_THREAD_COUNT - 1) ? (objectList->count() / COLLISION_MULTI_THREAD_COUNT) : (objectList->count() - head);
		if (size == 0) {
			continue;
		}
		threadList[i] = new std::thread([](cocos2d::Array *list, int start, int size) {
			for (int i = 0; i < size; i++) {
				auto object = dynamic_cast<agtk::Object *>((cocos2d::Ref *)list->getObjectAtIndex(start + i));
				object->updateTimelineListCache();
			}
		}, objectList, head, size);
		CCLOG("%x: %d+%d", threadList[i]->get_id(), head, size);
		head += size;
	}
	for (int i = 0; i < COLLISION_MULTI_THREAD_COUNT; i++) {
		auto thread = threadList[i];
		if (thread) {
			thread->join();
			delete thread;
		}
	}
#else
	if (objectList) {
		cocos2d::Ref *ref = nullptr;
		CCARRAY_FOREACH(objectList, ref) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
			auto object = static_cast<agtk::Object *>(ref);
#else
			auto object = dynamic_cast<agtk::Object *>(ref);
#endif
			object->updateTimelineListCache();
		}
	}
#endif
}

agtk::ObjectAction *Object::setup(int objectActionId, int moveDirectionId, int forceDirectionId, bool bInertia)
{
	int currentObjectActionId = this->getCurrentObjectAction()->getId();
	if (objectActionId < 0) {
		objectActionId = currentObjectActionId;
	}
#ifdef USE_PREVIEW
	if (objectActionId != this->getCurrentObjectAction()->getId()) {
		//変更先のアクションID情報をEditorに送信する。
		auto scene = GameManager::getInstance()->getCurrentScene();
		if (scene->getPreviewInstanceId() == this->getInstanceId()) {
			auto message = cocos2d::String::createWithFormat("object feedbackActionInfo { \"actionId\": %d, \"objectId\": %d, \"instanceId\": %d }", objectActionId, this->getObjectData()->getId(), this->getInstanceId());
			auto gameManager = GameManager::getInstance();
			auto ws = gameManager->getWebSocket();
			if (ws) {
				ws->send(message->getCString());
			}
		}
	}
#endif


	bool bChangeMoveDirection = (this->getMoveDirection() != moveDirectionId);
	this->setMoveDirection((moveDirectionId != -1) ? moveDirectionId : _moveDirectionOld);
//	CCLOG("moveDirection(%d):%s[%d]:%d", this->getObjectData()->getId(), this->getObjectData()->getName(), this->getId(), this->getMoveDirection());
	auto nowObjectAction = this->getCurrentObjectAction();
	auto nextObjectAction = this->getObjectAction(objectActionId);
	auto objectActionData = nextObjectAction->getObjectActionData();
	int animMotionId = objectActionData->getAnimMotionId();
	bool bTakeOverMotion = objectActionData->getTakeOverMotion();
	int nextObjectActionId = objectActionData->getId();
	int prevObjectActionId = nowObjectAction->getObjectActionData()->getId();
//	CCLOG("nextAction:%s[%d]:%s,%d,%d", this->getObjectData()->getName(), this->getId(), objectActionData->getName(), nextObjectActionId, prevObjectActionId);
	auto projectData = GameManager::getInstance()->getProjectData();
	auto objectData = this->getObjectData();
	auto animationData = projectData->getAnimationData(objectData->getAnimationId());
	float x = 0.0f;
	float y = 0.0f;
	int dispDirection = this->getDispDirection();
	int nextAnimMotionId = bTakeOverMotion ? this->getCurrentAnimMotionId() : animMotionId;
	auto player = this->getPlayer();
	if (player) {
		x = player->getPosition().x;
		y = player->getPosition().y;
	}
	//アクションが切り替わる前に、残っているアクションを実行。
	if (nextAnimMotionId != this->getCurrentAnimMotionId()) {
		//実行アクション「オブジェクトを生成」を実行。
		this->updateExecActionObjectCreate();
	}

	//遷移前のモーションを引き継ぐ（※移動方向が同じ場合）
	if (bTakeOverMotion && bChangeMoveDirection == false) {

		if (animationData != nullptr) {
			auto motionData = animationData->getMotionData(nextAnimMotionId);
			if (motionData != nullptr) {
				if (player == nullptr) {
				}
				if (player) {
					auto basePlayer = this->getBasePlayer();
					auto lastDirectionId = -1;
					if (basePlayer) {
						auto currentAnimationMotion = basePlayer->getCurrentAnimationMotion();
						if (currentAnimationMotion) {
							auto currentDirection = currentAnimationMotion->getCurrentDirection();
							if (currentDirection) {
								auto directionData = currentDirection->getDirectionData();
								if (directionData) {
									lastDirectionId = directionData->getId();
								}
							}
						}
					}
					int directionId = forceDirectionId >= 0 ? forceDirectionId : this->getAnimDirectionId(this->getObjectData(), nowObjectAction->getObjectActionData(), -1, bTakeOverMotion);
					if (directionId != lastDirectionId) {
						int fixedFrame = -1;
						if (basePlayer) {
							fixedFrame = this->getBasePlayer()->getFixedFrame();
						}
						bool bTakeOverFlag = true;

						player->play(nextAnimMotionId, directionId, bTakeOverFlag);
						if (_resourceSetId >= 0) {
							player->setResourceSetId(_resourceSetId);
						}
						x = player->getPosition().x;
						y = player->getPosition().y;
						this->setBasePlayer(player->getBasePlayer());
						if (this->getBasePlayer()) {
							this->getBasePlayer()->setFixedFrame(fixedFrame);
						}
					}
				}
			}
		}
	} else {
		if (animationData != nullptr) {
			auto motionData = animationData->getMotionData(nextAnimMotionId);
			if (motionData != nullptr) {
				if (player == nullptr) {
					auto pos = this->getPosition();
					auto scale = cocos2d::Vec2(this->getScaleX(), this->getScaleY());
					auto rotation = this->getRotation();
					player = this->createPlayer(animationData);
					auto playerPos = Scene::getPositionCocos2dFromScene(pos, this->getSceneData());
					//小数点以下を切り捨てる。
					playerPos.x = (int)playerPos.x;
					playerPos.y = (int)playerPos.y;
					player->setPosition(playerPos);
					this->setScale(scale);
					this->setRotation(rotation);
				}
				if (player) {
					int fixedFrame = -1;
					if (this->getBasePlayer()) {
						fixedFrame = this->getBasePlayer()->getFixedFrame();
					}
					int directionId = forceDirectionId >= 0 ? forceDirectionId : this->getAnimDirectionId(this->getObjectData(), bTakeOverMotion ? nowObjectAction->getObjectActionData() : objectActionData, -1, bTakeOverMotion);
					bool bTakeOverFlag = (nextObjectActionId == prevObjectActionId);
					auto prevObjectActionData = objectData->getActionData(prevObjectActionId);
					if (prevObjectActionData->getTakeOverMotion()) {
						bTakeOverFlag = (nextObjectActionId == this->getPrevObjectActionId());
					}

					player->play(nextAnimMotionId, directionId, bTakeOverFlag);
					if (_resourceSetId >= 0) {
						player->setResourceSetId(_resourceSetId);
					}
					x = player->getPosition().x;
					y = player->getPosition().y;
					this->setBasePlayer(player->getBasePlayer());
					if (this->getBasePlayer()) {
						this->getBasePlayer()->setFixedFrame(fixedFrame);
					}
				}
			}
			else {
				if (player) {
					player->setVisible(false);//非表示に。
					this->removeChild(player);
					this->setPlayer(nullptr);
				}
				this->setBasePlayer(nullptr);
			}
		}
		else {
			if (player) {
				player->setVisible(false);//非表示に。
				this->removeChild(player);
				this->setPlayer(nullptr);
			}
			this->setBasePlayer(nullptr);
		}
	}

	if (this->getPlayer() == nullptr) {
		//アニメーションがない場合かつ、浮動小数点切り捨て。
		x = (int)x;
		y = (int)y;
	}

	auto currentObjectAction = this->getCurrentObjectAction();

	//アクションが変更になったフラグ。
	bool bChangeAction = nextObjectAction != currentObjectAction;
	//アニメーションが変更になったフラグ。
	bool bChangeAnimMotion = (nextAnimMotionId != this->getCurrentAnimMotionId());

	//アクションが変更され、次のアクションがジャンプを実行し、ジャンプ中または落下中の軌道修正不可がONの場合
	if(bChangeAction && nextObjectAction->getObjectActionData()->getJumpable() && (objectData->getMovableWhenJumping() || objectData->getMovableWhenFalling())) {
		// 再ジャンプフラグをON
		_reJumpFlag = true;
	}

	//ジャンプ入力フラグOFF
	_jumpInputFlag = false;

	//setup nextObjectAction
	nextObjectAction->setup(bChangeAction);
	this->setCurrentObjectAction(nextObjectAction);
	if (bTakeOverMotion) {
		this->setTakeOverAnimMotionId(nextAnimMotionId);
	}
	else {
		this->setTakeOverAnimMotionId(-1);
	}
	this->setCurrentAnimMotionId(nextAnimMotionId);
	if ((nextObjectAction->getIsCommon() == false && currentObjectAction->getIsCommon() == false)
	|| (nextObjectAction->getIsCommon() == false && currentObjectAction->getIsCommon() == true && objectActionId != this->getPrevObjectActionId())) {
		this->setPrevObjectActionId(objectActionId);
	}
	nextObjectAction->setPreActionID(this->getPrevObjectActionId());
	nextObjectAction->setDispDirection(dispDirection > 0 ? dispDirection : this->getDispDirection());
	if (nextObjectAction != currentObjectAction) {
		if (currentObjectAction->getObjectFilterEffectCommandData() != nullptr) {
			nextObjectAction->setObjectFilterEffectCommandData(currentObjectAction->getObjectFilterEffectCommandData());
		}
		currentObjectAction->setObjectFilterEffectCommandData(nullptr);
	}

	//debug
#ifdef USE_PREVIEW
	this->setupDebugDisplayArea();
#endif
#if defined(USE_WALL_DEBUG_DISPLAY)
	this->removeWallDebugDisplay();
	this->createWallDebugDisplay();
#endif

	this->wallCollisionCorrection(x, y, bChangeAction ? kChangeActionSetup : kChangeActionNone);
	// オブジェクト座標が変化したので矩形情報を更新する。
	auto objectCollision = this->getObjectCollision();
	if (objectCollision) {
		objectCollision->lateUpdateWallHitInfoGroup();
	}

	auto movement = this->getObjectMovement();
	//アクション切り替わりに強制移動方向処理をリセットする。
	if (bChangeAction) {
		auto objectTemplate = this->getObjectTemplateMove();
		if (objectActionData->getTakeOverMotion() == false && objectTemplate->getState() == agtk::ObjectTemplateMove::kStateIdle && !movement->getForceMove()->isMoving() && !movement->isInputDirectionForceFlag()) {
			movement->resetDirectionForce();//強制移動方向をリセットする。
		}
		//プレイヤーで、戦車・車タイプの場合。
		auto objectData = this->getObjectData();
		if (objectData->isGroupPlayer() && (objectData->getMoveType() == agtk::data::ObjectData::kMoveCar || objectData->getMoveType() == agtk::data::ObjectData::kMoveTank)) {
			auto direction = movement->getDirection();
			movement->resetDirectionForce();
			movement->setDirection(direction);
		}
		// ACT2-4001 アクションが切り替わる時にテンプレート移動を解除される（※解除されないように修正）
		//プレイヤーがテンプレート移動中の場合。
		//if (objectData->isGroupPlayer() && objectTemplate->getState() != agtk::ObjectTemplateMove::kStateIdle) {
		//	objectTemplate->end();
		//}

		//ジャンプ動作を行う。
		if (objectActionData->getJumpable()) {
			this->setJumpAction();
		}

		//直前のアクションの慣性を引き継ぐ。
		this->setupInertia(bInertia);

		//ジャンプ動作を行う or 重力の影響を受けない。
		if (objectActionData->getJumpable() || (objectActionData->getIgnoreGravity() && objectActionData->getGravity() == 100.0)) {
			//垂直方向をZEROにする。
			if (objectActionData->getJumpable() && this->_jumping) {
				movement->setKeepJumping(true);
				movement->setKeepVertVelocity(movement->getVertVelocity());
			}
			movement->setVertVelocity(cocos2d::Vec2::ZERO);
		}
		
		//実行アクション「オブジェクトを生成」リクエストを破棄。
		this->getExecActionObjectCreateList()->removeAllObjects();
		//アクション切り替わりで弾生成待機状態のFireBulletインスタンスを破棄。
		if (!isBullet()) {
			BulletManager::getInstance()->removeParentObject(this, true);
		}
	}

	//弾オブジェクト
	auto bullet = dynamic_cast<agtk::Bullet *>(this);
	if (bullet) {
		//初期アクションのみ弾処理有効、その後アクションが切り替わると無効。
		bullet->setBulletIgnored(true);//無効。
		//重力、ジャンプ有効に。
		movement->setIgnoredGravity(false);
		movement->setIgnoredJump(false);
	}

	// 物理演算用ノードが存在しない場合
	if (nullptr == this->getphysicsNode()) {
		this->setupPhysicsBody(true);
	}

	//位置を再設定する。
	this->setPosition(this->getPosition());

	//スケールを設定する。
	auto playObjectData = this->getPlayObjectData();
	auto variableScalingX = playObjectData->getVariableData(agtk::data::kObjectSystemVariableScalingX);
	auto variableScalingY = playObjectData->getVariableData(agtk::data::kObjectSystemVariableScalingY);
	this->setScale(cocos2d::Vec2(variableScalingX->getValue() * 0.01f, variableScalingY->getValue() * 0.01f));

	//オブジェクトにフィルター効果を設定する。
	nextObjectAction->execActionObjectFilterEffect();

	return this->getCurrentObjectAction();
}

void Object::setupInertia(bool bInertia)
{
	auto objectAction = this->getCurrentObjectAction();
	if (objectAction == nullptr) {
		return;
	}
	auto objectActionData = objectAction->getObjectActionData();

	auto movement = this->getObjectMovement();

	//直前のアクションの慣性を引き継ぐ。
	auto playObjectData = this->getPlayObjectData();
	float seconds = playObjectData->getVariableData(agtk::data::kObjectSystemVariableDurationToTakeOverAccelerationMoveSpeed)->getValue();
	if (bInertia || seconds > 0) {
		movement->getMoveSpeed()->setValue(objectActionData->getMoveSpeed(), seconds);
		movement->getUpDownMoveSpeed()->setValue(objectActionData->getUpDownMoveSpeed(), seconds);
		movement->getTurnSpeed()->setValue(objectActionData->getTurnSpeed(), seconds);
	}
	else {
		movement->getMoveSpeed()->setValue(objectActionData->getMoveSpeed());
		movement->getUpDownMoveSpeed()->setValue(objectActionData->getUpDownMoveSpeed());
		movement->getTurnSpeed()->setValue(objectActionData->getTurnSpeed());
		_collision = false;
		this->setFalling(false);
		_jumpTop = false;
	}
}

agtk::ObjectAction *Object::getObjectAction(int id)
{
	auto data = this->getObjectActionList()->objectForKey(id);
	if (data == nullptr) {
		auto actionList = this->getObjectActionList();
		if (actionList->count() == 0) {
			return nullptr;
		}
		auto keys = actionList->allKeys();
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
		auto key = static_cast<cocos2d::Integer *>(keys->getObjectAtIndex(0));
#else
		auto key = dynamic_cast<cocos2d::Integer *>(keys->getObjectAtIndex(0));
#endif
		data = actionList->objectForKey(key->getValue());
		id = key->getValue();
	}
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
	auto objectAction = static_cast<ObjectAction *>(data);
#else
	auto objectAction = dynamic_cast<ObjectAction *>(data);
#endif
	CC_ASSERT(objectAction);
	CC_ASSERT(objectAction->getId() == id);
	return objectAction;
}

int Object::getActionId(agtk::data::ObjectCommandData *commandData)
{
	//アクションIDを取得
	auto objectData = this->getObjectData();
	auto actionDataList = objectData->getActionList();
	auto scenePartObjectData = this->getScenePartObjectData();
	cocos2d::DictElement *el;

	CCDICT_FOREACH(actionDataList, el) {
		auto actionData = dynamic_cast<agtk::data::ObjectActionData *>(el->getObject());
		cocos2d::DictElement *el2;
		auto objCommandList = actionData->getObjCommandList();
		CCDICT_FOREACH(objCommandList, el2) {
			auto objCommandData = dynamic_cast<agtk::data::ObjectCommandData *>(el2->getObject());
			if (objCommandData->getInstanceConfigurable()) {
				auto instanceObjCommandList = dynamic_cast<cocos2d::__Dictionary *>(scenePartObjectData->getActionCommandListObject()->objectForKey(actionData->getId()));
				auto instanceCommandData = dynamic_cast<agtk::data::ObjectCommandData *>(instanceObjCommandList->objectForKey(commandData->getId()));
				if (instanceCommandData != nullptr && instanceCommandData == commandData) {
					return actionData->getId();
				}
			} else {
				if (objCommandData == commandData) {
					return actionData->getId();
				}
			}
		}
	}
	if (objectData->getCommonActionSettingFlag()) {
		auto commonActionDataList = objectData->getCommonActionSettingList();
		cocos2d::DictElement *el;
		CCDICT_FOREACH(commonActionDataList, el) {
			auto commonActionData = dynamic_cast<agtk::data::ObjectCommonActionSettingData *>(el->getObject());
			auto actionData = commonActionData->getObjAction();
			cocos2d::DictElement *el2;
			auto objCommandList = actionData->getObjCommandList();
			CCDICT_FOREACH(objCommandList, el2) {
				auto objCommandData = dynamic_cast<agtk::data::ObjectCommandData *>(el2->getObject());
				if (objCommandData->getInstanceConfigurable()) {
					auto instanceObjCommandList = dynamic_cast<cocos2d::__Dictionary *>(scenePartObjectData->getCommonActionCommandListObject()->objectForKey(actionData->getId()));
					auto instanceCommandData = dynamic_cast<agtk::data::ObjectCommandData *>(instanceObjCommandList->objectForKey(commandData->getId()));
					if (instanceCommandData != nullptr && instanceCommandData == commandData) {
						return actionData->getId();
					}
				} else {
					if (objCommandData == commandData) {
						return actionData->getId();
					}
				}
			}
		}
	}
	CC_ASSERT(0);//見つかりません。
	return -1;
}

agtk::data::ObjectCommandData *Object::getCommandData(int actionId, int commandDataId)
{
	auto objectData = this->getObjectData();
	auto actionData = objectData->getActionData(actionId);
	auto scenePartObjectData = this->getScenePartObjectData();
	if (scenePartObjectData != nullptr) {
		auto instanceObjCommandList = scenePartObjectData->getActionCommandListObject();
		auto baseCommandData = actionData->getObjectCommandData(commandDataId);
		if (baseCommandData->getInstanceConfigurable()) {
			auto instanceObjCommandList = dynamic_cast<cocos2d::__Dictionary *>(scenePartObjectData->getActionCommandListObject()->objectForKey(actionId));
			auto instanceCommandData = dynamic_cast<agtk::data::ObjectCommandData *>(instanceObjCommandList->objectForKey(commandDataId));
			CC_ASSERT(instanceCommandData);
			CC_ASSERT(instanceCommandData->getId() == commandDataId);
			return instanceCommandData;
		}
	}
	return actionData->getObjectCommandData(commandDataId);
}

/**
 * アニメーション方向IDの取得
 * @param	objectData			オブジェクト設定データ
 * @param	objectActionData	オブジェクトアクション設定データ
 * @param	forceAnimMotionId	指定モーションID
 * @return						アニメーション方向ID
 */
int Object::getAnimDirectionId(agtk::data::ObjectData *objectData, agtk::data::ObjectActionData *objectActionData, int forceAnimMotionId)
{
	return this->getAnimDirectionId(objectData, objectActionData, forceAnimMotionId, false);
}

/**
* アニメーション方向IDの取得
* @param	objectData			オブジェクト設定データ
* @param	objectActionData	オブジェクトアクション設定データ
* @param	forceAnimMotionId	指定モーションID
* @param	takeOverMotion		モーション引き継ぐ
* @return						アニメーション方向ID
*/
int Object::getAnimDirectionId(agtk::data::ObjectData *objectData, agtk::data::ObjectActionData *objectActionData, int forceAnimMotionId, bool bTakeOverMotion)
{
	auto projectData = GameManager::getInstance()->getProjectData();
	auto animationData = projectData->getAnimationData(objectData->getAnimationId());
	int animMotionId = objectActionData->getAnimMotionId();

	// ACT2-4643
	// 「遷移前のモーションを引き継ぐ」を設定していても、モーションが「設定しない」以外だと
	// 「遷移前のモーションを引き継ぐ」が実行されない不具合を修正
	if (bTakeOverMotion) {
		auto player = this->getPlayer();
		if (player) {
			auto basePlayer = player->getBasePlayer();
			if (basePlayer) {
				animMotionId = basePlayer->getCurrentAnimationMotion()->getMotionData()->getId();
			}
		}
	}
	auto motionData = animationData->getMotionData(forceAnimMotionId >= 0 ? forceAnimMotionId : animMotionId);
	if (motionData == nullptr) {
		return -1;
	}

	//アクションデータで直接モーションの指定がある。
	if (objectActionData->getAnimDirectionId() >= 0) {
		auto directionData = motionData->getDirectionData(objectActionData->getAnimDirectionId());
		if (directionData != nullptr) {
			//「回転の自動生成」以外。
			if (!directionData->getAutoGeneration()) {
				this->setDispDirection(agtk::CalcDirectionId(directionData->getDirectionBit()));
				//移動方向IDと表示方向IDで違いがあり、プレイヤーグループでかつ入力操作できるオブジェクト場合。
				auto objectMovement = this->getObjectMovement();
				if (objectMovement != nullptr && (objectData->isGroupPlayer() && objectData->getOperatable()) && this->getMoveDirection() > 0 && this->getDispDirection() > 0 && this->getMoveDirection() != this->getDispDirection()) {
					//移動方向がゼロの場合。
					if (objectMovement->getDirection() == cocos2d::Vec2::ZERO) {
						//移動方向IDに表示方向IDを設定する。
						this->setMoveDirection(this->getDispDirection());
					}
				}
			}
			return directionData->getId();
		}
	}

	agtk::data::DirectionData *directionData = nullptr;
	auto player = this->getPlayer();
	if (player) {
		auto basePlayer = player->getBasePlayer();
		if (basePlayer) {
			auto currentDirection = basePlayer->getCurrentAnimationMotion()->getCurrentDirection();
			if (currentDirection != nullptr) {
				directionData = currentDirection->getDirectionData();
			}
		}
	}
	int moveDirectionBit = this->getMoveDirectionBit();
	int dispDirectionBit = this->getDispDirectionBit();
	auto newDirectionData = this->getDirectionData(moveDirectionBit, dispDirectionBit, motionData, directionData);
	// 表示方向を別操作で指定する and 現在のアクションが対応するアクションに含まれている場合
	if (objectData->getMoveDispDirectionSettingFlag() &&
		objectData->getMoveDispDirectionSetting()->checkExistsActionInfo(objectActionData->getId())) {
		// 表示方向から方向データを取得
		newDirectionData = this->getDirectionData(dispDirectionBit, dispDirectionBit, motionData, directionData);
	}
	// 表示方向が入力方向と同期する場合
	else {
		// 入力方向から表示方向を設定
		if (this->getOwnParentObject() && objectData->getTakeoverDispDirection()) {
			newDirectionData = this->getDirectionData(dispDirectionBit, dispDirectionBit, motionData, directionData);
		}
		else if (newDirectionData && ((newDirectionData->getDirectionBit() & this->getMoveDirectionBit()) || (newDirectionData->getAutoGeneration() && this->getMoveDirection() > 0))) {
			this->setDispDirection(this->getMoveDirection());
		}
		else {
			if (directionData && newDirectionData) {
				if (!(dispDirectionBit & newDirectionData->getDirectionBit()) && !newDirectionData->getAutoGeneration()) {
					this->setDispDirection(agtk::CalcDirectionId(newDirectionData->getDirectionBit()));
					newDirectionData = this->getDirectionData(moveDirectionBit, this->getMoveDirectionBit(), motionData, directionData);
				}
			}
		}
	}
	return newDirectionData ? newDirectionData->getId() : -1;
}


agtk::data::DirectionData *Object::getDirectionData(int moveDirectionBit, int dispDirectionBit, agtk::data::MotionData *motionData, agtk::data::DirectionData *directionData)
{
	if (directionData) {
		//１．次の移動方向で、単体BitのdirectionDataがある場合、優先される。
		//２．次の移動方向が、現状のdirectionDataに存在する場合、維持。
		//３．MotionData::getDirectionDataByDirectionBitにより選ぶ。
		std::function<int(int)> bitsCount = [&](int bits) {
			int count = 0;
			int mask = 1;
			while (mask != 0) {
				if (bits & mask) {
					count++;
				}
				mask = mask << 1;
			}
			return count;
		};

		//欲しいデータは次の移動方向
		auto directionDataList = motionData->getDirectionList();
		cocos2d::DictElement *el;
		CCDICT_FOREACH(directionDataList, el) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
			auto data = static_cast<agtk::data::DirectionData *>(el->getObject());
#else
			auto data = dynamic_cast<agtk::data::DirectionData *>(el->getObject());
#endif
			int bits = data->getDirectionBit();
			int count = bitsCount(bits);
			if ((count == 1) && (bits & moveDirectionBit)) {
				return data;
			}
		}
		// 現状のdirectionDataで次の移動方向がある場合。
		{
			cocos2d::DictElement *el2;
			CCDICT_FOREACH(directionDataList, el2) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
				auto data = static_cast<agtk::data::DirectionData *>(el2->getObject());
#else
				auto data = dynamic_cast<agtk::data::DirectionData *>(el2->getObject());
#endif
				if ((data->getDirectionBit() == directionData->getDirectionBit()) && (directionData->getDirectionBit() & moveDirectionBit)) {
					return data;
				}
			}
		}
		// 次の移動方向があるデータを選び出し、現状のdirectionDataと選び出したデータを上下左右で方向成分が同じデータを選択する。
		if (directionData != nullptr) {
			cocos2d::DictElement *el3;
			auto tmpDirectionList = cocos2d::__Array::create();
			CCDICT_FOREACH(directionDataList, el3) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
				auto data = static_cast<agtk::data::DirectionData *>(el3->getObject());
#else
				auto data = dynamic_cast<agtk::data::DirectionData *>(el3->getObject());
#endif
				if (data->getDirectionBit() & moveDirectionBit) {
					tmpDirectionList->addObject(data);
					continue;
				}
			}
			if (tmpDirectionList->count() > 1) {
				static unsigned int bitList[] = {
					0x00e,//下
					0x380,//上
					0x092,//左
					0x248,//右
				};
				for (auto bit : bitList) {
					if (bit & directionData->getDirectionBit()) {
						cocos2d::Ref *ref;
						CCARRAY_FOREACH(tmpDirectionList, ref) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
							auto data = static_cast<agtk::data::DirectionData *>(ref);
#else
							auto data = dynamic_cast<agtk::data::DirectionData *>(ref);
#endif
							if (bit & data->getDirectionBit()) {
								return data;
							}
						}
					}
				}
			}
		}
	}
	return motionData->getDirectionDataByDirectionBit(moveDirectionBit, dispDirectionBit);
}


void Object::setDispDirection(int dispDirection)
{
	//         上
	//    +---+---+---+
	//    | 7 | 8 | 9 |
	//    +---+---+---+
	// 左 | 4 |   | 6 | 右
	//    +---+---+---+
	//    | 1 | 2 | 3 |
	//    +---+---+---+
	//         下

	if (_dispDirection != dispDirection) {
		_dispDirectionOld = _dispDirection;
		_dispDirection = dispDirection;

		auto objectData = this->getObjectData();
		if (objectData->getMoveType() == agtk::data::ObjectData::kMoveNormal) {
			//左(1,4,7)から右(3,6,9)、または右(3,6,9)から左(1,4,7)へ向きが変わった場合に_dispFlipYが変化する。
			if ((_dispDirection == 3 || _dispDirection == 6 || _dispDirection == 9)) {
				_dispFlipY = false;
			}
			if ((_dispDirection == 1 || _dispDirection == 4 || _dispDirection == 7)) {
				//右から左
				_dispFlipY = true;
			}
		}
	}
}

int Object::getMoveDirectionBit()
{
	auto moveDirection = this->getMoveDirection();
	if (moveDirection < 0) {
		return 0;
	}
	return 1 << moveDirection;
}

int Object::getMoveDirection()
{
	return _moveDirection;
}

void Object::setMoveDirection(int direction)
{
	if (_moveDirection != direction) {
		_moveDirectionOld = _moveDirection;
		_moveDirection = direction;
	}
}

int Object::getDispDirectionBit()
{
	auto dispDirection = this->getDispDirection();
	if (dispDirection < 0) {
		return 0;
	}
	return 1 << dispDirection;
}

int Object::getCurrentDirectionBit()
{
	auto player = getBasePlayer();
	auto motion = player->getCurrentAnimationMotion();
	if (motion == nullptr) {
		return 0;
	}
	auto direction = motion->getCurrentDirection();
	if (direction == nullptr) {
		return 0;
	}
	auto directionData = direction->getDirectionData();
	return directionData->getDirectionBit();
}

int Object::calcDispDirection(agtk::data::DirectionData *directionData)
{
	if (directionData == nullptr) {
		auto player = getBasePlayer();
		if (player == nullptr) {
			return 0;
		}
		auto motion = player->getCurrentAnimationMotion();
		if (motion == nullptr) {
			return 0;
		}
		auto direction = motion->getCurrentDirection();
		if (direction == nullptr) {
			return 0;
		}
		directionData = direction->getDirectionData();
	}
	auto projectData = GameManager::getInstance()->getProjectData();

	int bit = directionData->getDirectionBit();
	if (directionData->getAutoGeneration()) {//自動生成
		switch (projectData->getGameType()) {
		case agtk::data::ProjectData::kGameTypeSideView:
			//サイドビューは右方向
			bit = 1 << 6;
			break;
		case agtk::data::ProjectData::kGameTypeTopView:
			//トップビューは下方向
			bit = 1 << 2;
			break;
		default:CC_ASSERT(0);
		}
	}
	return agtk::CalcDirectionId(bit);
}

// #AGTK-NX
#if defined(USE_PREVIEW) || ((CC_TARGET_PLATFORM == CC_PLATFORM_NX) && defined(AGTK_DEBUG))
//#ifdef USE_PREVIEW
void Object::setupDebugDisplayArea()
{
	auto player = this->getPlayer();
	if (player) {
		auto debugManager = DebugManager::getInstance();
		player->setVisibleTimeline(agtk::data::TimelineInfoData::kTimelineWall, debugManager->getCollisionWallEnabled());
		player->setVisibleTimeline(agtk::data::TimelineInfoData::kTimelineAttack, debugManager->getCollisionAttackEnabled());
		player->setVisibleTimeline(agtk::data::TimelineInfoData::kTimelineHit, debugManager->getCollisionHitEnabled());
		player->setVisibleTimeline(agtk::data::TimelineInfoData::kTimelineConnection, debugManager->getCollisionConnectionEnabled());
	}
}

void Object::setVisibleDebugDisplayArea(agtk::data::TimelineInfoData::EnumTimelineType type, bool bVisible)
{
	auto player = this->getPlayer();
	if (player) player->setVisibleTimeline(type, bVisible);
}

void Object::setVisibleDebugDisplayAreaAll(bool bVisible)
{
	auto player = this->getPlayer();
	if (player) player->setVisibelTimelineAll(bVisible);
}
#endif

#ifdef AGTK_DEBUG
void Object::setVisibleDebugDisplayPlayer(bool bVisible)
{
	auto player = this->getBasePlayer();
	if (player == nullptr) {
		return;
	}
	switch (player->getType()) {
	case agtk::BasePlayer::Image: {
		auto p = dynamic_cast<agtk::ImagePlayer *>(player);
		p->setVisibleDebugDisplay(bVisible);
		break; }
	case agtk::BasePlayer::SpriteStudio: {
		auto p = dynamic_cast<agtk::SSPlayer *>(player);
		p->setVisibleDebugDisplay(bVisible);
		break; }
	case agtk::BasePlayer::Gif: {
		auto p = dynamic_cast<agtk::GifPlayer *>(player);
		p->setVisibleDebugDisplay(bVisible);
		break; }
	case agtk::BasePlayer::Spine: {
		//auto p = dynamic_cast<agtk::SpinePlayer *>(player);
		//p->setVisibleDebugDisplay(bVisible);
		break; }
	default:CC_ASSERT(0);
	}
}
#endif

#if defined(USE_WALL_DEBUG_DISPLAY)
void Object::createWallDebugDisplay()
{
	auto pm = PrimitiveManager::getInstance();
	auto player = this->getBasePlayer();

	auto p = ObjectWallDebugDisplay::create(this);
	this->setWallDebugDisplay(p);
}

void Object::removeWallDebugDisplay()
{
	auto wallDebugDisplay = dynamic_cast<ObjectWallDebugDisplay *>(this->getWallDebugDisplay());
	wallDebugDisplay->destroy();
	this->setWallDebugDisplay(nullptr);
}

void Object::setVisibleWallDebugDisplay(bool bVisible)
{
	auto wallDebugDisplay = dynamic_cast<ObjectWallDebugDisplay *>(this->getWallDebugDisplay());
	auto array = wallDebugDisplay->getPrimitiveNodeList();
	cocos2d::Ref *ref = nullptr;
	CCARRAY_FOREACH(array, ref) {
		auto primitiveNode = dynamic_cast<cocos2d::Node *>(ref);
		primitiveNode->setVisible(bVisible);
	}
}

bool Object::isVisibleWallDebugDisplay()
{
	//全て表示状態の場合はTRUE,それ以外はFALSE
	auto wallDebugDisplay = dynamic_cast<ObjectWallDebugDisplay *>(this->getWallDebugDisplay());
	auto array = wallDebugDisplay->getPrimitiveNodeList();
	cocos2d::Ref *ref = nullptr;
	CCARRAY_FOREACH(array, ref) {
		auto primitiveNode = dynamic_cast<cocos2d::Node *>(ref);
		if (primitiveNode->isVisible() == false) {
			return false;
		}
	}
	return true;
}

void Object::updateWallDebugDisplay(float dt)
{
	auto p = dynamic_cast<ObjectWallDebugDisplay *>(this->getWallDebugDisplay());
	if (p == nullptr) {
		return;
	}
	p->update(dt);
}

#endif

cocos2d::Node *Object::getAreaNode(int id)
{
	cocos2d::Rect rect = cocos2d::Rect::ZERO;
	auto player = this->getBasePlayer();
	cocos2d::Vec2 position = this->getPlayer()->getPosition();
	cocos2d::Size contentSize = this->getContentSize();

	//オブジェクト無効の場合。
	if (this->getDisabled()) {
		return nullptr;
	}

	auto animationTimelineList = player->getCurrentAnimationMotion()->getCurrentDirection()->getAnimationTimelineList();
	cocos2d::DictElement *el = nullptr;
	CCDICT_FOREACH(animationTimelineList, el) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
		auto animationTimeline = static_cast<agtk::AnimationTimeline *>(el->getObject());
#else
		auto animationTimeline = dynamic_cast<agtk::AnimationTimeline *>(el->getObject());
#endif
		if (animationTimeline->getTimelineInfoData()->getId() == id) {
			cocos2d::Rect rect = animationTimeline->getRect();
			auto n = cocos2d::Node::create();
			float x = rect.origin.x;
			float y = -(rect.origin.y + rect.size.height);
			n->setPosition(position + cocos2d::Vec2(x, y));
			n->setContentSize(rect.size);
			n->setRotation(this->getRotation());
			return n;
		}
	}
	return nullptr;
}

cocos2d::Node *Object::getAreaNode(agtk::data::TimelineInfoData::EnumTimelineType type)
{
	cocos2d::Rect rect = cocos2d::Rect::ZERO;
	auto player = this->getBasePlayer();
	if (player == nullptr) {
		return nullptr;
	}
	//オブジェクト無効の場合。
	if (this->getDisabled()) {
		return nullptr;
	}
	cocos2d::Vec2 playerOrigin = player->getOrigin();
	cocos2d::Vec2 position = this->getPlayer()->getPosition();
	cocos2d::Size contentSize = this->getContentSize();

	auto animationTimelineList = player->getCurrentAnimationMotion()->getCurrentDirection()->getAnimationTimelineList();
	cocos2d::DictElement *el = nullptr;
	CCDICT_FOREACH(animationTimelineList, el) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
		auto animationTimeline = static_cast<agtk::AnimationTimeline *>(el->getObject());
#else
		auto animationTimeline = dynamic_cast<agtk::AnimationTimeline *>(el->getObject());
#endif
		if (animationTimeline->getTimelineInfoData()->getTimelineType() == agtk::data::TimelineInfoData::kTimelineConnection) {
			continue;
		}
		bool bWallAreaAttack = this->getObjectVisible()->isWallAreaAttackWhenInvincible() && (type == agtk::data::TimelineInfoData::kTimelineAttack) && (animationTimeline->getTimelineInfoData()->getTimelineType() == agtk::data::TimelineInfoData::kTimelineHit);//無敵関連のあたり判定を攻撃判定にする。
		if ((animationTimeline->getTimelineInfoData()->getTimelineType() == type || bWallAreaAttack) || (type == agtk::data::TimelineInfoData::kTimelineMax)) {
			cocos2d::Rect r = animationTimeline->getRect();
			r.origin.x += playerOrigin.x;
			r.origin.y = playerOrigin.y - r.origin.y - r.size.height;
			if (rect.size.equals(cocos2d::Size::ZERO)) {
				rect = r;
			} else {
				rect.merge(r);
			}
		}
	}

	auto n = cocos2d::Node::create();
	n->setPosition(position + rect.origin);
	n->setContentSize(rect.size);
	n->setRotation(this->getRotation());
	return n;
}

cocos2d::__Array *Object::getAreaArray(agtk::data::TimelineInfoData::EnumTimelineType type)
{
	return getAreaArray(cocos2d::Vec2::ZERO, type);
}

cocos2d::__Array *Object::getAreaArray(cocos2d::Vec2 move, agtk::data::TimelineInfoData::EnumTimelineType type)
{
	auto nodeArray = cocos2d::__Array::create();
	auto basePlayer = this->getBasePlayer();
	if (basePlayer == nullptr) {
		return nodeArray;
	}
	//オブジェクト無効の場合。
	if (this->getDisabled()) {
		return nodeArray;
	}
	auto player = this->getPlayer();
	auto origin = basePlayer->getOrigin();
	auto position = player->getPosition() + move + origin;
	cocos2d::Size contentSize = this->getContentSize();
	auto offset = basePlayer->getOffset();

	auto currentDirection = basePlayer->getCurrentAnimationMotion()->getCurrentDirection();
	if (currentDirection != nullptr) {
		auto animationTimelineList = currentDirection->getAnimationTimelineList();
		cocos2d::DictElement *el = nullptr;
		CCDICT_FOREACH(animationTimelineList, el) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
			auto animationTimeline = static_cast<agtk::AnimationTimeline *>(el->getObject());
#else
			auto animationTimeline = dynamic_cast<agtk::AnimationTimeline *>(el->getObject());
#endif
			bool bWallAreaAttack = this->getObjectVisible()->isWallAreaAttackWhenInvincible() && (type == agtk::data::TimelineInfoData::kTimelineAttack) && (animationTimeline->getTimelineInfoData()->getTimelineType() == agtk::data::TimelineInfoData::kTimelineHit);//無敵関連のあたり判定を攻撃判定にする。
			if (animationTimeline->getTimelineInfoData()->getTimelineType() == type || bWallAreaAttack) {
				cocos2d::Rect rect = animationTimeline->getRect();
				auto n = cocos2d::Node::create();
				float x = rect.origin.x + offset.x;
				float y = -(rect.origin.y + rect.size.height) + offset.y;
				n->setPosition(position + cocos2d::Vec2(x, y));
				n->setContentSize(rect.size);
				n->setRotation(this->getRotation());
				n->setTag(animationTimeline->getTimelineInfoData()->getId());//※タグにIDを設定する。
				nodeArray->addObject(n);
			}
		}
	}
	return nodeArray;
}

bool Object::getTimelineAreaList(agtk::data::TimelineInfoData::EnumTimelineType type, std::vector<agtk::Vertex4> &vertex4List)
{
	auto basePlayer = this->getBasePlayer();
	if (basePlayer == nullptr) {
		return false;
	}
	//オブジェクト無効の場合。
	if (this->getDisabled()) {
		return false;
	}

	std::function<agtk::Vertex4(agtk::Object *, agtk::AnimationTimeline *)> calcTimelineArea = [&](agtk::Object *object, agtk::AnimationTimeline *animationTimeline)
	{
		agtk::Vertex4 vertex;
		auto player = object->getPlayer();
		auto basePlayer = object->getBasePlayer();
		auto offset = basePlayer->getOffset();
		auto angle = player->getAutoGenerationNode()->getRotation();
		auto scale = basePlayer->getInnerScale();
		scale.x *= this->getScaleX();
		scale.y *= this->getScaleY();
		auto rect = animationTimeline->getRect();
		auto origin = basePlayer->getOrigin();
		auto pos = player->getPosition();

		float x = rect.origin.x + offset.x + origin.x;
		float y = -(rect.origin.y + rect.size.height) + (-offset.y) + origin.y;
		auto v = cocos2d::Vec2(x, y).rotateByAngle(cocos2d::Vec2::ZERO, -CC_DEGREES_TO_RADIANS(angle));		
		vertex[0] = cocos2d::Vec2(pos.x + v.x * scale.x, pos.y + v.y * scale.y);
		v = cocos2d::Vec2(x, (y + rect.size.height)).rotateByAngle(cocos2d::Vec2::ZERO, -CC_DEGREES_TO_RADIANS(angle));
		vertex[1] = cocos2d::Vec2(pos.x + v.x * scale.x, pos.y + v.y * scale.y);
		v = cocos2d::Vec2((x + rect.size.width), (y + rect.size.height)).rotateByAngle(cocos2d::Vec2::ZERO, -CC_DEGREES_TO_RADIANS(angle));
		vertex[2] = cocos2d::Vec2(pos.x + v.x * scale.x, pos.y + v.y * scale.y);
		v = cocos2d::Vec2((x + rect.size.width), y).rotateByAngle(cocos2d::Vec2::ZERO, -CC_DEGREES_TO_RADIANS(angle));
		vertex[3] = cocos2d::Vec2(pos.x + v.x * scale.x, pos.y + v.y * scale.y);
		return vertex;
	};

	auto currentDirection = basePlayer->getCurrentAnimationMotion()->getCurrentDirection();
	if (currentDirection != nullptr) {
		auto animationTimelineList = currentDirection->getAnimationTimelineList();
		cocos2d::DictElement *el = nullptr;
		CCDICT_FOREACH(animationTimelineList, el) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
			auto animationTimeline = static_cast<agtk::AnimationTimeline *>(el->getObject());
#else
			auto animationTimeline = dynamic_cast<agtk::AnimationTimeline *>(el->getObject());
#endif
			bool bWallAreaAttack = this->getObjectVisible()->isWallAreaAttackWhenInvincible() && (type == agtk::data::TimelineInfoData::kTimelineAttack) && (animationTimeline->getTimelineInfoData()->getTimelineType() == agtk::data::TimelineInfoData::kTimelineHit);//無敵関連のあたり判定を攻撃判定にする。
			if (animationTimeline->getTimelineInfoData()->getTimelineType() == type || bWallAreaAttack) {
				auto v = calcTimelineArea(this, animationTimeline);
				vertex4List.push_back(v);
			}
		}
	}
	return vertex4List.size() > 0;
}

bool Object::getTimeline(agtk::data::TimelineInfoData::EnumTimelineType type, int id, agtk::Vertex4 &vertex4)
{
	auto basePlayer = this->getBasePlayer();
	if (basePlayer == nullptr) {
		return false;
	}
	//オブジェクト無効の場合。
	if (this->getDisabled()) {
		return false;
	}
	auto player = this->getPlayer();
	return player->convertToLayerSpaceTimelineVertex4(id, vertex4);
}

void Object::getTimelineList(agtk::data::TimelineInfoData::EnumTimelineType type, std::vector<agtk::Vertex4> &vertex4List)
{
#ifdef USE_COLLISION_MEASURE
	if (type == agtk::data::TimelineInfoData::kTimelineWall) {
#if 0
		if (this->getId() == 1 && this->_playObjectData->getId() == 2 && strcmp(this->_objectData->getName(), "bullet") == 0) {
			CCLOG("%s(%d)", this->_objectData->getName(), this->getId());
		}
#endif
		wallCollisionCount++;
	}
	if (type == agtk::data::TimelineInfoData::kTimelineHit) {
		hitCollisionCount++;
	}
	if (type == agtk::data::TimelineInfoData::kTimelineAttack) {
		attackCollisionCount++;
	}
	if (type == agtk::data::TimelineInfoData::kTimelineConnection) {
		connectionCollisionCount++;
	}
	if (type == agtk::data::TimelineInfoData::kTimelineMaxWithoutConnection) {
		woConnectionCollisionCount++;
	}
#endif
	auto basePlayer = this->getBasePlayer();
	if (basePlayer == nullptr) {
		return;
	}
	//オブジェクト無効の場合。
	if (this->getDisabled()) {
		return;
	}
	auto player = this->getPlayer();
	// VertListCache更新判定
#ifdef USE_MULTITHREAD_OBJECT_UPDATE
	player->lock();
#endif
	if (player->isUpdateTimelineVertListCache(type)) {
		player->VertListCacheClear(type);
		player->updateTimelineVertListCache(type);
	}
	player->getTimelineVertListCache(type, vertex4List);
#ifdef USE_MULTITHREAD_OBJECT_UPDATE
	player->unlock();
#endif

	// 壁当たり判定取得を行うと、他の判定がくっついてきて
	// 当たり判定処理がおかしくなったのでここで無理やり処理を終了する
	if (type == agtk::data::TimelineInfoData::kTimelineWall) {
		return;
	}

	if (type == agtk::data::TimelineInfoData::kTimelineAttack && this->getObjectVisible()->isWallAreaAttackWhenInvincible()) {
#ifdef USE_MULTITHREAD_OBJECT_UPDATE
		player->lock();
#endif
		// VertListCache更新判定
		if (player->isUpdateTimelineVertListCache(type)) {
			player->VertListCacheClear(agtk::data::TimelineInfoData::kTimelineHit);
			player->updateTimelineVertListCache(agtk::data::TimelineInfoData::kTimelineHit);
		}
		player->getTimelineVertListCache(agtk::data::TimelineInfoData::kTimelineHit, vertex4List);
#ifdef USE_MULTITHREAD_OBJECT_UPDATE
		player->unlock();
#endif
	}
}

// #AGTK-NX #AGTK-WIN
#ifdef USE_30FPS_4
void Object::getWallTimelineList(std::vector<agtk::Vertex4> &vertex4List)
{
	auto mf = this->_middleFrameStock.getUpdatedMiddleFrame();
	if (this->_bUseMiddleFrame && mf->_hasMiddleFrame) {
		// 中間フレームの壁判定を取得対象とする
		vertex4List = mf->_wallList;
		Vec2 diff = this->getPosition() - mf->_objectPos;
		diff.y = -diff.y;
		for (auto it = vertex4List.begin(); it != vertex4List.end(); it++) {
			for (int i = 0; i < 4; i++) {
				Vec2& p = (*it)[i];
				p += diff;
			}
		}
	}
	else {
		this->getTimelineList(agtk::data::TimelineInfoData::kTimelineWall, vertex4List);
	}
}
#endif

void Object::updateTimelineListCacheSingle(agtk::data::TimelineInfoData::EnumTimelineType type)
{
	auto basePlayer = this->getBasePlayer();
	if (basePlayer == nullptr) {
		return;
	}
	//オブジェクト無効の場合。
	if (this->getDisabled()) {
		return;
	}
	auto player = this->getPlayer();
	player->updateTimelineVertListCache(type);

	// 壁当たり判定取得を行うと、他の判定がくっついてきて
	// 当たり判定処理がおかしくなったのでここで無理やり処理を終了する
	if (type == agtk::data::TimelineInfoData::kTimelineWall) {
		return;
	}

	if (type == agtk::data::TimelineInfoData::kTimelineAttack && this->getObjectVisible()->isWallAreaAttackWhenInvincible()) {
		player->updateTimelineVertListCache(agtk::data::TimelineInfoData::kTimelineHit, true);
	}
}

void Object::getAreaTotalRect(agtk::data::TimelineInfoData::EnumTimelineType type, Point *pos, Size *size)
{
	auto array = getAreaArray(type);
	float minX = 0, minY = 0, maxX = -1, maxY = -1;
	if (array->count() > 0){
		cocos2d::Ref *ref = nullptr;
		bool first = true;
		CCARRAY_FOREACH(array, ref) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
			auto node = static_cast<cocos2d::Node *>(ref);
#else
			auto node = dynamic_cast<cocos2d::Node *>(ref);
#endif
			auto position = node->getPosition();
			auto contentSize = node->getContentSize();
			auto rotation = node->getRotation();
			//todo: rotationを考慮する。
			auto x0 = position.x;
			auto y0 = position.y;
			auto x1 = position.x + contentSize.width - 1;
			auto y1 = position.y + contentSize.height - 1;
			if (first){
				minX = x0;
				minY = y0;
				maxX = x1;
				maxY = y1;
				continue;
			}
			if (x0 < minX) minX = x0;
			if (y0 < minY) minY = y0;
			if (x1 > maxX) maxX = x1;
			if (y1 > maxY) maxY = y1;
		}
	}
	pos->setPoint(minX, minY);
	size->setSize(maxX - minX + 1, maxY - minY + 1);
}

/**
* 指定の接続点の座標取得
* @param	id	接続点ID
* @param	out	接続点座標出力用Vec2(プレイヤーの原点からの座標)
* @return		接続点のキーフレームが存在したか？
*/
bool Object::getConnectionPoint(int id, cocos2d::Vec2 *out)
{
	auto basePlayer = this->getBasePlayer();
	auto player = this->getPlayer();
	if (basePlayer == nullptr || player == nullptr) {
		return false;
	}
	//オブジェクト無効の場合。
	if (this->getDisabled()) {
		return false;
	}
	auto animationTimelineList = basePlayer->getCurrentAnimationMotion()->getCurrentDirection()->getAnimationTimelineList();
	cocos2d::DictElement *el = nullptr;
	CCDICT_FOREACH(animationTimelineList, el) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
		auto animationTimeline = static_cast<agtk::AnimationTimeline *>(el->getObject());
#else
		auto animationTimeline = dynamic_cast<agtk::AnimationTimeline *>(el->getObject());
#endif

		auto timelineInfo = animationTimeline->getTimelineInfoData();
		if (timelineInfo->getId() == id && timelineInfo->getAreaList()->count() > 0) {
			cocos2d::Rect rect = animationTimeline->getRect();
			cocos2d::Size size = player->getContentSize();
			cocos2d::Vec2 pos;

			auto trans = player->getTransformNode()->getNodeToWorldTransform();
			auto trans2 = player->getWorldToNodeTransform();
			pos.x = rect.origin.x;
			pos.y = (size.height - rect.origin.y);
			pos = PointApplyTransform(pos, trans);
			pos = PointApplyTransform(pos, trans2);
			out->x = pos.x;
			out->y = pos.y;
			return true;
		}
	}

	return false;
}

cocos2d::Rect Object::getAreaRect(int id)
{
	cocos2d::Vec2 position = this->getPosition();
	cocos2d::Size contentSize = this->getContentSize();

	auto player = this->getBasePlayer();
	if (player == nullptr) {
		return cocos2d::Rect(position.x, position.y, 0, 0);
	}
	//オブジェクト無効の場合。
	if (this->getDisabled()) {
		return cocos2d::Rect(position.x, position.y, 0, 0);
	}
	auto animationTimelineList = player->getCurrentAnimationMotion()->getCurrentDirection()->getAnimationTimelineList();
	cocos2d::DictElement *el = nullptr;
	CCDICT_FOREACH(animationTimelineList, el) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
		auto animationTimeline = static_cast<agtk::AnimationTimeline *>(el->getObject());
#else
		auto animationTimeline = dynamic_cast<agtk::AnimationTimeline *>(el->getObject());
#endif
		if (animationTimeline->getTimelineInfoData()->getId() == id) {
			cocos2d::Rect r = animationTimeline->getRect();
			cocos2d::Vec2 origin = player->getOrigin();
			r.origin.x = r.origin.x + position.x + origin.x;
			r.origin.y = r.origin.y + position.y - origin.y;
			return r;
		}
	}
	CC_ASSERT(0);
	return cocos2d::Rect::ZERO;
}

bool Object::getAreaBackSide(int id)
{
	auto player = this->getBasePlayer();
	if (player == nullptr) {
		return false;
	}
	//オブジェクト無効の場合。
	if (this->getDisabled()) {
		return false;
	}
	auto animationTimelineList = player->getCurrentAnimationMotion()->getCurrentDirection()->getAnimationTimelineList();
	cocos2d::DictElement *el = nullptr;
	CCDICT_FOREACH(animationTimelineList, el) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
		auto animationTimeline = static_cast<agtk::AnimationTimeline *>(el->getObject());
#else
		auto animationTimeline = dynamic_cast<agtk::AnimationTimeline *>(el->getObject());
#endif
		if (animationTimeline->getTimelineInfoData()->getId() == id) {

			return animationTimeline->getBackSide();
		}
	}

	return false;
}

agtk::AreaData *Object::getAreaData(int id)
{
	cocos2d::Vec2 position = this->getPosition();
	cocos2d::Size contentSize = this->getContentSize();

	auto player = this->getBasePlayer();
	if (player == nullptr) {
		return nullptr;
	}
	//オブジェクト無効の場合。
	if (this->getDisabled()) {
		return nullptr;
	}
	auto animationTimelineList = player->getCurrentAnimationMotion()->getCurrentDirection()->getAnimationTimelineList();
	cocos2d::DictElement *el = nullptr;
	CCDICT_FOREACH(animationTimelineList, el) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
		auto animationTimeline = static_cast<agtk::AnimationTimeline *>(el->getObject());
#else
		auto animationTimeline = dynamic_cast<agtk::AnimationTimeline *>(el->getObject());
#endif
		if (animationTimeline->getTimelineInfoData()->getId() == id) {
			cocos2d::Rect r = animationTimeline->getRect();
			cocos2d::Vec2 origin = player->getOrigin();
			r.origin.x = r.origin.x + origin.x;
			r.origin.y = r.origin.y - origin.y;
			auto areaData = agtk::AreaData::create(position, r.size, r.origin, this->getPlayer()->getAutoGenerationNode()->getRotation() - this->getPlayer()->getCenterRotation());
			areaData->calcArea();
			return areaData;
		}
	}
	CC_ASSERT(0);
	return nullptr;
}

bool Object::existsArea(int id)
{
	auto basePlayer = this->getBasePlayer();
	if (basePlayer == nullptr) {
		return false;
	}
	//オブジェクト無効の場合。
	if (this->getDisabled()) {
		return false;
	}
	auto animationTimelineList = basePlayer->getCurrentAnimationMotion()->getCurrentDirection()->getAnimationTimelineList();
	cocos2d::DictElement *el = nullptr;
	CCDICT_FOREACH(animationTimelineList, el) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
		auto animationTimeline = static_cast<agtk::AnimationTimeline *>(el->getObject());
#else
		auto animationTimeline = dynamic_cast<agtk::AnimationTimeline *>(el->getObject());
#endif

		auto data = animationTimeline->getTimelineInfoData();
		if (data->getId() == id) {
			auto player = this->getPlayer();
			bool valid = (player) ? player->getTimelineValid(id) : true;

			// キーフレームの設定があり、かつ存在する場合
			return (data->getAreaList()->count() > 0) && valid;
		}
	}

	return false;
}

void Object::addChildObject(agtk::Object * child, cocos2d::Vec2 connectOffset, int connectId)
{
	// オブジェクトリストにまだ追加されていない場合、
	auto childrenList = this->getChildrenObjectList();
	if (!childrenList->containsObject(child)) {
		// リストに追加する
		childrenList->addObject(child);

		// 子オブジェクトに新しい親オブジェクトを設定する
		child->changeParentObject(this, connectOffset, connectId);
	}
}

void Object::removeChildObject(agtk::Object * child)
{
	// 子オブジェクトリストから対象の子オブジェクトを除去する
	auto childrenList = this->getChildrenObjectList();
	if (childrenList->containsObject(child)) {
		childrenList->removeObject(child);
	}
}

void Object::changeParentObject(agtk::Object * parent, cocos2d::Vec2 connectOffset, int connectId)
{
	// 親オブジェクトが存在する場合
	if (getOwnParentObject()) {
		// 現在の親オブジェクトとの親子関係を削除
		getOwnParentObject()->removeChildObject(this);
	}

	// 親オブジェクトを設定
	setOwnParentObject(parent);

	// 追従箇所のオフセットを設定
	_parentFollowPosOffset = connectOffset;

	// 追従箇所の親オブジェクトの接続点ID
	_parentFollowConnectId = connectId;
}

/**
* オブジェクトの座標を行動範囲制限内に入るように変更する
*/
void Object::changePositionInLimitArea(agtk::data::SceneData* sceneData)
{
	// 座標変更を行うか？
	bool shouldChangePos = false;

	// プレイヤーオブジェクトは行動範囲制限の影響を受ける
	if (this->getObjectData()->isGroupPlayer()) {
		shouldChangePos = true;
	}

	// 座標変更を行う場合
	if (shouldChangePos) {
		auto objectCollision = this->getObjectCollision();
		if (objectCollision) {
			// オブジェクトの壁当たり判定矩形を取得
			auto objectRect = objectCollision->getOldWallHitInfoGroup()->getRect();

			// オブジェクトの矩形がない場合は位置の確認のしようがないので処理しない
			if (objectRect.size.equals(Size::ZERO)) {
				return;
			}

			// 行動範囲制限の矩形を取得
			auto sceneRect = Rect(
				sceneData->getLimitAreaX(),
				sceneData->getLimitAreaY() + sceneData->getLimitAreaHeight(), 
				sceneData->getLimitAreaWidth(), 
				sceneData->getLimitAreaHeight()
			);
			sceneRect.origin = agtk::Scene::getPositionCocos2dFromScene(sceneRect.origin, sceneData);
			
			// オブジェクトの矩形が行動範囲制限より大きい場合は動かしようがないので何もしない
			if (objectRect.size.width > sceneRect.size.width || objectRect.size.height > sceneRect.size.height) {
				return;
			}

			// 行動範囲制限の無効。
			if (sceneData->getDisableLimitArea()) {
				return;
			}

			// オブジェクトの移動を行う
			Vec2 move = Vec2::ZERO;

			// 行動範囲制限左にオブジェクトが接触している場合
			if (objectRect.getMinX() < sceneRect.getMinX()) {
				move.x = sceneRect.getMinX() - objectRect.getMinX();
			}
			// 行動範囲制限右にオブジェクトが接触している場合
			else if (sceneRect.getMaxX() < objectRect.getMaxX()) {
				move.x = sceneRect.getMaxX() - objectRect.getMaxX();
			}

			// 行動範囲制限上にオブジェクトが接触している場合
			if (sceneRect.getMaxY() < objectRect.getMaxY()) {
				move.y = objectRect.getMaxY() - sceneRect.getMaxY();
			}
			// 行動範囲制限下にオブジェクトが接触している場合
			else if (objectRect.getMinY() < sceneRect.getMinY()) {
				move.y = objectRect.getMinY() - sceneRect.getMinY();
			}

			// 移動が発生した場合かつ、プレイヤーの行動制限が有効の場合。
			if (!move.isZero()) {
				// 座標を更新
				move.y *= -1;
				this->addPosition(move);

				// 過去の位置を上書き
				this->setOldPosition(this->getPosition());
				// 過去のオブジェクトの矩形も更新しておく
				objectCollision->lateUpdateWallHitInfoGroup();
			}
		}
	}
}

void Object::getSwitchInfoCreateConnectObject()
{
	auto list = this->getSwitchInfoCreateConnectObjectList();
	auto settingList = getObjectData()->getConnectSettingList();
	cocos2d::DictElement *el;
	CCDICT_FOREACH(settingList, el) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
		auto setting = static_cast<agtk::data::ObjectConnectSettingData *>(el->getObject());
#else
		auto setting = dynamic_cast<agtk::data::ObjectConnectSettingData *>(el->getObject());
#endif
		// 「このオブジェクトのスイッチ」にチェックが入っている場合
		if (setting->getObjectSwitch() && setting->getObjectSwitchId() != -1) {
			// オブジェクト固有のスイッチを取得
			auto switchData = getPlayObjectData()->getSwitchData(setting->getObjectSwitchId());
			if (switchData != nullptr) {
				list->setObject(cocos2d::__Bool::create(switchData->getValue()), (intptr_t)switchData);
			}
		}
		// 「システム共通」にチェックが入っている場合
		else if (setting->getObjectSwitch() && setting->getSystemSwitchId() != -1) {
			// システム共通のスイッチを取得
			auto switchData = GameManager::getInstance()->getPlayData()->getCommonSwitchData(setting->getSystemSwitchId());
			if (switchData != nullptr) {
				list->setObject(cocos2d::__Bool::create(switchData->getValue()), (intptr_t)switchData);
			}
		}
	}
}

// #AGTK-NX
#ifdef USE_SAR_OPTIMIZE_4
void Object::checkCreateConnectObject(bool bInnerFunctionCall, bool bUpdateSwitch, agtk::data::PlaySwitchData* updatedSwitch)
{
	if (!getObjectData()->getConnectSettingFlag()) { return; }
	bool bNonSettingActive = bInnerFunctionCall || (bInnerFunctionCall == false && _frameCount == 0);

	auto settingList = getObjectData()->getConnectSettingList();
	cocos2d::DictElement *el = nullptr;
	CCDICT_FOREACH(settingList, el) {
		auto setting = static_cast<agtk::data::ObjectConnectSettingData *>(el->getObject());
		bool isActive = false;

		// 「このオブジェクトのスイッチ」にチェックが入っている場合
		if (setting->getObjectSwitch()) {
			if (setting->getObjectSwitchId() == -1) {
				if (bUpdateSwitch) continue;
				// 「無し」設定時は常にアクティブ
				isActive = bNonSettingActive;
			}
			else {
				// オブジェクト固有のスイッチを取得
				auto switchData = getPlayObjectData()->getSwitchData(setting->getObjectSwitchId());
				if (switchData != nullptr) {
					if (switchData == updatedSwitch) {
						if (switchData->checkChangeValue()) {
							isActive = switchData->getValue();
						}
						else {
							//変更がない場合。
							continue;
						}
					}
					else {
						isActive = switchData->getValue();
					}
				}
			}
		}
		// 「システム共通」にチェックが入っている場合
		else {
			if (setting->getSystemSwitchId() == -1) {
				if (bUpdateSwitch) continue;
				// 「無し」設定時は常にアクティブ
				isActive = bNonSettingActive;
			}
			else {
				// システム共通のスイッチを取得
				auto switchData = GameManager::getInstance()->getPlayData()->getCommonSwitchData(setting->getSystemSwitchId());
				if (switchData != nullptr) {
					if (switchData == updatedSwitch) {
						if (switchData->checkChangeValue()) {
							isActive = switchData->getValue();
						}
						else {
							//変更がない場合。
							continue;
						}
					}
					else {
						isActive = switchData->getValue();
					}
				}
			}
		}

		// 接続する場合
		if (isActive) {

			// 接続するオブジェクトが既に接続済みの場合は処理しない
			cocos2d::Ref *ref = nullptr;
			bool isConnected = false;
			CCARRAY_FOREACH(this->getConnectObjectList(), ref) {
				// #AGTK-NX
#ifdef STATIC_DOWN_CAST
				auto connectObj = static_cast<agtk::ConnectObject *>(ref);
#else
				auto connectObj = dynamic_cast<agtk::ConnectObject *>(ref);
#endif
				if (connectObj->getObjectConnectSettingData()->getId() == setting->getId()) {
					isConnected = true;
					break;
				}
			}

			if (!isConnected) {
				// 対象のオブジェクトを生成する
				ConnectObject * obj = ConnectObject::create(this, setting->getId(), -1);
				if (obj) {
					this->getConnectObjectList()->addObject(obj);

					// 接続するオブジェクトに紐付いた物理オブジェクトを生成する
					this->getSceneLayer()->createPhysicsObjectWithObject(obj);
				}
			}
		}
		// 接続解除する場合
		else {
			// オブジェクトを削除
			cocos2d::Ref *ref = nullptr;
			CCARRAY_FOREACH(this->getConnectObjectList(), ref) {
				// #AGTK-NX
#ifdef STATIC_DOWN_CAST
				auto connectObj = static_cast<agtk::ConnectObject *>(ref);
#else
				auto connectObj = dynamic_cast<agtk::ConnectObject *>(ref);
#endif
				if (connectObj->getObjectConnectSettingData()->getId() == setting->getId()) {
					// 接続解除
					connectObj->unconnect();
					//破棄リクエスト。
					if (connectObj->getForceDisabled()) {
						connectObj->setLateRemove(true);
					}

					// リストから除去する
					this->getConnectObjectList()->removeObject(connectObj);
					break;
				}
			}
		}
	}
}
#else
void Object::checkCreateConnectObject(bool bInnerFunctionCall, bool bUpdateSwitch)
{
	if (!getObjectData()->getConnectSettingFlag()) { return; }
	bool bNonSettingActive = bInnerFunctionCall || (bInnerFunctionCall == false && _frameCount == 0);

	auto switchInfoList = this->getSwitchInfoCreateConnectObjectList();
	auto settingList = getObjectData()->getConnectSettingList();
	cocos2d::DictElement *el = nullptr;
	CCDICT_FOREACH(settingList, el) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
		auto setting = static_cast<agtk::data::ObjectConnectSettingData *>(el->getObject());
#else
		auto setting = dynamic_cast<agtk::data::ObjectConnectSettingData *>(el->getObject());
#endif

		bool isActive = false;

		// 「このオブジェクトのスイッチ」にチェックが入っている場合
		if (setting->getObjectSwitch()) {
			if (setting->getObjectSwitchId() == -1) {
				if (bUpdateSwitch) continue;
				// 「無し」設定時は常にアクティブ
				isActive = bNonSettingActive;
			}
			else {
				// オブジェクト固有のスイッチを取得
				auto switchData = getPlayObjectData()->getSwitchData(setting->getObjectSwitchId());
				if (switchData != nullptr) {
					auto boolian = dynamic_cast<cocos2d::__Bool *>(switchInfoList->objectForKey((intptr_t)switchData));
					if (boolian != nullptr) {
						if (boolian->getValue() != switchData->getValue()) {
							isActive = switchData->getValue();
						}
						else {
							//変更がない場合。
							continue;
						}
					}
					else {
						isActive = switchData->getValue();
					}
				}
			}
		}
		// 「システム共通」にチェックが入っている場合
		else {
			if (setting->getSystemSwitchId() == -1) {
				if (bUpdateSwitch) continue;
				// 「無し」設定時は常にアクティブ
				isActive = bNonSettingActive;
			}
			else {
				// システム共通のスイッチを取得
				auto switchData = GameManager::getInstance()->getPlayData()->getCommonSwitchData(setting->getSystemSwitchId());
				if (switchData != nullptr) {
					auto boolian = dynamic_cast<cocos2d::__Bool *>(switchInfoList->objectForKey((intptr_t)switchData));
					if (boolian != nullptr) {
						if (boolian->getValue() != switchData->getValue()) {
							isActive = switchData->getValue();
						}
						else {
							//変更がない場合。
							continue;
						}
					}
					else {
						isActive = switchData->getValue();
					}
				}
			}
		}

		// 接続する場合
		if (isActive) {

			// 接続するオブジェクトが既に接続済みの場合は処理しない
			cocos2d::Ref *ref = nullptr;
			bool isConnected = false;
			CCARRAY_FOREACH(this->getConnectObjectList(), ref) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
				auto connectObj = static_cast<agtk::ConnectObject *>(ref);
#else
				auto connectObj = dynamic_cast<agtk::ConnectObject *>(ref);
#endif
				if (connectObj->getObjectConnectSettingData()->getId() == setting->getId()) {
					isConnected = true;
					break;
				}
			}
			
			if (!isConnected) {
				// 対象のオブジェクトを生成する
				ConnectObject * obj = ConnectObject::create(this, setting->getId(), -1);
				if (obj) {
					this->getConnectObjectList()->addObject(obj);

					//親オブジェクトが強制表示・非表示状態の場合、接続オブジェクトも同じステータスにする。
					if (_forceVisibleState != ForceVisibleState::kIgnore) {
						obj->setForceVisibleState(_forceVisibleState);
						obj->setWaitDuration300(-1);
					}

					// 接続するオブジェクトに紐付いた物理オブジェクトを生成する
					this->getSceneLayer()->createPhysicsObjectWithObject(obj);
				}
			}
		}
		// 接続解除する場合
		else {
			// オブジェクトを削除
			cocos2d::Ref *ref = nullptr;
			CCARRAY_FOREACH(this->getConnectObjectList(), ref) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
				auto connectObj = static_cast<agtk::ConnectObject *>(ref);
#else
				auto connectObj = dynamic_cast<agtk::ConnectObject *>(ref);
#endif
				if (connectObj->getObjectConnectSettingData()->getId() == setting->getId()) {
					// 接続解除
					connectObj->unconnect();
					//破棄リクエスト。
					if (connectObj->getForceDisabled()) {
						connectObj->setLateRemove(true);
					}
					if (connectObj->getForceVisibleState() != ForceVisibleState::kIgnore) {
						//強制的に表示・非表示状態だった場合は強制無効にする。
						connectObj->setForceVisibleState(ForceVisibleState::kIgnore);
						connectObj->setWaitDuration300(0);
					}

					// リストから除去する
					this->getConnectObjectList()->removeObject(connectObj);
					break;
				}
			}
		}
	}
	switchInfoList->removeAllObjects();
}
#endif

void Object::unconnectAllConnectObject()
{
	// 全ての接続オブジェクトとの接続を無効化する
	cocos2d::Ref *ref = nullptr;
	CCARRAY_FOREACH(this->getConnectObjectList(), ref) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
		auto connectObj = static_cast<agtk::ConnectObject *>(ref);
#else
		auto connectObj = dynamic_cast<agtk::ConnectObject *>(ref);
#endif
		connectObj->unconnect();
	}
}

// #AGTK-NX
#ifdef USE_SAR_OPTIMIZE_4
void Object::setupWatchSwitchList()
{
	auto watchList = new std::vector<intptr_t>();
	setWatchSwitchList(watchList);

	auto settingList = getObjectData()->getConnectSettingList();
	cocos2d::DictElement *el;
	CCDICT_FOREACH(settingList, el) {
		auto setting = static_cast<agtk::data::ObjectConnectSettingData *>(el->getObject());
		// 「このオブジェクトのスイッチ」にチェックが入っている場合
		if (setting->getObjectSwitch() && setting->getObjectSwitchId() != -1) {
			// オブジェクト固有のスイッチを取得
			auto switchData = getPlayObjectData()->getSwitchData(setting->getObjectSwitchId());
			if (switchData != nullptr) {
				intptr_t key = reinterpret_cast<intptr_t>(switchData);
				auto result = std::find(watchList->begin(), watchList->end(), key);
				if (result == watchList->end()) {
					getWatchSwitchList()->push_back(key);
				}
			}
		}
		// 「システム共通」にチェックが入っている場合
		else if (!setting->getObjectSwitch() && setting->getSystemSwitchId() != -1) {
			// システム共通のスイッチを取得
			auto switchData = GameManager::getInstance()->getPlayData()->getCommonSwitchData(setting->getSystemSwitchId());
			if (switchData != nullptr) {
				intptr_t key = reinterpret_cast<intptr_t>(switchData);
				auto result = std::find(watchList->begin(), watchList->end(), key);
				if (result == watchList->end()) {
					getWatchSwitchList()->push_back(key);
				}
			}
		}
	}
#if 0 // GameManager::updateObjectVariableAndSwitch()で無効スイッチ/無敵スイッチが変化する/しないに関わらず処理するように変更
	// 無効スイッチ
	{
		auto playObjectData = getPlayObjectData();
		auto switchData = playObjectData->getSwitchData(agtk::data::kObjectSystemSwitchDisabled);
		intptr_t key = reinterpret_cast<intptr_t>(switchData);
		auto result = std::find(watchList->begin(), watchList->end(), key);
		if (result == watchList->end()) {
			getWatchSwitchList()->push_back(key);
		}
	}

	//無敵スイッチ
	{
		auto playObjectData = getPlayObjectData();
		auto switchData = playObjectData->getSwitchData(agtk::data::kObjectSystemSwitchInvincible);
		intptr_t key = reinterpret_cast<intptr_t>(switchData);
		auto result = std::find(watchList->begin(), watchList->end(), key);
		if (result == watchList->end()) {
			getWatchSwitchList()->push_back(key);
		}
	}
#endif
}

void Object::registerSwitchWatcher()
{
	auto watchList = getWatchSwitchList();
	for (auto it = watchList->begin(); it != watchList->end(); it++)
	{
		GameManager::getInstance()->addSwitchWatcher(this, *it);
	}
}

void Object::unregisterSwitchWatcher()
{
	auto watchList = getWatchSwitchList();
	for (auto it = watchList->begin(); it != watchList->end(); it++)
	{
		GameManager::getInstance()->removeSwitchWatcher(this, *it);
	}
}

#endif

void Object::removeAfterimage()
{
	_objectAfterImage->stop();
}

void Object::removeSelf(bool bDisappearFlag, bool bIgnoredReappearCondition, unsigned int removeOption)
{
	auto scene = GameManager::getInstance()->getCurrentScene();
	auto sceneLayer = this->getSceneLayer();
#ifdef USE_PREVIEW
	//オブジェクトプレビューのインスタンスが破棄されたことをEditorに通知する。
	if (scene->getPreviewInstanceId() == this->getInstanceId()) {
		scene->setPreviewObjectId(-1);
		scene->setPreviewInstanceId(-1);
		auto message = cocos2d::String::createWithFormat("object feedbackInstanceInfo { \"instanceId\": %d, \"objectId\": %d }", -1, -1);
		auto gameManager = GameManager::getInstance();
		auto ws = gameManager->getWebSocket();
		if (ws) {
			ws->send(message->getCString());
		}
	}
#endif
	//----------------------------------------------------------------------------------------------------------
	//オブジェクト消滅時の設定
	auto objectData = this->getObjectData();
	if (objectData->getDisappearSettingFlag() && bDisappearFlag && !this->getDisappearFlag()) {
		auto scene = GameManager::getInstance()->getCurrentScene();
		auto camera = scene->getCamera();
		auto disappearSetting = objectData->getDisappearSetting();

		auto objectAction = this->getCurrentObjectAction();
		//実行アクション追加。
		if (disappearSetting->getObjCommandList()) {
			auto scenePartData = this->getScenePartObjectData();
			cocos2d::__Dictionary *objCommandList = nullptr;
			if (scenePartData == nullptr) {
				objCommandList = disappearSetting->getObjCommandList();
			}
			else {
				objCommandList = ObjectAction::getObjCommandListByInstanceConfigurable(
					disappearSetting->getObjCommandList(),
					scenePartData->getDisappearActionCommandList(),
					this
				);
			}
			cocos2d::DictElement *el = nullptr;
			CCDICT_FOREACH(objCommandList, el) {
				auto p = dynamic_cast<agtk::data::ObjectCommandData *>(el->getObject());
				objectAction->getObjCommandList()->addObject(p);
			}
			//追加した実行アクションを実行するため、_lateRemoveフラグを落とす。
			this->setLateRemove(false);
		}
		//他オブジェクトの体力を減らす
		if (disappearSetting->getOtherObjectDecrementHp()) {
			cocos2d::__Array *objectList = cocos2d::__Array::create();
			int decrement = -disappearSetting->getDecrement();//減少量
			switch (disappearSetting->getTargetObjectType()) {
			case agtk::data::ObjectDisappearSettingData::kTargetObjectByType: {//オブジェクトの種類で指定
				if (disappearSetting->getTargetObjectGroup() == -1)
				{//すべてのオブジェクト
// #AGTK-NX
#ifdef USE_SAR_OPTIMIZE_1
					auto objectAll = scene->getObjectAllReference(sceneLayer->getType());
#else
					auto objectAll = scene->getObjectAll(sceneLayer->getType());
#endif
					objectList->addObjectsFromArray(objectAll);
				}
				else
				{
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
						auto object = static_cast<agtk::Object *>(ref);
#else
						auto object = dynamic_cast<agtk::Object *>(ref);
#endif
						if (object->getObjectData()->getGroup() == disappearSetting->getTargetObjectGroup()) {
							objectList->addObject(object);
						}
					}
				}
				break;
			}
			case agtk::data::ObjectDisappearSettingData::kTargetObjectById: {//オブジェクトで指定
				if (disappearSetting->getTargetObjectId() == -3) {//自身以外のオブジェクト
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
						auto object = static_cast<agtk::Object *>(ref);
#else
						auto object = dynamic_cast<agtk::Object *>(ref);
#endif
						if (object != this) {
							objectList->addObject(object);
						}
					}
				}
				else if (disappearSetting->getTargetObjectId() == -2) {//自身のオブジェクト
					objectList->addObject(this);
				}
				else {
// #AGTK-NX
#ifdef USE_SAR_OPTIMIZE_3
					auto objectAll = scene->getObjectAll(disappearSetting->getTargetObjectId(), sceneLayer->getType());
					objectList->addObjectsFromArray(objectAll);
#else
#ifdef USE_SAR_OPTIMIZE_1
					auto objectAll = scene->getObjectAllReference(sceneLayer->getType());
#else
					auto objectAll = scene->getObjectAll(sceneLayer->getType());
#endif
					cocos2d::Ref *ref = nullptr;
					CCARRAY_FOREACH(objectAll, ref) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
						auto object = static_cast<agtk::Object *>(ref);
#else
						auto object = dynamic_cast<agtk::Object *>(ref);
#endif
						if (object->getObjectData()->getId() == disappearSetting->getTargetObjectId()) {
							objectList->addObject(object);
						}
					}
#endif
				}
				break; }
			default:CC_ASSERT(0);
			}
			cocos2d::Vec2 adjust((float)disappearSetting->getCameraRangeAdjust(), (float)disappearSetting->getCameraRangeAdjust());
			cocos2d::Ref *ref = nullptr;
			CCARRAY_FOREACH(objectList, ref) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
				auto object = static_cast<agtk::Object *>(ref);
#else
				auto object = dynamic_cast<agtk::Object *>(ref);
#endif
				bool hpSetFlag = false;
				if (disappearSetting->getDamageRange() == agtk::data::ObjectDisappearSettingData::kRangeWholeScene) {//シーン全体
					hpSetFlag = true;
				}
				else {//カメラ範囲内
					CC_ASSERT(disappearSetting->getDamageRange() == agtk::data::ObjectDisappearSettingData::kRangeInsideCamera);
					bool bWithinCamera = camera->isPositionScreenWithinCamera(
						cocos2d::Rect(
							object->getPosition().x, object->getPosition().y,
							object->getContentSize().width, object->getContentSize().height
						),
						adjust
					);
					if (bWithinCamera) {
						hpSetFlag = true;
					}
				}
				if (hpSetFlag) {
					object->getPlayObjectData()->addHp(decrement);
					object->getPlayObjectData()->adjustData();
				}
			}
		}
		this->setDisappearFlag(true);
		return;
	}

	// 物理パーツを削除
	if (!bDisappearFlag) {
		backupPhysicsGeometry();
	}
	removeAllPhysicsParts();

	scene->removeObjectVariableTimer(this);
	sceneLayer->removeObject(this, bIgnoredReappearCondition, removeOption);

	//単体インスタンスIDに設定されているオブジェクトの場合。
	auto projectPlayData = GameManager::getInstance()->getPlayData();
	int objectId = objectData->getId();
	auto variableData = projectPlayData->getVariableData(objectId, agtk::data::kObjectSystemVariableSingleInstanceID);
	if ((int)variableData->getValue() == this->getInstanceId()) {
		//共通オブジェクトでインスタンスIDが若いオブジェクトIDを設定する。
		auto objectAll = scene->getObjectAll(objectId, sceneLayer->getType());
		cocos2d::Ref *ref = nullptr;
		agtk::Object *object = nullptr;
		CCARRAY_FOREACH(objectAll, ref) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
			auto p = static_cast<agtk::Object *>(ref);
#else
			auto p = dynamic_cast<agtk::Object *>(ref);
#endif
			if (p != this) {
				if (object != nullptr) {
					if (p->getInstanceId() < object->getInstanceId()) {
						object = p;
					}
				}
				else {
					object = p;
				}
			}
		}
		if (object) {
			variableData->setValue((double)object->getInstanceId());
		}
		else {
			//共通オブジェクトが見つからない場合は、0を設定する。
			variableData->setValue(0);
		}
	}

	if (!this->getRemoveLayerMoveFlag()) {
		//シーンに配置された同インスタンス数を再設定。
		auto objectAll = scene->getObjectAll(objectId, sceneLayer->getType());
		scene->setObjectInstanceCount(objectId, objectAll->count());
		scene->updateObjectInstanceCount(objectId);
	}

	auto camera = scene->getCamera();
	camera->resetTargetObject(this);

	//破棄。
	this->finalize(removeOption);
}

void Object::removeCollisionHitAttack(agtk::Object *object)
{
	auto list = this->getCollisionHitAttackList();
	cocos2d::Ref *ref;
	CCARRAY_FOREACH(list, ref) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
		auto p = static_cast<ObjectCollision::ObjectWallIntersectTemp *>(ref);
#else
		auto p = dynamic_cast<ObjectCollision::ObjectWallIntersectTemp *>(ref);
#endif
		if (p->getObject() == object) {
			list->removeObject(p);
			break;
		}
	}
}

void Object::removeCollisionAttackHit(agtk::Object *object)
{
	auto list = this->getCollisionAttackHitList();
	cocos2d::Ref *ref;
	CCARRAY_FOREACH(list, ref) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
		auto p = static_cast<ObjectCollision::ObjectWallIntersectTemp *>(ref);
#else
		auto p = dynamic_cast<ObjectCollision::ObjectWallIntersectTemp *>(ref);
#endif
		if (p->getObject() == object) {
			list->removeObject(p);
			break;
		}
	}
}

void Object::removeCollisionWallWall(agtk::Object *object)
{
	auto list = this->getCollisionWallWallList();
	if (list->containsObject(object)) {
		list->removeObject(object);
	}
}

//

void Object::callbackDetectionWallCollision(CollisionNode* collisionObject)
{
#ifdef USE_COLLISION_MEASURE
	roughWallCollisionCount++;
#endif
	auto collision = this->getObjectWallCollision();
	auto object = dynamic_cast<agtk::Object *>(collisionObject->getNode());

	if (object) {
		CC_ASSERT(object->getLayerId() == this->getLayerId());

		//collision->addObject(object);
		auto list = this->getCollisionWallWallList();
		if (!list->containsObject(object)) {
			list->addObject(object);
		}
	}
}

void Object::updateCollisionWallWallList()
{
	if (_collisionWallWallChecked) {
		return;
	}
	_collisionWallWallChecked = true;
	auto list = this->getCollisionWallWallList();
	list->removeAllObjects();

	//objectが置かれているSceneLayerを求め、使う。
	auto scene = GameManager::getInstance()->getCurrentScene();
	auto sceneLayer = scene->getSceneLayer(this->getLayerId());
	if (!sceneLayer) {
		return;
	}
	//CollisionComponent * component = dynamic_cast<CollisionComponent *>(this->getComponent(componentName));
	CollisionComponent * component = dynamic_cast<CollisionComponent *>(this->getComponent(CollisionComponent::getCollisionComponentName(CollisionComponent::kGroupWall)));
	if (component != nullptr) {
		CollisionNode * collisionObject = component->getCollisionNode();
		if (collisionObject != nullptr) {
			auto objectData = this->getObjectData();
			CollisionDetaction *wallCollisionDetection = nullptr;
			sceneLayer->setWallCollisionObject(this);
			const DetectWallCollisionFunction &func = CC_CALLBACK_1(Object::callbackDetectionWallCollision, this);
			sceneLayer->setDetectWallCollisionFunc(&func);
			for (int group = 0; group < sceneLayer->getGroupWallCollisionDetections()->count(); ++group) {
				wallCollisionDetection = nullptr;
				if (objectData->isCollideWithObjectGroup(group)) {
					wallCollisionDetection = sceneLayer->getGroupWallCollisionDetection(group);
				}
				if (!wallCollisionDetection) {
					continue;
				}
				wallCollisionDetection->scanSingle(collisionObject);
			}
			sceneLayer->setWallCollisionObject(nullptr);
			sceneLayer->setDetectWallCollisionFunc(nullptr);
		}
	}

	auto newList = cocos2d::__Array::create();
	auto object = this;

	std::vector<Vertex4> wallCollisionList;
	object->getTimelineList(agtk::data::TimelineInfoData::kTimelineWall, wallCollisionList);

	auto wallGroup = WallHitInfoGroup::create(this);
#ifdef USE_MULTITHREAD_OBJECT_UPDATE
	AutoDeleter<agtk::WallHitInfoGroup> deleter(wallGroup);
#endif
	wallGroup->addWallHitInfo(wallCollisionList);

	for (unsigned int i = 0; i < wallGroup->getWallHitInfoListCount(); i++) {
		auto wallHitInfo = wallGroup->getWallHitInfo(i);

		getTouchingObjectWallHitInfoAnyway(wallHitInfo, this, list, newList);
	}

	setCollisionWallWallList(newList);
}

void Object::setVisible(bool visible)
{
	float seconds = 0.0f;
	auto objectVisible = this->getObjectVisible();
	objectVisible->start(visible, seconds);
	this->updateVisible(seconds);
}

bool Object::isVisible()
{
	return _innerObjectVisible;
}

void Object::updateVisible(float delta)
{
	bool bDamagedVisible = true;
	cocos2d::Ref *ref;
	CCARRAY_FOREACH(this->getDamagedList(), ref) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
		auto p = static_cast<agtk::ObjectDamaged *>(ref);
#else
		auto p = dynamic_cast<agtk::ObjectDamaged *>(ref);
#endif
		p->update(delta);
		bDamagedVisible &= p->getVisible();
	}
	auto objectVisible = this->getObjectVisible();
	objectVisible->update(delta);

	_innerObjectVisible = objectVisible->getVisible() & bDamagedVisible;

	auto sceneLayer = this->getSceneLayer();
	if (sceneLayer->getType() == agtk::SceneLayer::kTypeMenu) {
		//メニューの場合は直接Nodeに描画有無を設定。
		Node::setVisible(_innerObjectVisible);
	}

	// プレイヤーのみ処理する(点滅)
	auto visible = (_forceVisibleState != ForceVisibleState::kIgnore) ? (bool)_forceVisibleState : (objectVisible->getVisible() & bDamagedVisible);
	this->setPlayerVisible(visible);
}

void Object::setForceVisibleState(ForceVisibleState state)
{
	_forceVisibleState = state;
	float seconds = 0.0f;
	this->updateVisible(seconds);
}

void Object::setWaitDuration300All(int duration300)
{
	this->setWaitDuration300(duration300);
	auto connectObjectList = this->getConnectObjectList();
	if (connectObjectList->count() > 0) {
		cocos2d::Ref *ref;
		CCARRAY_FOREACH(connectObjectList, ref) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
			auto connectObject = static_cast<agtk::Object *>(ref);
#else
			auto connectObject = dynamic_cast<agtk::Object *>(ref);
#endif
			connectObject->setWaitDuration300All(duration300);
		}
	}
}

void Object::setForceVisibleStateAll(agtk::Object::ForceVisibleState state)
{
	this->setForceVisibleState(state);
	//接続オブジェクトがある場合は、親オブジェクトと同様に強制表示状態を無効にする。
	auto connectObjectList = this->getConnectObjectList();
	if (connectObjectList->count() > 0) {
		cocos2d::Ref *ref;
		CCARRAY_FOREACH(connectObjectList, ref) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
			auto connectObject = static_cast<agtk::Object *>(ref);
#else
			auto connectObject = dynamic_cast<agtk::Object *>(ref);
#endif
			connectObject->setForceVisibleStateAll(state);
		}
	}
}

/**
* 被ダメージ処理
* @param	object	攻撃側オブジェクト
*/
void Object::damaged(agtk::Object *object, agtk::data::PlayObjectData *damageRatePlayObjectData)
{
	//無敵モードをチェックする。
	auto objectData = this->getObjectData();
	if (objectData->isGroupPlayer()) {
		//プレイヤーがダメージを受けない状態で進める事ができる。
		auto debugManager = DebugManager::getInstance();
		if (debugManager->getInvincibleModeEnabled()) {
			return;
		}
	}

	bool takeOverDamageRate = true;
	if (!damageRatePlayObjectData) {
		damageRatePlayObjectData = this->getPlayObjectData();
		takeOverDamageRate = false;
	}
	// 親オブジェクトがいて「子オブジェクト時の被ダメージ」が「対象を親オブジェクトに変更」に設定されている場合
	if (this->getOwnParentObject() && objectData->getChildDamageType() == agtk::data::ObjectData::EnumChildDamageType::kChildDamageParent) {
		// 親オブジェクトへダメージを発生させる
		this->getOwnParentObject()->damaged(object, (objectData->getTakeOverDamageRateToParent() ? damageRatePlayObjectData : nullptr));
	}
	// 自身へダメージの場合
	else {

		bool bDefaultDamaged = true;
		auto damagedList = this->getDamagedList();
		cocos2d::Ref *ref;
		CCARRAY_FOREACH(damagedList, ref) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
			auto damaged = static_cast<agtk::ObjectDamaged *>(ref);
#else
			auto damaged = dynamic_cast<agtk::ObjectDamaged *>(ref);
#endif
			if (!damaged->checkIgnored(object)) {
				bDefaultDamaged = false;
			}
		}
		bool bCriticalDamaged = false;
		// デフォルトの被ダメージ処理
		if (bDefaultDamaged) {
			auto attackObjectData = object->getObjectData();
			auto attackPlayObjectData = object->getPlayObjectData();
			auto damagedObjectData = this->getObjectData();
			auto damagedPlayObjectData = this->getPlayObjectData();
			double criticalRate = 0.0;

			// 「攻撃にクリティカルを設定」がONの場合
			if (attackObjectData->getCritical()) {
				auto variableCriticalIncidence = object->getPlayObjectData()->getVariableData(agtk::data::kObjectSystemVariableCriticalIncidence);
				criticalRate = variableCriticalIncidence->getValue();
			}

			// 最小攻撃力から最大攻撃力の間で攻撃力を算出
			auto attackRate = object->getVariableAttackRate();
			auto minAttackValue = (int)attackPlayObjectData->getVariableData(agtk::data::EnumObjectSystemVariable::kObjectSystemVariableMinimumAttack)->getValue();
			auto maxAttackValue = (int)attackPlayObjectData->getVariableData(agtk::data::EnumObjectSystemVariable::kObjectSystemVariableMaximumAttack)->getValue();
// #AGTK-NX
#if (CC_TARGET_PLATFORM == CC_PLATFORM_NX)
#else
			double attack = (minAttackValue + (maxAttackValue - minAttackValue + 1) * rand() / (1 + RAND_MAX)) * attackRate / 100.0;
#endif

			// クリティカルが発生する場合
			if (criticalRate > 0.0 && criticalRate >= AGTK_RANDOM(0.0, 100.0)) {
				auto variableCriticalRatio = object->getPlayObjectData()->getVariableData(agtk::data::kObjectSystemVariableCriticalRatio);
				attack *= variableCriticalRatio->getValue() * 0.01;
				bCriticalDamaged = true;
			}

			// ダメージ算出
			double damage = attack * (getBaseParamDamageRate(damageRatePlayObjectData) * 0.01);
			damagedPlayObjectData->addHp(-damage);
			//被ダメージ値設定。
			damagedPlayObjectData->getVariableData(agtk::data::EnumObjectSystemVariable::kObjectSystemVariableDamageValue)->setValue(damage);
			//クリティカルを受けた。
			if (bCriticalDamaged) {
				auto criticalDamaged = damagedPlayObjectData->getSwitchData(agtk::data::EnumObjectSystemSwitch::kObjectSystemSwitchCriticalDamaged);
				criticalDamaged->setValue(true);
// #AGTK-NX #AGTK-WIN
#ifdef USE_30FPS
				criticalDamaged->requestValue(false, Director::getInstance()->getAnimationInterval() / GameManager::getInstance()->getFrameProgressScale() * 3);
#else
				criticalDamaged->requestValue(false, Director::getInstance()->getAnimationInterval() * 3);
#endif
			}
			//変数・スイッチデータ調整
			damagedPlayObjectData->adjustData();
		}
		else {
			// 追加の「被ダメージ設定」分ダメージ
			cocos2d::Ref *ref;
			CCARRAY_FOREACH(damagedList, ref) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
				auto damaged = static_cast<agtk::ObjectDamaged *>(ref);
#else
				auto damaged = dynamic_cast<agtk::ObjectDamaged *>(ref);
#endif
				double value = 0.0;
				if (damaged->getDamage(object, value, bCriticalDamaged, damageRatePlayObjectData, takeOverDamageRate)) {
					auto playObjectData = this->getPlayObjectData();
					playObjectData->addHp(-value);
					//被ダメージ値設定。
					playObjectData->getVariableData(agtk::data::EnumObjectSystemVariable::kObjectSystemVariableDamageValue)->setValue(value);
					//クリティカルを受けた。
					if (bCriticalDamaged) {
						auto criticalDamaged = playObjectData->getSwitchData(agtk::data::EnumObjectSystemSwitch::kObjectSystemSwitchCriticalDamaged);
						criticalDamaged->setValue(true);
// #AGTK-NX #AGTK-WIN
#ifdef USE_30FPS
						criticalDamaged->requestValue(false, Director::getInstance()->getAnimationInterval() / GameManager::getInstance()->getFrameProgressScale() * 3);
#else
						criticalDamaged->requestValue(false, Director::getInstance()->getAnimationInterval() * 3);
#endif
					}
					playObjectData->adjustData();
					damaged->start(object);//被ダメージ表示開始。
					// ヒットストップの効果時間が設定されているか
					if (damaged->getDamagedSettingData()->getDioEffectDuration() > 0) {
						damaged->dioStart(object);
					}
				}
			}
		}
	}
}

void Object::playAction(int actionId, int moveDirectionId, int forceDirectionId)
{
	auto objectActionOld = this->getCurrentObjectAction();
	objectActionOld->setIgnored(false); //以前のアクションの実行アクションが次に呼び出されたとき無視しないようにする(ACT2-6050)
	auto objectAction = this->setup(actionId, moveDirectionId, forceDirectionId);
	//ジャンプ動作を行う。
	auto objectActionData = objectAction->getObjectActionData();
	if (objectActionData->getJumpable()) {
		this->setJumpAction();
	}
	_bRequestPlayAction = objectActionOld != objectAction ? 2 : 1;//アクション再生リクエストフラグ。
}

void Object::playActionTemplateMove(int actionId, int moveDirectionId)
{
	auto objectActionOld = this->getCurrentObjectAction();
	this->getObjectCollision()->lateUpdateWallHitInfoGroup();//壁判定情報を更新。
	auto objectAction = this->setup(actionId, moveDirectionId);
	//ジャンプ動作を行う。
	auto objectActionData = objectAction->getObjectActionData();
	if (objectActionData->getJumpable() && !_jumping && !_jumpTop) {
		this->setJumpAction();
	}
	_bRequestPlayActionTemplateMove = objectActionOld != objectAction ? 2 : 1;//アクション再生リクエストフラグ。
}

void Object::pauseAction(float seconds)
{
	auto timerPauseAction = this->getTimerPauseAction();
	timerPauseAction->start(seconds, [&]() {
		auto objectAction = this->getCurrentObjectAction();
		objectAction->setIgnored(false);
	});
	auto objectAction = this->getCurrentObjectAction();
	objectAction->setIgnored(true);
}

void Object::pauseAnimation(float seconds)
{
	auto timerPauseAnimation = this->getTimerPauseAnimation();
	timerPauseAnimation->start(seconds, [&]() {
		auto basePlayer = this->getBasePlayer();
		if (!basePlayer) {
			return;
		}
		basePlayer->getCurrentAnimationMotion()->setIgnored(false);
	});
	auto basePlayer = this->getBasePlayer();
	if (!basePlayer) {
		return;
	}
	basePlayer->getCurrentAnimationMotion()->setIgnored(true);
}

bool Object::checkAttackableObject(agtk::Object * targetObj) {
	// 親オブジェクトがいる場合
	if (this->getOwnParentObject()) {
		// 親には攻撃判定が当たらない場合
		if (this->getObjectData()->getUnattackableToParent()) {
			// 攻撃対象が親オブジェクトの場合は攻撃不可である
			if (this->getOwnParentObject()->getId() == targetObj->getId()) {
				return false;
			}
		}
	}
	//破棄するフラグが立っている。
	if (this->getLateRemove()) {
		return false;
	}

	// targetObjに対して攻撃可能である
	return true;
}

/**
* オブジェクトの生成
* @param	commandData	コマンドデータ
* @note	オブジェクトを生成します
*/
void Object::execActionObjectCreate(agtk::data::ObjectCommandData *commandData)
{
	auto cmd = dynamic_cast<agtk::data::ObjectCommandObjectCreateData *>(commandData);
	CC_ASSERT(cmd);

	agtk::ViewportLightSceneLayer *viewportLightSceneLayer = nullptr;
	// ワーク変数
	auto sceneLayer = this->getSceneLayer();
	auto scene = sceneLayer->getScene();
	if (sceneLayer->getType() == agtk::SceneLayer::kTypeScene) {
		viewportLightSceneLayer = scene->getViewportLight()->getViewportLightSceneLayer(this->getLayerId());
	}

	auto objectData = this->getObjectData();
	cocos2d::__Array *objectList = nullptr;

	// 生成位置を算出するメソッド	
	std::function<bool(agtk::Object *, cocos2d::Vec2 &)> calcObjectPosition = [&](agtk::Object *object, cocos2d::Vec2 &pos) {
		int connectId = cmd->getConnectId();
		if (cmd->getUseConnect() && cmd->getCreatePosition() == agtk::data::ObjectCommandObjectCreateData::kPositionCenter && connectId >= 0) {
			//接続点を使用
			agtk::Vertex4 vertex4;
			if (object->getTimeline(agtk::data::TimelineInfoData::kTimelineConnection, connectId, vertex4)) {
				pos = Scene::getPositionSceneFromCocos2d(vertex4.addr()[0], object->getSceneData());
			}
			else {
				return false;
			}
		}
		else {
			//オブジェクトの中心点。
			pos = object->getCenterPosition();
		}
		pos += Object::getSceneLayerScrollDiff(sceneLayer, object->getSceneLayer());
		return true;
	};

	// 生成位置で対象となるオブジェクトリストを作成
	switch (cmd->getCreatePosition())
	{
		// ----------------------------------------
		// このオブジェクトの位置
		// ----------------------------------------
	case agtk::data::ObjectCommandObjectCreateData::kPositionCenter:
	{
		objectList = cocos2d::__Array::create();
		objectList->addObject(this);
		break;
	}
	// ----------------------------------------
	// ロックしたオブジェクトの位置
	// ----------------------------------------
	case agtk::data::ObjectCommandObjectCreateData::kPositionLockObjectCenter:
	{
		// このオブジェクトがロックしているオブジェクトのリストを取得
		objectList = scene->getObjectAllLocked(this->getInstanceId(), sceneLayer->getType());
		break;
	}
	default:CC_ASSERT(0);
	}

	// 対象となるオブジェクトが存在しない場合
	if (nullptr == objectList || objectList->count() <= 0) {
		// 何もしない
		return;
	}

	cocos2d::Ref *ref = nullptr;
	CCARRAY_FOREACH(objectList, ref) {

		// 生成位置対象オブジェクト
		auto targetObject = dynamic_cast<agtk::Object *>(ref);

		//オブジェクト位置算出
		cocos2d::Vec2 pos;
		if (calcObjectPosition(targetObject, pos) == false) {
			//位置を抽出できなかった場合。
			continue;
		}

		//位置を調整
		pos.x += (float)cmd->getAdjustX();
		pos.y += (float)cmd->getAdjustY();

		//生成時のオブジェクトの向きを、このオブジェクトに合わせる。
		int dispDirection = -1;
		if (cmd->getUseRotation()) {
			dispDirection = this->getDispDirection();
		}
#if defined(USE_RUNTIME)
		auto projectData = GameManager::getInstance()->getProjectData();
		if (projectData->getObjectData(cmd->getObjectId())->getTestplayOnly()) {
			continue;
		}
#endif
		//オブジェクト生成
		auto newObject = agtk::Object::create(
			sceneLayer,
			cmd->getObjectId(),
			cmd->getActionId(),//initialActionId
			pos,
			cocos2d::Vec2(1, 1),//scale
			0,//rotation
			dispDirection//向き
			, -1, -1, -1
		);

		//「生成時のオブジェクトの向きを、このオブジェクトに合わせる」での後処理。
		if (cmd->getUseRotation()) {
			//※agtk::Object::createメソッドのdispDirection（向き）に直接値を入れると移動方向まで設定されるため、移動方向をZEROとする。
			newObject->getObjectMovement()->setDirection(cocos2d::Vec2::ZERO);

			if (this->isAutoGeneration()) {//「回転の自動生成」の場合。
				auto player = this->getPlayer();
				if (player != nullptr) {
					auto angle = player->getCenterRotation();
					auto direction = agtk::GetDirectionFromDegrees(angle);
					direction = this->directionCorrection(direction);
					if (newObject->isAutoGeneration()) {
						newObject->getObjectMovement()->setDirection(direction);
						auto newPlayer = newObject->getPlayer();
						if (newPlayer != nullptr) {
							newPlayer->setCenterRotation(angle);
						}
					}
				}
			}
		}

		//オブジェクトをレイヤーに追加。
#ifdef USE_COLLISION_OPTIMIZATION
#else
#if defined(USE_WALL_COLLISION)	// 2017/04/17 agusa-k: 壁判定用
		auto wallCollision = scene->getWallCollisionDetection();
		if (_object->getSceneLayer()->getType() == agtk::SceneLayer::kTypeMenu) {
			wallCollision = scene->getWallCollisionDetectionMenu();
		}
		newObject->addComponent(CollisionComponent::create(wallCollision, 1));
#endif
#endif
		sceneLayer->addCollisionDetaction(newObject);
		newObject->setId(sceneLayer->publishObjectId());
		newObject->getPlayObjectData()->setInstanceId(scene->getObjectInstanceId(newObject));
		newObject->getPlayObjectData()->setInstanceCount(scene->incrementObjectInstanceCount(newObject->getObjectData()->getId()));
		scene->updateObjectInstanceCount(newObject->getObjectData()->getId());
		newObject->setLayerId(this->getLayerId());
		newObject->setPhysicsBitMask(this->getLayerId(), scene->getSceneData()->getId());
		newObject->setSceneIdOfFirstCreated(this->getSceneIdOfFirstCreated());
		auto newObjectData = newObject->getObjectData();
		if (viewportLightSceneLayer && newObjectData->getViewportLightSettingFlag() && newObjectData->getViewportLightSettingList()->count() > 0) {
			auto viewportLightObject = ViewportLightObject::create(newObject, scene->getViewportLight(), sceneLayer);
			viewportLightSceneLayer->getViewportLightObjectList()->addObject(viewportLightObject);
		}
		sceneLayer->addObject(newObject);

		// オブジェクトに紐付いた物理オブジェクトを生成
		sceneLayer->createPhysicsObjectWithObject(newObject);

		//子オブジェクトとして生成
		if (cmd->getChildObject()) {
			cocos2d::Vec2 pos;
			auto connectId = -1;
			switch (cmd->getCreatePosition()) {
			case agtk::data::ObjectCommandObjectCreateData::kPositionCenter: {
				if (cmd->getUseConnect()) {
					connectId = cmd->getConnectId();
				}
				pos = cocos2d::Vec2(cmd->getAdjustX(), cmd->getAdjustY());
				break; }
			case agtk::data::ObjectCommandObjectCreateData::kPositionLockObjectCenter: {
				pos = newObject->getPosition() - this->getPosition();
				break; }
			default:CC_ASSERT(0);
			}
			// 親オブジェクトのインスタンスIDを保存
			newObject->getPlayObjectData()->setParentObjectInstanceId(this->getInstanceId());
			this->addChildObject(newObject, pos, connectId);

			// ヒットストップ中に生成された場合、現在のヒットストップ情報を子にセット
			if (this->getDioExecuting() && this->getDioChild()) {
				newObject->setDioExecuting(this->getDioExecuting());
				newObject->setDioFrame(this->getDioFrame());
				newObject->setDioGameSpeed(this->getDioGameSpeed());
				newObject->setDioEffectDuration(this->getDioEffectDuration());
				newObject->setDioParent(this->getDioParent());
				newObject->setDioChild(this->getDioChild());
			}
		}
		//このオブジェクトより表示の優先度を下げる。※接続点の表示位置より優先されます
		if (cmd->getLowerPriority()) {
			this->addConnectObjectDispPriority(newObject, cmd->getLowerPriority());
		}
		//生成したオブジェクトがグリッドに吸着する
		if (cmd->getGridMagnet()) {
			newObject->setNeedAbsorbToTileCorners(true);
		}
	}
}

/**
 * 自身に付与されているパーティクルの削除
 */
void Object::removeParticles(int targetParticleId)
{
	ParticleManager::getInstance()->removeParticlesOfFollowed(this, targetParticleId);
}

/**
* 自身に付与されているパーティクルの発生を停止する
*/
void Object::stopEmitteParticles(int targetParticleId, bool isReset)
{
	ParticleManager::getInstance()->stopEmitteParticlesOfFollowed(this, targetParticleId, isReset);
}

// =========================================
// Object 物理演算系
/**
* 物理空間用ボディのセットアップ
* @param	isInit	初期化セットアップか？
*/
void Object::setupPhysicsBody(bool isInit)
{
	// 物理演算に関する設定がOFF の場合
	if (!this->getObjectData()->getPhysicsSettingFlag()) {
		// なにもしない
		return;
	}

	// プレイヤーを取得
	auto player = this->getPlayer();

#ifdef FIX_ACT2_4879
#else
	if (nullptr == player) {
		CCASSERT(player, "Object without Player cannot attach Physics Body.");
		return;
	}
#endif

	// 壁判定のノードを取得
	std::vector<Vertex4> wallCollisionList;
	this->getTimelineList(agtk::data::TimelineInfoData::kTimelineWall, wallCollisionList);

	// 物理演算の基本設定データ取得
	auto playObjectData = this->getPlayObjectData();
	auto isAffect = playObjectData->getSwitchData(agtk::data::kObjectSystemSwitchAffectOtherObjects)->getValue();
	auto isAffected = playObjectData->getSwitchData(agtk::data::kObjectSystemSwitchAffectedByOtherObjects)->getValue();
	auto isfollowConnectedPhysics = playObjectData->getSwitchData(agtk::data::kObjectSystemSwitchFollowConnectedPhysics)->getValue();
	auto physicsSetting = this->getObjectData()->getPhysicsSetting();
	
	// 壁判定の矩形とプレイヤーの原点とサイズから物理オブジェクトの配置オフセットを返す
	auto getPhysicsCenterOffset = [](const Rect &rect, agtk::Player *player) -> Vec2 {
		auto pos = player->getPosition();
		auto rectCenter = rect.origin + (rect.size * 0.5f);
		return (rectCenter - pos);
	};

	// 初期設定の場合
	if (isInit) {
		// -------------------------------------------------------------------------
		// 物理ボディを生成(シェイプ無し)
		auto physicsBody = cocos2d::PhysicsBody::create();

		// -------------------------------------------------------------------------
		// 物理マテリアルに密度・摩擦係数・反発係数を設定して生成
		auto material = PHYSICSBODY_MATERIAL_DEFAULT;

		// 物理演算の影響を受ける場合
		if (isAffected) {
			material.density = physicsSetting->getDensity();
			material.friction = physicsSetting->getFriction();
			material.restitution = physicsSetting->getRepulsion();
			physicsBody->setMass(physicsSetting->getMass());
		}
		else {
			material.density = isAffect ? 1000.0f : PHYSICS_INFINITY;
			material.friction = 1.0f;
			material.restitution = 0.5f;
			physicsBody->setMass(isAffect ? INT_MAX : PHYSICS_INFINITY);
		}

		this->setPhysicsMaterial(material);

		// -------------------------------------------------------------------------
		// 物理シェイプを生成
		// 接続された物理オブジェクトの動作を優先がONの場合
		if (isfollowConnectedPhysics) {
#ifdef FIX_ACT2_4879
			if (player) {
				auto rect = Rect(player->getPosition().x, player->getPosition().y, 1, 1);
				auto offset = cocos2d::Vec2::ZERO;
				auto shape = cocos2d::PhysicsShapeBox::create(rect.size, material, offset);
				physicsBody->addShape(shape, false);
			}
			else {
// #AGTK-NX
#if (CC_TARGET_PLATFORM == CC_PLATFORM_NX)
#else
				auto rect = cocos2d::Rect::Rect(0, 0, 1, 1);
#endif
				auto offset = cocos2d::Vec2::ZERO;
				auto shape = cocos2d::PhysicsShapeBox::create(rect.size, material, offset);
				physicsBody->addShape(shape, false);

			}
#else
			auto rect = Rect(player->getPosition().x, player->getPosition().y, 1, 1);
			auto offset = getPhysicsCenterOffset(rect, player);
			auto shape = cocos2d::PhysicsShapeBox::create(rect.size, material, offset);
			physicsBody->addShape(shape, false);
#endif
			physicsBody->setMass(0.01f);
		}
		else {
#ifdef FIX_ACT2_4879
			if (player) {
				// 壁判定が存在する場合
				if (wallCollisionList.size() > 0) {
					// 壁判定の数だけ物理シェイプを生成
					for (auto wallInfo : wallCollisionList) {
						auto rect = wallInfo.getRect();
						auto offset = getPhysicsCenterOffset(rect, player);
						auto shape = cocos2d::PhysicsShapeBox::create(rect.size, material, offset);
						physicsBody->addShape(shape, false);
					}
				}
			}
			/*else {
				auto rect = cocos2d::Rect::Rect(0, 0, 1, 1);
				auto offset = cocos2d::Vec2::ZERO;
				auto shape = cocos2d::PhysicsShapeBox::create(rect.size, material, offset);
				physicsBody->addShape(shape, false);

			}*/
#else
			// 壁判定が存在する場合
			if (wallCollisionList.size() > 0) {
				// 壁判定の数だけ物理シェイプを生成
				for (auto wallInfo : wallCollisionList) {
					auto rect = wallInfo.getRect();
					auto offset = getPhysicsCenterOffset(rect, player);
					auto shape = cocos2d::PhysicsShapeBox::create(rect.size, material, offset);
					physicsBody->addShape(shape, false);
				}
			}
#endif
		}

		// -------------------------------------------------------------------------
		// 物理ボディの各種設定
		physicsBody->setDynamic(true);
		physicsBody->setGravityEnable(isfollowConnectedPhysics);
		physicsBody->setRotationEnable(isfollowConnectedPhysics);
		physicsBody->setGroup((!isAffect && !isAffected) || isfollowConnectedPhysics ? GameManager::EnumPhysicsGroup::kNoneObject : GameManager::EnumPhysicsGroup::kObject);
		physicsBody->setCollisionBitmask(0);
		physicsBody->setCategoryBitmask(0);
		physicsBody->setContactTestBitmask(INT_MAX);

		// -------------------------------------------------------------------------
		// 物理ボディの非衝突グループ設定
		int bit = 0;
		cocos2d::Ref * ref = nullptr;
		CCARRAY_FOREACH(physicsSetting->getNonCollisionGroup(), ref) {
			auto num = dynamic_cast<cocos2d::Integer *>(ref)->getValue();

			bit |= (1 << num);
		}
		physicsBody->setTag(bit);

		// -------------------------------------------------------------------------
		// 物理ボディを持つノードを生成
		auto physicsNode = Node::create();
		physicsNode->setPhysicsBody(physicsBody);
		physicsNode->unscheduleUpdate();
#ifdef FIX_ACT2_4879
		if (player) {
			physicsNode->setPosition(player->getPosition());
			physicsNode->setContentSize(player->getContentSize());
		}
		else {
			auto pos = Scene::getPositionCocos2dFromScene(_objectPosition, this->getSceneData());
			physicsNode->setPosition(pos);
			physicsNode->setContentSize(cocos2d::Size::ZERO);
		}
#else
		physicsNode->setPosition(player->getPosition());
		physicsNode->setContentSize(player->getContentSize());
#endif

		// オブジェクトに参照を保持
		this->setphysicsNode(physicsNode);
		this->addChild(physicsNode, physicsNode->getLocalZOrder(), "physicsNode");

		// シーンパーツIDにデフォルト値を設定
		this->setScenePartsId(DEFAULT_SCENE_PARTS_ID);
	}
	else {
		// 物理用ノード取得
		auto physicsNode = this->getphysicsNode();

		// 物理用ノードが存在しない場合
		if (nullptr == physicsNode) {
			// 何もしない
			return;
		}

		// -------------------------------------------------------------------------
		// 接続物理オブジェクト優先でない場合
		if (!isfollowConnectedPhysics) {
			
			if (!isAffected) {
				// 物理用ノードをプレイヤーに追従させる
#ifdef FIX_ACT2_4879
				if (player) {
					physicsNode->setPosition(player->getPosition());
					physicsNode->setRotation(player->getRotation());
				}
				else {
					auto pos = Scene::getPositionCocos2dFromScene(_objectPosition, this->getSceneData());
					physicsNode->setPosition(pos);
					physicsNode->setRotation(0.0f);
				}
#else
				physicsNode->setPosition(player->getPosition());
				physicsNode->setRotation(player->getRotation());
#endif
			}
			else {
				// 物理用ノードに衝突による押し戻しを反映する
				physicsNode->setPosition(physicsNode->getPosition() + this->getCollisionPushBackVec());
				this->setCollisionPushBackVec(Vec2::ZERO);

				float x = std::roundf((physicsNode->getPosition().x) * 10000.0f) * 0.0001f;
				float y = std::roundf((physicsNode->getPosition().y) * 10000.0f) * 0.0001f;

#ifdef FIX_ACT2_4879
				if (player) {
					player->setPosition(x, y);
				}
				else {
					setPosition(Scene::getPositionSceneFromCocos2d(cocos2d::Vec2(x, y), this->getSceneData()));
				}
#else
				player->setPosition(x, y);
#endif

				// プレイヤータイプでない または 表示方向を操作で変更しない場合
				if (!getObjectData()->isGroupPlayer() || !getObjectData()->getMoveDispDirectionSettingFlag()) {
#ifdef FIX_ACT2_4879
					if (player) {
						player->setCenterRotation(physicsNode->getRotation());
					}
#else
					player->setCenterRotation(physicsNode->getRotation());
#endif
				}

				this->setOldPosition(this->getPosition());

			}

			// -------------------------------------------------------------------------
			// 物理ボディと各種衝突設定値と物理マテリアルを取得
			auto physicsBody = physicsNode->getPhysicsBody();
			auto tag = physicsBody->getTag();
			auto categoryBitmask = physicsBody->getCategoryBitmask();
			auto collisionBitmask = physicsBody->getCollisionBitmask();
			auto mass = physicsBody->getMass();
			auto material = this->getPhysicsMaterial();

			// -------------------------------------------------------------------------
			// 新しく壁判定に合わせたシェイプを生成

			// 一旦物理シェイプを削除
			physicsBody->removeAllShapes();

#ifdef FIX_ACT2_4879
			if (player) {
				// 壁判定が存在する場合
				if (wallCollisionList.size() > 0) {
					// 壁判定の数だけ物理シェイプを生成
					for (auto wallInfo : wallCollisionList) {
						auto rect = wallInfo.getRect();
						auto offset = getPhysicsCenterOffset(rect, player);
						auto shape = cocos2d::PhysicsShapeBox::create(rect.size, material, offset);
						physicsBody->addShape(shape, false);
					}
				}
			}
			/*else {
				auto rect = cocos2d::Rect::Rect(0, 0, 8, 8);
				auto offset = cocos2d::Vec2::ZERO;
				auto shape = cocos2d::PhysicsShapeBox::create(rect.size, material, offset);
				physicsBody->addShape(shape, false);

			}*/
#else
			// 壁判定が存在する場合
			if (wallCollisionList.size() > 0) {
				// 壁判定の数だけ物理シェイプを生成
				for (auto wallInfo : wallCollisionList) {
					auto rect = wallInfo.getRect();
					auto offset = getPhysicsCenterOffset(rect, player);
					auto shape = cocos2d::PhysicsShapeBox::create(rect.size, material, offset);
					physicsBody->addShape(shape, false);
				}
			}
#endif

			// 質量を再設定
			physicsBody->setMass(mass);

			// シェイプを交換したので衝突判定用ビットマスクを再設定
			physicsBody->setGroup((!isAffect && !isAffected) || isfollowConnectedPhysics ? GameManager::EnumPhysicsGroup::kNoneObject : GameManager::EnumPhysicsGroup::kObject);
			physicsBody->setContactTestBitmask(INT_MAX);
			physicsBody->setCategoryBitmask(categoryBitmask);
			physicsBody->setCollisionBitmask(collisionBitmask);
			physicsBody->setTag(tag);

			if (!isAffected) {
				physicsBody->resetForces();
			}
		}
	}
}

/**
* 物理用ビットマスクの設定
* @param	ビットマスク
*/
void Object::setPhyiscBitMask(int bitmask)
{
	// 物理用ノード取得
	auto physicsNode = this->getphysicsNode();

	// 物理用ノードが無い場合
	if (!physicsNode) {
		// スキップ
		return;
	}

	// 物理設定取得
	auto physicsSetting = this->getObjectData()->getPhysicsSetting();
	
	// 「他の物理オブジェクトに影響を与える」 または 「物理影響を受ける」または「接続された物理オブジェクトの動作を優先」でない場合
	//if ((physicsSetting->getAffectPhysics() || physicsSetting->getPhysicsAffected()) && !physicsSetting->getFollowConnectedPhysics()) {
		// 物理用のビットマスクを設定
		auto body = physicsNode->getPhysicsBody();
		body->setCategoryBitmask(bitmask);
		body->setCollisionBitmask(bitmask);
	//}
}

/**
 * 物理用ビットマスクの設定（メニューレイヤーを考慮）
 */
void Object::setPhysicsBitMask(int layerId, int sceneId)
{
	auto physicsLayer = layerId - 1;
	if (sceneId == agtk::data::SceneData::kMenuSceneId) {
		physicsLayer = physicsLayer + agtk::PhysicsBase::kMenuPhysicsLayerShift;
	}
	auto bitmask = (1 << physicsLayer);
	setPhyiscBitMask(bitmask);
}

void Object::restorePhysicsGeometry()
{
	if (_physicsPartsList->count() > 0) {
		cocos2d::Ref *ref = nullptr;
		CCARRAY_FOREACH(_physicsPartsList, ref) {
			auto physicsDisk = dynamic_cast<agtk::PhysicsDisk *>(ref);
			auto physicsRectangle = dynamic_cast<agtk::PhysicsRectangle *>(ref);
			auto physicsRopeParts = dynamic_cast<agtk::PhysicsRopeParts *>(ref);
			if (physicsDisk) {
				auto it = _physicsGeometryMap.find(physicsDisk->getScenePartsId());
				if(it != _physicsGeometryMap.end()){
					auto physicsGeometry = &it->second;
					physicsDisk->setPosition(physicsGeometry->_x, physicsGeometry->_y);
					physicsDisk->setRotation(physicsGeometry->_rotation);
				}
			}
			else if (physicsRectangle) {
				auto it = _physicsGeometryMap.find(physicsRectangle->getScenePartsId());
				if (it != _physicsGeometryMap.end()) {
					auto physicsGeometry = &it->second;
					physicsRectangle->setPosition(physicsGeometry->_x, physicsGeometry->_y);
					physicsRectangle->setRotation(physicsGeometry->_rotation);
				}
			}
			else if (physicsRopeParts) {
				auto it = _physicsRopePartsGeometryMap.find(physicsRopeParts->getScenePartsId());
				if (it != _physicsRopePartsGeometryMap.end()) {
					auto &map = it->second;
					auto it2 = map.find(physicsRopeParts->getIdx());
					if (it2 != map.end()) {
						auto physicsGeometry = &it2->second;
						physicsRopeParts->setPosition(physicsGeometry->_x, physicsGeometry->_y);
						physicsRopeParts->setRotation(physicsGeometry->_rotation);
					}
				}
			}
			else {
				//cocos2d::log("skip: %p", ref);
			}
		}
	}
	_physicsGeometryMap.clear();
	_physicsRopePartsGeometryMap.clear();
}

void Object::backupPhysicsGeometry()
{
	_physicsGeometryMap.clear();
	_physicsRopePartsGeometryMap.clear();
	if (_physicsPartsList->count() > 0) {
		cocos2d::Ref *ref = nullptr;
		CCARRAY_FOREACH(_physicsPartsList, ref) {
			auto physicsDisk = dynamic_cast<agtk::PhysicsDisk *>(ref);
			auto physicsRectangle = dynamic_cast<agtk::PhysicsRectangle *>(ref);
			auto physicsRopeParts = dynamic_cast<agtk::PhysicsRopeParts *>(ref);
			if (physicsDisk) {
				auto pos = physicsDisk->getPosition();
				_physicsGeometryMap.insert(std::make_pair(physicsDisk->getScenePartsId(), PhysicsGeometry(pos.x, pos.y, physicsDisk->getRotation())));
			}
			else if (physicsRectangle) {
				auto pos = physicsRectangle->getPosition();
				_physicsGeometryMap.insert(std::make_pair(physicsRectangle->getScenePartsId(), PhysicsGeometry(pos.x, pos.y, physicsRectangle->getRotation())));
			}
			else if (physicsRopeParts) {
				auto pos = physicsRopeParts->getPosition();
				auto id = physicsRopeParts->getScenePartsId();
				if (_physicsRopePartsGeometryMap.find(id) == _physicsRopePartsGeometryMap.end()) {
					_physicsRopePartsGeometryMap.insert(std::make_pair(id, std::map<int, PhysicsGeometry>()));
				}
				auto &map = _physicsRopePartsGeometryMap[id];
				pos = physicsRopeParts->getPosition();
				map.insert(std::make_pair(physicsRopeParts->getIdx(), PhysicsGeometry(pos.x, pos.y, physicsRopeParts->getRotation())));

			}
			else {
				//cocos2d::log("skip: %p", ref);
			}
		}
	}
}

/**
* 自身に付随する物理パーツの全削除
*/
void Object::removeAllPhysicsParts()
{
	auto sceneLayer = this->getSceneLayer();

	// 物理パーツをシーンレイヤーから削除
	if (_physicsPartsList->count() > 0) {
		cocos2d::Ref *ref = nullptr;
		CCARRAY_FOREACH(_physicsPartsList, ref) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
			auto node = static_cast<cocos2d::Node *>(ref);
#else
			auto node = dynamic_cast<cocos2d::Node *>(ref);
#endif
			sceneLayer->getPhysicsObjectList()->removeObject(node);
			node->removeFromParent();
		}
		// リストから全削除
		_physicsPartsList->removeAllObjects();
	}

#ifdef USE_REDUCE_RENDER_TEXTURE
	if (_frontPhysicsNode) {
		_frontPhysicsNode->removeFromParent();
		setFrontPhysicsNode(nullptr);
	}
	if (_backPhysicsNode) {
		_backPhysicsNode->removeFromParent();
		setBackPhysicsNode(nullptr);
	}
#else
	_drawBackPhysicsPartsList->removeAllObjects();
	_drawFrontPhysicsPartsList->removeAllObjects();
#endif

	auto physicsNode = this->getphysicsNode();
	if (physicsNode) {
		physicsNode->removeFromParent();
		this->setphysicsNode(nullptr);
	}
}

// =========================================
// Object 重なり演出関係
void Object::setUpArroundCharacterView()
{
	_effectTargetType = 0;
	_isDrawShilhouette = false;
	_isTransparent = false;

	auto objectData = this->getObjectData();

	// プレイヤーがあり、重なり演出がONの場合
	if(objectData->getAroundCharacterViewSettingFlag()) {

		// 重なり演出設定データ取得
		auto settingData = objectData->getAroundCharacterViewSetting();

		// ビュータイプ取得
		auto viewType = settingData->getViewType();

		// 表示条件取得
		unsigned int tileOn = settingData->getTileOn() ? OVERLAP_EFFECT_TARGET_TILE : 0;
		unsigned int objOn = settingData->getObjectOn() ? OVERLAP_EFFECT_TARGET_OBJ : 0;
		_effectTargetType = tileOn | objOn;

		// 表示条件がどちらもONで無い場合
		if (_effectTargetType == 0) {
			// スキップ
			return;
		}

		// 周辺透過の場合
		if (viewType != agtk::data::ObjectAroundCharacterViewSettingData::EnumViewType::kDrawSOLID) {
			// 透過範囲のサイズ
			auto size = Size(settingData->getWidth(), settingData->getHeight());

			// 範囲描画用ノード
			auto drawNode = DrawNode::create();
			auto color = Color4F(0, 0, 0, 1.0f - (float)settingData->getTransparency() / 100.0f);

			// 矩形の場合
			if (viewType == agtk::data::ObjectAroundCharacterViewSettingData::EnumViewType::kArroundTransparentRect) {
				// 矩形のアンカーポイントが中心になるよう描画
				drawNode->drawSolidRect(size * -0.5f, size * 0.5f, color);
			}
			// 円形の場合
			else {
				Point circlePoints[360];
				int pointMax = 360;
				auto radius = size * 0.5f;
				for (int i = 0; i < pointMax; i++) {
					auto x = this->getScaleX() * radius.width * cos(CC_DEGREES_TO_RADIANS(i));
					auto y = this->getScaleY() * radius.height * sin(CC_DEGREES_TO_RADIANS(i));
					circlePoints[i] = Point(x, y);
				}
				drawNode->drawPolygon(circlePoints, pointMax, color, 0, color);
			}

			drawNode->setVisible(false);
			drawNode->setPosition(Vec2::ZERO);
			drawNode->setBlendFunc({ GL_ZERO, GL_SRC_ALPHA });
			this->setTransparentMaskNode(drawNode);

			_isTransparent = true;
		}
		// ベタ塗りの場合
		else {
			// オブジェクトシルエット生成
			auto silhouette = agtk::ObjectSilhouetteImage::create(this);
			this->setSilhouetteNode(silhouette);

			_isDrawShilhouette = true;
		}
	}
}

/**
* 透過用マスクノードの描画
* @param	renderer		レンダラー
* @param	viewMatrix		ビュー行列
* @param	targetType		演出対象タイプ(1:タイルが重なった / 2:オブジェクトが重なった / 3:両方)
*/
void Object::drawTransparentMaskNode(Renderer *renderer, cocos2d::Mat4 viewMatrix, int targetType)
{
	// 重なり演出(透過)が ON かつ プレイヤーが存在し、表示中でカメラの範囲内かつ、指定の演出タイプの場合
	if (_transparentMaskNode && this->getPlayer() && _visible && isVisitableByVisitingCamera() && (_effectTargetType & targetType)) {
		if (_overlapFlag & targetType){
			// オブジェクトの中心座標に描画
			auto pos = agtk::Scene::getPositionCocos2dFromScene(this->getPosition()) + this->getPlayer()->getBasePlayer()->getOrigin() + Vec2(getContentSize().width, -getContentSize().height) * 0.5f;
			_transparentMaskNode->setPosition(pos);
			_transparentMaskNode->setVisible(true);
			_transparentMaskNode->visit(renderer, viewMatrix, false);
			_transparentMaskNode->setVisible(false);
		}
	}
}

/**
* シルエット(ベタ塗り)の描画
* @param	renderer		レンダラー
* @param	viewMatrix		ビュー行列
* @param	targetType		演出対象タイプ(1:タイルが重なった / 2:オブジェクトが重なった / 3:両方)
*/
void Object::drawSilhouette(Renderer *renderer, cocos2d::Mat4 viewMatrix, int targetType)
{
	// シルエット描画　かつ　シルエットがある　かつ　表示中　かつ　カメラ範囲内かつ、指定の演出タイプの場合
	if (_isDrawShilhouette && _silhouetteNode && _visible && isVisitableByVisitingCamera() && (_effectTargetType & targetType)) {
		if (_overlapFlag & targetType) {
			// シルエット描画
			_silhouetteNode->setVisible(true);
			_silhouetteNode->visit(renderer, viewMatrix);
			_silhouetteNode->setVisible(false);
		}
	}
}

#ifdef USE_REDUCE_RENDER_TEXTURE
/**
 * @brief targetTypeの透過用マスクノードの描画またはシルエット描画が必要なとき真を返す。
 */
bool Object::isTransparentMaskSilhouette(int targetType)
{
	if(_visible && isVisitableByVisitingCamera() && (_effectTargetType & targetType) && (_overlapFlag & targetType)){
		if (_transparentMaskNode && this->getPlayer()) {
			return true;
		}
		if (_isDrawShilhouette && _silhouetteNode) {
			return true;
		}
	}
	return false;
}
#endif

/**
* タイルの角に吸着する
*/
void Object::absorbToTileCorners()
{
	// 衝突判定用データ
	CollisionCorner rect;

	// タイルサイズを取得
	auto tileSize = GameManager::getInstance()->getProjectData()->getTileSize();
	auto sceneHeight = GameManager::getInstance()->getProjectData()->getScreenHeight() * this->getSceneData()->getVertScreenCount();

	// 自身の壁判定の矩形を取得
	cocos2d::Node *node = this->getAreaNode(agtk::data::TimelineInfoData::kTimelineWall);
	
	// 壁判定が無い場合
	if (node == nullptr || node->getContentSize().equals(Size::ZERO)) {
		// 自身の当たり判定の矩形を取得
		node = this->getAreaNode(agtk::data::TimelineInfoData::kTimelineHit);

		// 当たり判定が無い場合
		if (node == nullptr || node->getContentSize().equals(Size::ZERO)) {
			// 吸着処理は行わない
			return;
		}
	}

	// getAreaNode()ではアニメーションフレームのオフセット値が考慮されないので.
	// ここでアニメーションフレームで設定されているオフセット値を取得する
	auto basePlayer = this->getBasePlayer();
	auto offset = Vec2::ZERO;
	if (basePlayer) {
		auto motion = basePlayer->getCurrentAnimationMotion();
		auto currentDirection = motion->getCurrentDirection();
		auto frameNo = motion->getFrameDataNo();;
		offset = currentDirection->getAnimationFrame(frameNo)->getFrameData()->getOffset();

		// 取得したオフセット値を判定用ノードの座標に反映
		auto nodePos = node->getPosition();
		node->setPosition(nodePos.x + offset.x, nodePos.y - offset.y);
	}

	// 壁判定または当たり判定の四方の角の座標を取得
	rect = CollisionUtils::getCorner(node);
	auto cornerPoints = rect.points();

	//cornerPointsの要素の内5つ目に当たる center は利用しないので除外する
	cornerPoints.pop_back();
	
	// 右上、左上、右下、左下の順にタイルの角への最短距離を求める
	auto moveToVec = Vec2::ZERO;
	auto distance = INT_MAX;
	for (auto point : cornerPoints) {
		auto px = point->x;
		auto py = sceneHeight - point->y;
		auto dx = (int)px % (int)tileSize.width;
		auto dy = (int)py % (int)tileSize.height;
		auto targetX = (px - dx) + (abs(dx) >= tileSize.width * 0.5f ? (dx < 0 ? -tileSize.width : tileSize.width) : 0);
		auto targetY = (py - dy) + (abs(dy) >= tileSize.height * 0.5f ? (dy < 0 ? -tileSize.height : tileSize.height) : 0);
		auto vec = cocos2d::Vec2(targetX - px, targetY - py);
		auto lengthSq = vec.getLengthSq();

		if (distance > lengthSq) {
			distance = lengthSq;
			moveToVec = vec;
		}
	}

	// 吸着させる
	auto pos = this->getPosition();
	auto newPos = Vec2(pos.x + moveToVec.x, pos.y + moveToVec.y);
	this->setPosition(newPos);
	this->setOldPosition(newPos);
	auto playObjectData = getPlayObjectData();
	playObjectData->getVariableData(agtk::data::kObjectSystemVariableX)->setExternalValue(newPos.x);
	playObjectData->getVariableData(agtk::data::kObjectSystemVariableY)->setExternalValue(newPos.y);
}

// オブジェクトが再生したサウンドをリストに追加
void Object::addBgmList(AudioManager::AudioInfo* audioInfo)
{
	auto bgmList = this->getBgmList();
	int bgmId = audioInfo->getId();
	if (bgmList->objectForKey(bgmId)) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
		auto bgmAudioList = static_cast<cocos2d::__Dictionary *>(bgmList->objectForKey(bgmId));
#else
		auto bgmAudioList = dynamic_cast<cocos2d::__Dictionary *>(bgmList->objectForKey(bgmId));
#endif
		CC_ASSERT(bgmAudioList);
		bgmAudioList->setObject(audioInfo, audioInfo->getAudioId());
	}
	else {
		auto bgmAudioList = cocos2d::__Dictionary::create();
		bgmAudioList->setObject(audioInfo, audioInfo->getAudioId());
		bgmList->setObject(bgmAudioList, bgmId);
	}

	// 終了コールバックをセット
	audioInfo->setPlayEndCallbackObject(this);
	audioInfo->setPlayEndCallback([&](AudioManager::AudioInfo *_audioInfo) {
		auto bgmList = _audioInfo->getPlayEndCallbackObject()->getBgmList();
		int bgmId = _audioInfo->getId();
		if (bgmList->objectForKey(bgmId)) {
			bgmList->removeObjectForKey(bgmId);
		}
		_audioInfo->setPlayEndCallback(nullptr);
		_audioInfo->setPlayEndCallbackObject(nullptr);
	});
}

void Object::addSeList(AudioManager::AudioInfo* audioInfo)
{
	auto seList = this->getSeList();
	int seId = audioInfo->getId();
	cocos2d::__Dictionary *seAudioList = nullptr;
	if (seList->objectForKey(seId)) {
		// 同じSEを再生した場合は、前のSEを停止しているのでリストから消しておく
		seAudioList = dynamic_cast<cocos2d::__Dictionary *>(seList->objectForKey(seId));
		AudioManager::AudioInfo * audioInfo;
		cocos2d::DictElement *el2;
		CCDICT_FOREACH(seAudioList, el2) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
			audioInfo = static_cast<AudioManager::AudioInfo *>(el2->getObject());
#else
			audioInfo = dynamic_cast<AudioManager::AudioInfo *>(el2->getObject());
#endif
			audioInfo->setPlayEndCallback(nullptr);
			audioInfo->setPlayEndCallbackObject(nullptr);
		}
		CC_ASSERT(seAudioList);
		seAudioList->removeAllObjects();
	}
	else {
		seAudioList = cocos2d::__Dictionary::create();
		CC_ASSERT(seAudioList);
		seList->setObject(seAudioList, seId);
	}
	seAudioList->setObject(audioInfo, audioInfo->getAudioId());

	// 終了コールバックをセット
	audioInfo->setPlayEndCallbackObject(this);
	audioInfo->setPlayEndCallback([&](AudioManager::AudioInfo *_audioInfo) {
		auto seList = _audioInfo->getPlayEndCallbackObject()->getSeList();
		int seId = _audioInfo->getId();
		if (seList->objectForKey(seId)) {
			seList->removeObjectForKey(seId);
		}
		_audioInfo->setPlayEndCallback(nullptr);
		_audioInfo->setPlayEndCallbackObject(nullptr);
	});
}

void Object::addVoiceList(AudioManager::AudioInfo* audioInfo)
{
	auto voiceList = this->getVoiceList();
	int voiceId = audioInfo->getId();
	cocos2d::__Dictionary *voiceAudioList = nullptr;
	if (voiceList->objectForKey(voiceId)) {
		voiceAudioList = dynamic_cast<cocos2d::__Dictionary *>(voiceList->objectForKey(voiceId));
		CC_ASSERT(voiceAudioList);
	}
	else {
		voiceAudioList = cocos2d::__Dictionary::create();
		CC_ASSERT(voiceAudioList);
		voiceList->setObject(voiceAudioList, voiceId);
	}
	voiceAudioList->setObject(audioInfo, audioInfo->getAudioId());

	// 終了コールバックをセット
	audioInfo->setPlayEndCallbackObject(this);
	audioInfo->setPlayEndCallback([&](AudioManager::AudioInfo *_audioInfo) {
		auto voiceList = _audioInfo->getPlayEndCallbackObject()->getVoiceList();
		int voiceId = _audioInfo->getId();
		if (voiceList->objectForKey(voiceId)) {
			voiceList->removeObjectForKey(voiceId);
		}
		_audioInfo->setPlayEndCallback(nullptr);
		_audioInfo->setPlayEndCallbackObject(nullptr);
	});
}

void Object::playBgmObject(int id)
{
	AudioManager::AudioInfo* _audioInfo = AudioManager::getInstance()->playBgm(id);
	if (_audioInfo != nullptr) {
		this->addBgmList(_audioInfo);
	}
}
void Object::playBgmObject(int id, bool loop, int volume, int pan, int pitch, float seconds, float currentTime)
{
	AudioManager::AudioInfo* _audioInfo = AudioManager::getInstance()->playBgm(id, loop, volume, pan, pitch, seconds, currentTime);
	if (_audioInfo != nullptr) {
		this->addBgmList(_audioInfo);
	}
}

void Object::playSeObject(int id)
{
	AudioManager::AudioInfo* _audioInfo = AudioManager::getInstance()->playSe(id);
	if (_audioInfo != nullptr) {
		this->addSeList(_audioInfo);
	}
}
void Object::playSeObject(int id, bool loop, int volume, int pan, int pitch, float seconds, float currentTime)
{
	AudioManager::AudioInfo* _audioInfo = AudioManager::getInstance()->playSe(id, loop, volume, pan, pitch, seconds, currentTime);
	if (_audioInfo != nullptr) {
		this->addSeList(_audioInfo);
	}
}

void Object::playVoiceObject(int id)
{
	AudioManager::AudioInfo* _audioInfo = AudioManager::getInstance()->playVoice(id);
	if (_audioInfo != nullptr) {
		this->addVoiceList(_audioInfo);
	}
}
void Object::playVoiceObject(int id, bool loop, int volume, int pan, int pitch, float seconds, float currentTime)
{
	AudioManager::AudioInfo* _audioInfo = AudioManager::getInstance()->playVoice(id, loop, volume, pan, pitch, seconds, currentTime);
	if (_audioInfo != nullptr) {
		this->addVoiceList(_audioInfo);
	}
}

void Object::stopBgmObject(int bgmId, float seconds)
{
	auto objBgmList = this->getBgmList();
	AudioManager::AudioInfo * objAudioInfo;
	if (objBgmList->objectForKey(bgmId)) {
		auto bgmAudioList = dynamic_cast<cocos2d::__Dictionary *>(objBgmList->objectForKey(bgmId));
		cocos2d::DictElement *el;
		CCDICT_FOREACH(bgmAudioList, el) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
			objAudioInfo = static_cast<AudioManager::AudioInfo *>(el->getObject());
#else
			objAudioInfo = dynamic_cast<AudioManager::AudioInfo *>(el->getObject());
#endif
			int _objAudioId = objAudioInfo->getAudioId();
			AudioManager::getInstance()->stopBgm(bgmId, seconds, _objAudioId);
			objAudioInfo->setPlayEndCallback(nullptr);
			objAudioInfo->setPlayEndCallbackObject(nullptr);
		}
		// リストから削除
		objBgmList->removeObjectForKey(bgmId);
	}
}

void Object::stopSeObject(int seId, float seconds)
{
	auto objSeList = this->getSeList();
	AudioManager::AudioInfo * objAudioInfo;
	if (objSeList->objectForKey(seId)) {
		auto seAudioList = dynamic_cast<cocos2d::__Dictionary *>(objSeList->objectForKey(seId));
		cocos2d::DictElement *el;
		CCDICT_FOREACH(seAudioList, el) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
			objAudioInfo = static_cast<AudioManager::AudioInfo *>(el->getObject());
#else
			objAudioInfo = dynamic_cast<AudioManager::AudioInfo *>(el->getObject());
#endif
			int _objAudioId = objAudioInfo->getAudioId();
			AudioManager::getInstance()->stopSe(seId, seconds, _objAudioId);
			objAudioInfo->setPlayEndCallback(nullptr);
			objAudioInfo->setPlayEndCallbackObject(nullptr);
		}

		// リストから削除
		objSeList->removeObjectForKey(seId);
	}
}

void Object::stopVoiceObject(int voiceId, float seconds)
{
	auto objVoiceList = this->getVoiceList();
	AudioManager::AudioInfo * objAudioInfo;
	if (objVoiceList->objectForKey(voiceId)) {
		auto voiceAudioList = dynamic_cast<cocos2d::__Dictionary *>(objVoiceList->objectForKey(voiceId));
		cocos2d::DictElement *el;
		CCDICT_FOREACH(voiceAudioList, el) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
			objAudioInfo = static_cast<AudioManager::AudioInfo *>(el->getObject());
#else
			objAudioInfo = dynamic_cast<AudioManager::AudioInfo *>(el->getObject());
#endif
			int _objAudioId = objAudioInfo->getAudioId();
			AudioManager::getInstance()->stopVoice(voiceId, seconds, _objAudioId);
			objAudioInfo->setPlayEndCallback(nullptr);
			objAudioInfo->setPlayEndCallbackObject(nullptr);
		}
		// リストから削除
		objVoiceList->removeObjectForKey(voiceId);
	}
}

void Object::stopAllBgmObject(float seconds)
{
	auto objBgmList = this->getBgmList();
	cocos2d::DictElement *el;
	CCDICT_FOREACH(objBgmList, el) {
		auto bgmAudioList = dynamic_cast<cocos2d::__Dictionary *>(el->getObject());
		AudioManager::AudioInfo * objAudioInfo;
		cocos2d::DictElement *el2;
		CCDICT_FOREACH(bgmAudioList, el2) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
			objAudioInfo = static_cast<AudioManager::AudioInfo *>(el2->getObject());
#else
			objAudioInfo = dynamic_cast<AudioManager::AudioInfo *>(el2->getObject());
#endif
			int _objBgmId = objAudioInfo->getId();
			int _objAudioId = objAudioInfo->getAudioId();
			AudioManager::getInstance()->stopBgm(_objBgmId, seconds, _objAudioId);
			objAudioInfo->setPlayEndCallback(nullptr);
			objAudioInfo->setPlayEndCallbackObject(nullptr);
		}
	}
	objBgmList->removeAllObjects();
}

void Object::stopAllSeObject(float seconds)
{
	auto objSeList = this->getSeList();
	cocos2d::DictElement *el;
	CCDICT_FOREACH(objSeList, el) {
		auto seAudioList = dynamic_cast<cocos2d::__Dictionary *>(el->getObject());
		AudioManager::AudioInfo * objAudioInfo;
		cocos2d::DictElement *el2;
		CCDICT_FOREACH(seAudioList, el2) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
			objAudioInfo = static_cast<AudioManager::AudioInfo *>(el2->getObject());
#else
			objAudioInfo = dynamic_cast<AudioManager::AudioInfo *>(el2->getObject());
#endif
			int _objSeId = objAudioInfo->getId();
			int _objAudioId = objAudioInfo->getAudioId();
			AudioManager::getInstance()->stopSe(_objSeId, seconds, _objAudioId);
			objAudioInfo->setPlayEndCallback(nullptr);
			objAudioInfo->setPlayEndCallbackObject(nullptr);
		}
	}
	objSeList->removeAllObjects();
}

void Object::stopAllVoiceObject(float seconds)
{
	auto objVoiceList = this->getVoiceList();
	cocos2d::DictElement *el;
	CCDICT_FOREACH(objVoiceList, el) {
		auto voiceAudioList = dynamic_cast<cocos2d::__Dictionary *>(el->getObject());
		AudioManager::AudioInfo * objAudioInfo;
		cocos2d::DictElement *el2;
		CCDICT_FOREACH(voiceAudioList, el2) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
			objAudioInfo = static_cast<AudioManager::AudioInfo *>(el2->getObject());
#else
			objAudioInfo = dynamic_cast<AudioManager::AudioInfo *>(el2->getObject());
#endif
			int _objVoiceId = objAudioInfo->getId();
			int _objAudioId = objAudioInfo->getAudioId();
			AudioManager::getInstance()->stopVoice(_objVoiceId, seconds, _objAudioId);
			objAudioInfo->setPlayEndCallback(nullptr);
			objAudioInfo->setPlayEndCallbackObject(nullptr);
		}
	}
	objVoiceList->removeAllObjects();
}

void Object::retrieveDisplayDirectionVariable()
{
	auto playObjectData = this->getPlayObjectData();
	auto dispDirection = this->getDispDirection();

	auto variableDisplayDirection = playObjectData->getVariableData(agtk::data::kObjectSystemVariableDisplayDirection);
	if (!variableDisplayDirection->isExternalValue()) {
		return;
	}
	auto newValue = variableDisplayDirection->getValue();
	auto angle = agtk::GetDegreeFromVector(agtk::GetDirectionFromMoveDirectionId(this->getDispDirection()));
	if (angle == -0.0f) {
		angle = 0;
	}
	if (newValue != angle) {
		auto directionId = agtk::GetMoveDirectionId(angle);
		auto newDirectionId = directionId;

		auto objectData = this->getObjectData();
		auto moveType = objectData->getMoveType();
		if (moveType == agtk::data::ObjectData::kMoveTank || moveType == agtk::data::ObjectData::kMoveCar) {
			//戦車・車タイプ
			if (newValue == angle + 1) {
				//時計回りに次の表示方向。
				newDirectionId = agtk::GetMoveDirectionId(angle + 45.0f);
			}
			else if (newValue == angle - 1) {
				//半時計回りに次の表示方向を探して設定してみる。
				newDirectionId = agtk::GetMoveDirectionId(angle - 45.0f);
			}
			else {
				//指定の表方向を探して設定してみる。
				newDirectionId = agtk::GetMoveDirectionId(newValue);
			}
			//移動方向も変更。
			auto objectMovement = this->getObjectMovement();
			objectMovement->setDirection(agtk::GetDirectionFromMoveDirectionId(newDirectionId));
		}
		else {
			//基本移動
			//近い表示方向を探して設定する。
			auto player = this->getPlayer();
			if (player) {
				agtk::data::MotionData *motionData = nullptr;
				agtk::data::DirectionData *curDirectionData = nullptr;
				agtk::data::DirectionData *newDirectionData = nullptr;
				do {
					auto basePlayer = player->getBasePlayer();
					if (!basePlayer) break;
					auto motion = basePlayer->getCurrentAnimationMotion();
					if (!motion) break;
					motionData = motion->getMotionData();
					if (!motionData) break;
					auto direction = motion->getCurrentDirection();
					if (!direction) break;
					curDirectionData = direction->getDirectionData();
					if (!curDirectionData) break;
					if (newValue == angle + 1) {
						//時計回りに次の表示方向。
						for (int i = 0; i < 7; i++) {
							auto directionId = agtk::GetMoveDirectionId(angle + (i + 1) * 45.0f);
							auto directionData = this->getDirectionData(1 << directionId, this->getDispDirectionBit(), motionData, curDirectionData);
							if (directionData && (directionData->getAutoGeneration() || directionData->getDirectionBit() & (1 << directionId))) {
								newDirectionId = directionId;
								newDirectionData = directionData;
								break;
							}
						}
					}
					else if (newValue == angle - 1) {
						//半時計回りに次の表示方向を探して設定してみる。
						for (int i = 0; i < 7; i++) {
							auto directionId = agtk::GetMoveDirectionId(angle - (i + 1) * 45.0f);
							auto directionData = this->getDirectionData(1 << directionId, this->getDispDirectionBit(), motionData, curDirectionData);
							if (directionData && (directionData->getAutoGeneration() || directionData->getDirectionBit() & (1 << directionId))) {
								newDirectionId = directionId;
								newDirectionData = directionData;
								break;
							}
						}
					}
					else {
						//指定の表方向を探して設定してみる。
						auto directionId = agtk::GetMoveDirectionId(newValue);
						auto directionData = this->getDirectionData(1 << directionId, this->getDispDirectionBit(), motionData, curDirectionData);
						if (directionData && (directionData->getAutoGeneration() || directionData->getDirectionBit() & (1 << directionId))) {
							newDirectionId = directionId;
							newDirectionData = directionData;
							break;
						}
					}
				} while (0);
				if (newDirectionData) {
					if (newDirectionData != curDirectionData) {
						player->play(motionData->getId(), newDirectionData->getId());
					}
					//移動方向も変更。
					setMoveDirection(newDirectionId);
				}
			}
			else {
				if (newValue == angle + 1) {
					//時計回りに次の表示方向。
					newDirectionId = agtk::GetMoveDirectionId(angle + 45.0f);
				}
				else if (newValue == angle - 1) {
					//半時計回りに次の表示方向を探して設定してみる。
					newDirectionId = agtk::GetMoveDirectionId(angle - 45.0f);
				}
				else {
					//指定の表方向を探して設定してみる。
					newDirectionId = agtk::GetMoveDirectionId(newValue);
				}
				setMoveDirection(newDirectionId);
			}

			this->setDispDirection(newDirectionId);
			angle = agtk::GetDegreeFromVector(agtk::GetDirectionFromMoveDirectionId(newDirectionId));
			if (angle == -0.0f) {
				angle = 0;
			}
		}
	}
	variableDisplayDirection->setExternalValue(angle);	//_valueExternalFlagをクリア。
}

void Object::updateDisplayDirectionVariable()
{
	auto playObjectData = this->getPlayObjectData();
	auto player = this->getPlayer();
	auto objectData = this->getObjectData();
	bool isPhysicsOn = objectData->getPhysicsSettingFlag();
	bool isRelayPhysics = isPhysicsOn && playObjectData->getSwitchData(agtk::data::kObjectSystemSwitchFollowConnectedPhysics)->getValue();
	bool isPhysicsAffected = isPhysicsOn && playObjectData->getSwitchData(agtk::data::kObjectSystemSwitchAffectedByOtherObjects)->getValue();

	// 物理に依存しない場合
	float angle = 0.0f;
	if (!isRelayPhysics) {
		// 物理影響を受けない場合
		if (!isPhysicsAffected) {
			if (player) {
				//回転で自動生成か戦車タイプ移動／車タイプ移動の場合
				if (this->isAutoGeneration() || objectData->getMoveType() == agtk::data::ObjectData::kMoveCar || objectData->getMoveType() == agtk::data::ObjectData::kMoveTank) {
					angle = player->getCenterRotation();
				}
				else {
					angle = player->getCenterRotation();
					auto degree = agtk::GetDegreeFromVector(agtk::GetDirectionFromMoveDirectionId(this->getDispDirection()));
					angle = GetDegree360(angle + degree);
				}
			}
		}
		else {
			auto physicsNode = this->getphysicsNode();
			if (physicsNode) {
				angle = physicsNode->getRotation();
			}
		}
	}
	else {
		if (player) {
			auto physicsNode = this->getphysicsNode();
			if (physicsNode) {
				// プレイヤータイプでない または 表示方向を操作で変更しない場合
				if (!objectData->isGroupPlayer() || !objectData->getMoveDispDirectionSettingFlag()) {
					//回転で自動生成か戦車タイプ移動／車タイプ移動の場合
					if (this->isAutoGeneration() || objectData->getMoveType() == agtk::data::ObjectData::kMoveCar || objectData->getMoveType() == agtk::data::ObjectData::kMoveTank) {
						angle = player->getCenterRotation();
					}
					else {
						angle = player->getCenterRotation();
						auto degree = agtk::GetDegreeFromVector(agtk::GetDirectionFromMoveDirectionId(this->getDispDirection()));
						angle = GetDegree360(angle + degree);
					}
				}
				else {
					angle = physicsNode->getRotation();
				}
			}
		}
	}
	auto variableDisplayDirection = playObjectData->getVariableData(agtk::data::kObjectSystemVariableDisplayDirection);
	variableDisplayDirection->setExternalValue(angle);
}

//タイルや坂のHP変更トリガーを整理する。
void Object::refreshHpChangeTrigger()
{
	std::list<std::tuple<int, int, int>> unmarkedKeyList;
	for (auto &pair : _hpChangeTriggerInfoMap) {
		if (!pair.second.marked) {
			unmarkedKeyList.push_back(pair.first);
		}
		else {
			pair.second.marked = false;
		}
	}
	for (auto &key : unmarkedKeyList) {
		_hpChangeTriggerInfoMap.erase(key);
	}
}

//HP変更のトリガーが発生するとき真を返す。
bool Object::isHpChangeTrigger(agtk::Tile *tile, agtk::data::TileData *tileData, agtk::Slope *slope, agtk::data::OthersSlopeData *slopeData)
{
	auto type = agtk::SceneGameSpeed::eTYPE_TILE;	//坂もタイル扱いでゲームスピードの変更の影響を受けるように。
	if (tile && tile->getSceneLayer()->getType() == agtk::SceneLayer::kTypeMenu) {
		type = agtk::SceneGameSpeed::eTYPE_TILE_OR_MENU;
	}
	//auto sceneLayer = tile->getSceneLayer();
	//auto scene = sceneLayer->getScene();
	auto scene = GameManager::getInstance()->getCurrentScene();
	auto timeScale = scene->getGameSpeed()->getTimeScale(type) * FRAME_PER_SECONDS;
	int id = -1;
	int x = 0;
	int y = 0;
	auto triggerPeriod = FRAME_PER_SECONDS;
	if (tile) {
		id = tile->getTilesetId();
		x = tile->getX();
		y = tile->getY();
		if (tileData->getTriggerPeriodically()) {
			triggerPeriod = tileData->getTriggerPeriod();
		}
	}
	else {
		id = -slope->getId();	//タイルとIDが衝突しないように。
		if (slopeData->getTriggerPeriodically()) {
			triggerPeriod = slopeData->getTriggerPeriod();
		}
	}
	HpChangeTriggerInfo *pInfo = nullptr;
	auto key = std::make_tuple(id, (int)x, (int)y);
	auto it = _hpChangeTriggerInfoMap.find(key);
	pInfo = (it == _hpChangeTriggerInfoMap.end()) ? nullptr : &it->second;
	if (!pInfo) {
		_hpChangeTriggerInfoMap.emplace(key, HpChangeTriggerInfo(0));
		pInfo = &_hpChangeTriggerInfoMap.at(key);
	}
	if (pInfo->marked) {
		//既に処理済。
		return false;
	}
	pInfo->marked = true;
	pInfo->elapsed += timeScale;
	if (pInfo->elapsed >= triggerPeriod) {
		pInfo->elapsed -= triggerPeriod;
		return true;
	}
	return false;
}

cocos2d::Vec2 Object::directionCorrection(cocos2d::Vec2 direction)
{
	if (direction == cocos2d::Vec2::ZERO) {
		return direction;
	}
	auto playObjectData = this->getPlayObjectData();
	cocos2d::Vec2 vec = cocos2d::Vec2::ZERO;
	auto wallMoveSpeed = this->getObjectMovement()->getWallMoveSpeed()->get();
	vec.x = playObjectData->getVariableData(data::kObjectSystemVariableHorizontalMove)->getValue() + wallMoveSpeed;
	vec.y = playObjectData->getVariableData(data::kObjectSystemVariableVerticalMove)->getValue() + wallMoveSpeed;

	// 上下、左右どちらかの移動量がない場合は斜め移動できないので補正をかけない
	if (vec.x == 0.0f || vec.y == 0.0f) {
		return direction;
	}

	vec.x *= direction.y;
	vec.y *= direction.x;
	float degree = CC_RADIANS_TO_DEGREES(vec.getAngle());
	direction = agtk::GetDirectionFromDegrees(degree);
	// ななめ移動の速度を自動調整
	if (this->getObjectData()->getDiagonalMoveWithPolar()) {
	}
	else {
		cocos2d::Vec2 absDirection;
		absDirection.x = fabsf(direction.x);
		absDirection.y = fabsf(direction.y);
		if (absDirection.x > absDirection.y) {
			direction.x = direction.x * 1.0f / absDirection.x;
			direction.y = direction.y * 1.0f / absDirection.x;
		}
		else {
			direction.x = direction.x * 1.0f / absDirection.y;
			direction.y = direction.y * 1.0f / absDirection.y;
		}
	}
	return direction;
}

#ifdef USE_MULTITHREAD_OBJECT_UPDATE
void Object::lock()
{
	//_mutex.lock();
	if (!_mutex.try_lock()) {
		_mutex.lock();
#ifdef USE_MULTITHREAD_MEASURE
		ThreadManager::objectBlockedCount++;
#endif
	}
}
#endif

#ifdef FIX_ACT2_4774
int Object::getPortalMoveDispBit()
{
	if (_portalMoveDispBit >= 0) {
		return _portalMoveDispBit;
	}
	_portalMoveDispBit = 0;
#if 1	// ACT2-4833 実際に移動した方向ベクトルを使う。
	// ACT2-5116 座標値に細かい少数点が入っている場合、移動していると判定されてしまうので小数点を切り捨てる
	auto objectPosition = Vec2((int)_objectPosition.x, (int)_objectPosition.y);
	auto premoveObjectPosition = Vec2((int)_premoveObjectPosition.x, (int)_premoveObjectPosition.y);

	auto moveVec = objectPosition - premoveObjectPosition;
	moveVec.y = -moveVec.y;
	if (moveVec.x != 0 || moveVec.y != 0) {
		int moveBit = 0;
		auto angle = fmod(RadianToDegree(moveVec.getAngle()) + 360.0f, 360.0f);
		// 右
		if (angle >= 360.0f - TANK_OR_CAR_DIRECTION_MARGINE || angle < 0.f + TANK_OR_CAR_DIRECTION_MARGINE) {
			moveBit |= (1 << 6);
		}
		// 右上
		else if (angle >= 45.0f - TANK_OR_CAR_DIRECTION_MARGINE && angle < 45.0f + TANK_OR_CAR_DIRECTION_MARGINE) {
			moveBit |= (1 << 9);
		}
		// 上
		else if (angle >= 90.0f - TANK_OR_CAR_DIRECTION_MARGINE && angle < 90.0f + TANK_OR_CAR_DIRECTION_MARGINE) {
			moveBit |= (1 << 8);
		}
		// 左上
		else if (angle >= 135.0f - TANK_OR_CAR_DIRECTION_MARGINE && angle < 135.0f + TANK_OR_CAR_DIRECTION_MARGINE) {
			moveBit |= (1 << 7);
		}
		// 左
		else if (angle >= 180.0f - TANK_OR_CAR_DIRECTION_MARGINE && angle < 180.0f + TANK_OR_CAR_DIRECTION_MARGINE) {
			moveBit |= (1 << 4);
		}
		// 左下
		else if (angle >= 225.0f - TANK_OR_CAR_DIRECTION_MARGINE && angle < 225.0f + TANK_OR_CAR_DIRECTION_MARGINE) {
			moveBit |= (1 << 1);
		}
		// 下
		else if (angle >= 270.0f - TANK_OR_CAR_DIRECTION_MARGINE && angle < 270.0f + TANK_OR_CAR_DIRECTION_MARGINE) {
			moveBit |= (1 << 2);
		}
		// 右下
		else if (angle >= 315 - TANK_OR_CAR_DIRECTION_MARGINE && angle < 315.0f + TANK_OR_CAR_DIRECTION_MARGINE) {
			moveBit |= (1 << 3);
		}
		_portalMoveDispBit = moveBit;
	}
#else
	if (this->getInputDirectionId() > 0) {

		int dispBit = 0;

		// 通常移動の場合
		if (this->getObjectData()->getMoveType() == agtk::data::ObjectData::EnumMoveType::kMoveNormal) {
			dispBit = (1 << this->getInputDirectionId());
		}
		// 戦車・車移動の場合
		else {
#if 1	//ACT2-4836: 前後移動入力があった場合、前後の移動ベクトルと±45度未満の範囲内の方向を有効にする。
			auto inputDirectionId = this->getInputDirectionId();
			auto direction = this->getObjectMovement()->getDirection();
			bool isInput = false;
			if (inputDirectionId >= 7 && inputDirectionId <= 9) {
				//前移動入力あり
				isInput = true;
			}
			else if (inputDirectionId >= 1 && inputDirectionId <= 3) {
				//後移動入力あり
				direction = -direction;
				isInput = true;
			}
			if (isInput) {
				auto angle = fmod(RadianToDegree(direction.getAngle()) + 360.0f, 360.0f);
				// 右
				if (angle >= 360.0f - TANK_OR_CAR_DIRECTION_MARGINE || angle < 0.f + TANK_OR_CAR_DIRECTION_MARGINE) {
					dispBit |= (1 << 6);
				}
				// 右上
				else if (angle >= 45.0f - TANK_OR_CAR_DIRECTION_MARGINE && angle < 45.0f + TANK_OR_CAR_DIRECTION_MARGINE) {
					dispBit |= (1 << 9);
				}
				// 上
				else if (angle >= 90.0f - TANK_OR_CAR_DIRECTION_MARGINE && angle < 90.0f + TANK_OR_CAR_DIRECTION_MARGINE) {
					dispBit |= (1 << 8);
				}
				// 左上
				else if (angle >= 135.0f - TANK_OR_CAR_DIRECTION_MARGINE && angle < 135.0f + TANK_OR_CAR_DIRECTION_MARGINE) {
					dispBit |= (1 << 7);
				}
				// 左
				else if (angle >= 180.0f - TANK_OR_CAR_DIRECTION_MARGINE && angle < 180.0f + TANK_OR_CAR_DIRECTION_MARGINE) {
					dispBit |= (1 << 4);
				}
				// 左下
				else if (angle >= 225.0f - TANK_OR_CAR_DIRECTION_MARGINE && angle < 225.0f + TANK_OR_CAR_DIRECTION_MARGINE) {
					dispBit |= (1 << 1);
				}
				// 下
				else if (angle >= 270.0f - TANK_OR_CAR_DIRECTION_MARGINE && angle < 270.0f + TANK_OR_CAR_DIRECTION_MARGINE) {
					dispBit |= (1 << 2);
				}
				// 右下
				else if (angle >= 315 - TANK_OR_CAR_DIRECTION_MARGINE && angle < 315.0f + TANK_OR_CAR_DIRECTION_MARGINE) {
					dispBit |= (1 << 3);
				}
			}
#else
			int shiftValue = 0;
			auto direction = this->getObjectMovement()->getDirection();
			auto angle = fmod(RadianToDegree(direction.getAngle()) + 360.0f, 360.0f);

			// 右
			if (angle >= 360.0f - TANK_OR_CAR_DIRECTION_MARGINE || angle < 0.f + TANK_OR_CAR_DIRECTION_MARGINE) {
				shiftValue = 6;
			}
			// 右上
			else if (angle >= 45.0f - TANK_OR_CAR_DIRECTION_MARGINE && angle < 45.0f + TANK_OR_CAR_DIRECTION_MARGINE) {
				shiftValue = 9;
			}
			// 上
			else if (angle >= 90.0f - TANK_OR_CAR_DIRECTION_MARGINE && angle < 90.0f + TANK_OR_CAR_DIRECTION_MARGINE) {
				shiftValue = 8;
			}
			// 左上
			else if (angle >= 135.0f - TANK_OR_CAR_DIRECTION_MARGINE && angle < 135.0f + TANK_OR_CAR_DIRECTION_MARGINE) {
				shiftValue = 7;
			}
			// 左
			else if (angle >= 180.0f - TANK_OR_CAR_DIRECTION_MARGINE && angle < 180.0f + TANK_OR_CAR_DIRECTION_MARGINE) {
				shiftValue = 4;
			}
			// 左下
			else if (angle >= 225.0f - TANK_OR_CAR_DIRECTION_MARGINE && angle < 225.0f + TANK_OR_CAR_DIRECTION_MARGINE) {
				shiftValue = 1;
			}
			// 下
			else if (angle >= 270.0f - TANK_OR_CAR_DIRECTION_MARGINE && angle < 270.0f + TANK_OR_CAR_DIRECTION_MARGINE) {
				shiftValue = 2;
			}
			// 右下
			else if (angle >= 315 - TANK_OR_CAR_DIRECTION_MARGINE && angle < 315.0f + TANK_OR_CAR_DIRECTION_MARGINE) {
				shiftValue = 3;
			}

			dispBit = (1 << shiftValue);

			CCLOG("shift: %d, bit: %d", shiftValue, dispBit);
#endif
		}

		_portalMoveDispBit = dispBit;
	}
#endif
	return _portalMoveDispBit;
}
#endif

//オブジェクトに付属する物理演算パーツのうち、オブジェクト本体と接着や回転軸でつながっているもののリストを返す。
cocos2d::__Array *Object::getPinAxisConnectedPhysicsBaseList()
{
	auto sceneLayer = this->getSceneLayer();
	auto physicsObjectList = sceneLayer->getPhysicsObjectList();
	auto physicsPartsList = this->getPhysicsPartsList();
	auto physicsBaseList = cocos2d::__Array::create();
	auto checkList = std::vector<cocos2d::Node *>();
	auto ignoreList = cocos2d::__Array::create();
	auto physicsNode = this->getphysicsNode();
	auto pinAxisList = std::vector<agtk::PhysicsBase *>();
	cocos2d::Ref *ref = nullptr;
	CCARRAY_FOREACH(physicsObjectList, ref) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
		auto physicsBase = static_cast<agtk::PhysicsBase *>(ref);
#else
		auto physicsBase = dynamic_cast<agtk::PhysicsBase *>(ref);
#endif
		auto pin = dynamic_cast<agtk::PhysicsPin *>(physicsBase);
		auto axis = dynamic_cast<agtk::PhysicsAxis *>(physicsBase);
		if (pin || axis) {
			pinAxisList.push_back(physicsBase);
		}
	}
	checkList.push_back(physicsNode);
	while (checkList.size() > 0) {
		auto newCheckList = std::vector<cocos2d::Node *>();
		for (auto checkNode : checkList) {
			for (auto connector : pinAxisList) {
				if (!connector->getFollowerPhysicsBody()) {
					continue;
				}
				auto joints = connector->getFollowerPhysicsBody()->getJoints();
				for (auto joint : joints) {
					auto bodyA = joint->getBodyA();
					auto bodyB = joint->getBodyB();
					auto owner1 = joint->getBodyA()->getOwner();
					auto owner2 = joint->getBodyB()->getOwner();
					if (owner1 == checkNode) {
						if (!physicsBaseList->containsObject(connector)) {
							physicsBaseList->addObject(connector);
						}
						if (owner2 != physicsNode && !physicsBaseList->containsObject(owner2)) {
							physicsBaseList->addObject(owner2);
							newCheckList.push_back(owner2);
						}
					}
					else if (owner2 == checkNode) {
						if (!physicsBaseList->containsObject(connector)) {
							physicsBaseList->addObject(connector);
						}
						if (owner1 != physicsNode && !physicsBaseList->containsObject(owner1)) {
							physicsBaseList->addObject(owner1);
							newCheckList.push_back(owner1);
						}
					}
				}
			}
		}
		checkList = newCheckList;
	}
	return physicsBaseList;
}

#if (CC_TARGET_PLATFORM == CC_PLATFORM_NX)
#endif

//-------------------------------------------------------------------------------------------------------------------
ConnectObject::ConnectObject()
{
	_objectConnectSettingData = nullptr;
	_connectBaseObject = nullptr;
	isConnecting = false;
	_connectionId = -1;
}

ConnectObject::~ConnectObject()
{
	CC_SAFE_RELEASE_NULL(_objectConnectSettingData);
	CC_SAFE_RELEASE_NULL(_connectBaseObject);
}

void ConnectObject::setCreate(ConnectObject* (*create)(agtk::Object * object, int connectObjectSettingId, int actionId))
{
	ConnectObject::create = create;
}


ConnectObject* ConnectObject::_create(agtk::Object * object, int connectObjectSettingId, int actionId)
{
	
		ConnectObject *pRet = new(std::nothrow) ConnectObject();
		if (pRet && pRet->init(object, connectObjectSettingId, actionId))
		{
			pRet->autorelease();
			return pRet;
		}
		else
		{
			delete pRet;
			pRet = nullptr;
			return nullptr;
		}
}

ConnectObject* ConnectObject::create2(agtk::Object *object, int connectObjectSettingId, cocos2d::Vec2 position, cocos2d::Vec2 scale, int initialActionId, int moveDirectionId)
{
	ConnectObject *pRet = new(std::nothrow) ConnectObject();
	if (pRet && pRet->init2(object, connectObjectSettingId, position, scale, initialActionId, moveDirectionId)) {
		pRet->autorelease();
		return pRet;
	}
	else
	{
		delete pRet;
		pRet = nullptr;
		return nullptr;
	}
}

bool ConnectObject::init(agtk::Object *object, int connectObjectSettingId, int actionId)
{
	auto sceneLayer = object->getSceneLayer();
	CC_ASSERT(sceneLayer);
	auto scene = sceneLayer->getScene();
	CC_ASSERT(scene);
	auto objectData = object->getObjectData();
	CC_ASSERT(objectData);

// #AGTK-NX
#ifdef STATIC_DOWN_CAST
	auto settingData = static_cast<agtk::data::ObjectConnectSettingData *>(objectData->getConnectSettingList()->objectForKey(connectObjectSettingId));
#else
	auto settingData = dynamic_cast<agtk::data::ObjectConnectSettingData *>(objectData->getConnectSettingList()->objectForKey(connectObjectSettingId));
#endif
	this->setObjectConnectSettingData(settingData);

	// 接続するオブジェクトが存在するかをチェック
	auto projectData = GameManager::getInstance()->getProjectData();
	CC_ASSERT(projectData);
	auto connectObjectData = projectData->getObjectData(settingData->getObjectId());
	if (connectObjectData == nullptr) {
		return false;
	}
	cocos2d::Vec2 pos = object->getPosition();

	switch (settingData->getPositionType()) {
		// このオブジェクトの中心
	case agtk::data::ObjectConnectSettingData::EnumPositionType::kPositionCenter: {
		pos = object->getCenterPosition();
	} break;

		// このオブジェクトの足元
	case agtk::data::ObjectConnectSettingData::EnumPositionType::kPositionFoot: {
		pos = object->getFootPosition();
	} break;
		// 接続点を使用
	case agtk::data::ObjectConnectSettingData::EnumPositionType::kPositionUseConnection: {
		int connectId = settingData->getConnectionId();
		pos = object->getCenterPosition();
		if (connectId > 0) {
			agtk::Vertex4 vertex4;
			if (object->getTimeline(agtk::data::TimelineInfoData::kTimelineConnection, connectId, vertex4)) {
				pos = Scene::getPositionSceneFromCocos2d(vertex4.addr()[0], object->getSceneData());
			}
		}
	}break;
	}

	int dispDirection = -1;

	// 位置の調整を反映する
	int adjustX = settingData->getAdjustX();
	int adjustY = settingData->getAdjustY();

	pos.x += adjustX;
	pos.y += adjustY;

	// 「生成時のオブジェクトの向きを、このオブジェクトに合わせる」にチェック時 又は、
	// 接続するオブジェクトの「親オブジェクトから引き継ぐ要素：表示方向」にチェック時、親の向きと同じ表示方向を設定する
	if (settingData->getSetDirectionToConnectObjectDirection() ||
		connectObjectData->getTakeoverDispDirection()) {
		// 親オブジェクトの向きを取得
		dispDirection = object->getDispDirection();
		// 方向未指定の場合
		if (dispDirection <= 0) {
			// 親オブジェクトの方向を再計算して取得
			dispDirection = object->calcDispDirection();
		}
	}

	// オブジェクト生成
	bool ret = Object::init(
		sceneLayer,
		settingData->getObjectId(),
		actionId,
		pos,
		cocos2d::Vec2(1, 1),
		0.0f,
		dispDirection,
		-1,
		-1
	);
	if (ret == false) {
		return false;
	}
	//表示方向の設定ありで、かつ「親オブジェクトから離れず追従する」以外の場合。
	if (dispDirection > 0 && this->getObjectData()->getFollowType() != agtk::data::ObjectData::EnumFollowType::kFollowClose) {
		//移動方向をクリアする。
		this->getObjectMovement()->setDirection(cocos2d::Vec2::ZERO);
	}
	sceneLayer->addCollisionDetaction(this);

	// 
	this->setConnectBaseObject(object);
	this->setId(sceneLayer->publishObjectId());
	this->getPlayObjectData()->setInstanceId(scene->getObjectInstanceId(this));
	this->getPlayObjectData()->setInstanceCount(scene->incrementObjectInstanceCount(this->getObjectData()->getId()));
	scene->updateObjectInstanceCount(this->getObjectData()->getId());
	this->setLayerId(sceneLayer->getLayerId());

	//視野・照明
	agtk::ViewportLightSceneLayer *viewportLightSceneLayer = nullptr;
	if (sceneLayer->getType() == agtk::SceneLayer::kTypeScene) {
		viewportLightSceneLayer = scene->getViewportLight()->getViewportLightSceneLayer(this->getLayerId());
	}
	if (viewportLightSceneLayer && this->getObjectData()->getViewportLightSettingFlag() && this->getObjectData()->getViewportLightSettingList()->count() > 0) {
		auto viewportLightObject = ViewportLightObject::create(this, scene->getViewportLight(), sceneLayer);
		viewportLightSceneLayer->getViewportLightObjectList()->addObject(viewportLightObject);
	}

	sceneLayer->addObject(this);

	// 初期Zオーダー値を取得
	initZorder = object->getLocalZOrder();

	int connectionId = (settingData->getPositionType() == agtk::data::ObjectConnectSettingData::EnumPositionType::kPositionUseConnection) ? settingData->getConnectionId() : -1;
	this->setConnectionId(connectionId);
	// 「このオブジェクトの子オブジェクトとする」チェック時、
	// 子オブジェクトとして設定する
	if (settingData->getChildObject()) {
		// 親(接続元)オブジェクトのインスタンスIDを保存
		this->getPlayObjectData()->setParentObjectInstanceId(object->getInstanceId());
		object->addChildObject(this, cocos2d::Vec2(adjustX, adjustY), connectionId);
	}

	// 「このオブジェクトより表示の優先度を下げる」チェック時、
	// Zオーダーを変更する
	if (settingData->getLowerPriority()) {
		this->getParent()->reorderChild(this, initZorder - 1);
	}
	object->addConnectObjectDispPriority(this, settingData->getLowerPriority());

	isConnecting = true;

	//接続点を使用の場合。
	if (settingData->getPositionType() == agtk::data::ObjectViewportLightSettingData::kPositionUseConnection) {
		//表示有無を設定。
		auto player = object->getPlayer();
		bool valid = true;
		if (player) {
			valid = player->getTimelineValid(settingData->getConnectionId());
		}
		auto oldForceDisabled = this->getForceDisabled();
		this->setForceDisabled(!valid);
		if (this->getForceDisabled() != oldForceDisabled) {
			this->setVisible(valid);
		}
		if (this->getPlayer()) {
			this->getPlayer()->setVisible(valid);
		}
	}

	return true;
}

bool ConnectObject::init2(agtk::Object *object, int connectObjectSettingId, cocos2d::Vec2 position, cocos2d::Vec2 scale, int initialActionId, int moveDirectionId)
{
	auto sceneLayer = object->getSceneLayer();
	CC_ASSERT(sceneLayer);
	auto scene = sceneLayer->getScene();
	CC_ASSERT(scene);
	auto objectData = object->getObjectData();
	CC_ASSERT(objectData);

// #AGTK-NX
#ifdef STATIC_DOWN_CAST
	auto settingData = static_cast<agtk::data::ObjectConnectSettingData *>(objectData->getConnectSettingList()->objectForKey(connectObjectSettingId));
#else
	auto settingData = dynamic_cast<agtk::data::ObjectConnectSettingData *>(objectData->getConnectSettingList()->objectForKey(connectObjectSettingId));
#endif
	this->setObjectConnectSettingData(settingData);

	// 接続するオブジェクトが存在するかをチェック
	auto projectData = GameManager::getInstance()->getProjectData();
	CC_ASSERT(projectData);
	auto connectObjectData = projectData->getObjectData(settingData->getObjectId());
	if (connectObjectData == nullptr) {
		return false;
	}

	// 位置の調整を反映する
	int adjustX = settingData->getAdjustX();
	int adjustY = settingData->getAdjustY();

	// オブジェクト生成
	bool ret = Object::init(
		sceneLayer,
		settingData->getObjectId(),
		initialActionId,
		position,
		scale,
		0.0f,
		moveDirectionId,
		-1,
		-1
	);
	if (ret == false) {
		return false;
	}
	//表示方向の設定ありで、かつ「親オブジェクトから離れず追従する」以外の場合。
	if (moveDirectionId > 0 && this->getObjectData()->getFollowType() != agtk::data::ObjectData::EnumFollowType::kFollowClose) {
		//移動方向をクリアする。
		this->getObjectMovement()->setDirection(cocos2d::Vec2::ZERO);
	}
	sceneLayer->addCollisionDetaction(this);

	// 
	this->setConnectBaseObject(object);
	this->setId(sceneLayer->publishObjectId());
	this->getPlayObjectData()->setInstanceId(scene->getObjectInstanceId(this));
	this->getPlayObjectData()->setInstanceCount(scene->incrementObjectInstanceCount(this->getObjectData()->getId()));
	scene->updateObjectInstanceCount(this->getObjectData()->getId());
	this->setLayerId(sceneLayer->getLayerId());

	//視野・照明
	agtk::ViewportLightSceneLayer *viewportLightSceneLayer = nullptr;
	if (sceneLayer->getType() == agtk::SceneLayer::kTypeScene) {
		viewportLightSceneLayer = scene->getViewportLight()->getViewportLightSceneLayer(this->getLayerId());
	}
	if (viewportLightSceneLayer && this->getObjectData()->getViewportLightSettingFlag() && this->getObjectData()->getViewportLightSettingList()->count() > 0) {
		auto viewportLightObject = ViewportLightObject::create(this, scene->getViewportLight(), sceneLayer);
		viewportLightSceneLayer->getViewportLightObjectList()->addObject(viewportLightObject);
	}

	sceneLayer->addObject(this);

	// 初期Zオーダー値を取得
	initZorder = object->getLocalZOrder();

	int connectionId = (settingData->getPositionType() == agtk::data::ObjectConnectSettingData::EnumPositionType::kPositionUseConnection) ? settingData->getConnectionId() : -1;
	this->setConnectionId(connectionId);
	// 「このオブジェクトの子オブジェクトとする」チェック時、
	// 子オブジェクトとして設定する
	if (settingData->getChildObject()) {
		// 親(接続元)オブジェクトのインスタンスIDを保存
		this->getPlayObjectData()->setParentObjectInstanceId(object->getInstanceId());
		object->addChildObject(this, cocos2d::Vec2(adjustX, adjustY), connectionId);
	}

	// 「このオブジェクトより表示の優先度を下げる」チェック時、
	// Zオーダーを変更する
	if (settingData->getLowerPriority()) {
		this->getParent()->reorderChild(this, initZorder - 1);
	}
	object->addConnectObjectDispPriority(this, settingData->getLowerPriority());

	isConnecting = true;

	//接続点を使用の場合。
	if (settingData->getPositionType() == agtk::data::ObjectViewportLightSettingData::kPositionUseConnection) {
		//表示有無を設定。
		auto player = object->getPlayer();
		bool valid = true;
		if (player) {
			valid = player->getTimelineValid(settingData->getConnectionId());
		}
		auto oldForceDisabled = this->getForceDisabled();
		this->setForceDisabled(!valid);
		if (this->getForceDisabled() != oldForceDisabled) {
			this->setVisible(valid);
		}
		if (this->getPlayer()) {
			this->getPlayer()->setVisible(valid);
		}
	}

	return true;
}

#ifdef USE_MULTITHREAD_OBJECT_UPDATE
void ConnectObject::objectUpdateBefore(float delta)
#else
void ConnectObject::update(float delta)
#endif
{
	// オブジェクトが接続状態の場合
	if (isConnecting) {

		// 子オブジェクトになっている場合
		if (getOwnParentObject()) {

			auto settingData = getObjectConnectSettingData();
			auto object = getOwnParentObject();

			// 「このオブジェクトより表示の優先度を下げる」未チェック時
			if (!settingData->getLowerPriority()) {

				// 現在のZオーダー値を取得
				int crntZOrder = this->getLocalZOrder();
				int nextZOrder = crntZOrder;

				if (object->getAreaBackSide(settingData->getConnectionId())) {
					nextZOrder = initZorder - 1;
				}
				else {
					nextZOrder = initZorder;
				}

				if (crntZOrder != nextZOrder) {
					this->getParent()->reorderChild(this, nextZOrder);
				}
			}

			//接続点を仕様の場合。
			if (settingData->getPositionType() == agtk::data::ObjectViewportLightSettingData::kPositionUseConnection) {
				//表示有無を設定。
				auto basePlayer = object->getPlayer();
				bool valid = true;
				if (basePlayer) {
					valid = basePlayer->getTimelineValid(settingData->getConnectionId());
				}
				auto oldForceDisabled = this->getForceDisabled();
				this->setForceDisabled(!valid);
				if (this->getForceDisabled() != oldForceDisabled) {
					this->setVisible(valid);
				}
			}
#ifdef USE_MULTITHREAD_OBJECT_UPDATE
			// オブジェクト固定処理を無視する
			Object::objectUpdateBefore(delta);
			return;
		}
		auto baseObj = getConnectBaseObject();
		if (baseObj) {
			Object::objectUpdateBefore(delta);
		}
	}
	// 接続状態が無効の場合
	else {
		Object::objectUpdateBefore(delta);
	}
}
void ConnectObject::objectUpdateAfter(float delta)
{
	// オブジェクトが接続状態の場合
	if (isConnecting) {

		// 子オブジェクトになっている場合
		if (getOwnParentObject()) {
			// オブジェクト固定処理を無視する
			Object::objectUpdateAfter(delta);
#else
			Object::update(delta);
#endif

			_parentFollowPosOffset = [&](void)->cocos2d::Vec2 {
				// 位置の調整を反映する
				int adjustX = getObjectConnectSettingData()->getAdjustX();
				int adjustY = getObjectConnectSettingData()->getAdjustY();
				return cocos2d::Vec2(adjustX, adjustY);
			}();

			return;
		}

		auto baseObj = getConnectBaseObject();

		if (baseObj) {

			auto position = this->getPosition();
#ifdef USE_MULTITHREAD_OBJECT_UPDATE
			Object::objectUpdateAfter(delta);
#else
			Object::update(delta);
#endif

			//接続点を私用の場合。
			auto settingData = getObjectConnectSettingData();
			if (settingData->getPositionType() == agtk::data::ObjectViewportLightSettingData::kPositionUseConnection) {
				//表示有無を設定。
				auto basePlayer = baseObj->getPlayer();
				bool valid = true;
				if (basePlayer) {
					valid = basePlayer->getTimelineValid(settingData->getConnectionId());
				}
				auto oldForceDisabled = this->getForceDisabled();
				this->setForceDisabled(!valid);
				if (this->getForceDisabled() != oldForceDisabled) {
					this->setVisible(valid);
				}
			}

			//位置。
			this->setOldPosition(position);
			auto player = this->getPlayer();
			if (player) {
				this->setOldWallPosition(player->getPosition());
			}
			this->setPosition(getConnectPosition());
		}
		else {
			unconnect();
		}
	}
#ifdef USE_MULTITHREAD_OBJECT_UPDATE
	// 接続状態が無効の場合
	else {
		Object::objectUpdateAfter(delta);
	}
#else
	// 接続状態が無効の場合
	else {
		Object::update(delta);
	}
#endif
}

#ifdef USE_MULTITHREAD_OBJECT_UPDATE
void ConnectObject::objectUpdateWallCollision(float delta)
{
	// オブジェクトが接続状態の場合
	if (isConnecting) {

		// 子オブジェクトになっている場合
		if (getOwnParentObject()) {
			// オブジェクト固定処理を無視する
			Object::objectUpdateWallCollision(delta);
			return;
		}
		auto baseObj = getConnectBaseObject();
		if (baseObj) {
			Object::objectUpdateWallCollision(delta);
		}
	}
	// 接続状態が無効の場合
	else {
		Object::objectUpdateWallCollision(delta);
	}
}

void ConnectObject::update(float delta)
{
// #AGTK-NX #AGTK-WIN
#ifdef USE_30FPS_3
	auto gm = GameManager::getInstance();
	gm->setPassIndex(0);
	gm->setPassCount(1);
	gm->setLastPass(true);
#endif
	this->objectUpdateBefore(delta);
// #AGTK-NX #AGTK-WIN
#ifdef USE_30FPS_3
	gm->restoreWallCollisionPass();
	auto orgDelta = delta;
	for (int passIndex = 0; passIndex < gm->getPassCount(); passIndex++) {
		gm->setPassIndex(passIndex);
		bool bLastPass = false;
		if (passIndex == gm->getPassCount() - 1) {
			bLastPass = true;
		}
		gm->setLastPass(bLastPass);
		if (gm->getPassCount() > 1) {
			if (!bLastPass) {
				// pass1
				// 2pass処理向けに位置と移動量の補正
				this->_iniPos = this->getPosition();
				Vec2 mv = this->getPosition() - this->getOldPosition();
				this->setPosition(this->getOldPosition() + mv * 0.5f);
				this->_halfMove = mv - mv * 0.5f;
				// delta補正
				delta = delta * 0.5f;
			}
			else {
				// pass2
				// 中間フレームの位置を記録
				Vec2 pass1Pos = this->getPosition();
				Vec2 diff = pass1Pos - this->_iniPos;
				if (diff != Vec2::ZERO) {
					// 中間地点を反映
					this->setPassedFrameCount(this->getFrameCount());
					this->setPassedFramePosition(pass1Pos);
				}
				// pass2での過去位置更新
				this->setOldPosition(pass1Pos);
				this->setOldPosition2(pass1Pos);
				auto player = this->getPlayer();
				if (player) {
					this->setOldWallPosition(player->getPosition());
				}
				// pass2用のWallHitInfoGroupに更新 
				this->_bUseMiddleFrame = true;
				this->getObjectCollision()->updateMiddleFrameWallHitInfoGroup();
				// 2pass処理向けに位置と移動量の補正
				// 2pass移動により本来の目標位置への移動に誤差が発生するため誤差が僅かならば補正
				Vec2 pass2Pos = this->getPosition() + this->_halfMove;
				diff = pass2Pos - this->_iniPos;
				diff.x = std::abs(diff.x);
				diff.y = std::abs(diff.y);
				if (diff.x > 0.0f && diff.x < 0.001f) {
					pass2Pos.x = this->_iniPos.x;
				}
				if (diff.y > 0.0f && diff.y < 0.001f) {
					pass2Pos.y = this->_iniPos.y;
				}
				this->setPosition(pass2Pos);
				// delta補正
				delta = orgDelta - delta;
			}
		}
#endif
	this->objectUpdateWallCollision(delta);
// #AGTK-NX #AGTK-WIN
#ifdef USE_30FPS_3
	} // for (int passIndex = 0; passIndex < gm->getPassCount(); passIndex++);
	gm->restoreWallCollisionPass();
	delta = orgDelta;
#endif
	this->objectUpdateAfter(delta);
}
#endif

void ConnectObject::unconnect()
{
	if (isConnecting == false) {
		return;
	}
	// 接続を無効化
	isConnecting = false;

	// 接続元オブジェクトから離脱する
	_connectBaseObject->removeChildObject(this);
	_connectBaseObject->removeConnectObjectDispPriority(this);
	changeParentObject(nullptr, Vec2::ZERO, -1);

	//移動情報をクリアする。
	this->getObjectMovement()->reset();

	// 接続元オブジェクトの参照を解除
	this->setConnectBaseObject(nullptr);
}

cocos2d::Vec2 ConnectObject::getConnectPosition() 
{
	auto baseObj = getConnectBaseObject();

	if (baseObj) {
		cocos2d::Vec2 pos = baseObj->getPosition();

		switch (getObjectConnectSettingData()->getPositionType()) {
			// このオブジェクトの中心
		case agtk::data::ObjectConnectSettingData::EnumPositionType::kPositionCenter: {
			pos = baseObj->getCenterPosition();
		} break;

			// このオブジェクトの足元
		case agtk::data::ObjectViewportLightSettingData::kPositionFoot: {
			pos = baseObj->getFootPosition();
		} break;

			// 接続点を使用
		case agtk::data::ObjectViewportLightSettingData::kPositionUseConnection: {
			pos = baseObj->getCenterPosition();
			if (getObjectConnectSettingData()->getConnectionId() > 0) {
				agtk::Vertex4 v4;
				auto ret = baseObj->getTimeline(agtk::data::TimelineInfoData::kTimelineConnection, getObjectConnectSettingData()->getConnectionId(), v4);
				if (ret) {
					pos = agtk::Scene::getPositionSceneFromCocos2d(v4.addr()[0], baseObj->getSceneData());
				}
			}
		}break;
		}

		// 位置の調整を反映する
		int adjustX = getObjectConnectSettingData()->getAdjustX();
		int adjustY = getObjectConnectSettingData()->getAdjustY();

		// オブジェクトの向きを取得
		auto areaDir = agtk::GetDirectionFromMoveDirectionId(baseObj->getDispDirection());
		// 左を向いている場合
		if (areaDir.x < 0) {
			adjustX *= -1;
		}
		// 下を向いている場合
		if (areaDir.y < 0) {
			adjustY *= -1;
		}

		pos.x += adjustX;
		pos.y += adjustY;

		return pos;
	}

	return cocos2d::Vec2::ZERO;
}



//-------------------------------------------------------------------------------------------------------------------
ObjectAfterImage::ObjectAfterImage()
{
	_baseObject = nullptr;
	_afterImageList = nullptr;
	_intervalDuration300 = 0;
	_minIntervalDuration300 = Director::getInstance()->getAnimationInterval() * 300;
}

ObjectAfterImage::~ObjectAfterImage()
{
	//CC_SAFE_RELEASE_NULL(_baseObject);//※retainしていないのでrelease不要です。
	CC_SAFE_RELEASE_NULL(_afterImageList);
}

bool ObjectAfterImage::init(agtk::Object * object)
{
	auto projectData = GameManager::getInstance()->getProjectData();
	CC_ASSERT(projectData);
	auto objectData = object->getObjectData();
	CC_ASSERT(objectData);

	_baseObject = object;

	auto imageList = cocos2d::__Array::create();
	this->setAfterImageList(imageList);

	// 「残像数」が1以上の場合
	if (objectData->getAfterimageCount() > 0) {
		
		// 表示間隔の初期値を設定
		_intervalDuration300 = std::fmax(objectData->getAfterimageInterval() * 300, _minIntervalDuration300);

		for (int i = 0; i < objectData->getAfterimageCount(); i++) {

			auto afterImage = agtk::ObjectAfterImage::AfterImage::create(object);
			imageList->addObject(afterImage);
		}
	}

	return true;
}

void ObjectAfterImage::update(float delta)
{
	auto afterImageList = this->getAfterImageList();

	if (afterImageList) {

		//スイッチ「残像を表示」をチェック。
		CC_ASSERT(_baseObject);
		auto playObjectData = _baseObject->getPlayObjectData();
		auto switchData = playObjectData->getSwitchData(agtk::data::kObjectSystemSwitchDisplayAfterimage);
		if (!switchData->getValue()) {
			//残像を非表示にする。
			for (int i = 0; i < afterImageList->count(); i++) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
				auto afterImage = static_cast<agtk::ObjectAfterImage::AfterImage *>(afterImageList->getObjectAtIndex(i));
#else
				auto afterImage = dynamic_cast<agtk::ObjectAfterImage::AfterImage *>(afterImageList->getObjectAtIndex(i));
#endif
				afterImage->hide();
			}
			return;
		}

		// 各残像を更新する
		for (int i = 0; i < afterImageList->count(); i++) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
			auto afterImage = static_cast<agtk::ObjectAfterImage::AfterImage *>(afterImageList->getObjectAtIndex(i));
#else
			auto afterImage = dynamic_cast<agtk::ObjectAfterImage::AfterImage *>(afterImageList->getObjectAtIndex(i));
#endif
			afterImage->update(delta);
		}

		// 表示間隔の処理
		if (_intervalDuration300 > 0) {
			_intervalDuration300 -= delta * 300;
			if (_intervalDuration300 <= 0) {

				int idx = 0;

				// 残像表示処理
				for (int i = 0; i < afterImageList->count(); i++) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
					auto afterImage = static_cast<agtk::ObjectAfterImage::AfterImage *>(afterImageList->getObjectAtIndex(i));
#else
					auto afterImage = dynamic_cast<agtk::ObjectAfterImage::AfterImage *>(afterImageList->getObjectAtIndex(i));
#endif
					if (!afterImage->getVisible()) {
						idx = i;

						break;
					}
				}

				// リストを再構築
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
				auto pop = static_cast<agtk::ObjectAfterImage::AfterImage *>(afterImageList->getObjectAtIndex(idx));
#else
				auto pop = dynamic_cast<agtk::ObjectAfterImage::AfterImage *>(afterImageList->getObjectAtIndex(idx));
#endif
				// 対象の残像が表示中かもしれないので、一旦非表示化を行う
				pop->hide();

				// 残像を表示する
				pop->show();

				afterImageList->removeObjectAtIndex(idx, false);
				afterImageList->addObject(pop);

				// 表示間隔を再設定
				_intervalDuration300 += std::fmax(_baseObject->getObjectData()->getAfterimageInterval() * 300, _minIntervalDuration300);
				// ACT2-4944 ここで表示間隔が0以下になってしまうと、それ以降処理されなくなってしまうので初期値を入れておく
				if (_intervalDuration300 <= 0){
					_intervalDuration300 = 0.000001f;	//次のフレームで必ず残像が表示されるように。
				}
			}
		}

		// Zオーダーを再設定
		for (int i = 0; i < afterImageList->count(); i++) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
			auto afterImage = static_cast<agtk::ObjectAfterImage::AfterImage *>(afterImageList->getObjectAtIndex(i));
#else
			auto afterImage = dynamic_cast<agtk::ObjectAfterImage::AfterImage *>(afterImageList->getObjectAtIndex(i));
#endif
			afterImage->setLocalZOrder(-(i + 1));
		}
	}
}

void ObjectAfterImage::stop()
{
	auto afterImageList = this->getAfterImageList();

	if (afterImageList) {
		// 各残像を更新する
		for (int i = 0; i < afterImageList->count(); i++) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
			auto afterImage = static_cast<agtk::ObjectAfterImage::AfterImage *>(afterImageList->getObjectAtIndex(i));
#else
			auto afterImage = dynamic_cast<agtk::ObjectAfterImage::AfterImage *>(afterImageList->getObjectAtIndex(i));
#endif
			afterImage->stop();
		}
	}
}

void ObjectAfterImage::drawAfterimage(cocos2d::Renderer *renderer, cocos2d::Mat4 &m)
{
	auto afterImageList = this->getAfterImageList();

	if (afterImageList) {
		cocos2d::Ref *ref = nullptr;
		CCARRAY_FOREACH(afterImageList, ref) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
			auto afterimage = static_cast<agtk::ObjectAfterImage::AfterImage *>(ref);
#else
			auto afterimage = dynamic_cast<agtk::ObjectAfterImage::AfterImage *>(ref);
#endif
			afterimage->draw(renderer, m);
		}
	}
}

ObjectAfterImage::AfterImage::AfterImage()
{
	_playerActionNo = -1;
	_playerActionDirectNo = -1;
	_visible = false;
	_isUsePlayer = false;
	_isUseEffect = false;
	_isUseParticle = false;

	_baseObject = nullptr;
	_player = nullptr;
	_effectAnimation = nullptr;
	_particleGroup = nullptr;
}

ObjectAfterImage::AfterImage::~AfterImage()
{
	//CC_SAFE_RELEASE_NULL(_baseObject);//※このクラス内でretainしていないため不要。
	//CC_SAFE_RELEASE_NULL(_player);//※このクラス内でretainしていないため不要。
	CC_SAFE_RELEASE_NULL(_effectAnimation);
	CC_SAFE_RELEASE_NULL(_particleGroup);
}

bool ObjectAfterImage::AfterImage::init(agtk::Object * object)
{
	_baseObject = object;

	auto projectData = GameManager::getInstance()->getProjectData();
	CC_ASSERT(projectData);
	auto objectData = object->getObjectData();
	CC_ASSERT(objectData);

	// 残像にアニメーションを使用する場合
	if (objectData->getUseAnimationAfterimage()) {
		int id = objectData->getAfterimageAnimationId();

		// アニメーションデータ取得
		auto animationData = projectData->getAnimationData(objectData->getAfterimageAnimationId());
		if (animationData != nullptr) {

			// アニメーションの種類に応じて処理を変える
			switch (animationData->getType()) {

				// モーションアニメーションの場合
			case agtk::data::AnimationData::kMotion: {
				auto player = agtk::Player::create(animationData);
#ifdef USE_REDUCE_RENDER_TEXTURE
				_baseObject->addChild(player, Object::kPartPriorityAfterimage);
#else
				_baseObject->addChild(player);
#endif
				player->setVisible(false);
				_player = player;

				// 再生するモーションデータを設定する
				auto motionList = animationData->getMotionList();
				cocos2d::DictElement *el = nullptr;
				CCDICT_FOREACH(motionList, el) {

// #AGTK-NX
#ifdef STATIC_DOWN_CAST
					auto motion = static_cast<agtk::data::MotionData *>(el->getObject());
#else
					auto motion = dynamic_cast<agtk::data::MotionData *>(el->getObject());
#endif
					_playerActionNo = motion->getId();

					// 方向データを設定する
					auto dirList = motion->getDirectionList();
					cocos2d::DictElement *el2 = nullptr;
					CCDICT_FOREACH(dirList, el2) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
						auto dir = static_cast<agtk::data::DirectionData *>(el2->getObject());
#else
						auto dir = dynamic_cast<agtk::data::DirectionData *>(el2->getObject());
#endif

						_playerActionDirectNo = dir->getId();
						break;
					}
					break;
				}
			} break;

				// エフェクトアニメーションの場合
			case agtk::data::AnimationData::kEffect: {

				_isUseEffect = true;

				// ここでエフェクト生成を行わない
			} break;

				// パーティクルアニメーションの場合
			case agtk::data::AnimationData::kParticle: {

				_isUseParticle = true;

				// ここでパーティクル生成を行わない
			} break;

			}
		}
	}

	else {
		// アニメーションデータ取得
		auto animationData = projectData->getAnimationData(objectData->getAnimationId());

		if (animationData != nullptr) {
			auto player = agtk::Player::create(animationData);
#ifdef USE_REDUCE_RENDER_TEXTURE
			_baseObject->addChild(player, Object::kPartPriorityAfterimage);
#else
			_baseObject->addChild(player);
#endif
			player->setVisible(false);
			_player = player;
		}
	}

	return true;
}

void ObjectAfterImage::AfterImage::update(float delta)
{
	if (!_visible) { return; }

	if (_duration300 >= 0) {
		auto objectData = _baseObject->getObjectData();

		// 残像がフェードアウトする場合
		if (objectData->getFadeoutAfterimage()) {

			// 経過時間を取得
			int elapsedDuration = objectData->getAfterimageDuration300() - _duration300;

			// 経過時間がフェードアウト開始時間に到達した場合
			if (elapsedDuration >= objectData->getFadeoutAfterimageStart300())
			{
				// フェードを行う時間を取得
				int fadeDuration300 = objectData->getAfterimageDuration300() - objectData->getFadeoutAfterimageStart300();

				// ゼロ除算回避
				if (fadeDuration300 > 0)
				{
					float alpha = 1.0f - (elapsedDuration - objectData->getFadeoutAfterimageStart300()) / fadeDuration300;
					if (alpha <= 0.0f)
					{
						alpha = 0.0f;
					}

					// プレイヤーが設定済みで、使用許可時
					if (_player != nullptr && _isUsePlayer) {
						auto shader = _player->getShader(agtk::Shader::kShaderColorAfterimageRbga);
						if(shader == nullptr) {
							shader = _player->setShader(agtk::Shader::kShaderColorAfterimageRbga, 1);
						}
						if (shader) shader->setShaderAlpha(alpha);
					}

					// エフェクトアニメーションが設定されている場合
					if (EffectManager::getInstance()->existsEffect(_effectAnimation)) {
						_effectAnimation->setAlpha(alpha);
					}

					// パーティクルアニメーションが設定されている場合
					if (ParticleManager::getInstance()->existsParticleGroup(_particleGroup)) {
						_particleGroup->setAfterimageAlpha(alpha);
					}
				}
			}
		}

		// プレイヤーが設定済みで、使用許可時
		if (_player != nullptr && _isUsePlayer)
		{
			// モーションアニメを使用する場合はデルタタイムを、
			// アニメーションしない残像の場合は時間を設定しないで更新する
			float dt = objectData->getUseAnimationAfterimage() ? delta : 0;
			_player->update(dt);
		}

		_duration300 -= delta * 300;
		if (_duration300 <= 0) {
			hide();
		}
	}
}

void ObjectAfterImage::AfterImage::show()
{
	_visible = true;

	auto objectData = _baseObject->getObjectData();

	//　表示時間を設定
	_duration300 = objectData->getAfterimageDuration300();

	if (_player && _baseObject->getPlayer()) {
		_isUsePlayer = true;
		_player->setVisible(true);

		// autoGenerationNodeのパラメータを反映
		auto autoGenerationNode = _player->getAutoGenerationNode();
		autoGenerationNode->setRotation(_baseObject->getPlayer()->getAutoGenerationNode()->getRotation());
		autoGenerationNode->setScale(
			_baseObject->getPlayer()->getAutoGenerationNode()->getScaleX(),
			_baseObject->getPlayer()->getAutoGenerationNode()->getScaleY()
		);

		// 回転量を設定
		_player->setRotation(_baseObject->getPlayer()->getRotation());

		//回転の自動生成。
		auto autoGenerationCenterNode = _player->getAutoGenerationCenterNode();
		autoGenerationCenterNode->setRotation(_baseObject->getPlayer()->getAutoGenerationCenterNode()->getRotation());

		// 座標、スケール、回転量を設定
		_player->setPosition(_baseObject->getPlayer()->getPosition());
		_player->setScale(_baseObject->getPlayer()->getScaleX(), _baseObject->getPlayer()->getScaleY());

		if (_playerActionNo != -1 && _playerActionDirectNo != -1) {
			_player->play(_playerActionNo, _playerActionDirectNo, false, true);
			_player->update(0);
		}
		else {
			// 現在再生中のアニメーションを設定
			auto currentAction = _baseObject->getCurrentObjectAction();
			
			int actionNo = _baseObject->getCurrentAnimMotionId();
			int directionNo = -1;
			if (_baseObject->getPlayer() != nullptr) {
				directionNo = _baseObject->getPlayer()->getCurrentDirectionNo();
			}
			if(actionNo == -1 && directionNo == -1) {
				actionNo = currentAction->getObjectActionData()->getAnimMotionId();
				directionNo = _baseObject->getAnimDirectionId(objectData, currentAction->getObjectActionData());
			}

			// 残像の再生
			_player->play(actionNo, directionNo, false, true);
			
			// 残像にオブジェクトのアニメーション情報を渡す
			auto basePlayer = _baseObject->getBasePlayer();
			if (basePlayer) {
				auto baseMotion = basePlayer->getCurrentAnimationMotion();
				auto playerMotion = basePlayer->getCurrentAnimationMotion();
				playerMotion->_seconds = baseMotion->_seconds;
				playerMotion->setFrameDataNo(baseMotion->getFrameDataNo());
				playerMotion->_bFrameFirst = true;
			}

			// 残像を更新する
			_player->update(0);
		}


		// 指定色で塗る場合
		if (objectData->getFillAfterimage()) {
			auto shader = _player->setShader(agtk::Shader::kShaderColorAfterimageRbga, 1);
			if (shader) {
				shader->setShaderRgbaColor(cocos2d::Color4B(
					objectData->getFillAfterimageR(),
					objectData->getFillAfterimageG(),
					objectData->getFillAfterimageB(),
					objectData->getFillAfterimageA()
				));
			}
		}

		if (objectData->getFadeoutAfterimage()) {
			auto shader = _player->getShader(agtk::Shader::kShaderColorAfterimageRbga);
			if (shader == nullptr) {
				shader = _player->setShader(agtk::Shader::kShaderColorAfterimageRbga, 1);
			}
			if (shader) shader->setShaderAlpha(1.0f);
		}
	}
	else {
		// プレイヤーの使用を停止
		_isUsePlayer = false;
	}

	if (_isUseEffect) {
		// エフェクトを生成
		createEffect();
	}

	if (_isUseParticle) {
		// パーティクルを生成
		createParticle();
	}
}

void ObjectAfterImage::AfterImage::hide()
{
	_visible = false;
	_duration300 = 0;

	if (_player) {
		_player->setVisible(false);
	}

	if (EffectManager::getInstance()->existsEffect(_effectAnimation)) {
		// エフェクトを削除
		_effectAnimation->deleteEffect();
		_effectAnimation = nullptr;
	}

	if (ParticleManager::getInstance()->existsParticleGroup(_particleGroup)) {
		// パーティクルを停止させる
		_particleGroup->changeProccess(agtk::ParticleGroup::PARTICLE_PROC_TYPE::STOP);
		_particleGroup = nullptr;
	}
}

void ObjectAfterImage::AfterImage::setLocalZOrder(int z)
{
	if (_player) {
		_player->setLocalZOrder(z);
	}

	/*
	if (_particleGroup) {
		_particleGroup->setLocalZOrder(z);
	}
	*/
}

void ObjectAfterImage::AfterImage::createEffect()
{
	auto projectData = GameManager::getInstance()->getProjectData();
	CC_ASSERT(projectData);
	auto objectData = _baseObject->getObjectData();
	CC_ASSERT(objectData);

	// レイヤーIDがまだ設定されていない場合は処理しない
	if (_baseObject->getLayerId() < 0) { return; }

	// アニメーションデータ取得
	auto animationData = projectData->getAnimationData(objectData->getAfterimageAnimationId());

	// エフェクト生成
#ifdef USE_REDUCE_RENDER_TEXTURE
	_effectAnimation = EffectManager::getInstance()->addEffectAnimation(_baseObject, _baseObject->getPosition(), -1, animationData, true);
#else
	_effectAnimation = EffectManager::getInstance()->addEffectAnimation(_baseObject->getPosition(), _baseObject->getLayerId(), -1, animationData);
#endif

	// 指定色で塗る場合
	if (objectData->getFillAfterimage()) {

		// シェーダーを設定
		_effectAnimation->setFillColor(
			cocos2d::Color4B(
				objectData->getFillAfterimageR(),
				objectData->getFillAfterimageG(),
				objectData->getFillAfterimageB(),
				objectData->getFillAfterimageA()
			)
		);
	}
}

void ObjectAfterImage::AfterImage::createParticle()
{
	// パーティクルを使用しない、既に生成済みの場合は処理しない
	if (!_isUseParticle || ParticleManager::getInstance()->existsParticleGroup(_particleGroup)) {
		return;
	}

	auto projectData = GameManager::getInstance()->getProjectData();
	CC_ASSERT(projectData);
	auto objectData = _baseObject->getObjectData();
	CC_ASSERT(objectData);

	// レイヤーIDがまだ設定されていない場合は処理しない
	if (_baseObject->getLayerId() < 0) { return; }

	// アニメーションデータ取得
	auto animationData = projectData->getAnimationData(objectData->getAfterimageAnimationId());

	// パーティクルを生成
	auto particleList = animationData->getParticleList();

	// パーティクルグループ生成
	_particleGroup = ParticleManager::getInstance()->addParticle(
#ifdef USE_REDUCE_RENDER_TEXTURE
		_baseObject,
#else
		nullptr,
#endif
		_baseObject->getSceneIdOfFirstCreated(),
		_baseObject->getLayerId(),
		animationData->getId(),
		_baseObject->getPosition(),
		0, 300, true,
#ifdef USE_REDUCE_RENDER_TEXTURE
		particleList, true);
#else
		particleList);
#endif

	// 塗りつぶし、あるいはフェードアウトが設定されている場合
	if (objectData->getFillAfterimage() || objectData->getFadeoutAfterimage()) {
		// シェーダを設定
		_particleGroup->setShaderAfterimage();

		// 塗りつぶし設定時
		if (objectData->getFillAfterimage()) {
			_particleGroup->setAfterimageColor(
				cocos2d::Color4B(
					objectData->getFillAfterimageR(),
					objectData->getFillAfterimageG(),
					objectData->getFillAfterimageB(),
					objectData->getFillAfterimageA()
				)
			);
		}
	}
}

void ObjectAfterImage::AfterImage::stop()
{
	if (_particleGroup) {
		_particleGroup->changeProccess(agtk::ParticleGroup::PARTICLE_PROC_TYPE::STOP);
	}
}

void ObjectAfterImage::AfterImage::draw(cocos2d::Renderer *renderer, cocos2d::Mat4 &m)
{
	if (!_visible) { return; }

	// プレイヤーを表示
	if (_player != nullptr) {
		auto parent = _player->getParent();

		_player->setVisible(true);
		_player->visit(renderer, m * parent->getNodeToWorldTransform(), false);
		_player->setVisible(false);
	}

	// エフェクト、パーティクルはそれぞれの管理クラスで描画がなされる想定
}


//-------------------------------------------------------------------------------------------------------------------
ObjectSceneLoop::ObjectSceneLoop()
{
	_object = nullptr;
	_player = nullptr;
}

ObjectSceneLoop::~ObjectSceneLoop()
{
	CC_SAFE_RELEASE_NULL(_player);
}

bool ObjectSceneLoop::init(agtk::Object* object)
{
	if (object == nullptr) {
		return false;
	}

	_object = object;

	auto projectData = GameManager::getInstance()->getProjectData();
	CC_ASSERT(projectData);
	auto objectData = _object->getObjectData();
	CC_ASSERT(objectData);

	// アニメーションデータ取得
	auto animationData = projectData->getAnimationData(objectData->getAnimationId());

	if (animationData != nullptr) {
		// プレイヤーを生成
		auto player = agtk::Player::create(animationData);
		this->setPlayer(player);
		this->addChild(player);
	}

	_object->addChild(this);

	return true;
}

void ObjectSceneLoop::update(float delta)
{
	// オブジェクト未設定時、非表示化
	if (_object == nullptr) {
		setVisible(false);
		return;
	}

	if (_player != nullptr) {
		auto objectData = _object->getObjectData();
		CC_ASSERT(objectData);

		// 現在再生中のアニメーションを設定
		auto currentAction = _object->getCurrentObjectAction();

		int actionNo = currentAction->getObjectActionData()->getAnimMotionId();
		int directionNo = _object->getAnimDirectionId(objectData, currentAction->getObjectActionData());

		// プレイヤーの再生
		_player->play(actionNo, directionNo);

		// プレイヤーにオブジェクトのアニメーション情報を渡す
		auto baseMotion = _object->getBasePlayer()->getCurrentAnimationMotion();
		auto playerMotion = _player->getBasePlayer()->getCurrentAnimationMotion();
		playerMotion->_seconds = baseMotion->_seconds;
		playerMotion->setFrameDataNo(baseMotion->getFrameDataNo());
		playerMotion->_bFrameFirst = true;

		// プレイヤーを更新する
		_player->update(0);
	}
}


//-------------------------------------------------------------------------------------------------------------------
// オブジェクトシルエット
/**
* コンストラクタ
*/
ObjectSilhouetteImage::ObjectSilhouetteImage()
{
	_baseObject = nullptr;
	_player = nullptr;

	_playerActionNo = -1;
	_playerActionDirectNo = -1;
}

/**
* デストラクタ
*/
ObjectSilhouetteImage::~ObjectSilhouetteImage()
{
}

/**
* 初期化
* @param	object	参照元オブジェクト
* @return			初期化の成否
*/
bool ObjectSilhouetteImage::init(agtk::Object * object)
{
	auto projectData = GameManager::getInstance()->getProjectData();
	CC_ASSERT(projectData);
	auto objectData = object->getObjectData();
	CC_ASSERT(objectData);
	auto silhouetteData = objectData->getAroundCharacterViewSetting();
	CC_ASSERT(silhouetteData);

	// オブジェクト参照
	_baseObject = object;

	// アニメーションデータ取得
	auto animationData = projectData->getAnimationData(objectData->getAnimationId());

	// アニメーションデータがある場合
	if (animationData != nullptr) {
		// シルエット用プレイヤーを生成
		auto player = agtk::Player::create(animationData);
		_baseObject->addChild(player);
		player->setVisible(false);
		_player = player;

		// 初回更新
		update(0);
	}

	return true;
}

/**
* 更新
* @param	delta	前フレームからの経過時間
*/
void ObjectSilhouetteImage::update(float delta)
{
	if (_player != nullptr)
	{
		auto objectData = _baseObject->getObjectData();

		// autoGenerationNodeのパラメータを反映
		auto autoGenerationNode = _player->getAutoGenerationNode();

		auto player = _baseObject->getPlayer();
		if (player) {
			autoGenerationNode->setRotation(player->getAutoGenerationNode()->getRotation());
			autoGenerationNode->setScale(player->getAutoGenerationNode()->getScaleX(), player->getAutoGenerationNode()->getScaleY());

			// 回転量を設定
			_player->setRotation(player->getRotation());

			//回転の自動生成。
			auto autoGenerationCenterNode = _player->getAutoGenerationCenterNode();
			autoGenerationCenterNode->setRotation(player->getAutoGenerationCenterNode()->getRotation());

			// 座標、スケール、回転量を設定
			_player->setPosition(player->getPosition());
			_player->setScale(player->getScaleX(), player->getScaleY());
		}

		if (_playerActionNo != -1 && _playerActionDirectNo != -1) {
			_player->play(_playerActionNo, _playerActionDirectNo, false, true);
			updateShader();
			_player->update(0);
		}
		else {
			// 現在再生中のアニメーションを設定
			auto currentAction = _baseObject->getCurrentObjectAction();

			int actionNo = currentAction->getObjectActionData()->getAnimMotionId();
			int directionNo = _baseObject->getAnimDirectionId(objectData, currentAction->getObjectActionData());

			// 残像の再生
			_player->play(actionNo, directionNo, false, true);

			// 残像にオブジェクトのアニメーション情報を渡す
			if (_baseObject->getBasePlayer()) {
				auto baseMotion = _baseObject->getBasePlayer()->getCurrentAnimationMotion();
				auto basePlayer = _player->getBasePlayer();
				if (basePlayer != nullptr) {
					auto playerMotion = basePlayer->getCurrentAnimationMotion();
					playerMotion->_seconds = baseMotion->_seconds;
					playerMotion->setFrameDataNo(baseMotion->getFrameDataNo());
					playerMotion->_bFrameFirst = true;
				}
			}

			// シェーダー更新
			updateShader();

			// 残像を更新する
			_player->update(0);
		}
	}
}

/**
* シルエット用シェーダー更新
*/
void ObjectSilhouetteImage::updateShader()
{
	auto silhouetteData = _baseObject->getObjectData()->getAroundCharacterViewSetting();
	auto shaderKind = (silhouetteData->getMultiplyFill()) ? agtk::Shader::kShaderColorSilhouetteimageRbgaMultiply : agtk::Shader::kShaderColorSilhouetteimageRbga;
	auto shader = _player->getShader(shaderKind);

	if (shader == nullptr) {
		// 色を設定
		_player->setShader(shaderKind, 1);
		shader = _player->getShader(shaderKind);
		if (shader == nullptr) {
			//setShaderでshaderインスタンスが作成されなかった。
			return;
		}
		shader->setShaderRgbaColor(cocos2d::Color4B(
			silhouetteData->getFillR(),
			silhouetteData->getFillG(),
			silhouetteData->getFillB(),
			silhouetteData->getFillA()
		));

		BlendFunc func;

		// 乗算の場合
		if (silhouetteData->getMultiplyFill()) {
			// 乗算合成
			func = { GL_ZERO, GL_SRC_COLOR };
		}
		// 通常の場合
		else {
			// アルファブレンド合成
			func = { GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA };
		}

		// シルエットの合成方法を設定
		_player->getRenderTexture()->getLastRenderTexture()->getSprite()->setBlendFunc(func);
	}
}

/**
* 描画
* @param	renderer	レンダラー
* @param	viewMatrix	ビュー行列
*/
void ObjectSilhouetteImage::visit(Renderer *renderer, cocos2d::Mat4 viewMatrix)
{
	if (_player && _player->isVisible()) {
		
		_beforeVisit.init(_player->getGlobalZOrder());
		_beforeVisit.func = []() {
			glColorMask(1, 1, 1, 0);
		};
		renderer->addCommand(&_beforeVisit);

		_player->visit(renderer, viewMatrix, false);

		_afterVisit.init(_player->getGlobalZOrder());
		_afterVisit.func = []() {
			glColorMask(1, 1, 1, 1);
		};
		renderer->addCommand(&_afterVisit);
	}
}

/**
* 表示切り替え
* @param	isOn	表示のON/OFF
*/
void ObjectSilhouetteImage::setVisible(bool isOn)
{
	if (_player) {
		_player->setVisible(isOn);
	}
}

#if defined(AGTK_DEBUG)
void dumpObjectInformation()
{
	CCLOG("objectCount:%d", Object::_objectCount);
}
#endif

NS_AGTK_END
