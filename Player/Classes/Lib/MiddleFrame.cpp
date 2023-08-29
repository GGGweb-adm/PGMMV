#include "Lib/MiddleFrame.h"
#ifdef USE_30FPS_4
#include "Manager/GameManager.h"

NS_AGTK_BEGIN

//-------------------------------------------------------------------------------------------------------------------

MiddleFrame::MiddleFrame()
{
	_hasMiddleFrame = false;
	_innerRotation = 0.0f;
	_innerScale = cocos2d::Vec2::ONE;
	_offset = cocos2d::Vec2::ZERO;
	_center = cocos2d::Vec2::ZERO;
}

//-------------------------------------------------------------------------------------------------------------------

MiddleFrameStock::MiddleFrameStock()
{
	_middleFrameIndex = 0;
	_object = nullptr;
}

bool MiddleFrameStock::init(Object* object)
{
	_object = object;
	updateEnterframe();
	return true;
}

void MiddleFrameStock::updateEnterframe()
{
	MiddleFrame* mf = getUpdatingMiddleFrame();
	Player* player = _object->getPlayer();
	BasePlayer* basePlayer = _object->getBasePlayer();

	if (player) {
		mf->_centerRotation = player->getCenterRotation();
	}
	else {
		mf->_centerRotation = 0.0f;
	}

	if (basePlayer) {
		mf->_innerScale = basePlayer->getInnerScale();
		mf->_innerRotation = basePlayer->getInnerRotation();
		mf->_offset = basePlayer->getOffset();
		mf->_center = basePlayer->getCenter();
	}
	else {
		mf->_innerScale = Vec2::ONE;
		mf->_innerRotation = 0.0f;
		mf->_offset = Vec2::ZERO;
		mf->_center = Vec2::ZERO;
	}
	mf->_hasMiddleFrame = false;
}

void MiddleFrameStock::updateAnimation()
{
	MiddleFrame* mf = getUpdatingMiddleFrame();
	BasePlayer* basePlayer = _object->getBasePlayer();

	if (basePlayer) {
		mf->_innerScale = basePlayer->getInnerScale();
		mf->_innerRotation = basePlayer->getInnerRotation();
		mf->_offset = basePlayer->getOffset();
		mf->_center = basePlayer->getCenter();
	}
}

void MiddleFrameStock::updatePhysics()
{
	MiddleFrame* mf = getUpdatingMiddleFrame();
	Player* player = _object->getPlayer();
	if (!player)
		return;
	auto physicsNode = _object->getphysicsNode();
	auto objectData = _object->getObjectData();
	if (physicsNode) {
		// �v���C���[�^�C�v�łȂ� �܂��� �\�������𑀍�ŕύX���Ȃ��ꍇ
		if (!objectData->isGroupPlayer() || !objectData->getMoveDispDirectionSettingFlag()) {
			auto rotate = physicsNode->getRotation();
			//��]�̎��������B
			if (_object->isAutoGeneration()) {
				auto direction = _object->getObjectMovement()->getDirection();
				if (direction == cocos2d::Vec2::ZERO) {
					if (GameManager::getInstance()->getProjectData()->getGameType() == agtk::data::ProjectData::kGameTypeSideView) {
						direction.x = 1;
					}
					else {
						direction = agtk::GetDirectionFromMoveDirectionId(_object->getDispDirection());
					}
				}
				float degree = agtk::GetDegreeFromVector(direction);
				mf->_centerRotation = GetDegree360(rotate + degree);
			}
			else {
				mf->_centerRotation = GetDegree360(rotate);
			}
			if (player->getCenterRotation() != mf->_centerRotation) {
				mf->_hasMiddleFrame = true;
			}
		}
	}
	else {
		mf->_centerRotation = 0.0f;
	}
}

void MiddleFrameStock::updateWall()
{
	MiddleFrame* mf = getUpdatingMiddleFrame();
	Player* player = _object->getPlayer();
	if (mf->_hasMiddleFrame && player) {
		BasePlayer* basePlayer = _object->getBasePlayer();
		if (basePlayer) {
			// Transform�p�����[�^�[��ޔ�
			float centerRotation = player->getCenterRotation();
			Vec2 innerScale = basePlayer->getInnerScale();
			float innerRotation = basePlayer->getInnerRotation();
			Vec2 offset = basePlayer->getOffset();
			Vec2 center = basePlayer->getCenter();

			player->setCenterRotation(mf->_centerRotation);
			basePlayer->setInnerScale(mf->_innerScale);
			basePlayer->setInnerRotation(mf->_innerRotation);
			basePlayer->setOffset(mf->_offset);
			basePlayer->setCenter(mf->_center);

			// �ǔ���̎擾�X�V
			auto motion = basePlayer->getCurrentAnimationMotion();
			auto directionData = motion->getCurrentDirection();

			std::vector<Vertex4> wallCollisionList;
			_object->getTimelineList(agtk::data::TimelineInfoData::kTimelineWall, wallCollisionList);
			if (wallCollisionList.size() > 0) {
				mf->_objectPos = _object->getPosition();
				mf->_wallList = wallCollisionList;
			}
			else {
				mf->_hasMiddleFrame = false;
			}

			// Transform�p�����[�^�[�𕜋A
			player->setCenterRotation(centerRotation);
			basePlayer->setInnerScale(innerScale);
			basePlayer->setInnerRotation(innerRotation);
			basePlayer->setOffset(offset);
			basePlayer->setCenter(center);
		}
		else {
			mf->_hasMiddleFrame = false;
		}
	}

	// ���ԃt���[�����X�V��̐؂�ւ�
	_middleFrameIndex = (_middleFrameIndex + 1) & 1;
}

NS_AGTK_END
#endif