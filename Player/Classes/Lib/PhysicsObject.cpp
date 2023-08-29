#include "PhysicsObject.h"
#include "GameManager.h"
#ifdef USE_PREVIEW
#include "DebugManager.h"
#endif

#define FORCE_TYPE_RAYCAST // 力系の実装をレイキャスト系にする
#define USE_VERLET_INTEGRATION // ロープの挙動に verlet integration を適応する
#define ROPE_MASS_RATE 0.1f //ロープの重さレート

NS_AGTK_BEGIN
//-------------------------------------------------------------------------------------------------------------------
//! 物理オブジェクト：共通
/**
* コンストラクタ
*/
PhysicsBase::PhysicsBase()
{
	_isContacting = false;
	_rootObject = nullptr;
	_followObject = nullptr;
	_followerPhysicsBody = nullptr;
	_connectToBaseList.clear();
	_parentScenePartId = -1;
	_priority = 0;
	_dispPriority = 0;
}

/**
* デストラクタ
*/
PhysicsBase::~PhysicsBase()
{
	this->clear();
}

void PhysicsBase::clear()
{
	this->setRootObject(nullptr);
	this->setFollowObject(nullptr);
	this->setFollowerPhysicsBody(nullptr);
	_connectToBaseList.clear();
}

/**
* 初期化
* @param	sceneData		シーンデータ
*/
bool PhysicsBase::init(agtk::data::SceneData *sceneData)
{
	return true;
}

/**
* 画像IDから画像ノードを生成する
* @param	imgId	画像ID
* @param	mask	マスク用ノード
* @param	type	画像の配置タイプ
* @param	scaling	画像の比率(比率指定時)
* @param	offset	画像の表示位置
* @return			スプライトノード
*/
cocos2d::Node* PhysicsBase::createImageNode(int imgId, cocos2d::Node *mask, agtk::data::PhysicsBaseData::EnumPlacement type, float scaling, Vec2 offset, const char* defaultFileName)
{
	Node *target = nullptr;
	auto fullSize = getContentSize();
	auto size = fullSize * 0.5f;

	// 画像IDから画像データを取得
	auto project = GameManager::getInstance()->getProjectData();
	auto imgFileName = imgId >= 0 ? project->getImageData(imgId)->getFilename() : defaultFileName;
	if (imgFileName == nullptr) {
		return nullptr;
	}
	// スプライト生成
	auto sprite = Sprite::create(imgFileName);
	sprite->setAnchorPoint(Vec2::ANCHOR_TOP_LEFT);
	sprite->setPosition(0, fullSize.height);
	sprite->getTexture()->setAliasTexParameters();

	// 配置設定：オリジナルサイズ
	if (type == agtk::data::PhysicsBaseData::EnumPlacement::kPlacementOriginalSize) {
		// 生成したスプライトをそのまま使用
		target = sprite;
	}
	// 配置設定：オブジェクトに合わせて拡縮
	else if (type == agtk::data::PhysicsBaseData::EnumPlacement::kPlacementFit) {
		auto scaleX = fullSize.width / sprite->getContentSize().width;
		auto scaleY = fullSize.height / sprite->getContentSize().height;
		sprite->setScaleX(scaleX);
		sprite->setScaleY(scaleY);
		target = sprite;
	}
	// 配置設定：タイル状
	else if (type == agtk::data::PhysicsBaseData::EnumPlacement::kPlacementTiling) {
		auto spriteW = sprite->getContentSize().width;
		auto spriteH = sprite->getContentSize().height;
		int numX = ceil(fullSize.width / spriteW);
		int numY = ceil(fullSize.height / spriteH);
		auto tileTexture = cocos2d::RenderTexture::create(fullSize.width, fullSize.height, cocos2d::Texture2D::PixelFormat::RGBA8888);
		tileTexture->beginWithClear(0, 0, 0, 0);
		for (int i = 0, max = numX * numY; i < max; i++) {
			auto tmpSpr = Sprite::create(imgFileName);
			auto tmpSize = tmpSpr->getContentSize();
			tmpSpr->setAnchorPoint(Vec2::ANCHOR_TOP_LEFT);
			tmpSpr->setPosition((i % numX) * tmpSize.width, fullSize.height - (i / numX) * tmpSize.height);
			tmpSpr->visit();
		}
		tileTexture->end();
		tileTexture->setPosition(size);
		target = tileTexture;
	}
	// 比率を指定
	else if (type == agtk::data::PhysicsBaseData::EnumPlacement::kPlacementScaling) {
		sprite->setScale(scaling);
		target = sprite;
	}

	// クリッピングノード生成
	auto clippingNode = ClippingNode::create(mask);
	clippingNode->setAlphaThreshold(1);
	clippingNode->addChild(target);

	// レンダーテクスチャに描画
	auto renderTexture = cocos2d::RenderTexture::create(fullSize.width, fullSize.height, cocos2d::Texture2D::PixelFormat::RGBA8888, GL_DEPTH24_STENCIL8);
	renderTexture->getSprite()->getTexture()->setAliasTexParameters();
	renderTexture->beginWithClear(0, 0, 0, 0);
	clippingNode->Node::visit();
	renderTexture->end();

	// 表示位置を調整(エディタはY軸が下方向でプラス)
	renderTexture->setPosition(size.width + offset.x, size.height - offset.y);

	return renderTexture;
}

/**
* ２点間の角度を自身の角度に設定
* @param	from	始点
* @param	to		終点
*/
void PhysicsBase::setRotationFromTwoVec(Vec2 from, Vec2 to)
{
	auto dx = to.x - from.x;
	auto dy = to.y - from.y;
	auto rad = atan2f(dy, dx);
	auto deg = CC_RADIANS_TO_DEGREES(rad);
	deg = 90 - (deg < 0 ? deg - 180.0f : deg + 180.0f);
	this->setRotation(deg);
}

void PhysicsBase::forceFollowPhysics()
{
#define USE_SKIP_PHYSICS_PIN_UPDATE	//ロープの端をオブジェクトに連結して引っ張るとロープ全体が引っ張られてしまうのを回避。
#ifdef USE_SKIP_PHYSICS_PIN_UPDATE
#else
	// 下位接続オブジェクトが存在する場合
	if (_followObject && _followerPhysicsBody) {

		if (!_followObject->getphysicsNode()) {
			this->setFollowObject(nullptr);
			this->setFollowerPhysicsBody(nullptr);
			return;
		}

		// 接続された物理オブジェクトの動作を優先する場合
		if (_followObject->getObjectData()->getPhysicsSetting()->getFollowConnectedPhysics()) {
			// 強制位置補正は行わない
			return;
		}

		// プレイデータ取得
		auto playObjectData = _followObject->getPlayObjectData();

		// 移動量取得
		Vec2 velocity = Vec2(playObjectData->getVariableData(agtk::data::kObjectSystemVariableVelocityX)->getValue(),
			-playObjectData->getVariableData(agtk::data::kObjectSystemVariableVelocityY)->getValue());

		// 接着された上位物理ボディの位置を補正
		auto owner = _followerPhysicsBody->getOwner();
		_followerPhysicsBody->resetForces();
		owner->setPosition(owner->getPosition() + velocity);

		// 接着されたオブジェクトに付けられたジョイントを全て網羅して同じように位置を補正する
		cocos2d::__Array *ignoreList = cocos2d::__Array::create();
		std::function<void(const std::vector<cocos2d::PhysicsJoint *> &joints, const Vec2 velocity, cocos2d::__Array * ignoreList)> setPositionRecursive = [&setPositionRecursive](const std::vector<cocos2d::PhysicsJoint *> &joints, const Vec2 velocity, cocos2d::__Array * ignoreList) {

			auto applyVelocity = [](cocos2d::__Array * ignoreList, cocos2d::PhysicsBody * body, const Vec2 velocity) {
				ignoreList->addObject(body);
				auto owner = body->getOwner();
				body->applyImpulse(-velocity * body->getMass());
				owner->setPosition(owner->getPosition() + velocity);
			};

			for (auto joint : joints) {
				// スプリングジョイントの場合はスキップ
				auto spring = dynamic_cast<cocos2d::PhysicsJointSpring*>(joint);
				if (spring) {
					continue;
				}
				auto bodyA = joint->getBodyA();
				auto bodyB = joint->getBodyB();
				auto isContainA = ignoreList->containsObject(bodyA);
				auto isContainB = ignoreList->containsObject(bodyB);
				if (isContainA && isContainB) {
					continue;
				}
				else if (!isContainA) {
					applyVelocity(ignoreList, bodyA, velocity);
					setPositionRecursive(bodyA->getJoints(), velocity, ignoreList);
				}
				else if (!isContainB) {
					applyVelocity(ignoreList, bodyB, velocity);
					setPositionRecursive(bodyB->getJoints(), velocity, ignoreList);
				}
			}
		};

		ignoreList->addObject(_followerPhysicsBody);
		ignoreList->addObject(_followObject->getphysicsNode()->getPhysicsBody());
		setPositionRecursive(_followerPhysicsBody->getJoints(), velocity, ignoreList);

		ignoreList->removeAllObjects();
	}
#endif
}

/**
* [DEBUG] デバッグ用の円形または扇形の範囲表示
* @param	angle		円弧の方向角度
* @param	fanAngle	円弧の範囲角度(0～360)
* @param	radius		円弧の半径
* @param	color		描画色(塗りつぶし)
* @param	drawNode	描画するためのノード
*/
void PhysicsBase::drawDebugCircleOrFan(int angle, int fanAngle, int radius, const cocos2d::Color4F &color, cocos2d::DrawNode *drawNode)
{
	// 完全な円か？
	bool isCircle = fanAngle == 360;

	// 開始角度とインデックス値と半径から指定角度の座標を求めるメソッド
	auto getPointFromAngleAndIdx = [](int startAngle, int idx, float radius) -> Point {
		auto rad = CC_DEGREES_TO_RADIANS(startAngle + idx);
		auto x = radius * cos(rad);
		auto y = radius * sin(rad);
		return Point(x, y);
	};

	// 不完全な円かつ弧の角度が 180 を超える場合
	if (!isCircle && fanAngle > 180) {
		// 180 度までの円弧を描画
		int pointMax1 = 180;
		Point *points1 = new (std::nothrow)Point[pointMax1];
		for (int idx = 0, startAngle = angle - fanAngle / 2; idx < pointMax1; idx++) {
			points1[idx] = getPointFromAngleAndIdx(startAngle, idx, radius);
		}

		// 描画
		drawNode->drawPolygon(points1, pointMax1, color, 1, color);

		// 解放
		CC_SAFE_DELETE_ARRAY(points1);

		// 残りの fanAngle - 180 度分の円弧を描画
		int pointMax2 = fanAngle - 180 + 1;
		Point *points2 = new (std::nothrow)Point[pointMax2];
		for (int idx = 0, startAngle = angle - fanAngle / 2 + 180; idx < pointMax2 - 1; idx++) {
			points2[idx] = getPointFromAngleAndIdx(startAngle, idx, radius);
		}

		// 原点追加
		points2[pointMax2 - 1] = Point(0, 0);

		// 描画
		drawNode->drawPolygon(points2, pointMax2, color, 1, color);

		// 解放
		CC_SAFE_DELETE_ARRAY(points2);
	}
	// 完全な円または180度以内の円弧の場合
	else {

		// 円ポリゴン描画用の点の要素数(不完全な円の場合は原点分として +1 する)
		int pointMax = fanAngle + (!isCircle ? 1 : 0);

		// 円ポリゴン描画用の点の配列を生成
		fanAngle = fanAngle < 1 ? 1 : fanAngle;
		Point *points = new (std::nothrow)Point[pointMax];

		for (int idx = 0, max = fanAngle, startAngle = angle - max / 2; idx < max; idx++) {
			points[idx] = getPointFromAngleAndIdx(startAngle, idx, radius);
		}

		// 不完全な円の場合
		if (!isCircle) {
			// 原点を追加する
			points[fanAngle] = Point(0, 0);
		}

		// 描画
		drawNode->drawSolidPoly(points, pointMax, color);

		// 解放
		CC_SAFE_DELETE_ARRAY(points);
	}
}

/**
* [DEBUG] デバッグ用の矩形範囲表示
* @param	direction	方向
* @param	widthHalf	矩形の幅の半分
* @param	distance	矩形の高さ
* @param	drawNode	描画するためのノード
*/
void PhysicsBase::drawDebugRectangle(const Vec2 &direction, float widthHalf, float distance, const cocos2d::Color4F &color, cocos2d::DrawNode *drawNode)
{
	// 矩形ポリゴン描画用の点配列を生成
	auto pointMax = 4;
	Point *points = new (std::nothrow)Point[pointMax];
	auto basePos = Vec2(-direction.y, direction.x);

	// 左上
	points[0] = basePos * widthHalf;
	// 右上
	points[1] = basePos * widthHalf + direction * distance;
	// 右下
	points[2] = basePos * (widthHalf - widthHalf * 2) + direction * distance;
	// 左下
	points[3] = basePos * (widthHalf - widthHalf * 2);

	// 描画
	drawNode->drawSolidPoly(points, pointMax, color);

	// 解放
	CC_SAFE_DELETE_ARRAY(points);
}

//-------------------------------------------------------------------------------------------------------------------
//! 物理オブジェクト：円形
/**
* コンストラクタ
*/
PhysicsDisk::PhysicsDisk() : PhysicsBase()
{
	_physicsData = nullptr;
}

/**
* デストラクタ
*/
PhysicsDisk::~PhysicsDisk()
{
	CC_SAFE_RELEASE_NULL(_physicsData);
}

/**
* 初期化
* @param	physicsPartsData		物理パーツデータ
* @param	sceneData		シーンデータ
* @param	layerId			レイヤーID
*/
bool PhysicsDisk::init(agtk::data::PhysicsPartData *physicsPartsData, agtk::data::SceneData *sceneData, int layerId, int parentScenePartId)
{
	auto physicsData = dynamic_cast<agtk::data::PhysicsDiskData *>(physicsPartsData->getPhysicsData());
	if (!physicsData) {
		return false;
	}

	if (!PhysicsBase::init(sceneData)) {
		return false;
	}

	this->setName("Disk");

	// シーンパーツID設定
	this->setScenePartsId(physicsPartsData->getId());
	this->setParentScenePartId(parentScenePartId);

	// 描画優先度を設定
	this->setPriority(physicsPartsData->getPriority());
	this->setDispPriority(physicsPartsData->getDispPriority());
	
	// 配置レイヤーID保持
	this->setLayerId(layerId);
	this->setSceneId(sceneData->getId());

	// 物理オブジェクトのアンカーポイントは中心
	this->setAnchorPoint(Vec2(0.5f, 0.5f));

	// 物理データを保持
	this->setPhysicsData(physicsData);

	// 座標を設定
	Vec2 pos = Vec2(physicsData->getX(), physicsData->getY());
	pos = Scene::getPositionSceneFromCocos2d(pos, sceneData);
	this->setPosition(pos);

	// 縦幅と横幅のそれぞれの半径を算出
	float radiusX = physicsData->getWidth() * 0.5f;
	float radiusY = physicsData->getHeight() * 0.5f;

	// 自身のサイズを設定
	this->setContentSize(Size(radiusX * 2, radiusY * 2));

	// 楕円形にも対応する為円系のポイント配列を生成
	int rate = (360 / CIRCLE_POINT_MAX);
	Point circlePoints[CIRCLE_POINT_MAX];
	for (int i = 0; i < CIRCLE_POINT_MAX; i++) {
		auto x = this->getScaleX() * radiusX * cos(CC_DEGREES_TO_RADIANS(i * rate));
		auto y = this->getScaleY() * radiusY * sin(CC_DEGREES_TO_RADIANS(i * rate));
		circlePoints[i] = Point(x, y);
	}

	// -------------------------------------------------------------------------
	// -- 見た目の設定 
	// -------------------------------------------------------------------------
	// 物理オブジェクトの表示をする場合
	if (!physicsData->getInvisible()) {
		// 描画用の円を生成
		auto circle = DrawNode::create();

		// カラーによる描画の場合
		if (physicsData->getColoring()) {
			// そのまま使用する
			auto color = Color4F((float)physicsData->getColorR() / 255.0f, (float)physicsData->getColorG() / 255.0f, (float)physicsData->getColorB() / 255.0f, (float)physicsData->getColorA() / 255.0f);
			circle->drawPolygon(circlePoints, CIRCLE_POINT_MAX, color, 0, color);
			circle->setPosition(radiusX, radiusY);
#if defined(AGTK_DEBUG)
			auto line = DrawNode::create();
			line->drawLine(Vec2::ZERO, Vec2(0, radiusY), Color4F::RED);
			line->setPosition(Vec2::ZERO);
			circle->addChild(line);
#endif
			this->addChild(circle);
		}
		// 画像の場合
		else {

			// 描画用のマスクとして使用する
			circle->drawPolygon(circlePoints, CIRCLE_POINT_MAX, Color4F::WHITE, 0, Color4F::WHITE);
			circle->setPosition(radiusX, radiusY);


			// 画像を生成
			auto image = createImageNode(
				physicsData->getImageId(),
				circle,
				physicsData->getPlacementType(),
				physicsData->getScaling(),
				Vec2(physicsData->getPlacementX(), physicsData->getPlacementY()),
				nullptr
			);
			if (image != nullptr) {
				this->addChild(image);
			}
		}
	}

	// -------------------------------------------------------------------------
	// -- 物理オブジェクトの設定 
	// -------------------------------------------------------------------------
	// 物理設定(密度・摩擦係数・反発係数)
	auto material = PHYSICSBODY_MATERIAL_DEFAULT;
	material.density = physicsData->getDensity();
	material.friction = physicsData->getFriction();
	material.restitution = physicsData->getRepulsion();

	PhysicsBody *physicsBody = nullptr;

	// 横半径と縦半径が同じ場合
	if (radiusX == radiusY) {
		// 真円形のポリゴンボディを生成
		physicsBody = PhysicsBody::createCircle(radiusX, material);
	}
	// 横半径と縦半径が異なる場合
	else {
		// 楕円形のポリゴンボディを生成
		physicsBody = PhysicsBody::createPolygon(circlePoints, CIRCLE_POINT_MAX, material);
	}

	// 質量設定
	physicsBody->setMass(physicsData->getMass() <= 0 ? 0.01f : physicsData->getMass());

	// 楕円形の慣性モーメント
	physicsBody->setMoment((physicsBody->getMass() * (pow(radiusX, 2) + pow(radiusY, 2))) / 4);

	physicsBody->setLinearDamping(0);
	physicsBody->setAngularDamping(0);

	// コリジョンイベント用の設定
	physicsBody->setGroup(GameManager::EnumPhysicsGroup::kPhysicalObject);
	physicsBody->setContactTestBitmask(INT_MAX);
	auto physicsLayer = (layerId - 1);
	if (sceneData->getId() == agtk::data::SceneData::kMenuSceneId) {
		physicsLayer = physicsLayer + agtk::PhysicsBase::kMenuPhysicsLayerShift;
	}
	auto bitmask = (1 << physicsLayer);
	physicsBody->setCategoryBitmask(bitmask);
	physicsBody->setCollisionBitmask(bitmask);

	// 非衝突グループ設定
	int bit = 0;
	cocos2d::Ref * ref = nullptr;
	CCARRAY_FOREACH(physicsData->getNonCollisionGroup(), ref) {
		auto num = dynamic_cast<cocos2d::Integer *>(ref)->getValue();

		bit |= (1 << num);
	}
	physicsBody->setTag(bit);

	// 物理ボディをアタッチ
	this->setPhysicsBody(physicsBody);
	this->unscheduleUpdate();

	// 角度を設定
	this->setRotation(physicsData->getRotation());

	return true;
}

/**
* 更新
* @param	dt	前フレームからの経過時間
*/
void PhysicsDisk::update(float dt)
{
	cocos2d::Vec2 pos = cocos2d::Vec2::ZERO;
	for (auto baseData : _connectToBaseList) {
		cocos2d::Vec2 pos = baseData._dummyNode->getPosition();
		cocos2d::Vec2 absolutePoint = this->convertToWorldSpace(pos);
		baseData._node->setPosition(absolutePoint);
		baseData._node->setRotation(this->getRotation());
	}
}

void PhysicsDisk::clear()
{
	PhysicsBase::clear();
	this->setPhysicsData(nullptr);
}

//-------------------------------------------------------------------------------------------------------------------
//! 物理オブジェクト：四角形
/**
* コンストラクタ
*/
PhysicsRectangle::PhysicsRectangle() : PhysicsBase()
{
	_physicsData = nullptr;
}

/**
* デストラクタ
*/
PhysicsRectangle::~PhysicsRectangle()
{
	CC_SAFE_RELEASE_NULL(_physicsData);
}

/**
* 初期化
* @param	physicsPartsData		物理パーツデータ
* @param	sceneData		シーンデータ
* @param	layerId			レイヤーID
*/
bool PhysicsRectangle::init(agtk::data::PhysicsPartData *physicsPartsData, agtk::data::SceneData *sceneData, int layerId, int parentScenePartId)
{

	auto physicsData = dynamic_cast<agtk::data::PhysicsRectangleData *>(physicsPartsData->getPhysicsData());
	if (!physicsData) {
		return false;
	}

	if (!PhysicsBase::init(sceneData)) {
		return false;
	}

	this->setName("Rectangle");

	// シーンパーツID設定
	this->setScenePartsId(physicsPartsData->getId());
	this->setParentScenePartId(parentScenePartId);

	// 描画優先度を設定
	this->setPriority(physicsPartsData->getPriority());
	this->setDispPriority(physicsPartsData->getDispPriority());

	// 配置レイヤーID保持
	this->setLayerId(layerId);
	this->setSceneId(sceneData->getId());

	// 物理オブジェクトのアンカーポイントは中心
	this->setAnchorPoint(Vec2(0.5f, 0.5f));

	// 物理データを保持
	this->setPhysicsData(physicsData);

	// 座標を設定
	Vec2 pos = Vec2(physicsData->getX(), physicsData->getY());
	pos = Scene::getPositionSceneFromCocos2d(pos, sceneData);
	this->setPosition(pos);

	// 自身のサイズを設定
	this->setContentSize(Size(physicsData->getWidth(), physicsData->getHeight()));

	// -------------------------------------------------------------------------
	// -- 見た目の設定 
	// -------------------------------------------------------------------------
	// 物理オブジェクトの表示をする場合
	if (!physicsData->getInvisible()) {
		// 描画用の矩形を生成
		auto rectangle = DrawNode::create();

		// カラーによる描画の場合
		if (physicsData->getColoring()) {
			// そのまま使用する
			auto color = Color4F((float)physicsData->getColorR() / 255.0f, (float)physicsData->getColorG() / 255.0f, (float)physicsData->getColorB() / 255.0f, (float)physicsData->getColorA() / 255.0f);
			rectangle->drawSolidRect(Vec2::ZERO, getContentSize(), color);
			this->addChild(rectangle);
		}
		// 画像の場合
		else {

			// 描画用のマスクとして使用する
			rectangle->drawSolidRect(Vec2::ZERO, getContentSize(), Color4F::WHITE);

			// 画像を生成
			auto image = createImageNode(
				physicsData->getImageId(),
				rectangle,
				physicsData->getPlacementType(),
				physicsData->getScaling(),
				Vec2(physicsData->getPlacementX(), physicsData->getPlacementY()),
				nullptr
			);
			if (image != nullptr) {
				this->addChild(image);
			}
		}
	}

	// -------------------------------------------------------------------------
	// -- 物理オブジェクトの設定 
	// -------------------------------------------------------------------------
	// 物理設定(密度・摩擦係数・反発係数)
	auto material = PHYSICSBODY_MATERIAL_DEFAULT;
	material.density = physicsData->getDensity();
	material.friction = physicsData->getFriction();
	material.restitution = physicsData->getRepulsion();

	// 矩形の物理ボディ生成
	auto physicsBody = PhysicsBody::createBox(getContentSize(), material);

	// 質量設定
	physicsBody->setMass(physicsData->getMass() <= 0 ? 0.01f : physicsData->getMass());

	// 長方形の慣性モーメント
	physicsBody->setMoment((physicsBody->getMass() * (pow(getContentSize().width, 2) + pow(getContentSize().height, 2))) / 3);

	physicsBody->setLinearDamping(0);
	physicsBody->setAngularDamping(0);

	// コリジョンイベント用の設定
	physicsBody->setGroup(GameManager::EnumPhysicsGroup::kPhysicalObject);
	physicsBody->setContactTestBitmask(INT_MAX);
	auto physicsLayer = (layerId - 1);
	if (sceneData->getId() == agtk::data::SceneData::kMenuSceneId) {
		physicsLayer = physicsLayer + agtk::PhysicsBase::kMenuPhysicsLayerShift;
	}
	auto bitmask = (1 << physicsLayer);
	physicsBody->setCategoryBitmask(bitmask);
	physicsBody->setCollisionBitmask(bitmask);

	// 非衝突グループ設定
	int bit = 0;
	cocos2d::Ref * ref = nullptr;
	CCARRAY_FOREACH(physicsData->getNonCollisionGroup(), ref) {
		auto num = dynamic_cast<cocos2d::Integer *>(ref)->getValue();

		bit |= (1 << num);
	}
	physicsBody->setTag(bit);

	// 物理ボディをアタッチ
	this->setPhysicsBody(physicsBody);
	this->unscheduleUpdate();

	// 角度を設定
	this->setRotation(physicsData->getRotation());


	return true;
}

/**
* 更新
* @param	dt	前フレームからの経過時間
*/
void PhysicsRectangle::update(float dt)
{
	cocos2d::Vec2 pos = cocos2d::Vec2::ZERO;
	for (auto baseData : _connectToBaseList) {
		cocos2d::Vec2 pos = baseData._dummyNode->getPosition();
		cocos2d::Vec2 absolutePoint = this->convertToWorldSpace(pos);
		baseData._node->setPosition(absolutePoint);
		baseData._node->setRotation(this->getRotation());
	}
}

void PhysicsRectangle::clear()
{
	PhysicsBase::clear();
	this->setPhysicsData(nullptr);
}

//-------------------------------------------------------------------------------------------------------------------
//! 物理オブジェクト：ロープ部品
/**
* コンストラクタ
*/
PhysicsRopeParts::PhysicsRopeParts() : PhysicsBase()
{
	_physicsData = nullptr;
	_ignoreBody = nullptr;
}

/**
* デストラクタ
*/
PhysicsRopeParts::~PhysicsRopeParts()
{
	CC_SAFE_RELEASE_NULL(_ignoreBody);
	CC_SAFE_RELEASE_NULL(_physicsData);
}

/**
* 初期化
* @param	physicsPartsData	物理パーツデータ
* @param	sceneData			シーンデータ
* @param	layerId				レイヤーID
* @param	idx					部品IDX
*/
bool PhysicsRopeParts::init(agtk::data::PhysicsPartData *physicsPartsData, agtk::data::SceneData *sceneData, int layerId, int idx, int parentScenePartId)
{
	auto physicsData = dynamic_cast<agtk::data::PhysicsRopeData *>(physicsPartsData->getPhysicsData());
	if (!physicsData) {
		return false;
	}

	if (!PhysicsBase::init(sceneData)) {
		return false;
	}

	this->setIdx(idx);

	this->setName(StringUtils::format("RopeParts_%d", idx).c_str());

	// シーンパーツID設定
	this->setScenePartsId(physicsPartsData->getId());
	this->setParentScenePartId(parentScenePartId);

	// 描画優先度を設定
	this->setPriority(physicsPartsData->getPriority());
	this->setDispPriority(physicsPartsData->getDispPriority());

	// 配置レイヤーID保持
	this->setLayerId(layerId);
	this->setSceneId(sceneData->getId());

	// 物理オブジェクトのアンカーポイントは中心
	this->setAnchorPoint(Vec2(0.5f, 0.5f));

	// 物理データを保持
	this->setPhysicsData(physicsData);

	// 自身のサイズを設定
	this->setContentSize(Size(physicsData->getWidth(), 0));

	return true;
}

/**
* 高さを加算
* @param	h	加算する高さ
*/
void PhysicsRopeParts::addContentSizeH(float h)
{
	// コンテントサイズ変更
	auto contentSize = this->getContentSize();
	contentSize.height += (h + ContentHeightAddition);
	this->setContentSize(contentSize);

	// 表示部分変更
	auto physicsData = this->getPhysicsData();

	// -------------------------------------------------------------------------
	// -- 見た目の設定 
	// -------------------------------------------------------------------------
	// 物理オブジェクトの表示をする場合
	if (!physicsData->getInvisible()) {
		// 描画用の矩形を生成
		auto rectangle = DrawNode::create();

		// カラーによる描画の場合
		if (physicsData->getColoring()) {
			// そのまま使用する
			auto color = Color4F((float)physicsData->getColorR() / 255.0f, (float)physicsData->getColorG() / 255.0f, (float)physicsData->getColorB() / 255.0f, (float)physicsData->getColorA() / 255.0f);
			rectangle->drawSolidRect(Vec2::ZERO, getContentSize(), color);
			this->addChild(rectangle);
		}
		// 画像の場合
		else {

			// 描画用のマスクとして使用する
			rectangle->drawSolidRect(Vec2::ZERO, getContentSize(), Color4F::WHITE);

			// 画像を生成
			auto image = createImageNode(
				physicsData->getImageId(),
				rectangle,
				physicsData->getPlacementType(),
				physicsData->getScaling(),
				Vec2(physicsData->getPlacementX(), physicsData->getPlacementY()),
				"img/rope_tex.png"
			);
			this->addChild(image);
		}
	}

	// コンテントサイズに合わせた物理シェイプを生成
	auto material = PHYSICSBODY_MATERIAL_DEFAULT;
	material.friction = 0.5f;
	material.restitution = 0;
	material.density = 1.0f;

	auto physicsBody = cocos2d::PhysicsBody::createBox(contentSize, material);
#define USE_ROPE_ADJUST	//ロープのふるまいを調整する。
#ifdef USE_ROPE_ADJUST
	const float mass = 0.05f;	//質量を上げると暴れる。下げるとバネっぽい振動が収まらなくなる。
	static volatile float moment = 1.0f;	//モーメントを上げると、節の回転が収まらなくなる。
	static volatile float damping = 10.0f;	//ロープの揺れを抑える。
	physicsBody->setMass(mass);
	physicsBody->setMoment(moment);
	physicsBody->setLinearDamping(damping);
	physicsBody->setAngularDamping(damping);
#else
#if 0
	//ロープの幅が異なっていても同じ動作になるようにする場合
	auto w = 1.0f;
#else
#if 0
	auto w = (getContentSize().width > 0 ? 1.0f : 0.01f);
#else
	// ロープの幅に比例
	auto w = getContentSize().width;
	if (w <= 0) {
		w = 1.0f;
	}
#endif
#endif
	physicsBody->setMass(w * getContentSize().height * material.density * ROPE_MASS_RATE);
	physicsBody->setMoment((physicsBody->getMass() * (pow(w, 2) + pow(getContentSize().height, 2))) / 3);
#endif

	// コリジョンイベント用の設定
	physicsBody->setGroup(GameManager::EnumPhysicsGroup::kRope);
	physicsBody->setContactTestBitmask(INT_MAX);
	auto physicsLayer = (this->getLayerId() - 1);
	if (this->getSceneId() == agtk::data::SceneData::kMenuSceneId) {
		physicsLayer = physicsLayer + agtk::PhysicsBase::kMenuPhysicsLayerShift;
	}
	auto bitmask = (1 << physicsLayer);
	physicsBody->setCategoryBitmask(bitmask);
	physicsBody->setCollisionBitmask(bitmask);

	// 非衝突グループ設定
	int bit = 0;
	cocos2d::Ref * ref = nullptr;
	CCARRAY_FOREACH(physicsData->getNonCollisionGroup(), ref) {
		auto num = dynamic_cast<cocos2d::Integer *>(ref)->getValue();

		bit |= (1 << num);
	}
	physicsBody->setTag(bit);

	physicsBody->setAngularVelocityLimit(1.0f);	//角速度制限を掛ける。小さいとなかなかまっすぐにならなくなる。

	this->setPhysicsBody(physicsBody);
	this->unscheduleUpdate();
}

/**
* 質量再計算
*/
void PhysicsRopeParts::reArangeMass()
{
	// 自身が末端に近いほど軽くする
	auto size = getContentSize();
	auto physicsBody = this->getPhysicsBody();
	auto rate = (_physicsData->getPointList()->count() + 1) / (_idx > 0 ? _idx : 1);
	physicsBody->setMass(size.width * size.height * rate);
	physicsBody->setMoment((physicsBody->getMass() * (pow(size.width, 2) + pow(size.height, 2))) / 3);
}

/**
* 更新
* @param	dt	前フレームからの経過時間
*/
void PhysicsRopeParts::update(float dt)
{
#ifdef USE_VERLET_INTEGRATION
	auto body = this->getPhysicsBody();
	auto joints = body->getJoints();

	auto diffPos = this->getPosition() - this->getOldPos();
	this->setPosition(this->getOldPos() + diffPos * 0.5f);

	for (auto joint : joints) {
		for (int integrate = 0; integrate < 1; integrate++) {
			auto jointLimit = dynamic_cast<PhysicsJointLimit *>(joint);

			if (jointLimit) {

				auto connect1Body = jointLimit->getBodyA();// 接続点1につながっている物理ボディ
				auto connect2Body = jointLimit->getBodyB();// 接続点2につながっている物理ボディ

				auto connect1IsDynamic = connect1Body->isDynamic();
				auto connect2IsDynamic = connect2Body->isDynamic();

				// 物理ボディを持つノードを取得
				auto connect1Node = connect1Body->getOwner();
				auto connect2Node = connect2Body->getOwner();

				// オブジェクトかチェック
				auto connect1Obj = dynamic_cast<agtk::Object *>(connect1Node->getParent());
				auto connect2Obj = dynamic_cast<agtk::Object *>(connect2Node->getParent());

				// 接続点1につながっている対象がObject かつ 物理影響を受けない場合
				if (connect1Obj != nullptr && !connect1Obj->getPlayObjectData()->getSwitchData(agtk::data::kObjectSystemSwitchAffectedByOtherObjects)->getValue()) {
					connect1IsDynamic = false;
				}

				// 接続点2につながっている対象がObject かつ 物理影響を受けない場合
				if (connect2Obj != nullptr && !connect2Obj->getPlayObjectData()->getSwitchData(agtk::data::kObjectSystemSwitchAffectedByOtherObjects)->getValue()) {
					connect2IsDynamic = false;
				}

				auto anchorA = jointLimit->getAnchr1();
				auto anchorB = jointLimit->getAnchr2();

				// 接続点1 と 接続点2 の座標を取得
				auto anchor1 = connect1IsDynamic ? anchorA.rotateByAngle(Vec2::ZERO, -CC_DEGREES_TO_RADIANS(connect1Node->getRotation())) : anchorA;
				auto anchor2 = connect2IsDynamic ? anchorB.rotateByAngle(Vec2::ZERO, -CC_DEGREES_TO_RADIANS(connect2Node->getRotation())) : anchorB;
				auto connect1Pos = connect1Node->getPosition() + anchor1;
				auto connect2Pos = connect2Node->getPosition() + anchor2;

				auto deltaPos = connect2Pos - connect1Pos;
				auto h = deltaPos.getLength();

				if (h == 0) return;

				auto diff = -h;
				auto offx = (diff * deltaPos.x / h) * 0.5f;
				auto offy = (diff * deltaPos.y / h) * 0.5f;

				if (connect1IsDynamic) {
					connect1Pos.x -= offx;
					connect1Pos.y -= offy;
				}

				if (connect2IsDynamic) {
					connect2Pos.x += offx;
					connect2Pos.y += offy;
				}

				connect1Node->setPosition(connect1Pos - anchor1);
				connect2Node->setPosition(connect2Pos - anchor2);
			}
		}
	}

	this->setOldPos(this->getPosition());
#else
	// ロープのパーツが小さすぎて暴れるので速度を発散させる
	auto body = this->getPhysicsBody();
	auto vel = body->getVelocity();
	auto magnitude = vel.getLength();
	float partsMax = _physicsData->getPointList()->count() + 1;
	auto mass = body->getMass();

	if (magnitude < partsMax * mass || _isContacting) {
		body->setVelocity(vel * (_isContacting ? 0.5f : DIVERGE_RATE));
		body->resetForces();
		_isContacting = false;
	}
#endif // USE_VERLET_INTEGRATION
}

void PhysicsRopeParts::clear()
{
	PhysicsBase::clear();
	this->setPhysicsData(nullptr);
	this->setIgnoreBody(nullptr);
}

//-------------------------------------------------------------------------------------------------------------------
//! 物理オブジェクト：接着
/**
* コンストラクタ
*/
PhysicsPin::PhysicsPin() : PhysicsBase()
{
	_physicsData = nullptr;
}

/**
* デストラクタ
*/
PhysicsPin::~PhysicsPin()
{
	CC_SAFE_RELEASE_NULL(_physicsData);
}

/**
* 初期化
* @param	physicsPartsData	物理パーツデータ
* @param	sceneData			シーンデータ
* @param	layerId				レイヤーID
*/
bool PhysicsPin::init(agtk::data::PhysicsPartData *physicsPartsData, agtk::data::SceneData *sceneData, int layerId, int parentScenePartId)
{
	auto physicsData = dynamic_cast<agtk::data::PhysicsPinData *>(physicsPartsData->getPhysicsData());
	if (!physicsData) {
		return false;
	}

	if (!PhysicsBase::init(sceneData)) {
		return false;
	}

	this->setName("Pin");

	// シーンパーツID設定
	this->setScenePartsId(physicsPartsData->getId());
	this->setParentScenePartId(parentScenePartId);

	// シーンパーツIDを設定
	this->setScenePartsId(physicsPartsData->getId());

	// 描画優先度を設定
	this->setPriority(physicsPartsData->getPriority());
	this->setDispPriority(physicsPartsData->getDispPriority());

	// 配置レイヤーID保持
	this->setLayerId(layerId);
	this->setSceneId(sceneData->getId());

	// 物理オブジェクトのアンカーポイントは中心
	this->setAnchorPoint(Vec2(0.5f, 0.5f));

	// 物理データを保持
	this->setPhysicsData(physicsData);

	// 座標を設定
	this->setPosition(Vec2::ZERO);

	// 自身のサイズを設定
	this->setContentSize(Size(physicsData->getWidth(), physicsData->getHeight()));

	// -------------------------------------------------------------------------
	// -- 見た目の設定 
	// -------------------------------------------------------------------------
	// 物理オブジェクトの表示をする場合
	if (!physicsData->getInvisible()) {
		// 描画用の矩形を生成
		auto rectangle = DrawNode::create();

		// カラーによる描画の場合
		if (physicsData->getColoring()) {
			// そのまま使用する
			auto color = Color4F((float)physicsData->getColorR() / 255.0f, (float)physicsData->getColorG() / 255.0f, (float)physicsData->getColorB() / 255.0f, (float)physicsData->getColorA() / 255.0f);
			rectangle->drawSolidRect(Vec2::ZERO, getContentSize(), color);
			this->addChild(rectangle);
		}
		// 画像の場合
		else {

			// 描画用のマスクとして使用する
			rectangle->drawSolidRect(Vec2::ZERO, getContentSize(), Color4F::WHITE);

			// 画像を生成
			auto image = createImageNode(
				physicsData->getImageId(),
				rectangle,
				physicsData->getPlacementType(),
				physicsData->getScaling(),
				Vec2(physicsData->getPlacementX(), physicsData->getPlacementY()),
				"img/pin_tex.png"
			);
			this->addChild(image);
		}
	}

	return true;
}

/**
* 更新
* @param	dt	前フレームからの経過時間
*/
void PhysicsPin::update(float dt)
{
	forceFollowPhysics();
}

void PhysicsPin::clear()
{
	PhysicsBase::clear();
	this->setPhysicsData(nullptr);
}

//-------------------------------------------------------------------------------------------------------------------
//! 物理オブジェクト：ばね
/**
* コンストラクタ
*/
PhysicsSpring::PhysicsSpring() : PhysicsBase()
{
	_physicsData = nullptr;
	_jointData = nullptr;
}

/**
* デストラクタ
*/
PhysicsSpring::~PhysicsSpring()
{
	CC_SAFE_RELEASE_NULL(_physicsData);
}

/**
* 初期化
* @param	physicsPartsData	物理パーツデータ
* @param	sceneData			シーンデータ
* @param	layerId				レイヤーID
* @param	jointData			ジョイントデータ
*/
bool PhysicsSpring::init(agtk::data::PhysicsPartData *physicsPartsData, agtk::data::SceneData *sceneData, int layerId, PhysicsJointSpring *jointData, int parentScenePartId)
{
	auto physicsData = dynamic_cast<agtk::data::PhysicsSpringData *>(physicsPartsData->getPhysicsData());

	if (!PhysicsBase::init(sceneData)) {
		return false;
	}

	this->setName("Spring");

	// シーンパーツIDを設定
	this->setScenePartsId(physicsPartsData->getId());
	this->setParentScenePartId(parentScenePartId);

	//描画優先度を設定
	this->setPriority(physicsPartsData->getPriority());
	this->setDispPriority(physicsPartsData->getDispPriority());

	// ジョイントデータ保持
	this->_jointData = jointData;

	// 配置レイヤーID保持
	this->setLayerId(layerId);
	this->setSceneId(sceneData->getId());

	// 物理オブジェクトのアンカーポイントは中心
	this->setAnchorPoint(Vec2(0.5f, 0.5f));

	// 物理データを保持
	this->setPhysicsData(physicsData);

	// 座標を設定
	Vec2 pos = Vec2::ZERO;
	this->setPosition(pos);

	// 自身のサイズを設定
	this->setContentSize(Size(physicsData->getWidth(), 0));

	return true;
}

/**
* セットアップ
* @param	springLength	バネの長さ
*/
void PhysicsSpring::setUp(float springLength)
{
	auto contentSize = getContentSize();
	contentSize.height = springLength;
	setContentSize(contentSize);

	auto physicsData = getPhysicsData();

	// -------------------------------------------------------------------------
	// -- 見た目の設定 
	// -------------------------------------------------------------------------
	// 物理オブジェクトの表示をする場合
	if (!physicsData->getInvisible()) {
		// 描画用の矩形を生成
		auto rectangle = DrawNode::create();

		// カラーによる描画の場合
		if (physicsData->getColoring()) {
			// そのまま使用する
			auto color = Color4F((float)physicsData->getColorR() / 255.0f, (float)physicsData->getColorG() / 255.0f, (float)physicsData->getColorB() / 255.0f, (float)physicsData->getColorA() / 255.0f);
			rectangle->drawSolidRect(Vec2::ZERO, contentSize, color);
			this->addChild(rectangle);
		}
		// 画像の場合
		else {

			// 描画用のマスクとして使用する
			rectangle->drawSolidRect(Vec2::ZERO, getContentSize(), Color4F::WHITE);

			// 画像を生成
			auto image = createImageNode(
				physicsData->getImageId(),
				rectangle,
				physicsData->getPlacementType(),
				physicsData->getScaling(),
				Vec2(physicsData->getPlacementX(), physicsData->getPlacementY()),
				"img/spring_tex.png"
			);
			this->addChild(image);
		}
	}
}

/**
* 更新
* @param	dt	前フレームからの経過時間
*/
void PhysicsSpring::update(float dt)
{
	if (nullptr == _jointData) {
		return;
	}

	if (_jointData->isDestoryed()) {
		this->removeFromParent();
		return;
	}

	if (_physicsData->getInvisible()) {
		return;
	}

	// ワーク変数
	auto connect1Body = _jointData->getBodyA();// 接続点1につながっている物理ボディ
	auto connect2Body = _jointData->getBodyB();// 接続点2につながっている物理ボディ
	auto contentHeight = this->getContentSize().height;//自身の高さ

	// 物理ボディを持つノードを取得
	auto connect1Node = connect1Body->getOwner();
	auto connect2Node = connect2Body->getOwner();
	bool isObject1 = connect1Body->getGroup() == GameManager::EnumPhysicsGroup::kObject;
	bool isObject2 = connect2Body->getGroup() == GameManager::EnumPhysicsGroup::kObject;

	// 接続点1 と 接続点2 の座標を取得
	auto anchor1 = _jointData->getAnchr1().rotateByAngle(Vec2::ZERO, -CC_DEGREES_TO_RADIANS(connect1Node->getRotation()));
	auto anchor2 = _jointData->getAnchr2().rotateByAngle(Vec2::ZERO, -CC_DEGREES_TO_RADIANS(connect2Node->getRotation()));
	auto connect1Pos = connect1Node->getPosition() + anchor1;
	auto connect2Pos = connect2Node->getPosition() + anchor2;

	// 接続点1 と 接続点2 の中間点を算出
	auto centerPos = (connect1Pos + connect2Pos) * 0.5f;

	// 接続点1 と 接続点2 の長さを取得
	auto length = connect1Pos.getDistance(connect2Pos);

	// 位置を変更
	this->setPosition(centerPos);

	// 角度を変更
	this->setRotationFromTwoVec(connect1Pos, connect2Pos);

	// Y軸のスケールを変更
	this->setScaleY((length / contentHeight));

}

void PhysicsSpring::clear()
{
	PhysicsBase::clear();
	this->setPhysicsData(nullptr);
	_jointData = nullptr;
}

//-------------------------------------------------------------------------------------------------------------------
//! 物理オブジェクト：回転軸
/**
* コンストラクタ
*/
PhysicsAxis::PhysicsAxis() : PhysicsBase()
{
	_physicsData = nullptr;
	_axisUpperTarget = nullptr;
	_axisLowerTarget = nullptr;
	_isBraking = false;
	_rotationRate = 0;
	_rotateDirectionValue = -1;
	_ignoreMortor = true;
}

/**
* デストラクタ
*/
PhysicsAxis::~PhysicsAxis()
{
	this->setAxisLowerTarget(nullptr);
	this->setAxisUpperTarget(nullptr);
	CC_SAFE_RELEASE_NULL(_physicsData);
}

/**
* 初期化
* @param	physicsPartsData	物理パーツデータ
* @param	sceneData			シーンデータ
* @param	layerId				レイヤーID
*/
bool PhysicsAxis::init(agtk::data::PhysicsPartData *physicsPartsData, agtk::data::SceneData *sceneData, int layerId, int parentScenePartId)
{
	auto physicsData = dynamic_cast<agtk::data::PhysicsAxisData *>(physicsPartsData->getPhysicsData());
	if (!physicsData) {
		return false;
	}

	if (!PhysicsBase::init(sceneData)) {
		return false;
	}

	this->setName("Axis");

	// シーンパーツIDを設定
	this->setScenePartsId(physicsPartsData->getId());
	this->setParentScenePartId(parentScenePartId);

	// 描画優先度を設定
	this->setPriority(physicsPartsData->getPriority());
	this->setDispPriority(physicsPartsData->getDispPriority());

	// 配置レイヤーID保持
	this->setLayerId(layerId);
	this->setSceneId(sceneData->getId());

	// 物理オブジェクトのアンカーポイントは中心
	this->setAnchorPoint(Vec2(0.5f, 0.5f));

	// 物理データを保持
	this->setPhysicsData(physicsData);

	// 座標を設定
	this->setPosition(Vec2::ZERO);

	// 自身のサイズを設定
	this->setContentSize(Size(physicsData->getWidth(), physicsData->getHeight()));

	// -------------------------------------------------------------------------
	// -- 見た目の設定 
	// -------------------------------------------------------------------------
	// 物理オブジェクトの表示をする場合
	if (!physicsData->getInvisible()) {
		// 描画用の矩形を生成
		auto rectangle = DrawNode::create();

		// カラーによる描画の場合
		if (physicsData->getColoring()) {
			// そのまま使用する
			auto color = Color4F((float)physicsData->getColorR() / 255.0f, (float)physicsData->getColorG() / 255.0f, (float)physicsData->getColorB() / 255.0f, (float)physicsData->getColorA() / 255.0f);
			rectangle->drawSolidRect(Vec2::ZERO, getContentSize(), color);
			this->addChild(rectangle);
		}
		// 画像の場合
		else {

			// 描画用のマスクとして使用する
			rectangle->drawSolidRect(Vec2::ZERO, getContentSize(), Color4F::WHITE);

			// 画像を生成
			auto image = createImageNode(
				physicsData->getImageId(),
				rectangle,
				physicsData->getPlacementType(),
				physicsData->getScaling(),
				Vec2(physicsData->getPlacementX(), physicsData->getPlacementY()),
				"img/axis_tex.png"
			);
			this->addChild(image);
		}
	}

	// 初期回転方向を設定
	_rotateDirectionValue = physicsData->getReverseDirection() ? 1 : -1;

	// モーター動作設定
	_ignoreMortor = !_physicsData->getAddRightRotation();

	return true;
}

/**
* 更新
* @param	dt	前フレームからの経過時間
*/
void PhysicsAxis::update(float dt)
{
	forceFollowPhysics();

	// 動作をスイッチで切替えがONの場合
	if (_physicsData->getEnableBySwitch()) {

		// ワーク変数
		auto gameManager = GameManager::getInstance();
		auto rootObject = this->getRootObject();

		// スイッチデータチェックメソッド
		auto checkSwitchData = [](cocos2d::__Array * switchList)->bool {
			bool ret = true;

			cocos2d::Ref *ref = nullptr;
			CCARRAY_FOREACH(switchList, ref) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
				auto switchData = static_cast<agtk::data::PlaySwitchData *>(ref);
#else
				auto switchData = dynamic_cast<agtk::data::PlaySwitchData *>(ref);
#endif
				ret &= (nullptr != switchData && GameManager::getInstance()->checkSwitchCondition(agtk::data::SwitchVariableConditionData::EnumSwitchValueType::kSwitchConditionOn, switchData->getValue(), switchData->isState()));
			}
			return ret;
		};

		if (_physicsData->getRightRotationSwitchId() > 0) {
			// 右回転用スイッチデータリスト取得
			cocos2d::__Array *rightRotationSwitchDataList = cocos2d::__Array::create();

			// 付随元オブジェクトがあり、自身の付随先オブジェクトのインスタンスが持つスイッチを参照する場合
			if (rootObject && _physicsData->getRightRotationSwitchObjectId() == agtk::data::ObjectCommandData::kSelfObject) {
				auto switchData = rootObject->getPlayObjectData()->getSwitchData(_physicsData->getRightRotationSwitchId());
				if (switchData) {
					rightRotationSwitchDataList->addObject(switchData);
				}
			}
			else {
				gameManager->getSwitchVariableDataList(
					_physicsData->getRightRotationSwitchQualifierId(),
					_physicsData->getRightRotationSwitchObjectId(),
					_physicsData->getRightRotationSwitchId(),
					true,
					rightRotationSwitchDataList
				);
			}

			// 右回転への切替えスイッチがONの場合
			if (checkSwitchData(rightRotationSwitchDataList)) {
				_rotateDirectionValue = -1;
				_ignoreMortor = false;
			}
		}

		if (_physicsData->getLeftRotationSwitchId() > 0) {
			// 左回転用スイッチデータリスト取得
			cocos2d::__Array *leftRotationSwitchDataList = cocos2d::__Array::create();

			// 付随元オブジェクトがあり、自身の付随先オブジェクトのインスタンスが持つスイッチを参照する場合
			if (rootObject && _physicsData->getLeftRotationSwitchObjectId() == agtk::data::ObjectCommandData::kSelfObject) {
				auto switchData = rootObject->getPlayObjectData()->getSwitchData(_physicsData->getLeftRotationSwitchId());
				if (switchData) {
					leftRotationSwitchDataList->addObject(switchData);
				}
			}
			else {
				gameManager->getSwitchVariableDataList(
					_physicsData->getLeftRotationSwitchQualifierId(),
					_physicsData->getLeftRotationSwitchObjectId(),
					_physicsData->getLeftRotationSwitchId(),
					true,
					leftRotationSwitchDataList
				);
			}

			// 左回転への切替えスイッチがONの場合
			if (checkSwitchData(leftRotationSwitchDataList)) {
				_rotateDirectionValue = 1;
				_ignoreMortor = false;
			}
		}

		// ブレーキ機能を追加がONの場合
		if (_physicsData->getAddBrakeFunction() && _physicsData->getBrakeSwitchId() > 0) {
			// ブレーキ用スイッチデータリスト取得
			cocos2d::__Array *brakeSwitchDataList = cocos2d::__Array::create();

			// 付随元オブジェクトがあり、自身の付随先オブジェクトのインスタンスが持つスイッチを参照する場合
			if (rootObject && _physicsData->getBrakeSwitchObjectId() == agtk::data::ObjectCommandData::kSelfObject) {
				auto switchData = rootObject->getPlayObjectData()->getSwitchData(_physicsData->getBrakeSwitchId());
				if (switchData) {
					brakeSwitchDataList->addObject(switchData);
				}
			}
			else {
				gameManager->getSwitchVariableDataList(
					_physicsData->getBrakeSwitchQualifierId(),
					_physicsData->getBrakeSwitchObjectId(),
					_physicsData->getBrakeSwitchId(),
					true,
					brakeSwitchDataList
				);
			}

			// ブレーキスイッチがONの場合
			if (checkSwitchData(brakeSwitchDataList)) {
				_isBraking = true;
			}
			else {
				_isBraking = false;
			}
		}
	}

	// 検証用動作切替えキーがONの場合
	if (_physicsData->getUseVerificationKey()) {
		
		auto inputManager = InputManager::getInstance();

		// 右回転用キーが押された場合
		if (inputManager->isTriggered(_physicsData->getRightRotationKeyId())) {
			_rotateDirectionValue = -1;
			_ignoreMortor = false;
		}
		// 左回転用キーが押された場合
		if (inputManager->isTriggered(_physicsData->getLeftRotationKeyId())) {
			_rotateDirectionValue = 1;
			_ignoreMortor = false;
		}

		// ブレーキ機能がON かつ ブレーキ用キーが押された場合
		if (_physicsData->getAddBrakeFunction() && inputManager->isPressed(_physicsData->getBrakeKeyId())) {
			_isBraking = true;
		}
		else {
			_isBraking = false;
		}
	}

	// モーター動作OFFの場合
	if (_ignoreMortor) {
		// ここで終了
		return;
	}

	// ブレーキ値(回転減衰率  / 100)
	auto brakeRate = _physicsData->getDampingRatio() * -0.01f;

	// 上位接続対象が存在かつ動的オブジェクトの場合
	if (_axisUpperTarget && _axisUpperTarget->isDynamic()) {
		// 上位接続対象用のトルク値
		float upperTorque = _rotateDirectionValue * _physicsData->getTorque() * DOT_PER_METER;

		// ブレーキ中かつ上位接続対象が回転方向と同方向に回転している場合
		if (_isBraking && (_rotateDirectionValue * _axisUpperTarget->getAngularVelocity() > 0)) {
			upperTorque *= brakeRate;
		}
		_axisUpperTarget->applyTorque(upperTorque);
	}

	// 下位接続対象が存在かつ動的オブジェクトの場合
	if (_axisLowerTarget && _axisLowerTarget->isDynamic()) {
		// 下位接続対象用のトルク値
		float lowerTorque = _rotateDirectionValue * _physicsData->getTorque() * DOT_PER_METER;

		// ブレーキ中かつ下位接続対象が回転方向と同方向に回転している場合
		if (_isBraking && (_rotateDirectionValue * _axisLowerTarget->getAngularVelocity() > 0)) {
			lowerTorque *= brakeRate;
		}
		_axisLowerTarget->applyTorque(lowerTorque);
	}
}

void PhysicsAxis::clear()
{
	PhysicsBase::clear();
	this->setPhysicsData(nullptr);
	this->setAxisUpperTarget(nullptr);
	this->setAxisLowerTarget(nullptr);
}

//-------------------------------------------------------------------------------------------------------------------
//! 物理オブジェクト：爆発
/**
* コンストラクタ
*/
PhysicsExprosion::PhysicsExprosion() : PhysicsBase()
{
	_physicsData = nullptr;
	_isActive = false;
	_isActiveOnce = false;
	_duration300 = 0;
	_connectedAnchor = Vec2::ZERO;
	_connectedTarget = nullptr;
	_forceDistance = 0;
}

/**
* デストラクタ
*/
PhysicsExprosion::~PhysicsExprosion()
{
	CC_SAFE_RELEASE_NULL(_connectedTarget);
	CC_SAFE_RELEASE_NULL(_physicsData);
}

/**
* 初期化
* @param	physicsData		物理データ
* @param	sceneData		シーンデータ
* @param	layerId			レイヤーID
*/
bool PhysicsExprosion::init(agtk::data::PhysicsExplosionData *physicsData, agtk::data::SceneData *sceneData, int layerId, int parentScenePartId)
{
	if (!PhysicsBase::init(sceneData)) {
		return false;
	}

	this->setName("explosion");

	// 親シーンパーツID設定
	this->setParentScenePartId(parentScenePartId);

	// 配置レイヤーID保持
	this->setLayerId(layerId);
	this->setSceneId(sceneData->getId());

	// 物理オブジェクトのアンカーポイントは中心
	this->setAnchorPoint(Vec2(0.5f, 0.5f));

	// 物理データを保持
	this->setPhysicsData(physicsData);

	// 座標を設定
	this->setPosition(Vec2::ZERO);

	// 扇形か？(扇タイプ or 方向と範囲を指定がOFF)
	this->setIsFan(physicsData->getrangeType() == RangeType::kRangeTypeFan || !physicsData->getLimitDirectionRange());

	auto distance = physicsData->getEffectiveDistance();

	// 距離無限の場合
	if (physicsData->getEffectiveInfinite()) {
		// シーンの最も長い方を距離とする
		auto size = Director::getInstance()->getVisibleSize();
		size.width = size.width * sceneData->getHorzScreenCount();
		size.height = size.height * sceneData->getVertScreenCount();
		distance = max(size.width, size.height);
	}

	// 力の及ぶ距離を設定
	this->setForceDistance(distance);

#ifdef USE_PREVIEW
	// デバッグ表示がONの場合
	if (DebugManager::getInstance()->getShowPhysicsBoxEnabled()) {
		showDebugVisible(true);
	}
#endif

	return true;
}

/**
* 更新
* @param	dt	前フレームからの経過時間
*/
void PhysicsExprosion::update(float dt)
{
	// 連結元がある場合
	if (_connectedTarget) {
		// 連結基の角度と位置から自身の位置を更新
		auto pos = _connectedTarget->getPosition();
		auto rad = -CC_DEGREES_TO_RADIANS(_connectedTarget->getRotation());
		auto anchor = _connectedAnchor.rotateByAngle(Vec2::ZERO, rad);
		this->setPosition(pos + anchor);
	}

	if (_duration300 > 0) {
		_duration300 -= dt * 300;
		if (_duration300 < 0) {
			_duration300 = 0;
		}
	}

	// 爆発中でない場合
	if (!_isActive) {
		// 動作をスイッチで切替えがON かつ スイッチデータがある場合
		if (_physicsData->getEnableBySwitch() && _physicsData->getSwitchId() > 0) {

			// スイッチデータリスト取得
			cocos2d::__Array *switchDataList = cocos2d::__Array::create();
			auto rootObject = this->getRootObject();

			// 付随元オブジェクトがあり、自身の付随先オブジェクトのインスタンスが持つスイッチを参照する場合
			if (rootObject && _physicsData->getSwitchObjectId() == agtk::data::ObjectCommandData::kSelfObject) {
				auto switchData = rootObject->getPlayObjectData()->getSwitchData(_physicsData->getSwitchId());
				if (switchData) {
					switchDataList->addObject(switchData);
				}
			}
			else {
				GameManager::getInstance()->getSwitchVariableDataList(
					_physicsData->getSwitchQualifierId(),
					_physicsData->getSwitchObjectId(),
					_physicsData->getSwitchId(),
					true,
					switchDataList
				);
			}


			// 爆発スイッチのONチェック
			cocos2d::Ref *ref = nullptr;
			CCARRAY_FOREACH(switchDataList, ref) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
				auto switchData = static_cast<agtk::data::PlaySwitchData *>(ref);
#else
				auto switchData = dynamic_cast<agtk::data::PlaySwitchData *>(ref);
#endif
				_isActive |= (nullptr != switchData && GameManager::getInstance()->checkSwitchCondition(agtk::data::SwitchVariableConditionData::EnumSwitchValueType::kSwitchConditionOn, switchData->getValue(), switchData->isState()));
			}
		}

		// 検証用動作切替えキーがON かつ 爆発用キーがトリガーされた場合
		if (_physicsData->getUseVerificationKey() && InputManager::getInstance()->isTriggered(_physicsData->getKeyId())) {
			_isActive = true;
		}

		// 「動作をスイッチで切り替え」が OFF かつ 「検証用に動作切り替えキーを追加」が OFF の場合
		if (!_isActiveOnce && !_physicsData->getEnableBySwitch() && !_physicsData->getUseVerificationKey()) {
			// 無条件に爆発する
			_isActive = true;
			_isActiveOnce = true;
		}

		// 爆発実行する場合
		if (_isActive) {
			// 最大に鳴るまでの時間を設定
			_duration300 = _physicsData->getDuration300();
		}
	}

	// 爆発中の場合
	if (_isActive) {
		// 爆発方向の角度を算出
		int angle = (90 - (int)_physicsData->getDirection() + 360) % 360;

		// 連結元があり、連結したオブジェクトに合わせて回転する場合
		if (_connectedTarget && _physicsData->getRotateAccordingToConnectedObject()) {
			// 連結元の角度を加算する
			angle = (angle - (int)(_connectedTarget->getRotation() - _connectedInitAngle)) % 360;

#ifdef USE_PREVIEW
			// デバッグ表示がONの場合
			if (DebugManager::getInstance()->getShowPhysicsBoxEnabled()) {
				auto debugView = this->getChildByName("debugVisible");
				if (debugView) {
					debugView->setRotation((int)(_connectedTarget->getRotation() - _connectedInitAngle) % 360);
				}
			}
#endif
		}

		// ---------------------------------------------------------------------
		// レイキャストが物理オブジェクトに接触した場合のメソッド
		// ---------------------------------------------------------------------
		auto func = [](PhysicsWorld& world, const PhysicsRayCastInfo& info, void* data) -> bool {

			// 始点に存在するものがある場合
			if (info.contact.equals(info.end)) {
				// 次の接触点へ
				return true;
			}

			auto forceData = (ForceData *)data;
			auto group = info.shape->getGroup();

			// 「床・坂・力系」オブジェクトでない場合
			if (group > 0) {
				// 物体の物理ボディ取得
				auto body = info.shape->getBody();
				int targetLayerId = -1;

				// 通常オブジェクトの場合
				if (info.shape->getGroup() == GameManager::EnumPhysicsGroup::kObject || info.shape->getGroup() == GameManager::EnumPhysicsGroup::kNoneObject) {

					auto physicsNode = body->getOwner();

					// 連結先オブジェクトの場合
					if (physicsNode == forceData->getConnectTarget()) {
						// 力を与える必要がない
						return true;
					}
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
					auto object = static_cast<agtk::Object *>(physicsNode->getParent());
					targetLayerId = object->getLayerId();
#else
					targetLayerId = dynamic_cast<agtk::Object *>(physicsNode->getParent())->getLayerId();
#endif
				}
				// 物理オブジェクトの場合
				else {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
					auto physicsObj = static_cast<agtk::PhysicsBase *>(body->getOwner());
#else
					auto physicsObj = dynamic_cast<agtk::PhysicsBase *>(body->getOwner());
#endif

					// 連結先オブジェクトの場合
					if (physicsObj == forceData->getConnectTarget()) {
						// 力を与える必要がない
						return true;
					}

					targetLayerId = physicsObj->getLayerId();
				}

				// 力を与える対象が同じレイヤーIDの場合
				if (forceData->getLayerId() == targetLayerId) {
					auto divMax = forceData->getDivMax();
					auto blastPow = forceData->getStrength() / (divMax < 1 ? 1 : divMax);										// 爆発力
					auto nodePos = body->getOwner()->getPosition();																// 物体の座標
					auto nodeRad = CC_DEGREES_TO_RADIANS(body->getRotation());													// 物体の角度のラジアン値
					auto bodyOffset = body->getPositionOffset().rotateByAngle(Vec2::ZERO, -nodeRad);							// 物体のオフセット(角度込み)
					auto offset = (info.contact - (nodePos + bodyOffset)).rotateByAngle(Vec2::ZERO, nodeRad);	// レイが衝突した位置の物体中心からのオフセット(角度込み)

					// 力の方向を力が加わる物体の角度に合わせて変更する
					auto direction = forceData->getDirection().rotateByAngle(Vec2::ZERO, nodeRad);

					// 接触点からレイキャスト方向へ瞬間的な力を加える
					body->applyImpulse(direction * blastPow, offset);
				}
			}
			else {
				// タイルの場合
				if (group == GameManager::EnumPhysicsGroup::kTile) {
					// タイルを詰めているボックスの場合は無視する
					auto box = dynamic_cast<PhysicsShapeBox *>(info.shape);
					if (box) return true;

// #AGTK-NX
#ifdef STATIC_DOWN_CAST
					auto tile = static_cast<agtk::Tile *>(info.shape->getBody()->getOwner());
					auto segment = static_cast<PhysicsShapeEdgeSegment *>(info.shape);
#else
					auto tile = dynamic_cast<agtk::Tile *>(info.shape->getBody()->getOwner());
					auto segment = dynamic_cast<PhysicsShapeEdgeSegment *>(info.shape);
#endif
					auto line = segment->getPointB() - segment->getPointA();
					line.normalize();
					auto lineNormal = Vec2(-line.y, line.x);
					auto dot = lineNormal.dot(-info.normal);
					// タイルの内側から or 別レイヤーのタイルであれば次の接触点へ
					return  (dot >= 0 || tile->getLayerId() != forceData->getLayerId());
				}
				// 坂の場合
				else if (group == GameManager::EnumPhysicsGroup::kSlope) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
					auto slope = static_cast<agtk::Slope *>(info.shape->getBody()->getOwner());
#else
					auto slope = dynamic_cast<agtk::Slope *>(info.shape->getBody()->getOwner());
#endif
					// 別レイヤーの坂であれば次の接触点へ
					return slope->getLayerId() != forceData->getLayerId();
				}
				// 力系の場合
				else {
					// 問答無用で次の接触点へ
					return true;
				}
			}

			// 接触点走査終了
			return false;
		};

		// 経過時間による力の割合算出
		auto powRate = _physicsData->getDuration300() > 0 ? (_physicsData->getDuration300() - _duration300) / _physicsData->getDuration300() : 1;

		// 力学データ生成
		auto forceData = ForceData::create(this->getLayerId());

		// 力の強さを設定
		forceData->setStrength(_physicsData->getStrength() * powRate);

		// 力の作用する距離
		forceData->setDistance(_forceDistance);

		// ---------------------------------------------------------------------
		// 円形の場合
		if (_isFan) {
			// 爆発範囲を算出(方向と範囲を指定がOFFの場合は360度)
			int fanAngle = !_physicsData->getLimitDirectionRange() ? 360 : abs((int)_physicsData->getFanAngle());

			// 力の分割数を設定
			forceData->setDivMax(fanAngle);

			// 扇形の爆発範囲内をレイキャスト
			for (int idx = 0, max = fanAngle, startAngle = angle - max / 2; idx <= max; idx++) {
				auto rad = CC_DEGREES_TO_RADIANS(startAngle + idx);
				Vec2 direction = Vec2(cos(rad), sin(rad));

				// 自身の位置から指定方向へレイキャストする
				auto startPos = this->getPosition();
				auto endPos = startPos + (direction * _forceDistance);

				// 力学データに方向を設定
				forceData->setDirection(direction);

				// 連結先がある場合
				if (_connectedTarget) {
					// 連結先には力を与えないので回避用に保持する
					forceData->setConnectTarget(_connectedTarget);
				}

				// レイキャスト
				GameManager::getInstance()->getPhysicsWorld()->rayCast(func, startPos, endPos, forceData);
			}
		}
		// ---------------------------------------------------------------------
		// 矩形の場合
		else {
			// 自身の角度から方向ベクトルを算出
			auto rad = CC_DEGREES_TO_RADIANS(angle);
			Vec2 direction = Vec2(cos(rad), sin(rad));

			// 力の分割数を設定
			forceData->setDivMax(_physicsData->getBandWidth());

			// 力学データに方向を設定
			forceData->setDirection(direction);

			// 矩形の爆発範囲内をレイキャスト
			for (int idx = 0, max = _physicsData->getBandWidth(), start = max / 2; idx <= max; idx++) {

				// 自身の位置から指定方向へレイキャストする
				auto pos = this->getPosition();
				auto startPos = pos + Vec2(-direction.y, direction.x) * (start - idx);
				auto endPos = startPos + (direction * _forceDistance);

				// レイキャスト
				GameManager::getInstance()->getPhysicsWorld()->rayCast(func, startPos, endPos, forceData);
			}
		}

		if (_duration300 == 0) {
			_isActive = false;
		}
	}
}

void PhysicsExprosion::clear()
{
	PhysicsBase::clear();
	this->setPhysicsData(nullptr);
	this->setConnectedTarget(nullptr);
}

/**
* デバッグ表示
* @param	isShow	表示のON/OFF
*/
void PhysicsExprosion::showDebugVisible(bool isShow)
{
#ifdef USE_PREVIEW
	auto debugView = this->getChildByName("debugVisible");

	if (!debugView && isShow) {
		// 角度
		int angle = (90 - (int)_physicsData->getDirection() + 360) % 360;

		auto debugNode = DrawNode::create();

		// ---------------------------------------------------------------------
		// 円形の場合
		if (_isFan) {
			// 爆発範囲を算出(方向と範囲を指定がOFFの場合は360度)
			int fanAngle = !_physicsData->getLimitDirectionRange() ? 360 : abs((int)_physicsData->getFanAngle());

			// デバッグ用の円弧を描画
			drawDebugCircleOrFan(angle, fanAngle, _forceDistance, Color4F::RED, debugNode);
		}
		// ---------------------------------------------------------------------
		// 矩形の場合
		else {
			// 自身の角度から方向ベクトルを算出
			auto rad = CC_DEGREES_TO_RADIANS(angle);
			
			// デバッグ用の矩形描画
			drawDebugRectangle(Vec2(cos(rad), sin(rad)), _physicsData->getBandWidth() * 0.5f, _forceDistance, Color4F::RED, debugNode);
		}
		this->addChild(debugNode, 1, "debugVisible");
	}
	else if (debugView) {
		debugView->setVisible(isShow);
	}
#endif //USE_PREVIEW
}

//-------------------------------------------------------------------------------------------------------------------
//! 物理オブジェクト：引力・斥力
/**
* コンストラクタ
*/
PhysicsAttraction::PhysicsAttraction() : PhysicsBase()
{
	_physicsData = nullptr;
	_connectedAnchor = Vec2::ZERO;
	_connectedTarget = nullptr;
	_isActiveAttract = false;
	_isActiveRepulsive = false;
	_forceDistance = 0;
}

/**
* デストラクタ
*/
PhysicsAttraction::~PhysicsAttraction()
{
	CC_SAFE_RELEASE_NULL(_connectedTarget);
	CC_SAFE_RELEASE_NULL(_physicsData);
}

/**
* 初期化
* @param	physicsData		物理データ
* @param	sceneData		シーンデータ
* @param	layerId			レイヤーID
*/
bool PhysicsAttraction::init(agtk::data::PhysicsForceData *physicsData, agtk::data::SceneData *sceneData, int layerId, int parentScenePartId)
{
	if (!PhysicsBase::init(sceneData)) {
		return false;
	}

	this->setName("Attraction");

	// 親シーンパーツID設定
	this->setParentScenePartId(parentScenePartId);

	// 配置レイヤーID保持
	this->setLayerId(layerId);
	this->setSceneId(sceneData->getId());

	// 物理オブジェクトのアンカーポイントは中心
	this->setAnchorPoint(Vec2(0.5f, 0.5f));

	// 物理データを保持
	this->setPhysicsData(physicsData);

	// 座標を設定
	this->setPosition(Vec2::ZERO);

	// 扇形か？(扇タイプ or 方向と範囲を指定がOFF)
	this->setIsFan(physicsData->getrangeType() == RangeType::kRangeTypeFan || !physicsData->getLimitDirectionRange());

	// 動作をスイッチで切り替え と 検証用動作切り替えがOFFの場合
	if (!physicsData->getEnableBySwitch() && !_physicsData->getUseVerificationKey()) {
		// 引力or斥力の発生フラグを設定
		this->setIsActiveAttract(physicsData->getAttractiveForce());
		this->setIsActiveRepulsive(!physicsData->getAttractiveForce());
	}

	auto distance = physicsData->getDistance();

	// 距離無限の場合
	if (physicsData->getInfinite()) {
		// シーンの最も長い方を距離とする
		auto size = Director::getInstance()->getVisibleSize();
		size.width = size.width * sceneData->getHorzScreenCount();
		size.height = size.height * sceneData->getVertScreenCount();
		distance = max(size.width, size.height);
	}

	// 力の及ぶ距離を設定
	this->setForceDistance(distance);

#ifdef USE_PREVIEW
	// デバッグ表示がONの場合
	if (DebugManager::getInstance()->getShowPhysicsBoxEnabled()) {
		showDebugVisible(true);
	}
#endif

	return true;
}

/**
* 更新
* @param	dt	前フレームからの経過時間
*/
void PhysicsAttraction::update(float dt)
{
	// 連結元がある場合
	if (_connectedTarget) {
		// 連結基の角度と位置から自身の位置を更新
		auto pos = _connectedTarget->getPosition();
		auto rad = -CC_DEGREES_TO_RADIANS(_connectedTarget->getRotation());
		auto anchor = _connectedAnchor.rotateByAngle(Vec2::ZERO, rad);
		this->setPosition(pos + anchor);
	}

	// 動作をスイッチで切替えがONの場合
	if (_physicsData->getEnableBySwitch()) {

		// ワーク変数
		auto gameManager = GameManager::getInstance();
		auto rootObject = this->getRootObject();

		// スイッチデータチェックメソッド
		auto checkSwitchData = [](cocos2d::__Array * switchList)->bool {
			bool ret = false;
			cocos2d::Ref *ref = nullptr;
			CCARRAY_FOREACH(switchList, ref) {
				auto switchData = dynamic_cast<agtk::data::PlaySwitchData *>(ref);
				ret |= (nullptr != switchData && GameManager::getInstance()->checkSwitchCondition(agtk::data::SwitchVariableConditionData::EnumSwitchValueType::kSwitchConditionOn, switchData->getValue(), switchData->isState()));
			}
			return ret;
		};

		// 引力に切替えるスイッチがある場合
		if (_physicsData->getAttractiveForceSwitchId() > 0) {
			//引力に切替える用スイッチデータリスト取得
			cocos2d::__Array *switchDataList = cocos2d::__Array::create();

			// 付随元オブジェクトがあり、自身の付随先オブジェクトのインスタンスが持つスイッチを参照する場合
			if (rootObject && _physicsData->getAttractiveForceSwitchObjectId() == agtk::data::ObjectCommandData::kSelfObject) {
				auto switchData = rootObject->getPlayObjectData()->getSwitchData(_physicsData->getAttractiveForceSwitchId());
				if (switchData) {
					switchDataList->addObject(switchData);
				}
			}
			else {
				gameManager->getSwitchVariableDataList(
					_physicsData->getAttractiveForceSwitchQualifierId(),
					_physicsData->getAttractiveForceSwitchObjectId(),
					_physicsData->getAttractiveForceSwitchId(),
					true,
					switchDataList
				);
			}

			// スイッチの状態で引力の発生のON/OFFを切り替え
			bool isOn = checkSwitchData(switchDataList);
			if (_isActiveAttract != isOn) {
				_isActiveAttract = isOn;
			}
		}
		
		// 斥力に切り替えるスイッチがある場合
		if (_physicsData->getRepulsiveForceSwitchId() > 0) {
			//斥力に切替える用スイッチデータリスト取得
			cocos2d::__Array *switchDataList = cocos2d::__Array::create();

			// 付随元オブジェクトがあり、自身の付随先オブジェクトのインスタンスが持つスイッチを参照する場合
			if (rootObject && _physicsData->getRepulsiveForceSwitchObjectId() == agtk::data::ObjectCommandData::kSelfObject) {
				auto switchData = rootObject->getPlayObjectData()->getSwitchData(_physicsData->getRepulsiveForceSwitchId());
				if (switchData) {
					switchDataList->addObject(switchData);
				}
			}
			else {
				gameManager->getSwitchVariableDataList(
					_physicsData->getRepulsiveForceSwitchQualifierId(),
					_physicsData->getRepulsiveForceSwitchObjectId(),
					_physicsData->getRepulsiveForceSwitchId(),
					true,
					switchDataList
				);
			}


			// スイッチの状態で斥力の発生のON/OFFを切り替え
			bool isOn = checkSwitchData(switchDataList);
			if (_isActiveRepulsive != isOn) {
				_isActiveRepulsive = isOn;
			}
		}
	}

	// 検証用動作切替えキーがON かつ 爆発用キーがトリガーされた場合
	if (_physicsData->getUseVerificationKey()) {

		// 引力へ切替え用のキーがトリガーされた場合
		if (InputManager::getInstance()->isTriggered(_physicsData->getAttractiveForceKeyId())) {
			_isActiveAttract = true;
		}
		// 斥力へ切替え用のキーがトリガーされた場合
		if (InputManager::getInstance()->isTriggered(_physicsData->getRepulsiveForceKeyId())) {
			_isActiveRepulsive = true;
		}
	}

	// 力の発生方向の角度を算出
	int angle = (90 - (int)_physicsData->getDirection() + 360) % 360;

	// 連結元があり、連結したオブジェクトに合わせて回転する場合
	if (_connectedTarget && _physicsData->getRotateAccordingToConnectedObject()) {
		// 連結元の角度を加算する
		angle = (angle - (int)(_connectedTarget->getRotation() - _connectedInitAngle)) % 360;

#ifdef USE_PREVIEW
		// デバッグ表示がONの場合
		if (DebugManager::getInstance()->getShowPhysicsBoxEnabled()) {
			auto debugView = this->getChildByName("debugVisible");
			if (debugView) {
				debugView->setRotation((int)(_connectedTarget->getRotation() - _connectedInitAngle) % 360);
			}
		}
#endif
	}

	// ---------------------------------------------------------------------
	// レイキャストが物理オブジェクトに接触した場合のメソッド
	// ---------------------------------------------------------------------
	auto func = [](PhysicsWorld& world, const PhysicsRayCastInfo& info, void* data) -> bool {

		// 始点に存在するものがある場合
		if (info.contact.equals(info.end)) {
			// スキップ
			return true;
		}

		// 「床・坂・力系」オブジェクトでない場合
		if (info.shape->getGroup() > 0) {

			auto forceData = (ForceData *)data;
			auto body = info.shape->getBody();
			int targetLayerId = -1;

			if (info.shape->getGroup() == GameManager::EnumPhysicsGroup::kObject || info.shape->getGroup() == GameManager::EnumPhysicsGroup::kNoneObject) {

				auto physicsNode = body->getOwner();

				// 連結先オブジェクトの場合
				if (physicsNode == forceData->getConnectTarget()) {
					// 力を与える必要がない
					return true;
				}

// #AGTK-NX
#ifdef STATIC_DOWN_CAST
				auto object = static_cast<agtk::Object *>(physicsNode->getParent());
#else
				auto object = dynamic_cast<agtk::Object *>(physicsNode->getParent());
#endif
				auto physicsSetting = object->getObjectData()->getPhysicsSetting();
				auto playObjectData = object->getPlayObjectData();

				// 接続された物理オブジェクトの動作を優先する場合
				if (playObjectData->getSwitchData(agtk::data::kObjectSystemSwitchFollowConnectedPhysics)->getValue()) {

					// 力を与えない
					return true;
				}

				targetLayerId = object->getLayerId();
			}
			else {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
				auto physicsObj = static_cast<agtk::PhysicsBase *>(body->getOwner());
#else
				auto physicsObj = dynamic_cast<agtk::PhysicsBase *>(body->getOwner());
#endif

				// 連結先オブジェクトの場合
				if (physicsObj == forceData->getConnectTarget()) {
					// 力を与える必要がない
					return true;
				}

				targetLayerId = physicsObj->getLayerId();
			}

			// 力を与える対象が同じレイヤーIDの場合
			if (forceData->getLayerId() == targetLayerId) {

				auto divMax = forceData->getDivMax();
				auto distance = info.contact.getDistance(info.start);																	// 始点から接触点までの距離
				auto distanceDamping = min((forceData->getIsConstant() ? 1.0f : 1.0f - (distance / forceData->getDistance())), 1.0f);	// 距離での減衰率
				auto forcePow = forceData->getStrength() * distanceDamping / (divMax < 1 ? 1 : divMax);									// 力
				auto nodePos = body->getOwner()->getPosition();																			// 物体の座標
				auto nodeRad = CC_DEGREES_TO_RADIANS(body->getRotation());																// 物体の角度のラジアン値
				auto bodyOffset = body->getPositionOffset().rotateByAngle(Vec2::ZERO, -nodeRad);										// 物体のオフセット(角度込み)
				auto offset = (info.contact - (nodePos + bodyOffset)).rotateByAngle(Vec2::ZERO, nodeRad);				// レイが衝突した位置の物体中心からのオフセット(角度込み)

				// 力の方向を力が加わる物体の角度に合わせて変更する
				auto direction = forceData->getDirection().rotateByAngle(Vec2::ZERO, nodeRad);

				// 接触点へ力を加える
				body->applyForce(direction * forcePow, offset);
			}

			return true;
		}

		return false;
	};

	// 力学データ生成
	auto forceData = ForceData::create(this->getLayerId());

	// 力の強さを設定
	forceData->setStrength(_physicsData->getStrength());

	// 力の作用は一定かのデータを設定
	forceData->setIsConstant(_physicsData->getConstantForce());

	// 力の作用する距離
	forceData->setDistance(_forceDistance);

	// ---------------------------------------------------------------------
	// 円形の場合
	if (_isFan) {
		// 力の範囲を算出(方向と範囲を指定がOFFの場合は360度)
		int fanAngle = !_physicsData->getLimitDirectionRange() ? 360 : abs((int)_physicsData->getFanAngle());

		// 力の分割数を設定
		forceData->setDivMax(fanAngle);

		// 扇形の力の範囲内をレイキャスト
		for (int idx = 0, max = fanAngle, startAngle = angle - max / 2; idx <= max; idx++) {

			auto rad = CC_DEGREES_TO_RADIANS(startAngle + idx);
			Vec2 direction = Vec2(cos(rad), sin(rad));

			// 自身の位置から指定方向へレイキャストする
			auto startPos = this->getPosition();
			auto endPos = startPos + (direction * _forceDistance);

			// 引力がONの場合
			if (_isActiveAttract) {
				// 力学データに方向を設定
				forceData->setDirection(direction * ATTRACT_DIRECTION);

				// レイキャスト
				GameManager::getInstance()->getPhysicsWorld()->rayCast(func, startPos, endPos, forceData);
			}

			// 斥力がONの場合
			if (_isActiveRepulsive) {
				// 力学データに方向を設定
				forceData->setDirection(direction * REPULSIVE_DIRECTION);

				// レイキャスト
				GameManager::getInstance()->getPhysicsWorld()->rayCast(func, startPos, endPos, forceData);
			}

			// 連結先がある場合
			if (_connectedTarget) {
				// 連結先には力を与えないので回避用に保持する
				forceData->setConnectTarget(_connectedTarget);
			}
		}
	}
	// ---------------------------------------------------------------------
	// 矩形の場合
	else {
		// 自身の角度から方向ベクトルを算出
		auto rad = CC_DEGREES_TO_RADIANS(angle);
		Vec2 direction = Vec2(cos(rad), sin(rad));

		// 力の分割数を設定
		forceData->setDivMax(_physicsData->getBandWidth());

		// 矩形の爆発範囲内をレイキャスト
		for (int idx = 0, max = _physicsData->getBandWidth(), start = max / 2; idx <= max; idx++) {

			// 自身の位置から指定方向へレイキャストする
			auto pos = this->getPosition();
			auto startPos = pos + Vec2(-direction.y, direction.x) * (start - idx);
			auto endPos = startPos + (direction * _forceDistance);

			// 引力がONの場合
			if (_isActiveAttract) {
				// 力学データに方向を設定
				forceData->setDirection(direction * ATTRACT_DIRECTION);

				// レイキャスト
				GameManager::getInstance()->getPhysicsWorld()->rayCast(func, startPos, endPos, forceData);
			}

			// 斥力がONの場合
			if (_isActiveRepulsive) {
				// 力学データに方向を設定
				forceData->setDirection(direction * REPULSIVE_DIRECTION);

				// レイキャスト
				GameManager::getInstance()->getPhysicsWorld()->rayCast(func, startPos, endPos, forceData);
			}

			// 連結先がある場合
			if (_connectedTarget) {
				// 連結先には力を与えないので回避用に保持する
				forceData->setConnectTarget(_connectedTarget);
			}

		}
	}
}

void PhysicsAttraction::clear()
{
	PhysicsBase::clear();
	this->setConnectedTarget(nullptr);
}

/**
* デバッグ表示
* @param	isShow	表示のON/OFF
*/
void PhysicsAttraction::showDebugVisible(bool isShow)
{
#ifdef USE_PREVIEW
	auto debugView = this->getChildByName("debugVisible");

	if (!debugView && isShow) {
		// 角度
		int angle = (90 - (int)_physicsData->getDirection() + 360) % 360;

		auto debugNode = DrawNode::create();

		// ---------------------------------------------------------------------
		// 円形の場合
		if (_isFan) {
			// 爆発範囲を算出(方向と範囲を指定がOFFの場合は360度)
			int fanAngle = !_physicsData->getLimitDirectionRange() ? 360 : abs((int)_physicsData->getFanAngle());

			// デバッグ用の円弧を描画
			drawDebugCircleOrFan(angle, fanAngle, _forceDistance, Color4F::RED, debugNode);
		}
		// ---------------------------------------------------------------------
		// 矩形の場合
		else {
			// 自身の角度から方向ベクトルを算出
			auto rad = CC_DEGREES_TO_RADIANS(angle);
			Vec2 direction = Vec2(cos(rad), sin(rad));

			// デバッグ用の矩形描画
			drawDebugRectangle(Vec2(cos(rad), sin(rad)), _physicsData->getBandWidth() * 0.5f, _forceDistance, Color4F::RED, debugNode);
		}
		this->addChild(debugNode, 1, "debugVisible");
	}
	else if (debugView) {
		debugView->setVisible(isShow);
	}
#endif //USE_PREVIEW
}
NS_AGTK_END
