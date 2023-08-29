#include "RenderTexture.h"
#include "Lib/BaseLayer.h"
#include "Lib/Object.h"
#include "Lib/Scene.h"
#include "Manager/GameManager.h"
#include "Manager/EffectManager.h"
#include "Manager/ImageManager.h"
#include "Manager/MovieManager.h"

NS_AGTK_BEGIN //---------------------------------------------------------------------------------//

static char *renderTextureName = "renderTexture";

//-------------------------------------------------------------------------------------------------------------------
RenderTexture::RenderTexture()
{
	_renderTextureArray = nullptr;
	_ignoredLayerPosition = false;
	_shader = nullptr;
}

RenderTexture::~RenderTexture()
{
	CC_SAFE_RELEASE_NULL(_renderTextureArray);
	CC_SAFE_RELEASE_NULL(_shader);
}

RenderTexture *RenderTexture::create(int w, int h, cocos2d::Texture2D::PixelFormat format, bool withDepthStencilBuffer)
{
	auto p = new (std::nothrow) RenderTexture();
	if (p && p->init(w, h, format, withDepthStencilBuffer)) {
		p->autorelease();
		return p;
	}
	CC_SAFE_DELETE(p);
	return nullptr;
}

bool RenderTexture::init(int w, int h, cocos2d::Texture2D::PixelFormat format, bool withDepthStencilBuffer)
{
	bool ret;
	if (withDepthStencilBuffer) {
		ret = this->initWithWidthAndHeight(w, h, format, GL_DEPTH24_STENCIL8);
	}
	else {
		ret = this->initWithWidthAndHeight(w, h, format);
	}
	if (!ret) {
		return false;
	}
	this->setPosition(cocos2d::Vec2(0, 0));
	this->setAnchorPoint(cocos2d::Vec2(0, 0));
	//this->beginWithClear(0, 0, 0, 0);
	//this->end();
	this->setClearColor(cocos2d::Color4F(0.0f, 0.0f, 0.0f, 0.0f));
	//
	this->getChildren().at(0)->setAnchorPoint(cocos2d::Vec2(0, 0));
	this->getSprite()->getTexture()->setAliasTexParameters();
	this->getSprite()->setBlendFunc(BlendFunc::ALPHA_PREMULTIPLIED);

	return true;
}

const cocos2d::Size& RenderTexture::getContentSize()
{
	auto children = this->getChildren();
	return children.at(0)->getContentSize();
}

cocos2d::Node *RenderTexture::getRenderTextureNode()
{
	auto children = this->getChildren();
	return children.at(0);
}

void RenderTexture::addRenderTexture(RenderTexture *renderTexture)
{
	if (_renderTextureArray == nullptr) {
		_renderTextureArray = cocos2d::__Array::create();
		_renderTextureArray->retain();
	}
	_renderTextureArray->addObject(renderTexture);
}

void RenderTexture::removeRenderTexture(RenderTexture *renderTexture)
{
	CC_ASSERT(_renderTextureArray);
	_renderTextureArray->removeObject(renderTexture);
}

void RenderTexture::removeAllRenderTexture()
{
	if (_renderTextureArray) {
		_renderTextureArray->removeAllObjects();
	}
}

agtk::Shader *RenderTexture::addShader(Shader::ShaderKind kind, float value, float seconds)
{
	auto shader = agtk::Shader::createShaderKind(this, this->getContentSize(), kind, 1);// baseNodeFlag ? -1 : 1);//階層-1は「すべて」、1は「1階層」まで。
	shader->setIntensity(value, seconds);
	this->setShader(shader);
	return shader;
}

cocos2d::Layer *RenderTexture::getLayer()
{
	cocos2d::Node *node = this->getParent();
	while (node) {
		auto layer = dynamic_cast<cocos2d::Layer *>(node);
		if (layer) {
			return layer;
		}
		node = node->getParent();
	}
	return nullptr;
}

void RenderTexture::update(float delta, cocos2d::Node *node, cocos2d::Mat4 m)
{
	auto renderer = cocos2d::Director::getInstance()->getRenderer();
	auto sprite = this->getSprite();
	sprite->setVisible(false);
	{
		this->setKeepMatrix(true);
		{
			cocos2d::Color4F clearColor = this->getClearColor();
			this->beginWithClear(clearColor.r, clearColor.g, clearColor.b, clearColor.a);

			if (node) {
				node->setVisible(true);
				node->visit(renderer, m, true);
				node->setVisible(false);
			}

			if (_renderTextureArray) {
				cocos2d::Ref *ref = nullptr;
				CCARRAY_FOREACH(_renderTextureArray, ref) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
					auto rt = static_cast<RenderTexture *>(ref);
#else
					auto rt = dynamic_cast<RenderTexture *>(ref);
#endif
					rt->visit(renderer, m, false);
				}
			}

			this->end();
		}
		this->setKeepMatrix(false);
	}
	sprite->setVisible(true);

	//shader
	auto shader = this->getShader();
	if (shader) {
		shader->update(delta);
	}
}

void RenderTexture::update(float delta)
{
	auto renderer = cocos2d::Director::getInstance()->getRenderer();

	cocos2d::Color4F color = this->getClearColor();
	this->beginWithClear(color.r, color.g, color.b, color.a);

	auto children = this->getChildren();
	for (int i = 0; i < this->getChildrenCount(); i++) {
		children.at(i)->visit();
	}

	if (_renderTextureArray) {
		cocos2d::Ref *ref = nullptr;
		CCARRAY_FOREACH(_renderTextureArray, ref) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
			auto rt = static_cast<RenderTexture *>(ref);
#else
			auto rt = dynamic_cast<RenderTexture *>(ref);
#endif
			auto director = Director::getInstance();
			cocos2d::Mat4 m(director->getMatrix(MATRIX_STACK_TYPE::MATRIX_STACK_MODELVIEW));
			rt->visit(renderer, m, false);
		}
	}
	this->end();

	//shader
	auto shader = this->getShader();
	if (shader) {
		shader->update(delta);
	}
}

void RenderTexture::reset()
{
	this->setShader(nullptr);
}

//-------------------------------------------------------------------------------------------------------------------
RenderTextureCtrl::RenderTextureCtrl()
{
	_renderTextureList = nullptr;
	_shaderList = nullptr;
	_baseNode = nullptr;
// #AGTK-NX
#if CC_TARGET_PLATFORM != CC_PLATFORM_NX
	_clearColor = cocos2d::Color4F::Color4F(0, 0, 0, 0);
#else
#endif
	_renderTextureSize = cocos2d::Size::ZERO;
	_layer = nullptr;

	_tileRenderTexture = nullptr;
	_objRenderTexture = nullptr;
	_isRenderTile = false;
	_isRenderObject = false;
}

RenderTextureCtrl::~RenderTextureCtrl()
{
	if (this->getType() == kTypeSceneTopMost) {
		auto layer = this->getLayer();
		cocos2d::Ref *ref;
		CCARRAY_FOREACH(_renderTextureList, ref) {
			auto renderTexture = dynamic_cast<agtk::RenderTexture *>(ref);
			layer->removeChild(renderTexture);
		}
	}
	else if (this->getType() == kTypeNormal ||
		this->getType() == kTypeSceneLayer ||
		this->getType() == kTypeSceneBackground) {
		auto parent = this->getBaseNode()->getParent();
		if (parent) {
			cocos2d::Ref *ref;
			CCARRAY_FOREACH(_renderTextureList, ref) {
				auto renderTexture = dynamic_cast<agtk::RenderTexture *>(ref);
				parent->removeChild(renderTexture);
			}
		}
	}
	if (_objRenderTexture) {
		_objRenderTexture->reset();
	}
	CC_SAFE_RELEASE_NULL(_layer);
	CC_SAFE_RELEASE_NULL(_renderTextureList);
	CC_SAFE_RELEASE_NULL(_shaderList);
	CC_SAFE_RELEASE_NULL(_baseNode);
	CC_SAFE_RELEASE_NULL(_tileRenderTexture);
	CC_SAFE_RELEASE_NULL(_objRenderTexture);
}

RenderTextureCtrl *RenderTextureCtrl::create(cocos2d::Node *node, cocos2d::Size renderTextureSize, int zOrder, bool withDepthStencilBuffer)
{
	auto p = new (std::nothrow) RenderTextureCtrl();
	if (p && p->init(node, renderTextureSize, zOrder, withDepthStencilBuffer)) {
		p->autorelease();
		return p;
	}
	CC_SAFE_DELETE(p);
	return nullptr;
}

RenderTextureCtrl *RenderTextureCtrl::createForSceneLayer(agtk::SceneLayer *sceneLayer, cocos2d::Size renderTextureSize, int zOrder)
{
	auto p = new (std::nothrow) RenderTextureCtrl();
	if (p && p->initForSceneLayer(sceneLayer, renderTextureSize, zOrder)) {
		p->autorelease();
		return p;
	}
	CC_SAFE_DELETE(p);
	return nullptr;
}

RenderTextureCtrl *RenderTextureCtrl::createForSceneBackground(agtk::SceneBackground *sceneBackground, cocos2d::Size renderTextureSize, int zOrder)
{
	auto p = new (std::nothrow) RenderTextureCtrl();
	if (p && p->initForSceneBackground(sceneBackground, renderTextureSize, zOrder)) {
		p->autorelease();
		return p;
	}
	CC_SAFE_DELETE(p);
	return nullptr;
}

RenderTextureCtrl *RenderTextureCtrl::createForSceneTopMost(agtk::SceneTopMost *sceneTopMost, cocos2d::Size renderTextureSize, int zOrder)
{
	auto p = new (std::nothrow) RenderTextureCtrl();
	if (p && p->initForSceneTopMost(sceneTopMost, renderTextureSize, zOrder)) {
		p->autorelease();
		return p;
	}
	CC_SAFE_DELETE(p);
	return nullptr;
}

bool RenderTextureCtrl::init(cocos2d::Node *node, cocos2d::Size renderTextureSize, int zOrder, bool withDepthStencilBuffer)
{
	if (!cocos2d::Node::init()) {
		return false;
	}
	this->setType(kTypeNormal);
	this->setBaseNode(node);
	this->setShaderList(cocos2d::__Array::create());
	this->setRenderTextureList(cocos2d::__Array::create());
	this->setRenderTextureSize(renderTextureSize);
	this->setContentSize(renderTextureSize);

	//add renderTexture
	auto renderTexture = this->addRenderTexture(renderTextureSize, withDepthStencilBuffer);
	node->getParent()->addChild(renderTexture, zOrder);
	return true;
}

bool RenderTextureCtrl::initForSceneLayer(agtk::SceneLayer *sceneLayer, cocos2d::Size renderTextureSize, int zOrder)
{
	if (!cocos2d::Node::init()) {
		return false;
	}
	this->setType(kTypeSceneLayer);
	this->setBaseNode(sceneLayer);
	this->setShaderList(cocos2d::__Array::create());
	this->setRenderTextureList(cocos2d::__Array::create());
	this->setRenderTextureSize(renderTextureSize);
	this->setContentSize(renderTextureSize);

	//add renderTexture
	auto renderTexture = this->addRenderTexture(renderTextureSize);
	if (renderTexture == nullptr) {
		return false;
	}
	renderTexture->setLocalZOrder(zOrder);
	auto initRenderTexture = [](cocos2d::RenderTexture *renderTexture) {
		renderTexture->setPosition(cocos2d::Vec2(0, 0));
		renderTexture->setAnchorPoint(cocos2d::Vec2(0, 0));
		renderTexture->setClearColor(cocos2d::Color4F(0.0f, 0.0f, 0.0f, 0.0f));
		renderTexture->clear(0, 0, 0, 0);
		renderTexture->getChildren().at(0)->setAnchorPoint(cocos2d::Vec2(0, 0));
		renderTexture->getSprite()->getTexture()->setAliasTexParameters();
		renderTexture->setVisible(false);
	};

	// タイル用レンダーテクスチャ
	{
		auto rt = agtk::RenderTexture::create(renderTextureSize.width, renderTextureSize.height);
		initRenderTexture(rt);
		rt->addShader(agtk::Shader::kShaderDefault, 0);
		this->setTileRenderTexture(rt);
	}

	// オブジェクト用レンダーテクスチャ
	{
		auto rt = agtk::RenderTexture::create(renderTextureSize.width, renderTextureSize.height);
		initRenderTexture(rt);
		rt->addShader(agtk::Shader::kShaderDefault, 0);
		this->setObjRenderTexture(rt);
	}
	return true;
}

bool RenderTextureCtrl::initForSceneBackground(agtk::SceneBackground *sceneBackground, cocos2d::Size renderTextureSize, int zOrder)
{
	if (!cocos2d::Node::init()) {
		return false;
	}
	this->setType(kTypeSceneBackground);
	this->setBaseNode(sceneBackground);
	this->setShaderList(cocos2d::__Array::create());
	this->setRenderTextureList(cocos2d::__Array::create());
	this->setRenderTextureSize(renderTextureSize);
	this->setContentSize(renderTextureSize);

	//add renderTexture
	auto renderTexture = this->addRenderTexture(renderTextureSize);
	if (renderTexture == nullptr) {
		return false;
	}
	renderTexture->setLocalZOrder(zOrder);
	return true;
}

bool RenderTextureCtrl::initForSceneTopMost(agtk::SceneTopMost *sceneTopMost, cocos2d::Size renderTextureSize, int zOrder)
{
	if (!cocos2d::Node::init()) {
		return false;
	}
	this->setType(kTypeSceneTopMost);
	this->setBaseNode(sceneTopMost);
	this->setShaderList(cocos2d::__Array::create());
	this->setRenderTextureList(cocos2d::__Array::create());
	this->setRenderTextureSize(renderTextureSize);
	this->setContentSize(renderTextureSize);

	//add renderTexture
	auto renderTexture = this->addRenderTexture(renderTextureSize);
	if (renderTexture == nullptr) {
		return false;
	}
	renderTexture->setLocalZOrder(zOrder);
	return true;
}

void RenderTextureCtrl::setClearColor(cocos2d::Color4F color)
{
	cocos2d::Ref *ref = nullptr;
	CCARRAY_FOREACH(this->getRenderTextureList(), ref) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
		auto p = static_cast<RenderTexture *>(ref);
#else
		auto p = dynamic_cast<RenderTexture *>(ref);
#endif
		CC_ASSERT(p);
		p->setClearColor(color);
	}
	_clearColor = color;
}

cocos2d::Color4F RenderTextureCtrl::getClearColor()
{
	return _clearColor;
}

agtk::RenderTexture *RenderTextureCtrl::getLastRenderTexture()
{
	return dynamic_cast<agtk::RenderTexture *>(this->getRenderTextureList()->getLastObject());
}

agtk::RenderTexture *RenderTextureCtrl::getFirstRenderTexture()
{
	auto renderTextureList = this->getRenderTextureList();
	if (renderTextureList->count() == 0) {
		return nullptr;
	}
	return dynamic_cast<agtk::RenderTexture *>(renderTextureList->getObjectAtIndex(0));
}

agtk::RenderTexture *RenderTextureCtrl::addRenderTexture(cocos2d::Size size, bool withDepthStencilBuffer)
{
	auto renderTexture = agtk::RenderTexture::create(size.width, size.height, cocos2d::Texture2D::PixelFormat::RGBA8888, withDepthStencilBuffer);
	auto sprite = renderTexture->getSprite();
	sprite->setAnchorPoint(cocos2d::Vec2::ZERO);
	sprite->getTexture()->setAliasTexParameters();
	sprite->setVisible(false);
	this->getRenderTextureList()->addObject(renderTexture);
	return renderTexture;
}

agtk::Shader *RenderTextureCtrl::addShader(Shader::ShaderKind kind, float value, float seconds)
{
	return this->addShader(kind, this->getRenderTextureSize(), value, seconds);
}

agtk::Shader *RenderTextureCtrl::addShader(Shader::ShaderKind kind, cocos2d::Size size, float value, float seconds)
{
	auto lastRenderTexture = this->getLastRenderTexture();
	int zOrder = lastRenderTexture->getLocalZOrder();

	//render texture
	auto renderTexture = addRenderTexture(size);
	auto sprite = renderTexture->getSprite();

	//shader
	auto shader = agtk::Shader::create(size, kind);
	shader->setIgnored(false);
	shader->setIntensity(value, seconds);
	shader->setShaderProgram(sprite);
	renderTexture->setShader(shader);

	if (this->getType() == kTypeSceneTopMost) {
		auto layer = this->getLayer();
		layer->addChild(renderTexture, zOrder);
	}
	else {
		auto node = this->getBaseNode();
		node->getParent()->addChild(renderTexture, zOrder);
	}
	this->getShaderList()->addObject(shader);

	// プロジェクトの画面効果が設定されている場合は一番後ろに配置されるように調整する。
	// 最前面クラスで最前面で表示させる必要があるため。
	auto projectData = GameManager::getInstance()->getProjectData();
	int screenEffectCount = projectData->getScreenEffectCount();

	if (screenEffectCount > 0) {
		int renderTextureCount = this->getRenderTextureList()->count();
		int shaderCount = this->getShaderList()->count();

		if (renderTextureCount > 2 && screenEffectCount < shaderCount) {
			for (int i = 1; i <= screenEffectCount; ++i) {
				int sortRenderTextureCount = renderTextureCount - i;
				int sortShaderCount = shaderCount - i;

				this->getRenderTextureList()->swap(sortRenderTextureCount, sortRenderTextureCount - 1);
				this->getShaderList()->swap(sortShaderCount, sortShaderCount - 1);
			}
		}
	}
	return shader;
}


agtk::Shader *RenderTextureCtrl::getShader(Shader::ShaderKind kind)
{
	cocos2d::Ref *ref = nullptr;
	CCARRAY_FOREACH(this->getShaderList(), ref) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
		auto shader = static_cast<agtk::Shader *>(ref);
#else
		auto shader = dynamic_cast<agtk::Shader *>(ref);
#endif
		if (shader->getKind() == kind) {
			return shader;
		}
	}
	return nullptr;
}

/**
* シェーダ使用判定
*/
bool RenderTextureCtrl::isUseShader()
{
	if (getShaderList()->count()) {
		return true;
	}

	if (getIsRenderTile() || getIsRenderObject()) {
		return true;
	}

	return false;
}

bool RenderTextureCtrl::isShader(Shader::ShaderKind kind)
{
	return this->getShader(kind) ? true : false;
}

void RenderTextureCtrl::removeShader(agtk::Shader *shader)
{
	cocos2d::RenderTexture *renderTexture = nullptr;
	auto renderTextureList = this->getRenderTextureList();
	cocos2d::Ref *ref;
	CCARRAY_FOREACH(renderTextureList, ref) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
		auto p = static_cast<agtk::RenderTexture *>(ref);
#else
		auto p = dynamic_cast<agtk::RenderTexture *>(ref);
#endif
		if (shader == p->getShader()) {
			renderTexture = p;
			break;
		}
	}
	this->getShaderList()->removeObject(shader);
	if (renderTexture) {
		renderTextureList->removeObject(renderTexture);
	}
	if (this->getType() == kTypeSceneTopMost) {
		auto layer = this->getLayer();
		layer->removeChild(renderTexture);
	}
	else {
		auto node = this->getBaseNode();
		node->getParent()->removeChild(renderTexture);
	}
}

void RenderTextureCtrl::removeShader(Shader::ShaderKind kind)
{
	//get shader
	auto shader = this->getShader(kind);
	if (shader == nullptr) {
		return;
	}
	this->removeShader(shader);
}
void RenderTextureCtrl::removeShaderAll()
{
	auto renderTextureList = this->getRenderTextureList();
	if (this->getType() == kTypeSceneTopMost) {
		auto layer = this->getLayer();
		if (!layer) {
//			CC_ASSERT(0);
			return;
		}
		cocos2d::Ref *ref;
		CCARRAY_FOREACH(renderTextureList, ref) {
			auto renderTexture = dynamic_cast<agtk::RenderTexture *>(ref);
			layer->removeChild(renderTexture);
		}
	}
	else if (this->getType() == kTypeNormal ||
		this->getType() == kTypeSceneLayer ||
		this->getType() == kTypeSceneBackground) {
		auto parent = this->getBaseNode()->getParent();
		if (parent) {
			cocos2d::Ref *ref;
			CCARRAY_FOREACH(renderTextureList, ref) {
				auto renderTexture = dynamic_cast<agtk::RenderTexture *>(ref);
				parent->removeChild(renderTexture);
			}
		}
	}
	this->getShaderList()->removeAllObjects();
}

void RenderTextureCtrl::update(float delta, cocos2d::Mat4 *viewMatrix, cocos2d::Camera * viewCamera)
{
	if (this->getLayer() == nullptr && this->getType() != kTypeNormal) {
		CC_ASSERT(0);
		return;
	}

	auto director = Director::getInstance();
	auto renderer = director->getRenderer();
	cocos2d::Vec3 pos(cocos2d::Vec3::ZERO);
	cocos2d::Vec3 scale(cocos2d::Vec3::ONE);
	auto camRot3D = viewCamera ? viewCamera->getRotation3D() : director->getRunningScene() ? director->getRunningScene()->getDefaultCamera()->getRotation3D() : cocos2d::Vec3::ZERO;
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
	auto renderTextureList = this->getRenderTextureList();
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
	for(int i = 0; i < renderTextureList->count(); i++){
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
		auto renderTexture = static_cast<agtk::RenderTexture *>(renderTextureList->getObjectAtIndex(i));
#else
		auto renderTexture = dynamic_cast<agtk::RenderTexture *>(renderTextureList->getObjectAtIndex(i));
#endif
		auto sprite = renderTexture->getSprite();
		if (i == 0) {
			auto node = this->getBaseNode();
			renderTexture->update(delta, node, m);
		} else {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
			auto renderTexture2 = static_cast<agtk::RenderTexture *>(renderTextureList->getObjectAtIndex(i - 1));
#else
			auto renderTexture2 = dynamic_cast<agtk::RenderTexture *>(renderTextureList->getObjectAtIndex(i - 1));
#endif
			CC_ASSERT(renderTexture2);
			auto sprite2 = renderTexture2->getSprite();
			renderTexture->update(delta, sprite2, m);

		}
		sprite->setPosition3D(pos);
		sprite->setRotation3D(camRot3D);
		sprite->setScale(scale.x, scale.y);
	}
}

/**
* 最前面の更新
*/
void RenderTextureCtrl::updateTopMost(float delta, cocos2d::Mat4 *viewMatrix, cocos2d::Camera * viewCamera)
{
	auto director = Director::getInstance();
	auto renderer = director->getRenderer();
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
	auto renderTextureList = this->getRenderTextureList();
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
			// 背景とメインシーンを追加(メニューシーンは除く)
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
			// sceneTopMost は未使用につき不要
#else
			auto sceneTopMost = dynamic_cast<agtk::SceneTopMost *>(this->getBaseNode());
#endif

			sprite->setVisible(false);
			{
				renderTexture->setKeepMatrix(true);
				{
					cocos2d::Color4F clearColor = renderTexture->getClearColor();
					renderTexture->beginWithClear(clearColor.r, clearColor.g, clearColor.b, clearColor.a);

					auto scene = GameManager::getInstance()->getCurrentScene();

					std::vector<cocos2d::Node *> sceneRenderSpriteList;

					{
						// 背景シーン
						auto sceneBackground = scene->getSceneBackground();
						auto renderTextureCtrl = sceneBackground->getRenderTexture();
						if (renderTextureCtrl) {
							sceneRenderSpriteList.push_back(renderTextureCtrl->getLastRenderTexture()->getSprite());
						}
						else {
							sceneRenderSpriteList.push_back(sceneBackground);
						}

						// メインシーン
						auto dic = scene->getSceneLayerList();
						cocos2d::DictElement *el = nullptr;
						CCDICT_FOREACH(dic, el) {
							// #AGTK-NX
#ifdef STATIC_DOWN_CAST
							auto sceneLayer = static_cast<agtk::SceneLayer *>(el->getObject());
#else
							auto sceneLayer = dynamic_cast<agtk::SceneLayer *>(el->getObject());
#endif
							auto renderTextureCtrl = sceneLayer->getRenderTexture();
							if (renderTextureCtrl && renderTextureCtrl->isUseShader()) {
								sceneRenderSpriteList.push_back(renderTextureCtrl->getLastRenderTexture()->getSprite());
							}
							else {
								sceneRenderSpriteList.push_back(sceneLayer);
							}
						}
					}

					{
						// メニュー以外
						for (auto sprite : sceneRenderSpriteList) {
							sprite->setVisible(true);
							sprite->visit(renderer, m, false);
							sprite->setVisible(false);
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
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
			auto renderTexture2 = static_cast<agtk::RenderTexture *>(renderTextureList->getObjectAtIndex(i - 1));
#else
			auto renderTexture2 = dynamic_cast<agtk::RenderTexture *>(renderTextureList->getObjectAtIndex(i - 1));
#endif
			CC_ASSERT(renderTexture2);
			auto sprite2 = renderTexture2->getSprite();
			renderTexture->update(delta, sprite2, m);

		}
		sprite->setPosition3D(pos);
		sprite->setRotation3D(camRot3D);
		sprite->setScale(scale.x, scale.y);
	}
}

/**
* 最前面(メニュー含む)の更新
*/
void RenderTextureCtrl::updateTopMostWithMenu(float delta, cocos2d::Mat4 *viewMatrix, cocos2d::Camera * viewCamera)
{
	auto director = Director::getInstance();
	auto renderer = director->getRenderer();
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
	auto renderTextureList = this->getRenderTextureList();
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
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
			// sceneTopMostWithMenu は未使用につき不要
#else
			auto sceneTopMostWithMenu = dynamic_cast<agtk::SceneTopMost *>(this->getBaseNode());
#endif

			sprite->setVisible(false);
			{
				renderTexture->setKeepMatrix(true);
				{
					cocos2d::Color4F clearColor = renderTexture->getClearColor();
					renderTexture->beginWithClear(clearColor.r, clearColor.g, clearColor.b, clearColor.a);

					auto scene = GameManager::getInstance()->getCurrentScene();
					auto sceneCamera = scene->getCamera();
					auto menuViewMatrix = sceneCamera->getMenuCamera()->getViewMatrix();

					std::vector<cocos2d::Node *> sceneRenderSpriteList;
					std::vector<cocos2d::Node *> menuRenderSpriteList;
					std::vector<cocos2d::Node *> menuTopMostRenderSpriteList;

					// 最前面にレンダーがなければ「背景シーン」「メインシーン」を追加後、「メニューシーン」を追加
					// 最前面にレンダーがあれば、そのレンダーに「メニューシーン」を追加
					auto sceneTopMost = scene->getSceneTopMost();
					auto renderTextureCtrl = sceneTopMost->getRenderTexture();
// #AGTK-NX
#if CC_TARGET_PLATFORM != CC_PLATFORM_NX
					if (renderTextureCtrl == false) {
#else
#endif
						// 背景シーン
						auto sceneBackground = scene->getSceneBackground();
						auto renderTextureCtrl = sceneBackground->getRenderTexture();
						if (renderTextureCtrl) {
							sceneRenderSpriteList.push_back(renderTextureCtrl->getLastRenderTexture()->getSprite());
						}
						else {
							sceneRenderSpriteList.push_back(sceneBackground);
						}

						// メインシーン
						auto dic = scene->getSceneLayerList();
						cocos2d::DictElement *el = nullptr;
						CCDICT_FOREACH(dic, el) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
							auto sceneLayer = static_cast<agtk::SceneLayer *>(el->getObject());
#else
							auto sceneLayer = dynamic_cast<agtk::SceneLayer *>(el->getObject());
#endif
							auto renderTextureCtrl = sceneLayer->getRenderTexture();
							if (renderTextureCtrl && renderTextureCtrl->isUseShader()) {
								sceneRenderSpriteList.push_back(renderTextureCtrl->getLastRenderTexture()->getSprite());
							}
							else {
								sceneRenderSpriteList.push_back(sceneLayer);
							}
						}
					}
					else {
						// 最前面 ※メニューなし
						sceneRenderSpriteList.push_back(renderTextureCtrl->getLastRenderTexture()->getSprite());
					}

					// メニューシーンを非表示
					// gameManager->getCurrentLayerをvisitすると濃くなるので非表示にしておく
					std::deque<bool> menuVisibleList;
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
							menuVisibleList.emplace_back(menuLayer->isVisible());
							menuLayer->setVisible(false);
						}
					}

					// デバッグ表示
					auto gameManager = GameManager::getInstance();
					auto layer = gameManager->getCurrentLayer();
					sceneRenderSpriteList.push_back(layer);

					// メニューシーン
					auto menuLayerList = scene->getMenuLayerList();
					cocos2d::DictElement *el = nullptr;
					CCDICT_FOREACH(menuLayerList, el) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
						auto menuLayer = static_cast<agtk::SceneLayer *>(el->getObject());
#else
						auto menuLayer = dynamic_cast<agtk::SceneLayer *>(el->getObject());
#endif
						if (menuLayer->getLayerId() != agtk::data::SceneData::kHudTopMostLayerId) {
							menuRenderSpriteList.push_back(menuLayer);
						}
						else {
							menuTopMostRenderSpriteList.push_back(menuLayer);
						}
					}

					{
						// メニュー以外
						auto topMost = scene->getSceneTopMost();
						auto lVisible = topMost->isVisible();
						topMost->setVisible(false);
						for (auto sprite : sceneRenderSpriteList) {
							sprite->setVisible(true);
							sprite->visit(renderer, m, false);
							sprite->setVisible(false);
						}
						topMost->setVisible(lVisible);

						// メニューシーン表示(非表示からの戻し)
						{
							int menuVisibleIndex = 0;

							auto menuLayerList = scene->getMenuLayerList();
							cocos2d::DictElement *el;
							CCDICT_FOREACH(menuLayerList, el) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
								auto menuLayer = static_cast<agtk::SceneLayer *>(el->getObject());
#else
								auto menuLayer = dynamic_cast<agtk::SceneLayer *>(el->getObject());
#endif

								menuLayer->setVisible(menuVisibleList[menuVisibleIndex]);

								++menuVisibleIndex;
							}
						}

						// メニューのみ
						for (auto sprite : menuRenderSpriteList) {
							sprite->setVisible(true);
							sprite->visit(renderer, menuViewMatrix, false);
							sprite->setVisible(false);
						}
						// メニューのみ(最前面+メニュー)
						for (auto sprite : menuTopMostRenderSpriteList) {
							sprite->setVisible(true);
							sprite->visit(renderer, m, false);
							sprite->setVisible(false);
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
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
			auto renderTexture2 = static_cast<agtk::RenderTexture *>(renderTextureList->getObjectAtIndex(i - 1));
#else
			auto renderTexture2 = dynamic_cast<agtk::RenderTexture *>(renderTextureList->getObjectAtIndex(i - 1));
#endif
			CC_ASSERT(renderTexture2);
			auto sprite2 = renderTexture2->getSprite();
			renderTexture->update(delta, sprite2, m);

		}
		sprite->setPosition3D(pos);
		sprite->setRotation3D(camRot3D);
		sprite->setScale(scale.x, scale.y);
	}
}

/**
* 最前面(メニュー含む)のメニューを無視した更新
*/
void RenderTextureCtrl::updateTopMostWithMenuIgnoreMenu(float delta, cocos2d::Mat4 *viewMatrix, cocos2d::Camera * viewCamera)
{
	auto director = Director::getInstance();
	auto renderer = director->getRenderer();
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
	auto renderTextureList = this->getRenderTextureList();
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
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
			// sceneTopMostWithMenu は未使用につき不要
#else
			auto sceneTopMostWithMenu = dynamic_cast<agtk::SceneTopMost *>(this->getBaseNode());
#endif

			sprite->setVisible(false);
			{
				renderTexture->setKeepMatrix(true);
				{
					cocos2d::Color4F clearColor = renderTexture->getClearColor();
					renderTexture->beginWithClear(clearColor.r, clearColor.g, clearColor.b, clearColor.a);

					auto scene = GameManager::getInstance()->getCurrentScene();
					auto sceneCamera = scene->getCamera();
					auto menuViewMatrix = sceneCamera->getMenuCamera()->getViewMatrix();

					std::vector<cocos2d::Node *> sceneRenderSpriteList;
					std::vector<cocos2d::Node *> menuRenderSpriteList;
					std::vector<cocos2d::Node *> menuTopMostRenderSpriteList;

					// 最前面にレンダーがなければ「背景シーン」「メインシーン」を追加後、「メニューシーン」を追加
					// 最前面にレンダーがあれば、そのレンダーに「メニューシーン」を追加
					auto sceneTopMost = scene->getSceneTopMost();
					auto renderTextureCtrl = sceneTopMost->getRenderTexture();
// #AGTK-NX
#if (CC_TARGET_PLATFORM == CC_PLATFORM_NX)
#else
					if (renderTextureCtrl == false) {
#endif
						// 背景シーン
						auto sceneBackground = scene->getSceneBackground();
						auto renderTextureCtrl = sceneBackground->getRenderTexture();
						if (renderTextureCtrl) {
							sceneRenderSpriteList.push_back(renderTextureCtrl->getLastRenderTexture()->getSprite());
						}
						else {
							sceneRenderSpriteList.push_back(sceneBackground);
						}

						// メインシーン
						auto dic = scene->getSceneLayerList();
						cocos2d::DictElement *el = nullptr;
						CCDICT_FOREACH(dic, el) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
							auto sceneLayer = static_cast<agtk::SceneLayer *>(el->getObject());
#else
							auto sceneLayer = dynamic_cast<agtk::SceneLayer *>(el->getObject());
#endif
							auto renderTextureCtrl = sceneLayer->getRenderTexture();
							if (renderTextureCtrl && renderTextureCtrl->isUseShader()) {
								sceneRenderSpriteList.push_back(renderTextureCtrl->getLastRenderTexture()->getSprite());
							}
							else {
								sceneRenderSpriteList.push_back(sceneLayer);
							}
						}
					}
					else {
						// 最前面 ※メニューなし
						sceneRenderSpriteList.push_back(renderTextureCtrl->getLastRenderTexture()->getSprite());
					}

					{
						// メニュー以外
						for (auto sprite : sceneRenderSpriteList) {
							sprite->setVisible(true);
							sprite->visit(renderer, m, false);
							sprite->setVisible(false);
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
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
			auto renderTexture2 = static_cast<agtk::RenderTexture *>(renderTextureList->getObjectAtIndex(i - 1));
#else
			auto renderTexture2 = dynamic_cast<agtk::RenderTexture *>(renderTextureList->getObjectAtIndex(i - 1));
#endif
			CC_ASSERT(renderTexture2);
			auto sprite2 = renderTexture2->getSprite();
			renderTexture->update(delta, sprite2, m);

		}
		sprite->setPosition3D(pos);
		sprite->setRotation3D(camRot3D);
		sprite->setScale(scale.x, scale.y);
	}
}

/**
* 最前面(メニュー含む)のメニューのみ更新
*/
void RenderTextureCtrl::updateTopMostWithMenuOnlyMenu(float delta, cocos2d::Mat4 *viewMatrix, cocos2d::Camera * viewCamera)
{
	auto director = Director::getInstance();
	auto renderer = director->getRenderer();
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
	auto renderTextureList = this->getRenderTextureList();
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
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
			// sceneTopMostWithMenu は未使用につき不要
#else
			auto sceneTopMostWithMenu = dynamic_cast<agtk::SceneTopMost *>(this->getBaseNode());
#endif

			sprite->setVisible(false);
			{
				renderTexture->setKeepMatrix(true);
				{
					cocos2d::Color4F clearColor = renderTexture->getClearColor();
					renderTexture->beginWithClear(clearColor.r, clearColor.g, clearColor.b, clearColor.a);

					auto scene = GameManager::getInstance()->getCurrentScene();
					auto sceneCamera = scene->getCamera();
					auto menuViewMatrix = sceneCamera->getMenuCamera()->getViewMatrix();

					std::vector<cocos2d::Node *> sceneRenderSpriteList;
					std::vector<cocos2d::Node *> menuRenderSpriteList;
					std::vector<cocos2d::Node *> menuTopMostRenderSpriteList;

					// メニューシーン
					auto menuLayerList = scene->getMenuLayerList();
					cocos2d::DictElement *el = nullptr;
					CCDICT_FOREACH(menuLayerList, el) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
						auto menuLayer = static_cast<agtk::SceneLayer *>(el->getObject());
#else
						auto menuLayer = dynamic_cast<agtk::SceneLayer *>(el->getObject());
#endif
						if (menuLayer->getLayerId() != agtk::data::SceneData::kHudTopMostLayerId) {
							menuRenderSpriteList.push_back(menuLayer);
						}
						else {
							menuTopMostRenderSpriteList.push_back(menuLayer);
						}
					}

					{
						// メニューのみ
						for (auto sprite : menuRenderSpriteList) {
							sprite->setVisible(true);
							sprite->visit(renderer, menuViewMatrix, false);
							sprite->setVisible(false);
						}
						// メニューのみ(最前面+メニュー)
						for (auto sprite : menuTopMostRenderSpriteList) {
							sprite->setVisible(true);
							sprite->visit(renderer, m, false);
							sprite->setVisible(false);
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
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
			auto renderTexture2 = static_cast<agtk::RenderTexture *>(renderTextureList->getObjectAtIndex(i - 1));
#else
			auto renderTexture2 = dynamic_cast<agtk::RenderTexture *>(renderTextureList->getObjectAtIndex(i - 1));
#endif
			CC_ASSERT(renderTexture2);
			auto sprite2 = renderTexture2->getSprite();
			renderTexture->update(delta, sprite2, m);

		}
		sprite->setPosition3D(pos);
		sprite->setRotation3D(camRot3D);
		sprite->setScale(scale.x, scale.y);
	}
}

/**
* 重なり演出付き更新
* @param	delta				前フレームからの経過時間
* @param	viewMatrix			ビュー行列
* @param	maskList			マスクオブジェクトリスト
* @param	objList				通常描画するオブジェクトリスト
* @param	tileMapList			通常描画するタイルマップリスト
* @param	ignoreVisibleObject	オブジェクト描画除外フラグ
*/
void RenderTextureCtrl::update(float delta, cocos2d::Mat4 *viewMatrix, cocos2d::__Array *maskList, cocos2d::__Array *objList, cocos2d::__Array *tileMapList, bool ignoreVisibleObject)
{
	auto director = Director::getInstance();
	auto renderer = director->getRenderer();
	cocos2d::Vec3 pos(cocos2d::Vec3::ZERO);
	cocos2d::Vec3 scale(cocos2d::Vec3::ONE);
	auto camRot3D = director->getRunningScene()->getDefaultCamera()->getRotation3D();
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
	auto sceneLayer = static_cast<agtk::SceneLayer *>(this->getBaseNode());
#else
	auto sceneLayer = dynamic_cast<agtk::SceneLayer *>(this->getBaseNode());
#endif
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
	auto renderTextureList = this->getRenderTextureList();
	CCARRAY_FOREACH(renderTextureList, ref) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
		auto renderTexture = static_cast<agtk::RenderTexture *>(ref);
#else
		auto renderTexture = dynamic_cast<agtk::RenderTexture *>(ref);
#endif
		renderTexture->getSprite()->setVisible(false);
	}

	// 透過マスク描画メソッド
	auto drawTransparentMask = [](cocos2d::Renderer *renderer, Mat4 &m, cocos2d::__Array *maskList, int targetType) {
		cocos2d::Ref *ref = nullptr;
		CCARRAY_FOREACH(maskList, ref) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
			auto obj = static_cast<agtk::Object *>(ref);
#else
			auto obj = dynamic_cast<agtk::Object *>(ref);
#endif
			auto player = obj->getPlayer();
			if (obj && player && obj->getObjectVisible()->getVisible()) {
				obj->drawTransparentMaskNode(renderer, m, targetType);
			}
		}
	};

	// シルエット描画メソッド
	auto drawSilhouette = [](cocos2d::Renderer *renderer, Mat4 &m, cocos2d::__Array *maskList, int targetType) {
		cocos2d::Ref *ref = nullptr;
		CCARRAY_FOREACH(maskList, ref) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
			auto obj = static_cast<agtk::Object *>(ref);
#else
			auto obj = dynamic_cast<agtk::Object *>(ref);
#endif
			auto player = obj->getPlayer();
			if (obj && player && obj->getObjectVisible()->getVisible()) {
				obj->drawSilhouette(renderer, m, targetType);
			}
		}
	};

#ifdef USE_REDUCE_RENDER_TEXTURE
	updateIsRenderTile(maskList, ignoreVisibleObject);
#endif
	// --------------------------------------------------------------
	// タイルをレンダリング
#ifdef USE_REDUCE_RENDER_TEXTURE
	if (_isRenderTile)
#endif
	{
		auto clearColor = _tileRenderTexture->getClearColor();
		_tileRenderTexture->setKeepMatrix(true);
		_tileRenderTexture->beginWithClear(clearColor.r, clearColor.g, clearColor.b, clearColor.a);
		{
			// タイル描画
#ifdef USE_REDUCE_RENDER_TEXTURE
			auto tileMapNode = sceneLayer->getTileMapNode();
			if (tileMapNode) {
				tileMapNode->setVisible(true);
				tileMapNode->visit(renderer, m * tileMapNode->getParent()->getNodeToParentTransform(), false);
				tileMapNode->setVisible(false);
			}
#else
			cocos2d::Ref *ref = nullptr;
			CCARRAY_FOREACH(tileMapList, ref) {
				auto tileMap = dynamic_cast<agtk::TileMap *>(ref);
				if (tileMap) {
					tileMap->setVisible(true);
					tileMap->visit(renderer, m * tileMap->getParent()->getNodeToParentTransform(), false);
					tileMap->setVisible(false);
				}
			}
#endif

			if (!ignoreVisibleObject) {
				// タイルに対する重なり演出(透過)マスク描画
				drawTransparentMask(renderer, m, maskList, Object::OVERLAP_EFFECT_TARGET_TILE);

				// タイルに対する重なり演出(ベタ)描画
				drawSilhouette(renderer, m, maskList, Object::OVERLAP_EFFECT_TARGET_TILE);
			}
		}
		_tileRenderTexture->end();
		_tileRenderTexture->setKeepMatrix(false);
		_tileRenderTexture->setPosition3D(pos);
		_tileRenderTexture->setRotation3D(camRot3D);
		_tileRenderTexture->setScale(scale.x, scale.y);
	}

#ifdef USE_REDUCE_RENDER_TEXTURE
	updateIsRenderObj(maskList, ignoreVisibleObject);
#endif
	// --------------------------------------------------------------
	// オブジェクトをレンダリング
#ifdef USE_REDUCE_RENDER_TEXTURE
	if(_isRenderObject)
#endif
	{
		auto effectManager = EffectManager::getInstance();
		auto imageManager = ImageManager::getInstance();
		auto movieManager = MovieManager::getInstance();
		auto particleManager = ParticleManager::getInstance();
		auto clearColor = _objRenderTexture->getClearColor();
		_objRenderTexture->getSprite()->setVisible(true);
		_objRenderTexture->setKeepMatrix(true);
		_objRenderTexture->beginWithClear(clearColor.r, clearColor.g, clearColor.b, clearColor.a);
		{
			if (!ignoreVisibleObject) {

#ifdef USE_REDUCE_RENDER_TEXTURE
				auto objectSetNode = sceneLayer->getObjectSetNode();
				if (objectSetNode) {
					objectSetNode->setVisible(true);
					objectSetNode->visit(renderer, m * objectSetNode->getParent()->getNodeToParentTransform(), false);
					objectSetNode->setVisible(false);
				}
				auto objectFrontNode = sceneLayer->getObjectFrontNode();
				if (objectFrontNode) {
					objectFrontNode->setVisible(true);
					objectFrontNode->visit(renderer, m * objectFrontNode->getParent()->getNodeToParentTransform(), false);
					objectFrontNode->setVisible(false);
				}
#else
				// オブジェクトに紐付いた物理オブジェクトの描画メソッド
				auto drawPhysicsObj = [](cocos2d::Renderer *renderer, cocos2d::Mat4 &m, cocos2d::__Array *physicsList) {
					cocos2d::Ref *ref = nullptr;
					CCARRAY_FOREACH(physicsList, ref) {
						auto p = dynamic_cast<cocos2d::Node *>(ref);
						p->setVisible(true);
						p->visit(renderer, m * p->getParent()->getNodeToParentTransform(), false);
						p->setVisible(false);
					}
				};

				// オブジェクト描画
				ref = nullptr;
				CCARRAY_FOREACH(objList, ref) {
					auto obj = dynamic_cast<agtk::Object *>(ref);
					auto player = obj->getPlayer();
					if (obj && player) {

						// 残像を描画
						obj->getObjectAfterImage()->drawAfterimage(renderer, m);

						// オブジェクトの奥に描画される物理オブジェクトを描画
						drawPhysicsObj(renderer, m, obj->getDrawBackPhysicsPartsList());

						cocos2d::Ref *ref2;

						//particle (backside)
						auto particleArray = particleManager->getParticleArray(obj);
						CCARRAY_FOREACH(particleArray, ref2) {
							auto particleGroup = dynamic_cast<agtk::ParticleGroup *>(ref2);
							if (particleGroup->getTargetObjctBackside()) {
								cocos2d::Ref *ref3;
								auto particleList = particleGroup->getLayerList();
								CCARRAY_FOREACH(particleList, ref3) {
									auto particle = dynamic_cast<agtk::Particle *>(ref3);
									if (!particle->isBlendAdditive() && particle->getParent()) {
										particle->setVisible(true);
										particle->visit(renderer, m * particle->getParent()->getNodeToParentTransform(), true);
										particle->setVisible(false);
									}
								}
							}
						}
						//effect (backside)
						auto effectArray = effectManager->getEffectArray(obj);
						CCARRAY_FOREACH(effectArray, ref2) {
							auto p = dynamic_cast<agtk::EffectAnimation *>(ref2);
							if (p->getTargetObjctBackside()) {
								p->setVisible(true);
								p->visit(renderer, m * p->getParent()->getNodeToParentTransform(), true);
								p->setVisible(false);
							}
						}
						//image (backside)
						auto imageArray = imageManager->getImageArray(obj);
						CCARRAY_FOREACH(imageArray, ref2) {
							auto p = dynamic_cast<agtk::Image *>(ref2);
							auto sprite = p->getSprite();
							if (!p->getImageShowData()->getPriority() && p->getObjectBackside() && sprite) {
								sprite->setVisible(true);
								sprite->visit(renderer, m * sprite->getParent()->getNodeToParentTransform(), true);
								sprite->setVisible(false);
							}
						}
						//movie (backside)
						auto movieArray = movieManager->getMovieArray(obj);
						CCARRAY_FOREACH(movieArray, ref2) {
							auto p = dynamic_cast<agtk::Movie *>(ref2);
							auto sprite = p->getVideoSprite();
							auto black = p->getFillBlackNode();
							if (!p->getMovieShowData()->getPriority() && p->getObjectBackside() && sprite) {
								//blacknode
								if (black != nullptr) {
									black->setVisible(true);
									black->visit(renderer, m * black->getParent()->getNodeToParentTransform(), true);
									black->setVisible(false);
								}
								//movienode
								sprite->setVisible(true);
								sprite->visit(renderer, m * sprite->getParent()->getNodeToParentTransform(), true);
								sprite->setVisible(false);
							}
						}
						if (obj->isVisible()) {
							
							player->setVisible(true);
							if (obj->getParent() != nullptr) {
								obj->visit(renderer, m * obj->getParent()->getNodeToParentTransform(), true);
							}
							player->setVisible(false);
						}
						//particle
						CCARRAY_FOREACH(particleArray, ref2) {
							auto particleGroup = dynamic_cast<agtk::ParticleGroup *>(ref2);
							if (!particleGroup->getTargetObjctBackside()) {
								cocos2d::Ref *ref3;
								auto particleList = particleGroup->getLayerList();
								CCARRAY_FOREACH(particleList, ref3) {
									auto particle = dynamic_cast<agtk::Particle *>(ref3);
									if (!particle->isBlendAdditive() && particle->getParent()) {
										particle->setVisible(true);
										particle->visit(renderer, m * particle->getParent()->getNodeToParentTransform(), true);
										particle->setVisible(false);
									}
								}
							}
						}
						//effect
						CCARRAY_FOREACH(effectArray, ref2) {
							auto p = dynamic_cast<agtk::EffectAnimation *>(ref2);
							if (!p->getTargetObjctBackside()) {
								p->setVisible(true);
								p->visit(renderer, m * p->getParent()->getNodeToParentTransform(), true);
								p->setVisible(false);
							}
						}
						//image
						CCARRAY_FOREACH(imageArray, ref2) {
							auto p = dynamic_cast<agtk::Image *>(ref2);
							auto sprite = p->getSprite();
							if (!p->getImageShowData()->getPriority() && !p->getObjectBackside() && sprite) {
								sprite->setVisible(true);
								sprite->visit(renderer, m * sprite->getParent()->getNodeToParentTransform(), true);
								sprite->setVisible(false);
							}
						}
						//movie
						CCARRAY_FOREACH(movieArray, ref2) {
							auto p = dynamic_cast<agtk::Movie *>(ref2);
							auto sprite = p->getVideoSprite();
							auto black = p->getFillBlackNode();
							if (!p->getMovieShowData()->getPriority() && !p->getObjectBackside() && sprite) {
								//blacknode
								if (black != nullptr) {
									black->setVisible(true);
									black->visit(renderer, m * black->getParent()->getNodeToParentTransform(), true);
									black->setVisible(false);
								}
								//movienode
								sprite->setVisible(true);
								sprite->visit(renderer, m * sprite->getParent()->getNodeToParentTransform(), true);
								sprite->setVisible(false);
							}
						}

						// オブジェクトの前に描画される物理オブジェクトを描画
						drawPhysicsObj(renderer, m, obj->getDrawFrontPhysicsPartsList());
					}
#ifdef FIX_ACT2_4879
					else if (obj) {
						// オブジェクトの奥に描画される物理オブジェクトを描画
						drawPhysicsObj(renderer, m, obj->getDrawBackPhysicsPartsList());
						// オブジェクトの前に描画される物理オブジェクトを描画
						drawPhysicsObj(renderer, m, obj->getDrawFrontPhysicsPartsList());
					}
#endif
				}
#endif

				// オブジェクトに対する重なり演出(透過)マスク描画
				drawTransparentMask(renderer, m, maskList, Object::OVERLAP_EFFECT_TARGET_OBJ);

				// オブジェクトに対する重なり演出(ベタ)描画
				drawSilhouette(renderer, m, maskList, Object::OVERLAP_EFFECT_TARGET_OBJ);

			}
		}
		_objRenderTexture->end();
		_objRenderTexture->setKeepMatrix(false);
		_objRenderTexture->setPosition3D(pos);
		_objRenderTexture->setRotation3D(camRot3D);
		_objRenderTexture->setScale(scale.x, scale.y);
	}
	else {
		_objRenderTexture->getSprite()->setVisible(false);
#ifdef USE_REDUCE_RENDER_TEXTURE
		auto objectSetNode = sceneLayer->getObjectSetNode();
		if (objectSetNode) {
			objectSetNode->setVisible(!ignoreVisibleObject);
		}
		auto objectFrontNode = sceneLayer->getObjectFrontNode();
		if (objectFrontNode) {
			objectFrontNode->setVisible(!ignoreVisibleObject);
		}
#endif
	}

	// --------------------------------------------------------------
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

					if (sceneLayer->getIsVisible()) {

#ifdef USE_REDUCE_RENDER_TEXTURE
						if (_isRenderTile) {
							// 重なり演出を含めたタイルを描画
							_tileRenderTexture->setVisible(true);
							_tileRenderTexture->visit(renderer, m, true);
							_tileRenderTexture->setVisible(false);
						}
						else {
							auto tileMapNode = sceneLayer->getTileMapNode();
							if (tileMapNode) {
								tileMapNode->setVisible(true);
								tileMapNode->visit(renderer, m * tileMapNode->getParent()->getNodeToParentTransform(), false);
								tileMapNode->setVisible(false);
							}
						}
#else
						// 重なり演出を含めたタイルを描画
						_tileRenderTexture->setVisible(true);
						_tileRenderTexture->visit(renderer, m, true);
						_tileRenderTexture->setVisible(false);
#endif

#ifdef USE_REDUCE_RENDER_TEXTURE
						//加算パーティクルを描画（オブジェクトの後ろ）
						auto additiveParticleBacksideNode = sceneLayer->getAdditiveParticleBacksideNode();
						if (additiveParticleBacksideNode) {
							additiveParticleBacksideNode->setVisible(true);
							additiveParticleBacksideNode->visit(renderer, m * additiveParticleBacksideNode->getParent()->getNodeToParentTransform(), false);
							additiveParticleBacksideNode->setVisible(false);
						}
						if (_isRenderObject) {
							// 重なり演出を含めたオブジェクトを描画
							_objRenderTexture->setVisible(true);
							_objRenderTexture->visit(renderer, m, true);
							_objRenderTexture->setVisible(false);
						}
						else {
							auto objectSetNode = sceneLayer->getObjectSetNode();
							if (objectSetNode) {
								objectSetNode->setVisible(!ignoreVisibleObject);
								objectSetNode->visit(renderer, m * objectSetNode->getParent()->getNodeToParentTransform(), false);
								objectSetNode->setVisible(false);
							}
							auto objectFrontNode = sceneLayer->getObjectFrontNode();
							if (objectFrontNode) {
								objectFrontNode->setVisible(!ignoreVisibleObject);
								objectFrontNode->visit(renderer, m * objectFrontNode->getParent()->getNodeToParentTransform(), false);
								objectFrontNode->setVisible(false);
							}
						}
#else
						// 重なり演出を含めたオブジェクトを描画
						_objRenderTexture->setVisible(true);
						_objRenderTexture->visit(renderer, m, true);
						_objRenderTexture->setVisible(false);
#endif

						// その他シーンレイヤーに紐付いたものを描画
						sceneLayer->setVisible(true);
						sceneLayer->visit(renderer, m, true);
						sceneLayer->setVisible(false);
#ifdef USE_REDUCE_RENDER_TEXTURE
						//加算パーティクルを描画
						auto additiveParticleNode = sceneLayer->getAdditiveParticleNode();
						if (additiveParticleNode) {
							additiveParticleNode->setVisible(true);
							additiveParticleNode->visit(renderer, m * additiveParticleNode->getParent()->getNodeToParentTransform(), false);
							additiveParticleNode->setVisible(false);
						}
#endif
					}

					renderTexture->end();
				}
				renderTexture->setKeepMatrix(false);
			}
			sprite->setVisible(true);
#if 0 // レンダーテクスチャを画像で確認する
			static int cnt = 0;
			if (cnt < 2) {
				{
					std::stringstream str;
					str << "tileTex";
					str << cnt;
					str << ".png";
					_tileRenderTexture->saveToFile(str.str());
				}
				{
					std::stringstream str;
					str << "objTex";
					str << cnt;
					str << ".png";
					_objRenderTexture->saveToFile(str.str());
				}
				{
					std::stringstream str;
					str << "layerTex";
					str << cnt;
					str << ".png";
					renderTexture->saveToFile(str.str());
				}
				++cnt;
			}
#endif
			//shader
			auto shader = renderTexture->getShader();
			if (shader) {
				shader->update(delta);
			}
		}
		else {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
			auto preRenderTexture = static_cast<agtk::RenderTexture *>(renderTextureList->getObjectAtIndex(i - 1));
#else
			auto preRenderTexture = dynamic_cast<agtk::RenderTexture *>(renderTextureList->getObjectAtIndex(i - 1));
#endif
			CC_ASSERT(preRenderTexture);
			auto preSprite = preRenderTexture->getSprite();
			renderTexture->update(delta, preSprite, m);
		}
		
		auto shader = renderTexture->getShader();
		if(shader && shader->getKind() == agtk::Shader::kShaderImage && shader->getUserData()) {
			auto filterEffect = dynamic_cast<agtk::data::FilterEffect *>(shader->getUserData());
			if (filterEffect != nullptr) {
				switch(filterEffect->getImagePlacement()){
				case agtk::data::FilterEffect::kPlacementCenter:{
					auto programState = shader->getProgramState();
					cocos2d::Size texSize = shader->getMaskTexture()->getContentSize();
					cocos2d::Size spriteSize = sprite->getContentSize();
					cocos2d::Vec2 sxy = cocos2d::Vec2(spriteSize.width / texSize.width, spriteSize.height / texSize.height);
					float x = sxy.x * (pos.x / spriteSize.width) + (1.0f - sxy.x) / 2.0f;
					float y = sxy.y * (-pos.y / spriteSize.height) + (1.0f - sxy.y) / 2.0f;
					programState->setUniformVec2("imgXy", cocos2d::Vec2(x, y));
					break; }
				case agtk::data::FilterEffect::kPlacementMagnify: {
					auto programState = shader->getProgramState();
					cocos2d::Size spriteSize = sprite->getContentSize();
					float x = (pos.x / spriteSize.width);
					float y = (-pos.y / spriteSize.height);
					programState->setUniformVec2("imgXy", cocos2d::Vec2(x, y));
					break; }
				case agtk::data::FilterEffect::kPlacementTiling: {
					auto texture2d = shader->getMaskTexture();
					auto programState = shader->getProgramState();
					cocos2d::Size spriteSize = sprite->getContentSize();
					cocos2d::Size texSize = texture2d->getContentSize();
					auto sxy = cocos2d::Vec2(spriteSize.width / texSize.width, spriteSize.height / texSize.height);
					float x = sxy.x * (pos.x / spriteSize.width);
					float y = sxy.y * (-pos.y / spriteSize.height);
					programState->setUniformVec2("imgXy", cocos2d::Vec2(x, y));
					break; }
				case agtk::data::FilterEffect::kPlacementKeepRatio: {
					auto programState = shader->getProgramState();
					cocos2d::Size texSize = shader->getMaskTexture()->getContentSize();
					cocos2d::Size spriteSize = sprite->getContentSize();
					auto sxy = (spriteSize.width / texSize.width <= spriteSize.height / texSize.height) ? Vec2(1, spriteSize.height / texSize.height * texSize.width / spriteSize.width) : Vec2(spriteSize.width / texSize.width * texSize.height / spriteSize.height, 1);
					float x = sxy.x * (pos.x / spriteSize.width) + (1.0f - sxy.x) / 2.0f;
					float y = sxy.y * (-pos.y / spriteSize.height) + (1.0f - sxy.y) / 2.0f;
					programState->setUniformVec2("imgXy", cocos2d::Vec2(x, y));
					break; }
				}
			}
		}
		sprite->setPosition3D(pos);
		sprite->setRotation3D(camRot3D);
		sprite->setScale(scale.x, scale.y);
	}
}

/**
* レンダータイルの存在判定の更新
* @param	objList				オブジェクトリスト
* @param	ignoreVisibleObject	オブジェクト描画除外フラグ
*/
void RenderTextureCtrl::updateIsRenderTile(cocos2d::__Array *objList, bool ignoreVisibleObject)
{
	_isRenderTile = false;

	if (!ignoreVisibleObject) {
		cocos2d::Ref *ref = nullptr;
		CCARRAY_FOREACH(objList, ref) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
			auto obj = static_cast<agtk::Object *>(ref);
#else
			auto obj = dynamic_cast<agtk::Object *>(ref);
#endif
#ifdef USE_REDUCE_RENDER_TEXTURE
			if (obj->isTransparentMaskSilhouette(Object::OVERLAP_EFFECT_TARGET_TILE)) {
				_isRenderTile = true;
				break;
			}
#endif
		}
	}
}

/**
* レンダーオブジェの存在判定の更新
* @param	maskList			オブジェクトリスト
* @param	ignoreVisibleObject	オブジェクト描画除外フラグ
*/
void RenderTextureCtrl::updateIsRenderObj(cocos2d::__Array *objList, bool ignoreVisibleObject)
{
	_isRenderObject = false;

	if (ignoreVisibleObject) {
		//後の処理でobjectSetNode等のvisibleをfalseにする。
	}
	else
	{
		cocos2d::Ref *ref = nullptr;
		CCARRAY_FOREACH(objList, ref) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
			auto obj = static_cast<agtk::Object *>(ref);
#else
			auto obj = dynamic_cast<agtk::Object *>(ref);
#endif
#ifdef USE_REDUCE_RENDER_TEXTURE
			if (obj->isTransparentMaskSilhouette(Object::OVERLAP_EFFECT_TARGET_OBJ)) {
				_isRenderObject = true;
				break;
			}
#endif
		}
	}
}

void RenderTextureCtrl::pauseShader()
{
	auto shaderList = this->getShaderList();
	cocos2d::Ref *ref;
	CCARRAY_FOREACH(shaderList, ref) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
		auto shader = static_cast<agtk::Shader *>(ref);
#else
		auto shader = dynamic_cast<agtk::Shader *>(ref);
#endif
		shader->pause();
	}
}

void RenderTextureCtrl::resumeShader()
{
	auto shaderList = this->getShaderList();
	cocos2d::Ref *ref;
	CCARRAY_FOREACH(shaderList, ref) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
		auto shader = static_cast<agtk::Shader *>(ref);
#else
		auto shader = dynamic_cast<agtk::Shader *>(ref);
#endif
		shader->resume();
	}
}

NS_AGTK_END //-----------------------------------------------------------------------------------//
