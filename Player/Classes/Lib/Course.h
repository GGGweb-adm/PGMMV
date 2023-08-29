#ifndef __COURSE_H__
#define __COURSE_H__

#include "Lib/Macros.h"
#include "Data/OthersData.h"
#include "Data/SceneData.h"

NS_AGTK_BEGIN

//-------------------------------------------------------------------------------------------------------------------
class Object;
class ObjectCourseMove;
class ObjectLoopMove;

//-------------------------------------------------------------------------------------------------------------------
class AGTKPLAYER_API OthersCourse : public cocos2d::Node
{
public:
	// コースの種類
	enum EnumCourseType
	{
		kCourseTypeNone,
		kCourseTypeLine,	// 直線
		kCourseTypeCurve,	// 曲線
		kCourseTypeCircle,	// 円
		kCourseTypeLoop,	// 360度ループ
	};

	static const int CURVE_DIVS = 8; // カーブの分割数

protected:

	class CoursePoint : public cocos2d::Ref
	{

	public:
		CoursePoint();
		virtual ~CoursePoint();
		virtual bool init();

	public:
		CREATE_FUNC(CoursePoint);

	public:
		int coursePointId;			// コース上のポイントID
		int reverseCoursePointId;	// 反転移動時のコース上のポイントID
		float nextLength;			// 次のポイントまでの距離
		float prevLength;			// 前のポイントまでの距離
		Point point;				// 座標
	};

protected:
	OthersCourse();
	~OthersCourse();
	bool init(int id);

	// コースの全長を算出
	void CalcLength(); 

	// 指定コースに設定された「スイッチ・変数を変更」イベントを行う
	virtual void changeSwitchVariable(int courseId);

	// 指定コースの移動速度を取得
	virtual float getMoveSpeed(int courseId);

	virtual agtk::data::OthersData::Point *getPoint(int pointId) { return nullptr; }

#ifdef USE_PREVIEW
	// デバッグ用表示
	void showDebugVisible(bool isShow, cocos2d::Node* node, cocos2d::Color4F color);
#endif

private:
	// ループを行うか判定する
	bool checkLoop(int currentLoopCount);

public:
	// コース上を移動する
	Vec2 moveCourse(ObjectCourseMove *move, float timeScale, bool& isReset);

	// 開始ポイント座標の取得
	Vec2 getStartPointPos(const ObjectCourseMove *move);

	// コースの種類を取得
	virtual EnumCourseType getCourseType();	

#ifdef USE_PREVIEW
	// デバッグ用表示
	virtual void showDebugVisible(bool isShow);	
#endif

protected:

	CC_SYNTHESIZE_READONLY(int, _id, Id);										// コースID
	CC_SYNTHESIZE_RETAIN(cocos2d::__Array*, _coursePointList, CoursePointList); // 各ポイントのリスト

	CC_SYNTHESIZE_READONLY(float, _length, Length);						// コースの全長
	CC_SYNTHESIZE_READONLY(bool, _loopUnlimited, LoopUnlimited);		// 無限ループするか？
	CC_SYNTHESIZE_READONLY(int, _loopCount, LoopCount);					// ループ回数
	CC_SYNTHESIZE_READONLY(bool, _reverseMove, ReverseMove);			// 反転移動するか？
	CC_SYNTHESIZE_READONLY(bool, _connectEndStart, ConnectEndStart);	// 始点と終点が接続されているか？
	CC_SYNTHESIZE(bool, _pausedBySwitch, PausedBySwitch);	// スイッチで移動が止まっているか？
};

//-------------------------------------------------------------------------------------------------------------------
class AGTKPLAYER_API OthersLineCourse : public OthersCourse
{
private:
	OthersLineCourse();
	virtual ~OthersLineCourse();
	virtual bool init(int id, agtk::data::OthersLineCourseData* courseData, agtk::data::SceneData* sceneData);

protected:
	// 指定コースに設定された「スイッチ・変数を変更」イベントを行う
	virtual void changeSwitchVariable(int courseId);

	// 指定コースの移動速度を取得
	virtual float getMoveSpeed(int courseId);

	virtual agtk::data::OthersData::Point *getPoint(int pointId);

public:
	CREATE_FUNC_PARAM3(OthersLineCourse, int, id, agtk::data::OthersLineCourseData*, courseData, agtk::data::SceneData*, sceneData);

	// コースの種類を取得
	virtual EnumCourseType getCourseType();

#ifdef USE_PREVIEW
	// デバッグ用表示
	virtual void showDebugVisible(bool isShow);
#endif

public:
	CC_SYNTHESIZE_RETAIN(agtk::data::OthersLineCourseData*, _courseData, CourseData);

};

//-------------------------------------------------------------------------------------------------------------------
class AGTKPLAYER_API OthersCurveCourse : public OthersCourse
{
private:
	OthersCurveCourse();
	virtual ~OthersCurveCourse();
	virtual bool init(int id, agtk::data::OthersCurveCourseData* courseData, agtk::data::SceneData* sceneData);

protected:
	// 指定コースに設定された「スイッチ・変数を変更」イベントを行う
	virtual void changeSwitchVariable(int courseId);

	// 指定コースの移動速度を取得
	virtual float getMoveSpeed(int courseId);

	virtual agtk::data::OthersData::Point *getPoint(int pointId);

public:
	CREATE_FUNC_PARAM3(OthersCurveCourse, int, id, agtk::data::OthersCurveCourseData*, courseData, agtk::data::SceneData*, sceneData);

	// コースの種類を取得
	virtual EnumCourseType getCourseType();

#ifdef USE_PREVIEW
	// デバッグ用表示
	virtual void showDebugVisible(bool isShow);
#endif

public:
	CC_SYNTHESIZE_RETAIN(agtk::data::OthersCurveCourseData*, _courseData, CourseData);
};

//-------------------------------------------------------------------------------------------------------------------
class AGTKPLAYER_API OthersCircleCourse : public OthersCourse
{
private:
	OthersCircleCourse();
	virtual ~OthersCircleCourse();
	virtual bool init(int id, agtk::data::OthersCircleCourseData* courseData, agtk::data::SceneData* sceneData);

protected:
	// 指定コースに設定された「スイッチ・変数を変更」イベントを行う
	virtual void changeSwitchVariable(int courseId);

	// 指定コースの移動速度を取得
	virtual float getMoveSpeed(int courseId);

	virtual agtk::data::OthersData::Point *getPoint(int pointId);

public:
	CREATE_FUNC_PARAM3(OthersCircleCourse, int, id, agtk::data::OthersCircleCourseData*, courseData, agtk::data::SceneData*, sceneData);

	// コースの種類を取得
	virtual EnumCourseType getCourseType();

#ifdef USE_PREVIEW
	// デバッグ用表示
	virtual void showDebugVisible(bool isShow);
#endif

public:
	CC_SYNTHESIZE_RETAIN(agtk::data::OthersCircleCourseData*, _courseData, CourseData);
};


//-------------------------------------------------------------------------------------------------------------------
class AGTKPLAYER_API OthersLoopCourse : public OthersCourse
{
private:
	OthersLoopCourse();
	virtual ~OthersLoopCourse();
	virtual bool init(int id, agtk::data::OthersLoopData* loopData, agtk::data::SceneData* sceneData);

	// 入口付近の坂の矩形を取得
	cocos2d::Rect getEnterCourseRect();
	// 出口付近の坂の矩形を取得
	cocos2d::Rect getExitCourseRect();
	// 円の坂の矩形を取得
	cocos2d::Rect getCircleCourseRect();
	// コースに接触するかをチェック
	bool checkHit(Object *object, Vec2 currentPos, Rect currentRect, Vec2 prevPos, Rect prevRect, Vec2& cross, bool& isUpSide);

public:
	CREATE_FUNC_PARAM3(OthersLoopCourse, int, id, agtk::data::OthersLoopData*, loopData, agtk::data::SceneData*, sceneData);

	// コースの種類を取得取得
	virtual EnumCourseType getCourseType();

#ifdef USE_PREVIEW
	// デバッグ用表示
	virtual void showDebugVisible(bool isShow);
#endif

	// コース上を移動
	Vec2 moveCourse(agtk::ObjectLoopMove* move, float timeScale, float moveSpeed, float& rotation, bool& isFinish);

	// コースの入り口に接触しているかをチェック
	bool checkHitEnter(agtk::Object* object, cocos2d::Point boundMin, cocos2d::Point boundMax);

	// コースの出口に接触しているかをチェック
	bool checkHitExit(agtk::Object* object, cocos2d::Point boundMin, cocos2d::Point boundMax);

	// コースに接触しているかをチェック
	bool checkHit(agtk::Object* object, cocos2d::Vec2& cross, cocos2d::Point boundMin, cocos2d::Point boundMax, bool& isUpSide);

	// 進んだ先でコースに接触するかをチェック
	bool checkHitAhead(agtk::Object* object, cocos2d::Vec2& cross, cocos2d::Point boundMin, cocos2d::Point boundMax, bool& isUpSide);

public:
	CC_SYNTHESIZE_RETAIN(agtk::data::OthersLoopData*, _loopData, LoopData);	// 360度ループの設定データ
	CC_SYNTHESIZE_READONLY(cocos2d::Rect, _rect, Rect);						// 矩形
	CC_SYNTHESIZE_READONLY(bool, _moveToRightEnter, MoveToRightEnter);		// 入口は右へ進むタイプか？
	CC_SYNTHESIZE_READONLY(bool, _moveToRightExit, MoveToRightExit);		// 出口は右へ進むタイプか？

	CC_SYNTHESIZE_RETAIN(cocos2d::__Array*, _slopeList, SlopeList);			// ループを通行できなかった時に使用する坂一覧
};

NS_AGTK_END

#endif // __COURSE_H__