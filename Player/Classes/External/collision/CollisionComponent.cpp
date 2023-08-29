/**
 ForTreeCollision v0.0.1
 CollisionComponent.hpp

 Copyright (c) 2016 Kazuki Oda

 This software is released under the MIT License.
 http://opensource.org/licenses/mit-license.php
*/

#include "CollisionComponent.hpp"
USING_NS_CC;

CollisionComponent* CollisionComponent::create(CollisionDetaction* detaction,
                                               unsigned int group) {
    auto ref = new CollisionComponent();
	ref->setName(CollisionComponent::getCollisionComponentName(group));
    if (ref && ref->initWithDetaction(detaction, group)) {
        ref->autorelease();
        return ref;
    }
    CC_SAFE_RELEASE(ref);
    return nullptr;
}

CollisionComponent::~CollisionComponent() {
#ifdef USE_MULTITHREAD_OBJECT_UPDATE
	if (m_node.getNode()) {
		m_node.setNode(nullptr);
	}
	m_detaction->remove(&m_node);
#else
    CC_SAFE_RELEASE_NULL(m_node);
#endif
    CC_SAFE_RELEASE_NULL(m_detaction);
}

bool CollisionComponent::initWithDetaction(CollisionDetaction* detaction,
                                           unsigned int group) {
    if (!Component::init()) {
        return false;
    }
    CC_SAFE_RETAIN(detaction);
    m_detaction = detaction;
    m_group = group;
    return true;
}

const std::string &CollisionComponent::getCollisionComponentName(int group)
{
	if (group < 0 || group >= kGroupMax) {
		static std::string unknownName("CollisionComponent?");
		return unknownName;
	}
	static std::string sComponentNameList[] = {
		"CollisionComponent0",
		"CollisionComponent1",
		"CollisionComponent2",
		"CollisionComponent3",
		"CollisionComponent4",
	};
	return sComponentNameList[group];
}

void CollisionComponent::onEnter() {
    Component::onEnter();
#ifdef USE_MULTITHREAD_OBJECT_UPDATE
	if (m_node.getNode()) {
		//already exists...
		return;
	}
	m_node.setNode(getOwner());
	m_node.setGroup(m_group);
	m_detaction->add(&m_node);
// #AGTK-NX
#ifdef USE_SAR_OPTIMIZE_5
	if (m_group == CollisionComponent::kGroupWall) {
		m_detaction->updateSingleWithoutScan(&m_node);
	}
#endif
#else
	if (m_node) {
		//already exists...
		return;
	}
    m_node = CollisionNode::create(getOwner(), m_group);
    m_node->retain();
    m_detaction->add(m_node);
#endif
}

void CollisionComponent::onExit() {
    Component::onExit();
#ifdef USE_MULTITHREAD_OBJECT_UPDATE
	if (!m_node.getNode()) {
		return;
	}
	m_detaction->remove(&m_node);
// #AGTK-NX
#ifdef USE_SAR_OPTIMIZE_5
	if (m_group == CollisionComponent::kGroupWall) {
		m_detaction->removeSingle(&m_node);
	}
#endif
	m_node.setNode(nullptr);
#else
	if (m_node == nullptr) {
		return;
	}
    if (m_node->getReferenceCount() > 1) {
        m_detaction->remove(m_node);
    }
	//※デストラクタで破棄するため。リリース処理をしない。
	//CC_SAFE_RELEASE_NULL(m_detaction);
	CC_SAFE_RELEASE_NULL(m_node);
#endif
}