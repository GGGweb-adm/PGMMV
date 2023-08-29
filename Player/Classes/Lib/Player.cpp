#include "Player.h"
#include "Manager/GameManager.h"
#include "Manager/DebugManager.h"

#ifdef _DEBUG
//#define SHOW_DEBUG_VISIBLE // RenderTexture と原点等の位置をデバッグ表示したい場合コメントアウトを外してください
#endif
#define USE_WALL_COLLISION_FIX // 壁判定用矩形算出のFIX版を使用
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
PlayerSprite::PlayerSprite()
{
	_opacityTimer = nullptr;
	_imageId = -1;
}

PlayerSprite::~PlayerSprite()
{
	CC_SAFE_RELEASE_NULL(_opacityTimer);
}

PlayerSprite *PlayerSprite::create(int imageId, int opacity, float seconds)
{
	auto p = new (std::nothrow) PlayerSprite();
	if (p && p->init(imageId, opacity, seconds)) {
		p->autorelease();
		return p;
	}
	CC_SAFE_DELETE(p);
	return nullptr;
}

bool PlayerSprite::init(int imageId, int opacity, float seconds)
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
	this->setImageId(imageId);
	this->setOpacity(0);
	opacityTimer->setValue(opacity, seconds);
	this->setOpacityTimer(opacityTimer);
	return true;
}

void PlayerSprite::update(float delta)
{
	auto opacityTimer = this->getOpacityTimer();
	if (opacityTimer) {
		opacityTimer->update(delta);
		this->setOpacity(opacityTimer->getValue());
	}
}

//-------------------------------------------------------------------------------------------------------------------
Player::Player()
{
	_basePlayer = nullptr;
	_centerNode = nullptr;
	_originNode = nullptr;
	_autoGenerationNode = nullptr;
	_nodePlayer = nullptr;
	_visibleCtrlNode = nullptr;
	_autoGenerationCenterNode = nullptr;
	_renderTexture = nullptr;
	_removeShaderList = nullptr;
	_execActionSpriteList = nullptr;
	_animationData = nullptr;
	_type = BasePlayer::None;
	_transformNode = nullptr;
// #AGTK-NX
#if defined(USE_PREVIEW) || ((CC_TARGET_PLATFORM == CC_PLATFORM_NX) && defined(AGTK_DEBUG))
//#ifdef USE_PREVIEW
	_timelineWallNodeList = nullptr;
	_timelineHitNodeList = nullptr;
	_timelineAttackNodeList = nullptr;
	_timelineConnectNodeList = nullptr;
#endif
	_animeTimelineWallList = nullptr;
	_animeTimelineHitList = nullptr;
	_animeTimelineAttackList = nullptr;
	_animeTimelineConnectList = nullptr;
	_isContinuePlayingAnime = false;
	_nextActionNo = -1;
	_nextActionDirectionNo = -1;
	_wallAreaAttackFlag = false;
	_renderTextureSize = Size::ZERO;
	_isUpdateTransformCache = true;
	_updateTransformDirty = false;
	_playerScale = cocos2d::Vec2::ONE;
	_clippingPreAdjustNode = nullptr;
	_clippingPostAdjustNode = nullptr;
	_clippingRectangleNode = nullptr;
	_centerRotationOld = 0;
	_fromPhysics = false;
	_resourceSetId = kInvalidResourceSetId;
	_frameCount = 0;
	_stockShaderInfoFlag = false;
	_stockShaderInfoList = nullptr;
}

Player::~Player()
{
// #AGTK-NX
#if defined(USE_PREVIEW) || ((CC_TARGET_PLATFORM == CC_PLATFORM_NX) && defined(AGTK_DEBUG))
//#ifdef USE_PREVIEW
	this->removeDisplayTimelineAll();
#endif
	this->removeBasePlayer();
	//	CC_SAFE_RELEASE_NULL(_basePlayer);
	CC_SAFE_RELEASE_NULL(_centerNode);
	CC_SAFE_RELEASE_NULL(_originNode);
	CC_SAFE_RELEASE_NULL(_autoGenerationNode);
	CC_SAFE_RELEASE_NULL(_nodePlayer);
	CC_SAFE_RELEASE_NULL(_visibleCtrlNode);
	CC_SAFE_RELEASE_NULL(_autoGenerationCenterNode);
	CC_SAFE_RELEASE_NULL(_renderTexture);
	CC_SAFE_RELEASE_NULL(_removeShaderList);
	CC_SAFE_RELEASE_NULL(_execActionSpriteList);
	CC_SAFE_RELEASE_NULL(_animationData);
	CC_SAFE_RELEASE_NULL(_transformNode);
// #AGTK-NX
#if defined(USE_PREVIEW) || ((CC_TARGET_PLATFORM == CC_PLATFORM_NX) && defined(AGTK_DEBUG))
//#ifdef USE_PREVIEW
	CC_SAFE_RELEASE_NULL(_timelineWallNodeList);
	CC_SAFE_RELEASE_NULL(_timelineHitNodeList);
	CC_SAFE_RELEASE_NULL(_timelineAttackNodeList);
	CC_SAFE_RELEASE_NULL(_timelineConnectNodeList);
#endif
	CC_SAFE_RELEASE_NULL(_animeTimelineWallList);
	CC_SAFE_RELEASE_NULL(_animeTimelineHitList);
	CC_SAFE_RELEASE_NULL(_animeTimelineAttackList);
	CC_SAFE_RELEASE_NULL(_animeTimelineConnectList);

	CC_SAFE_RELEASE_NULL(_visibleCtrlNode);
	CC_SAFE_RELEASE_NULL(_clippingPreAdjustNode);
	CC_SAFE_RELEASE_NULL(_clippingPostAdjustNode);
	CC_SAFE_RELEASE_NULL(_clippingRectangleNode);
	CC_SAFE_RELEASE_NULL(_stockShaderInfoList);
}

bool Player::init(agtk::data::AnimationData *animationData)
{
	if (cocos2d::Node::init() == false) {
		CC_ASSERT(0);
		return false;
	}
	this->setAnimationData(animationData);
	_resourceSetId = animationData->getResourceSetIdList()[0];

	auto resourceInfoData = animationData->getResourceInfoData();

	// 原点ノード
	auto originNode = cocos2d::Node::create();
	if (originNode == nullptr) {
		CC_ASSERT(0);
		return false;
	}
	this->setOriginNode(originNode);

	// 中心点ノード
	auto centerNode = CenterNode::create();
	if (centerNode == nullptr) {
		CC_ASSERT(0);
		return false;
	}
	this->setCenterNode(centerNode);

	// 回転で自動生成時の初期角度設定用ノード
	auto autoGenerationNode = cocos2d::Node::create();
	if (autoGenerationNode == nullptr) {
		CC_ASSERT(0);
		return false;
	}
	this->setAutoGenerationNode(autoGenerationNode);

	// 回転で自動生成された場合の回転設定用ノード
	auto autoGenerationCenterNode = cocos2d::Node::create();
	if (autoGenerationCenterNode == nullptr) {
		CC_ASSERT(0);
		return false;
	}
	this->setAutoGenerationCenterNode(autoGenerationCenterNode);
	
	// 表示関連用ノード
	//! アニメーションフレームの回転とスケールを担当
	auto visibleCtrlNode = cocos2d::Node::create();
	if (visibleCtrlNode == nullptr) {
		CC_ASSERT(0);
		return false;
	}
	this->setVisibleCtrlNode(visibleCtrlNode);

	// 各ノードの繋がりを設定
	// this - origin - autoGenerationCenter - autoGeneration - center - visibleCtrl - (image/Gif/SpriteStudio/Spine)
	centerNode->addChild(visibleCtrlNode);
	autoGenerationNode->addChild(centerNode);
	autoGenerationCenterNode->addChild(autoGenerationNode);
	originNode->addChild(autoGenerationCenterNode);
	this->addChild(originNode);

	//シェーダー破棄リスト
	auto removeShaderList = cocos2d::__Array::create();
	if (removeShaderList == nullptr) {
		return false;
	}
	this->setRemoveShaderList(removeShaderList);

// #AGTK-NX
#if defined(USE_PREVIEW) || ((CC_TARGET_PLATFORM == CC_PLATFORM_NX) && defined(AGTK_DEBUG))
//#ifdef USE_PREVIEW
	//壁判定、当たり判定、攻撃判定、接続点 リスト生成
	this->setTimelineWallNodeList(cocos2d::__Array::create());
	this->setTimelineHitNodeList(cocos2d::__Array::create());
	this->setTimelineAttackNodeList(cocos2d::__Array::create());
	this->setTimelineConnectNodeList(cocos2d::__Array::create());
#endif
	this->setAnimeTimelineWallList(cocos2d::__Array::create());
	this->setAnimeTimelineHitList(cocos2d::__Array::create());
	this->setAnimeTimelineAttackList(cocos2d::__Array::create());
	this->setAnimeTimelineConnectList(cocos2d::__Array::create());
	//
	this->setExecActionSpriteList(cocos2d::__Array::create());

	this->setTransformNode(cocos2d::Node::create());
	this->setStockShaderInfoList(cocos2d::__Array::create());
	return true;
}

void Player::createBasePlayer(agtk::data::ResourceInfoData *resourceInfoData, int imageId)
{
	cocos2d::Node *nodePlayer = nullptr;
	auto animationData = this->getAnimationData();
	auto autoGenerationNode = this->getAutoGenerationNode();
	auto visibleCtrlNode = this->getVisibleCtrlNode();
	int biggestEdgeLength = 0;//縦横のうち大きい辺の長さ

	//リソースデータから、アニメーションリソースを取得する
	const char *playerName = "player";
	if (resourceInfoData->getImage()) {
		auto player = agtk::ImagePlayer::createWithAnimationData(animationData, resourceInfoData, imageId);
		if (player == nullptr) {
			CC_ASSERT(0);
			return;
		}
		visibleCtrlNode->addChild(player);
		nodePlayer = player;
		this->setType(BasePlayer::Image);

		// RenderTexture用のサイズを取得
		auto imageData = GameManager::getInstance()->getProjectData()->getImageData(resourceInfoData->getImageId());
		if (imageData != nullptr) {
			int width = imageData->getTexWidth() / resourceInfoData->getHDivCount();
			int height = imageData->getTexHeight() / resourceInfoData->getVDivCount();
			biggestEdgeLength = width > height ? width : height;
			visibleCtrlNode->setContentSize(Size(width, height));
		}
	}
	//gif, spine, spritestudio
	else {
		int imageId = resourceInfoData->getImageId();
		auto projectData = GameManager::getInstance()->getProjectData();
		auto animationOnlyData = projectData->getAnimationOnlyData(imageId);
		CC_ASSERT(animationOnlyData);

		// animationOnlyData の中から最も大きいサイズを biggestEdgeLength に設定する
		cocos2d::Ref *ref = nullptr;
		CCARRAY_FOREACH(animationOnlyData->getAnimationInfoList(), ref) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
			auto p = static_cast<agtk::data::AnimationOnlyData::AnimationInfoData *>(ref);
#else
			auto p = dynamic_cast<agtk::data::AnimationOnlyData::AnimationInfoData *>(ref);
#endif
			auto imageW = p->getImageWidth();
			auto imageH = p->getImageHeight();
			auto bigValue = imageW > imageH ? imageW : imageH;

			if (bigValue > biggestEdgeLength) {
				biggestEdgeLength = bigValue;
			}
		}

		switch (animationOnlyData->getDataType()) {
		case agtk::data::AnimationOnlyData::Gif: {
			auto player = agtk::GifPlayer::createWithAnimationData(animationData, resourceInfoData);
			if (player == nullptr) {
				CC_ASSERT(0);
				return;
			}

			visibleCtrlNode->addChild(player);
			nodePlayer = player;
			this->setType(BasePlayer::Gif);
			break; }
		case agtk::data::AnimationOnlyData::Spine: {
			//auto player = agtk::SpinePlayer::createWithAnimationData(animationData, resourceInfoData);
			//if (player == nullptr) {
				CC_ASSERT(0);
				return;
			//}

			//visibleCtrlNode->addChild(player);
			//nodePlayer = player;
			//this->setType(BasePlayer::Spine);
			break; }
		case agtk::data::AnimationOnlyData::SpriteStudio: {
			auto player = agtk::SSPlayer::createWithAnimationData(animationData, resourceInfoData);
			if (player == nullptr) {
				CC_ASSERT(0);
				return;
			}

			if (player->getRotation() != 0 || player->getHasRotation()) {
				visibleCtrlNode->addChild(player);
			}
			else {
				setClippingPreAdjustNode(cocos2d::Node::create());
				setClippingPostAdjustNode(cocos2d::Node::create());
				setClippingRectangleNode(cocos2d::ClippingRectangleNode::create());
				_clippingRectangleNode->setAnchorPoint(cocos2d::Vec2(0, 0));

#if 0//def AGTK_DEBUG	//クリッピング矩形の視覚化
				auto drawNode = DrawNode::create();
				auto color = Color4F(1.0f, 0, 0, 0.25f);
				drawNode->setAnchorPoint(cocos2d::Vec2(0.5, 0.5));
				drawNode->drawSolidRect(cocos2d::Vec2(0, 0), cocos2d::Vec2(4096, 4096), color);
				drawNode->setPosition(0, 0);
				_clippingRectangleNode->addChild(drawNode);
#endif

				_clippingPostAdjustNode->addChild(player);
				_clippingRectangleNode->addChild(_clippingPostAdjustNode);
				_clippingPreAdjustNode->addChild(_clippingRectangleNode);
				visibleCtrlNode->addChild(_clippingPreAdjustNode);
			}
			nodePlayer = player;
			this->setType(BasePlayer::SpriteStudio);
			break; }
		default:
			CC_ASSERT(0);
			return;
		}
	}

	//node player
	if (nodePlayer == nullptr) {
		CC_ASSERT(0);
		return;
	}
	this->setNodePlayer(nodePlayer);
	//base player
	_basePlayer = dynamic_cast<agtk::BasePlayer *>(nodePlayer);
	if (_basePlayer == nullptr) {
		CC_ASSERT(0);
		return;
	}

	// basePlayerにオブジェクト情報を保存
	agtk::Object * _object = dynamic_cast<agtk::Object *>(this->getParent());
	if (_object != nullptr) {
		_basePlayer->setObjectNode(_object);
	}

	//! create rendertexture
	{
		// biggestEdgeLength が2のべき乗でない場合
		//! 注：RenderTextureは２のべき乗である必要がある
		if ((biggestEdgeLength & (biggestEdgeLength - 1))) {
			// 最も近い2のべき乗サイズを求める
			int value = 0x1;
			for (int val = (int)biggestEdgeLength; val > 0; val = val >> 1) {
				value = value << 1;
			}
			biggestEdgeLength = value;
		}

		auto renderTextureSize = cocos2d::Size(biggestEdgeLength, biggestEdgeLength);
		this->setRenderTextureSize(renderTextureSize);
		//SpriteStudioで回転がある場合は、ahaderでキャンバスの大きさでマスクする。
		if (this->getType() == BasePlayer::SpriteStudio) {
			//Mask使用時にDepthBuffer, StencilBufferが必要なため、当面、常にRenderTextureを確保するように。
			//if (this->getRotation() != 0 || _basePlayer->getHasRotation()) {
				createRenderTexture();
			//}
		}
	}

	{
		//! --------------------------------------------------------------
		// basePlayer のオフセット移動・スケール・回転のイベント登録
		auto centerNode = this->getCenterNode();
		_basePlayer->onSetPosition = [this, centerNode](float x, float y) {
			// ※ACT2-2642 上右部が１ドット途切れるバグの対応。とりあえず小数点以下を切り捨てにすると上右部が現れるため。
			centerNode->setPosition((int)x, (int)y);
			centerNode->setAnimePositionFlag(true);
			this->setCenterPosition(cocos2d::Vec2(x, y));
		};
		_basePlayer->onSetScale = [visibleCtrlNode](float x, float y) {
			visibleCtrlNode->setScale(x, y);
		};
		_basePlayer->onSetRotation = [visibleCtrlNode](float rotation) {
			visibleCtrlNode->setRotation(rotation);
		};

		// visibleCtrlNode を _basePlayer の参照用メンバに設定
		_basePlayer->_refVisibleCtrlNode = visibleCtrlNode;
	}
	
	//変換ノード
	this->getNodePlayer()->addChild(this->getTransformNode());

	// モーションが変更されて矩形が変わる可能性があるのでTransformCacheを更新可能にする
	_isUpdateTransformCache = true;
	_updateTransformDirty = false;
	_frameCount = 0;

#ifdef SHOW_DEBUG_VISIBLE
	//! デバッグ表示
	{
		//! [DEBUG] RenderTextureの範囲表示
		auto rtc = this->getRenderTexture();
		if (rtc) {
			((agtk::RenderTexture *)rtc->getRenderTextureList()->getObjectAtIndex(0))->setClearColor(Color4F(0, 1, 0, 0.5f));
		}

		//! [DEBUG] OriginNode の位置表示
		auto origin_dot = DrawNode::create();
		origin_dot->drawDot(Vec2::ZERO, 5, Color4F(Color3B::BLUE, 1));
		_originNode->addChild(origin_dot);

		//! [DEBUG] AutoGenerationNode の位置表示
		auto autogen_dot = DrawNode::create();
		autogen_dot->drawDot(Vec2::ZERO, 6, Color4F(Color3B::MAGENTA, 1));
		_autoGenerationNode->addChild(autogen_dot);

		//! [DEBUG] CenterNode の位置表示
		auto center_dot = DrawNode::create();
		center_dot->drawDot(Vec2::ZERO, 3, Color4F(Color3B::YELLOW, 1));
		_centerNode->addChild(center_dot);

		//! [DEBUG] VisibleCtrlNode の位置表示
		auto visible_ctrl_dot = DrawNode::create();
		visible_ctrl_dot->drawDot(Vec2::ZERO, 4, Color4F(Color3B::RED, 1));
		_visibleCtrlNode->addChild(visible_ctrl_dot);

		//! [DEBUG] transformNode の位置表示
		auto transform_dot = DrawNode::create();
		transform_dot->drawDot(Vec2::ZERO, 2, Color4F(Color3B::GREEN, 1));
		this->getTransformNode()->addChild(transform_dot);
	}
#endif
}

void Player::removeBasePlayer()
{
	//shader (sprite studio)
	if (this->getType() == BasePlayer::SpriteStudio) {
		auto shader = this->getShader(agtk::Shader::kShaderAlphaMask);
		if (shader) shader->resetMaskTexture();
	}
	this->removeAllShader();
	auto removeShaderList = this->getRemoveShaderList();
	//remove shader
	if (removeShaderList->count() > 0) {
		auto renderTextureCtrl = this->getRenderTexture();
		if (renderTextureCtrl != nullptr) {
			cocos2d::Ref *ref;
			CCARRAY_FOREACH(removeShaderList, ref) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
				auto shader = static_cast<agtk::Shader *>(ref);
#else
				auto shader = dynamic_cast<agtk::Shader *>(ref);
#endif
				renderTextureCtrl->removeShader(shader->getKind());
			}
		}
		removeShaderList->removeAllObjects();
	}

	//remove renderTexture
	auto renderTextureCtrl = this->getRenderTexture();
	if (renderTextureCtrl) {
		this->removeChild(renderTextureCtrl);
		this->setRenderTexture(nullptr);
	}

	//remove basePlayer
	auto nodePlayer = this->getNodePlayer();
	auto transformNode = this->getTransformNode();
	if (nodePlayer && transformNode) {
		nodePlayer->removeChild(transformNode);
	}

	this->getVisibleCtrlNode()->removeChild(nodePlayer);
	this->setNodePlayer(nullptr);
	_basePlayer = nullptr;
}

/**
* Player の更新
* @param	dt	前フレームからの経過時間
*/
void Player::update(float dt)
{
	cocos2d::Mat4 m = cocos2d::Mat4::IDENTITY;
	//SpriteStudio用の変換マトリクスの準備。
	if (this->getType() == BasePlayer::SpriteStudio) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
		auto ssplayer = static_cast<agtk::SSPlayer *>(this->getNodePlayer());
#else
		auto ssplayer = dynamic_cast<agtk::SSPlayer *>(this->getNodePlayer());
#endif
		ssplayer->setNodeToWorldTransformFlag(false);
		auto renderTexture = this->getRenderTexture();
		if (renderTexture) {
			auto renderTexSize = renderTexture->getContentSize();
			auto rect = ssplayer->getAnimationOnlyRect();
			float x = (rect.size.width - rect.origin.x);
			float y = (rect.size.height - rect.origin.y) + rect.size.height;
			m.translate(Vec3(x, y, 0));
			ssplayer->setNodeToWorldTransform(m * ssplayer->getNodeToParentTransform());
		}
	}

	//update node
//#AGTK-NX #AGTK-WIN
#ifdef USE_SAR_OPTIMIZE_1
	struct NodeInfo
	{
		int		i;
		Node*	parent;
		Node*	node;
	};

	std::vector<NodeInfo> updateNodeStack;
	updateNodeStack.reserve(20);
	{
		auto* children = &this->getChildren();
		if(0 < this->getChildrenCount()) {
			NodeInfo info;
			info.i = 0;
			info.parent = this;
			info.node = children->at(0);
			updateNodeStack.push_back(info);
		}

		while (!updateNodeStack.empty())
		{
			auto it = updateNodeStack.end()-1;
			Node* curNode = it->node;

			auto renderTexture = dynamic_cast<agtk::RenderTexture *>(curNode);
			if (!renderTexture)
			{
				// NN_LOG("node: %d, %d, %d\n", curNode->_ID, it->i, it->parent->getChildrenCount());
				curNode->update(dt);

				if (++(it->i) >= it->parent->getChildrenCount())
				{
					updateNodeStack.pop_back();
				}
				else {
					it->node = it->parent->getChildren().at(it->i);
				}

				children = &curNode->getChildren();
				if (0 < curNode->getChildrenCount()) {
					NodeInfo childinfo;
					childinfo.i = 0;
					childinfo.parent = curNode;
					childinfo.node = children->at(0);
					updateNodeStack.push_back(childinfo);
				}
			}
			else
			{
				if (++(it->i) >= it->parent->getChildrenCount())
				{
					updateNodeStack.pop_back();
				}
				else {
					it->node = it->parent->getChildren().at(it->i);
				}
			}
		}
	}
#else
	std::function<void(cocos2d::Node *)> updateChildren = [&](cocos2d::Node *node) {
		auto children = node->getChildren();
		for (int i = 0; i < node->getChildrenCount(); i++) {
			auto child = children.at(i);
			auto renderTexture = dynamic_cast<agtk::RenderTexture *>(child);
			if (renderTexture) {
				continue;
			}
			child->update(dt);
			updateChildren(child);
		}
	};
	updateChildren(this);
#endif

	//remove shader
	if (this->getRemoveShaderList()->count()) {
		auto renderTextureCtrl = this->getRenderTexture();
		auto removeShaderList = this->getRemoveShaderList();
		if (renderTextureCtrl) {
			cocos2d::Ref *ref = nullptr;
			CCARRAY_FOREACH(removeShaderList, ref) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
				auto shader = static_cast<agtk::Shader *>(ref);
#else
				auto shader = dynamic_cast<agtk::Shader *>(ref);
#endif
				renderTextureCtrl->removeShader(shader->getKind());
			}
		}
		removeShaderList->removeAllObjects();
	}

	cocos2d::Size contentSize;
	if (this->getNodePlayer() != nullptr) {
		contentSize = this->getNodePlayer()->getContentSize();
		//set size
		this->setContentSize(contentSize);
	}

	//update center position
	this->getCenterNode()->update(dt);

	//upper sprite
	auto execActionSpriteList = this->getExecActionSpriteList();
	cocos2d::Ref *ref;
	CCARRAY_FOREACH(execActionSpriteList, ref) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
		auto sprite = static_cast<agtk::PlayerSprite *>(ref);
#else
		auto sprite = dynamic_cast<agtk::PlayerSprite *>(ref);
#endif
		sprite->update(dt);
		auto opacity = sprite->getOpacityTimer();
		if ((opacity->getState() == agtk::EventTimer::kStateEnd || opacity->getState() == agtk::EventTimer::kStateIdle) && opacity->getValue() == 0.0f) {
			execActionSpriteList->removeObject(sprite);
			this->getOriginNode()->removeChild(sprite);
			break;
		}
	}

	// update renderTexture
	auto renderTexture = this->getRenderTexture();
	if (renderTexture) {
		auto renderTexSize = renderTexture->getContentSize();
		if (this->getType() == BasePlayer::SpriteStudio) {
			//計算済み。
		}
		else {
			//小数点を含むとテクスチャがゆがむため、小数点切り捨てして整数とする。
			int tx = _basePlayer->_leftupAnchorPoint.x * contentSize.width + (renderTexSize.width - contentSize.width) * 0.5f;
			int ty = _basePlayer->_leftupAnchorPoint.y * contentSize.height + (renderTexSize.height - contentSize.height) * 0.5f;
			m.translate(Vec3(tx, ty, 0));
		}
		renderTexture->update(dt, &m);
	}

// #AGTK-NX
#if defined(USE_PREVIEW) || ((CC_TARGET_PLATFORM == CC_PLATFORM_NX) && defined(AGTK_DEBUG))
//#ifdef USE_PREVIEW
	//wallArea, attackArea, hitArea, connection
	this->updateDisplayTimeline(agtk::data::TimelineInfoData::EnumTimelineType::kTimelineWall);
	this->updateDisplayTimeline(agtk::data::TimelineInfoData::EnumTimelineType::kTimelineHit);
	this->updateDisplayTimeline(agtk::data::TimelineInfoData::EnumTimelineType::kTimelineAttack);
	this->updateDisplayTimeline(agtk::data::TimelineInfoData::EnumTimelineType::kTimelineConnection);
#endif

	_isUpdateTransformCache = true;
	_updateTransformDirty = false;
	_frameCount++;
}

cocos2d::Vec2 Player::calcAnchorPoint(cocos2d::Size& size)
{
	cocos2d::Vec2 anchor = cocos2d::Vec2::ZERO;
	if (this->getBasePlayer() == nullptr) {
		return anchor;
	}
	auto animationData = this->getBasePlayer()->getAnimationData();
	switch (animationData->getOriginType()) {
	case agtk::data::AnimationData::kOriginLeftUp:
		anchor.x = 0.5f;
		anchor.y = -0.5f;
		break;
	case agtk::data::AnimationData::kOriginCenter:
		anchor.x = 0.0f;
		anchor.y = 0.0f;
		break;
	case agtk::data::AnimationData::kOriginFoot:
		anchor.x = 0.0f;
		anchor.y = 0.5f;
		break;
	case agtk::data::AnimationData::kOriginXy:
		anchor.x = 0.5f - animationData->getOriginX() / size.width;
		anchor.y = -(0.5f - animationData->getOriginY() / size.height);
		break;
	default:CC_ASSERT(0);
	}
	return anchor;
}

void Player::play(int actionNo, int actionDirectNo, bool bTakeoverFrames, bool bIgnoredSound)
{
	if (actionNo < 0) {
		//remove
// #AGTK-NX
#if defined(USE_PREVIEW) || ((CC_TARGET_PLATFORM == CC_PLATFORM_NX) && defined(AGTK_DEBUG))
//#ifdef USE_PREVIEW
		this->removeDisplayTimelineAll();
#endif
		this->removeBasePlayer();
		return;
	}
	// アニメ継続再生を行っていない場合のみ再生処理を行う
	if (!_isContinuePlayingAnime) {
		if (this->compareResourceInfo(actionNo, actionDirectNo)) {
			//remove
// #AGTK-NX
#if defined(USE_PREVIEW) || ((CC_TARGET_PLATFORM == CC_PLATFORM_NX) && defined(AGTK_DEBUG))
//#ifdef USE_PREVIEW
			this->removeDisplayTimelineAll();
#endif
			//シェーダー実行中の場合は、次のPlayerへ引き継ぐ。
			auto shaderInfoList = cocos2d::__Array::create();
			for (int i = 0; i < Shader::kShaderMax; i++) {
				auto shaderKind = (Shader::ShaderKind)i;
				if (shaderKind == Shader::kShaderAlphaMask) continue;
				auto shader = this->getShader(shaderKind);
				if (shader != nullptr) {
					auto shaderInfo = ShaderInfo::create(shader);
					shaderInfoList->addObject(shaderInfo);
				}
			}

			this->removeBasePlayer();

			auto animationData = this->getAnimationData();
			int resourceInfoId = this->getResourceInfoId(actionNo, actionDirectNo);
			auto resourceInfoData = animationData->getResourceInfoData(resourceInfoId);
			//create
			auto imageId = resourceInfoData->getImageIdByResourceSetId(_resourceSetId);
			this->createBasePlayer(resourceInfoData, imageId);

			//実行中のシェーダーを再設定。
			if (shaderInfoList->count() > 0) {
				for (int i = 0; i < shaderInfoList->count(); i++) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
					auto shaderInfo = static_cast<ShaderInfo *>(shaderInfoList->getObjectAtIndex(i));
#else
					auto shaderInfo = dynamic_cast<ShaderInfo *>(shaderInfoList->getObjectAtIndex(i));
#endif
					auto shader = this->getShader(shaderInfo->getKind());
					if (shader == nullptr) {
						shader = this->setShader(shaderInfo->getKind(), shaderInfo->getShaderValue());
					}
					if (shader != nullptr) {
						shaderInfo->apply(shader);
					}
				}
			}
		}

		bool bFirstUpdate = false;
		if (_basePlayer != nullptr) {
			_nextActionNo = -1;
			_nextActionDirectionNo = -1;
			auto motion = _basePlayer->getCurrentAnimationMotion();
			float seconds = 0.0f;
			bool reverse = false;
			if (bTakeoverFrames && motion) {
				//モーションIDが同じ場合は、現在のモージョンのフレームを引き継ぐ。
				auto motionId = motion->getMotionData()->getId();
				if (motionId == actionNo) {
					seconds = motion->_seconds;
					reverse = motion->_bReverse;
				}
			}
			_basePlayer->play(actionNo, actionDirectNo, seconds, bIgnoredSound, reverse);

			//set size
			auto contentSize = this->getNodePlayer()->getContentSize();
			this->setContentSize(contentSize);
			bFirstUpdate = true;
		}

// #AGTK-NX
#if defined(USE_PREVIEW) || ((CC_TARGET_PLATFORM == CC_PLATFORM_NX) && defined(AGTK_DEBUG))
//#ifdef USE_PREVIEW
		// タイムライン表示生成
		this->createDisplayTimeline();
		this->setVisibelTimelineAll(false);
#endif
		// アニメタイムラインリスト生成
		this->createAnimationTimelineList();

		//座標を確定するためにここで一度更新しておく。
		if (bFirstUpdate) {
			this->update(0.0f);
		}
	}
	else {
		// 継続再生終了時に次のアニメを再生できるように引数を退避させる
		_nextActionNo = actionNo;
		_nextActionDirectionNo = actionDirectNo;
	}
}

void Player::continuePlayCurrentAnime()
{
	_isContinuePlayingAnime = true;
}

void Player::stopAnimeContinuePlaying()
{
	_isContinuePlayingAnime = false;

	// 退避させた引数をもとに次に再生するアニメを設定する
	if (_nextActionNo >= 0 && _nextActionDirectionNo >= 0) {
		play(_nextActionNo, _nextActionDirectionNo);
	}
}

cocos2d::Vec2 Player::addPosition(const cocos2d::Vec2 add)
{
	auto pos = this->getPosition();
	this->setPosition(pos + add);
	return pos;
}

void Player::setPosition(const Vec2 &position, bool fromPhysics /* = false*/)
{
	this->setPosition(position.x, position.y, fromPhysics);
}

/**
 * @brief アニメプレイヤーの位置を更新する。fromPhysics==trueのときはNodeの表示位置が更新されない。位置が変化したときにトランスフォームの更新フラグを立てる。
 */
void Player::setPosition(float x, float y, bool fromPhysics /* = false*/)
{
	auto oldPos = Node::getPosition();
	auto oldRealPos = _realPosition;

	_realPosition.x = x;
	_realPosition.y = y;
	_fromPhysics = fromPhysics;

	//物理演算処理による位置の場合は四捨五入
	if (fromPhysics) {
		x = std::roundf(x);
		y = std::roundf(y);
	}
	//浮動小数点切り捨て
	x = (int)x;
	y = (int)y;

	Node::setPosition(x, y);

	if (oldRealPos.x != _realPosition.x || oldRealPos.y != _realPosition.y) {
		_isUpdateTransformCache = true;
		_updateTransformDirty = false;
	}
}

/**
 * @brief アニメプレイヤーの位置を返す。Nodeの表示位置とは限らない。
 */
const cocos2d::Vec2& Player::getPosition() const
{
	return _realPosition;
}

void Player::setRotation(float rotation)
{
	auto originNode = this->getOriginNode();
	originNode->setRotation(rotation);
	if (this->getType() == BasePlayer::SpriteStudio) {
		if (rotation != 0 && !_renderTexture) {
			createRenderTexture();
		}
	}
}

float Player::addRotation(float rotation)
{
	float localRotation = this->getRotation();
	this->setRotation(localRotation + rotation);
	return localRotation;
}

float Player::getRotation() const
{
	auto originNode = this->getOriginNode();
	return originNode->getRotation();
}

void Player::setCenterRotation(float rotation)
{
	//回転で自動生成オンの時、ベクトルの原点が通常上向きから右向きとなるため、90度マイナスとする。
	float adjustRotation = 0.0f;
	if (this->isAutoGeneration()) {
		auto projectData = GameManager::getInstance()->getProjectData();
		adjustRotation = (projectData->getGameType() == agtk::data::ProjectData::kGameTypeSideView) ? -90 : -180;
	}
	auto autoGenerationNode = this->getAutoGenerationNode();
	float rot = rotation + adjustRotation;
	rot = GetDegree360(rot);

	auto nowCenterRotation = this->getCenterRotation();
	auto nextCenterRotation = GetDegree360(rot - adjustRotation);;
	if (nextCenterRotation != nowCenterRotation) {
		_centerRotationOld = nowCenterRotation;
	}
	autoGenerationNode->setRotation(rot);
	// ACT2-6406対応 回転で自動生成アニメーションの初フレームで反転がおかしくなるのを修正
	setScale(getScaleX(), getScaleY());
}

float Player::getCenterRotation()
{
	auto autoGenerationNode = this->getAutoGenerationNode();
	float rot = autoGenerationNode->getRotation();
	float adjustRotation = 0.0f;
	if (this->isAutoGeneration()) {
		auto projectData = GameManager::getInstance()->getProjectData();
		adjustRotation = (projectData->getGameType() == agtk::data::ProjectData::kGameTypeSideView) ? -90 : -180;
	}
	rot = GetDegree360(rot - adjustRotation);
	return  rot;
}

void Player::setScale(float x, float y)
{
	auto autoGenerationNode = this->getAutoGenerationNode();
	if (autoGenerationNode != nullptr) {
		auto rotation = this->getCenterRotation();
		float sx = x;
		float sy = y;
		if (GameManager::getInstance()->getProjectData()->getGameType() == agtk::data::ProjectData::kGameTypeSideView) {
			// サイドビュー
#if 1
			// ACT2-6361 反転が無視される問題の対応
			if (this->isAutoGeneration() && this->isFlipY() && (rotation > 180.0f)) {
				sy = -1.0f * y;
			}
#endif
#if 0
			if (_centerRotationOld > 0 && _centerRotationOld < 180) {
				// 直前まで右側方向が入っていた場合、表示方向でいう所の1、4、6のみ反転させる
				if (this->isAutoGeneration() && this->isFlipY() && (225.0f <= rotation && rotation <= 315.0f)) {
					sy = -1.0f * y;
				}
			}
			else {
				// 直前まで左側方向が入っていた場合、表示方向でいう所の1、2、4、6、7を反転させる
				if (this->isAutoGeneration() && this->isFlipY() && (0.0f >= rotation || rotation >= 180.0f)) {
					sy = -1.0f * y;
				}
			}
#endif
		}
		else {
			// トップビュー
			if (this->isAutoGeneration() && this->isFlipY() && (0.0f <= rotation && rotation <= 180.0f)) {
				sx = -1.0f * x;
			}
		}
		autoGenerationNode->setScale(sx, sy);
	}
	this->setPlayerScale(cocos2d::Vec2(x, y));
}

float Player::getScaleX() const
{
	return this->getPlayerScale().x;
}

float Player::getScaleY() const
{
	return this->getPlayerScale().y;
}

float Player::getAutoGenerationRotation()
{
	auto node = this->getAutoGenerationNode();
	return node->getRotation();
}

bool Player::isAutoGeneration()
{
	auto basePlayer = this->getBasePlayer();
	if (basePlayer == nullptr) {
		return false;
	}
	auto currentAnimationMotion = basePlayer->getCurrentAnimationMotion();
	if (currentAnimationMotion != nullptr) {
		auto currentDirection = currentAnimationMotion->getCurrentDirection();
		if (currentDirection != nullptr) {
			auto directionData = currentDirection->getDirectionData();
			if (directionData != nullptr) {
				return directionData->getAutoGeneration();
			}
			else {
				return false;
			}
		}
		else {
			return false;
		}
	}
	return false;
}

bool Player::isFlipY()
{
	auto basePlayer = this->getBasePlayer();
	if (basePlayer == nullptr) {
		return false;
	}
	auto currentAnimationMotion = basePlayer->getCurrentAnimationMotion();
	if (currentAnimationMotion != nullptr) {
		auto currentDirection = currentAnimationMotion->getCurrentDirection();
		if (currentDirection != nullptr) {
			auto directionData = currentDirection->getDirectionData();
			if (directionData != nullptr) {
				return directionData->getYFlip();
			}
			else {
				return false;
			}
		}
		else {
			return false;
		}
	}
	return false;
}

cocos2d::Vec2 Player::getDispPosition()
{
	return Node::getPosition();
}

/**
* 中心点の座標を取得
* @return	中心点座標
*/
cocos2d::Vec2 Player::getCenterNodePosition()
{
	//スプライトスタジオ用。
	if (this->getType() == BasePlayer::SpriteStudio) {
		return this->getCenterPositionForSpriteStudio();
	}
	auto autoGenerationNode = this->getAutoGenerationNode();
	float angle = autoGenerationNode->getRotation();
	cocos2d::Vec2 v = this->getCenterPosition();
	return _realPosition + v.rotateByAngle(cocos2d::Vec2::ZERO, CC_DEGREES_TO_RADIANS(angle));
}

cocos2d::Vec2 Player::getCenterNodePosition2()
{
	auto autoGenerationNode = this->getAutoGenerationNode();
	float angle = autoGenerationNode->getRotation();
	auto offset = getCenterOffset();
	auto center = getCenterPosition();
	return _realPosition + offset.rotateByAngle(center, CC_DEGREES_TO_RADIANS(angle));
}

cocos2d::Vec2 Player::getCenterOffset()
{
	cocos2d::Vec2 offset = cocos2d::Vec2::ZERO;
	if (this->getBasePlayer() == nullptr) {
		return offset;
	}
	auto size = this->getContentSize();
	auto anchorPoint = this->calcAnchorPoint(size);
	return cocos2d::Vec2(anchorPoint.x * size.width, anchorPoint.y * size.height);
}

cocos2d::Vec2 Player::getCenterPositionForSpriteStudio()
{
	auto basePlayer = this->getBasePlayer();
	cocos2d::Vec2 center = cocos2d::Vec2::ZERO;
	cocos2d::Vec2 leftup = cocos2d::Vec2::ZERO;
	cocos2d::Vec2 offset = cocos2d::Vec2::ZERO;
	float rotation = 0.0f;
	if (basePlayer != nullptr) {
		rotation = basePlayer->getInnerRotation();
		center = this->getBasePlayer()->getCenter();
		auto size = this->getContentSize();
		auto animationData = basePlayer->getAnimationData();
		switch (animationData->getOriginType()) {
		case agtk::data::AnimationData::kOriginLeftUp:
			leftup.x = 0.0f * size.width;
			leftup.y = 0.0f * size.height;
			break;
		case agtk::data::AnimationData::kOriginCenter:
			leftup.x = -0.5f * size.width;
			leftup.y = 0.5f * size.height;
			break;
		case agtk::data::AnimationData::kOriginFoot:
			leftup.x = -0.5f * size.width;
			leftup.y = 1.0f * size.height;
			break;
		case agtk::data::AnimationData::kOriginXy:
			leftup.x = -animationData->getOriginX();
			leftup.y = animationData->getOriginY();
			break;
		default:CC_ASSERT(0);
		}
		offset = basePlayer->getOffset();
	}
	auto centerPoint = leftup + cocos2d::Vec2(center.x, -center.y);
	auto value = centerPoint.rotateByAngle(cocos2d::Vec2::ZERO, CC_DEGREES_TO_RADIANS(-rotation));
	return _realPosition + value + cocos2d::Vec2(offset.x, -offset.y);
}

agtk::Shader *Player::setShader(Shader::ShaderKind kind, float value, float seconds)
{
	return this->setShader(kind, value, seconds, 0.0f);
}

agtk::Shader *Player::setShader(Shader::ShaderKind kind, float value, float seconds, float timer)
{
	if (this->getNodePlayer() == nullptr) {
		return nullptr;
	}
	// RenderTexture が無い場合
	if (this->getRenderTexture() == nullptr) {
		createRenderTexture();
	}

	CC_ASSERT(this->getRenderTexture());
	agtk::Shader *shader = this->getShader(kind);
	if (shader == nullptr) {
		shader = this->getRenderTexture()->addShader(kind, value);
		CC_ASSERT(shader);
	}
	else {
		if (_stockShaderInfoFlag) {
			if (kind != Shader::kShaderAlphaMask) {
				auto shaderInfo = ShaderInfo::create(shader);
				this->getStockShaderInfoList()->addObject(shaderInfo);
			}
		}
	}
	shader->setIntensity(value, seconds);
	shader->setIgnored(false);//有効にする。
	shader->getValue()->setTimer(timer);
	return shader;
}

agtk::Shader *Player::setShader(Shader::ShaderKind kind, agtk::ShaderValue *shaderValue)
{
	if (this->getNodePlayer() == nullptr) {
		return nullptr;
	}
	// RenderTexture が無い場合
	if (this->getRenderTexture() == nullptr) {
		createRenderTexture();
	}

	CC_ASSERT(this->getRenderTexture());
	agtk::Shader *shader = this->getShader(kind);
	if (shader == nullptr) {
		shader = this->getRenderTexture()->addShader(kind, shaderValue->getNextValue());
		CC_ASSERT(shader);
	}
	else {
		if (_stockShaderInfoFlag) {
			if (kind != Shader::kShaderAlphaMask) {
				auto shaderInfo = ShaderInfo::create(shader);
				this->getStockShaderInfoList()->addObject(shaderInfo);
			}
		}
	}
	*shader->getValue() = *shaderValue;
	return shader;
}

agtk::Shader *Player::setShader(Shader::ShaderKind kind, const rapidjson::Value &shaderData)
{
	if (this->getNodePlayer() == nullptr) {
		return nullptr;
	}
	// RenderTexture が無い場合
	if (this->getRenderTexture() == nullptr) {
		createRenderTexture();
	}

	float value = shaderData["value"].GetDouble();
	float seconds = shaderData["seconds"].GetDouble();
	CC_ASSERT(this->getRenderTexture());
	agtk::Shader *shader = this->getShader(kind);
	if (shader == nullptr) {
		shader = this->getRenderTexture()->addShader(kind, value);
		CC_ASSERT(shader);
	}
	else {
		if (_stockShaderInfoFlag) {
			if (kind != Shader::kShaderAlphaMask) {
				auto shaderInfo = ShaderInfo::create(shader);
				this->getStockShaderInfoList()->addObject(shaderInfo);
			}
		}
	}
	shader->setIntensity(value, seconds);
	shader->getValue()->setJsonData(shaderData);
	shader->setIgnored(shaderData["ignored"].GetBool());
	return shader;
}


void Player::removeShader(Shader::ShaderKind kind, float seconds)
{
	auto renderTextureCtrl = this->getRenderTexture();
	if (renderTextureCtrl == nullptr) {
		return;
	}
	auto shader = this->getShader(kind);
	if (shader == nullptr) {
		return;
	}

	//一時保持中のシェーダーがある場合は復活するように。
	auto stockShaderInfoList = this->getStockShaderInfoList();
	if(stockShaderInfoList->count() > 0) {
		cocos2d::Ref *ref;
		CCARRAY_FOREACH(stockShaderInfoList, ref) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
			auto shaderInfo = static_cast<ShaderInfo *>(ref);
#else
			auto shaderInfo = dynamic_cast<ShaderInfo *>(ref);
#endif
			if (shaderInfo->getKind() == kind) {
				shaderInfo->apply(shader);
				stockShaderInfoList->removeObject(shaderInfo);
				return;
			}
		}
	}
	
	shader->setIntensity(0.0f, seconds);
	auto value = shader->getValue();
	if (seconds == 0.0f) {
		this->getRemoveShaderList()->addObject(shader);
	}
	else {
		value->setEndFunc([&, shader]() {
			this->getRemoveShaderList()->addObject(shader);
		});
	}
}

void Player::removeAllShader()
{
	this->getStockShaderInfoList()->removeAllObjects();
	for (int i = 0; i < Shader::kShaderMax; i++) {
		Shader::ShaderKind shaderKind = (Shader::ShaderKind)i;
		if (this->getShader(shaderKind) == nullptr) {
			continue;
		}
		this->removeShader(shaderKind);
	}
}

agtk::Shader *Player::getShader(Shader::ShaderKind kind)
{
	auto renderTextureCtrl = this->getRenderTexture();
	if (renderTextureCtrl == nullptr) {
		return nullptr;
	}

	return this->getRenderTexture()->getShader(kind);
}

void Player::createRenderTexture()
{
	if (_renderTexture) {
		return;
	}
	if (_clippingRectangleNode) {
		_clippingPostAdjustNode->removeChild(_nodePlayer, false);
		_visibleCtrlNode->addChild(_nodePlayer);
		_visibleCtrlNode->removeChild(_clippingRectangleNode);
		CC_SAFE_RELEASE_NULL(_clippingPreAdjustNode);
		CC_SAFE_RELEASE_NULL(_clippingRectangleNode);
		CC_SAFE_RELEASE_NULL(_clippingPostAdjustNode);
	}

	auto nodePlayer = this->getNodePlayer();
	auto renderTextureSize = this->getRenderTextureSize();
	agtk::RenderTextureCtrl *renderTextureCtrl;
	if (this->getType() == BasePlayer::SpriteStudio) {
		renderTextureCtrl = agtk::RenderTextureCtrl::create(nodePlayer, renderTextureSize, 0, true);
	} else {
		renderTextureCtrl = agtk::RenderTextureCtrl::create(nodePlayer, renderTextureSize);
	}
	this->addChild(renderTextureCtrl);
	this->setRenderTexture(renderTextureCtrl);
	if (this->getType() == BasePlayer::SpriteStudio) {
		auto shader = this->setShader(agtk::Shader::kShaderAlphaMask, 1.0f);

// #AGTK-NX
#ifdef STATIC_DOWN_CAST
		auto ssplayer = static_cast<agtk::SSPlayer *>(nodePlayer);
#else
		auto ssplayer = dynamic_cast<agtk::SSPlayer *>(nodePlayer);
#endif
		CC_ASSERT(ssplayer);
		ssplayer->createTexture2D(renderTextureSize.width, renderTextureSize.height);
		auto texture2d = ssplayer->getTexture2D();
		shader->setMaskTexture(texture2d);
	}
}

void Player::setExecActionSprite(int imageId, int opacity, float seconds)
{
	auto execActionSpriteList = this->getExecActionSpriteList();
	cocos2d::Ref *ref;
	CCARRAY_FOREACH(execActionSpriteList, ref) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
		auto sprite = static_cast<agtk::PlayerSprite *>(ref);
#else
		auto sprite = dynamic_cast<agtk::PlayerSprite *>(ref);
#endif
		if (sprite->getImageId() == imageId) {
			return;
		}
	}
	auto sprite = agtk::PlayerSprite::create(imageId, opacity, seconds);
	this->getExecActionSpriteList()->addObject(sprite);
	this->getOriginNode()->addChild(sprite);
}

void Player::removeExecActionSprite(float seconds)
{
	auto execActionSpriteList = this->getExecActionSpriteList();
	cocos2d::Ref *ref;
	CCARRAY_FOREACH(execActionSpriteList, ref) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
		auto sprite = static_cast<agtk::PlayerSprite *>(ref);
#else
		auto sprite = dynamic_cast<agtk::PlayerSprite *>(ref);
#endif
		sprite->getOpacityTimer()->setValue(0, seconds);
	}
}

bool Player::compareResourceInfo(int actionNo, int actionDirectNo)
{
	return this->getResourceInfoId(actionNo, actionDirectNo) != this->getCurrentResourceInfoId();
}

int Player::getResourceInfoId(int actionNo, int actionDirectNo)
{
	auto animationData = this->getAnimationData();
	auto directionData = animationData->getDirectionData(actionNo, actionDirectNo);
	return directionData ? directionData->getResourceInfoId() : -1;
}

int Player::getCurrentResourceInfoId()
{
	if (_basePlayer == nullptr) {
		return -1;
	}
	auto motion = _basePlayer->getCurrentAnimationMotion();
	if (motion == nullptr) {
		return -1;
	}
	auto direction = motion->getCurrentDirection();
	if (direction) {
		auto directionData = direction->getDirectionData();
		if (directionData) {
			return directionData->getResourceInfoId();
		}
	}
	return -1;
}

int Player::getCurrentActionNo()
{
	int actionNo = -1;
	CC_ASSERT(_basePlayer);
	auto motion = _basePlayer->getCurrentAnimationMotion();
	if (motion) {
		actionNo = motion->getMotionData()->getId();
	}
	return actionNo;
}

int Player::getCurrentDirectionNo()
{
	int directionNo = -1;
	CC_ASSERT(_basePlayer);
	auto motion = _basePlayer->getCurrentAnimationMotion();
	if (motion) {
		auto currentDirection = motion->getCurrentDirection();
		if (currentDirection) {
			auto directionData = currentDirection->getDirectionData();
			if (directionData) {
				directionNo = directionData->getId();
			}
		}
	}
	return directionNo;
}

agtk::data::DirectionData *Player::getCurrentDirectionData()
{
	agtk::data::DirectionData *directionData = nullptr;
	CC_ASSERT(_basePlayer);
	auto motion = _basePlayer->getCurrentAnimationMotion();
	if (motion) {
		directionData = motion->getCurrentDirection()->getDirectionData();
	}
	return directionData;
}

/**
* タイムライン Vertex4変換
*/
bool Player::convertToLayerSpaceTimelineVertex4(int id, agtk::Vertex4 &vertex4, EnumCollidedBit bit, int dot)
{
	auto currentDirection = _basePlayer->getCurrentAnimationMotion()->getCurrentDirection();
	if (currentDirection == nullptr) {
		return false;
	}
	auto animationTimelineList = currentDirection->getAnimationTimelineList();
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
	auto animationTimeline = static_cast<agtk::AnimationTimeline *>(animationTimelineList->objectForKey(id));
#else
	auto animationTimeline = dynamic_cast<agtk::AnimationTimeline *>(animationTimelineList->objectForKey(id));
#endif
	if (animationTimeline->getTimelineInfoData()->getAreaList()->count() == 0) {
		//キーフレームに判定情報がない。
		return false;
	}
	this->convertToWorldSpaceTimelineVertex4(animationTimeline, vertex4, bit, dot);
	return true;
}

bool Player::getTimelineBackside(int id)
{
	if (id < 0) {
		return false;
	}
	auto currentDirection = _basePlayer->getCurrentAnimationMotion()->getCurrentDirection();
	if (currentDirection == nullptr) {
		return false;
	}
	auto animationTimelineList = currentDirection->getAnimationTimelineList();
	auto animationTimeline = dynamic_cast<agtk::AnimationTimeline *>(animationTimelineList->objectForKey(id));
	if (animationTimeline == nullptr) {
		return false;
	}
	return animationTimeline->getBackSide();
}
/**
* タイムライン Rect変換
*/
bool Player::getTimelineValid(int id)
{
	if (id < 0) {
		return false;
	}
	auto currentDirection = _basePlayer->getCurrentAnimationMotion()->getCurrentDirection();
	if (currentDirection == nullptr) {
		return false;
	}
	auto animationTimelineList = currentDirection->getAnimationTimelineList();
	auto animationTimeline = dynamic_cast<agtk::AnimationTimeline *>(animationTimelineList->objectForKey(id));
	if (animationTimeline == nullptr) {
		return false;
	}
	return animationTimeline->getValid();
}

void Player::convertToLayerSpaceTimelineRect(int id, cocos2d::Rect &rect)
{
	agtk::Vertex4 vertex4;
	this->convertToLayerSpaceTimelineVertex4(id, vertex4);
	rect = vertex4.getRect();
}

void Player::convertToLayerSpaceTimelineVertex4List(agtk::data::TimelineInfoData::EnumTimelineType type, std::vector<agtk::Vertex4> &vertex4List, EnumCollidedBit bit, int dot)
{
#if 1
	// kTimelineMaxの場合はすべての種別を生成
	if (type == agtk::data::TimelineInfoData::kTimelineMax) {
		for (int ListTypeNum = 0; ListTypeNum < agtk::data::TimelineInfoData::kTimelineMax; ListTypeNum++) {
			cocos2d::__Array *animeList = getAnimeTimelineList((agtk::data::TimelineInfoData::EnumTimelineType)ListTypeNum);
			for (int ListCount = 0; ListCount < animeList->count(); ListCount++) {
				auto animationTimeline = dynamic_cast<agtk::AnimationTimeline *>(animeList->getObjectAtIndex(ListCount));
				agtk::Vertex4 vertices;
				if (this->convertToWorldSpaceTimelineVertex4(animationTimeline, vertices, bit, dot)) {
					vertex4List.push_back(vertices);
				}
			}
		}
		return;
	}
	// kTimelineMaxWithoutConnectionの場合は接続点以外すべて生成
	else if (type == agtk::data::TimelineInfoData::kTimelineMaxWithoutConnection) {
		for (int ListTypeNum = 0; ListTypeNum < agtk::data::TimelineInfoData::kTimelineMax; ListTypeNum++) {
			if (ListTypeNum == agtk::data::TimelineInfoData::kTimelineConnection) {
				continue;
			}
			cocos2d::__Array *animeList = getAnimeTimelineList((agtk::data::TimelineInfoData::EnumTimelineType)ListTypeNum);
			for (int ListCount = 0; ListCount < animeList->count(); ListCount++) {
				auto animationTimeline = dynamic_cast<agtk::AnimationTimeline *>(animeList->getObjectAtIndex(ListCount));
				agtk::Vertex4 vertices;
				if (this->convertToWorldSpaceTimelineVertex4(animationTimeline, vertices, bit, dot)) {
					vertex4List.push_back(vertices);
				}
			}
		}
		return;
	}

	cocos2d::__Array *animeList = getAnimeTimelineList(type);
	for (int ListCount = 0; ListCount < animeList->count(); ListCount++) {
		auto animationTimeline = dynamic_cast<agtk::AnimationTimeline *>(animeList->getObjectAtIndex(ListCount));
		agtk::Vertex4 vertices;
		if (this->convertToWorldSpaceTimelineVertex4(animationTimeline, vertices, bit, dot)) {
			vertex4List.push_back(vertices);
		}
	}
#else
	auto currentDirection = _basePlayer->getCurrentAnimationMotion()->getCurrentDirection();
	if (currentDirection == nullptr) {
		return;
	}

	auto animationTimelineList = currentDirection->getAnimationTimelineList();
	cocos2d::DictElement *el;
	CCDICT_FOREACH(animationTimelineList, el) {
		auto animationTimeline = dynamic_cast<agtk::AnimationTimeline *>(el->getObject());
		if ((animationTimeline->getTimelineInfoData()->getTimelineType() == type)
		|| (type == agtk::data::TimelineInfoData::kTimelineMax)
		|| (type == agtk::data::TimelineInfoData::kTimelineMaxWithoutConnection && animationTimeline->getTimelineInfoData()->getTimelineType() != agtk::data::TimelineInfoData::kTimelineConnection)) {
			agtk::Vertex4 vertices;
			if (this->convertToWorldSpaceTimelineVertex4(animationTimeline, vertices, bit, dot)) {
				vertex4List.push_back(vertices);
			}
		}
	}
#endif
}

void Player::convertToLayerSpaceTimelineRectList(agtk::data::TimelineInfoData::EnumTimelineType type, std::vector<cocos2d::Rect> &rectList)
{
	std::vector<agtk::Vertex4> vertex4List;
	this->convertToLayerSpaceTimelineVertex4List(type, vertex4List);
	for (auto vertex4 : vertex4List) {
		rectList.push_back(vertex4.getRect());
	}
}

/**
* Vertex4キャッシュリスト取得
*/
void Player::getTimelineVertListCache(agtk::data::TimelineInfoData::EnumTimelineType type, std::vector<agtk::Vertex4> &vertex4List, EnumCollidedBit bit, int dot)
{
	// kTimelineMaxの場合はすべての種別を生成
	if (type == agtk::data::TimelineInfoData::kTimelineMax) {
		for (int ListTypeNum = 0; ListTypeNum < agtk::data::TimelineInfoData::kTimelineMax; ListTypeNum++) {
			cocos2d::__Array *animeList = getAnimeTimelineList((agtk::data::TimelineInfoData::EnumTimelineType)ListTypeNum);
			std::vector<agtk::Vertex4> *VertList = getTimelineVertList(type);
			vertex4List.insert(vertex4List.end(), VertList->begin(), VertList->end()); // 連結
		}
		return;
	}

	// kTimelineMaxWithoutConnectionの場合は接続点以外すべて生成
	if (type == agtk::data::TimelineInfoData::kTimelineMaxWithoutConnection) {
		for (int ListTypeNum = 0; ListTypeNum < agtk::data::TimelineInfoData::kTimelineMax; ListTypeNum++) {
			if (ListTypeNum == agtk::data::TimelineInfoData::kTimelineConnection) {
				continue;
			}
			cocos2d::__Array *animeList = getAnimeTimelineList((agtk::data::TimelineInfoData::EnumTimelineType)ListTypeNum);
			std::vector<agtk::Vertex4> *VertList = getTimelineVertList(type);
			vertex4List.insert(vertex4List.end(), VertList->begin(), VertList->end());
		}
		return;
	}

	std::vector<agtk::Vertex4> *VertList = getTimelineVertList(type);
	vertex4List.insert(vertex4List.end(), VertList->begin(), VertList->end()); // 連結
}

/**
* アニメーションタイムラインVertex4キャッシュリスト更新
*/
void Player::updateTimelineVertListCache(agtk::data::TimelineInfoData::EnumTimelineType type, bool wallAreaAttackWhenInvincible, EnumCollidedBit bit, int dot)
{
	std::vector<agtk::Vertex4> *VertList;
	cocos2d::Rect *Rectlist;
	if (wallAreaAttackWhenInvincible) {
		VertList = getTimelineVertList(agtk::data::TimelineInfoData::kTimelineAttack);
		Rectlist = getTimelineRectList(agtk::data::TimelineInfoData::kTimelineAttack);
	}
	else {
		VertList = getTimelineVertList(type);
		Rectlist = getTimelineRectList(type);
	}

	// Vertex4リストを更新
	cocos2d::__Array *animeList = getAnimeTimelineList(type);
	for (int ListCount = 0; ListCount < animeList->count(); ListCount++) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
		auto animationTimeline = static_cast<agtk::AnimationTimeline *>(animeList->getObjectAtIndex(ListCount));
#else
		auto animationTimeline = dynamic_cast<agtk::AnimationTimeline *>(animeList->getObjectAtIndex(ListCount));
#endif
		
		agtk::Vertex4 vertices;
		if (this->convertToWorldSpaceTimelineVertex4(animationTimeline, vertices, bit, dot)) {
			VertList->push_back(vertices);
		}
	}
	// Rectリストも更新
	if (VertList->size() > 0) {
		*Rectlist = agtk::Vertex4::getRectMerge(*VertList);
	}
}

void Player::VertListCacheClear(agtk::data::TimelineInfoData::EnumTimelineType type)
{
	// キャッシュのクリア
	std::vector<agtk::Vertex4> *VertList = getTimelineVertList(type);
	cocos2d::Rect *Rectlist = getTimelineRectList(type);
	VertList->clear();
	*Rectlist = cocos2d::Rect::ZERO;
}

// VertListCache更新判定
bool Player::isUpdateTimelineVertListCache(agtk::data::TimelineInfoData::EnumTimelineType type) {
	if (_isUpdateTransformCache) {
		return true;
	}
	else if (_updateTransformDirty) {
		return true;
	}
	else {
		bool isWallType = (type == agtk::data::TimelineInfoData::kTimelineWall);
		if (isWallType && _updateTransformDirty == false) {
			return true;
		}
	}
	return false;
}

#ifdef USE_MULTITHREAD_OBJECT_UPDATE
void Player::lock()
{
	if (!_mutex.try_lock()) {
		//THREAD_PRINTF("Player::already locked: %p\n", this);
		_mutex.lock();
#ifdef USE_MULTITHREAD_MEASURE
		ThreadManager::playerBlockedCount++;
#endif
	}
	//THREAD_PRINTF("Player::lock: %p\n", this);
}

void Player::unlock()
{
	//THREAD_PRINTF("Player::unlock: %p\n", this);
	_mutex.unlock();
}
#endif

void Player::updateTransformCache()
{
	auto object = dynamic_cast<agtk::Object *>(this->getParent());
	if (object == nullptr) {
		return;
	}

	// ※ACT2-2642 上右部が１ドット途切れるバグの対応。とりあえず小数点以下を切り捨てにすると上右部が現れるため。
	auto pos = _centerNode->getPosition();
	auto oldPos = _centerNode->getOldPosition();
	{
		_centerNode->setPosition(this->getCenterPosition());
		_parentToWorldTransformCache2 = this->getTransformNode()->getNodeToWorldTransform();

	}
	_centerNode->setPosition(pos);
	_centerNode->setOldPosition(oldPos);

	_parentToWorldTransformCache = this->getTransformNode()->getNodeToWorldTransform();

	// GameScene の行列を使用していたので 自身のオブジェクトが所属する SceneLayer の行列を取得する
	auto sceneLayerPos = object->getSceneLayer()->getPosition3D() * -1;
	_playerToSceneLayerTransformCache = Mat4::IDENTITY;
	Mat4::createTranslation(sceneLayerPos, &_playerToSceneLayerTransformCache);
}

cocos2d::Vec2 Player::convertToLayerSpacePosition(cocos2d::Vec2 pos, bool bUpdate)
{
	auto playerSize = this->getContentSize();
	auto offset = this->getBasePlayer()->getOriginOffset();
	pos += offset;
	//CC_ASSERT(playerSize.width > 0.0f && playerSize.height > 0.0f);	//画像が設定されていない場合にwidth, heightが0になるため、コメントアウト。
	pos.y = playerSize.height - pos.y;
	cocos2d::Vec2 p = pos;

	if (bUpdate) {
		_isUpdateTransformCache = true;
	}
	if (_isUpdateTransformCache) {
		if (this->getRealPosition() != this->_position) {
			_transformDirty = true;
		}
		updateTransformCache();

		_isUpdateTransformCache = false;
	}

	p = PointApplyTransform(p, _parentToWorldTransformCache);
	p = PointApplyTransform(p, _playerToSceneLayerTransformCache);
	return p;
}

cocos2d::Vec2 Player::convertToLayerSpacePosition2(cocos2d::Vec2 pos, bool bUpdate)
{
	auto playerSize = this->getContentSize();
	auto offset = this->getBasePlayer()->getOriginOffset();
	pos += offset;
	//CC_ASSERT(playerSize.width > 0.0f && playerSize.height > 0.0f);	//画像が設定されていない場合にwidth, heightが0になるため、コメントアウト。
	pos.y = playerSize.height - pos.y;
	cocos2d::Vec2 p = pos;

	if (bUpdate) {
		_isUpdateTransformCache = true;
	}
	if (_isUpdateTransformCache) {
		if (this->getRealPosition() != this->_position) {
			_transformDirty = true;
		}
		updateTransformCache();

		_isUpdateTransformCache = false;
	}

	p = PointApplyTransform(p, _parentToWorldTransformCache2);
	p = PointApplyTransform(p, _playerToSceneLayerTransformCache);
	return p;
}

bool Player::convertToWorldSpaceTimelineVertex4(agtk::AnimationTimeline *animationTimeline, agtk::Vertex4 &vertex4, EnumCollidedBit bit, int dot)
{
	// (x0,y0)   (x1,y1)
	//    +-------+
	//    |       |
	//    |       |
	//    |       |
	//    +-------+
	// (x3,y3)   (x2,y2)

#ifdef USE_COLLISION_MEASURE
	callCount++;
#endif
	auto rect = animationTimeline->getRect();
	auto type = animationTimeline->getTimelineInfoData()->getTimelineType();
	auto size = rect.size;

	if (animationTimeline->getTimelineInfoData()->getAreaList()->count() == 0) {
		//キーフレームに判定情報がない。
#ifdef USE_COLLISION_MEASURE
		noInfoCount++;
#endif
		return false;
	}
	// 2018.02.21 modify :tada
	// ワールド座標への変換行列の取得はコストが高いので壁判定等を持たないかを確認してから処理を行うよう順序を変更。
	if (size.width == 0 && size.height == 0 && type != agtk::data::TimelineInfoData::kTimelineConnection) {
#ifdef USE_COLLISION_MEASURE
		noInfoCount++;
#endif
		return false;
	}

	cocos2d::Vec2 leftUp = cocos2d::Vec2(kCollidedBitLeft & bit ? -dot : 0.0f, kCollidedBitUp & bit ? dot : 0.0f);
	cocos2d::Vec2 leftDown = cocos2d::Vec2(kCollidedBitLeft & bit ? -dot : 0.0f, kCollidedBitDown & bit ? -dot : 0.0f);
	cocos2d::Vec2 rightUp = cocos2d::Vec2(kCollidedBitRight & bit ? dot : 0.0f, kCollidedBitUp & bit ? dot : 0.0f);
	cocos2d::Vec2 rightDown = cocos2d::Vec2(kCollidedBitRight & bit ? dot : 0.0f, kCollidedBitDown & bit ? -dot : 0.0f);

	auto gm = GameManager::getInstance();

	bool isWallType = (type == agtk::data::TimelineInfoData::kTimelineWall);
	if (isWallType && _isUpdateTransformCache == false && _updateTransformDirty == false) {
		_isUpdateTransformCache = true;
	}
	
	if (_isUpdateTransformCache) {

		// 壁当たり判定情報を取得する場合のみ、小数点誤差が発生している場合があるので
		// _transformDirtyをtrueにし、正しいtransformを取得できるようにする
		if (isWallType) {
			_transformDirty = true;
		}
		updateTransformCache();

		_isUpdateTransformCache = false;
	}


	auto playerSize = this->getContentSize();
	auto pos = rect.origin;
	auto offset = this->getBasePlayer()->getOriginOffset();
	pos += offset;

	CC_ASSERT(playerSize.width > 0.0f && playerSize.height > 0.0f);
	pos.y = playerSize.height - pos.y;
	switch (type) {
	case agtk::data::TimelineInfoData::kTimelineAttack:
	case agtk::data::TimelineInfoData::kTimelineHit: {
		vertex4[0] = PointApplyTransform(PointApplyTransform(pos + leftUp, _parentToWorldTransformCache), _playerToSceneLayerTransformCache);//左上
		vertex4[1] = PointApplyTransform(PointApplyTransform(pos + rightUp + cocos2d::Vec2(size.width, 0), _parentToWorldTransformCache), _playerToSceneLayerTransformCache);//右上
		vertex4[2] = PointApplyTransform(PointApplyTransform(pos + rightDown + cocos2d::Vec2(size.width, -size.height), _parentToWorldTransformCache), _playerToSceneLayerTransformCache);//右下
		vertex4[3] = PointApplyTransform(PointApplyTransform(pos + leftDown + cocos2d::Vec2(0, -size.height), _parentToWorldTransformCache), _playerToSceneLayerTransformCache);//左下
		break; }
	case agtk::data::TimelineInfoData::kTimelineWall: {

		pos.x += size.width * 0.5f;
		pos.y -= size.height * 0.5f;

#ifdef USE_WALL_COLLISION_FIX
		pos = PointApplyTransform(pos, _parentToWorldTransformCache);
		pos = PointApplyTransform(pos, _playerToSceneLayerTransformCache);

		// スケールに合わせてサイズを変更
		Vec3 scale;
		_parentToWorldTransformCache.getScale(&scale);
		size.width *= scale.x;
		size.height *= scale.y;

		vertex4[3] = pos + leftUp    + cocos2d::Vec2(-size.width * 0.5f,  size.height * 0.5f);//左上
		vertex4[2] = pos + rightUp   + cocos2d::Vec2( size.width * 0.5f,  size.height * 0.5f);//右上
		vertex4[1] = pos + rightDown + cocos2d::Vec2( size.width * 0.5f, -size.height * 0.5f);//右下
		vertex4[0] = pos + leftDown  + cocos2d::Vec2(-size.width * 0.5f, -size.height * 0.5f);//左下
#else
		pos = PointApplyTransform(pos, _parentToWorldTransformCache);
		pos = PointApplyTransform(pos, _playerToSceneLayerTransformCache);

		// スケールに合わせてサイズを変更
		Vec3 scale;
		_parentToWorldTransformCache.getScale(&scale);
		size.width *= scale.x;
		size.height *= scale.y;

		auto rotate = CC_DEGREES_TO_RADIANS(this->getRotation());
		auto v = cocos2d::Vec2(-size.width * 0.5f, size.height * 0.5f) + leftUp;
		vertex4[3] = pos + agtk::GetRotateByAngle(v, -rotate) * v.length();//左上
		v = cocos2d::Vec2(size.width * 0.5f, size.height * 0.5f) + rightUp;
		vertex4[2] = pos + agtk::GetRotateByAngle(v, -rotate) * v.length();//右上
		v = cocos2d::Vec2(size.width * 0.5f, -size.height * 0.5f) + rightDown;
		vertex4[1] = pos + agtk::GetRotateByAngle(v, -rotate) * v.length();//右下
		v = cocos2d::Vec2(-size.width * 0.5f, -size.height * 0.5f) + leftDown;
		vertex4[0] = pos + agtk::GetRotateByAngle(v, -rotate) * v.length();//左下
#endif
		break; }
	case agtk::data::TimelineInfoData::kTimelineConnection: {
		pos = PointApplyTransform(pos, _parentToWorldTransformCache);
		pos = PointApplyTransform(pos, _playerToSceneLayerTransformCache);
		vertex4[0] = pos;
		vertex4[1] = pos;
		vertex4[2] = pos;
		vertex4[3] = pos;
		break; }
	}

#if 0
	//デバッグ機能（浮動小数点切り捨て）
	//※ACT2-1645 描画上では重なっているが、実際は重なっていない事でバグになっているので、四捨五入で丸め込むように調整。
	if (agtk::DebugManager::getInstance()->getTruncatedToDecimalPoint()) {
		for (int i = 0; i < vertex4.length(); i++) {
			auto v = vertex4[i];
			v.x = roundf(v.x);
			v.y = roundf(v.y);
			vertex4[i] = v;
		}
	}
#endif

	return true;
}

const Mat4& Player::getNodeToParentTransform() const
{
	// NodeのgetNodeToParentTransformでは_realPositionを使用せず_positionで計算していたため
	// オブジェクトの座標と壁当たり判定とで誤差が発生していたため_realPositionで計算するようオーバーライドした関数

	if (_transformDirty)
	{
		// Translate values
		float x = _realPosition.x;
		float y = _realPosition.y;
		float z = _positionZ;

		if (_ignoreAnchorPointForPosition)
		{
			x += _anchorPointInPoints.x;
			y += _anchorPointInPoints.y;
		}

		bool needsSkewMatrix = (_skewX || _skewY);

		// Build Transform Matrix = translation * rotation * scale
		Mat4 translation;
		//move to anchor point first, then rotate
		Mat4::createTranslation(x, y, z, &translation);

		Mat4::createRotation(_rotationQuat, &_transform);

		if (_rotationZ_X != _rotationZ_Y)
		{
			// Rotation values
			// Change rotation code to handle X and Y
			// If we skew with the exact same value for both x and y then we're simply just rotating
			float radiansX = -CC_DEGREES_TO_RADIANS(_rotationZ_X);
			float radiansY = -CC_DEGREES_TO_RADIANS(_rotationZ_Y);
			float cx = cosf(radiansX);
			float sx = sinf(radiansX);
			float cy = cosf(radiansY);
			float sy = sinf(radiansY);

			float m0 = _transform.m[0], m1 = _transform.m[1], m4 = _transform.m[4], m5 = _transform.m[5], m8 = _transform.m[8], m9 = _transform.m[9];
			_transform.m[0] = cy * m0 - sx * m1, _transform.m[4] = cy * m4 - sx * m5, _transform.m[8] = cy * m8 - sx * m9;
			_transform.m[1] = sy * m0 + cx * m1, _transform.m[5] = sy * m4 + cx * m5, _transform.m[9] = sy * m8 + cx * m9;
		}
		_transform = translation * _transform;

		if (_scaleX != 1.f)
		{
			_transform.m[0] *= _scaleX, _transform.m[1] *= _scaleX, _transform.m[2] *= _scaleX;
		}
		if (_scaleY != 1.f)
		{
			_transform.m[4] *= _scaleY, _transform.m[5] *= _scaleY, _transform.m[6] *= _scaleY;
		}
		if (_scaleZ != 1.f)
		{
			_transform.m[8] *= _scaleZ, _transform.m[9] *= _scaleZ, _transform.m[10] *= _scaleZ;
		}

		// FIXME:: Try to inline skew
		// If skew is needed, apply skew and then anchor point
		if (needsSkewMatrix)
		{
			float skewMatArray[16] =
			{
				1, (float)tanf(CC_DEGREES_TO_RADIANS(_skewY)), 0, 0,
				(float)tanf(CC_DEGREES_TO_RADIANS(_skewX)), 1, 0, 0,
				0,  0,  1, 0,
				0,  0,  0, 1
			};
			Mat4 skewMatrix(skewMatArray);

			_transform = _transform * skewMatrix;
		}

		// adjust anchor point
		if (!_anchorPointInPoints.isZero())
		{
			// FIXME:: Argh, Mat4 needs a "translate" method.
			// FIXME:: Although this is faster than multiplying a vec4 * mat4
			_transform.m[12] += _transform.m[0] * -_anchorPointInPoints.x + _transform.m[4] * -_anchorPointInPoints.y;
			_transform.m[13] += _transform.m[1] * -_anchorPointInPoints.x + _transform.m[5] * -_anchorPointInPoints.y;
			_transform.m[14] += _transform.m[2] * -_anchorPointInPoints.x + _transform.m[6] * -_anchorPointInPoints.y;
		}
		_updateTransformDirty = true;
	}

	if (_additionalTransform)
	{
		// This is needed to support both Node::setNodeToParentTransform() and Node::setAdditionalTransform()
		// at the same time. The scenario is this:
		// at some point setNodeToParentTransform() is called.
		// and later setAdditionalTransform() is called every time. And since _transform
		// is being overwritten everyframe, _additionalTransform[1] is used to have a copy
		// of the last "_transform without _additionalTransform"
		if (_transformDirty)
			_additionalTransform[1] = _transform;

		if (_transformUpdated)
			_transform = _additionalTransform[1] * _additionalTransform[0];
		_updateTransformDirty = true;
	}

	_transformDirty = _additionalTransformDirty = false;

	return _transform;
}
// #AGTK-NX
#if defined(USE_PREVIEW) || ((CC_TARGET_PLATFORM == CC_PLATFORM_NX) && defined(AGTK_DEBUG))
//#ifdef USE_PREVIEW
/**
* タイムライン表示生成
*/
void Player::createDisplayTimeline()
{
	auto primitiveManager = PrimitiveManager::getInstance();
	if (_basePlayer == nullptr) {
		return;
	}
	//wallArea, hitArea, attackArea, connection
	auto currentDirection = _basePlayer->getCurrentAnimationMotion()->getCurrentDirection();
	if (currentDirection == nullptr) {
		return;
	}
	auto animationTimelineList = currentDirection->getAnimationTimelineList();
	cocos2d::DictElement *el;
	int wallListCount = 0;
	int hitListCount = 0;
	int attackListCount = 0;
	int connectListCount = 0;
	cocos2d::Vec2 scale = _basePlayer->getInnerScale();
	scale.x *= this->getPlayerScale().x * _basePlayer->_scale.x;
	scale.y *= this->getPlayerScale().y * _basePlayer->_scale.y;
	cocos2d::__Array *arr = cocos2d::__Array::create();
	// アニメーションタイムラインリストから1つずつ取り出して処理を行う
	CCDICT_FOREACH(animationTimelineList, el) {
		auto animationTimeline = dynamic_cast<agtk::AnimationTimeline *>(el->getObject());
		cocos2d::__Array *list = nullptr;
		bool bCreateFlag = false;
		cocos2d::Rect rect = animationTimeline->getRect();
		cocos2d::Size size = this->getContentSize();
		// タイムライン情報を取得
		auto timelineInfoData = animationTimeline->getTimelineInfoData();
		if (timelineInfoData->getAreaList()->count() == 0) {
			continue;
		}
		// タイプ別に生成
		agtk::data::TimelineInfoData::EnumTimelineType type = timelineInfoData->getTimelineType();
		PrimitiveNode *node = nullptr;
		switch (type) {
		// 壁判定
		case agtk::data::TimelineInfoData::EnumTimelineType::kTimelineWall: {
			agtk::Vertex4 vertex4 = this->getTimelineRectCenterVertex4(rect, size, scale);
			list = this->getTimelineWallNodeList();
			if (wallListCount < list->count()) {
				node = dynamic_cast<PrimitiveNode *>(list->getObjectAtIndex(wallListCount));
				node->setPolygon(0, 0, vertex4.addr(), vertex4.length());
			} else {
				node = primitiveManager->createPolygon(0, 0, vertex4.addr(), vertex4.length(), cocos2d::Color4F::GREEN);
				bCreateFlag = true;
			}
			wallListCount++;
			break; }
		// 当たり判定
		case agtk::data::TimelineInfoData::EnumTimelineType::kTimelineHit: {
			agtk::Vertex4 vertex4 = this->getTimelineRectVertex4(rect, size);
			list = this->getTimelineHitNodeList();
			if (hitListCount < list->count()) {
				node = dynamic_cast<PrimitiveNode *>(list->getObjectAtIndex(hitListCount));
				node->setPolygon(0, 0, vertex4.addr(), vertex4.length());
			}
			else {
				node = primitiveManager->createPolygon(0, 0, vertex4.addr(), vertex4.length(), cocos2d::Color4F::BLUE);
				bCreateFlag = true;
			}
			hitListCount++;
			break; }
		// 攻撃判定
		case agtk::data::TimelineInfoData::EnumTimelineType::kTimelineAttack: {
			agtk::Vertex4 vertex4 = this->getTimelineRectVertex4(rect, size);
			list = this->getTimelineAttackNodeList();
			if (attackListCount < list->count()) {
				node = dynamic_cast<PrimitiveNode *>(list->getObjectAtIndex(attackListCount));
				node->setPolygon(0, 0, vertex4.addr(), vertex4.length());
			}
			else {
				node = primitiveManager->createPolygon(0, 0, vertex4.addr(), vertex4.length(), cocos2d::Color4F::RED);
				bCreateFlag = true;
			}
			attackListCount++;
			break; }
		// 接続点
		case agtk::data::TimelineInfoData::EnumTimelineType::kTimelineConnection: {
			auto pos = this->getTimelinePosition(rect, size);
			list = this->getTimelineConnectNodeList();
			if (connectListCount < list->count()) {
				node = dynamic_cast<PrimitiveNode *>(list->getObjectAtIndex(connectListCount));
				node->setCircle(pos.x, pos.y, 3);
			}
			else {
				node = primitiveManager->createCircle(pos.x, pos.y, 3, cocos2d::Color4F::YELLOW);
				bCreateFlag = true;
			}
			connectListCount++;
			break; }
		}
		if (bCreateFlag) {
			list->addObject(node);	// 各タイムラインリストに追加。更新はこのリストからノードを取得する
			this->addChild(node);
		}
	}
	//remove
	std::function<void(PrimitiveNode *, cocos2d::__Array *)> removeNode = [&](PrimitiveNode *node, cocos2d::__Array *list) {
		this->removeChild(node);
		primitiveManager->remove(node);
		list->removeObject(node);
	};
	auto timelineWallList = this->getTimelineWallNodeList();
	while (this->getTimelineCountMax(agtk::data::TimelineInfoData::kTimelineWall) < timelineWallList->count()) {
		auto p = dynamic_cast<PrimitiveNode *>(timelineWallList->getLastObject());
		removeNode(p, timelineWallList);
	}
	auto timelineHitList = this->getTimelineHitNodeList();
	while (this->getTimelineCountMax(agtk::data::TimelineInfoData::kTimelineHit) < timelineHitList->count()) {
		auto p = dynamic_cast<PrimitiveNode *>(timelineHitList->getLastObject());
		removeNode(p, timelineHitList);
	}
	auto timelineAttackList = this->getTimelineAttackNodeList();
	while (this->getTimelineCountMax(agtk::data::TimelineInfoData::kTimelineAttack) < timelineAttackList->count()) {
		auto p = dynamic_cast<PrimitiveNode *>(timelineAttackList->getLastObject());
		removeNode(p, timelineAttackList);
	}
	auto timelineConnectList = this->getTimelineConnectNodeList();
	while (this->getTimelineCountMax(agtk::data::TimelineInfoData::kTimelineConnection) < timelineConnectList->count()) {
		auto p = dynamic_cast<PrimitiveNode *>(timelineConnectList->getLastObject());
		removeNode(p, timelineConnectList);
	}
}

/**
* タイムライン表示全削除
*/
void Player::removeDisplayTimelineAll()
{
	this->removeDisplayTimeline(agtk::data::TimelineInfoData::kTimelineWall);
	this->removeDisplayTimeline(agtk::data::TimelineInfoData::kTimelineHit);
	this->removeDisplayTimeline(agtk::data::TimelineInfoData::kTimelineAttack);
	this->removeDisplayTimeline(agtk::data::TimelineInfoData::kTimelineConnection);
}

/**
* タイムライン表示削除
* @param	type								タイムラインタイプ
*/
void Player::removeDisplayTimeline(agtk::data::TimelineInfoData::EnumTimelineType type)
{
	// 指定したタイプのタイムラインをすべて削除
	auto primitiveManager = PrimitiveManager::getInstance();
	auto list = this->getTimelineNodeList(type);
	if (list) {
		for (int i = 0; i < list->count(); i++) {
			auto p = dynamic_cast<PrimitiveNode *>(list->getObjectAtIndex(i));
			this->removeChild(p);
			primitiveManager->remove(p);
		}
		list->removeAllObjects();
	}
}

/**
* タイムライン表示更新
*/
void Player::updateDisplayTimelineAll()
{
	cocos2d::DictElement *el;
	if (_basePlayer == nullptr) {
		return;
	}
	auto currentDirection = _basePlayer->getCurrentAnimationMotion()->getCurrentDirection();
	if (currentDirection != nullptr) {
		// ノード取得用カウンタ
		int wallListCount = 0;
		int hitListCount = 0;
		int attackListCount = 0;
		int connectListCount = 0;

		cocos2d::Vec2 scale = _basePlayer->getInnerScale();
		scale.x *= this->getPlayerScale().x * _basePlayer->_scale.x;
		scale.y *= this->getPlayerScale().y * _basePlayer->_scale.y;
		auto animationTimelineList = currentDirection->getAnimationTimelineList();
		// アニメーションタイムラインリストから取り出して処理を行う
		CCDICT_FOREACH(animationTimelineList, el) {
			auto animationTimeline = dynamic_cast<agtk::AnimationTimeline *>(el->getObject());
			cocos2d::Rect rect = animationTimeline->getRect();
			cocos2d::Size size = this->getContentSize();
			// タイムライン情報を取得
			auto timelineInfoData = animationTimeline->getTimelineInfoData();
			if (timelineInfoData->getAreaList()->count() == 0) {
				continue;
			}
			// タイプ別に更新
			auto type = animationTimeline->getTimelineInfoData()->getTimelineType();
			switch (type) {
			// 壁判定
			case agtk::data::TimelineInfoData::EnumTimelineType::kTimelineWall: {
				agtk::Vertex4 vertex4 = this->getTimelineRectCenterVertex4(rect, size, scale);
				auto prim = getTimelinePrimitiveNode(type, wallListCount);
				prim->setPolygon(0, 0, vertex4.addr(), vertex4.length());
				wallListCount++;
				break; }
			// 当たり判定
			case agtk::data::TimelineInfoData::EnumTimelineType::kTimelineHit: {
				agtk::Vertex4 vertex4 = this->getTimelineRectVertex4(rect, size);
				auto prim = getTimelinePrimitiveNode(type, hitListCount);
				if (this->getWallAreaAttackFlag()) {
					prim->setRGBA(1, 0, 0, 1);//RED
				}
				else {
					prim->setRGBA(0, 0, 1, 1);//BLUE
				}
				prim->setPolygon(0, 0, vertex4.addr(), vertex4.length());
				hitListCount++;
				break; }
			// 攻撃判定
			case agtk::data::TimelineInfoData::EnumTimelineType::kTimelineAttack: {
				agtk::Vertex4 vertex4 = this->getTimelineRectVertex4(rect, size);
				auto prim = getTimelinePrimitiveNode(type, attackListCount);
				prim->setPolygon(0, 0, vertex4.addr(), vertex4.length());
				attackListCount++;
				break; }
			// 接続点
			case agtk::data::TimelineInfoData::EnumTimelineType::kTimelineConnection: {
				cocos2d::Vec2 pos = this->getTimelinePosition(rect, size);
				auto prim = this->getTimelinePrimitiveNode(type, connectListCount);
				prim->setCircle(pos.x, pos.y, animationTimeline->getValid() ? 3 : 0);
				connectListCount++;
				break; }
			}
		}
	}
}

/**
* タイムライン表示更新
*/
void Player::updateDisplayTimeline(agtk::data::TimelineInfoData::EnumTimelineType type)
{
	if (_basePlayer == nullptr) {
		return;
	}

	cocos2d::Vec2 scale = _basePlayer->getInnerScale();
	scale.x *= this->getPlayerScale().x * _basePlayer->_scale.x;
	scale.y *= this->getPlayerScale().y * _basePlayer->_scale.y;
	cocos2d::__Array *nodeList = getTimelineNodeList(type);
	cocos2d::__Array *animeList = getAnimeTimelineList(type);

	auto debugManager = DebugManager::getInstance();
	for (int ListCount = 0; ListCount < nodeList->count(); ListCount++) {
		auto animationTimeline = dynamic_cast<agtk::AnimationTimeline *>(animeList->getObjectAtIndex(ListCount));
		cocos2d::Rect rect = animationTimeline->getRect();
		cocos2d::Size size = this->getContentSize();
		// タイムライン情報を取得
		auto timelineInfoData = animationTimeline->getTimelineInfoData();
		if (timelineInfoData->getAreaList()->count() == 0) {
			continue;
		}

		// タイプ別に更新
		switch (type) {
		// 壁判定
		case agtk::data::TimelineInfoData::EnumTimelineType::kTimelineWall: {
			auto prim = dynamic_cast<PrimitiveNode *>(nodeList->getObjectAtIndex(ListCount));
			prim->setVisible(debugManager->getCollisionWallEnabled());
			if (!debugManager->getCollisionWallEnabled()) {
				break;
			}
			agtk::Vertex4 vertex4 = this->getTimelineRectCenterVertex4(rect, size, scale);
			prim->setPolygon(0, 0, vertex4.addr(), vertex4.length());
			break; }
		// 当たり判定
		case agtk::data::TimelineInfoData::EnumTimelineType::kTimelineHit: {
			auto prim = dynamic_cast<PrimitiveNode *>(nodeList->getObjectAtIndex(ListCount));
			prim->setVisible(debugManager->getCollisionHitEnabled());
			if (!debugManager->getCollisionHitEnabled()) {
				break;
			}
			agtk::Vertex4 vertex4 = this->getTimelineRectVertex4(rect, size);
			if (this->getWallAreaAttackFlag()) {
				prim->setRGBA(1, 0, 0, 1);//RED
			}
			else {
				prim->setRGBA(0, 0, 1, 1);//BLUE
			}
			prim->setPolygon(0, 0, vertex4.addr(), vertex4.length());
			break; }
		// 攻撃判定
		case agtk::data::TimelineInfoData::EnumTimelineType::kTimelineAttack: {
			auto prim = dynamic_cast<PrimitiveNode *>(nodeList->getObjectAtIndex(ListCount));
			prim->setVisible(debugManager->getCollisionAttackEnabled());
			if (!debugManager->getCollisionAttackEnabled()) {
				break;
			}
			agtk::Vertex4 vertex4 = this->getTimelineRectVertex4(rect, size);
			prim->setPolygon(0, 0, vertex4.addr(), vertex4.length());
			break; }
		// 接続点
		case agtk::data::TimelineInfoData::EnumTimelineType::kTimelineConnection: {
			auto prim = dynamic_cast<PrimitiveNode *>(nodeList->getObjectAtIndex(ListCount));
			prim->setVisible(debugManager->getCollisionConnectionEnabled());
			if (!debugManager->getCollisionConnectionEnabled()) {
				break;
			}
			cocos2d::Vec2 pos = this->getTimelinePosition(rect, size);
			prim->setCircle(pos.x, pos.y, 3);
			break; }
		}
	}
}
#endif

/**
* アニメーションタイムラインリスト生成
*/
void Player::createAnimationTimelineList()
{
	auto primitiveManager = PrimitiveManager::getInstance();
	if (_basePlayer == nullptr) {
		return;
	}
	auto currentDirection = _basePlayer->getCurrentAnimationMotion()->getCurrentDirection();
	if (currentDirection == nullptr) {
		return;
	}
	auto animationTimelineList = currentDirection->getAnimationTimelineList();
	cocos2d::DictElement *el;

	getAnimeTimelineWallList()->removeAllObjects();
	getAnimeTimelineHitList()->removeAllObjects();
	getAnimeTimelineAttackList()->removeAllObjects();
	getAnimeTimelineConnectList()->removeAllObjects();
	// アニメーションタイムラインリストから1つずつ取り出して処理を行う
	CCDICT_FOREACH(animationTimelineList, el) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
		auto animationTimeline = static_cast<agtk::AnimationTimeline *>(el->getObject());
#else
		auto animationTimeline = dynamic_cast<agtk::AnimationTimeline *>(el->getObject());
#endif
		cocos2d::__Array *animeList = nullptr;
		// タイムライン情報を取得
		auto timelineInfoData = animationTimeline->getTimelineInfoData();
		if (timelineInfoData->getAreaList()->count() == 0) {
			continue;
		}
		agtk::data::TimelineInfoData::EnumTimelineType type = timelineInfoData->getTimelineType();
		animeList = this->getAnimeTimelineList(type);

		animeList->addObject(animationTimeline);
	}
}

// #AGTK-NX
#if defined(USE_PREVIEW) || ((CC_TARGET_PLATFORM == CC_PLATFORM_NX) && defined(AGTK_DEBUG))
//#ifdef USE_PREVIEW
/**
* タイムライン表示設定
* @param	type								タイムラインタイプ
* @param	visible								表示/非表示
*/
void Player::setVisibleTimeline(agtk::data::TimelineInfoData::EnumTimelineType type, bool visible)
{
	// 指定した種別のタイムラインノードリストを取得
	auto list = this->getTimelineNodeList(type);
	if (list) {
		cocos2d::Ref *ref;
		CCARRAY_FOREACH(list, ref) {
			// リストから取り出し表示設定を行う
			auto p = dynamic_cast<PrimitiveNode *>(ref);
			p->setVisible(visible);
		}
	}
}

/**
* 全タイムライン表示設定
* @param	visible								表示/非表示
*/
void Player::setVisibelTimelineAll(bool visible)
{
	this->setVisibleTimeline(agtk::data::TimelineInfoData::kTimelineWall, visible);
	this->setVisibleTimeline(agtk::data::TimelineInfoData::kTimelineHit, visible);
	this->setVisibleTimeline(agtk::data::TimelineInfoData::kTimelineAttack, visible);
	this->setVisibleTimeline(agtk::data::TimelineInfoData::kTimelineConnection, visible);
}
#endif

cocos2d::Vec2 Player::getTimelinePosition(cocos2d::Rect rect, cocos2d::Size size)
{
	auto trans = this->getTransformNode()->getNodeToWorldTransform();
	auto trans2 = this->getWorldToNodeTransform();
	auto pos = rect.origin;
	auto offset = this->getBasePlayer()->getOriginOffset();
	pos.x += offset.x;
	pos.y += offset.y;
	pos.y = size.height - pos.y;
	pos = PointApplyTransform(pos, trans);
	pos = PointApplyTransform(pos, trans2);
	return pos;
}

agtk::Vertex4 Player::getTimelineRectVertex4(cocos2d::Rect rect, cocos2d::Size size)
{
	agtk::Vertex4 vertex4;
	auto trans = this->getTransformNode()->getNodeToWorldTransform();
	auto trans2 = this->getWorldToNodeTransform();
	auto pos = rect.origin;
	auto offset = this->getBasePlayer()->getOriginOffset();
	pos.x += offset.x;
	pos.y += offset.y;
	pos.y = size.height - pos.y;
	vertex4[0] = PointApplyTransform(PointApplyTransform(pos + cocos2d::Vec2(              0, -rect.size.height), trans), trans2);//左上
	vertex4[1] = PointApplyTransform(PointApplyTransform(pos + cocos2d::Vec2(rect.size.width, -rect.size.height), trans), trans2);//右上
	vertex4[2] = PointApplyTransform(PointApplyTransform(pos + cocos2d::Vec2(rect.size.width,                 0), trans), trans2);//右下
	vertex4[3] = PointApplyTransform(PointApplyTransform(pos + cocos2d::Vec2(              0,                 0), trans), trans2);//左下
	return vertex4;
}

agtk::Vertex4 Player::getTimelineRectCenterVertex4(cocos2d::Rect rect, cocos2d::Size size, cocos2d::Vec2 scale)
{
	agtk::Vertex4 vertex4;
	auto trans = this->getTransformNode()->getNodeToWorldTransform();
	auto trans2 = this->getWorldToNodeTransform();
	cocos2d::Vec2 pos = rect.origin;
	auto offset = this->getBasePlayer()->getOriginOffset();
	pos.x += offset.x + rect.size.width * 0.5f;
	pos.y += offset.y + rect.size.height * 0.5f;
	pos.y = size.height - pos.y;
	pos = PointApplyTransform(pos, trans);
	pos = PointApplyTransform(pos, trans2);
	cocos2d::Size sz(rect.size.width * scale.x, rect.size.height * scale.y);
	vertex4[3] = pos + cocos2d::Vec2(-sz.width * 0.5f, sz.height * 0.5f);//左上
	vertex4[2] = pos + cocos2d::Vec2(sz.width * 0.5f, sz.height * 0.5f);//右上
	vertex4[1] = pos + cocos2d::Vec2(sz.width * 0.5f, -sz.height * 0.5f);//右下
	vertex4[0] = pos + cocos2d::Vec2(-sz.width * 0.5f, -sz.height * 0.5f);//左下
	return vertex4;
}

int Player::getTimelineCountMax(agtk::data::TimelineInfoData::EnumTimelineType type)
{
	auto currentDirection = _basePlayer->getCurrentAnimationMotion()->getCurrentDirection();
	if (currentDirection == nullptr) {
		return 0;
	}
	int count = 0;
	auto animationTimelineList = currentDirection->getAnimationTimelineList();
	cocos2d::DictElement *el;
	CCDICT_FOREACH(animationTimelineList, el) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
		auto animationTimeline = static_cast<agtk::AnimationTimeline *>(el->getObject());
#else
		auto animationTimeline = dynamic_cast<agtk::AnimationTimeline *>(el->getObject());
#endif
		auto timelineInfoData = animationTimeline->getTimelineInfoData();
		if (type == timelineInfoData->getTimelineType() && timelineInfoData->getAreaList()->count() > 0) {
			count++;
		}
	}
	return count;
}

// #AGTK-NX
#if defined(USE_PREVIEW) || ((CC_TARGET_PLATFORM == CC_PLATFORM_NX) && defined(AGTK_DEBUG))
//#ifdef USE_PREVIEW
PrimitiveNode *Player::getTimelinePrimitiveNode(agtk::data::TimelineInfoData::EnumTimelineType type, int id)
{
	switch (type) {
	case agtk::data::TimelineInfoData::EnumTimelineType::kTimelineWall:
		CC_ASSERT(id < this->getTimelineWallNodeList()->count());
		return dynamic_cast<PrimitiveNode *>(this->getTimelineWallNodeList()->getObjectAtIndex(id));
	case agtk::data::TimelineInfoData::EnumTimelineType::kTimelineHit:
		CC_ASSERT(id < this->getTimelineHitNodeList()->count());
		return dynamic_cast<PrimitiveNode *>(this->getTimelineHitNodeList()->getObjectAtIndex(id));
	case agtk::data::TimelineInfoData::EnumTimelineType::kTimelineAttack:
		CC_ASSERT(id < this->getTimelineAttackNodeList()->count());
		return dynamic_cast<PrimitiveNode *>(this->getTimelineAttackNodeList()->getObjectAtIndex(id));
	case agtk::data::TimelineInfoData::EnumTimelineType::kTimelineConnection:
		return dynamic_cast<PrimitiveNode *>(this->getTimelineConnectNodeList()->getObjectAtIndex(id));
	}
	return nullptr;
}

/**
* タイムラインノードリスト取得
*/
cocos2d::__Array *Player::getTimelineNodeList(agtk::data::TimelineInfoData::EnumTimelineType type)
{
	cocos2d::__Array *list = nullptr;
	switch (type) {
	case agtk::data::TimelineInfoData::EnumTimelineType::kTimelineWall: {
		list = this->getTimelineWallNodeList();
		break; }
	case agtk::data::TimelineInfoData::EnumTimelineType::kTimelineHit: {
		list = this->getTimelineHitNodeList();
		break; }
	case agtk::data::TimelineInfoData::EnumTimelineType::kTimelineAttack: {
		list = this->getTimelineAttackNodeList();
		break; }
	case agtk::data::TimelineInfoData::EnumTimelineType::kTimelineConnection: {
		list = this->getTimelineConnectNodeList();
		break; }
	}
	return list;
}
#endif

/**
* タイムラインアニメーションリスト取得
*/
cocos2d::__Array *Player::getAnimeTimelineList(agtk::data::TimelineInfoData::EnumTimelineType type)
{
	cocos2d::__Array *list = nullptr;
	switch (type) {
	case agtk::data::TimelineInfoData::EnumTimelineType::kTimelineWall: {
		list = this->getAnimeTimelineWallList();
		break; }
	case agtk::data::TimelineInfoData::EnumTimelineType::kTimelineHit: {
		list = this->getAnimeTimelineHitList();
		break; }
	case agtk::data::TimelineInfoData::EnumTimelineType::kTimelineAttack: {
		list = this->getAnimeTimelineAttackList();
		break; }
	case agtk::data::TimelineInfoData::EnumTimelineType::kTimelineConnection: {
		list = this->getAnimeTimelineConnectList();
		break; }
	}
	return list;
}

/**
* タイムラインVertex4リスト取得
*/
std::vector<agtk::Vertex4> *Player::getTimelineVertList(agtk::data::TimelineInfoData::EnumTimelineType type)
{
	switch (type) {
	case agtk::data::TimelineInfoData::EnumTimelineType::kTimelineWall: {
		return &_wallVertListCache; }
	case agtk::data::TimelineInfoData::EnumTimelineType::kTimelineHit: {
		return &_hitVertListCache; }
	case agtk::data::TimelineInfoData::EnumTimelineType::kTimelineAttack: {
		return &_attackVertListCache; }
	case agtk::data::TimelineInfoData::EnumTimelineType::kTimelineConnection:
	case agtk::data::TimelineInfoData::EnumTimelineType::kTimelineMaxWithoutConnection: {
		return &_connectVertListCache; }
	default: {
		CC_ASSERT(0);
		return &_wallVertListCache; }
	}
}

/**
* タイムラインRectリスト取得
*/
cocos2d::Rect* Player::getTimelineRectList(agtk::data::TimelineInfoData::EnumTimelineType type)
{
	switch (type) {
	case agtk::data::TimelineInfoData::EnumTimelineType::kTimelineWall: {
		return &_wallVertRectCache; }
	case agtk::data::TimelineInfoData::EnumTimelineType::kTimelineHit: {
		return &_hitVertRectCache; }
	case agtk::data::TimelineInfoData::EnumTimelineType::kTimelineAttack: {
		return &_attackVertRectCache; }
	case agtk::data::TimelineInfoData::EnumTimelineType::kTimelineConnection:
	case agtk::data::TimelineInfoData::EnumTimelineType::kTimelineMaxWithoutConnection: {
		return &_connectVertRectCache; }
	default: {
		CC_ASSERT(0);
		return &_wallVertRectCache; }
	}
}

/**
* 描画 (オーバーライド)
* @param	renderer			レンダラー
* @param	parentTransform		親ノードのtransform
* @param	parentFlags			親ノードフラグ
*/
void Player::visit(Renderer *renderer, const Mat4& parentTransform, uint32_t parentFlags)
{
	auto tmpPos = _realPosition;
	if (_fromPhysics == true) {
		// 物理演算により得た位置情報は_RealPositionを四捨五入する
		_realPosition = Vec2(std::roundf(_realPosition.x), std::roundf(_realPosition.y));
	}
	// 自身でオーバーライドしている getNodeToParentTransform() で _RealPosition を使うので
	// 描画時だけ _RealPosition の小数点以下を切り捨てる
	_realPosition = Vec2((int)_realPosition.x, (int)_realPosition.y);

	if (this->getType() == BasePlayer::SpriteStudio && _clippingRectangleNode) {
		auto scene = GameManager::getInstance()->getCurrentScene();
		auto camera = scene->getCamera();

		// カメラ設定でクリッピング使用判定をする
		if (!camera->isUseClipping() && !_renderTexture) {
			auto motion = _basePlayer->getCurrentAnimationMotion();
			if (motion) {
				int actionNo = motion->getMotionData()->getId();
				int actionDirectNo = motion->getCurrentDirection()->getDirectionData()->getId();
				if (actionNo >= 0 && actionDirectNo >= 0) {
					createRenderTexture();
					play(actionNo, actionDirectNo);
				}
			}
		}
		else {
			//ClippingRectangleNodeでクリップ描画させるために、スケーリングや平行移動を_clippingPreAdjustNodeで取り消して、_clippingPostAdjustNodeで戻すようにしている。
			float scaleX = getScaleX();
			float scaleY = getScaleY();
			Node *parent = this->getParent();
			while (parent) {
				scaleX *= parent->getScaleX();
				scaleY *= parent->getScaleY();
				parent = parent->getParent();
			}
			auto &contentSize = this->getContentSize();
			auto projectData = GameManager::getInstance()->getProjectData();
			auto scene = GameManager::getInstance()->getCurrentScene();
			auto sceneData = scene->getSceneData();
			float sceneHeight = projectData->getScreenHeight() * sceneData->getVertScreenCount();
			auto ssplayer = dynamic_cast<agtk::SSPlayer *>(_basePlayer);
			auto size = ssplayer->getContentSize();
			auto originOrg = ssplayer->getOriginOrg();
			auto camaraScale = camera->getScale();

			_clippingPreAdjustNode->setPosition(-contentSize.width / 2, -contentSize.height / 2 - sceneHeight + sceneHeight - contentSize.height);
			_clippingPostAdjustNode->setPosition(-(-contentSize.width / 2), -(-contentSize.height / 2 - sceneHeight + sceneHeight - contentSize.height));
			_clippingRectangleNode->setClippingRegion(cocos2d::Rect(
				size.width / 2 - originOrg.x,
				originOrg.y - size.height / 2,
				(size.width / scaleX) / (1.0 / camaraScale.x),
				(size.height / scaleY) / (1.0 / camaraScale.y)));
			//cocos2d::log("ClippingRegion: %f, %f, %f, %f", size.width / 2 - originOrg.x, originOrg.y - size.height / 2, size.width / scaleX, size.height / scaleY);
		}
	}

	Node::visit(renderer, parentTransform, parentFlags);

	_realPosition = tmpPos;
}

void Player::setResourceSetId(int resourceSetId)
{
	bool found = false;
	for (auto id : _animationData->getResourceSetIdList()) {
		if (id == resourceSetId) {
			found = true;
			break;
		}
	}
	if (!found) {
		//存在しない。
		return;
	}
	if (_resourceSetId == resourceSetId) {
		//すでに設定している。
		return;
	}
	_resourceSetId = resourceSetId;
	_basePlayer->setResourceSetId(resourceSetId);
	int motionId = -1;
	int directionId = -1;
	auto currentMotion = _basePlayer->getCurrentAnimationMotion();
	if (currentMotion) {
		motionId = currentMotion->getMotionData()->getId();
		auto currentDirection = currentMotion->getCurrentDirection();
		if (currentDirection) {
			directionId = currentDirection->getDirectionData()->getId();
		}
	}
	this->play(motionId, directionId, true, false);
}

void Player::setResourceSetName(string resourceSetName)
{
	int index = -1;
	for (auto i = 0; i < _animationData->getResourceSetNameList().size(); i++) {
		if (_animationData->getResourceSetNameList()[i] == resourceSetName) {
			index = i;
			break;
		}
	}
	
	if (index < 0) {
		//存在しない。
		return;
	}
	this->setResourceSetId(_animationData->getResourceSetIdList()[index]);
}

int Player::getResourceSetIdByName(string resourceSetName)
{
	for (auto i = 0; i < _animationData->getResourceSetNameList().size(); i++) {
		if (_animationData->getResourceSetNameList()[i] == resourceSetName) {
			return _animationData->getResourceSetIdList()[i];
		}
	}

	return -1;
}

NS_AGTK_END
