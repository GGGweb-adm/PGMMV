#include "Scene.h"
#include "External/collision/CollisionComponent.hpp"
#include "External/collision/CollisionUtils.hpp"
#include "External/collision/CollisionDetaction.hpp"
#include "Manager/GameManager.h"
#include "Manager/AudioManager.h"
#include "Manager/PrimitiveManager.h"
#include "Manager/ParticleManager.h"
#include "Manager/EffectManager.h"
#include "Manager/BulletManager.h"
#include "Manager/ImageManager.h"
#include "Manager/MovieManager.h"
#include "Data/ProjectData.h"
#include "Portal.h"
#include "PhysicsObject.h"
#include "Slope.h"
#include "Lib/ViewportLight.h"
#include "Manager/GuiManager.h"
#include "Manager/DebugManager.h"
#include "Manager/ThreadManager.h"
#include "Manager/JavascriptManager.h"

#define USE_TILE_COLLISION_FIX		// タイルとの衝突判定修正版
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

//-------------------------------------------------------------------------------------------------------------------
SceneVariableTimer::SceneVariableTimer()
{
	_variableData = nullptr;
	_object = nullptr;
	_duration = -1.0f;
}

SceneVariableTimer::~SceneVariableTimer()
{
	CC_SAFE_RELEASE_NULL(_variableData);
	CC_SAFE_RELEASE_NULL(_object);
}

bool SceneVariableTimer::init(agtk::data::PlayVariableData *data, EnumCountType type, agtk::Object *object)
{
	CC_ASSERT(data);
	this->setCountType(type);
	this->setVariableData(data);
	this->setObject(object);
	return true;
}

void SceneVariableTimer::start(double seconds)
{
	this->getVariableData()->setValue(seconds);
	_duration = 0.0f;
}

void SceneVariableTimer::update(float dt)
{
	if (_duration < 0.0f) {
		return;
	}
	auto scene = GameManager::getInstance()->getCurrentScene();
	if (_object) {
		if (_object->getDisabled() || _object->getWaitDuration300() == -1) {
			return;
		}
		auto timeScale = scene->getGameSpeed()->getTimeScale(_object);
		auto sceneLayer = _object->getSceneLayer();
		if (!sceneLayer->getActiveFlg()) {
			return;
		}
		auto sceneLayerType = sceneLayer->getType();
		if (sceneLayerType == agtk::SceneLayer::kTypeMenu) {
			timeScale = scene->getGameSpeed()->getTimeScale(SceneGameSpeed::eTYPE_MENU);
		}
		dt *= timeScale;
	}
	double value = this->getVariableData()->getValue();
	_duration += dt;
	if (this->getCountType() == kCountUp) {
		value += dt;
	}
	else if (this->getCountType() == kCountDown) {
		value -= dt;
		if (value < 0.0f) value = 0.0f;
	}
	this->getVariableData()->setValue(value);
}

//-------------------------------------------------------------------------------------------------------------------
SceneSprite::SceneSprite()
{
	_opacityTimer = nullptr;
	_imageId = -1;
}

SceneSprite::~SceneSprite()
{
	CC_SAFE_RELEASE_NULL(_opacityTimer);
}

SceneSprite *SceneSprite::create(int imageId, int opacity, float seconds)
{
	auto p = new (std::nothrow) SceneSprite();
	if (p && p->init(imageId, opacity, seconds)) {
		p->autorelease();
		return p;
	}
	CC_SAFE_DELETE(p);
	return nullptr;
}

bool SceneSprite::init(int imageId, int opacity, float seconds)
{
	auto projectData = GameManager::getInstance()->getProjectData();
	auto image = projectData->getImageData(imageId);
	if (cocos2d::Sprite::initWithFile(image->getFilename()) == false) {
		return false;
	}
	auto opacityTimer = OpacityTimer::create(0);
	if (opacityTimer == nullptr) {
		return false;
	}
	this->getTexture()->setAliasTexParameters();
	this->setImageId(imageId);
	this->setOpacity(0);
	opacityTimer->setValue(opacity, seconds);
	this->setOpacityTimer(opacityTimer);
	return true;
}

void SceneSprite::update(float delta)
{
	auto opacityTimer = this->getOpacityTimer();
	if (opacityTimer) {
		opacityTimer->update(delta);
		this->setOpacity(opacityTimer->getValue());
	}
}

//-------------------------------------------------------------------------------------------------------------------
SceneBackgroundSprite *SceneBackgroundSprite::createWithTexture(cocos2d::Texture2D *texture)
{
	auto p = new (std::nothrow) SceneBackgroundSprite();
	if (p && p->initWithTexture(texture)) {
		p->autorelease();
		return p;
	}
	CC_SAFE_DELETE(p);
	return nullptr;
}

void SceneBackgroundSprite::setPosition(const Vec2 &position)
{
	this->setPosition(position.x, position.y);
}

void SceneBackgroundSprite::setPosition(float x, float y)
{
	_realPosition.x = x;
	_realPosition.y = y;

	//浮動小数点切り捨て
	x = (int)x;
	y = (int)y;

	Node::setPosition(x, y);
}

const cocos2d::Vec2& SceneBackgroundSprite::getPosition() const
{
	return _realPosition;
}

//-------------------------------------------------------------------------------------------------------------------
SceneBackground::SceneBackground()
{
	_scene = nullptr;
	_sceneData = nullptr;
	_spriteBackgroundList = nullptr;
	_sceneSpriteList = nullptr;
	_shader = nullptr;
	_renderTexture = nullptr;
	_textureBuffer = nullptr;
	_removeShaderList = nullptr;
	_gifAnimation = nullptr;
	_baseLayerZOrder = 0;
	_lastMovePosition = cocos2d::Vec2::ZERO;
}

SceneBackground::~SceneBackground()
{
	auto node = this->getChildByName("bgcolor");
	if (node) {
		auto primitiveManager = PrimitiveManager::getInstance();
		primitiveManager->remove(node);
	}
	CC_SAFE_RELEASE_NULL(_sceneData);
	CC_SAFE_RELEASE_NULL(_spriteBackgroundList);
	CC_SAFE_RELEASE_NULL(_sceneSpriteList);
	CC_SAFE_RELEASE_NULL(_shader);
	CC_SAFE_RELEASE_NULL(_renderTexture);
	CC_SAFE_DELETE(_textureBuffer);
	CC_SAFE_RELEASE_NULL(_removeShaderList);
	CC_SAFE_RELEASE_NULL(_gifAnimation);
}

bool SceneBackground::init(agtk::Scene *scene)
{
	if (!cocos2d::Node::init()) {
		return false;
	}
	if (scene == nullptr) {
		CC_ASSERT(0);
		return false;
	}
	_scene = scene;
	auto sceneData = _scene->getSceneData();
	this->setSceneData(sceneData);
	auto projectData = GameManager::getInstance()->getProjectData();
	this->setSceneSize(cocos2d::Size(
		projectData->getScreenWidth() * sceneData->getHorzScreenCount(),
		projectData->getScreenHeight() * sceneData->getVertScreenCount()
	));
	this->setSceneBackgroundSize(this->getSceneSize());
	cocos2d::Color3B color = sceneData->getBgColor();
	float width = this->getSceneSize().width;
	float height = this->getSceneSize().height;
	auto prim = PrimitiveManager::getInstance()->createPlate(0, 0, width, height, cocos2d::Color4F(color));
	prim->setAnchorPoint(cocos2d::Vec2(0, 0));//左下を軸にする。
	this->addChild(prim, 0, "bgcolor");

	auto spriteBackgroundList = cocos2d::__Array::create();
	if (spriteBackgroundList == nullptr) {
		return false;
	}
	this->setSpriteBackgroundList(spriteBackgroundList);

	if (sceneData->getSetBgImageFlag()) {
		float texWidth = 0.0f;
		float texHeight = 0.0f;
		auto image = projectData->getImageData(sceneData->getBgImageId());
		if (image != nullptr) {
			cocos2d::Texture2D *texture2d = nullptr;
			if (strstr(image->getFilename(), ".gif") != nullptr) {
				auto gifAnimation = agtk::GifAnimation::create(image->getFilename());
				if (gifAnimation == nullptr) {
					return false;
				}
				this->setGifAnimation(gifAnimation);
				gifAnimation->play();

				texture2d = new Texture2D();
				int width = gifAnimation->getWidth();
				int height = gifAnimation->getHeight();
				auto bm = gifAnimation->getBitmap();
				texWidth = bm->getWidth();
				texHeight = bm->getHeight();
				texture2d->initWithData(
					bm->getAddr(),
					bm->getLength(),
					Texture2D::PixelFormat::RGBA8888,
					bm->getWidth(),
					bm->getHeight(),
					cocos2d::Size(bm->getWidth(), bm->getHeight())
				);
			}
			else {
				texture2d = CreateTexture2D(image->getFilename(), (sceneData->getBgImagePlacement() == agtk::data::SceneData::kTiling), &_textureBuffer, &texWidth, &texHeight);
				texture2d->setAliasTexParameters();
			}

			std::function<float(float, float)> calcVariable = [&](float p, float n) {
				CC_ASSERT(p > 0 && n > 0);
				float v = n;
				while (1) {
					if (v >= p) {
						break;
					}
					v += n;
				}
				return v;
			};
			this->setSceneBackgroundSize(cocos2d::Size(
				calcVariable(this->getSceneSize().width, texWidth),
				calcVariable(this->getSceneSize().height, texHeight)
			));

			auto sprite0 = this->createSpriteBackground(texture2d);
			this->addChild(sprite0);
			spriteBackgroundList->addObject(sprite0);

			// 条件（bgImageMoveSpeed != 0　スクロール速度が０以外）の場合、スプライトを４つ生成する。
			//  +-------+-------+
			//  |   3   |   2   |
			//  +-------+-------+
			//  |   0   |   1   |
			//  +-------+-------+

			//タイル設定時にシェーダーによりタイリングする。
			bool bTiling = (sceneData->getBgImagePlacement() == agtk::data::SceneData::kTiling);
			agtk::Shader *shader = nullptr;
			if (bTiling) {
				shader = agtk::Shader::createShaderKind(sprite0, this->getSceneBackgroundSize(), agtk::Shader::kShaderTextureRepeat, 1);
				shader->setIntensity(1.0);
				shader->setIgnored(false);
				shader->setShaderTextureRepeat(texture2d->getContentSize(), cocos2d::Size(texWidth, texHeight));
				this->setShader(shader);

				float width = this->getSceneBackgroundSize().width;
				float height = this->getSceneBackgroundSize().height;
				float sceneHeight = this->getSceneSize().height;
				sprite0->setPosition(0, sceneHeight - height);
			}
			auto pos0 = sprite0->getPosition();
			_sprite0Position = pos0;

			if (width < this->getSceneBackgroundSize().width || height < this->getSceneBackgroundSize().height) {
				width = this->getSceneBackgroundSize().width;
				height = this->getSceneBackgroundSize().height;
			}

			if (sceneData->getBgImageMoveSpeed() != 0.0 || sceneData->getBgMoveSpeedX() != 100.0 || sceneData->getBgMoveSpeedY() != 100.0) {
				//sprite 1
				auto sprite1 = this->createSpriteBackground(texture2d);
				if (bTiling) {
					sprite1->setGLProgramState(shader->getProgramState());
				}
				sprite1->setPosition(pos0 + cocos2d::Vec2(width, 0));
				this->addChild(sprite1);
				spriteBackgroundList->addObject(sprite1);
				//sprite 2
				auto sprite2 = this->createSpriteBackground(texture2d);
				if (bTiling) {
					sprite2->setGLProgramState(shader->getProgramState());
				}
				sprite2->setPosition(pos0 + cocos2d::Vec2(width, height));
				this->addChild(sprite2);
				spriteBackgroundList->addObject(sprite2);
				//sprite 3
				auto sprite3 = this->createSpriteBackground(texture2d);
				if (bTiling) {
					sprite3->setGLProgramState(shader->getProgramState());
				}
				sprite3->setPosition(pos0 + cocos2d::Vec2(0, height));
				this->addChild(sprite3);
				spriteBackgroundList->addObject(sprite3);
			}

			// texture2d はcocos の MemoryPool に登録されていないのでここで参照カウンタを1つ下げる必要がある
			CC_SAFE_RELEASE(texture2d);
		}
	}

	//sprite
	auto sceneSpriteList = cocos2d::__Array::create();
	if (sceneSpriteList == nullptr) {
		CC_ASSERT(0);
		return false;
	}
	this->setSceneSpriteList(sceneSpriteList);
	this->setRemoveShaderList(cocos2d::__Array::create());

	return true;
}

cocos2d::Sprite *SceneBackground::createSpriteBackground(cocos2d::Texture2D *texture2d)
{
	auto sceneData = this->getSceneData();
	auto projectData = GameManager::getInstance()->getProjectData();
	float width = projectData->getScreenWidth() * sceneData->getHorzScreenCount();
	float height = projectData->getScreenHeight() * sceneData->getVertScreenCount();
	auto sprite = agtk::SceneBackgroundSprite::createWithTexture(texture2d);
	sprite->setAnchorPoint(cocos2d::Vec2(0, 0));
	switch (sceneData->getBgImagePlacement()) {
	case agtk::data::SceneData::kCenter: {//中央
		float sceneWidth = this->getSceneSize().width;
		float sceneHeight = this->getSceneSize().height;
		float width = sprite->getContentSize().width;
		float height = sprite->getContentSize().height;
		sprite->setPosition((sceneWidth - width) * 0.5f, (sceneHeight - height) * 0.5f);
		break; }
	case agtk::data::SceneData::kMagnify: {//拡大
		float hScale = (projectData->getScreenWidth() * sceneData->getHorzScreenCount()) / sprite->getContentSize().width;
		float vScale = (projectData->getScreenHeight() * sceneData->getVertScreenCount()) / sprite->getContentSize().height;
		sprite->setScale(hScale, vScale);
		this->setSceneBackgroundSize(cocos2d::Size(width, height));
		break; }
	case agtk::data::SceneData::kTiling: {//タイル
		//※画像は２のべき乗サイズの必要があります。
		//const Texture2D::TexParams tp = { GL_LINEAR, GL_LINEAR, GL_REPEAT, GL_REPEAT };
		width = this->getSceneBackgroundSize().width;
		height = this->getSceneBackgroundSize().height;
		//sprite->getTexture()->setTexParameters(tp);
		sprite->setTextureRect(cocos2d::Rect(0, 0, width, height));
		break; }
	case agtk::data::SceneData::kMagnifyKeepRatio: {//比率を維持して拡大
		float hScale = (projectData->getScreenWidth() * sceneData->getHorzScreenCount()) / sprite->getContentSize().width;
		float vScale = (projectData->getScreenHeight() * sceneData->getVertScreenCount()) / sprite->getContentSize().height;
		if (hScale < vScale) {
			sprite->setScale(hScale);
			this->setSceneBackgroundSize(cocos2d::Size(sprite->getContentSize().width * hScale, sprite->getContentSize().height * hScale));
			auto pos = sprite->getPosition();
			sprite->setPosition(cocos2d::Vec2(pos.x, pos.y - (sprite->getContentSize().height * hScale - height) * 0.5f));
		}
		else {
			sprite->setScale(vScale);
			this->setSceneBackgroundSize(cocos2d::Size(sprite->getContentSize().width * vScale, sprite->getContentSize().height * vScale));
			auto pos = sprite->getPosition();
			sprite->setPosition(cocos2d::Vec2(pos.x - (sprite->getContentSize().width * vScale - width) * 0.5f, pos.y));
		}
		break; }
	default:CC_ASSERT(0);
	}
	return sprite;
}

void SceneBackground::update(float dt)
{
	PROFILING("SceneBackground::update", profiling);
	auto sceneData = this->getSceneData();

	//カメラのシーン表示制限が有効な場合。
	auto camera = _scene->getCamera();
	if (camera->isDisableLimitCamera() == false) {
		auto camPos = camera->getPosition();
		auto screenSize = camera->getScreenSize();
		_sprite0Position = cocos2d::Vec2(camPos.x - screenSize.width * 0.5f, camPos.y - screenSize.height * 0.5f);
	}

	auto removeShaderList = this->getRemoveShaderList();
	if (removeShaderList->count()) {
		auto renderTextureCtrl = this->getRenderTexture();
		if (renderTextureCtrl) {
			cocos2d::Ref *ref;
			CCARRAY_FOREACH(removeShaderList, ref) {
				auto shader = dynamic_cast<agtk::Shader *>(ref);
				renderTextureCtrl->removeShader(shader);
			}
			removeShaderList->removeAllObjects();

			if (isRemoveRenderTexture()) {
				removeRenderTexture();
			}
		}
	}

	auto gifAnimation = this->getGifAnimation();
	if (gifAnimation) {
		bool ret = gifAnimation->update(dt);
		if (ret) {
			auto bitmap = gifAnimation->getBitmap();
			cocos2d::Size contentSize(bitmap->getWidth(), bitmap->getHeight());
			auto texture = new Texture2D();
			texture->initWithData(
				bitmap->getAddr(),
				bitmap->getLength(),
				Texture2D::PixelFormat::RGBA8888,
				bitmap->getWidth(),
				bitmap->getHeight(),
				contentSize
			);
			cocos2d::Ref *ref;
			CCARRAY_FOREACH(this->getSpriteBackgroundList(), ref) {
				auto sprite = dynamic_cast<cocos2d::Sprite *>(ref);
				if (sprite != nullptr) {
					sprite->setTexture(texture);
				}
			}
		}
	}

	//sprite bg
	if (sceneData->getSetBgImageFlag() && (sceneData->getBgImageMoveSpeed() != 0.0f)) {
		cocos2d::Ref *ref = nullptr;
		auto spriteBackgroundList = this->getSpriteBackgroundList();

		// 背景スプライトリストがある場合
		// ※背景画像が「設定無し」の場合は背景スプライトリストは空である事に注意
		if (spriteBackgroundList->count() > 0) {
			CCARRAY_FOREACH(spriteBackgroundList, ref) {
				auto sprite = dynamic_cast<cocos2d::Sprite *>(ref);
				if (sprite != nullptr) {
					auto v = agtk::GetDirectionFromDegrees(sceneData->getBgImageMoveDirection());
					v *= sceneData->getBgImageMoveSpeed();
					cocos2d::Vec2 pos = sprite->getPosition();
					pos += v;
					sprite->setPosition(pos);
				}
			}
			adjustBackgroundSpritePosition();
		}
	}

	auto spriteBackgroundList = this->getSpriteBackgroundList();
	cocos2d::Ref *ref = nullptr;
	CCARRAY_FOREACH(spriteBackgroundList, ref) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
		auto sprite = static_cast<agtk::SceneBackgroundSprite *>(ref);
#else
		auto sprite = dynamic_cast<agtk::SceneBackgroundSprite *>(ref);
#endif
		sprite->update(dt);
	}

	//sprite
	//update
	auto sceneSpriteList = this->getSceneSpriteList();
	CCARRAY_FOREACH(sceneSpriteList, ref){
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
		auto sceneSprite = static_cast<agtk::SceneSprite *>(ref);
#else
		auto sceneSprite = dynamic_cast<agtk::SceneSprite *>(ref);
#endif
		sceneSprite->update(dt);
	}
	//remove
	while (1) {
		bool bRemove = false;
		CCARRAY_FOREACH(sceneSpriteList, ref) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
			auto sceneSprite = static_cast<agtk::SceneSprite *>(ref);
#else
			auto sceneSprite = dynamic_cast<agtk::SceneSprite *>(ref);
#endif
			auto opacity = sceneSprite->getOpacityTimer();
			if (opacity->getState() == agtk::EventTimer::kStateEnd && opacity->getValue() == 0.0f) {
				//remove
				this->removeChild(sceneSprite);
				sceneSpriteList->removeObject(sceneSprite);
				bRemove = true;
				break;
			}
		}
		if (bRemove == false) break;
	}
}

void SceneBackground::setMovePosition(const cocos2d::Vec2 &position)
{
	cocos2d::Ref *ref = nullptr;
	auto spriteBackgroundList = this->getSpriteBackgroundList();

	// 背景スプライトリストがある場合
	// ※背景画像が「設定無し」の場合は背景スプライトリストは空である事に注意
	if (spriteBackgroundList->count() > 0) {
		auto move = position - this->getLastMovePosition();
		this->setLastMovePosition(position);
		CCARRAY_FOREACH(spriteBackgroundList, ref) {
			auto sprite = dynamic_cast<cocos2d::Sprite *>(ref);
			if (sprite != nullptr) {
				cocos2d::Vec2 pos = sprite->getPosition() + move;
				sprite->setPosition(pos);
			}
		}
	}
	adjustBackgroundSpritePosition();
}

void SceneBackground::adjustBackgroundSpritePosition()
{
	cocos2d::Ref *ref = nullptr;
	auto spriteBackgroundList = this->getSpriteBackgroundList();

	// 背景スプライトリストがある場合
	// ※背景画像が「設定無し」の場合は背景スプライトリストは空である事に注意
	if (spriteBackgroundList->count() > 0) {
		//ループ処理
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
		auto sprite0 = static_cast<cocos2d::Sprite *>(spriteBackgroundList->getObjectAtIndex(0));
#else
		auto sprite0 = dynamic_cast<cocos2d::Sprite *>(spriteBackgroundList->getObjectAtIndex(0));
#endif
		CC_ASSERT(sprite0);

		auto sceneSize = this->getSceneSize();
		if (sceneSize.width < this->getSceneBackgroundSize().width || sceneSize.height < this->getSceneBackgroundSize().height) {
			sceneSize = this->getSceneBackgroundSize();
		}
		auto pos0 = sprite0->getPosition();
		pos0 -= _sprite0Position;

		float dx = 0.0f;
		float dy = 0.0f;
		bool bLoop = false;
		if (pos0.x <= -sceneSize.width) {
			dx = ((int)-pos0.x / (int)sceneSize.width) * sceneSize.width;
			bLoop = true;
		}
		else if (pos0.x > 0) {
			dx = -(((int)pos0.x + (int)sceneSize.width - 1) / (int)sceneSize.width) * sceneSize.width;
			bLoop = true;
		}
		if (pos0.y <= -sceneSize.height) {
			dy = ((int)-pos0.y / (int)sceneSize.height) * sceneSize.height;
			bLoop = true;
		}
		else if (pos0.y > 0) {
			dy = -(((int)pos0.y + (int)sceneSize.height - 1) / (int)sceneSize.height) * sceneSize.height;
			bLoop = true;
		}
		if (bLoop) {
			cocos2d::Ref *ref = nullptr;
			CCARRAY_FOREACH(this->getSpriteBackgroundList(), ref) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
				auto sprite = static_cast<cocos2d::Sprite *>(ref);
#else
				auto sprite = dynamic_cast<cocos2d::Sprite *>(ref);
#endif
				auto p = sprite->getPosition();
				sprite->setPosition(p + cocos2d::Vec2(dx, dy));
			}
		}
	}
}

void SceneBackground::updateRenderer(float delta, cocos2d::Mat4 *viewMatrix)
{
	auto renderTexture = this->getRenderTexture();
	if (renderTexture) {
		renderTexture->update(delta, viewMatrix);
	}
	else {
		setVisible(true);
	}
}

void SceneBackground::createShader(cocos2d::Layer *layer)
{
	auto projectData = GameManager::getInstance()->getProjectData();

	auto renderTextureCtrl = this->getRenderTexture();
// #AGTK-NX
#if CC_TARGET_PLATFORM != CC_PLATFORM_NX
	if (renderTextureCtrl == false) {
#else
#endif
		bool isCreate = false;

		{
			//シーン効果
			auto sceneData = this->getSceneData();
			auto sceneFilterEffectList = sceneData->getSceneFilterEffectList();
			cocos2d::DictElement *el;
			CCDICT_FOREACH(sceneFilterEffectList, el) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
				auto filterEffectData = static_cast<agtk::data::SceneFilterEffectData *>(el->getObject());
#else
				auto filterEffectData = dynamic_cast<agtk::data::SceneFilterEffectData *>(el->getObject());
#endif
				if (filterEffectData->getDisabled()) {
					continue;
				}

				if (filterEffectData->getLayerIndex() == agtk::data::SceneFilterEffectData::kLayerIndexAllSceneLayers ||
					filterEffectData->getLayerIndex() == agtk::data::SceneFilterEffectData::kLayerIndexBackground) {//背景 or 全てのシーンレイヤー
					isCreate = true;
					break;
				}
			}
		}

		if (isCreate == false) {
			return;
		}
		createRenderTexture();
		renderTextureCtrl = this->getRenderTexture();
	}
	renderTextureCtrl->setLayer(layer);

	std::function<void(agtk::data::SceneFilterEffectData *)> createFilterEffect = [&](agtk::data::SceneFilterEffectData *data) {
		if (data->getDisabled()) {
			return;
		}
		auto filterEffect = data->getFilterEffect();
		float seconds = (float)filterEffect->getDuration300() / 300.0f;
		switch (filterEffect->getEffectType()) {
		case agtk::data::FilterEffect::kEffectNoise: {//ノイズ
			this->setShader(agtk::Shader::kShaderNoisy, (float)filterEffect->getNoise() / 100.0f, seconds);
			break; }
		case agtk::data::FilterEffect::kEffectMosaic: {//モザイク
			this->setShader(agtk::Shader::kShaderMosaic, (float)filterEffect->getMosaic() / 100.0f, seconds);
			break; }
		case agtk::data::FilterEffect::kEffectMonochrome: {//モノクロ
			this->setShader(agtk::Shader::kShaderColorGray, (float)filterEffect->getMonochrome() / 100.0f, seconds);
			break; }
		case agtk::data::FilterEffect::kEffectSepia: {//セピア
			this->setShader(agtk::Shader::kShaderColorSepia, (float)filterEffect->getSepia() / 100.0f, seconds);
			break; }
		case agtk::data::FilterEffect::kEffectNegaPosiReverse: {//ネガ反転
			this->setShader(agtk::Shader::kShaderColorNega, (float)filterEffect->getNegaPosiReverse() / 100.0f, seconds);
			break; }
		case agtk::data::FilterEffect::kEffectDefocus: {//ぼかし
			this->setShader(agtk::Shader::kShaderBlur, (float)filterEffect->getDefocus() / 100.0f, seconds);
			break; }
		case agtk::data::FilterEffect::kEffectChromaticAberration: {//色収差
			this->setShader(agtk::Shader::kShaderColorChromaticAberration, (float)filterEffect->getChromaticAberration() / 100.0f, seconds);
			break; }
		case agtk::data::FilterEffect::kEffectDarkness: {//暗闇
			this->setShader(agtk::Shader::kShaderColorDark, (float)filterEffect->getDarkness() / 100.0f, seconds);
			break; }
		case agtk::data::FilterEffect::kEffectTransparency: {//透明
			this->setShader(agtk::Shader::kShaderTransparency, (float)filterEffect->getTransparency() / 100.0f, seconds);
			break; }
		case agtk::data::FilterEffect::kEffectBlink: {//点滅
			// 点滅はオブジェクトのみに実装。
			break; }
		case agtk::data::FilterEffect::kEffectDispImage: {//画像表示
			if (filterEffect->getImageId() >= 0) {
				auto projectData = GameManager::getInstance()->getProjectData();
				auto imageData = projectData->getImageData(filterEffect->getImageId());
				cocos2d::Size texSizeDef;

				// create texture2d
				auto texture2d = CreateTexture2D(imageData->getFilename(), (filterEffect->getImagePlacement() == agtk::data::FilterEffect::kPlacementTiling), nullptr, &texSizeDef.width, &texSizeDef.height);
				if (filterEffect->getImagePlacement() == agtk::data::FilterEffect::kPlacementTiling) {
					//タイリング
					Texture2D::TexParams tRepeatParams;
					tRepeatParams.magFilter = GL_LINEAR;
					tRepeatParams.minFilter = GL_LINEAR;
					tRepeatParams.wrapS = GL_REPEAT;
					tRepeatParams.wrapT = GL_REPEAT;
					texture2d->setTexParameters(tRepeatParams);
				}
				else {
					texture2d->setAliasTexParameters();
				}

				auto sceneSize = projectData->getSceneSize(this->getSceneData());
				auto shader = renderTextureCtrl->addShader(agtk::Shader::kShaderImage, sceneSize, (float)(100.0f - filterEffect->getImageTransparency()) / 100.0f, seconds);
				shader->setMaskTexture(texture2d);
				// set imgPlacement
				int imgPlacement = filterEffect->getImagePlacement();
				auto programState = shader->getProgramState();
				programState->setUniformInt("imgPlacement", imgPlacement);
				// set resolution
				float width = sceneSize.width;
				float height = sceneSize.height;
				programState->setUniformVec2("resolution", cocos2d::Vec2(width, height));
				// set imgResolution
				auto imgResolution = (imgPlacement == 2) ? Vec2(texture2d->getContentSize().width, texture2d->getContentSize().height) : Vec2(width, height);
				programState->setUniformVec2("imgResolution", imgResolution);
				// set imgSizeRate
				auto imgSizeRate = cocos2d::Vec2(texSizeDef.width / texture2d->getContentSize().width, texSizeDef.height / texture2d->getContentSize().height);
				programState->setUniformVec2("imgSizeRate", imgSizeRate);
				// set sxy
				float imgSourceWidth = texSizeDef.width;
				float imgSourceHeight = texSizeDef.height;
				auto sxy = (imgPlacement == 3) ? ((width / imgSourceWidth <= height / imgSourceHeight) ? Vec2(1, height / imgSourceHeight * imgSourceWidth / width) : Vec2(width / imgSourceWidth * imgSourceHeight / height, 1)) : (imgSourceWidth > 0 && imgSourceHeight > 0 ? Vec2(width / imgSourceWidth, height / imgSourceHeight) : Vec2(1, 1));
				programState->setUniformVec2("sxy", sxy);
				// set imgXy
				auto imgXy = Vec2((1 - sxy.x) / 2, (1 - sxy.y) / 2);
				programState->setUniformVec2("imgXy", imgXy);
				// set FilterEffect
				shader->setUserData(filterEffect);
			}
			break; }
		case agtk::data::FilterEffect::kEffectFillColor: {//指定色で塗る
			this->setShader(agtk::Shader::kShaderColorRgba, 1.0f, seconds);
			auto shader = this->getShader(agtk::Shader::kShaderColorRgba);
			shader->setShaderRgbaColor(cocos2d::Color4B(filterEffect->getFillR(), filterEffect->getFillG(), filterEffect->getFillB(), filterEffect->getFillA()));
			break; }
		default:CC_ASSERT(0);
		}
	};

	//シーン効果
	auto sceneData = this->getSceneData();
	auto sceneFilterEffectList = sceneData->getSceneFilterEffectList();
	cocos2d::DictElement *el;
	CCDICT_FOREACH(sceneFilterEffectList, el) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
		auto filterEffectData = static_cast<agtk::data::SceneFilterEffectData *>(el->getObject());
#else
		auto filterEffectData = dynamic_cast<agtk::data::SceneFilterEffectData *>(el->getObject());
#endif
		if (filterEffectData->getLayerIndex() == agtk::data::SceneFilterEffectData::kLayerIndexAllSceneLayers ||
			filterEffectData->getLayerIndex() == agtk::data::SceneFilterEffectData::kLayerIndexBackground) {//背景 or 全てのシーンレイヤー
			createFilterEffect(filterEffectData);
		}
	}
}

void SceneBackground::setShader(Shader::ShaderKind kind, float value, float seconds)
{
	// レンダーテクスチャが作成されていない場合は作成する
	auto renderTexture = this->getRenderTexture();
// #AGTK-NX	
#if CC_TARGET_PLATFORM != CC_PLATFORM_NX
	if (renderTexture == false) {
#else
#endif
		createObjectCommandRenderTexture();
		renderTexture = this->getRenderTexture();
	}

	auto shader = this->getShader(kind);
	if (shader == nullptr) {
		renderTexture->addShader(kind, value, seconds);
	}
	else {
		shader->setIntensity(value, seconds);
		shader->setIgnored(false);//有効に。
	}
}

agtk::Shader *SceneBackground::getShader(Shader::ShaderKind kind)
{
	auto renderTexture = this->getRenderTexture();
// #AGTK-NX
#if CC_TARGET_PLATFORM != CC_PLATFORM_NX
	if (renderTexture == false) {
#else
#endif
		return nullptr;
	}

	auto projectData = GameManager::getInstance()->getProjectData();
	auto shaderList = renderTexture->getShaderList();
	for (int idx = 0; idx < shaderList->count(); idx++) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
		auto shader = static_cast<agtk::Shader *>(shaderList->getObjectAtIndex(idx));
#else
		auto shader = dynamic_cast<agtk::Shader *>(shaderList->getObjectAtIndex(idx));
#endif
		if (shader->getKind() == kind) {
			return shader;
		}
	}
	return nullptr;
}

void SceneBackground::removeShader(Shader::ShaderKind kind, float seconds)
{
	auto renderTexture = this->getRenderTexture();
// #AGTK-NX
#if CC_TARGET_PLATFORM != CC_PLATFORM_NX
	if (renderTexture == false) {
#else
#endif
		return;
	}

	auto shader = this->getShader(kind);
	if (shader == nullptr) {
		return;
	}
	shader->setIntensity(0.0f, seconds);
	auto value = shader->getValue();
	value->setEndFunc([&, shader]() {
		this->getRemoveShaderList()->addObject(shader);
	});
}

void SceneBackground::createRenderTexture()
{
	auto projectData = GameManager::getInstance()->getProjectData();
	auto renderTextureCtrl = agtk::RenderTextureCtrl::createForSceneBackground(this, projectData->getScreenSize());
	this->addChild(renderTextureCtrl);
	this->setRenderTexture(renderTextureCtrl);
}

void SceneBackground::createObjectCommandRenderTexture()
{
	auto projectData = GameManager::getInstance()->getProjectData();

	auto baseLayer = _scene->getBaseLayer();

	auto renderTextureCtrl = agtk::RenderTextureCtrl::createForSceneBackground(this, projectData->getScreenSize());
	renderTextureCtrl->setLayer(baseLayer);
	this->addChild(renderTextureCtrl);
	this->setRenderTexture(renderTextureCtrl);
	
	baseLayer->Node::addChild(renderTextureCtrl->getFirstRenderTexture(), _baseLayerZOrder, this->getName());
}

bool SceneBackground::isRemoveRenderTexture()
{
	auto renderTextureCtrl = this->getRenderTexture();
	if (renderTextureCtrl) {
		if (renderTextureCtrl->getShaderList()->count() == 0) {
			return true;
		}
	}

	return false;
}

void SceneBackground::removeRenderTexture()
{
	setVisible(true);

	auto renderTextureCtrl = this->getRenderTexture();
	if (renderTextureCtrl) {
		renderTextureCtrl->removeFromParentAndCleanup(true);
		this->setRenderTexture(nullptr);
	}
	this->getRemoveShaderList()->removeAllObjects();
}

void SceneBackground::createSceneSprite(int imageId, int opacity, cocos2d::Vec2 pos, float seconds)
{
	auto sceneSprite = agtk::SceneSprite::create(imageId, opacity, seconds);
	sceneSprite->setPosition(pos);
	this->getSceneSpriteList()->addObject(sceneSprite);
	this->addChild(sceneSprite);
}

void SceneBackground::removeSceneSprite(float seconds)
{
	auto sceneSpriteList = this->getSceneSpriteList();
	cocos2d::Ref *ref = nullptr;
	CCARRAY_FOREACH(sceneSpriteList, ref) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
		auto sceneSprite = static_cast<agtk::SceneSprite *>(ref);
#else
		auto sceneSprite = dynamic_cast<agtk::SceneSprite *>(ref);
#endif
		sceneSprite->getOpacityTimer()->setValue(0, seconds);
	}
}

//-------------------------------------------------------------------------------------------------------------------
SceneTopMostSprite *SceneTopMostSprite::createWithTexture(cocos2d::Texture2D *texture)
{
	auto p = new (std::nothrow) SceneTopMostSprite();
	if (p && p->initWithTexture(texture)) {
		p->autorelease();
		return p;
	}
	CC_SAFE_DELETE(p);
	return nullptr;
}

void SceneTopMostSprite::setPosition(const Vec2 &position)
{
	this->setPosition(position.x, position.y);
}

void SceneTopMostSprite::setPosition(float x, float y)
{
	_realPosition.x = x;
	_realPosition.y = y;

	//浮動小数点切り捨て
	x = (int)x;
	y = (int)y;

	Node::setPosition(x, y);
}

const cocos2d::Vec2& SceneTopMostSprite::getPosition() const
{
	return _realPosition;
}

//-------------------------------------------------------------------------------------------------------------------
SceneTopMost::SceneTopMost()
{
	_scene = nullptr;
	_sceneData = nullptr;
	_spriteTopMostList = nullptr;
	_sceneSpriteList = nullptr;
	_renderTexture = nullptr;
	_renderTextureFront = nullptr;
	_withMenuRenderTexture = nullptr;
	_removeShaderList = nullptr;
	_removeWithMenuShaderList = nullptr;
	_baseLayerZOrder = 0;
}

SceneTopMost::~SceneTopMost()
{
	CC_SAFE_RELEASE_NULL(_sceneData);
	CC_SAFE_RELEASE_NULL(_spriteTopMostList);
	CC_SAFE_RELEASE_NULL(_sceneSpriteList);
	CC_SAFE_RELEASE_NULL(_renderTexture);
	CC_SAFE_RELEASE_NULL(_renderTextureFront);
	CC_SAFE_RELEASE_NULL(_withMenuRenderTexture);
	CC_SAFE_RELEASE_NULL(_removeShaderList);
	CC_SAFE_RELEASE_NULL(_removeWithMenuShaderList);
}

bool SceneTopMost::init(agtk::Scene *scene)
{
	if (!cocos2d::Node::init()) {
		return false;
	}
	if (scene == nullptr) {
		CC_ASSERT(0);
		return false;
	}
	_scene = scene;
	auto sceneData = _scene->getSceneData();
	this->setSceneData(sceneData);
	auto projectData = GameManager::getInstance()->getProjectData();
	this->setSceneSize(cocos2d::Size(
		projectData->getScreenWidth() * sceneData->getHorzScreenCount(),
		projectData->getScreenHeight() * sceneData->getVertScreenCount()
		));
	
	auto spriteTopMostList = cocos2d::__Array::create();
	if (spriteTopMostList == nullptr) {
		return false;
	}
	this->setSpriteTopMostList(spriteTopMostList);

	//sprite
	auto sceneSpriteList = cocos2d::__Array::create();
	if (sceneSpriteList == nullptr) {
		CC_ASSERT(0);
		return false;
	}
	this->setSceneSpriteList(sceneSpriteList);
	this->setRemoveShaderList(cocos2d::__Array::create());
	this->setRemoveWithMenuShaderList(cocos2d::__Array::create());

	return true;
}

void SceneTopMost::update(float dt)
{
	PROFILING("SceneTopMost::update", profiling);

	// 最前面にレンダーが作られている場合は、他のレンダーも必要であれば作成する
	{
		auto renderTextureCtrl = this->getRenderTexture();
		if (renderTextureCtrl) {
			auto renderTextureFrontCtrl = this->getRenderTextureFront();
			// remove shader
			auto removeShaderList = this->getRemoveShaderList();
			if (removeShaderList->count()) {
				cocos2d::Ref *ref;
				CCARRAY_FOREACH(removeShaderList, ref) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
					auto shader = static_cast<agtk::Shader *>(ref);
#else
					auto shader = dynamic_cast<agtk::Shader *>(ref);
#endif
					renderTextureCtrl->removeShader(shader);
					if (renderTextureFrontCtrl != nullptr) {
						renderTextureFrontCtrl->removeShader(shader);
					}
				}
				removeShaderList->removeAllObjects();

				if (renderTextureCtrl->getShaderList()->count() == 0) {
					removeOtherRenderTexture();
					removeRenderTexture();
				}
			}
		}
	}

	// 最前面(メニュー含む)にレンダーが作られている場合は、他のレンダーも必要であれば作成する
	{
		auto withMenuRenderTextureCtrl = this->getWithMenuRenderTexture();
		if (withMenuRenderTextureCtrl) {
			// remove shader
			auto removeShaderList = this->getRemoveWithMenuShaderList();
			if (removeShaderList->count()) {
				cocos2d::Ref *ref;
				CCARRAY_FOREACH(removeShaderList, ref) {
					auto shader = dynamic_cast<agtk::Shader *>(ref);
					withMenuRenderTextureCtrl->removeShader(shader);
				}
				removeShaderList->removeAllObjects();

				if (withMenuRenderTextureCtrl->getShaderList()->count() == 0) {
					removeOtherRenderTexture();
					removeWithMenuRenderTexture();
				}
			}
		}
	}

	auto spriteTopMostList = this->getSpriteTopMostList();
	cocos2d::Ref *ref = nullptr;
	CCARRAY_FOREACH(spriteTopMostList, ref) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
		auto sprite = static_cast<agtk::SceneTopMostSprite *>(ref);
#else
		auto sprite = dynamic_cast<agtk::SceneTopMostSprite *>(ref);
#endif
		sprite->update(dt);
	}

	//sprite
	//update
	auto sceneSpriteList = this->getSceneSpriteList();
	CCARRAY_FOREACH(sceneSpriteList, ref) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
		auto sceneSprite = static_cast<agtk::SceneSprite *>(ref);
#else
		auto sceneSprite = dynamic_cast<agtk::SceneSprite *>(ref);
#endif
		sceneSprite->update(dt);
	}
	//remove
	while (1) {
		bool bRemove = false;
		CCARRAY_FOREACH(sceneSpriteList, ref) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
			auto sceneSprite = static_cast<agtk::SceneSprite *>(ref);
#else
			auto sceneSprite = dynamic_cast<agtk::SceneSprite *>(ref);
#endif
			auto opacity = sceneSprite->getOpacityTimer();
			if (opacity->getState() == agtk::EventTimer::kStateEnd && opacity->getValue() == 0.0f) {
				//remove
				this->removeChild(sceneSprite);
				sceneSpriteList->removeObject(sceneSprite);
				bRemove = true;
				break;
			}
		}
		if (bRemove == false) break;
	}
}

/**
* レンダー更新
*/
void SceneTopMost::updateRenderer(float delta, cocos2d::Mat4 *viewMatrix)
{
	auto renderTexture = this->getRenderTexture();
	if (renderTexture) {
		renderTexture->updateTopMost(delta, viewMatrix);

		//update shader (renderTextureFront)
		auto renderTextureFront = this->getRenderTextureFront();
		if (renderTextureFront != nullptr) {
			auto renderTextureList = renderTextureFront->getRenderTextureList();
			for (int i = 0; i < renderTextureList->count(); i++) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
				auto renderTexture = static_cast<agtk::RenderTexture *>(renderTextureList->getObjectAtIndex(i));
#else
				auto renderTexture = dynamic_cast<agtk::RenderTexture *>(renderTextureList->getObjectAtIndex(i));
#endif
				auto shader = renderTexture->getShader();
				if (shader != nullptr) {
					shader->update(delta);
				}
			}
		}
	}
	auto withMenuRenderTexture = this->getWithMenuRenderTexture();
	if (withMenuRenderTexture) {
		withMenuRenderTexture->updateTopMostWithMenu(delta, viewMatrix);
	}
}

void SceneTopMost::updateRenderer(float delta, cocos2d::Mat4 *viewMatrix, int fromLayerId, int toLayerId)
{
	auto renderTexture = this->getRenderTexture();
	if (renderTexture != nullptr) {
		this->innerUpdateRenderer(delta, viewMatrix, fromLayerId, toLayerId, true, renderTexture);
	}
}

void SceneTopMost::updateRendererFront(float delta, cocos2d::Mat4 *viewMatrix, int fromLayerId, int toLayerId)
{
	auto renderTexture = this->getRenderTextureFront();
	if (renderTexture != nullptr) {
		this->innerUpdateRenderer(delta, viewMatrix, fromLayerId, toLayerId, false, renderTexture);
	}
}

void SceneTopMost::innerUpdateRenderer(float delta, cocos2d::Mat4 *viewMatrix, int fromLayerId, int toLayerId, bool bBackgroundLayer, agtk::RenderTextureCtrl *renderTextureCtrl)
{
	auto director = Director::getInstance();
	auto renderer = director->getRenderer();
	cocos2d::Camera *viewCamera = nullptr;

	cocos2d::Vec3 pos(cocos2d::Vec3::ZERO);
	cocos2d::Vec3 scale(cocos2d::Vec3::ONE);
	auto camRot3D = viewCamera ? viewCamera->getRotation3D() : director->getRunningScene()->getDefaultCamera()->getRotation3D();
	cocos2d::Mat4 m(director->getMatrix(MATRIX_STACK_TYPE::MATRIX_STACK_MODELVIEW));
	if (viewMatrix) {
		//逆変換。
		m.set(*viewMatrix);
		auto vm = viewMatrix->getInversed();
		vm.getTranslation(&pos);
		vm.getScale(&scale);
	}

	//非表示。
	cocos2d::Ref *ref = nullptr;
	auto renderTextureList = renderTextureCtrl->getRenderTextureList();
	CCARRAY_FOREACH(renderTextureList, ref) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
		auto renderTexture = static_cast<agtk::RenderTexture *>(ref);
#else
		auto renderTexture = dynamic_cast<agtk::RenderTexture *>(ref);
#endif
		renderTexture->getSprite()->setVisible(false);
	}

	//render texture
	for (int i = 0; i < renderTextureList->count(); i++) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
		auto renderTexture = static_cast<agtk::RenderTexture *>(renderTextureList->getObjectAtIndex(i));
#else
		auto renderTexture = dynamic_cast<agtk::RenderTexture *>(renderTextureList->getObjectAtIndex(i));
#endif
		auto sprite = renderTexture->getSprite();

		if (i == 0) {

			sprite->setVisible(false);
			{
				renderTexture->setKeepMatrix(true);
				{
					cocos2d::Color4F clearColor = renderTexture->getClearColor();
					renderTexture->beginWithClear(clearColor.r, clearColor.g, clearColor.b, clearColor.a);

					{
						auto scene = GameManager::getInstance()->getCurrentScene();
						// 背景シーン
						auto sceneBackground = scene->getSceneBackground();
						auto renderTextureCtrl = sceneBackground->getRenderTexture();
						cocos2d::Node *sprite = nullptr;
						if (bBackgroundLayer) {
							if (renderTextureCtrl) {
								sprite = renderTextureCtrl->getLastRenderTexture()->getSprite();
								sprite->setVisible(true);
								sprite->visit(renderer, *viewMatrix, false);
								sprite->setVisible(false);
							}
							else {
								sprite = sceneBackground;
								sprite->setVisible(true);
								sprite->visit(renderer, *viewMatrix, true);
								sprite->setVisible(false);
							}
						}


						// メインシーン
						auto dic = scene->getSceneLayerList();
						cocos2d::DictElement *el = nullptr;
						CCDICT_FOREACH(dic, el) {
							auto sceneLayer = dynamic_cast<agtk::SceneLayer *>(el->getObject());
							if (fromLayerId <= sceneLayer->getLayerId() && (sceneLayer->getLayerId() <= toLayerId || toLayerId < 0)) {
								auto renderTextureCtrl = sceneLayer->getRenderTexture();
								if (renderTextureCtrl && renderTextureCtrl->isUseShader()) {
									sprite = renderTextureCtrl->getLastRenderTexture()->getSprite();
									sprite->setVisible(true);
									sprite->visit(renderer, *viewMatrix, false);
									sprite->setVisible(false);
								}
								else {
									sprite = sceneLayer;
									sprite->setVisible(true);
									sprite->visit(renderer, *viewMatrix, true);
									sprite->setVisible(false);
								}
							}
						}
					}
					renderTexture->end();
				}
				renderTexture->setKeepMatrix(false);
			}
			sprite->setVisible(true);

			//shader
			auto shader = renderTexture->getShader();
			if (shader) {
				shader->update(delta);
			}
		}
		else {
			auto renderTexture2 = dynamic_cast<agtk::RenderTexture *>(renderTextureList->getObjectAtIndex(i - 1));
			auto sprite2 = renderTexture2->getSprite();
			renderTexture->update(delta, sprite2, *viewMatrix);
		}
		sprite->setPosition3D(pos);
		sprite->setRotation3D(camRot3D);
		sprite->setScale(scale.x, scale.y);
	}
}

/**
* メニューを無視したレンダー更新
*/
void SceneTopMost::updateRendererIgnoreMenu(float delta, cocos2d::Mat4 *viewMatrix)
{
	auto withMenuRenderTexture = this->getWithMenuRenderTexture();
	if (withMenuRenderTexture) {
		withMenuRenderTexture->updateTopMostWithMenuIgnoreMenu(delta, viewMatrix);
	}
}

/**
* メニューのみレンダー更新
*/
void SceneTopMost::updateRendererOnlyMenu(float delta, cocos2d::Mat4 *viewMatrix)
{
	auto withMenuRenderTexture = this->getWithMenuRenderTexture();
	if (withMenuRenderTexture) {
		withMenuRenderTexture->updateTopMostWithMenuOnlyMenu(delta, viewMatrix);
	}
}

/**
* シェーダー作成
*/
void SceneTopMost::createShader(cocos2d::Layer *layer)
{
	auto projectData = GameManager::getInstance()->getProjectData();

	// レンダーテクスチャが作成されていない場合は作成する
	auto renderTextureCtrl = this->getRenderTexture();
// #AGTK-NX
#if CC_TARGET_PLATFORM != CC_PLATFORM_NX
	if (renderTextureCtrl == false) {
#else
#endif
		bool isCreate = false;

		//シーン効果
		auto sceneData = this->getSceneData();
		auto sceneFilterEffectList = sceneData->getSceneFilterEffectList();
		cocos2d::DictElement *el;
		CCDICT_FOREACH(sceneFilterEffectList, el) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
			auto filterEffectData = static_cast<agtk::data::SceneFilterEffectData *>(el->getObject());
#else
			auto filterEffectData = dynamic_cast<agtk::data::SceneFilterEffectData *>(el->getObject());
#endif
			if (filterEffectData->getDisabled()) {
				continue;
			}

			if (filterEffectData->getLayerIndex() == agtk::data::SceneFilterEffectData::kLayerIndexTopMost) {//最前面
				isCreate = true;
				break;
			}
		}

		if (isCreate == false) {
			return;
		}

		createRenderTexture();

		renderTextureCtrl = this->getRenderTexture();
	}
	renderTextureCtrl->setLayer(layer);
	this->getRenderTextureFront()->setLayer(layer);

	std::function<void(agtk::data::SceneFilterEffectData *)> createFilterEffect = [&](agtk::data::SceneFilterEffectData *data) {
		if (data->getDisabled()) {
			return;
		}
		auto filterEffect = data->getFilterEffect();
		float seconds = (float)filterEffect->getDuration300() / 300.0f;
		switch (filterEffect->getEffectType()) {
		case agtk::data::FilterEffect::kEffectNoise: {//ノイズ
			this->setShader(agtk::Shader::kShaderNoisy, (float)filterEffect->getNoise() / 100.0f, seconds);
			break; }
		case agtk::data::FilterEffect::kEffectMosaic: {//モザイク
			this->setShader(agtk::Shader::kShaderMosaic, (float)filterEffect->getMosaic() / 100.0f, seconds);
			break; }
		case agtk::data::FilterEffect::kEffectMonochrome: {//モノクロ
			this->setShader(agtk::Shader::kShaderColorGray, (float)filterEffect->getMonochrome() / 100.0f, seconds);
			break; }
		case agtk::data::FilterEffect::kEffectSepia: {//セピア
			this->setShader(agtk::Shader::kShaderColorSepia, (float)filterEffect->getSepia() / 100.0f, seconds);
			break; }
		case agtk::data::FilterEffect::kEffectNegaPosiReverse: {//ネガ反転
			this->setShader(agtk::Shader::kShaderColorNega, (float)filterEffect->getNegaPosiReverse() / 100.0f, seconds);
			break; }
		case agtk::data::FilterEffect::kEffectDefocus: {//ぼかし
			this->setShader(agtk::Shader::kShaderBlur, (float)filterEffect->getDefocus() / 100.0f, seconds);
			break; }
		case agtk::data::FilterEffect::kEffectChromaticAberration: {//色収差
			this->setShader(agtk::Shader::kShaderColorChromaticAberration, (float)filterEffect->getChromaticAberration() / 100.0f, seconds);
			break; }
		case agtk::data::FilterEffect::kEffectDarkness: {//暗闇
			this->setShader(agtk::Shader::kShaderColorDark, (float)filterEffect->getDarkness() / 100.0f, seconds);
			break; }
		case agtk::data::FilterEffect::kEffectTransparency: {//透明
			this->setShader(agtk::Shader::kShaderTransparency, (float)filterEffect->getTransparency() / 100.0f, seconds);
			break; }
		case agtk::data::FilterEffect::kEffectBlink: {//点滅
			// 点滅はオブジェクトのみに実装。
			break; }
		case agtk::data::FilterEffect::kEffectDispImage: {//画像表示
			if (filterEffect->getImageId() >= 0) {
				auto projectData = GameManager::getInstance()->getProjectData();
				auto imageData = projectData->getImageData(filterEffect->getImageId());
				cocos2d::Size texSizeDef;

				// create texture2d
				auto texture2d = CreateTexture2D(imageData->getFilename(), (filterEffect->getImagePlacement() == agtk::data::FilterEffect::kPlacementTiling), nullptr, &texSizeDef.width, &texSizeDef.height);
				if (filterEffect->getImagePlacement() == agtk::data::FilterEffect::kPlacementTiling) {
					//タイリング
					Texture2D::TexParams tRepeatParams;
					tRepeatParams.magFilter = GL_LINEAR;
					tRepeatParams.minFilter = GL_LINEAR;
					tRepeatParams.wrapS = GL_REPEAT;
					tRepeatParams.wrapT = GL_REPEAT;
					texture2d->setTexParameters(tRepeatParams);
				}
				else {
					texture2d->setAliasTexParameters();
				}

				auto sceneSize = projectData->getSceneSize(this->getSceneData());
				auto shader = renderTextureCtrl->addShader(agtk::Shader::kShaderImage, sceneSize, (float)(100.0f - filterEffect->getImageTransparency()) / 100.0f, seconds);
				shader->setMaskTexture(texture2d);
				// set imgPlacement
				int imgPlacement = filterEffect->getImagePlacement();
				auto programState = shader->getProgramState();
				programState->setUniformInt("imgPlacement", imgPlacement);
				// set resolution
				float width = sceneSize.width;
				float height = sceneSize.height;
				programState->setUniformVec2("resolution", cocos2d::Vec2(width, height));
				// set imgResolution
				auto imgResolution = (imgPlacement == 2) ? Vec2(texture2d->getContentSize().width, texture2d->getContentSize().height) : Vec2(width, height);
				programState->setUniformVec2("imgResolution", imgResolution);
				// set imgSizeRate
				auto imgSizeRate = cocos2d::Vec2(texSizeDef.width / texture2d->getContentSize().width, texSizeDef.height / texture2d->getContentSize().height);
				programState->setUniformVec2("imgSizeRate", imgSizeRate);
				// set sxy
				float imgSourceWidth = texSizeDef.width;
				float imgSourceHeight = texSizeDef.height;
				auto sxy = (imgPlacement == 3) ? ((width / imgSourceWidth <= height / imgSourceHeight) ? Vec2(1, height / imgSourceHeight * imgSourceWidth / width) : Vec2(width / imgSourceWidth * imgSourceHeight / height, 1)) : (imgSourceWidth > 0 && imgSourceHeight > 0 ? Vec2(width / imgSourceWidth, height / imgSourceHeight) : Vec2(1, 1));
				programState->setUniformVec2("sxy", sxy);
				// set imgXy
				auto imgXy = Vec2((1 - sxy.x) / 2, (1 - sxy.y) / 2);
				programState->setUniformVec2("imgXy", imgXy);
				// set FilterEffect
				shader->setUserData(filterEffect);
			}
			break; }
		case agtk::data::FilterEffect::kEffectFillColor: {//指定色で塗る
			this->setShader(agtk::Shader::kShaderColorRgba, 1.0f, seconds);
			auto shader = this->getShader(agtk::Shader::kShaderColorRgba);
			shader->setShaderRgbaColor(cocos2d::Color4B(filterEffect->getFillR(), filterEffect->getFillG(), filterEffect->getFillB(), filterEffect->getFillA()));
			break; }
		default:CC_ASSERT(0);
		}
	};

	//シーン効果
	auto sceneData = this->getSceneData();
	auto sceneFilterEffectList = sceneData->getSceneFilterEffectList();
	cocos2d::DictElement *el;
	CCDICT_FOREACH(sceneFilterEffectList, el) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
		auto filterEffectData = static_cast<agtk::data::SceneFilterEffectData *>(el->getObject());
#else
		auto filterEffectData = dynamic_cast<agtk::data::SceneFilterEffectData *>(el->getObject());
#endif
		if (filterEffectData->getLayerIndex() == agtk::data::SceneFilterEffectData::kLayerIndexTopMost) {//最前面
			createFilterEffect(filterEffectData);
		}
	}
}

/**
* シェーダー作成(メニュー含む)
*/
void SceneTopMost::createWithMenuShader(cocos2d::Layer *layer)
{
	auto projectData = GameManager::getInstance()->getProjectData();

	// レンダーテクスチャが作成されていない場合は作成する
	auto renderTextureCtrl = this->getWithMenuRenderTexture();
// #AGTK-NX
#if CC_TARGET_PLATFORM != CC_PLATFORM_NX
	if (renderTextureCtrl == false) {
#else
#endif
		bool isCreate = false;

		float value = 0.0f;
		if (projectData->getScreenEffect(0, &value)) {//レトロゲーム機
			isCreate = true;
		}
		else if (projectData->getScreenEffect(1, &value)) {//ぼかし
			isCreate = true;
		}
		else if (projectData->getScreenEffect(2, &value)) {//アナログTV
			isCreate = true;
		}

		//シーン効果
		auto sceneData = this->getSceneData();
		auto sceneFilterEffectList = sceneData->getSceneFilterEffectList();
		cocos2d::DictElement *el;
		CCDICT_FOREACH(sceneFilterEffectList, el) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
			auto filterEffectData = static_cast<agtk::data::SceneFilterEffectData *>(el->getObject());
#else
			auto filterEffectData = dynamic_cast<agtk::data::SceneFilterEffectData *>(el->getObject());
#endif
			if (filterEffectData->getDisabled()) {
				continue;
			}

			if (filterEffectData->getLayerIndex() == agtk::data::SceneFilterEffectData::kLayerIndexTopMostWithMenu) {//最前面(メニュー含む)
				isCreate = true;
				break;
			}
		}

		if (isCreate == false) {
			return;
		}

		createWithMenuRenderTexture();

		renderTextureCtrl = this->getWithMenuRenderTexture();
	}
	renderTextureCtrl->setLayer(layer);

	float value = 0.0f;
	if (projectData->getScreenEffect(0, &value)) {//アナログTV
		renderTextureCtrl->addShader(agtk::Shader::kShaderCRTMonitor, value * 0.01f);
	}
	if (projectData->getScreenEffect(1, &value)) {//ぼかし
		renderTextureCtrl->addShader(agtk::Shader::kShaderBlur, value * 0.01f);
	}
	if (projectData->getScreenEffect(2, &value)) {//レトロゲーム機
		renderTextureCtrl->addShader(agtk::Shader::kShaderColorGameboy, value * 0.01f);
	}

	std::function<void(agtk::data::SceneFilterEffectData *)> createFilterEffect = [&](agtk::data::SceneFilterEffectData *data) {
		if (data->getDisabled()) {
			return;
		}
		auto filterEffect = data->getFilterEffect();
		float seconds = (float)filterEffect->getDuration300() / 300.0f;
		switch (filterEffect->getEffectType()) {
		case agtk::data::FilterEffect::kEffectNoise: {//ノイズ
			this->setWithMenuShader(agtk::Shader::kShaderNoisy, (float)filterEffect->getNoise() / 100.0f, seconds);
			break; }
		case agtk::data::FilterEffect::kEffectMosaic: {//モザイク
			this->setWithMenuShader(agtk::Shader::kShaderMosaic, (float)filterEffect->getMosaic() / 100.0f, seconds);
			break; }
		case agtk::data::FilterEffect::kEffectMonochrome: {//モノクロ
			this->setWithMenuShader(agtk::Shader::kShaderColorGray, (float)filterEffect->getMonochrome() / 100.0f, seconds);
			break; }
		case agtk::data::FilterEffect::kEffectSepia: {//セピア
			this->setWithMenuShader(agtk::Shader::kShaderColorSepia, (float)filterEffect->getSepia() / 100.0f, seconds);
			break; }
		case agtk::data::FilterEffect::kEffectNegaPosiReverse: {//ネガ反転
			this->setWithMenuShader(agtk::Shader::kShaderColorNega, (float)filterEffect->getNegaPosiReverse() / 100.0f, seconds);
			break; }
		case agtk::data::FilterEffect::kEffectDefocus: {//ぼかし
			this->setWithMenuShader(agtk::Shader::kShaderBlur, (float)filterEffect->getDefocus() / 100.0f, seconds);
			break; }
		case agtk::data::FilterEffect::kEffectChromaticAberration: {//色収差
			this->setWithMenuShader(agtk::Shader::kShaderColorChromaticAberration, (float)filterEffect->getChromaticAberration() / 100.0f, seconds);
			break; }
		case agtk::data::FilterEffect::kEffectDarkness: {//暗闇
			this->setWithMenuShader(agtk::Shader::kShaderColorDark, (float)filterEffect->getDarkness() / 100.0f, seconds);
			break; }
		case agtk::data::FilterEffect::kEffectTransparency: {//透明
			this->setWithMenuShader(agtk::Shader::kShaderTransparency, (float)filterEffect->getTransparency() / 100.0f, seconds);
			break; }
		case agtk::data::FilterEffect::kEffectBlink: {//点滅
			// 点滅はオブジェクトのみに実装。
			break; }
		case agtk::data::FilterEffect::kEffectDispImage: {//画像表示
			if (filterEffect->getImageId() >= 0) {
				auto projectData = GameManager::getInstance()->getProjectData();
				auto imageData = projectData->getImageData(filterEffect->getImageId());
				cocos2d::Size texSizeDef;

				// create texture2d
				auto texture2d = CreateTexture2D(imageData->getFilename(), (filterEffect->getImagePlacement() == agtk::data::FilterEffect::kPlacementTiling), nullptr, &texSizeDef.width, &texSizeDef.height);
				if (filterEffect->getImagePlacement() == agtk::data::FilterEffect::kPlacementTiling) {
					//タイリング
					Texture2D::TexParams tRepeatParams;
					tRepeatParams.magFilter = GL_LINEAR;
					tRepeatParams.minFilter = GL_LINEAR;
					tRepeatParams.wrapS = GL_REPEAT;
					tRepeatParams.wrapT = GL_REPEAT;
					texture2d->setTexParameters(tRepeatParams);
				}
				else {
					texture2d->setAliasTexParameters();
				}

				auto sceneSize = projectData->getSceneSize(this->getSceneData());
				auto shader = renderTextureCtrl->addShader(agtk::Shader::kShaderImage, sceneSize, (float)(100.0f - filterEffect->getImageTransparency()) / 100.0f, seconds);
				shader->setMaskTexture(texture2d);
				// set imgPlacement
				int imgPlacement = filterEffect->getImagePlacement();
				auto programState = shader->getProgramState();
				programState->setUniformInt("imgPlacement", imgPlacement);
				// set resolution
				float width = sceneSize.width;
				float height = sceneSize.height;
				programState->setUniformVec2("resolution", cocos2d::Vec2(width, height));
				// set imgResolution
				auto imgResolution = (imgPlacement == 2) ? Vec2(texture2d->getContentSize().width, texture2d->getContentSize().height) : Vec2(width, height);
				programState->setUniformVec2("imgResolution", imgResolution);
				// set imgSizeRate
				auto imgSizeRate = cocos2d::Vec2(texSizeDef.width / texture2d->getContentSize().width, texSizeDef.height / texture2d->getContentSize().height);
				programState->setUniformVec2("imgSizeRate", imgSizeRate);
				// set sxy
				float imgSourceWidth = texSizeDef.width;
				float imgSourceHeight = texSizeDef.height;
				auto sxy = (imgPlacement == 3) ? ((width / imgSourceWidth <= height / imgSourceHeight) ? Vec2(1, height / imgSourceHeight * imgSourceWidth / width) : Vec2(width / imgSourceWidth * imgSourceHeight / height, 1)) : (imgSourceWidth > 0 && imgSourceHeight > 0 ? Vec2(width / imgSourceWidth, height / imgSourceHeight) : Vec2(1, 1));
				programState->setUniformVec2("sxy", sxy);
				// set imgXy
				auto imgXy = Vec2((1 - sxy.x) / 2, (1 - sxy.y) / 2);
				programState->setUniformVec2("imgXy", imgXy);
				// set FilterEffect
				shader->setUserData(filterEffect);
			}
			break; }
		case agtk::data::FilterEffect::kEffectFillColor: {//指定色で塗る
			this->setWithMenuShader(agtk::Shader::kShaderColorRgba, 1.0f, seconds);
			auto shader = this->getWithMenuShader(agtk::Shader::kShaderColorRgba);
			shader->setShaderRgbaColor(cocos2d::Color4B(filterEffect->getFillR(), filterEffect->getFillG(), filterEffect->getFillB(), filterEffect->getFillA()));
			break; }
		default:CC_ASSERT(0);
		}
	};

	//シーン効果
	auto sceneData = this->getSceneData();
	auto sceneFilterEffectList = sceneData->getSceneFilterEffectList();
	cocos2d::DictElement *el;
	CCDICT_FOREACH(sceneFilterEffectList, el) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
		auto filterEffectData = static_cast<agtk::data::SceneFilterEffectData *>(el->getObject());
#else
		auto filterEffectData = dynamic_cast<agtk::data::SceneFilterEffectData *>(el->getObject());
#endif
		if (filterEffectData->getLayerIndex() == agtk::data::SceneFilterEffectData::kLayerIndexTopMostWithMenu) {//最前面(メニュー含む)
			createFilterEffect(filterEffectData);
		}
	}
}

/**
* シェーダーセット
*/
void SceneTopMost::setShader(Shader::ShaderKind kind, float value, float seconds)
{
	// レンダーテクスチャが作成されていない場合は作成する
	auto renderTextureCtrl = this->getRenderTexture();
// #AGTK-NX
#if CC_TARGET_PLATFORM != CC_PLATFORM_NX
	if (renderTextureCtrl == false) {
#else
#endif
		createObjectCommandRenderTexture();
		renderTextureCtrl = this->getRenderTexture();
	}

	auto shader = this->getShader(kind);
	if (shader == nullptr) {
		renderTextureCtrl->addShader(kind, value, seconds);
		auto renderTextureFrontCtrl = this->getRenderTextureFront();
		if (renderTextureFrontCtrl != nullptr) {
			renderTextureFrontCtrl->addShader(kind, value, seconds);
		}
	}
	else {
		shader->setIntensity(value, seconds);
		shader->setIgnored(false);//有効に。
	}
}

/**
* シェーダーセット(メニュー含む)
*/
void SceneTopMost::setWithMenuShader(Shader::ShaderKind kind, float value, float seconds)
{
	// レンダーテクスチャが作成されていない場合は作成する
	auto renderTextureCtrl = this->getWithMenuRenderTexture();
// #AGTK-NX
#if CC_TARGET_PLATFORM != CC_PLATFORM_NX
	if (renderTextureCtrl == false) {
#else
#endif
		createObjectCommandWithMenuRenderTexture();
		renderTextureCtrl = this->getWithMenuRenderTexture();
	}

	auto shader = this->getWithMenuShader(kind);
	if (shader == nullptr) {
		renderTextureCtrl->addShader(kind, value, seconds);
	}
	else {
		shader->setIntensity(value, seconds);
		shader->setIgnored(false);//有効に。
	}
}

/**
* シェーダー取得
*/
agtk::Shader *SceneTopMost::getShader(Shader::ShaderKind kind)
{
	auto renderTexture = this->getRenderTexture();
// #AGTK-NX
#if CC_TARGET_PLATFORM != CC_PLATFORM_NX
	if (renderTexture == false) {
#else
#endif
		return nullptr;
	}

	auto projectData = GameManager::getInstance()->getProjectData();
	auto shaderList = renderTexture->getShaderList();
	for (int idx = 0; idx < shaderList->count(); idx++) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
		auto shader = static_cast<agtk::Shader *>(shaderList->getObjectAtIndex(idx));
#else
		auto shader = dynamic_cast<agtk::Shader *>(shaderList->getObjectAtIndex(idx));
#endif
		if (shader->getKind() == kind) {
			return shader;
		}
	}
	return nullptr;
}

/**
* シェーダー取得(メニュー含む)
*/
agtk::Shader *SceneTopMost::getWithMenuShader(Shader::ShaderKind kind)
{
	auto renderTextureCtrl = this->getWithMenuRenderTexture();
// #AGTK-NX
#if CC_TARGET_PLATFORM != CC_PLATFORM_NX
	if (renderTextureCtrl == false) {
#else
#endif
		return nullptr;
	}

	//※内部固定シェーダー（レトロゲーム機、ぼかし、アナログTV)があり、それ以外のシェーダーを得られるようにする。
	//また「ぼかし」は、内部固定シェーダーと別に、ここから指定出来るようにする。
	auto projectData = GameManager::getInstance()->getProjectData();
	auto shaderList = renderTextureCtrl->getShaderList();
	int count = projectData->getScreenEffectCount();
	if (shaderList->count() > count) {//内部固定シェーダーを飛ばす。
		int max = shaderList->count() - count;
		for (int idx = 0; idx < max; idx++) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
			auto shader = static_cast<agtk::Shader *>(shaderList->getObjectAtIndex(idx));
#else
			auto shader = dynamic_cast<agtk::Shader *>(shaderList->getObjectAtIndex(idx));
#endif
			if (shader->getKind() == kind) {
				return shader;
			}
		}
	}
	return nullptr;
}

/**
* シェーダー削除
*/
void SceneTopMost::removeShader(Shader::ShaderKind kind, float seconds)
{
	auto renderTexture = this->getRenderTexture();
// #AGTK-NX
#if CC_TARGET_PLATFORM != CC_PLATFORM_NX
	if (renderTexture == false) {
#else
#endif
		return;
	}

	auto shader = this->getShader(kind);
	if (shader == nullptr) {
		return;
	}
	shader->setIntensity(0.0f, seconds);
	auto value = shader->getValue();
	value->setEndFunc([&, shader]() {
		this->getRemoveShaderList()->addObject(shader);
	});
}

/**
* シェーダー削除(メニュー含む)
*/
void SceneTopMost::removeWithMenuShader(Shader::ShaderKind kind, float seconds)
{
	auto renderTextureCtrl = this->getWithMenuRenderTexture();
// #AGTK-NX
#if CC_TARGET_PLATFORM != CC_PLATFORM_NX
	if (renderTextureCtrl == false) {
#else
#endif
		return;
	}

	auto shader = this->getWithMenuShader(kind);
	if (shader == nullptr) {
		return;
	}
	shader->setIntensity(0.0f, seconds);
	auto value = shader->getValue();
	value->setEndFunc([&, shader]() {
		this->getRemoveWithMenuShaderList()->addObject(shader);
	});
}

/**
* レンダーテクスチャ作成
*/
void SceneTopMost::createRenderTexture()
{
	// レンダー作成時は他のシーンオブジェクトを表示させる
	setVisible(true);

	auto projectData = GameManager::getInstance()->getProjectData();

	auto renderTextureCtrl = agtk::RenderTextureCtrl::createForSceneTopMost(this, projectData->getScreenSize(), BaseLayer::ZOrder::TopMost);
	this->addChild(renderTextureCtrl);
	this->setRenderTexture(renderTextureCtrl);

	// create renderTextureFront
	auto renderTextureFrontCtrl = agtk::RenderTextureCtrl::createForSceneTopMost(this, projectData->getScreenSize(), BaseLayer::ZOrder::TopMost);
	this->addChild(renderTextureFrontCtrl);
	this->setRenderTextureFront(renderTextureFrontCtrl);
}

/**
* レンダーテクスチャ作成(メニュー含む)
*/
void SceneTopMost::createWithMenuRenderTexture()
{
	// レンダー作成時は他のシーンオブジェクトを表示させる
	setVisible(true);

	auto projectData = GameManager::getInstance()->getProjectData();

	auto renderTextureCtrl = agtk::RenderTextureCtrl::createForSceneTopMost(this, projectData->getScreenSize(), BaseLayer::ZOrder::TopMostWithMenu);
	this->addChild(renderTextureCtrl);
	this->setWithMenuRenderTexture(renderTextureCtrl);
}

/**
* オブジェクトコマンドでのレンダーテクスチャ生成
*/
void SceneTopMost::createObjectCommandRenderTexture()
{
	// レンダー作成時は他のシーンオブジェクトを表示させる
	setVisible(true);

	auto projectData = GameManager::getInstance()->getProjectData();

	auto baseLayer = _scene->getBaseLayer();

	auto renderTextureCtrl = agtk::RenderTextureCtrl::createForSceneTopMost(this, projectData->getScreenSize(), BaseLayer::ZOrder::TopMost);
	renderTextureCtrl->setLayer(baseLayer);
	this->addChild(renderTextureCtrl);

	baseLayer->Node::addChild(renderTextureCtrl->getFirstRenderTexture(), _baseLayerZOrder, this->getName());

	this->setRenderTexture(renderTextureCtrl);

	// create renderTextureFront
	auto renderTextureFrontCtrl = agtk::RenderTextureCtrl::createForSceneTopMost(this, projectData->getScreenSize(), BaseLayer::ZOrder::TopMost);
	renderTextureFrontCtrl->setLayer(baseLayer);
	this->addChild(renderTextureFrontCtrl);

	baseLayer->Node::addChild(renderTextureFrontCtrl->getFirstRenderTexture(), _baseLayerZOrder, this->getName());
	this->setRenderTextureFront(renderTextureFrontCtrl);
}

/**
* オブジェクトコマンドでのレンダーテクスチャ生成(メニュー含む)
*/
void SceneTopMost::createObjectCommandWithMenuRenderTexture()
{
	// レンダー作成時は他のシーンオブジェクトを表示させる
	setVisible(true);

	auto projectData = GameManager::getInstance()->getProjectData();

	auto baseLayer = _scene->getBaseLayer();

	auto renderTextureCtrl = agtk::RenderTextureCtrl::createForSceneTopMost(this, projectData->getScreenSize(), BaseLayer::ZOrder::TopMostWithMenu);
	renderTextureCtrl->setLayer(baseLayer);
	this->addChild(renderTextureCtrl);

	baseLayer->Node::addChild(renderTextureCtrl->getFirstRenderTexture(), _baseLayerZOrder, this->getName());

	this->setWithMenuRenderTexture(renderTextureCtrl);
}

void SceneTopMost::createOtherRenderTexture()
{
	auto scene = GameManager::getInstance()->getCurrentScene();

	// 背景
	{
		auto sceneBackground = scene->getSceneBackground();
		auto renderTextureCtrl = sceneBackground->getRenderTexture();
// #AGTK-NX
#if CC_TARGET_PLATFORM != CC_PLATFORM_NX
		if (renderTextureCtrl == false) {
#else
#endif
			sceneBackground->createObjectCommandRenderTexture();
		}
	}
}

/**
* レンダーテクスチャ削除
*/
void SceneTopMost::removeRenderTexture()
{
	// レンダー削除時は他のシーンオブジェクトを表示させる
	setVisible(true);

	auto renderTexture = this->getRenderTexture();
	if (renderTexture) {
		renderTexture->removeFromParentAndCleanup(true);
		this->setRenderTexture(nullptr);
	}
	auto renderTextureFront = this->getRenderTextureFront();
	if (renderTextureFront) {
		renderTextureFront->removeFromParentAndCleanup(true);
		this->setRenderTextureFront(nullptr);
	}
	this->getRemoveShaderList()->removeAllObjects();
}

/**
* レンダーテクスチャ削除(メニュー含む)
*/
void SceneTopMost::removeWithMenuRenderTexture()
{
	// レンダー削除時は他のシーンオブジェクトを表示させる
	setVisible(true);

	auto renderTexture = this->getWithMenuRenderTexture();
	if (renderTexture) {
		renderTexture->removeFromParentAndCleanup(true);
		this->setWithMenuRenderTexture(nullptr);
	}
	this->getRemoveWithMenuShaderList()->removeAllObjects();
}

void SceneTopMost::removeOtherRenderTexture()
{
	auto scene = GameManager::getInstance()->getCurrentScene();

	// 背景
	{
		auto sceneBackground = scene->getSceneBackground();
		if (sceneBackground->isRemoveRenderTexture()) {
			sceneBackground->removeRenderTexture();
		}
	}
}

void SceneTopMost::createSceneSprite(int imageId, int opacity, cocos2d::Vec2 pos, float seconds)
{
	auto sceneSprite = agtk::SceneSprite::create(imageId, opacity, seconds);
	sceneSprite->setPosition(pos);
	this->getSceneSpriteList()->addObject(sceneSprite);
	this->addChild(sceneSprite);
}

void SceneTopMost::removeSceneSprite(float seconds)
{
	auto sceneSpriteList = this->getSceneSpriteList();
	cocos2d::Ref *ref = nullptr;
	CCARRAY_FOREACH(sceneSpriteList, ref) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
		auto sceneSprite = static_cast<agtk::SceneSprite *>(ref);
#else
		auto sceneSprite = dynamic_cast<agtk::SceneSprite *>(ref);
#endif
		sceneSprite->getOpacityTimer()->setValue(0, seconds);
	}
}

void SceneTopMost::setVisibleRenderTexture(bool visible)
{
	auto withMenuRenderTextureCtrl = this->getWithMenuRenderTexture();
	if (withMenuRenderTextureCtrl != nullptr) {
		auto lastRenderTexture = withMenuRenderTextureCtrl->getLastRenderTexture();
		if (lastRenderTexture != nullptr) {
			lastRenderTexture->setVisible(visible);
		}
	}
	auto renderTextureCtrl = this->getRenderTexture();
	if (renderTextureCtrl != nullptr) {
		auto lastRenderTexture = renderTextureCtrl->getLastRenderTexture();
		if (lastRenderTexture != nullptr) {
			lastRenderTexture->setVisible(visible);
		}
		auto renderTextureFrontCtrl = this->getRenderTextureFront();
		if (renderTextureFrontCtrl != nullptr) {
			auto lastRenderTexture = renderTextureFrontCtrl->getLastRenderTexture();
			if (lastRenderTexture != nullptr) {
				lastRenderTexture->setVisible(visible);
			}
		}
	}
}

//-------------------------------------------------------------------------------------------------------------------
SceneGravity::SceneGravity()
{
	_sceneData = nullptr;
	_gravity = cocos2d::Vec2(0.0f, -1.0f);
	_duration300 = 0;
	_duration = 0.0f;
	_rotation = 180.0f;
	_timerRotation = nullptr;
	this->setTimerRotation(TimerFloat::create(0.0f));
}

SceneGravity::~SceneGravity()
{
	CC_SAFE_RELEASE_NULL(_sceneData);
	CC_SAFE_RELEASE_NULL(_timerRotation);
}

bool SceneGravity::init(agtk::data::SceneData *sceneData)
{
	this->setSceneData(sceneData);
	this->setDuration300(-1);//時間制限なし。
	this->setRotation(sceneData->getGravityDirection());
	this->set(100.0f, this->getRotation(), 0, true);
	return true;
}

void SceneGravity::update(float dt)
{
	auto timerRotation = this->getTimerRotation();
	if (timerRotation->getState() != agtk::EventTimer::kStateIdle) {
		timerRotation->update(dt);
		this->set(100.0f, timerRotation->getValue(), 0, true, false);
	}

	if (this->getDuration300() < 0) {
		return;
	}

	float duration = 300.0f * dt;
	_duration += duration;
	if (this->getDuration300() <= _duration) {
		auto sceneData = this->getSceneData();
		this->set(100.0f, sceneData->getGravityDirection(), 0, true);
	}

}

void SceneGravity::set(float gravity, float rotation, int duration300, bool bDurationUnlimited, bool isContainPhyiscsWorld)
{
	_rotation = rotation;
	_gravity = agtk::GetDirectionFromDegrees(rotation);
	_gravity *= this->getSceneData()->getGravityAccel() * (gravity * 0.01f);//重力(%)
	if (bDurationUnlimited) {//時間制限なし
		this->setDuration300(-1);
	}
	else {
		this->setDuration300(duration300);//効果時間
	}
	_duration = 0.0f;

	if (isContainPhyiscsWorld) {
		// 物理空間の重力も設定
		GameManager::getInstance()->setGravity(this->getSceneData()->getGravityAccel(), rotation);
	}
}

//-------------------------------------------------------------------------------------------------------------------
SceneWater::SceneWater()
{
	_sceneData = nullptr;
	_duration = 0;
}

SceneWater::~SceneWater()
{
	CC_SAFE_RELEASE_NULL(_sceneData);
}

bool SceneWater::init(agtk::data::SceneData *sceneData)
{
	this->setSecenData(sceneData);
	this->set(0.0f, 0, true);
	return true;
}

void SceneWater::update(float dt)
{
	if (this->getDuration300() < 0) {
		return;
	}
	int duration = 300 * dt;
	_duration += duration;
	if (this->getDuration300() <= _duration) {
		this->set(0.0f, 0, true);
	}
}

void SceneWater::set(float water, int duration300, bool bDurationUnlimited)
{
	this->setWater(water);
	if (bDurationUnlimited) {//時間制限なし
		this->setDuration300(-1);
	}
	else {
		this->setDuration300(duration300);//効果時間
	}
	_duration = 0;
}

//-------------------------------------------------------------------------------------------------------------------
GameSpeed::GameSpeed() 
	:_timeScale(1.f)
	,_duration300(DURATION_UNLIMITED)
	,_duration(0)
#ifdef USE_PREVIEW
	,_paused(false)
#endif()
	,_state(eStateIdle)
{
	_timeScaleNext = _timeScale;
	_timeScalePrev = _timeScale;
}
GameSpeed::~GameSpeed()
{
}

bool GameSpeed::update(float dt)
{
	float _dt = (300.0f * FRAME_PER_SECONDS) * (dt * FRAME60_RATE);
	switch (_state) {
	case eStateIdle:
		break;
	case eStateStart:
		if (dt == 0.0f) {
			break;
		}
		_state = eStateUpdate;
		_timeScale = _timeScaleNext;
	case eStateUpdate:
		if (this->getDuration300() == DURATION_UNLIMITED) {
			return true;
		}
		_duration += _dt;
		if(_duration + (_duration * FLT_EPSILON) >= this->getDuration300()) {
			_timeScale = 1.0f;
			_duration = 0.0f;
			_duration300 = DURATION_UNLIMITED;
			_timeScale = _timeScaleNext;
			_state = eStateEnd;
		}
		break;
	case eStateEnd:
		_state = eStateIdle;
		return false;
	}
	return true;
}

void GameSpeed::set(float gameSpeed)
{
	set(gameSpeed, getDuration300());
}

void GameSpeed::set(float gameSpeed, float duration300)
{
	setTimeScale(gameSpeed * 0.01f);
	setDuration300(duration300);
	setDuration(0.0f);
	setState(eStateStart);
}

float GameSpeed::getTimeScale() const 
{
#ifdef USE_PREVIEW
	if (getPaused()) {
		return 0.f;
	}
#endif
	return _timeScale;
}

void GameSpeed::setTimeScale(float scale)
{
	_timeScalePrev = _timeScale;
	_timeScaleNext = scale;
}

SceneGameSpeed::SceneGameSpeed()
{
	_timeScales.reserve(10); // 適当に。
}

SceneGameSpeed::~SceneGameSpeed()
{
}

bool SceneGameSpeed::init(agtk::data::SceneData *sceneData)
{
	return true;
}

void SceneGameSpeed::update(float dt)
{
	std::list<Data> deleteList;
	for (auto & elem : _timeScales) {
		if (elem.gs.update(dt) == false) {
			// 効果時間終了
			deleteList.push_back(elem);
		}
	}
	// 効果終了したものを管理リストから削除
	for (auto& elem : deleteList) {
		remove(elem);
	}
}

void SceneGameSpeed::set(Type type, int targettingType, int targetObjectGroup, int targetObjectId, int targetQualifierId, cocos2d::RefPtr<cocos2d::Array> targetObjList,float gameSpeed, float duration)
{
	Target t;
	t._type = type;
	t._targetObjectType = targettingType;
	t._targetObjectGroup = targetObjectGroup;
	t._targetObjectId = targetObjectId;
	t._targetQualifierId = targetQualifierId;
	t._targetObjectList = targetObjList;

	GameSpeed gs;
	gs.set(gameSpeed, duration);

	set(t, gs);
}

void SceneGameSpeed::set(const Target& t, const GameSpeed& gs)
{
	auto exist = find(t);
	if (exist) {
		remove(*exist);
	}

	// 新規登録
	Data data;
	data.target = t;
	data.gs = gs;
	_timeScales.emplace_back(data);
}

float SceneGameSpeed::getTimeScale(agtk::Object const* object)const
{
	CC_ASSERT(object);
	return getTimeScale(eTYPE_OBJECT, object);
}

float SceneGameSpeed::getTimeScale(Type type, agtk::Object const* object)const
{	
	if (type == eTYPE_OBJECT) {
		// GUIによるオブジェクト停止が発生している場合はタイムスケールを0で返す
		if (GuiManager::getInstance()->getObjectStop()) {
			return 0.0f;
		}
		CC_ASSERT(object);
	}

	// 条件に一致する timeScale を検索する。
	// 後ろから走査することで、あとから実行されたものを優先する。
	for (auto it = _timeScales.rbegin(); it != _timeScales.rend(); ++it)
	{
		auto const & data = *it;
		auto const & t = data.target;

		if (type == t._type || (type == eTYPE_TILE_OR_MENU && (t._type == eTYPE_TILE || t._type == eTYPE_MENU)))
		{
			GameSpeed const * gs = nullptr;
			if (t._type == eTYPE_OBJECT)
			{
				// 対象リストから検索
				auto targetFindFunc = [&]() {
					cocos2d::Ref *ref = nullptr;
					CCARRAY_FOREACH(t._targetObjectList.get(), ref) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
						auto target = static_cast<agtk::Object *>(ref);
#else
						auto target = dynamic_cast<agtk::Object *>(ref);
#endif
						if (target->getId() == object->getId()) {
							gs = &data.gs;
							break;
						}
					}
				};
				
				const std::function<void()> func[] = {
					[&]() {
						if (t._targetObjectGroup == data::ObjectCommandGameSpeedChangeData::kObjectGroupAll || t._targetObjectGroup == object->getObjectData()->getGroup()) {
							gs = &data.gs;
						}
					},
					[&]() {
						if (t._targetQualifierId == data::ObjectCommandData::kQualifierSingle) {//単体
							if (t._targetObjectList->containsObject((cocos2d::Ref *)object)) {
								gs = &data.gs;
							}
						} else if (t._targetQualifierId == data::ObjectCommandData::kQualifierWhole) {//全体
							if (t._targetObjectId == object->getObjectData()->getId()) {
								gs = &data.gs;
							}
						} else {
							if (t._targetObjectId == object->getObjectData()->getId()) {
								auto instanceId = object->getPlayObjectData()->getInstanceId();
								if (t._targetQualifierId == instanceId) {
									gs = &data.gs;
								}
							}
						}
					},
					targetFindFunc,
					targetFindFunc,
					[&]() { 
						gs = &data.gs; 
					},
				};
// #AGTK-NX
#if CC_TARGET_PLATFORM == CC_PLATFORM_NX
#else
				static_assert(ARRAYSIZE(func) == agtk::data::ObjectCommandGameSpeedChangeData::EnumTargettingType::kTargettingMax,"not match array size" );
#endif

				func[t._targetObjectType]();
			}	
			else 
			{// 対象がオブジェクトでない場合
				gs = &data.gs;
			}
			if (gs) {
				return gs->getTimeScale();
			}
		}
	}
	return 1.0f;
}

void SceneGameSpeed::removeObject(agtk::Object* object)
{
	for (auto & elem : _timeScales) {
		if (elem.target._targetObjectList.get()) {
			elem.target._targetObjectList->removeObject(object);
		}
	}
}

#ifdef USE_PREVIEW
void SceneGameSpeed::setPaused(bool pause)
{
	for (auto & elem : _timeScales) {
		elem.gs.setPaused(pause);
	}
}
#endif

SceneGameSpeed::Data* SceneGameSpeed::find(const Target& target)
{
	for (auto & elem : _timeScales) {
		if (elem.target == target) {
			return &elem;
		}
	}
	return nullptr;
}

void SceneGameSpeed::remove(const Data& delete_data)
{
	_timeScales.erase(
		std::remove_if(_timeScales.begin(), _timeScales.end(), [&delete_data](const Data& d) { return d.target == delete_data.target; })
		, _timeScales.end()
		);
}

//-------------------------------------------------------------------------------------------------------------------
SceneShake::SceneShake()
{
	_fadeType = kFadeTypeNone;
	_sceneData = nullptr;
	_objCommand = nullptr;
	_state = kStateIdle;
	_duration = 0.0f;
	_shakeX = nullptr;
	_shakeY = nullptr;
	_flipX = false;
	_flipY = false;
}

SceneShake::~SceneShake()
{
	CC_SAFE_RELEASE_NULL(_sceneData);
	CC_SAFE_RELEASE_NULL(_objCommand);
	CC_SAFE_RELEASE_NULL(_shakeX);
	CC_SAFE_RELEASE_NULL(_shakeY);
}

bool SceneShake::init(agtk::data::SceneData *sceneData)
{
	this->setFadeType(kFadeTypeNone);
	this->setState(kStateIdle);
	this->setSceneData(sceneData);
	auto shakeX = ShakeTimer::create(0);
	if (shakeX == nullptr) {
		CC_ASSERT(0);
		return false;
	}
	this->setShakeX(shakeX);
	auto shakeY = ShakeTimer::create(0);
	if (shakeY == nullptr) {
		CC_ASSERT(0);
		return false;
	}
	this->setShakeY(shakeY);
	return true;
}

void SceneShake::update(float df)
{
	auto shakeX = this->getShakeX();
	auto shakeY = this->getShakeY();

	switch (this->getFadeType()) {
	case kFadeTypeFadeNone: this->updateFadeNone(df); break;
	case kFadeTypeFadeIn: this->updateFadeIn(df); break;
	case kFadeTypeFadeOut: this->updateFadeOut(df); break;
	case kFadeTypeFadeInOut: this->updateFadeInOut(df); break;
	}
	_duration += df;
	shakeX->update(df);
	shakeY->update(df);
	this->setMoveXY(cocos2d::Vec2(shakeX->getValue(), shakeY->getValue()));
}

void SceneShake::updateFadeNone(float df)
{
	auto cmd = this->getObjCommand();
	auto shakeX = this->getShakeX();
	auto shakeY = this->getShakeY();

	switch (this->getState()) {
	case kStateStart: {
		float seconds = ((float)cmd->getInterval300() / 300.0f) * 0.5f;
		float x = cmd->getWidth() + AGTK_RANDOM(0, cmd->getWidthDispersion()) - (float)cmd->getWidthDispersion() * 0.5f;
		float y = cmd->getHeight() + AGTK_RANDOM(0, cmd->getHeightDispersion()) - (float)cmd->getHeightDispersion() * 0.5f;
		x = (_flipX) ? -x : x;
		y = (_flipY) ? -y : y;
		_flipX = !_flipX;
		_flipY = !_flipY;
		shakeX->setValue(x, seconds);
		shakeY->setValue(y, seconds);
		this->setState(kStateShaking);
		break; }
	case kStateShaking: {
		if (shakeX->getState() == agtk::EventTimer::kStateEnd || shakeY->getState() == agtk::EventTimer::kStateEnd) {
			float seconds = (float)cmd->getInterval300() / 300.0f;
			float x = cmd->getWidth() + AGTK_RANDOM(0, cmd->getWidthDispersion()) - (float)cmd->getWidthDispersion() * 0.5f;
			float y = cmd->getHeight() + AGTK_RANDOM(0, cmd->getHeightDispersion()) - (float)cmd->getHeightDispersion() * 0.5f;
			x = (_flipX) ? -x : x;
			y = (_flipY) ? -y : y;
			_flipX = !_flipX;
			_flipY = !_flipY;
			shakeX->setValue(x, seconds);
			shakeY->setValue(y, seconds);
		}
		float duration = (float)cmd->getDuration300() / 300.0;
		if (duration <= _duration) {
			this->setState(kStateEnd);
		}
		break; }
	case kStateEnd:
		this->setObjCommand(nullptr);
		this->setState(kStateIdle);
		this->setFadeType(kFadeTypeNone);
		shakeX->end();
		shakeY->end();
		shakeX->setValue(0);
		shakeY->setValue(0);
		break;
	}
}

void SceneShake::updateFadeIn(float df)
{
	auto cmd = this->getObjCommand();
	auto shakeX = this->getShakeX();
	auto shakeY = this->getShakeY();

	float duration = (float)cmd->getDuration300() / 300.0;
	float shakeStrength = _duration / duration;

	switch (this->getState()) {
	case kStateStart: {
		float seconds = ((float)cmd->getInterval300() / 300.0f) * 0.5f;
		float x = cmd->getWidth() + AGTK_RANDOM(0, cmd->getWidthDispersion()) - (float)cmd->getWidthDispersion() * 0.5f;
		float y = cmd->getHeight() + AGTK_RANDOM(0, cmd->getHeightDispersion()) - (float)cmd->getHeightDispersion() * 0.5f;
		x *= shakeStrength;
		y *= shakeStrength;
		x = (_flipX) ? -x : x;
		y = (_flipY) ? -y : y;
		_flipX = !_flipX;
		_flipY = !_flipY;
		shakeX->setValue(x, seconds);
		shakeY->setValue(y, seconds);
		this->setState(kStateShaking);
		break; }
	case kStateShaking: {
		if (shakeX->getState() == agtk::EventTimer::kStateEnd || shakeY->getState() == agtk::EventTimer::kStateEnd) {
			float seconds = (float)cmd->getInterval300() / 300.0f;
			float x = cmd->getWidth() + AGTK_RANDOM(0, cmd->getWidthDispersion()) - (float)cmd->getWidthDispersion() * 0.5f;
			float y = cmd->getHeight() + AGTK_RANDOM(0, cmd->getHeightDispersion()) - (float)cmd->getHeightDispersion() * 0.5f;
			x *= shakeStrength;
			y *= shakeStrength;
			x = (_flipX) ? -x : x;
			y = (_flipY) ? -y : y;
			_flipX = !_flipX;
			_flipY = !_flipY;
			shakeX->setValue(x, seconds);
			shakeY->setValue(y, seconds);
		}
		if (duration <= _duration) {
			this->setState(kStateEnd);
		}
		break; }
	case kStateEnd:
		this->setObjCommand(nullptr);
		this->setState(kStateIdle);
		this->setFadeType(kFadeTypeNone);
		shakeX->end();
		shakeY->end();
		shakeX->setValue(0);
		shakeY->setValue(0);
		break;
	}
}

void SceneShake::updateFadeOut(float df)
{
	auto cmd = this->getObjCommand();
	auto shakeX = this->getShakeX();
	auto shakeY = this->getShakeY();

	float duration = (float)cmd->getDuration300() / 300.0;
	float shakeStrength = 1.0f - _duration / duration;

	switch (this->getState()) {
	case kStateStart: {
		float seconds = ((float)cmd->getInterval300() / 300.0f) * 0.5f;
		float x = cmd->getWidth() + AGTK_RANDOM(0, cmd->getWidthDispersion()) - (float)cmd->getWidthDispersion() * 0.5f;
		float y = cmd->getHeight() + AGTK_RANDOM(0, cmd->getHeightDispersion()) - (float)cmd->getHeightDispersion() * 0.5f;
		x *= shakeStrength;
		y *= shakeStrength;
		x = (_flipX) ? -x : x;
		y = (_flipY) ? -y : y;
		_flipX = !_flipX;
		_flipY = !_flipY;
		shakeX->setValue(x, seconds);
		shakeY->setValue(y, seconds);
		this->setState(kStateShaking);
		break; }
	case kStateShaking: {
		if (shakeX->getState() == agtk::EventTimer::kStateEnd || shakeY->getState() == agtk::EventTimer::kStateEnd) {
			float seconds = (float)cmd->getInterval300() / 300.0f;
			float x = cmd->getWidth() + AGTK_RANDOM(0, cmd->getWidthDispersion()) - (float)cmd->getWidthDispersion() * 0.5f;
			float y = cmd->getHeight() + AGTK_RANDOM(0, cmd->getHeightDispersion()) - (float)cmd->getHeightDispersion() * 0.5f;
			x *= shakeStrength;
			y *= shakeStrength;
			x = (_flipX) ? -x : x;
			y = (_flipY) ? -y : y;
			_flipX = !_flipX;
			_flipY = !_flipY;
			shakeX->setValue(x, seconds);
			shakeY->setValue(y, seconds);
		}
		if (duration <= _duration) {
			this->setState(kStateEnd);
		}
		break; }
	case kStateEnd:
		this->setObjCommand(nullptr);
		this->setState(kStateIdle);
		this->setFadeType(kFadeTypeNone);
		shakeX->end();
		shakeY->end();
		shakeX->setValue(0);
		shakeY->setValue(0);
		break;
	}
}

void SceneShake::updateFadeInOut(float df)
{
	auto cmd = this->getObjCommand();
	auto shakeX = this->getShakeX();
	auto shakeY = this->getShakeY();

	float duration = (float)cmd->getDuration300() / 300.0;
	float shakeStrength = 0.0f;
	if (_duration / duration < 0.5f) {//フェードイン
		shakeStrength = AGTK_LINEAR_INTERPOLATE(0.0, 1.0, 0.5, _duration / duration);
	}
	else {//フェードアウト
		shakeStrength = AGTK_LINEAR_INTERPOLATE(1.0, 0.0, 0.5, _duration / duration - 0.5);
	}

	switch (this->getState()) {
	case kStateStart: {
		float seconds = ((float)cmd->getInterval300() / 300.0f) * 0.5f;
		float x = cmd->getWidth() + AGTK_RANDOM(0, cmd->getWidthDispersion()) - (float)cmd->getWidthDispersion() * 0.5f;
		float y = cmd->getHeight() + AGTK_RANDOM(0, cmd->getHeightDispersion()) - (float)cmd->getHeightDispersion() * 0.5f;
		x *= shakeStrength;
		y *= shakeStrength;
		x = (_flipX) ? -x : x;
		y = (_flipY) ? -y : y;
		_flipX = !_flipX;
		_flipY = !_flipY;
		shakeX->setValue(x, seconds);
		shakeY->setValue(y, seconds);
		this->setState(kStateShaking);
		break; }
	case kStateShaking: {
		if (shakeX->getState() == agtk::EventTimer::kStateEnd || shakeY->getState() == agtk::EventTimer::kStateEnd) {
			float seconds = (float)cmd->getInterval300() / 300.0f;
			float x = cmd->getWidth() + AGTK_RANDOM(0, cmd->getWidthDispersion()) - (float)cmd->getWidthDispersion() * 0.5f;
			float y = cmd->getHeight() + AGTK_RANDOM(0, cmd->getHeightDispersion()) - (float)cmd->getHeightDispersion() * 0.5f;
			x *= shakeStrength;
			y *= shakeStrength;
			x = (_flipX) ? -x : x;
			y = (_flipY) ? -y : y;
			_flipX = !_flipX;
			_flipY = !_flipY;
			shakeX->setValue(x, seconds);
			shakeY->setValue(y, seconds);
		}
		if (duration <= _duration) {
			this->setState(kStateEnd);
		}
		break; }
	case kStateEnd:
		this->setObjCommand(nullptr);
		this->setState(kStateIdle);
		this->setFadeType(kFadeTypeNone);
		shakeX->end();
		shakeY->end();
		shakeX->setValue(0);
		shakeY->setValue(0);
		break;
	}
}

void SceneShake::start(agtk::data::ObjectCommandSceneShakeData *objCommand)
{
	this->setObjCommand(objCommand);
	if (!objCommand->getFadeIn() && !objCommand->getFadeOut()) {//フェードなし
		this->setFadeType(kFadeTypeFadeNone);
	}
	else if (objCommand->getFadeIn() && !objCommand->getFadeOut()) {//フェードイン
		this->setFadeType(kFadeTypeFadeIn);
	}
	else if (!objCommand->getFadeIn() && objCommand->getFadeOut()) {//フェードアウト
		this->setFadeType(kFadeTypeFadeOut);
	}
	else if (objCommand->getFadeIn() && objCommand->getFadeOut()) {//両方（フェードイン・フェードアウト）
		this->setFadeType(kFadeTypeFadeInOut);
	}
	this->setState(kStateStart);
	this->setMoveXY(cocos2d::Vec2::ZERO);
	_duration = 0.0f;
	_flipX = AGTK_RANDOM(0, 100) > 50 ? true : false;
	_flipY = AGTK_RANDOM(0, 100) > 50 ? true : false;
}

void SceneShake::stop()
{
	this->setState(kStateEnd);
}

//-------------------------------------------------------------------------------------------------------------------
SceneLayer::SceneLayer()
{
	_scene = nullptr;
#ifdef USE_REDUCE_RENDER_TEXTURE
	_tileMapNode = nullptr;
#endif
	_tileMapList = nullptr;
#ifdef USE_REDUCE_RENDER_TEXTURE
	_objectSetNode = nullptr;
	_objectFrontNode = nullptr;
	_additiveParticleNode = nullptr;
	_additiveParticleBacksideNode = nullptr;
#endif
	_objectList = nullptr;
	_createObjectList = nullptr;
	_uncreateObjectList = nullptr;
	_deleteObjectList = nullptr;
	_sceneData = nullptr;
	_groupCollisionDetections = nullptr;
	_commonCollisionDetection = nullptr;
	_groupRoughWallCollisionDetections = nullptr;
	_sceneSpriteList = nullptr;
	_layerData = nullptr;
	_portalObjectList = nullptr;
	_physicsObjectList = nullptr;
	_loopCourseList = nullptr;
	_slopeList = nullptr;
	_renderTexture = nullptr;
	_blendAdditive = false;
	_activeFlg = true;
	_removeShaderList = nullptr;
	_isVisible = true;
	_alphaValue = nullptr;
	_positionValue = nullptr;
	_removeSelfFlag = false;
	_isFirstCollisionCheck = false;
	_groupWallCollisionDetections = nullptr;
	_wallCollisionObject = nullptr;
	_detectWallCollisionFunc = nullptr;
	_wallCollisionInit = false;
	_isShaderColorDarkMask = false;
	_initChildrenCount = 0;
	_menuWorkMargin = -1;
}

SceneLayer::~SceneLayer()
{
#ifdef USE_REDUCE_RENDER_TEXTURE
	CC_SAFE_RELEASE_NULL(_tileMapNode);
#endif
	CC_SAFE_RELEASE_NULL(_tileMapList);
#ifdef USE_REDUCE_RENDER_TEXTURE
	CC_SAFE_RELEASE_NULL(_objectSetNode);
	CC_SAFE_RELEASE_NULL(_objectFrontNode);
	CC_SAFE_RELEASE_NULL(_additiveParticleNode);
	CC_SAFE_RELEASE_NULL(_additiveParticleBacksideNode);
#endif
	if(_physicsObjectList) {
		cocos2d::Ref *ref;
		CCARRAY_FOREACH(_physicsObjectList, ref) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
			auto p = static_cast<agtk::PhysicsBase *>(ref);
#else
			auto p = dynamic_cast<agtk::PhysicsBase *>(ref);
#endif
			p->clear();
		}
		CC_SAFE_RELEASE_NULL(_physicsObjectList);
	}
	CC_SAFE_RELEASE_NULL(_slopeList);
	CC_SAFE_RELEASE_NULL(_loopCourseList);
	CC_SAFE_RELEASE_NULL(_portalObjectList);
	CC_SAFE_RELEASE_NULL(_objectList);
	CC_SAFE_RELEASE_NULL(_createObjectList);
	CC_SAFE_RELEASE_NULL(_uncreateObjectList);
	CC_SAFE_RELEASE_NULL(_deleteObjectList);
	CC_SAFE_RELEASE_NULL(_sceneData);
	CC_SAFE_RELEASE_NULL(_groupCollisionDetections);
	CC_SAFE_RELEASE_NULL(_commonCollisionDetection);
	CC_SAFE_RELEASE_NULL(_groupRoughWallCollisionDetections);
	CC_SAFE_RELEASE_NULL(_groupWallCollisionDetections);
#ifdef USE_MULTITHREAD_OBJECT_UPDATE
	_wallCollisionObject = nullptr;
#else
	CC_SAFE_RELEASE_NULL(_wallCollisionObject);
#endif
	CC_SAFE_RELEASE_NULL(_sceneSpriteList);
	CC_SAFE_RELEASE_NULL(_layerData);
	CC_SAFE_RELEASE_NULL(_renderTexture);
	CC_SAFE_RELEASE_NULL(_removeShaderList);
	CC_SAFE_RELEASE_NULL(_alphaValue);
	CC_SAFE_RELEASE_NULL(_positionValue);

}

SceneLayer *SceneLayer::create(agtk::Scene *scene, agtk::data::SceneData *sceneData, int layerId, bool isPortal, bool bBlendAdditive, int startPointGroupIdx)
{
	auto p = new (std::nothrow) SceneLayer();
	if (p && p->init(scene, sceneData, layerId, isPortal, bBlendAdditive, startPointGroupIdx)) {
		p->autorelease();
		return p;
	}
	CC_SAFE_DELETE(p);
	return nullptr;
}

SceneLayer *SceneLayer::createMenu(agtk::Scene *scene, agtk::data::SceneData *sceneData, int layerId, cocos2d::Size sceneSize, bool bBlendAdditive)
{
	auto p = new (std::nothrow) SceneLayer();
	if (p && p->initMenu(scene, sceneData, layerId, sceneSize, bBlendAdditive)) {
		p->autorelease();
		return p;
	}
	CC_SAFE_DELETE(p);
	return nullptr;
}

bool SceneLayer::createRenderTexture(bool bBlendAdditive, int zOrder)
{
	auto projectData = GameManager::getInstance()->getProjectData();
	//renderTexture
	cocos2d::Size size = projectData->getScreenSize();
//	if (projectData->getMagnifyWindow()) {
//		size.width *= projectData->getWindowMagnification();
//		size.height *= projectData->getWindowMagnification();
//	}
	auto renderTextureCtrl = agtk::RenderTextureCtrl::createForSceneLayer(this, size, zOrder);
	if (renderTextureCtrl == nullptr) {
		return false;
	}
	this->addChild(renderTextureCtrl);
	this->setRenderTexture(renderTextureCtrl);

	_blendAdditiveFlag = bBlendAdditive;
	if (bBlendAdditive && renderTextureCtrl->getLastRenderTexture()) {//加算
		this->setBlendAdditiveRenderTextureSprite();
	}
	return true;
}

/**
* シーンレイヤーの初期化
* @param	scene								親シーンのインスタンス
* @param	sceneData							シーンデータ
* @param	layerId								シーンレイヤーID
* @param	isIgnoreCreateSameStartPointObj		同一のスタートポイントのオブジェクト生成を行わないフラグ
* @param	bBlendAdditive						加算合成タイプのレイヤーか？
* @param	startPointGroupIdx					スタートポイントグループIDX
* @return	初期化の可否
*/
bool SceneLayer::init(agtk::Scene *scene, agtk::data::SceneData *sceneData, int layerId, bool isIgnoreCreateSameStartPointObj, bool bBlendAdditive, int startPointGroupIdx)
{
	if (!cocos2d::Node::init()) {
		return false;
	}
	if (scene == nullptr) {
		CC_ASSERT(0);
		return false;
	}
	_scene = scene;
	
	auto sceneSize = scene->getSceneSize();

	this->setType(kTypeScene);
	this->setSceneData(sceneData);
	this->setContentSize(cocos2d::Size(sceneSize));
	this->setLayerId(layerId);
	auto layerData = sceneData->getLayer(layerId);
	this->setLayerData(layerData);
	this->setBlendAdditive(bBlendAdditive);
	this->setIsVisible(true);

	cocos2d::__Array *physicsObjectList = cocos2d::__Array::create();
	this->setPhysicsObjectList(physicsObjectList);

	// 子供の初期化
	initChildren();

	// タイルマップ初期化
	initTileMapList(sceneData, layerData, false);

	// オブジェクト初期化
	initObject(sceneData, layerId, false, isIgnoreCreateSameStartPointObj, startPointGroupIdx);

	//sprite list
	auto sceneSpriteList = cocos2d::__Array::create();
	if (sceneSpriteList == nullptr) {
		CC_ASSERT(0);
		return false;
	}
	this->setSceneSpriteList(sceneSpriteList);

	//setup render texture
	int zOrder = (sceneData->getLayerList()->count() + 2) - (layerId == agtk::data::SceneData::kHudMenuLayerId ? sceneData->getLayerList()->count() : layerId);
	if (!createRenderTexture(bBlendAdditive, zOrder)) {
		return false;
	}
	this->setRemoveShaderList(cocos2d::__Array::create());

	this->setAlphaValue(agtk::TimerFloat::create(255.0f));
	this->setPositionValue(agtk::TimerVec2::create(cocos2d::Vec2::ZERO));

	// 初回のみの事前衝突判定のフラグを立てる
	this->_isFirstCollisionCheck = true;

	return true;
}
/**
* メニューレイヤーの初期化
* @param	scene								親シーンのインスタンス
* @param	sceneData							シーンデータ
* @param	layerId								シーンレイヤーID
* @param	sceneSize							シーンサイズ
* @param	bBlendAdditive						加算合成タイプのレイヤーか？
* @return	初期化の可否
*/
bool SceneLayer::initMenu(agtk::Scene *scene, agtk::data::SceneData *sceneData, int layerId, cocos2d::Size sceneSize, bool bBlendAdditive)
{
	if (!cocos2d::Node::init()) {
		return false;
	}
	if (scene == nullptr) {
		CC_ASSERT(0);
		return false;
	}
	_scene = scene;
	this->setType(kTypeMenu);
	this->setSceneData(sceneData);
	this->setContentSize(cocos2d::Size(sceneSize));
	this->setLayerId(layerId);
	auto layerData = sceneData->getLayer(layerId);
	this->setLayerData(layerData);
	this->setBlendAdditive(bBlendAdditive);
	this->setIsVisible(true);

	cocos2d::__Array *physicsObjectList = cocos2d::__Array::create();
	this->setPhysicsObjectList(physicsObjectList);

	// 子供の初期化
	initChildren();

	// タイルマップ初期化
	initTileMapList(sceneData, layerData, true, &sceneSize);

	// オブジェクト初期化
	initObject(sceneData, layerId, true);

	//sprite list
	auto sceneSpriteList = cocos2d::__Array::create();
	if (sceneSpriteList == nullptr) {
		CC_ASSERT(0);
		return false;
	}
	this->setSceneSpriteList(sceneSpriteList);

	this->setRemoveShaderList(cocos2d::__Array::create());

	this->setAlphaValue(agtk::TimerFloat::create(255.0f));
	this->setPositionValue(agtk::TimerVec2::create(cocos2d::Vec2::ZERO));
	return true;
}

/**
* 初期化時に必要なNodeを追加する、また、初期時の子供の数をセットする関数
* ※CameraFlag::USER○のvisibleの切り替え処理を子供の数で判定しているため(Menuのみ)
*/
void SceneLayer::initChildren()
{
	// メモ : Menuの場合は初期時の子供数は「9」


	// 各オブジェクトの初期化
	{
		// タイルマップ初期化
#ifdef USE_REDUCE_RENDER_TEXTURE
		auto tileMapNode = cocos2d::Node::create();
		this->addChild(tileMapNode);
		setTileMapNode(tileMapNode);
#endif

		// オブジェクト初期化
#ifdef USE_REDUCE_RENDER_TEXTURE
		auto additiveParticleBacksideNode = cocos2d::Node::create();
		this->addChild(additiveParticleBacksideNode, 1);
		this->setAdditiveParticleBacksideNode(additiveParticleBacksideNode);
		auto objectSetNode = changeRunningNode::create();
		this->addChild(objectSetNode, 1);
		this->setObjectSetNode(objectSetNode);
		auto objectFrontNode = cocos2d::Node::create();
		this->addChild(objectFrontNode, 1);
		this->setObjectFrontNode(objectFrontNode);
		auto additiveParticleNode = cocos2d::Node::create();
		this->addChild(additiveParticleNode, 1);
		this->setAdditiveParticleNode(additiveParticleNode);
#endif
	}

	// 現在の子供の数を取得
	_initChildrenCount = getCountChildrenRecursively(this);
}

/**
* タイルマップ初期化
* @param	sceneData								シーンデータ
* @param	layerData								レイヤーデータ
* @param	isMemuLayer								メニューレイヤーか？
* @param	memuSize								メニューレイヤーサイズ
* @return	初期化の可否
*/
bool SceneLayer::initTileMapList(agtk::data::SceneData *sceneData, agtk::data::LayerData *layerData, bool isMemuLayer, cocos2d::Size *memuSize)
{
	auto sceneSize = _scene->getSceneSize();

	// タイルマップリスト
	auto tileMapList = cocos2d::Array::create();
	auto tileMap = TileMap::create(layerData->getTileList(), this);
#ifdef USE_REDUCE_RENDER_TEXTURE
	auto tileMapNode = getTileMapNode();
	tileMapNode->addChild(tileMap);
#else
	this->addChild(tileMap, 0, "tileMap");
#endif
	tileMapList->addObject(tileMap);

	// 初期時の子供数を加算(行動範囲のサイズ + tileMapNodeの対してのaddChild()分)
	_initChildrenCount += tileMap->getLimitTileList()->count() + 1;

	// メニューレイヤーでない場合はシーンの上下左右を繋げる設定を行う
	if (!isMemuLayer) {
		// シーンの上下を繋げる場合
		if (sceneData->getVerticalLoop()) {
			// 上下に同じタイルマップを生成する
			float posYArr[] = { sceneSize.y, -sceneSize.y };
			for (float y : posYArr) {
				auto tileMap2 = TileMap::create(layerData->getTileList(), this);
				tileMap2->setPosition(Vec2(0, y));
#ifdef USE_REDUCE_RENDER_TEXTURE
				tileMapNode->addChild(tileMap2);
#else
				this->addChild(tileMap2, 0, "tileMap");
#endif
				tileMapList->addObject(tileMap2);

				// タイルマップの表示は停止させておく
				tileMap2->setVisible(false);
			}
		}

		// シーンの左右を繋げる場合
		if (sceneData->getHorizontalLoop()) {
			// 左右に同じタイルマップを生成する
			float posXArr[] = { -sceneSize.x, sceneSize.x };
			for (float x : posXArr) {
				auto tileMap2 = TileMap::create(layerData->getTileList(), this);
				tileMap2->setPosition(Vec2(x, 0));
#ifdef USE_REDUCE_RENDER_TEXTURE
				tileMapNode->addChild(tileMap2);
#else
				this->addChild(tileMap2, 0, "tileMap");
#endif
				tileMapList->addObject(tileMap2);

				// タイルマップの表示は停止させておく
				tileMap2->setVisible(false);
			}
		}

		// シーンの上下左右を繋げる場合
		if (sceneData->getVerticalLoop() && sceneData->getHorizontalLoop()) {
			// 左上、左下、右上、右下に同じタイルマップを生成する
			Vec2 posArr[] = {
				Vec2(-sceneSize.x, sceneSize.y),
				Vec2(-sceneSize.x, -sceneSize.y),
				Vec2(sceneSize.x, sceneSize.y),
				Vec2(sceneSize.x, -sceneSize.y)
			};

			for (Vec2 pos : posArr) {
				auto tileMap2 = TileMap::create(layerData->getTileList(), this);
				tileMap2->setPosition(pos);
#ifdef USE_REDUCE_RENDER_TEXTURE
				tileMapNode->addChild(tileMap2);
#else
				this->addChild(tileMap2, 0, "tileMap");
#endif
				tileMapList->addObject(tileMap2);

				// タイルマップの表示は停止させておく
				tileMap2->setVisible(false);
			}
		}
	}
	this->setTileMapList(tileMapList);

	// タイルマップ初期化
	{
		auto projectData = GameManager::getInstance()->getProjectData();
		int tileWidth = projectData->getTileWidth();
		int tileHeight = projectData->getTileWidth();
		int sceneHorzTileCount = 0;
		int sceneVertTileCount = 0;
		// メニューレイヤー処理
		if (isMemuLayer) {
			sceneHorzTileCount = ceil(memuSize->width / tileWidth);
			sceneVertTileCount = ceil(memuSize->height / tileHeight);
		} 
		// シーンレイヤー処理
		else {
			sceneHorzTileCount = ceil(sceneSize.x / tileWidth);
			sceneVertTileCount = ceil(sceneSize.y / tileHeight);
		}
		if (this->getTileMapList()) {
			cocos2d::Ref *ref = nullptr;
			CCARRAY_FOREACH(this->getTileMapList(), ref) {
				auto tileMap = dynamic_cast<agtk::TileMap*>(ref);
				if (tileMap) {
					tileMap->initTileMap(sceneHorzTileCount, sceneVertTileCount);
				}
			}
		}
	}

	return true;
}

/**
* オブジェクト初期化
* @param	sceneData							シーンデータ
* @param	layerId								シーンレイヤーID
* @param	isMemuLayer							メニューレイヤーか？
* @param	memuSize							メニューレイヤーサイズ
* @param	isIgnoreCreateSameStartPointObj		同一のスタートポイントのオブジェクト生成を行わないフラグ
* @param	startPointGroupIdx					スタートポイントグループIDX
* @return	初期化の可否
*/
bool SceneLayer::initObject(agtk::data::SceneData *sceneData, int layerId, bool isMemuLayer, bool isIgnoreCreateSameStartPointObj, int startPointGroupIdx)
{
	//objectList
	auto sceneId = this->getSceneData()->getId();
	initObjectList(sceneData, layerId, sceneId, isMemuLayer, isIgnoreCreateSameStartPointObj, startPointGroupIdx);

	// portalObject
	cocos2d::__Array *portalObjList = cocos2d::__Array::create();
	// ポータルのデータチェック
	// シーンレイヤー処理
	if (!isMemuLayer)
	{
		auto projectData = GameManager::getInstance()->getProjectData();
		cocos2d::__Array *portalDataList = cocos2d::__Array::create();
		//cocos2d::__Array *portalObjList = cocos2d::__Array::create();
		projectData->getPortalDataList(projectData->getTransitionPortalList(), sceneId, layerId, portalDataList);

		// ポータルデータリストがある場合
		if (portalDataList->count() > 0) {
			cocos2d::Ref *ref = nullptr;
			int portalIdx = 0;
			CCARRAY_FOREACH(portalDataList, ref) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
				auto data = static_cast<agtk::data::TransitionPortalData *>(ref);
#else
				auto data = dynamic_cast<agtk::data::TransitionPortalData *>(ref);
#endif
				auto areaSettingDataList = data->getAreaSettingList();

				// ポータルAとポータルBのデータチェック
				for (int i = 0; i < agtk::data::TransitionPortalData::EnumPortalType::MAX; i++) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
					auto areaSettingData = static_cast<agtk::data::AreaSettingData *>(areaSettingDataList->getObjectAtIndex(i));
#else
					auto areaSettingData = dynamic_cast<agtk::data::AreaSettingData *>(areaSettingDataList->getObjectAtIndex(i));
#endif

					// エリア設定データでシーンIDとシーンレイヤーIDが一致する場合
					if (areaSettingData->getSceneId() == sceneId && areaSettingData->getLayerIndex() == layerId - 1) {

						// ポータルオブジェクトを生成
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
						auto moveSettingData = static_cast<agtk::data::MoveSettingData *>(data->getMoveSettingList()->getObjectAtIndex(i));
#else
						auto moveSettingData = dynamic_cast<agtk::data::MoveSettingData *>(data->getMoveSettingList()->getObjectAtIndex(i));
#endif
						auto portal = Portal::create(data->getId(), portalIdx++, i, areaSettingDataList, data->getMovableList(), moveSettingData, sceneData);

						portalObjList->addObject(portal);
						this->addChild(portal);
					}
				}
			}
		}
	}
	this->setPortalObjectList(portalObjList);

	// 「坂」「360度ループ」生成
	cocos2d::__Array *slopeList = cocos2d::__Array::create();
	cocos2d::__Array *loopCourseList = cocos2d::__Array::create();
	// シーンレイヤー処理
	auto scenePartOthersList = sceneData->getScenePartOthersList(layerId - 1);

	if (scenePartOthersList->count() > 0)
	{
		cocos2d::Ref *ref = nullptr;
		CCARRAY_FOREACH(scenePartOthersList, ref)
		{
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
			auto part = static_cast<agtk::data::ScenePartOthersData*>(ref);
#else
			auto part = dynamic_cast<agtk::data::ScenePartOthersData*>(ref);
#endif

			// データが「坂」の情報の場合、坂を生成する
			if (part->getOthersType() == agtk::data::ScenePartOthersData::kOthersSlope)
			{
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
				auto slope = Slope::create(static_cast<agtk::data::OthersSlopeData*>(part->getPart()), this->getLayerId(), sceneData);
#else
				auto slope = Slope::create(dynamic_cast<agtk::data::OthersSlopeData*>(part->getPart()), this->getLayerId(), sceneData);
#endif
				slopeList->addObject(slope);
				this->addChild(slope);
			}
			// データが「360度ループ」の情報の場合、360度ループを生成する
			else if (part->getOthersType() == agtk::data::ScenePartOthersData::kOthersLoop) {
				auto course = OthersLoopCourse::create(
					part->getId(),
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
					static_cast<agtk::data::OthersLoopData*>(part->getPart()),
#else
					dynamic_cast<agtk::data::OthersLoopData*>(part->getPart()),
#endif
					sceneData
				);
				loopCourseList->addObject(course);
				this->addChild(course);
			}
		}
	}
	this->setSlopeList(slopeList);
	this->setLoopCourseList(loopCourseList);

	// 坂の接続情報を設定
	// シーンレイヤー処理
	if (!isMemuLayer) {
		if (this->getSlopeList()->count() > 1) {
			cocos2d::Ref* ref = nullptr;
			CCARRAY_FOREACH(this->getSlopeList(), ref) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
				auto slope = static_cast<agtk::Slope*>(ref);
#else
				auto slope = dynamic_cast<agtk::Slope*>(ref);
#endif
				slope->checkConnect(this->getSlopeList());
			}
		}
	}

	//etc.
	createGroupCollisionDetections(true);
	createGroupRoughWallCollisionDetections();
	createGroupWallCollisionDetections();

	// オブジェクトが存在する場合
	if (this->getObjectList()->count() > 0) {
		// オブジェクトに衝突判定コンポーネントを追加
		// ついでにオブジェクトに付随する物理オブジェクトを生成
		cocos2d::Ref *ref = nullptr;
		CCARRAY_FOREACH(this->getObjectList(), ref) {
			auto object = dynamic_cast<agtk::Object *>(ref);
			if (object) {
				this->addCollisionDetaction(object);
				// オブジェクトに紐付いた物理オブジェクトの生成
				this->createPhysicsObjectWithObject(object);
			}
		}
	}

	// ポータルオブジェクトに衝突判定コンポーネントを追加
	// シーンレイヤー処理
	if (!isMemuLayer) {
		// ポータルオブジェクトが存在する場合
		if (this->getPortalObjectList()->count() > 0) {
			auto collision = this->getCommonCollisionDetection();
			cocos2d::Ref *ref = nullptr;
			CCARRAY_FOREACH(this->getPortalObjectList(), ref) {
				auto portal = dynamic_cast<agtk::Portal *>(ref);
				if (portal) {
					auto collisionComponent = CollisionComponent::create(collision, CollisionComponent::kGroupPortal);
					portal->addComponent(collisionComponent);
					portal->unscheduleUpdate();//※addComponetでupdateが呼ばれるので、ループ一元化のためここで破棄する。
				}
			}
		}
	}

	//physics object
	this->createPhysicsObject(sceneData);

	return true;
}

/**
* オブジェクトリスト初期化
* @param	sceneData							シーンデータ
* @param	layerId								シーンレイヤーID
* @param	sceneId								シーンID
* @param	isMemuLayer							メニューレイヤーか？
* @param	memuSize							メニューレイヤーサイズ
* @param	isIgnoreCreateSameStartPointObj		同一のスタートポイントのオブジェクト生成を行わないフラグ
* @param	startPointGroupIdx					スタートポイントグループIDX
* @return	初期化の可否
*/
bool SceneLayer::initObjectList(agtk::data::SceneData *sceneData, int layerId, int sceneId, bool isMemuLayer, bool isIgnoreCreateSameStartPointObj, int startPointGroupIdx)
{
	CC_ASSERT(layerId - 1 >= 0);
	auto scenePartList = sceneData->getScenePartObjectList(layerId - 1);
	objectId = 0;
	auto initcreateObjectList = cocos2d::__Array::create();		//初期生成されるオブジェクトリスト
	auto createObjectList = cocos2d::__Array::create();
	auto uncreateObjList = cocos2d::__Array::create();			//条件が満たされず生成されなかった初期生成オブジェクトリスト
	auto deleteObjList = cocos2d::__Array::create();			//削除済みオブジェクトリスト
#ifdef USE_REDUCE_RENDER_TEXTURE
	auto objectSetNode = getObjectSetNode();
	auto objectFrontNode = getObjectFrontNode();
	auto additiveParticleNode = getAdditiveParticleNode();
	auto additiveParticleBacksideNode = getAdditiveParticleBacksideNode();
#endif

	if (scenePartList->count()) {
		class StartPointObjectData : public cocos2d::Ref {
		private:
			StartPointObjectData() {
				_data = nullptr;
				_objectId = nullptr;
				_playerId = -1;
				_scenePartObjectDataIdx = 0;
			}
			virtual ~StartPointObjectData() {
				CC_SAFE_RELEASE_NULL(_data);
				CC_SAFE_RELEASE_NULL(_objectId);
			};
		public:
			CREATE_FUNC_PARAM2(StartPointObjectData, agtk::data::ScenePartObjectData *, data, cocos2d::Integer *, id);
		private:
			virtual bool init(agtk::data::ScenePartObjectData *data, cocos2d::Integer *id) {
				this->setScenePartObjectData(data);
				this->setObjectId(id);
				return true;
			}
		private:
			CC_SYNTHESIZE_RETAIN(agtk::data::ScenePartObjectData *, _data, ScenePartObjectData);
			CC_SYNTHESIZE_RETAIN(cocos2d::Integer *, _objectId, ObjectId);
			CC_SYNTHESIZE(int, _playerId, PlayerId);
			CC_SYNTHESIZE(int, _scenePartObjectDataIdx, ScenePartObjectDataIdx);
		};

		{
			auto projectData = GameManager::getInstance()->getProjectData();
			auto projectPlayData = GameManager::getInstance()->getPlayData();

			cocos2d::Ref *ref = nullptr;
			int scenePartObjectDataIdx = 0;
			auto selectStartPointObjectList = cocos2d::__Dictionary::create();
			auto removeList = cocos2d::__Array::create();

			// スタートポイントオブジェクト関連
			// メニューレイヤー処理
			if (isMemuLayer) {
				auto startPointObjectList = cocos2d::__Dictionary::create();
				int playerCount = projectData->getPlayerCount();
				for (int i = 0; i < playerCount; i++) {
					auto arr = cocos2d::__Array::create();
					startPointObjectList->setObject(arr, i);
				}

				//スタートポイントオブジェクトを抽出する。
				//cocos2d::Ref *ref = nullptr;			// ネストの外で使用したいので移動
				//int scenePartObjectDataIdx = 0;		// ネストの外で使用したいので移動
				CCARRAY_FOREACH(scenePartList, ref) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
					auto scenePartObjectData = static_cast<agtk::data::ScenePartObjectData *>(ref);
#else
					auto scenePartObjectData = dynamic_cast<agtk::data::ScenePartObjectData *>(ref);
#endif
					// スタートポイントオブジェクトでない場合
					if (scenePartObjectData->isStartPointObjectData() == false) {
						scenePartObjectDataIdx++;
						continue;
					}

					//				//ポータル移動でスタートポイントオブジェクトの持越しがある場合
					//				if (isPortal) {
					//					std::function<bool(agtk::data::ScenePartObjectData *)> checkPortalTouchedPlayer = [&](agtk::data::ScenePartObjectData *scenePartObjectData) {
					//						auto gameManager = GameManager::getInstance();
					//						auto portalTouchedPlayerList = gameManager->getPortalTouchedPlayerList();
					//						cocos2d::Ref *ref;
					//						CCARRAY_FOREACH(portalTouchedPlayerList, ref) {
					//							auto p = dynamic_cast<agtk::PortalTouchedData *>(ref);
					//							if (p->getObject()->getScenePartObjectData() == scenePartObjectData) {
					//								return true;
					//							}
					//						}
					//						return false;
					//					};
					//					if (checkPortalTouchedPlayer(scenePartObjectData)) {
					//						//持越しがあるので、ここでは生成しない。
					//						continue;
					//					}
					//				}

					//				// プレイヤーの数だけplayerIdをインクリメント
					//				for (int playerId = 0; playerId < playerCount; playerId++) {
					//
					//					// 該当プレイヤーかつ該当スタートポイントグループの場合
					//					if (scenePartObjectData->getStartPointPlayerBit() & (1 << playerId) && scenePartObjectData->getStartPointGroupIndex() == startPointGroupIdx) {
					//						// プロジェクト共通変数から各プレイヤー毎のオブジェクトIDを取得
					//						auto playVariableData = projectPlayData->getVariableData(agtk::data::PlayData::COMMON_PLAY_DATA_ID, agtk::data::kProjectSystemVariable1PCharacter + playerId);
					//						auto objectId = cocos2d::Integer::create((int)playVariableData->getValue());
					//						if (objectId->getValue() == -1.0f) {
					//							//オブジェクトIDが-1の場合は「プレイヤーキャラクター管理」で設定した一番上の情報を使う。
					//							auto playerCharacterList = projectData->getPlayerCharacterData()->getPlayerCharacterLine(0);
					//							objectId = dynamic_cast<cocos2d::Integer *>(playerCharacterList->getObjectAtIndex(playerId));
					//						}
					//						auto arr = dynamic_cast<cocos2d::__Array *>(startPointObjectList->objectForKey(playerId));
					//						auto startPointObjectData = StartPointObjectData::create(scenePartObjectData, objectId);
					//						startPointObjectData->setPlayerId(playerId + 1);
					//						startPointObjectData->setScenePartObjectDataIdx(scenePartObjectDataIdx);
					//						arr->addObject(startPointObjectData);
					//					}
					//
					//				}
					scenePartObjectDataIdx++;
				}
				//スタートポイントで指定したオブジェクトを割り当てる。
				//auto selectStartPointObjectList = cocos2d::__Dictionary::create();		// ネストの外で使用したいので移動
				//auto removeList = cocos2d::__Array::create();								// ネストの外で使用したいので移動
				cocos2d::DictElement *el;
				CCDICT_FOREACH(startPointObjectList, el) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
					auto dataList = static_cast<cocos2d::__Array *>(el->getObject());
#else
					auto dataList = dynamic_cast<cocos2d::__Array *>(el->getObject());
#endif
					if (dataList->count() == 0) {
						continue;
					}
					if (removeList->count() > 0) {
						cocos2d::Ref *ref;
						auto rmList = cocos2d::__Array::create();
						CCARRAY_FOREACH(dataList, ref) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
							auto p1 = static_cast<StartPointObjectData *>(ref);
#else
							auto p1 = dynamic_cast<StartPointObjectData *>(ref);
#endif
							cocos2d::Ref *ref2;
							CCARRAY_FOREACH(removeList, ref2) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
								auto p2 = static_cast<StartPointObjectData *>(ref2);
#else
								auto p2 = dynamic_cast<StartPointObjectData *>(ref2);
#endif
								if (p1->getScenePartObjectData() == p2->getScenePartObjectData()) {
									rmList->addObject(p1);
								}
							}
						}
						dataList->removeObjectsInArray(rmList);
					}
					if (dataList->count() == 0) {
						continue;
					}

					int pid = el->getIntKey();
					int rnum = AGTK_RANDOM(0, dataList->count() - 1);
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
					auto startPointObjectData = static_cast<StartPointObjectData *>(dataList->getObjectAtIndex(rnum));
#else
					auto startPointObjectData = dynamic_cast<StartPointObjectData *>(dataList->getObjectAtIndex(rnum));
#endif
					selectStartPointObjectList->setObject(startPointObjectData, startPointObjectData->getScenePartObjectDataIdx());

					removeList->addObject(startPointObjectData);
				}
			}
			// シーンレイヤー処理
			else {
				std::vector<cocos2d::__Array *> startPointObjectList;
				int playerCount = projectData->getPlayerCount();
				for (int i = 0; i < playerCount; i++) {
					auto arr = cocos2d::__Array::create();
					startPointObjectList.push_back(arr);
				}

				//スタートポイントオブジェクトを抽出する。
				//cocos2d::Ref *ref = nullptr;			// ネストの外で使用したいので移動
				//int scenePartObjectDataIdx = 0;		// ネストの外で使用したいので移動
				CCARRAY_FOREACH(scenePartList, ref) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
					auto scenePartObjectData = static_cast<agtk::data::ScenePartObjectData *>(ref);
#else
					auto scenePartObjectData = dynamic_cast<agtk::data::ScenePartObjectData *>(ref);
#endif
					// スタートポイントオブジェクトでない場合
					if (scenePartObjectData->isStartPointObjectData() == false) {
						scenePartObjectDataIdx++;
						continue;
					}

					// スタートポイントによるオブジェクト生成回避フラグ
					bool isSkipCreateStartPointObj = false;

					// プレイヤーの数だけplayerIdをインクリメント
					for (int playerId = 0; playerId < playerCount; playerId++) {

						// 該当プレイヤーかつ該当スタートポイントグループの場合
						if (scenePartObjectData->getStartPointPlayerBit() & (1 << playerId) && scenePartObjectData->getStartPointGroupIndex() == startPointGroupIdx) {
							// プロジェクト共通変数から各プレイヤー毎のオブジェクトIDを取得
							auto playVariableData = projectPlayData->getVariableData(agtk::data::PlayData::COMMON_PLAY_DATA_ID, agtk::data::kProjectSystemVariable1PCharacter + playerId);
							auto objectId = cocos2d::Integer::create((int)playVariableData->getValue());

							auto objectData = projectData->getObjectData(objectId->getValue());
							if (objectId->getValue() == -1.0f || objectData == nullptr) {
								//オブジェクトIDが-1の場合は「プレイヤーキャラクター管理」で設定した一番上の情報を使う。
								auto playerCharacterList = projectData->getPlayerCharacterData()->getPlayerCharacterLine(0);
								objectId = dynamic_cast<cocos2d::Integer *>(playerCharacterList->getObjectAtIndex(playerId));
							}

							if (objectId->getValue() != -1.0f) {
								// 同一のスタートポイントオブジェクト生成除外がONの場合
								if (isIgnoreCreateSameStartPointObj) {
									// ゲームマネージャに登録された除外対象と一致するかチェック
									auto list = GameManager::getInstance()->getIgnoreCreatePlayerList();
									cocos2d::Ref *ref = nullptr;
									CCARRAY_FOREACH(list, ref) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
										auto ignoreData = static_cast<GameManager::IgnoreCreateObjectData *>(ref);
#else
										auto ignoreData = dynamic_cast<GameManager::IgnoreCreateObjectData *>(ref);
#endif
										if (playerId + 1 == ignoreData->getPlayerId() && objectId->getValue() == ignoreData->getObjectId()) {
											isSkipCreateStartPointObj = true;
											break;
										}
									}
									if (isSkipCreateStartPointObj) {
										break;
									}
								}

								auto arr = startPointObjectList[playerId];
								auto startPointObjectData = StartPointObjectData::create(scenePartObjectData, objectId);
								startPointObjectData->setPlayerId(playerId + 1);
								startPointObjectData->setScenePartObjectDataIdx(scenePartObjectDataIdx);
								arr->addObject(startPointObjectData);

								//Resources共通の○PオブジェクトにオブジェクトIDを設定する。
								playVariableData->setValue((double)objectId->getValue());
							}
						}
					}

					if (!isSkipCreateStartPointObj) {
						scenePartObjectDataIdx++;
					}
				}

				//スタートポイントで指定したオブジェクトを割り当てる。
				//auto selectStartPointObjectList = cocos2d::__Dictionary::create();		// ネストの外で使用したいので移動
				//auto removeList = cocos2d::__Array::create();								// ネストの外で使用したいので移動

				std::random_shuffle(std::begin(startPointObjectList), std::end(startPointObjectList));

				for (auto dataList : startPointObjectList) {
					if (dataList->count() == 0) {
						continue;
					}
					if (removeList->count() > 0) {
						cocos2d::Ref *ref;
						auto rmList = cocos2d::__Array::create();
						CCARRAY_FOREACH(dataList, ref) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
							auto p1 = static_cast<StartPointObjectData *>(ref);
#else
							auto p1 = dynamic_cast<StartPointObjectData *>(ref);
#endif
							cocos2d::Ref *ref2;
							CCARRAY_FOREACH(removeList, ref2) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
								auto p2 = static_cast<StartPointObjectData *>(ref2);
#else
								auto p2 = dynamic_cast<StartPointObjectData *>(ref2);
#endif
								if (p1->getScenePartObjectData() == p2->getScenePartObjectData()) {
									rmList->addObject(p1);
								}
							}
						}
						dataList->removeObjectsInArray(rmList);
					}
					if (dataList->count() == 0) {
						continue;
					}

					int rnum = AGTK_RANDOM(0, dataList->count() - 1);
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
					auto startPointObjectData = static_cast<StartPointObjectData *>(dataList->getObjectAtIndex(rnum));
#else
					auto startPointObjectData = dynamic_cast<StartPointObjectData *>(dataList->getObjectAtIndex(rnum));
#endif
					selectStartPointObjectList->setObject(startPointObjectData, startPointObjectData->getScenePartObjectDataIdx());

					removeList->addObject(startPointObjectData);
				}
			}


			//オブジェクトを生成する。
			scenePartObjectDataIdx = 0;
			CCARRAY_FOREACH(scenePartList, ref) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
				auto scenePartData = static_cast<agtk::data::ScenePartObjectData *>(ref);
#else
				auto scenePartData = dynamic_cast<agtk::data::ScenePartObjectData *>(ref);
#endif
				//ポータル移動で、移動元から移動元へ戻った場合。
				// シーンレイヤー処理
				if (!isMemuLayer) {
					bool bPortalTouchedPlayer = false;
					auto gameManager = GameManager::getInstance();
					auto portalTouchedPlayerList = gameManager->getPortalTouchedPlayerList();
					cocos2d::Ref *ref;
					CCARRAY_FOREACH(portalTouchedPlayerList, ref) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
						auto p = static_cast<PortalTouchedData *>(ref);
#else
						auto p = dynamic_cast<PortalTouchedData *>(ref);
#endif
						auto object = p->getObject();
						auto _scenePartObjectData = object->getScenePartObjectData();
						if (_scenePartObjectData == scenePartData) {
							bPortalTouchedPlayer = true;
							break;
						}
					}
					if (bPortalTouchedPlayer) {
						scenePartObjectDataIdx++;
						continue;
					}
				}

				//スタートポイントオブジェクト生成。
				auto startPointObjectData = dynamic_cast<StartPointObjectData *>(selectStartPointObjectList->objectForKey(scenePartObjectDataIdx));
				if (startPointObjectData != nullptr) {
					CC_ASSERT(startPointObjectData->getScenePartObjectData() == scenePartData);
#if defined(USE_RUNTIME)
					if (!projectData->getObjectData(startPointObjectData->getObjectId()->getValue())->getTestplayOnly()) {
#endif
						// ACT2-6205 オブジェクトの初期化
						auto scene = GameManager::getInstance()->getCurrentScene();
						if (nullptr != scene && scene->getRequestSwitchInit()) {
							startPointObjectData->getScenePartObjectData()->setX(scene->getObjectPlayer()->getFootPosition().x);
							startPointObjectData->getScenePartObjectData()->setY(scene->getObjectPlayer()->getFootPosition().y);
						}
						
						auto object = agtk::Object::createWithSceneDataAndScenePartObjectData(this, startPointObjectData->getScenePartObjectData(), startPointObjectData->getObjectId()->getValue());
						object->setId(objectId++);
						object->setLayerId(layerId);
						object->setScenePartsId(startPointObjectData->getScenePartObjectData()->getId());
						object->setPhysicsBitMask(layerId, sceneData->getId());
						object->getPlayObjectData()->setPlayerId(startPointObjectData->getPlayerId());
						initcreateObjectList->addObject(object);
#ifdef USE_REDUCE_RENDER_TEXTURE
						objectSetNode->addChild(object);
#else
						this->addChild(object);
#endif
						// 共通変数のインスタンスID設定
						// シーンレイヤー処理
						if (!isMemuLayer) {
							int playerId = startPointObjectData->getPlayerId() - 1;
							if (playerId >= 0) {
								auto playVariableData = projectPlayData->getVariableData(agtk::data::PlayData::COMMON_PLAY_DATA_ID, agtk::data::kProjectSystemVariable1PInstance + playerId);
								playVariableData->setValue((double)object->getInstanceId());

								if (checkStartPointObjectTakeoverStatesAsScene(startPointObjectData->getScenePartObjectData())) {
									auto data = getStartPointObjectTakeoverStatesAsScene(startPointObjectData->getScenePartObjectData());
									object->setPlayObjectData(data->getPlayObjectData());
								}
							}
						}
#if defined(USE_RUNTIME)
					}
#endif
				}
				else {	// オブジェクト生成（スタートポイントオブジェクト以外）
					if (scenePartData->isStartPointObjectData()) {
						scenePartObjectDataIdx++;
						continue;
					}
					// 復活不可のオブジェクトの場合
					if (checkNotReappearObject(scenePartData)) {
						// オブジェクトを生成しない
						scenePartObjectDataIdx++;
						continue;
					}
					//「アクションで復活」オブジェクトの場合
					if (checkCommandReappearObject(scenePartData)) {
#if 0	// ACT2-5053 「アクションで復活」オブジェクトはシーン切り替えで生成しない。
						auto objectData = projectData->getObjectData(scenePartData->getObjectId());
						CC_ASSERT(objectData);
						if (objectData->getAppearCondition() == agtk::data::ObjectData::EnumAppearCondition::kAppearConditionCameraNear) {
							// 未生成オブジェクトリストに追加しておく
							uncreateObjList->addObject(scenePartData);
							//破棄
							this->removeCommandReappearObject(scenePartData);
						}
#endif
						scenePartObjectDataIdx++;
						continue;
					}
					//「消滅後の復活条件：シーンが切り替わった」オブジェクトの場合
					if (checkSceneChangeReappearObject(scenePartData)) {
						auto objectData = projectData->getObjectData(scenePartData->getObjectId());
						CC_ASSERT(objectData);
						if (objectData->getAppearCondition() == agtk::data::ObjectData::EnumAppearCondition::kAppearConditionCameraNear) {
							// 未生成オブジェクトリストに追加しておく
							uncreateObjList->addObject(scenePartData);
							// 破棄
							this->removeSceneChangeReappearObject(scenePartData);
						}
						scenePartObjectDataIdx++;
						continue;
					}
					// オブジェクトの出現条件を満たしていない場合
					if (!checkObjectAppearCondition(scenePartData)) {
						// 未生成オブジェクトリストに追加しておく
						uncreateObjList->addObject(scenePartData);
						// オブジェクトを生成しない
						scenePartObjectDataIdx++;
						continue;
					}
					//「シーン終了時の状態を維持」をチェックする。
					if (checkObjectTakeoverStatesAsScene(scenePartData)) {
#if defined(USE_RUNTIME)
						if (!projectData->getObjectData(scenePartData->getObjectId())->getTestplayOnly()) {
#endif
							//オブジェクト生成。
							auto data = getObjectTakeoverStatesAsScene(scenePartData);
							auto object = createTakeoverStatesObject(data, scenePartData);
							initcreateObjectList->addObject(object);
#ifdef USE_REDUCE_RENDER_TEXTURE
							objectSetNode->addChild(object);
#else
							this->addChild(object);
#endif
							// 情報を破棄する
							// メニューレイヤー処理
							if (isMemuLayer)
								GameManager::getInstance()->getSceneEndTakeoverStatesObjectList()->removeObject(data);
							scenePartObjectDataIdx++;
#if defined(USE_RUNTIME)
						}
#endif
						continue;
					}
#if defined(USE_RUNTIME)
					if (!projectData->getObjectData(scenePartData->getObjectId())->getTestplayOnly()) {
#endif
						auto object = agtk::Object::createWithSceneDataAndScenePartObjectData(this, scenePartData, -1);
						object->setId(objectId++);
						object->setLayerId(layerId);
						object->setScenePartsId(scenePartData->getId());
						object->setPhysicsBitMask(layerId, sceneData->getId());
						initcreateObjectList->addObject(object);
#ifdef USE_REDUCE_RENDER_TEXTURE
						objectSetNode->addChild(object);
#else
						this->addChild(object);
#endif
#if defined(USE_RUNTIME)
					}
#endif
				}
				scenePartObjectDataIdx++;
			}
		}
	}

	//auto sceneId = this->getSceneData()->getId();
	//シーン終了時の状態維持情報を破棄する。
	GameManager::getInstance()->removeTakeoverStatesObject(sceneId, layerId);

	// シーン切り替えで復活するオブジェクト
	std::function<bool(agtk::data::ObjectReappearData *, int, int, cocos2d::__Array *)> checkReappearObject = [&](agtk::data::ObjectReappearData *reappearData, int sceneId, int layerId, cocos2d::__Array *objectList) {

		// 現在のシーンと対象のシーンが同じ場合
		if (reappearData->getSceneId() != sceneId) {
			return false;
		}
		if (reappearData->getSceneLayerId() != layerId) {
			return false;
		}
		cocos2d::Ref *ref;
		CCARRAY_FOREACH(objectList, ref) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
			auto object = static_cast<agtk::Object *>(ref);
#else
			auto object = dynamic_cast<agtk::Object *>(ref);
#endif
			if (reappearData->getScenePartsId() == object->getScenePartObjectData()->getId()) {
				return false;
			}
		}
		return true;
	};
	auto sceneChangeReappearObjectList = GameManager::getInstance()->getSceneChangeReappearObjectList();
	if (sceneChangeReappearObjectList->count() > 0) {
		cocos2d::Ref *ref = nullptr;
		CCARRAY_FOREACH(sceneChangeReappearObjectList, ref) {
			auto reappearData = dynamic_cast<agtk::data::ObjectReappearData *>(ref);
			if (checkReappearObject(reappearData, sceneId, layerId, initcreateObjectList)) {

				// 削除されたオブジェクトリストに追加する
				deleteObjList->addObject(reappearData);
			}
		}
	}

	this->setObjectList(initcreateObjectList);
	createObjectList->addObjectsFromArray(initcreateObjectList);
	this->setCreateObjectList(createObjectList);
	this->setUncreateObjectList(uncreateObjList);
	this->setDeleteObjectList(deleteObjList);

// #AGTK-NX
#ifdef USE_SAR_OPTIMIZE_1
	{
		cocos2d::Ref *ref = nullptr;
		CCARRAY_FOREACH(initcreateObjectList, ref) {
			auto object = static_cast<agtk::Object*>(ref);
			auto objectData = object->getObjectData();
			if (objectData)
			{
				_objectIdMap.insert(std::pair<int, agtk::Object*>(objectData->getId(), object));
			}
			_instanceIdMap.insert(std::pair<int, agtk::Object*>(object->getInstanceId(), object));
#ifdef USE_SAR_OPTIMIZE_4
			object->registerSwitchWatcher();
#endif
		}
	}
	this->setIsObjectListUpdated(true);
#else
#ifdef USE_SAR_OPTIMIZE_4
	{
		cocos2d::Ref *ref = nullptr;
		CCARRAY_FOREACH(initcreateObjectList, ref) {
			object->registerSwitchWatcher();
		}
	}
#endif
#endif

	return true;
}

/**
* シーンレイヤー終了前処理
*/
void SceneLayer::end()
{
	auto objectList = this->getObjectList();
	if (objectList != nullptr) {
		cocos2d::Ref *ref;
		CCARRAY_FOREACH(objectList, ref) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
			auto obj = static_cast<agtk::Object *>(ref);
#else
			auto obj = dynamic_cast<agtk::Object *>(ref);
#endif
			bool bIgnoredReappearCondition = true;//※シーン終了での消滅後の復活条件は無視。
			this->removeObject(obj, bIgnoredReappearCondition, false, false);
		}
		objectList->removeAllObjects();
	}
}

#ifdef USE_MULTITHREAD_OBJECT_UPDATE
void SceneLayer::objectUpdateWallCollisionThread(ThreadManager::ThreadInfo *threadInfo)
{
	auto subInfo = (ObjectUpdateThreadInfo *)threadInfo->subInfo;
	int i = subInfo->i;
	auto tmpObjectVector = subInfo->tmpObjectVector;
	auto begin = subInfo->begin;
	auto end = subInfo->end;
	float delta = subInfo->delta;
	int index = -1;
	auto usedThreadCount = ThreadManager::getUsedThreadCount();
#ifdef USE_MULTITHREAD_MEASURE
	long maxDuration = 0;
#endif
	for(int j = begin; j < end; j++){
#ifdef USE_MULTITHREAD_MEASURE
		auto startTime2 = chrono::high_resolution_clock::now();
#endif
		if ((j % usedThreadCount) != i) {
			continue;
		}
		auto object = (*tmpObjectVector)[j];
		//THREAD_PRINTF("j: %d, object: 0x%x", j, object);
		object->objectUpdateWallCollision(delta);
#ifdef USE_MULTITHREAD_MEASURE
		auto now2 = chrono::high_resolution_clock::now();
		auto duration2 = static_cast<long>(chrono::duration_cast<chrono::microseconds>(now2 - startTime2).count());
		if (duration2 > maxDuration) {
			maxDuration = duration2;
		}
#endif
	}
#ifdef USE_MULTITHREAD_MEASURE
	threadInfo->maxDuration += maxDuration;
#endif
}
#endif
void SceneLayer::update(float delta)
{
	PROFILING("SceneLayer::update", profiling);
	// 非アクティブの場合
	if (!_activeFlg) {
		// スキップ
		return;
	}

	//優先度計算
	updateObjectPriority();

	updateShader();

	// オブジェクト出現チェック
	auto uncreateObjList = this->getUncreateObjectList();
	
	if (uncreateObjList) {
		for (int i = uncreateObjList->count() - 1; i >= 0; i--) {
			auto partData = dynamic_cast<agtk::data::ScenePartObjectData *>(uncreateObjList->getObjectAtIndex(i));
			if (partData) {
				if (checkObjectAppearCondition(partData)) {
					// オブジェクトを出現させる
					appearObject(partData);

					uncreateObjList->removeObjectAtIndex(i);
				}
			}
		}
	}

	// 消滅したオブジェクト出現チェック
	auto deleteObjList = this->getDeleteObjectList();

	if (deleteObjList) {
		for (int i = 0; i < deleteObjList->count(); i++) {

			auto reappearData = dynamic_cast<agtk::data::ObjectReappearData *>(deleteObjList->getObjectAtIndex(i));
			
			if (reappearData) {
				if (checkObjectReappearCondition(reappearData)) {
					// ACT2-4922 復活時出現条件を満たしていなければUncreateObjectListに追加する。
					auto partData = this->getInitiallyPlacedScenePartObjectDataForReappearData(reappearData);
					if (partData) {
						if (checkObjectAppearCondition(partData)) {
							// オブジェクトを出現させる
							appearObject(partData);
						}
						else {
							this->getUncreateObjectList()->addObject(partData);
						}
					}
					else {
						// オブジェクトを出現させる
						reappearObject(reappearData);
					}

					deleteObjList->removeObjectAtIndex(i);
				}
			}
		}
	}

// #AGTK-NX
#ifdef USE_SAR_OPTIMIZE_1
	this->setIsObjectListUpdated(true);
#endif

	CC_PROFILER_SET_FRAME_POINT(cocos2d::String::createWithFormat("SceneLayer::update(%s) before object update", _layerData->getName())->getCString());
	auto objectList = this->getObjectList();
	//メニューシーンの場合（※スライド・フェード演出中とメニュー非表示時はオブジェクト処理を止める）
	if (this->getType() == agtk::SceneLayer::kTypeMenu) {
		bool bStop = (this->isDisplay() == false);
		if (this->getAlphaValue()->getState() != EventTimer::kStateIdle) {
			bStop = true;
		}
		if (this->getPositionValue()->getState() != EventTimer::kStateIdle) {
			bStop = true;
		}
		if (_menuWorkMargin != 0) {
			if (_menuWorkMargin > 0) {
				_menuWorkMargin--;
			}
			bStop = false;
		}
		if (bStop) {
			objectList = nullptr;
		}
	}

#ifdef USE_MULTITHREAD_OBJECT_UPDATE
	auto threadManager = ThreadManager::getInstance();
#endif
	//update object
	if (objectList) {

		auto tmpObjectList = cocos2d::__Array::create();
		if (objectList->count() > 0) {
			tmpObjectList->addObjectsFromArray(objectList);
		}

		// 初回のみの衝突判定
		if (_isFirstCollisionCheck) {
			Object::updateTimelineListCache(tmpObjectList);

			auto wallComponentName = CollisionComponent::getCollisionComponentName(CollisionComponent::kGroupWall);
			for (int group = -1; group < getGroupCollisionDetections()->count(); group++) {
				CollisionDetaction *collisionDetection = getGroupCollisionDetection(group);
				if (collisionDetection) {
					collisionDetection->updateSpaceStatus();
				}
			}
			for (int group = 0; group < getGroupRoughWallCollisionDetections()->count(); ++group) {
				CollisionDetaction *collisionDetection = getGroupRoughWallCollisionDetection(group);
				if (collisionDetection) {
					collisionDetection->updateSpaceStatus();
				}
			}
			if (tmpObjectList->count() > 0) {
				cocos2d::Ref *ref = nullptr;
				CCARRAY_FOREACH(tmpObjectList, ref) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
					auto object = static_cast<agtk::Object *>(ref);
#else
					auto object = dynamic_cast<agtk::Object *>(ref);
#endif
					CollisionComponent * component = dynamic_cast<CollisionComponent *>(object->getComponent(wallComponentName));
					if (component != nullptr) {
						CollisionNode * collisionObject = component->getCollisionNode();
						if (collisionObject != nullptr) {
							auto objectData = object->getObjectData();
							auto playObjectData = object->getPlayObjectData();
							//ポータル、当たり判定・攻撃判定
							auto point = Scene::getPositionCocos2dFromScene(object->getPosition());
							auto lastLevel = collisionObject->getLevel();
							auto lastGroup = collisionObject->getGroup();
							for (int group = -1; group < 2; group++) {
								//todo CommonCollisionに対してはWallで、HitCollisionに対しては、Attackで判定したい。
								CollisionDetaction *collisionDetection = nullptr;
								CollisionComponent::EnumGroup eg = CollisionComponent::EnumGroup::kGroupMax;
								if (group == -1) {
									collisionDetection = this->getCommonCollisionDetection();
									eg = CollisionComponent::kGroupObjectCenter;
								}
								else {
									if (playObjectData->isHitObjectGroup(group)) {
										collisionDetection = getGroupCollisionDetection(group);
										eg = CollisionComponent::kGroupAttack;
									}
								}
								if (collisionDetection) {
									collisionObject->setGroup(eg);
									int const level = collisionDetection->calcLevel(collisionObject);
									collisionObject->setLevel(level);
									collisionDetection->scanSingle(collisionObject);
								}
							}
							//壁判定
							if (objectData->getCollideWithObjectGroupBit()) {
								int level = -1;
								for (int group = 0; group < getGroupRoughWallCollisionDetections()->count(); group++) {
									CollisionDetaction *collisionDetection = nullptr;
									if (objectData->isCollideWithObjectGroup(group)) {
										CollisionDetaction *collisionDetection = getGroupRoughWallCollisionDetection(group);
									}
									if (collisionDetection) {
										if (level < 0) {
											collisionObject->setGroup(CollisionComponent::kGroupRoughWall);
											level = collisionDetection->calcLevel(collisionObject);
										}
										collisionObject->setLevel(level);
										collisionDetection->scanSingle(collisionObject);
									}
								}
							}
							collisionObject->setLevel(lastLevel);
							collisionObject->setGroup(lastGroup);
						}
					}
				}
			}

			//clear collision
			cocos2d::Ref *ref = nullptr;
			CCARRAY_FOREACH(tmpObjectList, ref) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
				auto object = static_cast<agtk::Object *>(ref);
#else
				auto object = dynamic_cast<agtk::Object *>(ref);
#endif
				object->clearCollision();
			}
			//update collision
			CCARRAY_FOREACH(tmpObjectList, ref) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
				auto object = static_cast<agtk::Object *>(ref);
#else
				auto object = dynamic_cast<agtk::Object *>(ref);
#endif
				object->updateCollision();
			}

			_isFirstCollisionCheck = false;
		}

// #AGTK-NX, #AGTK-WIN
#ifdef USE_30FPS_4
		// 30FPSでの中間フレームの壁判定を得るためのフレーム毎の初期化
		{
			cocos2d::Ref *ref;
			CCARRAY_FOREACH(tmpObjectList, ref) {
				auto object = dynamic_cast<agtk::Object *>(ref);
				object->_middleFrameStock.updateEnterframe();
				object->_bUseMiddleFrame = false;
			}
		}
#endif

#ifdef USE_MULTITHREAD_OBJECT_UPDATE
		CC_PROFILER_SET_FRAME_POINT("SceneLayer::update thread start");
// #AGTK-NX #AGTK-WIN
#ifdef USE_30FPS_3
		cocos2d::Ref *ref;
		int updatedHead = 0;

		// 30FPS対応のため壁判定処理を60FPS毎に2パスで行うための初期設定
		GameManager* gm = GameManager::getInstance();
		int passCount = gm->getFrameProgressScale(); // 1 or 2
#ifdef USE_30FPS_4
		if(passCount == 1) {
			// 60FPSでも中間フレーム情報がある場合は壁判定処理を2パスで処理
			CCARRAY_FOREACH(tmpObjectList, ref) {
				auto object = dynamic_cast<agtk::Object *>(ref);
				auto mf = object->_middleFrameStock.getUpdatedMiddleFrame();
				if (mf->_hasMiddleFrame) {
					passCount = 2;
					break;
				}
			}
		}
#endif
		auto orgDelta = delta;
		for(int passIndex = 0; passIndex < passCount; passIndex++) {
			bool bLastPass = false;
			if (passIndex == passCount - 1) {
				bLastPass = true;
			}

			gm->setPassIndex(passIndex);
			gm->setPassCount(passCount);
			gm->setLastPass(bLastPass);
			gm->saveWallCollisionPass();
#endif
		//「処理の優先度」「コース移動」『他のオブジェクトから「押し戻されない」か』「上に乗っている」で並び替え。
// #AGTK-NX #AGTK-WIN
#ifndef USE_30FPS_3
		cocos2d::Ref *ref;
#endif
		{
			class PtrIndex {
			public:
				PtrIndex(agtk::Object *object, int index) : _object(object), _index(index) {}
				agtk::Object *_object;
				int _index;
			};
			std::vector<PtrIndex> list;
			int index = 0;
			CCARRAY_FOREACH(tmpObjectList, ref) {
				auto object = dynamic_cast<agtk::Object *>(ref);
				list.emplace_back(PtrIndex(object, index));
				index++;
			}
// #AGTK-NX #AGTK-WIN
#ifdef USE_30FPS_3
			auto sortFunc = [](PtrIndex &item1, PtrIndex &item2) {
#else
			std::sort(list.begin(), list.end(), [](PtrIndex &item1, PtrIndex &item2) {
#endif
				auto objectData1 = item1._object->getObjectData();
				auto sceneObjectData1 = item1._object->getScenePartObjectData();
				auto objectData2 = item2._object->getObjectData();
				auto sceneObjectData2 = item2._object->getScenePartObjectData();
				auto prio1 = objectData1->getPriority();
				auto prio2 = objectData2->getPriority();
				if (prio1 > prio2) {
					return true;
				}
				else if (prio1 < prio2) {
					return false;
				}
				if ((sceneObjectData1 && sceneObjectData1->getCourseScenePartId() >= 0) && (!sceneObjectData2 || sceneObjectData2->getCourseScenePartId() < 0)) {
					return true;
				}
				if ((!sceneObjectData1 || sceneObjectData1->getCourseScenePartId() < 0) && (sceneObjectData2 && sceneObjectData2->getCourseScenePartId() >= 0)) {
					return false;
				}
				if (!objectData1->getPushedbackByObject() && objectData2->getPushedbackByObject()) {
					return true;
				}
				if (objectData1->getPushedbackByObject() && !objectData2->getPushedbackByObject()) {
					return false;
				}
				if (item1._object->getDownWallObjectList()->containsObject(item2._object)) {
					return false;
				}
				if (item2._object->getDownWallObjectList()->containsObject(item1._object)) {
					return true;
				}
				if (item1._object->getUpWallObjectList()->containsObject(item2._object)) {
					return true;
				}
				if (item2._object->getUpWallObjectList()->containsObject(item1._object)) {
					return false;
				}
				return item1._index < item2._index;
// #AGTK-NX #AGTK-WIN
#ifdef USE_30FPS_3
			};
			if (passIndex == 0) {
				// pass1でのオブジェクトソート
				std::sort(list.begin(), list.end(), sortFunc);
			}
			else {
				// pass2でのオブジェクトソート
				auto headIt = list.end();
				int index = 0;
				for (auto it = list.begin(); it != list.end(); it++) {
					if (index == updatedHead) {
						headIt = it;
						break;
					}
					index++;
				}
				std::sort(headIt, list.end(), sortFunc);
			}
#else
			});
#endif
			tmpObjectList->removeAllObjects();
			for (auto item : list) {
				tmpObjectList->addObject(item._object);
			}
		}
#if 0
		std::string str;
		CCARRAY_FOREACH(tmpObjectList, ref) {
			auto object = dynamic_cast<agtk::Object *>(ref);
			auto objectData = object->getObjectData();
			char buf[16];
			sprintf(buf, "%d", objectData->getPriority());
			str = str + ", " + buf + "-";
			auto sceneObjectData = object->getScenePartObjectData();
			if (sceneObjectData && sceneObjectData->getCourseScenePartId() >= 0) {
				str = str + "cr(" + sceneObjectData->getName() + ")";
			}
			else {
				if (objectData->getPushedbackByObject()) {
					str = str + "pb(";
				}
				else {
					str = str + "!pb";
				}
				if (sceneObjectData) {
					str += sceneObjectData->getName();
				}
				else {
					str += " ";
					str += objectData->getName();
				}
				str += ")";
			}
		}
		if (str.length() > 0) {
			cocos2d::log("PushedBack: %s", str.c_str());
		}
#endif
// #AGTK-NX #AGTK-WIN
#ifndef USE_30FPS_3
		int updatedHead = 0;
#endif
// #AGTK-NX #AGTK-WIN
#ifdef USE_30FPS_3
		//　Objectのupdate(), objectUpdateBefore() は最初のpassのみ実行
		if(passIndex == 0) {
#endif
		int i = 0;
		CCARRAY_FOREACH(tmpObjectList, ref) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
			auto object = static_cast<agtk::Object *>(ref);
#else
			auto object = dynamic_cast<agtk::Object *>(ref);
#endif
			auto objectData = object->getObjectData();
			auto sceneObjectData = object->getScenePartObjectData();
			if ((sceneObjectData && sceneObjectData->getCourseScenePartId() >= 0) || !objectData->getPushedbackByObject()) {
				updatedHead = i + 1;
			}
			i++;
		}
		i = 0;
		CCARRAY_FOREACH(tmpObjectList, ref) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
			auto object = static_cast<agtk::Object *>(ref);
#else
			auto object = dynamic_cast<agtk::Object *>(ref);
#endif
			auto objectData = object->getObjectData();
			auto sceneObjectData = object->getScenePartObjectData();
			if (i < updatedHead) {
				object->update(delta);
			}
			else {
// #AGTK-NX #AGTK-WIN
#ifdef USE_30FPS_3
				gm->setPassIndex(0);
				gm->setPassCount(1);
				gm->setLastPass(true);
#endif
				object->objectUpdateBefore(delta);
// #AGTK-NX #AGTK-WIN
#ifdef USE_30FPS_3
				gm->restoreWallCollisionPass();
				if (passCount > 1) {
					// 2pass処理向けに位置と移動量の補正
					object->_iniPos = object->getPosition();
					Vec2 mv = object->getPosition() - object->getOldPosition();
					object->setPosition(object->getOldPosition() + mv * 0.5f);
					object->_halfMove = mv - mv * 0.5f;
				}
#endif
			}
			i++;
		}
// #AGTK-NX #AGTK-WIN
#ifdef USE_30FPS_3
			if (passCount > 1) {
				// delta補正
				delta = delta * 0.5f;
			}
		} // if (passIndex == 0) {}
		else {
			if (passCount > 1) {
				int i = 0;
				CCARRAY_FOREACH(tmpObjectList, ref) {
					auto object = static_cast<agtk::Object *>(ref);
					if (i >= updatedHead) {
						// 中間フレームの位置を記録
						Vec2 pass1Pos = object->getPosition();
						Vec2 diff = pass1Pos - object->_iniPos;
						if (diff != Vec2::ZERO) {
							// 中間地点を反映
							object->setPassedFrameCount(object->getFrameCount());
							object->setPassedFramePosition(pass1Pos);
						}
						// pass2での過去位置更新
						object->setOldPosition(pass1Pos);
						object->setOldPosition2(pass1Pos);
						auto player = object->getPlayer();
						if (player) {
							object->setOldWallPosition(player->getPosition());
						}
						// pass2用のWallHitInfoGroupに更新
						object->_bUseMiddleFrame = true;
						object->getObjectCollision()->updateMiddleFrameWallHitInfoGroup();
						// 2pass処理向けに位置と移動量の補正
						// 2pass移動により本来の目標位置への移動に誤差が発生するため誤差が僅かならば補正
						Vec2 pass2Pos = object->getPosition() + object->_halfMove;
						diff = pass2Pos - object->_iniPos;
						diff.x = std::abs(diff.x);
						diff.y = std::abs(diff.y);
						if (diff.x > 0.0f && diff.x < 0.001f) {
							pass2Pos.x = object->_iniPos.x;
						}
						if (diff.y > 0.0f && diff.y < 0.001f) {
							pass2Pos.y = object->_iniPos.y;
						}
						object->setPosition(pass2Pos);
					}
					i++;
				}
				// delta補正
				delta = orgDelta - delta;
			}
		}
#endif
		if (tmpObjectList->count() > 0) {
#ifdef USE_MULTITHREAD_MEASURE
			cocos2d::log("enter multi-threading: %d", tmpObjectList->count());
#endif
#ifdef USE_MULTITHREAD_MEASURE
			_commonCollisionDetection->setStateChangeCounter(0);
			for (int group = 0; group < getGroupCollisionDetections()->count(); ++group) {
				auto collisionDetection = getGroupCollisionDetection(group);
				collisionDetection->setStateChangeCounter(0);
			}
			for (int group = 0; group < getGroupRoughWallCollisionDetections()->count(); ++group) {
				auto collisionDetection = getGroupRoughWallCollisionDetection(group);
				collisionDetection->setStateChangeCounter(0);
			}
			for (int group = 0; group < getGroupWallCollisionDetections()->count(); ++group) {
				auto collisionDetection = getGroupWallCollisionDetection(group);
				collisionDetection->setStateChangeCounter(0);
			}
#endif
			auto usedThreadCount = ThreadManager::getUsedThreadCount();
			auto objectUpdateThreadInfoList = new ObjectUpdateThreadInfo[usedThreadCount - 1];
			long duration = 0;
			long duration0 = 0;
#ifdef USE_MULTITHREAD_MEASURE
			ThreadManager::clearBlockedCount();
#endif
			if (usedThreadCount <= 1 || tmpObjectList->count() < 10) {	//10は仮置き。
				//一定数以下はシングルスレッドで処理。
#ifdef USE_MULTITHREAD_MEASURE
				auto startTime = chrono::high_resolution_clock::now();
#endif
				int index = 0;
				CCARRAY_FOREACH(tmpObjectList, ref) {
					if (index >= updatedHead) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
						auto object = static_cast<agtk::Object *>(ref);
#else
						auto object = dynamic_cast<agtk::Object *>(ref);
#endif
						object->objectUpdateWallCollision(delta);
					}
					index++;
				}
#ifdef USE_MULTITHREAD_MEASURE
				duration = static_cast<long>(chrono::duration_cast<chrono::microseconds>(chrono::high_resolution_clock::now() - startTime).count());
#endif
#ifdef USE_MULTITHREAD_MEASURE
				for (int i = 0; i < usedThreadCount - 1; i++) {
					auto threadInfo = threadManager->getThreadInfo(i);
					auto subInfo = &objectUpdateThreadInfoList[i];
					threadInfo->subInfo = subInfo;
					threadInfo->buf[0] = 0;
				}
#endif
			}
			else {
				int order = 0;
				cocos2d::Ref *ref;
				std::vector<Object *> tmpObjectVector;
				tmpObjectVector.reserve(tmpObjectList->count());
				int index = 0;
				CCARRAY_FOREACH(tmpObjectList, ref) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
					auto object = static_cast<agtk::Object *>(ref);
#else
					auto object = dynamic_cast<agtk::Object *>(ref);
#endif
					object->_wallCollisionUpdateOrder = order++;
					if (index >= updatedHead) {
						tmpObjectVector.push_back(object);
					}
					index++;
				}
				// スレッドの開始準備
				for (int i = 0; i < usedThreadCount - 1; i++) {
					auto threadInfo = threadManager->getThreadInfo(i);
					threadInfo->func = &objectUpdateWallCollisionThread;
					auto subInfo = &objectUpdateThreadInfoList[i];
					threadInfo->subInfo = subInfo;
					subInfo->i = i + 1;
					subInfo->tmpObjectVector = &tmpObjectVector;
					subInfo->begin = 0;
					subInfo->end = tmpObjectVector.size();
					subInfo->delta = delta;
					if (threadInfo->state != ThreadManager::kThreadStateIdle && threadInfo->state != ThreadManager::kThreadStateFinished) {
						cocos2d::log("not idle not finished: %d", threadInfo->state);
					}
					threadInfo->state = ThreadManager::kThreadStateReady;	// スレッド処理開始準備
#ifdef USE_MULTITHREAD_MEASURE
					cocos2d::log("%d: %s", i, threadInfo->buf);
					threadInfo->buf[0] = 0;
					threadInfo->maxDuration = 0;
#endif
					threadInfo->localMutex.unlock();
				}
#ifdef USE_MULTITHREAD_MEASURE
				auto startTime0 = chrono::high_resolution_clock::now();
#endif
				{
					// １つはメインスレッドで行う
#ifdef USE_MULTITHREAD_MEASURE
					auto startTime = chrono::high_resolution_clock::now();
#endif
					for (int j = 0; j < (int)tmpObjectVector.size(); j++) {
						if ((j % usedThreadCount) != 0) {
							continue;
						}
						auto object = tmpObjectVector[j];
						object->objectUpdateWallCollision(delta);
					}
#ifdef USE_MULTITHREAD_MEASURE
					duration = static_cast<long>(chrono::duration_cast<chrono::microseconds>(chrono::high_resolution_clock::now() - startTime).count());
#endif
				}
				{
					PROFILING("Waiting thread", profiler);
					for (int i = 0; i < usedThreadCount - 1; i++) {
						auto threadInfo = threadManager->getThreadInfo(i);
						threadInfo->localMutex2.lock();
						if (threadInfo->state != ThreadManager::kThreadStateFinished) {
							cocos2d::log("not finished: %d", threadInfo->state);
						}
						threadInfo->localMutex3.unlock();
						threadInfo->localMutex.lock();
						threadInfo->localMutex2.unlock();
						threadInfo->localMutex3.lock();
					}
				}
#ifdef USE_MULTITHREAD_MEASURE
				duration0 = static_cast<long>(chrono::duration_cast<chrono::microseconds>(chrono::high_resolution_clock::now() - startTime0).count());
#endif
			}
#ifdef USE_MULTITHREAD_MEASURE
			char buf[2048];
			buf[0] = 0;
			sprintf(buf, "%dus(%dus), ", duration, duration0);
			for (int i = 0; i < usedThreadCount - 1; i++) {
				auto threadInfo = threadManager->getThreadInfo(i);
				//auto subInfo = (ObjectUpdateThreadInfo *)threadInfo->subInfo;
				snprintf(buf + strlen(buf), sizeof(buf) - strlen(buf) - 1, "%d:(%s), ", i + 1, threadInfo->buf);
			}
			if (ThreadManager::mtVectorBlockedCount > 0) {
				snprintf(buf + strlen(buf), sizeof(buf) - strlen(buf) - 1, "mtVectorBlockedCount:%d, ", ThreadManager::mtVectorBlockedCount);
			}
			if (ThreadManager::objectBlockedCount > 0) {
				snprintf(buf + strlen(buf), sizeof(buf) - strlen(buf) - 1, "objectBlockedCount:%d, ", ThreadManager::objectBlockedCount);
			}
			if (ThreadManager::playerBlockedCount > 0) {
				snprintf(buf + strlen(buf), sizeof(buf) - strlen(buf) - 1, "playerBlockedCount:%d, ", ThreadManager::playerBlockedCount);
			}
			cocos2d::log("duration: %s", buf);
#endif
#ifdef USE_MULTITHREAD_MEASURE
			bool blocked = false;
			buf[0] = 0;
			if (_commonCollisionDetection->getStateChangeCounter() > 0) {
				blocked = true;
			}
			sprintf(buf, "%d", _commonCollisionDetection->getStateChangeCounter());
			auto stateChangeCounter = _commonCollisionDetection->getStateChangeCounter();
			for (int group = 0; group < getGroupCollisionDetections()->count(); ++group) {
				auto collisionDetection = getGroupCollisionDetection(group);
				if (collisionDetection->getStateChangeCounter() > 0) {
					blocked = true;
				}
				snprintf(buf + strlen(buf), sizeof(buf) - strlen(buf) - 1, ", %d:%d", group, collisionDetection->getStateChangeCounter());
			}
			for (int group = 0; group < getGroupRoughWallCollisionDetections()->count(); ++group) {
				auto collisionDetection = getGroupRoughWallCollisionDetection(group);
				if (collisionDetection->getStateChangeCounter() > 0) {
					blocked = true;
				}
				snprintf(buf + strlen(buf), sizeof(buf) - strlen(buf) - 1, ", %d:%d", group, collisionDetection->getStateChangeCounter());
			}
			for (int group = 0; group < getGroupWallCollisionDetections()->count(); ++group) {
				auto collisionDetection = getGroupWallCollisionDetection(group);
				if (collisionDetection->getStateChangeCounter() > 0) {
					blocked = true;
				}
				snprintf(buf + strlen(buf), sizeof(buf) - strlen(buf) - 1, ", %d:%d", group, collisionDetection->getStateChangeCounter());
			}
			if (blocked) {
				cocos2d::log("CollisionDetection mtx blocked count: %s", buf);
			}
#endif
			delete[] objectUpdateThreadInfoList;
			objectUpdateThreadInfoList = nullptr;
		}
// #AGTK-NX #AGTK-WIN
#ifdef USE_30FPS_3
		} // for(int passIndex = 0; passIndex < passCount; passIndex++);
		// オブジェクトの初回に一度壁判定処理を行う為1passで処理する設定に戻す
		gm->setPassIndex(1);
		gm->setPassCount(1);
		gm->setLastPass(true);
		delta = orgDelta;
#endif
		int index = 0;
		CCARRAY_FOREACH(tmpObjectList, ref) {
			if (index >= updatedHead) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
				auto object = static_cast<agtk::Object *>(ref);
#else
				auto object = dynamic_cast<agtk::Object *>(ref);
#endif
				object->objectUpdateAfter(delta);
			}
			index++;
		}
// #AGTK-NX
#ifndef USE_SAR_OPTIMIZE_5_1
		{
			auto collisionDetection = getCommonCollisionDetection();
			collisionDetection->deleteNullptrm_spaceArray();
		}
		for (int group = 0; group < getGroupCollisionDetections()->count(); ++group) {
			auto collisionDetection = getGroupCollisionDetection(group);
			collisionDetection->deleteNullptrm_spaceArray();
		}
		for (int group = 0; group < getGroupRoughWallCollisionDetections()->count(); ++group) {
			auto collisionDetection = getGroupRoughWallCollisionDetection(group);
			collisionDetection->deleteNullptrm_spaceArray();
		}
		for (int group = 0; group < getGroupWallCollisionDetections()->count(); ++group) {
			auto collisionDetection = getGroupWallCollisionDetection(group);
			collisionDetection->deleteNullptrm_spaceArray();
		}
#endif // USE_SAR_OPTIMIZE_5
		CC_PROFILER_SET_FRAME_POINT("SceneLayer::update thread end");
#else
		cocos2d::Ref *ref;
		CCARRAY_FOREACH(tmpObjectList, ref) {
			auto object = dynamic_cast<agtk::Object *>(ref);
			object->update(delta);
		}
#endif

		// オブジェクトの消滅判定
		CCARRAY_FOREACH_REVERSE(tmpObjectList, ref) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
			auto object = static_cast<agtk::Object *>(ref);
#else
			auto object = dynamic_cast<agtk::Object *>(ref);
#endif
			// 消滅条件を達成した場合は、オブジェクトを削除する
			if (checkObjectDisappearCondition(object)) {
				object->removeSelf();
			}
		}
	}
	CC_PROFILER_SET_FRAME_POINT(cocos2d::String::createWithFormat("SceneLayer::update(%s) after object update", _layerData->getName())->getCString());

	// update portal
	if (this->getPortalObjectList()) {
		cocos2d::Ref *ref = nullptr;
		CCARRAY_FOREACH(this->getPortalObjectList(), ref) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
			auto portal = static_cast<agtk::Portal *>(ref);
#else
			auto portal = dynamic_cast<agtk::Portal *>(ref);
#endif
			portal->update(delta);
		}
	}

	//update physics object
	if (this->getPhysicsObjectList()) {
		cocos2d::Ref *ref = nullptr;
		CCARRAY_FOREACH(this->getPhysicsObjectList(), ref) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
			auto node = static_cast<cocos2d::Node *>(ref);
#else
			auto node = dynamic_cast<cocos2d::Node *>(ref);
#endif
			node->update(delta);
		}
	}

	//update tile
	if (this->getTileMapList()) {
		cocos2d::Ref *ref = nullptr;
		CCARRAY_FOREACH(this->getTileMapList(), ref) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
			auto tileMap = static_cast<agtk::TileMap *>(ref);
#else
			auto tileMap = dynamic_cast<agtk::TileMap *>(ref);
#endif
			tileMap->update(delta);
		}
	}
	CC_PROFILER_SET_FRAME_POINT(cocos2d::String::createWithFormat("SceneLayer::update(%s) after update tiles", _layerData->getName())->getCString());

	// collision
	Object::updateTimelineListCache(objectList);
#if 0	//debug: overlay
	if (objectList) {
		cocos2d::Ref *ref = nullptr;
		CCARRAY_FOREACH(objectList, ref) {
			auto object = dynamic_cast<agtk::Object *>(ref);
			auto switchData = object->getPlayObjectData()->getSwitchData(2006);

			if (switchData) {
				switchData->setValue(false);
			}
		}
	}
#endif
	if (getGroupCollisionDetections()) {
		for (int group = 0; group < getGroupCollisionDetections()->count(); group++) {
			//ポータルは位置が不変のため、CommonCollisionDetactionの処理は不要。当たりコリジョンだけ更新。
			CollisionDetaction *collisionDetection = getGroupCollisionDetection(group);
			if (collisionDetection) {
				collisionDetection->updateSpaceStatus();
			}
		}
	}
	if (getGroupRoughWallCollisionDetections()) 
	{
		for (int group = 0; group < getGroupRoughWallCollisionDetections()->count(); ++group) {
			//壁コリジョン更新。
			CollisionDetaction *collisionDetection = getGroupRoughWallCollisionDetection(group);
			if (collisionDetection) {
#if 0	//debug: limit
				if (strcmp(this->_layerData->getName(), "レイヤー1") == 0) {
					CCLOG("%s", this->_layerData->getName());
				}
#endif
				collisionDetection->updateSpaceStatus();
			}
		}
	}
	if (objectList) {
		cocos2d::Ref *ref = nullptr;
		auto hitComponentName = CollisionComponent::getCollisionComponentName(CollisionComponent::kGroupHit);
		CCARRAY_FOREACH(objectList, ref) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
			auto object = static_cast<agtk::Object *>(ref);
#else
			auto object = dynamic_cast<agtk::Object *>(ref);
#endif
			CollisionComponent * component = dynamic_cast<CollisionComponent *>(object->getComponent(hitComponentName));
			if (component != nullptr) {
				CollisionNode * collisionObject = component->getCollisionNode();
				if (collisionObject != nullptr && collisionObject->getNode() != nullptr) {
					auto objectData = object->getObjectData();
					auto playObjectData = object->getPlayObjectData();
					//ポータルコリジョン、当たりコリジョン・攻撃判定
					auto lastLevel = collisionObject->getLevel();
					auto lastGroup = collisionObject->getGroup();
// #AGTK-NX #AGTK-WIN
#ifdef USE_30FPS_1
					// 30FPSの時、中間点でも判定結果を得るための処理
					cocos2d::Vec2 orgPos;
					int passCount = (int)GameManager::getInstance()->getFrameProgressScale();
					if (object->getPassedFrameCount() != object->getFrameCount()) {
						passCount = 1;
					}
					else {
						orgPos = object->getPosition();
					}
					
					for (int passIndex = 0; passIndex < passCount; passIndex++) {
						if (passIndex < passCount - 1) {
							object->setPosition(object->getPassedFramePosition());
						}
						else {
							if (passCount > 1) {
								object->setPosition(orgPos);
							}
						}

#endif
					for (int group = -1; group < getGroupCollisionDetections()->count(); group++) {
						//todo CommonCollisionに対してはWallで、HitCollisionに対しては、Attackで判定したい。
						CollisionDetaction *collisionDetection = nullptr;
						CollisionComponent::EnumGroup eg = CollisionComponent::kGroupMax;
						if (group == -1) {
							collisionDetection = this->getCommonCollisionDetection();
							eg = CollisionComponent::kGroupObjectCenter;
						}
						else {
							if (playObjectData->isHitObjectGroup(group)) {
								collisionDetection = getGroupCollisionDetection(group);
								eg = CollisionComponent::kGroupAttack;
							}
						}
						if (collisionDetection) {
							collisionObject->setGroup(eg);
							int const level = collisionDetection->calcLevel(collisionObject);
							collisionObject->setLevel(level);
							collisionDetection->scanSingle(collisionObject);
						}
					}
// #AGTK-NX #AGTK-WIN
#ifdef USE_30FPS_1
					}
#endif
					//壁判定
					int level = -1;
					for (int group = 0; group < getGroupRoughWallCollisionDetections()->count(); ++group) {
						CollisionDetaction *collisionDetection = nullptr;

						if (objectData->isCollideWithObjectGroup(group)) {
							collisionDetection = getGroupRoughWallCollisionDetection(group);
						}
						
						if (collisionDetection) {
							if (level < 0) {
								collisionObject->setGroup(CollisionComponent::kGroupRoughWall);
								level = collisionDetection->calcLevel(collisionObject);
							}
							collisionDetection->scanSingle(collisionObject);
						}
					}
					collisionObject->setLevel(lastLevel);
					collisionObject->setGroup(lastGroup);
#if 0	//debug: overlay
					auto variableData = object->getPlayObjectData()->getVariableData(2002);

					if (variableData) {
						variableData->setValue(collisionObject->getLevel());
					}
#endif
				}
			}
		}
	}
	CC_PROFILER_SET_FRAME_POINT(cocos2d::String::createWithFormat("SceneLayer::update(%s) after collision update", _layerData->getName())->getCString());

	if (objectList) {
		//clear collision
		cocos2d::Ref *ref = nullptr;
		CCARRAY_FOREACH(objectList, ref) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
			auto object = static_cast<agtk::Object *>(ref);
#else
			auto object = dynamic_cast<agtk::Object *>(ref);
#endif
			object->clearCollision();
		}
		//update collision
		CCARRAY_FOREACH(objectList, ref) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
			auto object = static_cast<agtk::Object *>(ref);
#else
			auto object = dynamic_cast<agtk::Object *>(ref);
#endif
			object->updateCollision();
		}
		// ポータル処理
		CCARRAY_FOREACH(objectList, ref) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
			auto object = static_cast<agtk::Object *>(ref);
#else
			auto object = dynamic_cast<agtk::Object *>(ref);
#endif
			object->updatePortalActivation();
		}
	}
	CC_PROFILER_SET_FRAME_POINT(cocos2d::String::createWithFormat("SceneLayer::update(%s) after object portal", _layerData->getName())->getCString());

	float _tmpDelta = delta;
	float menuTimeScale = GameManager::getInstance()->getCurrentScene()->getGameSpeed()->getTimeScale(SceneGameSpeed::eTYPE_MENU);
	if (this->getType() == kTypeMenu) {
		delta *= menuTimeScale;
		//position
		auto positionValue = this->getPositionValue();
		positionValue->update(delta);
		this->setPosition(positionValue->getValue().x, positionValue->getValue().y);

		//alpha
		auto alphaValue = this->getAlphaValue();
		alphaValue->update(delta);
		auto value = alphaValue->getValue();
		auto rtCtrl = this->getRenderTexture();
		if (rtCtrl) {
			auto sprite = rtCtrl->getFirstRenderTexture()->getSprite();
			if (sprite) {
				sprite->setOpacity(value);
			}
		}
		else {
			// 子要素にも透明値を反映させる
			updateCascadeOpacityEnabled(this, true);
			this->setOpacity(value);
		}

		EffectManager::getInstance()->setAlpha(this, value / 255.0f);
		ParticleManager::getInstance()->setAlpha(this, value / 255.0f);

		if (this->getRemoveSelfFlag() && value == 0.0f) {
			auto scene = GameManager::getInstance()->getCurrentScene();
			auto menuLayerList = scene->getMenuLayerList();
			cocos2d::DictElement *el;
			CCDICT_FOREACH(menuLayerList, el) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
				auto menuLayer = static_cast<agtk::SceneLayer *>(el->getObject());
#else
				auto menuLayer = dynamic_cast<agtk::SceneLayer *>(el->getObject());
#endif
				if (menuLayer == this) {
#if 1
					// menuLayer削除での不足処理対応
					// *** ACT2-6304
					auto objGroup = GameManager::getInstance()->getProjectData()->getObjectGroup();
					menuLayer->removeFromParentAndCleanup(true);
					menuLayer->removeRenderTexture();
					for (int group = -1; group < menuLayer->getGroupCollisionDetections()->count(); group++) {
						CollisionDetaction *collisionDetection = menuLayer->getGroupCollisionDetection(group);
						if (collisionDetection) {
							collisionDetection->reset();
						}
					}
					for (int group = 0; group < menuLayer->getGroupRoughWallCollisionDetections()->count(); ++group) {
						CollisionDetaction *collisionDetection = menuLayer->getGroupRoughWallCollisionDetection(group);
						if (collisionDetection) {
							collisionDetection->reset();
						}
					}
					for (int group = 0; group < objGroup->count(); ++group) {
						CollisionDetaction *collisionDetection = menuLayer->getGroupWallCollisionDetection(group);
						if (collisionDetection) {
							collisionDetection->reset();
						}
					}
					menuLayer->end();

					menuLayerList->removeObjectForKey(el->getIntKey());
					return;
				}
			}
#else
					menuLayerList->removeObjectForKey(el->getIntKey());
					break;
				}
			}
			auto layer = GameManager::getInstance()->getCurrentLayer();
			layer->removeChild(this);
			this->setRemoveSelfFlag(false);
#endif
		}
	}
	delta = _tmpDelta;

	//sprite
	//update
	auto sceneSpriteList = this->getSceneSpriteList();
	cocos2d::Ref *ref = nullptr;
	CCARRAY_FOREACH(sceneSpriteList, ref) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
		auto sceneSprite = static_cast<agtk::SceneSprite *>(ref);
#else
		auto sceneSprite = dynamic_cast<agtk::SceneSprite *>(ref);
#endif
		sceneSprite->update(delta);
	}
	//remove
	while (1) {
		bool bRemove = false;
		CCARRAY_FOREACH(sceneSpriteList, ref) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
			auto sceneSprite = static_cast<agtk::SceneSprite *>(ref);
#else
			auto sceneSprite = dynamic_cast<agtk::SceneSprite *>(ref);
#endif
			auto opacity = sceneSprite->getOpacityTimer();
			if (opacity->getState() == agtk::EventTimer::kStateEnd && opacity->getValue() == 0.0f) {
				this->removeChild(sceneSprite);
				sceneSpriteList->removeObject(sceneSprite);
				bRemove = true;
				break;
			}
		}
		if (bRemove == false) break;
	}

	//描画の優先度計算
	updateObjectDispPriority();
	CC_PROFILER_SET_FRAME_POINT(cocos2d::String::createWithFormat("SceneLayer::update(%s) after updateObjectDispPriority", _layerData->getName())->getCString());
}

//メニューレイヤーのオブジェクトが停止しているかを返す。
bool SceneLayer::isMenuObjectStop()
{
	//メニューシーンの場合（※スライド・フェード演出中とメニュー非表示時はオブジェクト処理を止める）
	if (this->getType() == agtk::SceneLayer::kTypeMenu) {
		bool bStop = (this->isDisplay() == false);
		if (this->getAlphaValue()->getState() != EventTimer::kStateIdle) {
			bStop = true;
		}
		if (this->getPositionValue()->getState() != EventTimer::kStateIdle) {
			bStop = true;
		}
		if (_menuWorkMargin != 0) {
			bStop = false;
		}
		return bStop;
	}
	return false;
}

#ifdef USE_REDUCE_RENDER_TEXTURE
void SceneLayer::visit(Renderer *renderer, const Mat4& parentTransform, uint32_t parentFlags)
{
	//Node::visit(renderer, _parent->getNodeToParentTransform(), parentFlags);
	Node::visit(renderer, parentTransform, parentFlags);
}
#endif

void SceneLayer::updateObjectPriority()
{
	//処理順番
	auto objectList = this->getCreateObjectList();
	struct ObjectSortInfo {
		agtk::Object *_object;
		int _priority;
	};
	cocos2d::Ref *ref;
	int objectCount = 0;
	auto listLength = objectList->count();
	std::vector<ObjectSortInfo> sortList;
	sortList.resize(listLength);
	CCARRAY_FOREACH(objectList, ref) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
		auto object = static_cast<agtk::Object *>(ref);
#else
		auto object = dynamic_cast<agtk::Object *>(ref);
#endif
		auto &p = sortList[objectCount];
		//計算されたプライオリティーがオーバーフローすると誤動作する。
		p._object = object;
		p._priority = object->getObjectData()->getPriority() * listLength + listLength - objectCount;
		objectCount++;
	}

	std::function<bool(const ObjectSortInfo &, const ObjectSortInfo &)> compare = [&](const ObjectSortInfo &p1, const ObjectSortInfo &p2) {
		return (p1._priority > p2._priority);
	};
	std::sort(sortList.begin(), sortList.end(), compare);

	auto newObjectList = cocos2d::__Array::create();
	for (auto &p : sortList) {
		newObjectList->addObject(p._object);
	}
	this->setObjectList(newObjectList);
}

void SceneLayer::updateObjectDispPriority()
{
	auto projectData = GameManager::getInstance()->getProjectData();

	auto objectList = [](cocos2d::__Array *objectList)->cocos2d::__Array * {
		auto connectObjectDispPriorityList = cocos2d::__Array::create();
		cocos2d::Ref *ref;
		CCARRAY_FOREACH(objectList, ref) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
			auto obj = static_cast<agtk::Object *>(ref);
#else
			auto obj = dynamic_cast<agtk::Object *>(ref);
#endif
			auto connectList = obj->getConnectObjectDispPriorityList();
			if (connectList->count() > 0) {
				cocos2d::Ref *ref2;
				CCARRAY_FOREACH(connectList, ref2) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
					auto p = static_cast<agtk::Object::ConnectObjectDispPriority *>(ref2);
#else
					auto p = dynamic_cast<agtk::Object::ConnectObjectDispPriority *>(ref2);
#endif
					connectObjectDispPriorityList->addObject(p->getObject());
				}
			}
		}
		auto list = cocos2d::__Array::create();
		CCARRAY_FOREACH(objectList, ref) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
			auto obj = static_cast<agtk::Object *>(ref);
#else
			auto obj = dynamic_cast<agtk::Object *>(ref);
#endif
			if (connectObjectDispPriorityList->containsObject(obj)) {
				continue;
			}
			list->addObject(obj);
		}
		return list;
	} (this->getObjectList());

	auto dispObjectList = cocos2d::__Array::create();
	cocos2d::Ref *ref;

	//表示優先度
	std::function<int(agtk::Object*)> getDispPriority = [](agtk::Object *obj) {
		int prio;
		int _prio = prio = obj->getObjectData()->getDispPriority();//オブジェクトの表示優先度
		int lprio = 999;
		if (obj->getScenePartObjectData() && !obj->getScenePartObjectData()->isStartPointObject()) {
			prio = obj->getScenePartObjectData()->getDispPriority();//シーンの表示優先度
			lprio = obj->getScenePartObjectData()->getPriority();//シーンのサブ表示優先度
		}
		int vprio = (int)obj->getPlayObjectData()->getVariableData(agtk::data::kObjectSystemVariableDispPriority)->getValue();//変数の表示優先度
		if (_prio != vprio) {
			prio = vprio;
		}
		return prio * 1000 + lprio;
	};
	std::function<int(agtk::Object*)> getTopviewDispPriority = [](agtk::Object *obj) {
		int prio;
		int _prio = prio = obj->getObjectData()->getDispPriority();//オブジェクトの表示優先度
		if (obj->getScenePartObjectData() && !obj->getScenePartObjectData()->isStartPointObject()) {
			prio = obj->getScenePartObjectData()->getDispPriority();//シーンの表示優先度
		}
		int vprio = (int)obj->getPlayObjectData()->getVariableData(agtk::data::kObjectSystemVariableDispPriority)->getValue();//変数の表示優先度
		if (_prio != vprio) {
			prio = vprio;
		}
		return prio;
	};
	auto getMenuDispPriority = [](agtk::Object *obj) -> int {
		int prio;
		int _prio = prio = obj->getObjectData()->getDispPriority();//オブジェクトの表示優先度
		if (obj->getScenePartObjectData() && !obj->getScenePartObjectData()->isStartPointObject()) {
			prio = obj->getScenePartObjectData()->getDispPriority();//シーンの表示優先度
		}
		int vprio = (int)obj->getPlayObjectData()->getVariableData(agtk::data::kObjectSystemVariableDispPriority)->getValue();//変数の表示優先度
		if (_prio != vprio) {
			prio = vprio;
		}
		return prio;
	};
	auto getPhysicsDispPriority = [](agtk::PhysicsBase *physics) -> int {
		int prio = physics->getDispPriority();
		int lprio = physics->getPriority();
		return prio * 1000 + lprio;
	};
	auto getTopviewPhysicsDispPriority = [](agtk::PhysicsBase *physics) -> int {
		int prio = physics->getDispPriority();
		return prio;
	};
	auto getMenuPhysicsDispPriority = [](agtk::PhysicsBase *physics) -> int {
		int prio = physics->getDispPriority();
		return prio;
	};
	//描画オブジェクトリスト設定。
	std::function<cocos2d::__Array *(cocos2d::__Array *, agtk::Object *)> setDispObjectList = [&](cocos2d::__Array *list, agtk::Object *object)->cocos2d::__Array* {
		auto connectObjectDispPriorityList = object->getConnectObjectDispPriorityList();
		cocos2d::Ref *ref;
		//奥
		CCARRAY_FOREACH(connectObjectDispPriorityList, ref) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
			auto p = static_cast<agtk::Object::ConnectObjectDispPriority *>(ref);
#else
			auto p = dynamic_cast<agtk::Object::ConnectObjectDispPriority *>(ref);
#endif
			//子オブジェクト
			agtk::Object *parentObject = p->getObject()->getOwnParentObject();
			agtk::Player *player = parentObject ? parentObject->getPlayer() : nullptr;
			bool bBackSide = false;
			if (parentObject != nullptr && player != nullptr) {
				bBackSide = player->getTimelineBackside(p->getObject()->getParentFollowConnectId());
			}
			//オブジェクト接続
			auto connectObject = dynamic_cast<agtk::ConnectObject *>(p->getObject());
			player = object->getPlayer();
			if (bBackSide == false && connectObject != nullptr && player != nullptr) {
				bBackSide = player->getTimelineBackside(connectObject->getConnectionId());
			}
			if (p->getLowerPriority() == true || bBackSide == true) {
				list = setDispObjectList(list, p->getObject());
			}
		}
		list->addObject(object);
		//手前
		CCARRAY_FOREACH(connectObjectDispPriorityList, ref) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
			auto p = static_cast<agtk::Object::ConnectObjectDispPriority *>(ref);
#else
			auto p = dynamic_cast<agtk::Object::ConnectObjectDispPriority *>(ref);
#endif
			//子オブジェクト
			agtk::Object *parentObject = p->getObject()->getOwnParentObject();
			agtk::Player *player = parentObject ? parentObject->getPlayer() : nullptr;
			bool bBackSide = false;
			if (parentObject != nullptr && player != nullptr) {
				bBackSide = player->getTimelineBackside(p->getObject()->getParentFollowConnectId());
			}
			//オブジェクト接続
			auto connectObject = dynamic_cast<agtk::ConnectObject *>(p->getObject());
			player = object->getPlayer();
			if (bBackSide == false && connectObject != nullptr && player != nullptr) {
				bBackSide = player->getTimelineBackside(connectObject->getConnectionId());
			}
			if (p->getLowerPriority() == false && bBackSide == false) {
				list = setDispObjectList(list, p->getObject());
			}
		}
		return list;
	};
#ifdef USE_REDUCE_RENDER_TEXTURE
	auto parent = this->getObjectSetNode();
#else
	auto parent = this;
#endif
	//シーンに直接配置した物理演算パーツ
	auto physicsObjectList = [](cocos2d::__Array *physicsList, cocos2d::Node *parent)->cocos2d::__Array * {
		auto list = cocos2d::__Array::create();
		cocos2d::Ref *ref;
		CCARRAY_FOREACH(physicsList, ref) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
			auto physics = static_cast<agtk::PhysicsBase *>(ref);
#else
			auto physics = dynamic_cast<agtk::PhysicsBase *>(ref);
#endif
			if (physics->getParent() == parent) {
				list->addObject(physics);
			}
		}
		return list;
	} (this->getPhysicsObjectList(), parent);

	auto gameType = projectData->getGameType();
	if (this->getType() == kTypeMenu && gameType == agtk::data::ProjectData::kGameTypeSideView) {
		std::vector<cocos2d::Node *> dispPriorityObjectList;
		CCARRAY_FOREACH(objectList, ref) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
			auto object = static_cast<agtk::Object *>(ref);
#else
			auto object = dynamic_cast<agtk::Object *>(ref);
#endif
			dispPriorityObjectList.push_back(object);
		}
		CCARRAY_FOREACH(physicsObjectList, ref) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
			auto physics = static_cast<agtk::PhysicsBase *>(ref);
#else
			auto physics = dynamic_cast<agtk::PhysicsBase *>(ref);
#endif
			dispPriorityObjectList.push_back(physics);
		}

		auto compareMenuObject = [&](cocos2d::Node *p1, cocos2d::Node *p2) -> bool {
			auto o1 = dynamic_cast<agtk::Object *>(p1);
			auto ph1 = dynamic_cast<agtk::PhysicsBase *>(p1);
			auto o2 = dynamic_cast<agtk::Object *>(p2);
			auto ph2 = dynamic_cast<agtk::PhysicsBase *>(p2);
			int prio1, prio2;
			if (o1) {
				prio1 = getMenuDispPriority(o1);
			}
			else {
				prio1 = getMenuPhysicsDispPriority(ph1);
			}
			if (o2) {
				prio2 = getMenuDispPriority(o2);
			}
			else {
				prio2 = getMenuPhysicsDispPriority(ph2);
			}
			if (prio1 < prio2) {
				return true;
			}
			else if (prio1 > prio2) {
				return false;
			}
			prio1 = 0;
			prio2 = 0;
			if (o1) {
				prio1 = o1->getInstanceId();
			}
			if (o2) {
				prio2 = o2->getInstanceId();
			}
			if (prio1 < prio2) {
				return true;
			}
			else if (prio1 > prio2) {
				return false;
			}
// #AGTK-NX
#if (CC_TARGET_PLATFORM == CC_PLATFORM_NX)
#else
			return (int)p1 < (int)p2;
#endif
		};

		std::sort(dispPriorityObjectList.begin(), dispPriorityObjectList.end(), compareMenuObject);

		for (auto node : dispPriorityObjectList) {
			auto object = dynamic_cast<agtk::Object *>(node);
			if (object) {
				dispObjectList = setDispObjectList(dispObjectList, object);
			}
			else {
				dispObjectList->addObject(node);
			}
		}
	} else
	switch (gameType) {
	case agtk::data::ProjectData::kGameTypeSideView: {
		std::vector<cocos2d::Node *> dispPriorityObjectList;
		CCARRAY_FOREACH(objectList, ref) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
			auto object = static_cast<agtk::Object *>(ref);
#else
			auto object = dynamic_cast<agtk::Object *>(ref);
#endif
			dispPriorityObjectList.push_back(object);
		}
		CCARRAY_FOREACH(physicsObjectList, ref) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
			auto physics = static_cast<agtk::PhysicsBase *>(ref);
#else
			auto physics = dynamic_cast<agtk::PhysicsBase *>(ref);
#endif
			dispPriorityObjectList.push_back(physics);
		}

		auto compareSideViewObject = [&](cocos2d::Node *p1, cocos2d::Node *p2) -> bool {
			auto o1 = dynamic_cast<agtk::Object *>(p1);
			auto ph1 = dynamic_cast<agtk::PhysicsBase *>(p1);
			auto o2 = dynamic_cast<agtk::Object *>(p2);
			auto ph2 = dynamic_cast<agtk::PhysicsBase *>(p2);
			int prio1, prio2;
			if (o1) {
				prio1 = getDispPriority(o1);
			}
			else {
				prio1 = getPhysicsDispPriority(ph1);
			}
			if (o2) {
				prio2 = getDispPriority(o2);
			}
			else {
				prio2 = getPhysicsDispPriority(ph2);
			}
			if (prio1 < prio2) {
				return true;
			}
			else if (prio1 > prio2) {
				return false;
			}
			prio1 = 0;
			prio2 = 0;
			if (o1) {
				prio1 = o1->getInstanceId();
			}
			if (o2) {
				prio2 = o2->getInstanceId();
			}
			if (prio1 < prio2) {
				return true;
			}
			else if (prio1 > prio2) {
				return false;
			}
// #AGTK-NX
#if (CC_TARGET_PLATFORM == CC_PLATFORM_NX)
#else
			return (int)p1 < (int)p2;
#endif

		};

		std::sort(dispPriorityObjectList.begin(), dispPriorityObjectList.end(), compareSideViewObject);

		for (auto node : dispPriorityObjectList) {
			auto object = dynamic_cast<agtk::Object *>(node);
			if (object) {
				dispObjectList = setDispObjectList(dispObjectList, object);
			}
			else {
				dispObjectList->addObject(node);
			}
		}
		break; }
	case agtk::data::ProjectData::kGameTypeTopView: {
		std::vector<cocos2d::Node *> dispPriorityObjectList;
		CCARRAY_FOREACH(objectList, ref) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
			auto object = static_cast<agtk::Object *>(ref);
#else
			auto object = dynamic_cast<agtk::Object *>(ref);
#endif
			dispPriorityObjectList.push_back(object);
		}
		CCARRAY_FOREACH(physicsObjectList, ref) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
			auto physics = static_cast<agtk::PhysicsBase *>(ref);
#else
			auto physics = dynamic_cast<agtk::PhysicsBase *>(ref);
#endif
			dispPriorityObjectList.push_back(physics);
		}

		auto screenHeight = projectData->getScreenHeight();
		auto sceneData = this->getSceneData();
		auto sceneHeight = projectData->getScreenHeight() * sceneData->getVertScreenCount();
		auto compareTopView = [&](cocos2d::Node *p1, cocos2d::Node *p2) -> bool {
			auto o1 = dynamic_cast<agtk::Object *>(p1);
			auto ph1 = dynamic_cast<agtk::PhysicsBase *>(p1);
			auto o2 = dynamic_cast<agtk::Object *>(p2);
			auto ph2 = dynamic_cast<agtk::PhysicsBase *>(p2);
			int prio1, prio2;
			if (o1) {
				prio1 = getTopviewDispPriority(o1);
			}
			else {
				prio1 = getTopviewPhysicsDispPriority(ph1);
			}
			if (o2) {
				prio2 = getTopviewDispPriority(o2);
			}
			else {
				prio2 = getTopviewPhysicsDispPriority(ph2);
			}
			if (prio1 < prio2) {
				return true;
			}
			else if (prio1 > prio2) {
				return false;
			}
			float y1, y2;
			if (o1) {
				y1 = o1->getDispPosition().y;
			}
			else {
				y1 = sceneHeight - ph1->getPosition().y;
			}
			if (o2) {
				y2 = o2->getDispPosition().y;
			}
			else {
				y2 = sceneHeight - ph2->getPosition().y;
			}
			if (y1 < y2) {
				return true;
			}
			if (y1 > y2) {
				return false;
			}
// #AGTK-NX
#if (CC_TARGET_PLATFORM == CC_PLATFORM_NX)
#else
			return (int)p1 < (int)p2;
#endif

		};

		std::sort(dispPriorityObjectList.begin(), dispPriorityObjectList.end(), compareTopView);

		for (auto node : dispPriorityObjectList) {
			auto object = dynamic_cast<agtk::Object *>(node);
			if (object) {
				dispObjectList = setDispObjectList(dispObjectList, object);
			}
			else {
				dispObjectList->addObject(node);
			}
		}
		break; }
	}
#ifndef USE_SAR_TEST_0
	CC_ASSERT(this->getObjectList()->count() + physicsObjectList->count() == dispObjectList->count());
#endif
	int zOrder = 1;
	CCARRAY_FOREACH(dispObjectList, ref) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
		auto node = static_cast<cocos2d::Node *>(ref);
#else
		auto node = dynamic_cast<cocos2d::Node *>(ref);
#endif
		parent->reorderChild(node, zOrder++);
	}
}

void SceneLayer::earlyUpdate(float delta)
{
	auto objectList = this->getObjectList();

	//update object
	if (objectList) {
		for (int i = 0; i < objectList->count(); i++) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
			auto object = static_cast<agtk::Object *>(objectList->getObjectAtIndex(i));
#else
			auto object = dynamic_cast<agtk::Object *>(objectList->getObjectAtIndex(i));
#endif
			object->earlyUpdate(delta);
		}
	}
}


void SceneLayer::lateUpdate(float delta)
{
	auto objectList = this->getObjectList();

	//update object
	if (objectList) {
// #AGTK-NX
#ifdef USE_SAR_OPTIMIZE_4
		std::vector<agtk::Object*> tmpObjectList;
		tmpObjectList.reserve(objectList->count());
		for (int i = 0; i < objectList->count(); i++)
		{
			auto object = static_cast<agtk::Object *>(objectList->getObjectAtIndex(i));
			tmpObjectList.push_back(object);
		}
		for (int i = 0; i < tmpObjectList.size(); i++) {
			tmpObjectList[i]->lateUpdate(delta);
		}
#else
		auto tmpObjectList = cocos2d::__Array::create();
		lSkip:
		for (int i = 0; i < objectList->count(); i++) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
			auto object = static_cast<agtk::Object *>(objectList->getObjectAtIndex(i));
#else
			auto object = dynamic_cast<agtk::Object *>(objectList->getObjectAtIndex(i));
#endif
			if (tmpObjectList->containsObject(object)) {
				continue;
			}
			tmpObjectList->addObject(object);
			bool bRemove = false;
			if (object->getLateRemove()) {
				bRemove = true;
			}
			object->lateUpdate(delta);
			if (bRemove) {
				//オブジェクト削除の場合、objectListが更新されるため、再度更新する。その際更新済みのobjectは無視する。
				goto lSkip;
			}
		}
#endif
	}
}

void SceneLayer::loopVertical(bool fixedCamera)
{
	cocos2d::Ref *ref = nullptr;

	// オブジェクトの座標を変更する
	auto objectList = this->getObjectList();
	CCARRAY_FOREACH(objectList, ref) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
		auto object = static_cast<agtk::Object*>(ref);
#else
		auto object = dynamic_cast<agtk::Object*>(ref);
#endif
		object->loopVertical(fixedCamera);
	}
}

void SceneLayer::loopHorizontal(bool fixedCamera)
{
	cocos2d::Ref *ref = nullptr;

	// オブジェクトの座標を変更する
	auto objectList = this->getObjectList();
	CCARRAY_FOREACH(objectList, ref) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
		auto object = static_cast<agtk::Object*>(ref);
#else
		auto object = dynamic_cast<agtk::Object*>(ref);
#endif
		object->loopHorizontal(fixedCamera);
	}
}

/**
* 重なり演出付きレンダラー更新
* @param	delta						前フレームからの経過時間
* @param	viewMatrix					カメラのビュー行列
* @param	lowerLayerObjList			自身より奥のレイヤーのオブジェクトリスト
* @param	ignoreVisibleObject			オブジェクト表示除外フラグ
*/
void SceneLayer::updateRenderer(float delta, cocos2d::Mat4 *viewMatrix, cocos2d::__Array *lowerLayerObjList, bool ignoreVisibleObject)
{
	bool isRender = false;

	// 通常描画の表示切り替え
	// 最前面ONから最前面OFFにした場合にもこれがないと表示されない
	auto visibleFunc = [this, ignoreVisibleObject](bool visible) {
#ifdef USE_REDUCE_RENDER_TEXTURE
		auto tileMapNode = getTileMapNode();
		if (tileMapNode) {
			tileMapNode->setVisible(visible);
		}
		auto objectSetNode = getObjectSetNode();
		if (objectSetNode) {
			objectSetNode->setVisible(!ignoreVisibleObject && visible);
		}
		auto objectFrontNode = getObjectFrontNode();
		if (objectFrontNode) {
			objectFrontNode->setVisible(!ignoreVisibleObject && visible);
		}
		auto additiveParticleNode = getAdditiveParticleNode();
		if (additiveParticleNode) {
			additiveParticleNode->setVisible(visible);
		}
		auto additiveParticleBacksideNode = getAdditiveParticleBacksideNode();
		if (additiveParticleBacksideNode) {
			additiveParticleBacksideNode->setVisible(visible);
		}
#endif
	};

	// レンダーがある場合シェーダーを使用しているか判定をする。
	auto renderTexture = this->getRenderTexture();
	if (renderTexture) {
		// レンダー使用判定の更新
		auto objectList = getObjectList();
		renderTexture->updateIsRenderTile(lowerLayerObjList, ignoreVisibleObject);
		renderTexture->updateIsRenderObj(lowerLayerObjList, ignoreVisibleObject);

		if (renderTexture->isUseShader()) {
			isRender = true;
		}
		else {
			// 非表示
			cocos2d::Ref *ref = nullptr;
			auto renderTextureList = renderTexture->getRenderTextureList();
			CCARRAY_FOREACH(renderTextureList, ref) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
				auto renderTexture = static_cast<agtk::RenderTexture *>(ref);
#else
				auto renderTexture = dynamic_cast<agtk::RenderTexture *>(ref);
#endif
				renderTexture->getSprite()->setVisible(false);
			}
		}
	}

	if (isRender) {
		setVisible(true);
		visibleFunc(false);

		// Zオーダーを基準に描画を行うようにリストを構築
		auto sortObjectList = cocos2d::Array::create();
		sortObjectList->addObjectsFromArray(this->getObjectList());
		sortObjectByLocalZOrder(sortObjectList);

		renderTexture->update(delta, viewMatrix, lowerLayerObjList, sortObjectList, this->getTileMapList(), ignoreVisibleObject);
	}
	else {
		// 通常描画の場合はここでgetIsVisible()を判定する
		// レンダーの場合レンダー処理で呼んでいるので不要
		bool isVisible = getIsVisible();
		setVisible(isVisible);
		visibleFunc(isVisible);

		auto objectList = getObjectList();
		if (objectList->count() >= 2) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
			auto obj = static_cast<cocos2d::Node *>(objectList->getObjectAtIndex(0));
#else
			auto obj = dynamic_cast<cocos2d::Node *>(objectList->getObjectAtIndex(0));
#endif
			if (obj->getParent()) {
				obj->getParent()->reorderChild(obj, obj->getLocalZOrder());
			}
		}
	}

	if (getIsVisible()) {
		// 現在のシーンレイヤーのオブジェクトをリストに追加
		lowerLayerObjList->addObjectsFromArray(getObjectList());
	}
}

void SceneLayer::createShader(cocos2d::Layer *layer)
{
	auto projectData = GameManager::getInstance()->getProjectData();
	auto renderTextureCtrl = this->getRenderTexture();
	CC_ASSERT(renderTextureCtrl);
	renderTextureCtrl->setLayer(layer);

	std::function<void(agtk::data::SceneFilterEffectData *)> createFilterEffect = [&](agtk::data::SceneFilterEffectData *data) {
		if (data->getDisabled()) {
			return;
		}
		auto filterEffect = data->getFilterEffect();
		float seconds = (float)filterEffect->getDuration300() / 300.0f;
		switch (filterEffect->getEffectType()) {
		case agtk::data::FilterEffect::kEffectNoise: {//ノイズ
			renderTextureCtrl->addShader(agtk::Shader::kShaderNoisy, (float)filterEffect->getNoise() / 100.0f, seconds);
			break; }
		case agtk::data::FilterEffect::kEffectMosaic: {//モザイク
			renderTextureCtrl->addShader(agtk::Shader::kShaderMosaic, (float)filterEffect->getMosaic() / 100.0f, seconds);
			break; }
		case agtk::data::FilterEffect::kEffectMonochrome: {//モノクロ
			renderTextureCtrl->addShader(agtk::Shader::kShaderColorGray, (float)filterEffect->getMonochrome() / 100.0f, seconds);
			break; }
		case agtk::data::FilterEffect::kEffectSepia: {//セピア
			renderTextureCtrl->addShader(agtk::Shader::kShaderColorSepia, (float)filterEffect->getSepia() / 100.0f, seconds);
			break; }
		case agtk::data::FilterEffect::kEffectNegaPosiReverse: {//ネガ反転
			renderTextureCtrl->addShader(agtk::Shader::kShaderColorNega, (float)filterEffect->getNegaPosiReverse() / 100.0f, seconds);
			break; }
		case agtk::data::FilterEffect::kEffectDefocus: {//ぼかし
			renderTextureCtrl->addShader(agtk::Shader::kShaderBlur, (float)filterEffect->getDefocus() / 100.0f, seconds);
			break; }
		case agtk::data::FilterEffect::kEffectChromaticAberration: {//色収差
			renderTextureCtrl->addShader(agtk::Shader::kShaderColorChromaticAberration, (float)filterEffect->getChromaticAberration() / 100.0f, seconds);
			break; }
		case agtk::data::FilterEffect::kEffectDarkness: {//暗闇
			setIsShaderColorDarkMask(true);
			auto shader = renderTextureCtrl->getShader(agtk::Shader::kShaderColorDarkMask);
			float value = (float)filterEffect->getDarkness() / 100.0f;
			if (shader == nullptr) {
				shader = renderTextureCtrl->addShader(agtk::Shader::kShaderColorDarkMask, value, seconds);
				auto texture2d = new cocos2d::Texture2D();
				auto sceneSize = this->getContentSize();
				auto sceneWidth = 8;
				auto sceneHeight = 8;
				auto buf = new unsigned char[sceneWidth * sceneHeight * 4];
				memset(buf, 0, sceneWidth * sceneHeight * 4);
				texture2d->initWithData(buf, sceneWidth * sceneHeight * 4, Texture2D::PixelFormat::RGBA8888, sceneWidth, sceneHeight, cocos2d::Size(sceneWidth, sceneHeight));
				delete[] buf;
				shader->setMaskTexture(texture2d);
			}
			else {
				shader->setIntensity(value, seconds);
			}
			break; }
		case agtk::data::FilterEffect::kEffectTransparency: {//透明
			renderTextureCtrl->addShader(agtk::Shader::kShaderTransparency, (float)filterEffect->getTransparency() / 100.0f, seconds);
			break; }
		case agtk::data::FilterEffect::kEffectBlink: {//点滅
			// 点滅はオブジェクトのみに実装。
			break; }
		case agtk::data::FilterEffect::kEffectDispImage: {//画像表示
			if (filterEffect->getImageId() >= 0) {
				auto projectData = GameManager::getInstance()->getProjectData();
				auto imageData = projectData->getImageData(filterEffect->getImageId());
				cocos2d::Size texSizeDef;

				// create texture2d
				auto texture2d = CreateTexture2D(imageData->getFilename(), (filterEffect->getImagePlacement() == agtk::data::FilterEffect::kPlacementTiling), nullptr, &texSizeDef.width, &texSizeDef.height);
				if (filterEffect->getImagePlacement() == agtk::data::FilterEffect::kPlacementTiling) {
					//タイリング
					Texture2D::TexParams tRepeatParams;
					tRepeatParams.magFilter = GL_LINEAR;
					tRepeatParams.minFilter = GL_LINEAR;
					tRepeatParams.wrapS = GL_REPEAT;
					tRepeatParams.wrapT = GL_REPEAT;
					texture2d->setTexParameters(tRepeatParams);
				}
				else {
					texture2d->setAliasTexParameters();
				}

				auto sceneSize = projectData->getSceneSize(this->getSceneData());
				auto shader = renderTextureCtrl->addShader(agtk::Shader::kShaderImage, sceneSize, (float)(100.0f - filterEffect->getImageTransparency()) / 100.0f, seconds);
				shader->setMaskTexture(texture2d);
				// set imgPlacement
				int imgPlacement = filterEffect->getImagePlacement();
				auto programState = shader->getProgramState();
				programState->setUniformInt("imgPlacement", imgPlacement);
				// set resolution
				float width = sceneSize.width;
				float height = sceneSize.height;
				programState->setUniformVec2("resolution", cocos2d::Vec2(width, height));
				// set imgResolution
				auto imgResolution = (imgPlacement == 2) ? Vec2(texture2d->getContentSize().width, texture2d->getContentSize().height) : Vec2(width, height);
				programState->setUniformVec2("imgResolution", imgResolution);
				// set imgSizeRate
				auto imgSizeRate = cocos2d::Vec2(texSizeDef.width / texture2d->getContentSize().width, texSizeDef.height / texture2d->getContentSize().height);
				programState->setUniformVec2("imgSizeRate", imgSizeRate);
				// set sxy
				float imgSourceWidth = texSizeDef.width;
				float imgSourceHeight = texSizeDef.height;
				auto sxy = (imgPlacement == 3) ? ((width / imgSourceWidth <= height / imgSourceHeight) ? Vec2(1, height / imgSourceHeight * imgSourceWidth / width) : Vec2(width / imgSourceWidth * imgSourceHeight / height, 1)) : (imgSourceWidth > 0 && imgSourceHeight > 0 ? Vec2(width / imgSourceWidth, height / imgSourceHeight) : Vec2(1, 1));
				programState->setUniformVec2("sxy", sxy);
				// set imgXy
				auto imgXy = Vec2((1 - sxy.x) / 2, (1 - sxy.y) / 2);
				programState->setUniformVec2("imgXy", imgXy);
				// set FilterEffect
				shader->setUserData(filterEffect);
			}
			break; }
		case agtk::data::FilterEffect::kEffectFillColor: {//指定色で塗る
			auto color = cocos2d::Color4B(filterEffect->getFillR(), filterEffect->getFillG(), filterEffect->getFillB(), filterEffect->getFillA());
			renderTextureCtrl->addShader(agtk::Shader::kShaderColorRgba, 1.0f, seconds);
			auto shader = renderTextureCtrl->getShader(agtk::Shader::kShaderColorRgba);
			if (shader) {
				shader->setShaderRgbaColor(color);
			}
			break; }
		default:CC_ASSERT(0);
		}
	};

	//シーン効果
	auto sceneData = this->getSceneData();
	auto sceneFilterEffectList = sceneData->getSceneFilterEffectList();
	cocos2d::DictElement *el;
	CCDICT_FOREACH(sceneFilterEffectList, el) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
		auto filterEffectData = static_cast<agtk::data::SceneFilterEffectData *>(el->getObject());
#else
		auto filterEffectData = dynamic_cast<agtk::data::SceneFilterEffectData *>(el->getObject());
#endif
		if (filterEffectData->getLayerIndex() == agtk::data::SceneFilterEffectData::kLayerIndexAllSceneLayers ||
			filterEffectData->getLayerIndex() + 1 == this->getLayerId()) {//レイヤー or 全てのシーンレイヤー
			createFilterEffect(filterEffectData);
		}
	}
}

void SceneLayer::setShader(Shader::ShaderKind kind, float value, float seconds)
{
	auto renderTexture = this->getRenderTexture();
// #AGTK-NX
#if CC_TARGET_PLATFORM != CC_PLATFORM_NX
	if (renderTexture == false) {
#else
#endif
		return;
	}

	auto shader = this->getShader(kind);
	if (shader == nullptr) {
		if (kind == agtk::Shader::kShaderColorDarkMask) {
			setIsShaderColorDarkMask(true);
		}
		renderTexture->addShader(kind, value, seconds);
		this->setBlendAdditiveRenderTextureSprite();
	}
	else {
		shader->setIntensity(value, seconds);
		shader->setIgnored(false);//有効に。
	}
}

agtk::Shader *SceneLayer::getShader(Shader::ShaderKind kind)
{
	auto renderTexture = this->getRenderTexture();
// #AGTK-NX
#if CC_TARGET_PLATFORM != CC_PLATFORM_NX
	if (renderTexture == false) {
#else
#endif
		return nullptr;
	}

	auto shaderList = renderTexture->getShaderList();
	for (int idx = 0; idx < shaderList->count(); idx++) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
		auto shader = static_cast<agtk::Shader *>(shaderList->getObjectAtIndex(idx));
#else
		auto shader = dynamic_cast<agtk::Shader *>(shaderList->getObjectAtIndex(idx));
#endif
		if (shader->getKind() == kind) {
			return shader;
		}
	}
	return nullptr;
}

void SceneLayer::removeShader(Shader::ShaderKind kind, float seconds)
{
	auto renderTexture = this->getRenderTexture();
// #AGTK-NX
#if CC_TARGET_PLATFORM != CC_PLATFORM_NX
	if (renderTexture == false) {
#else
#endif
		return;
	}

	auto shader = this->getShader(kind);
	if (shader != nullptr) {
		shader->setIntensity(0.0f, seconds);
		auto value = shader->getValue();
		value->setEndFunc([&, shader]() {
			auto v = shader->getValue();
			if (v->getValue() == 0.0f) {
				this->getRemoveShaderList()->addObject(shader);
			}
		});
	}
}

void SceneLayer::updateShader()
{
	auto rtCtrl = this->getRenderTexture();
// #AGTK-NX
#if CC_TARGET_PLATFORM != CC_PLATFORM_NX
	if (rtCtrl == false) {
#else
#endif
		return;
	}

	// シェーダー削除
	auto removeShaderList = this->getRemoveShaderList();
	while (removeShaderList->count()) {
		cocos2d::Ref *ref;
		CCARRAY_FOREACH(removeShaderList, ref) {
			bool isRemove = false;

// #AGTK-NX
#ifdef STATIC_DOWN_CAST
			auto shader = static_cast<agtk::Shader *>(ref);
#else
			auto shader = dynamic_cast<agtk::Shader *>(ref);
#endif
			if (shader->getKind() != agtk::Shader::kShaderColorDarkMask) {
				isRemove = true;
			}
			else {
				setIsShaderColorDarkMask(false);

				auto viewportLightSceneLayer = _scene->getViewportLight()->getViewportLightSceneLayer(this->getLayerId());
				if (viewportLightSceneLayer->containsVisibleViewportLightSwitch() == false) {
					isRemove = true;
				}
			}

			if (isRemove) {
				rtCtrl->removeShader(shader);
			}
		}
		removeShaderList->removeAllObjects();
		this->setBlendAdditiveRenderTextureSprite();
	}
}

void SceneLayer::setBlendAdditiveRenderTextureSprite()
{
	if (_blendAdditiveFlag == false) {
		return;
	}
	auto renderTextureCtrl = this->getRenderTexture();
	if (renderTextureCtrl) {
		if (renderTextureCtrl->getLastRenderTexture()) {
			auto sprite = renderTextureCtrl->getLastRenderTexture()->getSprite();
			cocos2d::BlendFunc blend = { GL_SRC_ALPHA, GL_ONE };
			sprite->setBlendFunc(blend);
		}
	}
}

bool SceneLayer::isExist()
{
	//tile
	if (this->getTileMapList()) {
		cocos2d::Ref *ref = nullptr;
		CCARRAY_FOREACH(this->getTileMapList(), ref) {
			auto tileMap = dynamic_cast<agtk::TileMap*>(ref);
			if (tileMap && tileMap->getChildrenCount() > 0) {
				return true;
			}
		}
	}
	//object
	auto objectList = this->getObjectList();
	if (objectList && objectList->count() > 0) {
		return true;
	}
	return false;
}

void SceneLayer::setScale(float scaleX, float scaleY)
{
	Node::setScale(scaleX, scaleY);
}

void SceneLayer::addCollisionDetection()
{
	//object
	auto objectList = this->getObjectList();
	if (objectList) {
		cocos2d::Ref *ref;
		CCARRAY_FOREACH(objectList, ref) {
			auto object = dynamic_cast<agtk::Object *>(ref);
			if (object) {
				this->addCollisionDetaction(object);
			}
		}
	}
}

void SceneLayer::addCollisionDetaction(agtk::Object *object)
{
	auto objectData = object->getObjectData();
	CollisionDetaction *wallCollisionDetection = getGroupWallCollisionDetection(objectData->getGroup());
	if (wallCollisionDetection) {
		auto cc = CollisionComponent::create(wallCollisionDetection, CollisionComponent::kGroupWall);
		if (object->getComponent(cc->getName()) != nullptr) {
			object->removeComponent(cc->getName());
		}
		object->addComponent(cc);
	}
	{
		CollisionDetaction *collisionDetection = getGroupCollisionDetection(objectData->getGroup());
		if (collisionDetection) {
			auto cc = CollisionComponent::create(collisionDetection, CollisionComponent::kGroupHit);
			if (object->getComponent(cc->getName()) != nullptr) {
				object->removeComponent(cc->getName());
			}
			object->addComponent(cc);
		}
	}
	if(objectData->getCollideWithObjectGroupBit()){
		CollisionDetaction *wallCollisionDetection = getGroupRoughWallCollisionDetection(objectData->getGroup());
		if (wallCollisionDetection) {
			auto cc = CollisionComponent::create(wallCollisionDetection, CollisionComponent::kGroupRoughWall);
			if (object->getComponent(cc->getName()) != nullptr) {
				object->removeComponent(cc->getName());
			}
			object->addComponent(cc);
		}
	}
	object->unscheduleUpdate();//※addComponetでupdateが呼ばれるので、ループ一元化のためここで破棄する。
}

void SceneLayer::addObject(agtk::Object *object)
{
#ifdef USE_REDUCE_RENDER_TEXTURE
	_objectSetNode->addChild(object);
#else
	this->addChild(object);
#endif
	this->getObjectList()->addObject(object);
	this->getCreateObjectList()->addObject(object);
// #AGTK-NX
#ifdef USE_SAR_OPTIMIZE_1
	this->addIdMap(object);
#endif
#ifdef USE_SAR_OPTIMIZE_4
	object->registerSwitchWatcher();
#endif
}

void SceneLayer::changeRunningNode::insertChildHelper(Node* child, int index, int localZOrder, int tag, const std::string &name, bool setTag)
{
	auto assertNotSelfChild
	([this, child]() -> bool
	{
		for (Node* parent(getParent()); parent != nullptr;
			parent = parent->getParent())
			if (parent == child)
				return false;

		return true;
	});
	(void)assertNotSelfChild;

	CCASSERT(assertNotSelfChild(),
		"A node cannot be the child of his own children");

	if (_children.empty())
	{
		this->childrenAlloc();
	}

	//insert child
	{
#if CC_ENABLE_GC_FOR_NATIVE_OBJECTS
		auto sEngine = ScriptEngineManager::getInstance()->getScriptEngine();
		if (sEngine)
		{
			sEngine->retainScriptObject(this, child);
		}
#endif // CC_ENABLE_GC_FOR_NATIVE_OBJECTS
		_transformUpdated = true;
		_reorderChildDirty = true;
		_children.insert(index, child);
		child->_setLocalZOrder(localZOrder);
	}

	if (setTag)
		child->setTag(tag);
	else
		child->setName(name);

	child->setParent(this);

	child->updateOrderOfArrival();

	if (_running)
	{
		child->onEnter();
		// prevent onEnterTransitionDidFinish to be called twice when a node is added in onEnter
		if (_isTransitionFinished)
		{
			child->onEnterTransitionDidFinish();
		}
	}

	if (_cascadeColorEnabled)
	{
		updateCascadeColor();
	}

	if (_cascadeOpacityEnabled)
	{
		updateCascadeOpacity();
	}
}

void SceneLayer::changeRunningNode::insertChildByIndex(Node *child, int localZOrder, int index)
{
	this->insertChildHelper(child, index, localZOrder, INVALID_TAG, child->getName(), false);
}

void SceneLayer::insertObject(agtk::Object *object, agtk::Object *targetObject)
{
#ifdef USE_REDUCE_RENDER_TEXTURE
	auto children = _objectSetNode->getChildren();
	int id = children.size();
	for (int i = 0; i < children.size(); i++) {
		auto child = children.at(i);
		if (child == dynamic_cast<cocos2d::Node *>(targetObject)) {
			id = i;
			break;
		}
	}
	_objectSetNode->insertChildByIndex(object, object->getLocalZOrder(), id + 1);
#else
	this->addChild(object);
#endif
	int index = this->getObjectList()->getIndexOfObject(targetObject);
	this->getObjectList()->insertObject(object, index + 1);
	index = this->getCreateObjectList()->getIndexOfObject(targetObject);
	this->getCreateObjectList()->insertObject(object, index + 1);
// #AGTK-NX
#ifdef USE_REDUCE_RENDER_TEXTURE
#ifdef USE_SAR_OPTIMIZE_1
	this->addIdMap(object);
#endif
#ifdef USE_SAR_OPTIMIZE_4
	object->registerSwitchWatcher();
#endif
#endif
}

void SceneLayer::removeObject(agtk::Object *object, bool bIgnoredReappearCondition, unsigned int removeOption, bool bIgnoredRemoveObjectList)
{
	bool bConnectObjectFlag = dynamic_cast<agtk::ConnectObject *>(object) ? true : false;//接続されているオブジェクト。
	auto objectData = object->getObjectData();
	if (bConnectObjectFlag || bIgnoredReappearCondition) {
		//接続されているオブジェクトは復活条件を無視。
		goto lSkip;
	}
	// 復活条件に応じて処理を変える
	switch (objectData->getReappearCondition()) {
	case agtk::data::ObjectData::EnumReappearCondition::kReappearConditionCameraFar: {

		// 復活するための情報データをリストに保持する
		auto reappearData = object->getObjectReappearData();
		if (reappearData) {
			reappearData->setReappearFlag(false);//復活のためのフラグを初期化。
			auto deleteObjectList = this->getDeleteObjectList();
			if (deleteObjectList->containsObject(reappearData) == false) {
				this->getDeleteObjectList()->addObject(reappearData);
			}
		}

	} break;

	case agtk::data::ObjectData::EnumReappearCondition::kReappearConditionSceneChange: {

		// 復活するための情報データをリストに保持する
		auto reappearData = object->getObjectReappearData();
		if (reappearData) {

			// シーンIDを設定する
			reappearData->setSceneId(this->getSceneData()->getId());

			// シーン切り替えで復活する消滅済みオブジェクトのリストへ追加する
			auto sceneChangeReappearObjectList = GameManager::getInstance()->getSceneChangeReappearObjectList();
			if (!sceneChangeReappearObjectList->containsObject(reappearData)) {
				sceneChangeReappearObjectList->addObject(reappearData);
			}
		}

	} break;

	case agtk::data::ObjectData::EnumReappearCondition::kReappearConditionByCommand: {

		auto gm = GameManager::getInstance();
		// シーン開始時に配置されるオブジェクトの場合、
		// シーン遷移時に生成できないようにするためにデータを保持する
		if (this->getSceneData()->getId() != agtk::data::SceneData::kMenuSceneId && !gm->isSceneChanging()) {

			auto reappearData = object->getObjectReappearData();
			if (reappearData) {
				// シーンIDを設定
				reappearData->setSceneId(this->getSceneData()->getId());
				// シーンパーツIDを設定
				reappearData->setScenePartsId(object->getScenePartsId());

				// 「アクションで復活」オブジェクトのリストへ追加する
				auto commandReappearObjectList = gm->getCommandReappearObjectList();
				if (!commandReappearObjectList->containsObject(reappearData)) {
					commandReappearObjectList->addObject(reappearData);
				}
			}
		}

	} break;

	case agtk::data::ObjectData::EnumReappearCondition::kReappearConditionNone: {
		if (_scene->getScenePhase() != agtk::Scene::kPhaseEnd) {//シーン終了時以外。
			// 復活させない場合はデータを保持。
			if (object->getScenePartsId() > 0 && this->getSceneData()->getId() != agtk::data::SceneData::kMenuSceneId) {
				auto reappearData = object->getObjectReappearData();
				if (reappearData) {
					// シーンIDを設定
					//reappearData->setSceneId(this->getSceneData()->getId());
					// シーンパーツIDを設定
					reappearData->setScenePartsId(object->getScenePartsId());

					// 復活できないオブジェクトのリストへ追加する
					auto notReappearObjectList = GameManager::getInstance()->getNotReappearObjectList();
					if (!notReappearObjectList->containsObject(reappearData)) {
						notReappearObjectList->addObject(reappearData);
					}
				}
			}
		}
	} break;
	}

lSkip:

// #AGTK-NX
#ifdef USE_SAR_OPTIMIZE_1
	this->setIsObjectListUpdated(true);
	this->removeIdMap(object);
#endif
// #AGTK-NX
#ifdef USE_SAR_OPTIMIZE_4
	object->unregisterSwitchWatcher();
#endif

	if (object->getRemoveLayerMoveFlag()) {
		// レイヤー移動するオブジェクトに接続している子がいる場合、別レイヤーに移動することによって表示の優先度がなくなる
		cocos2d::Ref *ref = nullptr;
		CCARRAY_FOREACH(object->getConnectObjectList(), ref) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
			auto connectObj = static_cast<agtk::ConnectObject *>(ref);
#else
			auto connectObj = dynamic_cast<agtk::ConnectObject *>(ref);
#endif
			auto connectBaseObject = connectObj->getConnectBaseObject();
			connectBaseObject->removeConnectObjectDispPriority(connectObj);
		}
	}
	else {
		//ポータル移動対象オブジェクトのチェック。
		bool bPortalObject = GameManager::getInstance()->checkPortalTouched(object);
		if (bPortalObject == false) {
			// 全ての接続オブジェクトとの接続を無効化する
			object->unconnectAllConnectObject();
		}
	}

	// 残像を除去
	object->removeAfterimage();

	object->finalize(removeOption);
	object->removeAllComponents();
	//object->removeDetactionWallCollision();
	if (bIgnoredRemoveObjectList) {
		this->getObjectList()->removeObject(object);
	}
	this->getCreateObjectList()->removeObject(object);
#ifdef USE_REDUCE_RENDER_TEXTURE
	_objectSetNode->removeChild(object);
#else
	this->removeChild(object);
#endif

// #AGTK-NX CollisionNodeの登録・削除タイミングをCollisionComponentのonEnter,onExitに移行
#ifndef USE_SAR_OPTIMIZE_5
	//※initForSingleでm_spaceArrayを呼べばリフレッシュされるっぽい？このタイミングでobjectがリリースされます。
	CollisionDetaction *wallCollisionDetection = getGroupWallCollisionDetection(objectData->getGroup());
	if (wallCollisionDetection) {
		wallCollisionDetection->initForSingle();
	}
#endif
	//ゲームスピード
	_scene->getGameSpeed()->removeObject(object);
}

// #AGTK-NX
#ifdef USE_SAR_OPTIMIZE_1
void SceneLayer::addIdMap(agtk::Object* object)
{
	this->setIsObjectListUpdated(true);
	auto objectData = object->getObjectData();
	if (objectData)
	{
		_objectIdMap.insert(std::pair<int, agtk::Object*>(objectData->getId(), object));
	}
	_instanceIdMap.insert(std::pair<int, agtk::Object*>(object->getInstanceId(), object));
}

void SceneLayer::removeIdMap(agtk::Object* object)
{
	auto objectData = object->getObjectData();
	if (objectData)
	{
		auto r = _objectIdMap.equal_range(objectData->getId());
		for (auto it = r.first; it != r.second; it++)
		{
			if (it->second == object)
			{
				_objectIdMap.erase(it);
				break;
			}
		}
	}
	_instanceIdMap.erase(object->getInstanceId());
}
#endif

// #AGTK-NX
#ifdef USE_SAR_OPTIMIZE_2
cocos2d::__Array *SceneLayer::getObjectAll(int objectId)
{
	auto arr = cocos2d::__Array::create();
	auto r = _objectIdMap.equal_range(objectId);
	for (auto it = r.first; it != r.second; it++)
	{
		arr->addObject(it->second);
	}
	return arr;
}

agtk::Object *SceneLayer::getObjectAll(int objectId, int instanceId)
{
	auto arr = cocos2d::__Array::create();
	if (objectId < 0)
	{
		if (instanceId < 0)
		{
			auto it = _instanceIdMap.begin();
			if (it != _instanceIdMap.end())
				return it->second;
		}
		else
		{
			auto it = _instanceIdMap.find(instanceId);
			if (it != _instanceIdMap.end())
				return it->second;
		}
	}
	else
	{
		if (instanceId < 0)
		{
			auto it = _objectIdMap.find(objectId);
			if (it != _objectIdMap.end())
				return it->second;
		}
		else
		{
			auto it = _instanceIdMap.find(instanceId);
			if (it != _instanceIdMap.end()) {
				if (it->second->getObjectData()->getId() == objectId) {
					return it->second;
				}
			}
		}
	}
	return nullptr;
}
#endif


void SceneLayer::appearObject(agtk::data::ScenePartObjectData *scenePartObjectData)
{
#if defined(USE_RUNTIME)
	auto projectData = GameManager::getInstance()->getProjectData();
	if (projectData->getObjectData(scenePartObjectData->getObjectId())->getTestplayOnly()) {
		return;
	}
#endif
	auto viewportLightSceneLayer = _scene->getViewportLight()->getViewportLightSceneLayer(this->getLayerId());
	// オブジェクトを生成
	auto object = agtk::Object::createWithSceneDataAndScenePartObjectData(this, scenePartObjectData, -1);
	object->setId(objectId++);
	object->setLayerId(this->getLayerId());
	object->setPhysicsBitMask(this->getLayerId(), this->getSceneData()->getId());
	object->setScenePartsId(scenePartObjectData->getId());

	//「シーンに配置された同インスタンス数」を増やす。
	this->getScene()->incrementObjectInstanceCount(object->getObjectData()->getId());

	// 付属する物理オブジェクトを生成
	this->createPhysicsObjectWithObject(object);

	// 当たり判定設定
	addCollisionDetaction(object);
	auto objectData = object->getObjectData();
	if (viewportLightSceneLayer && objectData->getViewportLightSettingFlag() && objectData->getViewportLightSettingList()->count() > 0) {
		auto viewportLightObject = ViewportLightObject::create(object, _scene->getViewportLight(), this);
		viewportLightSceneLayer->getViewportLightObjectList()->addObject(viewportLightObject);
	}
	addObject(object);
}

void SceneLayer::reappearObject(agtk::data::ObjectReappearData *reappearData)
{
#if defined(USE_RUNTIME)
	auto projectData = GameManager::getInstance()->getProjectData();
	if (projectData->getObjectData(reappearData->getObjectId())->getTestplayOnly()) {
		return;
	}
#endif
	CC_ASSERT(_scene);
	auto viewportLightSceneLayer = _scene->getViewportLight()->getViewportLightSceneLayer(this->getLayerId());

	// オブジェクトを生成
	auto object = agtk::Object::createReappearData(this, reappearData);
	object->setId(objectId++);
	object->setLayerId(this->getLayerId());
	object->setPhysicsBitMask(this->getLayerId(), this->getSceneData()->getId());
	object->setScenePartsId(reappearData->getScenePartsId());

	if (object->getScenePartObjectData()) {
		//ScenePartDataのインスタンスIDを設定する。
		object->getPlayObjectData()->setInstanceId(object->getScenePartObjectData()->getId());
	}
	else {
		object->getPlayObjectData()->setInstanceId(this->getScene()->getObjectInstanceId(object));
	}
	object->getPlayObjectData()->setInstanceCount(this->getScene()->incrementObjectInstanceCount(object->getObjectData()->getId()));
	this->getScene()->updateObjectInstanceCount(object->getObjectData()->getId());

	// 付属する物理オブジェクトを生成
	this->createPhysicsObjectWithObject(object);

	//復活データにScenePartDataのIDを設定する。
	object->getObjectReappearData()->setScenePartsId(reappearData->getScenePartsId());

	// 当たり判定設定
	addCollisionDetaction(object);
	auto objectData = object->getObjectData();
	if (viewportLightSceneLayer && objectData->getViewportLightSettingFlag() && objectData->getViewportLightSettingList()->count() > 0) {
		auto viewportLightObject = ViewportLightObject::create(object, _scene->getViewportLight(), this);
		viewportLightSceneLayer->getViewportLightObjectList()->addObject(viewportLightObject);
	}
	addObject(object);
}

agtk::Object *SceneLayer::createTakeoverStatesObject(agtk::ObjectTakeoverStatesData *data, agtk::data::ScenePartObjectData *scenePartObjectData)
{
	CC_ASSERT(_scene);

	// オブジェクトを生成
	auto object = agtk::Object::create(
		this,
		data->getObjectId(),
		data->getActionNo(),
		data->getPosition(),
		cocos2d::Vec2(scenePartObjectData->getScalingX() * 0.01f, scenePartObjectData->getScalingY() * 0.01f),
		scenePartObjectData->getRotation(),
		data->getDispDirection(),
		-1,
		-1,
		data->getTakeOverAnimMotionId()
	);
	object->setPrevObjectActionId(data->getPrevActionNo());
	object->getObjectMovement()->setDirection(data->getMoveDirection());
	object->setId(objectId++);
	object->setLayerId(this->getLayerId());
	object->setPhysicsBitMask(this->getLayerId(), this->getSceneData()->getId());
	object->setScenePartsId(data->getScenePartsId());
	object->setPlayObjectData(data->getPlayObjectData());
	object->setScenePartObjectData(scenePartObjectData);
	object->setTakeOverAnimMotionId(data->getTakeOverAnimMotionId());
	object->setObjectPosInCamera(data->getObjectPosInCamera());
	object->setSceneIdOfFirstCreated(data->getSceneIdOfFirstCreated());

	return object;
}

int SceneLayer::publishObjectId() 
{
	return objectId++;
}

bool SceneLayer::checkNotReappearObject(agtk::data::ScenePartObjectData *scenePartObjectData)
{
	auto notReappearObjectList = GameManager::getInstance()->getNotReappearObjectList();
	cocos2d::Ref* ref = nullptr;

	CCARRAY_FOREACH(notReappearObjectList, ref) {
		auto reappearData = dynamic_cast<agtk::data::ObjectReappearData*>(ref);

		if (reappearData) {

			// シーンIDとシーンパーツIDが一致した場合、復活条件なしのオブジェクトである
			if (reappearData->getSceneId() == this->getSceneData()->getId() &&
				reappearData->getScenePartsId() == scenePartObjectData->getId()) {
				return true;
			}
		}
	}

	return false;
}

bool SceneLayer::checkCommandReappearObject(agtk::data::ScenePartObjectData *scenePartObjectData)
{
	auto commandReappearObjectList = GameManager::getInstance()->getCommandReappearObjectList();
	cocos2d::Ref* ref = nullptr;

	CCARRAY_FOREACH(commandReappearObjectList, ref) {
		auto reappearData = dynamic_cast<agtk::data::ObjectReappearData*>(ref);

		if (reappearData) {

			// シーンIDとシーンパーツIDが一致した場合、復活条件なしのオブジェクトである
			if (reappearData->getSceneId() == this->getSceneData()->getId()
			&&  reappearData->getScenePartsId() == scenePartObjectData->getId()) {
				return true;
			}
		}
	}

	return false;
}

void SceneLayer::removeCommandReappearObject(agtk::data::ScenePartObjectData *scenePartObjectData)
{
	auto commandReappearObjectList = GameManager::getInstance()->getCommandReappearObjectList();
	cocos2d::Ref* ref;
	CCARRAY_FOREACH(commandReappearObjectList, ref) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
		auto reappearData = static_cast<agtk::data::ObjectReappearData*>(ref);
#else
		auto reappearData = dynamic_cast<agtk::data::ObjectReappearData*>(ref);
#endif
		// シーンIDとシーンパーツIDが一致した場合、復活条件なしのオブジェクトである
		if (reappearData->getSceneId() == this->getSceneData()->getId()
			&& reappearData->getScenePartsId() == scenePartObjectData->getId()) {
			commandReappearObjectList->removeObject(reappearData);
		}
	}
}

bool SceneLayer::checkSceneChangeReappearObject(agtk::data::ScenePartObjectData *scenePartObjectData)
{
	auto changeReappearObjectList = GameManager::getInstance()->getSceneChangeReappearObjectList();
	cocos2d::Ref* ref = nullptr;

	CCARRAY_FOREACH(changeReappearObjectList, ref) {
		auto reappearData = dynamic_cast<agtk::data::ObjectReappearData*>(ref);

		if (reappearData) {

			// シーンIDとシーンパーツIDが一致した場合、復活条件なしのオブジェクトである
			if (reappearData->getSceneId() == this->getSceneData()->getId()
				&& reappearData->getScenePartsId() == scenePartObjectData->getId()) {
				return true;
			}
		}
	}

	return false;
}

agtk::data::ScenePartObjectData *SceneLayer::getInitiallyPlacedScenePartObjectDataForReappearData(agtk::data::ObjectReappearData *reappearData)
{
	auto scenePartObjectList = this->getSceneData()->getScenePartObjectList(getLayerId() - 1);
	cocos2d::Ref *ref = nullptr;
	CCARRAY_FOREACH(scenePartObjectList, ref) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
		auto scenePartObjectData = static_cast<agtk::data::ScenePartObjectData *>(ref);
#else
		auto scenePartObjectData = dynamic_cast<agtk::data::ScenePartObjectData *>(ref);
#endif
		// シーンIDとシーンパーツIDが一致した場合、初期配置のオブジェクトである
		if (reappearData->getSceneId() == this->getSceneData()->getId()
			&& reappearData->getScenePartsId() == scenePartObjectData->getId()) {
			return scenePartObjectData;
		}
	}
	return nullptr;
}

void SceneLayer::removeSceneChangeReappearObject(agtk::data::ScenePartObjectData *scenePartObjectData)
{
	auto changeReappearObjectList = GameManager::getInstance()->getSceneChangeReappearObjectList();
	cocos2d::Ref* ref;
	CCARRAY_FOREACH(changeReappearObjectList, ref) {
		auto reappearData = dynamic_cast<agtk::data::ObjectReappearData*>(ref);
		if (reappearData) {
			// シーンIDとシーンパーツIDが一致した場合、復活条件なしのオブジェクトである
			if (reappearData->getSceneId() == this->getSceneData()->getId() && reappearData->getScenePartsId() == scenePartObjectData->getId()) {
				changeReappearObjectList->removeObject(reappearData);
				break;
			}
		}
	}
}

bool SceneLayer::checkObjectAppearCondition(agtk::data::ScenePartObjectData *scenePartObjectData)
{
	CC_ASSERT(scenePartObjectData);

	auto projectData = GameManager::getInstance()->getProjectData();
	CC_ASSERT(projectData);
	auto objectData = projectData->getObjectData(scenePartObjectData->getObjectId());
	CC_ASSERT(objectData);

	// 出現条件の判定を行う
	switch (objectData->getAppearCondition()) {
		// 「カメラが近づいたら」
	case agtk::data::ObjectData::EnumAppearCondition::kAppearConditionCameraNear: {
		auto scene = GameManager::getInstance()->getCurrentScene();

		if (scene != nullptr) {

			int tileCnt = objectData->getAppearConditionTileCount();
			int width = projectData->getTileWidth() * tileCnt;
			int height = projectData->getTileHeight() * tileCnt;

			// オブジェクト生成前では生成対象の幅、高さを取得できないので
			// オブジェクトの生成位置を基に、
			// カメラの描画幅＋タイル枚数分の範囲内にいるかを判定する
			if (scene->getCamera()->isPositionScreenWithinCamera(
				cocos2d::Rect(
					scenePartObjectData->getX(), scenePartObjectData->getY(),
					0, 0
				),
				cocos2d::Vec2(width, height)
			)) {
				return true;
			}
			else {
				return false;
			}
		}
	} break;

		// 「常時出現」
	case agtk::data::ObjectData::EnumAppearCondition::kAppearConditionAlways: {
		// 何もしない
	} return true;
	}

	return false;
}

agtk::ObjectTakeoverStatesData* SceneLayer::getObjectTakeoverStatesAsScene(agtk::data::ScenePartObjectData *scenePartObjectData)
{
	auto sceneEndTakeoverStatesObjectList = GameManager::getInstance()->getSceneEndTakeoverStatesObjectList();
	cocos2d::Ref* ref = nullptr;

	CCARRAY_FOREACH(sceneEndTakeoverStatesObjectList, ref) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
		auto data = static_cast<agtk::ObjectTakeoverStatesData *>(ref);
#else
		auto data = dynamic_cast<agtk::ObjectTakeoverStatesData *>(ref);
#endif
		// シーンIDとシーンパーツIDが一致した場合。
		if (data->getSceneId() == this->getSceneData()->getId()
		&& data->getSceneLayerId() == this->getLayerId()
		&& data->getScenePartsId() == scenePartObjectData->getId()
		&& data->getObjectId() == scenePartObjectData->getObjectId()) {
			return data;
		}
	}
	return nullptr;
}

agtk::ObjectTakeoverStatesData* SceneLayer::getStartPointObjectTakeoverStatesAsScene(agtk::data::ScenePartObjectData *scenePartObjectData)
{
	auto sceneEndTakeoverStatesObjectList = GameManager::getInstance()->getSceneEndTakeoverStatesObjectList();
	cocos2d::Ref* ref = nullptr;

	CCARRAY_FOREACH(sceneEndTakeoverStatesObjectList, ref) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
		auto data = static_cast<agtk::ObjectTakeoverStatesData *>(ref);
#else
		auto data = dynamic_cast<agtk::ObjectTakeoverStatesData *>(ref);
#endif

		if (data->getSceneLayerId() == this->getLayerId()
			&& data->getScenePartsId() == scenePartObjectData->getId()
			&& data->getObjectId() == scenePartObjectData->getObjectId()) {
			return data;
		}

	}
	return nullptr;
}

bool SceneLayer::checkObjectTakeoverStatesAsScene(agtk::data::ScenePartObjectData *scenePartObjectData)
{
	return (this->getObjectTakeoverStatesAsScene(scenePartObjectData) != nullptr) ? true : false;
}

bool SceneLayer::checkStartPointObjectTakeoverStatesAsScene(agtk::data::ScenePartObjectData *scenePartObjectData)
{
	return (this->getStartPointObjectTakeoverStatesAsScene(scenePartObjectData) != nullptr) ? true : false;
}

bool SceneLayer::checkObjectDisappearCondition(agtk::Object* object)
{
	CC_ASSERT(object);

	auto projectData = GameManager::getInstance()->getProjectData();
	CC_ASSERT(projectData);

	auto objectData = object->getObjectData();
	CC_ASSERT(objectData);

	auto playObjectData = object->getPlayObjectData();
	CC_ASSERT(playObjectData);

	// 「カメラ外に出たら消滅」設定時
	if (objectData->getDisappearWhenOutOfCamera()) {
		auto scene = GameManager::getInstance()->getCurrentScene();

		if (scene != nullptr && scene->getSceneCreateSkipFrameFlag() == false) {

			int tileCnt = objectData->getDisappearCameraMarginTileCount();
			int width = projectData->getTileWidth() * tileCnt;
			int height = projectData->getTileHeight() * tileCnt;
			auto size = object->getContentSize();

			// オブジェクトの位置を基に
			// カメラの描画幅＋タイル枚数分の範囲外にいるかを判定する
			if (!scene->getCamera()->isPositionScreenWithinCamera(
				cocos2d::Rect(object->getLeftDownPosition(), size),
				cocos2d::Vec2(width, height)
			)) {
				return true;
			}
		}
	}

	// 「体力がなくなったら強制的に消滅」設定時、HPが0以下に設定された場合は消滅させる
	if (objectData->getDisappearWhenHp0() && playObjectData->getHp() <= 0) {
		return true;
	}

	return false;
}

bool SceneLayer::checkObjectReappearCondition(agtk::data::ObjectReappearData *reappearData)
{
	CC_ASSERT(reappearData);

	auto projectData = GameManager::getInstance()->getProjectData(); 
	CC_ASSERT(projectData);
	auto objectData = projectData->getObjectData(reappearData->getObjectId());
	CC_ASSERT(objectData);

	switch (objectData->getReappearCondition()) {
		// 「カメラ外に出たら」
	case agtk::data::ObjectData::EnumReappearCondition::kReappearConditionCameraFar: {
		auto scene = GameManager::getInstance()->getCurrentScene();

		if (scene != nullptr) {
			int tileCnt = objectData->getReappearConditionTileCount();
			int width = projectData->getTileWidth() * tileCnt;
			int height = projectData->getTileHeight() * tileCnt;

			// オブジェクト生成前では生成対象の幅、高さを取得できないので
			// オブジェクトの生成位置を基に、
			// カメラの描画幅＋タイル枚数分の範囲外にいるかを判定する
			bool bWithinCamera = scene->getCamera()->isPositionScreenWithinCamera(
				cocos2d::Rect(
					reappearData->getInitialPosition().x, reappearData->getInitialPosition().y,
					0, 0
				),
				cocos2d::Vec2(width, height));
			if (!bWithinCamera && reappearData->getReappearFlag() == false) {
				reappearData->setReappearFlag(true);
			}
			return bWithinCamera && reappearData->getReappearFlag();
		}
	} break;

		// 「シーンが切り替わるまで復活しない」
	case agtk::data::ObjectData::EnumReappearCondition::kReappearConditionSceneChange: {

		// リストから除去する
		GameManager::getInstance()->getSceneChangeReappearObjectList()->removeObject(reappearData);

		// シーンが切り替わってからしか呼ばれない想定なので、
		// そのまま生成を許可する
	} return true;

		// 「アクションで復活」
	case agtk::data::ObjectData::EnumReappearCondition::kReappearConditionByCommand: {
		// 他のアクション等で出現させるため、出現不可とする
	} return false;


		// 「無し」
	case agtk::data::ObjectData::EnumReappearCondition::kReappearConditionNone: {
		// 復活しないため、出現不可とする
	} return false;

	}

	return false;
}

void SceneLayer::createSceneSprite(int imageId, int opacity, cocos2d::Vec2 pos, float seconds)
{
	auto sceneSprite = agtk::SceneSprite::create(imageId, opacity, seconds);
	sceneSprite->setPosition(pos);
	this->getSceneSpriteList()->addObject(sceneSprite);
	this->addChild(sceneSprite);
}

void SceneLayer::removeSceneSprite(float seconds)
{
	auto sceneSpriteList = this->getSceneSpriteList();
	cocos2d::Ref *ref = nullptr;
	CCARRAY_FOREACH(sceneSpriteList, ref) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
		auto sceneSprite = static_cast<agtk::SceneSprite *>(ref);
#else
		auto sceneSprite = dynamic_cast<agtk::SceneSprite *>(ref);
#endif
		sceneSprite->getOpacityTimer()->setValue(0, seconds);
	}
}

/**
* Zオーダーを基準にオブジェクトのバブルソートを行う
*/
void SceneLayer::sortObjectByLocalZOrder(cocos2d::Array* objectList)
{
	// Zオーダーを基準にバブルソートを行う
	for (int i = 0; i < objectList->count() - 1; i++) {
		for (int j = objectList->count() - 1; j > i; j--) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
			auto obj = static_cast<agtk::Object *>(objectList->getObjectAtIndex(j));
			auto obj2 = static_cast<agtk::Object *>(objectList->getObjectAtIndex(j - 1));
#else
			auto obj = dynamic_cast<agtk::Object *>(objectList->getObjectAtIndex(j));
			auto obj2 = dynamic_cast<agtk::Object *>(objectList->getObjectAtIndex(j - 1));
#endif

			if (obj->getLocalZOrder() < obj2->getLocalZOrder()) {
				objectList->swap(j, j - 1);
			}
		}
	}
}

static void limitTileCoords(int &minTileX, int &maxTileX, int &minTileY, int &maxTileY, agtk::TileMap *tileMap)
{
	if (minTileX < 0) {
		minTileX = 0;
	}
	auto tileMapWidth = tileMap->getTileMapWidth();
	if (maxTileX >= tileMapWidth) {
		maxTileX = tileMapWidth;
	}
	if (minTileY < 0) {
		minTileY = 0;
	}
	auto tileMapHeight = tileMap->getTileMapHeiht();
	if (maxTileY >= tileMapHeight) {
		maxTileY = tileMapHeight;
	}
}

/**
 * 衝突するタイルリストの取得
 * @param	boundMin	衝突検出用
 * @param	boundMax	衝突検出用
 * @return				衝突すると判定されたタイルリスト
 */
#ifdef USE_MULTITHREAD_OBJECT_UPDATE
std::vector<agtk::Tile *> SceneLayer::getCollisionTileList(cocos2d::Point boundMin, cocos2d::Point boundMax, Vec2 moveVec)
#else
cocos2d::__Array *SceneLayer::getCollisionTileList(cocos2d::Point boundMin, cocos2d::Point boundMax, Vec2 moveVec)
#endif
{
#ifdef USE_MULTITHREAD_OBJECT_UPDATE
	std::vector<agtk::Tile *> arr;
#endif
	auto projectData = GameManager::getInstance()->getProjectData();
	auto scene = this->getScene();
	auto sceneData = scene->getSceneData();

#ifdef USE_TILE_COLLISION_FIX
	auto tileMapList = this->getTileMapList();
	if (tileMapList == nullptr) {
#ifdef USE_MULTITHREAD_OBJECT_UPDATE
		return arr;
#else
		return cocos2d::__Array::create();
#endif
	}

	cocos2d::Rect crntRect = Rect(boundMin, cocos2d::Size(boundMax - boundMin));
	cocos2d::Rect prevRect = Rect(boundMin - moveVec, crntRect.size);

	cocos2d::Rect rect = crntRect;
	rect.merge(prevRect);

	int tileWidth = projectData->getTileWidth();
	int tileHeight = projectData->getTileWidth();
	auto sceneSize = scene->getSceneSize();

	int minTileX = floor(rect.getMinX() / tileWidth);
	int maxTileX = floor(rect.getMaxX() / tileWidth);
	int minTileY = floor((sceneSize.y - rect.getMaxY()) / tileHeight);
	int maxTileY = floor((sceneSize.y - rect.getMinY()) / tileHeight);

#ifdef USE_MULTITHREAD_OBJECT_UPDATE
#else
	cocos2d::__Array *arr = cocos2d::__Array::create();
#endif
	cocos2d::Ref *ref = nullptr;
	CCARRAY_FOREACH(tileMapList, ref) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
		auto tileMap = static_cast<agtk::TileMap *>(ref);
#else
		auto tileMap = dynamic_cast<agtk::TileMap *>(ref);
#endif
		limitTileCoords(minTileX, maxTileX, minTileY, maxTileY, tileMap);	//ACT2-6410: Limit Tile Coords
		for (int j = minTileY; j <= maxTileY; j++) {
			for (int i = minTileX; i <= maxTileX; i++) {
				auto tile = tileMap->getTile(i, j);
				if (tile != nullptr) {
#ifdef USE_MULTITHREAD_OBJECT_UPDATE
					arr.push_back(tile);
#else
					arr->addObject(tile);
#endif
				}
			}
		}

		//行動制限無効
		if (!sceneData->getDisableLimitArea()) {
			//行動制限用タイル
			auto limitTileList = tileMap->getLimitTileList();
			cocos2d::Ref *ref2;
			CCARRAY_FOREACH(limitTileList, ref2) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
				auto limitTile = static_cast<agtk::LimitTile *>(ref2);
#else
				auto limitTile = dynamic_cast<agtk::LimitTile *>(ref2);
#endif
				if (limitTile->convertToWorldSpaceRect().intersectsRect(rect)) {
#ifdef USE_MULTITHREAD_OBJECT_UPDATE
					arr.push_back(limitTile);
#else
					arr->addObject(limitTile);
#endif
				}
			}
		}
	}
#else
	auto tileMapList = this->getTileMapList();
	if (tileMapList == nullptr) {
#ifdef USE_MULTITHREAD_OBJECT_UPDATE
		return arr;
#else
		return cocos2d::__Array::create();
#endif
	}

	int tileWidth = projectData->getTileWidth();
	int tileHeight = projectData->getTileWidth();
	auto sceneSize = scene->getSceneSize();
	int horzTileCount = (int)ceil(sceneSize.x / tileWidth);
	int vertTileCount = (int)ceil(sceneSize.y / tileHeight);
	int vertPadding = (int)sceneSize.y % (int)tileHeight;

#ifdef USE_MULTITHREAD_OBJECT_UPDATE
#else
	cocos2d::__Array *arr = cocos2d::__Array::create();
#endif


	cocos2d::Rect crntRect = Rect(boundMin, cocos2d::Size(boundMax - boundMin));
	cocos2d::Rect prevRect = Rect(boundMin - moveVec, crntRect.size);

	//cocos2d::Rect rect(boundMin, cocos2d::Size(boundMax - boundMin));
	cocos2d::Rect rect = crntRect;
	rect.merge(prevRect);
	cocos2d::Ref *ref = nullptr;
	CCARRAY_FOREACH(tileMapList, ref) {
		auto tileMap = dynamic_cast<agtk::TileMap *>(ref);
		for (int j = 0; j < tileMap->getTileMapHeiht(); j++) {
			for (int i = 0; i < tileMap->getTileMapWidth(); i++) {
				auto tile = tileMap->getTile(i, j);
				if (tile != nullptr) {
					if (tile->convertToWorldSpaceRect().intersectsRect(rect)) {
#ifdef USE_MULTITHREAD_OBJECT_UPDATE
						arr.push_back(tile);
#else
						arr->addObject(tile);
#endif
					}
				}
			}
		}
		//行動制限用タイル
		auto limitTileList = tileMap->getLimitTileList();
		cocos2d::Ref *ref2;
		CCARRAY_FOREACH(limitTileList, ref2) {
			auto limitTile = dynamic_cast<agtk::LimitTile *>(ref2);
			if (limitTile->convertToWorldSpaceRect().intersectsRect(rect)) {
#ifdef USE_MULTITHREAD_OBJECT_UPDATE
				arr.push_back(limitTile);
#else
				arr->addObject(limitTile);
#endif
			}
		}
	}
#endif
	return arr;
}

/**
* 衝突するタイルリストの取得（タイルに重なったら）
* @param	boundMin	衝突検出用
* @param	boundMax	衝突検出用
* @return				衝突すると判定されたタイルリスト
*/
#ifdef USE_MULTITHREAD_OBJECT_UPDATE
std::vector<agtk::Tile *> SceneLayer::getCollisionTileOverlapList(cocos2d::Point boundMin, cocos2d::Point boundMax)
#else
cocos2d::__Array *SceneLayer::getCollisionTileOverlapList(cocos2d::Point boundMin, cocos2d::Point boundMax)
#endif
{
#ifdef USE_MULTITHREAD_OBJECT_UPDATE
	std::vector<agtk::Tile *> arr;
#endif
	auto projectData = GameManager::getInstance()->getProjectData();
	auto scene = GameManager::getInstance()->getCurrentScene();

#ifdef USE_TILE_COLLISION_FIX
	auto tileMapList = this->getTileMapList();
	if (tileMapList == nullptr) {
#ifdef USE_MULTITHREAD_OBJECT_UPDATE
		return arr;
#else
		return cocos2d::__Array::create();
#endif
	}

	int tileWidth = projectData->getTileWidth();
	int tileHeight = projectData->getTileHeight();
	auto sceneSize = scene->getSceneSize();

	int minTileX = floor(boundMin.x / tileWidth) - 1;
	int maxTileX = floor(boundMax.x / tileWidth) + 1;
	int minTileY = floor((sceneSize.y - boundMax.y) / tileHeight) - 1;
	int maxTileY = floor((sceneSize.y - boundMin.y) / tileHeight) + 1;

	cocos2d::Rect rect(boundMin, cocos2d::Size(boundMax - boundMin));
#ifdef USE_MULTITHREAD_OBJECT_UPDATE
#else
	cocos2d::__Array *arr = cocos2d::__Array::create();
#endif
	cocos2d::Ref *ref = nullptr;
	CCARRAY_FOREACH(tileMapList, ref) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
		auto tileMap = static_cast<agtk::TileMap *>(ref);
#else
		auto tileMap = dynamic_cast<agtk::TileMap *>(ref);
#endif

		// 各タイルマップが所属するシーンレイヤーはスクロールする為スクロール分ずらしてチェックする
		//auto sceneLayerPos = ((agtk::SceneLayer *)tileMap->getParent())->getPosition();
		auto sceneLayerPos = this->getPosition();

		// X軸でスクロールしている場合
		if (sceneLayerPos.x != 0) {
			int num = sceneLayerPos.x / tileWidth;
			int mod = (int)sceneLayerPos.x % tileWidth;
			int add = (mod != 0 ? num < 0 ? -1 : 1 : 0);
			minTileX -= num + add;
			maxTileX -= num + add;
		}

		// Y軸でスクロールしている場合
		if (sceneLayerPos.y != 0) {
			int num = sceneLayerPos.y / tileHeight;
			int mod = (int)sceneLayerPos.y % tileHeight;
			int add = (mod != 0 ? num <= 0 ? -1 : 1 : 0);
			minTileY += num + add;
			maxTileY += num + add;
		}

		limitTileCoords(minTileX, maxTileX, minTileY, maxTileY, tileMap);	//ACT2-6410: Limit Tile Coords

		for (int j = minTileY; j <= maxTileY; j++) {
			for (int i = minTileX; i <= maxTileX; i++) {
				auto tile = tileMap->getTile2(i, j);
				if (tile != nullptr) {
					agtk::Vertex4 vertex4;
					tile->convertToLayerSpaceVertex4(vertex4);
					vertex4 += sceneLayerPos;
					if (vertex4.getRect().intersectsRect(rect)) {
#ifdef USE_MULTITHREAD_OBJECT_UPDATE
						arr.push_back(tile);
#else
						arr->addObject(tile);
#endif
					}
				}
			}
		}
	}
#else
	auto tileMapList = this->getTileMapList();
	if (tileMapList == nullptr) {
		return cocos2d::__Array::create();
	}

	int tileWidth = projectData->getTileWidth();
	int tileHeight = projectData->getTileWidth();
	auto sceneSize = scene->getSceneSize();
	int horzTileCount = (int)ceil(sceneSize.x / tileWidth);
	int vertTileCount = (int)ceil(sceneSize.y / tileHeight);
	int vertPadding = (int)sceneSize.y % (int)tileHeight;

#ifdef USE_MULTITHREAD_OBJECT_UPDATE
#else
	cocos2d::__Array *arr = cocos2d::__Array::create();
#endif

	cocos2d::Rect rect(boundMin, cocos2d::Size(boundMax - boundMin));
	cocos2d::Ref *ref = nullptr;
	CCARRAY_FOREACH(tileMapList, ref) {
		auto tileMap = dynamic_cast<agtk::TileMap *>(ref);
		for (int j = 0; j < tileMap->getTileMapHeiht(); j++) {
			for (int i = 0; i < tileMap->getTileMapWidth(); i++) {
				auto tile = tileMap->getTile2(i, j);
				if (tile != nullptr) {
					if (tile->convertToLayerSpaceRect().intersectsRect(rect)) {
#ifdef USE_MULTITHREAD_OBJECT_UPDATE
						arr.push_back(tile);
#else
						arr->addObject(tile);
#endif
					}
				}
			}
		}
	}
#endif
	return arr;
}

/**
* 衝突するタイルリストの取得（タイルに重なったら）
* @param	boundMin	衝突検出用
* @param	boundMax	衝突検出用
* @return				衝突すると判定されたタイルリスト
*/
#ifdef USE_MULTITHREAD_OBJECT_UPDATE
std::vector<agtk::Tile *> SceneLayer::getCollisionTileOverlapMaskList(cocos2d::Point boundMin, cocos2d::Point boundMax)
#else
cocos2d::__Array *SceneLayer::getCollisionTileOverlapMaskList(cocos2d::Point boundMin, cocos2d::Point boundMax)
#endif
{
#ifdef USE_MULTITHREAD_OBJECT_UPDATE
	std::vector<agtk::Tile *> arr;
#endif
	auto projectData = GameManager::getInstance()->getProjectData();
	auto scene = GameManager::getInstance()->getCurrentScene();

#ifdef USE_TILE_COLLISION_FIX
	auto tileMapList = this->getTileMapList();
	if (tileMapList == nullptr) {
#ifdef USE_MULTITHREAD_OBJECT_UPDATE
		return arr;
#else
		return cocos2d::__Array::create();
#endif
	}

	int tileWidth = projectData->getTileWidth();
	int tileHeight = projectData->getTileHeight();
	auto sceneSize = scene->getSceneSize();

	int minTileX = floor(boundMin.x / tileWidth) - 1;
	int maxTileX = floor(boundMax.x / tileWidth) + 1;
	int minTileY = floor((sceneSize.y - boundMin.y) / tileHeight) - 1;
	int maxTileY = floor((sceneSize.y - boundMax.y) / tileHeight) + 1;

	cocos2d::Rect rect(
		cocos2d::Point(boundMin.x, boundMin.y - (boundMin.y - boundMax.y)),
		cocos2d::Size(boundMax.x - boundMin.x, boundMin.y - boundMax.y));
#ifdef USE_MULTITHREAD_OBJECT_UPDATE
#else
	cocos2d::__Array *arr = cocos2d::__Array::create();
#endif
	cocos2d::Ref *ref = nullptr;
	CCARRAY_FOREACH(tileMapList, ref) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
		auto tileMap = static_cast<agtk::TileMap *>(ref);
#else
		auto tileMap = dynamic_cast<agtk::TileMap *>(ref);
#endif

		// 各タイルマップが所属するシーンレイヤーはスクロールする為スクロール分ずらしてチェックする
		//auto sceneLayerPos = ((agtk::SceneLayer *)tileMap->getParent())->getPosition();
		auto sceneLayerPos = this->getPosition();

		// X軸でスクロールしている場合
		if (sceneLayerPos.x != 0) {
			int num = sceneLayerPos.x / tileWidth;
			int mod = (int)sceneLayerPos.x % tileWidth;
			int add = (mod != 0 ? num < 0 ? -1 : 1 : 0);
			minTileX -= num + add;
			maxTileX -= num + add;
		}

		// Y軸でスクロールしている場合
		if (sceneLayerPos.y != 0) {
			int num = sceneLayerPos.y / tileHeight;
			int mod = (int)sceneLayerPos.y % tileHeight;
			int add = (mod != 0 ? num <= 0 ? -1 : 1 : 0);
			minTileY += num + add;
			maxTileY += num + add;
		}

		limitTileCoords(minTileX, maxTileX, minTileY, maxTileY, tileMap);	//ACT2-6410: Limit Tile Coords

		for (int j = minTileY; j <= maxTileY; j++) {
			for (int i = minTileX; i <= maxTileX; i++) {
				auto tile = tileMap->getTile2(i, j);
				if (tile != nullptr) {
					agtk::Vertex4 vertex4;
					tile->convertToLayerSpaceVertex4(vertex4);
					vertex4 += sceneLayerPos;
					if (vertex4.getRect().intersectsRect(rect)) {
#ifdef USE_MULTITHREAD_OBJECT_UPDATE
						arr.push_back(tile);
#else
						arr->addObject(tile);
#endif
					}
				}
			}
		}
	}
#else
	auto tileMapList = this->getTileMapList();
	if (tileMapList == nullptr) {
		return cocos2d::__Array::create();
	}

	int tileWidth = projectData->getTileWidth();
	int tileHeight = projectData->getTileWidth();
	auto sceneSize = scene->getSceneSize();
	int horzTileCount = (int)ceil(sceneSize.x / tileWidth);
	int vertTileCount = (int)ceil(sceneSize.y / tileHeight);
	int vertPadding = (int)sceneSize.y % (int)tileHeight;

#ifdef USE_MULTITHREAD_OBJECT_UPDATE
#else
	cocos2d::__Array *arr = cocos2d::__Array::create();
#endif

	cocos2d::Rect rect(boundMin, cocos2d::Size(boundMax - boundMin));
	cocos2d::Ref *ref = nullptr;
	CCARRAY_FOREACH(tileMapList, ref) {
		auto tileMap = dynamic_cast<agtk::TileMap *>(ref);
		for (int j = 0; j < tileMap->getTileMapHeiht(); j++) {
			for (int i = 0; i < tileMap->getTileMapWidth(); i++) {
				auto tile = tileMap->getTile2(i, j);
				if (tile != nullptr) {
					if (tile->convertToLayerSpaceRect().intersectsRect(rect)) {
#ifdef USE_MULTITHREAD_OBJECT_UPDATE
						arr.push_back(tile);
#else
						arr->addObject(tile);
#endif
					}
				}
			}
		}
	}
#endif
	return arr;
}
#ifdef USE_MULTITHREAD_OBJECT_UPDATE
std::vector<agtk::Tile *> SceneLayer::getCollisionTileList(agtk::Vertex4 &v)
#else
cocos2d::__Array *SceneLayer::getCollisionTileList(agtk::Vertex4 &v)
#endif
{
#ifdef USE_MULTITHREAD_OBJECT_UPDATE
	std::vector<agtk::Tile *> arr;
#endif
	auto projectData = GameManager::getInstance()->getProjectData();
	auto scene = this->getScene();
	auto sceneData = scene->getSceneData();

	auto tileMapList = this->getTileMapList();
	if (tileMapList == nullptr) {
#ifdef USE_MULTITHREAD_OBJECT_UPDATE
		return arr;
#else
		return cocos2d::__Array::create();
#endif
	}
	auto rect = v.getRect();

	int tileWidth = projectData->getTileWidth();
	int tileHeight = projectData->getTileWidth();
	auto sceneSize = scene->getSceneSize();

	int minTileX = floor(rect.getMinX() / tileWidth);
	int maxTileX = floor(rect.getMaxX() / tileWidth);
	int minTileY = floor((sceneSize.y - rect.getMaxY()) / tileHeight);
	int maxTileY = floor((sceneSize.y - rect.getMinY()) / tileHeight);

#ifdef USE_MULTITHREAD_OBJECT_UPDATE
#else
	auto arr = cocos2d::__Array::create();
#endif
	cocos2d::Ref *ref;
	CCARRAY_FOREACH(tileMapList, ref) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
		auto tileMap = static_cast<agtk::TileMap *>(ref);
#else
		auto tileMap = dynamic_cast<agtk::TileMap *>(ref);
#endif
		limitTileCoords(minTileX, maxTileX, minTileY, maxTileY, tileMap);	//ACT2-6410: Limit Tile Coords

		for (int j = minTileY; j <= maxTileY; j++) {
			for (int i = minTileX; i <= maxTileX; i++) {
				auto tile = tileMap->getTile(i, j);
				if (tile) {
					auto tileRect = cocos2d::Rect(tile->getPosition(), tile->getContentSize());
					if (agtk::Vertex4::intersectsRect(v, tileRect)) {
#ifdef USE_MULTITHREAD_OBJECT_UPDATE
						arr.push_back(tile);
#else
						arr->addObject(tile);
#endif
					}
				}
			}
		}

		//行動制限無効
		if (!sceneData->getDisableLimitArea()) {
			//行動制限用タイル
			auto limitTileList = tileMap->getLimitTileList();
			cocos2d::Ref *ref2;
			CCARRAY_FOREACH(limitTileList, ref2) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
				auto limitTile = static_cast<agtk::LimitTile *>(ref2);
#else
				auto limitTile = dynamic_cast<agtk::LimitTile *>(ref2);
#endif
				if (limitTile->convertToWorldSpaceRect().intersectsRect(rect)) {
#ifdef USE_MULTITHREAD_OBJECT_UPDATE
					arr.push_back(limitTile);
#else
					arr->addObject(limitTile);
#endif
				}
			}
		}
	}
	return arr;
}

#ifdef USE_MULTITHREAD_OBJECT_UPDATE
std::vector<agtk::Tile *> SceneLayer::getCollisionTile2List(agtk::Vertex4 &v)
#else
cocos2d::__Array *SceneLayer::getCollisionTile2List(agtk::Vertex4 &v)
#endif
{
#ifdef USE_MULTITHREAD_OBJECT_UPDATE
	std::vector<agtk::Tile *> arr;
#endif
	auto tileMapList = this->getTileMapList();
	if (tileMapList == nullptr) {
#ifdef USE_MULTITHREAD_OBJECT_UPDATE
		return arr;
#else
		return cocos2d::__Array::create();
#endif
	}

	auto projectData = GameManager::getInstance()->getProjectData();
	auto scene = GameManager::getInstance()->getCurrentScene();
	auto rect = v.getRect();

	int tileWidth = projectData->getTileWidth();
	int tileHeight = projectData->getTileWidth();
	auto sceneSize = scene->getSceneSize();

	int minTileX = floor(rect.getMinX() / tileWidth) - 1;
	int maxTileX = floor(rect.getMaxX() / tileWidth) + 1;
	int minTileY = floor((sceneSize.y - rect.getMaxY()) / tileHeight) - 1;
	int maxTileY = floor((sceneSize.y - rect.getMinY()) / tileHeight) + 1;

#ifdef USE_MULTITHREAD_OBJECT_UPDATE
#else
	auto arr = cocos2d::__Array::create();
#endif
	cocos2d::Ref *ref;
	CCARRAY_FOREACH(tileMapList, ref) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
		auto tileMap = static_cast<agtk::TileMap *>(ref);
#else
		auto tileMap = dynamic_cast<agtk::TileMap *>(ref);
#endif

		// 各タイルマップが所属するシーンレイヤーはスクロールする為スクロール分ずらしてチェックする
		auto sceneLayerPos = ((agtk::SceneLayer *)tileMap->getParent())->getPosition();
		
		// X軸でスクロールしている場合
		if (sceneLayerPos.x != 0) {
			int num = sceneLayerPos.x / tileWidth;
			int mod = (int)sceneLayerPos.x % tileWidth;
			int add = (mod != 0 ? num < 0 ? -1 : 1 : 0);
			minTileX -= num + add;
			maxTileX -= num + add;
		}
		
		// Y軸でスクロールしている場合
		if (sceneLayerPos.y != 0) {
			int num = sceneLayerPos.y / tileHeight;
			int mod = (int)sceneLayerPos.y % tileHeight;
			int add = (mod != 0 ? num < 0 ? -1 : 1 : 0);
			minTileY -= num + add;
			maxTileY -= num + add;
		}

		limitTileCoords(minTileX, maxTileX, minTileY, maxTileY, tileMap);	//ACT2-6410: Limit Tile Coords

		for (int j = minTileY; j <= maxTileY; j++) {
			for (int i = minTileX; i <= maxTileX; i++) {
				auto tile = tileMap->getTile2(i, j);
				if (tile != nullptr) {
					agtk::Vertex4 vertex4;
					tile->convertToLayerSpaceVertex4(vertex4);
					vertex4 += sceneLayerPos;
					if (agtk::Vertex4::intersectsVertex4(vertex4, v)) {
#ifdef USE_MULTITHREAD_OBJECT_UPDATE
						arr.push_back(tile);
#else
						arr->addObject(tile);
#endif
					}
				}
			}
		}
	}
	return arr;
}

#ifdef USE_MULTITHREAD_OBJECT_UPDATE
agtk::MtVector<agtk::Slope *> *SceneLayer::getCollisionSlopeList(cocos2d::Point boundMin, cocos2d::Point boundMax, Vec2 moveVec)
#else
cocos2d::__Array *SceneLayer::getCollisionSlopeList(cocos2d::Point boundMin, cocos2d::Point boundMax, Vec2 moveVec)
#endif
{
	auto projectData = GameManager::getInstance()->getProjectData();
	auto scene = GameManager::getInstance()->getCurrentScene();

#ifdef USE_MULTITHREAD_OBJECT_UPDATE
	auto arr = new agtk::MtVector<agtk::Slope *>();
#else
	cocos2d::__Array *arr = cocos2d::__Array::create();
#endif

	// オブジェクトの矩形
	auto objectRect = Rect(boundMin.x, boundMin.y, boundMax.x - boundMin.x, boundMax.y - boundMin.y);
	// 過去の位置の矩形
	auto prevRect = Rect(boundMin.x - moveVec.x, boundMin.y - moveVec.y, objectRect.size.width, objectRect.size.height);

	// 過去の位置の矩形とマージしてオブジェクトの矩形を広げる
	objectRect.merge(prevRect);

	// 坂の矩形の広げる量
	float addWidth = 1.0f;
	float addHeight = 1.0f;

	// 現在の矩形だけで坂と接触しているかを判定すると
	// 坂を下りきる際に判定抜けが発生したので
	// 現在の矩形と過去の矩形を考慮して当たるかを判定する
	for (int i = 0; i < getSlopeList()->count(); i++)
	{
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
		auto slope = static_cast<agtk::Slope*>(getSlopeList()->getObjectAtIndex(i));
#else
		auto slope = dynamic_cast<agtk::Slope*>(getSlopeList()->getObjectAtIndex(i));
#endif

		auto slopeRect = slope->convertToLayerSpaceRect();

		slopeRect.origin.x -= addWidth * 0.5f;
		slopeRect.origin.y -= addHeight * 0.5f;
		slopeRect.size.width += addWidth;
		slopeRect.size.height += addHeight;

		if (objectRect.intersectsRect(slopeRect)) {
#ifdef USE_MULTITHREAD_OBJECT_UPDATE
			arr->push_back(slope);
#else
			arr->addObject(slope);
#endif
		}
	}

	return arr;
}

cocos2d::__Array *SceneLayer::getCollisionLoopCourseList(cocos2d::Point boundMin, cocos2d::Point boundMax)
{
	auto projectData = GameManager::getInstance()->getProjectData();
	auto scene = GameManager::getInstance()->getCurrentScene();

	cocos2d::__Array *arr = cocos2d::__Array::create();

	auto objectRect = agtk::Scene::getRectSceneFromCocos2d(Rect(boundMin.x, boundMin.y, boundMax.x - boundMin.x, boundMax.y - boundMin.y));

	for (int i = 0; i < getLoopCourseList()->count(); i++) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
		auto course = static_cast<agtk::OthersLoopCourse*>(getLoopCourseList()->getObjectAtIndex(i));
#else
		auto course = dynamic_cast<agtk::OthersLoopCourse*>(getLoopCourseList()->getObjectAtIndex(i));
#endif

		// 矩形範囲にいるかチェックする
		if (objectRect.intersectsRect(course->getRect()))
		{
			arr->addObject(course);
		}
	}

	return arr;
}

/**
* 他のシーンからのオブジェクト追加
* @param	object			オブジェクト
* @param	parentObject	親オブジェクト
* @return					追加したオブジェクト
*/
agtk::Object *SceneLayer::addOtherSceneObject(agtk::Object * object, Vec2 apperPos, agtk::Object *parentObject)
{
	auto sceneData = this->getSceneData();
	auto objList = this->getObjectList();
	auto layerId = this->getLayerId();
	CC_ASSERT(_scene);
	auto viewportLightSceneLayer = _scene->getViewportLight()->getViewportLightSceneLayer(layerId);

#if defined(USE_RUNTIME)
	auto projectData = GameManager::getInstance()->getProjectData();
	if (object->getObjectData()->getTestplayOnly()) {
		return nullptr;
	}
#endif
	int actionId = object->getCurrentObjectAction()->getId();
	int moveDirectionId = object->getDispDirection();
	int dispDirectionId = moveDirectionId;
	if(object->getObjectData()->getTakeoverDispDirection() && parentObject != nullptr) {
		moveDirectionId = object->getInitialMoveDirection();
		dispDirectionId = parentObject->getDispDirection();
	}
	// オブジェクトを新規生成
	agtk::Object *newObject = nullptr;
	auto objectAction = object->getCurrentObjectAction();
	int directionId = objectAction->getObjectActionData()->getAnimMotionId();
	if (directionId < 0) {
		//モーション設定無し。
		auto player = object->getPlayer();
		if (player) {
			directionId = player->getBasePlayer()->getCurrentAnimationMotion()->getMotionData()->getId();
		}
		newObject = agtk::Object::create(
			this,
			object->getObjectData()->getId(),
			actionId,
			apperPos,
			Vec2(object->getScale(), object->getScale()),
			object->getRotation(),
			moveDirectionId,
			-1, -1, directionId
		);
	}
	else {
		newObject = agtk::Object::create(
			this,
			object->getObjectData()->getId(),
			actionId,
			apperPos,
			Vec2(object->getScale(), object->getScale()),
			object->getRotation(),
			moveDirectionId
			, -1, -1, -1
		);
	}
	newObject->setId(objectId++);
	newObject->setLayerId(layerId);
	newObject->setPhysicsBitMask(layerId, sceneData->getId());
	newObject->setInitialMoveDirection(object->getInitialMoveDirection());
	newObject->setSceneIdOfFirstCreated(object->getSceneIdOfFirstCreated());
	newObject->setScenePartsId(object->getScenePartsId());
	newObject->setScenePartObjectData(object->getScenePartObjectData());
	newObject->setDisabled(object->getDisabled());
	newObject->getObjectVisible()->setVisible(object->getObjectVisible()->getVisible());
	newObject->setPrevObjectActionId(object->getPrevObjectActionId());// 遷移前のアクションIDを引き継ぐ
	if (directionId >= 0) {
		newObject->getPlayer()->setVisible(object->getPlayer()->isVisible());
	}
	auto newObjectData = newObject->getObjectData();
	if (newObjectData->getViewportLightSettingFlag() && newObjectData->getViewportLightSettingList()->count() > 0) {
		auto viewportLightObject = ViewportLightObject::create(newObject, _scene->getViewportLight(), this);
		viewportLightSceneLayer->getViewportLightObjectList()->addObject(viewportLightObject);
	}
	newObject->setDispDirection(dispDirectionId);
// #AGTK-NX InstanceIdが決定した後でSceneLayerへ追加するように変更
#ifndef USE_SAR_OPTIMIZE_1
	this->addObject(newObject);
#endif

	// ジャンプ引き継ぎ
	newObject->setJumpAction(object->getJumpActionFlag());
	newObject->setJumping(object->getJumping());
	newObject->getObjectMovement()->setVertVelocity(object->getObjectMovement()->getVertVelocity());

	//移動スピードを引き継ぐ。
	auto newObjectMovement = newObject->getObjectMovement();
	auto objectMovement = object->getObjectMovement();
	newObjectMovement->setMoveSpeed(objectMovement->getMoveSpeed());
	newObjectMovement->setUpDownMoveSpeed(objectMovement->getUpDownMoveSpeed());
	newObjectMovement->setTurnSpeed(objectMovement->getTurnSpeed());

	// 変数とスイッチを引き継ぎ
	auto playObjectDataOfSrcObject = object->getPlayObjectData();
	auto playObjectDataOfNewObject = newObject->getPlayObjectData();
	playObjectDataOfNewObject->takeOverVariableList(playObjectDataOfSrcObject);
	playObjectDataOfNewObject->takeOverSwitchList(playObjectDataOfSrcObject);

	// 素材セットIDを引き継ぎ
	auto player = newObject->getPlayer();
	if (player) {
		player->setResourceSetId(object->getResourceSetId());
	}
	newObject->setResourceSetId(object->getResourceSetId());

	//位置情報は引き継がない。
	auto variableX = playObjectDataOfNewObject->getVariableData(agtk::data::kObjectSystemVariableX);
	variableX->setValue(apperPos.x);
	auto variableY = playObjectDataOfNewObject->getVariableData(agtk::data::kObjectSystemVariableY);
	variableY->setValue(apperPos.y);

	//表示方向変数を更新。
	newObject->updateDisplayDirectionVariable();

	// インスタンスIDを更新
	auto scene = this->getScene();
	playObjectDataOfNewObject->setInstanceId(scene->getObjectInstanceId(newObject));
	playObjectDataOfNewObject->setInstanceCount(scene->incrementObjectInstanceCount(newObject->getObjectData()->getId()));
	scene->updateObjectInstanceCount(newObject->getObjectData()->getId());

// #AGTK-NX
#ifdef USE_SAR_OPTIMIZE_1
	this->addObject(newObject);
#endif

	//共通１P～４PインスタンスID設定
	if (newObject->getScenePartObjectData() && newObject->getScenePartObjectData()->isStartPointObject()) {
		int playerId = playObjectDataOfNewObject->getPlayerId() - 1;
		if (playerId >= 0) {
			auto projectPlayData = GameManager::getInstance()->getPlayData();
			auto playVariableData = projectPlayData->getVariableData(agtk::data::PlayData::COMMON_PLAY_DATA_ID, agtk::data::kProjectSystemVariable1PInstance + playerId);
			playVariableData->setValue((double)playObjectDataOfNewObject->getInstanceId());
		}
	}
	// コリジョンコンに登録
	addCollisionDetaction(newObject);

	// ポータルワープ情報を引き継ぎ
	newObject->setIsPortalWarped(object->getIsPortalWarped());

	// このオブジェクト用の単体インスタンスIDはシーン初期化時に設定されないのでここで設定する
	auto projectPlayData = GameManager::getInstance()->getPlayData();
	auto playObjectList = projectPlayData->getObjectList();
	cocos2d::DictElement *el = nullptr;
	CCDICT_FOREACH(playObjectList, el) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
		auto playObjectData = static_cast<agtk::data::PlayObjectData *>(el->getObject());
#else
		auto playObjectData = dynamic_cast<agtk::data::PlayObjectData *>(el->getObject());
#endif
		if (playObjectData->getId() == newObject->getObjectData()->getId()) {
			//単体として指定されている同オブジェクトのインスタンスのインスタンスIDが格納。
			auto variableData = playObjectData->getVariableData(agtk::data::kObjectSystemVariableSingleInstanceID);
			variableData->setValue((double)newObject->getInstanceId());
			//シーンに配置された同インスタンスの数が格納。
			auto instanceCountVariableData = playObjectData->getVariableData(agtk::data::kObjectSystemVariableInstanceCount);
			playObjectData->setInstanceCount(instanceCountVariableData->getValue() + 1);
		}
	}

	// 再出現用のデータを設定
	auto newReappearData = newObject->getObjectReappearData();
	auto reappearData = object->getObjectReappearData();
	newReappearData->setSceneId(reappearData->getSceneId());

#if 1//ACT2-2465の修正でその他実行アクションが再実行されるようにしたが、ACT2-2167の問題が再発し、ACT2-4837の問題が発生するため、アクションコマンドは再実行されないようにする。
	// アクションコマンドを再実行しないように空にしておく
	newObject->getCurrentObjectAction()->getObjCommandList()->removeAllObjects();
#endif
	return newObject;
}

void SceneLayer::addChild(Node * child)
{
	this->addChild(child, child->getLocalZOrder(), child->getName());
}

void SceneLayer::addChild(Node * child, int localZOrder)
{
	this->addChild(child, localZOrder, child->getName());
}

void SceneLayer::addChild(Node* child, int localZOrder, int tag)
{
	if (this->getType() == kTypeMenu) {
		child->setCameraMask((unsigned short)cocos2d::CameraFlag::USER1);
	}
	this->_addChild(child, localZOrder, tag);
}

void SceneLayer::addChild(Node* child, int localZOrder, const std::string &name)
{
	if (this->getType() == kTypeMenu) {
		child->setCameraMask((unsigned short)cocos2d::CameraFlag::USER1);
	}
	this->_addChild(child, localZOrder, name);
}

void SceneLayer::removeChild(Node *child, bool cleanup)
{
	Node::removeChild(child, cleanup);
}

void SceneLayer::removeChildByTag(int tag, bool cleanup)
{
	Node::removeChildByTag(tag, cleanup);
}

void SceneLayer::removeChildByName(const std::string &name, bool cleanup)
{
	Node::removeChildByName(name, cleanup);
}

void SceneLayer::removeRenderTexture()
{
	auto renderTexture = this->getRenderTexture();
	if (renderTexture) {
		renderTexture->getFirstRenderTexture()->removeFromParentAndCleanup(true);
		renderTexture->removeFromParentAndCleanup(true);
		this->setRenderTexture(nullptr);
	}
}

void SceneLayer::_addChild(Node *child, int localZOrder, int tag)
{
	Node::addChild(child, localZOrder, tag);
}

void SceneLayer::_addChild(Node *child, int localZOrder, const std::string &name)
{
	Node::addChild(child, localZOrder, name);
}

/**
* 物理オブジェクトの生成
* @param	sceneData	シーンデータ
*/
void SceneLayer::createPhysicsObject(agtk::data::SceneData *sceneData)
{
	auto layerId = this->getLayerId();
	auto physicsObjList = cocos2d::__Array::create();
	auto scenePartPhysicsList = sceneData->getScenePartPhysicsList();
	std::list<PinnedPhysicsInfo> pinnedPhysicsInfoList;

	// 物理オブジェクトデータがある場合
	if (scenePartPhysicsList->count()) {

		// 接続系オブジェクト用配列
		// ※シーンパーツリストの都合上接続系オブジェクトが接続先より先に存在する事がある
		cocos2d::__Array *pinJointList = cocos2d::__Array::create();
		cocos2d::__Array *connectionObjList = cocos2d::__Array::create();

		// 先に物理オブジェクトを生成する
		cocos2d::DictElement *el = nullptr;
		CCDICT_FOREACH(scenePartPhysicsList, el) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
			auto p = static_cast<agtk::data::PhysicsPartData *>(el->getObject());
#else
			auto p = dynamic_cast<agtk::data::PhysicsPartData *>(el->getObject());
#endif

			// 配置レイヤーが異なる場合
			if (p->getLayerIndex() + 1 != layerId) {
				// スキップ
				continue;
			}

			// ------------------------------------------------------------------------
			// 円形
			if (p->getType() == agtk::data::PhysicsPartData::EnumPhysicsType::kDisk) {
				// 円形オブジェクト生成
				auto disk = PhysicsDisk::create(p, sceneData, layerId, -1);

				// シーンレイヤーに登録
#ifdef USE_REDUCE_RENDER_TEXTURE
				this->getObjectSetNode()->addChild(disk, p->getPriority() + p->getDispPriority() * 1000);
#else
				this->addChild(disk);
#endif

				// 物理オブジェクトリストに登録
				physicsObjList->addObject(disk);

				CCLOG("add Disk: %d", disk->getScenePartsId());
			}
			// ------------------------------------------------------------------------
			// 四角形
			else if (p->getType() == agtk::data::PhysicsPartData::EnumPhysicsType::kRectangle) {
				// 矩形物理オブジェクト生成
				auto rectangle = PhysicsRectangle::create(p, sceneData, layerId, -1);

				// シーンレイヤーに登録
#ifdef USE_REDUCE_RENDER_TEXTURE
				this->getObjectSetNode()->addChild(rectangle, p->getPriority() + p->getDispPriority() * 1000);
#else
				this->addChild(rectangle);
#endif

				// 物理オブジェクトリストに登録
				physicsObjList->addObject(rectangle);

				CCLOG("add Rectangke: %d", rectangle->getScenePartsId());
			}
			// ------------------------------------------------------------------------
			// 接着
			else if (p->getType() == agtk::data::PhysicsPartData::EnumPhysicsType::kPin) {
				// 接着オブジェクトリストに登録
				pinJointList->addObject(p);
			}
			// ------------------------------------------------------------------------
			// 「接着」以外の接続系の場合
			else {
				// 接続系オブジェクトリストに登録
				connectionObjList->addObject(p);
			}
		}

		// 「接着」を生成
		// ※シーンに接着される物理オブジェクトは静的物理オブジェクトになる為、
		// 　登録順序に関係なく先に接着を実行する必要がある
		cocos2d::Ref * ref = nullptr;
		CCARRAY_FOREACH(pinJointList, ref) {

			// 「接着」ジョイント生成
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
			auto p = static_cast<agtk::data::PhysicsPartData *>(ref);
#else
			auto p = dynamic_cast<agtk::data::PhysicsPartData *>(ref);
#endif
			createPinJoint(p, sceneData, layerId, physicsObjList, nullptr, pinnedPhysicsInfoList);
		}

		// 接続系オブジェクトの生成
		ref = nullptr;
		CCARRAY_FOREACH(connectionObjList, ref) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
			auto p = static_cast<agtk::data::PhysicsPartData *>(ref);
#else
			auto p = dynamic_cast<agtk::data::PhysicsPartData *>(ref);
#endif

			// ------------------------------------------------------------------------
			// ロープ
			if (p->getType() == agtk::data::PhysicsPartData::EnumPhysicsType::kRope) {
				// ロープ生成
				createRopeJoint(p, sceneData, layerId, physicsObjList, nullptr);
			}
			// ------------------------------------------------------------------------
			// バネ
			else if (p->getType() == agtk::data::PhysicsPartData::EnumPhysicsType::kSpring) {

				// バネの生成
				createSpringJoint(p, sceneData, layerId, physicsObjList, nullptr);
			}
			// ------------------------------------------------------------------------
			// 回転軸
			else if (p->getType() == agtk::data::PhysicsPartData::EnumPhysicsType::kAxis) {
				
				// 回転軸の生成
				createAxisJoint(p, sceneData, layerId, physicsObjList, nullptr);
			}
			// ------------------------------------------------------------------------
			// 爆発
			else if (p->getType() == agtk::data::PhysicsPartData::EnumPhysicsType::kExplosion) {
				// 爆発を生成
				createExplode(p, sceneData, layerId, physicsObjList, nullptr);
			}
			// ------------------------------------------------------------------------
			// 引力・斥力
			else if (p->getType() == agtk::data::PhysicsPartData::EnumPhysicsType::kForce) {
				// 引力・斥力を生成
				createForce(p, sceneData, layerId, physicsObjList, nullptr);
			}
		}
	}

	if (physicsObjList->count() > 0) {
		this->getPhysicsObjectList()->addObjectsFromArray(physicsObjList);
	}
	auto gameManager = GameManager::getInstance();
	for (auto &info : pinnedPhysicsInfoList) {
		if (info.lower->getTarget()->getPhysicsBody()->isDynamic() || info.upper->getTarget()->getPhysicsBody()->isDynamic()) {
			createPhysicsJointPin(info.upper, info.lower);
		}
	}
}

void SceneLayer::createPhysicsJointPin(agtk::SceneLayer::ConnectTarget *upper, agtk::SceneLayer::ConnectTarget *lower)
{
	// 接着部分のジョイント生成
	auto joint = PhysicsJointPin::construct(lower->getTarget()->getPhysicsBody(), upper->getTarget()->getPhysicsBody(), upper->getPos() + upper->getAnchor());
	joint->setCollisionEnable(false);
	GameManager::getInstance()->getPhysicsWorld()->addJoint(joint);

	// 回転抑止用のジョイント生成
	auto joint2 = PhysicsJointPin::construct(lower->getTarget()->getPhysicsBody(), upper->getTarget()->getPhysicsBody(), upper->getPos() + upper->getAnchor() + Vec2(29, 0));
	joint2->setCollisionEnable(false);
	GameManager::getInstance()->getPhysicsWorld()->addJoint(joint2);
	auto joint3 = PhysicsJointPin::construct(lower->getTarget()->getPhysicsBody(), upper->getTarget()->getPhysicsBody(), upper->getPos() + upper->getAnchor() + Vec2(-23, -5));
	joint3->setCollisionEnable(false);
	GameManager::getInstance()->getPhysicsWorld()->addJoint(joint3);
	auto joint4 = PhysicsJointPin::construct(lower->getTarget()->getPhysicsBody(), upper->getTarget()->getPhysicsBody(), upper->getPos() + upper->getAnchor() + Vec2(11, 19));
	joint4->setCollisionEnable(false);
	GameManager::getInstance()->getPhysicsWorld()->addJoint(joint4);
	auto joint5 = PhysicsJointPin::construct(lower->getTarget()->getPhysicsBody(), upper->getTarget()->getPhysicsBody(), upper->getPos() + upper->getAnchor() + Vec2(-13, -17));
	joint5->setCollisionEnable(false);
	GameManager::getInstance()->getPhysicsWorld()->addJoint(joint5);
}

/**
* オブジェクトに紐付いた物理オブジェクトの生成
* @param	object	オブジェクト
*/
void SceneLayer::createPhysicsObjectWithObject(agtk::Object *object)
{
	CC_ASSERT(object);

	// オブジェクトの「物理演算に関する設定」が OFF の場合
	if (!object->getObjectData()->getPhysicsSettingFlag()) {
		// オブジェクトに紐付いた物理オブジェクトを生成しない
		return;
	} 
	int parentScenePartId = object && object->getScenePartObjectData() ? object->getScenePartObjectData()->getId() : -1;

	// オブジェクトに紐付いた物理パーツリストを取得
	cocos2d::__Dictionary *partPhysicsList = nullptr;
	auto physicsPartListInscenePartObjectData = object->getScenePartObjectData() ? object->getScenePartObjectData()->getPhysicsPartList() : nullptr;
	auto physicsPartListInObjectData = object->getObjectData()->getPhysicsPartList();
	std::list<PinnedPhysicsInfo> pinnedPhysicsInfoList;

	// シーンパーツデータ内にこのオブジェクトと紐づく物理パーツがある場合
	if (physicsPartListInscenePartObjectData && physicsPartListInscenePartObjectData->count() > 0) {
		partPhysicsList = object->getScenePartObjectData()->getPhysicsPartList();
	}
	// オブジェクトデータ内にこのオブジェクトと紐づく物理パーツがある場合
	else if (physicsPartListInObjectData && physicsPartListInObjectData->count() > 0) {
		partPhysicsList = object->getObjectData()->getPhysicsPartList();
	}
	// 紐づく物理パーツが無い場合
	else {
		// オブジェクトに紐付いた物理オブジェクトを生成しない
		return;
	}

	auto layerId = this->getLayerId();
	auto physicsObjList = cocos2d::__Array::create();
	auto sceneData = this->getSceneData();

	// 物理オブジェクトパーツデータリストがある場合
	if (partPhysicsList->count() > 0) {

		// オブジェクトの座標
		auto objectPos = Scene::getPositionSceneFromCocos2d(object->getPosition(), sceneData);

		// ==================================================================
		// 先に物理オブジェクトを生成する
		// 接続系オブジェクト用配列
		// ※シーンパーツリストの都合上接続系オブジェクトが接続先より先に存在する事がある
		cocos2d::__Array *pinJointList = cocos2d::__Array::create();
		cocos2d::__Array *connectionObjList = cocos2d::__Array::create();

		// 物理オブジェクト間の優先度順に生成する
		for (int targetPriority = 0, max = partPhysicsList->count(); targetPriority <= max; targetPriority++) {

			// 指定優先度値のオブジェクトを取得する
			auto obj = partPhysicsList->objectForKey(targetPriority);
			auto p = dynamic_cast<agtk::data::PhysicsPartData *>(obj);
			if (nullptr == p) {
				continue;
			}

			// ------------------------------------------------------------------------
			// 円形
			if (p->getType() == agtk::data::PhysicsPartData::EnumPhysicsType::kDisk) {
				// 円形オブジェクト生成
				auto disk = PhysicsDisk::create(p, sceneData, layerId, parentScenePartId);

				// 円形物理オブジェクトの座標を基準に座標を再設定
				//! ※物理データの座標はオブジェクトの原点からの距離の為再設定の必要がある
				disk->setPosition(Vec2(objectPos.x + disk->getPhysicsData()->getX(), objectPos.y - disk->getPhysicsData()->getY()));

#ifdef USE_REDUCE_RENDER_TEXTURE
#else
				// シーンレイヤーに登録
				this->addChild(disk);
#endif

				// 物理オブジェクトリストに登録
				physicsObjList->addObject(disk);

				// オブジェクトに紐付いた物理オブジェクト用リストに登録
				object->getPhysicsPartsList()->addObject(disk);

				// 描画用リストへの登録
				registDrawListForPhysicsPartsWithObjectByDrawPriority(object, disk);

				CCLOG("add Disk: %d [with obj]", disk->getScenePartsId());
			}
			// ------------------------------------------------------------------------
			// 四角形
			else if (p->getType() == agtk::data::PhysicsPartData::EnumPhysicsType::kRectangle) {
				// 矩形物理オブジェクト生成
				auto rectangle = PhysicsRectangle::create(p, sceneData, layerId, parentScenePartId);

				// 矩形物理オブジェクトの座標を基準に座標を再設定
				//! ※物理データの座標はオブジェクトの原点からの距離の為再設定の必要がある
				rectangle->setPosition(Vec2(objectPos.x + rectangle->getPhysicsData()->getX(), objectPos.y - rectangle->getPhysicsData()->getY()));

#ifdef USE_REDUCE_RENDER_TEXTURE
#else
				// シーンレイヤーに登録
				this->addChild(rectangle);
#endif

				// 物理オブジェクトリストに登録
				physicsObjList->addObject(rectangle);

				// オブジェクトに紐付いた物理オブジェクト用リストに登録
				object->getPhysicsPartsList()->addObject(rectangle);

				// 描画用リストへの登録
				registDrawListForPhysicsPartsWithObjectByDrawPriority(object, rectangle);

				CCLOG("add Rectangke: %d [with obj]", rectangle->getScenePartsId());
			}
			// ------------------------------------------------------------------------
			// 接着
			else if (p->getType() == agtk::data::PhysicsPartData::EnumPhysicsType::kPin) {
				// 「接着」リストに登録
				pinJointList->addObject(p);
			}
			// ------------------------------------------------------------------------
			// 「接着」以外の接続系の場合
			else {
				// 接続系オブジェクトリストに登録
				connectionObjList->addObject(p);
			}
		}

		// ==================================================================
		// 「接着」生成
		cocos2d::Ref * ref = nullptr;
		CCARRAY_FOREACH(pinJointList, ref) {

			// 「接着」ジョイント生成
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
			auto p = static_cast<agtk::data::PhysicsPartData *>(ref);
#else
			auto p = dynamic_cast<agtk::data::PhysicsPartData *>(ref);
#endif
			createPinJoint(p, sceneData, layerId, physicsObjList, object, pinnedPhysicsInfoList);

		}

		// ==================================================================
		// 接続系物理オブジェクト生成
		ref = nullptr;
		CCARRAY_FOREACH(connectionObjList, ref) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
			auto p = static_cast<agtk::data::PhysicsPartData *>(ref);
#else
			auto p = dynamic_cast<agtk::data::PhysicsPartData *>(ref);
#endif

			// ------------------------------------------------------------------------
			// ロープ
			if (p->getType() == agtk::data::PhysicsPartData::EnumPhysicsType::kRope) {
				// ロープ生成
				createRopeJoint(p, sceneData, layerId, physicsObjList, object);
			}
			// ------------------------------------------------------------------------
			// バネ
			else if (p->getType() == agtk::data::PhysicsPartData::EnumPhysicsType::kSpring) {

				// バネの生成
				createSpringJoint(p, sceneData, layerId, physicsObjList, object);
			}
			// ------------------------------------------------------------------------
			// 回転軸
			else if (p->getType() == agtk::data::PhysicsPartData::EnumPhysicsType::kAxis) {

				// 回転軸の生成
				createAxisJoint(p, sceneData, layerId, physicsObjList, object);
			}
			// ------------------------------------------------------------------------
			// 爆発
			else if (p->getType() == agtk::data::PhysicsPartData::EnumPhysicsType::kExplosion) {

				// 爆発を生成
				createExplode(p, sceneData, layerId, physicsObjList, object);
			}
			// ------------------------------------------------------------------------
			// 引力・斥力
			else if (p->getType() == agtk::data::PhysicsPartData::EnumPhysicsType::kForce) {

				// 引力・斥力を生成
				createForce(p, sceneData, layerId, physicsObjList, object);
			}
		}

		if (physicsObjList->count() > 0) {
			this->getPhysicsObjectList()->addObjectsFromArray(physicsObjList);
		}
	}
	auto gameManager = GameManager::getInstance();
	for (auto &info : pinnedPhysicsInfoList) {
		if (info.lower->getTarget()->getPhysicsBody()->isDynamic() || info.upper->getTarget()->getPhysicsBody()->isDynamic()) {
			createPhysicsJointPin(info.upper, info.lower);
		}
	}
	object->restorePhysicsGeometry();
}

#pragma region 物理ジョイントの生成メソッド群 -------------------------------------------------------------------
/**
* 接着ジョイントの生成
* @param	physicsPartData パーツデータ
* @param	sceneData		シーンデータ
* @param	layerId			レイヤーID
* @param	physicsObjList	格納先の物理オブジェクトリスト
* @param	object			物理パーツが紐付いているオブジェクト
*/
void SceneLayer::createPinJoint(agtk::data::PhysicsPartData *physicsPartData, agtk::data::SceneData *sceneData, int layerId, cocos2d::__Array *physicsObjList, agtk::Object *object, std::list<PinnedPhysicsInfo> & pinnedPhysicsInfoList)
{
	// 接着のデータを取得
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
	auto data = static_cast<agtk::data::PhysicsPinData *>(physicsPartData->getPhysicsData());
#else
	auto data = dynamic_cast<agtk::data::PhysicsPinData *>(physicsPartData->getPhysicsData());
#endif

	// 接着元と接着先が無い場合
	if (data->getLowerId() < 0 && data->getUpperId() < 0) {
		// 何もしない
		return;
	}

	// 接着オブジェクトを生成(見た目用)
	int parentScenePartId = object && object->getScenePartObjectData() ? object->getScenePartObjectData()->getId() : -1;
	auto pin = PhysicsPin::create(physicsPartData, sceneData, layerId, parentScenePartId);

	ConnectTarget *lower = ConnectTarget::create();	// 上位接続設定
	ConnectTarget *upper = ConnectTarget::create();	// 下位接続設定

	bool isWithObject = (object != nullptr);

	// -------------------------------------------------------------------------------
	// 下位の連結対象設定
	// -------------------------------------------------------------------------------
	// 接着元がある場合
	if (data->getLowerId() > 0) {
		bool isObject = false;
		Node* obj = nullptr;

		// オブジェクトに紐付いた物理パーツの場合
		if (isWithObject) {
			// ベースとなるオブジェクトに接続する場合
			if (data->getLowerId() == agtk::Object::DEFAULT_SCENE_PARTS_ID) {
				isObject = true;
				obj = object->getphysicsNode();
			}
			// 他の物理オブジェクトに接続する場合
			else {
				// 物理オブジェクトを検索
				obj = getConnectedPhysicsPartsFromList(physicsObjList, data->getLowerId());
			}
		}
		// シーンに直接配置した物理パーツの場合
		else {
			obj = getConnectedTarget(data->getLowerId(), data->getLowerSubId(), physicsObjList, &isObject);
		}

		if (obj) {
			auto anchor = Vec2(data->getLowerX(), -data->getLowerY());
			auto pos = obj->getPosition();

			// 通常オブジェクトに接続している場合
			if (isObject) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
				auto object = static_cast<agtk::Object *>(obj->getParent());
#else
				auto object = dynamic_cast<agtk::Object *>(obj->getParent());
#endif
				getObjectAnchorAndPos(object, &pos, &anchor);

				// 接続オブジェクトの参照を保持
				pin->setFollowObject(object);

				if (isWithObject && !object->getPlayObjectData()->getSwitchData(agtk::data::kObjectSystemSwitchFollowConnectedPhysics)->getValue()) {
					// 下位接続オブジェクトの参照を保持
					pin->setFollowObject(object);
				}
			}
			else {
				// ロープ部品と接続しているかチェック
				bool isRope = false;
				obj = checkConnectToRope(obj, data->getLowerId(), data->getLowerX(), &anchor, &pos, physicsObjList, &isRope);

				// 下位接続物理ボディの参照を保持
				pin->setFollowerPhysicsBody(obj->getPhysicsBody());
			}

			lower->setAnchor(rotateAnchor(anchor, obj->getRotation()));
			lower->setPos(pos);
			lower->setTarget(obj);
		}
	}

	CCASSERT(data->getUpperId() > 0, "Upper Object must be attached !!");

	// -------------------------------------------------------------------------------
	// 上位の連結対象設定
	// -------------------------------------------------------------------------------
	bool isObject = false;
	Node* obj = nullptr;

	// オブジェクトに紐付いた物理パーツの場合
	if (isWithObject) {
		// ベースとなるオブジェクトに接続する場合
		if (data->getUpperId() == agtk::Object::DEFAULT_SCENE_PARTS_ID) {
			isObject = true;
			obj = object->getphysicsNode();
		}
		// 他の物理オブジェクトに接続する場合
		else {
			// 物理オブジェクトを検索
			obj = getConnectedPhysicsPartsFromList(physicsObjList, data->getUpperId());
		}
	}
	// シーンに直接配置した物理パーツの場合
	else {
		obj = getConnectedTarget(data->getUpperId(), data->getUpperSubId(), physicsObjList, &isObject);
	}

	if (obj) {
		auto anchor = Vec2(data->getUpperX(), -data->getUpperY());
		auto pos = obj->getPosition();

		// 通常オブジェクトに接続している場合
		if (isObject) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
			auto object = static_cast<agtk::Object *>(obj->getParent());
#else
			auto object = dynamic_cast<agtk::Object *>(obj->getParent());
#endif
			getObjectAnchorAndPos(object, &pos, &anchor);
			pin->setPosition(anchor);

			if (!pin->getFollowObject() && (!isWithObject || !object->getPlayObjectData()->getSwitchData(agtk::data::kObjectSystemSwitchFollowConnectedPhysics)->getValue())) {
				// 接続オブジェクトの参照を保持
				pin->setFollowObject(object);
			}
		}
		else {
			// ロープ部品と接続しているかチェック
			bool isRope = false;
			obj = checkConnectToRope(obj, data->getUpperId(), data->getUpperX(), &anchor, &pos, physicsObjList, &isRope);
			pin->setPosition(anchor + obj->getContentSize() * 0.5f);

			if (!pin->getFollowerPhysicsBody()) {
				// 上位接続物理ボディの参照を保持
				pin->setFollowerPhysicsBody(obj->getPhysicsBody());
			}
		}

		// 接着の見た目を上位オブジェクトに設定
		// ACT2-4667 表示優先度が機能しないのでレイヤー配置するように変更
		// obj->addChild(pin, pin->getPriority()); 
		Node* dummyNode = Node::create();
		dummyNode->setAnchorPoint(pin->getAnchorPoint());
		dummyNode->setScale(pin->getScale());
		dummyNode->setRotation(pin->getRotation());
		dummyNode->setContentSize(pin->getContentSize());
		dummyNode->setPosition(pin->getPosition());
		dummyNode->setVisible(false);

		obj->addChild(dummyNode, pin->getPriority());
#ifdef USE_REDUCE_RENDER_TEXTURE
		this->getObjectSetNode()->addChild(pin, pin->getPriority() + pin->getDispPriority() * 1000);
#else
		this->addChild(pin, pin->getPriority());
#endif

		auto physicsBase = dynamic_cast<agtk::PhysicsBase *>(obj);
		if (physicsBase) {
			auto nodeList = physicsBase->getConnectToBaseList();
			nodeList.push_back({ dummyNode, pin });
			physicsBase->setConnectToBaseList(nodeList);
		}

		upper->setAnchor(rotateAnchor(anchor, obj->getRotation()));
		upper->setPos(pos);
		upper->setTarget(obj);
	}

	// -------------------------------------------------------------------------------
	// 連結処理
	// -------------------------------------------------------------------------------
	// 下位接続対象が存在する場合
	if (lower->getTarget()) {

		//createPhysicsJointPin(upper, lower);
		PinnedPhysicsInfo info;
		info.upper = upper;
		info.lower = lower;
		pinnedPhysicsInfoList.emplace_back(info);

		if (pin->getFollowObject()) {
			// 接着ジョイントを物理オブジェクトリストに登録
			physicsObjList->addObject(pin);

		}
	}
	// 下位接続対象が存在しない場合(シーンに固定される場合)
	else {
		// 上位接続対象を静的な物理オブジェクトにする
#ifdef USE_PHYSICS_STATIC_MASS_MOMENT
		auto body = upper->getTarget()->getPhysicsBody();
		body->setDynamic(false);
		body->setMass(PHYSICS_INFINITY);
		body->setMoment(PHYSICS_INFINITY);
#else
		upper->getTarget()->getPhysicsBody()->setDynamic(false);
#endif
	}
	{
		// 上位パーツがオブジェクトに紐付いている場合は、そのオブジェクトのパーツリストに入れる。
		auto target = upper->getTarget();
		cocos2d::Ref *ref = nullptr;
		auto objectList = this->getObjectList();
		CCARRAY_FOREACH(objectList, ref) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
			auto o = static_cast<agtk::Object *>(ref);
#else
			auto o = dynamic_cast<agtk::Object *>(ref);
#endif
			if (o->getPhysicsPartsList()->containsObject(target)) {
				o->getPhysicsPartsList()->addObject(pin);
			}
		}
	}

	if (isWithObject) {
		// 描画用リストへの登録
		registDrawListForPhysicsPartsWithObjectByDrawPriority(object, pin);
	}

	CCLOG("add Pin: %d %s", pin->getScenePartsId(), ((object != nullptr) ? "[with Obj]" : ""));
}

/**
* ロープジョイントの生成
* @param	physicsPartData パーツデータ
* @param	sceneData		シーンデータ
* @param	layerId			レイヤーID
* @param	physicsObjList	格納先の物理オブジェクトリスト
* @param	object			物理パーツが紐付いているオブジェクト
*/
void SceneLayer::createRopeJoint(agtk::data::PhysicsPartData *physicsPartData, agtk::data::SceneData *sceneData, int layerId, cocos2d::__Array *physicsObjList, agtk::Object *object)
{
	// ロープのパーツを生成
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
	auto data = static_cast<agtk::data::PhysicsRopeData *>(physicsPartData->getPhysicsData());
#else
	auto data = dynamic_cast<agtk::data::PhysicsRopeData *>(physicsPartData->getPhysicsData());
#endif
	int pointMax = data->getPointList()->count();

	bool isWithObject = (object != nullptr);

	// -------------------------------------------------------
	// ロープ部品をジョイント
	// -------------------------------------------------------
	bool isSetPartsStartPos = false;// 部品の始点を設定したフラグ
	bool isSetPartsEndPos = false;// 部品の終点を設定したフラグ

	ConnectTarget *startTarget = ConnectTarget::create();
	ConnectTarget *endTarget = ConnectTarget::create();
	ConnectTarget *preTarget = ConnectTarget::create();
	ConnectTarget *curTarget = ConnectTarget::create();

	auto ignoreTargetList = cocos2d::__Array::create();//ロープとの衝突を回避する対象リスト
	auto ropeList = cocos2d::__Array::create();// ロープリスト
	auto objectPos = isWithObject ? Scene::getPositionSceneFromCocos2d(object->getPosition(), sceneData) : Vec2::ZERO;//オブジェクトの座標(シーンに直接配置される場合は(0, 0)とする)

	// ※ +2 は始点と終点
	for (int idx = 0, idxMax = pointMax + 2, ropeIdx = 0; idx < idxMax;) {

		// 始点の場合
		if (idx == 0) {
			// 始点のデフォルトはシーン上の座標
			auto pos = Vec2::ZERO;
			auto anchor = Vec2::ZERO;
			cocos2d::Node *target = nullptr;

			// オブジェクトに紐付いている物理パーツの場合
			if (isWithObject) {
				pos = Vec2(objectPos.x + data->getConnectX1(), objectPos.y - data->getConnectY1());
			}
			// シーンに直接配置される物理パーツの場合
			else {
				pos = Scene::getPositionSceneFromCocos2d(Vec2(data->getConnectX1(), data->getConnectY1()), sceneData);
			}

			// 連結先オブジェクトがある場合
			if (data->getConnectId1() > 0) {
				// 接続先オブジェクトのノードを取得
				bool isObject = false;
				bool isRope = false;

				// オブジェクトに紐付いている物理パーツの場合
				if (isWithObject) {
					// ベースとなるオブジェクトに接続する場合
					if (data->getConnectId1() == agtk::Object::DEFAULT_SCENE_PARTS_ID) {
						isObject = true;
						target = object->getphysicsNode();
					}
					// 他の物理オブジェクトに接続する場合
					else {

						// 物理オブジェクトを検索
						target = getConnectedPhysicsPartsFromList(physicsObjList, data->getConnectId1());
					}
				}
				// シーンに直接配置される物理パーツの場合
				else {
					target = getConnectedTarget(data->getConnectId1(), data->getConnectSubId1(), physicsObjList, &isObject);
				}

				if (target) {
					anchor = Vec2(data->getConnectX1(), -data->getConnectY1());
					pos = target->getPosition();

					// 通常オブジェクトに接続している場合
					if (isObject) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
						auto object = static_cast<agtk::Object *>(target->getParent());
#else
						auto object = dynamic_cast<agtk::Object *>(target->getParent());
#endif
						getObjectAnchorAndPos(object, &pos, &anchor);
					}
					else {
						// ロープ部品と接続しているかチェック
						target = checkConnectToRope(target, data->getConnectId1(), data->getConnectX1(), &anchor, &pos, physicsObjList, &isRope);
					}

					if (!isRope) {
						ignoreTargetList->addObject(target);
					}

					pos += rotateAnchor(anchor, target->getRotation());
				}
			}
			// シーンに接続される場合
			else {

				// オブジェクトに紐付いている物理パーツの場合
				if (isWithObject) {
					target = createStaticPhysicNode(pos, false);
					object->getPhysicsPartsList()->addObject(target);
				}
				// シーンに直接配置される物理パーツの場合
				else {
					// 固定用の見えない固定物理オブジェクトを生成する
					target = createStaticPhysicNode(Vec2(data->getConnectX1(), data->getConnectY1()));
				}

				this->addChild(target);
			}

			// 始点情報を保持
			preTarget->setPos(pos);
			preTarget->setAnchor(anchor);
			preTarget->setTarget(target);

			startTarget->setPos(pos);
			startTarget->setAnchor(anchor);
			startTarget->setTarget(target);

			isSetPartsStartPos = true;
			idx++;
		}
		// 中継点の場合
		else if (idx < idxMax - 1) {

			// 座標を算出
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
			auto posData = static_cast<cocos2d::__Array *>(data->getPointList()->getObjectAtIndex(idx - 1));
#else
			auto posData = dynamic_cast<cocos2d::__Array *>(data->getPointList()->getObjectAtIndex(idx - 1));
#endif
			auto pos = Vec2::ZERO;

			// オブジェクトに紐付いている物理パーツの場合
			if (isWithObject) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
				pos.x = objectPos.x + static_cast<cocos2d::Double *>(posData->getObjectAtIndex(0))->getValue();
				pos.y = objectPos.y - static_cast<cocos2d::Double *>(posData->getObjectAtIndex(1))->getValue();
#else
				pos.x = objectPos.x + dynamic_cast<cocos2d::Double *>(posData->getObjectAtIndex(0))->getValue();
				pos.y = objectPos.y - dynamic_cast<cocos2d::Double *>(posData->getObjectAtIndex(1))->getValue();
#endif
			}
			// シーンに直接配置される物理パーツの場合
			else {
				pos = Scene::getPositionSceneFromCocos2d(Vec2(
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
					static_cast<cocos2d::Double *>(posData->getObjectAtIndex(0))->getValue(),
					static_cast<cocos2d::Double *>(posData->getObjectAtIndex(1))->getValue()
#else
					dynamic_cast<cocos2d::Double *>(posData->getObjectAtIndex(0))->getValue(),
					dynamic_cast<cocos2d::Double *>(posData->getObjectAtIndex(1))->getValue()
#endif
				), sceneData);
			}

			if (!isSetPartsStartPos) {
				// 始点情報を保持
				preTarget->setPos(pos);
				isSetPartsStartPos = true;
				idx++;
			}
			else {
				curTarget->setPos(pos);
				isSetPartsEndPos = true;
			}
		}
		// 終点の場合
		else {

			// 座標を算出
			auto pos = Vec2::ZERO;
			auto anchor = Vec2(data->getConnectX2(), -data->getConnectY2());
			cocos2d::Node *target = nullptr;

			// オブジェクトに紐付いている物理パーツの場合
			if (isWithObject) {
				pos.x = objectPos.x + data->getConnectX2();
				pos.y = objectPos.y - data->getConnectY2();
			}
			// シーンに直接配置される物理パーツの場合
			else {
				pos = Scene::getPositionSceneFromCocos2d(Vec2(data->getConnectX2(), data->getConnectY2()), sceneData);
			}

			// 接続先オブジェクトがある場合
			if (data->getConnectId2() > 0) {
				// 接続先オブジェクトのノードを取得
				bool isObject = false;
				bool isRope = false;

				// オブジェクトに紐付いている物理パーツの場合
				if (isWithObject) {
					// ベースとなるオブジェクトに接続する場合
					if (data->getConnectId2() == agtk::Object::DEFAULT_SCENE_PARTS_ID) {
						isObject = true;
						target = object->getphysicsNode();
					}
					// 他の物理オブジェクトに接続する場合
					else {
						// 物理オブジェクトを検索
						target = getConnectedPhysicsPartsFromList(physicsObjList, data->getConnectId2());
					}
				}
				// シーンに直接配置される物理パーツの場合
				else {
					target = getConnectedTarget(data->getConnectId2(), data->getConnectSubId2(), physicsObjList, &isObject);
				}

				if (target) {
					pos = target->getPosition();

					// 通常オブジェクトに接続している場合
					if (isObject) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
						auto object = static_cast<agtk::Object *>(target->getParent());
#else
						auto object = dynamic_cast<agtk::Object *>(target->getParent());
#endif
						getObjectAnchorAndPos(object, &pos, &anchor);
					}
					else {
						// ロープ部品と接続しているかチェック
						target = checkConnectToRope(target, data->getConnectId2(), data->getConnectX2(), &anchor, &pos, physicsObjList, &isRope);
					}

					if (!isRope) {
						ignoreTargetList->addObject(target);
					}

					// 終点と繋がる対象を保持
					pos += rotateAnchor(anchor, target->getRotation());
					endTarget->setPos(pos);
					endTarget->setAnchor(anchor);
					endTarget->setTarget(target);
				}
			}

			// 終点情報を保持
			curTarget->setPos(pos);

			isSetPartsEndPos = true;
			idx++;
		}

		// 始点と終点を設定した場合
		if (isSetPartsStartPos && isSetPartsEndPos) {

			auto prePos = preTarget->getPos();
			auto curPos = curTarget->getPos();

			// ロープ部品の生成
			int parentScenePartId = object && object->getScenePartObjectData() ? object->getScenePartObjectData()->getId() : -1;
			auto ropeParts = PhysicsRopeParts::create(physicsPartData, sceneData, layerId, ropeIdx++, parentScenePartId);

			// 物理オブジェクトリストに登録
			physicsObjList->addObject(ropeParts);

			// シーンレイヤーに配置
#ifdef USE_REDUCE_RENDER_TEXTURE
			this->getObjectSetNode()->addChild(ropeParts, ropeParts->getPriority() + ropeParts->getDispPriority() * 1000);
#else
			this->addChild(ropeParts);
#endif

			// オブジェクトに紐付いている物理パーツの場合
			if (isWithObject) {
				// 紐付いているオブジェクトの物理パーツリストに登録
				object->getPhysicsPartsList()->addObject(ropeParts);
			}

			// 始点と終点の長さを算出
			auto length = curPos.getDistance(prePos);

			// エディタ上で始点と終点が伸び切ってしまう事があるのでここで補正
			auto fixLength = data->getLength() * DOT_PER_METER / data->getPointList()->count();

			if (length > fixLength) {
				length = fixLength;
			}

			// 部品の高さに算出した長さを加算
			ropeParts->addContentSizeH(length);

			// 始点と終点の中間点を算出
			auto centerPos = (curPos + prePos) * 0.5f;

			// 中間点の位置に部品を移動
			ropeParts->setPosition(centerPos);
			ropeParts->setOldPos(centerPos);

			// 部品を算出した角度へ変更
			ropeParts->setRotationFromTwoVec(prePos, curPos);

			// 
			curTarget->setTarget(ropeParts);
			curTarget->setAnchor(Vec2(0, length * 0.5f));

			// 始点と終点の設定したフラグを初期化
			isSetPartsStartPos = false;
			isSetPartsEndPos = false;

			// 一つ前のロープ部品がある場合
			if (preTarget->getTarget()) {
				// 生成したロープ部品とジョイント
				jointTwoConnectTarget(preTarget, curTarget);
			}

			// 生成したロープ部品を一つ前のロープ部品として保持
			preTarget->setTarget(ropeParts);
			preTarget->setPos(curPos);
			preTarget->setAnchor(Vec2(0, -length * 0.5f));
			curTarget->setTarget(nullptr);

			// ロープの衝突回避チェック用データ生成
			auto connectRope = ConnectRope::create();
			connectRope->setTarget(ropeParts);
			connectRope->setPrePos(prePos);
			connectRope->setEndPos(curPos);
			ropeList->addObject(connectRope);

			// オブジェクトに紐付いている物理パーツの場合
			if (isWithObject) {
				// 描画用リストへの登録
				registDrawListForPhysicsPartsWithObjectByDrawPriority(object, ropeParts);
			}
		}
	}

	// 終点に接続するオブジェクトがある場合
	if (endTarget->getTarget()) {
		// 終点に接続するオブジェクトとロープを接続
		jointTwoConnectTarget(preTarget, endTarget);

		// 始点または終点に接続するオブジェクトが動的である場合
		if (endTarget->getTarget()->getPhysicsBody()->isDynamic() || startTarget->getTarget()->getPhysicsBody()->isDynamic()) {
			auto len = data->getLength() * DOT_PER_METER + preTarget->getTarget()->getContentSize().height * 0.5f;
			auto joint2 = PhysicsJointLimit::construct(startTarget->getTarget()->getPhysicsBody(), endTarget->getTarget()->getPhysicsBody(), startTarget->getAnchor(), endTarget->getAnchor(), 0, len);
			GameManager::getInstance()->getPhysicsWorld()->addJoint(joint2);
		}
	}
	// 終点に接続するオブジェクトが無い場合
	else {
		// 重さを再計算
		// ※終端に向けて軽くすることでロープらしさを出す
		cocos2d::Ref *ref = nullptr;
		CCARRAY_FOREACH(ropeList, ref) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
			auto connectRope = static_cast<ConnectRope*>(ref);
			auto ropeParts = static_cast<PhysicsRopeParts *>(connectRope->getTarget());
#else
			auto connectRope = dynamic_cast<ConnectRope*>(ref);
			auto ropeParts = dynamic_cast<PhysicsRopeParts *>(connectRope->getTarget());
#endif
#define CANCEL_REARRANGE_MASS	//終端に向けて軽くする処理を無効化
#ifdef CANCEL_REARRANGE_MASS
#else
			ropeParts->reArangeMass();
#endif
		}
	}

	// ロープが接続先対象にめり込んでいるかチェック
	checkIgnoreRopeCollision(ropeList, ignoreTargetList);

	CCLOG("add Rope: %d %s", physicsPartData->getId(), (isWithObject ? "[with Obj]" : ""));
}

/**
* バネジョイントの生成
* @param	physicsPartData パーツデータ
* @param	sceneData		シーンデータ
* @param	layerId			レイヤーID
* @param	physicsObjList	格納先の物理オブジェクトリスト
* @param	object			物理パーツが紐付いているオブジェクト
*/
void SceneLayer::createSpringJoint(agtk::data::PhysicsPartData *physicsPartData, agtk::data::SceneData *sceneData, int layerId, cocos2d::__Array *physicsObjList, agtk::Object *object)
{
	// バネデータ取得
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
	auto data = static_cast<agtk::data::PhysicsSpringData *>(physicsPartData->getPhysicsData());
#else
	auto data = dynamic_cast<agtk::data::PhysicsSpringData *>(physicsPartData->getPhysicsData());
#endif

	// 始点と終点の接続先がシーンの場合は生成しない
	if (data->getConnectId1() < 0 && data->getConnectId2() < 0) {
		return;
	}

	bool isWithObject = (object != nullptr);

	// ワーク変数
	ConnectTarget *connect1 = ConnectTarget::create();	// 上位接続設定
	ConnectTarget *connect2 = ConnectTarget::create();	// 下位接続設定

														// 始点がオブジェクトに接続されている場合
	if (data->getConnectId1() > -1) {

		bool isObject = false;
		cocos2d::Node *obj = nullptr;

		// オブジェクトに付随する物理パーツの場合
		if (isWithObject) {
			// ベースとなるオブジェクトに接続する場合
			if (data->getConnectId1() == agtk::Object::DEFAULT_SCENE_PARTS_ID) {
				isObject = true;
				obj = object->getphysicsNode();
			}
			// 他の物理オブジェクトに接続する場合
			else {
				// 物理オブジェクトを検索
				obj = getConnectedPhysicsPartsFromList(physicsObjList, data->getConnectId1());
			}
		}
		// シーンに直接配置される物理パーツの場合
		else {
			obj = getConnectedTarget(data->getConnectId1(), data->getConnectSubId1(), physicsObjList, &isObject);
		}

		if (obj) {
			auto anchor = Vec2(data->getConnectX1(), -data->getConnectY1());
			auto pos = obj->getPosition();

			// 通常オブジェクトに接続している場合
			if (isObject) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
				auto object = static_cast<agtk::Object *>(obj->getParent());
#else
				auto object = dynamic_cast<agtk::Object *>(obj->getParent());
#endif
				getObjectAnchorAndPos(object, &pos, &anchor);
			}
			// 物理オブジェクトに接続している場合
			else {
				// ロープ部品と接続しているかチェック
				bool isRope = false;
				obj = checkConnectToRope(obj, data->getConnectId1(), data->getConnectX1(), &anchor, &pos, physicsObjList, &isRope);
			}

			pos += rotateAnchor(anchor, obj->getRotation());
			connect1->setAnchor(anchor);
			connect1->setPos(pos);
			connect1->setTarget(obj);
		}
	}
	// 始点がシーンに接続されている場合
	else {
		// 固定用の見えない固定物理オブジェクトを生成する
		cocos2d::Node * obj = nullptr;

		if (isWithObject) {
			// 紐付いているオブジェクトの座標からシーンへの座標を算出する
			auto objectPos = Scene::getPositionSceneFromCocos2d(object->getPosition(), sceneData);
			auto pos = Vec2(objectPos.x + data->getConnectX1(), objectPos.y - data->getConnectY1());
			obj = createStaticPhysicNode(pos, false);

			connect1->setPos(pos);
			connect1->setTarget(obj);
			object->getPhysicsPartsList()->addObject(obj);
		}
		else {
			obj = createStaticPhysicNode(Vec2(data->getConnectX1(), data->getConnectY1()));

			connect1->setPos(obj->getPosition());
		}

		connect1->setAnchor(Vec2::ZERO);
		connect1->setTarget(obj);
		this->addChild(obj);
	}

	// 終点がオブジェクトに接続されている場合
	if (data->getConnectId2() > -1) {
		bool isObject = false;
		cocos2d::Node* obj = nullptr;

		// オブジェクトに付随する物理パーツの場合
		if (isWithObject) {
			// ベースとなるオブジェクトに接続する場合
			if (data->getConnectId2() == agtk::Object::DEFAULT_SCENE_PARTS_ID) {
				isObject = true;
				obj = object->getphysicsNode();
			}
			// 他の物理オブジェクトに接続する場合
			else {
				// 物理オブジェクトを検索
				obj = getConnectedPhysicsPartsFromList(physicsObjList, data->getConnectId2());
			}
		}
		// シーンに直接配置される物理パーツの場合
		else {
			obj = getConnectedTarget(data->getConnectId2(), data->getConnectSubId2(), physicsObjList, &isObject);
		}

		if (obj) {
			auto anchor = Vec2(data->getConnectX2(), -data->getConnectY2());
			auto pos = obj->getPosition();

			// 通常オブジェクトに接続している場合
			if (isObject) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
				auto object = static_cast<agtk::Object *>(obj->getParent());
#else
				auto object = dynamic_cast<agtk::Object *>(obj->getParent());
#endif
				getObjectAnchorAndPos(object, &pos, &anchor);
			}
			// 物理オブジェクトに接続している場合
			else {
				// ロープ部品と接続しているかチェック
				bool isRope = false;
				obj = checkConnectToRope(obj, data->getConnectId2(), data->getConnectX2(), &anchor, &pos, physicsObjList, &isRope);
			}

			pos += rotateAnchor(anchor, obj->getRotation());
			connect2->setAnchor(anchor);
			connect2->setPos(pos);
			connect2->setTarget(obj);
		}
	}
	// 終点がシーンに接続されている場合
	else {
		// 固定用の見えない固定物理オブジェクトを生成する
		cocos2d::Node *obj = nullptr;

		if (isWithObject) {
			// 紐付いているオブジェクトの座標からシーンへの座標を算出する
			auto objectPos = Scene::getPositionSceneFromCocos2d(object->getPosition(), sceneData);
			auto pos = Vec2(objectPos.x + data->getConnectX2(), objectPos.y - data->getConnectY2());
			obj = createStaticPhysicNode(pos, false);

			connect2->setPos(pos);
			object->getPhysicsPartsList()->addObject(obj);
		}
		else {
			auto pos = Vec2(data->getConnectX2(), data->getConnectY2());
			obj = createStaticPhysicNode(pos);

			connect2->setPos(obj->getPosition());
		}

		connect2->setAnchor(Vec2::ZERO);
		connect2->setTarget(obj);
		this->addChild(obj);
	}

	// バネを生成して接続する
	auto joint = PhysicsJointSpring::construct(
		connect1->getTarget()->getPhysicsBody(),			// 接続点1
		connect2->getTarget()->getPhysicsBody(),			// 接続点2
		connect1->getAnchor(),								// 接続点1のオブジェクト上の位置
		connect2->getAnchor(),								// 接続点2のオブジェクト上の位置
		data->getSpringConstant() * 1000 / DOT_PER_METER,	// バネ定数
		data->getDampingCoefficient()						// 減衰係数
	);

	joint->createConstraints();
	joint->setRestLength(data->getNaturalLength() * DOT_PER_METER);	// 自然長を設定
	GameManager::getInstance()->getPhysicsWorld()->addJoint(joint);

	// バネオブジェクト生成
	int parentScenePartId = object && object->getScenePartObjectData() ? object->getScenePartObjectData()->getId() : -1;
	auto spring = PhysicsSpring::create(physicsPartData, sceneData, layerId, joint, parentScenePartId);

	// 接続点1と接続点2の長さを算出
	auto length = connect2->getPos().getDistance(connect1->getPos());

	// バネオブジェクトの長さを設定
	spring->setUp(length);

	// 接続点1と接続点2の中間点を算出
	auto centerPos = (connect1->getPos() + connect2->getPos()) * 0.5f;

	// バネオブジェクトの位置を設定
	spring->setPosition(centerPos);

	// 部品を算出した角度へ変更
	spring->setRotationFromTwoVec(connect1->getPos(), connect2->getPos());

	// シーンレイヤーに配置
#ifdef USE_REDUCE_RENDER_TEXTURE
	this->getObjectSetNode()->addChild(spring, spring->getPriority() + spring->getDispPriority() * 1000);
#else
	this->addChild(spring);
#endif

	// 物理オブジェクトリストに登録
	physicsObjList->addObject(spring);

	// オブジェクトに紐付く物理パーツの場合
	if (isWithObject) {
		// 紐付いているオブジェクトの物理パーツリストに登録
		object->getPhysicsPartsList()->addObject(spring);
	}

	// 連結点1とバネの角度を固定する場合
	if (data->getFixConnectAngle1()) {
		auto groovVecNormal = (connect2->getPos() - connect1->getPos()).getNormalized();
		auto joint = PhysicsJointGroove::construct(
			connect1->getTarget()->getPhysicsBody(),
			connect2->getTarget()->getPhysicsBody(),
			connect1->getAnchor(),
			connect1->getAnchor() + (groovVecNormal * length * 10),
			connect2->getAnchor());
		joint->createConstraints();
		GameManager::getInstance()->getPhysicsWorld()->addJoint(joint);
	}

	// 連結点2とバネの角度を固定する場合
	if (data->getFixConnectAngle2()) {
		auto groovVecNormal = (connect1->getPos() - connect2->getPos()).getNormalized();
		auto joint = PhysicsJointGroove::construct(
			connect2->getTarget()->getPhysicsBody(),
			connect1->getTarget()->getPhysicsBody(),
			connect2->getAnchor(),
			connect2->getAnchor() + (groovVecNormal * length * 10),
			connect1->getAnchor());
		joint->createConstraints();
		GameManager::getInstance()->getPhysicsWorld()->addJoint(joint);
	}

	if (isWithObject) {
		// 描画用リストへの登録
		registDrawListForPhysicsPartsWithObjectByDrawPriority(object, spring);

	}

	CCLOG("add Spring: %d %s", spring->getScenePartsId(), (isWithObject ? "[with Obj]" : ""));
}

/**
* 回転軸の生成
* @param	physicsPartData パーツデータ
* @param	sceneData		シーンデータ
* @param	layerId			レイヤーID
* @param	physicsObjList	格納先の物理オブジェクトリスト
* @param	object			物理パーツが紐付いているオブジェクト
*/
void SceneLayer::createAxisJoint(agtk::data::PhysicsPartData *physicsPartData, agtk::data::SceneData *sceneData, int layerId, cocos2d::__Array *physicsObjList, agtk::Object *object)
{
	// 回転軸のデータを取得
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
	auto data = static_cast<agtk::data::PhysicsAxisData *>(physicsPartData->getPhysicsData());
#else
	auto data = dynamic_cast<agtk::data::PhysicsAxisData *>(physicsPartData->getPhysicsData());
#endif

	// 接続元と接続先が無い場合
	if (data->getLowerId() < 0 && data->getUpperId() < 0) {
		// 何もしない
		return;
	}

	ConnectTarget *lower = ConnectTarget::create();	// 上位接続設定
	Vec2 lowerBaseAnchor = Vec2::ZERO;
	ConnectTarget *upper = ConnectTarget::create();	// 下位接続設定
	Vec2 upperBaseAnchor = Vec2::ZERO;

	bool isWithObject = (object != nullptr);

	// 回転軸オブジェクトを生成(見た目用)
	int parentScenePartId = object && object->getScenePartObjectData() ? object->getScenePartObjectData()->getId() : -1;
	auto axis = PhysicsAxis::create(physicsPartData, sceneData, layerId, parentScenePartId);

	// -------------------------------------------------------------------------------
	// 下位の連結対象設定
	// -------------------------------------------------------------------------------
	// 接続元がある場合
	if (data->getLowerId() > 0) {
		bool isObject = false;
		cocos2d::Node* obj = nullptr;

		// オブジェクトに付随する物理パーツの場合
		if (isWithObject) {
			// ベースとなるオブジェクトに接続する場合
			if (data->getLowerId() == agtk::Object::DEFAULT_SCENE_PARTS_ID) {
				isObject = true;
				obj = object->getphysicsNode();
			}
			// 他の物理オブジェクトに接続する場合
			else {
				// 物理オブジェクトを検索
				obj = getConnectedPhysicsPartsFromList(physicsObjList, data->getLowerId());
			}
		}
		// シーンに直接配置される物理パーツの場合
		else {
			obj = getConnectedTarget(data->getLowerId(), data->getLowerSubId(), physicsObjList, &isObject);
		}

		if (obj) {
			lowerBaseAnchor = Vec2(data->getLowerX(), -data->getLowerY());
			auto pos = obj->getPosition();

			// 通常オブジェクトに接続している場合
			if (isObject) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
				auto object = static_cast<agtk::Object *>(obj->getParent());
#else
				auto object = dynamic_cast<agtk::Object *>(obj->getParent());
#endif
				getObjectAnchorAndPos(object, &pos, &lowerBaseAnchor);

				// シーンに直接配置される or 付随先オブジェクトが接続された物理パーツの動作を優先しない場合
				if (!isWithObject || !object->getPlayObjectData()->getSwitchData(agtk::data::kObjectSystemSwitchFollowConnectedPhysics)->getValue()) {

					// 下位接続オブジェクトの参照を保持
					axis->setFollowObject(object);
				}
			}
			else {
				// ロープ部品と接続しているかチェック
				bool isRope = false;
				obj = checkConnectToRope(obj, data->getLowerId(), data->getLowerX(), &lowerBaseAnchor, &pos, physicsObjList, &isRope);

				// 下位接続オブジェクトの物理ボディを設定
				axis->setFollowerPhysicsBody(obj->getPhysicsBody());
			}

			lower->setAnchor(rotateAnchor(lowerBaseAnchor, obj->getRotation()));
			lower->setPos(pos);
			lower->setTarget(obj);
		}
	}
	else {

		Vec2 pos = Vec2::ZERO;
		cocos2d::Node *staticObj = nullptr;

		if (isWithObject) {
			// 紐付いているオブジェクトの座標からシーンへの座標を算出する
			auto objectPos = Scene::getPositionSceneFromCocos2d(object->getPosition(), sceneData);
			pos = Vec2(objectPos.x + data->getLowerX(), objectPos.y - data->getLowerY());

			staticObj = createStaticPhysicNode(pos, false);
			object->getPhysicsPartsList()->addObject(staticObj);

		}
		else {
			// 固定用の見えない固定物理オブジェクトを生成する
			pos = Scene::getPositionSceneFromCocos2d(Vec2(data->getLowerX(), data->getLowerY()), sceneData);

			staticObj = createStaticPhysicNode(Vec2(data->getLowerX(), data->getLowerY()));
		}

		this->addChild(staticObj);
		lower->setPos(pos);
		lower->setAnchor(Vec2::ZERO);
		lower->setTarget(staticObj);
	}

	// -------------------------------------------------------------------------------
	// 上位の連結対象設定
	// -------------------------------------------------------------------------------
	CCASSERT(data->getUpperId() > 0, "Upper Object must be attached !!");
	bool isObject = false;
	cocos2d::Node *obj = nullptr;

	// オブジェクトに紐付いている物理パーツの場合
	if (isWithObject) {
		// ベースとなるオブジェクトに接続する場合
		if (data->getUpperId() == agtk::Object::DEFAULT_SCENE_PARTS_ID) {
			isObject = true;
			obj = object->getphysicsNode();
		}
		// 他の物理オブジェクトに接続する場合
		else {
			// 物理オブジェクトを検索
			obj = getConnectedPhysicsPartsFromList(physicsObjList, data->getUpperId());
		}
	}
	// シーンに直接配置する物理パーツの場合
	else {
		obj = getConnectedTarget(data->getUpperId(), data->getUpperSubId(), physicsObjList, &isObject);
	}

	if (obj) {
		upperBaseAnchor = Vec2(data->getUpperX(), -data->getUpperY());
		auto pos = obj->getPosition();

		// 通常オブジェクトに接続している場合
		if (isObject) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
			auto object = static_cast<agtk::Object *>(obj->getParent());
#else
			auto object = dynamic_cast<agtk::Object *>(obj->getParent());
#endif
			getObjectAnchorAndPos(object, &pos, &upperBaseAnchor);

			// 回転軸が追従するオブジェクトが未指定 AND (シーンに直接配置される OR 接続先オブジェクトが接続された物理パーツの動作を優先しない) 場合
			if (!axis->getFollowObject() && (!isWithObject || !object->getPlayObjectData()->getSwitchData(agtk::data::kObjectSystemSwitchFollowConnectedPhysics)->getValue())) {
				// 上位接続オブジェクトの参照を保持
				axis->setFollowObject(object);
			}
		}
		else {
			// ロープ部品と接続しているかチェック
			bool isRope = false;
			obj = checkConnectToRope(obj, data->getUpperId(), data->getUpperX(), &upperBaseAnchor, &pos, physicsObjList, &isRope);

			if (!axis->getFollowerPhysicsBody()) {
				axis->setFollowerPhysicsBody(obj->getPhysicsBody());
			}
		}

		upper->setAnchor(rotateAnchor(upperBaseAnchor, obj->getRotation()));
		upper->setPos(pos);
		upper->setTarget(obj);
	}

	// 回転軸の位置を設定
	axis->setPosition(upperBaseAnchor + (isObject ? Vec2::ZERO : upper->getTarget()->getContentSize() * 0.5f));
	physicsObjList->addObject(axis);

	// ACT2-4667 表示優先度が機能しないのでレイヤー配置するように変更
	// upper->getTarget()->addChild(axis);

	Node* dummyNode = Node::create();
	dummyNode->setAnchorPoint(axis->getAnchorPoint());
	dummyNode->setScale(axis->getScale());
	dummyNode->setRotation(axis->getRotation());
	dummyNode->setContentSize(axis->getContentSize());
	dummyNode->setPosition(axis->getPosition());
	dummyNode->setVisible(false);

	upper->getTarget()->addChild(dummyNode, axis->getPriority());
#ifdef USE_REDUCE_RENDER_TEXTURE
	this->getObjectSetNode()->addChild(axis, axis->getPriority() + axis->getDispPriority() * 1000);
#else
	this->addChild(axis);
#endif

	auto physicsBase = dynamic_cast<agtk::PhysicsBase *>(upper->getTarget());
	if (physicsBase) {
		auto nodeList = physicsBase->getConnectToBaseList();
		nodeList.push_back({ dummyNode, axis });
		physicsBase->setConnectToBaseList(nodeList);
	}

	if (isWithObject) {
		axis->setRootObject(object);
	}

	// -------------------------------------------------------------------------------
	// 回転用パラメータ設定
	// -------------------------------------------------------------------------------
	// RPMから単位秒当たりの最大回転角度を算出
	float limitAngularVelocity = data->getRpm() / 10.0f;

	// 最大回転角度を設定
	lower->getTarget()->getPhysicsBody()->setAngularVelocityLimit(limitAngularVelocity);
	upper->getTarget()->getPhysicsBody()->setAngularVelocityLimit(limitAngularVelocity);

	// 回転対象を設定
	axis->setAxisUpperTarget(upper->getTarget()->getPhysicsBody());
	axis->setAxisLowerTarget(lower->getTarget()->getPhysicsBody());

	// -------------------------------------------------------------------------------
	// 連結処理
	// -------------------------------------------------------------------------------
	// 上位接続対象または下位接続対象のどちらかが動的物理オブジェクトの場合
	if (upper->getTarget()->getPhysicsBody()->isDynamic() || lower->getTarget()->getPhysicsBody()->isDynamic()) {
		bool createPin = true;
		if (obj && isObject && !lower->getTarget()->getPhysicsBody()->isDynamic()) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
			auto object = static_cast<agtk::Object *>(obj->getParent());
#else
			auto object = dynamic_cast<agtk::Object *>(obj->getParent());
#endif
			auto playObjectData = object->getPlayObjectData();
			auto physicsSetting = object->getObjectData()->getPhysicsSetting();
			if (!playObjectData->getSwitchData(agtk::data::kObjectSystemSwitchAffectedByOtherObjects)->getValue() && !playObjectData->getSwitchData(agtk::data::kObjectSystemSwitchAffectOtherObjects)->getValue() && !playObjectData->getSwitchData(agtk::data::kObjectSystemSwitchFollowConnectedPhysics)->getValue()) {
				createPin = false;
			}
		}
		if (createPin) {
			// 下位接続対象と上位接続対象をピンでジョイント
			auto joint1 = PhysicsJointPin::construct(upper->getTarget()->getPhysicsBody(), lower->getTarget()->getPhysicsBody(), lower->getPos() + lower->getAnchor());
			joint1->setCollisionEnable(false);
			GameManager::getInstance()->getPhysicsWorld()->addJoint(joint1);
		}

	}
	{
		// 上位パーツがオブジェクトに紐付いている場合は、そのオブジェクトのパーツリストに入れる。
		auto target = upper->getTarget();
		cocos2d::Ref *ref = nullptr;
		auto objectList = this->getObjectList();
		CCARRAY_FOREACH(objectList, ref) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
			auto o = static_cast<agtk::Object *>(ref);
#else
			auto o = dynamic_cast<agtk::Object *>(ref);
#endif
			if (o->getPhysicsPartsList()->containsObject(target)) {
				o->getPhysicsPartsList()->addObject(axis);
			}
		}
	}

	if (isWithObject) {

		// 描画用リストへの登録
		registDrawListForPhysicsPartsWithObjectByDrawPriority(object, axis);

	}

	CCLOG("add Axis: %d %s", axis->getScenePartsId(), (isWithObject ? "[with Obj]" : ""));
}

/**
* 爆発ポイントの生成
* @param	physicsPartData パーツデータ
* @param	sceneData		シーンデータ
* @param	layerId			レイヤーID
* @param	physicsObjList	格納先の物理オブジェクトリスト
* @param	object			物理パーツが紐付いているオブジェクト
*/
void SceneLayer::createExplode(agtk::data::PhysicsPartData *physicsPartData, agtk::data::SceneData *sceneData, int layerId, cocos2d::__Array *physicsObjList, agtk::Object *object)
{
	// 爆発のデータを取得
	auto data = dynamic_cast<agtk::data::PhysicsExplosionData *>(physicsPartData->getPhysicsData());
	if (!data) {
		return;
	}

	bool isWithObject = (object != nullptr);

	// 爆発用オブジェクト生成
	int parentScenePartId = object && object->getScenePartObjectData() ? object->getScenePartObjectData()->getId() : -1;
	auto explosion = PhysicsExprosion::create(data, sceneData, layerId, parentScenePartId);

	// オブジェクトに接続されている場合
	if (data->getConnectId() > 0) {
		Vec2 connectAnchor = Vec2::ZERO;

		bool isObject = false;
		cocos2d::Node* obj = nullptr;

		// オブジェクトに紐付いている場合
		if (isWithObject) {
			// ベースとなるオブジェクトに接続する場合
			if (data->getConnectId() == agtk::Object::DEFAULT_SCENE_PARTS_ID) {
				isObject = true;
				obj = object->getphysicsNode();
			}
			// 他の物理オブジェクトに接続する場合
			else {
				// 物理オブジェクトを検索
				obj = getConnectedPhysicsPartsFromList(physicsObjList, data->getConnectId());
			}
		}
		// シーンに直接配置される場合
		else {
			// 接続されているオブジェクトを取得する
			obj = getConnectedTarget(data->getConnectId(), data->getConnectSubId(), physicsObjList, &isObject);
		}

		if (obj) {
			connectAnchor = Vec2(data->getConnectX(), -data->getConnectY());
			auto pos = obj->getPosition();

			// 通常オブジェクトに接続している場合
			if (isObject) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
				auto object = static_cast<agtk::Object *>(obj->getParent());
#else
				auto object = dynamic_cast<agtk::Object *>(obj->getParent());
#endif
				getObjectAnchorAndPos(object, &pos, &connectAnchor);
			}
			else {
				// ロープ部品と接続しているかチェック
				bool isRope = false;
				obj = checkConnectToRope(obj, data->getConnectId(), data->getConnectX(), &connectAnchor, &pos, physicsObjList, &isRope);
			}

			// 爆発の位置・連結元のアンカー・連結基オブジェクト・連結元オブジェクトの角度を設定
			explosion->setPosition(pos + rotateAnchor(connectAnchor, obj->getRotation()));
			explosion->setConnectedAnchor(connectAnchor);
			explosion->setConnectedTarget(obj);
			explosion->setConnectedInitAngle(obj->getRotation());
		}
	}
	// シーンに接続される場合
	else {
		if (isWithObject) {
			// 紐付いているオブジェクトの座標からシーンへの座標を算出する
			auto objectPos = Scene::getPositionSceneFromCocos2d(object->getPosition(), sceneData);
			explosion->setPosition(Vec2(objectPos.x + data->getConnectX(), objectPos.y - data->getConnectY()));
		}
		else {
			auto pos = Scene::getPositionSceneFromCocos2d(Vec2(data->getConnectX(), data->getConnectY()), sceneData);
			explosion->setPosition(pos);
		}
	}

	this->addChild(explosion);
	physicsObjList->addObject(explosion);

	if (isWithObject) {
		explosion->setRootObject(object);
		object->getPhysicsPartsList()->addObject(explosion);
	}

	CCLOG("add Explosion: %d %s", explosion->getScenePartsId(), ((isWithObject ? "[with Obj]" : "")));
}

/**
* 引力・斥力ポイントの生成
* @param	physicsPartData パーツデータ
* @param	sceneData		シーンデータ
* @param	layerId			レイヤーID
* @param	physicsObjList	格納先の物理オブジェクトリスト
* @param	object			物理パーツが紐付いているオブジェクト
*/
void SceneLayer::createForce(agtk::data::PhysicsPartData *physicsPartData, agtk::data::SceneData *sceneData, int layerId, cocos2d::__Array *physicsObjList, agtk::Object *object)
{
	// 引力・斥力のデータを取得
	auto data = dynamic_cast<agtk::data::PhysicsForceData *>(physicsPartData->getPhysicsData());
	if (!data) {
		return;
	}

	bool isWithObject = (object != nullptr);

	// 引力・斥力用オブジェクト生成
	int parentScenePartId = object && object->getScenePartObjectData() ? object->getScenePartObjectData()->getId() : -1;
	auto attract = PhysicsAttraction::create(data, sceneData, layerId, parentScenePartId);

	// オブジェクトに接続されている場合
	if (data->getConnectId() > 0) {
		Vec2 connectAnchor = Vec2::ZERO;

		bool isObject = false;
		cocos2d::Node* obj = nullptr;

		// オブジェクトに紐付いている場合
		if (isWithObject) {
			// ベースとなるオブジェクトに接続する場合
			if (data->getConnectId() == agtk::Object::DEFAULT_SCENE_PARTS_ID) {
				isObject = true;
				obj = object->getphysicsNode();
			}
			// 他の物理オブジェクトに接続する場合
			else {
				// 物理オブジェクトを検索
				obj = getConnectedPhysicsPartsFromList(physicsObjList, data->getConnectId());
			}
		}
		// シーンに直接配置される場合
		else {
			obj = getConnectedTarget(data->getConnectId(), data->getConnectSubId(), physicsObjList, &isObject);
		}

		if (obj) {
			connectAnchor = Vec2(data->getConnectX(), -data->getConnectY());
			auto pos = obj->getPosition();

			// 通常オブジェクトに接続している場合
			if (isObject) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
				auto object = static_cast<agtk::Object *>(obj->getParent());
#else
				auto object = dynamic_cast<agtk::Object *>(obj->getParent());
#endif
				getObjectAnchorAndPos(object, &pos, &connectAnchor);
			}
			else {
				// ロープ部品と接続しているかチェック
				bool isRope = false;
				obj = checkConnectToRope(obj, data->getConnectId(), data->getConnectX(), &connectAnchor, &pos, physicsObjList, &isRope);
			}

			// 引力・斥力の位置・連結元のアンカー・連結基オブジェクト・連結元オブジェクトの角度を設定
			attract->setPosition(pos + rotateAnchor(connectAnchor, obj->getRotation()));
			attract->setConnectedAnchor(connectAnchor);
			attract->setConnectedTarget(obj);
			attract->setConnectedInitAngle(obj->getRotation());
		}
	}
	// シーンに接続される場合
	else {
		if (isWithObject) {
			// 紐付いているオブジェクトの座標からシーンへの座標を算出する
			auto objectPos = Scene::getPositionSceneFromCocos2d(object->getPosition(), sceneData);
			attract->setPosition(Vec2(objectPos.x + data->getConnectX(), objectPos.y - data->getConnectY()));
		}
		else {
			auto pos = Scene::getPositionSceneFromCocos2d(Vec2(data->getConnectX(), data->getConnectY()), sceneData);
			attract->setPosition(pos);
		}
	}

	this->addChild(attract);
	physicsObjList->addObject(attract);

	if (isWithObject) {
		attract->setRootObject(object);
		object->getPhysicsPartsList()->addObject(attract);
	}

	CCLOG("add Force: %d %s", attract->getScenePartsId(), ((isWithObject ? "[with Obj]" : "")));
}
#pragma endregion

void SceneLayer::createGroupCollisionDetections(bool isNeedCommon)
{
	auto commonCollisionCheckFunc = [&](CollisionNode* collisionObject1, CollisionNode* collisionObject2) {
#ifdef USE_COLLISION_MEASURE
		roughHitCollisionCount++;
#endif
		auto object1 = dynamic_cast<agtk::Object *>(collisionObject1->getNode());
		auto object2 = dynamic_cast<agtk::Object *>(collisionObject2->getNode());
		agtk::data::ObjectData *objectData1 = nullptr;
		agtk::data::ObjectData *objectData2 = nullptr;

		// ---------------------------------------------------------------------------
		// ※getTimelineList() は処理コストが別格なため先に一度だけコールする
		// ---------------------------------------------------------------------------
		if (object1) {
			objectData1 = object1->getObjectData();
		}
		if (object2) {
			objectData2 = object2->getObjectData();
		}

		// ポータルとの衝突判定メソッド
		auto checkPortalCollision = [](agtk::ObjectCollision* collision, agtk::Object *object, agtk::Portal *portal) {
			// 2018.04.06 ACT2-1563 変更 [オブジェクトの中心点 -> オブジェクトの原点]
			// オブジェクトの原点とポータルのバウンディングボックスを取得
			auto point = Scene::getPositionCocos2dFromScene(object->getPosition());
			auto portalBox = portal->getBoundingBox();
#ifdef FIX_ACT2_4774
			if (!portal->getIsMovable() || collision->getPortalList()->containsObject(portal)) {
				return;
			}
			bool portalHit = portalBox.containsPoint(point);
			// ポータル移動直後で移動先のポータルで出現直後の再移動をチェック。移動によりポータルの外に出てしまうことを考慮し、
			if(object->getIsPortalWarped() && object->getWarpedTransitionPortalId() == portal->getId()){
				auto dispBit = object->getPortalMoveDispBit() & portal->getRePortalDirectionBit();
				if (dispBit) {
					auto premovePoint = Scene::getPositionCocos2dFromScene(object->getPremoveObjectPosition());
					if (portalBox.containsPoint(premovePoint)) {
						portalHit = true;
						object->setIsPortalWarped(false);
						object->setWarpedTransitionPortalId(-1);
					}
				}
			}
			if (portalHit) {
				// 衝突ポータルリストに登録
				collision->getPortalList()->addObject(portal);
				//オブジェクトの「ポータルに触れた」スイッチの変更。
				auto switchData = object->getPlayObjectData()->getSwitchData(agtk::data::kObjectSystemSwitchPortalTouched);
				bool bReadOnly = switchData->getReadOnly();
				switchData->setReadOnly(false);
				//オブジェクトの「ポータルに触れた」スイッチをON
				switchData->setValue(true);
				switchData->setReadOnly(bReadOnly);
			}
#else
			// オブジェクトの中心点がポータルのバウンディングボックスに含まれていて、衝突ポータルリストに未登録の場合、かつ移動可能の場合。
			if (portalBox.containsPoint(point) && !collision->getPortalList()->containsObject(portal) && portal->getIsMovable()) {
				// 衝突ポータルリストに登録
				collision->getPortalList()->addObject(portal);

				// ポータル移動直後で最初に触れたポータルが無い場合
				if (object->getIsPortalWarped() && !object->_firstTouchedPortalName) {
					// 最初に触れたポータルのユニーク名を設定
					object->_firstTouchedPortalName = portal->getName().c_str();
				}
			}

			//オブジェクトの「ポータルに触れた」スイッチの変更。
			auto switchData = object->getPlayObjectData()->getSwitchData(agtk::data::kObjectSystemSwitchPortalTouched);
			bool bReadOnly = switchData->getReadOnly();
			switchData->setReadOnly(false);
			if (collision->getPortalList()->count() > 0 && switchData->getValue() == false) {
				//オブジェクトの「ポータルに触れた」スイッチをON
				switchData->setValue(true);
			}
			else if (collision->getPortalList()->count() == 0 && switchData->getValue() == true) {
				//オブジェクトの「ポータルに触れた」スイッチをOFF
				switchData->setValue(false);
			}
			switchData->setReadOnly(bReadOnly);
#endif
		};

		if (object1) {
			// object1 がオブジェクトの場合
			auto portal = dynamic_cast<agtk::Portal *>(collisionObject2->getNode());
			// object2 がポータルの場合
			if (portal) {
				auto collision = object1->getObjectCollision();
				checkPortalCollision(collision, object1, portal);
			}
		}

		// object2 がオブジェクトの場合
		if (object2) {
			auto portal = dynamic_cast<agtk::Portal *>(collisionObject1->getNode());
			// object1 がポータルの場合
			if (portal) {
				auto collision = object2->getObjectCollision();
				checkPortalCollision(collision, object2, portal);
			}
		}
	};
	auto hitCollisionCheckFunc = [&](CollisionNode* collisionObject1, CollisionNode* collisionObject2) {
#ifdef USE_COLLISION_MEASURE
		roughHitCollisionCount++;
#endif
		auto object1 = dynamic_cast<agtk::Object *>(collisionObject1->getNode());
		auto object2 = dynamic_cast<agtk::Object *>(collisionObject2->getNode());
		if (object1 == object2) {
			//自身への当たりは無視。
			return;
		}
		agtk::data::ObjectData *objectData1 = nullptr;
		unsigned int groupBit1;
		//unsigned int hitGroupBit1;
		agtk::data::ObjectData *objectData2 = nullptr;
		//unsigned int groupBit2;
		unsigned int hitGroupBit2;

		// ---------------------------------------------------------------------------
		// ※getTimelineList() は処理コストが別格なため先に一度だけコールする
		// ---------------------------------------------------------------------------
		if (object1) {
			objectData1 = object1->getObjectData();
			groupBit1 = (1 << objectData1->getGroup());
			//hitGroupBit1 = (playObjectData1->getHitPlayerGroup() ? (1 << 0) : 0) | (playObjectData1->getHitEnemyGroup() ? (1 << 1) : 0);
		}
		if (object2) {
			objectData2 = object2->getObjectData();
			auto playObjectData2 = object2->getPlayObjectData();
			//groupBit2 = (1 << objectData2->getType());
			hitGroupBit2 = playObjectData2->getHitObjectGroupBit();
		}
		CC_ASSERT(groupBit1 & hitGroupBit2);

		//bool attackHit12 = false;
		bool attackHit21 = false;

		if (object2->getPlayer()) {
			if (object1->getPlayer()) {
				if ((hitGroupBit2 & groupBit1)) {
// #AGTK-NX
#if CC_TARGET_PLATFORM != CC_PLATFORM_NX
					auto &attackVert2 = object2->getPlayer()->getAttackVertListCache();
#else
#endif
					if (attackVert2.size() > 0) {
// #AGTK-NX
#if CC_TARGET_PLATFORM != CC_PLATFORM_NX
						auto &hitVert1 = object1->getPlayer()->getHitVertListCache();
#else
#endif
						if (hitVert1.size() > 0) {
							auto attackRect2 = object2->getPlayer()->getAttackVertRectCache();
							auto hitRect1 = object1->getPlayer()->getHitVertRectCache();
							Rect checkHitRect = attackRect2;
							// 壁当たり判定の補正により近接していても接触できない事象が発生しているので
							// object1の矩形を少し補正
							checkHitRect.origin.x -= TILE_COLLISION_THRESHOLD;
							checkHitRect.origin.y -= TILE_COLLISION_THRESHOLD;
							checkHitRect.size.width += TILE_COLLISION_THRESHOLD * 2;
							checkHitRect.size.height += TILE_COLLISION_THRESHOLD * 2;
							if (checkHitRect.intersectsRect(hitRect1)) {
								attackHit21 = true;
							}
						}
					}
				}
			}
		}

		// object2 がオブジェクトの場合
		if (object2) {
			auto collision = object2->getObjectCollision();
			//todo 当たりと衝突のhitは分離した方が効率が良い。
			if (attackHit21) {
				collision->addHitObject(object1);
			}
		}
	};

	if (isNeedCommon)
	{
		auto collision = CollisionDetaction::create();
		collision->init(this, 3, false, commonCollisionCheckFunc);
		collision->name = "CollisionDetection:common";
		this->setCommonCollisionDetection(collision);
	}

	cocos2d::__Array* ary = cocos2d::Array::create();
	auto objGroup = GameManager::getInstance()->getProjectData()->getObjectGroup();
	for (int group = 0; group < objGroup->count(); ++group) 
	{
		auto collision = CollisionDetaction::create();
		collision->init(this, 3, false, hitCollisionCheckFunc);
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
		collision->name = std::string("CollisionDetection:") + static_cast<cocos2d::__String*>(objGroup->getObjectAtIndex(group))->getCString();
#else
		collision->name = std::string("CollisionDetection:") + dynamic_cast<cocos2d::__String*>(objGroup->getObjectAtIndex(group))->getCString();
#endif
		ary->addObject(collision);
	}
	setGroupCollisionDetections(ary);
}

void SceneLayer::createGroupRoughWallCollisionDetections()
{
	auto wallCollisionCheckFunc = [&](CollisionNode* collisionObject1, CollisionNode* collisionObject2) {
#ifdef USE_COLLISION_MEASURE
		roughHitCollisionCount++;
#endif
		auto object1 = dynamic_cast<agtk::Object *>(collisionObject1->getNode());
		auto object2 = dynamic_cast<agtk::Object *>(collisionObject2->getNode());
		if (object1 == object2) {
			//自身との壁判定は無視。
			return;
		}
		agtk::data::ObjectData *objectData1 = nullptr;
		unsigned int groupBit1;
		agtk::data::ObjectData *objectData2 = nullptr;
		unsigned int collideGroupBit2;

		// ---------------------------------------------------------------------------
		// ※getTimelineList() は処理コストが別格なため先に一度だけコールする
		// ---------------------------------------------------------------------------
		if (object1) {
			objectData1 = object1->getObjectData();
			groupBit1 = objectData1->getGroupBit();
		}
		if (object2) {
			objectData2 = object2->getObjectData();
			collideGroupBit2 = objectData2->getCollideWithObjectGroupBit();
		}
		CC_ASSERT(groupBit1 & collideGroupBit2);

		bool wallHit21 = false;
		if (object2->getPlayer()) {
			if (object1->getPlayer()) {
				if ((collideGroupBit2 & groupBit1)) {
// #AGTK-NX
#if CC_TARGET_PLATFORM != CC_PLATFORM_NX
					auto &wallVert2 = object2->getPlayer()->getWallVertListCache();
#else
#endif
					if (wallVert2.size() > 0) {
// #AGTK-NX
#if CC_TARGET_PLATFORM != CC_PLATFORM_NX
						auto &wallVert1 = object1->getPlayer()->getWallVertListCache();
#else
#endif
						if (wallVert1.size() > 0) {
							auto wallRect2 = object2->getPlayer()->getWallVertRectCache();
							auto wallRect1 = object1->getPlayer()->getWallVertRectCache();
							Rect checkWallRect = wallRect2;
							// 壁当たり判定の補正により近接していても接触できない事象が発生しているので
							// object1の矩形を少し補正
							checkWallRect.origin.x -= TILE_COLLISION_THRESHOLD;
							checkWallRect.origin.y -= TILE_COLLISION_THRESHOLD;
							checkWallRect.size.width += TILE_COLLISION_THRESHOLD * 2;
							checkWallRect.size.height += TILE_COLLISION_THRESHOLD * 2;
							if (checkWallRect.intersectsRect(wallRect1)) {
								wallHit21 = true;
							}
						}
					}
				}
			}
		}

		// object2 がオブジェクトの場合
		if (object2) {
			auto collision = object2->getObjectCollision();
			//todo 当たりと衝突のhitは分離した方が効率が良い。
			if (wallHit21) {
				collision->addWallObject(object1);
			}
		}
	};

	cocos2d::__Array* ary = cocos2d::Array::create();
	auto objGroup = GameManager::getInstance()->getProjectData()->getObjectGroup();
	for (int group = 0; group < objGroup->count(); ++group) {
		auto wallCollision = CollisionDetaction::create();
		wallCollision->init(this, 3, false, wallCollisionCheckFunc);
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
		wallCollision->name = std::string("groupRoughWallCollisionDetection:") + static_cast<cocos2d::__String*>(objGroup->getObjectAtIndex(group))->getCString();
#else
		wallCollision->name = std::string("groupRoughWallCollisionDetection:") + dynamic_cast<cocos2d::__String*>(objGroup->getObjectAtIndex(group))->getCString();
#endif
		ary->addObject(wallCollision);
	}
	setGroupRoughWallCollisionDetections(ary);
}

CollisionDetaction* SceneLayer::getGroupCollisionDetection(int group)const
{
	if (group == -1) 
	{
		return getCommonCollisionDetection();
	}
	else
	{
		CC_ASSERT(getGroupCollisionDetections());
		CC_ASSERT(0 <= group && group < _groupCollisionDetections->count());
		return dynamic_cast<CollisionDetaction*>(_groupCollisionDetections->getObjectAtIndex(group));
	}
}

CollisionDetaction* SceneLayer::getGroupRoughWallCollisionDetection(int group)const
{
	CC_ASSERT(getGroupRoughWallCollisionDetections());
	CC_ASSERT(0 <= group && group < _groupRoughWallCollisionDetections->count());
	return dynamic_cast<CollisionDetaction*>(_groupRoughWallCollisionDetections->getObjectAtIndex(group));
}

void SceneLayer::createGroupWallCollisionDetections()
{
	cocos2d::__Array* ary = cocos2d::Array::create();
	auto objGroup = GameManager::getInstance()->getProjectData()->getObjectGroup();
	for (int group = 0; group < objGroup->count(); ++group) {
		auto wallCollision = CollisionDetaction::create();
		wallCollision->init(this, 3, false, CC_CALLBACK_2(SceneLayer::callbackDetectionWallCollision, this));
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
		wallCollision->name = std::string("groupWallCollisionDetection:") + static_cast<cocos2d::__String*>(objGroup->getObjectAtIndex(group))->getCString();
#else
		wallCollision->name = std::string("groupWallCollisionDetection:") + dynamic_cast<cocos2d::__String*>(objGroup->getObjectAtIndex(group))->getCString();
#endif
		ary->addObject(wallCollision);
	}
	setGroupWallCollisionDetections(ary);
}

CollisionDetaction* SceneLayer::getGroupWallCollisionDetection(int group)const
{
	CC_ASSERT(getGroupWallCollisionDetections());
	CC_ASSERT(0 <= group && group < _groupWallCollisionDetections->count());
// #AGTK-NX
#if defined(STATIC_DOWN_CAST) && defined(FORCE_STATIC_DOWN_CAST)
	return static_cast<CollisionDetaction*>(_groupWallCollisionDetections->getObjectAtIndex(group));
#else
	return dynamic_cast<CollisionDetaction*>(_groupWallCollisionDetections->getObjectAtIndex(group));
#endif
}

/**
* 物理オブジェクトをシーンに固定する用のノードを生成
* @param	pos	固定用ノードの座標
* @return		固定用ノード
*/
cocos2d::Node *SceneLayer::createStaticPhysicNode(cocos2d::Vec2 pos, bool isConvertPos)
{
	auto sceneData = this->getSceneData();
	auto node = cocos2d::Node::create();

	if (isConvertPos) {
		pos = Scene::getPositionSceneFromCocos2d(pos, sceneData);
	}
	node->setPosition(pos);

	// 物理設定(密度・摩擦係数・反発係数)
	auto material = PHYSICSBODY_MATERIAL_DEFAULT;

	// 矩形の物理ボディ生成
	auto physicsBody = PhysicsBody::createBox(node->getContentSize(), PHYSICSBODY_MATERIAL_DEFAULT);

	// コリジョンイベント用の設定
	physicsBody->setGroup(GameManager::EnumPhysicsGroup::kNone);
	physicsBody->setContactTestBitmask(0);
	physicsBody->setCategoryBitmask(0);
	physicsBody->setCollisionBitmask(0);
	physicsBody->setDynamic(false);
#ifdef USE_PHYSICS_STATIC_MASS_MOMENT
	physicsBody->setMass(PHYSICS_INFINITY);
	physicsBody->setMoment(PHYSICS_INFINITY);
#endif

	// 物理ボディをアタッチ
	node->setPhysicsBody(physicsBody);

	return node;
}

/**
* 指定のリストから物理パーツを取得
* @param	list			検索対象となる物理パーツリストデータ
* @param	scenePartsId	取得したい物理パーツのシーンパーツID
* @return					物理オブジェクトのベースクラス
*/
agtk::PhysicsBase *SceneLayer::getConnectedPhysicsPartsFromList(cocos2d::__Array* list, int scenePartsId)
{
	cocos2d::Ref * ref = nullptr;
	CCARRAY_FOREACH(list, ref) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
		auto physicsBase = static_cast<agtk::PhysicsBase *>(ref);
#else
		auto physicsBase = dynamic_cast<agtk::PhysicsBase *>(ref);
#endif
		if (physicsBase->getScenePartsId() == scenePartsId) {
			return physicsBase;
		}
	}

	return nullptr;

}

/**
* 指定のシーンパーツIDの接続対象となるノードを取得
* @param	scenePartsId		シーンパーツID
* @param	connectSubId		接続先サブID(シーンからオブジェクトに紐付いた物理パーツへの接続ID)
* @param	phyiscsObjectList	物理オブジェクトリスト
* @return						IDが一致するノード
*/
cocos2d::Node *SceneLayer::getConnectedTarget(int scenePartsId, int connectSubId, cocos2d::__Array* phyiscsObjectList, bool *isObject)
{
	// ワーク変数
	cocos2d::Node *target = nullptr;
	bool isTargetFound = false;

	// 「オブジェクトの物理演算設定で作成された物理オブジェクトへの接続でない」場合
	if (connectSubId == agtk::data::PhysicsBaseData::NO_CONNECT_SUB_ID) {
		// オブジェクトを検索
		cocos2d::Ref *ref = nullptr;
		CCARRAY_FOREACH(this->getObjectList(), ref) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
			auto obj = static_cast<agtk::Object *>(ref);
#else
			auto obj = dynamic_cast<agtk::Object *>(ref);
#endif
			// 「シーンパーツIDが一致」
			// オブジェクトに物理用ノードがある
			// 物理用ノードに物理ボディコンポーネントがアタッチされている
			// 以上全てを満たす場合
			if (obj->getScenePartsId() == scenePartsId &&
				obj->getphysicsNode() &&
				obj->getphysicsNode()->getPhysicsBody()) {

				target = obj->getphysicsNode();
				*isObject = true;
				isTargetFound = true;
				break;
			}
		}
	}

	// 接続先対象が見つかっていない場合
	if (!isTargetFound) {

		// 検索するIDを選択
		int targetScenePartsId = connectSubId != agtk::data::PhysicsBaseData::NO_CONNECT_SUB_ID ? connectSubId : scenePartsId;

		// オブジェクトに紐付いた物理オブジェクトに接続する場合
		if (connectSubId != agtk::data::PhysicsBaseData::NO_CONNECT_SUB_ID)
		{
			// 紐付けられているオブジェクトを検索
			cocos2d::Ref *ref = nullptr;
			CCARRAY_FOREACH(this->getObjectList(), ref) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
				auto obj = static_cast<agtk::Object *>(ref);
#else
				auto obj = dynamic_cast<agtk::Object *>(ref);
#endif
				if (obj->getScenePartsId() == scenePartsId) {
					// 紐付けられているオブジェクトが持つ物理パーツリストから指定の物理オブジェクトを取得
					target = getConnectedPhysicsPartsFromList(obj->getPhysicsPartsList(), targetScenePartsId);
					break;
				}
			}
		}
		// シーンに配置された物理オブジェクトに接続する場合
		else {

			// 指定の物理オブジェクトリストがある場合
			if (phyiscsObjectList) {
				// 物理オブジェクトを検索
				target = getConnectedPhysicsPartsFromList(phyiscsObjectList, targetScenePartsId);
			}

			if (!target) {
				// シーンが持つ物理オブジェクトリストを検索
				target = getConnectedPhysicsPartsFromList(this->getPhysicsObjectList(), targetScenePartsId);
			}
		}
	}

	return target;
}

/**
* ロープと接続しているかチェック
* @param	target			チェック対象オブジェクト
* @param	connectId		接続ID
* @param	connectIdx		接続IDX
* @param	anchor			ロープと接続していた場合のロープへのアンカーポイント(出力)
* @param	pos				ロープと接続していた場合のロープの座標(出力)
* @param	physicsObjList	物理オブジェクトリスト
* @param	isRope			ロープを対象としているか？(出力)
* @return					接続先オブジェクト
*/
cocos2d::Node *SceneLayer::checkConnectToRope(cocos2d::Node *target, int connectId, int connectIdx, cocos2d::Vec2 *anchor, cocos2d::Vec2 *pos, cocos2d::__Array *physicsObjList, bool* isRope)
{
	cocos2d::Node* result = target;
	auto rope = dynamic_cast<PhysicsRopeParts *>(target);

	*isRope = (nullptr != rope);

	// ロープを対象としている場合
	if (rope) {

		bool isLastIdx = false;

		// 終点と接続する場合
		if (rope->getPhysicsData()->getPointList()->count() + 1 == connectIdx) {
			isLastIdx = true;
			connectIdx -= 1;
		}

		// 物理オブジェクトリストからロープ部品を検索
		cocos2d::Ref *ref = nullptr;
		CCARRAY_FOREACH(physicsObjList, ref) {
			auto obj = dynamic_cast<PhysicsRopeParts *>(ref);
			if (obj && obj->getScenePartsId() == connectId) {
				if ((obj->getIdx() == connectIdx)) {
					anchor->x = 0;
					anchor->y = obj->getContentSize().height * (isLastIdx ? -0.5f : 0.5f);
					pos->x = obj->getPositionX();
					pos->y = obj->getPositionY();
					result = obj;
					break;
				}
			}
		}
	}

	return result;
}

/**
* ロープの衝突回避チェック
* @param	ropeList	チェックするロープパーツリスト
* @param	ignoreList	回避対象オブジェクトリスト
*/
void SceneLayer::checkIgnoreRopeCollision(cocos2d::__Array *ropeList, cocos2d::__Array *ignoreTargetList)
{
	cocos2d::Ref *ref = nullptr;
	CCARRAY_FOREACH(ropeList, ref) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
		auto connectRope = static_cast<ConnectRope*>(ref);
		auto ropeParts = static_cast<PhysicsRopeParts *>(connectRope->getTarget());
#else
		auto connectRope = dynamic_cast<ConnectRope*>(ref);
		auto ropeParts = dynamic_cast<PhysicsRopeParts *>(connectRope->getTarget());
#endif
		auto ropeBox = ropeParts->getBoundingBox();

		cocos2d::Ref *ref2 = nullptr;
		CCARRAY_FOREACH(ignoreTargetList, ref2) {
			auto physicsBase = dynamic_cast<PhysicsBase *>(ref2);
			cocos2d::Rect box;
			cocos2d::PhysicsBody *body = nullptr;

			// 物理オブジェクトの場合
			if (physicsBase) {
				box = physicsBase->getBoundingBox();
				body = physicsBase->getPhysicsBody();
			}
			// 通常オブジェクトの場合
			else {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
				auto physicsNode = static_cast<cocos2d::Node *>(ref2);
#else
				auto physicsNode = dynamic_cast<cocos2d::Node *>(ref2);
#endif
				box = physicsNode->getBoundingBox();
				body = physicsNode->getPhysicsBody();

				// boxはオブジェクトの座標からの矩形なので左下からの矩形にする
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
				auto obj = static_cast<agtk::Object *>(physicsNode->getParent());
#else
				auto obj = dynamic_cast<agtk::Object *>(physicsNode->getParent());
#endif
				auto basePlayer = obj->getBasePlayer();
				if (basePlayer) {
					auto origin = basePlayer->getOrigin();
					box.origin.x += origin.x;
					box.origin.y += (origin.y - box.size.height);
				}
			}

			// ロープパーツが回避対象オブジェクトにめり込んでいる場合
			if (box.intersectsRect(ropeBox)) {
				// 衝突回避対象に登録する
				ropeParts->setIgnoreBody(body);
			}
		}

		// タイルにめり込んでいる可能性があるのでタイルに対してもチェックする
		auto tileList = this->getCollisionTileList(Point(ropeBox.getMinX(), ropeBox.getMinY()), Point(ropeBox.getMaxX(), ropeBox.getMaxY()));
#ifdef USE_MULTITHREAD_OBJECT_UPDATE
		if(tileList.size() > 0){
			for(auto tile: tileList){
#else
		if (tileList && tileList->count() > 0) {
			cocos2d::Ref *ref = nullptr;
			CCARRAY_FOREACH(tileList, ref) {
				auto tile = dynamic_cast<agtk::Tile *>(ref);
#endif
				auto tilePhysicsBody = tile->getPhysicsBody();
				// タイルに物理ボディが無い場合
				if (tilePhysicsBody == nullptr) {
					// マージ先の物理ボディを取得する
					cocos2d::Ref *ref2 = nullptr;
					CCARRAY_FOREACH(this->getTileMapList(), ref2) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
						auto tileMap = static_cast<agtk::TileMap *>(ref2);
#else
						auto tileMap = dynamic_cast<agtk::TileMap *>(ref2);
#endif
						auto margedTile = tileMap->getTile(tile->getPhysicsMargedTileX(), tile->getPhysicsMargedTileY());
						if (margedTile) {
							tilePhysicsBody = margedTile->getPhysicsBody();
							break;
						}
					}
				}

				// 衝突回避対象に登録する
				ropeParts->setIgnoreBody(tilePhysicsBody);
			}
		}
	}
}

/**
* オブジェクトから座標とアンカーポイントを取得
* @param	object	対象オブジェクト
* @param	pos		座標(出力)
* @param	anchor	アンカーポイント(出力)
*/
void SceneLayer::getObjectAnchorAndPos(agtk::Object *object, Vec2 *pos, Vec2 *anchor)
{
	auto rect = object->getRect();
#ifdef FIX_ACT2_4879
	auto origin = cocos2d::Vec2::ZERO;
	auto basePlayer = object->getBasePlayer();
	if (basePlayer) {
		origin = basePlayer->getOrigin();
	}
#else
	auto origin = object->getBasePlayer()->getOrigin();
#endif
	anchor->x += origin.x;
	anchor->y += origin.y;
}

/**
* アンカーを指定の角度に回転させて返す
* @param	anchor	回転させたいアンカーポイント
* @param	angle	回転角度(DEGREE)
* @return			回転後のアンカーポイント
*/
cocos2d::Vec2 SceneLayer::rotateAnchor(Vec2 anchor, float angle)
{
	return anchor.rotateByAngle(Vec2::ZERO, -CC_DEGREES_TO_RADIANS(angle));
}

/**
* 二つの接続対象をジョイントする
* @param	pre		接続対象
* @param	cur		接続対象
*/
void SceneLayer::jointTwoConnectTarget(ConnectTarget *pre, ConnectTarget *cur)
{
	auto joint1 = PhysicsJointPin::construct(pre->getTarget()->getPhysicsBody(), cur->getTarget()->getPhysicsBody(), pre->getAnchor(), cur->getAnchor());
	joint1->createConstraints();//※set系メソッドを行う前に createConstraints をコールしないとクラッシュするので注意
	joint1->setCollisionEnable(false);
	//joint1->setMaxForce(7500);
	//joint1->setMaxBias(0);
	GameManager::getInstance()->getPhysicsWorld()->addJoint(joint1);

	auto joint2 = PhysicsJointLimit::construct(pre->getTarget()->getPhysicsBody(), cur->getTarget()->getPhysicsBody(), pre->getAnchor(), cur->getAnchor(), 0, 0);
	joint2->createConstraints();//※set系メソッドを行う前に createConstraints をコールしないとクラッシュするので注意
	joint2->setCollisionEnable(false);
	GameManager::getInstance()->getPhysicsWorld()->addJoint(joint2);
}

/**
* オブジェクトと物理パーツの描画優先度を元にオブジェクトの描画用リストへ登録
* @param	object					物理パーツの紐付く対象のオブジェクト
* @param	physicBase				物理パーツ
*/
void SceneLayer::registDrawListForPhysicsPartsWithObjectByDrawPriority(agtk::Object *object, agtk::PhysicsBase *physicBase)
{
	// オブジェクトと物理パーツの描画優先度を取得
	int objDrawPriority = object->getObjectData()->getPriorityInPhysics();
	int physicsDrawPriority = physicBase->getPriority();

#ifdef USE_REDUCE_RENDER_TEXTURE
#else
	// 挿入先を検索するメソッド
	auto getInsertIdx = [](int priority, cocos2d::__Array *list) -> int {
		int maxCount = list->count();
		for (int i = 0; i < maxCount; i++) {
			auto p = dynamic_cast<agtk::PhysicsBase *>(list->getObjectAtIndex(i));
			if (priority < p->getPriority()) {
				return i;
			}
		}
		return maxCount;
	};
#endif

	// オブジェクトの描画優先度より小さい場合
	if (physicsDrawPriority < objDrawPriority) {
#ifdef USE_REDUCE_RENDER_TEXTURE
		if (!object->getBackPhysicsNode()) {
			auto node = cocos2d::Node::create();
			object->addChild(node, Object::kPartPriorityBackPhysics);
			object->setBackPhysicsNode(node);
		}
		if (!physicBase->getParent()) {
			object->getBackPhysicsNode()->addChild(physicBase, physicsDrawPriority);
		}
#else
		// オブジェクトの後面に描画する用リストに物理パーツを挿入
		int insertIdx = getInsertIdx(physicsDrawPriority, object->getDrawBackPhysicsPartsList());
		object->getDrawBackPhysicsPartsList()->insertObject(physicBase, insertIdx);
#endif
	}
	// オブジェクトの描画優先度より大きい場合
	else {
#ifdef USE_REDUCE_RENDER_TEXTURE
		if (!object->getFrontPhysicsNode()) {
			auto node = cocos2d::Node::create();
			object->addChild(node, Object::kPartPriorityFrontPhysics);
			object->setFrontPhysicsNode(node);
		}
		if (!physicBase->getParent()) {
			object->getFrontPhysicsNode()->addChild(physicBase, physicsDrawPriority);
		}
#else
		// オブジェクトの前面に描画する用リストに物理パーツを挿入
		int insertIdx = getInsertIdx(physicsDrawPriority, object->getDrawFrontPhysicsPartsList());
		object->getDrawFrontPhysicsPartsList()->insertObject(physicBase, insertIdx);
#endif
	}

#ifdef USE_REDUCE_RENDER_TEXTURE
#else
	//! 物理パーツはシーンレイヤーのRenderTextureへの書き込み時にVisibleをOn/Offするので非表示化する
	physicBase->setVisible(false);
#endif
}

void SceneLayer::setIsVisible(bool visible)
{
	_isVisible = visible;

	auto particleManager = ParticleManager::getInstance();
	particleManager->setVisible(this->getLayerId(), visible);
}

/**
 * このSceneLayerがフェードアウトして見えないとき真を返す。
 */
bool SceneLayer::isFadeout()
{
	//フェードアウト（非表示状態）
	auto alphaValue = this->getAlphaValue();
	return alphaValue->getValue() == 0.0f ? true : false;
}

bool SceneLayer::isSlideout()
{
	//スライドアウト（画面外にスライドした状態）
	auto positionValue = this->getPositionValue();
	return (positionValue->getValue() != cocos2d::Vec2::ZERO) && (positionValue->getValue() == positionValue->getNextValue() || positionValue->getValue() == positionValue->getPrevValue());
}

void SceneLayer::setPosition(float x, float y)
{
	Node::setPosition(x, y);
}

void SceneLayer::setPosition(cocos2d::Vec2 &pos, float duration)
{
	auto p = this->getPositionValue();
	p->setValue(pos, duration);
}

void SceneLayer::setFade(float alpha, float duration, bool bRemove)
{
	auto alphaValue = this->getAlphaValue();
	alphaValue->setValue(alpha, duration);
	if (duration == 0.0f) {
		alphaValue->update(0.0f);
	}
	this->setRemoveSelfFlag(bRemove);
}

/**
* アクションによるオブジェクトの復活
* @param	objectId	復活対象のオブジェクトID
*/
void SceneLayer::reappearObjectByAction(int objectId)
{
	auto thisSceneId = this->getSceneData()->getId();
	auto thisSceneLayerId = this->getLayerId();

	// 復活対象をリストから探し出し復活させるメソッド
	auto checkAndReAppearFromList = [this, objectId, thisSceneId, thisSceneLayerId](cocos2d::__Array *list) {
		
		for (int i = list->count() - 1; i >= 0; i--)
		{
			auto ref = list->getObjectAtIndex(i);
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
			auto reappearData = static_cast<agtk::data::ObjectReappearData *>(ref);
#else
			auto reappearData = dynamic_cast<agtk::data::ObjectReappearData *>(ref);
#endif

			// 存在していたシーン かつ 存在していたシーンレイヤー かつ 対象のオブジェクトの場合
			if (reappearData->getObjectId() == objectId && reappearData->getSceneId() == thisSceneId && reappearData->getSceneLayerId() == thisSceneLayerId)
			{
				// ACT2-4922 復活時出現条件を満たしていなければUncreateObjectListに追加する。
				auto partData = this->getInitiallyPlacedScenePartObjectDataForReappearData(reappearData);
				if (partData) {
					if (checkObjectAppearCondition(partData)) {
						// オブジェクトを出現させる
						this->appearObject(partData);
					}
					else {
						this->getUncreateObjectList()->addObject(partData);
					}
				}
				else {
					// 復活
					reappearData->setReappearFlag(true);
					this->reappearObject(reappearData);
				}

				// リストから削除
				list->removeObjectAtIndex(i);
			}
		}
	};

	// 「カメラ外に出たら消滅」するオブジェクトから対象オブジェクトを検索
	checkAndReAppearFromList(this->getDeleteObjectList());

	// 「シーンが切り替わったら消滅」するオブジェクトから対象オブジェクトを検索
	checkAndReAppearFromList(GameManager::getInstance()->getSceneChangeReappearObjectList());

	// 「アクションで復活」のオブジェクトから対象オブジェクトを検索
	checkAndReAppearFromList(GameManager::getInstance()->getCommandReappearObjectList());
}

/**
* アクションによるオブジェクトの有効化
* @param	objectId	有効化対象のオブジェクトID
*/
void SceneLayer::enableObjectByAction(int objectId)
{
	cocos2d::Ref *ref = nullptr;
	CCARRAY_FOREACH(this->getObjectList(), ref) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
		auto object = static_cast<agtk::Object *>(ref);
#else
		auto object = dynamic_cast<agtk::Object *>(ref);
#endif

		if (objectId == object->getObjectData()->getId()) {
			//有効化
			object->setDisabled(false);
			//表示する
			object->setVisible(true);
		}
	}
}

void SceneLayer::createReappearObject()
{
	bool bRemove;
	do {
		bRemove = false;
		cocos2d::Ref *ref;
		auto deleteObjList = this->getDeleteObjectList();
		CCARRAY_FOREACH(deleteObjList, ref) {
			auto reappearData = dynamic_cast<agtk::data::ObjectReappearData *>(ref);
			if (reappearData) {
				// 消滅したオブジェクト出現チェック
				if (checkObjectReappearCondition(reappearData)) {
					// ACT2-4922 復活時出現条件を満たしていなければUncreateObjectListに追加する。
					auto partData = this->getInitiallyPlacedScenePartObjectDataForReappearData(reappearData);
					if (partData) {
						if (checkObjectAppearCondition(partData)) {
							// オブジェクトを出現させる
							this->appearObject(partData);
						}
						else {
							this->getUncreateObjectList()->addObject(partData);
						}
					}
					else {
						// オブジェクトを出現させる
						this->reappearObject(reappearData);
					}
					deleteObjList->removeObject(reappearData);
					bRemove = true;
					break;
				}
			}
		}
	} while (bRemove);
}

void SceneLayer::callbackDetectionWallCollision(CollisionNode* collisionObject1, CollisionNode* collisionObject2)
{
#ifdef USE_COLLISION_MEASURE
	roughWallCollisionCount++;
#endif

#ifdef USE_MULTITHREAD_OBJECT_UPDATE
	// updateThreadWallCollision()で保存したobjectとFuncの取り出し
	Object *wallCollisionObject;
	const DetectWallCollisionFunction* detectWallCollisionFunc;
	bool mainThread = true;
	auto threadManager = ThreadManager::getInstance();
	for (int i = 0; i < AGTK_THREAD_COUNT - 1; i++) {
		auto threadInfo = threadManager->getThreadInfo(i);
		if (std::this_thread::get_id() == threadInfo->thread->get_id()) {
			// サブスレッドから呼ばれた場合
			mainThread = false;
			auto subInfo = (ObjectUpdateThreadInfo *)threadInfo->subInfo;
			wallCollisionObject = subInfo->wallCollisionObject;
			detectWallCollisionFunc = subInfo->detectWallCollisionFunc;
			break;
		}
	}
	if (mainThread) {
		// メインスレッドから呼ばれた場合
		wallCollisionObject = getWallCollisionObject();
		detectWallCollisionFunc = getDetectWallCollisionFunc();
	}

	if (dynamic_cast<Object *>(collisionObject1->getNode()) == wallCollisionObject) {
		(*detectWallCollisionFunc)(collisionObject2);
	}
	else if (dynamic_cast<Object *>(collisionObject2->getNode()) == wallCollisionObject) {
		(*detectWallCollisionFunc)(collisionObject1);
	}
	else {
		CC_ASSERT(0);
	}
#else
	if (dynamic_cast<Object *>(collisionObject1->getNode()) == getWallCollisionObject()) {
		(*getDetectWallCollisionFunc())(collisionObject2);
	}
	else if (dynamic_cast<Object *>(collisionObject2->getNode()) == getWallCollisionObject()) {
		(*getDetectWallCollisionFunc())(collisionObject1);
	}
	else {
		CC_ASSERT(0);
	}
#endif
}

void SceneLayer::updateWallCollision(Object *wallCollisionObject, const DetectWallCollisionFunction& func)
{
	//CollisionComponent * component = dynamic_cast<CollisionComponent *>(wallCollisionObject->getComponent(componentName));
	CollisionComponent * component = dynamic_cast<CollisionComponent *>(wallCollisionObject->getComponent(CollisionComponent::getCollisionComponentName(CollisionComponent::kGroupWall)));
	if (component != nullptr) {
		CollisionNode * collisionObject = component->getCollisionNode();
		if (collisionObject != nullptr) {
			auto objectData = wallCollisionObject->getObjectData();
			if (!objectData->getCollideWithObjectGroupBit()) {
				return;
			}
			CollisionDetaction *wallCollisionDetection = getGroupWallCollisionDetection(objectData->getGroup());
			if (wallCollisionDetection) {
				wallCollisionDetection->updateSingleWithoutScan(collisionObject);
			}
			wallCollisionDetection = nullptr;
			setWallCollisionObject(wallCollisionObject);
			setDetectWallCollisionFunc(&func);
			for (int group = 0; group < getGroupWallCollisionDetections()->count(); ++group) {
				wallCollisionDetection = nullptr;
				if (objectData->isCollideWithObjectGroup(group)) {
					wallCollisionDetection = getGroupWallCollisionDetection(group);
				}
				if (!wallCollisionDetection) {
					continue;
				}
				wallCollisionDetection->scanSingle(collisionObject);
			}
			setWallCollisionObject(nullptr);
			setDetectWallCollisionFunc(nullptr);
		}
	}
}

#ifdef USE_MULTITHREAD_OBJECT_UPDATE
/* 
マルチスレッド用　updateWallCollision
サブスレッドから呼ばれた場合は_objectUpdateThreadInfoList[]に引数を保存
*/
void SceneLayer::updateThreadWallCollision(Object *wallCollisionObject, const DetectWallCollisionFunction& func)
{
	//CollisionComponent * component = dynamic_cast<CollisionComponent *>(wallCollisionObject->getComponent(componentName));
	CollisionComponent * component = dynamic_cast<CollisionComponent *>(wallCollisionObject->getComponent(CollisionComponent::getCollisionComponentName(CollisionComponent::kGroupWall)));
	if (component != nullptr) {
		CollisionNode * collisionObject = component->getCollisionNode();
		if (collisionObject != nullptr) {
			auto objectData = wallCollisionObject->getObjectData();
			if (!objectData->getCollideWithObjectGroupBit()) {
				return;
			}

			CollisionDetaction *wallCollisionDetection = getGroupWallCollisionDetection(objectData->getGroup());
			if (wallCollisionDetection) {
				wallCollisionDetection->updateSingleWithoutScan(collisionObject);
			}
			wallCollisionDetection = nullptr;

			// 引数の保存先
			int i;
			bool mainThread = true;
			auto threadManager = ThreadManager::getInstance();
			ThreadManager::ThreadInfo *threadInfo = nullptr;
			ObjectUpdateThreadInfo *subInfo = nullptr;
			for (i = 0; i < AGTK_THREAD_COUNT - 1; i++) {
				threadInfo = threadManager->getThreadInfo(i);
				if (std::this_thread::get_id() == threadInfo->thread->get_id()) {
					// サブスレッドから呼ばれた場合
					mainThread = false;
					subInfo = (ObjectUpdateThreadInfo *)threadInfo->subInfo;
					subInfo->wallCollisionObject = wallCollisionObject;
					subInfo->detectWallCollisionFunc = &func;
					break;
				}
			}
			if (mainThread) {
				// メインスレッドから呼ばれた場合
				setWallCollisionObject(wallCollisionObject);
				setDetectWallCollisionFunc(&func);
			}

			for (int group = 0; group < getGroupWallCollisionDetections()->count(); ++group) {
				wallCollisionDetection = nullptr;
				if (objectData->isCollideWithObjectGroup(group)) {
					wallCollisionDetection = getGroupWallCollisionDetection(group);
				}
				if (!wallCollisionDetection) {
					continue;
				}
				wallCollisionDetection->scanSingle(collisionObject);
			}

			// 終了前に初期化
			if (mainThread) {
				setWallCollisionObject(nullptr);
				setDetectWallCollisionFunc(nullptr);
			}
			else {
				subInfo->wallCollisionObject = nullptr;
				subInfo->detectWallCollisionFunc = nullptr;
			}

		}
	}
}
#endif

/**
* @brief 再帰的にsetCascadeOpacityEnabledを呼ぶ関数
*/
void SceneLayer::updateCascadeOpacityEnabled(Node *parent, bool enabled)
{
	parent->setCascadeOpacityEnabled(enabled);
	for (int i = 0; i < parent->getChildrenCount(); ++i) {
		updateCascadeOpacityEnabled(parent->getChildren().at(i), enabled);
	}
}

/**
* @brief 子Nodeの数を再帰的に処理し取得する
* @param	node	子Nodeを取得したいNode
*/
int SceneLayer::getCountChildrenRecursively(cocos2d::Node* node)
{
	int count = 0;

	node->enumerateChildren("//.*", [&count](cocos2d::Node*) -> bool {
		count++;
		return false;
	});
	return count;
}

/**
* @brief 子孫の数が指定した数を超えているかを判定する
* @param	node	子孫数を取得するNode
* @param	count	指定数
*/
bool SceneLayer::isChildrenCountOver(cocos2d::Node *node, int &count)
{
	count -= node->getChildrenCount();
	if (count < 0) {
		return true;
	}
	for (auto &child : node->getChildren()) {
		if (isChildrenCountOver(child, count)) {
			return true;
		}
	}
	return false;
}

/**
 * @brief このSceneLayerがゲーム画面内に一部でも表示されていて、AlphaValueが0でないとき、真を返す。
 * @todo メニューシーンサイズの計算をしているが、メニューシーンサイズは画面サイズと一致するため、処理を簡略化できる。
 */
bool SceneLayer::isDisplay()
{
	auto projectData = GameManager::getInstance()->getProjectData();
	auto menuSceneData = projectData->getMenuSceneData();
	auto sceneSize = projectData->getSceneSize(menuSceneData);
	auto screenRect = cocos2d::Rect(0, 0, sceneSize.width - 1, sceneSize.height - 1);

	auto pos = this->getPosition();
	auto size = this->getContentSize();
	if (screenRect.intersectsRect(cocos2d::Rect(pos.x, pos.y, size.width - 1, size.height - 1)) == false) {
		return false;
	}
	if (this->getAlphaValue()->getValue() == 0) {
		return false;
	}
	return true;
}


//-------------------------------------------------------------------------------------------------------------------
Scene::Scene()
{
	_layer = nullptr;
	_sceneBackground = nullptr;
	_sceneLayerList = nullptr;
	_menuLayerList = nullptr;
	_sceneTopMost = nullptr;
	_renderTextureCtrlList = nullptr;
	_camera = nullptr;
	_gravity = nullptr;
	_water = nullptr;
	_gameSpeed = nullptr;
	_shake = nullptr;
	_sceneData = nullptr;
	_variableTimerList = nullptr;
	_cameraObjectList = nullptr;
	_courseList = nullptr;

	_currentCameraObject = nullptr;

	_waitDuration300 = 0;
	_objectInstanceId = 0;
	_objectInstanceMenuId = 0;
#ifdef USE_PREVIEW
	_previewObjectId = -1;
	_previewInstanceId = -1;
	_watchPhysicsPartIdList = nullptr;
	_physicsRequestd = false;
#endif
	_createType = kCreateTypeNone;
	_viewportLight = nullptr;
#ifdef USE_PREVIEW
	_debugLimitAreaSprite = nullptr;
	_debugLimitCameraSprite = nullptr;
#endif
	_ignoredUpdateActionFlag = false;

	_requestSwitchInit = false;//変数・スイッチ初期化要求スイッチ
	_requestSwitchReset = false;//リセット要求スイッチ
	_requestSwitchSaveFile = false;//セーブ要求スイッチ
	_requestSwitchDeleteFile = false;//削除要求スイッチ
	_requestSwitchCopyFile = false;//コピー要求スイッチ
	_requestSwitchLoadFile = false;//ロード要求スイッチ

	_cameraUpdated = false;
	_sceneCreateSkipFrameFlag = false;
// #AGTK-NX
#ifdef USE_SAR_OPTIMIZE_1
	_sceneObjectListCache = nullptr;
	_menuObjectListCache = nullptr;
	_allObjectListCache = nullptr;
#endif

}

Scene::~Scene()
{
	CC_SAFE_RELEASE_NULL(_layer);
	CC_SAFE_RELEASE_NULL(_sceneBackground);
	CC_SAFE_RELEASE_NULL(_sceneLayerList);
	CC_SAFE_RELEASE_NULL(_menuLayerList);
	CC_SAFE_RELEASE_NULL(_sceneTopMost);
	CC_SAFE_RELEASE_NULL(_renderTextureCtrlList);
	CC_SAFE_RELEASE_NULL(_camera);
	CC_SAFE_RELEASE_NULL(_gravity);
	CC_SAFE_RELEASE_NULL(_water);
	CC_SAFE_RELEASE_NULL(_gameSpeed);
	CC_SAFE_RELEASE_NULL(_shake);
	CC_SAFE_RELEASE_NULL(_sceneData);
	CC_SAFE_RELEASE_NULL(_variableTimerList);
	CC_SAFE_RELEASE_NULL(_cameraObjectList);
	CC_SAFE_RELEASE_NULL(_courseList);

	CC_SAFE_RELEASE_NULL(_currentCameraObject);
#ifdef USE_PREVIEW
	CC_SAFE_RELEASE_NULL(_watchPhysicsPartIdList);
#endif
	CC_SAFE_RELEASE_NULL(_viewportLight);
#ifdef USE_PREVIEW
	if (_debugLimitAreaSprite) {
		_debugLimitAreaSprite->removeFromParentAndCleanup(true);
		CC_SAFE_RELEASE_NULL(_debugLimitAreaSprite);
	}
	if (_debugLimitCameraSprite) {
		_debugLimitCameraSprite->removeFromParentAndCleanup(true);
		CC_SAFE_RELEASE_NULL(_debugLimitCameraSprite);
	}
#endif
// #AGTK-NX
#ifdef USE_SAR_OPTIMIZE_1
	CC_SAFE_RELEASE_NULL(_sceneObjectListCache);
	CC_SAFE_RELEASE_NULL(_menuObjectListCache);
	CC_SAFE_RELEASE_NULL(_allObjectListCache);
#endif
}

Scene *Scene::create(agtk::data::SceneData *sceneData, int startPointGroupIdx, EnumCreateType type)
{
	auto p = new (std::nothrow) Scene();
	if (p && p->init(sceneData, startPointGroupIdx, type)) {
		p->autorelease();
		return p;
	}
	CC_SAFE_DELETE(p);
	return nullptr;
}

bool Scene::init(agtk::data::SceneData *sceneData, int startPointGroupIdx, EnumCreateType type)
{
	AGTK_DEBUG_ACTION_LOG("%d,%s", __LINE__, __FUNCTION__);
	if (!cocos2d::Node::init()) {
		return false;
	}
	if (sceneData == nullptr) {
		return false;
	}
	this->setSceneData(sceneData);
	this->setCreateType(type);

	auto gameManager = GameManager::getInstance();
	auto sceneLayerList = cocos2d::__Dictionary::create();
	auto renderTextureCtrlList = cocos2d::__Dictionary::create();

	// physic
	gameManager->setAirResistance(sceneData->getAirResistance());

	//scene size
	auto projectData = GameManager::getInstance()->getProjectData();
	int tileWidth = projectData->getTileWidth();
	int tileHeight = projectData->getTileHeight();
	auto sceneSize = projectData->getSceneSize(sceneData);
	//auto horzScreenCount = sceneData->getHorzScreenCount();
	//auto vertScreenCount = sceneData->getVertScreenCount();
	//auto screenWidth = projectData->getScreenWidth();
	//auto screenHeight = projectData->getScreenHeight();
	//cocos2d::Vec2(screenWidth * horzScreenCount, screenHeight * vertScreenCount));
	this->setSceneSize(sceneSize);
	this->setContentSize(Size(this->getSceneSize()));

	int sceneHorzTileCount = 0;
	int sceneVertTileCount = 0;
	//sceneHorzTileCount = ceil((float)screenWidth * horzScreenCount / tileWidth);
	//sceneVertTileCount = ceil((float)screenHeight * vertScreenCount / tileHeight);
	sceneHorzTileCount = ceil(sceneSize.width / tileWidth);
	sceneVertTileCount = ceil(sceneSize.height / tileHeight);

	//scene topmost
	// ベースシーンになるため、this->addChild()は一番最初にしておく
	auto topMost = agtk::SceneTopMost::create(this);
	this->setSceneTopMost(topMost);
	this->addChild(topMost);

	// ACT2-4990 オブジェクト作成時にコースポイント座標取得するように変更した際に
	// この時点ではパーツ情報が存在しないのでここの処理を移動
	auto cameraObjectList = cocos2d::__Array::create();
	auto courseList = cocos2d::__Array::create();
	std::function<void(cocos2d::__Dictionary*, cocos2d::__Array*, cocos2d::__Array*)> func = [&](cocos2d::__Dictionary *partList, cocos2d::__Array* cameraObjectList, cocos2d::__Array* courseList) {
		cocos2d::DictElement *el = nullptr;
		CCDICT_FOREACH(partList, el)
		{
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
			auto p = static_cast<agtk::data::ScenePartOthersData *>(el->getObject());
#else
			auto p = dynamic_cast<agtk::data::ScenePartOthersData *>(el->getObject());
#endif
			switch (p->getOthersType())
			{
				// カメラ
			case agtk::data::ScenePartOthersData::kOthersCamera:
			{
				if (cameraObjectList == nullptr) {
					break;
				}

				auto cameraObj = CameraObject::create(
					p->getId(),
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
					static_cast<agtk::data::OthersCameraData*>(p->getPart())
#else
					dynamic_cast<agtk::data::OthersCameraData*>(p->getPart())
#endif
					);
				cameraObjectList->addObject(cameraObj);
				this->addChild(cameraObj);
			} break;

			// コース（直線）
			case agtk::data::ScenePartOthersData::kOthersLineCourse:
			{
				if (courseList == nullptr) {
					break;
				}

				auto course = OthersLineCourse::create(
					p->getId(),
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
					static_cast<agtk::data::OthersLineCourseData*>(p->getPart()),
#else
					dynamic_cast<agtk::data::OthersLineCourseData*>(p->getPart()),
#endif
					sceneData
					);
				courseList->addObject(course);
				this->addChild(course);
			} break;

			// コース（曲線）
			case agtk::data::ScenePartOthersData::kOthersCurveCourse:
			{
				if (courseList == nullptr) {
					break;
				}

				auto course = OthersCurveCourse::create(
					p->getId(),
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
					static_cast<agtk::data::OthersCurveCourseData*>(p->getPart()),
#else
					dynamic_cast<agtk::data::OthersCurveCourseData*>(p->getPart()),
#endif
					sceneData
					);
				courseList->addObject(course);
				this->addChild(course);
			} break;

			// コース（円）
			case agtk::data::ScenePartOthersData::kOthersCircleCourse:
			{
				if (courseList == nullptr) {
					break;
				}

				auto course = OthersCircleCourse::create(
					p->getId(),
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
					static_cast<agtk::data::OthersCircleCourseData*>(p->getPart()),
#else
					dynamic_cast<agtk::data::OthersCircleCourseData*>(p->getPart()),
#endif
					sceneData
					);
				courseList->addObject(course);
				this->addChild(course);
			} break;
			}
		}
	};

	auto partList = sceneData->getScenePartOthersList();
	func(partList, cameraObjectList, courseList);

	this->setCameraObjectList(cameraObjectList);
	this->setCourseList(courseList);

	//scene background
	auto bg = agtk::SceneBackground::create(this);
	this->setSceneBackground(bg);
	topMost->addChild(bg);

	//scene layer
	int startZOrder = BaseLayer::ZOrder::Scene + BaseLayer::ADD_ZORDER;

	auto layerList = sceneData->getLayerList();
	for (int layerId = (int)layerList->count(); layerId >= 1; layerId--) {
		auto layerData = sceneData->getLayer(layerId);
		if (layerData == nullptr) continue;
		CC_ASSERT(layerData);
		CC_ASSERT(layerData->getLayerId() == layerId);
		auto sceneLayer = agtk::SceneLayer::create(this, sceneData, layerData->getLayerId(), (this->getCreateType() == kCreateTypePortal), false, startPointGroupIdx);
#ifdef USE_AGTK
		gameManager->getPhysicsWorld()->setDebugLayer(sceneLayer);
#endif
		topMost->addChild(sceneLayer, startZOrder + BaseLayer::ADD_ZORDER * ((int)layerList->count() - layerId));
		sceneLayerList->setObject(sceneLayer, layerData->getLayerId());
		auto rtCtrl = sceneLayer->getRenderTexture();
		if (rtCtrl) {
			renderTextureCtrlList->setObject(rtCtrl, layerId);
		}
	}

	this->setSceneLayerList(sceneLayerList);
	this->setRenderTextureCtrlList(renderTextureCtrlList);

	//viewport & light
	auto viewportLight = ViewportLight::create(this);
	this->setViewportLight(viewportLight);
	//camera
	auto camera = Camera::create(this);
	camera->setLayerList(this->getSceneLayerList());
	this->setCamera(camera);

	//gravity
	auto gravity = SceneGravity::create(sceneData);
	this->setGravity(gravity);
	//water
	auto water = SceneWater::create(sceneData);
	this->setWater(water);
	this->setSceneData(sceneData);
	//gameSpeed
	auto gameSpeed = SceneGameSpeed::create(sceneData);
	this->setGameSpeed(gameSpeed);
	//shake
	auto shake = SceneShake::create(sceneData);
	this->setShake(shake);
	//preload
	GameManager::getInstance()->preloadSceneData(sceneData);

	// wait 
	setWaitDuration300(0);

	//variable timer list
	auto variableTimerList = cocos2d::__Array::create();
	this->setVariableTimerList(variableTimerList);

	// ACT2-4990 オブジェクト作成時にコースポイント座標取得するように変更した際に
	// この時点ではパーツ情報が存在しないのでここの処理を上に移動
#if 0
	// camera, course
	auto cameraObjectList = cocos2d::__Array::create();
	auto courseList = cocos2d::__Array::create();
	std::function<void(cocos2d::__Dictionary*, cocos2d::__Array*, cocos2d::__Array*)> func = [&](cocos2d::__Dictionary *partList, cocos2d::__Array* cameraObjectList, cocos2d::__Array* courseList) {
		cocos2d::DictElement *el = nullptr;
		CCDICT_FOREACH(partList, el)
		{
			auto p = dynamic_cast<agtk::data::ScenePartOthersData *>(el->getObject());
			switch (p->getOthersType())
			{
				// カメラ
			case agtk::data::ScenePartOthersData::kOthersCamera:
			{
				if (cameraObjectList == nullptr) {
					break;
				}

				auto cameraObj = CameraObject::create(
					p->getId(),
					dynamic_cast<agtk::data::OthersCameraData*>(p->getPart())
				);
				cameraObjectList->addObject(cameraObj);
				this->addChild(cameraObj);
				} break;

			// コース（直線）
			case agtk::data::ScenePartOthersData::kOthersLineCourse:
			{
				if (courseList == nullptr) {
					break;
				}

				auto course = OthersLineCourse::create(
					p->getId(),
					dynamic_cast<agtk::data::OthersLineCourseData*>(p->getPart()),
					sceneData
				);
				courseList->addObject(course);
				this->addChild(course);
				} break;

			// コース（曲線）
			case agtk::data::ScenePartOthersData::kOthersCurveCourse:
			{
				if (courseList == nullptr) {
					break;
				}

				auto course = OthersCurveCourse::create(
					p->getId(),
					dynamic_cast<agtk::data::OthersCurveCourseData*>(p->getPart()),
					sceneData
				);
				courseList->addObject(course);
				this->addChild(course);
				} break;

			// コース（円）
			case agtk::data::ScenePartOthersData::kOthersCircleCourse:
			{
				if (courseList == nullptr) {
					break;
				}

				auto course = OthersCircleCourse::create(
					p->getId(),
					dynamic_cast<agtk::data::OthersCircleCourseData*>(p->getPart()),
					sceneData
				);
				courseList->addObject(course);
				this->addChild(course);
				} break;
			}
		}
	};

	auto partList = sceneData->getScenePartOthersList();
	func(partList, cameraObjectList, courseList);

	this->setCameraObjectList(cameraObjectList);
	this->setCourseList(courseList);
#endif

	auto menuLayerList = cocos2d::__Dictionary::create();
	int menuLayerId = sceneData->getInitialMenuLayerId();
	if (menuLayerId > 0) {
		auto menuData = projectData->getMenuSceneData();
		auto menuLayerData = menuData->getLayer(sceneData->getInitialMenuLayerId());
		if (menuLayerData != nullptr) {
			CC_ASSERT(menuLayerData->getLayerId() == menuLayerId);
			// メニュー用course
			partList = menuData->getScenePartOthersList();
			func(partList, nullptr, courseList);

			auto menuLayer = agtk::SceneLayer::createMenu(this, menuData, menuLayerData->getLayerId(), projectData->getSceneSize(menuData), false);
			menuLayer->setType(agtk::SceneLayer::kTypeMenu);
			topMost->addChild(menuLayer, BaseLayer::ZOrder::Menu);
			menuLayerList->setObject(menuLayer, menuLayerData->getLayerId());
		}
	}
	{
		menuLayerId = agtk::data::SceneData::kHudMenuLayerId;
		auto menuData = projectData->getMenuSceneData();
		auto menuLayerData = menuData->getLayer(menuLayerId);
		if (menuLayerData != nullptr) {
			CC_ASSERT(menuLayerData->getLayerId() == menuLayerId);
			auto menuLayer = agtk::SceneLayer::createMenu(this, menuData, menuLayerData->getLayerId(), projectData->getSceneSize(menuData), false);
			menuLayer->setType(agtk::SceneLayer::kTypeMenu);
			topMost->addChild(menuLayer, BaseLayer::ZOrder::HudMenu);
			menuLayerList->setObject(menuLayer, menuLayerData->getLayerId());
		}
	}
	{
		menuLayerId = agtk::data::SceneData::kHudTopMostLayerId;
		auto menuData = projectData->getMenuSceneData();
		auto menuLayerData = menuData->getLayer(menuLayerId);
		if (menuLayerData != nullptr) {
			CC_ASSERT(menuLayerData->getLayerId() == menuLayerId);
			auto menuLayer = agtk::SceneLayer::createMenu(this, menuData, menuLayerData->getLayerId(), projectData->getSceneSize(menuData), false);
			menuLayer->setType(agtk::SceneLayer::kTypeMenu);
			topMost->addChild(menuLayer, BaseLayer::ZOrder::HudTopMost);
			menuLayerList->setObject(menuLayer, menuLayerData->getLayerId());
		}
	}
	this->setMenuLayerList(menuLayerList);

	//単体インスタンスIDのクリア
	auto projectPlayData = GameManager::getInstance()->getPlayData();
	{
		auto objectList = projectPlayData->getObjectList();
		cocos2d::DictElement *el = nullptr;
		CCDICT_FOREACH(objectList, el) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
			auto playObjectData = static_cast<agtk::data::PlayObjectData *>(el->getObject());
#else
			auto playObjectData = dynamic_cast<agtk::data::PlayObjectData *>(el->getObject());
#endif
			auto objectData = playObjectData->getObjectData();
			if (objectData) {
				auto variableData = projectPlayData->getVariableData(objectData->getId(), agtk::data::kObjectSystemVariableSingleInstanceID);
				variableData->setValue(0);
			}
		}
	}
	//単体インスタンスIDを設定。
	{
		auto objectSingleIdHash = sceneData->getObjectSingleIdHash();
		cocos2d::DictElement *el = nullptr;
		CCDICT_FOREACH(objectSingleIdHash, el) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
			auto p = static_cast<cocos2d::__Integer *>(el->getObject());
#else
			auto p = dynamic_cast<cocos2d::__Integer *>(el->getObject());
#endif
			auto variableData = projectPlayData->getVariableData(el->getIntKey(), agtk::data::kObjectSystemVariableSingleInstanceID);
			variableData->setValue((double)p->getValue());
		}
	}
	//singleIdHashで未設定のオブジェクト
	{
		auto playObjectList = projectPlayData->getObjectList();
		cocos2d::DictElement *el = nullptr;
		CCDICT_FOREACH(playObjectList, el) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
			auto playObjectData = static_cast<agtk::data::PlayObjectData *>(el->getObject());
#else
			auto playObjectData = dynamic_cast<agtk::data::PlayObjectData *>(el->getObject());
#endif
			//単体として指定されている同オブジェクトのインスタンスのインスタンスIDが格納。
			auto variableData = playObjectData->getVariableData(agtk::data::kObjectSystemVariableSingleInstanceID);
			auto objectList = this->getObjectAll(playObjectData->getId());
			if (objectList->count() > 0 && variableData->getValue() == 0.0f) {
				agtk::Object *object = nullptr;
				cocos2d::Ref *ref = nullptr;
				CCARRAY_FOREACH(objectList, ref) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
					auto p = static_cast<agtk::Object *>(ref);
#else
					auto p = dynamic_cast<agtk::Object *>(ref);
#endif
					if (object != nullptr) {
						if (p->getInstanceId() < object->getInstanceId()) {
							object = p;
						}
					}
					else {
						object = p;
					}
				}
				if (object) {
					variableData->setValue((double)object->getInstanceId());
				}
			}
			//シーンに配置された同インスタンスの数が格納。
			playObjectData->setInstanceCount((double)objectList->count());
		}
	}
	//シーンに配置された同インスタンス数を設定。
	{
// #AGTK-NX
#ifdef USE_SAR_OPTIMIZE_1
		auto objectList = this->getObjectAllReference(SceneLayer::kTypeScene);
#else
		auto objectList = this->getObjectAll(SceneLayer::kTypeScene);
#endif
		cocos2d::Ref *ref;
		CCARRAY_FOREACH(objectList, ref) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
			auto object = static_cast<agtk::Object *>(ref);
#else
			auto object = dynamic_cast<agtk::Object *>(ref);
#endif
			auto playObjectData = object->getPlayObjectData();

			//auto instanceCountVariableData = projectPlayData->getVariableData(object->getObjectData()->getId(), agtk::data::kObjectSystemVariableInstanceCount);
			auto count = this->getObjectInstanceCount(object->getObjectData()->getId());
			playObjectData->setInstanceCount(count);
			//auto variableData = playObjectData->getVariableData(agtk::data::kObjectSystemVariableInstanceCount);
			//variableData->setValue(instanceCountVariableData->getValue());
		}
	}

#if 1
	//シーンデータとすべてのレイヤーデータに配置されているオブジェクトデータのインスタンスIDの最大値を計算する。
	unsigned int maxInstanceId = 0;
	std::function<void(agtk::data::SceneData *)> updateMaxInstanceId = [&maxInstanceId](agtk::data::SceneData *scData) {
		auto layerList = scData->getLayerList();
		cocos2d::DictElement *el = nullptr;
		CCDICT_FOREACH(layerList, el) {
			auto layerData = dynamic_cast<agtk::data::LayerData *>(el->getObject());
			if (layerData == nullptr) continue;
			auto layerId = layerData->getLayerId();
			auto scenePartList = scData->getScenePartObjectList(layerId - 1);
			cocos2d::Ref *ref = nullptr;
			int scenePartObjectDataIdx = 0;
			CCARRAY_FOREACH(scenePartList, ref) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
				auto scenePartData = static_cast<agtk::data::ScenePartData *>(ref);
#else
				auto scenePartData = dynamic_cast<agtk::data::ScenePartData *>(ref);
#endif
				auto instanceId = scenePartData->getId();
				if (instanceId > maxInstanceId) {
					maxInstanceId = instanceId;
				}
			}
		}
	};
	updateMaxInstanceId(sceneData);
	updateMaxInstanceId(projectData->getMenuSceneData());

	//インスタンスの最大値を取得。
	_objectInstanceId = maxInstanceId;
	_objectInstanceMenuId = maxInstanceId + agtk::data::SceneData::kMenuSceneId;
#else
	//シーンに配置後のインスタンスIDの最大値計算する。
	std::function<int(bool)> getMaxInstanceId = [&](bool bMenuScene) {
		int maxId = 0;
		cocos2d::Ref *ref;
		auto objectList = this->getObjectAll(agtk::SceneLayer::kTypeMax);
		CCARRAY_FOREACH(objectList, ref) {
			auto p = dynamic_cast<agtk::Object *>(ref);
			if (p->getSceneData()->isMenuScene() == bMenuScene) {
				int instanceId = p->getInstanceId();
				if (instanceId > maxId) maxId = instanceId;
			}
		}
		return maxId;
	};

	//インスタンスの最大値を取得。
	_objectInstanceId = getMaxInstanceId(false);
	_objectInstanceMenuId = getMaxInstanceId(true);
#endif

	// 初期カメラに切り替える
	changeCamera(getInitialCameraObject());

	// シーン経過時間
	setElapsedTime(0);

	//出現オブジェクトを生成する。
	this->createReappearObject();

	AGTK_DEBUG_ACTION_LOG("%d,%s", __LINE__, __FUNCTION__);
	return true;
}

agtk::Object *Scene::getObjectPlayer()
{
	cocos2d::DictElement *el = nullptr;
	auto sceneLayerList = this->getSceneLayerList();
	CCDICT_FOREACH(sceneLayerList, el) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
		auto sceneLayer = static_cast<agtk::SceneLayer *>(el->getObject());
#else
		auto sceneLayer = dynamic_cast<agtk::SceneLayer *>(el->getObject());
#endif
		cocos2d::Ref *ref = nullptr;
		CCARRAY_FOREACH(sceneLayer->getObjectList(), ref) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
			auto object = static_cast<agtk::Object *>(ref);
#else
			auto object = dynamic_cast<agtk::Object *>(ref);
#endif
			auto objectData = object->getObjectData();
			// 「プレイヤーグループ」かつ「入力デバイスで操作する」の両方がチェックされている場合
			if (objectData->isGroupPlayer() && objectData->getOperatable()) {
				return object;
			}
		}
	}
	return nullptr;
}

agtk::Object *Scene::getObjectLocked(int objectInstanceId)
{
	//指定オブジェクトIDでロックされているオブジェクトを取得する
	cocos2d::DictElement *el = nullptr;
	auto sceneLayerList = this->getSceneLayerList();
	CCDICT_FOREACH(sceneLayerList, el) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
		auto sceneLayer = static_cast<agtk::SceneLayer *>(el->getObject());
#else
		auto sceneLayer = dynamic_cast<agtk::SceneLayer *>(el->getObject());
#endif
		cocos2d::Ref *ref = nullptr;
		CCARRAY_FOREACH(sceneLayer->getObjectList(), ref) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
			auto object = static_cast<agtk::Object *>(ref);
#else
			auto object = dynamic_cast<agtk::Object *>(ref);
#endif
			if (object->getPlayObjectData()->isLocked(objectInstanceId)) {
				cocos2d::log("ロックドオブジェクト: %s", object->getObjectData()->getName());
				return object;
			}
		}
	}
	return nullptr;
}

agtk::CameraObject *Scene::getInitialCameraObject()
{
	agtk::CameraObject *initialCameraObj = nullptr;

	cocos2d::Ref *ref = nullptr;
	CCARRAY_FOREACH(this->getCameraObjectList(), ref) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
		auto data = static_cast<agtk::CameraObject*>(ref);
#else
		auto data = dynamic_cast<agtk::CameraObject*>(ref);
#endif

		if (initialCameraObj == nullptr) {
			initialCameraObj = data;
		}
		else {
			// 今、設定されているデータのIDより若いIDの場合、初期化カメラとする
			if (initialCameraObj->getId() > data->getId()) {
				initialCameraObj = data;
			}
		}
	}

	// 初期カメラがない場合はエラー
	CC_ASSERT(initialCameraObj);

	return initialCameraObj;
}

agtk::OthersCourse *Scene::getOthersCourse(int courseId)
{
	if (courseId >= 0) {
		// 指定されたコースを取得する
		auto courseList = this->getCourseList();

		cocos2d::Ref *ref = nullptr;
		CCARRAY_FOREACH(courseList, ref) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
			auto course = static_cast<agtk::OthersCourse*>(ref);
#else
			auto course = dynamic_cast<agtk::OthersCourse*>(ref);
#endif
			if (course->getId() == courseId) {
				return course;
			}
		}
	}

	return nullptr;
}

void Scene::changeCamera(agtk::CameraObject* cameraObject)
{
	// カメラオブジェクトが未設定なら処理しない
	if (cameraObject == nullptr) {
		return;
	}

	auto camera = getCamera();

	if (camera) {
		// カメラのデータを取得取得
		auto cameraData = cameraObject->getCameraData();

		Vec2 pos = Vec2::ZERO;

		// 初期設定：カメラの初期位置が「プレイヤー」場合
		if (cameraData->getInitialPlayerPosition()) {
			// プレイヤーの座標を設定
			auto player = this->getObjectPlayer();

			if (player) {
				pos = player->getCenterPosition();
			}
		}
		// 初期設定：カメラの初期位置が「座標で指定」の場合
		else {
			// 設定されている座標は左端の座標なので中心位置を求める
			pos = Vec2(cameraData->getX(), cameraData->getY());
			pos.x += cameraData->getWidth() * 0.5f;
			pos.y += cameraData->getHeight() * 0.5f;
		}

		// カメラの表示サイズを設定
		camera->setDisplaySize(cameraData->getWidth(), cameraData->getHeight());

		// 初期位置を設定
		camera->setTargetPosition(pos, true);

		if (camera->isAutoScroll() == false) {
			// 「カメラの追従対象」を設定
			switch (cameraData->getFollowTargetType()) {
				// 「プレイヤー」
				case agtk::data::OthersCameraData::kFollowTargetPlayer: {
					// プレイヤーを設定
					auto player = this->getObjectPlayer();

					if (player) {
						camera->setTargetObject(player);
					}
				} break;

				// 「オブジェクト」
				case agtk::data::OthersCameraData::kFollowTargetObject: {

					// 指定IDのオブジェクトリストを取得
					auto objectList = this->getObjectAll(cameraData->getObjectId(), SceneLayer::kTypeScene);

					// オブジェクトが存在した場合した場合
					if (objectList->count() > 0) {
						// リストの先頭のオブジェクトを追従対象に設定する設定する
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
						auto object = static_cast<agtk::Object*>(objectList->getObjectAtIndex(0));
#else
						auto object = dynamic_cast<agtk::Object*>(objectList->getObjectAtIndex(0));
#endif
						camera->setTargetObject(object);
					}

				} break;

				// 「コース」
				case agtk::data::OthersCameraData::kFollowTargetCourse: {

					auto courseScenePartId = cameraData->getCourseScenePartId();
					auto startPointId = cameraData->getStartPointId();

					// 該当コースが存在する場合
					if (courseScenePartId > -1 && startPointId > -1) {
						// コース移動の初期化
						cameraObject->initCourse(courseScenePartId, startPointId, this);
					}
					// 存在しない場合
					else {
						// 追従対象無しと同じ処理とする
						camera->setTargetFixedPosition(pos);
					}

				} break;

				// 「無し」
				case agtk::data::OthersCameraData::kFollowTargetNone: {
					// カメラを固定位置に設定する
					camera->setTargetFixedPosition(pos);
				} break;
			}
		}
		// 自動スクロールがONの場合は追従させない
		else {
			// カメラを固定位置に設定する
			camera->setTargetFixedPosition(pos);
		}

		// 使用中のカメラオブジェクトを保持
		setCurrentCameraObject(cameraObject);
	}
}

void Scene::start(cocos2d::Layer *layer, bool bSetupScreen)
{
	AGTK_DEBUG_ACTION_LOG("%d,%s", __LINE__, __FUNCTION__);
	this->setScenePhase(kPhaseStart);
	auto camera = this->getCamera();
	if (camera) {
		camera->start();
	}
	auto gameManager = GameManager::getInstance();
	gameManager->setCurrentScene(this);
	auto projectData = gameManager->getProjectData();

	CC_ASSERT(layer);
	this->setLayer(layer);

	//ウインドウ表示およびフルスクリーン、解像度を変更
	if (bSetupScreen) {
#if (CC_TARGET_PLATFORM != CC_PLATFORM_NX) // #AGTK-NX
		int maxPixelScale = -1;
		if (projectData->getAdjustPixelMagnification()) {
			maxPixelScale = 0;	//拡大率制限無し。
			if (projectData->getPixelMagnificationType() == agtk::data::ProjectData::kPixelMagnificationTypeUseWindowMaginification) {
				maxPixelScale = projectData->getMagnifyWindow() ? projectData->getWindowMagnification() : 1;
			}
		}
		auto view = Director::getInstance()->getOpenGLView();
		view->limitPixelMagnificationByInteger(maxPixelScale);
		//ウインドウ表示:0,フルスクリーン:1
		if ((bool)projectData->getScreenSettings() != agtk::IsFullScreen()) {
			agtk::ChangeScreen(projectData->getScreenSettings());
		}

		//ウインドウ表示の場合。
		if (!agtk::IsFullScreen()) {
			//解像度を変更。
			auto screenSize = projectData->getScreenSize();
			auto nowScreenSize = agtk::GetScreenResolutionSize();
			float magnifyWindow = projectData->getMagnifyWindow() ? projectData->getWindowMagnification() : 1.0f;
			auto windowSize = screenSize * magnifyWindow;
			auto nowWindowSize = agtk::GetFrameSize();

			if ((screenSize.width != nowScreenSize.width || screenSize.height != nowScreenSize.height)
			|| (windowSize.width != nowWindowSize.width || windowSize.height != nowWindowSize.height)
			|| (view->getFrameZoomFactor() != magnifyWindow)) {
				agtk::ChangeScreenResolutionSize(screenSize, magnifyWindow);
			}
		}
#endif
		//メニューシーン用カメラを初期化。
		auto menuCamera = camera->getMenuCamera();
		menuCamera->initDefault();

		//最前面シーン用カメラを初期化。
		auto topMostCamera = camera->getTopMostCamera();
		topMostCamera->initDefault();
	}
	else {
		//プレイデータ調整する。
		auto playData = GameManager::getInstance()->getPlayData();
		playData->adjustData();
	}

	//リロードする際のシーンIDをセットする。
	auto debugManager = DebugManager::getInstance();
	int selectSceneId = debugManager->findSelectSceneId(this->getSceneData()->getId());
	debugManager->setSelectSceneId(selectSceneId);
	debugManager->createDisplayData();
	if (debugManager->getShowDebugDispGrid()) {
		debugManager->createGridLine();
	}
	if (debugManager->getShowTileWallFlag()) {
		debugManager->showTileWallView(true);
	}
#ifdef USE_PREVIEW
	if (debugManager->getShowPartOthersFlag()) {
		debugManager->showPartOthersView(true);
	}
	if (debugManager->getShowPhysicsBoxEnabled()) {
		GameManager::getInstance()->setPhysicsDebugVisible(true);
	}
	if (debugManager->getShowPortalFlag()) {
		debugManager->showPortalView(true);
	}
	if (debugManager->getShowLimitAreaFlag()) {
		debugManager->showLimitArea(true);
	}
	if (debugManager->getShowLimitCameraFlag()) {
		debugManager->showLimitCamera(true);
	}
#endif

#ifdef USE_PREVIEW
	auto ws = GameManager::getInstance()->getWebSocket();
	if (ws) {
		auto message = cocos2d::String::createWithFormat("system feedbackSceneInfo { \"sceneId\": %d }", this->getSceneData()->getId());
		ws->send(message->getCString());
	}
#endif
	//シーンを作成したので、このままではupdate(float delta)の引数deltaの値が増え、
	//フレームレートが下がるので、ここで次回deltaTimeを0クリアする。
	auto director = Director::getInstance();
	director->setNextDeltaTimeZero(true);
	this->setScenePhase(kPhasePlaying);

	//変数・スイッチ変更による更新
	GameManager::getInstance()->updateByChangingVariableAndSwitch();
	AGTK_DEBUG_ACTION_LOG("%d,%s", __LINE__, __FUNCTION__);
}

void Scene::stop()
{
	this->setScenePhase(kPhaseStop);
	auto camera = this->getCamera();
	if (camera) {
		camera->end();
	}

	auto debugManager = DebugManager::getInstance();
	debugManager->removeDisplayData();

	auto gameManager = GameManager::getInstance();
	gameManager->setCurrentScene(nullptr);

//	auto baseLayer = dynamic_cast<agtk::BaseLayer *>(gameManager->getCurrentLayer());
//	baseLayer->detachScene(this);

	//シーンを作成したので、このままではupdate(float delta)の引数deltaの値が増え、
	//フレームレートが下がるので、ここで次回deltaTimeを0クリアする。
	auto director = Director::getInstance();
	director->setNextDeltaTimeZero(true);
}

void Scene::end(bool forceGC)
{
#ifdef USE_AGTK
	if(forceGC){
		GameManager::getInstance()->getPhysicsWorld()->setDebugLayer(nullptr);
	}
#endif
	_objectInstanceId = 0;//インスタンスID（オブジェクトが作成されるとオブジェクトのインスタンスIDに割り振られる）のクリア。
	_objectInstanceMenuId = 0;
	this->setScenePhase(kPhaseEnd);
	PoolManager::getInstance()->getCurrentPool()->clear();
	//※この関数実行後、自シーンクラスは自ら破棄されます。
#ifdef USE_PREVIEW
	if (getPreviewInstanceId() >= 0) {
		setPreviewObjectId(-1);
		setPreviewInstanceId(-1);
		auto message = cocos2d::String::createWithFormat("object feedbackInstanceInfo { \"instanceId\": %d, \"objectId\": %d }", -1, -1);
		auto gameManager = GameManager::getInstance();
		auto ws = gameManager->getWebSocket();
		if (ws) {
			ws->send(message->getCString());
		}
	}
	ParticleManager::getInstance()->clearPreview();
#endif

	//シーン終了時の状態を維持
	{
		auto gameManager = GameManager::getInstance();
// #AGTK-NX
#ifdef USE_SAR_OPTIMIZE_1
		auto objectList = this->getObjectAllReference(SceneLayer::kTypeAll);
#else
		auto objectList = this->getObjectAll(SceneLayer::kTypeAll);
#endif
		cocos2d::Ref *ref;
		CCARRAY_FOREACH(objectList, ref) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
			auto obj = static_cast<agtk::Object *>(ref);
#else
			auto obj = dynamic_cast<agtk::Object *>(ref);
#endif
			if (this->getSceneData()->isScenePartObject(obj->getScenePartObjectData()) == false && !obj->getSceneData()->isMenuScene()) {
				continue;
			}
			auto objectData = obj->getObjectData();
			if (objectData->getTakeoverStatesAtSceneEnd()) {
				auto sceneEndTakeoverStatesObjectList = gameManager->getSceneEndTakeoverStatesObjectList();
				auto data = ObjectTakeoverStatesData::create(obj);
				if (data != nullptr) {
#if defined(AGTK_DEBUG)
					data->dump();
#endif
					sceneEndTakeoverStatesObjectList->addObject(data);
				}
			}
		}
	}

	//「アクションで復活」条件のオブジェクトで、シーン配置以外のオブジェクトは破棄する。
	GameManager::getInstance()->removeCommandReappearObjectList(true);

	//bg
	auto bg = this->getSceneBackground();
	if (bg) {
		bg->removeFromParentAndCleanup(true);
		bg->removeRenderTexture();
	}
	this->setSceneBackground(nullptr);

	auto objGroup = GameManager::getInstance()->getProjectData()->getObjectGroup();
	//sceneLayer
	cocos2d::DictElement *el = nullptr;
	auto sceneLayerList = this->getSceneLayerList();
	CCDICT_FOREACH(sceneLayerList, el) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
		auto p = static_cast<agtk::SceneLayer *>(el->getObject());
#else
		auto p = dynamic_cast<agtk::SceneLayer *>(el->getObject());
#endif
		p->removeFromParentAndCleanup(true);
		p->removeRenderTexture();
		for (int group = -1; group < p->getGroupCollisionDetections()->count(); group++) {
			CollisionDetaction *collisionDetection = p->getGroupCollisionDetection(group);
			if (collisionDetection) {
				collisionDetection->reset();
			}
		}
		for (int group = 0; group < p->getGroupRoughWallCollisionDetections()->count(); ++group) {
			CollisionDetaction *collisionDetection = p->getGroupRoughWallCollisionDetection(group);
			if (collisionDetection) {
				collisionDetection->reset();
			}
		}
		for (int group = 0; group < objGroup->count(); ++group) {
			CollisionDetaction *collisionDetection = p->getGroupWallCollisionDetection(group);
			if (collisionDetection) {
				collisionDetection->reset();
			}
		}
		p->end();
	}
	//topMost
	auto topMost = this->getSceneTopMost();
	if (topMost) {
		topMost->removeFromParentAndCleanup(true);
		topMost->removeRenderTexture();
		topMost->removeWithMenuRenderTexture();
	}
	this->setSceneTopMost(nullptr);

	//menuLayer
	if (this->getMenuLayerList()->count()) {
		cocos2d::DictElement *el;
		auto menuLayerList = this->getMenuLayerList();
		CCDICT_FOREACH(menuLayerList, el) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
			auto p = static_cast<agtk::SceneLayer *>(el->getObject());
#else
			auto p = dynamic_cast<agtk::SceneLayer *>(el->getObject());
#endif
			p->removeFromParentAndCleanup(true);
			p->removeRenderTexture();
			for (int group = -1; group < p->getGroupCollisionDetections()->count(); group++) {
				CollisionDetaction *collisionDetection = p->getGroupCollisionDetection(group);
				if (collisionDetection) {
					collisionDetection->reset();
				}
			}
			for (int group = 0; group < p->getGroupRoughWallCollisionDetections()->count(); ++group) {
				CollisionDetaction *collisionDetection = p->getGroupRoughWallCollisionDetection(group);
				if (collisionDetection) {
					collisionDetection->reset();
				}	
			}
			for (int group = 0; group < objGroup->count(); ++group) {
				CollisionDetaction *collisionDetection = p->getGroupWallCollisionDetection(group);
				if (collisionDetection) {
					collisionDetection->reset();
				}
			}
			p->end();
		}
	}
	//renderTextureCtrl
	auto renderTextureCtrlList = this->getRenderTextureCtrlList();
	CCDICT_FOREACH(renderTextureCtrlList, el) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
		auto p = static_cast<agtk::RenderTextureCtrl *>(el->getObject());
#else
		auto p = dynamic_cast<agtk::RenderTextureCtrl *>(el->getObject());
#endif
		p->removeFromParentAndCleanup(true);
	}
	this->setRenderTextureCtrlList(nullptr);

	//camera
	auto camera = this->getCamera();
	if (camera) {
		camera->end();
	}

	cocos2d::Ref *ref = nullptr;
	CCARRAY_FOREACH(this->getCameraObjectList(), ref) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
		auto p = static_cast<agtk::CameraObject *>(ref);
#else
		auto p = dynamic_cast<agtk::CameraObject *>(ref);
#endif
		p->removeFromParentAndCleanup(true);
	}

	//particle
	ParticleManager::getInstance()->removeAllParticles();
	//gui
	GuiManager::getInstance()->clearGui(this);
	//effect
	EffectManager::getInstance()->clearEffect();
	//bullet
	BulletManager::getInstance()->removeAllBullet();

	this->removeFromParentAndCleanup(true);
#ifdef USE_PREVIEW
	auto ws = GameManager::getInstance()->getWebSocket();
	if (ws) {
		auto message = cocos2d::String::createWithFormat("system feedbackSceneInfo { \"sceneId\": %d }", -1);
		ws->send(message->getCString());
	}
#endif
	GameManager::getInstance()->setCurrentScene(nullptr);

	DebugManager::getInstance()->reset();
	if (forceGC) {
		JavascriptManager::forceGC();
	}
}

void Scene::update(float delta)
{
	if (this->getRequestSwitchLoadFile()) {
		//セーブデータのロード待機中。ゲームが進行しないように、何もせずに抜ける。
		return;
	}
	PROFILING("Scene::update", profiling);
	_elapsedTime += delta;

	// GUI用のデルタタイム
	float guiDelta = delta;

	// メッセージ表示によるゲーム停止が発生した時、デルタタイムを0にし
	// GUI以外のゲームの進行を停止させる
	if (GuiManager::getInstance()->getGameStop()) {
		delta *= 0.0f;
	}

	// 待ちフレームをデクリメント
	bool bWait = false;
	if (_waitDuration300 < 0) {
		//※_waitDuration300にマイナス値を設定した場合は時間制限なしに待ちになる。
		bWait = true;
	}
	else {
		_waitDuration300 -= (int)(delta * 300);
		if (_waitDuration300 <= 0) {
			_waitDuration300 = 0;
		}
		else {
			bWait = true;
		}
	}

	//shader (pause or resume)
	bWait ? this->pauseShader() : this->resumeShader();

	if (bWait) {
		//※「オプション：画像表示中はゲームの動作を一時停止」で、画像表示の情報を更新するために必要。
		// image
		ImageManager::getInstance()->update(delta);
		// movie
		MovieManager::getInstance()->update(delta);
		//camera
		this->updateCamera(delta);
		// render texture
		this->updateSceneRenderTextureList(delta);
		return;
	}

	// update variable timer
	this->updateVariableTimer(delta);

	this->getGravity()->update(delta);
	this->getWater()->update(delta);
	this->getGameSpeed()->update(delta);
	this->getShake()->update(delta);

	{
		cocos2d::DictElement *el = nullptr;
		auto sceneLayerList = this->getSceneLayerList();
		CCDICT_FOREACH(sceneLayerList, el) {
			auto sceneLayer = dynamic_cast<agtk::SceneLayer *>(el->getObject());
			if (sceneLayer) {
				if (!sceneLayer->getWallCollisionInit()) {
					for (int group = 0; group < sceneLayer->getGroupWallCollisionDetections()->count(); ++group) {
						CollisionDetaction *wallCollisionDetection = sceneLayer->getGroupWallCollisionDetection(group);
						if (!wallCollisionDetection) {
							continue;
						}
						wallCollisionDetection->initForSingle();
					}
					sceneLayer->setWallCollisionInit(true);
				}
			}
		}
	}
	CC_PROFILER_SET_FRAME_POINT("before layer update");
	//earlyUpdate
	cocos2d::DictElement *el = nullptr;
	{
		//layer
		auto sceneLayerList = this->getSceneLayerList();
		CCDICT_FOREACH(sceneLayerList, el) {
			auto sceneLayer = dynamic_cast<agtk::SceneLayer *>(el->getObject());
			if (sceneLayer) {
				sceneLayer->earlyUpdate(delta);
			}
		}

		//menu
		auto menuLayerList = this->getMenuLayerList();
		CCDICT_FOREACH(menuLayerList, el) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
			auto menuLayer = static_cast<agtk::SceneLayer *>(el->getObject());
#else
			auto menuLayer = dynamic_cast<agtk::SceneLayer *>(el->getObject());
#endif
			menuLayer->earlyUpdate(delta);
		}
	}


	// Physics
	// ※オブジェクトに紐付いた物理オブジェクトが優先される事があるのでオブジェクトの更新より先に物理オブジェクトを更新する
#ifdef USE_30FPS_2
	if ((int)GameManager::getInstance()->getFrameProgressScale() > 1) {
		// 30FPSの場合のように更新間隔が長くなると一回のPhysics更新での衝突インパルスが大きくなりすぎて
		// 計算過程でvelocityが不正な値となってしまうことがある。その為、分割更新により物理解決を行う。
		float deltaHalf = delta*0.5f;
		GameManager::getInstance()->updatePhysics(deltaHalf);
// #AGTK-NX, #AGTK-WIN
#ifdef USE_30FPS_4
		// 30FPSでの中間フレームの壁判定を得るためのPhysics設定
		{
// #AGTK-NX
#ifdef USE_SAR_OPTIMIZE_1
			auto objectList = getObjectAllReference();
#else
			auto objectList = getObjectAll();
#endif
			cocos2d::Ref *ref = nullptr;
			CCARRAY_FOREACH(objectList, ref) {
				auto object = static_cast<agtk::Object *>(ref);
				object->_middleFrameStock.updatePhysics();
			}
		}
#endif
		GameManager::getInstance()->updatePhysics(delta - deltaHalf);
	}
	else {
		GameManager::getInstance()->updatePhysics(delta);
	}
#else
	GameManager::getInstance()->updatePhysics(delta);
#endif
	CC_PROFILER_SET_FRAME_POINT("after updatePhysics");

// #AGTK-NX, #AGTK-WIN
#ifdef USE_30FPS_4
	// カレントフレームのFPSに関わらず中間フレームの壁判定作成条件が成立する場合は壁判定を作成する
	{
// #AGTK-NX
#ifdef USE_SAR_OPTIMIZE_1
		auto objectList = getObjectAllReference();
#else
		auto objectList = getObjectAll();
#endif

		cocos2d::Ref *ref = nullptr;
		CCARRAY_FOREACH(objectList, ref) {
			auto object = static_cast<agtk::Object *>(ref);
			object->_middleFrameStock.updateWall();
		}
	}
#endif

	//bg
	auto bg = this->getSceneBackground();
	if (bg) {
		bg->update(delta);
	}
	CC_PROFILER_SET_FRAME_POINT("after SceneBackground update");

	//lateUpdate
	{
		//layer
		auto sceneLayerList = this->getSceneLayerList();
		CCDICT_FOREACH(sceneLayerList, el) {
			auto sceneLayer = dynamic_cast<agtk::SceneLayer *>(el->getObject());
			if (sceneLayer) {
				sceneLayer->update(delta);
			}
		}
		CC_PROFILER_SET_FRAME_POINT("after sceneLayer update");

		//menu
		auto menuLayerList = this->getMenuLayerList();
		CCDICT_FOREACH(menuLayerList, el) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
			auto menuLayer = static_cast<agtk::SceneLayer *>(el->getObject());
#else
			auto menuLayer = dynamic_cast<agtk::SceneLayer *>(el->getObject());
#endif
			menuLayer->update(delta);
		}
	}
	CC_PROFILER_SET_FRAME_POINT("after layer update");

	//topMost
	auto topMost = this->getSceneTopMost();
	if (topMost) {
		topMost->update(delta);
	}
	CC_PROFILER_SET_FRAME_POINT("after SceneTopMost update");

	// particle
	ParticleManager::getInstance()->update(delta);

	// effect
	EffectManager::getInstance()->update(delta);

	// bullet
	BulletManager::getInstance()->update(delta);

	// viewport & light
	this->getViewportLight()->update(delta);

	// image
	ImageManager::getInstance()->update(delta);

	// movie
	MovieManager::getInstance()->update(delta);

	//camera
	this->updateCamera(delta);
	CC_PROFILER_SET_FRAME_POINT("after updateCamera");

	// カメラの座標更新後に別の更新関数を呼び出す
	auto sceneLayerList2 = this->getSceneLayerList();
	CCDICT_FOREACH(sceneLayerList2, el) {
		auto sceneLayer = dynamic_cast<agtk::SceneLayer *>(el->getObject());
		if (sceneLayer) {
			sceneLayer->lateUpdate(delta);
		}
	}
	auto menuLayerList2 = this->getMenuLayerList();
	CCDICT_FOREACH(menuLayerList2, el) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
		auto menuLayer = static_cast<agtk::SceneLayer *>(el->getObject());
#else
		auto menuLayer = dynamic_cast<agtk::SceneLayer *>(el->getObject());
#endif
		menuLayer->lateUpdate(delta);
	}
	CC_PROFILER_SET_FRAME_POINT("after SceneLayer lateUpdate");

	// シーンを繋げる処理
	procLoop();

	// gui
	GuiManager::getInstance()->update(guiDelta);

	// primitive
	PrimitiveManager::getInstance()->update(delta);

	//render texture
	this->updateSceneRenderTextureList(delta);

	CC_PROFILER_SET_FRAME_POINT("after updateSceneRenderTextureList");

#ifdef USE_PREVIEW
	auto previewMode = GameManager::getInstance()->getPreviewMode();
	if (previewMode && previewMode->compare("physics") == 0 && this->getPhysicsRequested()) {
		this->setPhysicsRequested(false);
		auto projectData = GameManager::getInstance()->getProjectData();
		float screenHeight = projectData->getScreenHeight();
		auto sceneData = this->getSceneData();
		auto sceneHeight = projectData->getScreenHeight() * sceneData->getVertScreenCount();
		std::list<std::string> infoList;
		auto watchPartIdList = this->getWatchPhysicsPartIdList();
		cocos2d::Ref *ref = nullptr;
		std::map<int, std::list<std::string> > ropeInfoMap;
		CCARRAY_FOREACH(watchPartIdList, ref) {
			auto p = dynamic_cast<cocos2d::Integer *>(ref);
			auto scenePartId = p->getValue();
			auto objectData = this->getObjectInstance(-1, scenePartId);
			if (objectData) {
// #AGTK-NX
#if CC_TARGET_PLATFORM != CC_PLATFORM_NX
				auto &position = objectData->getDispPosition();
#else
#endif
				CCLOG("%d: (%f, %f), %f", scenePartId, position.x, position.y, objectData->getRotation());
				std::string str = cocos2d::String::createWithFormat("\"scenePartId\": %d, \"x\": %f, \"y\": %f, \"rotation\": %f", scenePartId, position.x, position.y, objectData->getRotation())->getCString();
				auto physicsPartsList = objectData->getPhysicsPartsList();
				cocos2d::Ref *ref2 = nullptr;
				std::list<std::string> subInfoList;
				std::map<int, std::list<std::string> > subRopeInfoMap;
				CCARRAY_FOREACH(physicsPartsList, ref2) {
					auto disk = dynamic_cast<PhysicsDisk *>(ref2);
					if (disk) {
						auto &pos = disk->getPosition();
						CCLOG("\t%d: Disk: (%f, %f), %f", disk->getScenePartsId(), pos.x, pos.y, disk->getRotationZX());
						subInfoList.push_back(cocos2d::String::createWithFormat("{ \"scenePartId\": %d, \"x\": %f, \"y\": %f, \"rotation\": %f }", disk->getScenePartsId(), pos.x, sceneHeight - pos.y, disk->getRotationZX())->getCString());
					}
					auto rectangle = dynamic_cast<PhysicsRectangle *>(ref2);
					if (rectangle) {
						auto &pos = rectangle->getPosition();
						CCLOG("\t%d: Rectangle: (%f, %f), %f", rectangle->getScenePartsId(), pos.x, pos.y, rectangle->getRotationZX());
						subInfoList.push_back(cocos2d::String::createWithFormat("{ \"scenePartId\": %d, \"x\": %f, \"y\": %f, \"rotation\": %f }", rectangle->getScenePartsId(), pos.x, sceneHeight - pos.y, rectangle->getRotationZX())->getCString());
					}
					auto rope = dynamic_cast<PhysicsRopeParts *>(ref2);
					if (rope) {
						auto physicsRopeData = rope->getPhysicsData();
						auto idx = rope->getIdx();
						if (idx == 0 || idx == physicsRopeData->getPointList()->count() + 1) {
							continue;
						}
						auto ropePartId = rope->getScenePartsId();
						if (subRopeInfoMap.find(ropePartId) == subRopeInfoMap.end()) {
							subRopeInfoMap.insert(std::make_pair(ropePartId, std::list<std::string>()));
						}
						CCLOG("\t%d: Rope%d: (%f, %f), %f", rope->getScenePartsId(), idx, rope->getPosition().x, rope->getPosition().y);
						auto &ropePos = rope->getPosition();
						subRopeInfoMap[ropePartId].push_back(cocos2d::String::createWithFormat("{ \"idx\": %d, \"x\": %f, \"y\": %f }", idx, ropePos.x, sceneHeight - ropePos.y)->getCString());
						if (idx == physicsRopeData->getPointList()->count()) {
							//todo
							auto vec = cocos2d::Vec2(0, rope->getContentSize().height - PhysicsRopeParts::ContentHeightAddition);
							auto newVec = vec.rotateByAngle(cocos2d::Vec2::ZERO, rope->getRotation() * (M_PI / 180.0f));
							subRopeInfoMap[ropePartId].push_back(cocos2d::String::createWithFormat("{ \"idx\": %d, \"x\": %f, \"y\": %f }", idx + 1, ropePos.x + newVec.x, sceneHeight - ropePos.y + newVec.y)->getCString());
						}
					}
					//PHYSICS_PREVIEW
				}
				for (auto &pair : subRopeInfoMap) {
					int scenePartId = pair.first;
					subInfoList.push_back(cocos2d::String::createWithFormat("{ \"scenePartId\": %d, \"pointList\": [ %s ] }", scenePartId, data::join(pair.second, ",").c_str())->getCString());
				}
				if (subInfoList.size() > 0) {
					str += cocos2d::String::createWithFormat(", \"physicsPartList\": [ %s ]", data::join(subInfoList, ", ").c_str())->getCString();
				}
				infoList.push_back(cocos2d::String::createWithFormat("{ %s }", str.c_str())->getCString());
			}
			else {
				auto arr = this->getScenePhysicsObject(scenePartId);
				cocos2d::Ref *ref2 = nullptr;
				CCARRAY_FOREACH(arr, ref2) {
					auto physicsBase = dynamic_cast<PhysicsBase *>(ref2);
					auto disk = dynamic_cast<PhysicsDisk *>(physicsBase);
					if (disk) {
						auto &pos = disk->getPosition();
						CCLOG("%d: Disk: (%f, %f), %f", disk->getScenePartsId(), pos.x, pos.y, disk->getRotationZX());
						infoList.push_back(cocos2d::String::createWithFormat("{ \"scenePartId\": %d, \"x\": %f, \"y\": %f, \"rotation\": %f }", disk->getScenePartsId(), pos.x, sceneHeight - pos.y, disk->getRotationZX())->getCString());
					}
					auto rectangle = dynamic_cast<PhysicsRectangle *>(physicsBase);
					if (rectangle) {
						auto &pos = rectangle->getPosition();
						CCLOG("%d: Rectangle: (%f, %f), %f", rectangle->getScenePartsId(), pos.x, pos.y, rectangle->getRotationZX());
						infoList.push_back(cocos2d::String::createWithFormat("{ \"scenePartId\": %d, \"x\": %f, \"y\": %f, \"rotation\": %f }", rectangle->getScenePartsId(), pos.x, sceneHeight - pos.y, rectangle->getRotationZX())->getCString());
					}
					auto rope = dynamic_cast<PhysicsRopeParts *>(physicsBase);
					if (rope) {
						auto physicsRopeData = rope->getPhysicsData();
						auto idx = rope->getIdx();
						if (idx == 0 || idx == physicsRopeData->getPointList()->count() + 1) {
							continue;
						}
						auto ropePartId = rope->getScenePartsId();
						if (ropeInfoMap.find(ropePartId) == ropeInfoMap.end()) {
							ropeInfoMap.insert(std::make_pair(ropePartId, std::list<std::string>()));
						}
						CCLOG("%d: Rope%d: (%f, %f), %f", rope->getScenePartsId(), idx, rope->getPosition().x, rope->getPosition().y);
						auto &ropePos = rope->getPosition();
						ropeInfoMap[ropePartId].push_back(cocos2d::String::createWithFormat("{ \"idx\": %d, \"x\": %f, \"y\": %f }", idx, ropePos.x, sceneHeight - ropePos.y)->getCString());
						if (idx == physicsRopeData->getPointList()->count()) {
							//todo
							auto vec = cocos2d::Vec2(0, rope->getContentSize().height - PhysicsRopeParts::ContentHeightAddition);
							auto newVec = vec.rotateByAngle(cocos2d::Vec2::ZERO, rope->getRotation() * (M_PI / 180.0f));
							ropeInfoMap[ropePartId].push_back(cocos2d::String::createWithFormat("{ \"idx\": %d, \"x\": %f, \"y\": %f }", idx + 1, ropePos.x + newVec.x, sceneHeight - ropePos.y + newVec.y)->getCString());
						}
					}
				}
				//PHYSICS_PREVIEW
			}
		}
		for (auto &pair : ropeInfoMap) {
			int scenePartId = pair.first;
			infoList.push_back(cocos2d::String::createWithFormat("{ \"scenePartId\": %d, \"pointList\": [ %s ] }", scenePartId, data::join(pair.second, ",").c_str())->getCString());
		}
		if (infoList.size() > 0) {
			std::string infoStr = data::join(infoList, ", ");
			auto message = cocos2d::String::createWithFormat("scene feedbackPhysics [ %s ]", infoStr.c_str());
			auto gameManager = GameManager::getInstance();
			auto ws = gameManager->getWebSocket();
			if (ws) {
				ws->send(message->getCString());
			}
		}
	}
#endif

	auto gameManager = GameManager::getInstance();
	auto projectPlayData = gameManager->getPlayData();

	// 変数・スイッチ初期化要求スイッチがONの場合
	if (projectPlayData->getCommonSwitchData(agtk::data::EnumProjectSystemSwitch::kProjectSystemSwitchInit)->getValue()) {
		this->setRequestSwitchInit(true);
		projectPlayData->getCommonSwitchData(agtk::data::EnumProjectSystemSwitch::kProjectSystemSwitchInit)->setValue(false);
	}
	// ACT2-6205 リセット要求スイッチがONの場合
	if (projectPlayData->getCommonSwitchData(agtk::data::EnumProjectSystemSwitch::kProjectSystemSwitchReset)->getValue()) {
		this->setRequestSwitchReset(true);
		projectPlayData->getCommonSwitchData(agtk::data::EnumProjectSystemSwitch::kProjectSystemSwitchReset)->setValue(false);
	}
	// セーブ要求スイッチがONの場合
	if (projectPlayData->getCommonSwitchData(agtk::data::EnumProjectSystemSwitch::kProjectSystemSwitchSaveFile)->getValue()) {
		this->setRequestSwitchSaveFile(true);
		projectPlayData->getCommonSwitchData(agtk::data::EnumProjectSystemSwitch::kProjectSystemSwitchSaveFile)->setValue(false);
	}
	// 削除要求スイッチがONの場合
	if (projectPlayData->getCommonSwitchData(agtk::data::EnumProjectSystemSwitch::kProjectSystemSwitchDeleteFile)->getValue()) {
		this->setRequestSwitchDeleteFile(true);
		projectPlayData->getCommonSwitchData(agtk::data::EnumProjectSystemSwitch::kProjectSystemSwitchDeleteFile)->setValue(false);
	}
	// コピー要求スイッチがONの場合
	if (projectPlayData->getCommonSwitchData(agtk::data::EnumProjectSystemSwitch::kProjectSystemSwitchCopyFile)->getValue()) {
		this->setRequestSwitchCopyFile(true);
		projectPlayData->getCommonSwitchData(agtk::data::EnumProjectSystemSwitch::kProjectSystemSwitchCopyFile)->setValue(false);
	}
	// ロード要求スイッチがONの場合
	if (projectPlayData->getCommonSwitchData(agtk::data::EnumProjectSystemSwitch::kProjectSystemSwitchLoadFile)->getValue()) {
		this->setRequestSwitchLoadFile(true);
		projectPlayData->getCommonSwitchData(agtk::data::EnumProjectSystemSwitch::kProjectSystemSwitchLoadFile)->setValue(false);
	}
	CC_PROFILER_SET_FRAME_POINT("leave Scene::update");

	auto list = this->getPhysicObjectAll(-1, false, false);
	gameManager->getPhysicsWorld()->debugDraw(list);
}

/**
* シーンが持つレンダーテクスチャリストの更新
* @param	delta				前フレームからの経過時間
* @param	ignoreVisibleObject	外オブジェクト表示除外フラグ
*/
void Scene::updateSceneRenderTextureList(float delta, bool ignoreVisibleObject, int layerId)
{
	auto camera = this->getCamera()->getCamera();
	auto viewMatrix = camera->getViewMatrix();

	// 背景
	{
		auto sceneBackground = this->getSceneBackground();
		if (sceneBackground) {
			sceneBackground->updateRenderer(delta, &viewMatrix);
		}
	}

	// メインシーン
	{
		auto sceneLayerList = this->getSceneLayerList();
		auto objList = Array::create();
		cocos2d::DictElement *el = nullptr;
		CCDICT_FOREACH(sceneLayerList, el) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
			auto sceneLayer = static_cast<agtk::SceneLayer *>(el->getObject());
#else
			auto sceneLayer = dynamic_cast<agtk::SceneLayer *>(el->getObject());
#endif
			sceneLayer->updateRenderer(delta, &viewMatrix, objList, ignoreVisibleObject);
		}
	}

	// メニューシーン
	// 通常MenuとkHudMenuLayerId CameraFlag::USER1
	// kHudTopMostLayerId        CameraFlag::USER2
	// ※CameraFlag::USER1は2つレイヤーで使われているので片方がTrueの場合は強制的にもう片方も表示させる
	{
		auto menuCamera = this->getCamera()->getMenuCamera();
		auto topMostCamera = this->getCamera()->getTopMostCamera();

		auto menuLayerList = this->getMenuLayerList();
		cocos2d::DictElement *el;

		bool isMenuVisible = false;
		CCDICT_FOREACH(menuLayerList, el) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
			auto menuLayer = static_cast<agtk::SceneLayer *>(el->getObject());
#else
			auto menuLayer = dynamic_cast<agtk::SceneLayer *>(el->getObject());
#endif

			// 通常MenuとkHudMenuLayerId
			if (menuLayer->getLayerId() != agtk::data::SceneData::kHudTopMostLayerId) {
#ifdef USE_DESIGNATED_RESOLUTION_RENDER
	// USER1で使用しているため、必ず表示させるように変更
#else
				int count = menuLayer->getInitChildrenCount();

				if (isMenuVisible || menuLayer->isChildrenCountOver(menuLayer, count)) {
					menuCamera->setVisible(true);
					isMenuVisible = true;
				}
				else {
					menuCamera->setVisible(false);
					isMenuVisible = false;
				}
#endif
				menuLayer->setCameraMask((unsigned short)cocos2d::CameraFlag::USER1);
			}
			// kHudTopMostLayerId
			else if (menuLayer->getLayerId() == agtk::data::SceneData::kHudTopMostLayerId) {
				int count = menuLayer->getInitChildrenCount();

				if (menuLayer->isChildrenCountOver(menuLayer, count)) {
					topMostCamera->setVisible(true);
				}
				else {
					topMostCamera->setVisible(false);
				}
				menuLayer->setCameraMask((unsigned short)cocos2d::CameraFlag::USER2);
			}
			menuLayer->setVisible(true);
		}

		// 描画順ソート(作成時にソートするとエラーがでるためここでやる)
		{
			// ソートする必要があるか判定
			bool isSort = false;
			int localZOrder = -1;
			cocos2d::DictElement *el;
			CCDICT_FOREACH(menuLayerList, el) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
				auto menuLayer = static_cast<agtk::SceneLayer *>(el->getObject());
#else
				auto menuLayer = dynamic_cast<agtk::SceneLayer *>(el->getObject());
#endif

				int setLocalZOrder = menuLayer->getLocalZOrder();
				if (localZOrder > setLocalZOrder) {
					isSort = true;
					break;
				}
				localZOrder = setLocalZOrder;
			}

			if (isSort) {
				struct LayerSortInfo {
					agtk::SceneLayer *sceneLayer;
					int localZOrder;
				};
				auto menuLayerList = this->getMenuLayerList();
				int layerCount = 0;
				auto listLength = menuLayerList->count();
				std::vector<LayerSortInfo> sortList;
				sortList.resize(listLength);
				cocos2d::DictElement *el;
				CCDICT_FOREACH(menuLayerList, el) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
					auto menuLayer = static_cast<agtk::SceneLayer *>(el->getObject());
#else
					auto menuLayer = dynamic_cast<agtk::SceneLayer *>(el->getObject());
#endif
					auto &p = sortList[layerCount];

					p.sceneLayer = menuLayer;
					p.localZOrder = menuLayer->getLocalZOrder();
					++layerCount;
				}
				std::sort(sortList.begin(), sortList.end(), [&](const LayerSortInfo &p1, const LayerSortInfo &p2) {
					return (p1.localZOrder < p2.localZOrder);
				});

				// ソートしたのをセット
				auto newObjectList = cocos2d::__Dictionary::create();
				for (auto &p : sortList) {
					newObjectList->setObject(p.sceneLayer, p.localZOrder);
				}
				this->setMenuLayerList(newObjectList);
			}
		}
	}

	// 最前面シーン
	{
		auto sceneTopMost = this->getSceneTopMost();
		if (sceneTopMost) {
			if (layerId < 0) {
				sceneTopMost->updateRenderer(delta, &viewMatrix);
			}
			else {
				CC_ASSERT(layerId > 1);
				sceneTopMost->updateRenderer(delta, &viewMatrix, layerId, -1);
				sceneTopMost->updateRendererFront(delta, &viewMatrix, 1, layerId - 1);
			}
		}
	}
}

/**
* シーンが持つレンダーテクスチャリストの更新
* @param	delta				前フレームからの経過時間
* @param	ignoreVisibleObject	外オブジェクト表示除外フラグ
*/
void Scene::updateSceneRenderTextureListIgnoreMenu(float delta, bool ignoreVisibleObject)
{
	auto camera = this->getCamera()->getCamera();
	auto viewMatrix = camera->getViewMatrix();

	// 最前面シーン
	{
		auto sceneTopMost = this->getSceneTopMost();
		if (sceneTopMost) {
			sceneTopMost->updateRendererIgnoreMenu(delta, &viewMatrix);
		}
	}
}

/**
* シーンが持つレンダーテクスチャリストの更新
* @param	delta				前フレームからの経過時間
* @param	ignoreVisibleObject	外オブジェクト表示除外フラグ
*/
void Scene::updateSceneRenderTextureListOnlyMenu(float delta, bool ignoreVisibleObject)
{
	auto camera = this->getCamera()->getCamera();
	auto viewMatrix = camera->getViewMatrix();

	// 最前面シーン
	{
		auto sceneTopMost = this->getSceneTopMost();
		if (sceneTopMost) {
			sceneTopMost->updateRendererOnlyMenu(delta, &viewMatrix);
		}
	}
}

void Scene::updateCamera(float dt)
{
	// カメラを取得
	auto camera = this->getCamera();

	auto cameraUpdated = _cameraUpdated;
	if (!_cameraUpdated) {
		_cameraUpdated = true;
	}
	{
		// カメラの切り替えが発生するかをチェック
		bool isChangeCamera = checkChangeCamera();

		if (camera != nullptr && getCurrentCameraObject() != nullptr) {
#ifdef FIX_ACT2_4471
			if (isChangeCamera && cameraUpdated) {	//※ACT2-5044 初期アクションによるカメラ設定変更をリセットしないため、初期更新時を除く。
				//カメラが切り替わったら、「カメラの表示領域を変更する」の効果をリセット。
				camera->setCommandZoom(cocos2d::Vec2::ONE);
			}
#endif
			// カメラのデータを取得
			auto cameraData = getCurrentCameraObject()->getCameraData();

			if (camera->isAutoScroll() == false) {
				// 追従対象に応じて処理を変える
				switch (cameraData->getFollowTargetType()) {
					// プレイヤー
					case agtk::data::OthersCameraData::kFollowTargetPlayer: {
						// プレイヤーの座標を設定
						auto player = this->getObjectPlayer();

						if (player) {
							camera->setTargetObject(player, isChangeCamera);
						}
					} break;

					// オブジェクト
					case agtk::data::OthersCameraData::kFollowTargetObject: {

						// 対象のオブジェクトリストを取得
						auto objectList = this->getObjectAll(cameraData->getObjectId(), SceneLayer::kTypeScene);

						if (objectList->count() > 0) {
							// 現在の追従対象を取得
							auto currentTargetObj = camera->getTargetObject();

							// 追従対象が未設定の場合
							if (currentTargetObj == nullptr || camera->getTargetType() == Camera::CameraTargetType::kCameraTargetNone) {
								// 追従対象を設定
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
								auto targetObj = static_cast<agtk::Object*>(objectList->getObjectAtIndex(0));
#else
								auto targetObj = dynamic_cast<agtk::Object*>(objectList->getObjectAtIndex(0));
#endif
								camera->setTargetObject(targetObj, isChangeCamera);
							}
							// 追従対象が存在する場合
							else {

								// 対象オブジェクトが消滅していないかを確認する
								bool existsObject = false;
								cocos2d::Ref* ref = nullptr;
								CCARRAY_FOREACH(objectList, ref) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
									auto obj = static_cast<agtk::Object*>(ref);
#else
									auto obj = dynamic_cast<agtk::Object*>(ref);
#endif

									if (obj->getLayerId() == currentTargetObj->getLayerId() &&
										obj->getId() == currentTargetObj->getId()) {
										existsObject = true;
										break;
									}
								}

								// 現在の追従対象が存在していない場合
								if (!existsObject) {
									// 追従対象を再設定
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
									auto targetObj = static_cast<agtk::Object*>(objectList->getObjectAtIndex(0));
#else
									auto targetObj = dynamic_cast<agtk::Object*>(objectList->getObjectAtIndex(0));
#endif
									camera->setTargetObject(targetObj, isChangeCamera);
								}
								else {
									camera->setTargetObject(currentTargetObj, isChangeCamera);
								}
							}
						}

					} break;

					// コース
					case agtk::data::OthersCameraData::kFollowTargetCourse: {

						auto course = getCurrentCameraObject()->getObjectCourseMove();

						// コース未生成時
						if (course == nullptr) {

							auto courseScenePartId = cameraData->getCourseScenePartId();
							auto startPointId = cameraData->getStartPointId();

							// 存在するコースの場合
							if (courseScenePartId > -1 && startPointId > -1) {
								// コース移動の初期化
								getCurrentCameraObject()->initCourse(courseScenePartId, startPointId, this);
								course = getCurrentCameraObject()->getObjectCourseMove();
							}
						}

						// コース取得成功時
						if (course != nullptr) {
							course->update(dt);

							// コース移動後の座標を取得
							auto pos = course->getCurrentPos();
							// Y座標を変換
							pos = getPositionCocos2dFromScene(pos);
							camera->setTargetPosition(pos, isChangeCamera, false);
						}
					} break;

					// 無し
					case agtk::data::OthersCameraData::kFollowTargetNone: {
						// 何もしない
					} break;
				}
			}

			bool isScrollActivated = false;				// スクロールで収める事ができたフラグ
			bool isNeedForcusToPlayableObj = false;		// スクロールで収められなかった場合に操作プレイヤーにフォーカスするフラグ
			Vec2 basePos = camera->getPosition();
			Vec2 forcusPos = Vec2::ZERO;

			if (camera->getTargetType() == Camera::CameraTargetType::kCameraTargetObject &&
				camera->getTargetObject() != nullptr) {
				basePos = camera->getTargetObject()->getCenterPosition();
			}
			else {
				basePos = camera->getTargetPosition();
			}

			// --------------------------------
			// カメラ矩形を取得するメソッド
			auto getCameraRect = [](agtk::Camera *camera, agtk::data::OthersCameraData *cameraData, bool bThreashold) -> Rect {
				// カメラのサイズを取得
				auto cameraSize = camera->getDisplaySize();
				auto cameraPos = camera->getPosition();
				if (bThreashold) {
					cameraPos.x += agtk::data::OthersCameraData::CAMERA_SCROLL_AND_SCALE_THREASHOLD;
					cameraPos.y += agtk::data::OthersCameraData::CAMERA_SCROLL_AND_SCALE_THREASHOLD;
				}

				// 閾値を考慮してカメラのサイズを変更する
				// 2018.02.09 閾値は固定になった
				if (bThreashold) {
#if 0
					cameraSize.width -= cameraData->getScaleThresholdX() * 2;
					cameraSize.height -= cameraData->getScaleThresholdY() * 2;
#else
					cameraSize.width -= agtk::data::OthersCameraData::CAMERA_SCROLL_AND_SCALE_THREASHOLD * 2.0f;
					cameraSize.height -= agtk::data::OthersCameraData::CAMERA_SCROLL_AND_SCALE_THREASHOLD * 2.0f;
#endif
				}

				return Rect(cameraPos.x - cameraSize.width * 0.5f, cameraPos.y - cameraSize.height * 0.5f, cameraSize.width, cameraSize.height);
			};

			// --------------------------------------
			// カメラに収めるオブジェクトの原点とそのサイズを元にオブジェクトを含む最小最大のポイントを算出するメソッド
			auto getObjectRect = [](agtk::Object *obj) {
				auto player = obj->getPlayer();
				auto contentSize = player ? player->getContentSize() : obj->getContentSize();
				auto pos = obj->getLeftDownPosition();
				return cocos2d::Rect(pos, contentSize);
			};
			auto modifyMinMaxXY = [&](cocos2d::__Array *objectList, float &minX, float &maxX, float &minY, float &maxY) {
				cocos2d::Rect objRect;
				for (int i = 0; i < objectList->count(); i++) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
					auto obj = static_cast<agtk::Object *>(objectList->getObjectAtIndex(i));
#else
					auto obj = dynamic_cast<agtk::Object *>(objectList->getObjectAtIndex(i));
#endif
					if (i == 0) {
						objRect = getObjectRect(obj);
					}
					else {
						objRect.merge(getObjectRect(obj));
					}
				}
				minX = objRect.getMinX();
				maxX = objRect.getMaxX();
				minY = objRect.getMinY();
				maxY = objRect.getMaxY();
			};

			//1P>2P>3P>4Pの順に存在するオブジェクトを取得する。
			std::function<agtk::Object*()> getPlayableObject = [&]()->agtk::Object* {
				auto playerMax = GameManager::getInstance()->getProjectData()->getPlayerCount();
				// 操作可能なオブジェクトリストを取得
				auto playableObjList = this->getObjectAllObjGroup(agtk::data::ObjectData::kObjGroupPlayer, true, SceneLayer::kTypeScene);
				for (int playerId = 1; playerId <= playerMax; playerId++) {
					auto controller = InputManager::getInstance()->getSelectController(playerId);
					if (controller != nullptr) {
						cocos2d::Ref *ref;
						CCARRAY_FOREACH(playableObjList, ref) {
							auto obj = dynamic_cast<agtk::Object *>(ref);
							auto playObjectData = obj->getPlayObjectData();
							auto attachedControllerId = playObjectData->getVariableData(agtk::data::kObjectSystemVariableControllerID)->getValue();
							// 1Pから順に割り当てられたコントローラーIDと一致する場合
							if (attachedControllerId == controller->getId()) {
								return obj;
							}
						}
					}
				}
				return nullptr;
			};

			// プレイヤーオブジェクトのリストを取得
			auto objectList = this->getObjectAllObjGroup(agtk::data::ObjectData::EnumObjGroup::kObjGroupPlayer, true, SceneLayer::kTypeScene);

#define FIX_ACT2_4471
#ifdef FIX_ACT2_4471
			if (cameraData->getScrollToShowAllPlayers() == false && cameraData->getScaleToShowAllPlayers() == false) {
				//プレイヤーオブジェクトを考慮せずにカメラを動かして良い。
				auto scaleVec = camera->getCameraScale()->getValue();
				auto defScaleVec = cocos2d::Vec2::ONE;
				if (scaleVec != defScaleVec) {
					camera->setZoom(basePos, defScaleVec, 0.1f);
				}
			} else
#endif
			// プレイヤーオブジェクトが２つ以上存在する場合
			if (objectList->count() > 1) {

				float minX = basePos.x;
				float maxX = basePos.x;
				float minY = basePos.y;
				float maxY = basePos.y;

				// カメラに収めるオブジェクトの原点とそのサイズを元にオブジェクトを含む最小最大のポイントを算出する
				modifyMinMaxXY(objectList, minX, maxX, minY, maxY);

				// 範囲に含める対象のオブジェクトを全て含む形の矩形を生成
				Rect objRect = Rect(minX, minY, maxX - minX, maxY - minY);

				// カメラの矩形を取得
				Rect cameraRect = getCameraRect(camera, cameraData, true);

				// ----------------------------------------------------------------------------------
				// 「すべてのプレイヤーを収めるようスクロールする」設定時
				if (cameraData->getScrollToShowAllPlayers()) {

					bool isAllPlayerInCamera = true;
					isAllPlayerInCamera &= cameraRect.containsPoint(Vec2(objRect.getMinX(), objRect.getMaxY()));
					isAllPlayerInCamera &= cameraRect.containsPoint(Vec2(objRect.getMaxX(), objRect.getMaxY()));
					isAllPlayerInCamera &= cameraRect.containsPoint(Vec2(objRect.getMinX(), objRect.getMinY()));
					isAllPlayerInCamera &= cameraRect.containsPoint(Vec2(objRect.getMaxX(), objRect.getMinY()));

					// 全てのプレイヤーオブジェクトが現在のカメラに収まっている
					// または
					// カメラをスクロールすれば収まる場合
					if (isAllPlayerInCamera || (objRect.size.width <= cameraRect.size.width && objRect.size.height <= cameraRect.size.height)) {

						// オブジェクト同士の中点をカメラの位置に設定する
						basePos = Vec2(objRect.getMidX(), objRect.getMidY());
						camera->setTargetPosition(basePos, isChangeCamera, false);

						isScrollActivated = true;

						//CCLOG("全て収まっている or カメラをスクロールすれば収まる: [%f, %f]", basePos.x, basePos.y);
					}
					// プレイヤーオブジェクトをカメラに収めることが出来ない場合
					else {

						// オブジェクト同士の中点をカメラの位置に設定する
						basePos = Vec2(objRect.getMidX(), objRect.getMidY());
						camera->setTargetPosition(basePos, isChangeCamera, false);

						isScrollActivated = false;

						//CCLOG("スクロールでは収められない: [%f, %f]", basePos.x, basePos.y);
					}
				}

				// ----------------------------------------------------------------------------------
				// 「すべてのプレイヤーを収めるよう表示を拡縮する」かつスクロールで収められなかった場合
				if (cameraData->getScaleToShowAllPlayers() && !isScrollActivated) {

					Vec2 scaleVec = camera->getCameraScale()->getValue();

					// オブジェクトを含む矩形の中心をベース位置に設定
					basePos = Vec2(objRect.getMidX(), objRect.getMidY());

					// カメラサイズよりオブジェクトを含む矩形が大きい場合
					auto maxScaling = cameraData->getMaxScaling() * 0.01;
					bool bChangeScale = false;
					float cameraScale = maxScaling > 1.0f ? 1.0f : maxScaling;
					if (objRect.size.width > cameraRect.size.width * cameraScale || objRect.size.height > cameraRect.size.height * cameraScale) {
						auto scaleX = objRect.size.width / cameraRect.size.width;
						auto scaleY = objRect.size.height / cameraRect.size.height;
						float scale = scaleX > scaleY ? scaleX : scaleY;
						scaleVec.x = scale;
						scaleVec.y = scale;
						bChangeScale = true;
					}
#if 1	// ACT2-5210 2体のプレイヤーを離してカメラサイズを広げてからプレイヤーを近づけたとき、カメラサイズが最小カメラサイズまで小さくならないことがあるのを修正。
					else if (scaleVec.x > cameraScale || scaleVec.y > cameraScale) {
						scaleVec.x = cameraScale;
						scaleVec.y = cameraScale;
						bChangeScale = true;
					}
#endif
					if (scaleVec.x > maxScaling && scaleVec.y > maxScaling) {
						scaleVec.x = maxScaling;
						scaleVec.y = maxScaling;
					}

					auto object = getPlayableObject();
					if (object != nullptr) {
						auto camRect = getCameraRect(camera, cameraData, false);
						auto objOneRect = getObjectRect(object);

						if (objRect.size.width > camRect.size.width * scaleVec.x) {
							objRect.size.width = camRect.size.width * scaleVec.x;
							if (objRect.origin.x < objOneRect.origin.x) {
								objRect.origin.x = (objOneRect.origin.x + objOneRect.size.width) - objRect.size.width;
							}
						}
						if (objRect.size.height > camRect.size.height * scaleVec.y) {
							objRect.size.height = camRect.size.height * scaleVec.y;
							if (objRect.origin.y < objOneRect.origin.y) {
								objRect.origin.y = (objOneRect.origin.y + objOneRect.size.height) - objRect.size.height;
							}
						}
					}

					// オブジェクト同士の中点をカメラの位置に設定する
					basePos = Vec2(objRect.getMidX(), objRect.getMidY());

					camera->setZoom(basePos, scaleVec, 0.1f);

					if (cameraData->getFollowTargetType() != agtk::data::OthersCameraData::kFollowTargetNone) {
						camera->setTargetPosition(basePos, isChangeCamera, false);
					}
					else {
						camera->setTargetFixedPosition(basePos, isChangeCamera);
					}

					// 拡縮でカメラに収めたので操作プレイヤーオブジェクトへのフォーカスをOFF
					isNeedForcusToPlayableObj = false;
				}
				//「すべてのプレイヤーを収めるように表示を拡縮する」がFALSEの場合。
				else if (cameraData->getScaleToShowAllPlayers() == false) {
					auto scaleVec = camera->getCameraScale()->getValue();
					auto defScaleVec = cocos2d::Vec2::ONE;
					if (scaleVec != defScaleVec) {
						camera->setZoom(basePos, defScaleVec, 0.1f);
					}
				}
				// ----------------------------------------------------------------------------------
				// 「すべてのプレイヤーを収めるよう表示を拡縮する」がOFFで
				// 操作プレイヤーオブジェクトにフォーカスする場合
				if (isNeedForcusToPlayableObj) {

					// 1P,2P,3P,4Pの順に操作対象オブジェクトを検索
					// プレイ人数取得
					auto gm = GameManager::getInstance();
					auto playerMax = gm->getProjectData()->getPlayerCount();
					auto playData = gm->getPlayData();
					bool isTargetFound = false;

					// 操作可能なオブジェクトリストを取得
					auto playableObjList = this->getObjectAllObjGroup(agtk::data::ObjectData::kObjGroupPlayer, true, SceneLayer::kTypeScene);

					for (int playerId = 1; playerId <= playerMax && !isTargetFound; playerId++) {

						// コントローラー取得
						auto controller = InputManager::getInstance()->getSelectController(playerId);

						// コントローラーがある場合
						if (controller) {

							cocos2d::Ref *ref = nullptr;
							CCARRAY_FOREACH(playableObjList, ref) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
								auto object = static_cast<agtk::Object *>(ref);
#else
								auto object = dynamic_cast<agtk::Object *>(ref);
#endif
								auto objectData = object->getObjectData();

								// 「プレイヤーグループ」かつ「入力デバイスで操作する」の両方がチェックされている場合
								if (objectData->isGroupPlayer() && objectData->getOperatable()) {
									// オブジェクトに割り当てられているコントローラーIDをチェック
									auto playObjectData = object->getPlayObjectData();
									auto attachedControllerId = playObjectData->getVariableData(agtk::data::kObjectSystemVariableControllerID)->getValue();

									// 1Pから順に割り当てられたコントローラーIDと一致する場合
									if (attachedControllerId == controller->getId()) {

										// オブジェクトを中心とするようにスクロール
										auto objPos = object->getPlayer()->getPosition();

										forcusPos = Vec2(objRect.getMidX(), objRect.getMidY());

										// オブジェクトを含む矩形の幅がカメラの矩形の幅より大きい場合
										if (objRect.size.width > cameraRect.size.width) {
											forcusPos.x = objPos.x;
										}
										// オブジェクトを含む矩形の高さがカメラの矩形の高さより大きい場合
										if (objRect.size.height > cameraRect.size.height) {
											forcusPos.y = objPos.y;
										}

										isTargetFound = true;
										break;
									}
								}
							}
						}
					}

					// カメラの位置を設定
					camera->setTargetPosition(forcusPos, isChangeCamera, false);
				}
			}
			else {
				//「すべてのプレイヤーを収めるよう表示を拡縮する」でプレイヤーが１つの場合。
				if (objectList->count() == 1 && cameraData->getScaleToShowAllPlayers()) {
					auto scaleVec = camera->getCameraScale()->getValue();
					auto defScaleVec = cocos2d::Vec2::ONE;
					if (scaleVec != defScaleVec) {
						camera->setZoom(basePos, defScaleVec, 0.1f);
					}
				}
			}
		}
	}

	// カメラ自体の更新を行う
	if (camera != nullptr) {
		camera->update(dt);
	}
}

void Scene::updateVisit(float delta)
{
	//※「オプション：画像表示中はゲームの動作を一時停止」で、画像表示の情報を更新するために必要。
	// image
	ImageManager::getInstance()->update(delta);
	// movie
	MovieManager::getInstance()->update(delta);
	//camera
	this->updateCamera(0.0f);
	// render texture
	this->updateSceneRenderTextureList(delta);
}

bool Scene::checkChangeCamera()
{
	auto projectPlayData = GameManager::getInstance()->getPlayData();

	cocos2d::Ref* ref = nullptr;

	// スイッチの値を更新
	CCARRAY_FOREACH(this->getCameraObjectList(), ref) {

		auto cameraObject = dynamic_cast<agtk::CameraObject*>(ref);

		if (cameraObject != nullptr) {
			auto cameraData = cameraObject->getCameraData();

			int objectId = cameraData->getSwitchObjectId();
			int switchId = cameraData->getSwitchId();
			int switchQualifierId = cameraData->getSwitchQualifierId();

			// 前のフレームのスイッチの値を更新
			cameraObject->setSwitchValueOld(cameraObject->getSwitchValue());

			/*
			CCLOG("------------------------------");
			CCLOG("%d, %d, %d", objectId, switchId, switchQualifierId);
			*/

			// 未設定の場合
			if (objectId < agtk::data::PlayData::COMMON_PLAY_DATA_ID) {
				// 常に有効化
				cameraObject->setSwitchValue(true);
			}
			// プロジェクト共通の場合共通の場合
			else if (objectId == agtk::data::PlayData::COMMON_PLAY_DATA_ID) {
				auto switchData = projectPlayData->getSwitchData(agtk::data::PlayData::COMMON_PLAY_DATA_ID, switchId);
				if (switchData != nullptr) {
					// スイッチの値を設定する
					cameraObject->setSwitchValue(switchData->getValue());
				}
				else {
					//「設定無し」の場合は有効化
					cameraObject->setSwitchValue(true);
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

							if (switchData != nullptr) {
								// スイッチの値を設定する
								cameraObject->setSwitchValue(switchData->getValue());
							}
							else {
								//「設定無し」の場合は有効化
								cameraObject->setSwitchValue(true);
							}
						}
					}
				}
			}
		}
	}

	// スイッチが切り替わった直後のカメラがあるかをチェック
	for (int i = this->getCameraObjectList()->count() - 1; i >= 0; i--) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
		auto cameraObject = static_cast<agtk::CameraObject*>(this->getCameraObjectList()->getObjectAtIndex(i));
#else
		auto cameraObject = dynamic_cast<agtk::CameraObject*>(this->getCameraObjectList()->getObjectAtIndex(i));
#endif

		// スイッチが切り替わったカメラの場合
		if (!cameraObject->getSwitchValueOld() && cameraObject->getSwitchValue()) {
			// カメラを切り替える
			changeCamera(cameraObject);
			return true;
		}
	}

	// 現在のカメラのスイッチがONかをチェックし、ONのままなら何もしない
	if (getCurrentCameraObject() != nullptr && getCurrentCameraObject()->getSwitchValue()) {
		return false;
	}

	// スイッチが切り替わった直後のカメラを発見できなかった場合
	// スイッチがONのカメラを使用するカメラとする
	for (int i = this->getCameraObjectList()->count() - 1; i >= 0; i--) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
		auto cameraObject = static_cast<agtk::CameraObject*>(this->getCameraObjectList()->getObjectAtIndex(i));
#else
		auto cameraObject = dynamic_cast<agtk::CameraObject*>(this->getCameraObjectList()->getObjectAtIndex(i));
#endif

		// フラグがONのカメラの場合
		if (cameraObject->getSwitchValue()) {
			// カメラを切り替える
			changeCamera(cameraObject);
			return true;
		}
	}

	return false;
}

void Scene::procLoop()
{
	cocos2d::DictElement *el = nullptr;
	auto sceneLayerList = this->getSceneLayerList();

	bool fixedCamera = false;

	// カメラの追従対象が「無し」の場合は固定カメラである
	if (getCurrentCameraObject()) {
		fixedCamera = getCurrentCameraObject()->getCameraData()->getFollowTargetType() == agtk::data::OthersCameraData::EnumFollowTargetType::kFollowTargetNone;
	}

	// シーンの上下を繋げる場合
	if (this->getSceneData()->getVerticalLoop()) {
		CCDICT_FOREACH(sceneLayerList, el) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
			auto sceneLayer = static_cast<agtk::SceneLayer*>(el->getObject());
#else
			auto sceneLayer = dynamic_cast<agtk::SceneLayer*>(el->getObject());
#endif
			sceneLayer->loopVertical(fixedCamera);
		}
	}

	// シーンの左右を繋げる場合
	if (this->getSceneData()->getHorizontalLoop()) {
		CCDICT_FOREACH(sceneLayerList, el) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
			auto sceneLayer = static_cast<agtk::SceneLayer*>(el->getObject());
#else
			auto sceneLayer = dynamic_cast<agtk::SceneLayer*>(el->getObject());
#endif
			sceneLayer->loopHorizontal(fixedCamera);
		}
	}
}

void Scene::createReappearObject()
{
	//layer
	cocos2d::DictElement *el = nullptr;
	auto sceneLayerList = this->getSceneLayerList();
	CCDICT_FOREACH(sceneLayerList, el) {
		auto sceneLayer = dynamic_cast<agtk::SceneLayer *>(el->getObject());
		if (sceneLayer) {
			sceneLayer->createReappearObject();
		}
	}

	//menu
	auto menuLayerList = this->getMenuLayerList();
	CCDICT_FOREACH(menuLayerList, el) {
		auto menuLayer = dynamic_cast<agtk::SceneLayer *>(el->getObject());
		if (menuLayer) {
			menuLayer->createReappearObject();
		}
	}
}

void Scene::setShader(int layerId, Shader::ShaderKind kind, float value, float seconds)
{
	//0: bg
	if (layerId == agtk::data::SceneData::kBackgroundLayerId) {
		if (this->getSceneBackground()) {
			this->getSceneBackground()->setShader(kind, value, seconds);
		}
	}
	// 999998: topMost
	else if (layerId == agtk::data::SceneData::kTopMostLayerId) {
		if (this->getSceneTopMost()) {
			this->getSceneTopMost()->setShader(kind, value, seconds);
		}
	}
	// 999999: topMost(メニュー含む)
	else if (layerId == agtk::data::SceneData::kTopMostWithMenuLayerId) {
		if (this->getSceneTopMost()) {
			this->getSceneTopMost()->setWithMenuShader(kind, value, seconds);
		}
	}
	//1～n: scenelayer
	else {
		auto sceneLayer = this->getSceneLayer(layerId);
		if (sceneLayer) {
			if (kind == agtk::Shader::kShaderColorDarkMask) {
				auto shader = sceneLayer->getShader(kind);
				if (shader == nullptr) {
					auto renderTextureCtrl = sceneLayer->getRenderTexture();
					shader = renderTextureCtrl->addShader(kind, value, seconds);
					auto texture2d = new cocos2d::Texture2D();
					auto sceneSize = this->getContentSize();
					auto sceneWidth = 8;
					auto sceneHeight = 8;
					auto buf = new unsigned char[sceneWidth * sceneHeight * 4];
					memset(buf, 0, sceneWidth * sceneHeight * 4);
					texture2d->initWithData(buf, sceneWidth * sceneHeight * 4, Texture2D::PixelFormat::RGBA8888, sceneWidth, sceneHeight, cocos2d::Size(sceneWidth, sceneHeight));
					delete[] buf;
					shader->setMaskTexture(texture2d);
				}
				else {
					sceneLayer->setShader(kind, value, seconds);
				}
			}
			else {
				sceneLayer->setShader(kind, value, seconds);
			}
		}
	}
}

void Scene::removeShader(int layerId, Shader::ShaderKind kind, float seconds)
{
	//0: bg
	if (layerId == agtk::data::SceneData::kBackgroundLayerId) {
		if (this->getSceneBackground()) {
			this->getSceneBackground()->removeShader(kind, seconds);
		}
	}
	// 999998: topMost
	else if (layerId == agtk::data::SceneData::kTopMostLayerId) {
		if (this->getSceneTopMost()) {
			this->getSceneTopMost()->removeShader(kind, seconds);
		}
	}
	// 999999: topMost(メニュー含む)
	else if (layerId == agtk::data::SceneData::kTopMostWithMenuLayerId) {
		if (this->getSceneTopMost()) {
			this->getSceneTopMost()->removeWithMenuShader(kind, seconds);
		}
	}
	//1～n: sceneLayer
	else {
		auto sceneLayer = this->getSceneLayer(layerId);
		if (sceneLayer) {
			sceneLayer->removeShader(kind, seconds);
		}
	}
}

void Scene::pauseShader()
{
	if (this->getSceneBackground()) {
		auto renderTextureCtrl = this->getSceneBackground()->getRenderTexture();
		if (renderTextureCtrl) {
			renderTextureCtrl->pauseShader();
		}
	}
	auto sceneLayerList = this->getSceneLayerList();
	cocos2d::DictElement *el;
	CCDICT_FOREACH(sceneLayerList, el) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
		auto sceneLayer = static_cast<agtk::SceneLayer *>(el->getObject());
#else
		auto sceneLayer = dynamic_cast<agtk::SceneLayer *>(el->getObject());
#endif
		auto rtCtrl = sceneLayer->getRenderTexture();
		if (rtCtrl) {
			rtCtrl->pauseShader();
		}
	}
	if (this->getSceneTopMost()) {
		auto renderTextureCtrl = this->getSceneTopMost()->getRenderTexture();
		if (renderTextureCtrl) {
			renderTextureCtrl->pauseShader();
		}
		auto withMenuRenderTextureCtrl = this->getSceneTopMost()->getWithMenuRenderTexture();
		if (withMenuRenderTextureCtrl) {
			withMenuRenderTextureCtrl->pauseShader();
		}
	}
}

void Scene::resumeShader()
{
	if (this->getSceneBackground()) {
		auto renderTextureCtrl = this->getSceneBackground()->getRenderTexture();
		if (renderTextureCtrl) {
			renderTextureCtrl->resumeShader();
		}
	}
	auto sceneLayerList = this->getSceneLayerList();
	cocos2d::DictElement *el;
	CCDICT_FOREACH(sceneLayerList, el) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
		auto sceneLayer = static_cast<agtk::SceneLayer *>(el->getObject());
#else
		auto sceneLayer = dynamic_cast<agtk::SceneLayer *>(el->getObject());
#endif
		auto rtCtrl = sceneLayer->getRenderTexture();
		if (rtCtrl) {
			rtCtrl->resumeShader();
		}
	}
	if (this->getSceneTopMost()) {
		auto renderTextureCtrl = this->getSceneTopMost()->getRenderTexture();
		if (renderTextureCtrl) {
			renderTextureCtrl->resumeShader();
		}
		auto withMenuRenderTextureCtrl = this->getSceneTopMost()->getWithMenuRenderTexture();
		if (withMenuRenderTextureCtrl) {
			withMenuRenderTextureCtrl->resumeShader();
		}
	}
}

agtk::RenderTextureCtrl *Scene::getRenderTextureCtrl(int layerId)
{
	CC_ASSERT(_renderTextureCtrlList);
	return dynamic_cast<agtk::RenderTextureCtrl *>(this->getRenderTextureCtrlList()->objectForKey(layerId));
}

agtk::SceneLayer *Scene::getSceneLayer(int layerId)
{
	if (!this->getSceneLayerList()) {
		return nullptr;
	}
// #AGTK-NX
#if defined(STATIC_DOWN_CAST) && defined(FORCE_STATIC_DOWN_CAST)
	return static_cast<agtk::SceneLayer *>(this->getSceneLayerList()->objectForKey(layerId));
#else
	return dynamic_cast<agtk::SceneLayer *>(this->getSceneLayerList()->objectForKey(layerId));
#endif
}

agtk::SceneLayer *Scene::getSceneLayerFront()
{
	return this->getSceneLayer(1);
}

agtk::SceneLayer *Scene::getMenuLayer(int layerId)
{
	auto menuLayerList = this->getMenuLayerList();
	cocos2d::DictElement *el;
	CCDICT_FOREACH(menuLayerList, el) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
		auto menuLayer = static_cast<agtk::SceneLayer *>(el->getObject());
#else
		auto menuLayer = dynamic_cast<agtk::SceneLayer *>(el->getObject());
#endif
		if (menuLayer->getLayerId() == layerId) {
			return menuLayer;
		}
	}
	return nullptr;
}

// #AGTK-NX
#ifdef USE_SAR_OPTIMIZE_1
cocos2d::__Array *Scene::getObjectAllReference(SceneLayer::EnumType sceneLayerType)
{
	bool isUpdated = false;
	cocos2d::__Dictionary* layerList = nullptr;
	if (sceneLayerType == SceneLayer::kTypeMax)
	{
		layerList = this->getSceneLayerList();
		cocos2d::DictElement *el = nullptr;
		CCDICT_FOREACH(layerList, el) {
			auto sceneLayer = static_cast<agtk::SceneLayer *>(el->getObject());
			if (sceneLayer->getIsObjectListUpdated())
			{
				isUpdated = true;
				break;
			}
		}
		if (!isUpdated)
		{
			layerList = this->getMenuLayerList();
			cocos2d::DictElement *el = nullptr;
			CCDICT_FOREACH(layerList, el) {
				auto sceneLayer = static_cast<agtk::SceneLayer *>(el->getObject());
				if (sceneLayer->getIsObjectListUpdated())
				{
					isUpdated = true;
					break;
				}
			}
		}
	}
	else
	{
		if (sceneLayerType == SceneLayer::kTypeScene)
		{
			layerList = this->getSceneLayerList();
			cocos2d::DictElement *el = nullptr;
			CCDICT_FOREACH(layerList, el) {
				auto sceneLayer = static_cast<agtk::SceneLayer *>(el->getObject());
				if (sceneLayer->getIsObjectListUpdated())
				{
					isUpdated = true;
					sceneLayer->setIsObjectListUpdated(false);
				}
			}
		}
		if (sceneLayerType == SceneLayer::kTypeMenu)
		{
			layerList = this->getMenuLayerList();
			cocos2d::DictElement *el = nullptr;
			CCDICT_FOREACH(layerList, el) {
				auto sceneLayer = static_cast<agtk::SceneLayer *>(el->getObject());
				if (sceneLayer->getIsObjectListUpdated())
				{
					isUpdated = true;
					sceneLayer->setIsObjectListUpdated(false);
				}
			}
		}
	}

	if (!isUpdated)
	{
		switch (sceneLayerType)
		{
			case SceneLayer::kTypeScene:
				if (this->getSceneObjectListCache())
				{
					return this->getSceneObjectListCache();
				}
				break;
			case SceneLayer::kTypeMenu:
				if (this->getMenuObjectListCache())
				{
					return this->getMenuObjectListCache();
				}
				break;
			case SceneLayer::kTypeMax:
				if (this->getAllObjectListCache())
				{
					return this->getAllObjectListCache();
				}
				break;
		}
	}

	if (sceneLayerType == SceneLayer::kTypeMax) {
		auto allArr = cocos2d::__Array::create();
		allArr->addObjectsFromArray(getObjectAllReference(SceneLayer::kTypeScene));
		allArr->addObjectsFromArray(getObjectAllReference(SceneLayer::kTypeMenu));
		setAllObjectListCache(allArr);
		return allArr;
	}

	auto arr = cocos2d::__Array::create();
	cocos2d::DictElement *el = nullptr;
	layerList = (sceneLayerType == SceneLayer::kTypeMenu) ? this->getMenuLayerList() : this->getSceneLayerList();
	CCDICT_FOREACH(layerList, el) {
		auto sceneLayer = static_cast<agtk::SceneLayer *>(el->getObject());
		if (sceneLayer->getObjectList() == nullptr) {
			continue;
		}
		if (sceneLayerType == SceneLayer::kTypeMenu && sceneLayer->isDisplay() == false) {
			continue;
		}
		arr->addObjectsFromArray(sceneLayer->getObjectList());
	}
	if (sceneLayerType == SceneLayer::kTypeScene)
	{
		setSceneObjectListCache(arr);
	}
	else {
		setMenuObjectListCache(arr);
	}

	return arr;
}
#endif

cocos2d::__Array *Scene::getObjectAll(SceneLayer::EnumType sceneLayerType)
{
// #AGTK-NX
#ifdef USE_SAR_OPTIMIZE_1
	auto arr = cocos2d::__Array::create();
	arr->addObjectsFromArray(this->getObjectAllReference(sceneLayerType));
	return arr;
#else
	if (sceneLayerType == SceneLayer::kTypeMax) {
		auto arr = this->getObjectAll(SceneLayer::kTypeScene);
		arr->addObjectsFromArray(this->getObjectAll(SceneLayer::kTypeMenu));
		return arr;
	}
	auto arr = cocos2d::__Array::create();
	cocos2d::DictElement *el = nullptr;
	auto sceneLayerList = (sceneLayerType == SceneLayer::kTypeMenu) ? this->getMenuLayerList() : this->getSceneLayerList();
	CCDICT_FOREACH(sceneLayerList, el) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
		auto sceneLayer = static_cast<agtk::SceneLayer *>(el->getObject());
#else
		auto sceneLayer = dynamic_cast<agtk::SceneLayer *>(el->getObject());
#endif
		if (sceneLayer->getObjectList() == nullptr) {
			continue;
		}
		if (sceneLayerType == SceneLayer::kTypeMenu && sceneLayer->isDisplay() == false) {
			continue;
		}
		arr->addObjectsFromArray(sceneLayer->getObjectList());
	}
	return arr;
#endif
}
#ifdef USE_MULTITHREAD_OBJECT_UPDATE
agtk::NrArray *Scene::getObjectAllNrArray(SceneLayer::EnumType sceneLayerType)
{
	auto arr = agtk::NrArray::create();
	if (sceneLayerType == SceneLayer::kTypeMax) {
		arr->addObjectsFromArray(this->getObjectAll(SceneLayer::kTypeScene));
		arr->addObjectsFromArray(this->getObjectAll(SceneLayer::kTypeMenu));
		return arr;
	}
	cocos2d::DictElement *el = nullptr;
	auto sceneLayerList = (sceneLayerType == SceneLayer::kTypeMenu) ? this->getMenuLayerList() : this->getSceneLayerList();
	CCDICT_FOREACH(sceneLayerList, el) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
		auto sceneLayer = static_cast<agtk::SceneLayer *>(el->getObject());
#else
		auto sceneLayer = dynamic_cast<agtk::SceneLayer *>(el->getObject());
#endif
		if (sceneLayer->getObjectList() == nullptr) {
			continue;
		}
		if (sceneLayerType == SceneLayer::kTypeMenu && sceneLayer->isDisplay() == false) {
			continue;
		}
		arr->addObjectsFromArray(sceneLayer->getObjectList());
	}
	return arr;
}
#endif

cocos2d::__Array *Scene::getObjectAll(int objectId, SceneLayer::EnumType sceneLayerType, int SceneLayerId)
{
	if (sceneLayerType == SceneLayer::kTypeMax) {
		auto arr = this->getObjectAll(objectId, SceneLayer::kTypeScene);
		arr->addObjectsFromArray(this->getObjectAll(objectId, SceneLayer::kTypeMenu));
		return arr;
	}
	auto arr = cocos2d::__Array::create();
	cocos2d::DictElement *el = nullptr;
	auto sceneLayerList = (sceneLayerType == SceneLayer::kTypeMenu) ? this->getMenuLayerList() : this->getSceneLayerList();
	CCDICT_FOREACH(sceneLayerList, el) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
		auto sceneLayer = static_cast<agtk::SceneLayer *>(el->getObject());
#else
		auto sceneLayer = dynamic_cast<agtk::SceneLayer *>(el->getObject());
#endif
		if (SceneLayerId != -1 && sceneLayer->getLayerId() != SceneLayerId) {
			continue;
		}
		if (sceneLayer->getObjectList() == nullptr) {
			continue;
		}
#ifdef USE_SAR_OPTIMIZE_2
		arr->addObjectsFromArray(sceneLayer->getObjectAll(objectId));
#else
		auto objectList = sceneLayer->getObjectList();
		cocos2d::Ref *ref = nullptr;
		CCARRAY_FOREACH(objectList, ref) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
			auto object = static_cast<agtk::Object *>(ref);
#else
			auto object = dynamic_cast<agtk::Object *>(ref);
#endif
			auto objectData = object->getObjectData();
			if (objectData->getId() == objectId) {
				arr->addObject(object);
			}
		}
#endif
	}
	return arr;
}

// #AGTK-NX
#ifdef USE_SAR_OPTIMIZE_3
agtk::Object *Scene::getObjectInstance(int objectId, int instanceId, SceneLayer::EnumType sceneLayerType, int sceneLayerId)
#else
agtk::Object *Scene::getObjectInstance(int objectId, int instanceId, SceneLayer::EnumType sceneLayerType)
#endif
{
	if (sceneLayerType == SceneLayer::kTypeMax) {
		auto object = this->getObjectInstance(objectId, instanceId, SceneLayer::kTypeScene);
		if (object == nullptr) {
			object = this->getObjectInstance(objectId, instanceId, SceneLayer::kTypeMenu);
		}
		return object;
	}
	cocos2d::DictElement *el = nullptr;
	auto sceneLayerList = (sceneLayerType == SceneLayer::kTypeMenu) ? this->getMenuLayerList() : this->getSceneLayerList();
	CCDICT_FOREACH(sceneLayerList, el) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
		auto sceneLayer = static_cast<agtk::SceneLayer *>(el->getObject());
#else
		auto sceneLayer = dynamic_cast<agtk::SceneLayer *>(el->getObject());
#endif
// #AGTK-NX
#ifdef USE_SAR_OPTIMIZE_3
		if (sceneLayerId != -1 && sceneLayer->getLayerId() != sceneLayerId) {
			continue;
		}
#endif
		auto objectList = sceneLayer->getObjectList();
		if (objectList->count() > 0) {
#ifdef USE_SAR_OPTIMIZE_2
			auto object = sceneLayer->getObjectAll(objectId, instanceId);
			if (object != nullptr)
			{
				return object;
			}
#else
			cocos2d::Ref *ref = nullptr;
			CCARRAY_FOREACH(objectList, ref) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
				auto object = static_cast<agtk::Object *>(ref);
#else
				auto object = dynamic_cast<agtk::Object *>(ref);
#endif
				auto objectData = object->getObjectData();
				if ((objectId < 0 || objectData->getId() == objectId) && (instanceId < 0 || object->getInstanceId() == instanceId)) {
					return object;
				}
			}
#endif
		}
	}
	return nullptr;
}

agtk::Object *Scene::getObjectInstanceByName(int objectId, const char *name)
{
	auto arr = cocos2d::__Array::create();
	cocos2d::DictElement *el = nullptr;
	auto sceneLayerList = this->getSceneLayerList();
	CCDICT_FOREACH(sceneLayerList, el) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
		auto sceneLayer = static_cast<agtk::SceneLayer *>(el->getObject());
#else
		auto sceneLayer = dynamic_cast<agtk::SceneLayer *>(el->getObject());
#endif
		if (sceneLayer->getObjectList() == nullptr) {
			continue;
		}
		auto objectList = sceneLayer->getObjectList();
		cocos2d::Ref *ref = nullptr;
		CCARRAY_FOREACH(objectList, ref) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
			auto object = static_cast<agtk::Object *>(ref);
#else
			auto object = dynamic_cast<agtk::Object *>(ref);
#endif
			if (object->getScenePartObjectData()) {
				auto objectData = object->getObjectData();
				if (objectId >= 0 && objectData->getId() != objectId) {
					continue;
				}
				if (strcmp(object->getScenePartObjectData()->getName(), name) == 0) {
					return object;
				}
			}
		}
		CCARRAY_FOREACH(objectList, ref) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
			auto object = static_cast<agtk::Object *>(ref);
#else
			auto object = dynamic_cast<agtk::Object *>(ref);
#endif
			if (!object->getScenePartObjectData()) {
				auto objectData = object->getObjectData();
				if (objectId >= 0 && objectData->getId() != objectId) {
					continue;
				}
				if (strcmp(objectData->getName(), name) == 0) {
					return object;
				}
			}
		}
	}
	return nullptr;
}

std::vector<cocos2d::Node *> Scene::getPhysicObjectAll(int isAffectOtherObjects, int isAffectedByOtherObjects, int isFollowConnectedPhysics, SceneLayer::EnumType sceneLayerType)
{
	if (sceneLayerType == SceneLayer::kTypeMax) {
		auto arr = this->getPhysicObjectAll(isAffectOtherObjects, isAffectedByOtherObjects, isFollowConnectedPhysics, SceneLayer::kTypeScene);
		auto arrMenu = this->getPhysicObjectAll(isAffectOtherObjects, isAffectedByOtherObjects, isFollowConnectedPhysics, SceneLayer::kTypeMenu);
		arr.insert(arr.end(), arrMenu.begin(), arrMenu.end());
		return arr;
	}
	std::vector<cocos2d::Node *> arr;
	cocos2d::DictElement *el = nullptr;
	auto sceneLayerList = (sceneLayerType == SceneLayer::kTypeMenu) ? this->getMenuLayerList() : this->getSceneLayerList();
	CCDICT_FOREACH(sceneLayerList, el) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
		auto sceneLayer = static_cast<agtk::SceneLayer *>(el->getObject());
#else
		auto sceneLayer = dynamic_cast<agtk::SceneLayer *>(el->getObject());
#endif
		auto objectList = sceneLayer->getObjectList();
		if (objectList->count() > 0) {
			cocos2d::Ref *ref = nullptr;
			CCARRAY_FOREACH(objectList, ref) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
				auto object = static_cast<agtk::Object *>(ref);
#else
				auto object = dynamic_cast<agtk::Object *>(ref);
#endif
				if (object->getphysicsNode() == nullptr) continue;

				auto playObjectData = object->getPlayObjectData();
				auto _isAffect = playObjectData->getSwitchData(agtk::data::kObjectSystemSwitchAffectOtherObjects)->getValue();
				auto _isAffected = playObjectData->getSwitchData(agtk::data::kObjectSystemSwitchAffectedByOtherObjects)->getValue();
				auto _isFollowConnectedPhysics = playObjectData->getSwitchData(agtk::data::kObjectSystemSwitchFollowConnectedPhysics)->getValue();
				bool bAffect = (isAffectOtherObjects == -1 || _isAffect == (bool)isAffectOtherObjects);
				bool bAffected = (isAffectedByOtherObjects == -1 || _isAffected == (bool)isAffectedByOtherObjects);
				bool bFollowConnectedPhysics = (isFollowConnectedPhysics == -1 || _isFollowConnectedPhysics == (bool)isFollowConnectedPhysics);
				if (bAffect && bAffected && bFollowConnectedPhysics) {
					arr.push_back(object);
				}
			}
		}
	}
	return arr;
}


int Scene::getTopPrioritySceneLayer()
{
	int zOrder = -1;

	//scene layer
	auto sceneLayerList = this->getSceneLayerList();
	cocos2d::DictElement *el;
	CCDICT_FOREACH(sceneLayerList, el) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
		auto sceneLayer = static_cast<agtk::SceneLayer *>(el->getObject());
#else
		auto sceneLayer = dynamic_cast<agtk::SceneLayer *>(el->getObject());
#endif
		auto rtCtrl = sceneLayer->getRenderTexture();
		if (rtCtrl) {
			int z = rtCtrl->getFirstRenderTexture()->getLocalZOrder();
			if (zOrder < z) {
				zOrder = z;
			}
		}
		else {
			int z = sceneLayer->getLocalZOrder();
			if (zOrder < z) {
				zOrder = z;
			}
		}
	}
	//menu layer
	auto menuLayerList = this->getMenuLayerList();
	CCDICT_FOREACH(menuLayerList, el) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
		auto menuSceneLayer = static_cast<agtk::SceneLayer *> (el->getObject());
#else
		auto menuSceneLayer = dynamic_cast<agtk::SceneLayer *> (el->getObject());
#endif
		int layerId = menuSceneLayer->getLayerId();
		if (layerId == agtk::data::SceneData::kHudMenuLayerId || layerId == agtk::data::SceneData::kHudTopMostLayerId) {
			continue;
		}
		auto rtCtrl = menuSceneLayer->getRenderTexture();
		if (rtCtrl) {
			int z = rtCtrl->getFirstRenderTexture()->getLocalZOrder();
			if (zOrder < z) {
				zOrder = z;
			}
		}
		else {
			int z = menuSceneLayer->getLocalZOrder();
			if (zOrder < z) {
				zOrder = z;
			}
		}
	}
	return zOrder;
}

#ifdef USE_PREVIEW
//シーンに直接配置されている物理演算パーツの中でscenePartIdに一致するもののリストを返す。
cocos2d::__Array *Scene::getScenePhysicsObject(int scenePartId)
{
	auto arr = cocos2d::__Array::create();
	cocos2d::DictElement *el = nullptr;
	auto sceneLayerList = this->getSceneLayerList();
	CCDICT_FOREACH(sceneLayerList, el) {
		auto sceneLayer = dynamic_cast<agtk::SceneLayer *>(el->getObject());
		if (sceneLayer->getPhysicsObjectList() == nullptr) {
			continue;
		}
		auto physicsObjectList = sceneLayer->getPhysicsObjectList();
		cocos2d::Ref *ref = nullptr;
		CCARRAY_FOREACH(physicsObjectList, ref) {
			auto physicsObject = dynamic_cast<agtk::PhysicsBase *>(ref);
			if (physicsObject->getParentScenePartId() != -1) {
				//オブジェクトに設定されている物理演算パーツ。
				continue;
			}
			if (physicsObject->getScenePartsId() == scenePartId) {
				arr->addObject(physicsObject);
			}
		}
	}
	return arr;
}
#endif

cocos2d::__Array *Scene::getObjectAll(cocos2d::Rect rect, SceneLayer::EnumType sceneLayerType, int SceneLayerId)
{
	if (sceneLayerType == SceneLayer::kTypeMax) {
		auto arr = this->getObjectAll(rect, SceneLayer::kTypeScene);
		arr->addObjectsFromArray(this->getObjectAll(rect, SceneLayer::kTypeMenu));
		return arr;
	}
	auto arr = cocos2d::__Array::create();
	cocos2d::DictElement *el = nullptr;
	auto sceneLayerList = (sceneLayerType == SceneLayer::kTypeMenu) ? this->getMenuLayerList() : this->getSceneLayerList();
	CCDICT_FOREACH(sceneLayerList, el) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
		auto sceneLayer = static_cast<agtk::SceneLayer *>(el->getObject());
#else
		auto sceneLayer = dynamic_cast<agtk::SceneLayer *>(el->getObject());
#endif
		if (SceneLayerId != -1 && sceneLayer->getLayerId() != SceneLayerId) {
			continue;
		}
		if (sceneLayer->getObjectList() == nullptr) {
			continue;
		}
		auto objectList = sceneLayer->getObjectList();
		cocos2d::Ref *ref = nullptr;
		CCARRAY_FOREACH(objectList, ref) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
			auto object = static_cast<agtk::Object *>(ref);
#else
			auto object = dynamic_cast<agtk::Object *>(ref);
#endif
			auto collisionList = object->getAreaArray(agtk::data::TimelineInfoData::kTimelineWall);
			cocos2d::Ref *ref2 = nullptr;
			CCARRAY_FOREACH(collisionList, ref2) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
				auto node = static_cast<cocos2d::Node *>(ref2);
#else
				auto node = dynamic_cast<cocos2d::Node *>(ref2);
#endif
				bool bCheck = rect.intersectsRect(node->getBoundingBox());
				if (bCheck) {
					arr->addObject(object);
				}
			}
		}
	}
	return arr;
}

cocos2d::__Array *Scene::getObjectAllObjGroup(agtk::data::ObjectData::EnumObjGroup group, bool isOperatable/* = false*/, SceneLayer::EnumType sceneLayerType /* = SceneLayer::kTypeScene */)
{
	if (sceneLayerType == SceneLayer::kTypeMax) {
		auto objectList = this->getObjectAllObjGroup(group, isOperatable, SceneLayer::kTypeScene);
		auto menuObjectList = this->getObjectAllObjGroup(group, isOperatable, SceneLayer::kTypeMenu);
		cocos2d::Ref *ref;
		CCARRAY_FOREACH(menuObjectList, ref) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
			auto obj = static_cast<agtk::Object *>(ref);
#else
			auto obj = dynamic_cast<agtk::Object *>(ref);
#endif
			if (objectList->containsObject(obj)) continue;
			objectList->addObject(obj);
		}
		return objectList;
	}
	auto arr = cocos2d::__Array::create();
	cocos2d::DictElement *el = nullptr;
	auto sceneLayerList = (sceneLayerType == SceneLayer::kTypeMenu) ? this->getMenuLayerList() : this->getSceneLayerList();
	CCDICT_FOREACH(sceneLayerList, el) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
		auto sceneLayer = static_cast<agtk::SceneLayer *>(el->getObject());
#else
		auto sceneLayer = dynamic_cast<agtk::SceneLayer *>(el->getObject());
#endif
		if (sceneLayer->getObjectList() == nullptr) {
			continue;
		}
		auto objectList = sceneLayer->getObjectList();
		cocos2d::Ref *ref = nullptr;
		CCARRAY_FOREACH(objectList, ref) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
			auto object = static_cast<agtk::Object *>(ref);
#else
			auto object = dynamic_cast<agtk::Object *>(ref);
#endif
			auto objectData = object->getObjectData();
			if (arr->containsObject(object)) continue;
			if(objectData->getGroup() == group && (!isOperatable || (isOperatable && objectData->getOperatable()))){
				arr->addObject(object);
			}
		}
	}
	return arr;
}

cocos2d::__Array *Scene::getObjectAllLocked(int objectInstanceId, SceneLayer::EnumType sceneLayerType)
{
	if (sceneLayerType == SceneLayer::kTypeMax) {
		auto arr = this->getObjectAllLocked(objectInstanceId, SceneLayer::kTypeScene);
		arr->addObjectsFromArray(this->getObjectAllLocked(objectInstanceId, SceneLayer::kTypeMenu));
		return arr;
	}
	auto arr = cocos2d::__Array::create();
	cocos2d::DictElement *el = nullptr;
	auto sceneLayerList = (sceneLayerType == SceneLayer::kTypeMenu) ? this->getMenuLayerList() : this->getSceneLayerList();
	CCDICT_FOREACH(sceneLayerList, el) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
		auto sceneLayer = static_cast<agtk::SceneLayer *>(el->getObject());
#else
		auto sceneLayer = dynamic_cast<agtk::SceneLayer *>(el->getObject());
#endif
		if (sceneLayer->getObjectList() == nullptr) {
			continue;
		}
		auto objectList = sceneLayer->getObjectList();
		cocos2d::Ref *ref = nullptr;
		CCARRAY_FOREACH(objectList, ref) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
			auto object = static_cast<agtk::Object *>(ref);
#else
			auto object = dynamic_cast<agtk::Object *>(ref);
#endif
			auto playObjectData = object->getPlayObjectData();
			if (playObjectData->isLocked(objectInstanceId)) {
				arr->addObject(object);
			}
		}
	}
	return arr;
}

cocos2d::RefPtr<cocos2d::__Array> Scene::getObjectAllFront(SceneLayer::EnumType sceneLayerType, int layerId)
{
	cocos2d::RefPtr<cocos2d::__Array> arr = cocos2d::__Array::create();
	if (sceneLayerType == SceneLayer::kTypeMax) 
	{
		for (int i = 0; i < SceneLayer::kTypeMax; ++i ) 
		{
			arr->addObjectsFromArray(this->getObjectAllFront(static_cast<SceneLayer::EnumType>(i), layerId));
		}
	}
	else
	{
		cocos2d::DictElement *el = nullptr;
		auto sceneLayerList = (sceneLayerType == SceneLayer::kTypeMenu) ? this->getMenuLayerList() : this->getSceneLayerList();
		CCDICT_FOREACH(sceneLayerList, el) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
			auto sceneLayer = static_cast<agtk::SceneLayer *>(el->getObject());
#else
			auto sceneLayer = dynamic_cast<agtk::SceneLayer *>(el->getObject());
#endif
			if (sceneLayer->getObjectList() == nullptr) {
				continue;
			}
			if (sceneLayerType == SceneLayer::kTypeMenu && sceneLayer->isDisplay() == false) {
				continue;
			}
			if (sceneLayer->getLayerId() >= layerId) {
				continue;
			}

			arr->addObjectsFromArray(sceneLayer->getObjectList());
		}
	}
	return arr;
}

agtk::BaseLayer *Scene::getBaseLayer()
{
// #AGTK-NX
#if defined(STATIC_DOWN_CAST) && defined(FORCE_STATIC_DOWN_CAST)
	auto baseLayer = static_cast<agtk::BaseLayer *>(this->getLayer());
#else
	auto baseLayer = dynamic_cast<agtk::BaseLayer *>(this->getLayer());
#endif
	CC_ASSERT(baseLayer);
	return baseLayer;
}

int Scene::getObjectInstanceId(int objectId, bool bMenuScene)
{
	auto projectPlayData = GameManager::getInstance()->getPlayData();
	auto variableData = projectPlayData->getVariableData(objectId, agtk::data::kObjectSystemVariableSingleInstanceID);
	int instanceId = (bMenuScene) ? ++_objectInstanceMenuId : ++_objectInstanceId;
	if (variableData->getValue() == 0.0f) {
		variableData->setValue((double)instanceId);
	}
	return instanceId;
}

int Scene::getObjectInstanceId(agtk::Object *object)
{
	return this->getObjectInstanceId(object->getObjectData()->getId(), object->getSceneData()->isMenuScene());
}

int Scene::getObjectInstanceId(bool bSceneMenu)
{
	return bSceneMenu ? _objectInstanceMenuId : _objectInstanceId;
}

/**
* 強制オブジェクトインスタンスIDカウンタ更新
* @param	instanceId	新しいインスタンスIDカウンタ
*/
void Scene::forceUpdateObjectInstanceId(int instanceId, bool bMenuScene)
{
	if (bMenuScene) {
		if (_objectInstanceMenuId < instanceId) {
			_objectInstanceMenuId = instanceId;
		}
		return;
	}
	if (_objectInstanceId < instanceId) {
		_objectInstanceId = instanceId;
	}
}

int Scene::getObjectInstanceCount(int objectId)
{
	auto projectPlayData = GameManager::getInstance()->getPlayData();
	auto instanceCountVariableData = projectPlayData->getVariableData(objectId, agtk::data::kObjectSystemVariableInstanceCount);
	return (int)instanceCountVariableData->getValue();
}

int Scene::setObjectInstanceCount(int objectId, int count)
{
	auto projectPlayData = GameManager::getInstance()->getPlayData();
	return projectPlayData->getObjectData(objectId)->setInstanceCount(count);
}

int Scene::incrementObjectInstanceCount(int objectId)
{
	int count = this->getObjectInstanceCount(objectId);
	this->setObjectInstanceCount(objectId, count + 1);
	return this->getObjectInstanceCount(objectId);
}

int Scene::decrementObjectInstanceCount(int objectId)
{
	int count = this->getObjectInstanceCount(objectId);
	this->setObjectInstanceCount(objectId, count - 1);
	return this->getObjectInstanceCount(objectId);
}

void Scene::updateObjectInstanceCount(int objectId)
{
	int count = this->getObjectInstanceCount(objectId);
	auto objectList = this->getObjectAll(objectId, SceneLayer::kTypeScene);
	cocos2d::Ref *ref;
	CCARRAY_FOREACH(objectList, ref) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
		auto object = static_cast<agtk::Object*>(ref);
#else
		auto object = dynamic_cast<agtk::Object*>(ref);
#endif
		auto playObjectData = object->getPlayObjectData();
		playObjectData->setInstanceCount(count);
	}
}

void Scene::setTakeOverMenuObject(cocos2d::__Array *menuObjectList)
{
// #AGTK-NX
#ifdef USE_SAR_OPTIMIZE_1
	auto newMenuObjectList = this->getObjectAllReference(SceneLayer::kTypeMenu);
#else
	auto newMenuObjectList = this->getObjectAll(SceneLayer::kTypeMenu);
#endif
	cocos2d::Ref *ref;
	CCARRAY_FOREACH(menuObjectList, ref) {
		cocos2d::Ref *ref2;
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
		auto object = static_cast<agtk::Object *>(ref);
#else
		auto object = dynamic_cast<agtk::Object *>(ref);
#endif
		auto objectData = object->getObjectData();
		auto playObjectData = object->getPlayObjectData();
		auto scenePartsId = object->getScenePartsId();
		CCARRAY_FOREACH(newMenuObjectList, ref2) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
			auto newObject = static_cast<agtk::Object *>(ref2);
#else
			auto newObject = dynamic_cast<agtk::Object *>(ref2);
#endif
			auto newObjectData = newObject->getObjectData();
			auto newPlayObjectData = newObject->getPlayObjectData();
			auto newScenePartsId = newObject->getScenePartsId();
			if (objectData->getId() == newObjectData->getId() && scenePartsId == newScenePartsId && object->getLayerId() == newObject->getLayerId()) {
				// 変数とスイッチを引き継ぎ
				newPlayObjectData->takeOverVariableList(playObjectData);
				newPlayObjectData->takeOverSwitchList(playObjectData);
				break;
			}
		}
	}
}

void Scene::setScale(float scaleX, float scaleY)
{
	auto sceneLayerList = this->getSceneLayerList();
	cocos2d::DictElement *el;
	CCDICT_FOREACH(sceneLayerList, el) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
		auto sceneLayer = static_cast<agtk::SceneLayer *>(el->getObject());
#else
		auto sceneLayer = dynamic_cast<agtk::SceneLayer *>(el->getObject());
#endif
		sceneLayer->setScale(scaleX, scaleY);
	}
	auto sceneBackground = this->getSceneBackground();
	if (sceneBackground) {
		sceneBackground->setScale(scaleX, scaleY);
	}
	auto sceneTopMost = this->getSceneTopMost();
	if (sceneTopMost) {
		sceneTopMost->setScale(scaleX, scaleY);
	}
	Node::setScale(scaleX, scaleY);
}

void Scene::startVariableTimer(agtk::data::PlayVariableData *data, bool bCountUp, double seconds, agtk::Object *object)
{
	agtk::SceneVariableTimer *variableTimer = nullptr;
	auto variableTimerList = this->getVariableTimerList();
	//リストの中から現在タイマーとして使っている変数を取得する。
	cocos2d::Ref *ref = nullptr;
	CCARRAY_FOREACH(variableTimerList, ref) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
		auto p = static_cast<agtk::SceneVariableTimer *>(ref);
#else
		auto p = dynamic_cast<agtk::SceneVariableTimer *>(ref);
#endif
		if (p->getVariableData() == data) {
			variableTimer = p;
			break;
		}
	}
	if (variableTimer == nullptr) {
		variableTimer = agtk::SceneVariableTimer::create(
			data,
			bCountUp ? agtk::SceneVariableTimer::kCountUp : agtk::SceneVariableTimer::kCountDown,
			object
		);
		variableTimerList->addObject(variableTimer);
		variableTimer->start(seconds);
	}
	else {
		auto countType = bCountUp ? agtk::SceneVariableTimer::kCountUp : agtk::SceneVariableTimer::kCountDown;
		if (variableTimer->getCountType() != countType) {
			variableTimer->setCountType(countType);
		}
		else {
			variableTimer->end();
			variableTimer->start(seconds);
		}
	}
}

void Scene::endVariableTimer(agtk::data::PlayVariableData *data)
{
	cocos2d::Ref *ref = nullptr;
	auto variableTimerList = this->getVariableTimerList();
	CCARRAY_FOREACH(variableTimerList, ref) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
		auto variableTimer = static_cast<agtk::SceneVariableTimer *>(ref);
#else
		auto variableTimer = dynamic_cast<agtk::SceneVariableTimer *>(ref);
#endif
		CC_ASSERT(variableTimer);
		if (variableTimer->getVariableData() == data) {
			variableTimer->end();
			variableTimerList->removeObject(variableTimer);
			break;
		}
	}
}

void Scene::removeObjectVariableTimer(agtk::Object *object)
{
	auto objectVariableList = object->getPlayObjectData()->getVariableList();
	cocos2d::DictElement *el = nullptr;
	CCDICT_FOREACH(objectVariableList, el) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
		auto variableData = static_cast<agtk::data::PlayVariableData *>(el->getObject());
#else
		auto variableData = dynamic_cast<agtk::data::PlayVariableData *>(el->getObject());
#endif
		cocos2d::Ref *ref = nullptr;
		auto variableTimerList = this->getVariableTimerList();
		CCARRAY_FOREACH(variableTimerList, ref) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
			auto variableTimer = static_cast<agtk::SceneVariableTimer *>(ref);
#else
			auto variableTimer = dynamic_cast<agtk::SceneVariableTimer *>(ref);
#endif
			if (variableTimer->getVariableData() == variableData) {
				variableTimerList->removeObject(variableTimer);
				break;
			}
		}
	}
}

void Scene::updateVariableTimer(float delta)
{
	if (GuiManager::getInstance()->getGameStop()) {
		return;
	}
	cocos2d::Ref *ref = nullptr;
	auto variableTimerList = this->getVariableTimerList();
	CCARRAY_FOREACH(variableTimerList, ref) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
		auto variableTimer = static_cast<agtk::SceneVariableTimer *>(ref);
#else
		auto variableTimer = dynamic_cast<agtk::SceneVariableTimer *>(ref);
#endif
		CC_ASSERT(variableTimer);
		variableTimer->update(delta);
	}
}

#ifdef USE_PREVIEW
void Scene::setVisibleObjectDebugDisplayArea(bool bVisible)
{
	cocos2d::DictElement *el = nullptr;
	auto sceneLayerList = this->getSceneLayerList();
	CCDICT_FOREACH(sceneLayerList, el) {
		auto sceneLayer = dynamic_cast<agtk::SceneLayer *>(el->getObject());
		cocos2d::Ref *ref = nullptr;
		CCARRAY_FOREACH(sceneLayer->getObjectList(), ref) {
			auto object = dynamic_cast<agtk::Object *>(ref);
			object->setVisibleDebugDisplayAreaAll(bVisible);
		}
	}
}
#endif

#ifdef AGTK_DEBUG
void Scene::setVisibleObjectDebugDisplayPlayer(bool bVisible)
{
	cocos2d::DictElement *el = nullptr;
	auto sceneLayerList = this->getSceneLayerList();
	CCDICT_FOREACH(sceneLayerList, el) {
		auto sceneLayer = dynamic_cast<agtk::SceneLayer *>(el->getObject());
		cocos2d::Ref *ref = nullptr;
		CCARRAY_FOREACH(sceneLayer->getObjectList(), ref) {
			auto object = dynamic_cast<agtk::Object *>(ref);
			object->setVisibleDebugDisplayPlayer(bVisible);
		}
	}
}
#endif

#ifdef USE_PREVIEW
void Scene::showDebugLimitArea(bool bShow)
{
	auto layer = GameManager::getInstance()->getCurrentLayer();
	auto sceneData = this->getSceneData();
	auto limitAreaRect = cocos2d::Rect(sceneData->getLimitAreaX(), sceneData->getLimitAreaY(), sceneData->getLimitAreaWidth(), sceneData->getLimitAreaHeight());
	auto limitAreaSp = this->getDebugLimitAreaSprite();
	if (limitAreaSp == nullptr) {
		auto s = Sprite::create();
		s->setAnchorPoint(Vec2::ANCHOR_TOP_LEFT);
		s->setTextureRect(cocos2d::Rect(cocos2d::Vec2::ZERO, limitAreaRect.size));
		s->setColor(Color3B::RED);
		s->setOpacity(100);
		layer->addChild(s, BaseLayer::ZOrder::Debug);
		this->setDebugLimitAreaSprite(s);
		limitAreaSp = s;
	}
	if (limitAreaSp) {
		cocos2d::Vec2 p = agtk::Scene::getPositionCocos2dFromScene(limitAreaRect.origin);
		limitAreaSp->setPosition(p);
	}
	limitAreaSp->setVisible(bShow);
}

void Scene::showDebugLimitCamera(bool bShow)
{
	auto layer = GameManager::getInstance()->getCurrentLayer();
	auto sceneData = this->getSceneData();
	auto rect = sceneData->getLimitCameraRect();
	auto sp = this->getDebugLimitCameraSprite();
	if (sp == nullptr) {
		sp = Sprite::create();
		sp->setAnchorPoint(Vec2::ANCHOR_TOP_LEFT);
		sp->setTextureRect(cocos2d::Rect(cocos2d::Vec2::ZERO, rect.size));
		sp->setColor(Color3B::BLUE);
		sp->setOpacity(100);
		layer->addChild(sp, BaseLayer::ZOrder::Debug);
		this->setDebugLimitCameraSprite(sp);
	}
	if (sp) {
		cocos2d::Vec2 p = agtk::Scene::getPositionCocos2dFromScene(rect.origin);
		sp->setPosition(p);
	}
	sp->setVisible(bShow);
}
#endif

/**
 * @brief cocos2d-x座標系(左下が原点)であるposの座標を、指定シーンデータでのアクツクMVシーン座標系(左上が原点)に変換したものを返す。
 */
cocos2d::Vec2 Scene::getPositionSceneFromCocos2d(const cocos2d::Vec2 &pos, agtk::data::SceneData const *sceneData)
{
	auto projectData = GameManager::getInstance()->getProjectData();
	float sceneHeight = projectData->getScreenHeight() * sceneData->getVertScreenCount();
	return cocos2d::Vec2(pos.x, sceneHeight - pos.y);
}

/**
* @brief 指定シーンデータでのアクツクMVシーン座標系(左上が原点)であるposの座標をcocos2d-x座標系(左下が原点)に変換したものを返す。
*/
cocos2d::Vec2 Scene::getPositionCocos2dFromScene(const cocos2d::Vec2 &pos, agtk::data::SceneData const *sceneData)
{
	auto projectData = GameManager::getInstance()->getProjectData();
	float sceneHeight = projectData->getScreenHeight() * sceneData->getVertScreenCount();
	return cocos2d::Vec2(pos.x, sceneHeight - pos.y);
}

/**
* @brief cocos2d-x座標系(左下が原点)であるposの座標を、指定シーン(nullptr指定ならカレントシーン)でのアクツクMVシーン座標系(左上が原点)に変換したものを返す。
*/
cocos2d::Vec2 Scene::getPositionSceneFromCocos2d(const cocos2d::Vec2 &pos, agtk::Scene *scene)
{
	if (scene == nullptr) {
		scene = GameManager::getInstance()->getCurrentScene();
	}
	if (scene == nullptr) {
		//CC_ASSERT(0);
		return cocos2d::Vec2::ZERO;
	}
	return cocos2d::Vec2(pos.x, scene->getSceneSize().y - pos.y);
}

/**
* @brief 指定シーン(nullptr指定ならカレントシーン)でのアクツクMVシーン座標系(左上が原点)であるposの座標を、cocos2d-x座標系(左下が原点)に変換したものを返す。
*/
cocos2d::Vec2 Scene::getPositionCocos2dFromScene(const cocos2d::Vec2 &pos, agtk::Scene *scene)
{
	if (scene == nullptr) {
		scene = GameManager::getInstance()->getCurrentScene();
	}
	if (scene == nullptr) {
		CC_ASSERT(0);
		return cocos2d::Vec2::ZERO;
	}
	return cocos2d::Vec2(pos.x, scene->getSceneSize().y - pos.y);
}

/**
* @brief cocos2d-x座標系(左下が原点)である矩形の座標を、指定シーン(nullptr指定ならカレントシーン)でのアクツクMVシーン座標系(左上が原点)に変換したものを返す。
*/
cocos2d::Rect Scene::getRectSceneFromCocos2d(const cocos2d::Rect &rect, agtk::Scene *scene)
{
	if (scene == nullptr) {
		scene = GameManager::getInstance()->getCurrentScene();
	}
	if (scene == nullptr) {
		CC_ASSERT(0);
		return cocos2d::Rect(cocos2d::Vec2::ZERO, rect.size);
	}
	cocos2d::Rect r;
	r.origin = getPositionSceneFromCocos2d(rect.origin, scene);
	r.origin.y -= rect.size.height;
	r.size = rect.size;
	return r;
}

/**
* @brief 指定シーン(nullptr指定ならカレントシーン)でのアクツクMVシーン座標系(左上が原点)である矩形の座標を、cocos2d-x座標系(左下が原点)に変換したものを返す。
*/
cocos2d::Rect Scene::getRectCocos2dFromScene(const cocos2d::Rect &rect, agtk::Scene *scene)
{
	if (scene == nullptr) {
		scene = GameManager::getInstance()->getCurrentScene();
	}
	if (scene == nullptr) {
		CC_ASSERT(0);
		return cocos2d::Rect(cocos2d::Vec2::ZERO, rect.size);
	}
	cocos2d::Rect r;
	r.origin = getPositionCocos2dFromScene(rect.origin, scene);
	r.origin.y -= rect.size.height;
	r.size = rect.size;
	return r;
}

/**
 * @brief アクツクMVの回転方向(時計回り)での数値angleを、cocos2d-xの回転方向(反時計回り)に変換したものを返す。
 */
float Scene::getAngleCocos2dFromScene(float angle)
{
	return angle - 90.0f;
}

/**
* @brief cocos2d-xの回転方向(反時計回り)での数値angleを、アクツクMVの回転方向(時計回り)に変換したものを返す。
*/
float Scene::getAngleSceneFromCocos2d(float angle)
{
	return 90.0f + angle;
}

float Scene::getAngleMathFromScene(float angle)
{
	return GetDegree360((360.0f - angle) + 90.0f);
}

float Scene::getAngleSceneFromMath(float angle)
{
	return GetDegree360(360.0f - (angle - 90.0f));
}

float Scene::getAngleMathFromCocos2d(float angle)
{
	auto a = getAngleSceneFromCocos2d(angle);
	return getAngleMathFromScene(a);
}

NS_AGTK_END
