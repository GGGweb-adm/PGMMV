#ifndef __BULLET_MANAGER_H__
#define __BULLET_MANAGER_H__

#include "cocos2d.h"
#include "Lib/Macros.h"
#include "Lib/Bullet.h"

USING_NS_CC;

NS_AGTK_BEGIN
//-------------------------------------------------------------------------------------------------------------------
class AGTKPLAYER_API FireBullet : public cocos2d::Ref
{
public:
	enum EnumState {
		kStateNone,
		kStateStart,//開始
		kStateFire,//発射
		kStateRemove,//破棄
		kStateEnd,//終了
		kStateWait,//待ち
	};
private:
	FireBullet();
	virtual ~FireBullet();
public:
	static FireBullet *create(agtk::Object *object, agtk::data::ObjectFireBulletSettingData *fireBulletSettingData, int connectId, int duration300);
	void update(float dt);
	bool start();
	void end();
private:
	virtual bool init(agtk::Object *object, agtk::data::ObjectFireBulletSettingData *fireBulletSettingData, int connectId, int duration300);
private:
	CC_SYNTHESIZE_RETAIN(agtk::Bullet *, _bullet, Bullet);
	CC_SYNTHESIZE_RETAIN(agtk::Object *, _object, Object);
	CC_SYNTHESIZE(int, _duration300, Duration300);
	CC_SYNTHESIZE(float, _duration, Duration);
	CC_SYNTHESIZE_RETAIN(agtk::data::ObjectFireBulletSettingData *, _fireBulletSettingData, FireBulletSettingData);
	CC_SYNTHESIZE(int, _connectId, ConnectId);
	CC_SYNTHESIZE(EnumState, _state, State);
	CC_SYNTHESIZE(int, _count, Count);
	CC_SYNTHESIZE(int, _maxCount, MaxCount);
	CC_SYNTHESIZE(int, _layerId, LayerId);
	CC_SYNTHESIZE(cocos2d::Vec2, _position, Position);
	CC_SYNTHESIZE(int, _dispDirection, DispDirection);
	CC_SYNTHESIZE(bool, _removeParentObjectFlag, RemoveParentObjectFlag);
	CC_SYNTHESIZE(bool, _isConnect, IsConnect);			// 接続点はあるか
	CC_SYNTHESIZE(EnumState, _beforeWaitState, BeforeWaitState);	// 待ち状態になる前の状態
	CC_SYNTHESIZE(int, _fireId, FireId);
};
NS_AGTK_END

//-------------------------------------------------------------------------------------------------------------------
// #AGTK-NX
#if CC_TARGET_PLATFORM != CC_PLATFORM_NX
class agtk::Bullet;
#endif
class AGTKPLAYER_API BulletManager : public cocos2d::Ref
{
private:
	BulletManager();
	static BulletManager *_bulletManager;
public:
	virtual ~BulletManager();
	static BulletManager* getInstance();
	static void purge();
	bool init();
	void update(float delta);
	int getBulletCount(agtk::Object *object, int fireBulletSettingId);
	void createBullet(agtk::Object *object, int fireBulletSettingId, int connectId);
	void removeParentObject(agtk::Object *object, bool bStartStateOnly = false);
	void removeBullet(agtk::Bullet *bullet);
	void removeAllBullet();
private:
	void updateRemoveFireBullet();
private:
	CC_SYNTHESIZE_RETAIN(cocos2d::__Array *, _fireBulletList, FireBulletList);
	CC_SYNTHESIZE_RETAIN(cocos2d::__Array *, _disabledLayerIdList, DisabledLayerIdList);
	static int _fireId;
};

#endif	//__BULLET_MANAGER_H__
