#ifndef __OBJECT_H__
#define	__OBJECT_H__

#include "Lib/Macros.h"
#include "Lib/Common.h"
#include "Lib/Player.h"
#include "Lib/Player/BasePlayer.h"
#include "Lib/Tile.h"
#include "Lib/Course.h"
#include "Lib/ObjectCommand.h"
#include "Lib/Effect.h"
#include "Lib/Collision.h"
#ifdef USE_MULTITHREAD_OBJECT_UPDATE
#include "Lib/NrArray.h"
#endif
// #AGTK-NX, #AGTK-WIN
#ifdef USE_30FPS_4
#include "Lib/MiddleFrame.h"
#endif
#include "Data/ObjectData.h"
#include "Data/SceneData.h"
#include "Data/AnimationData.h"
#include "Data/PlayData.h"
#include "Manager/PrimitiveManager.h"
#include "Manager/ParticleManager.h"

#include "Manager/AudioManager.h"
NS_AGTK_BEGIN

//-------------------------------------------------------------------------------------------------------------------
class Object;
class ObjectVisible;
#define FIX_ACT2_5401
#ifdef FIX_ACT2_5401
class AGTKPLAYER_API ObjectInvincible : public cocos2d::Ref
{
private:
	ObjectInvincible();
	virtual ~ObjectInvincible();
public:
	CREATE_FUNC_PARAM2(ObjectInvincible, agtk::Object *, object, agtk::data::ObjectInvincibleSettingData *, invincibleSettingData);
	virtual void update(float delta);
	void start();
	void end();
	bool isWallAreaAttack();
	bool isExecuting() { return _executing; }
private:
	virtual bool init(agtk::Object *object, agtk::data::ObjectInvincibleSettingData *invincibleSettingData);
	void setFilterEffect();
	void resetFilterEffect();
private:
	agtk::Object *_object;
	CC_SYNTHESIZE_RETAIN(agtk::data::ObjectInvincibleSettingData *, _invincibleSettingData, InvincibleSettingData);
	CC_SYNTHESIZE(bool, _visible, Visible);
	CC_SYNTHESIZE_RETAIN(cocos2d::__Array *, _bgmIdList, BgmIdList);
	bool _executing;
	float _frame;
};
#endif
class AGTKPLAYER_API ObjectDamageInvincible : public cocos2d::Ref
{
private:
	ObjectDamageInvincible();
	virtual ~ObjectDamageInvincible();
public:
	CREATE_FUNC_PARAM(ObjectDamageInvincible, agtk::Object *, object);
	bool start(bool bSwitchDamageInvincible = false);//点滅開始
	void end();//点滅終了
	void update(float dt);//更新して、visible情報を得る。
	bool isInvincible();//無敵状態チェック
	bool isBlink();//点滅
	bool isExecuting();
	void initCallback(agtk::ObjectVisible *objectVisible);
	bool isInvincibleStartAcceptable();
	void switchChanged(bool projectCommon, int id, bool value);
private:
	virtual bool init(agtk::Object *object);
private:
	agtk::Object *_object;
	CC_SYNTHESIZE_RETAIN(agtk::data::ObjectData *, _objectData, ObjectData);
	float _frame300;
	bool _bInvincible;
	bool _bBlink;
	bool _bUpdate;
	bool _bUpdateFuncFlag;
#ifdef FIX_ACT2_5401
#else
	int _updateCount;
	int _startCount;
#endif
	CC_SYNTHESIZE_READONLY(bool, _switchDamageInvincible, SwitchDamageInvincible);
#ifdef FIX_ACT2_5401
	CC_SYNTHESIZE_RETAIN(ObjectInvincible *, _objectInvincible, ObjectInvincible);
#endif
	std::list<std::pair<agtk::data::PlaySwitchData *, int>> _switchChangedCallbackIdKeyList;
	bool _isInvincibleStartAcceptable;	//trueのとき、無敵開始受け入れ可能：現在の無敵を終了して、新たな無敵を開始させることができる。
};

//-------------------------------------------------------------------------------------------------------------------
#ifdef FIX_ACT2_5401
#else
class AGTKPLAYER_API ObjectInvincible : public cocos2d::Ref
{
private:
	ObjectInvincible();
	virtual ~ObjectInvincible();
public:
	CREATE_FUNC_PARAM2(ObjectInvincible, agtk::Object *, object, agtk::data::ObjectInvincibleSettingData *, invincibleSettingData);
	virtual void update(float delta);
	void start();
	void end();
	bool isWallAreaAttack();
	bool isExecuting() { return _executing; }
private:
	virtual bool init(agtk::Object *object, agtk::data::ObjectInvincibleSettingData *invincibleSettingData);
	void setFilterEffect();
	void resetFilterEffect();
private:
	agtk::Object *_object;
	CC_SYNTHESIZE_RETAIN(agtk::data::ObjectInvincibleSettingData *, _invincibleSettingData, InvincibleSettingData);
	CC_SYNTHESIZE(bool, _visible, Visible);
	CC_SYNTHESIZE_RETAIN(cocos2d::__Array *, _bgmIdList, BgmIdList);
	bool _executing;
	float _frame;
};
#endif

//-------------------------------------------------------------------------------------------------------------------
class AGTKPLAYER_API ObjectVisible : public cocos2d::Ref
{
public:
	class VisibleTimer : public agtk::EventTimer {
	private:
		VisibleTimer() : agtk::EventTimer() {};
		virtual ~VisibleTimer() {};
	public:
		CREATE_FUNC(VisibleTimer);
		void start(bool visible, bool nowVisible, float seconds) {
			agtk::EventTimer::start(seconds);
			_visible = visible;
			_preVisible = nowVisible;
		}
		virtual void end() {
			agtk::EventTimer::end();
			_visible = _preVisible;
		}
	private:
		virtual bool init() {
			if (agtk::EventTimer::init() == false) {
				return false;
			}
			_visible = true;
			_preVisible = true;
			return true;
		}
	private:
		CC_SYNTHESIZE_READONLY(bool, _visible, Visible);
		bool _preVisible;
	};
private:
	ObjectVisible();
	virtual ~ObjectVisible();
public:
	CREATE_FUNC_PARAM2(ObjectVisible, bool, visible, agtk::Object *, object);
	void update(float dt);
	void start(bool visible, float seconds);
	void end();
	bool isWallAreaAttackWhenInvincible();
	bool isInvincible();
	void startBlink(float interval, float seconds);
	void endBlink(float seconds);
	bool isBlinking();
	void takeOverBlink(ObjectVisible *objectVisible);
private:
	virtual bool init(bool visible, agtk::Object *object);
private:
	CC_SYNTHESIZE(bool, _visible, Visible);
	CC_SYNTHESIZE_RETAIN(VisibleTimer *, _visibleTimer, VisibleTimer);
	CC_SYNTHESIZE_RETAIN(ObjectDamageInvincible *, _objectDamageInvincible, ObjectDamageInvincible);
	CC_SYNTHESIZE_RETAIN(cocos2d::__Array *, _objectInvincibleList, ObjectInvincibleList);
	bool _blinking;
	bool _blinkVisible;
	bool _blinkForceVisible;
	float _blinkInterval;
	float _blinkTime;
	float _blinkTargetInterval;
	float _blinkTargetDuration;
	float _blinkTargetTime;
	bool _blinkEndOnTargetReached;
	bool _blinkForceVisibleOnTargetReached;
};

//-------------------------------------------------------------------------------------------------------------------
class AGTKPLAYER_API ObjectDamaged : public cocos2d::Ref
{
private:
	ObjectDamaged();
	virtual ~ObjectDamaged();
public:
	CREATE_FUNC_PARAM2(ObjectDamaged, agtk::Object *, object, agtk::data::ObjectDamagedSettingData *, damagedSettingData);
	void update(float dt);
	void start(agtk::Object *attackObject);
	void end();
	bool getDamage(agtk::Object *attackObject, double& damage, bool& bCriticalDamaged, agtk::data::PlayObjectData *damageRatePlayObjectData, bool takeOverDamageRate);
	bool checkIgnored(agtk::Object *attackObject);
private:
	virtual bool init(agtk::Object *object, agtk::data::ObjectDamagedSettingData *damagedSettingData);
	void setFilterEffect();
	void resetFilterEffect();
	bool checkAttribute(int attribute);
private:
	agtk::Object *_object;
	CC_SYNTHESIZE_RETAIN(agtk::data::ObjectDamagedSettingData *, _damagedSettingData, DamagedSettingData);
	CC_SYNTHESIZE(bool, _visible, Visible);
	bool _executing;
	float _frame;
// ヒットストップ関連
public:
	void dioStart(agtk::Object *attackObject);	// ヒットストップ開始　オブジェクトのゲームスピードを変更する
};

//-------------------------------------------------------------------------------------------------------------------
class Scene;
class SceneLayer;
class AGTKPLAYER_API ObjectTemplateMove : public cocos2d::Ref
{
public:
	enum EnumState {
		kStateIdle,
		kStateStart,
		kStateExecute,
		kStateEnd,
	};
private:
	ObjectTemplateMove();
	virtual ~ObjectTemplateMove();
public:
	CREATE_FUNC_PARAM(ObjectTemplateMove, agtk::Object *, object);
	virtual void update(float delta);
	void start(agtk::data::ObjectCommandTemplateMoveData *objCommand);
	void end(bool bImmediate = false);
	bool isIgnoredObjectWall();//オブジェクトの壁判定
	bool isIgnoredTileWall();//タイルの壁判定
	bool isMoving() { return this->getState() != kStateIdle; };//移動中。
	agtk::SceneLayer *getSceneLayer();
public:
	void resetMoveInfo();
	void setMoveInfo(bool locked, cocos2d::Vec2 direction);
	bool& locked();//moveInfo.locked
	cocos2d::Vec2& direction();//moveInfo.direction
	bool isBoundMove();
	agtk::data::ObjectCommandTemplateMoveData::EnumMoveType getMoveType();
	void endFrameCount() { _frameCount = _frameCountMax; }
private:
	virtual bool init(agtk::Object *object);
	void updateMoveHorizontal(float dt);//左右移動
	void updateMoveVertical(float dt);//上下移動
	void updateMoveBound(float dt);//バウンド
	void updateMoveRandom(float dt);//ランダム移動
	void updateMoveNearObject(float dt);//近くのオブジェクトへ移動
	void updateMoveApartNearObject(float dt);//近くのオブジェクトから離れる
	void updateMoveStop(float dt);//停止
private:
	agtk::Object *getNearObject(int objectGroup, bool lock);
	bool playDirectionMove(double degree, bool bIgnoredRequestPlayAction = false);
	void resetDirection(bool bResetDirection = false);
	cocos2d::Vec2 getDirection();
	cocos2d::Vec2 setDirection(cocos2d::Vec2 v);
	float getDegree();
	float setDegree(float degree);
	float getTimeScale(float dt);
private:
	agtk::Object *_object;
	CC_SYNTHESIZE_RETAIN(agtk::data::ObjectCommandTemplateMoveData *, _objCommand, ObjCommand);
	CC_SYNTHESIZE(EnumState, _state, State);
	bool _moveFlip;//左右移動、上下移動のフリップ
	float _degree, _degreeOld;//移動角度
	cocos2d::Vec2 _direction, _directionOld;//移動方向（バウンドで使用）
	struct {
		bool locked;
		cocos2d::Vec2 direction;
	} _moveInfo;
	CC_SYNTHESIZE(int, _boundColliedWallBit, BoundColliedWallBit);//バウンドで壁にぶつかった時の上下左右のタイルの壁判定の接触情報（移動先でのあたり判定がなくなれば0をセット）
	CC_SYNTHESIZE(bool, _fallFromStepFlag, FallFromStepFlag);
	int _actionFrames;//アクションフレーム数
	CC_SYNTHESIZE(float, _frameCount, FrameCount);
	CC_SYNTHESIZE(float, _frameCountMax, FrameCountMax);
	struct OldCollied {
		struct {
			bool up;
			bool down;
		} wall;
		OldCollied() {
			wall.down = false;
			wall.up = false;
		}
	} _oldCollied;
};

//-------------------------------------------------------------------------------------------------------------------
class Scene;
class CameraObject;
class AGTKPLAYER_API ObjectCourseMove : public cocos2d::Ref
{
private:
	ObjectCourseMove();
	virtual ~ObjectCourseMove();
	virtual bool init(agtk::Object *object, int courseId, int coursePoint, agtk::Scene *scene);
	virtual bool init(CameraObject* cameraObject, int courseId, int coursePointId, agtk::Scene *scene);

public:
	CREATE_FUNC_PARAM4(ObjectCourseMove, agtk::Object *, object, int, courseId, int, coursePointId, agtk::Scene *, scene);
	CREATE_FUNC_PARAM4(ObjectCourseMove, CameraObject*, cameraObject, int, courseId, int, coursePointId, agtk::Scene *, scene);

	virtual void update(float delta);
	void reset();
	bool isMoving(){ return _moving; }
	void setup(const agtk::Object *object);
	Vec2 getStartPointPos();

private:
	agtk::Object *_object;
	bool _moving;

	CC_SYNTHESIZE_RETAIN(agtk::OthersCourse*, _course, Course);

	CC_SYNTHESIZE(bool, _isFirst, IsFirst);					// 初回の移動か？
	CC_SYNTHESIZE(float, _moveDist, MoveDist);				// 移動距離
	CC_SYNTHESIZE(int, _currentPointId, CurrentPointId);	// 現在のポイントID
	CC_SYNTHESIZE(int, _currentPointIdx, CurrentPointIdx);	// 現在のポイントIDX
	CC_SYNTHESIZE(int, _loopCount, LoopCount);				// 現在のループ回数
	CC_SYNTHESIZE(bool, _reverseMove, ReverseMove);			// 反転移動中か？
	CC_SYNTHESIZE(bool, _reverseCourse, ReverseCourse);		// 逆回転
	CC_SYNTHESIZE(cocos2d::Vec2, _move, Move);				// 移動量

	CC_SYNTHESIZE_READONLY(cocos2d::Vec2, _currentPos, CurrentPos); // 現在位置
};

//-------------------------------------------------------------------------------------------------------------------
class AGTKPLAYER_API ObjectLoopMove : public cocos2d::Ref
{
private:
	ObjectLoopMove();
	virtual ~ObjectLoopMove();
	virtual bool init(agtk::Object *object);

public:
	CREATE_FUNC_PARAM(ObjectLoopMove, agtk::Object*, object);

	virtual void update(float delta);

	// 移動開始
	void startMove(agtk::OthersLoopCourse* course, bool isReverse);
	// 移動終了
	void finishMove();

	// 移動中か？
	bool getMoving();
	// 自動移動中か？
	bool getMovingAuto();

private:
	agtk::Object* _object;
	agtk::OthersLoopCourse* _course;
	float _moveSpeed;
	float _rotation;	// コース進入時のオブジェクトの回転値
	bool _isMoveRight;	// 右へ移動するタイプのループか？

	CC_SYNTHESIZE(float, _moveDist, MoveDist);						// 移動距離
	CC_SYNTHESIZE(int, _currentPointIdx, CurrentPointIdx);			// 現在のポイントIDX
	CC_SYNTHESIZE(bool, _reverseMove, ReverseMove);					// 反転移動中か？
	CC_SYNTHESIZE_READONLY(cocos2d::Vec2, _currentPos, CurrentPos); // 現在位置

	CC_SYNTHESIZE(cocos2d::Vec2, _moveVelocity, MoveVelocity);
};

//-------------------------------------------------------------------------------------------------------------------
class EffectAnimation;
class AGTKPLAYER_API ObjectAfterImage : public cocos2d::Ref
{
private:
	class AfterImage : public cocos2d::Ref
	{
	private:
		AfterImage();
		virtual ~AfterImage();

	public:
		CREATE_FUNC_PARAM(AfterImage, agtk::Object *, object);
		virtual bool init(agtk::Object * object);
		void update(float delta);
		void show(); // 残像を表示
		void hide(); // 残像を非表示化
		void setLocalZOrder(int z);
		void stop();
		void draw(cocos2d::Renderer *renderer, cocos2d::Mat4 &m);

	private:
		agtk::Object * _baseObject;
		agtk::Player * _player;
		agtk::EffectAnimation *_effectAnimation;
		agtk::ParticleGroup * _particleGroup;

		int _playerActionNo;	// 
		int _playerActionDirectNo; // 

		float _duration300; // 表示時間

		bool _isUsePlayer; // プレイヤーを使用するか？
		bool _isUseEffect; // エフェクトを使用するか？
		bool _isUseParticle; // パーティクルを使用するか？
		CC_SYNTHESIZE(bool, _visible, Visible);


		void createEffect(); // エフェクトを生成
		void createParticle(); // パーティクルを生成する
	};

private:
	ObjectAfterImage();
	virtual ~ObjectAfterImage();

public:
	CREATE_FUNC_PARAM(ObjectAfterImage, agtk::Object *, object);
	virtual bool init(agtk::Object * object);
	void update(float delta);
	void stop();

	void drawAfterimage(cocos2d::Renderer *renderer, cocos2d::Mat4 &m);
	
private:

	CC_SYNTHESIZE_RETAIN(cocos2d::__Array *, _afterImageList, AfterImageList);

	agtk::Object * _baseObject;
	float _intervalDuration300; // 表示間隔
	float _minIntervalDuration300; // 表示間隔最小値
};

//-------------------------------------------------------------------------------------------------------------------
class AGTKPLAYER_API ObjectSilhouetteImage : public cocos2d::Ref
{
private:
	ObjectSilhouetteImage();
	virtual ~ObjectSilhouetteImage();

public:
	CREATE_FUNC_PARAM(ObjectSilhouetteImage, agtk::Object *, object);
	virtual bool init(agtk::Object * object);
	void update(float delta);
	void setVisible(bool isOn);
	void visit(Renderer *renderer, cocos2d::Mat4 viewMatrix);
	void updateShader();
private:
	agtk::Object * _baseObject;
	agtk::Player * _player;
	int _playerActionNo;	// 
	int _playerActionDirectNo; // 
	CustomCommand _beforeVisit;
	CustomCommand _afterVisit;
};

//-------------------------------------------------------------------------------------------------------------------
// カメラ固定、シーン上下左右接続時にシーンからはみ出た分だけ反対側から出現したようにみせるためのオブジェクト
class AGTKPLAYER_API ObjectSceneLoop : public cocos2d::Node
{
private:
	ObjectSceneLoop();
	virtual ~ObjectSceneLoop();
	virtual bool init(agtk::Object* object);

public:
	CREATE_FUNC_PARAM(ObjectSceneLoop, agtk::Object*, object);
	virtual void update(float delta);

private:
	agtk::Object *_object;

	CC_SYNTHESIZE_RETAIN(agtk::Player*, _player, Player);
};

//-------------------------------------------------------------------------------------------------------------------
class Object;
class AGTKPLAYER_API ObjectWallIntersect : public cocos2d::Ref
{
public:
	enum EnumWallType {
		kWallTypeUp,
		kWallTypeLeft,
		kWallTypeRight,
		kWallTypeDown,
		kWallTypeMax,
	};
private:
	ObjectWallIntersect();
	virtual ~ObjectWallIntersect() {};
public:
	CREATE_FUNC_PARAM2(ObjectWallIntersect, EnumWallType, wallType, cocos2d::Vec2, point);
	int getWallBit();
	static bool getWallIntersect(cocos2d::Rect rect1, cocos2d::Rect rect2, cocos2d::__Array *wallIntersectList1, cocos2d::__Array *wallIntersectList2);
	static bool getWallIntersect(agtk::AreaData *area1, agtk::AreaData *area2, cocos2d::__Array *wallIntersectList1, cocos2d::__Array * wallIntersectList2);
	static bool getWallIntersect(agtk::Vertex4 &v1, agtk::Vertex4 &v2, cocos2d::__Array *wallIntersectList1, cocos2d::__Array *wallIntersectList2);
	static int getWallBit(cocos2d::__Array *wallIntersectList);
private:
	virtual bool init(EnumWallType wallType, cocos2d::Vec2 p);
private:
	CC_SYNTHESIZE_READONLY(EnumWallType, _wallType, WallType);
	CC_SYNTHESIZE_READONLY(cocos2d::Vec2, _point, Point);
};

//-------------------------------------------------------------------------------------------------------------------
#ifdef USE_MULTITHREAD_OBJECT_UPDATE
class LockObjList {
public:
	LockObjList() {

	}
	~LockObjList();
	void Add(Object *obj);
	void Remove(Object *obj);
	class AutoLocker {
	public:
		AutoLocker(LockObjList *lockObjList, Object *obj) : _lockObjList(lockObjList), _object(obj) {
			_lockObjList->Add(_object);
		}
		~AutoLocker() {
			_lockObjList->Remove(_object);
		}
	protected:
		LockObjList *_lockObjList;
		Object *_object;
	};
protected:
	std::vector<Object *> _lockedObjList;
	//todo ロックが戻ってきたときに、wallCollisionGroupを更新させる必要がある。
};
#endif
//-------------------------------------------------------------------------------------------------------------------
class AGTKPLAYER_API ObjectCollision : public cocos2d::Ref
{
public:
	class ObjectWallIntersectTemp : public cocos2d::Ref
	{
	private:
		ObjectWallIntersectTemp();
		virtual ~ObjectWallIntersectTemp();
	public:
		CREATE_FUNC_PARAM2(ObjectWallIntersectTemp, agtk::Object *, object, cocos2d::__Array *, wallList);
	private:
		virtual bool init(agtk::Object *object, cocos2d::__Array *wallList);
	private:
		CC_SYNTHESIZE_READONLY(agtk::Object *, _object, Object);
		CC_SYNTHESIZE_RETAIN(cocos2d::__Array *, _wallList, WallList);
	};

	// 坂当たり確認用矩形情報を格納するためのクラス
	class SlopeCheckRect : public cocos2d::Ref
	{
	private:
		SlopeCheckRect();
		virtual ~SlopeCheckRect();

	public:
		CREATE_FUNC(SlopeCheckRect);

		// 保持している各パラメータをリセット
		void reset(); 
		// 矩形情報を更新
		void updateRect(cocos2d::Rect& rect, agtk::WallHitInfoGroup* wallHitInfoGroup, cocos2d::Vec2& moveVec);

	private:
		virtual bool init();

		// 判定矩形
		CC_SYNTHESIZE(cocos2d::Rect, _rect, Rect); 
		// 各壁当たり矩形の左下と判定矩形の左下との差を格納するリスト
		CC_SYNTHESIZE(std::vector<cocos2d::Vec2>, _wallRectDifferenceList, InfoGroupDifferenceList);
	};

	// タイルの壁判定の接触情報
	struct HitTileWall {
		int bit; // オブジェクトの上下左右どこが接触しているか
		agtk::Tile *tile; // 接触しているタイル
	};
	using HitTileWalls = std::vector<HitTileWall>;

private:
	ObjectCollision();
	virtual ~ObjectCollision();
public:
	CREATE_FUNC_PARAM(ObjectCollision, agtk::Object *, object);
	void update();
	void reset();
#ifdef USE_MULTITHREAD_OBJECT_UPDATE
	cocos2d::Vec2 updateWallHitInfo(float tryX, float tryY, WallHitInfo &wallHitInfo, int *pTileWalBit, int *pAheadTileWallBit, cocos2d::Vec2 move, std::vector<agtk::Tile *> tileList);
	cocos2d::Vec2 newUpdateWallHitInfo(LockObjList &lockObjList, WallHitInfo &wallHitInfo, int *pTileWalBit, HitTileWalls* pLinkConditionTileWall, HitTileWalls* pAheadTileWallBit, int* pSlopeBit, cocos2d::Vec2 move, agtk::MtVector<agtk::Object *>* upObjectList, agtk::MtVector<agtk::Object *>* downObjectList, std::vector<agtk::Tile *> tileList, agtk::MtVector<agtk::Slope *> *slopeList, bool checkSlope);
	void updateWall(LockObjList &lockObjList, float tryX, float tryY, float *moveX, float *moveY, int *pTileWallBit, HitTileWalls* pLinkConditionTileWall, HitTileWalls* pAheadTileWall, int *pSlopeBit, agtk::MtVector<agtk::Object *> *pLeftWallObjectList, agtk::MtVector<agtk::Object *> *pRightWallObjectList, agtk::MtVector<agtk::Object *> *pUpWallObjectList, agtk::MtVector<agtk::Object *> *pDownWallObjectList, bool *buriedInWallFlag, cocos2d::Vec2 &returnedPos);
#else
	cocos2d::Vec2 updateWallHitInfo(float tryX, float tryY, WallHitInfo &wallHitInfo, int *pTileWalBit, int *pAheadTileWallBit, cocos2d::Vec2 move, cocos2d::__Array *tileList);
	cocos2d::Vec2 newUpdateWallHitInfo(WallHitInfo &wallHitInfo, int *pTileWalBit, HitTileWalls* pLinkConditionTileWall, HitTileWalls* pAheadTileWallBit, int* pSlopeBit, cocos2d::Vec2 move, cocos2d::Array* upObjectList, cocos2d::Array* downObjectList, cocos2d::__Array *tileList, cocos2d::__Array *slopeList, bool checkSlope);

	void updateWall(float tryX, float tryY, float *moveX, float *moveY, int *pTileWallBit, HitTileWalls* pLinkConditionTileWall, HitTileWalls* pAheadTileWall, int *pSlopeBit, cocos2d::Array *pLeftWallObjectList, cocos2d::Array *pRightWallObjectList, cocos2d::Array *pUpWallObjectList, cocos2d::Array *pDownWallObjectList, bool *buriedInWallFlag, cocos2d::Vec2 &returnedPos);
#endif

	void updateLoopCourse(); // 360度ループ当たり処理

public:
	void addObject(agtk::Object *object);
	void addHitObject(agtk::Object *object);
	void addWallObject(agtk::Object *object);
	void removeObject(agtk::Object *object);
	void removeHitObject(agtk::Object *object);
	void removeWallObject(agtk::Object *object);

	void initWallHitInfoGroup();
	void updateWallHitInfoGroup();
	void lateUpdateWallHitInfoGroup();
// #AGTK-NX #AGTK-WIN
#ifdef USE_30FPS_3
	void updateMiddleFrameWallHitInfoGroup();
#endif

#ifdef USE_MULTITHREAD_OBJECT_UPDATE
	void pushObject(LockObjList &lockObjList, agtk::WallHitInfoGroup *infoGroup, cocos2d::Vec2& pushVec, cocos2d::Rect& pushRect, agtk::Object* pushObject, bool bPushedMove = false);//オブジェクトを押し出す
	void pushRect(LockObjList &lockObjList, cocos2d::Vec2& pushVec, cocos2d::Rect& rect, MtVector<Object *> *skipObjList);
	float checkPushRect(LockObjList &lockObjList, float move, bool checkX, bool canPushed, cocos2d::Rect& objectRect1, cocos2d::Rect& objectRect2, cocos2d::Rect& checkRect, agtk::Object* pushObject, MtVector<Object *> *skipObjList);
	std::vector<agtk::Tile *> getWallCollisionTileList(cocos2d::Vec2 move, int collideWithTileGroupBit = -1);
#else
	void pushObject(cocos2d::Vec2& pushVec, cocos2d::Rect& pushRect, agtk::Object* pushObject);//オブジェクトを押し出す
	void pushRect(cocos2d::Vec2& pushVec, cocos2d::Rect& rect, cocos2d::Array* skipObjList);
	float checkPushRect(float move, bool checkX, bool canPushed, cocos2d::Rect& objectRect1, cocos2d::Rect& objectRect2, cocos2d::Rect& checkRect, agtk::Object* pushObject, cocos2d::Array* skipObjList);
	cocos2d::__Array *getWallCollisionTileList(cocos2d::Vec2 move, int collideWithTileGroupBit = -1);
#endif
private:
	virtual bool init(agtk::Object *object);
private:
	agtk::Object *_object;
#ifdef USE_MULTITHREAD_OBJECT_UPDATE
	CC_SYNTHESIZE_RETAIN(agtk::NrArray *, _objectList, ObjectList);
#else
	CC_SYNTHESIZE_RETAIN(cocos2d::__Array *, _objectList, ObjectList);
#endif
	CC_SYNTHESIZE_RETAIN(cocos2d::__Array *, _hitObjectList, HitObjectList);
	CC_SYNTHESIZE_RETAIN(cocos2d::__Array *, _wallObjectList, WallObjectList);
	// 衝突したポータルリスト
	CC_SYNTHESIZE_RETAIN(cocos2d::__Array *, _portalList, PortalList);

	// 坂当たり判定用矩形情報
	CC_SYNTHESIZE_RETAIN(SlopeCheckRect *, _slopeCheckRect, SlopeCheckRect);

#ifdef USE_MULTITHREAD_OBJECT_UPDATE
	CC_SYNTHESIZE(agtk::WallHitInfoGroup *, _wallHitInfoGroup, WallHitInfoGroup);
	CC_SYNTHESIZE(agtk::WallHitInfoGroup *, _oldWallHitInfoGroup, OldWallHitInfoGroup);// 前フレームのWallHitInfoGroup
	CC_SYNTHESIZE(agtk::WallHitInfoGroup *, _prevWallHitInfoGroup, PrevWallHitInfoGroup);//壁判定用前フレームのWallHitInfoGroup
#else
	CC_SYNTHESIZE_RETAIN(agtk::WallHitInfoGroup *, _wallHitInfoGroup, WallHitInfoGroup);
	CC_SYNTHESIZE_RETAIN(agtk::WallHitInfoGroup *, _oldWallHitInfoGroup, OldWallHitInfoGroup);// 前フレームのWallHitInfoGroup
	CC_SYNTHESIZE_RETAIN(agtk::WallHitInfoGroup *, _prevWallHitInfoGroup, PrevWallHitInfoGroup);//壁判定用前フレームのWallHitInfoGroup
#endif
	CC_SYNTHESIZE(cocos2d::Vec2, _returnedPos, ReturnedPos);//戻されたフラグ
	CC_SYNTHESIZE(bool, _buriedInWallFlag, BuriedInWallFlag);//壁に埋まったフラグ
#ifdef USE_MULTITHREAD_OBJECT_UPDATE
	CC_SYNTHESIZE_INSTANCE(agtk::MtVector<agtk::Object *> *, _leftWallObjectList, LeftWallObjectList);
	CC_SYNTHESIZE_INSTANCE(agtk::MtVector<agtk::Object *> *, _rightWallObjectList, RightWallObjectList);
	CC_SYNTHESIZE_INSTANCE(agtk::MtVector<agtk::Object *> *, _upWallObjectList, UpWallObjectList);
	CC_SYNTHESIZE_INSTANCE(agtk::MtVector<agtk::Object *> *, _downWallObjectList, DownWallObjectList);
#else
	CC_SYNTHESIZE_RETAIN(cocos2d::Array *, _leftWallObjectList, LeftWallObjectList);
	CC_SYNTHESIZE_RETAIN(cocos2d::Array *, _rightWallObjectList, RightWallObjectList);
	CC_SYNTHESIZE_RETAIN(cocos2d::Array *, _upWallObjectList, UpWallObjectList);
	CC_SYNTHESIZE_RETAIN(cocos2d::Array *, _downWallObjectList, DownWallObjectList);
#endif
	void checkAttackHitWall(float x, float y,int tileGroupBit);// 攻撃判定が壁に当たっているかチェック
};

//-------------------------------------------------------------------------------------------------------------------
class WallHitInfo;
class AGTKPLAYER_API ObjectMovement : public cocos2d::Ref
{
public:
	class MoveElement : public cocos2d::Ref {
	public:
		CREATE_FUNC_PARAM2(MoveElement, cocos2d::Vec2, move, float, rotate);
		cocos2d::Vec2 calcMove() {
			return _move.rotateByAngle(cocos2d::Vec2::ZERO, -_rotate);
		}
		void addMove(cocos2d::Vec2 move) {
			_move += move;
		}
	private:
		virtual bool init(cocos2d::Vec2 move, float rotate) {
			_move = move;
			_rotate = rotate;
			return true;
		}
		CC_SYNTHESIZE(cocos2d::Vec2, _move, Move);
		CC_SYNTHESIZE(float, _rotate, Rotate);
	};
	class Value : public cocos2d::Ref
	{
	private:
		Value() {};
		virtual ~Value() {};
	public:
		CREATE_FUNC_PARAM(Value, float, value);
	public:
		virtual bool init(float value) {
			_initialValue = value;
			_value = value;
			_locked = false;
			_reset = false;
			return true;
		}
		void update() {
			if (_locked == false && _reset == true) {
				_value = _initialValue;
				_reset = false;
			}
		}
		float set(float value) {
			float v = _value;
			_value = value;
			return v;
		}
		float get() {
			return _value;
		}
		void reset(bool bForce = false) {
			_reset = true;
			if (bForce) {
				_locked = false;
				_value = _initialValue;
			}
		}
		void lock() { _locked = true; };
		void unlock() { _locked = false; };
		bool isValue() { return _value != 0.0f ? true : false; };
		bool isLock() { return _locked; };
		float getInitialValue() { return _initialValue; }
#if defined(AGTK_DEBUG)
		void dump() {
			CCLOG("initialValue:%f", _initialValue);
			CCLOG("value:%f", _value);
			CCLOG("locked:%d", _locked);
			CCLOG("reset:%d", _reset);
		}
#endif
	private:
		float _initialValue;
		float _value;
		bool _locked;
		bool _reset;
	};
	//強制移動
	class ForceMove : public cocos2d::Ref {
	public:
		enum EnumType{ kTypeParam, kTypeTime, kTypePushPull };
	public:
		CREATE_FUNC_PARAM(ForceMove, agtk::Object *, object);
		void startTime(cocos2d::Vec2 startPosition, cocos2d::Vec2 endPosition, float speed, bool finalGridMagnet);
		void startParam(cocos2d::Vec2 startPosition, cocos2d::Vec2 endPosition, float changeMoveSpeed, bool finalGridMagnet, bool movePosition, cocos2d::Vec2 direction = cocos2d::Vec2::ZERO);
		void startPushPull(cocos2d::Vec2 startPosition, cocos2d::Vec2 endPosition);
		void end();
		bool isMoving() { return _moving; };
		void update(float dt);
		cocos2d::Vec2 move();
		bool isContinueAction();
	private:
		virtual bool init(agtk::Object *object);
	private:
		CC_SYNTHESIZE(cocos2d::Vec2, _startPosition, StartPosition);
		CC_SYNTHESIZE(cocos2d::Vec2, _endPosition, EndPosition);
		CC_SYNTHESIZE(cocos2d::Vec2, _position, Position);
		CC_SYNTHESIZE(cocos2d::Vec2, _oldPosition, OldPosition);
		CC_SYNTHESIZE(float, _speed, Speed);
		CC_SYNTHESIZE(cocos2d::Vec2, _direction, Direction);
		CC_SYNTHESIZE(cocos2d::Vec2, _inertia, Inertia);
		CC_SYNTHESIZE(bool, _ignoredEndReset, IgnoredEndReset);
		bool _moving;//移動中
		float _range;
		float _length;
		float _seconds;
		float _continueActionFrame;
		CC_SYNTHESIZE(float, _maxSeconds, MaxSeconds);
		CC_SYNTHESIZE(EnumType, _type, Type);
		agtk::Object *_object;
		CC_SYNTHESIZE(float, _changeMoveSpeed, ChangeMoveSpeed);
		bool _finalGridMagnet;
#ifdef FIX_ACT2_5237
		CC_SYNTHESIZE(bool, _followCameraMoving, FollowCameraMoving);
		CC_SYNTHESIZE(cocos2d::Vec2, _targetPosInCamera, TargetPosInCamera);
#endif
// #AGTK-NX #AGTK-WIN
#ifdef USE_30FPS
		CC_SYNTHESIZE(bool, _warpMoved, WarpMoved);
#endif
	};
	class TimerFloat : public agtk::EventTimer
	{
	private:
		TimerFloat();
		virtual ~TimerFloat();
	public:
		CREATE_FUNC_PARAM(TimerFloat, float, value);
	private:
		virtual bool init(float value);
	public:
		float setValue(float value, float seconds = 0.0f);
		float addValue(float value, float seconds = 0.0f);
		float getValue() { return _value; }
		bool isChanged() { return _value != _oldValue ? true : false; }
	private:
		float _value;
		float _nextValue;
		float _prevValue;
		float _oldValue;
	};
private:
	ObjectMovement();
	virtual ~ObjectMovement();
public:
	CREATE_FUNC_PARAM(ObjectMovement, agtk::Object *, object);
public:
	virtual bool init(agtk::Object *object);
	void update(float dt);
	void reset();
	void resetX();
	void resetY();
private:
	cocos2d::Vec2 move(float dt);
	cocos2d::Vec2 jump(float dt);
	cocos2d::Vec2 gravity(float dt);
	float water();
	cocos2d::Vec2 slip(cocos2d::Vec2 v, bool bDirection);
public:
	void startForceMoveTime(cocos2d::Vec2 startPosition, cocos2d::Vec2 endPosition, float speed, bool finalGridMagnet = false);
	void startForceMoveParam(cocos2d::Vec2 startPosition, cocos2d::Vec2 endPosition, float changeMoveSpeed, bool finalGridMagnet, bool movePosition, cocos2d::Vec2 direction = cocos2d::Vec2::ZERO);
	void startForceMovePushPull(cocos2d::Vec2 startPosition, cocos2d::Vec2 endPosition);
	void setForceMoveChangeMoveSpeed(float changeMoveSpeed = 100.0f);
public:
	void setDirection(cocos2d::Vec2 v);
	void setDirectionForce(cocos2d::Vec2 v, bool bIgnoredChangeAction = false);
	void resetDirectionForce();
	cocos2d::Vec2 getDirection();
	cocos2d::Vec2 getDirectionDirect() {
		return _direction;
	}
	void setDisplayDistanceMove(cocos2d::Vec2 direction, float distance);
#ifdef USE_MULTITHREAD_OBJECT_UPDATE
	bool isMoveLift(agtk::MtVector<agtk::Object *> *list = nullptr);
#else
	bool isMoveLift(cocos2d::__Array *list = nullptr);
#endif
	cocos2d::Vec2 getMoveXy() { return cocos2d::Vec2(_moveX, _moveY); };
public:
	void setInputDirection(cocos2d::Vec2 v);
	void setInputDirectionForce(cocos2d::Vec2 v);
	void resetInputDirectionForce();
	cocos2d::Vec2 getInputDirection();
	bool isInputDirectionForceFlag() { return _inputDirectionForceFlag; }
protected:
	void accelTankAndCarMoveVelocity(cocos2d::Vec2 move, float rotate, bool bAccelMove);
	void decelTankAndCarMoveVelocity(bool bAccelMove);
	cocos2d::Vec2 calcTankAndCarMoveVelocity(bool bAccelMove);
	void calcForwardBackwardMoveLength(float &forwardMoveLength, float &backwardMoveLength);
	void updateMoveVelocity();
protected:
	bool isFreeMovingEnabled();
	bool isAcceleration();
	cocos2d::Vec2 calcDiagonalMoveWithPolar(cocos2d::Vec2 v, cocos2d::Vec2 direction);
	void updateObjectTemplateMove(cocos2d::Vec2 move, cocos2d::Vec2 gravity);
	bool checkWallCollision(std::vector<Vertex4>& wallCollisionList, cocos2d::Vec2 move, cocos2d::__Array *collisionTileList, int *wallBit, WallHitInfo& wallHitInfo);
#ifdef USE_MULTITHREAD_OBJECT_UPDATE
	void checkWallAndFloor(bool& hitWall, bool& hitFloor, std::vector<Vertex4> &wallCollisionList, std::vector<Vertex4> &oldWallCollisionList, agtk::MtVector<agtk::Slope *> *slopeList, cocos2d::Vec2& move, cocos2d::Vec2 oldMove, bool& bLeftWallCollied, bool& bRightWallCollied);
#else
	void ObjectMovement::checkWallAndFloor(bool& hitWall, bool& hitFloor, std::vector<Vertex4> &wallCollisionList, std::vector<Vertex4> &oldWallCollisionList, cocos2d::__Array *slopeList, cocos2d::Vec2& move, cocos2d::Vec2& oldMove, bool& bLeftWallCollied, bool& bRightWallCollied);
#endif
	float getMoveSpeedRate() { return this->getMoveSpeed()->getValue() * 0.01f; };
	float getUpDownMoveSpeedRate() { return this->getUpDownMoveSpeed()->getValue() * 0.01f; };
	float getTurnSpeedRate() { return this->getTurnSpeed()->getValue() * 0.01f; };
	float getTimeScale(float dt);
private:
	agtk::Object *_object;
	CC_SYNTHESIZE(float, _moveX, MoveX);
	CC_SYNTHESIZE(float, _moveY, MoveY);
	CC_SYNTHESIZE(cocos2d::Vec2, _moveVelocity, MoveVelocity);//移動量
	CC_SYNTHESIZE_RETAIN(cocos2d::__Array *, _moveVelocityList, MoveVelocityList);//移動量リスト
	CC_SYNTHESIZE(cocos2d::Vec2, _vertVelocity, VertVelocity);//垂直移動量（jump,gravity)
	cocos2d::Vec2 _vertVelocityTemp;//移動量（一時保持）
	cocos2d::Vec2 _moveVelocityTemp;//垂直移動量（一時保持）
	cocos2d::Vec2 _direction;
	cocos2d::Vec2 _directionForce;
	bool _directionForceFlag;
	bool _resetDirectionXFlag;
	bool _resetDirectionYFlag;
	bool _directionForceIgnoredChangeActionFlag;
	CC_SYNTHESIZE_READONLY(cocos2d::Vec2, _directionOld, DirectionOld);
	//CC_SYNTHESIZE(cocos2d::Vec2, _inputDirection, InputDirection);//入力方向
	cocos2d::Vec2 _inputDirection;
	cocos2d::Vec2 _inputDirectionForce;
	bool _inputDirectionForceFlag;
	CC_SYNTHESIZE(cocos2d::Vec2, _slipAcceleration, SlipAcceleration);//スリップ加速度
	CC_SYNTHESIZE(cocos2d::Vec2, _slipVelocity, SlipVelocity);
	CC_SYNTHESIZE(cocos2d::Vec2, _slipVelocityMax, SlipVelocityMax);
	CC_SYNTHESIZE(bool, _ignoredJump, IgnoredJump);//ジャンプ無効
	CC_SYNTHESIZE(bool, _ignoredGravity, IgnoredGravity);//重力無効
	CC_SYNTHESIZE_RETAIN(Value *, _wallMoveSpeed, WallMoveSpeed);//壁移動速度
	CC_SYNTHESIZE_RETAIN(Value *, _wallJump, WallJump);//壁ジャンプ
	CC_SYNTHESIZE_RETAIN(Value *, _wallSlip, WallSlip);//壁滑る
	CC_SYNTHESIZE_RETAIN(Value *, _wallGravityEffect, WallGravityEffect);//壁重力
	CC_SYNTHESIZE_RETAIN(ForceMove *, _forceMove, ForceMove);//強制移動
	CC_SYNTHESIZE_RETAIN(Value *, _wallFriction, WallFriction);//摩擦係数
	CC_SYNTHESIZE_RETAIN(TimerFloat *, _moveSpeed, MoveSpeed);
	CC_SYNTHESIZE_RETAIN(TimerFloat *, _upDownMoveSpeed, UpDownMoveSpeed);
	CC_SYNTHESIZE_RETAIN(TimerFloat *, _turnSpeed, TurnSpeed);
	CC_SYNTHESIZE(float, _wallMoveX, WallMoveX);
	CC_SYNTHESIZE(float, _wallMoveY, WallMoveY);
	CC_SYNTHESIZE(bool, _canMoveLift, CanMoveLift);//リフト移動できる状態か？
#ifdef USE_MULTITHREAD_OBJECT_UPDATE
	CC_SYNTHESIZE_INSTANCE(agtk::MtVector<agtk::Object *> *, _objectMoveLiftList, ObjectMoveLiftList);//リフト移動時の乗っているオブジェクトリスト
#else
	CC_SYNTHESIZE_RETAIN(cocos2d::__Array *, _objectMoveLiftList, ObjectMoveLiftList);//リフト移動時の乗っているオブジェクトリスト
#endif
	CC_SYNTHESIZE(cocos2d::Vec2, _gravity, Gravity);
	CC_SYNTHESIZE(int, _fixedJumpDirectionId, FixedJumpDirectionId);//軌道修正不可な場合の入力方向ID
	CC_SYNTHESIZE(float, _preTimeScale, PreTimeScale);//前フレームのタイムスケール値
	CC_SYNTHESIZE(bool, _timeScaleZeroFlag, TimeScaleZeroFlag);//タイムスケールが０の時（復帰時に元のタイムスケールに戻す）
	CC_SYNTHESIZE(float, _zeroPreTimeScale, ZeroPreTimeScale);//タイムスケールが0になる前のフレームのタイムスケール値
	CC_SYNTHESIZE(float, _jumpDuration, JumpDuration);//制御ジャンプ時の期間（秒）
	CC_SYNTHESIZE(bool, _jumpStartFloor, JumpStartFloor);//ジャンプ開始時の地面接触フラグ
	CC_SYNTHESIZE(bool, _keepJumping, KeepJumping);//２段ジャンプ時にVertVelocityの退避を使う。
	CC_SYNTHESIZE(cocos2d::Vec2, _keepVertVelocity, KeepVertVelocity);//２段ジャンプ時に退避したVertVelocity
	CC_SYNTHESIZE(unsigned int, _moveSideBit, MoveSideBit);//移動sしようとする方向の内、上下左右成分があるかをビットで表現
	CC_SYNTHESIZE(float, _distanceMax, DistanceMax);//最大移動距離。マイナス時は距離制限なし。
	CC_SYNTHESIZE(float, _movedDistance, MovedDistance);//移動した距離。

	//「その他の実行アクション」の「表示方向と同じ方へ移動」
	struct DisplayDistanceMove {
		bool _flag;//実行フラグ(On/Off)
		cocos2d::Vec2 _move;//現在の距離
		float _distanceMax;//最大距離(移動距離）
		DisplayDistanceMove() {
			clear();
		}
		void start(float distance) {
			_flag = true;
			_move = cocos2d::Vec2::ZERO;
			_distanceMax = distance;
		}
		cocos2d::Vec2 add(cocos2d::Vec2 move) {
			float delta = move.getLength();
			float distance = _move.getLength();
			if (_distanceMax >= 0 && distance + delta > _distanceMax) {
				delta = _distanceMax - distance;
			}
			cocos2d::Vec2 m = move.getNormalized() * delta;
			_move += m;
			return m;
		}
		bool isEnd() { return (_distanceMax >= 0 && _move.getLength() >= _distanceMax) ? true : false; }
		bool isExecuting() { return _flag; }
		void clear() {
			_flag = false;
			_move = cocos2d::Vec2::ZERO;
			_distanceMax = 0.0f;
		}
		void end() { clear(); }
	};
	DisplayDistanceMove _displayDistanceMove;
	struct OldCollied {
		struct {
			bool up;
			bool down;
		} wall;
		OldCollied() {
			wall.down = false;
			wall.up = false;
		}
	} _oldCollied;
};

//-------------------------------------------------------------------------------------------------------------------
class Scene;
class SceneLayer;
class ConnectObject;
class EffectAnimation;
class Slope;
class AGTKPLAYER_API Object : public cocos2d::Node
{
public:
	// デフォルトのシーンパーツID(物理用)
	static const int DEFAULT_SCENE_PARTS_ID = 99999998;
#ifdef USE_REDUCE_RENDER_TEXTURE
	// オブジェクトに紐づく表示パーツの優先度
	enum {
		kPartPriorityAfterimage = -10,
		kPartPriorityBackPhysics,
		kPartPriorityBackParticle,
		kPartPriorityBackEffect,
		kPartPriorityBackImage,
		kPartPriorityBackMovie,
		kPartPriorityPlayer = 0,
		kPartPriorityFrontParticle,
		kPartPriorityFrontEffect,
		kPartPriorityFrontImage,
		kPartPriorityFrontMovie,
		kPartPriorityFrontPhysics,
	};
#endif

	enum EnumChangeAction {
		kChangeActionNone,
		kChangeActionSetup,
		kChangeActionUpdate,
	};

	/* 衝突した物理オブジェクトデータ */
	class HitPhysicsObjData : public cocos2d::Ref
	{
	private:
		HitPhysicsObjData() { _physicsBody = nullptr; }
		~HitPhysicsObjData() { CC_SAFE_RELEASE_NULL(_physicsBody); }
	public:
		CREATE_FUNC(HitPhysicsObjData);
	private:
		virtual bool init() { return true; }
		CC_SYNTHESIZE(cocos2d::Vec2, _directionVec, DirectionVec);//衝突した物理オブジェクトへの方向ベクトル
		CC_SYNTHESIZE_RETAIN(cocos2d::PhysicsBody *, _physicsBody, PhysicsBody);//衝突した物理ボディ
	};
protected:
	Object();
	virtual ~Object();
public:
#ifdef USE_REDUCE_RENDER_TEXTURE
	void setRunning(bool running) { _running = running; }
#endif
	static Object *createReappearData(agtk::SceneLayer *sceneLayer, agtk::data::ObjectReappearData *reappearData);
	static Object *(*create)(agtk::SceneLayer *sceneLayer, int objectId, int initialActionId, cocos2d::Vec2 position, cocos2d::Vec2 scale, float rotation, int moveDirectionId, int courseId, int coursePointId, int forceAnimMotionId);
	static void setCreate(Object *(*create)(agtk::SceneLayer *sceneLayer, int objectId, int initialActionId, cocos2d::Vec2 position, cocos2d::Vec2 scale, float rotation, int moveDirectionId, int courseId, int coursePointId, int forceAnimMotionId));
	static Object *_create(agtk::SceneLayer *sceneLayer, int objectId, int initialActionId, cocos2d::Vec2 position, cocos2d::Vec2 scale, float rotation, int moveDirectionId = -1, int courseId = -1, int coursePointId = -1, int forceAnimMotionId = -1);
	static Object *(*createWithSceneDataAndScenePartObjectData)(agtk::SceneLayer *sceneLayer, agtk::data::ScenePartObjectData *scenePartObjectData, int forceObjectId);
	static void setCreateWithSceneDataAndScenePartObjectData(Object *(*createWithSceneDataAndScenePartObjectData)(agtk::SceneLayer *sceneLayer, agtk::data::ScenePartObjectData *scenePartObjectData, int forceObjectId));
	static Object *_createWithSceneDataAndScenePartObjectData(agtk::SceneLayer *sceneLayer, agtk::data::ScenePartObjectData *scenePartObjectData, int forceObjectId = -1);
	static Object *createScenePartObjectData(agtk::SceneLayer *sceneLayer, int objectId, int initialActionId, cocos2d::Vec2 position, cocos2d::Vec2 scale, float rotation, int moveDirectionId = -1, int courseId = -1, int coursePointId = -1, int forceAnimMotionId = -1, agtk::data::ScenePartObjectData* scenePartObjectData = nullptr);
	void initialize(agtk::Scene *scene, agtk::SceneLayer *sceneLayer);
	void finalize(unsigned int removeOption);
#ifdef USE_MULTITHREAD_OBJECT_UPDATE
	virtual void objectUpdateBefore(float dt);
	virtual void objectUpdateWallCollision(float dt);
	virtual void objectUpdateAfter(float dt);
#endif
	virtual void update(float dt);
	virtual const cocos2d::Vec2& getPosition() const;
	virtual const cocos2d::Size& getContentSize() const;
	virtual float getScaleX() const;
	virtual float getScaleY() const;
	virtual float getRotation() const;
	virtual void setPosition(const Vec2 &position);
	virtual void setPosition(float x, float y);
	void addPosition(const Vec2 &v);
	cocos2d::Vec2 getCenterPosition();
	cocos2d::Vec2 getFootPosition();
	cocos2d::Vec2 getLeftDownPosition();
	cocos2d::Rect getRect();
	cocos2d::Vec2 getDispPosition();
	cocos2d::Vec2 getCenterDispPosition();
	void setScale(cocos2d::Vec2 scale);
	void setRotation(float rotation);
	virtual void setVisible(bool visible);
	virtual bool isVisible();
	void updateVisible(float delta);
	int getInstanceId();//インスタンスID
	int getControllerId();//コントローラーID
	int getPlayerId();//プレイヤーID
	int getAttackAttribute();//属性
	bool getDisabled();
	bool setDisabled(bool bDisabled);
	bool isBullet();
	agtk::SceneLayer *getSceneLayer();
	void setSceneLayer(agtk::SceneLayer *sceneLayer);
	void earlyUpdate(float dt);
	void lateUpdate(float dt);	// カメラの座標更新後に呼ばれる更新処理
#define FIX_ACT2_4732
#ifdef FIX_ACT2_4732
	void setMoveAnimDispDirection(int directionId);
#endif
	void updateAsChildBefore(float dt);
	void updateAsChild(float dt); // 子オブジェクトとして更新する
	void updatePortalActivation();// ポータルアクティベーション処理
	float updateDuration(float dt);
	void updateExecActionObjectCreate();
	bool isBuriedInWall();//壁に埋まっている？
	int calcDispDirection(agtk::data::DirectionData *directionData = nullptr);
	void setInputDirectionId(int inputId);
	int getInputDirectionId();
	int getInputDirectionIdOld();
	bool isInvincible();//無敵有無
	void setJumpAction(bool flag = true);
	int getDispDirectionByCurrentDirectionData();
	bool isConnectObject();
	int getActionId(agtk::data::ObjectCommandData *commandData);
	agtk::data::ObjectCommandData *getCommandData(int actionId, int commandDataId);

	void loopVertical(bool fixedCamera);// シーンの上下を繋げる処理
	void loopHorizontal(bool fixedCamera);// シーンの左右を繋げる処理 
#ifdef USE_MULTITHREAD_OBJECT_UPDATE
	bool getWallCollisionUpdateOrder() {
		return _wallCollisionUpdateOrder;
	}
#endif

	float getGameSpeed();
	// シーンレイヤーのスクロール速度の違いでオブジェクト間の座標の基準が異なってしまうのに対し、シーンレイヤー間の座標のズレを計算して返す。
#if (CC_TARGET_PLATFORM != CC_PLATFORM_NX)
	static cocos2d::Vec2 Object::getSceneLayerScrollDiff(agtk::SceneLayer *mySceneLayer, agtk::SceneLayer *targetSceneLayer);
#else
#endif
	
protected:
	virtual bool init(agtk::SceneLayer *sceneLayer, int objectId, int initialActionId, cocos2d::Vec2 position, cocos2d::Vec2 scale, float rotation, int moveDirectionId, int courseId, int coursePointId, int changeAnimMotionId = -1);
	virtual bool initWithSceneDataAndScenePartObjectData(agtk::SceneLayer *sceneLayer, agtk::data::ScenePartObjectData *scenePartObjectData, int forceObjectId);
	bool initObjectActionList(agtk::data::ObjectData *objectData);
	agtk::Player *createPlayer(agtk::data::AnimationData *animationData);
	ObjectAction *setup(int objectActionId, int moveDirectionId, int forceDirectionId = -1, bool bInertia = false);
	ObjectAction *getObjectAction(int id);
	void changePositionInLimitArea(agtk::data::SceneData* sceneData); // 行動範囲制限内に座標を変更する
	void execActionObjectCreate(agtk::data::ObjectCommandData *commandData);//実行アクション「オブジェクトを生成」
	int getAnimDirectionId(agtk::data::ObjectData *objectData, agtk::data::ObjectActionData *objectActionData, int forceAnimMotionId, bool bTakeOverMotion);

public:
	int getMoveDirectionBit();
	int getMoveDirection();
	void setMoveDirection(int direction);
	int getDispDirectionBit();
	int getCurrentDirectionBit();
	int getAnimDirectionId(agtk::data::ObjectData *objectData, agtk::data::ObjectActionData *objectActionData, int forceAnimMotionId = -1);
	agtk::data::DirectionData *getDirectionData(int moveDirectionBit, int dispDirectionBit, agtk::data::MotionData *motionData, agtk::data::DirectionData *directionData);
	int getDispDirection() { return _dispDirection; };
	void setDispDirection(int dispDirection);
	bool isAutoGeneration();
	void setUpdateOneshot(bool loadflg = false);	// _updateOneshotFunctionをセット

public:
// #AGTK-NX
#if defined(USE_PREVIEW) || ((CC_TARGET_PLATFORM == CC_PLATFORM_NX) && defined(AGTK_DEBUG))
//#ifdef USE_PREVIEW
	void setupDebugDisplayArea();
	void setVisibleDebugDisplayArea(agtk::data::TimelineInfoData::EnumTimelineType type, bool bVisible);
	void setVisibleDebugDisplayAreaAll(bool bVisible);
#endif
#ifdef AGTK_DEBUG
	void setVisibleDebugDisplayPlayer(bool bVisible);
#endif
	void updateInputPlayer();
	int updateInputPlayerMoveNormal();
	int updateInputPlayerMoveTankCar();
	void updateInputPlayerDirection();
#if defined(USE_WALL_DEBUG_DISPLAY)
public:
	void createWallDebugDisplay();
	void removeWallDebugDisplay();
	void setVisibleWallDebugDisplay(bool bVisible);
	bool isVisibleWallDebugDisplay();
	void updateWallDebugDisplay(float dt);
#endif
public:
	cocos2d::Node *getAreaNode(agtk::data::TimelineInfoData::EnumTimelineType type = agtk::data::TimelineInfoData::kTimelineMax);
	cocos2d::Node *getAreaNode(int id);
	cocos2d::__Array *getAreaArray(agtk::data::TimelineInfoData::EnumTimelineType type);
	cocos2d::__Array *getAreaArray(cocos2d::Vec2 move, agtk::data::TimelineInfoData::EnumTimelineType type);
	bool getTimelineAreaList(agtk::data::TimelineInfoData::EnumTimelineType type, std::vector<agtk::Vertex4> &vertex4List);
	cocos2d::Rect getAreaRect(int id);
	agtk::AreaData *getAreaData(int id);
	bool getAreaBackSide(int id);
	bool getTimeline(agtk::data::TimelineInfoData::EnumTimelineType type, int id, agtk::Vertex4 &vertex4);
	void getTimelineList(agtk::data::TimelineInfoData::EnumTimelineType type, std::vector<agtk::Vertex4> &vertex4List);
	void updateTimelineListCacheSingle(agtk::data::TimelineInfoData::EnumTimelineType type);
	bool getConnectionPoint(int id, cocos2d::Vec2 *out);//指定の接続点の座標取得
	bool existsArea(int id);
	void getAreaTotalRect(agtk::data::TimelineInfoData::EnumTimelineType type, Point *pos, Size *size);
	void callbackDetactionWallCollision(CollisionNode* collisionObject);
	void removeDetactionWallCollision();
	void wallCollisionCorrection(float x, float y, EnumChangeAction changeAction = kChangeActionNone);
	void updateTileGimmickHitCenterPos(); // オブジェクトの中心に接触したタイルのギミックの更新を行う

	void addChildObject(agtk::Object * child, cocos2d::Vec2 connectOffset, int connectId = -1); // 子オブジェクトを追加する
	void removeChildObject(agtk::Object * child); // オブジェクトを解放する
	void changeParentObject(agtk::Object * parent, cocos2d::Vec2 connectOffset, int connectId); // 親オブジェクトの変更を行う

	void getSwitchInfoCreateConnectObject();
// #AGTK-NX
#ifdef USE_SAR_OPTIMIZE_4
	void checkCreateConnectObject(bool bInnerFunctionCall, bool bUpdateSwitch, agtk::data::PlaySwitchData* updatedSwitch = nullptr);// 接続オブジェクトを生成するかをチェックする
#else
	void checkCreateConnectObject(bool bInnerFunctionCall, bool bUpdateSwitch);// 接続オブジェクトを生成するかをチェックする
#endif
	void unconnectAllConnectObject(); // 全ての接続オブジェクトとの接続を無効化する
	void removeAfterimage(); // 残像の除去
	void clearCollision();
	bool isCollision();
	void updateCollision();
	enum RemoveOption {
		kRemoveOptionKeepChildObjectBit = (1 << 0),
		kRemoveOptionKeepConnectObjectBit = (1 << 1),
		kRemoveOptionKeepOwnParentObjectBit = (1 << 2),
	};
	void removeSelf(bool bDisappearFlag = true, bool bIgnoredReappearCondition = false, unsigned int removeOption = 0);
	void playAction(int actionId, int moveDirectionId, int forceDirectionId = -1);//指定アクションを再生する。
	void playActionTemplateMove(int actionId, int moveDirectionId);
	void pauseAction(float seconds);//アクション一時停止（指定時間中停止）
	void pauseAnimation(float seconds);//アニメーション一時停止（指定時間中停止）

	bool checkAttackableObject(agtk::Object * targetObj); // 攻撃可能なオブジェクトか判定する

	void removeParticles(int targetParticleId = agtk::data::ObjectCommandParticleRemoveData::ALL_PARTICLE);// 自身に付与されているパーティクルを削除する
	void stopEmitteParticles(int targetParticleId, bool isReset = true);// 自身に付与されているパーティクルの発生を停止する

	bool getJumping() { return _jumping; }
	void setJumping(bool flg) { _jumping = flg; }
	void setFalling(bool falling) { _fallingOld = _falling; _falling = falling; }

	void removeCollisionHitAttack(agtk::Object *object);
	void removeCollisionAttackHit(agtk::Object *object);
	void removeCollisionWallWall(agtk::Object *object);
	void setupInertia(bool bInertia);
	void damaged(agtk::Object *object, agtk::data::PlayObjectData *damageRatePlayObjectData);
	void callbackDetectionWallCollision(CollisionNode* collisionObject);
	void updateCollisionWallWallList();
	void updateTimelineListCache();
	static void updateTimelineListCache(cocos2d::Array *objectList);

	//壁に埋まった処理。
	bool getBuriedInWallFlag();
	void setBuriedInWallFlag(bool flag);
	void setBuriedInWallFlag(bool flag, int framesMax);
	void updateBurieInWall();

	class ConnectObjectDispPriority : public cocos2d::Ref
	{
	public:
		CREATE_FUNC_PARAM2(ConnectObjectDispPriority, agtk::Object *, object, bool, bLowerPriority);
	private:
		virtual bool init(agtk::Object *object, bool bLowerPriority) {
			_object = object;
			_lowerPriority = bLowerPriority;
			return true;
		}
	private:
		CC_SYNTHESIZE(bool, _lowerPriority, LowerPriority);
		CC_SYNTHESIZE(agtk::Object *, _object, Object);
	};
	CC_SYNTHESIZE_RETAIN(cocos2d::__Array *, _connectObjectDispPriorityList, ConnectObjectDispPriorityList);
	void addConnectObjectDispPriority(agtk::Object *object, bool bLowerPriority);
	void removeConnectObjectDispPriority(agtk::Object *object);

	void absorbToTileCorners();//タイルの角に吸着する
	bool isExternalValueXy() { return _isExternalValueXyFlag; }
	void retrieveDisplayDirectionVariable();
	void updateDisplayDirectionVariable();

	int getTileWallBit(ObjectCollision::HitTileWalls const & htws) {
		int bit = 0;
		for (auto const & ele : htws) {
			bit |= ele.bit;
		}
		return bit;
	}
	int getLinkConditionTileWallBit() {
		return getTileWallBit(_linkConditionTileWall);
	}
	int getAheadTileWallBit() {
		return getTileWallBit(_aheadTileWall);
	}
	void refreshHpChangeTrigger();
	bool isHpChangeTrigger(agtk::Tile *tile, agtk::data::TileData *tileData, agtk::Slope *slope, agtk::data::OthersSlopeData *slopeData);
#ifdef FIX_ACT2_4774
	int getPortalMoveDispBit();
#endif
	// 上下左右の移動量が異なる時の方向補正
	cocos2d::Vec2 directionCorrection(cocos2d::Vec2 direction);
#ifdef USE_MULTITHREAD_OBJECT_UPDATE
	void lock();
	void unlock() {
		_mutex.unlock();
	}
	std::mutex &getMutex() { return _mutex; }
#endif
#ifdef USE_MULTITHREAD_OBJECT_UPDATE
	void retainWallObjectList();
	void autoReleaseWallObjectList();
	void autoReleaseRetainWallObjectList(agtk::MtVector<agtk::Object *> *leftWallObjectList, agtk::MtVector<agtk::Object *> *rightWallObjectList, agtk::MtVector<agtk::Object *> *upWallObjectList, agtk::MtVector<agtk::Object *> *downWallObjectList);
	void autoReleaseRetainTileWallList(ObjectCollision::HitTileWalls *linkConditionTileWallList, ObjectCollision::HitTileWalls *aheadTileWallList);
#endif
	cocos2d::__Array *getPinAxisConnectedPhysicsBaseList();
protected:
	CC_SYNTHESIZE(unsigned int, _id, Id);
	CC_SYNTHESIZE(unsigned int, _scenePartsId, ScenePartsId);//シーンパーツID
	CC_SYNTHESIZE_RETAIN(agtk::data::SceneData *, _sceneData, SceneData);
	CC_SYNTHESIZE_RETAIN(agtk::data::ScenePartObjectData *, _scenePartObjectData, ScenePartObjectData);
	CC_SYNTHESIZE_RETAIN(agtk::data::ObjectData *, _objectData, ObjectData);
	CC_SYNTHESIZE_RETAIN(agtk::data::ObjectReappearData *, _objectReappearData, ObjectReappearData);
	CC_SYNTHESIZE_RETAIN(ObjectAction *, _currentObjectAction, CurrentObjectAction);
	CC_SYNTHESIZE_RETAIN(cocos2d::__Dictionary *, _objectActionList, ObjectActionList);
	CC_SYNTHESIZE_RETAIN(agtk::ObjectMovement *, _objectMovement, ObjectMovement);
	CC_SYNTHESIZE_RETAIN(agtk::Player *, _player, Player);
	CC_SYNTHESIZE(agtk::BasePlayer *, _basePlayer, BasePlayer);
	CC_SYNTHESIZE(int, _prevObjectActionId, PrevObjectActionId);//遷移前のアクションID
	CC_SYNTHESIZE(int, _currentAnimMotionId, CurrentAnimMotionId);//モーションID
	CC_SYNTHESIZE(int, _takeOverAnimMotionId, TakeOverAnimMotionId);//遷移前のモーションを引き継ぐ時のモーションID
	CC_SYNTHESIZE(int, _resourceSetId, ResourceSetId);//リソースセットID
	CC_SYNTHESIZE(string, _resourceSetName, ResourceSetName);//リソースセット名前
	bool _objectMoveLift;//オブジェクトが上に乗っているフラグ
	bool _jumping;//ジャンプ中フラグ
	bool _falling, _fallingOld;//落下中フラグ
	bool _jumpTop;//ジャンプして頂点になったフラグ
	bool _floor, _floorTile, _floorObject, _floorSlope, _floorOld;//床の上フラグ
	bool _collision;//当たり判定有無
	bool _jumpInputFlag;//ジャンプ入力フラグ
	bool _reJumpFlag;//再ジャンプフラグ
	cocos2d::Vec2 _objectPosition;	//オブジェクトのシーン座標系の位置。アニメプレイヤーは別途位置情報を持つ。
#ifdef FIX_ACT2_4774
	CC_SYNTHESIZE(cocos2d::Vec2, _premoveObjectPosition, PremoveObjectPosition);//移動前のオブジェクト座標
#endif
	cocos2d::Vec2 _objectScale;
	float _objectRotation;
	cocos2d::Size _objectSize;
	bool _innerObjectVisible;
	agtk::SceneLayer *_sceneLayer;
	CC_SYNTHESIZE(bool, _jumpActionFlag, JumpActionFlag);
	CC_SYNTHESIZE(bool, _disappearFlag, DisappearFlag);
	CC_SYNTHESIZE(bool, _forceDisabled, ForceDisabled);
	CC_SYNTHESIZE_RETAIN(agtk::data::PlayObjectData *, _playObjectData, PlayObjectData);
	//collision
	CC_SYNTHESIZE_RETAIN(agtk::ObjectCollision *, _objectCollision, ObjectCollision);
	CC_SYNTHESIZE_RETAIN(agtk::ObjectCollision *, _objectWallCollision, ObjectWallCollision);
	//todo: 回転やスケーリングへに要対応
	CC_SYNTHESIZE(cocos2d::Vec2, _oldWallPosition, OldWallPosition);	//オブジェクトを瞬間移動させた場合は、再設定する必要あり。
	CC_SYNTHESIZE(cocos2d::Vec2, _oldWallSize, OldWallSize);

	CC_SYNTHESIZE(int, _tileWallBit, TileWallBit);//上下左右のタイルの壁判定の接触情報
	CC_SYNTHESIZE(ObjectCollision::HitTileWalls, _linkConditionTileWall, LinkConditionTileWall);//上下左右のタイルの壁判定の接触情報（アクションの条件設定の判定で使用する）
	CC_SYNTHESIZE(ObjectCollision::HitTileWalls, _aheadTileWall, AheadTileWall);//1ドット進んだ先での上下左右のタイルの壁判定の接触情報

	CC_SYNTHESIZE(int, _objectWallBit, ObjectWallBit);//オブジェクトの壁判定の接触情報
	CC_SYNTHESIZE(int, _objectSameLayerWallBit, ObjectSameLayerWallBit);	// 同じレイヤーの壁判定の接触情報
#ifdef USE_MULTITHREAD_OBJECT_UPDATE
	//CC_SYNTHESIZE(int, _prevObjectSameLayerWallBit, PrevObjectSameLayerWallBit);	// 前の同じレイヤーの壁判定の接触情報
	CC_SYNTHESIZE(bool, _changeActionFlag, ChangeActionFlag);						// アクションが変更フラグ
	CC_SYNTHESIZE(cocos2d::Vec2, _oldDispPos, OldDispPos);							// update前の表示座標
	CC_SYNTHESIZE(float, _dt, Dt);
	CC_SYNTHESIZE(bool, _returnFlag, ReturnFlag);
#endif
	CC_SYNTHESIZE(int, _slopeBit, SlopeBit);// 上下の坂接触情報
	CC_SYNTHESIZE(int, _aheadSlopeBit, AheadSlopeBit);//進んだ先の上下の坂接触情報
	CC_SYNTHESIZE(int, _layerId, LayerId);	//自分が置かれているレイヤーID
	//壁に埋まったフラグ
	struct BuriedInWall {
		bool flag;
		bool retainFlag;
		int frames;
		int framesMax;
		BuriedInWall() {
			flag = false;
			retainFlag = false;
			frames = 0;
			framesMax = 0;
		};
	} _buriedInWall;
	CC_SYNTHESIZE(int, _frameCount, FrameCount);
	CC_SYNTHESIZE_RETAIN(cocos2d::__Dictionary *, _switchInfoCreateConnectObjectList, SwitchInfoCreateConnectObjectList);
	CC_SYNTHESIZE(cocos2d::Vec2, _returnedPos, ReturnedPos);//戻されたフラグ
#ifdef USE_MULTITHREAD_OBJECT_UPDATE
	CC_SYNTHESIZE_INSTANCE(agtk::MtVector<agtk::Object *> *, _leftWallObjectList, LeftWallObjectList);
	CC_SYNTHESIZE_INSTANCE(agtk::MtVector<agtk::Object *> *, _rightWallObjectList, RightWallObjectList);
	CC_SYNTHESIZE_INSTANCE(agtk::MtVector<agtk::Object *> *, _upWallObjectList, UpWallObjectList);
	CC_SYNTHESIZE_INSTANCE(agtk::MtVector<agtk::Object *> *, _downWallObjectList, DownWallObjectList);
	CC_SYNTHESIZE_INSTANCE(agtk::MtVector<agtk::Slope *> *, _slopeTouchedList, SlopeTouchedList); // 接触している坂リスト
	CC_SYNTHESIZE_INSTANCE(agtk::MtVector<agtk::Slope *> *, _passableSlopeTouchedList, PassableSlopeTouchedList); // 接触中の通過可能な坂リスト
#else
	CC_SYNTHESIZE_RETAIN(cocos2d::Array *, _leftWallObjectList, LeftWallObjectList);
	CC_SYNTHESIZE_RETAIN(cocos2d::Array *, _rightWallObjectList, RightWallObjectList);
	CC_SYNTHESIZE_RETAIN(cocos2d::Array *, _upWallObjectList, UpWallObjectList);
	CC_SYNTHESIZE_RETAIN(cocos2d::Array *, _downWallObjectList, DownWallObjectList);
	CC_SYNTHESIZE_RETAIN(cocos2d::Array *, _slopeTouchedList, SlopeTouchedList); // 接触している坂リスト
	CC_SYNTHESIZE_RETAIN(cocos2d::Array *, _passableSlopeTouchedList, PassableSlopeTouchedList); // 接触中の通過可能な坂リスト
#endif
	CC_SYNTHESIZE(float, _slopeTouchedFrame, SlopeTouchedFrame); // 坂に接触しているフレーム数
	CC_SYNTHESIZE(double, _variableAttackRate, VariableAttackRate);//攻撃割合（最小・最大攻撃力に対しての割合）

	CC_SYNTHESIZE_RETAIN(cocos2d::__Array *, _damagedList, DamagedList);//被ダメージリスト
	CC_SYNTHESIZE_RETAIN(cocos2d::__Array *, _collisionAttackHitList, CollisionAttackHitList);//MyObject.attack -> OtherObject.hit
	CC_SYNTHESIZE_RETAIN(cocos2d::__Array *, _collisionHitAttackList, CollisionHitAttackList);//MyObject.hit -> OtherObject.attack
	CC_SYNTHESIZE_RETAIN(cocos2d::__Array *, _attackObjectList, AttackObjectList);//攻撃判定に接触しているオブジェクトリスト
	CC_SYNTHESIZE_RETAIN(cocos2d::__Array *, _attackerObjectInstanceIdList, AttackerObjectInstanceIdList);//このオブジェクトにダメージを与えたオブジェクトのインスタンスIDリスト
	CC_SYNTHESIZE(bool, _collisionWallWallChecked, CollisionWallWallChecked); // 自オブジェクトの壁判定と壁判定が触れている他オブジェクトのリストが作成済みか
	CC_SYNTHESIZE_RETAIN(cocos2d::__Array *, _collisionWallWallList, CollisionWallWallList);//自オブジェクトの壁判定と壁判定が触れている他オブジェクトのリスト
	CC_SYNTHESIZE_RETAIN(cocos2d::__Array *, _collisionPortalHitList, CollisionPortalHitList);//MyObject.attack -> OtherObject.hit

	CC_SYNTHESIZE_RETAIN(agtk::Object *, _ownParentObject, OwnParentObject); // 自身の親オブジェクト(ParentObjectだと他のクラスの変数とかぶったのでOwnParentObjectに設定)
	CC_SYNTHESIZE_RETAIN(cocos2d::__Array *, _childrenObjectList, ChildrenObjectList); // 子オブジェクトリスト

	CC_SYNTHESIZE(cocos2d::Vec2, _oldPosition, OldPosition);
	CC_SYNTHESIZE(cocos2d::Vec2, _oldPosition2, OldPosition2);//一回の更新(Object::update)に、壁判定処理(wallCollisionCorrection)で得た位置情報を設定する。
	CC_SYNTHESIZE(cocos2d::Vec2, _oldScale, OldScale);
	CC_SYNTHESIZE(cocos2d::Size, _oldSize, OldSize);
	bool _isExternalValueXyFlag;
#if defined(USE_WALL_DEBUG_DISPLAY)
	CC_SYNTHESIZE_RETAIN(cocos2d::Ref *, _wallDebugDisplay, WallDebugDisplay);
#endif
	CC_SYNTHESIZE(int, _initialActionId, InitialActionId);//初期アクションID
	//移動方向(1:左下,2:下,3:右下,4:左,6:右,7:右上,8:上,9:左上)
	CC_SYNTHESIZE(int, _initialMoveDirection, InitialMoveDirection);//
	int _moveDirection;
	int _moveDirectionOld;
	//表示方向
	int _dispDirection;
	int _dispDirectionOld;
	bool _dispFlipY;
	//CC_SYNTHESIZE(int, _inputDirectionId, InputDirectionId);
	int _inputDirectionId;
	int _inputDirectionIdOld;
	CC_SYNTHESIZE(float, _timeScale, TimeScale);//タイムスケール
	CC_SYNTHESIZE(float, _timeScaleTmp, TimeScaleTmp);//タイムスケール一時保持。マイナス値の場合は未保持状態を表す。updateDuration()でgetTimeScale()からコピーされ、update(dt)の終わりにsetTimeScale()で戻し、未保持状態に戻る。
	CC_SYNTHESIZE_RETAIN(cocos2d::__Array *, _connectObjectList, ConnectObjectList);// 接続したオブジェクトリスト
	CC_SYNTHESIZE(double, _duration, Duration);	//ゲームスピード変更の影響を考慮した、処理させたい時間間隔。
	CC_SYNTHESIZE_RETAIN(agtk::ObjectVisible *, _objectVisible, ObjectVisible);
	CC_SYNTHESIZE_RETAIN(agtk::EventTimer *, _timerPauseAction, TimerPauseAction);
	CC_SYNTHESIZE_RETAIN(agtk::EventTimer *, _timerPauseAnimation, TimerPauseAnimation);
	CC_SYNTHESIZE_RETAIN(agtk::ObjectTemplateMove *, _objectTemplateMove, ObjectTemplateMove);
	CC_SYNTHESIZE_RETAIN(agtk::ObjectCourseMove *, _objectCourseMove, ObjectCourseMove);
	CC_SYNTHESIZE_RETAIN(agtk::ObjectLoopMove*, _objectLoopMove, ObjectLoopMove);
	CC_SYNTHESIZE_RETAIN(agtk::ObjectAfterImage *, _objectAfterImage, ObjectAfterImage);
	CC_SYNTHESIZE(bool, _removeLayerMoveFlag, RemoveLayerMoveFlag);//実行アクション「レイヤー移動」で削除処理。
	CC_SYNTHESIZE_RETAIN(cocos2d::__Array *, _execActionObjectCreateList, ExecActionObjectCreateList);//実行アクション「オブジェクトを生成」リスト
#ifdef USE_REDUCE_RENDER_TEXTURE
	CC_SYNTHESIZE_RETAIN(cocos2d::Node *, _frontPhysicsNode, FrontPhysicsNode);
	CC_SYNTHESIZE_RETAIN(cocos2d::Node *, _backPhysicsNode, BackPhysicsNode);
#endif
	CC_SYNTHESIZE(bool, _pushedbackByObject, PushedbackByObject);//このオブジェクトが現時点で「他のオブジェクトから押し戻される」かどうか。※子オブジェクトが「親オブジェクトから離れず追従する」のときも押し戻されない扱いにしたいために使用。

#define FIX_ACT2_5335
#ifdef FIX_ACT2_5335
	struct FollowPosInfo {
		FollowPosInfo() : _pos(cocos2d::Vec2::ZERO), _dt(0) {}
		cocos2d::Vec2 _pos;
		float _dt;
	};
	std::vector<FollowPosInfo> _parentFollowPosList;
	int _parentFollowPosHead;
	int _parentFollowPosTail;
	float _parentFollowPosWeight;
#else
	int _parentFollowDuration300; // 子オブジェクト時、親オブジェクトを追従する時の待ちフレーム
	cocos2d::Vec2 _parentFollowPos; // 子オブジェクト、親オブジェクトを追従する時の座標
#endif
	CC_SYNTHESIZE(cocos2d::Vec2, _parentFollowPosOffset, ParentFollowPosOffset); // 子オブジェクト、親オブジェクトを追従する時の座標設定時に使用するオフセット
	CC_SYNTHESIZE(int, _parentFollowConnectId, ParentFollowConnectId); // 子オブジェクト、親オブジェクトを追従する時の親オブジェクトの接続点ID

	CC_SYNTHESIZE(cocos2d::Vec2, _objectPosInCamera, ObjectPosInCamera);//カメラからのオブジェクト位置（「カメラとの位置関係を固定」用）

	CC_SYNTHESIZE_RETAIN(cocos2d::Array *, _objectEffectList, ObjectEffectList);// 「オブジェクトにエフェクトを表示」で設定されたエフェクトリスト

	// カメラ固定、シーン上下左右接続時にシーンからはみ出た分だけ反対側から出現したようにみせるためのオブジェクト
	CC_SYNTHESIZE_RETAIN(agtk::ObjectSceneLoop*, _objectSceneLoop, ObjectSceneLoop);

	CC_SYNTHESIZE(bool, _lateRemove, LateRemove); // lateUpdate内でオブジェクト消滅を行うか確認するフラグ

	// ヒットストップ関連
	CC_SYNTHESIZE(float, _dioGameSpeed, DioGameSpeed);					// ヒットストップ用ゲームスピード
	CC_SYNTHESIZE(bool, _dioExecuting, DioExecuting);					// ヒットストップ用実行中判定
	CC_SYNTHESIZE(float, _dioFrame, DioFrame);							// ヒットストップ用経過時間
	CC_SYNTHESIZE(float, _dioEffectDuration, DioEffectDuration);		// ヒットストップ用効果時間
	CC_SYNTHESIZE(bool, _dioParent, DioParent);							// ヒットストップ親を含むか
	CC_SYNTHESIZE(bool, _dioChild, DioChild);							// ヒットストップ子を含むか
#ifdef FIX_ACT2_4774
	int _portalMoveDispBit;	//ポータルの再移動方向BITと論理積と論理積を取るための数値。
#endif
	// ------------------------------------------------------------------
	// ロード後のUpdateで必要となるオブジェクト
	// ※ロードと一緒に呼ばれるのではなく、ロード後のObject::update(float dt)で1回だけ呼ばれているので注意

	std::function<void(void)> _updateOneshotFunction;//設定するとupdate時に一度だけ実行する関数

	// 接続オブジェクトロードリスト
	struct ConnectObjectLoadList {
		int instanceId;
		int settingId;
		int actionId;
		ConnectObjectLoadList() {
			instanceId = 0;
			settingId = 0;
			actionId = -1;
		};
	};

	// コース移動ロードデータ
	class AGTKPLAYER_API CourseMoveLoadData : public cocos2d::Ref{
	public:
		CREATE_FUNC(CourseMoveLoadData);
	private:
		virtual bool init() { return true; }
		CC_SYNTHESIZE(bool, _isFirst, IsFirst);
		CC_SYNTHESIZE(float, _moveDist, MoveDist);
		CC_SYNTHESIZE(int, _currentPointId, CurrentPointId);
		CC_SYNTHESIZE(int, _currentPointIdx, CurrentPointIdx);
		CC_SYNTHESIZE(int, _loopCount, LoopCount);
		CC_SYNTHESIZE(bool, _reverseMove, ReverseMove);
		CC_SYNTHESIZE(bool, _reverseCourse, ReverseCourse);
		CC_SYNTHESIZE(cocos2d::Vec2, _move, Move);
		CC_SYNTHESIZE(cocos2d::Vec2, _currentPos, CurrentPos);
	};

	CC_SYNTHESIZE(std::vector<ConnectObjectLoadList>, _connectObjectLoadList, ConnectObjectLoadList);
	CC_SYNTHESIZE(std::vector<ConnectObjectLoadList>, _connectObjectPortalLoadList, ConnectObjectPortalLoadList);
	CC_SYNTHESIZE(CourseMoveLoadData *, _courseMoveLoadData, CourseMoveLoadData);
public:
	LimitTileSetList* getLimitTileSetList()const { return _limitTileSetList.get(); }
private:
	std::unique_ptr<LimitTileSetList> _limitTileSetList;

public:
	CC_SYNTHESIZE(int, _waitDuration300, WaitDuration300);//待ちフレーム。オブジェクト更新のたびに0にまで減らしていく。マイナス値の場合と、正の値のときはオブジェクト更新を進めない。
	friend class ObjectAction;

	CC_SYNTHESIZE(bool, _isPortalWarped, IsPortalWarped);//ポータルワープを実施したフラグ

#ifdef FIX_ACT2_4774
	CC_SYNTHESIZE(int, _warpedTransitionPortalId, WarpedTransitionPortalId);//ワープしたポータルのID
#else
	const char *_firstTouchedPortalName;//移動直後に触れたポータル名
#endif

	CC_SYNTHESIZE(int, _sceneIdOfFirstCreated, SceneIdOfFirstCreated);//最初に生成されたシーンのID

	CC_SYNTHESIZE(bool, _needAbsorbToTileCorners, NeedAbsorbToTileCorners);//タイルの角への吸着要求フラグ

	bool _bFirstCollisionCheck;//初回時の衝突チェックフラグ
	int _bRequestPlayAction;//アクション再生リクエストフラグ（0：リクエスト無し,1:リクエスト有り,アクション変化無し,2:リクエスト有り、アクション変化あり）
	int _bRequestPlayActionTemplateMove;//テンプレート移動用アクション再生リクエストフラグ（0：リクエスト無し,1:リクエスト有り,アクション変化無し,2:リクエスト有り、アクション変化あり）
	bool _bNoClearCollision;//当たり判定情報をクリアしないフラグ
	bool _bTouchGimmickCounted;// ギミックの接触カウント済みか

	// ------------------------------------------------------------------
	// 物理関係
private:
	CC_SYNTHESIZE(cocos2d::PhysicsMaterial, _physicsMaterial, PhysicsMaterial);//物理オブジェクトマテリアル
	CC_SYNTHESIZE(cocos2d::Vec2, _physicsOffset, PhysicsOffset);//物理オブジェクト配置オフセット
	CC_SYNTHESIZE(cocos2d::Size, _physicsSize, PhysicsSize);//物理オブジェクトのサイズ
	CC_SYNTHESIZE_RETAIN(cocos2d::__Array *, _hitPhysicsObjList, HitPhysicsObjList);//衝突した物理オブジェクトへの方向ベクトル
	CC_SYNTHESIZE_RETAIN(cocos2d::__Array *, _physicsPartsList, PhysicsPartsList);//自身に付随する物理パーツリスト
#ifdef USE_REDUCE_RENDER_TEXTURE
#else
	CC_SYNTHESIZE_RETAIN(cocos2d::__Array *, _drawBackPhysicsPartsList, DrawBackPhysicsPartsList);//自身より奥に描画する物理パーツリスト
	CC_SYNTHESIZE_RETAIN(cocos2d::__Array *, _drawFrontPhysicsPartsList, DrawFrontPhysicsPartsList);//自身より前に描画する物理パーツリスト
#endif
	CC_SYNTHESIZE_RETAIN(cocos2d::Node *, _physicsNode, physicsNode);//物理ノード
	CC_SYNTHESIZE(cocos2d::Vec2, _phsycisNodeOldPosition, PhysicsNodeOldPosition);
	CC_SYNTHESIZE(cocos2d::Vec2, _collisionPushBackVec, CollisionPushBackVec);//衝突による押し戻しベクトル
public:
	void setupPhysicsBody(bool isInit);//物理空間用ボディのセットアップ
	void restorePhysicsGeometry();
	void backupPhysicsGeometry();
private:
	void removeAllPhysicsParts();//自身に付随する物理パーツの全削除
public:
	void setPhyiscBitMask(int bitmask);//物理用ビットマスクの設定
	void setPhysicsBitMask(int layerId, int sceneId);//物理用ビットマスクの設定（メニューレイヤーを考慮）

	// ------------------------------------------------------------------
	//強制表示・非表示。
	enum ForceVisibleState { kIgnore = -1, kNotVisible, kVisible, };//無効=-1、非表示=0、表示=1。
	void setForceVisibleState(ForceVisibleState state);
	ForceVisibleState getForceVisibleState() { return _forceVisibleState; }
	void setWaitDuration300All(int duration300);
	void setForceVisibleStateAll(agtk::Object::ForceVisibleState state);
private:
	ForceVisibleState _forceVisibleState;

public:
	// ------------------------------------------------------------------
	// 重なり演出関係
	static const int OVERLAP_EFFECT_TARGET_TILE = (1<<0);	// 重なり演出：タイルが重なった場合
	static const int OVERLAP_EFFECT_TARGET_OBJ = (1<<1);	// 重なり演出：オブジェクトが重なった場合
	void setOverlapFlag(bool on, int flag) {
		if (on) {
			_overlapFlag |= flag;
		}
		else {
			_overlapFlag &= ~flag;
		}
	}
private:
	bool _isDrawShilhouette;//ベタ塗り(シルエット)タイプフラグ
	bool _isTransparent;//透過タイプフラグ

	unsigned int _effectTargetType;//重なり演出実行タイプフラグ(OVERLAP_EFFECT_TARGET_***)
	CC_SYNTHESIZE_RETAIN(cocos2d::DrawNode *, _transparentMaskNode, TransparentMaskNode);//透過用マスクノード
	CC_SYNTHESIZE_RETAIN(ObjectSilhouetteImage *, _silhouetteNode, SilhouetteNode);//シルエットノード
	int _overlapFlag;// 重なりフラグ
#ifdef USE_MULTITHREAD_OBJECT_UPDATE
#ifdef USE_MULTITHREAD_MEASURE
	bool _destructed;
#endif
	std::mutex _mutex;
	int _wallCollisionUpdateOrder;
	bool _updating;
#endif
	CC_SYNTHESIZE(bool, _playerVisible, PlayerVisible);

	struct PhysicsGeometry {
		PhysicsGeometry() : _x(0), _y(0), _rotation(0) {}
		PhysicsGeometry(float x, float y, float rotation): _x(x), _y(y), _rotation(rotation) {}
		float _x;
		float _y;
		float _rotation;
	};
	std::map<int, PhysicsGeometry> _physicsGeometryMap;
	std::map<int, std::map<int, PhysicsGeometry>> _physicsRopePartsGeometryMap;

public:
	void drawTransparentMaskNode(Renderer *renderer, cocos2d::Mat4 viewMatrix, int targetType);//透過用マスクノードの描画
	void drawSilhouette(Renderer *renderer, cocos2d::Mat4 viewMatrix, int targetType);//シルエット描画
#ifdef USE_REDUCE_RENDER_TEXTURE	//agusa-k
	bool isTransparentMaskSilhouette(int targetType);//targetTypeの透過用マスクノードの描画またはシルエット描画が必要なとき真を返す。
#endif
	void visit(Renderer *renderer, const Mat4& parentTransform, uint32_t parentFlags);
	void updateFixedFrame(int value);
private:
	void setUpArroundCharacterView();//重なり演出設定
protected:
	class HpChangeTriggerInfo {
	public:
		HpChangeTriggerInfo(float _elapsed) :
			elapsed(_elapsed), marked(true) {}
		float elapsed;
		bool marked;
	};
	std::map<std::tuple<int, int, int>, HpChangeTriggerInfo> _hpChangeTriggerInfoMap;	//tuple(id, x, y)

public:
	static int _objectCount;

	// オブジェクトが再生したサウンド情報保持
	CC_SYNTHESIZE_RETAIN(cocos2d::__Dictionary *, _bgmList, BgmList);
	CC_SYNTHESIZE_RETAIN(cocos2d::__Dictionary *, _seList, SeList);
	CC_SYNTHESIZE_RETAIN(cocos2d::__Dictionary *, _voiceList, VoiceList);

	void addBgmList(AudioManager::AudioInfo* audioInfo);
	void addSeList(AudioManager::AudioInfo* audioInfo);
	void addVoiceList(AudioManager::AudioInfo* audioInfo);

	void playBgmObject(int id);
	void playBgmObject(int id, bool loop, int volume, int pan, int pitch, float seconds = 0.0f, float currentTime = 0.0f);
	void playSeObject(int id);
	void playSeObject(int id, bool loop, int volume, int pan, int pitch, float seconds = 0.0f, float currentTime = 0.0f);
	void playVoiceObject(int id);
	void playVoiceObject(int id, bool loop, int volume, int pan, int pitch, float seconds = 0.0f, float currentTime = 0.0f);

	void stopBgmObject(int bgmId, float seconds = 0.0f);
	void stopSeObject(int seId, float seconds = 0.0f);
	void stopVoiceObject(int voiceId, float seconds = 0.0f);
	void stopAllBgmObject(float seconds = 0.0f);
	void stopAllSeObject(float seconds = 0.0f);
	void stopAllVoiceObject(float seconds = 0.0f);

// #AGTK-NX
#ifdef USE_SAR_OPTIMIZE_4
	public:
	CC_SYNTHESIZE(std::vector<intptr_t>*, _watchSwitchList, WatchSwitchList);
	void setupWatchSwitchList();
	void registerSwitchWatcher();
	void unregisterSwitchWatcher();
#endif
// #AGTK-NX, #AGTK-WIN
#ifdef USE_30FPS_1
	public:
	CC_SYNTHESIZE(int, _passedFrameCount, PassedFrameCount); // _passedFramePositionがセットされたObject::_frameCountの値
	CC_SYNTHESIZE(cocos2d::Vec2, _passedFramePosition, PassedFramePosition); // 30FPSの時、60FPSならば前フレームで移動先となっていたはずのオブジェクト位置を保持
#endif
#if (CC_TARGET_PLATFORM == CC_PLATFORM_NX)
#endif
// #AGTK0NX, #AGTK-WIN
#ifdef USE_30FPS_3
public:
	cocos2d::Vec2 _halfMove;
	cocos2d::Vec2 _iniPos;
	bool _tempBuriedInWallFlag;
	cocos2d::Vec2 _tempReturnedPos;
	int _tempTileWallBit;
	ObjectCollision::HitTileWalls _tempLinkConditionTileWall;
	int _tempSlopeBit;
	ObjectCollision::HitTileWalls _tempAheadTileWall;
#ifdef USE_30FPS_4
	bool _bUseMiddleFrame;
	MiddleFrameStock _middleFrameStock;
	void getWallTimelineList(std::vector<agtk::Vertex4> &vertex4List);
#endif
#endif
};

//-------------------------------------------------------------------------------------------------------------------
class AGTKPLAYER_API ConnectObject : public agtk::Object
{
private:
	ConnectObject();
	virtual ~ConnectObject();

public:
	static ConnectObject* (*create)(agtk::Object * object, int connectObjectSettingId, int actionId);
	static void setCreate(ConnectObject* (*create)(agtk::Object * object, int connectObjectSettingId, int actionId));
	static ConnectObject* _create(agtk::Object * object, int connectObjectSettingId, int actionId);
	static ConnectObject* create2(agtk::Object *object, int connectObjectSettingId, cocos2d::Vec2 position, cocos2d::Vec2 scale, int initialActionId, int moveDirectionId);
#ifdef USE_MULTITHREAD_OBJECT_UPDATE
	virtual void objectUpdateBefore(float delta);
	virtual void objectUpdateAfter(float delta);
	virtual void objectUpdateWallCollision(float delta);
	virtual void update(float delta);
#else
	virtual void update(float delta);
#endif
	void unconnect(); // 接続を無効化する

	void changeConnectBaseObject(agtk::Object * object) { if (_connectBaseObject != object) this->setConnectBaseObject(object);};
	void changeObjectConnectSettingData(agtk::data::ObjectConnectSettingData * settingData) { this->setObjectConnectSettingData(settingData); };
	bool getIsConnecting() { return isConnecting; };
	void setIsConnecting(bool _isConnecting) { isConnecting = _isConnecting; };

private:
	virtual bool init(agtk::Object *object, int connectObjectSettingId, int actionId);
	virtual bool init2(agtk::Object *object, int connectObjectSettingId, cocos2d::Vec2 position, cocos2d::Vec2 scale, int initialActionId, int moveDirectionId);

	cocos2d::Vec2 getConnectPosition();
private:
	CC_SYNTHESIZE_RETAIN(agtk::data::ObjectConnectSettingData*, _objectConnectSettingData, ObjectConnectSettingData);
	CC_SYNTHESIZE_RETAIN(agtk::Object *, _connectBaseObject, ConnectBaseObject); // 接続元オブジェクト
	bool isConnecting;
	int initZorder; // 初期Zオーダー
	CC_SYNTHESIZE(int, _connectionId, ConnectionId);//接続点ID
};

#if defined(AGTK_DEBUG)
void dumpObjectInformation();
#endif

NS_AGTK_END

#endif	//__OBJECT_H__
