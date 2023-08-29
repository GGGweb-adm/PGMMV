#ifndef __BULLET_H__
#define	__BULLET_H__

#include "Lib/Macros.h"
#include "Lib/Object.h"

NS_AGTK_BEGIN

//-------------------------------------------------------------------------------------------------------------------
class Bullet;
class AGTKPLAYER_API InitialBulletLocus : public cocos2d::Ref
{
protected:
	InitialBulletLocus();
	virtual ~InitialBulletLocus();
public:
	static InitialBulletLocus *create(data::ObjectFireBulletSettingData *objectFireBulletSettingData);
	virtual bool initial(agtk::Object *parent, int connectId, agtk::Bullet *bullet, int count, int maxCount) = 0;
	bool initial(agtk::Object *parent, int connectId);
protected:
	virtual bool init(data::ObjectFireBulletSettingData *objectFireBulletSettinData);
protected:
	CC_SYNTHESIZE_RETAIN(data::ObjectFireBulletSettingData *, _objectFireBulletSettingData, ObjectFireBulletSettingData);
	CC_SYNTHESIZE(int, _dispDirection, DispDirection);//表示方向
	CC_SYNTHESIZE(cocos2d::Vec2, _direction, Direction);//移動方向
	CC_SYNTHESIZE(cocos2d::Vec2, _position, Position);//位置
	CC_SYNTHESIZE_RETAIN(agtk::Object *, _targetObject, TargetObject);
};

//-------------------------------------------------------------------------------------------------------------------
class AGTKPLAYER_API InitialBulletLocusFree : public agtk::InitialBulletLocus//飛び方を指定しない
{
protected:
	InitialBulletLocusFree() : InitialBulletLocus() {};
public:
	CREATE_FUNC_PARAM(InitialBulletLocusFree, data::ObjectFireBulletSettingData *, objectFireBulletSettingData);
	virtual bool initial(agtk::Object *parent, int connectId, agtk::Bullet *bullet, int count, int maxCount);
};

//-------------------------------------------------------------------------------------------------------------------
class AGTKPLAYER_API InitialBulletLocusFireObjectDirection : public agtk::InitialBulletLocus//発射元オブジェクトの表示方向
{
protected:
	InitialBulletLocusFireObjectDirection() : InitialBulletLocus() {};
public:
	CREATE_FUNC_PARAM(InitialBulletLocusFireObjectDirection, data::ObjectFireBulletSettingData *, objectFireBulletSettingData);
	virtual bool initial(agtk::Object *parent, int connectId, agtk::Bullet *bullet, int count, int maxCount);
};

//-------------------------------------------------------------------------------------------------------------------
class AGTKPLAYER_API InitialBulletLocusTowardObject : public agtk::InitialBulletLocus//画面内の発射元以外のオブジェクト方向を選ぶ
{
protected:
	InitialBulletLocusTowardObject() : InitialBulletLocus() {};
public:
	CREATE_FUNC_PARAM(InitialBulletLocusTowardObject, data::ObjectFireBulletSettingData *, objectFireBulletSettingData);
	virtual bool initial(agtk::Object *parent, int connectId, agtk::Bullet *bullet, int count, int maxCount);
};

//-------------------------------------------------------------------------------------------------------------------
class AGTKPLAYER_API InitialBulletLocusOneDirection : public agtk::InitialBulletLocus//一定方向に飛ぶ
{
protected:
	InitialBulletLocusOneDirection() : InitialBulletLocus() {};
public:
	CREATE_FUNC_PARAM(InitialBulletLocusOneDirection, data::ObjectFireBulletSettingData *, objectFireBulletSettingData);
	virtual bool initial(agtk::Object *parent, int connectId, agtk::Bullet *bullet, int count, int maxCount);
};

//-------------------------------------------------------------------------------------------------------------------
class AGTKPLAYER_API NextBulletLocus : public cocos2d::Ref
{
public:
	enum EnumState {
		kStateNone,
		kStateStart,//開始
		kStateAction,//実行
		kStateToTarget,//ターゲットオブジェクトへ（ブーメラン用）
		kStateToParent,//親オブジェクトへ（ブーメラン用）
		kStateEnd,//終了
	};
protected:
	NextBulletLocus();
	virtual ~NextBulletLocus();
public:
	static NextBulletLocus *create(data::ObjectFireBulletSettingData *data);
	void initial(agtk::Object *parent, agtk::Bullet *bullet);
	virtual void start(int count, int maxCount) = 0;
	virtual void end() = 0;
	virtual void update(float delta) = 0;
	void setTargetObject(agtk::Object *object);
	agtk::Object *getTargetObject() { return _targetObject; };
	cocos2d::Vec2 getDirection() { return _direction; };
	cocos2d::Vec2 getDirectionOld() { return _directionOld; };
	void setDirection(cocos2d::Vec2 vec, bool bInitialize = false);
protected:
	virtual bool init(data::ObjectFireBulletSettingData *data);
protected:
	CC_SYNTHESIZE_RETAIN(data::ObjectFireBulletSettingData *, _objectFireBulletSettingData, ObjectFireBulletSettingData);
	cocos2d::Vec2 _direction;//方向
	cocos2d::Vec2 _directionOld;//方向（１フレーム前）
	CC_SYNTHESIZE(EnumState, _state, State);//状態
	CC_SYNTHESIZE(int, _connectId, ConnectId);//接続点ID
	agtk::Object *_targetObject;
	agtk::Object *_parent;
	agtk::Bullet *_bullet;
};

//-------------------------------------------------------------------------------------------------------------------
class AGTKPLAYER_API NextBulletLocusFree : public agtk::NextBulletLocus//飛び方を指定しない
{
private:
	NextBulletLocusFree() : NextBulletLocus() {};
public:
	CREATE_FUNC_PARAM(NextBulletLocusFree, data::ObjectFireBulletSettingData *, objectFireBulletSettingData);
	virtual void start(int count, int maxCount);
	virtual void end();
	virtual void update(float delta);
private:
	virtual bool init(data::ObjectFireBulletSettingData *objectFireBulletSettingData);
};
	
//-------------------------------------------------------------------------------------------------------------------
class AGTKPLAYER_API NextBulletLocusFollowLockedObject : public agtk::NextBulletLocus//発射元がロックしているオブジェクトを追尾
{
private:
	NextBulletLocusFollowLockedObject();
	virtual ~NextBulletLocusFollowLockedObject() {};
public:
	CREATE_FUNC_PARAM(NextBulletLocusFollowLockedObject, data::ObjectFireBulletSettingData *, objectFireBulletSettingData);
	virtual void start(int count, int maxCount);
	virtual void end();
	virtual void update(float delta);
private:
	virtual bool init(data::ObjectFireBulletSettingData *objectFireBulletSettingData);
	bool innerStart();
private:
	CC_SYNTHESIZE(float, _duration, Duration);
	CC_SYNTHESIZE(int, _startDelay300, StartDelay300);
	CC_SYNTHESIZE(int, _startDelayDispersion300, StartDelayDispersion300);
	CC_SYNTHESIZE(float, _performance, Performance);
	CC_SYNTHESIZE(cocos2d::Vec2, _oldPosition, OldPosition);
	CC_SYNTHESIZE(int, _count, Count);
	CC_SYNTHESIZE(int, _maxCount, MaxCount);
	CC_SYNTHESIZE(bool, _moved, Moved);
};

//-------------------------------------------------------------------------------------------------------------------
class AGTKPLAYER_API NextBulletLocusFollowObjectInsideCamera : public agtk::NextBulletLocus//画面内のオブジェクトを自動追尾
{
private:
	NextBulletLocusFollowObjectInsideCamera();
	virtual ~NextBulletLocusFollowObjectInsideCamera() {};
public:
	CREATE_FUNC_PARAM(NextBulletLocusFollowObjectInsideCamera, data::ObjectFireBulletSettingData *, objectFireBulletSettingData);
	virtual void start(int count, int maxCount);
	virtual void end();
	virtual void update(float delta);
private:
	virtual bool init(data::ObjectFireBulletSettingData *objectFireBulletSettingData);
	bool innerStart();
private:
	CC_SYNTHESIZE(float, _duration, Duration);
	CC_SYNTHESIZE(int, _startDelay300, StartDelay300);
	CC_SYNTHESIZE(int, _startDelayDispersion300, StartDelayDispersion300);
	CC_SYNTHESIZE(float, _performance, Performance);
	CC_SYNTHESIZE(cocos2d::Vec2, _oldPosition, OldPosition);
	CC_SYNTHESIZE(int, _count, Count);
	CC_SYNTHESIZE(int, _maxCount, MaxCount);
	CC_SYNTHESIZE(bool, _moved, Moved);
};

//-------------------------------------------------------------------------------------------------------------------
class AGTKPLAYER_API NextBulletLocusBoomerang : public agtk::NextBulletLocus//ブーメラン起動
{
private:
	NextBulletLocusBoomerang();
	virtual ~NextBulletLocusBoomerang() {};
public:
	CREATE_FUNC_PARAM(NextBulletLocusBoomerang, data::ObjectFireBulletSettingData *, objectFireBulletSettingData);
	virtual void start(int count, int maxCount);
	virtual void end();
	virtual void update(float delta);
private:
	virtual bool init(data::ObjectFireBulletSettingData *objectFireBulletSettingData);
private:
	CC_SYNTHESIZE(float, _duration, Duration);
	CC_SYNTHESIZE(int, _turnDuration300, TurnDuration300);
	CC_SYNTHESIZE(float, _performance, Performance);
	CC_SYNTHESIZE(bool, _decelBeforeTurn, DecelBeforeTurn);//折り返すまでに減速
	CC_SYNTHESIZE(bool, _turnWhenTouchingWall, TurnWhenTouchingWall);//壁や当たり判定に接触で折り返す
	CC_SYNTHESIZE(cocos2d::Vec2, _oldPosition, OldPosition);
	CC_SYNTHESIZE(float, _decelMoveSpeed, DecelMoveSpeed);
};

//-------------------------------------------------------------------------------------------------------------------
class AGTKPLAYER_API BulletLocus : public cocos2d::Ref
{
protected:
	BulletLocus();
	virtual ~BulletLocus();
public:
	static BulletLocus *create(agtk::Bullet *bullet, agtk::Object *object, data::ObjectFireBulletSettingData *objectFireBulletSettingData, int connectId);
	bool initial(int count, int maxCount);
	void start(int count, int maxCount);
	void end();
	virtual void update(float delta);
	void setBullet(agtk::Bullet *bullet);
	agtk::Bullet *getBullet();
	float getMoveSpeed();//移動速度（弾のオプション）
protected:
	virtual bool init(agtk::Bullet *bullet, agtk::Object *object, data::ObjectFireBulletSettingData *objectFireBulletSettingData, int connectId);
public:
	static cocos2d::Vec2 calcDirection(data::ObjectFireBulletSettingData *objectFireBulletSettingData, float degree, int count, int maxCount);
	static cocos2d::Vec2 getFixDirection(float degrees, float range, int count, int max);//固定
	static cocos2d::Vec2 getRandomDirection(float degrees, float range);//ランダム
	static cocos2d::Vec2 getWaiperDirection(float degrees, float range, int count, int max);//ワイパー
private:
	CC_SYNTHESIZE_RETAIN(agtk::data::ObjectFireBulletSettingData *, _objectFireBulletSettingData, ObjectFireBulletSettingData);
	CC_SYNTHESIZE_RETAIN(agtk::InitialBulletLocus *, _initialBulletLocus, InitialBulletLocus);
	CC_SYNTHESIZE_RETAIN(agtk::NextBulletLocus *, _nextBulletLocus, NextBulletLocus);
	CC_SYNTHESIZE(cocos2d::Vec2, _direction, Direction);
	CC_SYNTHESIZE(int, _connectId, ConnectId);
	CC_SYNTHESIZE_RETAIN(agtk::Object *, _parentObject, ParentObject);
	agtk::Bullet *_bullet;
};

//-------------------------------------------------------------------------------------------------------------------
class AGTKPLAYER_API Bullet : public agtk::Object
{
private:
	Bullet();
	virtual ~Bullet();
public:
	static Bullet *(*create)(agtk::Object *object, agtk::data::ObjectFireBulletSettingData *fireBulletSettingData, int connectId, int count, int maxCount);
	static void setCreate(Bullet *(*create)(agtk::Object *object, agtk::data::ObjectFireBulletSettingData *fireBulletSettingData, int connectId, int count, int maxCount));
	static Bullet *_create(agtk::Object *object, agtk::data::ObjectFireBulletSettingData *fireBulletSettingData, int connectId, int count, int maxCount);
	void start(int count, int maxCount);
	void end();
	virtual void update(float delta);
#ifdef USE_MULTITHREAD_OBJECT_UPDATE
	virtual void objectUpdateBefore(float delta);
#endif
private:
	virtual bool init(agtk::Object *object, agtk::data::ObjectFireBulletSettingData *fireBulletSettingData, int connectId, int count, int maxCount);
private:
	CC_SYNTHESIZE_RETAIN(agtk::data::ObjectFireBulletSettingData *, _objectFireBulletSettingData, ObjectFireBulletSettingData);
	CC_SYNTHESIZE_RETAIN(BulletLocus *, _bulletLocus, BulletLocus);
	CC_SYNTHESIZE_RETAIN(agtk::Object *, _parentObject, ParentObject);
	CC_SYNTHESIZE(bool, _bulletIgnored, BulletIgnored);//弾機能無効。
};

NS_AGTK_END

#endif	//__BULLET_H__
