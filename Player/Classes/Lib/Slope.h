#ifndef __SLOPE_H__
#define __SLOPE_H__

#include "Lib/Macros.h"
#include "Lib/Common.h"
#include "Data/OthersData.h"
#include "Data/SceneData.h"
#include "External/collision/CollisionUtils.hpp"
#ifdef USE_MULTITHREAD_OBJECT_UPDATE
#include "Lib/Tile.h"
#endif

NS_AGTK_BEGIN
class Object;
class WallHitInfoGroup;
class Tile;

class AGTKPLAYER_API Slope : public cocos2d::Node
{
public:
	// 坂の種類
	enum Type
	{
		kTypeUp,	// 上り坂（左低右高）
		kTypeDown	// 下り坂（左高右低）
	};

private:
	bool _passableFromUpper;	// 上から通過できるか？
	bool _passableFromLower;	// 下から通過できるか？

	cocos2d::Rect _worldSpaceRect;		// ワールド空間での坂の矩形
	cocos2d::Point _worldStartPoint;	// ワールド空間での坂の始点
	cocos2d::Point _worldEndPoint;		// ワールド空間での坂の終点

	static const float SLIP_MAX_FRAME; // 坂の影響度が最大になるまでのフレーム（30フレームで設定）

protected:
	bool _firstUpdateWorldSpaceRectFlag;	// updateWorldSpaceRect関数の初回更新フラグ
	Object *_objectTmp;

private:
	Slope();
	virtual ~Slope();
	virtual bool init(agtk::data::OthersSlopeData* slopeData, int layerId, agtk::data::SceneData* sceneData);
	virtual bool init(cocos2d::Vec2 p0, cocos2d::Vec2 p1, bool passableUpper, bool passableLower);

	// 指定線分と坂が当たったか？
	bool checkHitLine(CollisionLine* line, cocos2d::Vec2& cross);

	// 指定矩形から指定矩形への矩形と坂が当たったか？
	bool checkHitRect(cocos2d::Rect& current, cocos2d::Rect& prev, bool& isHitPrev);
	
	// 対象位置が坂の上にいるかチェックする
	bool checkUpSide(Vec2& targetPos);
// #AGTK-NX
#if (CC_TARGET_PLATFORM == CC_PLATFORM_NX)
#else
	bool checkUpSide(float x, float y) { return checkUpSide(cocos2d::Vec2(x, y)); }
#endif

	// 坂と接触しているかチェックする（その他パーツ360度ループ用）
	bool checkHit(Vec2 currentPos, Rect currentRect, Vec2 prevPos, Rect prevRect, Vec2& cross, bool& isUpSide, bool jumping);

	// 上り坂（左低右高）の坂の上にいるときの処理
	bool procUpSideUpSlope(Vec2& cross, Vec2& moveVec, Rect& crntRect, Rect& prevRect, bool isSlip, float touchFrame300);

	// 上り坂（左高右低）の坂の下にいるときの処理
	bool procDownSideUpSlope(Vec2& cross, Vec2& moveVec, Rect& crntRect, Rect& prevRect, bool& hitUp, bool& hitDown);

	// 下り坂（左高右低）の坂の上にいるときの処理
	bool procUpSideDownSlope(Vec2& cross, Vec2& moveVec, Rect& crntRect, Rect& prevRect, bool isSlip, float touchFrame300);

	// 下り坂（左高右低）の坂の下にいるときの処理
	bool procDownSideDownSlope(Vec2& cross, Vec2& moveVec, Rect& crntRect, Rect& prevRect, bool& hitUp, bool& hitDown);

	// 坂滑り用の係数を取得
	float getSlipValue();

	// ワールド空間での坂の矩形情報を更新する
	void updateWorldSpaceRect();

public:
	// その他パーツとして使用する坂を生成する
	CREATE_FUNC_PARAM3(Slope, agtk::data::OthersSlopeData*, slopeData, int, layerId, agtk::data::SceneData*, sceneData);
	
	// 360度ループで使用する坂を生成する
	CREATE_FUNC_PARAM4(Slope, cocos2d::Vec2, p0, cocos2d::Vec2, p1, bool, passableUpper, bool, passableLower);

	cocos2d::Point start;
	cocos2d::Point end;

	CC_SYNTHESIZE(bool, _move, Move);	// 移動可能な坂か？
	CC_SYNTHESIZE_RETAIN(agtk::data::OthersSlopeData*, _slopeData, SlopeData);

	void setConnectStartSlope(Slope* slope, bool canMove); // 始点と接続している坂を設定する
	void setConnectEndSlope(Slope* slope, bool canMove); // 終点と接続している坂を設定する

	CC_SYNTHESIZE_READONLY(Slope*, _connectStartSlope, ConnectStartSlope);
	CC_SYNTHESIZE_READONLY(Slope*, _connectEndSlope, ConnectEndSlope);
	CC_SYNTHESIZE_READONLY(bool, _canMoveStartSlope, CanMoveStartSlope);
	CC_SYNTHESIZE_READONLY(bool, _canMoveEndSlope, CanMoveEndSlope);

	CC_SYNTHESIZE_READONLY(cocos2d::Rect, _rect, Rect);
	CC_SYNTHESIZE_READONLY(float, _degree, Degree);
	CC_SYNTHESIZE_READONLY(Type, _type, Type);
	CC_SYNTHESIZE_READONLY(int, _layerId, LayerId);
	CC_SYNTHESIZE(int, _id, Id);	// 坂を区別するために割り振ったID

	cocos2d::Rect getWorldSpaceRect() { return _worldSpaceRect; }
	cocos2d::Point getWorldStartPoint() { return _worldStartPoint; }
	cocos2d::Point getWorldEndPoint() { return _worldEndPoint; }

	void setObjectTmp(agtk::Object *object) { _objectTmp = object; }

	static int _serial;

	bool simpleCheckHit(Rect crntRect, Rect oldRect, Vec2& moveVec, bool touched, bool& touchPassable);
#ifdef USE_MULTITHREAD_OBJECT_UPDATE
	bool checkHit(Rect crntRect, Rect oldRect, Vec2& moveVec, bool touched, bool jumping, bool& hitUp, bool& hitDown, bool& touchPassable, bool isSlip, float touchFrame300, Object *object, std::vector<agtk::Tile *> &tileList); // 通過できるかをチェックする
#else
	bool checkHit(Rect crntRect, Rect oldRect, Vec2& moveVec, bool touched, bool jumping, bool& hitUp, bool& hitDown, bool& touchPassable, bool isSlip, float touchFrame300, Object *object, cocos2d::Array *tileList); // 通過できるかをチェックする
#endif
	bool checkHitRect(Rect& rect, bool bUpdateWorldSpaceRectFlag = false);

	void checkConnect(cocos2d::Array* slopeList);

	// 坂との接触位置とオブジェクトの移動量を基に座標を算出する
	Vec2 calcMovePosition(float moveX, Vec2& startPos, cocos2d::Size& objectSize);
	Vec2 calcMovePosition(float moveX, float moveY, Vec2& startPos, cocos2d::Size& objectSize);

#ifdef USE_MULTITHREAD_OBJECT_UPDATE
	bool checkPushBackX(bool isPlayer, Rect& checkRect, Rect& oldRect, std::vector<agtk::Tile *> &tileList, float moveX, float& pushVal, float correctVal);
	bool checkPushBackY(Rect& checkRect, Rect& oldRect, std::vector<agtk::Tile *> &tileList, float moveY, float& pushVal, float correctVal);
#else
	bool checkPushBackX(bool isPlayer, Rect& checkRect, Rect& oldRect, cocos2d::Array* tileList, float moveX, float& pushVal, float correctVal);
	bool checkPushBackY(Rect& checkRect, Rect& oldRect, cocos2d::Array *tileList, float moveY, float& pushVal, float correctVal);
#endif

	// 坂が指定されたタイルの上部と接続されているか？
	bool checkConnectTileUp(Rect& tileRect, Rect& objectRect, Vec2& moveVec, bool bUpdateWorldSpaceRectFlag = false);

	bool checkConnectTileUp(agtk::Tile *tile);
	bool checkConnectTileDown(agtk::Tile *tile);
	bool checkConnectTileLeft(agtk::Tile *tile);
	bool checkConnectTileRight(agtk::Tile *tile);

	void adjustMoveVecForNotMovingObjectAlongSlope(const Vec2 &point, const Vec2 &iniMoveVec, Vec2 &moveVec);

	// 指定矩形の端から坂の上までの距離を算出
	//（オブジェクトを坂の上にうまく配置するために使用）
	bool calcDistToUpper(float& dist, Rect& rect);

	// 坂との接触チェックで使用するWallHitInfoのIDを取得する
	int getCheckHitWallHitInfoId(WallHitInfoGroup* group, Vec2& moveVec);

	void convertToWorldSpaceVertex4(agtk::Vertex4 &vertex4);
	cocos2d::Rect convertToLayerSpaceRect();

	bool calcNearestDistancePoint(Vec2 p, Vec2& pp, float *distance = nullptr);
	bool calcNearestDistancePoint(float px, float py, Vec2& pp, float *distance = nullptr) { return calcNearestDistancePoint(cocos2d::Vec2(px, py), pp, distance); }

#ifdef USE_PREVIEW
	// デバッグ用表示
	void showDebugVisible(bool isShow);
#endif

	friend class OthersLoopCourse;
};

NS_AGTK_END

#endif // __SLOPE_H__