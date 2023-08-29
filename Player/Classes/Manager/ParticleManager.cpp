#include "2d/CCRenderTexture.h"
#include "ParticleManager.h"
#include "GameManager.h"
#include "Lib/WebSocket.h"
#include "Lib/Particle.h"
#include "Lib/Scene.h"
#include "GameScene.h"

#ifdef USE_PREVIEW
#include "Lib/SharedMemory.h"
#endif

USING_NS_CC;
USING_NS_AGTK;

//--------------------------------------------------------------------------------------------------------------------
// シングルトンのインスタンス
//--------------------------------------------------------------------------------------------------------------------
ParticleManager* ParticleManager::_particleManager = NULL;

//--------------------------------------------------------------------------------------------------------------------
// コンストラクタ
//--------------------------------------------------------------------------------------------------------------------
ParticleManager::ParticleManager()
: _particleGroupList(nullptr)
, _layerList(nullptr)
, _boundingBoxList(nullptr)
, _renderTexture(nullptr)
, _sceneLayerRenderTexture(nullptr)
, _capture(false)
, _captureOffset(-1)
, _captureSize(-1)
, _showBox(false)
, _layerIndex(0)
, _disabledLayerIdList(nullptr)
{
	setParticleGroupList(cocos2d::__Array::create());
	setLayerList(cocos2d::Array::create());
	setBoundingBoxList(cocos2d::Array::create());
	setDisabledLayerIdList(cocos2d::Array::create());
}

//--------------------------------------------------------------------------------------------------------------------
// デストラクタ
//--------------------------------------------------------------------------------------------------------------------
ParticleManager::~ParticleManager()
{
	CC_SAFE_RELEASE_NULL(_renderTexture);
	CC_SAFE_RELEASE_NULL(_boundingBoxList);
	CC_SAFE_RELEASE_NULL(_layerList);
	CC_SAFE_RELEASE_NULL(_particleGroupList);
	CC_SAFE_RELEASE_NULL(_disabledLayerIdList);
	CC_SAFE_RELEASE_NULL(_sceneLayerRenderTexture);
}

//--------------------------------------------------------------------------------------------------------------------
// インスタンスを取得する。
//--------------------------------------------------------------------------------------------------------------------
ParticleManager* ParticleManager::getInstance()
{
	if(!_particleManager){
		_particleManager = new ParticleManager();
	}
	return _particleManager;
}

//--------------------------------------------------------------------------------------------------------------------
// シングルトンを破棄する。
//--------------------------------------------------------------------------------------------------------------------
void ParticleManager::purge()
{
	if(!_particleManager){
		return;
	}
	ParticleManager *m = _particleManager;
	_particleManager = NULL;
	m->release();
}

//--------------------------------------------------------------------------------------------------------------------
// 初期化
//--------------------------------------------------------------------------------------------------------------------
bool ParticleManager::init()
{
	return true;
}

//--------------------------------------------------------------------------------------------------------------------
// パーティクル追加
// @param	sceneId				配置対象のシーンID
// @param	sceneLayerId		配置対象のシーンレイヤーID
// @param	particleId			パーティクルID
// @param	pos					パーティクル生成位置
// @param	connectionId		接続点ID
// @param	duration300			パーティクルグループの表示時間
// @param	isLoop				パーティクルグループの表示時間が無制限か？
// @param	particleDataList	パーティクルデータリスト
//--------------------------------------------------------------------------------------------------------------------

#ifdef USE_REDUCE_RENDER_TEXTURE
ParticleGroup * ParticleManager::addParticle(cocos2d::Node *followTarget, int sceneId, int sceneLayerId, int particleId, const cocos2d::Vec2 pos, int connectionId, float duration300, bool durationUnlimited, const cocos2d::__Dictionary * particleDataList, bool bForceBack, cocos2d::Node* objectNode)
#else
ParticleGroup * ParticleManager::addParticle(cocos2d::Node *followTarget, int sceneId, int sceneLayerId, int particleId, const cocos2d::Vec2 pos, int connectionId, float duration300, bool durationUnlimited, const cocos2d::__Dictionary * particleDataList, cocos2d::Node* objectNode)
#endif
{
#ifdef USE_REDUCE_RENDER_TEXTURE
	auto particleGroup = ParticleGroup::create(followTarget, sceneId, sceneLayerId, particleId, pos, connectionId, duration300, durationUnlimited, particleDataList, bForceBack, objectNode);
#else
	auto particleGroup = ParticleGroup::create(followTarget, sceneId, sceneLayerId, particleId, pos, connectionId, duration300, durationUnlimited, particleDataList, objectNode);
#endif
	if (nullptr != particleGroup) {
		_particleGroupList->addObject(particleGroup);
	}

#ifdef USE_REDUCE_RENDER_TEXTURE
	particleGroup->updateBackside();
#else
	//「画像の裏側に設定」
	auto object = dynamic_cast<agtk::Object *>(followTarget);
	if (object) {
		auto player = object->getPlayer();
		if (player) {
			particleGroup->setTargetObjctBackside(player->getTimelineBackside(connectionId));
		}
	}
#endif

	return particleGroup;
}

#ifdef USE_REDUCE_RENDER_TEXTURE
agtk::ParticleGroup * ParticleManager::addParticle(cocos2d::Node *followTarget, int sceneId, int sceneLayerId, ParticleGroupBackup *backup)
{
	auto projectData = GameManager::getInstance()->getProjectData();
	auto animationData = projectData->getAnimationData(backup->_particleId);
	auto particleList = animationData->getParticleList();
	auto particleGroup = ParticleGroup::create(followTarget, sceneId, sceneLayerId, backup->_particleId, backup->_pos, backup->_connectionId, 0, backup->_durationUnlimited, particleList, backup->_forceBack, followTarget);
	if (nullptr != particleGroup) {
		_particleGroupList->addObject(particleGroup);
	}

	particleGroup->updateBackside();

	return particleGroup;
}
#endif

//--------------------------------------------------------------------------------------------------------------------
// パーティクル削除
// @param	particleGroup	パーティクルグループ
//--------------------------------------------------------------------------------------------------------------------
void ParticleManager::removeParticle(agtk::ParticleGroup *particleGroup)
{
	auto particleGroupList = this->getParticleGroupList();
	if (particleGroupList->containsObject(particleGroup)) {
		particleGroup->changeProccess(ParticleGroup::PARTICLE_PROC_TYPE::STOP);
		particleGroup->deleteParticle();
		particleGroupList->removeObject(particleGroup);
	}
}

//--------------------------------------------------------------------------------------------------------------------
// 指定の追従先のパーティクルの削除
// @param	dest	追従先
//--------------------------------------------------------------------------------------------------------------------
void ParticleManager::removeParticlesOfFollowed(const cocos2d::Node * dest, int targetParticleId)
{
	if (nullptr == dest || nullptr == _particleGroupList || _particleGroupList->count() == 0) {
		return;
	}

	for (int i = _particleGroupList->count() - 1; i >= 0; i--) {
		auto particleGroup = dynamic_cast<ParticleGroup *>(_particleGroupList->getObjectAtIndex(i));
		bool isTarget = (targetParticleId == agtk::data::ObjectCommandParticleRemoveData::ALL_PARTICLE || particleGroup->getId() == targetParticleId);
		if (nullptr != particleGroup && particleGroup->getFollowTarget() == dest && isTarget) {
			particleGroup->changeProccess(ParticleGroup::PARTICLE_PROC_TYPE::STOP);
			particleGroup->deleteParticle();
			_particleGroupList->removeObjectAtIndex(i);
		}
	}
}

//--------------------------------------------------------------------------------------------------------------------
// パーティクルの発生停止
// dest					パーティクルを発生させているオブジェクト
// targetParticleId		停止したいパーティクルのID ( -1 で全て )
// isReset				パーティクル発生の初期化
// bFollowedNull		ターゲットをNULLにして紐付きを切り離す
//--------------------------------------------------------------------------------------------------------------------
void ParticleManager::stopEmitteParticlesOfFollowed(const cocos2d::Node * dest, int targetParticleId, bool isReset, bool bFollowedNull)
{
	if (nullptr == dest || nullptr == _particleGroupList || _particleGroupList->count() == 0) {
		return;
	}

	for (int i = _particleGroupList->count() - 1; i >= 0; i--) {
		auto particleGroup = dynamic_cast<ParticleGroup *>(_particleGroupList->getObjectAtIndex(i));
		bool isTarget = (targetParticleId == agtk::data::ObjectCommandParticleRemoveData::ALL_PARTICLE || particleGroup->getId() == targetParticleId);
		if (nullptr != particleGroup && particleGroup->getFollowTarget() == dest && isTarget) {
			particleGroup->changeProccess(ParticleGroup::PARTICLE_PROC_TYPE::STOP, isReset);
			if (bFollowedNull) {
				particleGroup->setFollowTarget(nullptr);
			}
			auto layerList = particleGroup->getLayerList();
			cocos2d::Ref *ref;
			CCARRAY_FOREACH(layerList, ref) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
				auto particle = static_cast<agtk::Particle *>(ref);
#else
				auto particle = dynamic_cast<agtk::Particle *>(ref);
#endif
				particle->setVisible(true);
			}
		}
	}
}

void ParticleManager::detachParticlesOfFollowed(const cocos2d::Node *dest, int targetParticleId)
{
	if (nullptr == dest || nullptr == _particleGroupList || _particleGroupList->count() == 0) {
		return;
	}
#ifdef USE_REDUCE_RENDER_TEXTURE
	auto destObj = dynamic_cast<agtk::Object *>((cocos2d::Node *)dest);
#endif

	for (int i = _particleGroupList->count() - 1; i >= 0; i--) {
		auto particleGroup = dynamic_cast<ParticleGroup *>(_particleGroupList->getObjectAtIndex(i));
		bool isTarget = (targetParticleId == agtk::data::ObjectCommandParticleRemoveData::ALL_PARTICLE || particleGroup->getId() == targetParticleId);
		if (nullptr != particleGroup && particleGroup->getFollowTarget() == dest && isTarget) {
			particleGroup->setFollowTarget(nullptr);
			particleGroup->setPauseEmissionFlag(true);//パーティクル生成を一時停止する。
			auto layerList = particleGroup->getLayerList();
			cocos2d::Ref *ref;
			CCARRAY_FOREACH(layerList, ref) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
				auto particle = static_cast<agtk::Particle *>(ref);
#else
				auto particle = dynamic_cast<agtk::Particle *>(ref);
#endif
				particle->setVisible(true);
				particle->setobjectNode(nullptr);
#ifdef USE_REDUCE_RENDER_TEXTURE
				if (destObj && particle->getParent() == destObj) {
					auto lRunning = destObj->isRunning();
					destObj->setRunning(false);
					particle->removeFromParentAndCleanup(false);
					auto sceneLayer = destObj->getSceneLayer();
					if (sceneLayer) {
						auto lRunning = sceneLayer->getObjectSetNode()->isRunning();
						sceneLayer->getObjectSetNode()->setRunning(false);
						sceneLayer->getObjectSetNode()->addChild(particle, destObj->getLocalZOrder());
						sceneLayer->getObjectSetNode()->setRunning(lRunning);
					}
					destObj->setRunning(lRunning);
				}
#endif
			}
		}
	}
}

void ParticleManager::addRemoveParticlesOfFollowed(const cocos2d::Node *dest, int targetParticleId, bool bAdd)
{
	if (nullptr == dest || nullptr == _particleGroupList || _particleGroupList->count() == 0) {
		return;
	}
#ifdef USE_REDUCE_RENDER_TEXTURE
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
	auto destObj = static_cast<agtk::Object *>((cocos2d::Node *)dest);
#else
	auto destObj = dynamic_cast<agtk::Object *>((cocos2d::Node *)dest);
#endif
#endif
	for (int i = _particleGroupList->count() - 1; i >= 0; i--) {
		auto particleGroup = dynamic_cast<ParticleGroup *>(_particleGroupList->getObjectAtIndex(i));
		bool isTarget = (targetParticleId == agtk::data::ObjectCommandParticleRemoveData::ALL_PARTICLE || particleGroup->getId() == targetParticleId);
		if (nullptr != particleGroup && particleGroup->getFollowTarget() == dest && isTarget) {
			auto layerList = particleGroup->getLayerList();
			cocos2d::Ref *ref;
			CCARRAY_FOREACH(layerList, ref) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
				auto particle = static_cast<agtk::Particle *>(ref);
#else
				auto particle = dynamic_cast<agtk::Particle *>(ref);
#endif
#ifdef USE_REDUCE_RENDER_TEXTURE
				auto lRunning = destObj->isRunning();
				destObj->setRunning(false);
				if (bAdd) {
					if (particle->getParentBeforeRemoveChild() == destObj) {
						destObj->addChild(particle);
					}
					particle->setParentBeforeRemoveChild(nullptr);
				} else {
					if (particle->getParent() == destObj) {
						particle->setParentBeforeRemoveChild(particle->getParent());
						particle->removeFromParentAndCleanup(false);
					}
				}
				destObj->setRunning(lRunning);
#endif
			}
		}
	}
}

//--------------------------------------------------------------------------------------------------------------------
// 全てのパーティクル削除
//--------------------------------------------------------------------------------------------------------------------
void ParticleManager::removeAllParticles()
{
	if (_particleGroupList && _particleGroupList->count() > 0) {
		for (int i = 0; i < _particleGroupList->count(); i++) {
			auto particleGroup = dynamic_cast<ParticleGroup *>(_particleGroupList->getObjectAtIndex(i));
			if (nullptr != particleGroup) {
				particleGroup->changeProccess(ParticleGroup::PARTICLE_PROC_TYPE::STOP);
				particleGroup->deleteParticle();
			}
		}
	}

	_particleGroupList->removeAllObjects();
	_disabledLayerIdList->removeAllObjects();
}


//--------------------------------------------------------------------------------------------------------------------
// エディターから送られてきたコマンドを実行する
//--------------------------------------------------------------------------------------------------------------------
#ifdef USE_PREVIEW
void ParticleManager::runCommand(const rapidjson::Value &json)
{
	// レンダーはプレビュー専用なのでここでなければ作成する
	if (!_renderTexture || !_sceneLayerRenderTexture)
	{
		Size size = Director::getInstance()->getWinSize();
		auto renderTexture = ParticleRenderTexture::create((int)size.width, (int)size.height);

		setRenderTexture(renderTexture);
		setSceneLayerRenderTexture(cocos2d::RenderTexture::create((int)size.width, (int)size.height));
	}

	auto len = json.Size();
	for(rapidjson::SizeType i = 0; i < len; i++){
		//rapidjson::SizeType memberCount = json[i].MemberCount();
		auto &commandObj = json[i];
		auto it = commandObj.MemberBegin();
		while(it != commandObj.MemberEnd()){
			auto command = cocos2d::String::create(it->name.GetString());
			auto &params = it->value;
			const char *p = command->getCString();
			if(strcmp(p, "start") == 0){
				auto pos = getScreenCenterPos(0);
				auto scene = GameManager::getInstance()->getCurrentScene();
				auto sceneCamera = scene->getCamera();// agtk::Camera
				auto camera = sceneCamera->getCamera();// cocos2d::Camera
				cocos2d::Ref *ref = nullptr;
				CCARRAY_FOREACH(this->getLayerList(), ref) {
					auto p = dynamic_cast<Particle *>(ref);
					auto emissionPos = Vec2(p->getEmitPos().x * camera->getScaleX(), p->getEmitPos().y * camera->getScaleY());
					p->setPosition(pos + emissionPos);
					p->play();
				}
				CCARRAY_FOREACH(this->getBoundingBoxList(), ref) {
					auto p = dynamic_cast<Particle *>(ref);
					auto emissionPos = Vec2(p->getEmitPos().x * camera->getScaleX(), p->getEmitPos().y * camera->getScaleY());
					p->setPosition(pos + emissionPos);
					p->play();
				}
			} else if (strcmp(p, "pause") == 0){
				cocos2d::Ref *ref = nullptr;
				CCARRAY_FOREACH(this->getLayerList(), ref) {
					auto p = dynamic_cast<Particle *>(ref);
					p->pause();
				}
				CCARRAY_FOREACH(this->getBoundingBoxList(), ref) {
					auto p = dynamic_cast<Particle *>(ref);
					p->pause();
				}
			} else if (strcmp(p, "stop") == 0){
				cocos2d::Ref *ref = nullptr;
				CCARRAY_FOREACH(this->getLayerList(), ref) {
					auto p = dynamic_cast<Particle *>(ref);
					p->stop(true);
				}
				CCARRAY_FOREACH(this->getBoundingBoxList(), ref) {
					auto p = dynamic_cast<Particle *>(ref);
					p->stop(true);
				}
			} else if (strcmp(p, "setLayers") == 0){
				//パーティクルシステムを指定数分作成する。作成済みのパーティクルシステム数の変動が発生。
				std::vector<int> layerIdList;
				std::vector<int> seedList;
				auto len = params.Size();
				for (rapidjson::SizeType i = 0; i < len; i++){
					auto layerId = params[i].IsInt() ? params[i].GetInt() : atoi(params[i].GetString());
					layerIdList.push_back(layerId);
					seedList.push_back(rand());
				}
				auto scene = GameManager::getInstance()->getCurrentScene();
				//パーティクルを追加するレイヤーを決定。
				agtk::SceneLayer *sceneLayer = nullptr;
				{
					cocos2d::DictElement *el = nullptr;
					auto sceneLayerList = scene->getSceneLayerList();
					agtk::SceneLayer *firstSceneLayer = nullptr;
					int layerIndex = sceneLayerList->count() - 1;
					CCDICT_FOREACH(sceneLayerList, el) {
						auto sl = dynamic_cast<agtk::SceneLayer *>(el->getObject());
						if (sl != nullptr) {
							if (!firstSceneLayer) {
								firstSceneLayer = sl;
							}
							if (layerIndex == _layerIndex) {
								sceneLayer = sl;
							}
						}
						layerIndex--;
					}
					if (!sceneLayer) {
						sceneLayer = firstSceneLayer;
					}
				}
				CCASSERT(sceneLayer, "SceneLayer not found");
				auto layerList = getLayerList();
				auto newLayerList = cocos2d::Array::create();
				auto boundingBoxList = getBoundingBoxList();
				auto newBoundingBoxList = cocos2d::Array::create();
				for (unsigned int i = 0; i < layerIdList.size(); i++){
					auto layerId = layerIdList[i];
					//探す
					Particle *particle = nullptr;
					for (int j = 0; j < layerList->count(); j++){
						auto p = dynamic_cast<Particle *>(layerList->getObjectAtIndex(j));
						if (p->getId() == layerId){
							particle = p;
							newLayerList->addObject(particle);
							layerList->removeObjectAtIndex(j);
							break;
						}
					}
					if (particle == nullptr){
						//無い。作る。
						particle = Particle::create(seedList[i]);
						particle->setId(layerId);
						particle->setSceneLayerId(sceneLayer->getLayerId());
						particle->stop(true);
						newLayerList->addObject(particle);
						sceneLayer->addChild(particle);
					}
					particle->setLocalZOrder(layerIdList.size() - i);
					particle = nullptr;
					for (int j = 0; j < boundingBoxList->count(); j++){
						auto p = dynamic_cast<Particle *>(boundingBoxList->getObjectAtIndex(j));
						if (p->getId() == layerId){
							particle = p;
							newBoundingBoxList->addObject(particle);
							boundingBoxList->removeObjectAtIndex(j);
							break;
						}
					}
					if (particle == nullptr){
						//無い。作る。
						particle = Particle::create(seedList[i]);
						particle->setVisible(getShowBox());
						particle->setId(layerId);
						particle->setSceneLayerId(sceneLayer->getLayerId());
						particle->stop(true);
						newBoundingBoxList->addObject(particle);
						sceneLayer->addChild(particle);
					}
					particle->setLocalZOrder(layerIdList.size() + 1);
				}
				//不要なものを削除する。
				for (int i = layerList->count() - 1; i >= 0; i--){
					auto particle = dynamic_cast<Particle *>(layerList->getObjectAtIndex(i));
					layerList->removeObjectAtIndex(i);
					sceneLayer->removeChild(particle);
				}
				this->setLayerList(newLayerList);
				for (int i = boundingBoxList->count() - 1; i >= 0; i--){
					auto particle = dynamic_cast<Particle *>(boundingBoxList->getObjectAtIndex(i));
					boundingBoxList->removeObjectAtIndex(i);
					sceneLayer->removeChild(particle);
				}
				this->setBoundingBoxList(newBoundingBoxList);
			} else if (strcmp(p, "setParameters") == 0){
				auto it = params.MemberBegin();
				auto pos = getScreenCenterPos(0);
				auto scene = GameManager::getInstance()->getCurrentScene();
				auto sceneCamera = scene->getCamera();// agtk::Camera
				auto camera = sceneCamera->getCamera();// cocos2d::Camera
				while(it != params.MemberEnd()){
					auto key = it->name.GetString();
					int layerId = atoi(key);
					auto layerList = this->getLayerList();
					for (int i = 0; i < layerList->count(); i++){
						auto particle = dynamic_cast<Particle *>(layerList->getObjectAtIndex(i));
						if (particle->getId() == layerId){
							bool bBlendAdditive = particle->isBlendAdditive();
							particle->setSkipResetSystemInSetTotalParticles(true);
							particle->setParameters(it->value, false);
							particle->setSkipResetSystemInSetTotalParticles(false);
							auto scene = GameManager::getInstance()->getCurrentScene();
							auto sceneLayer = scene->getSceneLayer(particle->getSceneLayerId());
							if (sceneLayer) {
								if ((particle->isBlendAdditive() && particle->isBlendAdditive() != bBlendAdditive && sceneLayer->getChildren().getIndex(particle) >= 0)
									|| (!particle->isBlendAdditive() && particle->isBlendAdditive() != bBlendAdditive && sceneLayer->getChildren().getIndex(particle) < 0)) {
									sceneLayer->removeChild(particle);
									sceneLayer->addChild(particle);
									auto emissionPos = Vec2(particle->getEmitPos().x * camera->getScaleX(), particle->getEmitPos().y * camera->getScaleY());
									particle->setPosition(pos + emissionPos);
									particle->play();
								}
							}
							//particle->setParameters(it->value, false);
							break;
						}
					}
					auto boundingBoxList = this->getBoundingBoxList();
					for (int i = 0; i < boundingBoxList->count(); i++){
						auto particle = dynamic_cast<Particle *>(boundingBoxList->getObjectAtIndex(i));
						if (particle->getId() == layerId){
							particle->setSkipResetSystemInSetTotalParticles(true);
							particle->setParameters(it->value, true);
							particle->setSkipResetSystemInSetTotalParticles(false);
							break;
						}
					}
					it++;
				}
			} else if (strcmp(p, "requestCapture") == 0){
				auto feedbackWay = params["feedbackWay"].GetString();
				if (strcmp(feedbackWay, "webSocket") == 0){
					setCapture(-1, -1);
					setCapture(true);
				} else if (strcmp(feedbackWay, "sharedMemory") == 0){
					auto offset = params["offset"].GetInt();
					auto size = params["size"].GetInt();
					setCapture(offset, size);
					setCapture(true);
				}

			} else if (strcmp(p, "showBox") == 0){
				bool show = params.GetBool();
				setShowBox(show);
				auto boundingBoxList = this->getBoundingBoxList();
				for (int i = 0; i < boundingBoxList->count(); i++){
					auto particle = dynamic_cast<Particle *>(boundingBoxList->getObjectAtIndex(i));
					particle->setVisible(show);
				}
			} else if (strcmp(p, "setLayerIndex") == 0){
				auto layerIndex = params.GetInt();
				if (layerIndex != getLayerIndex()) {
					setLayerIndex(layerIndex);
					//既存のパーティクルは一旦削除。
					auto layerList = getLayerList();
					auto boundingBoxList = getBoundingBoxList();
					for (int i = layerList->count() - 1; i >= 0; i--) {
						auto particle = dynamic_cast<Particle *>(layerList->getObjectAtIndex(i));
						layerList->removeObjectAtIndex(i);
						particle->getParent()->removeChild(particle);
					}
					this->setLayerList(cocos2d::Array::create());
					for (int i = boundingBoxList->count() - 1; i >= 0; i--) {
						auto particle = dynamic_cast<Particle *>(boundingBoxList->getObjectAtIndex(i));
						boundingBoxList->removeObjectAtIndex(i);
						particle->getParent()->removeChild(particle);
					}
					this->setBoundingBoxList(cocos2d::Array::create());
				}
			} else {
				CCLOG("Unsupported command: %s", p);
			}
			it++;
		}
	}
}

cocos2d::Vec2 ParticleManager::getScreenCenterPos(float delta)
{
	auto winSize = Director::getInstance()->getWinSize();
	auto scene = GameManager::getInstance()->getCurrentScene();
	auto sceneCamera = scene->getCamera();// agtk::Camera

										  // カメラの回転等を反映する為ここで更新を挟む
	sceneCamera->update(delta);

	auto camera = sceneCamera->getCamera();// cocos2d::Camera

										   // 左下を原点としたcocos2dカメラの中心座標
	auto pos = sceneCamera->getPosition();

	// パーティクルプレビューは画面中央が原点だが
	// particle の getEmitPos() は「左上原点・正の方向が右上」とした値が返ってくるので
	// 画面中央を 0 となるよう pos を調整する
	pos.x -= winSize.width * 0.5f * camera->getScaleX();
	pos.y += winSize.height * 0.5f * camera->getScaleY();
	return pos;
}

//--------------------------------------------------------------------------------------------------------------------
//! パーティクルの描画結果をキャプチャーして、エディターに送り返す。
// ※こちらのupdateはプレビュー専用です
//--------------------------------------------------------------------------------------------------------------------
void ParticleManager::update(float delta, GameManager *gameManager)
{
	if (!gameManager ||this->getLayerList()->count() == 0){
		return;
	}
#if 1
	auto scene = GameManager::getInstance()->getCurrentScene();
	auto sceneCamera = scene->getCamera();// agtk::Camera
	auto camera = sceneCamera->getCamera();// cocos2d::Camera
	auto pos = getScreenCenterPos(delta);
#else
	auto winSize = Director::getInstance()->getWinSize();
	auto scene = GameManager::getInstance()->getCurrentScene();
	auto sceneCamera = scene->getCamera();// agtk::Camera
	
	// カメラの回転等を反映する為ここで更新を挟む
	sceneCamera->update(delta);

	auto camera = sceneCamera->getCamera();// cocos2d::Camera
	
	// 左下を原点としたcocos2dカメラの中心座標
	auto pos = sceneCamera->getPosition();

	// パーティクルプレビューは画面中央が原点だが
	// particle の getEmitPos() は「左上原点・正の方向が右上」とした値が返ってくるので
	// 画面中央を 0 となるよう pos を調整する
	pos.x -= winSize.width * 0.5f * camera->getScaleX();
	pos.y += winSize.height * 0.5f * camera->getScaleY();
#endif

	cocos2d::Ref * ref = nullptr;
	bool finished = true;
	bool allStopped = true;
	CCARRAY_FOREACH(_layerList, ref) {
		auto p = dynamic_cast<Particle *>(ref);
		auto emissionPos = Vec2(p->getEmitPos().x * camera->getScaleX(), p->getEmitPos().y * camera->getScaleY());
		p->setPosition(pos + emissionPos);
		p->update(delta);
		if (!p->isFinish()){
			finished = false;
		}
		if (!p->getStopped()) {
			allStopped = false;
		}
	}
	if (this->getLayerList()->count() > 0 && finished && !allStopped) {
		auto gameManager = GameManager::getInstance();
		auto ws = gameManager ? gameManager->getWebSocket() : nullptr;
		if (ws) {
			ws->send("particle finished");
		}
		cocos2d::Ref *ref = nullptr;
		CCARRAY_FOREACH(this->getLayerList(), ref) {
			auto p = dynamic_cast<Particle *>(ref);
			if (!p->getStopped()) {
				p->stop(true);
			}
		}
		CCARRAY_FOREACH(this->getBoundingBoxList(), ref) {
			auto p = dynamic_cast<Particle *>(ref);
			if (!p->getStopped()) {
				p->stop(true);
			}
		}
	}
	CCARRAY_FOREACH(_boundingBoxList, ref) {
		auto p = dynamic_cast<Particle *>(ref);
		p->setPosition(pos + p->getEmitPos());
		p->update(delta);
	}
	if (getCapture()) {
		if (_renderTexture) {
			Size size = Director::getInstance()->getWinSize();

			//画面サイズが変わっていれば作り直す。
			if (_renderTexture->getWidth() != size.width || _renderTexture->getHeight() != size.height) {
				//CCLOG("Size: %d, %d <=> %d, %d", _renderTexture->getWidth(), _renderTexture->getHeight(), size.width, size.height);
				setRenderTexture(nullptr);
				auto renderTexture = ParticleRenderTexture::create((int)size.width, (int)size.height);
				//CCLOG("Remake rednerTexture: %p", renderTexture);
				setRenderTexture(renderTexture);
			}
			setCapture(false);

			Vec2 screenCenter = Vec2(size.width * 0.5f, size.height * 0.5f);
			auto sceneSize = scene->getSceneSize();
			auto sceneSizeCenter = sceneSize * 0.5f;

			if (_sceneLayerRenderTexture) {
				auto contentSize = _sceneLayerRenderTexture->getContentSize();
				// シーンサイズが異なっている場合再生成
				if (contentSize.width != sceneSize.x || contentSize.height != sceneSize.y) {
					setSceneLayerRenderTexture(nullptr);
					setSceneLayerRenderTexture(cocos2d::RenderTexture::create(sceneSize.x, sceneSize.y, Texture2D::PixelFormat::RGBA8888));
				}

				auto renderer = Director::getInstance()->getRenderer();
				auto sceneViewMatrix = camera->getViewMatrix();

				bool isMenu = false;

				//! カメラの移動、拡縮、回転を反映するため
				//! 背景と各シーンレイヤーをオフスクリーンにレンダリング
				_sceneLayerRenderTexture->beginWithClear(0, 0, 0, 1);
				{
					auto renderTextureCtrl = scene->getSceneTopMost()->getRenderTexture();
					auto withMenuRenderTextureCtrl = scene->getSceneTopMost()->getWithMenuRenderTexture();
					if (renderTextureCtrl || withMenuRenderTextureCtrl) {
						if (withMenuRenderTextureCtrl) {
							auto topMostSprite = withMenuRenderTextureCtrl->getLastRenderTexture()->getSprite();
							topMostSprite->setVisible(true);
							topMostSprite->visit(renderer, sceneViewMatrix, false);
							topMostSprite->setVisible(false);
						}
						else if (renderTextureCtrl) {
							auto topMostSprite = renderTextureCtrl->getLastRenderTexture()->getSprite();
							topMostSprite->setVisible(true);
							topMostSprite->visit(renderer, sceneViewMatrix, false);
							topMostSprite->setVisible(false);

							isMenu = true;
						}
					}
					else {
						// 背景
						{
							auto sceneBackground = scene->getSceneBackground();
							auto renderTextureCtrl = sceneBackground->getRenderTexture();
							if (renderTextureCtrl) {
								auto bgSprite = renderTextureCtrl->getLastRenderTexture()->getSprite();
								bgSprite->setVisible(true);
								bgSprite->visit(renderer, sceneViewMatrix, false);
								bgSprite->setVisible(false);
							}
							else {
								sceneBackground->visit(renderer, sceneViewMatrix, false);
							}
						}

						// シーンレイヤー
						{
							auto dic = scene->getSceneLayerList();
							cocos2d::DictElement *el = nullptr;
							CCDICT_FOREACH(dic, el) {
								bool isRender = false;

								auto sceneLayer = dynamic_cast<agtk::SceneLayer *>(el->getObject());
								auto renderTextureCtrl = sceneLayer->getRenderTexture();
								if (renderTextureCtrl) {
									if (renderTextureCtrl->isUseShader()) {
										isRender = true;
									}
								}

								if (isRender) {
									auto sprite = renderTextureCtrl->getLastRenderTexture()->getSprite();
									sprite->setVisible(true);
									sprite->visit(renderer, sceneViewMatrix, false);
									sprite->setVisible(false);
								}
								else {
									sceneLayer->visit(renderer, sceneViewMatrix, false);
								}
							}
						}

						isMenu = true;
					}
				}
				_sceneLayerRenderTexture->end();

				//! カメラの回転・移動・拡縮をオフスクリーンテクスチャに反映
				{
#ifdef USE_REDUCE_RENDER_TEXTURE
					auto sprite = _sceneLayerRenderTexture->getSprite();
					sprite->setPosition(screenCenter);
#else
					auto scale = Vec3::ZERO;
					auto rotate = Quaternion::identity();
					auto pos3D = Vec3::ZERO;
					sceneViewMatrix.getScale(&scale);
					sceneViewMatrix.getRotation(&rotate);
					sceneViewMatrix.getTranslation(&pos3D);

					auto sprite = _sceneLayerRenderTexture->getSprite();
					sprite->setAnchorPoint(Vec2(sceneCamera->getPosition().x / sceneSize.x, sceneCamera->getPosition().y / sceneSize.y));
					sprite->setPosition(screenCenter);
					sprite->setRotationQuat(rotate);
					sprite->setScale(scale.x, scale.y);
#endif
				}

				//! オフスクリーンテクスチャとメニューシーンを
				//! キャプチャ用テクスチャにレンダリング
				_renderTexture->beginWithClear(0, 0, 0, 1);
				{
					// オフスクリーンレンダリングしたシーン
					_sceneLayerRenderTexture->getSprite()->visit();

					if (isMenu) {
						auto menuViewMatrix = sceneCamera->getMenuCamera()->getViewMatrix();
						auto menuLayerList = scene->getMenuLayerList();
						cocos2d::DictElement *el;
						CCDICT_FOREACH(menuLayerList, el) {
							auto menuLayer = dynamic_cast<agtk::SceneLayer *>(el->getObject());
							menuLayer->visit(renderer, menuViewMatrix, false);
						}
					}
				}
				_renderTexture->end();
			}

			_renderTexture->capture([&](Image *image, cocos2d::Node *node) {
				CCLOG("captured.");
				CCLOG("image: %p", image);
				int smOffset = getCaptureOffset();
				int smSize = getCaptureSize();
				setCapture(-1, -1);
				static unsigned char *lBuf = nullptr;
				if (lBuf) {
					delete[] lBuf;
					lBuf = nullptr;
				}
				if (smOffset >= 0 && smSize > 0) {
					auto message = cocos2d::String::createWithFormat("particle feedbackCaptureInfo %d %d %d %d", image->getWidth(), image->getHeight(), smOffset, smSize);
					unsigned char *head = SharedMemory::instance()->getAreaHead(smOffset, smSize);
					if (head) {
						memcpy(head, image->getData(), std::min((int)image->getDataLen(), smSize));
						auto gameManager = GameManager::getInstance();
						auto ws = gameManager ? gameManager->getWebSocket() : nullptr;
						if (ws) {
							ws->send(message->getCString());
						}
					}
				} else {
					auto header = cocos2d::String::createWithFormat("particle feedbackCapture %d %d ", image->getWidth(), image->getHeight());
					unsigned char *buf = new unsigned char[header->length() + image->getDataLen()];
					::memcpy(buf, header->getCString(), header->length());
					::memcpy(buf + header->length(), image->getData(), image->getDataLen());
					lBuf = buf;
					auto gameManager = GameManager::getInstance();
					auto ws = gameManager ? gameManager->getWebSocket() : nullptr;
					if (ws) {
						ws->send(buf, header->length() + image->getDataLen());
					}
				}
			});
		}
		else
		{
			setCapture(false);
		}
	}
}
#endif

//--------------------------------------------------------------------------------------------------------------------
// パーティクルの更新
// @param	delta	前フレームからの経過時間
//--------------------------------------------------------------------------------------------------------------------
void ParticleManager::update(float delta)
{
	//CCLOG("ParticleManager::update(%f)", delta);
	if (nullptr == _particleGroupList || _particleGroupList->count() == 0) {
		return;
	}

	for (int i = _particleGroupList->count() - 1; i >= 0; i--) {
		auto particleGroup = dynamic_cast<ParticleGroup *>(_particleGroupList->getObjectAtIndex(i));

		if (nullptr != particleGroup) {
			if (particleGroup->isStopped()) {
				particleGroup->deleteParticle();
				_particleGroupList->removeObjectAtIndex(i);
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
					if (particleGroup->getSceneId() != agtk::data::SceneData::kMenuSceneId && particleGroup->getSceneLayerId() == p->getValue()) {
						isDisabled = true;
						break;
					}
				}
				if (!isDisabled) {
					// タイムスケールをデルタ値にかける
					if (particleGroup->getSceneId() == agtk::data::SceneData::kMenuSceneId) {
						//メニュー関連用
						particleGroup->update(delta * GameManager::getInstance()->getCurrentScene()->getGameSpeed()->getTimeScale(SceneGameSpeed::eTYPE_MENU));
					}
					else {
						//エフェクト用
						particleGroup->update(delta * GameManager::getInstance()->getCurrentScene()->getGameSpeed()->getTimeScale(SceneGameSpeed::eTYPE_EFFECT));
					}
				}
			}
		}
	}
}

//--------------------------------------------------------------------------------------------------------------------
//! キャプチャー情報を設定する。(SharedMemoryの情報を格納する目的。)
//--------------------------------------------------------------------------------------------------------------------
void ParticleManager::setCapture(int offset, int size)
{
	setCaptureOffset(offset);
	setCaptureSize(size);
}

//--------------------------------------------------------------------------------------------------------------------
// 指定したパーティクルグループの実態が存在するか？
// @return true = 実態が存在する false = 実態が存在しない
//--------------------------------------------------------------------------------------------------------------------
bool ParticleManager::existsParticleGroup(ParticleGroup* particle)
{
	bool result = false;

	if (_particleGroupList != nullptr && _particleGroupList->count() > 0 && particle != nullptr) {
		result = _particleGroupList->containsObject(particle);
	}

	return result;
}

void ParticleManager::setVisible(int layerId, bool visible)
{
	for (int i = _particleGroupList->count() - 1; i >= 0; i--) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
		auto particleGroup = static_cast<ParticleGroup *>(_particleGroupList->getObjectAtIndex(i));
#else
		auto particleGroup = dynamic_cast<ParticleGroup *>(_particleGroupList->getObjectAtIndex(i));
#endif
		if (particleGroup->getSceneLayerId() == layerId) {
			auto layerList = particleGroup->getLayerList();
			cocos2d::Ref *ref;
			CCARRAY_FOREACH(layerList, ref) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
				auto particle = static_cast<agtk::Particle *>(ref);
#else
				auto particle = dynamic_cast<agtk::Particle *>(ref);
#endif
				particle->setVisible(visible);
			}
		}
	}
}

void ParticleManager::setAlpha(cocos2d::Node *sceneLayer, float alpha)
{
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
	auto tmpSceneLayer = static_cast<agtk::SceneLayer *>(sceneLayer);
#else
	auto tmpSceneLayer = dynamic_cast<agtk::SceneLayer *>(sceneLayer);
#endif
	for (int i = _particleGroupList->count() - 1; i >= 0; i--) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
		auto particleGroup = static_cast<ParticleGroup *>(_particleGroupList->getObjectAtIndex(i));
#else
		auto particleGroup = dynamic_cast<ParticleGroup *>(_particleGroupList->getObjectAtIndex(i));
#endif
		int sceneId = tmpSceneLayer->getSceneData()->getId();
		int sceneLayerId = tmpSceneLayer->getLayerId();
		if (particleGroup->getSceneId() == sceneId && particleGroup->getSceneLayerId() == sceneLayerId) {
			auto layerList = particleGroup->getLayerList();
			cocos2d::Ref *ref;
			CCARRAY_FOREACH(layerList, ref) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
				auto particle = static_cast<agtk::Particle *>(ref);
#else
				auto particle = dynamic_cast<agtk::Particle *>(ref);
#endif
				particle->setAlpha(alpha);
			}
		}
	}
}

cocos2d::__Array *ParticleManager::getParticleArray(cocos2d::Node *object)
{
	auto list = cocos2d::__Array::create();
	cocos2d::Ref *ref;
	CCARRAY_FOREACH(_particleGroupList, ref) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
		auto particle = static_cast<agtk::ParticleGroup *>(ref);
#else
		auto particle = dynamic_cast<agtk::ParticleGroup *>(ref);
#endif
		if (particle->getFollowTarget() == object) {
			list->addObject(particle);
		}
	}
	return list;
}

#ifdef USE_REDUCE_RENDER_TEXTURE
cocos2d::__Array *ParticleManager::getParticleGroupBackupList(cocos2d::Node *object)
{
	auto list = cocos2d::__Array::create();
	cocos2d::Ref *ref;
	CCARRAY_FOREACH(_particleGroupList, ref) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
		auto particleGroup = static_cast<agtk::ParticleGroup *>(ref);
#else
		auto particleGroup = dynamic_cast<agtk::ParticleGroup *>(ref);
#endif
		if (particleGroup->getFollowTarget() == object) {
			//オブジェクトのエフェクトでON/OFFされるパーティクルはバックアップ不要。
			bool isObjectEffect = false;
			auto obj = dynamic_cast<agtk::Object *>(object);
			if (obj && obj->getObjectData()->getEffectSettingFlag()) {
				cocos2d::Ref *ref = nullptr;
				CCARRAY_FOREACH(obj->getObjectEffectList(), ref) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
					auto objectEffect = static_cast<agtk::ObjectEffect*>(ref);
#else
					auto objectEffect = dynamic_cast<agtk::ObjectEffect*>(ref);
#endif
					if (objectEffect->getParticleGroup() == particleGroup) {
						isObjectEffect = true;
						break;
					}
				}
			}
			if (!isObjectEffect && !particleGroup->isStopped() && particleGroup->isActive() && particleGroup->getIsDurationUnlimited()) {
				bool isLoop = false;
				cocos2d::Ref *ref = nullptr;
				auto layerList = particleGroup->getLayerList();
				CCARRAY_FOREACH(layerList, ref) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
					auto particle = static_cast<agtk::Particle *>(ref);
#else
					auto particle = dynamic_cast<agtk::Particle *>(ref);
#endif
					if (particle->getLoop()) {
						isLoop = true;
						break;
					}
				}
				if (isLoop) {
					auto particleGroupBackup = ParticleManager::ParticleGroupBackup::create(particleGroup);
					list->addObject(particleGroupBackup);
				}
			}
		}
	}
	return list;
}
#endif

#ifdef USE_PREVIEW
//--------------------------------------------------------------------------------------------------------------------
//! プレビュー用のデータをクリアする
//--------------------------------------------------------------------------------------------------------------------
void ParticleManager::clearPreview()
{
	//CCLOG("%s: %d called", __FUNCTION__, __LINE__);
	for (int i = _layerList->count() - 1; i >= 0; i--) {
		auto particle = dynamic_cast<Particle *>(_layerList->getObjectAtIndex(i));
		_layerList->removeObjectAtIndex(i);
		particle->getParent()->removeChild(particle);
	}
	for (int i = _boundingBoxList->count() - 1; i >= 0; i--) {
		auto particle = dynamic_cast<Particle *>(_boundingBoxList->getObjectAtIndex(i));
		_boundingBoxList->removeObjectAtIndex(i);
		particle->getParent()->removeChild(particle);
	}
}
#endif


/////////////////////////////////////////////////////////////////////
#ifdef USE_REDUCE_RENDER_TEXTURE
ParticleManager::ParticleGroupBackup::ParticleGroupBackup()
: _particleId(-1)
, _pos()
, _connectionId(-1)
, _duration300(0)
, _durationUnlimited(false)
, _forceBack(false)
{
}

ParticleManager::ParticleGroupBackup::~ParticleGroupBackup()
{
}

ParticleManager::ParticleGroupBackup *ParticleManager::ParticleGroupBackup::create(agtk::ParticleGroup *group)
{
	auto p = new ParticleGroupBackup();
	p->init(group);
	return p;
}

void ParticleManager::ParticleGroupBackup::init(agtk::ParticleGroup *group)
{
	this->_particleId = group->getId();
	this->_pos = group->getFollowAdjustPos();
	this->_connectionId = group->getConnectionId();
	this->_duration300 = group->getDuration300();
	this->_durationUnlimited = group->getIsDurationUnlimited();
	this->_forceBack = group->getForceBack();
}
#endif

/////////////////////////////////////////////////////////////////////
ParticleRenderTexture::ParticleRenderTexture()
: _captureCommand()
, _captureCallback(nullptr)
, _width(0)
, _height(0)
{
	_node = nullptr;
}

ParticleRenderTexture::~ParticleRenderTexture()
{
}

void ParticleRenderTexture::capture(std::function<void(Image *, cocos2d::Node *node)> callback)
{
	_captureCallback = callback;

	//std::string fullpath = FileUtils::getInstance()->getWritablePath() + fileName;
	_captureCommand.init(_globalZOrder);
	_captureCommand.func = CC_CALLBACK_0(ParticleRenderTexture::onCapture, this);

	Director::getInstance()->getRenderer()->addCommand(&_captureCommand);
}

void ParticleRenderTexture::onCapture()
{
	Image *image = newImage(true);
	if (image)
	{
		if (_captureCallback)
		{
			_captureCallback(image, _node);
		}
	}
	CC_SAFE_DELETE(image);
}

ParticleRenderTexture *ParticleRenderTexture::create(int w, int h)
{
	ParticleRenderTexture *ret = new (std::nothrow) ParticleRenderTexture();

	if (ret && ret->initWithWidthAndHeight(w, h, Texture2D::PixelFormat::RGBA8888, 0))
	{
		ret->setWidth(w);
		ret->setHeight(h);
		ret->autorelease();
		return ret;
	}
	CC_SAFE_DELETE(ret);
	return nullptr;
}

