#include "Slope.h"
#include "Scene.h"
#include "Object.h"
#include "Collision.h"
#include "Manager/GameManager.h"
#include "Manager/DebugManager.h"

NS_AGTK_BEGIN

int Slope::_serial = 1;
//-------------------------------------------------------------------------------------------------------------------
const float Slope::SLIP_MAX_FRAME = 30.0f / 60.0f * 300.0f; // 坂の影響度が最大になるまでのフレーム（30フレームで設定）

//-------------------------------------------------------------------------------------------------------------------
Slope::Slope()
{
	_layerId = -1;
	_canMoveStartSlope = false;
	_canMoveEndSlope = false;

	_slopeData = nullptr;
	_connectStartSlope = nullptr;
	_connectEndSlope = nullptr;
	_firstUpdateWorldSpaceRectFlag = false;

	_worldSpaceRect = Rect::ZERO;
	_worldStartPoint = Point::ZERO;
	_worldEndPoint = Point::ZERO;
	_id = _serial;
	_serial = (_serial % 10000);
	_objectTmp = nullptr;
}

Slope::~Slope()
{
	CC_SAFE_RELEASE_NULL(_slopeData);

	_connectStartSlope = nullptr;
	_connectEndSlope = nullptr;
}

bool Slope::init(agtk::data::OthersSlopeData* slopeData, int layerId, agtk::data::SceneData* sceneData)
{
	_move = true;
	_layerId = layerId;

	float x = 0;
	float y = 0;
	float w = 0;
	float h = 0;

	if (slopeData->getStartX() > slopeData->getEndX())
	{
		x = slopeData->getEndX();
		w = slopeData->getStartX() - x;

	}
	else
	{
		x = slopeData->getStartX();
		w = slopeData->getEndX() - x;
	}

	if (slopeData->getStartY() > slopeData->getEndY())
	{
		y = slopeData->getEndY();
		h = slopeData->getStartY() - y;
	}
	else
	{
		y = slopeData->getStartY();
		h = slopeData->getEndY() - y;
	}

	_rect = cocos2d::Rect(x, y, w, h);

	setSlopeData(slopeData);

	// 坂の角度を取得
	Vec2 v1 = Vec2(slopeData->getEndX() - slopeData->getStartX(), slopeData->getEndY() - slopeData->getStartY());
	v1.normalize();
	Vec2 v2 = Vec2(1, 0);

	float dot = Vec2::dot(v1, v2);
	_degree = CC_RADIANS_TO_DEGREES(acosf(dot));

	// 右上方向フラグ
	bool isRightUp = true;

	// 坂が右上方向の場合
	if (v1.x >= 0 && v1.y < 0)
	{
		start = Point(slopeData->getStartX(), slopeData->getStartY());
		end = Point(slopeData->getEndX(), slopeData->getEndY());

	}
	// 坂が右下方向の場合
	else if (v1.x >= 0 && v1.y >= 0)
	{
		start = Point(slopeData->getStartX(), slopeData->getStartY());
		end = Point(slopeData->getEndX(), slopeData->getEndY());

		_degree *= -1;

		isRightUp = false;

	}
	// 坂が左上方向の場合
	else if (v1.x < 0 && v1.y < 0)
	{
		// 坂が右下方向になるように座標を設定
		end = Point(slopeData->getStartX(), slopeData->getStartY());
		start = Point(slopeData->getEndX(), slopeData->getEndY());

		_degree = -(180 - _degree);

		isRightUp = false;
	}
	// 坂が左下方向の場合
	else
	{
		// 坂が右上方向になるように座標を設定
		end = Point(slopeData->getStartX(), slopeData->getStartY());
		start = Point(slopeData->getEndX(), slopeData->getEndY());

		_degree = 180 - _degree;
	}

	// 坂の種類を設定
	_type = (_degree >= 0) ? kTypeUp : kTypeDown;

	// 通過可能設定
	_passableFromUpper = slopeData->getPassableFromUpper();
	_passableFromLower = slopeData->getPassableFromLower();

	// ------------------------------------------------------------------------------
	// 物理オブジェクト設定
	// ------------------------------------------------------------------------------
	// 物理演算ボディ設定
	auto physicsMaterial = PHYSICSBODY_MATERIAL_DEFAULT;
	physicsMaterial.restitution = slopeData->getPhysicsRepulsion();
	physicsMaterial.friction = slopeData->getPhysicsFriction();

	auto adjust = 1.0f;
	auto segment = PhysicsBody::createEdgeSegment(
		agtk::Scene::getPositionCocos2dFromScene(start + (isRightUp ? Point(adjust, -adjust) : Point(adjust, adjust)), sceneData),
		agtk::Scene::getPositionCocos2dFromScene(end + (isRightUp ? Point(-adjust, adjust) : Point(-adjust, -adjust)), sceneData),
		physicsMaterial
	);
	segment->setDynamic(false);
	segment->setGroup(GameManager::EnumPhysicsGroup::kSlope);
	segment->setContactTestBitmask(INT_MAX);
	auto physicsLayer = (layerId - 1);
	if (sceneData->getId() == agtk::data::SceneData::kMenuSceneId) {
		physicsLayer = physicsLayer + agtk::PhysicsBase::kMenuPhysicsLayerShift;
	}
	auto bitmask = (1 << physicsLayer);
	segment->setCategoryBitmask(bitmask);
	segment->setCollisionBitmask(bitmask);
	this->setPhysicsBody(segment);

	return true;
}

bool Slope::init(cocos2d::Vec2 p0, cocos2d::Vec2 p1, bool passableUpper, bool passableLower)
{
	_move = false;

	float x = 0;
	float y = 0;
	float w = 0;
	float h = 0;

	if (p0.x > p1.x)
	{
		x = p1.x;
		w = p0.x - x;

	}
	else
	{
		x = p0.x;
		w = p1.x - x;
	}

	if (p0.y > p1.y)
	{
		y = p1.y;
		h = p0.y - y;
	}
	else
	{
		y = p0.y;
		h = p1.y - y;
	}

	_rect = cocos2d::Rect(x, y, w, h);

	//setSlopeData(slopeData);

	// 坂の角度を取得
	Vec2 v1 = Vec2(p1.x - p0.x, p1.y - p0.y);
	v1.normalize();
	Vec2 v2 = Vec2(1, 0);

	float dot = Vec2::dot(v1, v2);
	_degree = CC_RADIANS_TO_DEGREES(acosf(dot));

	// 坂が右上方向の場合
	if (v1.x >= 0 && v1.y < 0)
	{
		start = Point(p0.x, p0.y);
		end = Point(p1.x, p1.y);

	}
	// 坂が右下方向の場合
	else if (v1.x >= 0 && v1.y >= 0)
	{
		start = Point(p0.x, p0.y);
		end = Point(p1.x, p1.y);

		_degree *= -1;

	}
	// 坂が左上方向の場合
	else if (v1.x < 0 && v1.y < 0)
	{
		// 坂が右下方向になるように座標を設定
		end = Point(p0.x, p0.y);
		start = Point(p1.x, p1.y);

		_degree = -(180 - _degree);

	}
	// 坂が左下方向の場合
	else
	{
		// 坂が右上方向になるように座標を設定
		end = Point(p0.x, p0.y);
		start = Point(p1.x, p1.y);

		_degree = 180 - _degree;
	}

	// 坂の種類を設定
	_type = (_degree >= 0) ? kTypeUp : kTypeDown;

	// 通過可能設定
	_passableFromUpper = passableUpper;
	_passableFromLower = passableLower;

	// 360度ループに物理設定はないので
	// 物理オブジェクトの設定は行わない

	return true;
}

bool Slope::checkHitLine(CollisionLine* line, cocos2d::Vec2& cross)
{
	// 事前にupdateWorldSpaceRect関数を呼び出し
	// ワールド空間での坂の情報を更新しておく必要がある
	CollisionLine slopeLine = { &_worldStartPoint, &_worldEndPoint };

	if (CollisionUtils::checkPushCross2(&slopeLine, line, &cross))
	{
		return true;
	}

	return false;
}

bool Slope::checkHitRect(cocos2d::Rect& rect, bool bUpdateWorldSpaceRectFlag)
{
	// 事前にワールド空間での坂の矩形情報を更新
	if (bUpdateWorldSpaceRectFlag && _firstUpdateWorldSpaceRectFlag == false) {
		updateWorldSpaceRect();
	}

	// 当たりチェック用の線分
	cocos2d::Point sp;
	cocos2d::Point ep;
	CollisionLine line = { &sp ,&ep };

	// 当たり座標格納用変数
	Vec2 crossUp;
	Vec2 crossDown;
	Vec2 crossLeft;
	Vec2 crossRight;

	// 上辺のチェック
	{
		sp.x = rect.getMinX();
		ep.x = rect.getMaxX();
		sp.y = rect.getMaxY();
		ep.y = rect.getMaxY();

		if (checkHitLine(&line, crossUp)) {
			return true;
		}
	}

	// 下辺のチェック
	{
		sp.y = rect.getMinY();
		ep.y = rect.getMinY();

		if (checkHitLine(&line, crossDown)) {
			return true;
		}
	}

	// 左辺のチェック
	{
		sp.x = rect.getMinX();
		ep.x = rect.getMinX();
		sp.y = rect.getMinY();
		ep.y = rect.getMaxY();

		if (checkHitLine(&line, crossLeft)) {
			return true;
		}
	}

	// 右辺のチェック
	{
		sp.x = rect.getMaxX();
		ep.x = rect.getMaxX();

		if (checkHitLine(&line, crossRight)) {
			return true;
		}
	}

	return  false;
}

bool Slope::checkHitRect(cocos2d::Rect& current, cocos2d::Rect&prev, bool& isHitPrev)
{
	isHitPrev = false;

	// 過去の矩形が坂と接触しているかをチェック1
	if (checkHitRect(prev)) {
		//CCLOG("過去の矩形がHIT");

		isHitPrev = true;
		return true;
	}

	// 現在の矩形が坂と接触しているかをチェック
	if (checkHitRect(current)) {
		//CCLOG("現在の矩形がHIT");
		return true;
	}

	// 進行方向のベクトルを取得
	Vec2 moveVec = Vec2(current.getMidX(), current.getMidY()) - Vec2(prev.getMidX(), prev.getMidY());

	// 矩形二つを補間する当たり判定用の線３つ
	cocos2d::Point sp1;
	cocos2d::Point ep1;
	cocos2d::Point sp2;
	cocos2d::Point ep2;
	cocos2d::Point sp3 = Point(prev.getMidX(), prev.getMidY());
	cocos2d::Point ep3 = Point(current.getMidX(), current.getMidY());

	CollisionLine line1 = { &sp1, &ep1 };
	CollisionLine line2 = { &sp2, &ep2 };
	CollisionLine line3 = { &sp3, &ep3 };

	// 移動方向に応じて補間する線を設定

	// 右に移動した場合
	if (moveVec.x > 0 && moveVec.y == 0) {

		sp1 = Point(prev.getMaxX(), prev.getMaxY());
		ep1 = Point(current.getMinX(), current.getMaxY());

		sp2 = Point(prev.getMaxX(), prev.getMinY());
		ep2 = Point(current.getMinX(), current.getMinY());

		//CCLOG("右移動");
	}
	// 左に移動した場合
	else if (moveVec.x < 0 && moveVec.y == 0) {
// #AGTK-NX #AGTK-WIN
#ifdef USE_SAR_PROVISIONAL
		sp1 = Point(prev.getMinX(), prev.getMaxY());
#else
		sp1 = Point(prev.getMidX(), prev.getMaxY());
#endif
		ep1 = Point(current.getMaxX(), current.getMaxY());

// #AGTK-NX #AGTK-WIN
#ifdef USE_SAR_PROVISIONAL
		sp2 = Point(prev.getMinX(), prev.getMinY());
#else
		sp2 = Point(prev.getMidX(), prev.getMinY());
#endif
		ep2 = Point(current.getMaxX(), current.getMinY());

		//CCLOG("左移動");
	}
	// 上に移動した場合
	else if (moveVec.x == 0 && moveVec.y > 0) {

		sp1 = Point(prev.getMaxX(), prev.getMaxY());
		ep1 = Point(current.getMaxX(), current.getMinY());

		sp2 = Point(prev.getMinX(), prev.getMaxY());
		ep2 = Point(current.getMinX(), current.getMinY());

		//CCLOG("上移動");
	}
	// 下に移動した場合
	else if (moveVec.x == 0 && moveVec.y < 0) {

		sp1 = Point(prev.getMaxX(), prev.getMinY());
		ep1 = Point(current.getMaxX(), current.getMaxY());

		sp2 = Point(prev.getMinX(), prev.getMinY());
		ep2 = Point(current.getMinX(), current.getMaxY());

		//CCLOG("下移動");
	}
	// 右上に移動した場合
	else if (moveVec.x > 0 && moveVec.y > 0) {

		sp1 = Point(prev.getMinX(), prev.getMaxY());
		ep1 = Point(current.getMinX(), current.getMaxY());

		sp2 = Point(prev.getMaxX(), prev.getMinY());
		ep2 = Point(current.getMaxX(), current.getMinY());

		//CCLOG("右上移動");
	}
	// 右下に移動した場合
	else if (moveVec.x > 0 && moveVec.y < 0) {

		sp1 = Point(prev.getMinX(), prev.getMinY());
		ep1 = Point(current.getMinX(), current.getMinY());

		sp2 = Point(prev.getMaxX(), prev.getMaxY());
		ep2 = Point(current.getMaxX(), current.getMaxY());

		//CCLOG("右下移動");
	}
	// 左上に移動した場合
	else if (moveVec.x < 0 && moveVec.y > 0) {

		sp1 = Point(prev.getMinX(), prev.getMinY());
		ep1 = Point(current.getMinX(), current.getMinY());

		sp2 = Point(prev.getMaxX(), prev.getMaxY());
		ep2 = Point(current.getMaxX(), current.getMaxY());

		//CCLOG("左上移動");
	}
	// 左下に移動した場合
	else if (moveVec.x < 0 && moveVec.y < 0) {

		sp1 = Point(prev.getMinX(), prev.getMaxY());
		ep1 = Point(current.getMinX(), current.getMaxY());

		sp2 = Point(prev.getMaxX(), prev.getMinY());
		ep2 = Point(current.getMaxX(), current.getMinY());

		//CCLOG("左下移動");

	}
	Vec2 cross;
	if (checkHitLine(&line1, cross)) {
		//CCLOG("補間１がHIT");
		return true;
	}

	if (checkHitLine(&line2, cross)) {
		//CCLOG("補間２がHIT");
		return true;
	}

	if (checkHitLine(&line3, cross)) {
		//CCLOG("補間３がHIT");
		return true;
	}

	return false;
}

bool Slope::simpleCheckHit(Rect crntRect, Rect oldRect, Vec2& moveVec, bool touched, bool& touchPassable)
{
	// 上からも下からも通過できる坂の場合は、通り抜ける
	if (_passableFromUpper && _passableFromLower) { 
		// 通過可能な坂に接触中
		touchPassable = true;
		return false;
	}

	// 事前にワールド空間での坂の矩形情報を更新
	updateWorldSpaceRect();

	Rect prevRect = Rect(crntRect.origin - moveVec, crntRect.size);

	// オブジェクトの矩形をもとに坂の上下にいるかを取得する
// #AGTK-NX
#if CC_TARGET_PLATFORM != CC_PLATFORM_NX
	bool isCrntUpSide = checkUpSide(Vec2(crntRect.getMidX(), crntRect.getMidY()));
	bool isPrevUpSide = checkUpSide(Vec2(oldRect.getMidX(), oldRect.getMidY()));
#else
#endif

	// 上から通過可能な坂で、上から下へ抜けた場合は通り抜ける
	if (_passableFromUpper && isPrevUpSide) {
		// 通過可能な坂に接触中
		touchPassable = true;
		return false;
	}
	// 下から通過可能な坂で、下から上へ抜けた場合は通り抜ける
	else if (_passableFromLower && !isPrevUpSide) {
		// 通過可能な坂に接触中
		touchPassable = true;
		return false;
	}

	// 上から通過できない坂で、過去の位置が坂の上にいる場合
	if (!_passableFromUpper && isPrevUpSide) {
		// 現在の位置も坂の上にいる
		isCrntUpSide = isPrevUpSide;
	}
	// 下から通過できない坂で、過去の位置が坂の下にいる場合
	else if (!_passableFromLower && !isPrevUpSide) {
		// 現在の位置も坂の下にいる
		isCrntUpSide = isPrevUpSide;
	}

	bool isPrevHit = false;

	float addHeight = abs(sinf(CC_DEGREES_TO_RADIANS(_degree)) * moveVec.x) + 1.0f;
	addHeight = isCrntUpSide ? addHeight : -addHeight;
	//	CCLOG("addHeight : %f", addHeight);

	// 前フレームの矩形が坂に接触するか
	bool isOldHit = false;
	oldRect.origin.y -= addHeight;
	oldRect.size.height += abs(addHeight);
	if (checkHitRect(oldRect, oldRect, isOldHit)) {
		//		CCLOG("前フレームの矩形が坂に接触");
	}
	oldRect.origin.y += addHeight;
	oldRect.size.height -= abs(addHeight);

	// 過去の矩形から現在の矩形までの間で坂に接触した場合
	bool isHitSlope = false;
	prevRect.origin.y -= addHeight;
	prevRect.size.height += abs(addHeight);
	if (checkHitRect(crntRect, prevRect, isPrevHit)) {
		isHitSlope = true;
	}
	prevRect.origin.y += addHeight;
	prevRect.size.height -= abs(addHeight);


	// 過去の矩形から現在の矩形までの間で坂に接触した場合
	if (isHitSlope || isOldHit) {

		// 通過可能な坂に接触状態の場合
		if (touched) {
			// 通過可能な坂に接触中とする
			touchPassable = true;
			// 通過可能な坂に接触中なので坂に当たらない状態とする
			return false;
		}
		return true;
	}
	return false;
}

#ifdef USE_MULTITHREAD_OBJECT_UPDATE
bool Slope::checkHit(Rect crntRect, Rect oldRect, Vec2& moveVec, bool touched, bool jumping, bool& hitUp, bool& hitDown, bool& touchPassable, bool isSlip, float touchFrame300, Object *object, std::vector<agtk::Tile *> &tileList)
#else
bool Slope::checkHit(Rect crntRect, Rect oldRect, Vec2& moveVec, bool touched, bool jumping, bool& hitUp, bool& hitDown, bool& touchPassable, bool isSlip, float touchFrame300, Object *object, cocos2d::Array *tileList)
#endif
{
	_objectTmp = object;//一時オブジェクト保持。

	//オブジェクト参照破棄。
	struct ObjectRelease {
		ObjectRelease(Object** pObject) { _pObject = pObject; }
		~ObjectRelease() { *_pObject = nullptr ; }
	private:
		Object** _pObject;
	} objectRelease(&_objectTmp);

	hitUp = false;
	hitDown = false;

	// 上からも下からも通過できる坂の場合は、通り抜ける
	if (_passableFromUpper && _passableFromLower) { 
		// 通過可能な坂に接触中
		touchPassable = true;
		return false;
	}

	// ワールド空間での坂の矩形情報を更新しておく
	updateWorldSpaceRect();

	Rect prevRect = Rect(crntRect.origin - moveVec, crntRect.size);

	// オブジェクトの矩形をもとに坂の上下にいるかを取得する
// #AGTK-NX
#if CC_TARGET_PLATFORM != CC_PLATFORM_NX
	bool isCrntUpSide = checkUpSide(Vec2(crntRect.getMidX(), crntRect.getMidY()));
	bool isPrevUpSide = checkUpSide(Vec2(oldRect.getMidX(), oldRect.getMidY()));
#else
#endif
	bool isCrntUpSideTmp = isCrntUpSide;//一時保持。

	// オブジェクトが坂をすり抜けたをチェックする。
	if (isPrevUpSide == false && isCrntUpSide == true) {
		auto sp = Point(oldRect.getMidX(), oldRect.getMidY());
		auto ep = Point(crntRect.getMidX(), crntRect.getMidY());
		CollisionLine line = { &sp, &ep };
		cocos2d::Vec2 cross;
		if (checkHitLine(&line, cross)) {
			isCrntUpSideTmp = isPrevUpSide;
		}
	}

	// 上から通過可能な坂で、上から下へ抜けた場合は通り抜ける
	if (_passableFromUpper && isPrevUpSide) {
		// 通過可能な坂に接触中
		touchPassable = true;

		return false;
	}
	// 下から通過可能な坂で、下から上へ抜けた場合は通り抜ける
	else if (_passableFromLower && !isPrevUpSide) {
		// 通過可能な坂に接触中
		touchPassable = true;

		return false;
	}

	// 上から通過できない坂で、過去の位置が坂の上にいる場合
	if (!_passableFromUpper && isPrevUpSide) {
		// 現在の位置も坂の上にいる
		isCrntUpSide = isPrevUpSide;
	}
	// 下から通過できない坂で、過去の位置が坂の下にいる場合
	else if (!_passableFromLower && !isPrevUpSide) {
		// 現在の位置も坂の下にいる
		isCrntUpSide = isPrevUpSide;
	}

	bool isPrevHit = false;

	float addHeight = 0.2f;

	// 前フレームの矩形が坂に接触するか
	bool isOldHit = false;
	oldRect.origin.y -= addHeight;
	oldRect.size.height += addHeight;
	if (checkHitRect(oldRect)) {
		//CCLOG("前フレームの矩形が坂に接触");
		isOldHit = true;
	}
	oldRect.origin.y += addHeight;
	oldRect.size.height -= addHeight;

	addHeight = abs(sinf(CC_DEGREES_TO_RADIANS(_degree)) * moveVec.x) + 0.2f;
	addHeight = (isCrntUpSideTmp) ? addHeight : -addHeight;
	//	CCLOG("addHeight : %f", addHeight);

	// 過去の矩形から現在の矩形までの間で坂に接触した場合
	bool isHitSlope = false;
	prevRect.origin.y -= addHeight;
	prevRect.size.height += abs(addHeight);
	if (checkHitRect(crntRect, prevRect, isPrevHit)) {
		isHitSlope = true;
	}
	prevRect.origin.y += addHeight;
	prevRect.size.height -= abs(addHeight);

	auto iniMoveVec = moveVec;
	// 過去の矩形から現在の矩形までの間で坂に接触した場合
	if (isHitSlope || isOldHit) {
		if (isOldHit) {
			prevRect = oldRect;
		}

		// 通過可能な坂に接触状態の場合
		if (touched) {
			// 通過可能な坂に接触中とする
			touchPassable = true;
			// 通過可能な坂に接触中なので坂に当たらない状態とする
			return false;
		}

		//タイルにぶつかっているかチェック。
// #AGTK-NX
#if CC_TARGET_PLATFORM != CC_PLATFORM_NX
		std::function<bool(std::vector<agtk::Tile *>, cocos2d::Vec2)> checkColliedTile = [](std::vector<agtk::Tile *> &tileList, cocos2d::Vec2 v) {
#else
#endif
#ifdef USE_MULTITHREAD_OBJECT_UPDATE
			if (tileList.size() > 0) {
				for (auto tile : tileList) {
#else
			if (tileList != nullptr) {
				cocos2d::Ref *ref;
				CCARRAY_FOREACH(tileList, ref) {
					auto tile = dynamic_cast<agtk::Tile *>(ref);
#endif
					auto tileRect = tile->convertToLayerSpaceRect();
					if (tileRect.containsPoint(v)) {
						return true;
					}
				}
			}
			return false;
		};

		cocos2d::Point sp;
		cocos2d::Point ep;
		CollisionLine line = { &sp, &ep };

		Vec2 cross;

		// 上り坂の場合
		if (getType() == kTypeUp) {
			// 坂の上にいる場合
			if (isCrntUpSide || isCrntUpSideTmp) {

				// 前のフレームで坂に接触状態でジャンプ中の場合、
				// 坂への張り付き防止のために坂に当たらない状態とする
				if (isOldHit && jumping/* && moveVec.x <= 0*/) {
					return false;
				}

				//坂下の端で、足元が坂の下の場合。
				if (prevRect.getMaxX() == _worldStartPoint.x && prevRect.getMinY() < _worldStartPoint.y) {
					return false;
				}
				//坂下の端で、足元が坂の下の場合。
				if (prevRect.getMinX() == _worldEndPoint.x && prevRect.getMinY() < _worldEndPoint.y) {
					return false;
				}

				sp = Point(prevRect.getMaxX(), _worldStartPoint.y);
				ep = Point(prevRect.getMaxX(), _worldEndPoint.y);

				Vec2 cross;
				if (checkHitLine(&line, cross)) {
					prevRect.origin.x = cross.x - prevRect.size.width;
					prevRect.origin.y = cross.y;
				}
				else {
					if (_worldStartPoint.x != _worldEndPoint.x) {//坂が垂直以外

						//坂の端にあるかチェックする。
						enum EnumCheckHit {
							kCheckHitNone,
							kCheckHitStartPoint,
							kCheckHitEndPoint,
						} checkHit = kCheckHitNone;

						bool bTileConnect = false;
#ifdef USE_MULTITHREAD_OBJECT_UPDATE
						if (tileList.size() > 0) {
							for(auto tile : tileList) {
#else
						if (tileList != nullptr) {
							cocos2d::Ref *ref;
							CCARRAY_FOREACH(tileList, ref) {
								auto tile = dynamic_cast<agtk::Tile *>(ref);
#endif
								if (checkConnectTileUp(tile) && moveVec.y < 0.0f) {
									bTileConnect = true; break;
								}
								if (checkConnectTileLeft(tile) && moveVec.x > 0.0f) {
									bTileConnect = true; break;
								}
								if (checkConnectTileRight(tile) && moveVec.x < 0.0f) {
									bTileConnect = true; break;
								}
							}
						}

						//坂の端にあるかチェック（左右）
						if (moveVec.x < 0 && (crntRect.getMinY() < _worldEndPoint.y && _worldEndPoint.y < crntRect.getMaxY()) && (this->getConnectEndSlope() == nullptr && bTileConnect == false)) {
							sp = Point(crntRect.getMinX(), crntRect.getMinY());
							ep = Point(crntRect.getMinX(), crntRect.getMaxY());
							checkHit = kCheckHitEndPoint;
						}
						else if (moveVec.x > 0 && (crntRect.getMinY() < _worldStartPoint.y && _worldStartPoint.y < crntRect.getMaxY()) && (this->getConnectStartSlope() == nullptr && bTileConnect == false)) {
							sp = Point(crntRect.getMaxX(), crntRect.getMinY());
							ep = Point(crntRect.getMaxX(), crntRect.getMaxY());
							checkHit = kCheckHitStartPoint;
						}
						if (checkHit != kCheckHitNone && _worldStartPoint.y != _worldEndPoint.y) {
							if (checkHitLine(&line, cross)) {
								moveVec.x = -moveVec.x;
								hitDown = true;
								goto lSkip;
							}
						}

						//坂の端にあるかチェック（上下）
						checkHit = kCheckHitNone;
						if (moveVec.y < 0 && (crntRect.getMinX() < _worldEndPoint.x && _worldEndPoint.x < crntRect.getMaxX()) && (this->getConnectEndSlope() == nullptr && bTileConnect == false)) {
							sp = Point(crntRect.getMinX(), crntRect.getMinY());
							ep = Point(crntRect.getMaxX(), crntRect.getMinY());
							checkHit = kCheckHitEndPoint;
						}
						else if (moveVec.y > 0 && (crntRect.getMinX() < _worldStartPoint.x && _worldStartPoint.x < crntRect.getMaxX()) && (this->getConnectStartSlope() == nullptr && bTileConnect == false)) {
							sp = Point(crntRect.getMinX(), crntRect.getMaxY());
							ep = Point(crntRect.getMaxX(), crntRect.getMaxY());
							checkHit = kCheckHitStartPoint;
						}
						if (checkHit != kCheckHitNone && _worldStartPoint.y != _worldEndPoint.y) {
							if (checkHitLine(&line, cross)) {
								moveVec.y = -moveVec.y;
								hitDown = true;
								goto lSkip;
							}
						}

						if (_worldEndPoint.x < prevRect.getMaxX()) {
							//１ドット近づける。
							if (prevRect.origin.y != _worldEndPoint.y && abs(prevRect.origin.y - _worldEndPoint.y) >= 1.0f) {
								prevRect.origin.y += (prevRect.origin.y < _worldEndPoint.y) ? 1.0f : -1.0f;
							}
							else {
								prevRect.origin.y = _worldEndPoint.y;
							}
						}
						else {
							//１ドット近づける。
							if (prevRect.origin.y != _worldStartPoint.y && abs(prevRect.origin.y - _worldStartPoint.y) >= 1.0f) {
								prevRect.origin.y += (prevRect.origin.y < _worldStartPoint.y) ? 1.0f : -1.0f;
							}
							else {
								prevRect.origin.y = _worldStartPoint.y;
							}
						}
					}
					else {
						//※坂が垂直の場合。
						//ぶつかる場合。
						float approachMoveX = (abs(prevRect.getMinX() - _worldStartPoint.x) < abs(prevRect.getMaxX() - _worldStartPoint.x)) ? -1 : 1;
						if ((approachMoveX < 0 && moveVec.x < 0) || (approachMoveX > 0 && moveVec.x > 0)) {
							moveVec.x = prevRect.origin.x - crntRect.origin.x;
						} else {
							hitUp = true;
						}
						float minY = _worldStartPoint.y > _worldEndPoint.y ? _worldEndPoint.y : _worldStartPoint.y;
						float maxY = _worldStartPoint.y > _worldEndPoint.y ? _worldStartPoint.y : _worldEndPoint.y;
						if (minY > prevRect.getMaxY() || maxY < prevRect.getMinY()) {
							moveVec.y = prevRect.origin.y - crntRect.origin.y;
							hitUp = _worldEndPoint.y > prevRect.getMaxY();
							hitDown = _worldStartPoint.y < prevRect.getMinY();
						}
						goto lSkip;
					}
				}

				if (procUpSideUpSlope(cross, moveVec, crntRect, prevRect, isSlip, touchFrame300)) {

					if (moveVec.y > 0 && (cross.y - crntRect.origin.y) < moveVec.y) {
						return false;
					}

					// 坂が水平の場合。
					if (_worldStartPoint.y == _worldEndPoint.y){
						if (moveVec.x == 0.0f) {
							moveVec.x = 0.0f;
							moveVec.y = prevRect.origin.y - crntRect.origin.y;
						}
						else {
							moveVec = cross - crntRect.origin;
							moveVec.x -= crntRect.size.width;
						}
					}
					else {
						auto gravity = cocos2d::Vec2::ZERO;
						if (object != nullptr) {
							gravity = object->getObjectMovement()->getGravity();
						}
						auto moveVecTmp = moveVec;
						if (gravity != cocos2d::Vec2::ZERO && moveVec.x == 0.0f) {
							moveVec = cross - crntRect.origin;
							moveVec.x = 0.0f;
						}
						else {
							moveVec = cross - crntRect.origin;
							moveVec.x -= crntRect.size.width;
						}

						//坂の始端の場合。
						if (_worldStartPoint.x >= cross.x && _worldStartPoint.y >= cross.y) {
							//タイルにぶつかっているかチェック。
							auto bCollied = checkColliedTile(tileList, cross);
							if (bCollied) {
								return false;
							}
						}
						//坂の終端の場合。
						if (_worldEndPoint.x <= cross.x && _worldEndPoint.y <= cross.y) {
							if (moveVecTmp.x < 0 && moveVec == cocos2d::Vec2::ZERO) {
								return false;
							}
							//移動がZEROで、Y軸方向に坂端に調整が入った場合。
							if (moveVecTmp == cocos2d::Vec2::ZERO && abs(cross.y - (crntRect.origin.y + moveVec.y)) <= FLT_EPSILON) {
								hitDown = true;
								return true;
							}
							//坂内から外へ移動する場合に移動方向が逆になった場合。
							if (moveVecTmp.x >= 0 && moveVec.x < 0) {
								moveVec.x = moveVecTmp.x;
							}
							if (moveVecTmp.y >= 0 && cross.y < crntRect.origin.y) {
								moveVec.y = moveVecTmp.y;
							}
						}
					}

					hitDown = true;
				}
				else {
					return false;
				}
			}
			// 坂の下にいる場合
			else
			{
				// 頭が坂の終点より下にある場合
				if (prevRect.getMaxY() <= _worldEndPoint.y && prevRect.getMinX() >= _worldStartPoint.x) {
					sp = Point(prevRect.getMinX(), _worldStartPoint.y);
					ep = Point(prevRect.getMinX(), _worldEndPoint.y);

					if (checkHitLine(&line, cross)) {
						prevRect.origin = cross;
						prevRect.origin.y -= prevRect.size.height;
					}
					else {
						//坂が水平の場合。
						if (_worldStartPoint.y == _worldEndPoint.y) {
							sp = Point(_worldStartPoint.x, crntRect.getMaxY());
							ep = Point(_worldEndPoint.x, crntRect.getMaxY());
							if (!checkHitLine(&line, cross)) {
								moveVec.y = 0.0f;
								hitUp = true;
								goto lSkip;
							}
						}
						if (_worldStartPoint.x != _worldEndPoint.x) {//坂が垂直以外
							if (_worldStartPoint.x < prevRect.getMinX()) {
								prevRect.origin.y = _worldEndPoint.y;
							}
							else {
								prevRect.origin.y = _worldStartPoint.y;
							}
							prevRect.origin.y -= prevRect.size.height;
						}
						else {
							//※坂が垂直の場合。
							float approachMoveX = (abs(prevRect.getMinX() - _worldStartPoint.x) < abs(prevRect.getMaxX() - _worldStartPoint.x)) ? -1 : 1;
							if ((approachMoveX < 0 && moveVec.x < 0) || (approachMoveX > 0 && moveVec.x > 0)) {
								moveVec.x = prevRect.origin.x - crntRect.origin.x;
							} else {
								hitUp = true;
							}
							float minY = _worldStartPoint.y > _worldEndPoint.y ? _worldEndPoint.y : _worldStartPoint.y;
							float maxY = _worldStartPoint.y > _worldEndPoint.y ? _worldStartPoint.y : _worldEndPoint.y;
							if (minY > prevRect.getMaxY() || maxY < prevRect.getMinY()) {
								moveVec.y = prevRect.origin.y - crntRect.origin.y;
								hitUp = _worldEndPoint.y > prevRect.getMaxY();
								hitDown = _worldStartPoint.y < prevRect.getMinY();
							}
							goto lSkip;
						}
					}
				}
				else {
					//坂下の端で、足元が坂の下の場合。
					if (prevRect.getMaxX() == _worldStartPoint.x && prevRect.getMinY() < _worldStartPoint.y) {
						return false;
					}

#if 1//ACT2-4078 坂下から上に移動する時、突き抜けて進むバグ修正。
					//頭が坂下に当たる場合。
					if (moveVec.y >= 0) {
						sp = Point(_worldStartPoint.x, crntRect.getMaxY());
						ep = Point(_worldEndPoint.x, crntRect.getMaxY());
						if (!checkHitLine(&line, cross)) {
							//移動オブジェクトの側面が坂の端に当たった場合。
							if ((moveVec.x < 0 && (crntRect.getMinY() < _worldEndPoint.y && _worldEndPoint.y < crntRect.getMaxY()) && this->getConnectEndSlope() == nullptr)
								|| (moveVec.x > 0 && (crntRect.getMinY() < _worldStartPoint.y && _worldStartPoint.y < crntRect.getMaxY()) && this->getConnectStartSlope() == nullptr)) {
								moveVec.x = -moveVec.x;
								goto lSkip;
							}
							//坂が水平の場合。
							if (_worldStartPoint.y == _worldEndPoint.y) {
								moveVec.y = 0.0f;
								hitUp = true;
								goto lSkip;
							}
							return false;
						}

						bool bTileConnect = false;
#ifdef USE_MULTITHREAD_OBJECT_UPDATE
						if (tileList.size() > 0) {
							for(auto tile : tileList) {						
#else
						if (tileList != nullptr) {
							cocos2d::Ref *ref;
							CCARRAY_FOREACH(tileList, ref) {
								auto tile = dynamic_cast<agtk::Tile *>(ref);
#endif
								if (checkConnectTileDown(tile) && moveVec.y > 0.0f) {
									bTileConnect = true; break;
								}
							}
						}
						if (bTileConnect == true && (crntRect.getMinX() < _worldStartPoint.x && _worldStartPoint.x < crntRect.getMaxX())) {
							sp = Point(crntRect.getMinX(), _worldStartPoint.y);
							ep = Point(crntRect.getMaxX(), _worldStartPoint.y);
							if (checkHitLine(&line, cross)) {
								moveVec.y = _worldStartPoint.y - prevRect.getMaxY();
								hitUp = true;
								goto lSkip;
							}
						}
					}
#else
					sp = Point(prevRect.getMinX(), _worldStartPoint.y);
					ep = Point(prevRect.getMinX(), _worldEndPoint.y);
					if (checkHitLine(&line, cross)) 
					{
						prevRect.origin.x = _worldEndPoint.x;
					}
					else {
						return false;
					}
#endif
				}
				
				if (procDownSideUpSlope(cross, moveVec, crntRect, prevRect, hitUp, hitDown)) {

					moveVec = cross - crntRect.origin;
					if (hitUp) {
						//moveVec.y -= (crntRect.size.height + 1.0f);
						moveVec.y -= crntRect.size.height;
					}

					//坂が垂直の場合
					if (_worldStartPoint.x == _worldEndPoint.x) {
						moveVec.x = 0.0f;
						if (prevRect.getMinY() > _worldEndPoint.y) {
							if (crntRect.getMinX() < _worldStartPoint.x && _worldStartPoint.x < crntRect.getMaxX()) {
								moveVec.y = prevRect.origin.y - crntRect.origin.y;
								hitDown = true;
							}
						}
						else {
							moveVec.y = 0.0f;
							moveVec.x = prevRect.origin.x - crntRect.origin.x;
							hitUp = false;
						}
						goto lSkip;
					}
				}
				else {
					return false;
				}
			}
		}
		//　下り坂の場合
		else {
			// 坂の上にいる場合
			if (isCrntUpSide || isCrntUpSideTmp) {

				// 前のフレームで坂に接触状態でジャンプ中の場合、
				// 坂への張り付き防止のために坂に当たらない状態とする
				if (isOldHit && jumping/* && moveVec.x >= 0*/) {
					return false;
				}

				//坂下の端で、足元が坂の下の場合。
				if (prevRect.getMinX() == _worldEndPoint.x && prevRect.getMinY() < _worldEndPoint.y) {
					return false;
				}
				//坂下の端で、足元が坂の下の場合。
				if (prevRect.getMaxX() == _worldStartPoint.x && prevRect.getMinY() < _worldStartPoint.y) {
					return false;
				}

				sp = Point(prevRect.getMinX(), _worldStartPoint.y);
				ep = Point(prevRect.getMinX(), _worldEndPoint.y);

				Vec2 cross;
				if (checkHitLine(&line, cross)) {
					prevRect.origin = cross;
				}
				else {
					if (_worldStartPoint.x != _worldEndPoint.x) {//坂が垂直以外

						//坂の端にあるかチェックする。
						enum EnumCheckHit {
							kCheckHitNone,
							kCheckHitStartPoint,
							kCheckHitEndPoint,
						} checkHit = kCheckHitNone;

						bool bTileConnect = false;
#ifdef USE_MULTITHREAD_OBJECT_UPDATE
						if (tileList.size() > 0) {
							for (auto tile : tileList) {
#else
						if (tileList != nullptr) {
							cocos2d::Ref *ref;
							CCARRAY_FOREACH(tileList, ref) {
								auto tile = dynamic_cast<agtk::Tile *>(ref);
#endif
								if (checkConnectTileUp(tile) && moveVec.y < 0.0f) {
									bTileConnect = true; break;
								}
								if (checkConnectTileRight(tile) && moveVec.x < 0.0f) {
									bTileConnect = true; break;
								}
								if (checkConnectTileLeft(tile) && moveVec.x > 0.0f) {
									bTileConnect = true; break;
								}
							}
						}

						//坂の端にあるかチェック（左右）
						if (moveVec.x <= 0 && (crntRect.getMinY() < _worldEndPoint.y && _worldEndPoint.y < crntRect.getMaxY()) && (this->getConnectEndSlope() == nullptr && bTileConnect == false)) {
							sp = Point(crntRect.getMinX(), crntRect.getMinY());
							ep = Point(crntRect.getMinX(), crntRect.getMaxY());
							checkHit = kCheckHitEndPoint;
						}
						else if (moveVec.x >= 0 && (crntRect.getMinY() < _worldStartPoint.y && _worldStartPoint.y < crntRect.getMaxY()) && (this->getConnectStartSlope() == nullptr && bTileConnect == false)) {
							sp = Point(crntRect.getMaxX(), crntRect.getMinY());
							ep = Point(crntRect.getMaxX(), crntRect.getMaxY());
							checkHit = kCheckHitStartPoint;
						}
						if (checkHit != kCheckHitNone && _worldStartPoint.y != _worldEndPoint.y) {
							if (checkHitLine(&line, cross)) {
								moveVec.x = -moveVec.x;
								hitDown = true;
								goto lSkip;
							}
						}

						//右下がり坂（UpSideDownSlope）
						//坂の端にあるかチェック（上から移動時）
						checkHit = kCheckHitNone;
						if (moveVec.y < 0 && (crntRect.getMinX() < _worldStartPoint.x && _worldStartPoint.x < crntRect.getMaxX())) {
							if ((this->getConnectEndSlope() == nullptr && bTileConnect == false) || (this->getConnectStartSlope() != nullptr && this->getConnectStartSlope()->_worldStartPoint.y < this->getConnectStartSlope()->_worldEndPoint.y)) {
								sp = Point(crntRect.getMinX(), crntRect.getMinY());
								ep = Point(crntRect.getMaxX(), crntRect.getMinY());
								checkHit = kCheckHitEndPoint;
							}
						}
						//坂の端にあるかチェック（下から移動時）
						else if (moveVec.y > 0 && (crntRect.getMinX() < _worldEndPoint.x && _worldEndPoint.x < crntRect.getMaxX())) {
							if ((this->getConnectStartSlope() == nullptr && bTileConnect == false)) {
								sp = Point(crntRect.getMinX(), crntRect.getMaxY());
								ep = Point(crntRect.getMaxX(), crntRect.getMaxY());
								checkHit = kCheckHitStartPoint;
							}
						}
						if (checkHit != kCheckHitNone && _worldStartPoint.y != _worldEndPoint.y) {
							if (checkHitLine(&line, cross)) {
								if (checkHit == kCheckHitEndPoint) {
									moveVec.y += (_worldStartPoint.y - crntRect.getMinY());
								}
								else if (checkHit == kCheckHitStartPoint) {
									moveVec.y += (_worldEndPoint.y - crntRect.getMaxY());
								}
								hitDown = true;
								goto lSkip;
							}
						}

						if (_worldStartPoint.x < prevRect.getMinX()) {
							//１ドット近づける。
							if (prevRect.origin.y != _worldEndPoint.y && abs(prevRect.origin.y - _worldEndPoint.y) >= 1.0f) {
								prevRect.origin.y += (prevRect.origin.y < _worldEndPoint.y) ? 1.0f : -1.0f;
							}
							else {
								prevRect.origin.y = _worldEndPoint.y;
							}
						}
						else {
							//１ドット近づける。
							if (prevRect.origin.y != _worldStartPoint.y && abs(prevRect.origin.y - _worldStartPoint.y) >= 1.0f) {
								prevRect.origin.y += (prevRect.origin.y < _worldStartPoint.y) ? 1.0f : -1.0f;
							}
							else {
								prevRect.origin.y = _worldStartPoint.y;
							}
						}
					}
					else {
						//※坂が垂直の場合。
						moveVec.x = prevRect.origin.x - crntRect.origin.x;
						float minY = _worldStartPoint.y > _worldEndPoint.y ? _worldEndPoint.y : _worldStartPoint.y;
						float maxY = _worldStartPoint.y > _worldEndPoint.y ? _worldStartPoint.y : _worldEndPoint.y;
						if (minY > prevRect.getMaxY() || maxY < prevRect.getMinY()) {
							moveVec.y = prevRect.origin.y - crntRect.origin.y;
							hitUp = _worldEndPoint.y > prevRect.getMaxY();
							hitDown = _worldStartPoint.y < prevRect.getMinY();
						}
						goto lSkip;
					}
				}

				if (procUpSideDownSlope(cross, moveVec, crntRect, prevRect, isSlip, touchFrame300)) {

					if (moveVec.y > 0 && (cross.y - crntRect.origin.y) < moveVec.y) {
						return false;
					}

					auto gravity = cocos2d::Vec2::ZERO;
					if (object != nullptr) {
						gravity = object->getObjectMovement()->getGravity();
					}
					auto moveVecTmp = moveVec;
					if (gravity != cocos2d::Vec2::ZERO && moveVec.x == 0.0f) {
						moveVec = cross - crntRect.origin;
						moveVec.x = 0.0f;
					}
					else {
						moveVec = cross - crntRect.origin;
					}

					//坂の始端の場合。
					if (cross.x <= _worldStartPoint.x && cross.y >= _worldStartPoint.y) {
						if (moveVecTmp.x > 0 && moveVec == cocos2d::Vec2::ZERO) {
							return false;
						}
						//移動がZEROで、Y軸方向に坂端に調整が入った場合。
						if (moveVecTmp == cocos2d::Vec2::ZERO && fabs(cross.y - (crntRect.origin.y + moveVec.y)) <= FLT_EPSILON) {
							hitDown = true;
							return true;
						}
						//坂内から外へ移動する場合に移動方向が逆になった場合。
						if (moveVecTmp.x <= 0 && moveVec.x > 0) {
							moveVec.x = moveVecTmp.x;
						}
						if (moveVecTmp.y >= 0 && cross.y < crntRect.origin.y) {
							moveVec.y = moveVecTmp.y;
						}
					}
					//坂の終端の場合。
					if (_worldEndPoint.x <= cross.x && _worldEndPoint.y >= cross.y) {
						//タイルにぶつかっているかチェック。
						auto bCollied = checkColliedTile(tileList, cross);
						if (bCollied) {
							return false;
						}
					}

					hitDown = true;
				}
				else {
					return false;
				}
			}
			// 坂の下にいる場合
			else {

				// 頭が坂の始点より下にある場合
				if (prevRect.getMaxY() <= _worldStartPoint.y && prevRect.getMaxX() <= _worldEndPoint.x) {
					sp = Point(prevRect.getMaxX(), _worldStartPoint.y);
					ep = Point(prevRect.getMaxX(), _worldEndPoint.y);

					Vec2 cross;
					if (checkHitLine(&line, cross)) {
						prevRect.origin.x = cross.x - prevRect.size.width;
						prevRect.origin.y = cross.y - prevRect.size.height;
					}
					else {
						if (_worldStartPoint.x != _worldEndPoint.x) {//坂が垂直以外
							if (prevRect.getMaxX() < _worldStartPoint.x) {
								prevRect.origin.y = _worldStartPoint.y;
							}
							else {
								prevRect.origin.y = _worldEndPoint.y;
							}
							prevRect.origin.y -= prevRect.size.height;
						}
					}
				}
				else {
					//坂下の端で、足元が坂の下の場合。
					if (prevRect.getMinX() == _worldEndPoint.x && prevRect.getMinY() < _worldEndPoint.y) {
						return false;
					}
#if 1//ACT2-4078 坂下から上に移動する時、突き抜けて進むバグ修正。
					//頭が坂下に当たる場合。
					if (moveVec.y >= 0) {
						sp = Point(_worldStartPoint.x, crntRect.getMaxY());
						ep = Point(_worldEndPoint.x, crntRect.getMaxY());
						if (!checkHitLine(&line, cross)) {
							if ((moveVec.x < 0 && (crntRect.getMinY() < _worldEndPoint.y && _worldEndPoint.y < crntRect.getMaxY()) && this->getConnectEndSlope() == nullptr)
								|| (moveVec.x > 0 && (crntRect.getMinY() < _worldStartPoint.y && _worldStartPoint.y < crntRect.getMaxY()) && this->getConnectStartSlope() == nullptr)) {
								moveVec.x = -moveVec.x;
								goto lSkip;
							}
							return false;
						}

						bool bTileConnect = false;
#ifdef USE_MULTITHREAD_OBJECT_UPDATE
						if (tileList.size() > 0) {
							for (auto tile : tileList) {
#else
						if (tileList != nullptr) {
							cocos2d::Ref *ref;
							CCARRAY_FOREACH(tileList, ref) {
								auto tile = dynamic_cast<agtk::Tile *>(ref);
#endif
								if (checkConnectTileDown(tile) && moveVec.y > 0.0f) {
									bTileConnect = true; break;
								}
							}
						}
						if (bTileConnect == true && (crntRect.getMinX() < _worldEndPoint.x && _worldEndPoint.x < crntRect.getMaxX())) {
							sp = Point(crntRect.getMinX(), _worldEndPoint.y);
							ep = Point(crntRect.getMaxX(), _worldEndPoint.y);
							if (checkHitLine(&line, cross)) {
								moveVec.y = _worldEndPoint.y - prevRect.getMaxY();
								hitUp = true;
								goto lSkip;
							}
						}
					}
#else
					sp = Point(prevRect.getMaxX(), _worldStartPoint.y);
					ep = Point(prevRect.getMaxX(), _worldEndPoint.y);

					Vec2 cross;
					if (checkHitLine(&line, cross)) {
						prevRect.origin.x = _worldStartPoint.x - prevRect.size.width;
					}
					else {
						return false;
					}
#endif
				}

				if (procDownSideDownSlope(cross, moveVec, crntRect, prevRect, hitUp, hitDown)) {
					moveVec = cross - crntRect.origin;
					if (hitUp) {
						moveVec.x -= crntRect.size.width;
						//moveVec.y -= (crntRect.size.height + 1.0f);
						moveVec.y -= crntRect.size.height;
					}

					//坂が垂直の場合
					if (_worldStartPoint.x == _worldEndPoint.x) {
						moveVec.x = 0.0f;
						if (prevRect.getMinY() > _worldStartPoint.y) {
							if (crntRect.getMinX() < _worldStartPoint.x && _worldStartPoint.x < crntRect.getMaxX()) {
								moveVec.y = prevRect.origin.y - crntRect.origin.y;
								hitDown = true;
							}
						}
						else {
							moveVec.y = 0.0f;
							moveVec.x = prevRect.origin.x - crntRect.origin.x;
							hitUp = false;
						}
					}
				}
				else {
					return false;
				}
			}
		}
lSkip:;
		// 足元が接触している場合少し浮かす
		if (hitDown) {
			moveVec.y += 0.1f;
		}
		// 頭が接触している場合少し沈める
		else if (hitUp) {
			moveVec.y -= 0.1f;
		}

		if (!this->getSlopeData()->getMoveObjectAlongSlope()) {
			adjustMoveVecForNotMovingObjectAlongSlope(Vec2(oldRect.getMidX(), oldRect.getMidY()), iniMoveVec, moveVec);
		}
		return true;
	}

	// 坂との接触はない
	return false;
}

bool Slope::checkHit(Vec2 currentPos, Rect currentRect, Vec2 prevPos, Rect prevRect, Vec2& cross, bool& isUpSide, bool jumping)
{
	return checkHitRect(currentRect);

#if 0
	bool isTouchPassable = false;
	return checkHit(currentPos, currentRect, prevPos, prevRect, cross, 0, isUpSide, false, isTouchPassable, jumping, false);
#endif
}

cocos2d::Vec2 Slope::calcMovePosition(float moveX, Vec2& startPos, cocos2d::Size& objectSize)
{
	// 事前にupdateWorldSpaceRect関数を呼び出し
	// ワールド空間での坂の情報を更新しておく必要がある

	Vec2 result = startPos;

	// 右へ移動したか？
	bool isMoveRight = moveX > 0;
	// 左へ移動したか？
	bool isMoveLeft = moveX < 0;

	// 坂の角度
	float rad = CC_DEGREES_TO_RADIANS(_degree);

	bool isCheck = true;

	// すでに坂を上りきっている場合
	if (moveX > 0 && _worldEndPoint.x <= startPos.x) {
		isCheck = false;
	}
	// すでに坂を下りきっている場合
	else if (moveX < 0 && startPos.x <= _worldStartPoint.x) {
		isCheck = false;
	}

	if (isCheck) {
		// 移動量を算出し移動結果を設定
		result = startPos + Vec2(cosf(rad), sinf(rad)) * moveX;
	}
	else {
		// 横移動のみ行う
		result.x += moveX;
	}

	// 次に接触する坂を取得
	auto nextSlope = this->getConnectEndSlope();

	// 上り坂の場合
	if (getType() == kTypeUp) {
		// 坂を上り切った場合
		if (isMoveRight && (result.y > _worldEndPoint.y)) {

			// 残り移動量を算出
			float length = (result - _worldEndPoint).getLength();

			// 坂の終点と接続している坂がある場合
			if (nextSlope != nullptr) {

				// 次の坂を移動できる場合
				if (getCanMoveEndSlope()) {
					// 次の坂が同じ種類の坂の場合
					if (nextSlope->getType() == getType()) {

						// 上から通過できる坂の場合
						if (nextSlope->getSlopeData()->getPassableFromUpper()) {
							// 余分に上った分だけ右に移動させる
							result = _worldEndPoint;
							result.x += length;
						}
						// 上から通過できない坂の場合
						else {
							result = nextSlope->calcMovePosition(length, _worldEndPoint, objectSize);
						}
					}
					else {

						// 余分に上った分だけ右に移動させる
						result = _worldEndPoint;
						result.x += length;
					}
				}
				// 次の坂が移動できない場合
				else {
					// 上り坂の終点同士が接続している場合
					if (nextSlope->getType() == kTypeUp) {
						// 余分に上った分だけ右に移動させる
						result = _worldEndPoint;
						result.x += length;
					}
					// 上り坂と下り坂の終点同士が接続している場合
					else {
					}
				}
			}
			else {
				// 余分に上った分だけ右に移動させる
				result = _worldEndPoint;
				result.x += length;
			}
		}
		// 坂を下りきった場合
		else if (isMoveLeft && (result.y < _worldStartPoint.y)) {
			// 残り移動量を算出
			float length = (result - _worldStartPoint).getLength();
			// 次に接触する坂を取得
			auto nextSlope = this->getConnectStartSlope();

			// 坂の始点と接続している坂がある場合
			if (nextSlope != nullptr) {
				// 次の坂が同じ種類の坂の場合
				if (nextSlope->getType() == getType()) {
					result = nextSlope->calcMovePosition(-length, _worldStartPoint, objectSize);
				}
				else {
				}
			}
			else {
				// 余分に下った分だけ左に移動させる
				result = _worldStartPoint;
				result.x -= length;
			}
		}
	}
	// 下り坂の場合
	else {
		// 坂を上り切った場合
		if (isMoveLeft && (result.y > _worldStartPoint.y)) {
			// 残り移動量を算出
			float length = (result - _worldStartPoint).getLength();
			// 次に接触する坂を取得
			auto nextSlope = this->getConnectStartSlope();

			// 坂の始点と接続している坂がある場合
			if (nextSlope != nullptr) {

				if (getCanMoveStartSlope()) {
					// 次の坂が同じ種類の坂の場合
					if (nextSlope->getType() == getType()) {
						// 上から通過できる坂の場合
						if (nextSlope->getSlopeData()->getPassableFromUpper()) {
							// 余分に上った分だけ左に移動させる
							result = _worldStartPoint;
							result.x -= length;
						}
						else {
							result = nextSlope->calcMovePosition(-length, _worldStartPoint, objectSize);
						}
					}
					else {

						// 余分に上った分だけ左に移動させる
						result = _worldStartPoint;
						result.x -= length;
					}
				}
				else {
					// 下り坂の始点同士が接続している場合
					if (nextSlope->getType() == kTypeDown) {
						// 余分に上った分だけ左に移動させる
						result = _worldStartPoint;
						result.x -= length;
					}
					// 上り坂と下り坂の始点同士が接続している場合
					else {
					}
				}
			}
			else {
				// 余分に上った分だけ左に移動させる
				result = _worldStartPoint;
				result.x -= length;
			}
		}
		// 坂を下りきった場合
		else if (isMoveRight && (result.y < _worldEndPoint.y)){
			// 残り移動量を算出
			float length = (result - _worldEndPoint).getLength();
			// 次に接触する坂を取得
			auto nextSlope = this->getConnectEndSlope();

			// 坂の終点と接続している坂がある場合
			if (nextSlope != nullptr) {
				// 次の坂が同じ種類の坂の場合
				if (nextSlope->getType() == getType()) {
					result = nextSlope->calcMovePosition(length, _worldEndPoint, objectSize);
				}
				else {
				}
			}
			else {
				// 余分に下った分だけ右に移動させる
				result = _worldEndPoint;
				result.x += length;
			}
		}
	}

	return result;
}

cocos2d::Vec2 Slope::calcMovePosition(float moveX, float moveY, Vec2& startPos, cocos2d::Size& objectSize)
{
	// 事前にupdateWorldSpaceRect関数を呼び出し
	// ワールド空間での坂の情報を更新しておく必要がある

	const float margin = 0.0001f;
	Vec2 result = startPos;

	// 右へ移動したか？
	bool isMoveRight = moveX > 0;
	// 左へ移動したか？
	bool isMoveLeft = moveX < 0;
	// 上へ移動したか？
	bool isMoveUp = moveY > 0;
	// 下へ移動したか？
	bool isMoveDown = moveY < 0;

	// 坂の角度
	float rad = CC_DEGREES_TO_RADIANS(_degree);

	cocos2d::Vec2 resultX;
	cocos2d::Vec2 resultY;

	//左右移動
	if (moveX != 0.0f) {
		bool isCheck = true;

		// すでに坂を上りきっている場合
		if (moveX > 0 && _worldEndPoint.x <= startPos.x + margin) {
			isCheck = false;
		}
		// すでに坂を下りきっている場合
		else if (moveX < 0 && startPos.x - margin <= _worldStartPoint.x) {
			isCheck = false;
		}

		if (isCheck) {
			// 移動量を算出し移動結果を設定
			resultX = Vec2(cosf(rad), sinf(rad)) * moveX;
		}
		else {
			// 横移動のみ行う
			resultX.x = moveX;
		}
	}

	//上下移動
	if (moveY != 0.0f) {
		bool isCheck = true;

		auto worldStartPoint = (this->getType() == kTypeUp ? _worldStartPoint : _worldEndPoint);
		auto worldEndPoint = (this->getType() == kTypeUp ? _worldEndPoint : _worldStartPoint);
		auto my = (this->getType() == kTypeUp) ? moveY : -moveY;

		if (moveY > 0 && worldEndPoint.y <= startPos.y + margin) {
			isCheck = false;
		}
		else if (moveY < 0 && startPos.y - margin <= worldStartPoint.y) {
			isCheck = false;
		}
		if (isCheck) {
			resultY = Vec2(cosf(rad), sinf(rad)) * my;
		}
		else {
			resultY.y = moveY;
		}
	}

	cocos2d::Vec2 move;
	if (resultY != cocos2d::Vec2::ZERO && resultX != cocos2d::Vec2::ZERO) {
		move.x = (resultX.x + resultY.x) * 0.5f;
		move.y = (resultX.y + resultY.y) * 0.5f;
	}
	else if (resultX != cocos2d::Vec2::ZERO) {
		move = resultX;
	}
	else if (resultY != cocos2d::Vec2::ZERO) {
		move = resultY;
	}
	result += move;

	// 次に接触する坂を取得
	auto nextSlope = this->getConnectEndSlope();

	std::function<cocos2d::Vec2(cocos2d::Vec2, cocos2d::Vec2)> adjustMove = [&](cocos2d::Vec2 v, cocos2d::Vec2 left) {
		if ((isMoveRight && isMoveUp) || (isMoveLeft && isMoveUp)) {
			v += left;
		}
		//else if (isMoveRight || isMoveLeft) {
		//	v.x += left.x;
		//}
		else if (isMoveUp || isMoveDown) {
			v.y += left.y;
		}
		return v;
	};

	// 上り坂の場合
	if (getType() == kTypeUp) {
		// 坂を上り切った場合
		if ((isMoveRight || isMoveUp) && (result.y > _worldEndPoint.y)) {
			// 残り移動量を算出
			auto leftVec = result - _worldEndPoint;
			// 坂の終点と接続している坂がある場合
			if (nextSlope != nullptr) {

				// 次の坂を移動できる場合
				if (getCanMoveEndSlope()) {
					// 次の坂が同じ種類の坂の場合
					if (nextSlope->getType() == getType()) {

						// 上から通過できる坂の場合
						if (nextSlope->getSlopeData()->getPassableFromUpper()) {
							// 余分に上った分だけ右に移動させる
							result = adjustMove(_worldEndPoint, leftVec);
						}
						// 上から通過できない坂の場合
						else {
							result = nextSlope->calcMovePosition(move.x, move.y, _worldEndPoint, objectSize);
						}
					}
					else {

						// 余分に上った分だけ右に移動させる
						result = adjustMove(_worldEndPoint, leftVec);
					}
				}
				// 次の坂が移動できない場合
				else {
					// 上り坂の終点同士が接続している場合
					if (nextSlope->getType() == kTypeUp) {
						// 余分に上った分だけ右に移動させる
						result = adjustMove(_worldEndPoint, leftVec);
					}
					// 上り坂と下り坂の終点同士が接続している場合
					else {
					}
				}
			}
			else {
				// 余分に上った分だけ右に移動させる
				result = adjustMove(_worldEndPoint, leftVec);
			}
		}
		// 坂を下りきった場合
		else if ((isMoveLeft || isMoveDown) && (result.y < _worldStartPoint.y)) {
			// 残り移動量を算出
			float length = (result - _worldStartPoint).getLength();
			//auto leftVec = result - _worldStartPoint;
			// 次に接触する坂を取得
			auto nextSlope = this->getConnectStartSlope();

			// 坂の始点と接続している坂がある場合
			if (nextSlope != nullptr) {
				// 次の坂が同じ種類の坂の場合
				if (nextSlope->getType() == getType()) {
					result = nextSlope->calcMovePosition(move.x, move.y, _worldStartPoint, objectSize);
				}
				else {
				}
			}
			else {
				// 坂が水平の場合
				if (_worldStartPoint.y == _worldEndPoint.y) {
					result.y = _worldEndPoint.y;
				}
				else {
					// 余分に下った分だけ左に移動させる
					result = _worldStartPoint;
					result.x -= length;
					//result = _worldStartPoint + leftVec;
				}
			}
		}
	}
	// 下り坂の場合
	else {
		// 坂を上り切った場合
		if ((isMoveLeft || isMoveUp) && (result.y > _worldStartPoint.y)) {
			// 残り移動量を算出
			auto leftVec = result - _worldStartPoint;
			// 次に接触する坂を取得
			auto nextSlope = this->getConnectStartSlope();

			// 坂の始点と接続している坂がある場合
			if (nextSlope != nullptr) {

				if (getCanMoveStartSlope()) {
					// 次の坂が同じ種類の坂の場合
					if (nextSlope->getType() == getType()) {
						// 上から通過できる坂の場合
						if (nextSlope->getSlopeData()->getPassableFromUpper()) {
							// 余分に上った分だけ左に移動させる
							result = adjustMove(_worldStartPoint, leftVec);
						}
						else {
							result = nextSlope->calcMovePosition(move.x, move.y, _worldStartPoint, objectSize);
						}
					}
					else {
						// 余分に上った分だけ左に移動させる
						result = adjustMove(_worldStartPoint, leftVec);
					}
				}
				else {
					//次の坂が垂直の場合。
					if (nextSlope->start.x == nextSlope->end.x) {
						result = adjustMove(_worldStartPoint, leftVec);
					} else
					// 下り坂の始点同士が接続している場合
					if (nextSlope->getType() == kTypeDown) {
						// 余分に上った分だけ左に移動させる
						result = adjustMove(_worldStartPoint, leftVec);
					}
					// 上り坂と下り坂の始点同士が接続している場合
					else {
					}
				}
			}
			else {
				// 余分に上った分だけ左に移動させる
				result = adjustMove(_worldStartPoint, leftVec);
			}
		}
		// 坂を下りきった場合
		else if ((isMoveRight || isMoveDown) && (result.y < _worldEndPoint.y)) {
			// 残り移動量を算出
			float length = (result - _worldEndPoint).getLength();
			//auto leftVec = result - _worldEndPoint;
			// 次に接触する坂を取得
			auto nextSlope = this->getConnectEndSlope();

			// 坂の終点と接続している坂がある場合
			if (nextSlope != nullptr) {
				// 次の坂が同じ種類の坂の場合
				if (nextSlope->getType() == getType()) {
					result = nextSlope->calcMovePosition(move.x, move.y, _worldEndPoint, objectSize);
				}
				else {
				}
			}
			else {
				// 余分に下った分だけ右に移動させる
				result = _worldEndPoint;
				result.x += length;
				//result = _worldEndPoint + leftVec;
			}
		}
	}

	return result;
}

bool Slope::checkUpSide(Vec2& targetPos)
{
	// 事前にupdateWorldSpaceRect関数を呼び出し
	// ワールド空間での坂の情報を更新しておく必要がある

	Vec2 slopeVec = _worldEndPoint - _worldStartPoint;
	slopeVec.normalize();

	Vec2 targetVec = targetPos - _worldStartPoint;
	targetVec.normalize();

	// 「坂との線分」と「坂の始点から対象位置までの線分」で外積
	float cross = slopeVec.cross(targetVec);

#if 0
	float deg = CC_RADIANS_TO_DEGREES(asinf(cross));
#endif

	return cross >= 0;
}

bool Slope::procUpSideUpSlope(Vec2& cross, Vec2& moveVec, Rect& crntRect, Rect& prevRect, bool isSlip, float touchFrame300)
{
	bool falling = (_objectTmp != nullptr) ? (_objectTmp->_falling || _objectTmp->_fallingOld) : false;
	bool floor = (_objectTmp != nullptr) ? (_objectTmp->_floor || _objectTmp->_floorOld) : false;
	cocos2d::Point sp;
	cocos2d::Point ep;
	CollisionLine line = { &sp, &ep };
	bool bSlopeHorizon = (_worldStartPoint.y == _worldEndPoint.y);

	if (isSlip) {
		// 坂滑り用の係数を取得
		float slipVal = getSlipValue();

		if (slipVal != 0 && (prevRect.getMaxX() <= _worldEndPoint.x)) {

			float per = clampf(touchFrame300 / SLIP_MAX_FRAME, 0.0f, 1.0f);
			if (per > 0) {
				// 坂にとどまる場合
				if (moveVec.x == 0) {
					per = 1.0f;
				}
				slipVal *= per;

				// 坂を上る場合
				if (moveVec.x > 0) {
					moveVec.x = (moveVec.x - (moveVec.x * slipVal));
				}
				// 坂を下る場合
				else if (moveVec.x < 0) {
					moveVec.x = (moveVec.x + (moveVec.x * slipVal));
				}
				// 坂にとどまる場合
				else {
					moveVec.x = -slipVal;
				}
			}
		}
	}

	std::function<bool()> checkCross = [&]() {

		if (checkHitLine(&line, cross)) {

			// 移動が発生している場合
			if (moveVec.x != 0 || moveVec.y != 0) {

				// 過去の矩形右下の位置を基準に移動を行いたいので
				// 坂と接触する位置を確認する
				sp = Point(prevRect.getMaxX(), prevRect.getMinY() - TILE_COLLISION_THRESHOLD);
				ep = Point(prevRect.getMaxX(), prevRect.getMaxY());

				// 過去の矩形右下が坂に接触しない場合
				if (!checkHitLine(&line, cross)) {

					// 矩形右下の位置を基準に移動を行いたいので
					// 坂と接触する位置を確認する
					sp = Point(prevRect.getMaxX(), prevRect.getMinY());
					ep = Point(crntRect.getMaxX(), crntRect.getMinY());

					if (!checkHitLine(&line, cross)) {

						//坂が水平になっている場合。
						if (bSlopeHorizon) {
							sp = Point(prevRect.getMinX(), prevRect.getMinY());
							ep = Point(crntRect.getMinX(), crntRect.getMinY());

							if (checkHitLine(&line, cross)) {
								cross.x += crntRect.size.width;
							}
						}
						//床にいる状態。
						else if(floor == true) {

							//オブジェクトが床にいる場合、床を移動していると仮定して移動先のY座標を過去に合わせる。
							sp = Point(prevRect.getMaxX(), prevRect.getMinY());
							ep = Point(crntRect.getMinX(), prevRect.getMinY());
							if (!checkHitLine(&line, cross)) {
								return false;
							}
						}
						else {
							return false;
						}
					}
				}
			}

			if (moveVec.x > 0) {
				// 坂の頂点に到達している場合
				if (cross.y >= _worldEndPoint.y) {
					cross.x = prevRect.getMaxX();
				}
				// 坂の頂点に到達している場合
				if (cross.x <= _worldStartPoint.x) {
					cross.y = prevRect.getMinY();
				}
				// 過去の矩形の位置から坂を上る
				if (falling) {
					cross = calcMovePosition(moveVec.x, cross, crntRect.size);
				}
				else {
					cross = calcMovePosition(moveVec.x, moveVec.y, cross, crntRect.size);
				}
			}
			else if (moveVec.x < 0) {
				// 坂の頂上で坂に吸い寄せられるのを防ぐために
				// 現在の矩形の右端が坂の頂上の左にいる場合のみ位置の再設定を行う
				if (crntRect.getMaxX() <= _worldEndPoint.x) {
					// 移動量を考慮した位置に設定する
					if (falling) {
						cross = calcMovePosition(moveVec.x, cross, crntRect.size);
					}
					else {
						cross = calcMovePosition(moveVec.x, moveVec.y, cross, crntRect.size);
					}
				}
				else if (crntRect.getMaxX() + moveVec.x <= _worldEndPoint.x && bSlopeHorizon == false) {

					cross = _worldEndPoint;

					// 坂の終点からの移動を行うために移動量を減退させる
					moveVec.x -= _worldEndPoint.x - crntRect.getMaxX();
					cross = calcMovePosition(moveVec.x, moveVec.y, cross, crntRect.size);
				}
				else {
					if (cross == _worldEndPoint) {
						cross = crntRect.origin;
						cross.x += crntRect.size.width;
						cross.y = _worldEndPoint.y;
					}
					else {
						cross = crntRect.origin;
						cross.x += crntRect.size.width;
						if (bSlopeHorizon) {
							cross.y = _worldEndPoint.y;
						}
					}
				}
			}
			else if (moveVec.y > 0) {
				// 坂の頂点に到達している場合
				if (cross.x <= _worldStartPoint.x) {
					cross.y = prevRect.getMinY();
				}
				// 過去の矩形の位置から坂を上る
				cross = calcMovePosition(moveVec.x, moveVec.y, cross, crntRect.size);
			}
			else if (moveVec.y < 0) {
				if (crntRect.getMinY() <= _worldEndPoint.y) {
					// 移動量を考慮した位置に設定する
					if (falling) {
						cross = calcMovePosition(moveVec.x, cross, crntRect.size);
						//接触しているところが始点・終点の場合かつオブジェクトの端の場合は接触していないと判断。
						if ((_worldStartPoint == cross && _worldStartPoint.x >= crntRect.getMaxX())
						||  (_worldEndPoint == cross && crntRect.getMinX() >= _worldEndPoint.x)) {
							return false;
						}
					}
					else {
						cross = calcMovePosition(moveVec.x, moveVec.y, cross, crntRect.size);
					}
				}
				else if (crntRect.getMinY() + moveVec.y <= _worldEndPoint.y) {

					cross = _worldEndPoint;

					// 坂の終点からの移動を行うために移動量を減退させる
					moveVec.y -= _worldEndPoint.y - crntRect.getMinY();
					if (falling) {
						cross = calcMovePosition(moveVec.x, cross, crntRect.size);
					}
					else {
						cross = calcMovePosition(moveVec.x, moveVec.y, cross, crntRect.size);
					}
				}
				else {
					cross = crntRect.origin;
					cross.x += crntRect.size.width;
				}
			}
			else {
				// 現在の矩形の右端に合わせて座標を設定
				sp = Point(crntRect.getMaxX(), _worldStartPoint.y);
				ep = Point(crntRect.getMaxX(), _worldEndPoint.y);

				if (checkHitLine(&line, cross)) {
					return true;
				}
				else {
					// 現在の矩形の左端に合わせて座標を設定
					sp = Point(crntRect.getMinX(), _worldStartPoint.y);
					ep = Point(crntRect.getMinX(), _worldEndPoint.y);

					if (checkHitLine(&line, cross)) {
						cross.x += crntRect.size.width;
						cross.y = _worldEndPoint.y;
						return true;
					}
					else {

						// オブジェクトの矩形より坂の幅の方が小さい場合
						if (getRect().size.width < crntRect.size.width) {
							// 矩形の幅より坂の幅の方が小さいために矩形の両端が坂に接触できない状態になっているので
							// 坂の頂上に配置する
							cross.x = crntRect.getMaxX();
							cross.y = _worldEndPoint.y;
							return true;
						}
						
						//坂が水平の場合。
						if (bSlopeHorizon) {
							//水平の場合、坂の上に配置する
							cross.x = crntRect.getMaxX();
							cross.y = _worldEndPoint.y;
							return true;
						}
					}
				}

				return false;
			}

			return true;
		}

		return false;
	};

	// 過去の矩形の足元が坂に接触するかを確認
	sp = Point(prevRect.getMinX() - 0.1f, prevRect.getMinY());
	ep = Point(prevRect.getMaxX() + 0.1f, prevRect.getMinY());

	if (checkCross()) {
		return true;
	}

//	CCLOG("過去の足元を抜けた");

	// 足元が坂に接触するかを確認
	if (moveVec.x >= 0) {
		sp = Point(prevRect.getMinX(), prevRect.getMinY());
		ep = Point(crntRect.getMaxX(), crntRect.getMinY());
	}
	else {
		sp = Point(prevRect.getMaxX(), prevRect.getMinY());
		ep = Point(crntRect.getMinX(), crntRect.getMinY());
	}

	if (checkCross()) {
		return true;
	}

//	CCLOG("足元を抜けた");

	// 現在の矩形と過去の矩形との対角線が坂に接触するかを確認
	if (moveVec.x >= 0) {
		sp = Point(prevRect.getMinX(), prevRect.getMaxY());
		ep = Point(crntRect.getMaxX(), crntRect.getMinY());
	}
	else {
		sp = Point(prevRect.getMaxX(), prevRect.getMinY());
		ep = Point(crntRect.getMinX(), crntRect.getMaxY());
	}

	if (checkCross()) {
		return true;
	}

//	CCLOG("対角線を抜けた");

	//坂が水平の場合。
	if (bSlopeHorizon) {
		//移動方向の先面
		if (moveVec.x >= 0) {
			sp = Point(prevRect.getMaxX(), prevRect.getMinY());
			ep = Point(prevRect.getMaxX(), prevRect.getMaxY());
		}
		else {
			sp = Point(prevRect.getMinX(), prevRect.getMinY());
			ep = Point(prevRect.getMinX(), prevRect.getMaxY());
		}
		if (checkCross()) {
			return true;
		}

		//移動先の後面
		if (moveVec.x >= 0) {
			sp = Point(prevRect.getMinX(), prevRect.getMinY());
			ep = Point(prevRect.getMinX(), prevRect.getMaxY());
		}
		else {
			sp = Point(prevRect.getMaxX(), prevRect.getMinY());
			ep = Point(prevRect.getMaxX(), prevRect.getMaxY());
		}
		if (checkCross()) {
			return true;
		}
	}

	return false;
}

bool Slope::procDownSideUpSlope(Vec2& cross, Vec2& moveVec, Rect& crntRect, Rect& prevRect, bool & hitUp, bool & hitDown)
{
	bool falling = (_objectTmp != nullptr) ? (_objectTmp->_falling || _objectTmp->_fallingOld) : false;
	cocos2d::Point sp;
	cocos2d::Point ep;
	CollisionLine line = { &sp, &ep };

	std::function<bool(bool)> checkCross = [&](bool checkBottom) {

		if (checkHitLine(&line, cross)) {

			// 左移動、停止中の場合
			if (moveVec.x <= 0) {

				// 足元の確認を行っている場合
				if (checkBottom) {
					hitDown = true;

					// 坂の頂上で坂に吸い寄せられるのを防ぐために
					// 現在の矩形の右端が坂の頂上の左にいる場合のみ位置の再設定を行う
					if (crntRect.getMaxX() <= _worldEndPoint.x) {
						// 移動量を考慮した位置に設定する
						cross = calcMovePosition(moveVec.x, moveVec.y, cross, crntRect.size);
					}
					else if (crntRect.getMaxX() + moveVec.x <= _worldEndPoint.x) {
						cross = _worldEndPoint;

						// 坂の終点からの移動を行うために移動量を減退させる
						moveVec.x -= _worldEndPoint.x - crntRect.getMaxX();
						cross = calcMovePosition(moveVec.x, moveVec.y, cross, crntRect.size);
					}
					else {
						cross = crntRect.origin;
						//cross.x += crntRect.size.width;
					}
				}
				// 坂の頂点に到達していない場合
				else {

					// 頭が接触している
					hitUp = true;

					if (crntRect.getMinX() < _worldStartPoint.x && _worldStartPoint.x < crntRect.getMaxX() && moveVec.x == 0 && moveVec.y > 0) {
						cross = crntRect.origin;
						cross.y = _worldStartPoint.y;
						return true;
					}

					// 矩形左上の位置を基準に移動を行いたいので
					// 坂と接触する位置を確認する
					if (falling) {
						sp = Point(crntRect.getMinX(), crntRect.getMaxY());
						ep = Point(crntRect.getMaxX(), crntRect.getMaxY());
						if (!checkHitLine(&line, cross)) {
							cross.x = prevRect.getMinX();
						}
						// 過去の矩形の位置から坂を上る
						cross = calcMovePosition(moveVec.x, cross, crntRect.size);
					}
					else {
						sp = Point(prevRect.getMinX(), prevRect.getMaxY()) - moveVec;
						ep = Point(crntRect.getMinX(), crntRect.getMaxY()) + moveVec;
						if (!checkHitLine(&line, cross)) {
							cross.x = prevRect.getMinX();
						}
						// 過去の矩形の位置から坂を上る
						cross = calcMovePosition(moveVec.x, moveVec.y, cross, crntRect.size);
					}
				}
			}
			else {

				if (checkBottom) {

					// 矩形右下の位置を基準に移動を行いたいので
					// 坂と接触する位置を確認する
					sp = Point(prevRect.getMaxX(), prevRect.getMinY()) - moveVec;
					ep = Point(crntRect.getMaxX(), crntRect.getMinY()) + moveVec;

					if (checkHitLine(&line, cross)) {
					}
					else {
						cross.x = prevRect.getMaxX();
					}

					// 坂の頂点に到達している場合
					if (cross.y >= _worldEndPoint.y) {
						cross.x = prevRect.getMaxX();
					}

					// 過去の矩形の位置から坂を上る
					cross = calcMovePosition(moveVec.x, moveVec.y, cross, crntRect.size);

					// 坂の下にいる場合は接触位置を左に設定したいので
					// 矩形の幅分戻すことで左に戻す
					cross.x -= crntRect.size.width;

					hitDown = true;
				}
				else {
					// 坂の頂上で坂に吸い寄せられるのを防ぐために
					// 現在の矩形の左端が坂の頂上の右にいる場合のみ位置の再設定を行う
					if (_worldStartPoint.x <= crntRect.getMinX()) {

						// オブジェクトに重力がある時
						if (_objectTmp && _objectTmp->getObjectMovement()->getGravity() != cocos2d::Vec2::ZERO) {
							sp = Point(crntRect.getMinX(), crntRect.getMinY()) + moveVec;
							ep = Point(crntRect.getMinX(), crntRect.getMaxY()) + moveVec;
							if (!checkHitLine(&line, cross)) {
								return false;
							}
						}

						// 移動量を考慮した位置に設定する
						cross = calcMovePosition(moveVec.x, moveVec.y, cross, crntRect.size);
					}
					else if (crntRect.getMinX() < _worldStartPoint.x && _worldStartPoint.x < crntRect.getMaxX()) {

						sp = Point(prevRect.getMaxX(), prevRect.getMaxY());
						ep = Point(crntRect.getMaxX(), crntRect.getMaxY());
						if (!checkHitLine(&line, cross)) {
							return false;
						}
						// 移動量を考慮した位置に設定する
						cross = calcMovePosition(moveVec.x, moveVec.y, cross, crntRect.size);
						cross.x -= crntRect.size.width;
					}
					else {
						cross = _worldStartPoint;

						// 坂の始点からの移動を行うために移動量を減退させる
						moveVec.x -= _worldStartPoint.x - crntRect.getMinX();
						cross = calcMovePosition(moveVec.x, moveVec.y, cross, crntRect.size);
					}
					hitUp = true;
				}
			}

			return true;
		}

		return false;
	};

	// 頭上が坂に接触するかを確認
	if (moveVec.x <= 0) {
		sp = Point(prevRect.getMaxX(), prevRect.getMaxY());
		ep = Point(crntRect.getMinX(), crntRect.getMaxY());
	}
	else {
		sp = Point(prevRect.getMinX(), prevRect.getMaxY());
		ep = Point(crntRect.getMaxX(), crntRect.getMaxY());
	}

	if (checkCross(false)) {
		return true;
	}

//	CCLOG("頭上を抜けた");

	// 足元が坂に接触するかを確認
	if (moveVec.x <= 0) {
		sp = Point(prevRect.getMaxX(), prevRect.getMinY());
		ep = Point(crntRect.getMinX(), crntRect.getMinY());
	}
	else {
		sp = Point(prevRect.getMinX(), prevRect.getMinY());
		ep = Point(crntRect.getMaxX(), crntRect.getMinY());
	}

	if (checkCross(true)) {
		return true;
	}

//	CCLOG("足元を抜けた");

	// 足元（1px下に補正）が坂に接触するかを確認
	if (moveVec.x <= 0) {
		sp = Point(prevRect.getMaxX(), prevRect.getMinY() - 1);
		ep = Point(crntRect.getMinX(), crntRect.getMinY() - 1);
	}
	else {
		sp = Point(prevRect.getMinX(), prevRect.getMinY() - 1);
		ep = Point(crntRect.getMaxX(), crntRect.getMinY() - 1);
	}

	if (checkCross(true)) {
		return true;
	}

//	CCLOG("足元（補正）を抜けた");

	// 現在の矩形と過去の矩形との対角線が坂に接触するかを確認
	if (moveVec.x <= 0) {
		sp = Point(prevRect.getMaxX(), prevRect.getMinY());
		ep = Point(crntRect.getMinX(), crntRect.getMaxY());
	}
	else {
		sp = Point(prevRect.getMinX(), prevRect.getMaxY());
		ep = Point(crntRect.getMaxX(), crntRect.getMinY());
	}
	if (checkCross(false)) {
		return true;
	}

//	CCLOG("対角線を抜けた");

	return false;
}

bool Slope::procUpSideDownSlope(Vec2& cross, Vec2& moveVec, Rect& crntRect, Rect& prevRect, bool isSlip, float touchFrame300)
{
	bool falling = (_objectTmp != nullptr) ? (_objectTmp->_falling || _objectTmp->_fallingOld) : false;
	bool floor = (_objectTmp != nullptr) ? (_objectTmp->_floor || _objectTmp->_floorOld) : false;
	cocos2d::Point sp;
	cocos2d::Point ep;
	CollisionLine line = { &sp, &ep };

	if (isSlip) {
		// 坂滑り用の係数を取得
		float slipVal = getSlipValue();

		if (slipVal != 0 && (_worldStartPoint.x <= prevRect.getMinX())) {

			float per = clampf(touchFrame300 / SLIP_MAX_FRAME, 0.0f, 1.0f);

			if (per > 0) {
				// 坂にとどまる場合
				if (moveVec.x == 0) {
					per = 1.0f;
				}
				slipVal *= per;

				// 坂を下る場合
				if (moveVec.x > 0) {
					moveVec.x = (moveVec.x + (moveVec.x * slipVal));
				}
				// 坂を上る場合
				else if (moveVec.x < 0) {
					moveVec.x = (moveVec.x - (moveVec.x * slipVal));
				}
				// 坂にとどまる場合
				else {
					moveVec.x = slipVal;
				}
			}
		}
	}

	std::function<bool()> checkCross = [&]() {

		if (checkHitLine(&line, cross)) {

			// 移動が発生している場合
			if (moveVec.x != 0 || moveVec.y != 0) {

				// 過去の矩形左下の位置を基準に移動を行いたいので
				// 坂と接触する位置を確認する
				sp = Point(prevRect.getMinX(), prevRect.getMinY() - TILE_COLLISION_THRESHOLD);
				ep = Point(prevRect.getMinX(), prevRect.getMaxY());

				// 過去の矩形左下が坂に接触しなかった場合
				if (!checkHitLine(&line, cross)) {

					// 矩形左下の位置を基準に移動を行いたいので
					// 坂と接触する位置を確認する
					sp = Point(prevRect.getMinX(), prevRect.getMinY());
					ep = Point(crntRect.getMinX(), crntRect.getMinY());

					if (!checkHitLine(&line, cross)) {

						//落ちている場合。
						if (falling) {
							sp = Point(prevRect.getMinX(), prevRect.getMaxY());
							ep = Point(crntRect.getMinX(), crntRect.getMinY());
							if (!checkHitLine(&line, cross)) {
								return false;
							}
						}
						else if (floor) {

							//オブジェクトが床にいる場合、床を移動していると仮定して移動先のY座標を過去に合わせる。
							sp = Point(prevRect.getMinX(), prevRect.getMinY());
							ep = Point(crntRect.getMaxX(), prevRect.getMinY());
							if (!checkHitLine(&line, cross)) {
								return false;
							}
						}
						else {
							return false;
						}
					}
				}
			}

			if (moveVec.x < 0) {
				if (cross.y >= _worldStartPoint.y) {
					cross.x = prevRect.getMinX();
				}
				// 過去の矩形の位置から坂を上る
				if (falling) {
					cross = calcMovePosition(moveVec.x, cross, crntRect.size);
				}
				else {
					cross = calcMovePosition(moveVec.x, moveVec.y, cross, crntRect.size);
				}
			}
			else if (moveVec.x > 0) {
				// 坂の頂上で坂に吸い寄せられるのを防ぐために
				// 現在の矩形の左端が坂の頂上の右にいる場合のみ位置の再設定を行う
				if (_worldStartPoint.x <= crntRect.getMinX()) {
					// 移動量を考慮した位置に設定する
					cross = calcMovePosition(moveVec.x, moveVec.y, cross, crntRect.size);
				}
				else if (_worldStartPoint.x <= crntRect.getMinX() + moveVec.x) {
					cross = _worldStartPoint;

					// 坂の始点からの移動を行うために移動量を減退させる
					moveVec.x -= _worldStartPoint.x - crntRect.getMinX();
					cross = calcMovePosition(moveVec.x, moveVec.y, cross, crntRect.size);
				}
				else {
					if (cross == _worldStartPoint) {
						cross.x = crntRect.origin.x;
					}
					else {
						cross = crntRect.origin;
					}
				}
			}
			else if (moveVec.y < 0) {
				if (cross.x >= _worldEndPoint.x) {
					cross.y = prevRect.getMinY();
				}
				// 過去の矩形の位置から坂を降りる。
				if (falling) {
					if (moveVec.x == 0.0f) {
						cross.x = prevRect.getMinX();
					}
					cross = calcMovePosition(moveVec.x, cross, crntRect.size);
					//接触しているところが始点・終点の場合かつオブジェクトの端の場合は接触していないと判断。
					if ((_worldStartPoint == cross && _worldStartPoint.x >= crntRect.getMaxX())
					||  (_worldEndPoint == cross && crntRect.getMinX() >= _worldEndPoint.x)) {
						return false;
					}
				}
				else {
					cross = calcMovePosition(moveVec.x, moveVec.y, cross, crntRect.size);
				}
			}
			else if (moveVec.y > 0) {
				if (_worldEndPoint.y <= crntRect.getMinY()) {

					// 移動量を考慮した位置に設定する
					cross = calcMovePosition(moveVec.x, moveVec.y, cross, crntRect.size);
				}
				else if (_worldEndPoint.y <= crntRect.getMinY() + moveVec.y) {
					cross = _worldEndPoint;

					moveVec.y -= _worldEndPoint.y - crntRect.getMinY();
					cross = calcMovePosition(moveVec.x, moveVec.y, cross, crntRect.size);
				}
				else {
					cross = crntRect.origin;
				}
			}
			else {
				// 現在の矩形の左端に合わせて座標を設定
				sp = Point(crntRect.getMinX(), _worldStartPoint.y);
				ep = Point(crntRect.getMinX(), _worldEndPoint.y);

				if (checkHitLine(&line, cross)) {
					return true;
				}
				else {
					// 現在の矩形の右端に合わせて座標を設定
					sp = Point(crntRect.getMaxX(), _worldStartPoint.y);
					ep = Point(crntRect.getMaxX(), _worldEndPoint.y);

					if (checkHitLine(&line, cross)) {
						cross.x -= crntRect.size.width;
						cross.y = _worldStartPoint.y;
						return true;
					}
					else {

						// オブジェクトの矩形より坂の幅の方が小さい場合
						if (getRect().size.width < crntRect.size.width) {
							// 矩形の幅より坂の幅の方が小さいために矩形の両端が坂に接触できない状態になっているので
							// 坂の頂上に配置する
							cross.x = crntRect.getMinX();
							cross.y = _worldStartPoint.y;
							return true;
						}
					}
				}

				return false;
			}

			return true;
		}

		return false;
	};

	// 過去の矩形の足元が坂に接触するかを確認
	sp = Point(prevRect.getMinX() - 0.1f, prevRect.getMinY());
	ep = Point(prevRect.getMaxX() + 0.1f, prevRect.getMinY());

	if (checkCross()) {
		return true;
	}

	//	CCLOG("過去の足元を抜けた");

	// 足元が坂に接触するかを確認
	if (moveVec.x <= 0) {
		sp = Point(prevRect.getMaxX(), prevRect.getMinY());
		ep = Point(crntRect.getMinX(), crntRect.getMinY());
	}
	else {
		sp = Point(prevRect.getMinX(), prevRect.getMinY());
		ep = Point(crntRect.getMaxX(), crntRect.getMinY());
	}
	
	if (checkCross()) {
		return true;
	}

//	CCLOG("足元を抜けた");

	// 現在の矩形と過去の矩形との対角線が坂に接触するかを確認
	if (moveVec.x <= 0) {
		sp = Point(prevRect.getMaxX(), prevRect.getMaxY());
		ep = Point(crntRect.getMinX(), crntRect.getMinY());
	}
	else {
		sp = Point(prevRect.getMinX(), prevRect.getMinY());
		ep = Point(crntRect.getMaxX(), crntRect.getMaxY());
	}

	if (checkCross()) {
		return true;
	}

//	CCLOG("対角線を抜けた");

	return false;
}

bool Slope::procDownSideDownSlope(Vec2& cross, Vec2& moveVec, Rect& crntRect, Rect& prevRect, bool& hitUp, bool& hitDown)
{
	bool falling = (_objectTmp != nullptr) ? (_objectTmp->_falling || _objectTmp->_fallingOld) : false;
	cocos2d::Point sp;
	cocos2d::Point ep;
	CollisionLine line = { &sp, &ep };

	std::function<bool(bool)> checkCross = [&](bool checkBottom) {

		if (checkHitLine(&line, cross)) {
			// 右移動、停止中の場合
			if (moveVec.x >= 0) {

				// 足元の確認を行っている場合
				if (checkBottom) {
					hitDown = true;

					// 坂の頂上で坂に吸い寄せられるのを防ぐために
					// 現在の矩形の左端が坂の頂上の右にいる場合のみ位置の再設定を行う
					if (_worldStartPoint.x <= crntRect.getMinX()) {

						// 移動量を考慮した位置に設定する
						cross = calcMovePosition(moveVec.x, moveVec.y, cross, crntRect.size);
					}
					else if (_worldStartPoint.x <= crntRect.getMinX() + moveVec.x) {
						cross = _worldStartPoint;

						// 坂の始点からの移動を行うために移動量を減退させる
						moveVec.x -= _worldStartPoint.x - crntRect.getMinX();
						cross = calcMovePosition(moveVec.x, moveVec.y, cross, crntRect.size);
					}
					else {
						cross = crntRect.origin;
					}
				}
				else {

					// 頭が接触している
					hitUp = true;

					if (crntRect.getMinX() < _worldEndPoint.x && _worldEndPoint.x < crntRect.getMaxX() && moveVec.x == 0 && moveVec.y > 0) {
						cross.x = crntRect.getMaxX();
						cross.y = _worldEndPoint.y;
						return true;
					}

					// 矩形右上の位置を基準に移動を行いたいので
					// 坂と接触する位置を確認する
					if (falling) {
						sp = Point(crntRect.getMinX(), crntRect.getMaxY());
						ep = Point(crntRect.getMaxX(), crntRect.getMaxY());
						if (!checkHitLine(&line, cross)) {
							cross.x = prevRect.getMaxX();
						}
					}
					else {
						sp = Point(prevRect.getMaxX(), prevRect.getMaxY()) - moveVec;
						ep = Point(crntRect.getMaxX(), crntRect.getMaxY()) + moveVec;
						if (!checkHitLine(&line, cross)) {
							cross.x = prevRect.getMaxX();
						}
					}

					// 過去の矩形の位置から坂を下る
					cross = calcMovePosition(moveVec.x, moveVec.y, cross, crntRect.size);
				}
			}
			else {
				if (checkBottom) {

					// 矩形左下の位置を基準に移動を行いたいので
					// 坂と接触する位置を確認する
					sp = Point(prevRect.getMinX(), prevRect.getMinY()) - moveVec;
					ep = Point(crntRect.getMinX(), crntRect.getMinY()) + moveVec;

					if (checkHitLine(&line, cross)) {
					}
					else {
						cross.x = prevRect.getMinX();
					}

					// 坂の頂点に到達している場合
					if (cross.y >= _worldStartPoint.y) {
						cross.x = prevRect.getMinX();
					}

					// 過去の矩形の位置から坂を上る
					cross = calcMovePosition(moveVec.x, moveVec.y, cross, crntRect.size);

					hitDown = true;
				}
				else {
					// 坂の頂上で坂に吸い寄せられるのを防ぐために
					// 現在の矩形の右端が坂の頂上の左にいる場合のみ位置の再設定を行う
					if (crntRect.getMaxX() <= _worldEndPoint.x) {
						// オブジェクトに重力がある時
						if (_objectTmp && _objectTmp->getObjectMovement()->getGravity() != cocos2d::Vec2::ZERO) {
							sp = Point(crntRect.getMaxX(), crntRect.getMinY()) + moveVec;
							ep = Point(crntRect.getMaxX(), crntRect.getMaxY()) + moveVec;
							if (!checkHitLine(&line, cross)) {
								return false;
							}
						}

						// 移動量を考慮した位置に設定する
						cross = calcMovePosition(moveVec.x, moveVec.y, cross, crntRect.size);
					}
					else if (crntRect.getMinX() < _worldEndPoint.x && _worldEndPoint.x < crntRect.getMaxX()) {
						sp = Point(prevRect.getMinX(), prevRect.getMaxY());
						ep = Point(crntRect.getMinX(), crntRect.getMaxY());
						if (!checkHitLine(&line, cross)) {
							return false;
						}
						cross.x += crntRect.size.width;
						// 移動量を考慮した位置に設定する
						cross = calcMovePosition(moveVec.x, moveVec.y, cross, crntRect.size);
					}
					else {
						cross = _worldEndPoint;

						// 坂の終点からの移動を行うために移動量を減退させる
						moveVec.x -= _worldEndPoint.x - crntRect.getMaxX();
						cross = calcMovePosition(moveVec.x, moveVec.y, cross, crntRect.size);
					}
					hitUp = true;

				}
			}

			return true;
		}

		return false;
	};

	// 頭上が坂に接触するかを確認
	if (moveVec.x >= 0) {
		sp = Point(prevRect.getMinX(), prevRect.getMaxY());
		ep = Point(crntRect.getMaxX(), crntRect.getMaxY());
	}
	else {
		sp = Point(prevRect.getMaxX(), prevRect.getMaxY());
		ep = Point(crntRect.getMinX(), crntRect.getMaxY());
	}

	if (checkCross(false)) {
		return true;
	}

//	CCLOG("頭上を抜けた");

	// 足元が坂に接触するかを確認
	if (moveVec.x >= 0) {
		sp = Point(prevRect.getMinX(), prevRect.getMinY());
		ep = Point(crntRect.getMaxX(), crntRect.getMinY());
	}
	else {
		sp = Point(prevRect.getMaxX(), prevRect.getMinY());
		ep = Point(crntRect.getMinX(), crntRect.getMinY());
	}

	if (checkCross(true)) {
		return true;
	}

//	CCLOG("足元を抜けた");

	// 足元（1px下に補正）が坂に接触するかを確認
	if (moveVec.x >= 0) {
		sp = Point(prevRect.getMinX(), prevRect.getMinY() - 1);
		ep = Point(crntRect.getMaxX(), crntRect.getMinY() - 1);
	}
	else {
		sp = Point(prevRect.getMaxX(), prevRect.getMinY() - 1);
		ep = Point(crntRect.getMinX(), crntRect.getMinY() - 1);
	}

	if (checkCross(true)) {
		return true;
	}

//	CCLOG("足元（補正）を抜けた");

	// 現在の矩形と過去の矩形との対角線が坂に接触するかを確認
	if (moveVec.x >= 0) {
		sp = Point(prevRect.getMinX(), prevRect.getMinY());
		ep = Point(crntRect.getMaxX(), crntRect.getMaxY());
	}
	else {
		sp = Point(prevRect.getMaxX(), prevRect.getMaxY());
		ep = Point(crntRect.getMinX(), crntRect.getMinY());
	}

	if (checkCross(false)) {
		return true;
	}

//	CCLOG("対角線を抜けた");

	return false;
}

#ifdef USE_MULTITHREAD_OBJECT_UPDATE
bool Slope::checkPushBackX(bool isPlayer, Rect& checkRect, Rect& oldRect, std::vector<agtk::Tile *> &tileList, float moveX, float& pushVal, float correctVal)
#else
bool Slope::checkPushBackX(bool isPlayer, Rect& checkRect, Rect& oldRect, cocos2d::Array* tileList, float moveX, float& pushVal, float correctVal)
#endif
{
	// 矩形サイズが0の場合は処理しない
	if (checkRect.size.equals(Size::ZERO)) {
		return false;
	}
	bool falling = (_objectTmp != nullptr) ? (_objectTmp->_falling || _objectTmp->_fallingOld) : false;
	bool jumping = (_objectTmp != nullptr) ? _objectTmp->_jumping : false;

	// 事前にupdateWorldSpaceRect関数を呼び出し
	// ワールド空間での坂の情報を更新しておく必要がある

	pushVal = 0;

	cocos2d::Point sp;
	cocos2d::Point ep;
	CollisionLine line = { &sp ,&ep };
	Vec2 cross;

	// 矩形左の接触確認
	sp = Point(checkRect.getMinX(), checkRect.getMinY());
	ep = Point(checkRect.getMinX(), checkRect.getMaxY());

	// オブジェクトの矩形をもとに坂の上下にいるかを取得する
// #AGTK-NX
#if CC_TARGET_PLATFORM != CC_PLATFORM_NX
	bool isPrevUpSide = checkUpSide(Vec2(oldRect.getMidX(), oldRect.getMidY()));
	bool isUpSide = checkUpSide(Vec2(checkRect.getMidX(), checkRect.getMidY()));
#else
#endif

	// 上から通過できない坂で、過去の位置が坂の上にいる場合
	if (!_passableFromUpper && isPrevUpSide) {
		// 現在の位置も坂の上にいる
		isUpSide = isPrevUpSide;
	}
	// 下から通過できない坂で、過去の位置が坂の下にいる場合
	else if (!_passableFromLower && !isPrevUpSide) {
		// 現在の位置も坂の下にいる
		isUpSide = isPrevUpSide;
	}

	// 坂の上にいる場合
	if (isUpSide) {
		// 上り坂の場合
		if (getType() == kTypeUp) {
			// 始点に接続している坂がある場合は処理しない
			auto connectStartSlope = getConnectStartSlope();
			if (connectStartSlope != nullptr) {
				if (getType() != connectStartSlope->getType()) {
					if (moveX > 0 && oldRect.getMaxX() <= _worldStartPoint.x) {

						//足元のY値が終点位置のY値以上の場合処理しない。
						if (connectStartSlope->_worldStartPoint.y <= oldRect.getMinY()) {
							return false;
						}

						// 矩形右の接触確認
						sp = Point(checkRect.getMaxX(), checkRect.getMinY());
						ep = Point(checkRect.getMaxX(), checkRect.getMaxY());

						// 接触した場合
						if (checkHitLine(&line, cross)) {
							pushVal = -((cross.x - _worldStartPoint.x) + correctVal);
							return true;
						}
					}
				}
				return false;
			}

			auto connectEndSlope = getConnectEndSlope();
			if (moveX > 0 && connectEndSlope != nullptr && connectEndSlope->getType() != getType() && connectEndSlope->getWorldStartPoint().x < _worldEndPoint.x) {
// #AGTK-NX
#if (CC_TARGET_PLATFORM == CC_PLATFORM_NX)
#else
				if (connectEndSlope->checkUpSide(Vec2(checkRect.getMidX(), checkRect.getMidY())) == false) {
#endif
					sp = Point(checkRect.getMinX() - moveX, checkRect.getMinY());
					ep = Point(checkRect.getMaxX() + moveX, checkRect.getMinY());
					// 接触した場合
					if (checkHitLine(&line, cross)) {
						pushVal = -((checkRect.getMaxX() - cross.x) + correctVal);
						if (calcNearestDistancePoint(checkRect.getMaxX(), checkRect.getMinY(), cross)) {
							pushVal = -((checkRect.getMaxX() - cross.x) + correctVal);
						}
						return true;
					}
					else {
						sp = Point(checkRect.getMinX() - moveX, checkRect.getMaxY());
						ep = Point(checkRect.getMaxX() + moveX, checkRect.getMaxY());
						// 接触した場合
						if (connectEndSlope->checkHitLine(&line, cross)) {
							pushVal = -((checkRect.getMaxX() - cross.x) + correctVal);
							if (calcNearestDistancePoint(checkRect.getMaxX(), checkRect.getMinY(), cross)) {
								pushVal = -((checkRect.getMaxX() - cross.x) + correctVal);
							}
							return true;
						}
					}
				}
			}

			if (checkRect.getMinY() < _worldStartPoint.y) {
				// 矩形右の接触確認
				sp = Point(checkRect.getMaxX(), checkRect.getMinY());
				ep = Point(checkRect.getMaxX(), checkRect.getMaxY());

				// 接触した場合
				if (checkHitLine(&line, cross)) {
					//落下中でかつ接触しているところが始点・終点の場合接触していないと判断。
					if ((falling || jumping) && (_worldStartPoint == cross || _worldEndPoint == cross)) {
						return false;
					}
					pushVal = -((checkRect.getMaxX() - cross.x) + correctVal);
					return true;
				}
			}
			else {
				// 右移動時のみ処理を行う
				if (moveX <= 0) { return false; }

				bool hitTile = false;
				float objectHeight = checkRect.size.height + correctVal;

				// 移動先でタイルに埋まってしまうかをチェック
#ifdef USE_MULTITHREAD_OBJECT_UPDATE
				for (auto tile : tileList) {
#else
				cocos2d::Ref* ref = nullptr;

				CCARRAY_FOREACH(tileList, ref) {
#endif
					bool hitLeft = false;
					bool hitRight = false;
					float leftH = 0;
					float rightH = 0;

#ifdef USE_MULTITHREAD_OBJECT_UPDATE
#else
					auto tile = dynamic_cast<agtk::Tile*>(ref);
#endif
					auto rect = tile->convertToLayerSpaceRect();
					auto wallRect = Rect(rect.origin, rect.size);

					// タイルが坂より低い位置にある場合は処理を飛ばす
					if (wallRect.getMaxY() <= _worldStartPoint.y) {
						continue;
					}

					// 通常のタイルの場合
					if (tile->getType() == agtk::Tile::kTypeTile) {

						// 確認用矩形より下にタイルがある場合は処理を飛ばす
						if (wallRect.getMinY() <= checkRect.getMinY()) {
							continue;
						}

						// タイル右と坂との距離を求める
						if (_worldStartPoint.x <= wallRect.getMaxX() && wallRect.getMaxX() <= _worldEndPoint.x) {

							sp = Point(wallRect.getMaxX(), _worldStartPoint.y);
							ep = Point(wallRect.getMaxX(), _worldEndPoint.y);

							if (checkHitLine(&line, cross)) {
								rightH = wallRect.getMinY() - cross.y;

								// タイル右側と坂との距離がオブジェクトの高さ以上ある場合は処理を飛ばす
								if (rightH > objectHeight) { continue; }

								hitRight = true;
							}
						}

						// タイル左と坂との距離を求める
						if (_worldStartPoint.x <= wallRect.getMinX() && wallRect.getMinX() <= _worldEndPoint.x) {
							sp = Point(wallRect.getMinX(), _worldStartPoint.y);
							ep = Point(wallRect.getMinX(), _worldEndPoint.y);

							if (checkHitLine(&line, cross)) {

								leftH = wallRect.getMinY() - cross.y;

								// タイルの壁当たり情報を基に通過できるかをチェックする
								auto wallBit = tile->getWallBit();

								if (wallBit != 0x0f) {

									// 左にも下にも壁当たり判定がついていない場合は
									// 通過できるので処理を飛ばす
									if (!(wallBit & (1 << agtk::data::TilesetData::Left)) &&
										!(wallBit & (1 << agtk::data::TilesetData::Down))) {
										continue;
									}

									// 左側と下側の両方に壁当たり判定が設定されている場合
									if (wallBit & (1 << agtk::data::TilesetData::Down) &&
										wallBit & (1 << agtk::data::TilesetData::Left)) {
										// タイル左側と坂との距離がオブジェクトの高さより低い場合は処理を飛ばす
										if (leftH < objectHeight) { continue; }
									}
									// 下側に壁当たり判定が設定されている場合
									else if (wallBit & (1 << agtk::data::TilesetData::Down)) {
										// タイル左側と坂との距離がオブジェクトの高さより低い場合は処理を飛ばす
										if (leftH < objectHeight) { continue; }
									}
									// 左側に壁当たり判定が設定されている場合
									else if (wallBit & (1 << agtk::data::TilesetData::Left)) {
										// タイル左側と坂との距離がオブジェクトの高さ以上ある場合は処理を飛ばす
										if (leftH > objectHeight) { continue; }
									}
								}

								hitLeft = true;
							}
						}
					}
					// 行動範囲制限の場合
					else {
						// プレイヤーオブジェクトでなければ処理しない
						if (!isPlayer) {
							continue;
						}

						auto name = tile->getName();

						// 上に設定された行動範囲制限の場合
						if (name.compare("limitTileUp") == 0) {
							// 行動範囲制限に埋まった量を算出
							float val = checkRect.getMaxY() - rect.getMinY();

							// 埋まりが発生している場合
							if (val > 0) {

								// Y方向の押し戻しを考慮した位置で坂に接触するかをチェック
								sp = Point(_worldStartPoint.x, checkRect.getMinY() - val);
								ep = Point(_worldEndPoint.x, sp.y);
								if (checkHitLine(&line, cross)) {

									// X方向の押し戻しを行い、押し戻しが必要ならば押し戻しを行う
									val = checkRect.getMaxX() - cross.x;
									if (val > 0) {
										pushVal = -val;
										return true;
									}
								}
							}
						}
						// 右に設定された行動範囲制限の場合
						else if (name.compare("limitTileRight") == 0) {

							// 壁に埋まっている量を算出
							float val = checkRect.getMaxX() - rect.getMinX();

							// 埋まっている場合は押し戻す
							if (val > 0) {
								pushVal = -val;
								return true;
							}
						}
					}


					// 坂との接触ができないので処理しない
					if (!hitLeft && !hitRight) { continue; }

					float val = 0;

					// どこまで押し戻すかを算出

					// オブジェクトの幅よりタイルと坂との間の方が小さい場合
					if (leftH < objectHeight) {
						// タイル左端まで押し戻す
						val = -((checkRect.getMaxX() - wallRect.getMinX()) + correctVal);

						hitTile = true;

						if (pushVal > val) {
							pushVal = val;
						}
					}
					// タイルと坂との間にオブジェクトが配置できそうな場合
					else if (leftH >= objectHeight) {
						// 配置できる位置を検索する
						sp = Point(checkRect.getMinX(), wallRect.getMinY() - objectHeight);
						ep = Point(checkRect.getMaxX(), wallRect.getMinY() - objectHeight);

						if (checkHitLine(&line, cross)) {

							float h = wallRect.getMinY() - cross.y - correctVal;
							if (h > checkRect.size.height + correctVal) {
								continue;
							}

							val = -((checkRect.getMaxX() - cross.x) + correctVal);

							hitTile = true;

							if (pushVal > val) {
								pushVal = val;
							}
						}
					}
				}

				return hitTile;
			}
		}
		else {
			// 終点に接続している坂がある場合は処理しない
			auto connectEndSlope = getConnectEndSlope();
			if (connectEndSlope != nullptr) {
				if (getType() != connectEndSlope->getType()) {
					if (moveX < 0 && _worldEndPoint.x <= oldRect.getMinX()) {

						//足元のY値が終点位置のY値以上の場合処理しない。
						if (connectEndSlope->_worldEndPoint.y <= oldRect.getMinY()) {
							return false;
						}

						// 矩形左の接触確認
						sp = Point(checkRect.getMinX(), checkRect.getMinY());
						ep = Point(checkRect.getMinX(), checkRect.getMaxY());

						// 接触した場合
						if (checkHitLine(&line, cross)) {
							pushVal = _worldEndPoint.x - cross.x + correctVal;
							return true;
						}
					}
				}
				return false;
			}

			auto connectStartSlope = getConnectStartSlope();
			if (moveX < 0 && connectStartSlope != nullptr && connectStartSlope->getType() != getType() && connectStartSlope->getWorldEndPoint().x > _worldStartPoint.x) {
// #AGTK-NX
#if (CC_TARGET_PLATFORM == CC_PLATFORM_NX)
#else
				if (connectStartSlope->checkUpSide(Vec2(checkRect.getMidX(), checkRect.getMidY())) == false) {
#endif
					sp = Point(checkRect.getMinX() + moveX, checkRect.getMinY());
					ep = Point(checkRect.getMaxX() - moveX, checkRect.getMinY());
					// 接触した場合
					if (checkHitLine(&line, cross)) {
						pushVal = (cross.x - checkRect.getMinX()) + correctVal;
						if (calcNearestDistancePoint(checkRect.getMinX(), checkRect.getMinY(), cross)) {
							pushVal = (cross.x - checkRect.getMinX()) + correctVal;
						}
						return true;
					}
					else {
						sp = Point(checkRect.getMinX() + moveX, checkRect.getMaxY());
						ep = Point(checkRect.getMaxX() - moveX, checkRect.getMaxY());
						// 接触した場合
						if (connectStartSlope->checkHitLine(&line, cross)) {
							pushVal = (cross.x - checkRect.getMinX()) + correctVal;
							if (calcNearestDistancePoint(checkRect.getMinX(), checkRect.getMinY(), cross)) {
								pushVal = (cross.x - checkRect.getMinX()) + correctVal;
							}
							return true;
						}
					}
				}
			}

			if (checkRect.getMinY() < _worldEndPoint.y) {
				// 矩形左の接触確認
				sp = Point(checkRect.getMinX(), checkRect.getMinY());
				ep = Point(checkRect.getMinX(), checkRect.getMaxY());

				// 接触した場合
				if (checkHitLine(&line, cross)) {
					//落下中でかつ接触しているところが始点・終点の場合接触していないと判断。
					if ((falling || jumping) && (_worldStartPoint == cross || _worldEndPoint == cross)) {
						return false;
					}
					pushVal = (cross.x - checkRect.getMinX()) + correctVal;
					return true;
				}
			}
			else {

				// 左移動時のみ処理を行う
				if (moveX >= 0) { return false; }

				bool hitTile = false;
				float objectHeight = checkRect.size.height + correctVal;

				// 移動先でタイルに埋まってしまうかをチェック
#ifdef USE_MULTITHREAD_OBJECT_UPDATE
				for (auto tile : tileList) {
#else
				cocos2d::Ref* ref = nullptr;

				CCARRAY_FOREACH(tileList, ref) {
#endif
					bool hitLeft = false;
					bool hitRight = false;
					float leftH = 0;
					float rightH = 0;

#ifdef USE_MULTITHREAD_OBJECT_UPDATE
#else
					auto tile = dynamic_cast<agtk::Tile*>(ref);
#endif
					auto rect = tile->convertToLayerSpaceRect();
					auto wallRect = Rect(rect.origin, rect.size);

					// タイルが坂より低い位置にある場合は処理を飛ばす
					if (wallRect.getMaxY() <= _worldEndPoint.y) {
						continue;
					}

					//矩形がタイルより左側にある場合（タイルがX軸で接触していない場合）
					if (checkRect.getMaxX() < wallRect.getMinX()) {
						continue;
					}

					// 通常のタイルの場合
					if (tile->getType() == agtk::Tile::kTypeTile) {

						// 確認用矩形より下にタイルがある場合は処理を飛ばす
						if (wallRect.getMinY() <= checkRect.getMinY()) {
							continue;
						}

						// タイル左と坂との距離を求める
						if (_worldStartPoint.x < wallRect.getMinX() && wallRect.getMinX() <= _worldEndPoint.x) {

							sp = Point(wallRect.getMinX(), _worldStartPoint.y);
							ep = Point(wallRect.getMinX(), _worldEndPoint.y);

							if (checkHitLine(&line, cross)) {
								leftH = wallRect.getMinY() - cross.y;

								// タイル左側と坂との距離がオブジェクトの高さ以上ある場合は処理を飛ばす
								if (leftH > objectHeight) { continue; }

								hitLeft = true;
							}
						}

						// タイル右と坂との距離を求める
						if (_worldStartPoint.x <= wallRect.getMaxX() && wallRect.getMaxX() <= _worldEndPoint.x) {
							sp = Point(wallRect.getMaxX(), _worldStartPoint.y);
							ep = Point(wallRect.getMaxX(), _worldEndPoint.y);

							if (checkHitLine(&line, cross)) {

								rightH = wallRect.getMinY() - cross.y;

								// タイルの壁当たり情報を基に通過できるかをチェック
								auto wallBit = tile->getWallBit();

								if (wallBit != 0x0f) {
									// 右にも下にも壁当たり判定がついていない場合は
									// 通過できるので処理を飛ばす
									if (!(wallBit & (1 << agtk::data::TilesetData::Right)) &&
										!(wallBit & (1 << agtk::data::TilesetData::Down))) {
										continue;
									}

									// 右側と下側の両方に壁当たり判定が設定されている場合
									if (wallBit & (1 << agtk::data::TilesetData::Down) &&
										wallBit & (1 << agtk::data::TilesetData::Right)) {
										// タイル右側と坂との距離がオブジェクトの高さより低い場合は処理を飛ばす
										if (rightH < objectHeight) { continue; }
									}
									// 下側に壁当たり判定が設定されている場合
									else if (wallBit & (1 << agtk::data::TilesetData::Down)) {
										// タイル右側と坂との距離がオブジェクトの高さより低い場合は処理を飛ばす
										if (rightH < objectHeight) { continue; }
									}
									// 右側に壁当たり判定が設定されている場合
									else if (wallBit & (1 << agtk::data::TilesetData::Right)) {
										// タイル右側と坂との距離がオブジェクトの高さ以上ある場合は処理を飛ばす
										if (rightH > objectHeight) { continue; }
									}
								}

								hitRight = true;

							}
						}
					}
					// 行動範囲制限の場合
					else {
						// プレイヤーオブジェクトでなければ処理しない
						if (!isPlayer) {
							continue;
						}

						auto name = tile->getName();

						// 上に設定された行動範囲制限の場合
						if (name.compare("limitTileUp") == 0) {
							// 行動範囲制限に埋まった量を算出
							float val = checkRect.getMaxY() - rect.getMinY();

							// 埋まりが発生している場合
							if (val > 0) {
								// Y方向の押し戻しを考慮した位置で坂に接触するかをチェック
								sp = Point(_worldStartPoint.x, checkRect.getMinY() - val);
								ep = Point(_worldEndPoint.x, sp.y);
								if (checkHitLine(&line, cross)) {

									// X方向の押し戻しを行い、押し戻しが必要なら押し戻しを行う
									val = cross.x - checkRect.getMinX();
									if (val > 0) {
										pushVal = val;
										return true;
									}
								}
							}
						}
						// 左に設定された行動範囲制限の場合
						else if (name.compare("limitTileLeft") == 0) {
							// 壁に埋まっている量を算出
							float val = rect.getMaxX() - checkRect.getMinX();

							// 壁に埋まっている場合は押し戻す
							if (val > 0) {
								pushVal = val;
								return true;
							}
						}
					}

					// 坂との接触ができないので処理しない
					if (!hitLeft && !hitRight) { continue;  }

					float val = 0;

					// どこまで押し戻すかを算出

					// オブジェクトの幅よりタイルと坂との間の方が小さい場合
					if (rightH < objectHeight) {
						// タイル右端まで押し戻す
						val = (wallRect.getMaxX() - checkRect.getMinX()) + correctVal;

						hitTile = true;

						if (pushVal < val) {
							pushVal = val;
						}
					}
					// タイルと坂との間にオブジェクトが配置できそうな場合
					else if (rightH >= objectHeight) {
						// 配置できる位置を検索する
						sp = Point(checkRect.getMinX(), wallRect.getMinY() - objectHeight);
						ep = Point(checkRect.getMaxX(), wallRect.getMinY() - objectHeight);

						if (checkHitLine(&line, cross)) {
							float h = wallRect.getMinY() - cross.y - correctVal;
							if (h > checkRect.size.height + correctVal) {
								continue;
							}

							val = (cross.x - checkRect.getMinX()) + correctVal;

							hitTile = true;

							if (pushVal < val) {
								pushVal = val;
							}
						}
					}
				}

				return hitTile;
			}
		}
	}
	else {

		// 矩形上の接触確認
		sp = Point(checkRect.getMinX(), checkRect.getMaxY());
		ep = Point(checkRect.getMaxX(), checkRect.getMaxY());

		// 接触した場合
		if (checkHitLine(&line, cross)) {
#if 1//「ACT2-3841 坂に接触するキャラクター移動で不具合」の修正（キャラクターが大きくズレ「移動」してしまうため）
#if 1//「ACT2-4962 坂の下にオブジェクトが衝突した時の判定が左右で相違の修正）
			// 上り坂の場合
			if (getType() == kTypeUp) {
				if(oldRect.getMinX() > _worldStartPoint.x) {
					pushVal = (cross.x - checkRect.getMinX()) + correctVal;
					auto connectStartSlope = getConnectStartSlope();
					if (moveX < 0 && connectStartSlope && connectStartSlope->getType() != getType()) {
						if (calcNearestDistancePoint(checkRect.getMinX(), checkRect.getMaxY(), cross)) {
							pushVal = (cross.x - checkRect.getMinX()) + correctVal;
						}
					}
				}
			}
			// 下り坂の場合
			else {
				if(_worldEndPoint.x > oldRect.getMaxX()) {
					pushVal = -((checkRect.getMaxX() - cross.x) + correctVal);
					auto connectEndSlope = getConnectEndSlope();
					if (moveX > 0 && connectEndSlope && connectEndSlope->getType() != getType()) {
						if (calcNearestDistancePoint(checkRect.getMaxX(), checkRect.getMaxY(), cross)) {
							pushVal = -((checkRect.getMaxX() - cross.x) + correctVal);
						}
					}
				}
			}
#endif
#else
			// 上り坂の場合
			if (getType() == kTypeUp) {

				if ((oldRect.getMaxX() - 1.0f) < _worldStartPoint.x) {
					pushVal = -((checkRect.getMaxX() - _worldStartPoint.x) + 1.0f + correctVal);
				}
				else {
					pushVal = (cross.x - checkRect.getMinX()) + correctVal;
				}
			}
			// 下り坂の場合
			else {

				if (_worldEndPoint.x < (oldRect.getMinX() + 1.0f)) {
					pushVal = (_worldEndPoint.x - checkRect.getMinX()) + 1.0f + correctVal;
				}
				else {
 					pushVal = -((checkRect.getMaxX() - cross.x) + correctVal);
				}
			}
#endif
			return true;
		}

		//坂に接していない。
		if (!_worldSpaceRect.intersectsRect(checkRect)) {
			return false;
		}

		bool bConnectSlope = false;
		if ((getConnectStartSlope() && getConnectStartSlope()->getType() != getType())
		|| (getConnectEndSlope() && getConnectEndSlope()->getType() != getType())) {
			bConnectSlope = true;
		}

		// 上り坂の場合
		if (getType() == kTypeUp) {

			//if ((oldRect.getMaxX() - 1.0f) < _worldStartPoint.x) {
			if (oldRect.getMaxX() < _worldStartPoint.x) {
				// 始点に接続している坂がある場合は処理しない
				if (getConnectStartSlope() != nullptr) { return false; }

				// 矩形右の接触確認
				sp = Point(checkRect.getMaxX(), _worldEndPoint.y);
				ep = Point(checkRect.getMaxX(), _worldStartPoint.y);

				if (checkHitLine(&line, cross)) {
					//pushVal = -((checkRect.getMaxX() - _worldStartPoint.x) + 1.0f + correctVal);
					pushVal = -((checkRect.getMaxX() - _worldStartPoint.x) + correctVal);
					if (pushVal != 0.0f) {
						return true;
					}
				}
			}
			else {
				// 矩形左の接触確認
				sp = Point(checkRect.getMinX(), checkRect.getMinY());
				ep = Point(checkRect.getMinX(), checkRect.getMaxY());

				// 接触した場合
				if (checkHitLine(&line, cross)) {
					//接触しているところが始点・終点の場合接触していないと判断。
					if (_worldStartPoint == cross || _worldEndPoint == cross) {
						return false;
					}

					pushVal = (cross.x - checkRect.getMinX()) + correctVal;
					if (bConnectSlope) {
						if (calcNearestDistancePoint(checkRect.getMinX(), checkRect.getMaxY(), cross)) {
							pushVal = (cross.x - checkRect.getMinX()) + correctVal;
						}
					}
					if (pushVal != 0.0f) {
						return true;
					}
				}
			}
		}
		// 下り坂の場合
		else {

			//if (_worldEndPoint.x < (oldRect.getMinX() + 1.0f)) {
			if (_worldEndPoint.x < oldRect.getMinX()) {
				// 終点に接続している坂がある場合は処理しない
				if (getConnectEndSlope() != nullptr) { return false; }

				// 矩形左の接触確認
				sp = Point(checkRect.getMinX(), _worldEndPoint.y);
				ep = Point(checkRect.getMinX(), _worldStartPoint.y);

				if (checkHitLine(&line, cross)) {
					//pushVal = (_worldEndPoint.x - checkRect.getMinX()) + 1.0f + correctVal;
					pushVal = (_worldEndPoint.x - checkRect.getMinX()) + correctVal;
					if (pushVal != 0.0f) {
						return true;
					}
				}
			}
			else {
				// 矩形右の接触確認
				sp = Point(checkRect.getMaxX(), checkRect.getMinY());
				ep = Point(checkRect.getMaxX(), checkRect.getMaxY());

				// 接触した場合
				if (checkHitLine(&line, cross)) {
					//接触しているところが始点・終点の場合接触していないと判断。
					if (_worldStartPoint == cross || _worldEndPoint == cross) {
						return false;
					}

					pushVal = -((checkRect.getMaxX() - cross.x) + correctVal);
					if (bConnectSlope) {
						if (calcNearestDistancePoint(checkRect.getMaxX(), checkRect.getMaxY(), cross)) {
							pushVal = -((checkRect.getMaxX() - cross.x) + correctVal);
						}
					}
					if (pushVal != 0.0f) {
						return true;
					}
				}
			}
		}
	}

	return false;
}

#ifdef USE_MULTITHREAD_OBJECT_UPDATE
bool Slope::checkPushBackY(Rect& checkRect, Rect& oldRect, std::vector<agtk::Tile *> &tileList, float moveY, float& pushVal, float correctVal)
#else
bool Slope::checkPushBackY(Rect& checkRect, Rect& oldRect, cocos2d::Array* tileList, tileList, float moveY, float& pushVal, float correctVal)
#endif
{
	// 矩形サイズが0の場合は処理しない
	if (checkRect.size.equals(Size::ZERO)) {
		return false;
	}
	pushVal = 0;

	// #AGTK-NX
#if CC_TARGET_PLATFORM != CC_PLATFORM_NX
	// オブジェクトの矩形をもとに坂の上下にいるかを取得する
	bool isPrevUpSide = checkUpSide(Vec2(oldRect.getMidX(), oldRect.getMidY()));
	bool isUpSide = checkUpSide(Vec2(checkRect.getMidX(), checkRect.getMidY()));
#else
#endif
	// 上から通過できない坂で、過去の位置が坂の上にいる場合
	if (!_passableFromUpper && isPrevUpSide) {
		// 現在の位置も坂の上にいる
		isUpSide = isPrevUpSide;
	}
	// 下から通過できない坂で、過去の位置が坂の下にいる場合
	else if (!_passableFromLower && !isPrevUpSide) {
		// 現在の位置も坂の下にいる
		isUpSide = isPrevUpSide;
	}

	cocos2d::Point sp;
	cocos2d::Point ep;
	CollisionLine line = { &sp ,&ep };
	Vec2 cross;

	bool bConnectSlope = false;
	if ((getConnectStartSlope() && getConnectStartSlope()->getType() != getType())
	|| (getConnectEndSlope() && getConnectEndSlope()->getType() != getType())) {
		bConnectSlope = true;
	}

	if (bConnectSlope) {
		// 坂の下にいる場合
		if (!isUpSide) {

			if (getType() == kTypeUp) {
				// 矩形左の接触確認。
				sp = Point(checkRect.getMinX(), checkRect.getMinY());
				ep = Point(checkRect.getMinX(), checkRect.getMaxY());

				// 接触した場合
				if (checkHitLine(&line, cross)) {
					pushVal = (cross.y - checkRect.getMaxY()) + correctVal;
					return true;
				}
			}
			else {

				// 矩形右の接触確認
				sp = Point(checkRect.getMaxX(), checkRect.getMinY());
				ep = Point(checkRect.getMaxX(), checkRect.getMaxY());

				// 接触した場合
				if (checkHitLine(&line, cross)) {
					pushVal = (cross.y - checkRect.getMaxY()) + correctVal;
					return true;
				}

			}
			return false;
		}
	}

	// 矩形下の接触確認
	sp = Point(checkRect.getMinX(), checkRect.getMinY());
	ep = Point(checkRect.getMaxX(), checkRect.getMinY());

	// 接触した場合
	bool bTouchLine = false;
	if (checkHitLine(&line, cross)) {
		bTouchLine = true;
	}
	else {
		// 矩形下の接触確認
		sp = Point(checkRect.getMinX(), checkRect.getMinY() - TILE_COLLISION_THRESHOLD);
		ep = Point(checkRect.getMaxX(), checkRect.getMinY() - TILE_COLLISION_THRESHOLD);
		if (checkHitLine(&line, cross)) {
			bTouchLine = true;
		}

		if (bConnectSlope) {
			// 上り坂にいる場合。
			if (getType() == kTypeUp) {
				// 矩形下の接触確認
				sp = Point(checkRect.getMaxX(), checkRect.getMinY());
				ep = Point(checkRect.getMaxX(), checkRect.getMaxY());
				if (checkHitLine(&line, cross)) {
					//接触しているところが始点・終点の場合接触していないと判断。
					if (_worldStartPoint != cross && _worldEndPoint != cross) {
						bTouchLine = true;
					}
				}
			}
			// 下り坂にいる場合。
			else {
				// 矩形下の接触確認
				sp = Point(checkRect.getMinX(), checkRect.getMinY());
				ep = Point(checkRect.getMinX(), checkRect.getMaxY());
				if (checkHitLine(&line, cross)) {
					//接触しているところが始点・終点の場合接触していないと判断。
					if (_worldStartPoint != cross && _worldEndPoint != cross) {
						bTouchLine = true;
					}
				}
			}
		}
	}

	if (bTouchLine) {
		// 上り坂にいる場合
		if (getType() == kTypeUp) {

			// 矩形右の接触確認
			sp = Point(checkRect.getMaxX(), checkRect.getMinY());
			ep = Point(checkRect.getMaxX(), checkRect.getMaxY());

			if (checkHitLine(&line, cross)) {
				pushVal = (cross.y - checkRect.getMinY()) + correctVal;
				return true;
			}
			else {
				// 坂の終点を取得
				cocos2d::Point slopeEndPoint = cocos2d::Point(agtk::Scene::getPositionCocos2dFromScene(end));


			}
		}
		// 下り坂にいる場合
		else {

			// 矩形左の接触確認
			sp = Point(checkRect.getMinX(), checkRect.getMinY());
			ep = Point(checkRect.getMinX(), checkRect.getMaxY());

			if (checkHitLine(&line, cross)) {
				pushVal = (cross.y - checkRect.getMinY()) + correctVal;
				return true;
			}
			else {
				// 矩形右の接触確認
				sp = Point(checkRect.getMaxX(), checkRect.getMinY());
				ep = Point(checkRect.getMaxX(), checkRect.getMaxY());

				if (checkHitLine(&line, cross)) {
					pushVal = (cross.y - checkRect.getMinY()) + correctVal;
					return true;
				}

				/*
				// 坂の始点を取得
				cocos2d::Point slopeStartPoint = cocos2d::Point(agtk::Scene::getPositionCocos2dFromScene(start));

				if (checkRect.getMinX() < slopeStartPoint.x && slopeStartPoint.x < checkRect.getMaxX()) {
					pushVal = (slopeStartPoint.y - checkRect.getMinY()) + correctVal;
					return true;
				}
				*/
			}
		}
	}
	else {
		// 上り坂にいる場合
		if (getType() == kTypeUp) {

			if ((_worldStartPoint.y < checkRect.getMaxY() && checkRect.getMaxY() < _worldEndPoint.y) && (isUpSide == false)) {

#ifdef USE_MULTITHREAD_OBJECT_UPDATE
				for (auto tile : tileList) {
#else
				cocos2d::Ref* ref = nullptr;
				CCARRAY_FOREACH(tileList, ref) {
					auto tile = dynamic_cast<agtk::Tile*>(ref);
#endif
					auto rect = tile->convertToLayerSpaceRect();
					cocos2d::Vec2 pos = rect.origin;
					cocos2d::Size size = rect.size;
					auto wallRect = Rect(pos, size);
					pos.x += TILE_COLLISION_THRESHOLD;
					size.width -= TILE_COLLISION_THRESHOLD * 2;
					auto checkWallRect = Rect(pos, size);

					if (checkWallRect.intersectsRect(checkRect)) {
						// 通常のタイルの場合
						if (tile->getType() == agtk::Tile::kTypeTile) {
							auto wallBit = tile->getWallBit();
							// タイル上にオブジェクトがある。
							if (moveY < 0.0f && (wallRect.getMaxY() > checkRect.getMinY() && (wallBit & (1 << agtk::data::TilesetData::Up)))) {
								pushVal = wallRect.getMaxY() - checkRect.getMinY();
							}
							// タイル下にオブジェクトがある。
							if (moveY > 0.0f && (wallRect.getMinY() < checkRect.getMaxY()) && (wallBit & (1 << agtk::data::TilesetData::Down))) {
								pushVal = wallRect.getMinY() - checkRect.getMaxY();
							}
							if (pushVal != 0.0f) {
								return true;
							}
						}
					}
				}
			}
		}
		// 下り坂にいる場合
		else {

			if ((_worldStartPoint.y > checkRect.getMaxY() && checkRect.getMaxY() > _worldEndPoint.y) && (isUpSide == false)) {
				
				// 移動先でタイルに埋まってしまうかをチェック
#ifdef USE_MULTITHREAD_OBJECT_UPDATE
				for (auto tile : tileList) {
#else
				cocos2d::Ref* ref = nullptr;
				CCARRAY_FOREACH(tileList, ref) {
					auto tile = dynamic_cast<agtk::Tile*>(ref);
#endif
					auto rect = tile->convertToLayerSpaceRect();
					cocos2d::Vec2 pos = rect.origin;
					cocos2d::Size size = rect.size;
					auto wallRect = Rect(pos, size);
					pos.x += TILE_COLLISION_THRESHOLD;
					size.width -= TILE_COLLISION_THRESHOLD * 2;
					auto checkWallRect = Rect(pos, size);

					if (checkWallRect.intersectsRect(checkRect)) {
						// 通常のタイルの場合
						if (tile->getType() == agtk::Tile::kTypeTile) {
							auto wallBit = tile->getWallBit();
							// タイル上にオブジェクトがある。
							if (moveY < 0.0f && (wallRect.getMaxY() > checkRect.getMinY() && (wallBit & (1 << agtk::data::TilesetData::Up)))) {
								pushVal = wallRect.getMaxY() - checkRect.getMinY();
							}
							// タイル下にオブジェクトがある。
							if (moveY > 0.0f && (wallRect.getMinY() < checkRect.getMaxY()) && (wallBit & (1 << agtk::data::TilesetData::Down))) {
								pushVal = wallRect.getMinY() - checkRect.getMaxY();
							}
							if (pushVal != 0.0f) {
								return true;
							}
						}
					}
				}
			}
		}
	}

	return false;
}

int Slope::getCheckHitWallHitInfoId(agtk::WallHitInfoGroup* group, cocos2d::Vec2& moveVec)
{
	if (group->getWallHitInfoListCount() == 1) {
		return 0;
	}

	// ワールド空間での坂の矩形情報を更新しておく
	updateWorldSpaceRect();
	
	int topId = -1;
	int bottomId = -1;
	int leftId = -1;
	int rightId = -1;

	float addHeight = 0.2f;

	std::vector<int> hitInfoIdList;

	// 各矩形が坂に接触しているかチェック
	for (unsigned int i = 0; i < group->getWallHitInfoListCount(); i++) {
		auto wallHitInfo = group->getWallHitInfo(i);

		auto boundMin = wallHitInfo.boundMin;
		auto boundMax = wallHitInfo.boundMax;

		bool isHitPrevRect = false;

		auto currentRect = Rect(boundMin, Size(boundMax - boundMin));
		auto prevRect = Rect(currentRect.origin - moveVec, currentRect.size);

		currentRect.origin.y -= addHeight;
		prevRect.origin.y -= addHeight;
		currentRect.size.height += addHeight;
		prevRect.size.height += addHeight;

		// 過去の矩形から現在の矩形までの間で坂に接触した場合
		if (checkHitRect(currentRect, prevRect, isHitPrevRect)) {

			hitInfoIdList.push_back(i);
		}
	}

	if (hitInfoIdList.size() <= 0) { return -1; }

	// 坂に接触している矩形のみで各上下左右の矩形のIDを設定する
	for (unsigned int i = 0; i < hitInfoIdList.size(); i++) {
		int id = hitInfoIdList[i];

		auto wallHitInfo = group->getWallHitInfo(id);

		// 一番上にいる壁当たりのIDを設定
		if (topId < 0) {
			topId = id;
		}
		else {
			if (group->getWallHitInfo(topId).boundMax.y < wallHitInfo.boundMax.y) {
				topId = id;
			}
		}
		
		// 一番下にある壁当たりのIDを設定
		if (bottomId < 0) {
			bottomId = id;
		}
		else {
			if (group->getWallHitInfo(bottomId).boundMax.y > wallHitInfo.boundMax.y) {
				bottomId = id;
			}
		}

		// 一番左にある壁当たりのIDを設定
		if (leftId < 0) {
			leftId = id;
		}
		else {
			if (group->getWallHitInfo(leftId).boundMin.x > wallHitInfo.boundMin.x) {
				leftId = id;
			}
		}

		// 一番右にある壁当たりのIDを設定
		if (rightId < 0) {
			rightId = id;
		}
		else {
			if (group->getWallHitInfo(rightId).boundMax.x < wallHitInfo.boundMax.x) {
				rightId = id;
			}
		}
	}

	// 各壁当たり情報を取得
	agtk::WallHitInfo topInfo, bottomInfo, leftInfo, rightInfo;
	if (topId >= 0)		{ topInfo = group->getWallHitInfo(topId); }
	if (bottomId >= 0)	{ bottomInfo = group->getWallHitInfo(bottomId); }
	if (leftId >= 0)	{ leftInfo = group->getWallHitInfo(leftId); }
	if (rightId >= 0)	{ rightInfo = group->getWallHitInfo(rightId); }

	// 上り坂の場合
	if (getType() == Type::kTypeUp) {

		// 一番下にある壁当たりと一番右にある壁当たりが同じ場合、そのIDを返す
		if (bottomId == rightId) { return bottomId; }

		// 一番下の壁当たりと一番右の壁当たりの右下で角度を算出
		Vec2 bottomPos = Vec2(bottomInfo.boundMax.x, bottomInfo.boundMin.y);
		Vec2 rightPos = Vec2(rightInfo.boundMax.x, rightInfo.boundMin.y);

		// 2つの壁当たりに高低差がある場合のみ角度を算出
		float height = abs(rightPos.y - bottomPos.y);
		if (height > 0) {

			// 右壁当たりが坂を上りきっている可能性があるので坂の終点を超えている場合は
			// 右壁当たり右下の座標を坂の終点に設定する
			if (rightInfo.boundMax.x > _worldEndPoint.x) {
				rightPos.x = _worldEndPoint.x;
			}

			float width = rightPos.x - bottomPos.x;
			float d = RadianToDegree(atan(height / width));

			if (_degree >= d) {
				return rightId;
			}
			else {
				return bottomId;
			}
		}
		// 
		else {
			return rightId;
		}
	}
	// 下り坂の場合
	else {

		// 一番下にある壁当たりと一番左にある壁当たりが同じ場合、そのIDを返す
		if (bottomId == leftId) { return bottomId; }

		// 一番下の壁当たりと一番左の壁当たりの左下で角度を算出
		Vec2 bottomPos = Vec2(bottomInfo.boundMin.x, bottomInfo.boundMin.y);
		Vec2 leftPos = Vec2(leftInfo.boundMin.x, leftInfo.boundMin.y);

		// 2つの壁当たりにleftPos場合のみ角度を算出
		float height = abs(leftPos.y - bottomPos.y);
		if (height > 0) {

			// 左壁当たりが坂を上りきっている可能性があるので坂の始点を超えている場合は
			// 左壁当たり左下の座標を坂の始点に設定する
			if (leftInfo.boundMin.x < _worldStartPoint.x) {
				leftPos.x = _worldStartPoint.x;
			}

			float width = leftPos.x - bottomPos.x;
			float d = RadianToDegree(atan(height / width));

			if (_degree <= d) {
				return leftId;
			}
			else {
				return bottomId;
			}
		}
		// 
		else {
			return leftId;
		}
	}

	return -1;
}

float Slope::getSlipValue()
{
	float val = abs(_degree) / 90.0f;

	return val * 1.5f;
}

void Slope::checkConnect(cocos2d::Array* slopeList)
{
	int idx = slopeList->getIndexOfObject(this);
	bool canMove = true;

	if (idx >= 0) {
		for (int i = 0; i < slopeList->count(); i++) {

			if (i == idx) { continue; }

// #AGTK-NX
#ifdef STATIC_DOWN_CAST
			auto slope = static_cast<Slope*>(slopeList->getObjectAtIndex(i));
#else
			auto slope = dynamic_cast<Slope*>(slopeList->getObjectAtIndex(i));
#endif

			// 始点と接続しているかをチェック
			if (this->getConnectStartSlope() == nullptr) {

				// 坂の始点同士が接続している場合
				if ((this->start.x == slope->start.x) &&
					(this->start.y == slope->start.y)) {

					// 坂同士つながっているが移動できない坂となっている
					this->setConnectStartSlope(slope, false);
					slope->setConnectStartSlope(this, false);
				}
				else if ((this->start.x == slope->end.x) &&
					(this->start.y == slope->end.y)) {

					this->setConnectStartSlope(slope, true);
					slope->setConnectEndSlope(this, true);
				}
			}

			// 終点と接続しているかをチェック
			if (this->getConnectEndSlope() == nullptr) {
				// 坂の終点と確認している坂の始点が接続している場合
				if ((this->end.x == slope->start.x) &&
					(this->end.y == slope->start.y)) {

					this->setConnectEndSlope(slope, true);
					slope->setConnectStartSlope(this, true);
				}
				// 坂の終点同士が接続している場合
				else if ((this->end.x == slope->end.x) &&
					(this->end.y == slope->end.y)) {

					// 坂同士つながっているが移動できない坂となっている
					this->setConnectEndSlope(slope, false);
					slope->setConnectEndSlope(this, false);
				}
			}
		}
	}
}

bool Slope::checkConnectTileUp(Rect& tileRect, Rect& objectRect, Vec2& moveVec, bool bUpdateWorldSpaceRectFlag)
{
	// 事前にワールド空間での坂の矩形情報を更新
	if (bUpdateWorldSpaceRectFlag && _firstUpdateWorldSpaceRectFlag == false) {
		updateWorldSpaceRect();
	}

	// オブジェクトの中心が坂の下にある場合は処理しない
// #AGTK-NX
#if CC_TARGET_PLATFORM != CC_PLATFORM_NX
	if (!checkUpSide(Vec2(objectRect.getMidX(), objectRect.getMidY()))) {
#else
#endif
		return false;
	}

	// 上り坂の場合
	if (getType() == kTypeUp) {

		//// 右移動でない場合は処理しない
		//if (moveVec.x <= 0) {
		//	return false;
		//}

		// タイル左上と同じ位置に坂の終点がある場合のみ坂に上れるので
		// 接続しているとみなす
		if (tileRect.getMinX() == _worldEndPoint.x &&
			tileRect.getMaxY() == _worldEndPoint.y) {
			return true;
		}
	}
	// 下り坂の場合
	else {

		//// 左移動でない場合は処理しない
		//if (moveVec.x >= 0) {
		//	return false;
		//}

		// タイル右上と同じ位置に坂の始点がある場合のみ坂に上れるので
		// 接続しているとみなす
		if (tileRect.getMaxX() == _worldStartPoint.x &&
			tileRect.getMaxY() == _worldStartPoint.y) {
			return true;
		}
	}

	return false;
}

bool Slope::checkConnectTileUp(agtk::Tile *tile)
{
	auto wallBit = tile->getWallBit();
	if(!(wallBit & (1 << agtk::data::TilesetData::Up))) {
		return false;
	}
	auto tileRect = tile->convertToLayerSpaceRect();

	// 上り坂の場合
	if (getType() == kTypeUp) {
		if (tileRect.getMinX() == _worldEndPoint.x && tileRect.getMaxY() == _worldEndPoint.y) {
			return true;
		}
	}
	// 下り坂の場合
	else {
		if (tileRect.getMaxX() == _worldStartPoint.x && tileRect.getMaxY() == _worldStartPoint.y) {
			return true;
		}
	}

	return false;
}

bool Slope::checkConnectTileDown(agtk::Tile *tile)
{
	auto wallBit = tile->getWallBit();
	if (!(wallBit & (1 << agtk::data::TilesetData::Down))) {
		return false;
	}
	auto tileRect = tile->convertToLayerSpaceRect();

	// 上り坂の場合
	if (getType() == kTypeUp) {
		if (tileRect.getMaxX() == _worldStartPoint.x &&  tileRect.getMinY() == _worldStartPoint.y) {
			return true;
		}
	}
	// 下り坂の場合
	else {
		if (tileRect.getMinX() == _worldEndPoint.x &&  tileRect.getMinY() == _worldEndPoint.y) {
			return true;
		}
	}
	return false;
}

bool Slope::checkConnectTileLeft(agtk::Tile *tile)
{
	auto wallBit = tile->getWallBit();
	if (!(wallBit & (1 << agtk::data::TilesetData::Left))) {
		return false;
	}
	auto tileRect = tile->convertToLayerSpaceRect();
	// 上り坂の場合
	if (getType() == kTypeUp) {
		if (tileRect.getMinX() == _worldStartPoint.x &&  tileRect.getMaxY() == _worldStartPoint.y) {
			return true;
		}
	}
	// 下り坂の場合
	else {
		if (tileRect.getMinX() == _worldStartPoint.x &&  tileRect.getMinY() == _worldStartPoint.y) {
			return true;
		}
	}
	return false;
}

bool Slope::checkConnectTileRight(agtk::Tile *tile)
{
	auto wallBit = tile->getWallBit();
	if (!(wallBit & (1 << agtk::data::TilesetData::Right))) {
		return false;
	}
	auto tileRect = tile->convertToLayerSpaceRect();
	// 上り坂の場合
	if (getType() == kTypeUp) {
		if (tileRect.getMaxX() == _worldEndPoint.x &&  tileRect.getMinY() == _worldEndPoint.y) {
			return true;
		}
	}
	// 下り坂の場合
	else {
		if (tileRect.getMaxX() == _worldEndPoint.x &&  tileRect.getMaxY() == _worldEndPoint.y) {
			return true;
		}
	}
	return false;
}

// ACT2-5461
// 「オブジェクトを吸着」のチェックがないとき向けに、吸着されないよう、moveVecを調整する。
void Slope::adjustMoveVecForNotMovingObjectAlongSlope(const Vec2 &point, const Vec2 &iniMoveVec, Vec2 &moveVec)
{
	//cocos2d::log("Slope::adjustMoveVecForNotMovingObjectAlongSlope: %f, %f  %f, %f  %f, %f", point.x, point.y, iniMoveVec.x, iniMoveVec.y, moveVec.x, moveVec.y);
	if (_firstUpdateWorldSpaceRectFlag == false) {
		updateWorldSpaceRect();
	}
	CollisionLine slopeLine = { &_worldStartPoint, &_worldEndPoint };
	auto slopeVec = (_worldEndPoint - _worldStartPoint).getNormalized();
	auto vec = (point - _worldStartPoint);
	auto norm = slopeVec * slopeVec.dot(vec);
	auto tanVec = cocos2d::Vec2(vec.x - norm.x, vec.y - norm.y);
	auto sign = tanVec.dot(iniMoveVec);
	//cocos2d::log("sign: %f", sign);
	if (sign <= 0) {
		//坂に近づこうとしているので調整不要。
		return;
	}
	if (iniMoveVec == cocos2d::Vec2::ZERO) {
		//cocos2d::log("iniMoveVec is ZERO!");
		moveVec = cocos2d::Vec2::ZERO;
		return;
	}
	auto len = iniMoveVec.getNormalized().dot(moveVec);
	if (len < 0) {
		//cocos2d::log("ZERO!");
		moveVec = cocos2d::Vec2::ZERO;
		return;
	}
	auto newVec = iniMoveVec.getNormalized() * len;
	//cocos2d::log("len: %f, newVec: %f, %f", len, newVec.x, newVec.y);
	moveVec = newVec;
}

bool Slope::calcDistToUpper(float& dist, Rect& rect)
{
	// 事前にupdateWorldSpaceRect関数を呼び出し
	// ワールド空間での坂の情報を更新しておく必要がある

	bool result = false;
	Vec2 cross;
	cocos2d::Point sp;
	cocos2d::Point ep;
	CollisionLine line = { &sp ,&ep };

	// 上り坂の場合
	if (getType() == kTypeUp) {
		// 矩形右側の接触位置を検索
		sp = Point(rect.getMaxX(), _worldStartPoint.y);
		ep = Point(rect.getMaxX(), _worldEndPoint.y);
	}
	// 下り坂の場合
	else {
		// 矩形左側の接触位置を検索
		sp = Point(rect.getMinX(), _worldStartPoint.y);
		ep = Point(rect.getMinX(), _worldEndPoint.y);
	}

	if (checkHitLine(&line, cross)) {
		dist = rect.getMinY() - cross.y;

		// 矩形が坂に埋まっていない場合のみ距離の算出ができたものとする
		if (dist > 0) {
			result = true;
		}
	}

	return result;
}

bool Slope::calcNearestDistancePoint(Vec2 p, Vec2& pp, float *distance)
{
	auto a = _worldStartPoint;
	auto b = _worldEndPoint;
	cocos2d::Vec2 ab = b - a;
	cocos2d::Vec2 ap = p - a;
	auto n = ab.getNormalized();
	float d = n.dot(ap);
	auto _pp = a + (n * d);
	auto nn = (pp - p).getNormalized();
	auto dd = (pp - p).getLength() + 1.0f;
	auto _ppp = p + nn * dd;
	if (cocos2d::Vec2::isSegmentIntersect(a, b, p, _ppp)) {
		if (distance) {
			*distance = _pp.getDistance(p);
		}
		pp = _pp;
		return true;
	}
	return false;
}

void Slope::setConnectStartSlope(Slope* slope, bool canMove) 
{
	_connectStartSlope = slope;
	_canMoveStartSlope = canMove;
}

void Slope::setConnectEndSlope(Slope* slope, bool canMove) 
{
	_connectEndSlope = slope;
	_canMoveEndSlope = canMove;
}

void Slope::updateWorldSpaceRect()
{
	// ワールド空間での坂の矩形情報を更新
	convertToLayerSpaceRect();

	if (getType() == kTypeUp) {
		_worldStartPoint = Point(_worldSpaceRect.getMinX(), _worldSpaceRect.getMinY());
		_worldEndPoint = Point(_worldSpaceRect.getMaxX(), _worldSpaceRect.getMaxY());
	}
	else {
		_worldStartPoint = Point(_worldSpaceRect.getMinX(), _worldSpaceRect.getMaxY());
		_worldEndPoint = Point(_worldSpaceRect.getMaxX(), _worldSpaceRect.getMinY());
	}

	_firstUpdateWorldSpaceRectFlag = true;

	if (_connectStartSlope && !_connectStartSlope->_firstUpdateWorldSpaceRectFlag) {
		_connectStartSlope->updateWorldSpaceRect();
	}
	if (_connectEndSlope && !_connectEndSlope->_firstUpdateWorldSpaceRectFlag) {
		_connectEndSlope->updateWorldSpaceRect();
	}
}

void Slope::convertToWorldSpaceVertex4(agtk::Vertex4 &vertex4)
{
	auto rect = agtk::Scene::getRectSceneFromCocos2d(this->getRect());

	// Tile.cppのconvertToWorldSpaceVertex4関数を参考に作成
	cocos2d::Mat4 m = cocos2d::Mat4::IDENTITY;
	auto parent = this->getParent();
	if (parent) {
		m = parent->getNodeToParentTransform();
	}

	vertex4[0] = PointApplyTransform(rect.origin + cocos2d::Vec2(0, rect.size.height), m);//左上
	vertex4[1] = PointApplyTransform(rect.origin + cocos2d::Vec2(rect.size.width, rect.size.height), m);//右上
	vertex4[2] = PointApplyTransform(rect.origin + cocos2d::Vec2(rect.size.width, 0), m);//右下
	vertex4[3] = PointApplyTransform(rect.origin + cocos2d::Vec2(0, 0), m);//左下
}

cocos2d::Rect Slope::convertToLayerSpaceRect()
{
	agtk::Vertex4 vertex4;
	this->convertToWorldSpaceVertex4(vertex4);

	_worldSpaceRect = vertex4.getRect();

	return _worldSpaceRect;
}

#ifdef USE_PREVIEW

void Slope::showDebugVisible(bool isShow)
{
	auto debugView = this->getChildByName("debugVisible");

	if (!debugView && isShow) {
		DrawNode *line = DrawNode::create();

		auto spos = agtk::Scene::getPositionCocos2dFromScene(start);
		auto epos = agtk::Scene::getPositionCocos2dFromScene(end);

		//line->drawSegment(spos, epos, 1.0f, _type == Type::kTypeUp ? Color4F::RED : Color4F::BLUE);
		line->drawSegment(spos, epos, 1.0f, Color4F::RED);
		this->addChild(line, 1, "debugVisible");

	}
	else if (debugView) {
		debugView->setVisible(isShow);
	}
}
#endif

NS_AGTK_END
