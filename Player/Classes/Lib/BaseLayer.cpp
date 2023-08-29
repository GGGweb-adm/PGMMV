#include "BaseLayer.h"
#include "Lib/Macros.h"
#include "Lib/Scene.h"
#include "Lib/ViewportLight.h"
#include "Manager/GameManager.h"

NS_AGTK_BEGIN

//-------------------------------------------------------------------------------------------------------------------
TimerFloat::TimerFloat()
{
	_initValue = 0.0f;
	_value = 0.0f;
	_oldValue = 0.0f;
	_nextValue = 0.0f;
	_prevValue = 0.0f;
}

bool TimerFloat::init(float value)
{
	if (agtk::EventTimer::init() == false) {
		return false;
	}
	_initValue = value;
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

float TimerFloat::setValue(float value, float seconds)
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

float TimerFloat::addValue(float value, float seconds)
{
	auto nowValue = this->getValue();
	this->setValue(nowValue + value, seconds);
	return nowValue;
}

void TimerFloat::reset()
{
	auto value = _initValue;
	_value = value;
	_oldValue = value;
	_prevValue = value;
	_nextValue = value;
}

//-------------------------------------------------------------------------------------------------------------------
TimerVec2::TimerVec2() : agtk::EventTimer()
{
	_initValue = cocos2d::Vec2::ZERO;
	_value = cocos2d::Vec2::ZERO;
	_oldValue = cocos2d::Vec2::ZERO;
	_prevValue = cocos2d::Vec2::ZERO;
	_nextValue = cocos2d::Vec2::ZERO;
}

bool TimerVec2::init(cocos2d::Vec2 value)
{
	if (agtk::EventTimer::init() == false) {
		return false;
	}
	_initValue = value;
	_value = value;
	_prevValue = value;
	_nextValue = value;
	_oldValue = value;

	this->setProcessingFunc([&](float dt) {
		_oldValue = _value;
		//_value = AGTK_LINEAR_INTERPOLATE(_prevValue, _nextValue, _seconds, _timer);
		_value = AGTK_PARABOLA_INTERPOLATE2(_prevValue, _nextValue, _seconds, _timer);
	});
	this->setEndFunc([&]() {
		_oldValue = _value;
		_value = _nextValue;
	});

	return true;
}

cocos2d::Vec2 TimerVec2::setValue(cocos2d::Vec2 value, float seconds)
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

cocos2d::Vec2 TimerVec2::addValue(cocos2d::Vec2 value, float seconds)
{
	auto v = this->getValue();
	this->setValue(v + value, seconds);
	return v;
}

void TimerVec2::reset()
{
	auto value = _initValue;
	_value = value;
	_oldValue = value;
	_prevValue = value;
	_nextValue = value;
}

//-------------------------------------------------------------------------------------------------------------------
BaseLayer::BaseLayer() : cocos2d::Layer()
{
	_args = nullptr;
	_timerRotation = nullptr;
	_timerScale = nullptr;
	_timerAnchorPoint = nullptr;
	_timerFlipX = nullptr;
	_timerFlipY = nullptr;
	this->setTimerRotation(agtk::TimerFloat::create(0.0f));
	this->setTimerScale(agtk::TimerVec2::create(cocos2d::Vec2(1, 1)));
	this->setTimerAnchorPoint(agtk::TimerVec2::create(cocos2d::Vec2(0.5, 0.5)));
	this->setTimerFlipX(agtk::TimerFlip::create());
	this->setTimerFlipY(agtk::TimerFlip::create());
}

BaseLayer::~BaseLayer()
{
	CC_SAFE_RELEASE_NULL(_args);
	CC_SAFE_RELEASE_NULL(_timerRotation);
	CC_SAFE_RELEASE_NULL(_timerScale);
	CC_SAFE_RELEASE_NULL(_timerAnchorPoint);
	CC_SAFE_RELEASE_NULL(_timerFlipX);
	CC_SAFE_RELEASE_NULL(_timerFlipY);
	removeAllChildrenWithCleanup(true);
}

void BaseLayer::update(float delta)
{
	//回転
	auto timerRotation = this->getTimerRotation();
	timerRotation->update(delta);
	this->setRotation(timerRotation->getValue());
	//スケール
	auto timerScale = this->getTimerScale();
	timerScale->update(delta);
	auto scale = timerScale->getValue();
	//左右反転
	auto timerFlipX = this->getTimerFlipX();
	timerFlipX->update(delta);
	this->setScaleX(timerFlipX->getScale() * scale.x);
	//上下反転
	auto timerFlipY = this->getTimerFlipY();
	timerFlipY->update(delta);
	this->setScaleY(timerFlipY->getScale() * scale.y);
	//アンカーポイント
	auto timerAnchorPoint = this->getTimerAnchorPoint();
	timerAnchorPoint->update(delta);
	Layer::setAnchorPoint(timerAnchorPoint->getValue());
}

void BaseLayer::setRotation(float rotate)
{
	cocos2d::Layer::setRotation(rotate);
}

void BaseLayer::setRotation(float rotate, cocos2d::Vec2 anchorPoint, float duration)
{
	this->getTimerRotation()->setValue(rotate, duration);
	this->setAnchorPoint(anchorPoint);
}

void BaseLayer::setFlipX(bool bFlipX, cocos2d::Vec2 anchorPoint, float duration)
{
	float scaleX = this->getScaleX();
	this->getTimerFlipX()->start(TimerFlip::getFlip(scaleX), bFlipX, duration);
	this->setAnchorPoint(anchorPoint);
}

void BaseLayer::setFlipY(bool bFlipY, cocos2d::Vec2 anchorPoint, float duration)
{
	float scaleY = this->getScaleY();
	this->getTimerFlipY()->start(TimerFlip::getFlip(scaleY), bFlipY, duration);
	this->setAnchorPoint(anchorPoint);
}

void BaseLayer::setScale(cocos2d::Vec2 scale, float duration)
{
	this->getTimerScale()->setValue(scale, duration);
}

cocos2d::Vec2 BaseLayer::getScale()
{
	return this->getTimerScale()->getValue();
}

cocos2d::Vec2 BaseLayer::setAnchorPoint(cocos2d::Vec2 value, float duration)
{
	return this->getTimerAnchorPoint()->setValue(value, duration);
}

cocos2d::Vec2 BaseLayer::getAnchorPoint()
{
	return this->getTimerAnchorPoint()->getValue();
}

void BaseLayer::reset()
{
	this->setAnchorPoint(Vec2::ANCHOR_MIDDLE);
	this->setScale(Vec2::ONE);
	this->setRotation(0);

	this->getTimerRotation()->reset();
	this->getTimerScale()->reset();
	this->getTimerAnchorPoint()->reset();
	this->getTimerFlipX()->reset();
	this->getTimerFlipY()->reset();
}

void BaseLayer::addChild(cocos2d::Node* child, int localZOrder, int tag)
{
	auto scene = dynamic_cast<agtk::Scene *>(child);
	if (scene) {
		scene->setLocalZOrder(localZOrder);
		this->attachScene(scene);
	}

	auto sceneLayer = dynamic_cast<agtk::SceneLayer *>(child);
	if (sceneLayer) {
		if (sceneLayer->getType() != agtk::SceneLayer::kTypeMenu) {
			//main scenelayer
			agtk::RenderTextureCtrl *renderTextureCtrl = sceneLayer->getRenderTexture();
			if (renderTextureCtrl) {
				Node::addChild(renderTextureCtrl->getFirstRenderTexture(), localZOrder, tag);
				sceneLayer->createShader(this);
			}
		}
		else {
			if (sceneLayer->getLayerId() != agtk::data::SceneData::kHudTopMostLayerId) {
				sceneLayer->setCameraMask((unsigned short)cocos2d::CameraFlag::USER1);
			}
			else {
				sceneLayer->setCameraMask((unsigned short)cocos2d::CameraFlag::USER2);
			}
		}
		return;
	}
	auto sceneBackground = dynamic_cast<agtk::SceneBackground *>(child);
	if (sceneBackground) {
		sceneBackground->setBaseLayerZOrder(localZOrder);
		sceneBackground->createShader(this);
		auto renderTextureCtrl = sceneBackground->getRenderTexture();
		if (renderTextureCtrl) {
			Node::addChild(renderTextureCtrl->getFirstRenderTexture(), localZOrder, tag);
		}
		return;
	}
	auto sceneTopMost = dynamic_cast<agtk::SceneTopMost *>(child);
	if (sceneTopMost) {
		sceneTopMost->setBaseLayerZOrder(localZOrder);
		sceneTopMost->createShader(this);
		auto renderTextureCtrl = sceneTopMost->getRenderTexture();
		if (renderTextureCtrl) {
			Node::addChild(renderTextureCtrl->getFirstRenderTexture(), localZOrder, tag);
		}
		sceneTopMost->createWithMenuShader(this);
		auto withMenuRenderTextureCtrl = sceneTopMost->getWithMenuRenderTexture();
		if (withMenuRenderTextureCtrl) {
			Node::addChild(withMenuRenderTextureCtrl->getFirstRenderTexture(), localZOrder, tag);
		}
		return;
	}

	Node::addChild(child, localZOrder, tag);
}

void BaseLayer::addChild(cocos2d::Node* child, int localZOrder, const std::string &name)
{
	auto scene = dynamic_cast<agtk::Scene *>(child);
	if (scene) {
		this->attachScene(scene);
	}
	auto sceneLayer = dynamic_cast<agtk::SceneLayer *>(child);
	if (sceneLayer) {
		if (sceneLayer->getType() != agtk::SceneLayer::kTypeMenu) {
			//main scenelayer
			agtk::RenderTextureCtrl *renderTextureCtrl = sceneLayer->getRenderTexture();
			if (renderTextureCtrl) {
				Node::addChild(renderTextureCtrl->getFirstRenderTexture(), localZOrder, name);
				sceneLayer->createShader(this);
			}
		}
		else {
			if (sceneLayer->getLayerId() != agtk::data::SceneData::kHudTopMostLayerId) {
				sceneLayer->setCameraMask((unsigned short)cocos2d::CameraFlag::USER1);
			}
			else {
				sceneLayer->setCameraMask((unsigned short)cocos2d::CameraFlag::USER2);
			}
		}
		return;
	}
	auto sceneBackground = dynamic_cast<agtk::SceneBackground *>(child);
	if (sceneBackground) {
		sceneBackground->setBaseLayerZOrder(localZOrder);
		sceneBackground->createShader(this);
		auto renderTextureCtrl = sceneBackground->getRenderTexture();
		if (renderTextureCtrl) {
			Node::addChild(renderTextureCtrl->getFirstRenderTexture(), localZOrder, name);
		}
		return;
	}
	auto sceneTopMost = dynamic_cast<agtk::SceneTopMost *>(child);
	if (sceneTopMost) {
		sceneTopMost->setBaseLayerZOrder(localZOrder);

		sceneTopMost->createShader(this);
		auto renderTextureCtrl = sceneTopMost->getRenderTexture();
		if (renderTextureCtrl) {
			Node::addChild(renderTextureCtrl->getFirstRenderTexture(), localZOrder, name);
		}
		sceneTopMost->createWithMenuShader(this);
		auto withMenuRenderTextureCtrl = sceneTopMost->getWithMenuRenderTexture();
		if (withMenuRenderTextureCtrl) {
			Node::addChild(withMenuRenderTextureCtrl->getFirstRenderTexture(), localZOrder, name);
		}
		return;
	}
	Node::addChild(child, localZOrder, name);
}

void BaseLayer::removeChild(cocos2d::Node *child, bool cleanup)
{
	auto scene = dynamic_cast<agtk::Scene *>(child);
	if (scene) {
		this->detachScene(scene);
	}
	auto sceneLayer = dynamic_cast<agtk::SceneLayer *>(child);
	if (sceneLayer) {
		//main scenelayer
		agtk::RenderTextureCtrl *renderTextureCtrl = sceneLayer->getRenderTexture();
		if (renderTextureCtrl) {
			renderTextureCtrl->removeShaderAll();
			renderTextureCtrl->setLayer(nullptr);
		}
		if (sceneLayer->getGroupCollisionDetections()) {
			for (int group = -1; group < sceneLayer->getGroupCollisionDetections()->count(); group++) {
				CollisionDetaction *collisionDetection = sceneLayer->getGroupCollisionDetection(group);
				if (collisionDetection) {
					collisionDetection->reset();
				}
			}
		}
		if (sceneLayer->getGroupRoughWallCollisionDetections()){
			for (int group = 0; group < sceneLayer->getGroupRoughWallCollisionDetections()->count(); ++group) {
				CollisionDetaction *collisionDetection = sceneLayer->getGroupRoughWallCollisionDetection(group);
				if (collisionDetection) {
					collisionDetection->reset();
				}
			}
		}
	}
	auto sceneBackground = dynamic_cast<agtk::SceneBackground *>(child);
	if (sceneBackground) {
		auto renderTextureCtrl = sceneBackground->getRenderTexture();
		if (renderTextureCtrl) {
			renderTextureCtrl->removeShaderAll();
			renderTextureCtrl->setLayer(nullptr);
		}
	}
	auto sceneTopMost = dynamic_cast<agtk::SceneTopMost *>(child);
	if (sceneTopMost) {
		auto renderTextureCtrl = sceneTopMost->getRenderTexture();
		if (renderTextureCtrl) {
			renderTextureCtrl->removeShaderAll();
			renderTextureCtrl->setLayer(nullptr);
		}
		auto withMenuRenderTextureCtrl = sceneTopMost->getWithMenuRenderTexture();
		if (withMenuRenderTextureCtrl) {
			withMenuRenderTextureCtrl->removeShaderAll();
			withMenuRenderTextureCtrl->setLayer(nullptr);
		}
	}
	Node::removeChild(child, cleanup);
}

void BaseLayer::attachScene(agtk::Scene *scene)
{
	int zOrder = scene->getLocalZOrder();
	//camera
	auto camera = scene->getCamera();
	camera->setLayer(this);

	auto runningScene = Director::getInstance()->getRunningScene();
	if (runningScene) {
		auto menuCamera = camera->getMenuCamera();
		runningScene->addChild(menuCamera);

		auto topMostCamera = camera->getTopMostCamera();
		runningScene->addChild(topMostCamera);
	}

	//gravity
	auto gravity = scene->getGravity();
	gravity->set(100.0f, gravity->getRotation(), 0, true);
	auto sceneData = scene->getSceneData();
	GameManager::getInstance()->setGravity(sceneData->getGravityAccel(), gravity->getRotation());
	//scene background
	auto sceneBackground = scene->getSceneBackground();
	this->addChild(sceneBackground, zOrder);
	//scene layer
	auto sceneLayerList = scene->getSceneLayerList();
	int layerNum = 1;
	cocos2d::DictElement *el;
	CCDICT_FOREACH(sceneLayerList, el) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
		auto sceneLayer = static_cast<agtk::SceneLayer *>(el->getObject());
#else
		auto sceneLayer = dynamic_cast<agtk::SceneLayer *>(el->getObject());
#endif
		this->addChild(sceneLayer, zOrder + layerNum * BaseLayer::ADD_ZORDER);//レイヤー間のZオーダーを10に。
		layerNum++;
	}
	// menu layer
	{
		auto menuLayerList = scene->getMenuLayerList();
		cocos2d::DictElement *el;
		CCDICT_FOREACH(menuLayerList, el) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
			auto menuLayer = static_cast<agtk::SceneLayer * > (el->getObject());
#else
			auto menuLayer = dynamic_cast<agtk::SceneLayer * > (el->getObject());
#endif
			this->addChild(menuLayer, zOrder + layerNum * BaseLayer::ADD_ZORDER);
			layerNum++;
		}
	}
	//scene topMost
	auto sceneTopMost = scene->getSceneTopMost();
	this->addChild(sceneTopMost, zOrder + layerNum * BaseLayer::ADD_ZORDER);

	//viewport & light
	auto viewportLight = scene->getViewportLight();
	viewportLight->setBaseLayer(this);
	viewportLight->createViewportLight();
}

void BaseLayer::detachScene(agtk::Scene *scene)
{
	//camera
	auto camera = scene->getCamera();
	if (camera) {
		camera->setLayer(nullptr);
	}
	auto runningScene = Director::getInstance()->getRunningScene();
	auto menuCamera = camera->getMenuCamera();
	auto topMostCamera = camera->getTopMostCamera();
	if (runningScene) {
		runningScene->removeChild(menuCamera);
		runningScene->removeChild(topMostCamera);
	}

	//scene background
	auto sceneBackground = scene->getSceneBackground();
	if (sceneBackground) {
		this->removeChild(sceneBackground);
	}
	//scene TopMost
	auto sceneTopMost = scene->getSceneTopMost();
	if (sceneTopMost) {
		this->removeChild(sceneTopMost);
	}
	//viewport & light
	auto viewportLight = scene->getViewportLight();
	viewportLight->removeViewportLight();
	//menu layer
	{
		auto menuLayerList = scene->getMenuLayerList();
		cocos2d::DictElement *el;
		CCDICT_FOREACH(menuLayerList, el) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
			auto menuLayer = static_cast<agtk::SceneLayer *>(el->getObject());
#else
			auto menuLayer = dynamic_cast<agtk::SceneLayer *>(el->getObject());
#endif
			this->removeChild(menuLayer);
		}
	}
	//scene layer
	auto sceneLayerList = scene->getSceneLayerList();
	cocos2d::DictElement *el;
	CCDICT_FOREACH(sceneLayerList, el) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
		auto sceneLayer = static_cast<agtk::SceneLayer *>(el->getObject());
#else
		auto sceneLayer = dynamic_cast<agtk::SceneLayer *>(el->getObject());
#endif
		this->removeChild(sceneLayer);
	}
}

NS_AGTK_END
