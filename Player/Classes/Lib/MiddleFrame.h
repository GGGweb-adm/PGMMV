#ifndef __MIDDLEFRAME_H__
#define	__MIDDLEFRAME_H__

#include "Lib/Macros.h"
// #AGTK-NX, #AGTK-WIN
#ifdef USE_30FPS_4
#include "Lib/Common.h"

NS_AGTK_BEGIN

//-------------------------------------------------------------------------------------------------------------------
class Object;

class AGTKPLAYER_API MiddleFrame
{
public:
	MiddleFrame();
	~MiddleFrame() {};

	bool _hasMiddleFrame;

	// 壁判定取得時のObject座標(Scene座標系)
	cocos2d::Vec2 _objectPos;

	// autoGeneration　Physicsによる更新
	float _centerRotation;				// player->getCenterRotation()/setCenterRotation()

	// nodePlayer (basePlayer)　Animationによる更新
	cocos2d::Vec2 _innerScale;			// basePlayer->getInnerScale()/setInnerScale()
	float _innerRotation;				// basePlayer->getInnerRotation()/setInnerRotation()
	cocos2d::Vec2 _offset;				// basePlayer->getOffset()/setOffset()
	cocos2d::Vec2 _center;				// basePlayer->getCenter()/setCenter()

	std::vector<Vertex4> _wallList;
};

struct AGTKPLAYER_API MiddleFrameStock
{
public:
	MiddleFrameStock();
	~MiddleFrameStock() {}
	bool init(Object* object);

	MiddleFrame* getUpdatingMiddleFrame() { return _frameData + _middleFrameIndex; }
	MiddleFrame* getUpdatedMiddleFrame() { return _frameData + ((_middleFrameIndex + 1) & 1); }

	void updateEnterframe();		// フレーム毎の初期化処理。アニメーション前。
	void updateAnimation();			// 30FPSでのAnimationMotion::update()から中間フレームのTransformパラメータを保存
	void updatePhysics();			// 30FPSでのGameManager::updatePhysics()の中間フレームのTransformパラメータを保存
	void updateWall();				// 30FPSでの中間フレームの壁判定情報を作成

	Object* _object;
	int _middleFrameIndex;
	MiddleFrame _frameData[2];
};

NS_AGTK_END

#endif
#endif	//__MIDDLEFRAME_H__