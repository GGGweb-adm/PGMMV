#include "EffectManager.h"
#include "Lib/Scene.h"
#include "GameManager.h"

USING_NS_CC;
USING_NS_AGTK;

//--------------------------------------------------------------------------------------------------------------------
// シングルトンのインスタンス
//--------------------------------------------------------------------------------------------------------------------
EffectManager* EffectManager::_effectManager = NULL;

//--------------------------------------------------------------------------------------------------------------------
// コンストラクタ
//--------------------------------------------------------------------------------------------------------------------
EffectManager::EffectManager()
{
	_effectList = nullptr;
	_disabledLayerIdList = nullptr;
}

//--------------------------------------------------------------------------------------------------------------------
// デストラクタ
//--------------------------------------------------------------------------------------------------------------------
EffectManager::~EffectManager()
{
	CC_SAFE_RELEASE_NULL(_disabledLayerIdList);
	CC_SAFE_RELEASE_NULL(_effectList);
}

//--------------------------------------------------------------------------------------------------------------------
// インスタンスを取得する。
//--------------------------------------------------------------------------------------------------------------------
EffectManager* EffectManager::getInstance()
{
	if (!_effectManager) {
		_effectManager = new EffectManager();
		_effectManager->init();
	}
	return _effectManager;
}

//--------------------------------------------------------------------------------------------------------------------
// シングルトンを破棄する。
//--------------------------------------------------------------------------------------------------------------------
void EffectManager::purge()
{
	if (!_effectManager) {
		return;
	}
	EffectManager *m = _effectManager;
	_effectManager = NULL;
	m->release();
}

//--------------------------------------------------------------------------------------------------------------------
// 初期化
//--------------------------------------------------------------------------------------------------------------------
bool EffectManager::init()
{
	setEffectList(cocos2d::Array::create());
	setDisabledLayerIdList(cocos2d::Array::create());
	return true;
}

//--------------------------------------------------------------------------------------------------------------------
// エフェクトの更新
// @param	delta	前フレームからの経過時間
//--------------------------------------------------------------------------------------------------------------------
void EffectManager::update(float delta)
{
	if (_effectList == nullptr || _effectList->count() <= 0) {
		return;
	}

	for (int i = _effectList->count() - 1; i >= 0; i--) {
		auto effect = dynamic_cast<EffectAnimation *>(_effectList->getObjectAtIndex(i));

		if (effect != nullptr) {
			if (effect->isDeletable()) {
				effect->remove();
				_effectList->removeObjectAtIndex(i);
			}
			else {
				bool isDisabled = false;
				cocos2d::Ref *ref = nullptr;
				CCARRAY_FOREACH(_disabledLayerIdList, ref) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
					auto p = static_cast<cocos2d::Integer *>(ref);
#else
					auto p = dynamic_cast<cocos2d::Integer *>(ref);
#endif
					if ((!effect->_targetObject || effect->_targetObject->getSceneData()->getId() != agtk::data::SceneData::kMenuSceneId) && effect->_sceneLayerId == p->getValue()) {
						isDisabled = true;
						break;
					}
				}
				if (!isDisabled) {
					agtk::SceneLayer *sceneLayer = nullptr;
					auto object = effect->getTargetObject();
					if (object != nullptr) {
						sceneLayer = object->getSceneLayer();
					}
					else {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
#ifdef USE_REDUCE_RENDER_TEXTURE
						sceneLayer = static_cast<agtk::SceneLayer *>(effect->getParent()->getParent());
#else
						sceneLayer = static_cast<agtk::SceneLayer *>(effect->getParent());
#endif
#else
#ifdef USE_REDUCE_RENDER_TEXTURE
						sceneLayer = dynamic_cast<agtk::SceneLayer *>(effect->getParent()->getParent());
#else
						sceneLayer = dynamic_cast<agtk::SceneLayer *>(effect->getParent());
#endif
#endif
					}
					if (sceneLayer != nullptr) {
						// タイムスケールをデルタ値にかける
						if (sceneLayer->getType() == agtk::SceneLayer::kTypeMenu) {
							//メニュー関連
							effect->update(delta * GameManager::getInstance()->getCurrentScene()->getGameSpeed()->getTimeScale(SceneGameSpeed::eTYPE_MENU));
						}
						else {
							//エフェクト用
							effect->update(delta * GameManager::getInstance()->getCurrentScene()->getGameSpeed()->getTimeScale(SceneGameSpeed::eTYPE_EFFECT));
						}
					}
				}
			}
		}
	}
}

//--------------------------------------------------------------------------------------------------------------------
// エフェクトアニメーションを追加
// @param	sceneLayerId	配置対象のシーンレイヤーID
// @param	duration300		エフェクトの表示時間（0未満を設定した場合はアニメーション再生完了後に自動削除される）
// @param	animationData	エフェクトアニメーションの情報が格納されたアニメーションデータ
// @param	zOrder			Zオーダー
//--------------------------------------------------------------------------------------------------------------------
EffectAnimation* EffectManager::addEffectAnimation(int sceneLayerId, int duration300, agtk::data::AnimationData *animationData, int zOrder)
{
	if (animationData == nullptr) {
		return nullptr;
	}

	// エフェクトアニメーションのみ処理する
	if (animationData->getType() == agtk::data::AnimationData::kEffect) {
		auto effect = agtk::EffectAnimation::create(sceneLayerId, animationData, zOrder);
		effect->setDuration300(duration300);
		effect->setIsCheckDuration(duration300 >= 0);

		_effectList->addObject(effect);

		return effect;
	}

	return nullptr;
}

#ifdef USE_REDUCE_RENDER_TEXTURE
//--------------------------------------------------------------------------------------------------------------------
// オブジェクトにエフェクトアニメーションを追加
// @param	object	接続させるオブジェクト
// @param	duration300		エフェクトの表示時間（0未満を設定した場合はアニメーション再生完了後に自動削除される）
// @param	animationData	エフェクトアニメーションの情報が格納されたアニメーションデータ
// @param	zOrder			Zオーダー
//--------------------------------------------------------------------------------------------------------------------
// オブジェクトにエフェクトアニメーションを生成
EffectAnimation *EffectManager::addEffectAnimation(agtk::Object *object, int duration300, agtk::data::AnimationData * animationData, int zOrder, bool bForceBack)
{
	if (animationData == nullptr) {
		return nullptr;
	}

	// エフェクトアニメーションのみ処理する
	if (animationData->getType() == agtk::data::AnimationData::kEffect) {
		auto effect = agtk::EffectAnimation::create(object, animationData, zOrder, bForceBack);
		effect->setDuration300(duration300);
		effect->setIsCheckDuration(duration300 >= 0);

		_effectList->addObject(effect);

		return effect;
	}

	return nullptr;
}
#endif

//--------------------------------------------------------------------------------------------------------------------
// エフェクトアニメーションを追加
// ※指定座標に生成
// @param	pos				指定座標
// @param	sceneLayerId	配置対象のシーンレイヤーID
// @param	duration300		エフェクトの表示時間（0未満を設定した場合はアニメーション再生完了後に自動削除される）
// @param	animationData	エフェクトアニメーションの情報が格納されたアニメーションデータ
//--------------------------------------------------------------------------------------------------------------------
EffectAnimation* EffectManager::addEffectAnimation(cocos2d::Vec2 pos, int sceneLayerId, int duration300, agtk::data::AnimationData *animationData)
{
	// エフェクトを生成
	EffectAnimation *effect = addEffectAnimation(sceneLayerId, duration300, animationData, 0);

	if (effect) {
		effect->setPosition(agtk::Scene::getPositionSceneFromCocos2d(pos));
	}

	return effect;
}

//--------------------------------------------------------------------------------------------------------------------
// エフェクトアニメーションを追加
// ※targetObjectに指定したオブジェクトに追従させる
// @param	targetObject	追従対象のオブジェクト
// @param	offset			調整位置
// @param	duration300		エフェクトの表示時間（0未満を設定した場合はアニメーション再生完了後に自動削除される）
// @param	animationData	エフェクトアニメーションの情報が格納されたアニメーションデータ
//--------------------------------------------------------------------------------------------------------------------
#ifdef USE_REDUCE_RENDER_TEXTURE
EffectAnimation* EffectManager::addEffectAnimation(agtk::Object *targetObject, cocos2d::Vec2 offset, int duration300, agtk::data::AnimationData *animationData, bool bForceBack)
{
	return addEffectAnimation(targetObject, offset, -1, duration300, animationData, bForceBack);
}
#else
EffectAnimation* EffectManager::addEffectAnimation(agtk::Object *targetObject, cocos2d::Vec2 offset, int duration300, agtk::data::AnimationData *animationData)
{
	return addEffectAnimation(targetObject, offset, -1, duration300, animationData);;
}
#endif

//--------------------------------------------------------------------------------------------------------------------
// エフェクトアニメーションを追加
// ※targetObjectに指定したオブジェクトに追従させる（接続点の設定あり）
// @param	targetObject	追従対象のオブジェクト
// @param	offset			調整位置
// @param	connectId		接続点のID（使用しないなら-1を設定する）
// @param	duration300		エフェクトの表示時間（0未満を設定した場合はアニメーション再生完了後に自動削除される）
// @param	animationData	エフェクトアニメーションの情報が格納されたアニメーションデータ
//--------------------------------------------------------------------------------------------------------------------
#ifdef USE_REDUCE_RENDER_TEXTURE
EffectAnimation* EffectManager::addEffectAnimation(agtk::Object *targetObject, cocos2d::Vec2 offset, int connectId, int duration300, agtk::data::AnimationData *animationData, bool bForceBack)
#else
EffectAnimation* EffectManager::addEffectAnimation(agtk::Object *targetObject, cocos2d::Vec2 offset, int connectId, int duration300, agtk::data::AnimationData *animationData)
#endif
{
	// エフェクトを生成
	int zOrder = targetObject->getLocalZOrder();
#ifdef USE_REDUCE_RENDER_TEXTURE
	EffectAnimation *effect = addEffectAnimation(targetObject, duration300, animationData, zOrder, bForceBack);
#else
	EffectAnimation *effect = addEffectAnimation(targetObject->getLayerId(), duration300, animationData, zOrder);
#endif

	if (effect) {
		effect->setTargetObject(targetObject);
		effect->setTargetObjectConnectId(connectId);
		effect->setOffset(offset);
#ifdef USE_REDUCE_RENDER_TEXTURE
		effect->updateBackside();
		if (bForceBack) {
			effect->setPosition(agtk::Scene::getPositionSceneFromCocos2d(offset));
		}
#else
		//「画像の裏側に設定」
		if (targetObject->getPlayer()) {
			effect->setTargetObjctBackside(targetObject->getPlayer()->getTimelineBackside(connectId));
		}
#endif

		effect->updatePosition();
	}

	return effect;
}

#ifdef USE_REDUCE_RENDER_TEXTURE
agtk::EffectAnimation *EffectManager::addEffectAnimation(agtk::Object *targetObject, EffectBackup *backup)
{
	auto projectData = GameManager::getInstance()->getProjectData();
	auto animationData = projectData->getAnimationData(backup->_effectId);
	return addEffectAnimation(targetObject, backup->_offset, backup->_connectionId, -1, animationData, backup->_forceBack);
}
#endif

//--------------------------------------------------------------------------------------------------------------------
// 指定オブジェクトに追従しているエフェクトを削除する
//--------------------------------------------------------------------------------------------------------------------
void EffectManager::removeEffect(agtk::Object *object, int effectId, bool bRemoveInstance)
{
	for (int i = _effectList->count() - 1; i >= 0; i--) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
		auto effect = static_cast<EffectAnimation *>(_effectList->getObjectAtIndex(i));
#else
		auto effect = dynamic_cast<EffectAnimation *>(_effectList->getObjectAtIndex(i));
#endif
		if (effect->getPlayer()->getAnimationData()->getId() == effectId) {
			if (effect) {
				// 指定オブジェクトのIDと同じオブジェクトのIDの場合、リストから削除する
				if (object == effect->getTargetObject()) {
					if (object->getId() == effect->getTargetObject()->getId()) {
						effect->setTargetObject(nullptr);
						if (bRemoveInstance) {
							effect->remove();
							_effectList->removeObjectAtIndex(i);
						}
					}
				}
			}
			else {
				_effectList->removeObjectAtIndex(i);
			}
		}
	}
}

//--------------------------------------------------------------------------------------------------------------------
// 指定オブジェクトに追従するエフェクトをすべて削除する。
//--------------------------------------------------------------------------------------------------------------------
void EffectManager::removeEffectAll(agtk::Object *object, bool bRemoveInstance)
{
	for (int i = _effectList->count() - 1; i >= 0; i--) {
		auto effect = dynamic_cast<EffectAnimation *>(_effectList->getObjectAtIndex(i));
		if (effect) {
			// 指定オブジェクトのIDと同じオブジェクトのIDの場合、リストから削除する
			if (object == effect->getTargetObject()) {
				if (object->getId() == effect->getTargetObject()->getId()) {
					if (bRemoveInstance) {
						effect->remove();
						_effectList->removeObjectAtIndex(i);
					}
					effect->setTargetObject(nullptr);
				}
			}
		}
		else {
			_effectList->removeObjectAtIndex(i);
		}
	}
}

//--------------------------------------------------------------------------------------------------------------------
// 全てのエフェクトを削除
//--------------------------------------------------------------------------------------------------------------------
void EffectManager::clearEffect()
{
	_effectList->removeAllObjects();
	_disabledLayerIdList->removeAllObjects();
}

//--------------------------------------------------------------------------------------------------------------------
// 指定したエフェクトが存在するか？
//--------------------------------------------------------------------------------------------------------------------
bool EffectManager::existsEffect(agtk::EffectAnimation* effect) 
{
	bool result = false;

	if (_effectList != nullptr && _effectList->count() > 0 && effect != nullptr)
	{
		result = _effectList->containsObject(effect);
	}

	return result;
}

cocos2d::__Array *EffectManager::getEffectArray(agtk::Object *object)
{
	auto list = cocos2d::__Array::create();
	if (_effectList == nullptr) {
		return list;
	}
	cocos2d::Ref *ref;
	CCARRAY_FOREACH(_effectList, ref) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
		auto p = static_cast<agtk::EffectAnimation *>(ref);
#else
		auto p = dynamic_cast<agtk::EffectAnimation *>(ref);
#endif
		if (p->getTargetObject() == object) {
			list->addObject(p);
		}
	}
	return list;
}

cocos2d::__Array *EffectManager::getEffectArray(agtk::SceneLayer *sceneLayer)
{
	auto list = cocos2d::__Array::create();
	if (_effectList == nullptr) {
		return list;
	}
	cocos2d::Ref *ref;
	CCARRAY_FOREACH(_effectList, ref) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
		auto p = static_cast<agtk::EffectAnimation *>(ref);
#else
		auto p = dynamic_cast<agtk::EffectAnimation *>(ref);
#endif
		auto object = p->getTargetObject();
		if(object && object->getSceneLayer() == sceneLayer) {
			list->addObject(p);
		}
	}
	return list;
}

void EffectManager::setAlpha(agtk::SceneLayer *sceneLayer , float alpha)
{
	auto list = this->getEffectArray(sceneLayer);
	cocos2d::Ref *ref;
	CCARRAY_FOREACH(list, ref) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
		auto p = static_cast<agtk::EffectAnimation *>(ref);
#else
		auto p = dynamic_cast<agtk::EffectAnimation *>(ref);
#endif
		p->setMainAlpha(alpha);
	}
}

#ifdef USE_REDUCE_RENDER_TEXTURE
cocos2d::__Array *EffectManager::getEffectBackupList(agtk::Object *object)
{
	auto list = cocos2d::__Array::create();
	if (_effectList == nullptr) {
		return list;
	}
	cocos2d::Ref *ref;
	CCARRAY_FOREACH(_effectList, ref) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
		auto effect = static_cast<agtk::EffectAnimation *>(ref);
#else
		auto effect = dynamic_cast<agtk::EffectAnimation *>(ref);
#endif
		if (effect->getTargetObject() != object) {
			continue;
		}
		//オブジェクトのエフェクトでON/OFFされるエフェクトはバックアップ不要。
		bool isObjectEffect = false;
		if (object->getObjectData()->getEffectSettingFlag()) {
			cocos2d::Ref *ref = nullptr;
			CCARRAY_FOREACH(object->getObjectEffectList(), ref) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
				auto objectEffect = static_cast<agtk::ObjectEffect*>(ref);
#else
				auto objectEffect = dynamic_cast<agtk::ObjectEffect*>(ref);
#endif
				if (objectEffect->getEffectAnimation() == effect) {
					isObjectEffect = true;
					break;
				}
			}
		}
		if (isObjectEffect) {
			continue;
		}
		if (effect->isStopped()) {
			continue;
		}
		if (effect->getIsCheckDuration()) {
			continue;
		}
		auto player = effect->getPlayer();
		if (!player) {
			continue;
		}
		auto animationData = player->getAnimationData();
		if (!animationData) {
			continue;
		}
		if (!animationData->getInfiniteLoop()) {
			continue;
		}
		auto effectBackup = EffectManager::EffectBackup::create(effect);
		list->addObject(effectBackup);
	}
	return list;
}
#endif

//////////////////////////////////////////////////////////////////////////
#ifdef USE_REDUCE_RENDER_TEXTURE
EffectManager::EffectBackup::EffectBackup()
: _effectId(-1)
, _offset()
, _connectionId(-1)
, _duration300(0)
, _forceBack(false)
{
}

EffectManager::EffectBackup::~EffectBackup()
{
}

EffectManager::EffectBackup *EffectManager::EffectBackup::create(agtk::EffectAnimation *effect)
{
	auto p = new EffectBackup();
	p->init(effect);
	return p;
}

void EffectManager::EffectBackup::init(agtk::EffectAnimation *effect)
{
	auto player = effect->getPlayer();
	if (player) {
		auto animationData = player->getAnimationData();
		if (animationData) {
			this->_effectId = animationData->getId();
		}
	}
	this->_offset = effect->getOffset();
	this->_connectionId = effect->getTargetObjectConnectId();
	this->_duration300 = effect->getDuration300();
	this->_forceBack = effect->getTargetObjctBackside();
}
#endif
