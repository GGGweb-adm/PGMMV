#include "Manager\ProjectLoadingManager.h"

#if defined(USE_BG_PROJECT_LOAD) || defined(USE_PRELOAD_TEX)
#include "Manager\GameManager.h"
#include "platform\CCImage.h"
#include "Data\AssetData.h"

ProjectLoadingManager* ProjectLoadingManager::_projectLoadingManager = nullptr;

ProjectLoadingManager::ProjectLoadingManager()
{
	_preloadTexList = nullptr;
}

ProjectLoadingManager::~ProjectLoadingManager()
{
	CC_SAFE_RELEASE_NULL(_preloadTexList);
}

ProjectLoadingManager* ProjectLoadingManager::getInstance()
{
	if (!_projectLoadingManager) {
		_projectLoadingManager = new ProjectLoadingManager();
	}
	return _projectLoadingManager;
}

void ProjectLoadingManager::load(std::string projectFilePath)
{
	_cacheTaskList.clear();

	auto imageMtTask = ImageMtTask::getInstance();

	std::function<void(void*)> setupCallback = [this](void* imageTask) {
		auto _imageTask = static_cast<ImageMtTask::ImageTask*>(imageTask);
		if (_imageTask->arg) {
			auto imageData = static_cast<agtk::data::ImageData*>(_imageTask->arg);
			imageData->setTexWidth(_imageTask->image->getWidth());
			imageData->setTexHeight(_imageTask->image->getHeight());
		};
#ifdef USE_PRELOAD_TEX
		// 後程キャッシュするための登録
		if (_preloadTexKeys.count(_imageTask->fullpath) != 0) {
			addCacheTask(_imageTask->image, _imageTask->fullpath);
		}
#endif
	};

#ifdef USE_BG_PROJECT_LOAD
	imageMtTask->setConfigure(true, setupCallback);
#else
	imageMtTask->setConfigure(false, setupCallback);
#endif
	imageMtTask->setEnable(true);
	//内部で cocosのreference countを利用しているのでプロジェクトロード処理をBGスレッドで行うことは困難と判明。
	//イメージセットアップ処理のみマルチスレッド化
	GameManager::getInstance()->loadJsonFile(projectFilePath);
}

bool ProjectLoadingManager::isDone()
{
	return ImageMtTask::getInstance()->isDone();
}

void ProjectLoadingManager::pause()
{
	ImageMtTask::getInstance()->pause();
}

bool ProjectLoadingManager::isPaused()
{
	return ImageMtTask::getInstance()->isPaused();
}

void ProjectLoadingManager::restart()
{
	ImageMtTask::getInstance()->restart();
}

void ProjectLoadingManager::join()
{
	auto imageMtTask = ImageMtTask::getInstance();
	imageMtTask->join();
	imageMtTask->setEnable(false);
}

bool ProjectLoadingManager::execOneTask()
{
	return ImageMtTask::getInstance()->imageSetupOne();
}

void ProjectLoadingManager::preparePreloadTex(rapidjson::Value& json, std::string projectPath)
{
	this->_preloadTexKeys.clear();
	this->setPreloadTexList(cocos2d::__Dictionary::create());

	std::map<int, std::string> imageList;

	std::function<void(rapidjson::Value&, std::string&)> addImageFilename;
	addImageFilename = [&](rapidjson::Value& json, std::string& projectPath) {
		if (json["folder"].GetBool()) {
			if (json.HasMember("children")) {
				for (rapidjson::SizeType i = 0; i < json["children"].Size(); i++) {
					addImageFilename(json["children"][i], projectPath);
				}
			}
			return;
		}
		int id = json["id"].GetInt();
		if (imageList.count(id) == 0) {
#ifdef USE_PREVIEW
			std::string filename = agtk::data::getFullFilename(json["filename"].GetString(), projectPath);
#else
			std::string filename = projectPath + json["filename"].GetString();
#endif
			std::string fullpath = CCFileUtils::getInstance()->fullPathForFilename(filename);
			imageList.insert(std::make_pair(id, fullpath));
		}
	};

	std::function<void(rapidjson::Value&)> addEffectTex;
	addEffectTex = [&](rapidjson::Value& json) {
		CC_ASSERT(json.HasMember("folder"));
		if (json["folder"].GetBool()) {
			if (json.HasMember("children")) {
				for (rapidjson::SizeType i = 0; i < json["children"].Size(); i++) {
					rapidjson::Value& animDataJson = json["children"][i];
					addEffectTex(animDataJson);
				}
			}
			return;
		}
		CC_ASSERT(json.HasMember("type"));
		auto animType = (agtk::data::AnimationData::EnumAnimType)json["type"].GetInt();
		if (animType == agtk::data::AnimationData::kEffect) {
			CC_ASSERT(json.HasMember("resourceInfo"));
			rapidjson::Value& resourceInfoJson = json["resourceInfo"];
			CC_ASSERT(resourceInfoJson.HasMember("image"));
			if (resourceInfoJson["image"].GetBool()) {
				CC_ASSERT(resourceInfoJson.HasMember("imageId"));
				int imageId = resourceInfoJson["imageId"].GetInt();
				if (_preloadTexKeys.count(imageList[imageId]) == 0) {
					_preloadTexKeys.insert(std::make_pair(imageList[imageId], 0));
				}
			}
		}
	};

	// ImageListから画像ファイル名をリストアップ
	CC_ASSERT(json.HasMember("imageList"));
	for (rapidjson::SizeType i = 0; i < json["imageList"].Size(); i++) {
		rapidjson::Value& imageJson = json["imageList"][i];
		addImageFilename(imageJson, projectPath);
	}

	// AnimationListからエフェクト画像をPreload対象として登録
	CC_ASSERT(json.HasMember("animationList"));
	for (rapidjson::SizeType i = 0; i < json["animationList"].Size(); i++) {
		rapidjson::Value& animDataJson = json["animationList"][i];
		addEffectTex(animDataJson);
	}
}

void ProjectLoadingManager::postProcessPreloadTex()
{
#ifndef USE_BG_PROJECT_LOAD
	// CacheImageListのImageをキャッシュ
#endif
	auto imageMtTask = ImageMtTask::getInstance();

	std::function<void(void*)> setupCallback = [this](void* imageTask) {
		auto _imageTask = static_cast<ImageMtTask::ImageTask*>(imageTask);
		// 後程キャッシュするための登録
		cocos2d::__String* keyObj = static_cast<cocos2d::__String*>(_imageTask->arg);
		addCacheTask(_imageTask->image, keyObj->getCString());
		keyObj->release();
	};

#ifdef USE_BG_PROJECT_LOAD
	imageMtTask->setConfigure(true, setupCallback);
#else
	imageMtTask->setConfigure(false, setupCallback);
#endif

	auto projectData = GameManager::getInstance()->getProjectData();

	std::function<void(cocos2d::__Dictionary*)> addParticleImage = [&](cocos2d::__Dictionary* children) {
		cocos2d::DictElement *el = nullptr;
		CCDICT_FOREACH(children, el) {
			auto p = static_cast<agtk::data::AnimationData *>(el->getObject());
			if (p->getChildren()) {
				addParticleImage(p->getChildren());
			}
			else {
				if (p->getType() == agtk::data::AnimationData::kParticle) {
					cocos2d::DictElement *el2 = nullptr;
					auto particleList = p->getParticleList();
					CCDICT_FOREACH(particleList, el2) {
						auto p2 = static_cast<agtk::data::AnimeParticleData*>(el2->getObject());
						int id = p2->getTemplateId();
						auto particleData = projectData->getParticleImageData(id);
						if (particleData) {
							std::string filename = particleData->getFilename()->_string;
							std::string key = filename;
							if (p2->getDisableAntiAlias()) {
								key.append("_DAA");
							}
							if (this->_preloadTexKeys.count(key) == 0) {
								this->_preloadTexKeys.insert(std::make_pair(key, 0));
								cocos2d::Image* img = new cocos2d::Image();
								cocos2d::__String* keyObj = new cocos2d::__String(key);
								img->initWithImageFileLateSetup(filename, keyObj);
							}
						}
					}
				}
			}
		}
	};

	addParticleImage(projectData->getAnimationList());
}

void ProjectLoadingManager::addCacheTask(Image* image, std::string key)
{
	CacheTask task;
	task.image = image;
	task.key = key;
	task.image->retain();

	_cacheTaskList.push_back(task);
}

void ProjectLoadingManager::postProcessCacheTex()
{
	auto texCache = CCDirector::getInstance()->getTextureCache();
	for (auto it = _cacheTaskList.begin(); it != _cacheTaskList.end(); it++) {
		if (!texCache->getTextureForKey(it->key)) {
			auto texture = texCache->addImage(it->image, it->key);
			auto preloadTexList = this->getPreloadTexList();
			preloadTexList->setObject(texture, it->key);
		}
		it->image->release();
	}
}

void ProjectLoadingManager::removePreloadTex()
{
	_preloadTexKeys.clear();
	this->setPreloadTexList(nullptr);
}

#endif