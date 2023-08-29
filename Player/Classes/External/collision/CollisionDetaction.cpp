/**
 ForTreeCollision v0.0.1
 CollisionDetection.cpp

 cocos2d-x 3.x用の4分木衝突判定クラス。
 以下公開されているソースコードをcocos2d-x v3用に修正＆改造したものです。
 https://github.com/gear1554/CollisionDetectionTest

 Copyright (c) 2016 Kazuki Oda

 This software is released under the MIT License.
 http://opensource.org/licenses/mit-license.php
*/

#include "Lib/Macros.h"
#include "CollisionComponent.hpp"
#include "CollisionDetaction.hpp"
#include "CollisionUtils.hpp"
#include "collision_point.h"
#include "Lib/Object.h"
#include "Lib/Tile.h"
#include "Lib/Scene.h"
#include "Lib/Portal.h"

#ifdef USE_MULTITHREAD_OBJECT_UPDATE
void CollisionDetaction::mtxLock()
{
#ifdef USE_MULTITHREAD_MEASURE
	if (!m_mtx.try_lock()) {
		m_mtx.lock();
		m_stateChangeCounter++;
	}
#else
	m_mtx.lock();
#endif
}
void CollisionDetaction::mtxUnlock()
{
	m_mtx.unlock();
}
#endif
USING_NS_CC;
static CollisionDetaction *s_instance = nullptr;

CollisionDetaction::CollisionDetaction()
    : m_field(nullptr)
    , m_unitSize()
    , m_dwCellNum(0)
    , m_uiLevel(0)
    , m_spaceArray(nullptr)
    , m_gameObjectArray(nullptr) {
    int i;
    m_iPow[0] = 1;
    for (i = 1; i < CLINER4TREEMANAGER_MAXLEVEL + 1; i++) {
        m_iPow[i] = m_iPow[i - 1] * 4;
    }
};

CollisionDetaction::~CollisionDetaction() {
    reset();
}

CollisionDetaction *CollisionDetaction::create() {
    auto ref = new CollisionDetaction();
    ref->autorelease();
    return ref;
}

#ifdef USE_MULTITHREAD_OBJECT_UPDATE
void CollisionDetaction::addLocal(CollisionNode *node) {
	if (m_gameObjectArray == nullptr) {
		return;
	}
#if defined(AGTK_DEBUG)
	if (m_gameObjectArray->containsObject(node)) {
		CC_ASSERT(0);
	}
#endif
	m_gameObjectArray->addObject(node);
}
void CollisionDetaction::add(CollisionNode *node) {
	mtxLock();
#ifdef USE_AGTK_MULTITHREAD_TEST
	Ref::setMtxLocked(true);
#endif
	addLocal(node);
#ifdef USE_AGTK_MULTITHREAD_TEST
	Ref::setMtxLocked(false);
#endif
	mtxUnlock();
}
#else
void CollisionDetaction::add(CollisionNode *node) {
    if (m_gameObjectArray == nullptr) {
        return;
    }
#if defined(AGTK_DEBUG)
	if (m_gameObjectArray->containsObject(node)) {
		CC_ASSERT(0);
	}
#endif
    m_gameObjectArray->addObject(node);
}
#endif

#ifdef USE_MULTITHREAD_OBJECT_UPDATE
void CollisionDetaction::remove(cocos2d::Node *node) {
	mtxLock();
#ifdef USE_AGTK_MULTITHREAD_TEST
	Ref::setMtxLocked(true);
#endif
	if (m_gameObjectArray != nullptr) {
		m_gameObjectArray->setnull(node);
	}
#ifdef USE_AGTK_MULTITHREAD_TEST
	Ref::setMtxLocked(false);
#endif
	mtxUnlock();
}
#else
void CollisionDetaction::remove(cocos2d::Node *node) {
    if (m_gameObjectArray == nullptr) {
        return;
    }
    m_gameObjectArray->erase(node);
}
#endif
#ifdef USE_MULTITHREAD_OBJECT_UPDATE
void CollisionDetaction::remove(CollisionNode *node) {
	mtxLock();
#ifdef USE_AGTK_MULTITHREAD_TEST
	Ref::setMtxLocked(true);
#endif
	if (m_gameObjectArray != nullptr) {
		m_gameObjectArray->setnull(node);
	}
#ifdef USE_AGTK_MULTITHREAD_TEST
	Ref::setMtxLocked(false);
#endif
	mtxUnlock();
}
#else
void CollisionDetaction::remove(CollisionNode *node) {
    if (m_gameObjectArray == nullptr) {
        return;
    }
    m_gameObjectArray->erase(node);
}
#endif

#ifdef USE_MULTITHREAD_OBJECT_UPDATE
void CollisionDetaction::resetLocal() {
	if (m_field && m_onexit) {
		m_field->setonExitTransitionDidStartCallback(m_onexit);
	}
	CC_SAFE_DELETE(m_gameObjectArray);
	CC_SAFE_RELEASE_NULL(m_spaceArray);
	m_onexit = nullptr;
	m_func = nullptr;
//    CCLOG("reset");
}
void CollisionDetaction::reset() {
	mtxLock();
#ifdef USE_AGTK_MULTITHREAD_TEST
	Ref::setMtxLocked(true);
#endif
	resetLocal();
#ifdef USE_AGTK_MULTITHREAD_TEST
	Ref::setMtxLocked(false);
#endif
	mtxUnlock();
}
#else
void CollisionDetaction::reset() {
    if (m_field) {
        m_field->setonExitTransitionDidStartCallback(m_onexit);
    }
    CC_SAFE_RELEASE_NULL(m_gameObjectArray);
    CC_SAFE_RELEASE_NULL(m_spaceArray);
    m_onexit = nullptr;
    m_func = nullptr;
//    CCLOG("reset");
}
#endif

bool CollisionDetaction::init(cocos2d::Node *field, int level, bool autoclean,
                              const DetectCollisionFunc &func) {
    CCASSERT(level < CLINER4TREEMANAGER_MAXLEVEL,
             "レベルがMAXLEVELを超えています。 ");
#ifdef USE_MULTITHREAD_OBJECT_UPDATE
	mtxLock();
#ifdef USE_AGTK_MULTITHREAD_TEST
	Ref::setMtxLocked(true);
#endif
	resetLocal();
#else
    reset();
#endif

    m_func = func;

#ifdef USE_MULTITHREAD_OBJECT_UPDATE
	m_gameObjectArray = new CollisionNodeList();
#else
    m_gameObjectArray = CollisionNodeList::create();
    m_gameObjectArray->retain();
#endif

    m_spaceArray = CollisionNodeMap::create();
    m_spaceArray->retain();

    m_field = field;

    if (autoclean) {
        m_onexit = m_field->getonExitTransitionDidStartCallback();
        auto callback = [this]() {
            if (m_onexit) {
                m_onexit();
            }
#ifdef USE_MULTITHREAD_OBJECT_UPDATE
			this->resetLocal();
#else
            this->reset();
#endif
        };
        m_field->setonExitTransitionDidStartCallback(callback);
    }

    Size winSize = field->getContentSize();
    m_dwCellNum = (m_iPow[level + 1] - 1) / 3;
    m_uiLevel = level;
    m_unitSize = Size((float)(winSize.width / (1 << level)),
                      (float)(winSize.height / (1 << level)));
    for (int i = 0; i < m_dwCellNum; i++) {
#ifdef USE_MULTITHREAD_OBJECT_UPDATE
		auto spaceCellArray = new CollisionNodeList();
#else
        auto spaceCellArray = CollisionNodeList::create();
#endif
        m_spaceArray->addObject(spaceCellArray);
    }

#ifdef USE_AGTK_MULTITHREAD_TEST
	Ref::setMtxLocked(false);
#endif
#ifdef USE_MULTITHREAD_OBJECT_UPDATE
	mtxUnlock();
#endif
    return true;
}

// Update Collision detection
#ifdef USE_MULTITHREAD_OBJECT_UPDATE
void CollisionDetaction::updateLocal() {
	if (m_spaceArray == nullptr) {
		return;
	}
	// Refresh space status
	m_spaceArray->refresh();

	if (!m_gameObjectArray || m_gameObjectArray->count() < 2) {
		return;
	}

	// Update game objects's space status
	for (int i = 0; i < (int)m_gameObjectArray->count(); i++) {
		auto obj = m_gameObjectArray->objectAtIndex(i);
		if (obj == nullptr) continue;
		updateSpaceStatusLocal(obj);
	}

	// Scan collision detection
	auto ary = new CollisionNodeList();
	scanCollisionDetectionLocal(0, ary);
	delete ary;
}
void CollisionDetaction::update() {
	mtxLock();
#ifdef USE_AGTK_MULTITHREAD_TEST
	Ref::setMtxLocked(true);
#endif
	updateLocal();
#ifdef USE_AGTK_MULTITHREAD_TEST
	Ref::setMtxLocked(false);
#endif
	mtxUnlock();
}
#else
void CollisionDetaction::update() {
    if (m_spaceArray == nullptr) {
        return;
    }
    // Refresh space status
    m_spaceArray->refresh();

    if (!m_gameObjectArray || m_gameObjectArray->count() < 2) {
        return;
    }

    // Update game objects's space status
    for (auto node : *m_gameObjectArray) {
        updateSpaceStatus(node);
    }

    // Scan collision detection
    auto ary = CollisionNodeList::create();
    scanCollisionDetection(0, ary);
}
#endif

//update()のうち、updateSpaceStatus(node)までを実行する。
#ifdef USE_MULTITHREAD_OBJECT_UPDATE
void CollisionDetaction::updateSpaceStatusLocal()
{
	if (m_spaceArray == nullptr) {
		return;
	}
	// Refresh space status
	m_spaceArray->refresh();

	if (!m_gameObjectArray || m_gameObjectArray->count() < 1) {
		return;
	}

	// Update game objects's space status
	for (int i = 0; i < (int)m_gameObjectArray->count(); i++) {
		auto obj = m_gameObjectArray->objectAtIndex(i);
		if (obj == nullptr) continue;
		updateSpaceStatusLocal(obj);
	}
}
void CollisionDetaction::updateSpaceStatus()
{
	mtxLock();
#ifdef USE_AGTK_MULTITHREAD_TEST
	Ref::setMtxLocked(true);
#endif
	updateSpaceStatusLocal();
#ifdef USE_AGTK_MULTITHREAD_TEST
	Ref::setMtxLocked(false);
#endif
	mtxUnlock();
}
#else
void CollisionDetaction::updateSpaceStatus()
{
	if (m_spaceArray == nullptr) {
		return;
	}
	// Refresh space status
	m_spaceArray->refresh();

	if (!m_gameObjectArray || m_gameObjectArray->count() < 1) {
		return;
	}

	// Update game objects's space status
	for (auto node : *m_gameObjectArray) {
		updateSpaceStatus(node);
	}
}
#endif

#ifdef USE_MULTITHREAD_OBJECT_UPDATE
void CollisionDetaction::initForSingleLocal()
{
	if (m_spaceArray == nullptr) {
		return;
	}
	// Refresh space status
	m_spaceArray->refresh();

	if (!m_gameObjectArray || m_gameObjectArray->count() < 2) {
		return;
	}

	// Update game objects's space status
	for (int i = 0; i < (int)m_gameObjectArray->count(); i++) {
		auto obj = m_gameObjectArray->objectAtIndex(i);
		if (obj == nullptr) continue;
		updateSpaceStatusLocal(obj);
	}
}
void CollisionDetaction::initForSingle()
{
	mtxLock();
#ifdef USE_AGTK_MULTITHREAD_TEST
	Ref::setMtxLocked(true);
#endif
	initForSingleLocal();
#ifdef USE_AGTK_MULTITHREAD_TEST
	Ref::setMtxLocked(false);
#endif
	mtxUnlock();
}
#else
void CollisionDetaction::initForSingle()
{
	if (m_spaceArray == nullptr) {
		return;
	}
	// Refresh space status
	m_spaceArray->refresh();

	if (!m_gameObjectArray || m_gameObjectArray->count() < 2) {
		return;
	}

	// Update game objects's space status
	for (auto node : *m_gameObjectArray) {
		updateSpaceStatus(node);
	}
}
#endif

#ifdef USE_MULTITHREAD_OBJECT_UPDATE
void CollisionDetaction::updateSingleLocal(CollisionNode *collisionObject)
{
	auto level = collisionObject->getLevel();
	if (level < 0 || level >= m_dwCellNum) {
	}
	else {
		auto spaceCellArray = m_spaceArray->get(level);
		if (spaceCellArray->containsObject(collisionObject)) {
			spaceCellArray->setnull(collisionObject);
		}
	}

	updateSpaceStatusLocal(collisionObject);

	// 子空間の当たりチェック
	level = collisionObject->getLevel();
	scanCollisionDetectionSingleLocal(level, collisionObject, true);

	int l = 0;
	for (int i = 0; i < CLINER4TREEMANAGER_MAXLEVEL; i++) {
		int num1 = (m_iPow[i] - 1) / 3;
		int num2 = (m_iPow[i + 1] - 1) / 3;

		if (num1 <= level && level < num2) {
			l = i;
			break;
		}
	}


	// 親空間のチェック
	while (l > 0) {
		int num1 = (m_iPow[l - 1] - 1) / 3;
		int num2 = (m_iPow[l] - 1) / 3;
		level = (level - num2) / 4 + num1;

		scanCollisionDetectionSingleLocal(level, collisionObject, false);

		l--;
	}
}
void CollisionDetaction::updateSingle(CollisionNode *collisionObject)
{
	mtxLock();
#ifdef USE_AGTK_MULTITHREAD_TEST
	Ref::setMtxLocked(true);
#endif
	updateSingleLocal(collisionObject);
#ifdef USE_AGTK_MULTITHREAD_TEST
	Ref::setMtxLocked(false);
#endif
	mtxUnlock();
}
#else
void CollisionDetaction::updateSingle(CollisionNode *collisionObject)
{
	auto level = collisionObject->getLevel();
	if (level < 0 || level >= m_dwCellNum) {
	} else {
		auto spaceCellArray = m_spaceArray->get(level);
		if (spaceCellArray->containsObject(collisionObject)) {
			spaceCellArray->erase(collisionObject);
		}
	}

	updateSpaceStatus(collisionObject);

	// 子空間の当たりチェック
	level = collisionObject->getLevel();
	scanCollisionDetectionSingle(level, collisionObject, true);

	int l = 0;
	for (int i = 0; i < CLINER4TREEMANAGER_MAXLEVEL; i++) {
		int num1 = (m_iPow[i] - 1) / 3;
		int num2 = (m_iPow[i + 1] - 1) / 3;

		if (num1 <= level && level < num2) {
			l = i;
			break;
		}
	}


	// 親空間のチェック
	while (l > 0) {
		int num1 = (m_iPow[l - 1] - 1) / 3;
		int num2 = (m_iPow[l] - 1) / 3;
		level = (level - num2) / 4 + num1;

		scanCollisionDetectionSingle(level, collisionObject, false);

		l--;
	}
}
#endif

// #AGTK-NX
#ifdef USE_SAR_OPTIMIZE_5
// Update Collision detection
#ifdef USE_MULTITHREAD_OBJECT_UPDATE
void CollisionDetaction::removeSingleLocal(CollisionNode *collisionObject) {
	auto lLevel = collisionObject->getLevel();
	if (lLevel < 0 || lLevel >= m_dwCellNum) {
	}
	else {
		if (m_spaceArray)
		{
			auto spaceCellArray = m_spaceArray->get(lLevel);
			spaceCellArray->setnull(collisionObject);
		}
	}
}
void CollisionDetaction::removeSingle(CollisionNode *collisionObject)
{
	mtxLock();
#ifdef USE_AGTK_MULTITHREAD_TEST
	Ref::setMtxLocked(true);
#endif
	removeSingleLocal(collisionObject);
#ifdef USE_AGTK_MULTITHREAD_TEST
	Ref::setMtxLocked(false);
#endif
	mtxUnlock();
}
#else
void CollisionDetaction::removeSingle(CollisionNode *collisionObject)
{
	auto lLevel = collisionObject->getLevel();
	if (lLevel < 0 || lLevel >= m_dwCellNum) {
	}
	else {
		auto spaceCellArray = m_spaceArray->get(lLevel);
		if (spaceCellArray->containsObject(collisionObject)) {
			spaceCellArray->setnull(collisionObject);
		}
	}
}
#endif
#endif

#ifdef USE_MULTITHREAD_OBJECT_UPDATE
void CollisionDetaction::scanCollisionDetectionSingleLocal(int spaceIndex, CollisionNode *collisionObject, bool checkChild)
{
	if (m_spaceArray == nullptr || !m_spaceArray->count()) {
		return;
	}

	auto spaceCellArray = m_spaceArray->get(spaceIndex);
	if (spaceCellArray == nullptr) {
		return;
	}
	for (int i = 0; i < (int)spaceCellArray->count(); i++) {
		auto obj = spaceCellArray->objectAtIndex(i);
		if (obj == nullptr) continue;
		if (obj == collisionObject) continue;
		checkHitLocal(obj, collisionObject);
	}

	// 子空間のチェックを行う場合
	if (checkChild) {
		int nextSpaceIndex;
		for (int i = 0; i < 4; i++) {
			nextSpaceIndex = spaceIndex * 4 + 1 + i;
			// 子空間があるかどうか
			if (nextSpaceIndex >= 0 && nextSpaceIndex < m_dwCellNum) {
				scanCollisionDetectionSingleLocal(nextSpaceIndex, collisionObject, true);
			}
		}
	}
	return;
}
void CollisionDetaction::scanCollisionDetectionSingle(int spaceIndex, CollisionNode *collisionObject, bool checkChild)
{
	mtxLock();
#ifdef USE_AGTK_MULTITHREAD_TEST
	Ref::setMtxLocked(true);
#endif
	scanCollisionDetectionSingleLocal(spaceIndex, collisionObject, checkChild);
#ifdef USE_AGTK_MULTITHREAD_TEST
	Ref::setMtxLocked(false);
#endif
	mtxUnlock();
}
#else
void CollisionDetaction::scanCollisionDetectionSingle(int spaceIndex, CollisionNode *collisionObject, bool checkChild)
{
	if (m_spaceArray == nullptr || !m_spaceArray->count()) {
		return;
	}

	auto spaceCellArray = m_spaceArray->get(spaceIndex);
	if (spaceCellArray == nullptr) {
		return;
	}
	for (auto obj : *spaceCellArray) {
		if (obj == collisionObject) continue;
		checkHit(obj, collisionObject);
	}

	// 子空間のチェックを行う場合
	if (checkChild) {
		int nextSpaceIndex;
		for (int i = 0; i < 4; i++) {
			nextSpaceIndex = spaceIndex * 4 + 1 + i;
			// 子空間があるかどうか
			if (nextSpaceIndex >= 0 && nextSpaceIndex < m_dwCellNum) {
				scanCollisionDetectionSingle(nextSpaceIndex, collisionObject, true);
			}
		}
	}
	return;
}
#endif

#ifdef USE_MULTITHREAD_OBJECT_UPDATE
void CollisionDetaction::updateSingleWithoutScan(CollisionNode *collisionObject)
{
	auto lLevel = collisionObject->getLevel();
	auto level = calcLevelLocal(collisionObject);

	if (level != lLevel) {
		mtxLock();
#ifdef USE_AGTK_MULTITHREAD_TEST
		Ref::setMtxLocked(true);
#endif
		if (lLevel < 0 || lLevel >= m_dwCellNum) {
		}
		else {
			auto spaceCellArray = m_spaceArray->get(lLevel);
			if (spaceCellArray->containsObject(collisionObject)) {
				spaceCellArray->setnull(collisionObject);
			}
		}
		collisionObject->setLevel(level);

		auto spaceCellArray = m_spaceArray->get(level);
		if (!spaceCellArray->containsObject(collisionObject)) {
			spaceCellArray->addObject(collisionObject);
		}
#ifdef USE_AGTK_MULTITHREAD_TEST
		Ref::setMtxLocked(false);
#endif
		mtxUnlock();
	}
}
#else
void CollisionDetaction::updateSingleWithoutScan(CollisionNode *collisionObject)
{
	auto level = collisionObject->getLevel();
	if (level < 0 || level >= m_dwCellNum) {
	}
	else {
		auto spaceCellArray = m_spaceArray->get(level);
		if (spaceCellArray->containsObject(collisionObject)) {
			spaceCellArray->erase(collisionObject);
		}
	}

	updateSpaceStatus(collisionObject);
}
#endif

#ifdef USE_MULTITHREAD_OBJECT_UPDATE
void CollisionDetaction::scanSingleLocal(CollisionNode *collisionObject)
{
	auto level = collisionObject->getLevel();
	if (level < 0 || level >= m_dwCellNum) {
	}
	else {
		/*auto spaceCellArray = m_spaceArray->get(level);
		if (spaceCellArray->containsObject(collisionObject)) {
		spaceCellArray->erase(collisionObject);
		}*/
	}

	//updateSpaceStatusLocal(collisionObject);

	// 子空間の当たりチェック
	scanCollisionDetectionSingleLocal(level, collisionObject, true);

	int l = 0;
	for (int i = 0; i < CLINER4TREEMANAGER_MAXLEVEL; i++) {
		int num1 = (m_iPow[i] - 1) / 3;
		int num2 = (m_iPow[i + 1] - 1) / 3;

		if (num1 <= level && level < num2) {
			l = i;
			break;
		}
	}


	// 親空間のチェック
	while (l > 0) {
		int num1 = (m_iPow[l - 1] - 1) / 3;
		int num2 = (m_iPow[l] - 1) / 3;
		level = (level - num2) / 4 + num1;

		scanCollisionDetectionSingleLocal(level, collisionObject, false);

		l--;
	}
}
void CollisionDetaction::scanSingle(CollisionNode *collisionObject)
{
	//mtxLock();
//#ifdef USE_AGTK_MULTITHREAD_TEST
	//Ref::setMtxLocked(true);
//#endif
	scanSingleLocal(collisionObject);
//#ifdef USE_AGTK_MULTITHREAD_TEST
	//Ref::setMtxLocked(false);
//#endif
	//mtxUnlock();
}
#else
void CollisionDetaction::scanSingle(CollisionNode *collisionObject)
{
	auto level = collisionObject->getLevel();
	if (level < 0 || level >= m_dwCellNum) {
	}
	else {
		/*auto spaceCellArray = m_spaceArray->get(level);
		if (spaceCellArray->containsObject(collisionObject)) {
			spaceCellArray->erase(collisionObject);
		}*/
	}

	//updateSpaceStatus(collisionObject);

	// 子空間の当たりチェック
	scanCollisionDetectionSingle(level, collisionObject, true);

	int l = 0;
	for (int i = 0; i < CLINER4TREEMANAGER_MAXLEVEL; i++) {
		int num1 = (m_iPow[i] - 1) / 3;
		int num2 = (m_iPow[i + 1] - 1) / 3;

		if (num1 <= level && level < num2) {
			l = i;
			break;
		}
	}


	// 親空間のチェック
	while (l > 0) {
		int num1 = (m_iPow[l - 1] - 1) / 3;
		int num2 = (m_iPow[l] - 1) / 3;
		level = (level - num2) / 4 + num1;

		scanCollisionDetectionSingle(level, collisionObject, false);

		l--;
	}
}
#endif

#ifdef USE_MULTITHREAD_OBJECT_UPDATE
int CollisionDetaction::calcLevelLocal(CollisionNode *collisionObject)
{
	// レベルを計算(オーバーヘッドを抑えるためあえて外出ししないでいる)
	int level = -1;
	Point pos;
	Size size;

	bool empty = true;
	auto node = collisionObject->getNode();
	auto object = dynamic_cast<NS_AGTK::Object *>(node);
	auto group = collisionObject->getGroup();
	if (group == CollisionComponent::kGroupWall)
	{
		empty = false;
		if (object) {
			std::vector<agtk::Vertex4> vertList;
// #AGTK-NX #AGTK-WIN
#ifdef USE_30FPS_4
			Rect rect;
			if (object->_bUseMiddleFrame) {
				auto mf = object->_middleFrameStock.getUpdatedMiddleFrame();
				Vec2 diff = object->getPosition() - mf->_objectPos;
				rect = agtk::Vertex4::getRectMerge(mf->_wallList);
				rect.origin += Vec2(diff.x, -diff.y);
			}
			else {
				object->getTimelineList(agtk::data::TimelineInfoData::kTimelineWall, vertList);
				rect = agtk::Vertex4::getRectMerge(vertList);
			}
#else
			object->getTimelineList(agtk::data::TimelineInfoData::kTimelineWall, vertList);
			auto rect = agtk::Vertex4::getRectMerge(vertList);
#endif
			pos = rect.origin;
			size = rect.size;

			// 移動ベクトルを取得
			Vec2 moveVec = object->getPosition() - object->getOldPosition();

			// 移動が発生している場合
			if (!(moveVec.x == 0 && moveVec.y == 0)) {
				// 前フレームの位置から現在の位置までの範囲で設定する
				Vec2 prevPos = Vec2(pos.x - moveVec.x, pos.y + moveVec.y);

				Rect crntRect = Rect(pos, size);
				Rect prevRect = Rect(prevPos, size);
				crntRect.merge(prevRect);

				pos.x = crntRect.getMidX();
				pos.y = crntRect.getMidY();

				size = crntRect.size;
			}
			else {
				// 取得した座標は左下の座標なので中心になるように変換
				pos.x += size.width * 0.5f;
				pos.y += size.height * 0.5f;
			}
		}
	}
	else if (group == CollisionComponent::kGroupRoughWall) {
		if (object->getPlayer()) {
			if (object->getPlayer()->getWallVertListCache().size() > 0) {
				empty = false;
				auto rect = object->getPlayer()->getWallVertRectCache();
				pos = rect.origin;
				size = rect.size;

				// 取得した座標は左下の座標なので中心になるように変換
				pos.x += size.width * 0.5f;
				pos.y += size.height * 0.5f
					;
			}
		}
	}
	else if (group == CollisionComponent::kGroupHit) {
		if (object->getPlayer()) {
			if (object->getPlayer()->getHitVertListCache().size() > 0) {
				auto rect = object->getPlayer()->getHitVertRectCache();
				pos = rect.origin;
				size = rect.size;

				// 取得した座標は左下の座標なので中心になるように変換
				pos.x += size.width * 0.5f;
				pos.y += size.height * 0.5f;
				empty = false;
			}
		}
	}
	else if (group == CollisionComponent::kGroupAttack) {
		if (object->getPlayer()) {
			if (object->getPlayer()->getAttackVertListCache().size() > 0) {
				auto rect = object->getPlayer()->getAttackVertRectCache();
				pos = rect.origin;
				size = rect.size;

				// 取得した座標は左下の座標なので中心になるように変換
				pos.x += size.width * 0.5f;
				pos.y += size.height * 0.5f;
				empty = false;
			}
		}
	}
	else if (group == CollisionComponent::kGroupPortal) {
		auto portal = dynamic_cast<agtk::Portal *>(node);
		if (portal) {
			pos = portal->getPosition();
#ifdef FIX_ACT2_4774
			size = portal->getContentSize() * 3;
			// 取得した座標は左下の座標なので中心になるように変換
			pos.x += size.width / (2 * 3);
			pos.y += size.height / (2 * 3);
#else
			size = portal->getContentSize();
			// 取得した座標は左下の座標なので中心になるように変換
			pos.x += size.width * 0.5f;
			pos.y += size.height * 0.5f;
#endif
			empty = false;
		}
	}
	else if (group == CollisionComponent::kGroupObjectCenter) {
		if (object) {
			pos = agtk::Scene::getPositionCocos2dFromScene(object->getPosition());
			size = cocos2d::Size(0, 0);
			empty = false;
		}
	}

	float left = pos.x - size.width;
	float top = pos.y + size.height;
	float right = pos.x + size.width;
	float bottom = pos.y - size.height;
	float width = m_unitSize.width;
	float height = m_unitSize.height;

	//当たり領域が存在しないときは、できるだけヒットに引っかからないように、一番小さな領域にセットする。
	if (empty) {
		level = m_dwCellNum - 1;
	}
	else
		if (left < 0 || bottom < 0 || m_field->getContentSize().width <= right || m_field->getContentSize().height <= top) {
			// いずれかの頂点がフィールド外へ出てしまう場合はlevelを0とし
			// 壁当たりチェックが行えるようにする
			level = 0;
		}
		else {
			int LT = collision_get_point_elem(left, top, width, height);
			int RB = collision_get_point_elem(right, bottom, width, height);

			int Def = RB ^ LT;
			unsigned int HiLevel = 0;
			for (int i = 0; i < m_uiLevel; i++) {
				int Check = (Def >> (i * 2)) & 0x3;
				if (Check != 0) HiLevel = i + 1;
			}
			level = RB >> (HiLevel * 2);
			int AddNum = (m_iPow[m_uiLevel - HiLevel] - 1) / 3;
			level += AddNum;
			// レベル計算(ここまで)
		}


		// 画面外へ出てしまった場合はlevelを0とし
		// 壁当たりチェックが行えるようにする
		if (level < 0 || level >= m_dwCellNum) {
			level = 0;
		}
		return level;
}
int CollisionDetaction::calcLevel(CollisionNode *collisionObject)
{
	mtxLock();
#ifdef USE_AGTK_MULTITHREAD_TEST
	Ref::setMtxLocked(true);
#endif
	auto ret = calcLevelLocal(collisionObject);
#ifdef USE_AGTK_MULTITHREAD_TEST
	Ref::setMtxLocked(false);
#endif
	mtxUnlock();
	return ret;
}
#else
int CollisionDetaction::calcLevel(CollisionNode *collisionObject)
{
	// レベルを計算(オーバーヘッドを抑えるためあえて外出ししないでいる)
	int level = -1;
	Point pos;
	Size size;

	bool empty = true;
	auto node = collisionObject->getNode();
	auto object = dynamic_cast<NS_AGTK::Object *>(node);
	auto group = collisionObject->getGroup();
	if (group == CollisionComponent::kGroupWall)
	{
		empty = false;
		if (object) {
			std::vector<agtk::Vertex4> vertList;
			object->getTimelineList(agtk::data::TimelineInfoData::kTimelineWall, vertList);
			auto rect = agtk::Vertex4::getRectMerge(vertList);
			pos = rect.origin;
			size = rect.size;

			// 移動ベクトルを取得
			Vec2 moveVec = object->getPosition() - object->getOldPosition();

			// 移動が発生している場合
			if (!(moveVec.x == 0 && moveVec.y == 0)) {
				// 前フレームの位置から現在の位置までの範囲で設定する
				Vec2 prevPos = Vec2(pos.x - moveVec.x, pos.y + moveVec.y);

				Rect crntRect = Rect(pos, size);
				Rect prevRect = Rect(prevPos, size);
				crntRect.merge(prevRect);

				pos.x = crntRect.getMidX();
				pos.y = crntRect.getMidY();

				size = crntRect.size;
			}
			else {
				// 取得した座標は左下の座標なので中心になるように変換
				pos.x += size.width * 0.5f;
				pos.y += size.height * 0.5f;
			}
		}
	}
	else if (group == CollisionComponent::kGroupRoughWall) {
		if (object->getPlayer()) {
			if (object->getPlayer()->getWallVertListCache().size() > 0) {
				empty = false;
				auto rect = object->getPlayer()->getWallVertRectCache();
				pos = rect.origin;
				size = rect.size;

				// 取得した座標は左下の座標なので中心になるように変換
				pos.x += size.width * 0.5f;
				pos.y += size.height * 0.5f
					;
			}
		}
	}
	else if (group == CollisionComponent::kGroupHit) {
		if (object->getPlayer()) {
			if (object->getPlayer()->getHitVertListCache().size() > 0) {
				auto rect = object->getPlayer()->getHitVertRectCache();
				pos = rect.origin;
				size = rect.size;

				// 取得した座標は左下の座標なので中心になるように変換
				pos.x += size.width * 0.5f;
				pos.y += size.height * 0.5f;
				empty = false;
			}
		}
	}
	else if (group == CollisionComponent::kGroupAttack) {
		if (object->getPlayer()) {
			if (object->getPlayer()->getAttackVertListCache().size() > 0) {
				auto rect = object->getPlayer()->getAttackVertRectCache();
				pos = rect.origin;
				size = rect.size;

				// 取得した座標は左下の座標なので中心になるように変換
				pos.x += size.width * 0.5f;
				pos.y += size.height * 0.5f;
				empty = false;
			}
		}
	}
	else if (group == CollisionComponent::kGroupPortal) {
		auto portal = dynamic_cast<agtk::Portal *>(node);
		if (portal) {
			pos = portal->getPosition();
			size = portal->getContentSize();
			// 取得した座標は左下の座標なので中心になるように変換
			pos.x += size.width * 0.5f;
			pos.y += size.height * 0.5f;
			empty = false;
		}
	}
	else if (group == CollisionComponent::kGroupObjectCenter) {
		if (object) {
			pos = agtk::Scene::getPositionCocos2dFromScene(object->getPosition());
			size = cocos2d::Size(0, 0);
			empty = false;
		}
	}

	float left = pos.x - size.width;
	float top = pos.y + size.height;
	float right = pos.x + size.width;
	float bottom = pos.y - size.height;
	float width = m_unitSize.width;
	float height = m_unitSize.height;

	//当たり領域が存在しないときは、できるだけヒットに引っかからないように、一番小さな領域にセットする。
	if (empty) {
		level = m_dwCellNum - 1;
	}
	else
	if (left < 0 || bottom < 0 || m_field->getContentSize().width <= right || m_field->getContentSize().height <= top) {
		// いずれかの頂点がフィールド外へ出てしまう場合はlevelを0とし
		// 壁当たりチェックが行えるようにする
		level = 0;
	}
	else {
		int LT = collision_get_point_elem(left, top, width, height);
		int RB = collision_get_point_elem(right, bottom, width, height);

		int Def = RB ^ LT;
		unsigned int HiLevel = 0;
		for (int i = 0; i < m_uiLevel; i++) {
			int Check = (Def >> (i * 2)) & 0x3;
			if (Check != 0) HiLevel = i + 1;
		}
		level = RB >> (HiLevel * 2);
		int AddNum = (m_iPow[m_uiLevel - HiLevel] - 1) / 3;
		level += AddNum;
		// レベル計算(ここまで)
	}


	// 画面外へ出てしまった場合はlevelを0とし
	// 壁当たりチェックが行えるようにする
	if (level < 0 || level >= m_dwCellNum) {
		level = 0;
	}
	return level;
}
#endif

#ifndef _WIN32
#pragma mark -
#pragma mark 4分木空間計算
#endif

/**
 * 4分木空間に配置する。
 */
#ifdef USE_MULTITHREAD_OBJECT_UPDATE
void CollisionDetaction::updateSpaceStatusLocal(CollisionNode *collisionObject) {
	int level = calcLevelLocal(collisionObject);

	collisionObject->setLevel(level);

	/*
	if (level < 0 || level >= m_dwCellNum) {
	return;
	}
	*/

	auto spaceCellArray = m_spaceArray->get(level);
	if (!spaceCellArray->containsObject(collisionObject)) {
		spaceCellArray->addObject(collisionObject);
	}
}
void CollisionDetaction::updateSpaceStatus(CollisionNode *collisionObject) {
	mtxLock();
#ifdef USE_AGTK_MULTITHREAD_TEST
	Ref::setMtxLocked(true);
#endif
	updateSpaceStatusLocal(collisionObject);
#ifdef USE_AGTK_MULTITHREAD_TEST
	Ref::setMtxLocked(false);
#endif
	mtxUnlock();
}
#else
void CollisionDetaction::updateSpaceStatus(CollisionNode *collisionObject) {
	int level = calcLevel(collisionObject);

	collisionObject->setLevel(level);

	/*
    if (level < 0 || level >= m_dwCellNum) {
        return;
    }
	*/

    auto spaceCellArray = m_spaceArray->get(level);
    if (!spaceCellArray->containsObject(collisionObject)) {
        spaceCellArray->addObject(collisionObject);
    }
}
#endif

#ifdef USE_MULTITHREAD_OBJECT_UPDATE
void CollisionDetaction::scanCollisionDetectionLocal(int spaceIndex,
	CollisionNodeList *stackArray) {
	if (!m_spaceArray->count()) {
		return;
	}

	auto spaceCellArray = m_spaceArray->get(spaceIndex);

	checkHitSpaceCellLocal(spaceCellArray, spaceCellArray);
	checkHitSpaceCellLocal(spaceCellArray, stackArray);

	bool childFlag = false;
	int ObjNum = 0;
	int nextSpaceIndex;
	for (int i = 0; i < 4; i++) {
		nextSpaceIndex = spaceIndex * 4 + 1 + i;
		// 子空間があるかどうか
		if (nextSpaceIndex >= 0 && nextSpaceIndex < m_dwCellNum) {
			if (!childFlag) {
				auto count = spaceCellArray->count();
				for (int i = 0; i < (int)count; i++) {
					auto obj = spaceCellArray->objectAtIndex(i);
					stackArray->addObject(obj);
				}
				ObjNum += count;
			}
			childFlag = true;
			scanCollisionDetectionLocal(nextSpaceIndex, stackArray);
		}
	}

	if (childFlag) {
		for (int i = 0; i < ObjNum; i++) {
			stackArray->removeLastObject();
		}
	}

	return;
}
void CollisionDetaction::scanCollisionDetection(int spaceIndex,
	CollisionNodeList *stackArray) {
	mtxLock();
#ifdef USE_AGTK_MULTITHREAD_TEST
	Ref::setMtxLocked(true);
#endif
	scanCollisionDetectionLocal(spaceIndex, stackArray);
#ifdef USE_AGTK_MULTITHREAD_TEST
	Ref::setMtxLocked(false);
#endif
	mtxUnlock();
}
#else
void CollisionDetaction::scanCollisionDetection(int spaceIndex,
                                                CollisionNodeList *stackArray) {
    if (!m_spaceArray->count()) {
        return;
    }

    auto spaceCellArray = m_spaceArray->get(spaceIndex);

    checkHitSpaceCell(spaceCellArray, spaceCellArray);
    checkHitSpaceCell(spaceCellArray, stackArray);

    bool childFlag = false;
    int ObjNum = 0;
    int nextSpaceIndex;
    for (int i = 0; i < 4; i++) {
        nextSpaceIndex = spaceIndex * 4 + 1 + i;
        // 子空間があるかどうか
        if (nextSpaceIndex >= 0 && nextSpaceIndex < m_dwCellNum) {
            if (!childFlag) {
                for (auto obj : *spaceCellArray) {
                    stackArray->addObject(obj);
                }
                ObjNum += spaceCellArray->count();
            }
            childFlag = true;
            scanCollisionDetection(nextSpaceIndex, stackArray);
        }
    }

    if (childFlag) {
        for (int i = 0; i < ObjNum; i++) {
            stackArray->removeLastObject();
        }
    }

    return;
}
#endif

#ifdef USE_MULTITHREAD_OBJECT_UPDATE
void CollisionDetaction::checkHitLocal(CollisionNode *collisionObject1,
	CollisionNode *collisionObject2) {
	if (m_func) {
		m_func(collisionObject1, collisionObject2);
	}
}
void CollisionDetaction::checkHit(CollisionNode *collisionObject1,
	CollisionNode *collisionObject2) {
	mtxLock();
#ifdef USE_AGTK_MULTITHREAD_TEST
	Ref::setMtxLocked(true);
#endif
	checkHitLocal(collisionObject1, collisionObject2);
#ifdef USE_AGTK_MULTITHREAD_TEST
	Ref::setMtxLocked(false);
#endif
	mtxUnlock();
}
#else
void CollisionDetaction::checkHit(CollisionNode *collisionObject1,
                                  CollisionNode *collisionObject2) {
    if (m_func) {
        m_func(collisionObject1, collisionObject2);
    }
}
#endif

#ifdef USE_MULTITHREAD_OBJECT_UPDATE
void CollisionDetaction::checkHitSpaceCellLocal(CollisionNodeList *array1,
	CollisionNodeList *array2) {
	if (array1 == array2) {
		size_t ary1count = array1->count();
		for (unsigned int i = 0; i < ary1count; i++) {
			size_t ary2count = array2->count();
			for (unsigned int j = i + 1; j < ary2count; j++) {
				checkHitLocal(array1->objectAtIndex(i), array2->objectAtIndex(j));
			}
		}
	}
	else {
		auto count1 = array1->count();
		auto count2 = array2->count();
		for (int i = 0; i < (int)count1; i++) {
			auto obj = array1->objectAtIndex(i);
			for (int j = 0; j < (int)count2; j++) {
				auto obj2 = array2->objectAtIndex(j);
				if (obj != obj2){
					checkHitLocal(obj, obj2);
				}
			}
		}
	}
}
void CollisionDetaction::checkHitSpaceCell(CollisionNodeList *array1,
	CollisionNodeList *array2) {
	mtxLock();
#ifdef USE_AGTK_MULTITHREAD_TEST
	Ref::setMtxLocked(true);
#endif
	checkHitSpaceCellLocal(array1, array2);
#ifdef USE_AGTK_MULTITHREAD_TEST
	Ref::setMtxLocked(false);
#endif
	mtxUnlock();
}
#else
void CollisionDetaction::checkHitSpaceCell(CollisionNodeList *array1,
                                           CollisionNodeList *array2) {
    if (array1 == array2) {
        size_t ary1count = array1->count();
        for (unsigned int i = 0; i < ary1count; i++) {
            size_t ary2count = array2->count();
            for (unsigned int j = i + 1; j < ary2count; j++) {
                checkHit(array1->objectAtIndex(i), array2->objectAtIndex(j));
            }
        }
    } else {
        for (auto obj : *array1) {
            for (auto obj2 : *array2) {
                if (obj != obj2) {
                    checkHit(obj, obj2);
                }
            }
        }
    }
}
#endif

CollisionDetaction *CollisionDetaction::getDefaultDetaction() {
    if (s_instance == nullptr) {
        s_instance = new CollisionDetaction();
    }
    return s_instance;
}
#ifdef USE_MULTITHREAD_OBJECT_UPDATE
#ifdef USE_MULTITHREAD_MEASURE
void CollisionDetaction::setStateChangeCounter(int i)
{
	m_stateChangeCounter = i;
}
int CollisionDetaction::getStateChangeCounter()
{
	return m_stateChangeCounter;
}
#endif

void CollisionDetaction::deleteNullptrm_spaceArray()
{
	CollisionNode *node = nullptr;

	if (m_gameObjectArray != nullptr) {
		m_gameObjectArray->erase(node);
	}

	if (m_spaceArray != nullptr) {
		for (int i = 0; i < (int)m_spaceArray->count(); i++) {
			auto spaceCellArray = m_spaceArray->get(i);
			spaceCellArray->erase(node);
		}
	}
}
#endif

#ifdef USE_SAR_TEST_0
void CollisionDetaction::dumpLocal(int spaceIndex)
{
	if (m_spaceArray == nullptr || !m_spaceArray->count()) {
		return;
	}

	auto spaceCellArray = m_spaceArray->get(spaceIndex);
	if (spaceCellArray == nullptr) {
		return;
	}
	for (int i = 0; i < (int)spaceCellArray->count(); i++) {
		auto obj = spaceCellArray->objectAtIndex(i);
		if (obj == nullptr) continue;
		agtk::Object* o = static_cast<agtk::Object*>(obj->getNode());
		auto objData = o->getObjectData();
		if (objData) {
			CCLOG("dump: %s %d", objData->getName(), o->getInstanceId());
		}
		else {
			CCLOG("dump: %s %d", o->getName().c_str(), o->getInstanceId());
		}
	}

	int nextSpaceIndex;
	for (int i = 0; i < 4; i++) {
		nextSpaceIndex = spaceIndex * 4 + 1 + i;
		// 子空間があるかどうか
		if (nextSpaceIndex >= 0 && nextSpaceIndex < m_dwCellNum) {
			dumpLocal(nextSpaceIndex);
		}
	}
}

void CollisionDetaction::dump()
{
	dumpLocal(0);
}
#endif
