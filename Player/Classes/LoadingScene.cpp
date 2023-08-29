#include "LoadingScene.h"
#include "Manager/GameManager.h"
#include "Manager/FontManager.h"

USING_NS_CC;

LoadingScene::LoadingScene()
{
}

LoadingScene::~LoadingScene()
{
}

bool LoadingScene::init()
{
	if(!Layer::init()) {
		return false;
	}

	auto visibleSize = Director::getInstance()->getVisibleSize();
	Vec2 origin = Director::getInstance()->getVisibleOrigin();

	FontManager *fm = FontManager::getInstance();
	auto text = fm->createWithArialFont("Loading", 24);
	text->setPosition(16, 32);
	this->addChild(text, kZOrderMain, kTagMain);

	//get game pad status in polling mode
	scheduleUpdate();

// #AGTK-NX #AGTK-WIN
#ifdef USE_30FPS
	float duration = Director::getInstance()->getAnimationInterval() / GameManager::getInstance()->getFrameProgressScale() * 100;
#else
	float duration = Director::getInstance()->getAnimationInterval() * 100;
#endif
	CCLOG("duration:%f", duration);
	this->runAction(CCSequence::create(DelayTime::create(duration), CallFunc::create([this](){ this->changeScene(); }), NULL));

    return true;
}

void LoadingScene::changeScene()
{
	GameManager::getInstance()->changeCanvas();
}

void LoadingScene::update(float delta)
{
}
