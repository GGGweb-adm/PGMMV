#include "Course.h"
#include "Scene.h"
#include "Object.h"
#include "Slope.h"
#include "Manager/GameManager.h"
#include "External/SplineInterp/SplineInterp.h"
#include "Data/PlayData.h"

NS_AGTK_BEGIN
//-------------------------------------------------------------------------------------------------------------------
OthersCourse::OthersCourse()
{
	_length = 0;
	_loopUnlimited = false;
	_loopCount = 0;
	_reverseMove = false;
	_connectEndStart = false;

	_coursePointList = nullptr;
	_pausedBySwitch = true;
}

OthersCourse::~OthersCourse()
{
	CC_SAFE_RELEASE_NULL(_coursePointList);
}

OthersCourse::EnumCourseType OthersCourse::getCourseType()
{
	return kCourseTypeNone;
}

bool OthersCourse::init(int id)
{
	_id = id;

	setCoursePointList(cocos2d::Array::create());

	return true;
}

void OthersCourse::CalcLength()
{
	_length = 0;
	int count = getCoursePointList()->count();
	if (count > 1) {
		for (int i = 0; i < count - 1; i++) {

			int nowIdx = i;
			int nextIdx = i + 1;

// #AGTK-NX
#ifdef STATIC_DOWN_CAST
			auto p1 = static_cast<CoursePoint*>(getCoursePointList()->getObjectAtIndex(nowIdx));
			auto p2 = static_cast<CoursePoint*>(getCoursePointList()->getObjectAtIndex(nextIdx));
#else
			auto p1 = dynamic_cast<CoursePoint*>(getCoursePointList()->getObjectAtIndex(nowIdx));
			auto p2 = dynamic_cast<CoursePoint*>(getCoursePointList()->getObjectAtIndex(nextIdx));
#endif

			Vec2 vec = Vec2(p2->point - p1->point);

			float length = vec.length();
			// 始点から終点への距離を始点に設定
			p1->nextLength = length;
			// 終点から視点への距離を終点に設定
			p2->prevLength = length;

			// 距離を加算する
			_length += length;
		}
	}
}

Vec2 OthersCourse::moveCourse(agtk::ObjectCourseMove *move, float timeScale, bool& isReset)
{
	Vec2 pos = Vec2::ZERO;
	isReset = false;

	// 初回の移動の場合
	if (move->getIsFirst()) {
		float length = 0;
		int idx = 0;

		for (int i = 0; i < _coursePointList->count(); i++) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
			auto p = static_cast<CoursePoint*>(_coursePointList->getObjectAtIndex(i));
#else
			auto p = dynamic_cast<CoursePoint*>(_coursePointList->getObjectAtIndex(i));
#endif

			// 指定コースポイントと同じポイントの場合
			if (p->coursePointId == move->getCurrentPointId()) {

				pos = p->point;
				idx = i;
				break;
			}
			// ポイントが違う場合
			else {
				// 移動距離を加算
				length += p->nextLength;
			}
		}

		// 現在のIDXを設定
		move->setCurrentPointIdx(idx);

		// 開始地点までの距離を移動済みとみなして設定
		move->setMoveDist(length);

		// 初回移動終了
		move->setIsFirst(false);

		// 逆回転
		if (this->getCourseType() == kCourseTypeCircle) {
			auto othersCircleCourse = dynamic_cast<OthersCircleCourse *>(this);
			if (othersCircleCourse) {
				move->setReverseCourse(othersCircleCourse->getCourseData()->getReverseCourse());
			}
		}

		// 初回スイッチ実行
		int currentPointId = move->getCurrentPointId();
		changeSwitchVariable(currentPointId);
	}
	// 2回目以降の移動の場合
	else {

		// 移動量を取得
		float moveVal = getMoveSpeed(move->getCurrentPointId()) * timeScale;

		//CCLOG("現在のポイントID : %d -- %f", move->getCurrentPointId(), moveVal);


		int currentPointId = 0;

		// 現在の移動距離を取得
		float dist = move->getMoveDist();

		while (true) {
			if (_pausedBySwitch) {
LCheckSwitch:
				auto p = getPoint(move->getCurrentPointId());
				auto switchValue = GameManager::getInstance()->getSwitchValue(p->getSwitchObjectId(), p->getSwitchQualifierId(), p->getSwitchId());
				if (switchValue != 0) {
					_pausedBySwitch = false;
				}
				if (_pausedBySwitch) {
					for (int i = 0; i < _coursePointList->count(); i++) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
						auto p = static_cast<CoursePoint*>(_coursePointList->getObjectAtIndex(i));
#else
						auto p = dynamic_cast<CoursePoint*>(_coursePointList->getObjectAtIndex(i));
#endif
						// 指定コースポイントと同じポイントの場合
						if (p->coursePointId == move->getCurrentPointId()) {
							return p->point;
						}
					}
				}

				// 残り移動距離が0になったら処理終了
				if (moveVal <= 0) { break; }
			}

			float m = moveVal;
			bool isFinish = false;

			// 現在のポイントIDを取得
			int prevPointId = move->getCurrentPointId();

			// 通常移動の場合
			if (!move->getReverseMove()) {

				// 現在の移動距離と移動量を足した値がコースの全長を越えた場合
				if (dist + moveVal > _length) {
					m = _length - dist;
					isFinish = true;
				}

				// 移動量分だけ移動距離を増加
				dist += m;
				// 移動量分だけ残り移動量を減らす
				moveVal -= m;

				float d = 0;
				for (int i = 0; i < _coursePointList->count(); i++) {

					int nowIdx = i;

// #AGTK-NX
#ifdef STATIC_DOWN_CAST
					auto p = static_cast<CoursePoint*>(_coursePointList->getObjectAtIndex(nowIdx));
#else
					auto p = dynamic_cast<CoursePoint*>(_coursePointList->getObjectAtIndex(nowIdx));
#endif

					// 移動先のポイント
					if (d <= dist && dist <= d + p->nextLength) {

						float per = (dist - d) / (p->nextLength);

						int nextIdx = (i + 1) % _coursePointList->count();

// #AGTK-NX
#ifdef STATIC_DOWN_CAST
						auto next = static_cast<CoursePoint*>(_coursePointList->getObjectAtIndex(nextIdx));
#else
						auto next = dynamic_cast<CoursePoint*>(_coursePointList->getObjectAtIndex(nextIdx));
#endif

						pos.x = AGTK_LINEAR_INTERPOLATE(p->point.x, next->point.x, 1.0f, per);
						pos.y = AGTK_LINEAR_INTERPOLATE(p->point.y, next->point.y, 1.0f, per);
						//CCLOG("現在のポイントID : %d -- %d", p->coursePointId, p->reverseCoursePointId);

						// 現在のポイントIDを更新する
						currentPointId = p->coursePointId;
						move->setCurrentPointId(currentPointId);

						// 現在のポイントIDXを更新する
						move->setCurrentPointIdx(nowIdx);

						// 終端到達時
						if (isFinish) {

							// 反転移動しない場合
							if (!getReverseMove()) {

								// 無限ループを行わない場合
								if (!getLoopUnlimited()) {
									// ループ回数を更新する
									int cnt = move->getLoopCount() + 1;
									if (cnt > getLoopCount()) {
										cnt = getLoopCount();
									}
									move->setLoopCount(cnt);
								}



								
								// ループが発生しない場合
								if (!checkLoop(move->getLoopCount())) {
									// 終点と始点が接続していない場合
									if (!getConnectEndStart()) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
										auto end = static_cast<CoursePoint*>(_coursePointList->getObjectAtIndex(_coursePointList->count() - 1));
#else
										auto end = dynamic_cast<CoursePoint*>(_coursePointList->getObjectAtIndex(_coursePointList->count() - 1));
#endif

										// 現在のポイントIDを更新する
										currentPointId = end->coursePointId;
									}
									// 始点と終点が接続している場合
									else {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
										auto start = static_cast<CoursePoint*>(_coursePointList->getObjectAtIndex(0));
#else
										auto start = dynamic_cast<CoursePoint*>(_coursePointList->getObjectAtIndex(0));
#endif

										// 現在のポイントIDを更新する
										currentPointId = start->coursePointId;
									}

									move->setCurrentPointId(currentPointId);
									// これ以上移動できないようにする
									moveVal = 0;
								}
								// ループが発生する場合
								else {
									// 始点と終点が接続していない場合
									if (!getConnectEndStart()) {

										isReset = true;

										// すぐに始点へ戻ってしまうので、終点のスイッチ変数変更イベントをここで行う
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
										auto end = static_cast<CoursePoint*>(_coursePointList->getObjectAtIndex(_coursePointList->count() - 1));
#else
										auto end = dynamic_cast<CoursePoint*>(_coursePointList->getObjectAtIndex(_coursePointList->count() - 1));
#endif
										changeSwitchVariable(end->coursePointId);

//										CCLOG("ID変更 : %d", end->coursePointId);
									}
									else {
										// 初期地点を取得
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
										auto start = static_cast<CoursePoint*>(_coursePointList->getObjectAtIndex(0));
#else
										auto start = dynamic_cast<CoursePoint*>(_coursePointList->getObjectAtIndex(0));
#endif

										// 現在のポイントIDを更新する
										currentPointId = start->coursePointId;
										move->setCurrentPointId(currentPointId);
										// 現在のポイントIDXを初期化する
										move->setCurrentPointIdx(0);
									}

									// 移動距離を初期化
									dist = 0;
								}
							}
							// 反転移動する場合
							else {
								// 反転移動中を設定
								move->setReverseMove(true);

								// 始点と終点が接続していない場合
								if (!getConnectEndStart()) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
									auto end = static_cast<CoursePoint*>(_coursePointList->getObjectAtIndex(_coursePointList->count() - 1));
#else
									auto end = dynamic_cast<CoursePoint*>(_coursePointList->getObjectAtIndex(_coursePointList->count() - 1));
#endif

									// 現在のポイントIDを更新する
									currentPointId = end->coursePointId;
									move->setCurrentPointId(end->coursePointId);
									// 現在のポイントIDXに終点を設定する
									move->setCurrentPointIdx(_coursePointList->count() - 1);

								}
								else {
									// 初期地点を取得
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
									auto start = static_cast<CoursePoint*>(_coursePointList->getObjectAtIndex(0));
#else
									auto start = dynamic_cast<CoursePoint*>(_coursePointList->getObjectAtIndex(0));
#endif

									// 現在のポイントIDを更新する
									currentPointId = start->coursePointId;
									move->setCurrentPointId(start->coursePointId);
									// 現在のポイントIDXに終点を設定する
									move->setCurrentPointIdx(_coursePointList->count() - 1);
								}


							}
						}

						break;
					}
					else {
						d += p->nextLength;
					}
				}
			}
			// 反転移動の場合
			else {

				// コースの始点を越えた場合
				if ((dist - moveVal) <= 0) {
					m = dist;
					isFinish = true;
				}

				// 移動量分だけ移動距離を増加
				dist -= m;
				// 移動量分だけ残り移動量を減らす
				moveVal -= m;

				float d = _length;
				for (int i = _coursePointList->count() - 1; i >= 0; i--) {

					int nowIdx = i;

// #AGTK-NX
#ifdef STATIC_DOWN_CAST
					auto p = static_cast<CoursePoint*>(_coursePointList->getObjectAtIndex(nowIdx));
#else
					auto p = dynamic_cast<CoursePoint*>(_coursePointList->getObjectAtIndex(nowIdx));
#endif

					if (i == 0) {
						d = 0;
					}

					if (d - p->prevLength <= dist &&  dist <= d)  {
						// 始点をチェックする場合前のポイントの長さがないため
						// 無理矢理0を設定している
						float per = (i == 0) ? 0.0f : (p->prevLength - ((_length - dist) - (_length - d))) / p->prevLength;

						int prevIdx = i - 1;
						if (prevIdx < 0) {
							prevIdx = 0;
						}
						CC_ASSERT(prevIdx >= 0);
						
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
						auto prev = static_cast<CoursePoint*>(_coursePointList->getObjectAtIndex(prevIdx));
#else
						auto prev = dynamic_cast<CoursePoint*>(_coursePointList->getObjectAtIndex(prevIdx));
#endif

						pos.x = AGTK_LINEAR_INTERPOLATE(prev->point.x, p->point.x, 1.0f, per);
						pos.y = AGTK_LINEAR_INTERPOLATE(prev->point.y, p->point.y, 1.0f, per);
						
						//CCLOG("現在のポイントID : %d -- %d", prev->coursePointId, prev->reverseCoursePointId);


						// 反転移動時のポイントIDを更新する
						currentPointId = prev->reverseCoursePointId;
						move->setCurrentPointId(currentPointId);

						// 現在のポイントIDXを更新する
						move->setCurrentPointIdx(prevIdx);

						// 始点到達時
						if (isFinish) {

							// 無限ループを行わない場合
							if (!getLoopUnlimited()) {
								// ループ回数を更新する
								int cnt = move->getLoopCount() + 1;
								if (cnt > getLoopCount()) {
									cnt = getLoopCount();
								}
								move->setLoopCount(cnt);
							}

							// 初期地点を取得
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
							auto start = static_cast<CoursePoint*>(_coursePointList->getObjectAtIndex(0));
#else
							auto start = dynamic_cast<CoursePoint*>(_coursePointList->getObjectAtIndex(0));
#endif

							// 現在のポイントIDを更新する
							currentPointId = start->coursePointId;
							move->setCurrentPointId(currentPointId);
							// 現在のポイントIDXに終点を設定する
							move->setCurrentPointIdx(_coursePointList->count() - 1);

							// ループが発生しない場合
							if (!checkLoop(move->getLoopCount())) {
								// これ以上移動できないようにする
								moveVal = 0;
							}
							// ループが発生する場合
							else {
								// 通常移動を設定
								move->setReverseMove(false);
							}
						}

						break;
					}
					else
					{
						d -= p->prevLength;
					}
				}
			}

			if (prevPointId != currentPointId) {
				changeSwitchVariable(currentPointId);
//				CCLOG("ID変更 : %d", currentPointId);
				_pausedBySwitch = true;
				// 現在の移動距離を更新
				move->setMoveDist(dist);
				goto LCheckSwitch;
			}

			// 残り移動距離が0になったら処理終了
			if (moveVal <= 0) { break; }
		}

		// 現在の移動距離を更新
		move->setMoveDist(dist);

	}

	return pos;
}

/**
* 開始ポイント座標の取得
*/
Vec2 OthersCourse::getStartPointPos(const ObjectCourseMove *move)
{
	Vec2 pos = Vec2::ZERO;

	for (int i = 0; i < _coursePointList->count(); i++) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
		auto p = static_cast<CoursePoint*>(_coursePointList->getObjectAtIndex(i));
#else
		auto p = dynamic_cast<CoursePoint*>(_coursePointList->getObjectAtIndex(i));
#endif

		// 指定コースポイントと同じポイントの場合
		if (p->coursePointId == move->getCurrentPointId()) {
			pos = p->point;
			break;
		}
	}
	return pos;
}

bool OthersCourse::checkLoop(int currentLoopCount)
{
	// ループ回数が無限の場合は、ループを許可する
	if (getLoopUnlimited()) { return true; }
	// 現在のループ回数が最大ループ回数まで到達していない場合は、ループを許可する
	if ((currentLoopCount) < getLoopCount()) { return true; }

	return false;
}

void OthersCourse::changeSwitchVariable(int courseId)
{
}

float OthersCourse::getMoveSpeed(int courseId)
{
	return 0;
}

#ifdef USE_PREVIEW
void OthersCourse::showDebugVisible(bool isShow)
{
}

void OthersCourse::showDebugVisible(bool isShow, cocos2d::Node* node, cocos2d::Color4F color)
{
	auto debugView = node->getChildByName("debugVisible");

	if (!debugView && isShow) {
		cocos2d::DrawNode *line = cocos2d::DrawNode::create();

		if (getCoursePointList()->count() > 1) {
			for (int i = 0; i < getCoursePointList()->count() - 1; i++) {

				auto p1 = dynamic_cast<CoursePoint*>(getCoursePointList()->getObjectAtIndex(i));
				auto p2 = dynamic_cast<CoursePoint*>(getCoursePointList()->getObjectAtIndex(i + 1));

				line->drawSegment(p1->point, p2->point, 1.0f, color);
			}
		}

		node->addChild(line, 1, "debugVisible");
	}
	else if (debugView) {
		debugView->setVisible(isShow);
	}
}
#endif

//-------------------------------------------------------------------------------------------------------------------
OthersCourse::CoursePoint::CoursePoint()
{
	coursePointId = -1;
	reverseCoursePointId = -1;
	nextLength = 0;
	prevLength = 0;
}

OthersCourse::CoursePoint::~CoursePoint()
{

}

bool OthersCourse::CoursePoint::init()
{
	point = Point::ZERO;

	return true;
}

//-------------------------------------------------------------------------------------------------------------------
OthersLineCourse::OthersLineCourse()
{
	_courseData = nullptr;
}

OthersLineCourse::~OthersLineCourse()
{
	CC_SAFE_RELEASE_NULL(_courseData);
}

bool OthersLineCourse::init(int id, agtk::data::OthersLineCourseData* courseData, agtk::data::SceneData* sceneData)
{
	// 基底を呼び出す
	OthersCourse::init(id);

	setCourseData(courseData);

	// コースの初期化
	int len = getCourseData()->getPointList()->count();
	int points = len;
	bool bEndStartConnected = (len >= 2 && getCourseData()->getConnectEndAndStart());

	// 終点と始点を接続する場合はポイントの数を増やす
	if (bEndStartConnected) {
		points += 1;
	}

	for (int i = 0; i < points; i++) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
		auto p1 = static_cast<agtk::data::OthersLineCourseData::Point*>(getCourseData()->getPointList()->getObjectAtIndex(i % len));
		auto p2 = static_cast<agtk::data::OthersLineCourseData::Point*>(getCourseData()->getPointList()->getObjectAtIndex((i + 1) % len));
#else
		auto p1 = dynamic_cast<agtk::data::OthersLineCourseData::Point*>(getCourseData()->getPointList()->getObjectAtIndex(i % len));
		auto p2 = dynamic_cast<agtk::data::OthersLineCourseData::Point*>(getCourseData()->getPointList()->getObjectAtIndex((i + 1) % len));
#endif

		auto coursePoint = CoursePoint::create();

		coursePoint->coursePointId = p1->getId();
		coursePoint->reverseCoursePointId = p2->getId();

		coursePoint->point.x = getCourseData()->getX() + p1->getOffsetX();
		coursePoint->point.y = getCourseData()->getY() + p1->getOffsetY();

		coursePoint->point = agtk::Scene::getPositionSceneFromCocos2d(coursePoint->point, sceneData);

		getCoursePointList()->addObject(coursePoint);
	}
	
	_loopCount = courseData->getLoopCount();
	_loopUnlimited = courseData->getLoopUnlimited();
	_reverseMove = courseData->getEndReverseMove();
	_connectEndStart = bEndStartConnected;

	// 全長を算出する
	OthersCourse::CalcLength();

	return true;
}

void OthersLineCourse::changeSwitchVariable(int courseId)
{
	cocos2d::Ref *ref = nullptr;
	int index = -1;
	CCARRAY_FOREACH(getCourseData()->getPointList(), ref) {
		index++;
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
		auto p = static_cast<agtk::data::OthersLineCourseData::Point*>(ref);
#else
		auto p = dynamic_cast<agtk::data::OthersLineCourseData::Point*>(ref);
#endif

		if (p->getId() == courseId) {
			// スイッチ・変数を変更する変更する
			auto sceneId = GameManager::getInstance()->getCurrentScene()->getSceneData()->getId();
			GameManager::getInstance()->calcSwichVariableChange(p->getSwitchVariableAssignList(), GameManager::kPlaceCourse, sceneId, this->getId(), index);
			// 変数・スイッチ変更時のオブジェクトに対して変更処理。
			GameManager::getInstance()->updateObjectVariableAndSwitch();
		}
	}
}

float OthersLineCourse::getMoveSpeed(int courseId)
{
	cocos2d::Ref *ref = nullptr;
	CCARRAY_FOREACH(getCourseData()->getPointList(), ref) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
		auto p = static_cast<agtk::data::OthersLineCourseData::Point*>(ref);
#else
		auto p = dynamic_cast<agtk::data::OthersLineCourseData::Point*>(ref);
#endif

		if (p->getId() == courseId) {
			return p->getMove();
		}
	}

	return 0;
}

agtk::data::OthersData::Point *OthersLineCourse::getPoint(int pointId)
{
	cocos2d::Ref *ref = nullptr;
	CCARRAY_FOREACH(getCourseData()->getPointList(), ref) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
		auto p = static_cast<agtk::data::OthersData::Point*>(ref);
#else
		auto p = dynamic_cast<agtk::data::OthersData::Point*>(ref);
#endif

		if (p->getId() == pointId) {
			return p;
		}
	}
	return nullptr;
}

OthersCourse::EnumCourseType OthersLineCourse::getCourseType()
{
	return kCourseTypeLine;
}

#ifdef USE_PREVIEW
void OthersLineCourse::showDebugVisible(bool isShow)
{
	OthersCourse::showDebugVisible(isShow, this, cocos2d::Color4F(
		getCourseData()->getR() / 255.0f,
		getCourseData()->getG() / 255.0f,
		getCourseData()->getB() / 255.0f,
		getCourseData()->getA() / 255.0f)
	);
}
#endif

//-------------------------------------------------------------------------------------------------------------------
OthersCurveCourse::OthersCurveCourse()
{
	_courseData = nullptr;
}

OthersCurveCourse::~OthersCurveCourse()
{
	CC_SAFE_RELEASE_NULL(_courseData);
}

bool OthersCurveCourse::init(int id, agtk::data::OthersCurveCourseData* courseData, agtk::data::SceneData* sceneData)
{
	// 基底を呼び出す
	OthersCourse::init(id);

	setCourseData(courseData);

	// コースの初期化
	float x = getCourseData()->getX();
	float y = getCourseData()->getY();

	int len = getCourseData()->getPointList()->count();
	int points = 0;
	bool bEndStartConnected = (len >= 2 && getCourseData()->getConnectEndAndStart());

	// 終点と始点をつなぐ場合
	if (bEndStartConnected) {
		points = len * CURVE_DIVS + 1;
	}
	else {
		points = (len - 1) * CURVE_DIVS + 1;
	}

	SplineInterp splineX, splineY;

	// 終点と始点をつなぐ場合
	if (bEndStartConnected) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
		auto p1 = static_cast<agtk::data::OthersCurveCourseData::Point*>(getCourseData()->getPointList()->getObjectAtIndex((len * 2 - 2) % len));
		auto p2 = static_cast<agtk::data::OthersCurveCourseData::Point*>(getCourseData()->getPointList()->getObjectAtIndex(len - 1));
#else
		auto p1 = dynamic_cast<agtk::data::OthersCurveCourseData::Point*>(getCourseData()->getPointList()->getObjectAtIndex((len * 2 - 2) % len));
		auto p2 = dynamic_cast<agtk::data::OthersCurveCourseData::Point*>(getCourseData()->getPointList()->getObjectAtIndex(len - 1));
#endif

		splineX.AddKey(-2, x + p1->getOffsetX());
		splineY.AddKey(-2, y + p1->getOffsetY());
		splineX.AddKey(-1, x + p2->getOffsetX());
		splineY.AddKey(-1, y + p2->getOffsetY());
	}

	for (int i = 0; i < len; i++) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
		auto p = static_cast<agtk::data::OthersCurveCourseData::Point*>(getCourseData()->getPointList()->getObjectAtIndex(i));
#else
		auto p = dynamic_cast<agtk::data::OthersCurveCourseData::Point*>(getCourseData()->getPointList()->getObjectAtIndex(i));
#endif

		splineX.AddKey(i, x + p->getOffsetX());
		splineY.AddKey(i, y + p->getOffsetY());
	}

	// 終点と始点をつなぐ場合
	if (bEndStartConnected) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
		auto p1 = static_cast<agtk::data::OthersCurveCourseData::Point*>(getCourseData()->getPointList()->getObjectAtIndex(0));
		auto p2 = static_cast<agtk::data::OthersCurveCourseData::Point*>(getCourseData()->getPointList()->getObjectAtIndex(1));
#else
		auto p1 = dynamic_cast<agtk::data::OthersCurveCourseData::Point*>(getCourseData()->getPointList()->getObjectAtIndex(0));
		auto p2 = dynamic_cast<agtk::data::OthersCurveCourseData::Point*>(getCourseData()->getPointList()->getObjectAtIndex(1));
#endif

		splineX.AddKey(len, x + p1->getOffsetX());
		splineY.AddKey(len, y + p1->getOffsetY());
		splineX.AddKey(len + 1, x + p2->getOffsetX());
		splineY.AddKey(len + 1, y + p2->getOffsetY());
	}

	for (int i = 0; i < points; i++) {
		float t = (float)i / CURVE_DIVS;
		auto x0 = splineX.GetInterpolated(t);
		auto y0 = splineY.GetInterpolated(t);

		int idx = (int)(i / CURVE_DIVS);


		if (idx >= getCourseData()->getPointList()->count()) {
			idx = getCourseData()->getPointList()->count() - 1;
		}

// #AGTK-NX
#ifdef STATIC_DOWN_CAST
		auto p1 = static_cast<agtk::data::OthersCurveCourseData::Point*>(getCourseData()->getPointList()->getObjectAtIndex(idx));
		auto p2 = static_cast<agtk::data::OthersCurveCourseData::Point*>(getCourseData()->getPointList()->getObjectAtIndex((idx + 1) % len));
#else
		auto p1 = dynamic_cast<agtk::data::OthersCurveCourseData::Point*>(getCourseData()->getPointList()->getObjectAtIndex(idx));
		auto p2 = dynamic_cast<agtk::data::OthersCurveCourseData::Point*>(getCourseData()->getPointList()->getObjectAtIndex((idx + 1) % len));
#endif

		auto coursePoint = CoursePoint::create();

		coursePoint->coursePointId = p1->getId();
		coursePoint->reverseCoursePointId = p2->getId();

		// 終端を接続しない場合
		if ((i == points - 1) && !bEndStartConnected) {
			coursePoint->reverseCoursePointId = coursePoint->coursePointId;
		}


		coursePoint->point.x = x0;
		coursePoint->point.y = y0;

		coursePoint->point = agtk::Scene::getPositionSceneFromCocos2d(coursePoint->point, sceneData);

		getCoursePointList()->addObject(coursePoint);
	}

	_loopCount = courseData->getLoopCount();
	_loopUnlimited = courseData->getLoopUnlimited();
	_reverseMove = courseData->getEndReverseMove();
	_connectEndStart = bEndStartConnected;

	// 全長を算出する
	OthersCourse::CalcLength();

	return true;
}

void OthersCurveCourse::changeSwitchVariable(int courseId)
{
	cocos2d::Ref *ref = nullptr;
	int index = -1;
	CCARRAY_FOREACH(getCourseData()->getPointList(), ref) {
		index++;
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
		auto p = static_cast<agtk::data::OthersCurveCourseData::Point*>(ref);
#else
		auto p = dynamic_cast<agtk::data::OthersCurveCourseData::Point*>(ref);
#endif
		if (p->getId() == courseId) {
			// スイッチ・変数を変更する変更する
			auto sceneId = GameManager::getInstance()->getCurrentScene()->getSceneData()->getId();
			GameManager::getInstance()->calcSwichVariableChange(p->getSwitchVariableAssignList(), GameManager::kPlaceCourse, sceneId, this->getId(), index);
			// 変数・スイッチ変更時のオブジェクトに対して変更処理。
			GameManager::getInstance()->updateObjectVariableAndSwitch();
		}
	}
}

float OthersCurveCourse::getMoveSpeed(int courseId)
{
	cocos2d::Ref *ref = nullptr;

	CCARRAY_FOREACH(getCourseData()->getPointList(), ref) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
		auto p = static_cast<agtk::data::OthersCurveCourseData::Point*>(ref);
#else
		auto p = dynamic_cast<agtk::data::OthersCurveCourseData::Point*>(ref);
#endif

		if (p->getId() == courseId) {
			return p->getMove();
		}
	}

	return 0;
}

agtk::data::OthersData::Point *OthersCurveCourse::getPoint(int pointId)
{
	cocos2d::Ref *ref = nullptr;
	CCARRAY_FOREACH(getCourseData()->getPointList(), ref) {
		auto p = dynamic_cast<agtk::data::OthersData::Point*>(ref);

		if (p->getId() == pointId) {
			return p;
		}
	}
	return nullptr;
}

OthersCourse::EnumCourseType OthersCurveCourse::getCourseType()
{
	return kCourseTypeCurve;
}

#ifdef USE_PREVIEW
void OthersCurveCourse::showDebugVisible(bool isShow)
{
	OthersCourse::showDebugVisible(isShow, this, cocos2d::Color4F(
		getCourseData()->getR() / 255.0f,
		getCourseData()->getG() / 255.0f,
		getCourseData()->getB() / 255.0f,
		getCourseData()->getA() / 255.0f)
	);
}
#endif

//-------------------------------------------------------------------------------------------------------------------
OthersCircleCourse::OthersCircleCourse()
{
	_courseData = nullptr;
}


OthersCircleCourse::~OthersCircleCourse()
{
	CC_SAFE_RELEASE_NULL(_courseData);
}

bool OthersCircleCourse::init(int id, agtk::data::OthersCircleCourseData* courseData, agtk::data::SceneData* sceneData)
{
	// 基底を呼び出す
	OthersCourse::init(id);

	setCourseData(courseData);

	// コースの初期化
	int len = getCourseData()->getPointList()->count();
	int points = len * CURVE_DIVS;

	// スイッチ区間は8区切りだが、最後のみ9区切りとなっているので注意
	// 最後の区間はひとつ前の区間と同じ値となっている
	for (int i = 0; i <= points; i++) {
		float theta;
		if(courseData->getReverseCourse()){
			theta = (float)i * 2 * M_PI / points;
		} else {
			theta = (float)-i * 2 * M_PI / points;
		}

		int idx;
		int p2Idx;
		if(courseData->getReverseCourse()){
			// 最後の区間はひとつ前の区間と同じ値にする
			if (i == points) {
				idx = 1;
			}
			else {
				idx = ((points - i + CURVE_DIVS - 1) / CURVE_DIVS);
				if (idx >= len) {
					idx = 0;
				}
			}

			if (idx == 0) {
				p2Idx = (len - 1) % len;
			}
			else {
				p2Idx = (idx - 1) % len;
			}
		} else {
			idx = (i / CURVE_DIVS);
			if (idx >= len) {
				idx = len - 1;
			}
			p2Idx = (idx + 1) % len;
		}

// #AGTK-NX
#ifdef STATIC_DOWN_CAST
		auto p1 = static_cast<agtk::data::OthersCircleCourseData::Point*>(getCourseData()->getPointList()->getObjectAtIndex(idx));
		auto p2 = static_cast<agtk::data::OthersCircleCourseData::Point*>(getCourseData()->getPointList()->getObjectAtIndex(p2Idx));
#else
		auto p1 = dynamic_cast<agtk::data::OthersCircleCourseData::Point*>(getCourseData()->getPointList()->getObjectAtIndex(idx));
		auto p2 = dynamic_cast<agtk::data::OthersCircleCourseData::Point*>(getCourseData()->getPointList()->getObjectAtIndex(p2Idx));
#endif


		auto coursePoint = CoursePoint::create();

		coursePoint->coursePointId = p1->getId();
		coursePoint->reverseCoursePointId = p2->getId();

		coursePoint->point.x = getCourseData()->getX() + getCourseData()->getRadiusX() * sinf(theta);
		coursePoint->point.y = getCourseData()->getY() + getCourseData()->getRadiusY() * cosf(theta);

		coursePoint->point = agtk::Scene::getPositionSceneFromCocos2d(coursePoint->point, sceneData);

		getCoursePointList()->addObject(coursePoint);
	}

	_loopCount = courseData->getLoopCount();
	_loopUnlimited = courseData->getLoopUnlimited();
	_reverseMove = courseData->getEndReverseMove();
	_connectEndStart = true;

	// 全長を算出する
	OthersCourse::CalcLength();

	return true;
}

void OthersCircleCourse::changeSwitchVariable(int courseId)
{
	cocos2d::Ref *ref = nullptr;
	int index = -1;
	CCARRAY_FOREACH(getCourseData()->getPointList(), ref) {
		index++;
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
		auto p = static_cast<agtk::data::OthersCircleCourseData::Point*>(ref);
#else
		auto p = dynamic_cast<agtk::data::OthersCircleCourseData::Point*>(ref);
#endif

		if (p->getId() == courseId) {
			//　スイッチ・変数を変更する
			auto sceneId = GameManager::getInstance()->getCurrentScene()->getSceneData()->getId();
			GameManager::getInstance()->calcSwichVariableChange(p->getSwitchVariableAssignList(), GameManager::kPlaceCourse, sceneId, this->getId(), index);
			// 変数・スイッチ変更時のオブジェクトに対して変更処理。
			GameManager::getInstance()->updateObjectVariableAndSwitch();
		}
	}
}

float OthersCircleCourse::getMoveSpeed(int courseId)
{
	cocos2d::Ref *ref = nullptr;
	CCARRAY_FOREACH(getCourseData()->getPointList(), ref) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
		auto p = static_cast<agtk::data::OthersCircleCourseData::Point*>(ref);
#else
		auto p = dynamic_cast<agtk::data::OthersCircleCourseData::Point*>(ref);
#endif

		if (p->getId() == courseId) {
			return p->getMove();
		}
	}

	return 0;
}

agtk::data::OthersData::Point *OthersCircleCourse::getPoint(int pointId)
{
	cocos2d::Ref *ref = nullptr;
	CCARRAY_FOREACH(getCourseData()->getPointList(), ref) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
		auto p = static_cast<agtk::data::OthersData::Point*>(ref);
#else
		auto p = dynamic_cast<agtk::data::OthersData::Point*>(ref);
#endif

		if (p->getId() == pointId) {
			return p;
		}
	}
	return nullptr;
}

OthersCourse::EnumCourseType OthersCircleCourse::getCourseType()
{
	return kCourseTypeCircle;
}

#ifdef USE_PREVIEW
void OthersCircleCourse::showDebugVisible(bool isShow)
{
	OthersCourse::showDebugVisible(isShow, this, cocos2d::Color4F(
		getCourseData()->getR() / 255.0f,
		getCourseData()->getG() / 255.0f,
		getCourseData()->getB() / 255.0f,
		getCourseData()->getA() / 255.0f)
	);
}
#endif

//-------------------------------------------------------------------------------------------------------------------
OthersLoopCourse::OthersLoopCourse()
{
	_loopData = nullptr;
	_slopeList = nullptr;
}

OthersLoopCourse::~OthersLoopCourse()
{
	CC_SAFE_RELEASE_NULL(_loopData);
	CC_SAFE_RELEASE_NULL(_slopeList);
}

bool OthersLoopCourse::init(int id, agtk::data::OthersLoopData* loopData, agtk::data::SceneData* sceneData)
{
	// 基底を呼び出す
	OthersCourse::init(id);

	setLoopData(loopData);

	// コースの初期化
	auto sine = sinf(loopData->getRotation() * M_PI / 180.0f);
	auto cosine = cosf(loopData->getRotation() * M_PI / 180.0f);
	SplineInterp splineX, splineY;
	int count = 0;
	splineX.AddKey(count, loopData->getStartX());
	splineY.AddKey(count, loopData->getStartY());
	count++;
	splineX.AddKey(count, loopData->getX() - loopData->getRadius() * sine);
	splineY.AddKey(count, loopData->getY() + loopData->getRadius() * cosine);
	count++;
	splineX.AddKey(count, loopData->getX() + loopData->getRadius() * cosine);
	splineY.AddKey(count, loopData->getY() + loopData->getRadius() * sine);
	count++;
	splineX.AddKey(count, loopData->getX() + loopData->getRadius() * sine);
	splineY.AddKey(count, loopData->getY() - loopData->getRadius() * cosine);
	count++;
	splineX.AddKey(count, loopData->getX() - loopData->getRadius() * cosine);
	splineY.AddKey(count, loopData->getY() - loopData->getRadius() * sine);
	count++;
	splineX.AddKey(count, loopData->getX() - loopData->getRadius() * sine);
	splineY.AddKey(count, loopData->getY() + loopData->getRadius() * cosine);
	count++;
	splineX.AddKey(count, loopData->getEndX());
	splineY.AddKey(count, loopData->getEndY());

	int points = (7 - 1) * OthersCourse::CURVE_DIVS + 1;

	float minX = loopData->getStartX();
	float maxX = loopData->getStartX();
	float minY = loopData->getStartY();
	float maxY = loopData->getStartY();

	for (int i = 0; i < points; i++) {

		float t = (float)i / OthersCourse::CURVE_DIVS;
		float x0 = splineX.GetInterpolated(t);
		float y0 = splineY.GetInterpolated(t);

		auto coursePoint = CoursePoint::create();


		coursePoint->coursePointId = i + 1;
		coursePoint->reverseCoursePointId = i + 1;

		coursePoint->point.x = x0;
		coursePoint->point.y = y0;

		if (x0 < minX) {
			minX = x0;
		}
		else if (maxX < x0) {
			maxX = x0;
		}

		if (y0 < minY) {
			minY = y0;
		}
		else if (maxY < y0) {
			maxY = y0;
		}

		coursePoint->point = agtk::Scene::getPositionSceneFromCocos2d(coursePoint->point, sceneData);
		getCoursePointList()->addObject(coursePoint);
	}

	_loopCount = 1;
	_loopUnlimited = false;
	_reverseMove = false;
	_connectEndStart = false;

	// 全長を算出する
	OthersCourse::CalcLength();

	// 当たり用矩形の設定
	_rect = Rect(minX, minY, maxX - minX, maxY - minY);

	// ループを通行できなかった時の坂を生成
	auto slopeList = cocos2d::Array::create();

	int cnt = 0;

	auto courseList = getCoursePointList();
	for (int i = 0; i < getCoursePointList()->count() - 4; i += 4) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
		auto p0 = static_cast<CoursePoint*>(getCoursePointList()->getObjectAtIndex(i));
		auto p1 = static_cast<CoursePoint*>(getCoursePointList()->getObjectAtIndex(i + 4));
#else
		auto p0 = dynamic_cast<CoursePoint*>(getCoursePointList()->getObjectAtIndex(i));
		auto p1 = dynamic_cast<CoursePoint*>(getCoursePointList()->getObjectAtIndex(i + 4));
#endif

		bool passableUpper = false;

		if (4 <= cnt && cnt < 8) {
			passableUpper = true;
		}

		auto slope = agtk::Slope::create(
			agtk::Scene::getPositionCocos2dFromScene(p0->point, sceneData),
			agtk::Scene::getPositionCocos2dFromScene(p1->point, sceneData),
			passableUpper,
			false
		);

		this->addChild(slope);

		// 最初と最後の坂だけ移動可能にする
		if (cnt == 0 || cnt == 11) {
			slope->setMove(true);
		}

		cnt++;

		slopeList->addObject(slope);
	}

	this->setSlopeList(slopeList);

	return true;
}

Vec2 OthersLoopCourse::moveCourse(agtk::ObjectLoopMove* move, float timeScale, float moveSpeed, float& rotation, bool& isFinish)
{
	Vec2 pos = Vec2::ZERO;

	// 移動量設定
	float moveVal = moveSpeed * timeScale;

	// 現在の移動距離を取得
	float dist = move->getMoveDist();

	while (true) {
		float m = moveVal;
		isFinish = false;

		// 始点から終点への移動の場合
		if (!move->getReverseMove()) {
			// 現在の移動距離と移動量を足した値がコースの全長を超えた場合
			if (dist + moveVal > _length) {
				m = _length - dist;
				isFinish = true;
			}

			// 移動量分だけ移動距離を増加
			dist += m;
			// 移動量分だけ残り移動量を減らす
			moveVal -= m;

			float d = 0;
			for (int i = 0; i < getCoursePointList()->count(); i++) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
				auto p = static_cast<CoursePoint*>(getCoursePointList()->getObjectAtIndex(i));
#else
				auto p = dynamic_cast<CoursePoint*>(getCoursePointList()->getObjectAtIndex(i));
#endif

				// 移動先のポイント
				if (d <= dist && dist <= d + p->nextLength) {
					float per = (dist - d) / (p->nextLength);

					int nextIdx = i + 1;
					if (nextIdx >= getCoursePointList()->count()) {
						nextIdx = i;
					}
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
					auto next = static_cast<CoursePoint*>(getCoursePointList()->getObjectAtIndex(nextIdx));
#else
					auto next = dynamic_cast<CoursePoint*>(getCoursePointList()->getObjectAtIndex(nextIdx));
#endif

					pos.x = AGTK_LINEAR_INTERPOLATE(p->point.x, next->point.x, 1.0f, per);
					pos.y = AGTK_LINEAR_INTERPOLATE(p->point.y, next->point.y, 1.0f, per);

					// コースの角度を算出
					Vec2 moveVec = next->point - p->point;
					Vec2 vec = Vec2(1, 0);

					float dot = vec.dot(moveVec.getNormalized());
					rotation = RadianToDegree(acosf(dot));
					if (moveVec.y >= 0) {
						rotation *= -1;
					}

					// 現在のポイントIDXを更新する
					move->setCurrentPointIdx(i);

					// 終端到達時
					if (isFinish) {
						// これ以上移動できないようにする
						moveVal = 0;
					}

					break;
				}
				else {
					d += p->nextLength;
				}
			}
		}
		// 終点から始点への移動の場合
		else {

			// コースの始点を超えた場合
			if ((dist - moveVal) <= 0) {
				m = dist;
				isFinish = true;
			}

			// 移動量分だけ移動距離を増加
			dist -= m;
			// 移動量分だけ残り移動量を減らす
			moveVal -= m;
			
			float d = _length;
			for (int i = getCoursePointList()->count() - 1; i >= 0; i--) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
				auto p = static_cast<CoursePoint*>(getCoursePointList()->getObjectAtIndex(i));
#else
				auto p = dynamic_cast<CoursePoint*>(getCoursePointList()->getObjectAtIndex(i));
#endif

				if (i == 0) {
					d = 0;
				}

				if (d - p->prevLength <= dist && dist <= d) {
					// 始点をチェックする場合前のポイントの長さがないため
					// 無理矢理0を設定している
					float per = (i == 0) ? 0.0f : (p->prevLength - ((_length - dist) - (_length - d))) / p->prevLength;

					int prevIdx = i - 1;
					if (prevIdx < 0) {
						prevIdx = 0;
					}

// #AGTK-NX
#ifdef STATIC_DOWN_CAST
					auto prev = static_cast<CoursePoint*>(getCoursePointList()->getObjectAtIndex(prevIdx));
#else
					auto prev = dynamic_cast<CoursePoint*>(getCoursePointList()->getObjectAtIndex(prevIdx));
#endif

					pos.x = AGTK_LINEAR_INTERPOLATE(prev->point.x, p->point.x, 1.0f, per);
					pos.y = AGTK_LINEAR_INTERPOLATE(prev->point.y, p->point.y, 1.0f, per);

					// コースの角度を算出
					Vec2 moveVec = p->point - prev->point;
					Vec2 vec = Vec2(1, 0);

					float dot = vec.dot(moveVec.getNormalized());
					rotation = RadianToDegree(acosf(dot));
					if (moveVec.y >= 0) {
						rotation *= -1;
					}

					// 現在のポイントIDXを更新する
					move->setCurrentPointIdx(prevIdx);

					// 始点到達時
					if (isFinish) {
						// これ以上移動できないようにする
						moveVal = 0;
					}

					break;
				}
				else
				{
					d -= p->prevLength;
				}
			}
		}

		// 残り移動距離が0になったら処理終了
		if (moveVal <= 0) { break; }
	}

	// 現在の移動距離を更新
	move->setMoveDist(dist);

	return pos;
}

bool OthersLoopCourse::checkHitEnter(agtk::Object* object, cocos2d::Point boundMin, cocos2d::Point boundMax)
{
	Vec2 currentPos = agtk::Scene::getPositionSceneFromCocos2d(object->getPosition());
	Vec2 prevPos = agtk::Scene::getPositionSceneFromCocos2d(object->getOldPosition());

	// オブジェクトの進行方向のベクトルを取得
	Vec2 moveVec = currentPos - prevPos;

	// 横への移動がない場合は確認しない
	if (moveVec.x == 0.0f) {
		return false;
	}

	// 始点情報を取得
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
	auto point0 = static_cast<CoursePoint*>(getCoursePointList()->getObjectAtIndex(0));
	auto point1 = static_cast<CoursePoint*>(getCoursePointList()->getObjectAtIndex(1));
	auto point2 = static_cast<CoursePoint*>(getCoursePointList()->getObjectAtIndex(OthersCourse::CURVE_DIVS / 2));
#else
	auto point0 = dynamic_cast<CoursePoint*>(getCoursePointList()->getObjectAtIndex(0));
	auto point1 = dynamic_cast<CoursePoint*>(getCoursePointList()->getObjectAtIndex(1));
	auto point2 = dynamic_cast<CoursePoint*>(getCoursePointList()->getObjectAtIndex(OthersCourse::CURVE_DIVS / 2));
#endif

	// コースの進行方向のベクトルを取得取得
	Vec2 pointVec = point1->point - point0->point;

	// 入口の進むタイプを設定
	_moveToRightEnter = (pointVec.x > 0);

	// 左から通行不可でコースのベクトルが方向の場合は通行不可
	if (getLoopData()->getDisabledFromLeft() && _moveToRightEnter) {
		return false;
	}

	// 右から通行不可でコースのベクトルが左方向の場合は通行不可
	if (getLoopData()->getDisabledFromRight() && !_moveToRightEnter) {
		return false;
	}

	float dot = moveVec.getNormalized().dot(pointVec.getNormalized());

	// コースの進行方向と同じ方向に進む場合のみチェックする
	if (dot > 0) {

		// 現在の位置のオブジェクトの矩形を設定
		cocos2d::Rect currentRect = Rect(boundMin.x, boundMin.y, boundMax.x - boundMin.x, boundMax.y - boundMin.y);
		//過去の位置のオブジェクトの矩形を設定
		cocos2d::Rect prevRect = Rect(currentRect.getMinX() - moveVec.x, currentRect.getMinY() - moveVec.y, currentRect.size.width, currentRect.size.height);

		// 入口のコースの矩形を設定
		cocos2d::Rect enterPointRect = Rect(point0->point.x, point0->point.y, 0, 0);

		if (pointVec.x < 0) {
			enterPointRect.origin.x = point2->point.x;
			enterPointRect.size.width = point0->point.x - point2->point.x;
		}
		else {
			enterPointRect.origin.x = point0->point.x;
			enterPointRect.size.width = point2->point.x - point0->point.x;
		}

		if (pointVec.y < 0) {
			enterPointRect.origin.y = point2->point.y;
			enterPointRect.size.height = point0->point.y - point2->point.y;
		}
		else {
			enterPointRect.origin.y = point0->point.y;
			enterPointRect.size.height = point2->point.y - point0->point.y;
		}


		// 入口のコースの矩形に接触している場合
		if (enterPointRect.intersectsRect(currentRect) ||
			enterPointRect.intersectsRect(prevRect)) {

			// 「ループに必要な移動量を設定」フラグが設定されている場合「
			if (getLoopData()->getNeedMoveFlag()) {

				moveVec.y = 0;

				// 必要な移動量に到達していない場合
				if (moveVec.getLengthSq() < (getLoopData()->getNeedMove() * getLoopData()->getNeedMove())) {
					return false;
				}
			}

			return true;
		}
	}
	else {


	}

	return false;
}

bool OthersLoopCourse::checkHitExit(agtk::Object* object, cocos2d::Point boundMin, cocos2d::Point boundMax)
{
	Vec2 currentPos = agtk::Scene::getPositionSceneFromCocos2d(object->getPosition());
	Vec2 prevPos = agtk::Scene::getPositionSceneFromCocos2d(object->getOldPosition());

	// オブジェクトの進行方向のベクトルを取得
	Vec2 moveVec = currentPos - prevPos;

	// 横への移動がない場合は確認しない
	if (moveVec.x == 0.0f) {
		return false;
	}

	// 終点情報を取得
	int pointMax = getCoursePointList()->count();
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
	auto point0 = static_cast<CoursePoint*>(getCoursePointList()->getObjectAtIndex(pointMax - 1));
	auto point1 = static_cast<CoursePoint*>(getCoursePointList()->getObjectAtIndex(pointMax - 2));
	auto point2 = static_cast<CoursePoint*>(getCoursePointList()->getObjectAtIndex(pointMax - 1 - (OthersCourse::CURVE_DIVS / 2)));
#else
	auto point0 = dynamic_cast<CoursePoint*>(getCoursePointList()->getObjectAtIndex(pointMax - 1));
	auto point1 = dynamic_cast<CoursePoint*>(getCoursePointList()->getObjectAtIndex(pointMax - 2));
	auto point2 = dynamic_cast<CoursePoint*>(getCoursePointList()->getObjectAtIndex(pointMax - 1 - (OthersCourse::CURVE_DIVS / 2)));
#endif

	// コースの進行方向のベクトルを取得取得
	Vec2 pointVec = point1->point - point0->point;


	// 出口の進むタイプを設定
	_moveToRightExit = (pointVec.x > 0);

	// 左から通行不可でコースのベクトルが方向の場合は通行不可
	if (getLoopData()->getDisabledFromLeft() && _moveToRightExit) {
		return false;
	}

	// 右から通行不可でコースのベクトルが左方向の場合は通行不可
	if (getLoopData()->getDisabledFromRight() && !_moveToRightExit) {
		return false;
	}

	float dot = moveVec.getNormalized().dot(pointVec.getNormalized());

	// コースの進行方向と同じ方向に進む場合のみチェックする
	if (dot > 0) {

		// 現在の位置のオブジェクトの矩形を設定
		cocos2d::Rect currentRect = Rect(boundMin.x, boundMin.y, boundMax.x - boundMin.x, boundMax.y - boundMin.y);
		//過去の位置のオブジェクトの矩形を設定
		cocos2d::Rect prevRect = Rect(currentRect.getMinX() - moveVec.x, currentRect.getMinY() - moveVec.y, currentRect.size.width, currentRect.size.height);

		// 出口のコースの矩形を設定
		cocos2d::Rect exitPointRect = Rect(point0->point.x, point0->point.y, 0, 0);

		if (pointVec.x < 0) {
			exitPointRect.origin.x = point2->point.x;
			exitPointRect.size.width = point0->point.x - point2->point.x;
		}
		else {
			exitPointRect.origin.x = point0->point.x;
			exitPointRect.size.width = point2->point.x - point0->point.x;
		}

		if (pointVec.y < 0) {
			exitPointRect.origin.y = point2->point.y;
			exitPointRect.size.height = point0->point.y - point2->point.y;
		}
		else {
			exitPointRect.origin.y = point0->point.y;
			exitPointRect.size.height = point2->point.y - point0->point.y;
		}

		// 出口のコースの矩形に接触している場合
		if (exitPointRect.intersectsRect(currentRect) ||
			exitPointRect.intersectsRect(prevRect)) {

			// 「ループに必要な移動量を設定」フラグが設定されている場合
			if (getLoopData()->getNeedMoveFlag()) {

				moveVec.y = 0;

				// 必要な移動量に到達していない場合
				if (moveVec.getLengthSq() < (getLoopData()->getNeedMove() * getLoopData()->getNeedMove())) {
					return false;
				}
			}

			return true;
		}
	}
	else {


	}

	return false;

}

bool OthersLoopCourse::checkHit(Object* object, Vec2& cross, cocos2d::Point boundMin, cocos2d::Point boundMax, bool& isUpSide)
{
	Vec2 currentPos = agtk::Scene::getPositionSceneFromCocos2d(object->getPosition());
	Vec2 prevPos = agtk::Scene::getPositionSceneFromCocos2d(object->getOldPosition());
	Vec2 moveVec = currentPos - prevPos;
	// 現在の位置のオブジェクトの矩形を設定
	cocos2d::Rect currentRect = Rect(boundMin.x, boundMin.y, boundMax.x - boundMin.x, boundMax.y - boundMin.y);
	// 過去の位置のオブジェクトの矩形を設定
	cocos2d::Rect prevRect = Rect(currentRect.getMinX() - moveVec.x, currentRect.getMinY() - moveVec.y, currentRect.size.width, currentRect.size.height);
	return this->checkHit(object, currentPos, currentRect, prevPos, prevRect, cross, isUpSide);
}

bool OthersLoopCourse::checkHitAhead(Object* object, Vec2& cross, cocos2d::Point boundMin, cocos2d::Point boundMax, bool& isUpSide)
{
	Vec2 currentPos = agtk::Scene::getPositionSceneFromCocos2d(object->getPosition());
	Vec2 prevPos = agtk::Scene::getPositionSceneFromCocos2d(object->getOldPosition());
	Vec2 nextPos = currentPos + (currentPos - prevPos);
	Vec2 moveVec = (nextPos - currentPos);
	// 現在の位置のオブジェクトの矩形を設定
	cocos2d::Rect currentRect = Rect(boundMin.x, boundMin.y, boundMax.x - boundMin.x, boundMax.y - boundMin.y);
	// 進んだ位置のオブジェクトの矩形を設定
	cocos2d::Rect nextRect = Rect(currentRect.getMinX() + moveVec.x, currentRect.getMinY() + moveVec.y, currentRect.size.width, currentRect.size.height);
	return this->checkHit(object, nextPos, nextRect, currentPos, currentRect, cross, isUpSide);
}

bool OthersLoopCourse::checkHit(Object *object, Vec2 currentPos, Rect currentRect, Vec2 prevPos, Rect prevRect, Vec2& cross, bool& isUpSide)
{
	//Vec2 currentPos = agtk::Scene::getPositionSceneFromCocos2d(object->getPosition());
	//Vec2 prevPos = agtk::Scene::getPositionSceneFromCocos2d(object->getOldPosition());

	// オブジェクトの進行方向のベクトルを取得
	Vec2 moveVec = currentPos - prevPos;


	// 現在の位置のオブジェクトの矩形を設定
	//cocos2d::Rect currentRect = Rect(boundMin.x, boundMin.y, boundMax.x - boundMin.x, boundMax.y - boundMin.y);
	//過去の位置のオブジェクトの矩形を設定
	//cocos2d::Rect prevRect = Rect(currentRect.getMinX() - moveVec.x, currentRect.getMinY() - moveVec.y, currentRect.size.width, currentRect.size.height);

	auto cocos2dCurrentRect = currentRect;
	auto cocos2dprevRect = prevRect;

	currentRect = agtk::Scene::getRectSceneFromCocos2d(currentRect);
	prevRect = agtk::Scene::getRectSceneFromCocos2d(prevRect);
	Rect rect;

	bool jumping = object->getJumping();

	// 入口からの移動か出口からの移動かで
	// チェックする順番を変える
	if (!object->getObjectLoopMove()->getReverseMove()) {
		// 入口に接触しているかをチェック
		rect = getEnterCourseRect();
		if (rect.intersectsRect(currentRect))
		{
			for (int i = 0; i < 6; i++) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
				auto slope = static_cast<agtk::Slope*>(getSlopeList()->getObjectAtIndex(i));
#else
				auto slope = dynamic_cast<agtk::Slope*>(getSlopeList()->getObjectAtIndex(i));
#endif

				if (slope->checkHit(currentPos, cocos2dCurrentRect, prevPos, cocos2dprevRect, cross, isUpSide, jumping)) {

					return true;
				}
			}
		}

		// 出口に接触しているかをチェック
		rect = getExitCourseRect();
		if (rect.intersectsRect(currentRect)) {
			for (int i = 6; i < getSlopeList()->count(); i++) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
				auto slope = static_cast<agtk::Slope*>(getSlopeList()->getObjectAtIndex(i));
#else
				auto slope = dynamic_cast<agtk::Slope*>(getSlopeList()->getObjectAtIndex(i));
#endif

				if (slope->checkHit(currentPos, cocos2dCurrentRect, prevPos, cocos2dprevRect, cross, isUpSide, jumping)) {

					return true;
				}

			}
		}
	}
	else {
		// 出口に接触しているかをチェック
		rect = getExitCourseRect();
		if (rect.intersectsRect(currentRect)) {
			for (int i = 6; i < getSlopeList()->count(); i++) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
				auto slope = static_cast<agtk::Slope*>(getSlopeList()->getObjectAtIndex(i));
#else
				auto slope = dynamic_cast<agtk::Slope*>(getSlopeList()->getObjectAtIndex(i));
#endif

				if (slope->checkHit(currentPos, cocos2dCurrentRect, prevPos, cocos2dprevRect, cross, isUpSide, jumping)) {

					return true;
				}

			}
		}

		// 入口に接触しているかをチェック
		rect = getEnterCourseRect();
		if (rect.intersectsRect(currentRect))
		{
			for (int i = 0; i < 6; i++) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
				auto slope = static_cast<agtk::Slope*>(getSlopeList()->getObjectAtIndex(i));
#else
				auto slope = dynamic_cast<agtk::Slope*>(getSlopeList()->getObjectAtIndex(i));
#endif

				if (slope->checkHit(currentPos, cocos2dCurrentRect, prevPos, cocos2dprevRect, cross, isUpSide, jumping)) {

					return true;
				}
			}
		}
	}

	// 円に接触しているかをチェック
	rect = getCircleCourseRect();
	if (rect.intersectsRect(currentRect)) {

		if (!object->getObjectLoopMove()->getReverseMove()) {
			// 始点から円の頂点までを接触チェックする
			for (int i = 0; i < 6; i++) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
				auto slope = static_cast<agtk::Slope*>(getSlopeList()->getObjectAtIndex(i));
#else
				auto slope = dynamic_cast<agtk::Slope*>(getSlopeList()->getObjectAtIndex(i));
#endif

				if (slope->checkHit(currentPos, cocos2dCurrentRect, prevPos, cocos2dprevRect, cross, isUpSide, jumping)) {

					return true;
				}
			}
		}
		else {
			// 終点から円の頂点までを接触チェックする
			for (int i = 6; i < getSlopeList()->count(); i++) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
				auto slope = static_cast<agtk::Slope*>(getSlopeList()->getObjectAtIndex(i));
#else
				auto slope = dynamic_cast<agtk::Slope*>(getSlopeList()->getObjectAtIndex(i));
#endif

				if (slope->checkHit(currentPos, cocos2dCurrentRect, prevPos, cocos2dprevRect, cross, isUpSide, jumping)) {

					return true;
				}

			}
		}
	}
	return false;
}

cocos2d::Rect OthersLoopCourse::getEnterCourseRect()
{
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
	auto p0 = static_cast<agtk::Slope*>(getSlopeList()->getObjectAtIndex(0));
	auto p1 = static_cast<agtk::Slope*>(getSlopeList()->getObjectAtIndex(1));
#else
	auto p0 = dynamic_cast<agtk::Slope*>(getSlopeList()->getObjectAtIndex(0));
	auto p1 = dynamic_cast<agtk::Slope*>(getSlopeList()->getObjectAtIndex(1));
#endif

	Rect rect0 = p0->getRect();
	Rect rect1 = p1->getRect();
	rect0.merge(rect1);

	return rect0;
}


cocos2d::Rect OthersLoopCourse::getExitCourseRect()
{
	int max = getSlopeList()->count();
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
	auto p0 = static_cast<agtk::Slope*>(getSlopeList()->getObjectAtIndex(max - 2));
	auto p1 = static_cast<agtk::Slope*>(getSlopeList()->getObjectAtIndex(max - 1));
#else
	auto p0 = dynamic_cast<agtk::Slope*>(getSlopeList()->getObjectAtIndex(max - 2));
	auto p1 = dynamic_cast<agtk::Slope*>(getSlopeList()->getObjectAtIndex(max - 1));
#endif

	Rect rect0 = p0->getRect();
	Rect rect1 = p1->getRect();
	rect0.merge(rect1);

	return rect0;
}

cocos2d::Rect OthersLoopCourse::getCircleCourseRect()
{
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
	auto p0 = static_cast<agtk::Slope*>(getSlopeList()->getObjectAtIndex(2));
#else
	auto p0 = dynamic_cast<agtk::Slope*>(getSlopeList()->getObjectAtIndex(2));
#endif

	Rect rect = p0->getRect();

	for (int i = 3; i < 10; i++) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
		auto p1 = static_cast<agtk::Slope*>(getSlopeList()->getObjectAtIndex(i));
#else
		auto p1 = dynamic_cast<agtk::Slope*>(getSlopeList()->getObjectAtIndex(i));
#endif
		rect.merge(p1->getRect());
	}

	return rect;
}

OthersCourse::EnumCourseType OthersLoopCourse::getCourseType()
{
	return kCourseTypeLoop;
}

#ifdef USE_PREVIEW
void OthersLoopCourse::showDebugVisible(bool isShow)
{
	OthersCourse::showDebugVisible(isShow, this, cocos2d::Color4F(0.0f, 1.0f, 0.0f, 1.0f));

	/*
	cocos2d::Ref* ref = nullptr;
	CCARRAY_FOREACH(getSlopeList(), ref) {
		auto slope = dynamic_cast<agtk::Slope*>(ref);
		slope->showDebugVisible(isShow);
	}
	*/
}
#endif

NS_AGTK_END