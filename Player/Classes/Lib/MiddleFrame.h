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

	// �ǔ���擾����Object���W(Scene���W�n)
	cocos2d::Vec2 _objectPos;

	// autoGeneration�@Physics�ɂ��X�V
	float _centerRotation;				// player->getCenterRotation()/setCenterRotation()

	// nodePlayer (basePlayer)�@Animation�ɂ��X�V
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

	void updateEnterframe();		// �t���[�����̏����������B�A�j���[�V�����O�B
	void updateAnimation();			// 30FPS�ł�AnimationMotion::update()���璆�ԃt���[����Transform�p�����[�^��ۑ�
	void updatePhysics();			// 30FPS�ł�GameManager::updatePhysics()�̒��ԃt���[����Transform�p�����[�^��ۑ�
	void updateWall();				// 30FPS�ł̒��ԃt���[���̕ǔ�������쐬

	Object* _object;
	int _middleFrameIndex;
	MiddleFrame _frameData[2];
};

NS_AGTK_END

#endif
#endif	//__MIDDLEFRAME_H__