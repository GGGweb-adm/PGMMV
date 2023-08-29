#include "Tile.h"
#include "Data/TileData.h"
#include "collision/CollisionComponent.hpp"
#include "collision/CollisionUtils.hpp"
#include "Manager/GameManager.h"
#include "Manager/AudioManager.h"
#include "Manager/EffectManager.h"
#include "Lib/Object.h"
#include "Lib/Camera.h"

#include "JavascriptManager.h"
#include "scripting/js-bindings/manual/ScriptingCore.h"
#include "Manager/DebugManager.h"

#define USE_MERGE_TILE_PHYSICS
#define FILLED_WALLBIT ((1 << agtk::data::TilesetData::Up) | (1 << agtk::data::TilesetData::Down) | (1 << agtk::data::TilesetData::Left) | (1 << agtk::data::TilesetData::Right))

NS_AGTK_BEGIN
//-------------------------------------------------------------------------------------------------------------------
Tile::Tile()
{
	_tilesetId = -1;
	_x = 0;
	_y = 0;
	_tileX = 0;
	_tileY = 0;
	_tileData = nullptr;
	_tileSize = cocos2d::Size::ZERO;
	_delta = 0.0f;
	_changeState = false;

	_touch = false;
	_touchTrigger = false;
	_touchRelease = false;
	_touchAttackBox = false;
	_touchCount = 0;
	_touchCountMax = 0;

	_type = kTypeTile;

	_objectOverlapped.set(false, nullptr);
	_sceneLayer = nullptr;

	_physicsMargedTileX = -1;
	_physicsMargedTileY = -1;
	_debugNode = nullptr;
	_removeFlag = false;
	_tilesetType = agtk::data::TilesetData::kMax;
	_beforeChangeGroup = -1;
}

Tile::~Tile()
{
	_objectOverlapped.reset();
	CC_SAFE_RELEASE_NULL(_tileData);
	//CC_SAFE_RELEASE_NULL(_sceneLayer);
	CC_SAFE_RELEASE_NULL(_debugNode);
}

Tile* Tile::create(data::Tile* tileData, cocos2d::Texture2D* texture, agtk::SceneLayer *sceneLayer, cocos2d::Size tileScale)
{
	Tile* ret = new (std::nothrow) Tile();
	if (ret && ret->init(tileData, texture, sceneLayer, tileScale))
	{
		ret->autorelease();
	}
	else
	{
		CC_SAFE_DELETE(ret);
	}
	return ret;
}

bool Tile::init(data::Tile* tile, cocos2d::Texture2D* texture, agtk::SceneLayer *sceneLayer, cocos2d::Size tileScale)
{
	if (!cocos2d::Sprite::initWithTexture(texture)) {
		return false;
	}
	_sceneLayer = sceneLayer;
	//CC_SAFE_RETAIN(_sceneLayer);

	auto sceneId = _sceneLayer->getSceneData()->getId();
	auto layerId = _sceneLayer->getLayerId();
	//sceneId
	this->setsceneId(sceneId);
	//layerId
	this->setLayerId(layerId);
	//tilesetId, x, y
	this->setTilesetId(tile->getTilesetId());
	this->setX(tile->getX());
	this->setY(tile->getY());
	this->setTileX((int)tile->getPosition().x);
	this->setTileY((int)tile->getPosition().y);
	this->setAnchorPoint(cocos2d::Vec2(0, 0));
	this->setTileScale(tileScale);
	//tileData
	auto project = GameManager::getInstance()->getProjectData();
	auto tileset = project->getTilesetData(tile->getTilesetId());
	auto tileData = tileset->getTileData(tile->getX() + tile->getY() * tileset->getHorzTileCount());
	this->setTileData(tileData);
	//tileSize
	this->setTileSize(project->getTileSize());
	float scaleFactor = Director::getInstance()->getContentScaleFactor();
	if (tile->getSubtileX(0, 0) < 0) {

		//uv setting
		this->setTextureRect(cocos2d::Rect(
			tile->getX() * this->getTileSize().width * scaleFactor,
			tile->getY() * this->getTileSize().height * scaleFactor,
			this->getTileSize().width * tileScale.width * scaleFactor,
			this->getTileSize().height * tileScale.height * scaleFactor
		));
		//argb
		if (tileData != nullptr) {
			this->setOpacity(tileData->getA());
			this->setColor(cocos2d::Color3B(tileData->getR(), tileData->getG(), tileData->getB()));
		}
	}
	else {
		this->setOpacity(0);
		this->setContentSize(this->getTileSize());
		//uv setting
		for (int j = 0; j < data::Tile::VertSubtileCount; j++) {
			for (int i = 0; i < data::Tile::HorzSubtileCount; i++) {
				auto subtile = Subtile::create(tile, texture, layerId);
				subtile->setTilesetId(tile->getTilesetId());
				subtile->setX(tile->getSubtileX(i, j));
				subtile->setY(tile->getSubtileY(i, j));
				subtile->setAnchorPoint(cocos2d::Vec2(0, 0));
				subtile->setPosition(
					this->getTileSize().width * scaleFactor / 2 * i,
					this->getTileSize().height * scaleFactor / 2 * (data::Tile::VertSubtileCount - 1 - j)
				);
				//uv setting
				subtile->setTextureRect(cocos2d::Rect(
					tile->getSubtileX(i, j) * this->getTileSize().width * scaleFactor / 2,
					tile->getSubtileY(i, j) * this->getTileSize().height * scaleFactor / 2,
					this->getTileSize().width * scaleFactor / 2,
					this->getTileSize().height * scaleFactor / 2
				));
				//argb
				if (tileData != nullptr) {
					subtile->setOpacity(tileData->getA());
					subtile->setColor(cocos2d::Color3B(tileData->getR(), tileData->getG(), tileData->getB()));
				}
				this->addChild(subtile);
				subtile->unscheduleUpdate();//※ループ一元化のためここで破棄する。
			}
		}
	}
	this->setTilesetType(tileset->getTilesetType());
	
	this->setChangeState(false);

	return true;
}

void Tile::update(float delta)
{
	if (this->getRemoveFlag()) {
		return;
	}
	_beforeChangeGroup = -1;
	auto tileData = this->getTileData();
	if (!tileData) {
		return;
	}
	_delta += delta;

	//アニメーション ----------------------------------------------------------------------------------------
	auto anim = tileData->getTileAnimationData();
	if (anim) {
		int maxFrame = tileData->getTileAnimationMaxFrame300();
		int curFrame = (int)(_delta * 300) % maxFrame;
		int cntTime = 0;
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
		auto p0 = static_cast<agtk::data::TileAnimationData *>(anim->getObjectAtIndex(0));
#else
		auto p0 = dynamic_cast<agtk::data::TileAnimationData *>(anim->getObjectAtIndex(0));
#endif
		for (int i = 0; i < anim->count(); i++) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
			auto p = static_cast<agtk::data::TileAnimationData *>(anim->getObjectAtIndex(i));
#else
			auto p = dynamic_cast<agtk::data::TileAnimationData *>(anim->getObjectAtIndex(i));
#endif
			int frame = p->getFrame300();
			if (cntTime <= curFrame && curFrame < cntTime + frame) {
				auto sz = this->getTileSize();
				auto tileScale = this->getTileScale();
				sz.width = sz.width * tileScale.width;
				sz.height = sz.height * tileScale.height;
				auto &children = this->getChildren();
				if (children.size() > 1) {
					for (int j = 0; j < data::Tile::VertSubtileCount; j++) {
						for (int i = 0; i < data::Tile::HorzSubtileCount; i++) {
							auto subtile = dynamic_cast<Subtile *>(children.at(i + j * data::Tile::HorzSubtileCount));
							if (subtile == nullptr) continue;
							subtile->setTextureRect(cocos2d::Rect(
								(subtile->getX() + (p->getX() - p0->getX()) * 2) * sz.width / 2,
								(subtile->getY() + (p->getY() - p0->getY()) * 2) * sz.height / 2,
								sz.width / 2,
								sz.height / 2
							));
						}
					}
				}
				else {
					this->setTextureRect(cocos2d::Rect(
						p->getX() * sz.width,
						p->getY() * sz.height,
						sz.width,
						sz.height
					));
			}
				break;
			}
			cntTime += frame;
		}
	}

	//プレイヤーのTouch,Release情報を更新する
	//_touchTriggerに毎回チェックを入れる。
	bool touchTrigger = (_touchTrigger && !_touch);
	if (_touchTrigger) {
		if (_touch == false) {
			_touch = true;
			_touchRelease = false;
		}
		if (_touchCount > _touchCountMax) {
			_touchCountMax = _touchCount;
			if (_touch && !touchTrigger) {
				touchTrigger = true;
			}
		}
		else if (_touchCount < _touchCountMax) {
			_touchCountMax = _touchCount;
			if (_touch) {
				_touchRelease = true;
			}
		}
		_touchTrigger = false;
	}
	else {
		if (_touch) {
			_touch = false;
			_touchRelease = true;
			_touchCountMax = 0;
		}
	}
	_touchCount = 0;

	//変更条件 ----------------------------------------------------------------------------------------------
	bool isChangeState = false;

	//プレイヤーが触れたら変更する
	if (tileData->getPlayerMoveType() == agtk::data::TileData::kPlayerMoveTouched && touchTrigger) {
		isChangeState = true;
	}
	//プレイヤーが離れたら変更する
	else if (tileData->getPlayerMoveType() == agtk::data::TileData::kPlayerMoveReleased && _touchRelease) {
		isChangeState = true;
	}
	// プレイヤーが重なったら変更する
	else if (tileData->getPlayerMoveType() == agtk::data::TileData::kPlayerMoveOverlapped && _objectOverlapped.flag()) {
		isChangeState = true;
	}
	//設定無し
	else if (tileData->getPlayerMoveType() == agtk::data::TileData::kPlayerMoveNone) {
		isChangeState = true;
	}

	// == 以下は AND 条件 ========================================
	//タイルに当たる攻撃判定が触れたら
	if (tileData->getTileAttackAreaTouched()) {
		isChangeState &= _touchAttackBox;
	}
	
	//時間経過で状態変更
	if (tileData->getTimePassed()) {
		isChangeState &= (tileData->getPassedTime() < _delta * 300);
	}

	//スイッチ、変数条件で変更
	if (tileData->getSwitchVariableCondition()) {
		isChangeState &= this->checkSwitchVariableCondition(tileData->getSwitchVariableConditionList());
	}

	this->setChangeState(isChangeState);

	//変更後の状態 ------------------------------------------------------------------------------------------
	auto scene = GameManager::getInstance()->getCurrentScene();
	if (this->getChangeState() && (this->getTilesetType() == agtk::data::TilesetData::kGimmick)) {

		//※シーン遷移中にタイル変化を仕様とする。2018.12.13 sakihama-h
		// if (this->getChangeState() && !scene->getIgnoredUpdateActionFlag()) {

		//SEを鳴らす。
		if (tileData->getPlaySe()) {
			AudioManager::getInstance()->playSe(tileData->getPlaySeId());
		}
		//オブジェクトが出現。
#if defined(USE_RUNTIME)
		auto projectData = GameManager::getInstance()->getProjectData();
		if (tileData->getAppearObject() && tileData->getAppearObjectId() >= 0 && !projectData->getObjectData(tileData->getAppearObjectId())->getTestplayOnly()) {
#else
		if (tileData->getAppearObject() && tileData->getAppearObjectId() >= 0) {
#endif
			// シーンレイヤーIDとシーンレイヤー取得
			auto sceneLayer = _sceneLayer;

			int moveDirection = -1;

			//オブジェクト生成
			auto object = Object::create(
				sceneLayer,
				tileData->getAppearObjectId(),
				-1,//initialActionId
				scene->getPositionSceneFromCocos2d(this->getAnchoredPosition(), scene->getSceneData()),
				cocos2d::Vec2(1, 1),//scale
				0,//rotation
				moveDirection//向き
				, -1, -1, -1
			);

			sceneLayer->addCollisionDetaction(object);
			object->setId(sceneLayer->publishObjectId());
			object->getPlayObjectData()->setInstanceId(scene->getObjectInstanceId(object));
			object->getPlayObjectData()->setInstanceCount(scene->incrementObjectInstanceCount(object->getObjectData()->getId()));
			scene->updateObjectInstanceCount(object->getObjectData()->getId());
			object->setLayerId(sceneLayer->getLayerId());
			object->setPhysicsBitMask(sceneLayer->getLayerId(), sceneLayer->getSceneData()->getId());
			sceneLayer->addObject(object);

			// オブジェクトに紐付いた物理オブジェクトを生成
			sceneLayer->createPhysicsObjectWithObject(object);

			auto newObjectData = object->getObjectData();
			auto viewportLightSceneLayer = scene->getViewportLight()->getViewportLightSceneLayer(this->getLayerId());
			if (viewportLightSceneLayer && newObjectData->getViewportLightSettingFlag() && newObjectData->getViewportLightSettingList()->count() > 0) {
				auto viewportLightObject = ViewportLightObject::create(object, scene->getViewportLight(), sceneLayer);
				viewportLightSceneLayer->getViewportLightObjectList()->addObject(viewportLightObject);
			}
		}

		//エフェクトを出現
		if (tileData->getShowEffect()) {
			// ワーク変数
			auto projectData = GameManager::getInstance()->getInstance()->getProjectData();
			auto scene = GameManager::getInstance()->getCurrentScene();

			// エフェクトの生成対象となるシーンレイヤID取得
			int sceneLayerId = this->getLayerId();

			// 再生するアニメーションデータを取得
			auto animationData = projectData->getAnimationData(tileData->getShowEffectId());

			// エフェクト生成
			EffectManager::getInstance()->addEffectAnimation(
				scene->getPositionSceneFromCocos2d(this->getAnchoredPosition(), scene->getSceneData()),
				sceneLayerId,
				-1,
				animationData
			);
		}

		// パーティクルを出現
		if (tileData->getShowParticle() && tileData->getShowParticleId() >= 0) {
			// ワーク変数
			auto projectData = GameManager::getInstance()->getProjectData();
			auto scene = GameManager::getInstance()->getCurrentScene();
			
			// 生成されるパーティクルリストを取得
			auto particleList = projectData->getAnimationData(tileData->getShowParticleId())->getParticleList();

			// パーティクル生成
			ParticleManager::getInstance()->addParticle(
				nullptr,
				this->getsceneId(),
				this->getLayerId(),
				tileData->getShowParticleId(),
				agtk::Scene::getPositionSceneFromCocos2d(this->getAnchoredPosition()),
				0,
				0,
				true,
				particleList
#ifdef USE_REDUCE_RENDER_TEXTURE
				, false
#endif
			);
		}

		//スイッチ、変数を変更。
		bool bSwitchVariableChanged = false;
		if (tileData->getSwitchVariableAssign()) {
			this->changeSwitchVariableAssign(tileData->getSwitchVariableAssignList());
			bSwitchVariableChanged = true;
		}

		//タイルを変更する。
		switch (tileData->getChangeTileType()) {
		case agtk::data::TileData::kChangeTileDisappear://タイルを消滅させる
		{
			// タイルマップからこのタイルの参照を削除
			auto scene = GameManager::getInstance()->getCurrentScene();
			auto sceneLayer = _sceneLayer;

			auto tileMapList = sceneLayer->getTileMapList();
			if (tileMapList) {
				cocos2d::Ref *ref = nullptr;
				CCARRAY_FOREACH(tileMapList, ref) {
					auto tileMap = dynamic_cast<agtk::TileMap*>(ref);
					if (tileMap) {
						cocos2d::Size size = this->getTileSize();
						tileMap->removeTileReference(this->getTileX(), this->getTileY(), &size);
						tileMap->removeTile2Reference(this->getTileX(), this->getTileY(), &size);
					}
				}
			}

			auto tileMap = dynamic_cast<agtk::TileMap *>(getParent()->getParent());
			if (tileMap) {
				tileMap->getChildrenList()->removeObject(this);
			}
			auto debugNode = this->getDebugNode();
			if (debugNode) {
				debugNode->removeFromParent();
			}
			// それから親ノードから削除する
			this->removeFromParent();
			this->setRemoveFlag(true);
		}break;
		case agtk::data::TileData::kChangeTileNone://タイルを変更しない
			break;
		case agtk::data::TileData::kChangeTile: {//タイルを変更する
			auto project = GameManager::getInstance()->getProjectData();
			auto tileset = project->getTilesetData(this->getTilesetId());
			_beforeChangeGroup = this->getTileData()->getGroup();
			auto data = tileset->getTileData(tileData->getChangeTileX() + tileData->getChangeTileY() * tileset->getHorzTileCount());

			auto sz = this->getTileSize();
			float scaleFactor = Director::getInstance()->getContentScaleFactor();
			this->setTextureRect(cocos2d::Rect(
				tileData->getChangeTileX() * this->getTileSize().width * scaleFactor,
				tileData->getChangeTileY() * this->getTileSize().height * scaleFactor,
				this->getTileSize().width * scaleFactor,
				this->getTileSize().height * scaleFactor
			));
			this->setX(tileData->getChangeTileX());
			this->setY(tileData->getChangeTileY());
			this->setTileData(data);
			if (data) {
				this->setColor(cocos2d::Color3B(data->getR(), data->getG(), data->getB()));
			}
			else {
				this->setColor(cocos2d::Color3B(255, 255, 255));
			}
			this->setDelta(0.0);

			//壁判定設定。
			auto sceneLayer = _sceneLayer;
			auto tileMapList = sceneLayer->getTileMapList();
			if (tileMapList) {
				cocos2d::Ref *ref = nullptr;
				CCARRAY_FOREACH(tileMapList, ref) {
					auto tileMap = dynamic_cast<agtk::TileMap*>(ref);
					if (tileMap) {
// #AGTK-NX
#if CC_TARGET_PLATFORM != CC_PLATFORM_NX
						tileMap->removeTileReference(this->getTileX(), this->getTileY(), &this->getTileSize());
						tileMap->removeTile2Reference(this->getTileX(), this->getTileY(), &this->getTileSize());
#else
#endif
						tileMap->addTileReference(this);
					}
				}
			}

			//「タイルの壁判定を表示」更新。
			auto debugNode = this->getDebugNode();
			if (debugNode) {
				auto visible = debugNode->isVisible();
				debugNode->removeFromParent();
				this->setDebugNode(nullptr);
				this->showDebugVisible(visible);
			}

			break; }
		default:CC_ASSERT(0);
		}
		if (bSwitchVariableChanged) {
			GameManager::getInstance()->updateObjectVariableAndSwitch();
		}
	}

	_objectOverlapped.reset();
	this->setChangeState(false);
	_touchRelease = false;
	this->setTouchAttackBox(false);
}

bool Tile::checkSwitchVariableCondition(cocos2d::__Array *switchVariableConditionList)
{
	auto projectPlayData = GameManager::getInstance()->getPlayData();
	auto scene = GameManager::getInstance()->getCurrentScene();
	cocos2d::Ref *ref = nullptr;
	CCARRAY_FOREACH(switchVariableConditionList, ref) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
		auto condition = static_cast<agtk::data::TileSwitchVariableConditionData *>(ref);
#else
		auto condition = dynamic_cast<agtk::data::TileSwitchVariableConditionData *>(ref);
#endif
		//スイッチ
		if (condition->getSwtch()) {
			std::function<bool(agtk::data::PlaySwitchData *)> checkSwitchCondition = [&condition](agtk::data::PlaySwitchData *switchData) {
				if (switchData == nullptr) {
					return false;
				}
				switch (condition->getSwitchValue()) {
				case 0: {//ON
					if (switchData->getValue() == true) return true;
					break; }
				case 1: {//OFF
					if (switchData->getValue() == false) return true;
					break; }
				case 2: {//OFF->ON
					if (switchData->isState() == agtk::data::PlaySwitchData::kStateOnFromOff) return true;
					break; }
				case 3: {//ON->OFF
					if (switchData->isState() == agtk::data::PlaySwitchData::kStateOffFromOn) return true;
					break; }
				}
				return false;
			};
			if (condition->getSwitchObjectId() == agtk::data::TileSwitchVariableConditionData::kTouchedObject) {//触れているオブジェクト
				switch (condition->getSwitchQualifierId()) {
				case agtk::data::TileSwitchVariableConditionData::kQualifierSingle: {//単体
					bool bCheck = false;
					auto objectList = scene->getObjectAll(this->getBoundingBox(), _sceneLayer->getType());
					cocos2d::Ref *ref = nullptr;
					CCARRAY_FOREACH(objectList, ref) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
						auto object = static_cast<agtk::Object *>(ref);
#else
						auto object = dynamic_cast<agtk::Object *>(ref);
#endif
						auto objectData = object->getObjectData();
						auto instanceId = projectPlayData->getVariableData(objectData->getId(), agtk::data::kObjectSystemVariableSingleInstanceID)->getValue();
						if (object->getInstanceId() != (int)instanceId) {
							continue;
						}
						auto playObjectData = object->getPlayObjectData();
						auto switchData = playObjectData->getSwitchData(condition->getSwitchId());
						if (checkSwitchCondition(switchData)) {
							bCheck = true;
							break;
						}
					}
					if (!bCheck) return false;
					break; }
				case agtk::data::TileSwitchVariableConditionData::kQualifierWhole: {//全体
					bool bCheck = false;
					auto objectList = scene->getObjectAll(this->getBoundingBox(), _sceneLayer->getType());
					cocos2d::Ref *ref = nullptr;
					CCARRAY_FOREACH(objectList, ref) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
						auto object = static_cast<agtk::Object *>(ref);
#else
						auto object = dynamic_cast<agtk::Object *>(ref);
#endif
						if (_objectOverlapped.flag() == false) continue;
						if (_objectOverlapped.object() != object) continue;
						auto playObjectData = object->getPlayObjectData();
						auto switchData = playObjectData->getSwitchData(condition->getSwitchId());
						if (switchData == nullptr) continue;
						if (checkSwitchCondition(switchData)) {
							bCheck = true;
							break;
						}
					}
					if (!bCheck) return false;
					break; }
				default:CC_ASSERT(0);
				}
			}
			else if (condition->getSwitchObjectId() == agtk::data::PlayData::COMMON_PLAY_DATA_ID) {//Resources 共通
				auto switchData = projectPlayData->getSwitchData(agtk::data::PlayData::COMMON_PLAY_DATA_ID, condition->getSwitchId());
				if (checkSwitchCondition(switchData) == false) {
					return false;
				}
			}
			else if (condition->getSwitchObjectId() > 0) {//オブジェクト
				switch (condition->getSwitchQualifierId()) {
				case agtk::data::TileSwitchVariableConditionData::kQualifierSingle: {//単体
					bool bCheck = false;
// #AGTK-NX
#ifdef USE_SAR_OPTIMIZE_3
					int instanceId = (int)projectPlayData->getVariableData(condition->getSwitchObjectId(), agtk::data::kObjectSystemVariableSingleInstanceID)->getValue();
					if (instanceId >= 0) {
						auto object = scene->getObjectInstance(condition->getSwitchObjectId(), instanceId, _sceneLayer->getType());
						if (object)
						{
							auto playObjectData = object->getPlayObjectData();
							auto switchData = playObjectData->getSwitchData(condition->getSwitchId());
							if (checkSwitchCondition(switchData)) {
								bCheck = true;
							}
						}
					}
#else
					auto objectList = scene->getObjectAll(condition->getSwitchObjectId(), _sceneLayer->getType());
					int instanceId = (int)projectPlayData->getVariableData(condition->getSwitchObjectId(), agtk::data::kObjectSystemVariableSingleInstanceID)->getValue();
					cocos2d::Ref *ref = nullptr;
					CCARRAY_FOREACH(objectList, ref) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
						auto object = static_cast<agtk::Object *>(ref);
#else
						auto object = dynamic_cast<agtk::Object *>(ref);
#endif
						if (object->getInstanceId() == instanceId) {
							auto playObjectData = object->getPlayObjectData();
							auto switchData = playObjectData->getSwitchData(condition->getSwitchId());
							if (checkSwitchCondition(switchData)) {
								bCheck = true;
								break;
							}
						}
					}
#endif
					if (!bCheck) return false;
					break; }
				case agtk::data::TileSwitchVariableConditionData::kQualifierWhole: {//全体
					bool bCheck = false;
					auto objectList = scene->getObjectAll(condition->getSwitchObjectId(), _sceneLayer->getType());
					cocos2d::Ref *ref;
					CCARRAY_FOREACH(objectList, ref) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
						auto object = static_cast<agtk::Object *>(ref);
#else
						auto object = dynamic_cast<agtk::Object *>(ref);
#endif
						if (_objectOverlapped.flag() == false) continue;
						if (_objectOverlapped.object() != object) continue;
						auto playObjectData = object->getPlayObjectData();
						auto switchData = playObjectData->getSwitchData(condition->getSwitchId());
						if (checkSwitchCondition(switchData)) {
							bCheck = true;
							break;
						}
					}
					if (!bCheck) return false;
					break; }
				default:CC_ASSERT(0);
				}
			}
			else {
				//エラー
				CC_ASSERT(0);
			}
		}
		//変数
		else {
			std::function<bool(agtk::data::PlayVariableData *)> checkVariableCondition = [&](agtk::data::PlayVariableData *variableData) {
				auto projectPlayData = GameManager::getInstance()->getPlayData();
				auto scene = GameManager::getInstance()->getCurrentScene();
				double compareValue = 0;
				switch (condition->getCompareValueType()) {
				case agtk::data::TileSwitchVariableConditionData::kCompareValue: {//定数
					compareValue = condition->getComparedValue();
					break; }
				case agtk::data::TileSwitchVariableConditionData::kCompareVariable: {//変数
#if 1
					if (!GameManager::getInstance()->getVariableValue(condition->getComparedVariableObjectId(), condition->getComparedVariableQualifierId(), condition->getComparedVariableId(), compareValue, nullptr)) {
						return false;
					}
#else
					if (condition->getComparedVariableObjectId() == agtk::data::PlayData::COMMON_PLAY_DATA_ID) {//Resources共通
						auto compareVariableData = projectPlayData->getVariableData(agtk::data::PlayData::COMMON_PLAY_DATA_ID, condition->getComparedVariableId());
						if (compareVariableData == nullptr) {
							return false;
						}
						compareValue = compareVariableData->getValue();
					}
					else if (condition->getComparedVariableObjectId() > 0) {//オブジェクト
						CC_ASSERT(condition->getComparedVariableQualifierId() == agtk::data::TileSwitchVariableConditionData::kQualifierSingle);
						auto objectList = scene->getObjectAll(condition->getVariableObjectId(), _sceneLayer->getType());
						int instanceId = (int)projectPlayData->getVariableData(condition->getVariableObjectId(), agtk::data::kObjectSystemVariableSingleInstanceID)->getValue();
						cocos2d::Ref *ref = nullptr;
						agtk::data::PlayVariableData *compareVariableData = nullptr;
						CCARRAY_FOREACH(objectList, ref) {
							auto object = dynamic_cast<agtk::Object *>(ref);
							if (object->getInstanceId() == instanceId) {
								auto playObjectData = object->getPlayObjectData();
								compareVariableData = playObjectData->getVariableData(condition->getComparedVariableId());
							}
						}
						if (compareVariableData == nullptr) {
							return false;
						}
						compareValue = compareVariableData->getValue();
					}
					else {
						//エラー
						CC_ASSERT(0);
					}
#endif
					break; }
				case agtk::data::TileSwitchVariableConditionData::kCompareNaN: {
					compareValue = NAN;
					break; }
				default:CC_ASSERT(0);
				}

				if (variableData == nullptr) {
					return false;
				}

				bool src_isnan = std::isnan(variableData->getValue());
				bool compare_isnan = std::isnan(compareValue);
				// 比較演算子タイプ別処理
				auto op = condition->getCompareOperator();
				if (src_isnan || compare_isnan) {
					//どちらかが非数なら特別処理。
					switch (op) {
					case 0:// "<"
						break;
					case 1:// "<="
						if (src_isnan && compare_isnan) return true;
						break;
					case 2:// "="
						if (src_isnan && compare_isnan) return true;
						break;
					case 3:// ">="
						if (src_isnan && compare_isnan) return true;
						break;
					case 4:// ">"
						break;
					case 5:// "!="
						if (src_isnan != compare_isnan) return true;
						break;
					default:CC_ASSERT(0);
					}
				}
				else {
					switch (op) {
					case 0:// "<"
						if (variableData && variableData->getValue() < compareValue) return true;
						break;
					case 1:// "<="
						if (variableData && variableData->getValue() <= compareValue) return true;
						break;
					case 2:// "="
						if (variableData && variableData->getValue() == compareValue) return true;
						break;
					case 3:// ">="
						if (variableData && variableData->getValue() >= compareValue) return true;
						break;
					case 4:// ">"
						if (variableData && variableData->getValue() > compareValue) return true;
						break;
					case 5:// "!="
						if (variableData && variableData->getValue() != compareValue) return true;
						break;
					default:CC_ASSERT(0);
					}
				}
				return false;
			};
			if (condition->getVariableObjectId() == agtk::data::TileSwitchVariableConditionData::kTouchedObject) {//触れているオブジェクト
				switch (condition->getVariableQualifierId()) {
				case agtk::data::TileSwitchVariableConditionData::kQualifierSingle: {//単体
					bool bCheck = false;
					auto objectList = scene->getObjectAll(this->getBoundingBox(), _sceneLayer->getType());
					cocos2d::Ref *ref = nullptr;
					CCARRAY_FOREACH(objectList, ref) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
						auto object = static_cast<agtk::Object *>(ref);
#else
						auto object = dynamic_cast<agtk::Object *>(ref);
#endif
						auto objectData = object->getObjectData();
						auto instanceId = projectPlayData->getVariableData(objectData->getId(), agtk::data::kObjectSystemVariableSingleInstanceID)->getValue();
						if (object->getInstanceId() != (int)instanceId) {
							continue;
						}
						auto playObjectData = object->getPlayObjectData();
						auto variableData = playObjectData->getVariableData(condition->getVariableId());
						if (checkVariableCondition(variableData)) {
							bCheck = true;
							break;
						}
					}
					if (!bCheck) return false;
					break; }
				case agtk::data::TileSwitchVariableConditionData::kQualifierWhole: {//全体
					bool bCheck = false;
					auto objectList = scene->getObjectAll(this->getBoundingBox(), _sceneLayer->getType());
					cocos2d::Ref *ref = nullptr;
					CCARRAY_FOREACH(objectList, ref) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
						auto object = static_cast<agtk::Object *>(ref);
#else
						auto object = dynamic_cast<agtk::Object *>(ref);
#endif
						if (_objectOverlapped.flag() == false) continue;
						if (_objectOverlapped.object() != object) continue;
						auto playObjectData = object->getPlayObjectData();
						auto variableData = playObjectData->getVariableData(condition->getVariableId());
						if (variableData == nullptr) continue;
						if (checkVariableCondition(variableData)) {
							bCheck = true;
							break;
						}
					}
					if (!bCheck) return false;
					break; }
				default:CC_ASSERT(0);
				}
			}
			else if (condition->getVariableObjectId() == agtk::data::PlayData::COMMON_PLAY_DATA_ID) {//Resources 共通
				auto variableData = projectPlayData->getVariableData(agtk::data::PlayData::COMMON_PLAY_DATA_ID, condition->getVariableId());
				if (checkVariableCondition(variableData) == false) {
					return false;
				}
			}
			else if (condition->getVariableObjectId() > 0) {//オブジェクト
				switch (condition->getVariableQualifierId()) {
				case agtk::data::TileSwitchVariableConditionData::kQualifierSingle: {//単体
					bool bCheck = false;
					auto variableId = condition->getVariableId();
					if (variableId == agtk::data::kObjectSystemVariableSingleInstanceID || variableId == agtk::data::kObjectSystemVariableInstanceCount) {
						auto variableData = projectPlayData->getVariableData(condition->getVariableObjectId(), variableId);
						if (checkVariableCondition(variableData)) {
							bCheck = true;
						}
					}
					else {
// #AGTK-NX
#ifdef USE_SAR_OPTIMIZE_3
						int instanceId = (int)projectPlayData->getVariableData(condition->getVariableObjectId(), agtk::data::kObjectSystemVariableSingleInstanceID)->getValue();
						if (instanceId >= 0) {
							auto object = scene->getObjectInstance(condition->getVariableObjectId(), instanceId, _sceneLayer->getType());
							if (object)
							{
								auto playObjectData = object->getPlayObjectData();
								auto variableData = playObjectData->getVariableData(variableId);
								if (checkVariableCondition(variableData)) {
									bCheck = true;
								}
							}
						}
#else
						auto objectList = scene->getObjectAll(condition->getVariableObjectId(), _sceneLayer->getType());
						int instanceId = (int)projectPlayData->getVariableData(condition->getVariableObjectId(), agtk::data::kObjectSystemVariableSingleInstanceID)->getValue();
						cocos2d::Ref *ref = nullptr;
						CCARRAY_FOREACH(objectList, ref) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
							auto object = static_cast<agtk::Object *>(ref);
#else
							auto object = dynamic_cast<agtk::Object *>(ref);
#endif
							if (object->getInstanceId() == instanceId) {
								auto playObjectData = object->getPlayObjectData();
								auto variableData = playObjectData->getVariableData(variableId);
								if (checkVariableCondition(variableData)) {
									bCheck = true;
									break;
								}
							}
						}
#endif
					}
					if (!bCheck) return false;
					break; }
				case agtk::data::TileSwitchVariableConditionData::kQualifierWhole: {//全体
					bool bCheck = false;
					auto variableId = condition->getVariableId();
					if (variableId == agtk::data::kObjectSystemVariableSingleInstanceID || variableId == agtk::data::kObjectSystemVariableInstanceCount) {
						auto variableData = projectPlayData->getVariableData(condition->getVariableObjectId(), variableId);
						if (checkVariableCondition(variableData)) {
							bCheck = true;
							break;
						}
					}
					else {
						auto objectList = scene->getObjectAll(condition->getVariableObjectId(), _sceneLayer->getType());
						cocos2d::Ref *ref = nullptr;
						CCARRAY_FOREACH(objectList, ref) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
							auto object = static_cast<agtk::Object *>(ref);
#else
							auto object = dynamic_cast<agtk::Object *>(ref);
#endif
							if (_objectOverlapped.flag() == false) continue;
							if (_objectOverlapped.object() != object) continue;
							auto variableData = object->getPlayObjectData()->getVariableData(variableId);
							if (checkVariableCondition(variableData)) {
								bCheck = true;
								break;
							}
						}
					}
					if (!bCheck) return false;
					break; }
				default:CC_ASSERT(0);
				}
			}
		}
	}
	return true;
}

void Tile::changeSwitchVariableAssign(cocos2d::__Array *switchVariableAssignList)
{
	auto projectPlayData = GameManager::getInstance()->getPlayData();
	auto scene = GameManager::getInstance()->getCurrentScene();
	bool result = true;
	cocos2d::Ref *ref = nullptr;
	int index = -1;
	bool isAllLayerObject = false;
	if (this->getTileData()->getPlayerMoveType() == agtk::data::TileData::kPlayerMoveOverlapped) {
		// 「オブジェクトがタイルに重なったら」の場合は全レイヤーから取得
		isAllLayerObject = true;
	}
	CCARRAY_FOREACH(switchVariableAssignList, ref) {
		index++;
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
		auto assign = static_cast<agtk::data::TileSwitchVariableAssignData *>(ref);
#else
		auto assign = dynamic_cast<agtk::data::TileSwitchVariableAssignData *>(ref);
#endif
		//スイッチ変更
		if (assign->getSwtch()) {
			std::function<void(agtk::data::PlaySwitchData *)> setSwitchData = [&assign](agtk::data::PlaySwitchData *switchData) {
				switch (assign->getSwitchValue()) {
				case agtk::data::TileSwitchVariableAssignData::kSwitchAssignOn:
					switchData->setValue(true);
					break;
				case agtk::data::TileSwitchVariableAssignData::kSwitchAssignOff:
					switchData->setValue(false);
					break;
				case agtk::data::TileSwitchVariableAssignData::kSwitchAssignToggle:
					switchData->setValue(!switchData->getValue());
					break;
				default:CC_ASSERT(0);
				}
			};
			if (assign->getSwitchObjectId() == agtk::data::TileSwitchVariableAssignData::kTouchedObject) {//触れているオブジェクト
				// ACT2-4982 ギミックタイルに触れたオブジェクトの判定にマージンがあるため、それを考慮に入れて調整。
				auto boundingBox = this->getBoundingBox();
				boundingBox.origin.x -= TILE_COLLISION_THRESHOLD;
				boundingBox.origin.y -= TILE_COLLISION_THRESHOLD;
				boundingBox.size.width += TILE_COLLISION_THRESHOLD * 2;
				boundingBox.size.height += TILE_COLLISION_THRESHOLD * 2;
				cocos2d::__Array *objectList;
				if (isAllLayerObject) {
					objectList = scene->getObjectAll(boundingBox, _sceneLayer->getType());
				}
				else {
					objectList = scene->getObjectAll(boundingBox, _sceneLayer->getType(), _sceneLayer->getLayerId());
				}
				switch (assign->getSwitchQualifierId()) {
				case agtk::data::TileSwitchVariableAssignData::kQualifierSingle: {//単体
					cocos2d::Ref *ref = nullptr;
					CCARRAY_FOREACH(objectList, ref) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
						auto object = static_cast<agtk::Object *>(ref);
#else
						auto object = dynamic_cast<agtk::Object *>(ref);
#endif
						auto objectData = object->getObjectData();
						auto instanceId = projectPlayData->getVariableData(objectData->getId(), agtk::data::kObjectSystemVariableSingleInstanceID)->getValue();
						if (object->getInstanceId() != (int)instanceId) {
							continue;
						}
						auto playObjectData = object->getPlayObjectData();
						auto switchData = playObjectData->getSwitchData(assign->getSwitchId());
						setSwitchData(switchData);
					}
					break; }
				case agtk::data::TileSwitchVariableAssignData::kQualifierWhole: {//全体
					cocos2d::Ref *ref = nullptr;
					CCARRAY_FOREACH(objectList, ref) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
						auto object = static_cast<agtk::Object *>(ref);
#else
						auto object = dynamic_cast<agtk::Object *>(ref);
#endif
						if (_objectOverlapped.flag() == false) continue;
						if (_objectOverlapped.object() != object) continue;
						auto playObjectData = object->getPlayObjectData();
						auto switchData = playObjectData->getSwitchData(assign->getSwitchId());
						if (switchData == nullptr) continue;
						setSwitchData(switchData);
					}
					break; }
				default:CC_ASSERT(0);
				}
			}
			else if (assign->getSwitchObjectId() == agtk::data::PlayData::COMMON_PLAY_DATA_ID) {//Resources共通
				auto switchData = projectPlayData->getSwitchData(agtk::data::PlayData::COMMON_PLAY_DATA_ID, assign->getSwitchId());
				setSwitchData(switchData);
			}
			else if(assign->getSwitchObjectId() > 0) {//オブジェクト
				switch (assign->getSwitchQualifierId()) {
				case agtk::data::TileSwitchVariableAssignData::kQualifierSingle: {//単体
// #AGTK-NX
#ifdef USE_SAR_OPTIMIZE_3
					int instanceId = (int)projectPlayData->getVariableData(assign->getSwitchObjectId(), agtk::data::kObjectSystemVariableSingleInstanceID)->getValue();
					agtk::Object* object = nullptr;
					if (instanceId >= 0) {
						if (isAllLayerObject) {
							object = scene->getObjectInstance(assign->getSwitchObjectId(), instanceId, _sceneLayer->getType());
						}
						else {
							object = scene->getObjectInstance(assign->getSwitchObjectId(), instanceId, _sceneLayer->getType(), _sceneLayer->getLayerId());
						}
					}
					if (object)
					{
						auto playObjectData = object->getPlayObjectData();
						auto switchData = playObjectData->getSwitchData(assign->getSwitchId());
						setSwitchData(switchData);
					}
#else
					cocos2d::__Array *objectList;
					if (isAllLayerObject) {
						objectList = scene->getObjectAll(assign->getSwitchObjectId(), _sceneLayer->getType());
					}
					else {
						objectList = scene->getObjectAll(assign->getSwitchObjectId(), _sceneLayer->getType(), _sceneLayer->getLayerId());
					}
					int instanceId = (int)projectPlayData->getVariableData(assign->getSwitchObjectId(), agtk::data::kObjectSystemVariableSingleInstanceID)->getValue();
					cocos2d::Ref *ref = nullptr;
					CCARRAY_FOREACH(objectList, ref) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
						auto object = static_cast<agtk::Object *>(ref);
#else
						auto object = dynamic_cast<agtk::Object *>(ref);
#endif
						if (object->getInstanceId() == instanceId) {
							auto playObjectData = object->getPlayObjectData();
							auto switchData = playObjectData->getSwitchData(assign->getSwitchId());
							setSwitchData(switchData);
						}
					}
#endif
					break; }
				case agtk::data::TileSwitchVariableAssignData::kQualifierWhole: {//全体
					cocos2d::__Array *objectList;
					if (isAllLayerObject) {
						objectList = scene->getObjectAll(assign->getSwitchObjectId(), _sceneLayer->getType());
					}
					else {
						objectList = scene->getObjectAll(assign->getSwitchObjectId(), _sceneLayer->getType(), _sceneLayer->getLayerId());
					}
					cocos2d::Ref *ref = nullptr;
					CCARRAY_FOREACH(objectList, ref) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
						auto object = static_cast<agtk::Object *>(ref);
#else
						auto object = dynamic_cast<agtk::Object *>(ref);
#endif
						if (_objectOverlapped.flag() == false) continue;
						if (_objectOverlapped.object() != object) continue;
						auto switchData = object->getPlayObjectData()->getSwitchData(assign->getSwitchId());
						setSwitchData(switchData);
					}
					break; }
				default:CC_ASSERT(0);
				}
			}
			else {
				//エラー
				CC_ASSERT(0);
			}
		}
		//変数変更
		else {
			std::function<void(agtk::data::PlayVariableData *, double)> setVariableData = [&assign, &index](agtk::data::PlayVariableData *variableData, double value) {
				if (variableData == nullptr) {
					return;
				}
				double nowValue = variableData->getValue();
				switch (assign->getVariableAssignOperator()) {
				case agtk::data::TileSwitchVariableAssignData::kVariableAssignOperatorSet:
					variableData->setValue(value);
					break;
				case agtk::data::TileSwitchVariableAssignData::kVariableAssignOperatorAdd:
					variableData->setValue(nowValue + value);
					break;
				case agtk::data::TileSwitchVariableAssignData::kVariableAssignOperatorSub:
					variableData->setValue(nowValue - value);
					break;
				case agtk::data::TileSwitchVariableAssignData::kVariableAssignOperatorMul:
					variableData->setValue(nowValue * value);
					break;
				case agtk::data::TileSwitchVariableAssignData::kVariableAssignOperatorDiv:
					if (value == 0) {
						variableData->setValue(std::nan("1"));
					}
					else {
						variableData->setValue(nowValue / value);
					}
					break;
				case agtk::data::TileSwitchVariableAssignData::kVariableAssignOperatorMod:
					if (value == 0) {
						variableData->setValue(std::nan("1"));
					}
					else {
						variableData->setValue((double)((int)nowValue % (int)value));
					}
					break;
				}
			};

			double value = 0.0f;
			switch (assign->getVariableAssignValueType()) {
			case 0://定数
				value = assign->getAssignValue();
				break;
			case 1: {//変数
				if (assign->getAssignVariableObjectId() == agtk::data::TileSwitchVariableAssignData::kTouchedObject) {//触れているオブジェクト
					//TODO: 触れたオブジェクトが複数存在するので、一番はじめに見つかったオブジェクトの値を得ています。処理的に正しい？
					CC_ASSERT(assign->getAssignVariableQualifierId() == agtk::data::TileSwitchVariableAssignData::kQualifierSingle);
					cocos2d::__Array *objectList;
					if (isAllLayerObject) {
						objectList = scene->getObjectAll(this->getBoundingBox(), _sceneLayer->getType());
					}
					else {
						objectList = scene->getObjectAll(this->getBoundingBox(), _sceneLayer->getType(), _sceneLayer->getLayerId());
					}
					agtk::data::PlayVariableData *assignVariableData = nullptr;
					cocos2d::Ref *ref = nullptr;
					CCARRAY_FOREACH(objectList, ref) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
						auto object = static_cast<agtk::Object *>(ref);
#else
						auto object = dynamic_cast<agtk::Object *>(ref);
#endif
						auto objectData = object->getObjectData();
						auto instanceId = projectPlayData->getVariableData(objectData->getId(), agtk::data::kObjectSystemVariableSingleInstanceID)->getValue();
						if (object->getInstanceId() != (int)instanceId) {
							continue;
						}
						auto playObjectData = object->getPlayObjectData();
						assignVariableData = playObjectData->getVariableData(assign->getAssignVariableId());
						break;
					}
					if (assignVariableData == nullptr) {
						continue;
					}
					value = assignVariableData->getValue();
				}
				else if (assign->getAssignVariableObjectId() == agtk::data::PlayData::COMMON_PLAY_DATA_ID) {//Resources共通
					auto assignVariableData = projectPlayData->getVariableData(agtk::data::PlayData::COMMON_PLAY_DATA_ID, assign->getAssignVariableId());
					if (assignVariableData == nullptr) {
						continue;
					}
					value = assignVariableData->getValue();
				}
				else if (assign->getAssignVariableObjectId() > 0) {//オブジェクト
					CC_ASSERT(assign->getAssignVariableQualifierId() == agtk::data::TileSwitchVariableAssignData::kQualifierSingle);
// #AGTK-NX
#ifdef USE_SAR_OPTIMIZE_3
					int instanceId = (int)projectPlayData->getVariableData(assign->getAssignVariableObjectId(), agtk::data::kObjectSystemVariableSingleInstanceID)->getValue();
					agtk::Object* object = nullptr;
					if (instanceId >= 0) {
						if (isAllLayerObject) {
							object = scene->getObjectInstance(assign->getAssignVariableObjectId(), instanceId, _sceneLayer->getType());
						}
						else {
							object = scene->getObjectInstance(assign->getAssignVariableObjectId(), instanceId, _sceneLayer->getType(), _sceneLayer->getLayerId());
						}
					}
					agtk::data::PlayVariableData *assignVariableData = nullptr;
					if (object)
					{
						auto playObjectData = object->getPlayObjectData();
						assignVariableData = playObjectData->getVariableData(assign->getAssignVariableId());
					}
#else
					cocos2d::__Array *objectList;
					if (isAllLayerObject) {
						objectList = scene->getObjectAll(assign->getAssignVariableObjectId(), _sceneLayer->getType());
					}
					else {
						objectList = scene->getObjectAll(assign->getAssignVariableObjectId(), _sceneLayer->getType(), _sceneLayer->getLayerId());
					}
					int instanceId = (int)projectPlayData->getVariableData(assign->getAssignVariableObjectId(), agtk::data::kObjectSystemVariableSingleInstanceID)->getValue();
					agtk::data::PlayVariableData *assignVariableData = nullptr;
					cocos2d::Ref *ref = nullptr;
					CCARRAY_FOREACH(objectList, ref) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
						auto object = static_cast<agtk::Object *>(ref);
#else
						auto object = dynamic_cast<agtk::Object *>(ref);
#endif
						if (object->getInstanceId() == instanceId) {
							auto playObjectData = object->getPlayObjectData();
							assignVariableData = playObjectData->getVariableData(assign->getAssignVariableId());
							break;
						}
					}
#endif
					if (assignVariableData == nullptr) {
						continue;
					}
					value = assignVariableData->getValue();
				}
				else {
					//エラー
					CC_ASSERT(0);
				}
				break; }
			case 2: {//乱数
				int v = assign->getRandomMin() + rand() % (assign->getRandomMax() - assign->getRandomMin() + 1);
				value = v;
				break; }
			case 3: {//スクリプト
				if (assign->getAssignScript()->length() > 0) {
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
							JS_GetProperty(cx, rscriptFunctions, "execTilesetTileGimmickSwitchVariableChange", &v);
							if (v.isObject()) {
								JS::RootedValue rexec(cx, v);
								jsval args[4];
								args[0] = JS::Int32Value(getTilesetId());
								args[1] = JS::Int32Value(getX());
								args[2] = JS::Int32Value(getY());
								args[3] = JS::Int32Value(index);
								ret = JS_CallFunctionValue(cx, rscriptFunctions, rexec, JS::HandleValueArray::fromMarkedLocation(4, args), &rv);
							}
						}
					}
#else
					auto scriptingCore = ScriptingCore::getInstance();
					auto context = scriptingCore->getGlobalContext();
					JS::RootedValue rv(context);
					JS::MutableHandleValue mhv(&rv);
					auto script = String::createWithFormat("(function(){ var tileX = %d, tileY = %d; return (%s\n); })()", getX(), getY(), assign->getAssignScript());
					auto ret = ScriptingCore::getInstance()->evalString(script->getCString(), mhv);
#endif
					if (!ret) {
						//スクリプトエラー
						auto errorStr = String::createWithFormat("Runtime error in changeSwitchVariableAssign(x: %d, y: %d, script: %s).", getX(), getY(), assign->getAssignScript())->getCString();
						agtk::DebugManager::getInstance()->getDebugExecuteLogWindow()->addLog(errorStr);
#if defined(USE_PREVIEW)
						auto fp = GameManager::getScriptLogFp();
						if (fp) {
							fwrite(errorStr, 1, strlen(errorStr), fp);
							fwrite("\n", 1, 1, fp);
						}
#endif
						value = std::nan("1");
					}
					if (rv.isDouble()) {
						value = rv.toDouble();
					}
					else if (rv.isInt32()) {
						value = rv.toInt32();
					}
					else {
						//数値でない
						value = std::nan("1");
					}
				}
				break; }
			default:CC_ASSERT(0);
			}

			if (assign->getVariableObjectId() == agtk::data::TileSwitchVariableAssignData::kTouchedObject) {//触れているオブジェクト
				cocos2d::__Array *objectList;
				if (isAllLayerObject) {
					objectList = scene->getObjectAll(this->getBoundingBox(), _sceneLayer->getType());
				}
				else {
					objectList = scene->getObjectAll(this->getBoundingBox(), _sceneLayer->getType(), _sceneLayer->getLayerId());
				}
				switch (assign->getVariableQualifierId()) {
				case agtk::data::TileSwitchVariableAssignData::kQualifierSingle: {//単体
					cocos2d::Ref *ref = nullptr;
					CCARRAY_FOREACH(objectList, ref) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
						auto object = static_cast<agtk::Object *>(ref);
#else
						auto object = dynamic_cast<agtk::Object *>(ref);
#endif
						auto objectData = object->getObjectData();
						auto instanceId = projectPlayData->getVariableData(objectData->getId(), agtk::data::kObjectSystemVariableSingleInstanceID)->getValue();
						if (object->getInstanceId() != (int)instanceId) {
							continue;
						}
						auto playObjectData = object->getPlayObjectData();
						auto variableData = playObjectData->getVariableData(assign->getVariableId());
						setVariableData(variableData, value);
					}
					break; }
				case agtk::data::TileSwitchVariableAssignData::kQualifierWhole: {//全体
					cocos2d::Ref *ref = nullptr;
					CCARRAY_FOREACH(objectList, ref) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
						auto object = static_cast<agtk::Object *>(ref);
#else
						auto object = dynamic_cast<agtk::Object *>(ref);
#endif
						if (_objectOverlapped.flag() == false) continue;
						if (_objectOverlapped.object() != object) continue;
						auto playObjectData = object->getPlayObjectData();
						auto variableData = playObjectData->getVariableData(assign->getVariableId());
						setVariableData(variableData, value);
					}
					break; }
				default:CC_ASSERT(0);
				}
			}
			else if (assign->getVariableObjectId() == agtk::data::PlayData::COMMON_PLAY_DATA_ID) {//Resources共通
				auto variableData = projectPlayData->getVariableData(agtk::data::PlayData::COMMON_PLAY_DATA_ID, assign->getVariableId());
				setVariableData(variableData, value);
			}
			else if (assign->getVariableObjectId() > 0) {//オブジェクト
				switch (assign->getVariableQualifierId()) {
				case agtk::data::TileSwitchVariableAssignData::kQualifierSingle: {//単体
																				  // #AGTK-NX
#ifdef USE_SAR_OPTIMIZE_3
					int instanceId = (int)projectPlayData->getVariableData(assign->getVariableObjectId(), agtk::data::kObjectSystemVariableSingleInstanceID)->getValue();
					agtk::Object* object = nullptr;
					if (instanceId >= 0) {
						if (isAllLayerObject) {
							object = scene->getObjectInstance(assign->getVariableObjectId(), instanceId, _sceneLayer->getType());
						}
						else {
							object = scene->getObjectInstance(assign->getVariableObjectId(), instanceId, _sceneLayer->getType(), _sceneLayer->getLayerId());
						}
					}
					if (object)
					{
						auto playObjectData = object->getPlayObjectData();
						setVariableData(playObjectData->getVariableData(assign->getVariableId()), value);
						object->getPlayObjectData()->adjustData();
					}
#else
					cocos2d::__Array *objectList;
					if (isAllLayerObject) {
						objectList = scene->getObjectAll(assign->getVariableObjectId(), _sceneLayer->getType());
					}
					else {
						objectList = scene->getObjectAll(assign->getVariableObjectId(), _sceneLayer->getType(), _sceneLayer->getLayerId());
					}
					int instanceId = (int)projectPlayData->getVariableData(assign->getVariableObjectId(), agtk::data::kObjectSystemVariableSingleInstanceID)->getValue();
					cocos2d::Ref *ref = nullptr;
					CCARRAY_FOREACH(objectList, ref) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
						auto object = static_cast<agtk::Object *>(ref);
#else
						auto object = dynamic_cast<agtk::Object *>(ref);
#endif
						if (object->getInstanceId() == instanceId) {
							auto playObjectData = object->getPlayObjectData();
							setVariableData(playObjectData->getVariableData(assign->getVariableId()), value);
							object->getPlayObjectData()->adjustData();
						}
					}
#endif
					break; }
				case agtk::data::TileSwitchVariableAssignData::kQualifierWhole: {//全体
					cocos2d::__Array *objectList;
					if (isAllLayerObject) {
						objectList = scene->getObjectAll(assign->getVariableObjectId(), _sceneLayer->getType());
					}
					else {
						objectList = scene->getObjectAll(assign->getVariableObjectId(), _sceneLayer->getType(), _sceneLayer->getLayerId());
					}
					cocos2d::Ref *ref = nullptr;
					CCARRAY_FOREACH(objectList, ref) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
						auto object = static_cast<agtk::Object *>(ref);
#else
						auto object = dynamic_cast<agtk::Object *>(ref);
#endif
						if (_objectOverlapped.flag() == false) continue;
						if (_objectOverlapped.object() != object) continue;
						auto playObjectData = object->getPlayObjectData();
						setVariableData(playObjectData->getVariableData(assign->getVariableId()), value);
					}
					break; }
				default:CC_ASSERT(0);
				}
			}
			else {
				//エラー
				CC_ASSERT(0);
			}
		}
	}
}

/**
 * アンカーを適応した座標の取得
 * @return	アンカーを適応した座標
 */
cocos2d::Vec2 Tile::getAnchoredPosition()
{
	cocos2d::Vec2 pos = this->getPosition();
	pos.x += (0.5f - this->getAnchorPoint().x) * this->getContentSize().width;
	pos.y += (0.5f - this->getAnchorPoint().y) * this->getContentSize().height;

	return pos;
}

/**
* 物理ボディのセットアップ
* @param	wallBit	壁ビット値
*/
void Tile::setupPhysicsBody(int wallBit)
{
	// wallbit -> 1:上 2:左 4:右 8:下

	auto tileData = this->getTileData();

	// 物理演算ボディ設定
	auto physicsMaterial = PhysicsMaterial(0.1f, 1.0f, 0.5f);
	if (nullptr != tileData) {
		physicsMaterial.restitution = tileData->getPhysicsRepulsion();
		physicsMaterial.friction = tileData->getPhysicsFriction();
	}

	auto pTile = PhysicsBody::create();
	auto halfSize = getContentSize() * 0.5f;

	// タイルサイズにピッタリだと1dotはみ出るので縮める
	if (halfSize.width > 1.0f) {
		halfSize.width--;
	}
	if (halfSize.height > 1.0f) {
		halfSize.height--;
	}

	// 上の壁がある場合
	if ((1 << agtk::data::TilesetData::Up) & wallBit) {
		auto wall = PhysicsShapeEdgeSegment::create(Vec2(-halfSize.width, halfSize.height), Vec2(halfSize.width, halfSize.height), physicsMaterial);
		pTile->addShape(wall);
	}
	// 左の壁がある場合
	if ((1 << agtk::data::TilesetData::Left) & wallBit) {
		auto wall = PhysicsShapeEdgeSegment::create(Vec2(-halfSize.width, -halfSize.height), Vec2(-halfSize.width, halfSize.height), physicsMaterial);
		pTile->addShape(wall);
	}
	// 右の壁がある場合
	if ((1 << agtk::data::TilesetData::Right) & wallBit) {
		auto wall = PhysicsShapeEdgeSegment::create(Vec2(halfSize.width, halfSize.height), Vec2(halfSize.width, -halfSize.height), physicsMaterial);
		pTile->addShape(wall);
	}
	// 下の壁がある場合
	if ((1 << agtk::data::TilesetData::Down) & wallBit) {
		auto wall = PhysicsShapeEdgeSegment::create(Vec2(halfSize.width, -halfSize.height), Vec2(-halfSize.width, -halfSize.height), physicsMaterial);
		pTile->addShape(wall);
	}

	// 四方が囲まれている場合
	if (FILLED_WALLBIT == wallBit) {
		// 中身を詰める
		auto box = PhysicsShapeBox::create(Size(getContentSize().width - 2, getContentSize().height - 2), physicsMaterial);
		pTile->addShape(box);
	}

	pTile->setDynamic(false);
	pTile->setGroup(GameManager::EnumPhysicsGroup::kTile);
	pTile->setContactTestBitmask(INT_MAX);
	auto physicsLayer = (this->getLayerId() - 1);
	if (_sceneId == agtk::data::SceneData::kMenuSceneId) {
		physicsLayer = physicsLayer + agtk::PhysicsBase::kMenuPhysicsLayerShift;
	}
	auto bitmask = (1 << physicsLayer);
	pTile->setCategoryBitmask(bitmask);
	pTile->setCollisionBitmask(bitmask);
	pTile->setGravityEnable(false);//重力不可
	pTile->setRotationEnable(false);//回転運動不可
	this->setPhysicsBody(pTile);
	this->unscheduleUpdate();
}

int Tile::getWallBit()
{
	if (this->getType() == kTypeLimitTile) {
		return 0x0f;
	}
	auto projectData = GameManager::getInstance()->getProjectData();
	CC_ASSERT(projectData);
	auto tileset = projectData->getTilesetData(this->getTilesetId());
	CC_ASSERT(tileset);
	int tileId = this->getY() * tileset->getHorzTileCount() + this->getX();
	return tileset->getWallSetting(tileId);
}

int Tile::getGroupBit() const
{
	int group = 0;
	if (this->getTileData()) {
		group = this->getTileData()->getGroup();
	}
	return 1 << group;
}

int Tile::getGroup() const
{
	int group = 0;
	if (this->getTileData()) {
		group = this->getTileData()->getGroup();
	}
	return group;
}

void Tile::convertToWorldSpaceVertex4(agtk::Vertex4 &vertex4)
{
	auto size = this->getContentSize();
	auto pos = this->getPosition();

	vertex4[0] = pos + cocos2d::Vec2(0, size.height);			//左上
	vertex4[1] = pos + cocos2d::Vec2(size.width, size.height);	//右上
	vertex4[2] = pos + cocos2d::Vec2(size.width, 0);			//右下
	vertex4[3] = pos + cocos2d::Vec2(0, 0);						//左下
}

void Tile::convertToLayerSpaceVertex4(agtk::Vertex4 &vertex4)
{
	this->convertToWorldSpaceVertex4(vertex4);
}

cocos2d::Rect Tile::convertToWorldSpaceRect()
{
	agtk::Vertex4 vertex4;
	this->convertToWorldSpaceVertex4(vertex4);
	return vertex4.getRect();
}

cocos2d::Rect Tile::convertToLayerSpaceRect()
{
	agtk::Vertex4 vertex4;
	this->convertToLayerSpaceVertex4(vertex4);
	return vertex4.getRect();
}

/**
* デバッグ表示
*/
void Tile::showDebugVisible(bool isShow)
{
// #AGTK-NX
#if defined(USE_PREVIEW) || ((CC_TARGET_PLATFORM == CC_PLATFORM_NX) && defined(AGTK_DEBUG))
//#ifdef USE_PREVIEW
	auto debugView = this->getDebugNode();

	if (!debugView && isShow) {

		auto screenSize = GameManager::getInstance()->getProjectData()->getScreenSize();
		auto position = this->getPosition();
		auto size = this->getContentSize();
		auto halfSize = this->getTileSize() * 0.5f;
		auto quarterSize = halfSize * 0.75f;
		auto wallBit = this->getWallBit();

		cocos2d::Vec2 pos = halfSize;
		if (this->getContentSize().height != this->getTileSize().height) {
			int x = this->getTileX() * this->getTileSize().width;
			int y = this->getTileY() * this->getTileSize().height + this->getTileSize().height;
			auto p = Scene::getPositionCocos2dFromScene(cocos2d::Vec2(x, y));
			pos = pos - (position - p);
		}

		auto drawNode = DrawNode::create(1.0f);
		drawNode->setPosition(this->getPosition() + pos);

		// 上の壁がある場合
		if ((1 << agtk::data::TilesetData::Up) & wallBit) {
			drawNode->drawLine(Vec2(-halfSize.width, quarterSize.height), Vec2(halfSize.width, quarterSize.height), Color4F::RED);
		}
		// 左の壁がある場合
		if ((1 << agtk::data::TilesetData::Left) & wallBit) {
			drawNode->drawLine(Vec2(-quarterSize.width, -halfSize.height), Vec2(-quarterSize.width, halfSize.height), Color4F::RED);
		}
		// 右の壁がある場合
		if ((1 << agtk::data::TilesetData::Right) & wallBit) {
			drawNode->drawLine(Vec2(quarterSize.width, halfSize.height), Vec2(quarterSize.width, -halfSize.height), Color4F::RED);
		}
		// 下の壁がある場合
		if ((1 << agtk::data::TilesetData::Down) & wallBit) {
			drawNode->drawLine(Vec2(-halfSize.width, -quarterSize.height), Vec2(halfSize.width, -quarterSize.height), Color4F::RED);
		}
		this->getParent()->getParent()->addChild(drawNode);	//TileMapに追加。
		setDebugNode(drawNode);

	}
	else if (debugView) {
		debugView->setVisible(isShow);
	}
#endif // USE_PREVIEW
}

//check update
bool Tile::isNeedUpdate()
{
	auto tileData = this->getTileData();
	if (tileData == nullptr) return false;
	if (tileData->getPlayerMoveType() != agtk::data::TileData::kPlayerMoveNone) {
		return true;
	}
	if(tileData->getTileAttackAreaTouched()) {
		return true;
	}
	if (tileData->getTimePassed()) {
		return true;
	}
	if (tileData->getSwitchVariableCondition()) {
		return true;
	}
	if (tileData->getChangeTileType() != agtk::data::TileData::kChangeTileNone) {
		return true;
	}
	if (tileData->getPlaySe()) {
		return true;
	}
	if (tileData->getAppearObject()) {
		return true;
	}
	if (tileData->getShowEffect()) {
		return true;
	}
	if (tileData->getShowParticle()) {
		return true;
	}
	if (tileData->getSwitchVariableAssign()) {
		return true;
	}
	if (tileData->getTileAnimationData()) {
		return true;
	}
	return false;
}

void Tile::setObjectOverlapped(bool flag, agtk::Object *object)
{
	_objectOverlapped.set(flag, object);
}

Tile::ObjectOverlapped::ObjectOverlapped()
{
	_flag = false;
	_object = nullptr;
}

Tile::ObjectOverlapped::~ObjectOverlapped()
{
	reset();
}

void Tile::ObjectOverlapped::set(bool flag, agtk::Object *object)
{
	this->_flag = flag;
	if (this->_object != object) {
		this->_object = object;
	}
}

void Tile::ObjectOverlapped::reset()
{
	this->_flag = false;
	this->_object = nullptr;
}

//-------------------------------------------------------------------------------------------------------------------
LimitTile::LimitTile(): agtk::Tile()
{
	_type = kTypeLimitTile;
	_object = nullptr;
}

void LimitTile::update(float delta)
{
}

bool LimitTile::init(int layerId, cocos2d::Size size)
{
	this->setLayerId(layerId);
	this->setContentSize(size);
	return true;
}

bool LimitTile::isNeedCheck(agtk::Object const * obj)const
{
	if (getObject() == nullptr && obj && obj->getObjectData()->isGroupPlayer()) {
		return true;		 // シーン用の行動範囲制限
	}
	if ( obj && getObject() == obj) {
		return true;		 // オブジェクト専用の行動範囲制限
	}
	return false;
}

std::unique_ptr<LimitTileSet> LimitTileSet::create(int layerId, cocos2d::Rect rc, cocos2d::Size tileSize, agtk::data::SceneData const * sceneData)
{
	struct TileInfo {
		const char* name;  // ノード名
		cocos2d::Vec2 pos; // 位置
		cocos2d::Size size;// サイズ
	} const Table[] = {
		{ "limitTileLeft", cocos2d::Vec2(rc.getMinX() - tileSize.width, rc.getMinY() + rc.size.height), cocos2d::Size(tileSize.width,rc.size.height) },
		{ "limitTileRight",cocos2d::Vec2(rc.getMinX() + rc.size.width, rc.getMinY() + rc.size.height), cocos2d::Size(tileSize.width,rc.size.height) },
		{ "limitTileUp",   cocos2d::Vec2(rc.getMinX(), rc.getMinY()), cocos2d::Size(rc.size.width, tileSize.height) },
		{ "limitTileDown", cocos2d::Vec2(rc.getMinX(), (rc.getMinY() + rc.size.height) + tileSize.height), cocos2d::Size(rc.size.width, tileSize.height) },
	};
	cocos2d::__Array* ary = cocos2d::__Array::create();
	for (auto const & data : Table) {
		auto tile = agtk::LimitTile::create(layerId, data.size);
		tile->setInitPos(agtk::Scene::getPositionCocos2dFromScene(data.pos, sceneData));
		tile->setPosition(tile->getInitPos());
		tile->setName(data.name);
		ary->addObject(tile);
	}

	std::unique_ptr<LimitTileSet> lts;
	lts.reset(new LimitTileSet);
	lts->setLimitTileList(ary);
	lts->setSceneData(sceneData);
	lts->setTileSize(tileSize);
	return std::move(lts);
}

void LimitTileSet::UpdatePosByCamera(agtk::Camera* camera)
{
	auto sceneData = this->getSceneData();
	auto screenSize = camera->getScreenSize();
	screenSize.width *= camera->getScale().x;
	screenSize.height *= camera->getScale().y;
	auto halfScreenSize = screenSize * 0.5f;

	if (camera != nullptr && (this->getScreenSize().width != screenSize.width || this->getScreenSize().height != screenSize.height)) {
		this->setScreenSize(screenSize);

		auto data = this->getSettingData();
		auto tileSize = this->getTileSize();
		cocos2d::Rect const rc(
			cocos2d::Vec2(data->getLeft(), data->getTop()),
			cocos2d::Size(screenSize.width - data->getRight() - data->getLeft(), screenSize.height - data->getBottom() - data->getTop())
		);

		struct TileInfo {
			cocos2d::Vec2 pos; // 位置
			cocos2d::Size size;// サイズ
		} const Table[] = {
			{ cocos2d::Vec2(rc.getMinX() - tileSize.width, rc.getMinY() + rc.size.height), cocos2d::Size(tileSize.width,rc.size.height) },
			{ cocos2d::Vec2(rc.getMinX() + rc.size.width, rc.getMinY() + rc.size.height), cocos2d::Size(tileSize.width,rc.size.height) },
			{ cocos2d::Vec2(rc.getMinX(), rc.getMinY()), cocos2d::Size(rc.size.width, tileSize.height) },
			{ cocos2d::Vec2(rc.getMinX(), (rc.getMinY() + rc.size.height) + tileSize.height), cocos2d::Size(rc.size.width, tileSize.height) },
		};

		auto list = _limitTileList.get();
		for (int i = 0; i < CC_ARRAYSIZE(Table); i++) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
			auto lt = static_cast<LimitTile *>(list->getObjectAtIndex(i));
#else
			auto lt = dynamic_cast<LimitTile *>(list->getObjectAtIndex(i));
#endif
			auto tableData = Table[i];
			lt->setContentSize(tableData.size);
			lt->setInitPos(agtk::Scene::getPositionCocos2dFromScene(tableData.pos, sceneData));
		}
	}

	if (camera) {
		auto list = _limitTileList.get();
		cocos2d::Ref* ref;

		auto ofs = camera->getPositionForZoom() - agtk::Scene::getPositionCocos2dFromScene(halfScreenSize, sceneData);
		CCARRAY_FOREACH(list, ref) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
			auto lt = static_cast<LimitTile *>(ref);
#else
			auto lt = dynamic_cast<LimitTile *>(ref);
#endif
			lt->setPosition(lt->getInitPos() + ofs);
		}
	}
}

bool LimitTileSet::isSwitchEnable(bool* existSwitch) const
{
	CC_ASSERT(_settingData);
	if (_limitTileList.get() ) {
		auto checkSwitch = [&, existSwitch](auto switchData) {
			if (switchData != nullptr) {
				if (existSwitch) {
					*existSwitch = true;
				}
				return	switchData->getValue();
			}
			else {
				if (existSwitch) {
					*existSwitch = false;
				}
				return true;
			}
		};
		if (_settingData->getObjectSwitch()) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
			auto playData = static_cast<LimitTile*>(_limitTileList->getObjectAtIndex(0))->getObject()->getPlayObjectData();// どの LimitTile も一緒なので先頭を取得	
#else
			auto playData = dynamic_cast<LimitTile*>(_limitTileList->getObjectAtIndex(0))->getObject()->getPlayObjectData();// どの LimitTile も一緒なので先頭を取得	
#endif
			auto switchData = playData->getSwitchData(_settingData->getObjectSwitchId());
			return checkSwitch(switchData);
		}
		else {
			auto playData = GameManager::getInstance()->getPlayData();
			auto switchData = playData->getSwitchData(agtk::data::PlayData::COMMON_PLAY_DATA_ID, _settingData->getSystemSwitchId());
			return checkSwitch(switchData);
		}
	}
	return false;
}

std::unique_ptr<LimitTileSetList> LimitTileSetList::create() 
{
	return std::move(std::unique_ptr<LimitTileSetList>(new LimitTileSetList));
}

void LimitTileSetList::add(std::unique_ptr<LimitTileSet>& lts)
{
	_limitTileSetList.push_back(std::move(lts)); 
}
LimitTileSet * LimitTileSetList::getEnableLimitTileSet()const
{
	// 有効なのは１つだけ。
	// スイッチが設定されているものがある場合はその中の一番上（エディタ的に）の項目
	// ない場合は残りの中で一番上の項目
	LimitTileSet* lts = nullptr;
	for (auto & data : _limitTileSetList) {
		bool isExistSwitch = false;
		bool enable = data->isSwitchEnable(&isExistSwitch);
		if (enable) {
			if (isExistSwitch) {
				//　スイッチが設定されているものが有効ならそれに決定（一番上のはず）
				lts = data.get();
				return lts;
			}
			else 
			{// スイッチが設定されていない一番上
				if (lts == nullptr) {
					lts = data.get();
				}
			}
		}
	}
	return lts;
}

//-------------------------------------------------------------------------------------------------------------------
Subtile::Subtile()
{
	_tilesetId = -1;
	_x = 0;
	_y = 0;
	_tileData = nullptr;
	_tileSize = cocos2d::Size::ZERO;
}

Subtile::~Subtile()
{
	CC_SAFE_RELEASE_NULL(_tileData);
}

bool Subtile::init(data::Tile* tile, cocos2d::Texture2D* texture, int layerId)
{
	if (!cocos2d::Sprite::initWithTexture(texture)) {
		return false;
	}
#if 0
	//layerId
	this->setLayerId(layerId);
	//tilesetId, x, y
	this->setTilesetId(tile->getTilesetId());
	this->setX(tile->getX());
	this->setY(tile->getY());
	this->setAnchorPoint(cocos2d::Vec2(0, 0));
	//tileData
	auto project = GameManager::getInstance()->getProjectData();
	auto tileset = project->getTilesetData(tile->getTilesetId());
	auto tileData = tileset->getTileData(tile->getX() + tile->getY() * tileset->getHorzTileCount());
	this->setTileData(tileData);
	//tileSize
	this->setTileSize(project->getTileSize());
	//uv setting
	float scaleFactor = Director::getInstance()->getContentScaleFactor();
	this->setTextureRect(cocos2d::Rect(
		tile->getX() * this->getTileSize().width * scaleFactor,
		tile->getY() * this->getTileSize().height * scaleFactor,
		this->getTileSize().width * scaleFactor,
		this->getTileSize().height * scaleFactor
	));
	//argb
	if (tileData != nullptr) {
		this->setOpacity(tileData->getA());
		this->setColor(cocos2d::Color3B(tileData->getR(), tileData->getG(), tileData->getB()));
	}

	this->setChangeState(false);
#endif

	return true;
}

#if 0
/**
* アンカーを適応した座標の取得
* @return	アンカーを適応した座標
*/
cocos2d::Vec2 Subtile::getAnchoredPosition()
{
	cocos2d::Vec2 pos = this->getPosition();
	pos.x += (0.5f - this->getAnchorPoint().x) * this->getContentSize().width;
	pos.y += (0.5f - this->getAnchorPoint().y) * this->getContentSize().height;

	return pos;
}
#endif

//-------------------------------------------------------------------------------------------------------------------
Tileset::Tileset()
{
	_tilesetData = nullptr;
	_gifAnimation = nullptr;
	_sceneLayer = nullptr;
}

Tileset::~Tileset()
{
	CC_SAFE_RELEASE_NULL(_tilesetData);
	CC_SAFE_RELEASE_NULL(_gifAnimation);
	//CC_SAFE_RELEASE_NULL(_sceneLayer);
}

bool Tileset::init(data::TilesetData* tilesetData, agtk::SceneLayer *sceneLayer)
{
	CC_ASSERT(tilesetData);
	_sceneLayer = sceneLayer;
	//CC_SAFE_RETAIN(sceneLayer);
	auto project = GameManager::getInstance()->getProjectData();
	auto imgData = project->getImageData(tilesetData->getImageId());
	if (strstr(imgData->getFilename(), ".gif") != nullptr || strstr(imgData->getFilename(), ".GIF") != nullptr) {
		auto project = GameManager::getInstance()->getProjectData();
		auto imgData = project->getImageData(tilesetData->getImageId());
		auto gifAnimation = agtk::GifAnimation::create(imgData->getFilename());
		if (gifAnimation == nullptr) {
			return false;
		}
		gifAnimation->play();
		auto texture2d = new Texture2D();
		int width = gifAnimation->getWidth();
		int height = gifAnimation->getHeight();
		auto bm = gifAnimation->getBitmap();
		texture2d->initWithData(
			bm->getAddr(),
			bm->getLength(),
			Texture2D::PixelFormat::RGBA8888,
			bm->getWidth(),
			bm->getHeight(),
			cocos2d::Size(bm->getWidth(), bm->getHeight())
		);
		this->setGifAnimation(gifAnimation);
		if (!cocos2d::SpriteBatchNode::initWithTexture(texture2d)) {
			return false;
		}
	}
	else {
		if (!cocos2d::SpriteBatchNode::initWithFile(imgData->getFilename())) {
			return false;
		}
	}
	this->setTilesetData(tilesetData);
	return true;
}

void Tileset::update(float delta)
{
	auto gifAnimation = this->getGifAnimation();
	if (gifAnimation) {
		bool ret = gifAnimation->update(delta);
		if (ret) {
			auto bitmap = gifAnimation->getBitmap();
			cocos2d::Size contentSize(bitmap->getWidth(), bitmap->getHeight());
			auto texture2d = new Texture2D();
			texture2d->initWithData(
				bitmap->getAddr(),
				bitmap->getLength(),
				Texture2D::PixelFormat::RGBA8888,
				bitmap->getWidth(),
				bitmap->getHeight(),
				contentSize
			);
			this->setTexture(texture2d);
		}
	}
	cocos2d::SpriteBatchNode::update(delta);
}

bool Tileset::isNeedUpdate()
{
	if (this->getGifAnimation()) {
		return true;
	}
#if CC_ENABLE_SCRIPT_BINDING
	if (0 != _updateScriptHandler) {
		return true;
	}
#endif

	if (_componentContainer && !_componentContainer->isEmpty())
	{
		return true;
	}
	return false;
}

//-------------------------------------------------------------------------------------------------------------------
TileMap::TileMap()
{
	_tileMapWidth = 0;
	_tileMapHeight = 0;
	_tileMap = nullptr;
	_tileMap2 = nullptr;
	_sceneLayer = nullptr;
	_childrenList = nullptr;
	_updatedTilesetList = nullptr;
	_updatedTileList = nullptr;
}

TileMap::~TileMap()
{
	if (_tileMap != nullptr){
		delete[] _tileMap;
		_tileMap = nullptr;
	}
	if (_tileMap2 != nullptr) {
		delete[] _tileMap2;
		_tileMap2 = nullptr;
	}
	//CC_SAFE_RELEASE_NULL(_sceneLayer);
	CC_SAFE_RELEASE_NULL(_childrenList);
	CC_SAFE_RELEASE_NULL(_updatedTilesetList);
	CC_SAFE_RELEASE_NULL(_updatedTileList);
}

bool TileMap::init(cocos2d::__Array *tileList, agtk::SceneLayer *sceneLayer)
{
	int layerId = sceneLayer->getLayerId();
	_sceneLayer = sceneLayer;
	//CC_SAFE_RETAIN(sceneLayer);

	auto sceneData = sceneLayer->getSceneData();

	setChildrenList(cocos2d::Array::create());
	setUpdatedTilesetList(cocos2d::Array::create());
	setUpdatedTileList(cocos2d::Array::create());

	// タイル用物理オブジェクト塊化用ワーク変数
	auto filledWallTileList = cocos2d::__Array::create();

	auto project = GameManager::getInstance()->getProjectData();
	auto arr = cocos2d::__Array::create();
	cocos2d::Ref *ref = nullptr;
	int tileId = 0;
	auto screenSize = project->getScreenSize();
	auto tileSize = project->getTileSize();
	cocos2d::Size sceneSize(screenSize.width * sceneData->getHorzScreenCount(), screenSize.height * sceneData->getVertScreenCount());
	std::map<string, int> nameCountMap;
	CCARRAY_FOREACH(tileList, ref) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
		auto data = static_cast<data::Tile *>(ref);
#else
		auto data = dynamic_cast<data::Tile *>(ref);
#endif
		int tilesetId = data->getTilesetId();
		auto tilesetData = project->getTilesetData(tilesetId);
		std::string name = std::string("tileset") + std::to_string(tilesetId);
		auto spriteBatchNode = dynamic_cast<Tileset *>(this->getChildByName(name));
		if (spriteBatchNode && spriteBatchNode->getChildrenCount() >= 65536 / 4) {
			spriteBatchNode->setName(String::createWithFormat("%s-%d", name.c_str(), (nameCountMap[name] - 1))->getCString());
			spriteBatchNode = nullptr;
		}
		if (spriteBatchNode == nullptr) {
			if (nameCountMap.find(name) == nameCountMap.end()) {
				nameCountMap.insert(std::make_pair(name, 0));
			}
			nameCountMap[name]++;
			spriteBatchNode = Tileset::create(tilesetData, sceneLayer);
			if (spriteBatchNode) {
				spriteBatchNode->getTexture()->setAliasTexParameters();
				this->addChild(spriteBatchNode, 0, name);
				_childrenList->addObject(spriteBatchNode);
				spriteBatchNode->unscheduleUpdate();//※ループ一元化のためここで破棄する。
				spriteBatchNode->setTag(PhysicsWorld::kOutOfPhysicsSimulationNodeTag);
			}
		}
		if (spriteBatchNode == nullptr) {
			//テクスチャが無い？
			CC_ASSERT(0);
			continue;
		}

		float x = data->getPosition().x * tileSize.width;
		float y = sceneSize.height - tileSize.height - data->getPosition().y * tileSize.height;

		// 上下左右のシーン接続を行う際、シーンのサイズがタイルサイズで綺麗に割り切れず
		// 接続したシーンのタイルが重なって表示されることがあるので
		// 端のタイルはシーンのサイズに合わせてスケール値を設定して表示されないようにする
		float scaleX = clampf((sceneSize.width - x) / tileSize.width, 0.0f, 1.0f);
		float scaleY = 1.0f;
		if (y < 0) {
			scaleY = clampf((tileSize.height + y) / tileSize.height, 0.0f, 1.0f);
			y = 0;
		}

		auto tile = Tile::create(data, spriteBatchNode->getTexture(), sceneLayer, cocos2d::Size(scaleX, scaleY));
		tile->setPosition(x, y);

		// 壁判定無しでないタイルであれば物理ボディをセットアップする
		int tileSetId = data->getY() * tilesetData->getHorzTileCount() + data->getX();
		int wallBit = tilesetData->getWallSetting(tileSetId);
		if (wallBit != 0) {

#ifdef USE_MERGE_TILE_PHYSICS
			// 四方全て壁判定 かつ ギミック設定の変更後の状態が「タイルを変更しない」の場合
			auto tileData = tile->getTileData();
			if (wallBit == FILLED_WALLBIT && (!tileData || tileData->getChangeTileType() == agtk::data::TileData::kChangeTileNone)) {
				filledWallTileList->addObject(tile);
			}
			// 四方全て壁判定でない場合
			else {
				tile->setupPhysicsBody(wallBit);
				spriteBatchNode->addPhysicsChild(tile);
			}
#else
			tile->setupPhysicsBody(wallBit);
			spriteBatchNode->addPhysicsChild(tile);
#endif
		}
		//CC_ASSERT(tile->getPosition().x >= 0);
		//CC_ASSERT(tile->getPosition().y >= 0);
		spriteBatchNode->addChild(tile, 0, tileId);
		_childrenList->addObject(tile);
		tile->unscheduleUpdate();//※ループ一元化のためここで破棄する。
		tileId++;
	}

	//行動範囲制限
	{
		float const limitAreaX = sceneData->isMenuScene() ? 0.0f : sceneData->getLimitAreaX();
		float const limitAreaY = sceneData->isMenuScene() ? 0.0f : sceneData->getLimitAreaY();
		float const limitAreaWidth = sceneData->isMenuScene() ? screenSize.width : sceneData->getLimitAreaWidth();
		float const limitAreaHeight = sceneData->isMenuScene() ? sceneSize.height : sceneData->getLimitAreaHeight();
		float const tileWidth = project->getTileWidth();
		float const tileHeight = project->getTileHeight();
		_limitTileSet = LimitTileSet::create(layerId, cocos2d::Rect(cocos2d::Vec2(limitAreaX, limitAreaY), cocos2d::Size(limitAreaWidth, limitAreaHeight)), cocos2d::Size(tileWidth, tileHeight), sceneData);
		auto limitTileList = _limitTileSet->getLimitTileList().get();
		cocos2d::Ref* ref = nullptr;
		CCARRAY_FOREACH(limitTileList, ref) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
			LimitTile* lt = static_cast<LimitTile*>(ref);
#else
			LimitTile* lt = dynamic_cast<LimitTile*>(ref);
#endif
			this->addChild(lt);
			lt->unscheduleUpdate();
		}
	}
#ifdef USE_MERGE_TILE_PHYSICS
	// タイルが変化しない四方が全て壁判定のタイルが存在する場合
	if (filledWallTileList->count() > 0) {
		// マージを実施
		auto sceneTileSize = Vec2(sceneSize.width / tileSize.width, sceneSize.height / tileSize.height);
		this->mergeTilePhysicsBox(filledWallTileList, sceneTileSize);
	}
#endif

	CCARRAY_FOREACH(_childrenList, ref) {
		auto tileset = dynamic_cast<agtk::Tileset *>(ref);
		if (tileset) {
			if (tileset->isNeedUpdate()) {
				_updatedTilesetList->addObject(tileset);
			}
			continue;
		}
		auto tile = dynamic_cast<Tile *>(ref);
		if (tile){
			if (tile->isNeedUpdate()) {
				_updatedTileList->addObject(tile);
			}
		}
	}
	return true;
}

void TileMap::addCollisionDetaction(CollisionDetaction *collision)
{
	auto project = GameManager::getInstance()->getProjectData();
}

void TileMap::initTileMap(int sceneHorzTileCount, int sceneVertTileCount)
{
	auto project = GameManager::getInstance()->getProjectData();
	auto tileSize = project->getTileSize();
	_tileMapWidth = sceneHorzTileCount;
	_tileMapHeight = sceneVertTileCount;
	_tileMap = new Tile *[_tileMapWidth * _tileMapHeight];
	memset(_tileMap, 0, sizeof(*_tileMap) * _tileMapWidth * _tileMapHeight);
	cocos2d::Ref *ref = nullptr;
	CCARRAY_FOREACH(_childrenList, ref) {
		auto p = dynamic_cast<Tile *>(ref);
		if (p) {
			if (p->getType() == agtk::Tile::kTypeLimitTile) {
				continue;
			}
			auto tilesetData = project->getTilesetData(p->getTilesetId());
			int wallSettingId = p->getX() + p->getY() * tilesetData->getHorzTileCount();
			if (!tilesetData->getWallSetting(wallSettingId)) {
				//壁判定なし。
				continue;
			}
			auto pos = p->getPosition();
			auto x = p->getTileX();
			auto y = p->getTileY();
			if (x < 0 || x >= _tileMapWidth || y < 0 || y >= _tileMapHeight){
//				CCASSERT(false, "Tile position ouf of range");
				continue;
			}
			_tileMap[x + y * _tileMapWidth] = p;
		}
	}
	//プレイヤーがタイルに重なったら。
	_tileMap2 = new Tile *[_tileMapWidth * _tileMapHeight];
	memset(_tileMap2, 0, sizeof(*_tileMap2) * _tileMapWidth * _tileMapHeight);
	CCARRAY_FOREACH(_childrenList, ref) {
		auto p = dynamic_cast<Tile *>(ref);
		if (p) {
			auto pos = p->getPosition();
			auto x = p->getTileX();
			auto y = p->getTileY();
			if (x < 0 || x >= _tileMapWidth || y < 0 || y >= _tileMapHeight) {
				CCASSERT(false, "Tile position ouf of range");
				continue;
			}
			_tileMap2[x + y * _tileMapWidth] = p;
		}
	}
}

void TileMap::update(float delta)
{
	//タイムスケールをデルタ値にかける
	if (_sceneLayer->getType() == SceneLayer::kTypeMenu) {
		//メニュー関連用
		delta *= GameManager::getInstance()->getCurrentScene()->getGameSpeed()->getTimeScale(SceneGameSpeed::eTYPE_MENU);
	}
	else {
		// タイル用
		delta *= GameManager::getInstance()->getCurrentScene()->getGameSpeed()->getTimeScale(SceneGameSpeed::eTYPE_TILE);
	}

	cocos2d::Ref *ref = nullptr;
	CCARRAY_FOREACH(_updatedTilesetList, ref) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
		auto tileset = static_cast<agtk::Tileset *>(ref);
#else
		auto tileset = dynamic_cast<agtk::Tileset *>(ref);
#endif
		tileset->update(delta);
	}
	//_updatedTileListのループ中に要素を削除させたいため、ループ処理を工夫する。
	auto count = _updatedTileList->count();
	int i = 0;
	while(i < count){
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
		auto tile = static_cast<agtk::Tile *>(_updatedTileList->objectAtIndex(i));
#else
		auto tile = dynamic_cast<agtk::Tile *>(_updatedTileList->objectAtIndex(i));
#endif
		tile->update(delta);
		if (tile->getRemoveFlag()) {
			_updatedTileList->removeObject(tile);
			count--;
			continue;
		}
		i++;
	}
}

Tile *TileMap::getTile(int tileX, int tileY)
{
	if (tileX < 0 || tileX >= _tileMapWidth || tileY < 0 || tileY >= _tileMapHeight){
		return nullptr;
	}
	return _tileMap[tileX + tileY * _tileMapWidth];
}

Tile *TileMap::getTile2(int tileX, int tileY)
{
	if (tileX < 0 || tileX >= _tileMapWidth || tileY < 0 || tileY >= _tileMapHeight) {
		return nullptr;
	}
	return _tileMap2[tileX + tileY * _tileMapWidth];
}

cocos2d::__Array *TileMap::getLimitTileList()
{
	return _limitTileSet->getLimitTileList().get();
}

/**
 * タイルの参照を削除
 * @param	tileX		タイルのX座標 (エディタ上での左上からのタイルX座標)
 * @param	tileY		タイルのY座標 (エディタ上での左上からのタイルY座標)
 * @param	tileSize	タイルサイズ
 */
void TileMap::removeTileReference(int tileX, int tileY, const cocos2d::Size* tileSize)
{
	if (tileX < 0 || tileX >= _tileMapWidth || tileY < 0 || tileY >= _tileMapHeight) {
		return ;
	}
	_tileMap[tileX + tileY * _tileMapWidth] = nullptr;
}

void TileMap::removeTile2Reference(int tileX, int tileY, const cocos2d::Size *tileSize)
{
	if (tileX < 0 || tileX >= _tileMapWidth || tileY < 0 || tileY >= _tileMapHeight) {
		return;
	}
	_tileMap2[tileX + tileY * _tileMapWidth] = nullptr;
}

void TileMap::addTileReference(agtk::Tile *tile)
{
	cocos2d::Size tileSize = tile->getTileSize();
	int tileX = tile->getTileX();
	int tileY = tile->getTileY();
	//tileMap（接触）にセット
	if (tileX < 0 || tileX >= _tileMapWidth || tileY < 0 || tileY >= _tileMapHeight) {
		return;
	}
	auto projectData = GameManager::getInstance()->getProjectData();
	auto tilesetData = projectData->getTilesetData(tile->getTilesetId());
	int wallSettingId = tile->getX() + tile->getY() * tilesetData->getHorzTileCount();
	//壁判定がある場合。
	if (tilesetData->getWallSetting(wallSettingId)) {
		_tileMap[tileX + tileY * _tileMapWidth] = tile;
	}

	//tileMap2（重なったら）にセット
	_tileMap2[tileX + tileY * _tileMapWidth] = tile;
}

/**
* タイルの物理オブジェクトを塊にマージする
* @param	list			マージ対象となるタイルリスト
* @param	maxTilesInScene	シーンに配置出来るX軸Y軸のタイル数
*/
void TileMap::mergeTilePhysicsBox(cocos2d::__Array *list, const cocos2d::Vec2 maxTilesInScene)
{
	Tile *startTile = nullptr;
	auto startTileFriction = 0.5f;
	auto startTileRepulsion = 1.0f;
	auto startTilePos = Vec2(-1, -1);
	int stepX = -1;
	int stepY = -1;
	int maxStepY = -1;
	
	std::vector<std::vector<int>> boxList;


	// 一塊の物理矩形を生成するメソッド
	auto createMergedBox = [this](int idx, std::vector<std::vector<int>> &boxList, agtk::Tile *startTile, float startTileRepulsion, float startTileFriction) -> int {
		
		int nextIdx = idx;
		int listLastSize = boxList.size() > 0 ? boxList[boxList.size() - 1].size() : 0;

		// 最後のリストが空の場合
		if (listLastSize == 0) {
			// 除去
			boxList.pop_back();
		}

		// リストに格納されているタイル数を算出
		int tileNumInList = 0;
		for (auto tileList : boxList) {
			tileNumInList += tileList.size();
		}

		// 開始タイルのみの場合
		if (tileNumInList == 1) {
			// 開始タイルに通常の物理設定を行う
			startTile->setupPhysicsBody(FILLED_WALLBIT);
			((cocos2d::SpriteBatchNode *)startTile->getParent())->addPhysicsChild(startTile);
		}
		// 二つ以上のタイルがつながっている場合
		else if (tileNumInList > 1) {

			// 最後のX軸ラインと最後から２つ前のX軸ラインとの個数が異なる場合
			if (boxList.size() > 1 && boxList[boxList.size() - 2].size() != listLastSize && listLastSize > 0) {
				// 最終ラインの0番目を次のidxに設定
				nextIdx = boxList[boxList.size() - 1][0];

				// 最後のラインを除去
				boxList.pop_back();
			}

			// 開始タイルを基準に大きな矩形の物理設定を行う
			{
				// 物理演算ボックス生成
				auto tileSize = startTile->getContentSize();
				auto tileSizeHalf = tileSize * 0.5f;
				Size mergedBoxSize = Size(tileSize.width * boxList.size() - 0, tileSize.height * boxList[0].size() - 0);
				Vec2 offset = Vec2(tileSizeHalf.width * (boxList.size() - 1), -tileSizeHalf.height * (boxList[0].size() - 1));
				auto pTile = PhysicsBody::createBox(mergedBoxSize, PhysicsMaterial(0.1f, startTileRepulsion, startTileFriction), offset);

				pTile->setName("MergeTile");
				pTile->setDynamic(false);
				pTile->setGravityEnable(false);//重力不可
				pTile->setRotationEnable(false);//回転運動不可
				pTile->setGroup(GameManager::EnumPhysicsGroup::kTile);
				pTile->setContactTestBitmask(INT_MAX);
				auto physicsLayer = (startTile->getLayerId() - 1);
				if (startTile->getsceneId() == agtk::data::SceneData::kMenuSceneId) {
					physicsLayer = physicsLayer + agtk::PhysicsBase::kMenuPhysicsLayerShift;
				}
				auto bitmask = (1 << physicsLayer);
				pTile->setCategoryBitmask(bitmask);
				pTile->setCollisionBitmask(bitmask);
#ifdef USE_PHYSICS_STATIC_MASS_MOMENT
				pTile->setMass(PHYSICS_INFINITY);
				pTile->setMoment(PHYSICS_INFINITY);
#endif
				startTile->setPhysicsBody(pTile);
				startTile->unscheduleUpdate();//※ループ一元化のためここで破棄する。
				((cocos2d::SpriteBatchNode *)startTile->getParent())->addPhysicsChild(startTile);

				// マージされたタイルにマージ先(startTile)へのタイルXYを保持する
				auto tileList = this->getChildren();
				int tileNumX = mergedBoxSize.width / tileSize.width;
				int tileNumY = mergedBoxSize.height / tileSize.height;
				for (int i = 0, baseX = startTile->getTileX(); i < tileNumX; i++) {
					for (int j = 0, baseY = startTile->getTileY(); j < tileNumY; j++) {
						for (auto tileNode : tileList) {
							auto tile = dynamic_cast<agtk::Tile*>(tileNode);
							if (tile && tile->getTileX() == baseX + i && tile->getTileY() == baseY + j) {
								tile->setPhysicsMargedTileX(baseX);
								tile->setPhysicsMargedTileY(baseY);
								break;
							}
						}
					}
				}
			}
		}

		return nextIdx;
	};

	// リストを順に走査
	for (int idx = 0, idxMax = list->count(); idx < idxMax;) {
		// リストに追加したフラグ
		bool added = false;

		// 指定IDXのタイルデータを取得
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
		auto tile = static_cast<agtk::Tile *>(list->getObjectAtIndex(idx));
#else
		auto tile = dynamic_cast<agtk::Tile *>(list->getObjectAtIndex(idx));
#endif
		auto tileData = tile->getTileData();
		auto tileFriction = tileData ? (float)tileData->getPhysicsFriction() : 0.5f;
		auto tileRepulsion = tileData ? (float)tileData->getPhysicsRepulsion() : 1.0f;
		auto tilePos = Vec2(tile->getTileX(), tile->getTileY());
	
		// 開始タイルが未決定の場合
		if (!startTile) {
			// 開始タイルとして設定
			startTile = tile;
			startTilePos = tilePos;
			startTileFriction = tileFriction;
			startTileRepulsion = tileRepulsion;

			stepX = 0;
			stepY = 1;
			maxStepY = -1;
			added = true;

			// リストへ登録
			std::vector<int> xLineTileList;
			xLineTileList.push_back(idx);
			boxList.push_back(xLineTileList);

			idx++;
		}
		// 開始タイルが決定済みの場合
		else {
			// 検索中のX軸ラインとは異なるタイルの場合
			if (startTilePos.x + stepX != tilePos.x) {

				// Y軸カウントの上限が未決定 または マップ下端に到達した または Y軸カウントに到達した場合
				if (maxStepY < 0 || stepY >= maxTilesInScene.y || stepY > maxStepY) {
					// Y軸カウントの上限を設定
					maxStepY = stepY - 1;
					
					// 次のX軸ラインへ
					stepX++;
					stepY = 0;

					// 次のX軸ライン用のリストを生成
					std::vector<int> xLineTileList;
					boxList.push_back(xLineTileList);
				}
			}

			// つながっているタイルの場合
			if (startTilePos.x + stepX == tilePos.x && startTilePos.y + stepY == tilePos.y) {

				// 摩擦係数と反発係数が同じ場合
				if (startTileFriction == tileFriction && startTileRepulsion == tileRepulsion) {
					// マージ対象リストに登録
					boxList[boxList.size() - 1].push_back(idx);

					// 下のタイルへ
					stepY++;

					added = true;

					idx++;
				}

				// Y軸カウントの上限を超える場合
				if (maxStepY >= 0 && maxStepY < stepY - 1) {
					added = false;
				}
			}
		}

		// つながっていない or 末端のタイルの場合
		if (!added || (idx == idxMax)) {
			// 一塊タイルの生成
			idx = createMergedBox(idx, boxList, startTile, startTileRepulsion, startTileFriction);
			startTile = nullptr;
			boxList.clear();
		}
	}
}
NS_AGTK_END
