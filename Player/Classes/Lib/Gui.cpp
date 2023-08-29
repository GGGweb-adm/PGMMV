#include "Gui.h"
#include "Lib/Scene.h"
#include "Manager/GameManager.h"
#include "Manager/FontManager.h"

#include <base/CCProfiling.h>

#define TEXT_SCROLL_SPEED_UP_RATE 2.0f	// テキストスクロールの速度アップ割合(デフォルトは 2.0 倍速)s

NS_AGTK_BEGIN
//-------------------------------------------------------------------------------------------------------------------
Gui::Gui()
{
	_targetObject = nullptr;
	_isDelete = false;
	_isClipping = false;
	_clipping = nullptr;
}

Gui::~Gui()
{
	_targetObject = nullptr;
	_clipping = nullptr;
}

#ifdef USE_REDUCE_RENDER_TEXTURE
void Gui::visit(Renderer *renderer, const Mat4& parentTransform, uint32_t parentFlags)
{
	Node::visit(renderer, parentTransform, parentFlags);
}
#endif

agtk::Object* Gui::getTargetObject()
{
	return _targetObject;
}

bool Gui::checkRemove(agtk::Object* object)
{
	if (_targetObject->getId() == object->getId()
	&&  _targetObject->getLayerId() == object->getLayerId()
	&& _targetObject->getSceneLayer() == object->getSceneLayer()) {
		return true;
	}

	return false;
}

bool Gui::getObjectStop()
{
	// 基本的にはUI表示によるオブジェクト停止は発生しない
	return false;
}

bool Gui::getGameStop()
{
	// 基本的にUI表示によるゲーム停止は発生しない
	return false;
}

bool Gui::isDelete()
{
	return _isDelete;
}

cocos2d::Vec2 Gui::getPositionTransform(cocos2d::Vec2 pos)
{
	auto object = this->getTargetObject();
	if(object) {
		auto sceneLayer = object->getSceneLayer();
		auto sceneData = sceneLayer->getSceneData();
		if(sceneData->getId() == agtk::data::SceneData::kMenuSceneId) {
			auto projectData = GameManager::getInstance()->getProjectData();
			cocos2d::Size size(
				projectData->getScreenWidth() * sceneData->getHorzScreenCount(),
				projectData->getScreenHeight() * sceneData->getVertScreenCount()
			);
			return cocos2d::Vec2(pos.x, size.height - pos.y);
		}
	}
	return agtk::Scene::getPositionSceneFromCocos2d(pos);
}

/**
* クリッピング使用判定(表示領域・判定/回転が設定されている場合はクリッピングは使わない)
*/
bool Gui::isUseClipping()
{
	if (_targetObject) {
		// 表示領域
		auto scaleX = _targetObject->getScaleX();
		auto scaleY = _targetObject->getScaleY();
		if (scaleX != 1.0f || scaleY != 1.0f) {
			return false;
		}
	}

	auto scene = GameManager::getInstance()->getCurrentScene();
	auto camera = scene->getCamera();

	return camera->isUseClipping();
}

//-------------------------------------------------------------------------------------------------------------------
TextGui::TextGui()
{
	_color = cocos2d::Color3B::WHITE;
	_alpha = 0;
	_string = "";
	_horzAlign = 0.0f;
	_vertAlign = 0.0f;
	_textHorzAlign = 0.0f;
	_height = 0;
	_realHeight = 0;
	_isUpdate = false;

	_fontData = nullptr;
	_lineNodeList = nullptr;
	_renderTex = nullptr;
	_baseNode = nullptr;
}

TextGui::~TextGui()
{
	CC_SAFE_RELEASE_NULL(_fontData);
	CC_SAFE_RELEASE_NULL(_lineNodeList);
	CC_SAFE_RELEASE_NULL(_renderTex);
	CC_SAFE_RELEASE_NULL(_baseNode);
}

bool TextGui::init(int fontId, cocos2d::Color3B color, char alpha)
{
	_color = color;
	_alpha = alpha;

	_textHorzAlign = 0.5f;

	// プロジェクトデータを取得
	auto projectData = GameManager::getInstance()->getProjectData();
	CCASSERT(projectData, "プロジェクトデータ取得失敗");

	// フォントデータを設定
	auto fontData = projectData->getFontData(fontId);
	if (fontData == nullptr) {
		return false;
	}
	this->setFontData(fontData);

	// リストを初期化
	this->setLineNodeList(cocos2d::Array::create());

	return true;
}

/**
* 「表示するテキスト」で表示するテキストUIの更新(クリッピング用)
*/
void TextGui::updateText(std::string text, int letterSpace, int lineSpace, float width, float height, int marginLR, int marginTB)
{
	{
		auto delTexFunc = [&] {
			if (_baseNode != nullptr) {
				CC_SAFE_RELEASE_NULL(_baseNode);
			}
			this->removeAllChildren();
		};
		_letterSpace = letterSpace;
		_lineSpace = lineSpace;
		_string = text;

		auto lineNodeList = this->getLineNodeList();

		// 文字列が空の場合は処理しない
		if (_string.size() <= 0) {
			delTexFunc();
			return;
		}

		float maxW = 0;
		float maxH = 0;
		float firstH = 0;
		int iniSize = 0;
		if (_fontData->getMainFontSetting()->getImageFontFlag()) {
			if (_fontData->getMainFontSetting()->getImageId() >= 0) {
				auto project = GameManager::getInstance()->getProjectData();
				auto imageData = project->getImageData(_fontData->getMainFontSetting()->getImageId());
				if (imageData) {
					iniSize = _fontData->getMainFontSetting()->getVertDivCount() ? imageData->getTexHeight() / _fontData->getMainFontSetting()->getVertDivCount() : 0;
				}
			}
		}
		else {
			iniSize = _fontData->getMainFontSetting()->getFontSize();
		}

		// 各行ごとにノードを生成
		{
			std::stringstream ss(_string);
			std::string str;
			int lineNum = 0;
			cocos2d::Color3B color = _color;
			int fontSize = iniSize;
			while (std::getline(ss, str)) {
				agtk::TextLineNode*	lineNode = nullptr;
				if (lineNum < lineNodeList->count()) {
					// 既にノードが存在するなら中身を書き換え
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
					lineNode = static_cast<agtk::TextLineNode*>(lineNodeList->getObjectAtIndex(lineNum));
#else
					lineNode = dynamic_cast<agtk::TextLineNode*>(lineNodeList->getObjectAtIndex(lineNum));
#endif
					lineNode->init(str, _fontData, letterSpace, iniSize, _color, &fontSize, &color);
					lineNode->setAnchorPoint(Vec2::ANCHOR_TOP_LEFT);
				}
				else {
					// ノードが足りないので作成
					lineNode = agtk::TextLineNode::create(str, _fontData, letterSpace, iniSize, _color, &fontSize, &color);
					lineNode->setAnchorPoint(Vec2::ANCHOR_TOP_LEFT);
					lineNodeList->addObject(lineNode);
				}

				auto size = lineNode->getContentSize();
				if (maxW <= size.width) {
					maxW = size.width;
				}
				maxH += size.height;
				firstH = lineNum == 0 ? maxH : firstH;// 行間の値が負数で座標が-になってしまった時の対応
				maxH += lineSpace;

				++lineNum;
			}
			maxH -= lineSpace;// 最後の行の行間は不要。

			// 余分なノード削除
			for (lineNum; lineNum < lineNodeList->count(); ++lineNum) {
				lineNodeList->removeObjectAtIndex(lineNum);
			}
		}

		if (maxH <= 0) {
			maxH = firstH;
		}

		// 横幅、縦幅のどちらかが0の場合、
		// レンダーテクスチャを生成すると失敗するので処理しない
		if (maxW <= 0 || maxH <= 0) {
			delTexFunc();
			return;
		}

		this->setRealHeight(maxH);
		//float realHeight = maxH;

		if (width > 0) {
			maxW = width;
		}

		if (height > 0) {
			maxH = height;
		}

		// マージン値を考慮して最大幅を変動させる
		maxW -= marginLR * 2;
		maxH -= marginTB * 2;

		// 横幅、縦幅のどちらかが0の場合、
		// レンダーテクスチャを生成すると失敗するので処理しない
		if (maxW <= 0 || maxH <= 0) {
			delTexFunc();
			return;
		}

		// 横幅と高さを2の倍数となるようにする
		auto modMaxW = fmod(maxW, 2.0f);
		auto modMaxH = fmod(maxH, 2.0f);
		if (modMaxW != 0) {
			maxW = maxW - modMaxW + 2.0f;
		}
		if (modMaxH != 0) {
			maxH = maxH - modMaxH + 2.0f;
		}

		// 文字をレンダーテクスチャに焼き付ける
		if (_width != maxW || _height != maxH || _baseNode == nullptr) {
			delTexFunc();
			_baseNode = cocos2d::Node::create();
			_baseNode->setContentSize(cocos2d::Size(maxW, maxH));
			_baseNode->setAnchorPoint(cocos2d::Vec2::ANCHOR_MIDDLE);
			_baseNode->retain();
			this->addChild(_baseNode);

			_width = maxW;
			_height = maxH;
			_textWidth = maxW;
			_textHeight = maxH;
		}
		else {
			_baseNode->removeAllChildren();
		}

		// 揃える方向に合わせて座標を設定
		Vec2 texPos = Vec2(maxW * _horzAlign, maxH * _vertAlign);
		// マージンを考慮してテクスチャの位置をずらす
		if (_horzAlign < 0) {
			texPos.x -= marginLR;
		}
		else if (_horzAlign > 0) {
			texPos.x += marginLR;
		}

		if (_vertAlign < 0) {
			texPos.y -= marginTB;
		}
		else if (_vertAlign > 0) {
			texPos.y += marginTB;
		}

		_baseNode->setPosition(texPos);
		{
			auto lineNodeList = this->getLineNodeList();

			// 左寄せ：0 中央:0.5 右寄せ：1 
			float horz = (-_textHorzAlign + 0.5f);
			// 上段：1 中央：0.5 下段：0
			float vert = (-_vertAlign + 0.5f);

			maxH = roundf(maxH * vert + _realHeight * (1.0f - vert));

			for (int i = 0; i < lineNodeList->count(); i++) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
				agtk::TextLineNode* lineNode = static_cast<agtk::TextLineNode*>(lineNodeList->getObjectAtIndex(i));
#else
				agtk::TextLineNode* lineNode = dynamic_cast<agtk::TextLineNode*>(lineNodeList->getObjectAtIndex(i));
#endif

				auto size = lineNode->getContentSize();

				lineNode->setPosition(roundf((maxW - size.width) * horz), maxH + size.height);

				cocos2d::Ref *ref2 = nullptr;
				CCARRAY_FOREACH(lineNode->getNodeList(), ref2) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
					auto node = static_cast<cocos2d::Node*>(ref2);
#else
					auto node = dynamic_cast<cocos2d::Node*>(ref2);
#endif

					// α値を設定
					node->setOpacity(_alpha);
				}

				lineNode->removeFromParent();
				_baseNode->addChild(lineNode);

				maxH -= size.height;

				if (i < lineNodeList->count() - 1) {
					maxH -= lineSpace;
				}
			}
		}
	}

	setContentSize(Size(_width, _height));
}

/**
* 「スクロールメッセージを設定」で表示するテキストUIの更新(クリッピング用)
*/
void TextGui::updateText(std::string text, int letterSpace, int lineSpace, float width, float height, int marginLR, int marginTB, float scrollY)
{
	_letterSpace = letterSpace;
	_lineSpace = lineSpace;

	float maxW = 0;
	float maxH = 0;
	float firstH = 0;
	int iniSize = 0;
	if (_fontData->getMainFontSetting()->getImageFontFlag()) {
		if (_fontData->getMainFontSetting()->getImageId() >= 0) {
			auto project = GameManager::getInstance()->getProjectData();
			auto imageData = project->getImageData(_fontData->getMainFontSetting()->getImageId());
			if (imageData) {
				iniSize = _fontData->getMainFontSetting()->getVertDivCount() ? imageData->getTexHeight() / _fontData->getMainFontSetting()->getVertDivCount() : 0;
			}
		}
	}
	else {
		iniSize = _fontData->getMainFontSetting()->getFontSize();
	}
	int fontSize = iniSize;
	auto iniColor = _color;
	cocos2d::Color3B color = iniColor;

	if (_width != width || _height != height) {
		if (_baseNode != nullptr) {
			this->removeChild(_baseNode);
			CC_SAFE_RELEASE_NULL(_baseNode);
		}
	}

	if (_string.compare(text) != 0) {
		_string = text;

		auto lineNodeList = this->getLineNodeList();

		// すでにノードを生成している場合は一度破棄する
		if (lineNodeList->count() > 0) {
			lineNodeList->removeAllObjects();
		}

		// レンダーテクスチャを生成している場合は一度破棄する
		if (_baseNode != nullptr) {
			CC_SAFE_RELEASE_NULL(_baseNode);
		}

		this->removeAllChildren();

		// 文字列が空の場合は処理しない
		if (_string.size() <= 0) {
			return;
		}

		FontManager *fm = FontManager::getInstance();

		// 改行コードをもとに文字列を分割
		std::vector<std::string> v;
		std::stringstream ss(_string);
		std::string buffer;
		while (std::getline(ss, buffer)) {
			v.push_back(buffer);
		}

		// 各行ごとにノードを生成
		for (unsigned int i = 0; i < v.size(); i++) {
			std::string str = v[i];

			auto lineNode = agtk::TextLineNode::create(str, _fontData, letterSpace, iniSize, iniColor, &fontSize, &color);
			lineNode->setAnchorPoint(Vec2::ANCHOR_TOP_LEFT);

			auto size = lineNode->getContentSize();

			lineNodeList->addObject(lineNode);

			if (maxW <= size.width) {
				maxW = size.width;
			}

			maxH += size.height;

			// 行間の値が負数で座標が-になってしまった時の対応
			if (i == 0) {
				firstH = maxH;
			}

			if (i < v.size() - 1) {
				maxH += lineSpace;
			}
		}

		if (maxH <= 0) {
			maxH = firstH;
		}
	}
	else {
		maxW = _textWidth;
		maxH = _textHeight;
	}

	// マージン値を考慮して最大幅を変動させる
	//maxW -= marginLR * 2;
	width -= marginLR * 2;
	//maxH -= marginTB * 2;
	height -= marginTB * 2;

	// 横幅、縦幅のどちらかが0の場合、
	// レンダーテクスチャを生成すると失敗するので処理しない
	if (maxW <= 0 || maxH <= 0 || width <= 0 || height <= 0) {
		return;
	}

	if (_baseNode == nullptr) {
		_baseNode = cocos2d::Node::create();
		_baseNode->setContentSize(cocos2d::Size(width,height));
		_baseNode->setAnchorPoint(cocos2d::Vec2::ANCHOR_MIDDLE);
		this->addChild(_baseNode);
	}
	else {
		_baseNode->removeAllChildren();
	}
	_width = width;
	_height = height;

	// 揃える方向に合わせて座標を設定
	Vec2 texPos = Vec2(_width * _horzAlign, maxH * _vertAlign);
	// マージンを考慮してテクスチャの位置をずらす
	if (_horzAlign < 0) {
		texPos.x -= marginLR;
	}
	else if (_horzAlign > 0) {
		texPos.x += marginLR;
	}

	if (_vertAlign < 0) {
		texPos.y -= marginTB;
	}
	else if (_vertAlign > 0) {
		texPos.y += marginTB;
	}

	_baseNode->setPosition(texPos);

	float offsetY = (height - _textHeight) * 0.5f;
	_baseNode->retain();
	{
		auto lineNodeList = this->getLineNodeList();

		// 左寄せ：0 中央:0.5 右寄せ：1 
		float horz = (-_textHorzAlign + 0.5f);

		for (int i = 0; i < lineNodeList->count(); i++) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
			agtk::TextLineNode* lineNode = static_cast<agtk::TextLineNode*>(lineNodeList->getObjectAtIndex(i));
#else
			agtk::TextLineNode* lineNode = dynamic_cast<agtk::TextLineNode*>(lineNodeList->getObjectAtIndex(i));
#endif

			auto size = lineNode->getContentSize();

			float x = (width - _textWidth) * (0.5f - _horzAlign);
			lineNode->setPosition(x + (maxW - size.width) * horz, maxH + size.height  + offsetY + scrollY);

			cocos2d::Ref *ref2 = nullptr;
			CCARRAY_FOREACH(lineNode->getNodeList(), ref2) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
				auto node = static_cast<cocos2d::Node*>(ref2);
#else
				auto node = dynamic_cast<cocos2d::Node*>(ref2);
#endif

				// α値を設定
				node->setOpacity(_alpha);
			}

			lineNode->removeFromParent();
			_baseNode->addChild(lineNode);

			maxH -= size.height;

			if (i < lineNodeList->count() - 1) {
				maxH -= lineSpace;
			}
		}
	}

	setContentSize(Size(_width, _height));
}

/**
* 「表示するテキスト」で表示するテキストUIの更新(レンダー用)
*/
void TextGui::updateTextRender(std::string text, int letterSpace, int lineSpace, float width, float height, int marginLR, int marginTB)
{
	{
		auto delTexFunc = [&] {
			if (_renderTex != nullptr) {
				CC_SAFE_RELEASE_NULL(_renderTex);
			}
			this->removeAllChildren();
		};
		_letterSpace = letterSpace;
		_lineSpace = lineSpace;
		_string = text;

		auto lineNodeList = this->getLineNodeList();

		// 文字列が空の場合は処理しない
		if (_string.size() <= 0) {
			delTexFunc();
			return;
		}

		float maxW = 0;
		float maxH = 0;
		float firstH = 0;
		int iniSize = 0;
		if (_fontData->getMainFontSetting()->getImageFontFlag()) {
			if (_fontData->getMainFontSetting()->getImageId() >= 0) {
				auto project = GameManager::getInstance()->getProjectData();
				auto imageData = project->getImageData(_fontData->getMainFontSetting()->getImageId());
				if (imageData) {
					iniSize = _fontData->getMainFontSetting()->getVertDivCount() ? imageData->getTexHeight() / _fontData->getMainFontSetting()->getVertDivCount() : 0;
				}
			}
		}
		else {
			iniSize = _fontData->getMainFontSetting()->getFontSize();
		}

		// 各行ごとにノードを生成
		{
			std::stringstream ss(_string);
			std::string str;
			int lineNum = 0;
			cocos2d::Color3B color = _color;
			int fontSize = iniSize;
			while (std::getline(ss, str)) {
				agtk::TextLineNode*	lineNode = nullptr;
				if (lineNum < lineNodeList->count()) {
					// 既にノードが存在するなら中身を書き換え
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
					lineNode = static_cast<agtk::TextLineNode*>(lineNodeList->getObjectAtIndex(lineNum));
#else
					lineNode = dynamic_cast<agtk::TextLineNode*>(lineNodeList->getObjectAtIndex(lineNum));
#endif
					lineNode->init(str, _fontData, letterSpace, iniSize, _color, &fontSize, &color);
					lineNode->setAnchorPoint(Vec2::ANCHOR_TOP_LEFT);
				}
				else {
					// ノードが足りないので作成
					lineNode = agtk::TextLineNode::create(str, _fontData, letterSpace, iniSize, _color, &fontSize, &color);
					lineNode->setAnchorPoint(Vec2::ANCHOR_TOP_LEFT);
					lineNodeList->addObject(lineNode);
				}

				auto size = lineNode->getContentSize();
				if (maxW <= size.width) {
					maxW = size.width;
				}
				maxH += size.height;
				firstH = lineNum == 0 ? maxH : firstH;// 行間の値が負数で座標が-になってしまった時の対応
				maxH += lineSpace;

				++lineNum;
			}
			maxH -= lineSpace;// 最後の行の行間は不要。

							  // 余分なノード削除
			for (lineNum; lineNum < lineNodeList->count(); ++lineNum) {
				lineNodeList->removeObjectAtIndex(lineNum);
			}
		}

		if (maxH <= 0) {
			maxH = firstH;
		}

		// 横幅、縦幅のどちらかが0の場合、
		// レンダーテクスチャを生成すると失敗するので処理しない
		if (maxW <= 0 || maxH <= 0) {
			delTexFunc();
			return;
		}

		this->setRealHeight(maxH);
		//float realHeight = maxH;

		if (width > 0) {
			maxW = width;
		}

		if (height > 0) {
			maxH = height;
		}

		// マージン値を考慮して最大幅を変動させる
		maxW -= marginLR * 2;
		maxH -= marginTB * 2;

		// 横幅、縦幅のどちらかが0の場合、
		// レンダーテクスチャを生成すると失敗するので処理しない
		if (maxW <= 0 || maxH <= 0) {
			delTexFunc();
			return;
		}

		// 横幅と高さを2の倍数となるようにする
		auto modMaxW = fmod(maxW, 2.0f);
		auto modMaxH = fmod(maxH, 2.0f);
		if (modMaxW != 0) {
			maxW = maxW - modMaxW + 2.0f;
		}
		if (modMaxH != 0) {
			maxH = maxH - modMaxH + 2.0f;
		}

		// 文字をレンダーテクスチャに焼き付ける
		if (_width != maxW || _height != maxH || _renderTex == nullptr) {
			delTexFunc();
			_renderTex = cocos2d::RenderTexture::create(maxW, maxH, cocos2d::Texture2D::PixelFormat::RGBA8888);
			this->addChild(_renderTex);
			_renderTex->retain();

			_width = maxW;
			_height = maxH;
			_textWidth = maxW;
			_textHeight = maxH;
		}

		// 揃える方向に合わせて座標を設定
		Vec2 texPos = Vec2(maxW * _horzAlign, maxH * _vertAlign);
		// マージンを考慮してテクスチャの位置をずらす
		if (_horzAlign < 0) {
			texPos.x -= marginLR;
		}
		else if (_horzAlign > 0) {
			texPos.x += marginLR;
		}

		if (_vertAlign < 0) {
			texPos.y -= marginTB;
		}
		else if (_vertAlign > 0) {
			texPos.y += marginTB;
		}

		_renderTex->setPosition(texPos);
		{
			_renderTex->setVisible(true);
			_renderTex->setKeepMatrix(true);
			_renderTex->beginWithClear(0, 0, 0, 0);

			auto lineNodeList = this->getLineNodeList();

			// 左寄せ：0 中央:0.5 右寄せ：1 
			float horz = (-_textHorzAlign + 0.5f);
			// 上段：1 中央：0.5 下段：0
			float vert = (-_vertAlign + 0.5f);

			maxH = roundf(maxH * vert + _realHeight * (1.0f - vert));

			for (int i = 0; i < lineNodeList->count(); i++) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
				agtk::TextLineNode* lineNode = static_cast<agtk::TextLineNode*>(lineNodeList->getObjectAtIndex(i));
#else
				agtk::TextLineNode* lineNode = dynamic_cast<agtk::TextLineNode*>(lineNodeList->getObjectAtIndex(i));
#endif

				auto size = lineNode->getContentSize();

				lineNode->setPosition(roundf((maxW - size.width) * horz), maxH + size.height);

				cocos2d::Ref *ref2 = nullptr;
				CCARRAY_FOREACH(lineNode->getNodeList(), ref2) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
					auto node = static_cast<cocos2d::Node*>(ref2);
#else
					auto node = dynamic_cast<cocos2d::Node*>(ref2);
#endif

					// α値を設定
					node->setOpacity(_alpha);
				}

				lineNode->visit();

				maxH -= size.height;

				if (i < lineNodeList->count() - 1) {
					maxH -= lineSpace;
				}
			}
			_renderTex->end();
			_renderTex->setKeepMatrix(false);
		}
	}

	setContentSize(Size(_width, _height));
}

/**
* 「スクロールメッセージを設定」で表示するテキストUIの更新(レンダー用)
*/
void TextGui::updateTextRender(std::string text, int letterSpace, int lineSpace, float width, float height, int marginLR, int marginTB, float scrollY)
{
	_letterSpace = letterSpace;
	_lineSpace = lineSpace;

	float maxW = 0;
	float maxH = 0;
	float firstH = 0;
	int iniSize = 0;
	if (_fontData->getMainFontSetting()->getImageFontFlag()) {
		if (_fontData->getMainFontSetting()->getImageId() >= 0) {
			auto project = GameManager::getInstance()->getProjectData();
			auto imageData = project->getImageData(_fontData->getMainFontSetting()->getImageId());
			if (imageData) {
				iniSize = _fontData->getMainFontSetting()->getVertDivCount() ? imageData->getTexHeight() / _fontData->getMainFontSetting()->getVertDivCount() : 0;
			}
		}
	}
	else {
		iniSize = _fontData->getMainFontSetting()->getFontSize();
	}
	int fontSize = iniSize;
	auto iniColor = _color;
	cocos2d::Color3B color = iniColor;

	if (_width != width || _height != height) {
		if (_renderTex != nullptr) {
			this->removeChild(_renderTex);
			CC_SAFE_RELEASE_NULL(_renderTex);
		}
	}

	if (_string.compare(text) != 0) {
		_string = text;

		auto lineNodeList = this->getLineNodeList();

		// すでにノードを生成している場合は一度破棄する
		if (lineNodeList->count() > 0) {
			lineNodeList->removeAllObjects();
		}

		// レンダーテクスチャを生成している場合は一度破棄する
		if (_renderTex != nullptr) {
			CC_SAFE_RELEASE_NULL(_renderTex);
		}

		this->removeAllChildren();

		// 文字列が空の場合は処理しない
		if (_string.size() <= 0) {
			return;
		}

		FontManager *fm = FontManager::getInstance();

		// 改行コードをもとに文字列を分割
		std::vector<std::string> v;
		std::stringstream ss(_string);
		std::string buffer;
		while (std::getline(ss, buffer)) {
			v.push_back(buffer);
		}

		// 各行ごとにノードを生成
		for (unsigned int i = 0; i < v.size(); i++) {
			std::string str = v[i];

			auto lineNode = agtk::TextLineNode::create(str, _fontData, letterSpace, iniSize, iniColor, &fontSize, &color);
			lineNode->setAnchorPoint(Vec2::ANCHOR_TOP_LEFT);

			auto size = lineNode->getContentSize();

			lineNodeList->addObject(lineNode);

			if (maxW <= size.width) {
				maxW = size.width;
			}

			maxH += size.height;

			// 行間の値が負数で座標が-になってしまった時の対応
			if (i == 0) {
				firstH = maxH;
			}

			if (i < v.size() - 1) {
				maxH += lineSpace;
			}
		}

		if (maxH <= 0) {
			maxH = firstH;
		}
	}
	else {
		maxW = _textWidth;
		maxH = _textHeight;
	}

	// マージン値を考慮して最大幅を変動させる
	//maxW -= marginLR * 2;
	width -= marginLR * 2;
	//maxH -= marginTB * 2;
	height -= marginTB * 2;

	// 横幅、縦幅のどちらかが0の場合、
	// レンダーテクスチャを生成すると失敗するので処理しない
	if (maxW <= 0 || maxH <= 0 || width <= 0 || height <= 0) {
		return;
	}

	if (_renderTex == nullptr) {
		// 文字をレンダーテクスチャに焼き付ける
		_renderTex = cocos2d::RenderTexture::create(width, height, cocos2d::Texture2D::PixelFormat::RGBA8888);
		this->addChild(_renderTex);
	}
	_width = width;
	_height = height;

	// 揃える方向に合わせて座標を設定
	Vec2 texPos = Vec2(_width * _horzAlign, maxH * _vertAlign);
	// マージンを考慮してテクスチャの位置をずらす
	if (_horzAlign < 0) {
		texPos.x -= marginLR;
	}
	else if (_horzAlign > 0) {
		texPos.x += marginLR;
	}

	if (_vertAlign < 0) {
		texPos.y -= marginTB;
	}
	else if (_vertAlign > 0) {
		texPos.y += marginTB;
	}

	_renderTex->setPosition(texPos);

	float offsetY = (height - _textHeight) * 0.5f;
	_renderTex->retain();
	{
		_renderTex->setVisible(true);
		_renderTex->setKeepMatrix(true);
		_renderTex->beginWithClear(0, 0, 0, 0);

		auto lineNodeList = this->getLineNodeList();

		// 左寄せ：0 中央:0.5 右寄せ：1 
		float horz = (-_textHorzAlign + 0.5f);

		for (int i = 0; i < lineNodeList->count(); i++) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
			agtk::TextLineNode* lineNode = static_cast<agtk::TextLineNode*>(lineNodeList->getObjectAtIndex(i));
#else
			agtk::TextLineNode* lineNode = dynamic_cast<agtk::TextLineNode*>(lineNodeList->getObjectAtIndex(i));
#endif

			auto size = lineNode->getContentSize();

			float x = (width - _textWidth) * (0.5f - _horzAlign);
			lineNode->setPosition(x + (maxW - size.width) * horz, maxH + size.height + offsetY + scrollY);

			cocos2d::Ref *ref2 = nullptr;
			CCARRAY_FOREACH(lineNode->getNodeList(), ref2) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
				auto node = static_cast<cocos2d::Node*>(ref2);
#else
				auto node = dynamic_cast<cocos2d::Node*>(ref2);
#endif

				// α値を設定
				node->setOpacity(_alpha);
			}

			lineNode->visit();

			maxH -= size.height;

			if (i < lineNodeList->count() - 1) {
				maxH -= lineSpace;
			}
		}
		_renderTex->end();
		_renderTex->setKeepMatrix(false);
	}

	setContentSize(Size(_width, _height));
}

void TextGui::setAlign(float horz, float vert)
{
	_horzAlign = horz;
	_vertAlign = vert;

	_textHorzAlign = _horzAlign;
}

//-------------------------------------------------------------------------------------------------------------------
ObjectParameterUi::ObjectParameterUi()
{
	_displayData = nullptr;
}

ObjectParameterUi::~ObjectParameterUi()
{
	_displayData = nullptr;
}

agtk::data::PlayVariableData * ObjectParameterUi::getPlayVariableData(bool UseParentVariable, int _variableObjectId, int _variableId)
{
	agtk::data::PlayVariableData * variableData = nullptr;

	// 「親オブジェクトの同名の変数を選択」にチェックが入っている場合
	if (UseParentVariable) {
		// 参照元リストを無視して、親オブジェクトからデータを取得する
		if (_targetObject->getOwnParentObject()) {
			variableData = _targetObject->getOwnParentObject()->getPlayObjectData()->getVariableData(_variableId);
		}
	}

	if (variableData == nullptr) {

		auto projectPlayData = GameManager::getInstance()->getPlayData();

		// 「player共通」変数使用時
		if (_variableObjectId == agtk::data::PlayData::COMMON_PLAY_DATA_ID) {
			variableData = projectPlayData->getVariableData(agtk::data::PlayData::COMMON_PLAY_DATA_ID, _variableId);
		}
		// 「自身のオブジェクト」変数使用時
		else if (_variableObjectId == -2) {
			variableData = _targetObject->getPlayObjectData()->getVariableData(_variableId);
		}
		// 共通オブジェクト選択時
		else if (_variableObjectId > 0) {
			// 単体指定の変数データリストを取得
			auto gameManager = GameManager::getInstance();
			auto variableDataList = cocos2d::Array::create();
			gameManager->getSwitchVariableDataList(agtk::data::kQualifierSingle, _variableObjectId, _variableId, false, variableDataList);

			// リストにデータが設定されている場合、リストの先頭を使用する
			if (variableDataList->count() > 0) {
				variableData = dynamic_cast<agtk::data::PlayVariableData*>(variableDataList->getObjectAtIndex(0));
			}
		}
		// オブジェクトが選択されていない場合
		else {
			return nullptr;
		}
	}

	return variableData;
}

//-------------------------------------------------------------------------------------------------------------------
ObjectParameterTextUi::ObjectParameterTextUi()
{
	_textGui = nullptr;
}

ObjectParameterTextUi::~ObjectParameterTextUi()
{
	CC_SAFE_RELEASE_NULL(_textGui);
}

bool ObjectParameterTextUi::init(agtk::Object * targetObject, agtk::data::ObjectAdditionalDisplayData * displayData)
{
	_targetObject = targetObject;
	_displayData = displayData;

	int fontId = -1;
	cocos2d::Color3B color;
	char alpha;

	// プロジェクトデータを取得
	auto projectData = GameManager::getInstance()->getProjectData();
	CCASSERT(projectData, "");

	// テキストを表示する場合
	if (_displayData->getShowText()) {
		// テキスト素材ID
		int textId = _displayData->getTextId();

		if (textId < 0) {// 設定無し。
			return false;
		}

		// テキストデータを取得
		auto textData = projectData->getTextData(textId);

		// フォントIDを取得
		fontId = textData->getFontId();

		// フォントが「設定無し」の場合は処理しない
		if (fontId < 0) {
			return false;
		}

		// 色を設定
		color.r = displayData->getTextColorR();
		color.g = displayData->getTextColorG();
		color.b = displayData->getTextColorB();

		// α値を設定
		alpha = displayData->getTextColorA();
	}
	else {
		// フォントIDを取得
		fontId = _displayData->getParamTextFontId();

		// フォントが「設定無し」の場合は処理しない
		if (fontId < 0) {
			return false;
		}

		// 色を設定
		color.r = displayData->getParamTextColorR();
		color.g = displayData->getParamTextColorG();
		color.b = displayData->getParamTextColorB();

		// α値を設定
		alpha = displayData->getParamTextColorA();
	}

	// テキスト表示用GUIを生成
	auto textGui = agtk::TextGui::create(fontId, color, alpha);
	this->setTextGui(textGui);
	this->addChild(textGui);
	_targetObject->addChild(this);

	// テキストを表示する場合
	if (_displayData->getShowText()) {
		// テキスト素材ID
		int textId = _displayData->getTextId();

		// テキストデータを取得
		auto textData = projectData->getTextData(textId);

		if (textData) {
			// テキストデータを更新
// #AGTK-NX
#if CC_TARGET_PLATFORM != CC_PLATFORM_NX
			getTextGui()->updateText(projectData->getExpandedText(nullptr, textData->getText(nullptr), std::list<int>(1, textId)), textData->getLetterSpacing(), textData->getLineSpacing(), -1, -1, 0, 0);
#else
#endif
			// 角度を設定
			getTextGui()->setRotation(_displayData->getRotation());
			// スケールを設定
			float scaleX = _displayData->getScaleX() * _targetObject->getScaleX();
			float scaleY = _displayData->getScaleY() * _targetObject->getScaleY();
			getTextGui()->setScaleX(scaleX * 0.01f);
			getTextGui()->setScaleY(scaleY * 0.01f);
		}
	}

	return true;
}

void ObjectParameterTextUi::update(float delta)
{
	// 追従対象が無くなった場合、処理できないので削除する
	if (_targetObject == nullptr) {
		_isDelete = true;
		return;
	}

	// 座標を設定
	cocos2d::Vec2 pos = _targetObject->getPosition();
	auto textGui = this->getTextGui();

	float scaleX = _displayData->getScaleX() * _targetObject->getScaleX() * 0.01f;
	float scaleY = _displayData->getScaleY() * _targetObject->getScaleY() * 0.01f;
	textGui->setScaleX(scaleX);
	textGui->setScaleY(scaleY);

	// オフセットを反映
	pos.x += _displayData->getAdjustX() * scaleX;
	pos.y += _displayData->getAdjustY() * scaleY;

	// テキストを表示を行わず、パラメータ表示を行う場合
	if (!_displayData->getShowText()) {
		// 表示文字列
		string str = "";

		// 変数データ
		agtk::data::PlayVariableData * variableData = getPlayVariableData(_displayData->getUseParentVariable(), _displayData->getVariableObjectId(), _displayData->getVariableId());

		// 変数データの取得に成功している場合
		if (variableData) {
			// 変数を表示
			double value = variableData->getValue();
			if (std::isnan(value)) {
				//非数の場合は空文字列にする。
				str = "";
			}
			else {
				str = std::to_string(value);

				auto list = std::vector<std::string>();
				auto separator = std::string(".");
				auto offset = std::string::size_type(0);
				while (1) {
					auto pos = str.find(separator, offset);
					if (pos == std::string::npos) {
						list.push_back(str.substr(offset));
						break;
					}
					list.push_back(str.substr(offset, pos - offset));
					offset = pos + separator.length();
				}

				// 小数点があるかをチェックし、小数点がある場合
				double decimal = value - (int)value;
				if (decimal != 0) {

					// 小数点以下第二位までを表示
					if (list[1].size() > 2) {
						list[1].erase(2, list[1].size() - 2);
					}

					// 小数点第二位が0の場合は表示を削る
					//if (list[1][1] == '0') {
					//	list[1].erase(1, 1);
					//}

					str = list[0] + "." + list[1];
				}
				// 小数点がない場合
				else {
					// 整数部分だけで表示
					str = list[0];
				}
			}
		}

		// テキストに変更が発生した場合
		if (str.compare(getTextGui()->getString()) != 0) {
			// テキストを更新
			getTextGui()->updateText(str, 0, 0, -1, -1, 0, 0);
			// 角度を設定
			getTextGui()->setRotation(_displayData->getRotation());
			// スケールを設定
			float scaleX = _displayData->getScaleX() * _targetObject->getScaleX();
			float scaleY = _displayData->getScaleY() * _targetObject->getScaleY();
			getTextGui()->setScaleX(scaleX * 0.01f);
			getTextGui()->setScaleY(scaleY * 0.01f);
		}
	}

	// 座標を更新
	pos = this->getPositionTransform(pos);

	//浮動小数点切り捨て
	pos.x = (int)pos.x;
	pos.y = (int)pos.y;

	getTextGui()->setPosition(pos);
}

void ObjectParameterTextUi::remove()
{
	if (_targetObject) {
		_targetObject->removeChild(this);
	}
}

//-------------------------------------------------------------------------------------------------------------------
const float ObjectParameterGaugeUi::DEFAULT_WIDTH = 24.0f;
const float ObjectParameterGaugeUi::DEFAULT_HEIGTH = 12.0f;

ObjectParameterGaugeUi::ObjectParameterGaugeUi()
{
	_drawNode = nullptr;
	_addOrder = 0;
}

ObjectParameterGaugeUi::~ObjectParameterGaugeUi()
{
	CC_SAFE_RELEASE_NULL(_drawNode);
}

bool ObjectParameterGaugeUi::init(agtk::Object * targetObject, agtk::data::ObjectAdditionalDisplayData * displayData)
{
	_targetObject = targetObject;
	_displayData = displayData;

	// 描画用ノードを生成
	this->_drawNode = cocos2d::DrawNode::create();
	CC_SAFE_RETAIN(this->_drawNode);
	this->addChild(this->_drawNode);
	_targetObject->addChild(this, _targetObject->getLocalZOrder() + this->getAddOrder());

	// 幅を設定
	width = DEFAULT_WIDTH * _displayData->getScaleX() * 0.01f * _targetObject->getScaleX();
	height = DEFAULT_HEIGTH * _displayData->getScaleY() * 0.01f * _targetObject->getScaleY();

	// 色を設定
	_color.r = displayData->getParamGaugeColorR() / 255.0f;
	_color.g = displayData->getParamGaugeColorG() / 255.0f;
	_color.b = displayData->getParamGaugeColorB() / 255.0f;
	_color.a = displayData->getParamGaugeColorA() / 255.0f;

	_bgColor.r = displayData->getParamGaugeBgColorR() / 255.0f;
	_bgColor.g = displayData->getParamGaugeBgColorG() / 255.0f;
	_bgColor.b = displayData->getParamGaugeBgColorB() / 255.0f;
	_bgColor.a = displayData->getParamGaugeBgColorA() / 255.0f;

	return true;
}

void ObjectParameterGaugeUi::update(float delta)
{
	this->_drawNode->clear();

	// 追従対象が無くなった場合、処理できないので削除する
	if (_targetObject == nullptr) {
		_isDelete = true;
		return;
	}

	bool bCreateSkipFrameFlag = false;
	auto scene = GameManager::getInstance()->getCurrentScene();
	if (scene != nullptr && scene->getSceneCreateSkipFrameFlag()) {
		bCreateSkipFrameFlag = true;
	}

	// 変数データを先に設定
	agtk::data::PlayVariableData * variableMaxData = nullptr;
	if (_displayData->getVariableMaxEnabled()) {
		variableMaxData = getPlayVariableData(_displayData->getVariableMaxUseParent(), _displayData->getVariableMaxObjectId(), _displayData->getVariableMaxVariableId());
	}
	agtk::data::PlayVariableData * variableData = getPlayVariableData(_displayData->getUseParentVariable(), _displayData->getVariableObjectId(), _displayData->getVariableId());

	// 変数データの取得に失敗しているので処理しない
	if (variableData == nullptr) {
		return;
	}

	float val = variableData->getValue();

	float max;
	if (variableMaxData == nullptr) {
		max = variableData->getVariableData()->getInitialValue();
	}
	else {
		max = variableMaxData->getValue();
	}
#if 0//「ACT2-3666 最大体力を変数で増やしてもゲージに反映されない」を修正前に戻す。sakihama-h, 2018.12.06
	if (variableData->getId() == agtk::data::EnumObjectSystemVariable::kObjectSystemVariableHP && _targetObject) {//体力値は最大体力値を上限として処理。
		auto maxVariableData = _targetObject->getPlayObjectData()->getVariableData(agtk::data::EnumObjectSystemVariable::kObjectSystemVariableMaxHP);
		max = maxVariableData->getValue();
	}
#endif

	if (max <= 0.0f) {
		// ゲージの最大値が0なので割り算できない
		CCASSERT(0, "");
		max = 1.0f;
	}

	// マイナス表示を行わないために値を0にする
	if (val < 0) {
		val = 0;
	}
	value = val;
	valueMax = max;

	// ゲージの表示割合を設定
	float lengthRate = val / max;
	if (lengthRate > 1.0f) {
		lengthRate = 1.0f;
	}

	// 上限に合わせてスケールを自動調整
	if (_displayData->getVariableMaxAutoScalingEnabled() && _displayData->getVariableMaxEnabled()) {
		double scale = max * _displayData->getScaleX() / variableMaxData->getVariableData()->getInitialValue();
		width = DEFAULT_WIDTH * scale * 0.01f * _targetObject->getScaleX();
	}

	cocos2d::Vec2 gaugeOrigin;
	gaugeOrigin.x = (int)_targetObject->getPosition().x;//ゲージの高さが少数点値によって高さの表示が変わるので整数に変換。
	gaugeOrigin.y = (int)_targetObject->getPosition().y;//ゲージの高さが少数点値によって高さの表示が変わるので整数に変換。

	float scaleX = _targetObject->getScaleX();
	float scaleY = _targetObject->getScaleY();

	// オフセットを反映
	gaugeOrigin.x += _displayData->getAdjustX() * scaleX;
	gaugeOrigin.y += _displayData->getAdjustY() * scaleY;

	// 描画ノードの位置を設定
	this->_drawNode->setPosition(this->getPositionTransform(gaugeOrigin));

	// ゲージの角度を設定		
	this->_drawNode->setRotation(_displayData->getRotation());

	cocos2d::Vec2 origin = cocos2d::Vec2(-width * 0.5f, -height * 0.5f);
	cocos2d::Vec2 destination = cocos2d::Vec2(width * 0.5f, height * 0.5f);

	// 背景を描画
	auto bgColor = _bgColor;
	if (bCreateSkipFrameFlag) bgColor.a = 0.0f;
	origin.x -= 1;
	origin.y -= 1;
	destination.x += 1;
	destination.y += 1;
	this->_drawNode->drawSolidRect(origin, destination, bgColor);

	// ゲージを描画
	auto color = _color;
	if (bCreateSkipFrameFlag) color.a = 0.0f;
	origin = cocos2d::Vec2(-width * 0.5f, -height * 0.5f);
	destination = cocos2d::Vec2(width * 0.5f, height * 0.5f);
	destination.x = origin.x;
	destination.x += width * lengthRate;
	this->_drawNode->drawSolidRect(origin, destination, color);

	if(this->getLocalZOrder() != _targetObject->getLocalZOrder() + this->getAddOrder()) {
		_targetObject->reorderChild(this, _targetObject->getLocalZOrder() + this->getAddOrder());
	}
}

void ObjectParameterGaugeUi::remove()
{
	this->removeChild(this->_drawNode);

	if (_targetObject) {
		_targetObject->removeChild(this);
	}
}

//-------------------------------------------------------------------------------------------------------------------
DummyParameterGaugeUi::DummyParameterGaugeUi()
{
	_drawNode = nullptr;
	_player = nullptr;
}

DummyParameterGaugeUi::~DummyParameterGaugeUi()
{
	CC_SAFE_RELEASE_NULL(_drawNode);
}

bool DummyParameterGaugeUi::init(agtk::Player *player, agtk::ObjectParameterGaugeUi *gaugeUi)
{
	if (player == nullptr || gaugeUi == nullptr) {
		return false;
	}
	_player = player;
	auto object = gaugeUi->getTargetObject();

	// 描画用ノードを生成
	this->_drawNode = cocos2d::DrawNode::create();
	CC_SAFE_RETAIN(this->_drawNode);
	this->addChild(this->_drawNode);
	_player->addChild(this);

	// スケール
	_scale.x = object->getScaleX();
	_scale.y = object->getScaleY();

	// 色を設定
	_color = gaugeUi->_color;
	_bgColor = gaugeUi->_bgColor;

	//幅高さ
	width = gaugeUi->width;
	height = gaugeUi->height;

	//値
	value = gaugeUi->value;
	valueMax = gaugeUi->valueMax;

	//調整位置。
	auto displayData = gaugeUi->getDisplayData();
	_adjustPos.x = displayData->getAdjustX();
	_adjustPos.y = displayData->getAdjustY();

	//回転
	_rotation = displayData->getRotation();

	return true;
}

void DummyParameterGaugeUi::update(float delta)
{
	// 追従対象が無くなった場合、処理できないので削除する
	if (_player == nullptr) {
		return;
	}
	this->_drawNode->clear();

	auto val = value;
	auto max = valueMax;

	// ゲージの表示割合を設定
	float lengthRate = val / max;
	if (lengthRate > 1.0f) {
		lengthRate = 1.0f;
	}

	cocos2d::Vec2 gaugeOrigin;
	// オフセットを反映
	gaugeOrigin.x += _adjustPos.x * _scale.x;
	gaugeOrigin.y -= _adjustPos.y * _scale.y;

	// 描画ノードの位置を設定
	this->_drawNode->setPosition(gaugeOrigin);

	// ゲージの角度を設定		
	this->_drawNode->setRotation(_rotation);

	cocos2d::Vec2 origin = cocos2d::Vec2(-width * 0.5f, -height * 0.5f);
	cocos2d::Vec2 destination = cocos2d::Vec2(width * 0.5f, height * 0.5f);

	// 背景を描画
	origin.x -= 1;
	origin.y -= 1;
	destination.x += 1;
	destination.y += 1;
	this->_drawNode->drawSolidRect(origin, destination, _bgColor);

	// ゲージを描画
	origin = cocos2d::Vec2(-width * 0.5f, -height * 0.5f);
	destination = cocos2d::Vec2(width * 0.5f, height * 0.5f);
	destination.x = origin.x;
	destination.x += width * lengthRate;
	this->_drawNode->drawSolidRect(origin, destination, _color);
}

//-------------------------------------------------------------------------------------------------------------------
ActionCommandMessageTextUi::ActionCommandMessageTextUi()
{
	_duration300 = 0.0f;
	_actionId = -1;

	_lockObject = nullptr;
	_textGui = nullptr;
	_messageWindowNode = nullptr;
	_data = nullptr;
	_waitOneFrame = false;
}

ActionCommandMessageTextUi::~ActionCommandMessageTextUi()
{
	CC_SAFE_RELEASE_NULL(_textGui);
	CC_SAFE_RELEASE_NULL(_messageWindowNode);
	CC_SAFE_RELEASE_NULL(_data);

	_lockObject = nullptr;
}

bool ActionCommandMessageTextUi::init(agtk::Object* targetObject, agtk::Object* lockObject, agtk::data::ObjectCommandMessageShowData* showData)
{
	_targetObject = targetObject;
	_lockObject = lockObject;
	setData(showData);

	int fontId = Gui::NO_SETTING;
	cocos2d::Color3B color;
	color.r = _data->getColorR();
	color.g = _data->getColorG();
	color.b = _data->getColorB();
	char alpha = _data->getColorA();

	// プロジェクトデータを取得
	auto projectData = GameManager::getInstance()->getProjectData();
	CCASSERT(projectData, "");

	// メッセージウィンドウを生成
	auto messageWindow = agtk::MessageWindowNode::create(_data);
	if (messageWindow != nullptr) {
		this->addChild(messageWindow);
		this->setMessageWindowNode(messageWindow);
	}

	// テキスト素材を表示する場合
	if (_data->getTextFlag()) {
		// テキスト素材ID
		int textId = _data->getTextId();

		// テキスト素材IDが「設定無し」でない場合
		if (textId > Gui::NO_SETTING) {
			// テキストデータを取得
			auto textData = projectData->getTextData(textId);

			if (nullptr == textData) {
				CC_ASSERT(0);
				return false;
			}

			// フォントIDを取得
			fontId = textData->getFontId();
		}
	}
	// 変数を表示する場合
	else {
		fontId = _data->getFontId();
	}

	// テキスト表示用GUIを生成
	auto textGui = agtk::TextGui::create(fontId, color, alpha);
	if (textGui == nullptr) {
		return false;
	}
	this->setTextGui(textGui);

	createObject(true);

	if (_data->getPriority()) {
		switch(_data->getPriorityType()) {
		case agtk::data::ObjectCommandActionExecData::kPriorityBackground: {
			// 「背景」設定時
			auto scene = GameManager::getInstance()->getCurrentScene();
			auto sceneBackground = scene->getSceneBackground();
			sceneBackground->addChild(this);
			break; }
		case agtk::data::ObjectCommandActionExecData::kPriorityMostFront: {
			// 「最前面」設定時
			// 最前面レイヤーに接続する
			auto targetSceneLayerType = _targetObject->getSceneLayer()->getType();
			auto scene = GameManager::getInstance()->getCurrentScene();
			auto sceneLayerList = targetSceneLayerType == agtk::SceneLayer::kTypeMenu ? scene->getMenuLayerList() : scene->getSceneLayerList();
			cocos2d::DictElement *el = nullptr;
			CCDICT_FOREACH(sceneLayerList, el) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
				auto sceneLayer = static_cast<agtk::SceneLayer *>(el->getObject());
#else
				auto sceneLayer = dynamic_cast<agtk::SceneLayer *>(el->getObject());
#endif
				if (sceneLayer->getLayerId() == 1) {
					sceneLayer->addChild(this, 1001);
					break;
				}
			}
			break;}
		case agtk::data::ObjectCommandActionExecData::kPriorityMostFrontWithMenu: {
			// 「最前面＋メニューシーン」設定時
			auto scene = GameManager::getInstance()->getCurrentScene();
			auto menuSceneLayer = scene->getMenuLayer(agtk::data::SceneData::kHudTopMostLayerId);
			menuSceneLayer->addChild(this);
			break; }
		default: CC_ASSERT(0);
		}
	}
	else {
		_targetObject->addChild(this);
	}


	// メッセージ方向揃えを設定
	float horz = 0.0f;
	float vert = 0.0f;
	switch (_data->getHorzAlign()) {
		case agtk::data::ObjectCommandMessageShowData::kHorzAlignLeft: 
			horz = 0.5f;
			break;

		case agtk::data::ObjectCommandMessageShowData::kHorzAlignRight:
			horz = -0.5f;
			break;
	}
	switch (_data->getVertAlign()) {
		case agtk::data::ObjectCommandMessageShowData::kVertAlignTop:
			vert = -0.5f;
			break;

		case agtk::data::ObjectCommandMessageShowData::kVertAlignBottom:
			vert = 0.5f;
			break;
	}
	getTextGui()->setAlign(horz, vert);

	// テキストを表示する場合
	if (_data->getTextFlag()) {
		createMessageTextUi();
	}

	// 表示時間を設定
	if (!_data->getDurationUnlimited()) {
		_duration300 = _data->getDuration300();
	}

	// 「オブジェクトのアクションが切り替わったら表示を終了」が設定されている場合
	if (_data->getActionChangeHide()) {
		auto currentAction = _targetObject->getCurrentObjectAction();

		if (currentAction != nullptr) {
			_actionId = currentAction->getId();
		}
	}

	// 生成直後の更新でキー判定を行わないよう1フレーム待機フラグを設定する
	_waitOneFrame = _data->getCloseByKey();

	return true;
}

void ActionCommandMessageTextUi::update(float delta)
{
	// 追従対象が無くなった場合、処理できないので削除する
	if (_targetObject == nullptr) {
		_isDelete = true;
		return;
	}

	// カメラの表示領域・判定/回転状態によってクリッピングかレンダーを使い分けるのでここで処理する
	createObject(false);

	// 変数表示の場合
	if (!_data->getTextFlag()) {

		// 文字列
		string str = "";

		// 変数データを取得
		auto variableData = getPlayVariableData();

		if (variableData != nullptr) {
			// 変数を表示
			double value = variableData->getValue();
			if (std::isnan(value)) {
				//非数の場合は空文字列にする。
				str = "";
			}
			else {
				str = std::to_string(value);

				// 整数と小数を分離
				auto list = std::vector<std::string>();
				auto separator = std::string(".");
				auto offset = std::string::size_type(0);
				while (1) {
					auto pos = str.find(separator, offset);
					if (pos == std::string::npos) {
						list.push_back(str.substr(offset));
						break;
					}
					list.push_back(str.substr(offset, pos - offset));
					offset = pos + separator.length();
				}

				// 数値がマイナスの場合は
				bool isMinus = variableData->getValue() < 0;
				if (isMinus) {
					// 整数文字から-を消す
					list[0].erase(0, 1);
				}

				// 桁数を指定する場合
				if (_data->getDigitFlag()) {

					if (_data->getDigits() > 0) {
						// 表示桁数より文字数が多い場合
						if (_data->getDigits() < static_cast<int>(list[0].length())) {

							// 指定桁数まで文字を削除
							int d = list[0].length() - _data->getDigits();
							list[0].erase(0, d);
						}

						// 桁数分0をつける場合
						if (_data->getZeroPadding()) {

							int d = _data->getDigits() - list[0].length();

							if (d > 0) {
								list[0] = std::string(d, '0') + list[0];
							}
						}
					}
					// 桁数指定が0の場合
					else {
						// 文字を空にする
						list[0] = "";
						isMinus = false;
					}
				}

				// カンマ区切りを行う場合
				if (_data->getComma()) {
					int idx = list[0].size() - 3;

					while (idx > 0) {
						list[0].insert(idx, ",");
						idx -= 3;
					}
				}

				if (isMinus) {
					list[0] = "-" + list[0];
				}

				// 整数部分を表示文字列に設定
				str = list[0];

				// 小数表示を行う場合
				if (!_data->getWithoutDecimalPlaces()) {
					// 小数がある場合
					if (list.size() > 1) {

						// 小数点以下第二位までを表示
						if (list[1].size() > 2) {
							list[1].erase(2, list[1].size() - 2);
						}
						// 小数点第二位までない場合
						else if (list[1].size() < 2) {
							while (list[1].size() < 2) {
								list[1] += "0";
							}
						}

						// 整数部分の表示がある場合
						if (list[0].size() > 0) {
							str += ".";
						}
						str += list[1];
					}
					// 小数がない場合
					else {
						// 整数部分の表示がある場合
						if (list[0].size() > 0) {
							str += ".";
						}

						str += "00";
					}
				}
			}
		}

		// テキストに変更が発生した場合
		if (str.compare(getTextGui()->getString()) != 0 || getTextGui()->getIsUpdate()) {
			getTextGui()->setIsUpdate(false);

			// クリッピング側のテキストを更新
			if (isUseClipping()) {
				getTextGui()->updateText(
					str, 0, 0,
					_data->getWindowWidth(),
					_data->getWindowHeight(),
					_data->getLeftRightMargin(),
					_data->getTopBottomMargin());
			}
			// レンダー側のテキストを更新
			else {
				getTextGui()->updateTextRender(
					str, 0, 0,
					_data->getWindowWidth(),
					_data->getWindowHeight(),
					_data->getLeftRightMargin(),
					_data->getTopBottomMargin());
			}
		}
	}
	else
	{
		// クリッピングとレンダーの切り替え発生時に固定文字の場合は1回更新させる
		if (getTextGui()->getIsUpdate()) {
			getTextGui()->setIsUpdate(false);

			createMessageTextUi();
		}
	}

	// 座標を設定
	cocos2d::Vec2 pos = Vec2::ZERO;

	switch (_data->getPositionType()) {
		// このオブジェクトの中心
		case agtk::data::ObjectCommandMessageShowData::kPositionCenter: {

			// 接続点を使用する場合
			if (_data->getUseConnect() && _data->getConnectId() >= 0) {
				int connectId = _data->getConnectId();
				agtk::Vertex4 vertex4;
				if (_targetObject->getTimeline(agtk::data::TimelineInfoData::kTimelineConnection, connectId, vertex4)) {
					pos = this->getPositionTransform(vertex4.addr()[0]);
				}

				// 接続点のキーフレームがない場合は非表示化を行う
				this->setVisible(_targetObject->existsArea(connectId));
			}
			// 接続点を使用しない場合
			else {
				// オブジェクトの中心位置を取得
				pos = _targetObject->getCenterPosition();
			}

			pos = this->getPositionTransform(pos);
		} break;

		// このオブジェクトがロックしたオブジェクトの中心
		case agtk::data::ObjectCommandMessageShowData::kPositionLockObjectCenter: {

			// ロック対象オブジェクトが存在する場合
			if (_lockObject != nullptr) {
				// ロック対象オブジェクトの中心位置を取得
				pos = _lockObject->getCenterPosition();
				pos = this->getPositionTransform(pos);
			}
			// ロック対象オブジェクトが存在しない場合は破棄する
			else {
				_isDelete = true;
				return;
			}
		} break;

		// 画面を基準にする
		case agtk::data::ObjectCommandMessageShowData::kPositionBaseScreen: {

			// カメラの位置を取得
			auto scene = GameManager::getInstance()->getCurrentScene();
			auto camera = scene->getCamera();

			auto cameraPos = camera->getPosition();
			auto size = camera->getScreenSize();

			//浮動小数点切り捨て
			cameraPos.x = (int)cameraPos.x;
			cameraPos.y = (int)cameraPos.y;

			// 画面内左上を基準になるように変換
			cameraPos.x -= size.width * 0.5f;
			cameraPos.y += size.height * 0.5f;
			pos = cameraPos;
			//メニューシーンの場合は、左上の基準位置の設定。
			if (_targetObject && _targetObject->getSceneData()->isMenuScene()) {
				pos = cocos2d::Vec2(0, size.height);
			}
		} break;
	}

	// 位置を調整分を反映
	pos.x += _data->getAdjustX();
	pos.y -= _data->getAdjustY();

	// 座標を更新
	auto windowSize = getMessageWindowNode()->getContentSize();
	Vec2 align = Vec2::ZERO;;

	switch (_data->getHorzAlign()) {
	case agtk::data::ObjectCommandMessageShowData::kHorzAlignLeft:
		align.x = -windowSize.width * 0.5f;
		break;

	case agtk::data::ObjectCommandMessageShowData::kHorzAlignRight:
		align.x = windowSize.width * 0.5f;
		break;
	}

	switch (_data->getVertAlign()) {
	case agtk::data::ObjectCommandMessageShowData::kVertAlignTop:
		align.y = windowSize.height * 0.5f;
		break;

	case agtk::data::ObjectCommandMessageShowData::kVertAlignBottom:
		align.y = -windowSize.height * 0.5f;
		break;
	}

	//浮動小数点切り捨て
	pos.x = (int)pos.x;
	pos.y = (int)pos.y;

	getTextGui()->setPosition(pos + align);
	if (getMessageWindowNode() != nullptr) {
		getMessageWindowNode()->setPosition(pos);
	}

	// クリッピング処理
	{
		if (isUseClipping()) {
			auto scene = GameManager::getInstance()->getCurrentScene();
			auto camera = scene->getCamera();
			auto cameraPos = camera->getPosition();
			auto cameraSize = camera->getScreenSize();

			cocos2d::Vec2 addCameraPos = cocos2d::Vec2::ZERO;
			if (_targetObject->getSceneLayer()->getCameraMask() != (unsigned short)cocos2d::CameraFlag::USER1) {
				addCameraPos = cocos2d::Vec2(cameraPos.x - (cameraSize.width / 2), cameraPos.y - (cameraSize.height / 2));
			}

			Rect clippingRect = Rect(
				getTextGui()->getPosition().x - align.x - getTextGui()->getContentSize().width / 2 - addCameraPos.x,
				getTextGui()->getPosition().y - align.y - getTextGui()->getContentSize().height / 2 - addCameraPos.y,
				getTextGui()->getContentSize().width,
				getTextGui()->getContentSize().height);

			//実行アクション「シーンを揺らす」場合は、揺れの移動量を加算。
			auto shake = scene->getShake();
			if (shake->isShaking() && _targetObject->getSceneLayer()->getType() != agtk::SceneLayer::kTypeMenu) {
				auto moveXy = shake->getMoveXY();
				clippingRect.origin += moveXy;
				//揺れす時にY軸で１ドットズレるのため補正。
				if (_targetObject->getSceneLayer()->getCameraMask() != (unsigned short)cocos2d::CameraFlag::USER1) {
					clippingRect.origin.y += 1;
				}
			}

			_clipping->setClippingRegion(clippingRect);
		}
	}

	// 表示時間が制限ありの場合
	if (!_data->getDurationUnlimited()) {
		// 残り時間を更新し0になれば削除する
		_duration300 -= delta * 300.0f;
		if (_duration300 <= 0.0f) {
			_isDelete = true;
			return;
		}
	}

	// 「オブジェクトのアクションが切り替わったら表示を終了」が設定されている場合
	if (_data->getActionChangeHide()) {
		auto currentAction = _targetObject->getCurrentObjectAction();
		if (currentAction != nullptr) {
			if (_actionId != currentAction->getId()) {
				_isDelete = true;
				//１フレーム表示が残るためここで非表示にする。
				this->setVisible(false);
				return;
			}
		}
	}

	// 「指定された入力で表示を終了」が設定されていて1フレーム待機フラグがOFFの場合
	if (_data->getCloseByKey() && !_waitOneFrame) {
		// 指定されたキーが入力された場合
		if (InputManager::getInstance()->isTriggered(_data->getKeyId())) {
			_isDelete = true;
			return;
		}
	}

	if (_waitOneFrame) {
		_waitOneFrame = false;
	}
}

/**
* オブジェクト作成
* @param	isInit	初期化判定
*/
void ActionCommandMessageTextUi::createObject(bool isInit)
{
	// 切り替わりが発生するなら削除してから作成
	bool nowState = isUseClipping();
	if (nowState != _isClipping || isInit) {
		_isClipping = nowState;
		getTextGui()->setIsUpdate(true);

		// 情報を削除する
		getTextGui()->removeFromParentAndCleanup(true);
		auto renderTex = getTextGui()->getRenderTex();
		if (renderTex) {
			renderTex->removeFromParentAndCleanup(true);
			getTextGui()->setRenderTex(nullptr);
		}
		auto baseNode = getTextGui()->getBaseNode();
		if (baseNode) {
			baseNode->removeFromParentAndCleanup(true);
			getTextGui()->setBaseNode(nullptr);
		}
		if (_clipping) {
			_clipping->removeFromParentAndCleanup(true);
			removeChild(_clipping);
			_clipping = nullptr;
		}

		// クリッピング側を作成
		if (_isClipping) {
			_clipping = ClippingRectangleNode::create();
			_clipping->addChild(getTextGui());
			this->addChild(_clipping);
		}
		// レンダー側を作成
		else {
			this->addChild(getTextGui());
		}
	}
}

void ActionCommandMessageTextUi::createMessageTextUi()
{
	// プロジェクトデータを取得
	auto projectData = GameManager::getInstance()->getProjectData();

	// テキスト素材ID
	int textId = _data->getTextId();

	// テキストデータを取得
	auto textData = projectData->getTextData(textId);

	if (textData) {
		// クリッピング側のテキストを更新
		if (isUseClipping()) {
// #AGTK-NX
#if (CC_TARGET_PLATFORM == CC_PLATFORM_NX)
#else
			getTextGui()->updateText(
				projectData->getExpandedText(nullptr, textData->getText(nullptr), std::list<int>(1, textId)).c_str(),
#endif
				textData->getLetterSpacing(),
				textData->getLineSpacing(),
				_data->getWindowWidth(),
				_data->getWindowHeight(),
				_data->getLeftRightMargin(),
				_data->getTopBottomMargin());
		}
		// レンダー側のテキストを更新
		else {
// #AGTK-NX
#if (CC_TARGET_PLATFORM == CC_PLATFORM_NX)
#else
			getTextGui()->updateTextRender(
				projectData->getExpandedText(nullptr, textData->getText(nullptr), std::list<int>(1, textId)).c_str(),
#endif
				textData->getLetterSpacing(),
				textData->getLineSpacing(),
				_data->getWindowWidth(),
				_data->getWindowHeight(),
				_data->getLeftRightMargin(),
				_data->getTopBottomMargin());
		}
	}
}

agtk::data::PlayVariableData* ActionCommandMessageTextUi::getPlayVariableData()
{
	agtk::data::PlayVariableData * variableData = nullptr;

	auto projectPlayData = GameManager::getInstance()->getPlayData();

	// 「自身のオブジェクト」変数使用時
	if (_data->getVariableObjectId() == agtk::data::ObjectCommandMessageShowData::kSelfObject) {
		variableData = _targetObject->getPlayObjectData()->getVariableData(_data->getVariableId());
//		CCLOG("自身のオブジェクト");
	}
	// 「プロジェクト共通」変数使用時
	else if (_data->getVariableObjectId() == agtk::data::PlayData::COMMON_PLAY_DATA_ID) {
		variableData = projectPlayData->getVariableData(agtk::data::PlayData::COMMON_PLAY_DATA_ID, _data->getVariableId());
//		CCLOG("プロジェクト共通");
	}
	// 「ロックしたオブジェクト」変数使用時
	else if (_data->getVariableObjectId() == agtk::data::ObjectCommandMessageShowData::kLockedObject) {

		auto scene = GameManager::getInstance()->getCurrentScene();
		CC_ASSERT(scene);
		auto sceneLayer = _targetObject->getSceneLayer();
// #AGTK-NX
#ifdef USE_SAR_OPTIMIZE_1
		auto objectAll = scene->getObjectAllReference(sceneLayer->getType());
#else
		auto objectAll = scene->getObjectAll(sceneLayer->getType());
#endif
		CC_ASSERT(objectAll);

		cocos2d::Ref *ref = nullptr;
		CCARRAY_FOREACH(objectAll, ref) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
			auto object = static_cast<agtk::Object*>(ref);
#else
			auto object = dynamic_cast<agtk::Object*>(ref);
#endif
			auto playObjectData = object->getPlayObjectData();

			if (playObjectData->isLocked(_targetObject->getInstanceId())) {
				variableData = object->getPlayObjectData()->getVariableData(_data->getVariableId());
				break;
			}
		}

//		CCLOG("ロックしたオブジェクト");
	}
	// 共通オブジェクト選択時
	else if (_data->getVariableObjectId() > 0) {
		// 変数データリストを取得
		auto gameManager = GameManager::getInstance();
		auto variableDataList = cocos2d::Array::create();
		gameManager->getSwitchVariableDataList(_data->getVariableQualifierId(), _data->getVariableObjectId(), _data->getVariableId(), false, variableDataList);

		// リストにデータが設定されている場合、リストの先頭を使用する
		if (variableDataList->count() > 0) {
			variableData = dynamic_cast<agtk::data::PlayVariableData*>(variableDataList->getObjectAtIndex(0));
		}
	}

	return variableData;
}

void ActionCommandMessageTextUi::remove()
{
	this->removeAllChildrenWithCleanup(true);
	this->removeFromParentAndCleanup(true);
}

bool ActionCommandMessageTextUi::checkRemove(agtk::Object* object) {
	bool isRemove = Gui::checkRemove(object);

	if (!isRemove && _lockObject != nullptr) {
		if (_lockObject->getId() == object->getId()) {
			isRemove = true;
		}
	}
 
	return isRemove;
}

bool ActionCommandMessageTextUi::getObjectStop()
{
	// ゲーム停止が発生している場合はオブジェクトも停止する
	if (getGameStop()) {
		return true;
	}

	return _data->getObjectStop();
}

bool ActionCommandMessageTextUi::getGameStop()
{
	return _data->getGameStop();
}

//-------------------------------------------------------------------------------------------------------------------
ActionCommandScrollMessageTextUi::ActionCommandScrollMessageTextUi()
{
	_actionId = -1;
	_scrollY = 0.0f;
	_scrollYMax = 0.0f;

	_textGui = nullptr;
	_messageWindowNode = nullptr;
	_data = nullptr;
	_lockObject = nullptr;

	_isAllowScrollSpeedUp = true;
}

ActionCommandScrollMessageTextUi::~ActionCommandScrollMessageTextUi()
{
	CC_SAFE_RELEASE_NULL(_textGui);
	CC_SAFE_RELEASE_NULL(_messageWindowNode);
	CC_SAFE_RELEASE_NULL(_data);
	_lockObject = nullptr;
}

bool ActionCommandScrollMessageTextUi::init(agtk::Object* targetObject, agtk::Object* lockObject, agtk::data::ObjectCommandScrollMessageShowData* showData)
{
	_targetObject = targetObject;
	_lockObject = lockObject;
	setData(showData);

	cocos2d::Color3B color;
	color.r = _data->getColorR();
	color.g = _data->getColorG();
	color.b = _data->getColorB();
	char alpha = _data->getColorA();

	// プロジェクトデータを取得
	auto projectData = GameManager::getInstance()->getProjectData();
	CCASSERT(projectData, "");

	// メッセージウィンドウを生成
	auto windowSize = cocos2d::Size(0, 0);
	auto messageWindow = agtk::MessageWindowNode::create(_data);
	if (messageWindow != nullptr) {
		this->addChild(messageWindow);
		this->setMessageWindowNode(messageWindow);
		windowSize = messageWindow->getContentSize();
	}

	// テキスト素材ID
	agtk::data::TextData* textData = nullptr;
	int textId = _data->getTextId();

	// テキスト素材IDが「設定無し」でない場合
	if (textId > Gui::NO_SETTING) {
		// テキストデータを取得
		textData = projectData->getTextData(textId);
	}
	else {
		// デフォルト値のなにもない TextData を生成
		textData = agtk::data::TextData::create();
	}

	// フォントIDを取得
	int fontId = textData->getFontId();

	// テキスト表示用GUIを生成
	auto textGui = agtk::TextGui::create(fontId, color, alpha);
	if (textGui == nullptr) {
		return false;
	}
	this->setTextGui(textGui);

	createObject(true);

	if (_data->getPriority()) {
		switch (_data->getPriorityType()) {
		case agtk::data::ObjectCommandActionExecData::kPriorityBackground: {
			// 「背景」設定時
			auto scene = GameManager::getInstance()->getCurrentScene();
			auto sceneBackground = scene->getSceneBackground();
			sceneBackground->addChild(this);
			break; }
		case agtk::data::ObjectCommandActionExecData::kPriorityMostFront: {
			// 「最前面」設定時
			// 最前面レイヤーに接続する
			auto scene = GameManager::getInstance()->getCurrentScene();
			auto sceneLayerList = scene->getSceneLayerList();
			cocos2d::DictElement *el = nullptr;
			CCDICT_FOREACH(sceneLayerList, el) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
				auto sceneLayer = static_cast<agtk::SceneLayer *>(el->getObject());
#else
				auto sceneLayer = dynamic_cast<agtk::SceneLayer *>(el->getObject());
#endif
				if (sceneLayer->getLayerId() == 1) {
					sceneLayer->addChild(this, 1001);
					break;
				}
			}
			break; }
		case agtk::data::ObjectCommandActionExecData::kPriorityMostFrontWithMenu: {
			// 「最前面＋メニューシーン」設定時
			auto scene = GameManager::getInstance()->getCurrentScene();
			auto menuSceneLayer = scene->getMenuLayer(agtk::data::SceneData::kHudTopMostLayerId);
			menuSceneLayer->addChild(this);
			break; }
		default: CC_ASSERT(0);
		}
	}
	else {
		_targetObject->addChild(this);
	}

	// メッセージ方向揃えを設定
	float horz = 0.0f;
	switch (_data->getHorzAlign()) {
	case agtk::data::ObjectCommandScrollMessageShowData::kHorzAlignLeft:
		horz = 0.5f;
		break;

	case agtk::data::ObjectCommandScrollMessageShowData::kHorzAlignRight:
		horz = -0.5f;
		break;
	}

	getTextGui()->setAlign(horz, 0.0f);

// #AGTK-NX	
#if CC_TARGET_PLATFORM != CC_PLATFORM_NX
	// クリッピング側のテキストを更新
	if (isUseClipping()) {
		getTextGui()->updateText(
			projectData->getExpandedText(nullptr, textData->getText(nullptr), std::list<int>(1, textId)),
			textData->getLetterSpacing(),
			textData->getLineSpacing(),
			windowSize.width,
			windowSize.height,
			_data->getLeftRightMargin(),
			_data->getTopBottomMargin());
	}
	// レンダー側のテキストを更新
	else {
		getTextGui()->updateTextRender(
			projectData->getExpandedText(nullptr, textData->getText(nullptr), std::list<int>(1, textId)),
			textData->getLetterSpacing(),
			textData->getLineSpacing(),
			windowSize.width,
			windowSize.height,
			_data->getLeftRightMargin(),
			_data->getTopBottomMargin());
	}
#else
#endif

	//! スクロール値を初期化
	auto texHeight = getTextGui()->getHeight();
	auto textRealHeight = getTextGui()->getRealHeight();

	// 下から上へ移動する場合
	if (_data->getScrollUp()) {
		// 初期スクロール開始位置とスクロール終了位置を設定
		_scrollY = -(windowSize.height - _data->getTopBottomMargin());
		_scrollYMax = textRealHeight + _data->getTopBottomMargin();
	}
	// 上から下へ移動する場合
	else {
		_scrollY = textRealHeight + _data->getTopBottomMargin();
		_scrollYMax = -(windowSize.height - _data->getTopBottomMargin());
	}

	// スクロールの設定をテキストに反映する
#if CC_TARGET_PLATFORM != CC_PLATFORM_NX
	// クリッピング側のテキストを更新
	if (isUseClipping()) {
		getTextGui()->updateText(
			projectData->getExpandedText(nullptr, textData->getText(nullptr), std::list<int>(1, textId)),
			textData->getLetterSpacing(),
			textData->getLineSpacing(),
			windowSize.width,
			windowSize.height,
			_data->getLeftRightMargin(),
			_data->getTopBottomMargin(),
			_scrollY);
	}
	// レンダー側のテキストを更新
	else {
		getTextGui()->updateTextRender(
			projectData->getExpandedText(nullptr, textData->getText(nullptr), std::list<int>(1, textId)),
			textData->getLetterSpacing(),
			textData->getLineSpacing(),
			windowSize.width,
			windowSize.height,
			_data->getLeftRightMargin(),
			_data->getTopBottomMargin(),
			_scrollY);
	}
#else
#endif

	// 「オブジェクトのアクションが切り替わったら表示を終了」が設定されている場合
	if (_data->getActionChangeHide()) {
		auto currentAction = _targetObject->getCurrentObjectAction();

		if (currentAction != nullptr) {
			_actionId = currentAction->getId();
		}
	}

	// キー入力で速度アップ可能な場合
	if (_data->getSpeedUpByKey()) {
		int keyId = _data->getKeyId();
		if (keyId > Gui::NO_SETTING) {
			// 既に速度アップ用のキーが押されている場合は押しっぱなしによるスクロールを許可しない
			_isAllowScrollSpeedUp = !InputManager::getInstance()->isPressed(keyId);
		}
	}

	return true;
}

void ActionCommandScrollMessageTextUi::update(float delta)
{
	// 追従対象が無くなった場合、処理できないので削除する
	if (_targetObject == nullptr) {
		_isDelete = true;
		return;
	}

	// カメラの表示領域・判定/回転状態によってクリッピングかレンダーを使い分けるのでここで処理する
	createObject(false);

	// 座標を設定
	cocos2d::Vec2 pos = Vec2::ZERO;

	switch (_data->getPositionType()) {
		// このオブジェクトの中心
		case agtk::data::ObjectCommandScrollMessageShowData::kPositionCenter: {

			// 接続点を使用する場合
			if (_data->getUseConnect() && _data->getConnectId() >= 0) {
				int connectId = _data->getConnectId();
				agtk::Vertex4 vertex4;
				if (_targetObject->getTimeline(agtk::data::TimelineInfoData::kTimelineConnection, connectId, vertex4)) {
					pos = this->getPositionTransform(vertex4.addr()[0]);
				}

				// 接続点のキーフレームがない場合は非表示化を行う
				this->setVisible(_targetObject->existsArea(connectId));
			}
			// 接続点を使用しない場合
			else {
				// オブジェクトの中心位置を取得
				pos = _targetObject->getCenterPosition();
			}

			pos = this->getPositionTransform(pos);
		} break;

			// このオブジェクトがロックしたオブジェクトの中心
		case agtk::data::ObjectCommandScrollMessageShowData::kPositionLockObjectCenter: {

			// ロック対象オブジェクトが存在する場合
			if (_lockObject != nullptr) {
				// ロック対象オブジェクトの中心位置を取得
				pos = _lockObject->getCenterPosition();
				pos = this->getPositionTransform(pos);
			}
			// ロック対象オブジェクトが存在しない場合は破棄する
			else {
				_isDelete = true;
				return;
			}
		} break;

			// 画面を基準にする
		case agtk::data::ObjectCommandScrollMessageShowData::kPositionBaseScreen: {

			// カメラの位置を取得
			auto scene = GameManager::getInstance()->getCurrentScene();
			auto camera = scene->getCamera();

			auto cameraPos = camera->getPosition();
			auto size = camera->getScreenSize();

			// 画面内左上を基準になるように変換
			cameraPos.x -= size.width * 0.5f;
			cameraPos.y += size.height * 0.5f;
			pos = cameraPos;
		} break;
	}

	// 位置を調整分を反映
	pos.x += _data->getAdjustX();
	pos.y -= _data->getAdjustY();

	// 指定入力でスクロールを速くする処理
	float speed = 1.0f;
	if (_data->getSpeedUpByKey()) {
		int keyId = _data->getKeyId();

		// 入力キーが設定されている
		if (keyId > Gui::NO_SETTING) {
			// スクロール加速許可がON
			// かつ
			// 設定されている入力キーを入力している場合
			if (_isAllowScrollSpeedUp && InputManager::getInstance()->isPressed(keyId)) {
				// スクロール速度を TEXT_SCROLL_SPEED_UP_RATE 倍にする
				speed *= TEXT_SCROLL_SPEED_UP_RATE;
			}
			// スクロール加速許可がOFF
			// かつ
			// 設定されている入力キーが離されている場合
			else if (!_isAllowScrollSpeedUp && InputManager::getInstance()->isReleased(keyId)) {
				// スクロール加速許可をON
				_isAllowScrollSpeedUp = true;
			}
		}
	}

	// 下から上へ移動する場合
	if (_data->getScrollUp()) {
		_scrollY += _data->getScrollSpeed() * speed;
	}
	// 上から下へ移動する場合
	else {
		_scrollY -= _data->getScrollSpeed() * speed;
	}
	
	// スクロール処理
	auto windowSize = getMessageWindowNode()->getContentSize();

	// クリッピング側のテキストを更新
	if (isUseClipping()) {
		getTextGui()->updateText(
			getTextGui()->getString(),
			getTextGui()->getLetterSpace(),
			getTextGui()->getLineSpace(),
			windowSize.width,
			windowSize.height,
			_data->getLeftRightMargin(),
			_data->getTopBottomMargin(),
			_scrollY);
	}
	// レンダー側のテキストを更新
	else {
		getTextGui()->updateTextRender(
			getTextGui()->getString(),
			getTextGui()->getLetterSpace(),
			getTextGui()->getLineSpace(),
			windowSize.width,
			windowSize.height,
			_data->getLeftRightMargin(),
			_data->getTopBottomMargin(),
			_scrollY);
	}

	// 座標を更新
	float align = 0;

	switch (_data->getHorzAlign()) {
	case agtk::data::ObjectCommandScrollMessageShowData::kHorzAlignLeft:
		align = -windowSize.width * 0.5f;
		break;

	case agtk::data::ObjectCommandScrollMessageShowData::kHorzAlignRight:
		align = windowSize.width * 0.5f;
		break;
	}	
	getTextGui()->setPosition(Vec2(pos.x + align, pos.y));
	getMessageWindowNode()->setPosition(pos);

	// クリッピング処理
	{
		if (isUseClipping()) {
			auto scene = GameManager::getInstance()->getCurrentScene();
			auto camera = scene->getCamera();
			auto cameraPos = camera->getPosition();
			auto cameraSize = camera->getScreenSize();

			cocos2d::Vec2 addCameraPos = cocos2d::Vec2::ZERO;
			if (_targetObject->getSceneLayer()->getCameraMask() != (unsigned short)cocos2d::CameraFlag::USER1) {
				addCameraPos = cocos2d::Vec2(cameraPos.x - (cameraSize.width / 2), cameraPos.y - (cameraSize.height / 2));
			}

			Rect clippingRect = Rect(
				getTextGui()->getPosition().x - align - getTextGui()->getContentSize().width / 2 - addCameraPos.x,
				getTextGui()->getPosition().y - getTextGui()->getContentSize().height / 2 - addCameraPos.y,
				getTextGui()->getContentSize().width,
				getTextGui()->getContentSize().height);

			//実行アクション「シーンを揺らす」場合は、揺れの移動量を加算。
			auto shake = scene->getShake();
			if (shake->isShaking() && _targetObject->getSceneLayer()->getType() != agtk::SceneLayer::kTypeMenu) {
				auto moveXy = shake->getMoveXY();
				clippingRect.origin += moveXy;
				//揺れす時にY軸で１ドットズレるのため補正。
				if (_targetObject->getSceneLayer()->getCameraMask() != (unsigned short)cocos2d::CameraFlag::USER1) {
					clippingRect.origin.y += 1;
				}
			}

			_clipping->setClippingRegion(clippingRect);
		}
	}

	// スクロール後の位置がウィンドウ外に出たかをチェックし、
	// ウィンドウ外へ出たならUIを削除する
	if (_data->getScrollUp()) {
		if (_scrollY >= _scrollYMax) {
			_isDelete = true;
			return;
		}
	}
	else {
		if (_scrollY <= _scrollYMax) {
			_isDelete = true;
			return;
		}
	}

	// 「オブジェクトのアクションが切り替わったら表示を終了」が設定されている場合
	if (_data->getActionChangeHide()) {
		auto currentAction = _targetObject->getCurrentObjectAction();
		if (currentAction != nullptr) {
			if (_actionId != currentAction->getId()) {
				_isDelete = true;
				//１フレーム表示が残るためここで非表示にする。
				this->setVisible(false);
				return;
			}
		}
	}
}

void ActionCommandScrollMessageTextUi::remove()
{
	this->removeAllChildrenWithCleanup(true);
	this->removeFromParentAndCleanup(true);
}

bool ActionCommandScrollMessageTextUi::checkRemove(agtk::Object* object)
{
	bool isRemove = Gui::checkRemove(object);

	if (!isRemove && _lockObject != nullptr) {
		if (_lockObject->getId() == object->getId()) {
			isRemove = true;
		}
	}

	return isRemove;
}

bool ActionCommandScrollMessageTextUi::getObjectStop()
{
	// ゲーム停止が発生している場合はオブジェクトも停止する
	if (getGameStop()) {
		return true;
	}

	return _data->getObjectStop();
}

bool ActionCommandScrollMessageTextUi::getGameStop()
{
	return _data->getGameStop();
}

/**
* オブジェクト作成
* @param	isInit	初期化判定
*/
void ActionCommandScrollMessageTextUi::createObject(bool isInit)
{
	// 切り替わりが発生するなら削除してから作成
	bool nowState = isUseClipping();
	if (nowState != _isClipping || isInit) {
		_isClipping = nowState;
		getTextGui()->setIsUpdate(true);

		// 情報を削除する
		getTextGui()->removeFromParentAndCleanup(true);
		auto renderTex = getTextGui()->getRenderTex();
		if (renderTex) {
			renderTex->removeFromParentAndCleanup(true);
			getTextGui()->setRenderTex(nullptr);
		}
		auto baseNode = getTextGui()->getBaseNode();
		if (baseNode) {
			baseNode->removeFromParentAndCleanup(true);
			getTextGui()->setBaseNode(nullptr);
		}
		if (_clipping) {
			_clipping->removeFromParentAndCleanup(true);
			removeChild(_clipping);
			_clipping = nullptr;
		}

		// クリッピング側を作成
		if (_isClipping) {
			_clipping = ClippingRectangleNode::create();
			_clipping->addChild(getTextGui());
			this->addChild(_clipping);
		}
		// レンダー側を作成
		else {
			this->addChild(getTextGui());
		}
	}
}

//-------------------------------------------------------------------------------------------------------------------
MessageWindowNode::MessageWindowNode()
{
}

MessageWindowNode::~MessageWindowNode()
{
}

bool MessageWindowNode::init(agtk::data::ObjectCommandMessageShowData* data)
{
	// サイズ
	cocos2d::Size size = cocos2d::Size::ZERO;

	switch (data->getWindowType()) {
		// 「無し」
		case agtk::data::ObjectCommandMessageShowData::kWindowNone:

			// 「無し」作成
			size = createNone(false, data->getWindowWidth(), data->getWindowHeight());			
			
			break;

		// 「テンプレートから選択」
		case agtk::data::ObjectCommandMessageShowData::kWindowTemplate:

			// テンプレートウィンドウ作成
			size = createTemplate(
				false, 
				data->getWindowWidth(), 
				data->getWindowHeight(), 
				data->getWindowTransparency(),
				(data->getTemplateId() == agtk::data::ObjectCommandMessageShowData::kWindowTemplateWhiteFrame),
				(data->getTemplateId() == agtk::data::ObjectCommandMessageShowData::kWindowTemplateBlack)
			);

			break;

		// 「画像素材から選択」
		case agtk::data::ObjectCommandMessageShowData::kWindowImage:
 
			// 画像ウィンドウ作成
			size = createImage(
				false, 
				data->getWindowWidth(),
				data->getWindowHeight(), 
				data->getImageId(), 
				data->getWindowTransparency()
			);
			break;
	}

	this->setContentSize(size);

	return true;
}

bool MessageWindowNode::init(agtk::data::ObjectCommandScrollMessageShowData* data)
{
	auto projectData = GameManager::getInstance()->getProjectData();
	CCASSERT(projectData, "");

	// サイズ
	cocos2d::Size size = cocos2d::Size::ZERO;

	switch (data->getBackgroundType()) {
		// 「無し」
		case agtk::data::ObjectCommandScrollMessageShowData::kBackgroundNone:

			// 「無し」作成
			size = createNone(false, data->getBackgroundWidth(), data->getBackgroundHeight());

			break;

		// 「テンプレートから選択」
		case agtk::data::ObjectCommandScrollMessageShowData::kBackgroundTemplate:

			// テンプレートウィンドウを作成
			size = createTemplate(
				false,
				data->getBackgroundWidth(),
				data->getBackgroundHeight(),
				data->getBackgroundTransparency(),
				(data->getTemplateId() == agtk::data::ObjectCommandScrollMessageShowData::kBackgroundTemplateWhiteFrame),
				(data->getTemplateId() == agtk::data::ObjectCommandScrollMessageShowData::kBackgroundTemplateBlack)
			);

			break;

		// 「画像素材から選択」
		case agtk::data::ObjectCommandScrollMessageShowData::kBackgroundImage:

			// 画像が「設定なし」の場合
			int imageId = data->getImageId();
			if (imageId < 0) {
				size = createNone(false, data->getBackgroundWidth(), data->getBackgroundHeight());
			}
			// 画像が設定されている場合
			else {
				// 画像ウィンドウ作成
				size = createImage(
					false,
					data->getBackgroundWidth(),
					data->getBackgroundHeight(),
					imageId,
					data->getBackgroundTransparency()
				);
			}

			break;
	}

	this->setContentSize(size);

	return true;
}

cocos2d::Size MessageWindowNode::createNone(bool useDefault, int width, int height)
{
	auto size = cocos2d::Size::ZERO;

	// デフォルトサイズがが設定されている場合
	if (useDefault) {
		auto projectData = GameManager::getInstance()->getProjectData();
		CCASSERT(projectData, "");

		// 画面解像度の1/4を設定
		size = projectData->getScreenSize();
		size.width *= 0.25f;
		size.height *= 0.25f;
	}
	else {
		size.width = width;
		size.height = height;
	}

	return size;
}

cocos2d::Size MessageWindowNode::createTemplate(bool useDefault, int width, int height, int transparency, bool drawFrame, bool colorBlack)
{
	auto size = cocos2d::Size::ZERO;

	// デフォルトサイズがが設定されている場合
	if (useDefault) {
		auto projectData = GameManager::getInstance()->getProjectData();
		CCASSERT(projectData, "");

		// 画面解像度の1/4を設定
		size = projectData->getScreenSize();
		size.width *= 0.25f;
		size.height *= 0.25f;
	}
	else {
		size.width = width;
		size.height = height;
	}

	// ウィンドウの表示位置を設定
	cocos2d::Vec2 origine(size.width * -0.5f, size.height * 0.5f);
	cocos2d::Vec2 dest(origine.x + size.width, origine.y - size.height);

	// カラー
	cocos2d::Color4F color(1.0f, 1.0f, 1.0f, 1.0f);

	// 描画用ノードを生成
	auto drawNode = cocos2d::DrawNode::create();

	// α値を設定
	color.a = ((float)(100 - transparency) * 0.01f);

	// ウィンドウの色が黒の場合
	if (colorBlack || drawFrame) {
		color.r = 0.0f;
		color.g = 0.0f;
		color.b = 0.0f;
	}

	// ウインドウのベース部分（矩形）
	drawNode->drawSolidRect(origine, dest, color);
	this->addChild(drawNode);

	// ウィンドウが枠の場合
	if (drawFrame) {
		auto frameDrawNode = cocos2d::DrawNode::create();
		color.r = 1.0f;
		color.g = 1.0f;
		color.b = 1.0f;
		frameDrawNode->drawRect(origine, dest, color);
		this->addChild(frameDrawNode);
	}

	return size;
}

cocos2d::Size MessageWindowNode::createImage(bool useDefault, int width, int height, int imageId, int transparency)
{
	// 画像IDが設定されていない場合は処理しない
	if (imageId < 0) {
		return cocos2d::Size(width, height);
	}

	auto projectData = GameManager::getInstance()->getProjectData();
	CCASSERT(projectData, "");

	// 画像IDから画像データを取得
	auto imgFileName = projectData->getImageData(imageId)->getFilename();

	// スプライトを生成
	auto sprite = Sprite::create(imgFileName);
	sprite->getTexture()->setAliasTexParameters();

	// アンカーポイントを設定
	sprite->setAnchorPoint(Vec2::ANCHOR_MIDDLE);

	// α値を設定
	sprite->setOpacity(255 * ((float)(100 - transparency) * 0.01f));

	auto size = cocos2d::Size::ZERO;

	// サイズを調整する場合
	if (!useDefault) {
		size.width = width;
		size.height = height;

		sprite->setContentSize(size);
	}
	else {
		size = sprite->getContentSize();
	}

	this->addChild(sprite);

	return size;
}

//-------------------------------------------------------------------------------------------------------------------
TextLineNode::TextLineNode()
{
	_nodeList = nullptr;
}

TextLineNode::~TextLineNode()
{
	CC_SAFE_RELEASE_NULL(_nodeList);
}

static int utf32IndexOf(const std::u32string &u32Str, char key, int head)
{
	if (head >= static_cast<int>(u32Str.length())) {
		return -1;
	}
	while (head < static_cast<int>(u32Str.length())) {
		auto s = u32Str.substr(head, 1);
		std::string u8Str;
		if (!StringUtils::UTF32ToUTF8(s, u8Str)) {
			return -1;
		}
		if (u8Str[0] == key) {
			return head;
		}
		head++;
	}
	return -1;
}

static int getInt(const std::string &numStr, int defValue)
{
	int num;
	if (agtk::data::ProjectData::isNumber(numStr.c_str(), &num)) {
		return num;
	}
	return defValue;
}

static int getHexInt(const std::string &numStr, int defValue)
{
	int num;
	if (agtk::data::ProjectData::isHexNumber(numStr.c_str(), &num)) {
		return num;
	}
	return defValue;
}

static void parseTag(const std::u32string &u32Str, int head, int size, int iniSize, const cocos2d::Color3B &iniColor, int *pNewHead, char *pTagName, int *pNewSize, cocos2d::Color3B *pNewColor)
{
	*pNewHead = head;
	*pTagName = 0;
	*pNewSize = size;
	*pNewColor = cocos2d::Color3B(255, 255, 255);
	auto s32 = u32Str.substr(head, 3);
	std::string tag;
	if (!StringUtils::UTF32ToUTF8(s32, tag)) {
		return;
	}
	if(tag == "\\S["){
		auto index = utf32IndexOf(u32Str, ']', head + 3);
		if (index >= 0) {
			auto word32 = u32Str.substr(head + 3, index - (head + 3));
			std::string word;
			if (!StringUtils::UTF32ToUTF8(word32, word)) {
				return;
			}
			if (word.size() == 0) {
				size = iniSize;
			}
			else if (word[0] == '+') {
				size = std::max(0, size + getInt(word.substr(1), 0));
			}
			else if (word[0] == '-') {
				size = std::max(0, size - getInt(word.substr(1), 0));
			}
			else {
				size = std::max(0, getInt(word, iniSize));
			}
			*pNewHead = index + 1;
			*pTagName = 'S';
			*pNewSize = size;
			return;
		}
	}
	else if (tag == "\\C[") {
		auto index = utf32IndexOf(u32Str, ']', head + 3);
		if (index >= 0) {
			auto word32 = u32Str.substr(head + 3, index - (head + 3));
			std::string word;
			if (!StringUtils::UTF32ToUTF8(word32, word)) {
				return;
			}
			cocos2d::Color3B rgb(255, 255, 255);
			if (word.length() == 0) {
				rgb = iniColor;
			}
			else if (word[0] == '#') {
				if (word.length() == 3 + 1) {
					auto v = getHexInt(word.substr(1), 0xfff);
					rgb = cocos2d::Color3B(
						(((v >> 8) & 0x0f) * 0xff / 0x0f),
						(((v >> 4) & 0x0f) * 0xff / 0x0f),
						(((v >> 0) & 0x0f) * 0xff / 0x0f)
					);
				}
				else if (word.length() == 6 + 1) {
					auto v = getHexInt(word.substr(1), 0xffffff);
					rgb = cocos2d::Color3B(
						((v >> 16) & 0xff),
						((v >> 8) & 0xff),
						((v >> 0) & 0xff));
				}
				else {
					rgb = cocos2d::Color3B(255, 255, 255);
				}
			}
			else {
				auto list = agtk::data::ProjectData::stringSplit(word, ',');
				if (list.size() < 3) {
				}
				else {
					rgb = cocos2d::Color3B(
						std::max(0, std::min(255, getInt(list[0], 255))),
						std::max(0, std::min(255, getInt(list[1], 255))),
						std::max(0, std::min(255, getInt(list[2], 255)))
					);
				}
			}
			*pNewHead = index + 1;
			*pTagName = 'C';
			*pNewColor = rgb;
			return;
		}
	}
	//console.log("null: " + JSON.stringify({head: head, tagName: null}));
	return;

}

bool TextLineNode::init(std::string str, agtk::data::FontData* fontData, int letterSpace, int iniSize, const cocos2d::Color3B &iniColor, int *pSize, cocos2d::Color3B *pColor)
{
	FontManager *fm = FontManager::getInstance();

	float x = 0;
	float height = 0;
	std::u32string u32Str;
	std::string u8Str;

	float firstWidth = 0;

	removeAllChildren();// 一旦全文字描画から除外

	// 文字列が空の場合
	if (str.size() <= 0) {

		// フォントの種類が画像フォントの場合
		if (fontData->getMainFontSetting()->getImageFontFlag()) {
			
			height = fm->getImageFontHeight(fontData);
		}
	}
	else {
		// cocos2dのラベルがutf32で文字を保持しているので変換する
		if (StringUtils::UTF8ToUTF32(str, u32Str)) {
			if (!getNodeList()) {
				setNodeList(cocos2d::Array::create());
			}

			auto nodeList = getNodeList();

			// 各文字ごとに処理を行う
			for (unsigned int j = 0; j < u32Str.length(); j++) {

				// 一文字ずつ抽出する
				auto s = u32Str.substr(j, 2);

				if (StringUtils::UTF32ToUTF8(s, u8Str)) {
					auto lastJ = j;
					if (u8Str == "\\\\") {
						u8Str = "\\";
						j++;
					}
					else {
						int newHead;
						char tagName;
						int newSize;
						cocos2d::Color3B newColor;
						parseTag(u32Str, j, *pSize, iniSize, iniColor, &newHead, &tagName, &newSize, &newColor);
						if (tagName == 'S') {
							*pSize = newSize;
							j = newHead - 1;
							continue;
						}
						else if (tagName == 'C') {
							*pColor = newColor;
							j = newHead - 1;
							continue;
						}
						else {
							s = u32Str.substr(j, 1);
							StringUtils::UTF32ToUTF8(s, u8Str);
						}
					}

					// 描画用ノードを作成
					cocos2d::Label* node = nullptr;
					if (j < static_cast<unsigned int>(nodeList->count())) {
						// 既にノードがある場合は再利用
						node = fm->createOrSetWithFontData(u8Str, fontData, iniSize, *pSize, dynamic_cast<cocos2d::Label*>(nodeList->getObjectAtIndex(j)));
					}
					else {
						// ノードが足りないので作成
						node = fm->createOrSetWithFontData(u8Str, fontData, iniSize, *pSize);
						if (node) {
							nodeList->addObject(node);
						}
					}

					if (node) {
						this->addChild(node);

						node->setColor(*pColor);

						// ピボットを左端になるように設定
						node->setAnchorPoint(Vec2::ANCHOR_TOP_LEFT);
						node->setHorizontalAlignment(cocos2d::TextHAlignment::CENTER);
						node->setVerticalAlignment(cocos2d::TextVAlignment::CENTER);

						// 座標を設定
						node->setPositionX(x);
					}

					float scaleX = 1.0f;

					// 画像フォントでプロポーショナルが設定されている場合
					if (fontData->getMainFontSetting()->getImageFontFlag() && !fontData->getMainFontSetting()->getFixedWidth()) {

						float fontW = fm->getImageFontWidth(fontData);
						float w = 1.0f;
						// 全角文字の場合
						if (isZenkaku(u8Str)) {
							w = fontData->getMainFontSetting()->getZenkakuWidth();
						}
						// 半角文字の場合
						else {
							w = fontData->getMainFontSetting()->getHankakuWidth();
						}

						if (fontW > 0) {
							scaleX = w / fontW;
						}
						else {
							scaleX = 0;
						}

						if (node) {
							// スケール値を設定
							node->setScaleX(node->getScaleX() * scaleX);
						}
					}

					// X座標を更新
					auto size = node ? node->getContentSize() : cocos2d::Size::ZERO;
					if (node) {
						size.width *= node->getScaleX();
						size.height *= node->getScaleY();
					}
					x += size.width;

					// 一文字目の横幅を格納
					if (lastJ == 0) {
						firstWidth = x;
					}

					// 文字間を設定
					if (j < u32Str.length() - 1) {
						x += letterSpace;
					}

					// 幅を更新
					if (height < size.height) {
						height = size.height;
					}
				}
			}
		}
	}

	// 文字間の値が負数で座標が-になってしまった時の対応
	if (x <= 0) {
		x = firstWidth;
	}


	setContentSize(Size(x, height));
	return true;
}

bool TextLineNode::isZenkaku(std::string str)
{
	bool rc = false;
	int i = 0;
	const char *c = str.c_str();

	if ((c[0] < 0x20) || (c[0] > 0x7e))
	{
		//半角以外
		rc = true;
	}

	return(rc);
}

NS_AGTK_END
