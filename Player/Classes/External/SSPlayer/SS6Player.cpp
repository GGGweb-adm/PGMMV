// 
//  SS6Player.cpp
//
#include "SS6Player.h"
#include "SS6PlayerData.h"
#include "SS6PlayerTypes.h"
#include "common/Animator/ssplayer_matrix.h"


namespace ss
{

/**
* SSPlayerControl
Cocos2d-xからSSPlayerを使用するためのラッパークラス
*/
cocos2d::GLProgram* SSPlayerControl::_defaultShaderProgram = nullptr;
cocos2d::GLProgram* SSPlayerControl::_MASKShaderProgram = nullptr;
cocos2d::GLProgram* SSPlayerControl::_partColorMIXONEShaderProgram = nullptr;
cocos2d::GLProgram* SSPlayerControl::_partColorMIXVERTShaderProgram = nullptr;
cocos2d::GLProgram* SSPlayerControl::_partColorMULShaderProgram = nullptr;
cocos2d::GLProgram* SSPlayerControl::_partColorADDShaderProgram = nullptr;
cocos2d::GLProgram* SSPlayerControl::_partColorSUBShaderProgram = nullptr;

std::map<int, int> SSPlayerControl::_MASK_uniform_map;
std::map<int, int> SSPlayerControl::_MIXONE_uniform_map;
std::map<int, int> SSPlayerControl::_MIXVERT_uniform_map;
std::map<int, int> SSPlayerControl::_MUL_uniform_map;
std::map<int, int> SSPlayerControl::_ADD_uniform_map;
std::map<int, int> SSPlayerControl::_SUB_uniform_map;

SSPlayerControl::SSPlayerControl()
{
	_ssp = nullptr;
	_position = cocos2d::Vec2(0,0);	//プレイヤーのポジション
	_enableRenderingBlendFunc = false;
}
SSPlayerControl::~SSPlayerControl()
{
	if (_ssp)
	{
		delete (_ssp);
		_ssp = nullptr;
	}
}

SSPlayerControl* SSPlayerControl::create(ResourceManager* resman)
{
	SSPlayerControl* obj = new SSPlayerControl();
	if (obj && obj->init())
	{
		obj->getSSPInstance()->setResourceManager(resman);
		obj->initCustomShaderProgram();

		obj->autorelease();
		obj->scheduleUpdate();
		return obj;
	}
	CC_SAFE_DELETE(obj);
	return nullptr;
}
Player* SSPlayerControl::getSSPInstance()
{
	if (_ssp == nullptr)
	{
		_ssp = Player::create();
		_ssp->_playercontrol = this;
	}
	return _ssp;
}

//sprite のオーバーライド
bool SSPlayerControl::init()
{
	if (!cocos2d::Sprite::init())
	{
		return false;
	}
	return true;
}

void SSPlayerControl::update(float dt)
{
	cocos2d::Mat4 mat = getNodeToWorldTransform();

	_ssp->setParentMatrix(mat.m, true);
	_ssp->setAlpha(_displayedOpacity);
	_ssp->update(dt);
}

void SSPlayerControl::onRenderingDraw(cocos2d::Renderer *renderer, const cocos2d::Mat4 &transform, uint32_t flags)
{
	//レンダリング描画
	SSRenderingBlendFuncEnable(true);
	onDraw(renderer, transform, flags);
	SSRenderingBlendFuncEnable(false);
}

void SSPlayerControl::onDraw(cocos2d::Renderer *renderer, const cocos2d::Mat4 &transform, uint32_t flags)
{
	//アップデートを行わずにDrawされた場合（RenderTextuer等）もあるので親ノードのマトリクスを設定する
	cocos2d::Mat4 mat = getNodeToWorldTransform();
	_ssp->setParentMatrix(mat.m, true);

	//プレイヤーの描画
	this->getGLProgram()->use();
	_ssp->draw();
}

void SSPlayerControl::draw(cocos2d::Renderer *renderer, const cocos2d::Mat4 &transform, uint32_t flags)
{
	if (_enableRenderingBlendFunc == false)
	{
		//通常描画
		_customCommand.init(_globalZOrder, transform, flags);
		_customCommand.func = CC_CALLBACK_0(SSPlayerControl::onDraw, this, renderer, transform, flags);
		renderer->addCommand(&_customCommand);
	}
	else
	{
		//レンダリング用描画
		_customCommandRendering.init(_globalZOrder, transform, flags);
		_customCommandRendering.func = CC_CALLBACK_0(SSPlayerControl::onRenderingDraw, this, renderer, transform, flags);
		renderer->addCommand(&_customCommandRendering);
	}
}

/// ポジションセットをオ－バーライドしてプレイヤーの位置を更新する
void SSPlayerControl::setPosition(const cocos2d::Vec2& position)
{
	setPosition(position.x, position.y);
}
void SSPlayerControl::setPosition(float x, float y)
{
	Sprite::setPosition(x, y);

	if ((_position.x != x) || (_position.y != y))
	{
		//座標が更新された場合のみ行う
		update(0);	//プレイヤー内部の座標を更新するために経過時間0で更新を行う
	}
	_position.x = x;
	_position.y = y;
}

//sprite のオーバーライドここまで
//sprite のシェーダー
static const GLchar * ssPositionTextureColor_vartex = R"(
attribute vec4 a_position;
attribute vec2 a_texCoord;
attribute vec4 a_color;

#ifdef GL_ES
varying lowp vec4 v_fragmentColor;
varying mediump vec2 v_texCoord;
#else
varying vec4 v_fragmentColor;
varying vec2 v_texCoord;
#endif

void main()
{
    gl_Position = CC_MVPMatrix * a_position;
    v_fragmentColor = a_color;
    v_texCoord = a_texCoord;
}
)"; 
const char* ssMASKPositionTextureColor_frag = R"(
#ifdef GL_ES
precision lowp float;
#endif

varying vec4 v_fragmentColor;
varying vec2 v_texCoord;

uniform float u_rate;

void main()
{
	vec4 pixel = texture2D(CC_Texture0, v_texCoord);
    if(pixel.a <= u_rate)
    {
        discard;
    }
    gl_FragColor = v_fragmentColor * pixel;
}
)";
static const GLchar * ssMIXONEPositionTextureColor_frag = R"(
#ifdef GL_ES
precision lowp float;
#endif

varying vec4 v_fragmentColor;
varying vec2 v_texCoord;

uniform float u_rate;

void main()
{
	vec4 pixel = texture2D(CC_Texture0, v_texCoord);
    gl_FragColor.rgb = ( v_fragmentColor.rgb * u_rate ) + ( pixel.rgb * ( 1.0 - u_rate ) );
    gl_FragColor.a = pixel.a * v_fragmentColor.a;
}
)";
static const GLchar * ssMIXVERTPositionTextureColor_frag = R"(
#ifdef GL_ES
precision lowp float;
#endif

varying vec4 v_fragmentColor;
varying vec2 v_texCoord;

uniform float u_rate;

void main()
{
	vec4 pixel = texture2D(CC_Texture0, v_texCoord);
    gl_FragColor.rgb = ( v_fragmentColor.rgb * v_fragmentColor.a ) + ( pixel.rgb * ( 1.0 - v_fragmentColor.a ) );
    gl_FragColor.a = pixel.a * u_rate;
}
)";
const char* ssMULPositionTextureColor_frag = R"(
#ifdef GL_ES
precision lowp float;
#endif

varying vec4 v_fragmentColor;
varying vec2 v_texCoord;

void main()
{
	vec4 pixel = texture2D(CC_Texture0, v_texCoord);
    gl_FragColor = v_fragmentColor * pixel;
}
)";
static const GLchar * ssADDPositionTextureColor_frag = R"(
#ifdef GL_ES
precision lowp float;
#endif

varying vec4 v_fragmentColor;
varying vec2 v_texCoord;

void main()
{
	vec4 pixel = texture2D(CC_Texture0, v_texCoord);
    gl_FragColor = pixel + v_fragmentColor;
    gl_FragColor.a = pixel.a * v_fragmentColor.a;
}
)";
static const GLchar * ssSUBPositionTextureColor_frag = R"(
#ifdef GL_ES
precision lowp float;
#endif

varying vec4 v_fragmentColor;
varying vec2 v_texCoord;

void main()
{
	vec4 pixel = texture2D(CC_Texture0, v_texCoord);
    gl_FragColor = pixel - v_fragmentColor;
    gl_FragColor.a = pixel.a * v_fragmentColor.a;
}
)";

void SSPlayerControl::initCustomShaderProgram( )
{
	using namespace cocos2d;

	if (SSPlayerControl::_defaultShaderProgram == nullptr )
	{
		SSPlayerControl::_defaultShaderProgram = this->getGLProgram();
	}
	if (SSPlayerControl::_MASKShaderProgram == nullptr)
	{
		GLProgram* p = nullptr;
		p = new GLProgram();

		p->initWithByteArrays(ssPositionTextureColor_vartex, ssMASKPositionTextureColor_frag);
		//	this->setShaderProgram(p);

		// 頂点情報のバインド  
		glBindAttribLocation(p->getProgram(), cocos2d::GLProgram::VERTEX_ATTRIB_POSITION, GLProgram::ATTRIBUTE_NAME_POSITION);
		glBindAttribLocation(p->getProgram(), cocos2d::GLProgram::VERTEX_ATTRIB_TEX_COORD, GLProgram::ATTRIBUTE_NAME_TEX_COORD);
		glBindAttribLocation(p->getProgram(), cocos2d::GLProgram::VERTEX_ATTRIB_COLOR, GLProgram::ATTRIBUTE_NAME_COLOR);

		p->link();
		p->updateUniforms();
		// 頂点に付加する情報のロケーションを取得  
		SSPlayerControl::_MASK_uniform_map[WVP] = glGetUniformLocation(p->getProgram(), GLProgram::UNIFORM_NAME_MVP_MATRIX);
		SSPlayerControl::_MASK_uniform_map[SAMPLER] = glGetUniformLocation(p->getProgram(), GLProgram::UNIFORM_NAME_SAMPLER0);
		SSPlayerControl::_MASK_uniform_map[RATE] = glGetUniformLocation(p->getProgram(), "u_rate");

		SSPlayerControl::_MASKShaderProgram = p;
	}
	if (SSPlayerControl::_partColorMIXONEShaderProgram == nullptr)
	{
		GLProgram* p = nullptr;
		p = new GLProgram();

		p->initWithByteArrays(ssPositionTextureColor_vartex, ssMIXONEPositionTextureColor_frag);
		//	this->setShaderProgram(p);

		// 頂点情報のバインド  
		glBindAttribLocation(p->getProgram(), cocos2d::GLProgram::VERTEX_ATTRIB_POSITION, GLProgram::ATTRIBUTE_NAME_POSITION);
		glBindAttribLocation(p->getProgram(), cocos2d::GLProgram::VERTEX_ATTRIB_TEX_COORD, GLProgram::ATTRIBUTE_NAME_TEX_COORD);
		glBindAttribLocation(p->getProgram(), cocos2d::GLProgram::VERTEX_ATTRIB_COLOR, GLProgram::ATTRIBUTE_NAME_COLOR);

		p->link();
		p->updateUniforms();
		// 頂点に付加する情報のロケーションを取得  
		SSPlayerControl::_MIXONE_uniform_map[WVP] = glGetUniformLocation(p->getProgram(), GLProgram::UNIFORM_NAME_MVP_MATRIX);
		SSPlayerControl::_MIXONE_uniform_map[SAMPLER] = glGetUniformLocation(p->getProgram(), GLProgram::UNIFORM_NAME_SAMPLER0);
		SSPlayerControl::_MIXONE_uniform_map[RATE] = glGetUniformLocation(p->getProgram(), "u_rate");

		SSPlayerControl::_partColorMIXONEShaderProgram = p;
	}
	if (SSPlayerControl::_partColorMIXVERTShaderProgram == nullptr)
	{
		GLProgram* p = nullptr;
		p = new GLProgram();

		p->initWithByteArrays(ssPositionTextureColor_vartex, ssMIXVERTPositionTextureColor_frag);
		//	this->setShaderProgram(p);

		// 頂点情報のバインド  
		glBindAttribLocation(p->getProgram(), cocos2d::GLProgram::VERTEX_ATTRIB_POSITION, GLProgram::ATTRIBUTE_NAME_POSITION);
		glBindAttribLocation(p->getProgram(), cocos2d::GLProgram::VERTEX_ATTRIB_TEX_COORD, GLProgram::ATTRIBUTE_NAME_TEX_COORD);
		glBindAttribLocation(p->getProgram(), cocos2d::GLProgram::VERTEX_ATTRIB_COLOR, GLProgram::ATTRIBUTE_NAME_COLOR);

		p->link();
		p->updateUniforms();
		// 頂点に付加する情報のロケーションを取得  
		SSPlayerControl::_MIXVERT_uniform_map[WVP] = glGetUniformLocation(p->getProgram(), GLProgram::UNIFORM_NAME_MVP_MATRIX);
		SSPlayerControl::_MIXVERT_uniform_map[SAMPLER] = glGetUniformLocation(p->getProgram(), GLProgram::UNIFORM_NAME_SAMPLER0);
		SSPlayerControl::_MIXVERT_uniform_map[RATE] = glGetUniformLocation(p->getProgram(), "u_rate");

		SSPlayerControl::_partColorMIXVERTShaderProgram = p;
	}
	if (SSPlayerControl::_partColorMULShaderProgram == nullptr)
	{
		GLProgram* p = nullptr;
		p = new GLProgram();

		p->initWithByteArrays(ssPositionTextureColor_vartex, ssMULPositionTextureColor_frag);
		//	this->setShaderProgram(p);

		// 頂点情報のバインド  
		glBindAttribLocation(p->getProgram(), cocos2d::GLProgram::VERTEX_ATTRIB_POSITION, GLProgram::ATTRIBUTE_NAME_POSITION);
		glBindAttribLocation(p->getProgram(), cocos2d::GLProgram::VERTEX_ATTRIB_TEX_COORD, GLProgram::ATTRIBUTE_NAME_TEX_COORD);
		glBindAttribLocation(p->getProgram(), cocos2d::GLProgram::VERTEX_ATTRIB_COLOR, GLProgram::ATTRIBUTE_NAME_COLOR);

		p->link();
		p->updateUniforms();
		// 頂点に付加する情報のロケーションを取得  
		SSPlayerControl::_MUL_uniform_map[WVP] = glGetUniformLocation(p->getProgram(), GLProgram::UNIFORM_NAME_MVP_MATRIX);
		SSPlayerControl::_MUL_uniform_map[SAMPLER] = glGetUniformLocation(p->getProgram(), GLProgram::UNIFORM_NAME_SAMPLER0);

		SSPlayerControl::_partColorMULShaderProgram = p;
	}
	if (SSPlayerControl::_partColorADDShaderProgram == nullptr)
	{
		GLProgram* p = nullptr;
		p = new GLProgram();

		p->initWithByteArrays(ssPositionTextureColor_vartex, ssADDPositionTextureColor_frag);
		//	this->setShaderProgram(p);

		// 頂点情報のバインド  
		glBindAttribLocation(p->getProgram(), cocos2d::GLProgram::VERTEX_ATTRIB_POSITION, GLProgram::ATTRIBUTE_NAME_POSITION);
		glBindAttribLocation(p->getProgram(), cocos2d::GLProgram::VERTEX_ATTRIB_TEX_COORD, GLProgram::ATTRIBUTE_NAME_TEX_COORD);
		glBindAttribLocation(p->getProgram(), cocos2d::GLProgram::VERTEX_ATTRIB_COLOR, GLProgram::ATTRIBUTE_NAME_COLOR);

		p->link();
		p->updateUniforms();
		// 頂点に付加する情報のロケーションを取得  
		SSPlayerControl::_ADD_uniform_map[WVP] = glGetUniformLocation(p->getProgram(), GLProgram::UNIFORM_NAME_MVP_MATRIX);
		SSPlayerControl::_ADD_uniform_map[SAMPLER] = glGetUniformLocation(p->getProgram(), GLProgram::UNIFORM_NAME_SAMPLER0);

		SSPlayerControl::_partColorADDShaderProgram = p;
	}
	if (SSPlayerControl::_partColorSUBShaderProgram == nullptr)
	{
		GLProgram* p = nullptr;
		p = new GLProgram();

		p->initWithByteArrays(ssPositionTextureColor_vartex, ssSUBPositionTextureColor_frag);
		//	this->setShaderProgram(p);

		// 頂点情報のバインド  
		glBindAttribLocation(p->getProgram(), cocos2d::GLProgram::VERTEX_ATTRIB_POSITION, GLProgram::ATTRIBUTE_NAME_POSITION);
		glBindAttribLocation(p->getProgram(), cocos2d::GLProgram::VERTEX_ATTRIB_TEX_COORD, GLProgram::ATTRIBUTE_NAME_TEX_COORD);
		glBindAttribLocation(p->getProgram(), cocos2d::GLProgram::VERTEX_ATTRIB_COLOR, GLProgram::ATTRIBUTE_NAME_COLOR);

		p->link();
		p->updateUniforms();
		// 頂点に付加する情報のロケーションを取得  
		SSPlayerControl::_SUB_uniform_map[WVP] = glGetUniformLocation(p->getProgram(), GLProgram::UNIFORM_NAME_MVP_MATRIX);
		SSPlayerControl::_SUB_uniform_map[SAMPLER] = glGetUniformLocation(p->getProgram(), GLProgram::UNIFORM_NAME_SAMPLER0);

		SSPlayerControl::_partColorSUBShaderProgram = p;
	}
}





/**
 * definition
 */

static const ss_u32 DATA_ID = 0x42505353;
static const ss_u32 DATA_VERSION = 11;


/**
 * utilites
 */
static void splitPath(std::string& directoty, std::string& filename, const std::string& path)
{
    std::string f = path;
    std::string d = "";

    size_t pos = path.find_last_of("/");
	if (pos == std::string::npos) pos = path.find_last_of("\\");	// for win

    if (pos != std::string::npos)
    {
        d = path.substr(0, pos+1);
        f = path.substr(pos+1);
    }

	directoty = d;
	filename = f;
}

// printf 形式のフォーマット
#ifndef va_copy
#    define va_copy(dest, src) ((dest) = (src))
#endif
static std::string Format(const char* format, ...){

	static std::vector<char> tmp(1000);

	va_list args, source;
	va_start(args, format);
	va_copy( source , args );

	while (1)
	{
		va_copy( args , source );
#if _WIN32
		//Windows
		if (_vsnprintf(&tmp[0], tmp.size(), format, args) == -1)
#else
        if (vsnprintf(&tmp[0], tmp.size(), format, args) == -1)
#endif
        {
			tmp.resize(tmp.size() * 2);
		}
		else
		{
			break;
		}
	}
	tmp.push_back('\0');
	std::string ret = &(tmp[0]);
	va_end(args);
	return ret;
}

//座標回転処理
//指定した座標を中心に回転後した座標を取得します
void get_uv_rotation(float *u, float *v, float cu, float cv, float deg)
{
	float dx = *u - cu; // 中心からの距離(X)
	float dy = *v - cv; // 中心からの距離(Y)

	float tmpX = (dx * cosf(SSRadianToDegree(deg))) - (dy * sinf(SSRadianToDegree(deg))); // 回転
	float tmpY = (dx * sinf(SSRadianToDegree(deg))) + (dy * cosf(SSRadianToDegree(deg)));

	*u = (cu + tmpX); // 元の座標にオフセットする
	*v = (cv + tmpY);

}

//乱数シードに利用するユニークIDを作成します。
//この値は全てのSS5プレイヤー共通で使用します
int seedMakeID = 123456;
//エフェクトに与えるシードを取得する関数
unsigned int getRandomSeed()
{
	seedMakeID++;	//ユニークIDを更新します。
	//時間＋ユニークIDにする事で毎回シードが変わるようにします。
	unsigned int rc = (unsigned int)time(0) + (seedMakeID);

	return(rc);
}



/**
 * ToPointer
 */
class ToPointer
{
public:
	explicit ToPointer(const void* base)
		: _base(static_cast<const char*>(base)) {}
	
	const void* operator()(ss_offset offset) const
	{
		return (_base + offset);
	}

private:
	const char*	_base;
};


/**
 * DataArrayReader
 */
class DataArrayReader
{
public:
	DataArrayReader(const ss_u16* dataPtr)
		: _dataPtr(dataPtr)
	{}

	ss_u16 readU16() { return *_dataPtr++; }
	ss_s16 readS16() { return static_cast<ss_s16>(*_dataPtr++); }

	unsigned int readU32()
	{
		unsigned int l = readU16();
		unsigned int u = readU16();
		return static_cast<unsigned int>((u << 16) | l);
	}

	int readS32()
	{
		return static_cast<int>(readU32());
	}

	float readFloat()
	{
		union {
			float			f;
			unsigned int	i;
		} c;
		c.i = readU32();
		return c.f;
	}
	
	void readColor(SSColor4B& color)
	{
		unsigned int raw = readU32();
		color.a = static_cast<unsigned char>(raw >> 24);
		color.r = static_cast<unsigned char>(raw >> 16);
		color.g = static_cast<unsigned char>(raw >> 8);
		color.b = static_cast<unsigned char>(raw);
	}
	
	ss_offset readOffset()
	{
		return static_cast<ss_offset>(readS32());
	}

private:
	const ss_u16*	_dataPtr;
};


/**
 * CellRef
 */
struct CellRef
{
	const Cell* cell;
	TextuerData texture;
	SSRect rect;
	std::string texname;
};


/**
 * CellCache
 */
class CellCache
{
public:
	CellCache()
	{
	}
	~CellCache()
	{
		releseReference();
	}

	static CellCache* create(const ProjectData* data, const std::string& imageBaseDir, const std::string& zipFilepath)
	{
		CellCache* obj = new CellCache();
		if (obj)
		{
			obj->init(data, imageBaseDir, zipFilepath);
		}
		return obj;
	}

	CellRef* getReference(int index)
	{
		if (index < 0 || index >= (int)_refs.size())
		{
			SSLOGERROR("Index out of range > %d", index);
			SS_ASSERT(0);
		}
		CellRef* ref = _refs.at(index);
		return ref;
	}

	//指定した名前のセルの参照テクスチャを変更する
	bool setCellRefTexture(const ProjectData* data, const char* cellName, long texture)
	{
		bool rc = false;

		ToPointer ptr(data);
		const Cell* cells = static_cast<const Cell*>(ptr(data->cells));

		//名前からインデックスの取得
		int cellindex = -1;
		for (int i = 0; i < data->numCells; i++)
		{
			const Cell* cell = &cells[i];
			const CellMap* cellMap = static_cast<const CellMap*>(ptr(cell->cellMap));
			const char* name = static_cast<const char*>(ptr(cellMap->name));
			if (strcmp(cellName, name) == 0)
			{
				CellRef* ref = getReference(i);
				ref->texture.handle = texture;
				rc = true;
			}
		}

		return(rc);
	}

	//指定したデータのテクスチャを破棄する
	bool releseTexture(const ProjectData* data)
	{
		bool rc = false;

		ToPointer ptr(data);
		const Cell* cells = static_cast<const Cell*>(ptr(data->cells));
		for (int i = 0; i < data->numCells; i++)
		{
			const Cell* cell = &cells[i];
			const CellMap* cellMap = static_cast<const CellMap*>(ptr(cell->cellMap));
			{
				CellRef* ref = _refs.at(i);
				if (ref->texture.handle != -1 )
				{
					SSTextureRelese(ref->texture.handle);
					ref->texture.handle = -1;
					rc = true;
				}
			}
		}
		return(rc);
	}

//#AGTK-NX
#if (CC_TARGET_PLATFORM == CC_PLATFORM_NX)
#endif
protected:
	void init(const ProjectData* data, const std::string& imageBaseDir, const std::string& zipFilepath)
	{

		SS_ASSERT2(data != NULL, "Invalid data");
		
		_textures.clear();
		_refs.clear();
		_texname.clear();

		ToPointer ptr(data);
		const Cell* cells = static_cast<const Cell*>(ptr(data->cells));

		for (int i = 0; i < data->numCells; i++)
		{
			const Cell* cell = &cells[i];
			const CellMap* cellMap = static_cast<const CellMap*>(ptr(cell->cellMap));
			
			if (cellMap->index >= (int)_textures.size())
			{
				const char* imagePath = static_cast<const char*>(ptr(cellMap->imagePath));
//#AGTK-NX
#if (CC_TARGET_PLATFORM == CC_PLATFORM_NX)
#else
				addTexture(imagePath, imageBaseDir, (SsTexWrapMode::_enum)cellMap->wrapmode, (SsTexFilterMode::_enum)cellMap->filtermode, zipFilepath);
#endif
			}

			//セル情報だけ入れておく
			//テクスチャの読み込みはゲーム側に任せる
			CellRef* ref = new CellRef();
			ref->cell = cell;
			ref->texture = _textures.at(cellMap->index);
			ref->texname = _texname.at(cellMap->index);
			ref->rect = SSRect(cell->x, cell->y, cell->width, cell->height);
			_refs.push_back(ref);
		}

	}
	//キャッシュの削除
	void releseReference(void)
	{
		for (int i = 0; i < (int)_refs.size(); i++)
		{
			CellRef* ref = _refs.at(i);
			if (ref->texture.handle != -1 )
			{
				SSTextureRelese(ref->texture.handle);
				ref->texture.handle = -1;
			}
			delete ref;
		}
		_refs.clear();
	}

	void addTexture(const std::string& imagePath, const std::string& imageBaseDir, SsTexWrapMode::_enum  wrapmode, SsTexFilterMode::_enum filtermode, const std::string& zipFilepath)
	{
		std::string path = "";
		
		if (isAbsolutePath(imagePath))
		{
			// 絶対パスのときはそのまま扱う
			path = imagePath;
		}
		else
		{
			// 相対パスのときはimageBaseDirを付与する
			path.append(imageBaseDir);
			size_t pathLen = path.length();
			if (pathLen && path.at(pathLen-1) != '/' && path.at(pathLen-1) != '\\')
			{
				path.append("/");
			}
			path.append(imagePath);
		}

		//テクスチャの読み込み
		long tex = SSTextureLoad(path.c_str(), wrapmode, filtermode, zipFilepath.c_str());
		SSLOG("load: %s", path.c_str());
		TextuerData texdata;
		texdata.handle = tex;
		int w;
		int h;
		SSGetTextureSize(texdata.handle, w, h);
		texdata.size_w = w;
		texdata.size_h = h;

		_textures.push_back(texdata);
		_texname.push_back(path);

	}

protected:
	std::vector<std::string>			_texname;
	std::vector<TextuerData>			_textures;
	std::vector<CellRef*>				_refs;
};


/**
* EffectCache
*/
class EffectCache
{
public:
	EffectCache()
	{
	}
	~EffectCache()
	{
		releseReference();
	}

	static EffectCache* create(const ProjectData* data, const std::string& imageBaseDir, CellCache* cellCache)
	{
		EffectCache* obj = new EffectCache();
		if (obj)
		{
			obj->init(data, imageBaseDir, cellCache);
			//			obj->autorelease();
		}
		return obj;
	}

	/**
	* エフェクトファイル名を指定してEffectRefを得る
	*/
	SsEffectModel* getReference(const std::string& name)
	{
		SsEffectModel* ref = _dic.at(name);
		return ref;
	}

	void dump()
	{
		std::map<std::string, SsEffectModel*>::iterator it = _dic.begin();
		while (it != _dic.end())
		{
			SSLOG("%s", (*it).second);
			++it;
		}
	}
protected:
	void init(const ProjectData* data, const std::string& imageBaseDir, CellCache* cellCache)
	{
		SS_ASSERT2(data != NULL, "Invalid data");

		ToPointer ptr(data);

		//ssbpからエフェクトファイル配列を取得
		const EffectFile* effectFileArray = static_cast<const EffectFile*>(ptr(data->effectFileList));

		for (int listindex = 0; listindex < data->numEffectFileList; listindex++)
		{
			//エフェクトファイル配列からエフェクトファイルを取得
			const EffectFile* effectFile = &effectFileArray[listindex];

			//保持用のエフェクトファイル情報を作成
			SsEffectModel *effectmodel = new SsEffectModel();
			std::string effectFileName = static_cast<const char*>(ptr(effectFile->name));

			//エフェクトファイルからエフェクトノード配列を取得
			const EffectNode* effectNodeArray = static_cast<const EffectNode*>(ptr(effectFile->effectNode));
			for (int nodeindex = 0; nodeindex < effectFile->numNodeList; nodeindex++)
			{
				const EffectNode* effectNode = &effectNodeArray[nodeindex];		//エフェクトノード配列からエフェクトノードを取得

				SsEffectNode *node = new SsEffectNode();
				node->arrayIndex = effectNode->arrayIndex;
				node->parentIndex = effectNode->parentIndex;
				node->type = (SsEffectNodeType::_enum)effectNode->type;
				node->visible = true;

				SsEffectBehavior behavior;
				//セル情報を作成
				behavior.CellIndex = effectNode->cellIndex;
				CellRef* cellRef = behavior.CellIndex >= 0 ? cellCache->getReference(behavior.CellIndex) : NULL;
				if (cellRef)
				{
					behavior.refCell.pivot_X = cellRef->cell->pivot_X;
					behavior.refCell.pivot_Y = cellRef->cell->pivot_Y;
					behavior.refCell.texture = cellRef->texture;
					behavior.refCell.texname = cellRef->texname;
					behavior.refCell.rect = cellRef->rect;
					behavior.refCell.cellIndex = behavior.CellIndex;
					std::string name = static_cast<const char*>(ptr(cellRef->cell->name));
					behavior.refCell.cellName = name;
					behavior.refCell.u1 = cellRef->cell->u1;
					behavior.refCell.v1 = cellRef->cell->v1;
					behavior.refCell.u2 = cellRef->cell->u2;
					behavior.refCell.v2 = cellRef->cell->v2;

				}
				//				behavior.CellName;
				//				behavior.CellMapName;
				behavior.blendType = (SsRenderBlendType::_enum)effectNode->blendType;

				//エフェクトノードからビヘイビア配列を取得
				const ss_offset* behaviorArray = static_cast<const ss_offset*>(ptr(effectNode->Behavior));
				for (int behaviorindex = 0; behaviorindex < effectNode->numBehavior; behaviorindex++)
				{
					//ビヘイビア配列からビヘイビアを取得
					const ss_u16* behavior_adr = static_cast<const ss_u16*>(ptr(behaviorArray[behaviorindex]));
					DataArrayReader reader(behavior_adr);

					//パラメータを作ってpush_backで登録していく
					int type = reader.readS32();
					switch (type)
					{
					case SsEffectFunctionType::Basic:
					{
						//基本情報
						EffectParticleElementBasic readparam;
						readparam.priority = reader.readU32();			//表示優先度
						readparam.maximumParticle = reader.readU32();		//最大パーティクル数
						readparam.attimeCreate = reader.readU32();		//一度に作成するパーティクル数
						readparam.interval = reader.readU32();			//生成間隔
						readparam.lifetime = reader.readU32();			//エミッター生存時間
						readparam.speedMinValue = reader.readFloat();		//初速最小
						readparam.speedMaxValue = reader.readFloat();		//初速最大
						readparam.lifespanMinValue = reader.readU32();	//パーティクル生存時間最小
						readparam.lifespanMaxValue = reader.readU32();	//パーティクル生存時間最大
						readparam.angle = reader.readFloat();				//射出方向
						readparam.angleVariance = reader.readFloat();		//射出方向範囲

						ParticleElementBasic *effectParam = new ParticleElementBasic();
						effectParam->setType((SsEffectFunctionType::enum_)type);				//コマンドの種類
						effectParam->priority = readparam.priority;							//表示優先度
						effectParam->maximumParticle = readparam.maximumParticle;			//最大パーティクル数
						effectParam->attimeCreate = readparam.attimeCreate;					//一度に作成するパーティクル数
						effectParam->interval = readparam.interval;							//生成間隔
						effectParam->lifetime = readparam.lifetime;							//エミッター生存時間
						effectParam->speed.setMinMax(readparam.speedMinValue, readparam.speedMaxValue);				//初速
						effectParam->lifespan.setMinMax(readparam.lifespanMinValue, readparam.lifespanMaxValue);	//パーティクル生存時間
						effectParam->angle = readparam.angle;								//射出方向
						effectParam->angleVariance = readparam.angleVariance;				//射出方向範囲

						behavior.plist.push_back(effectParam);												//パラメータを追加
						break;
					}
					case SsEffectFunctionType::RndSeedChange:
					{
						//シード上書き
						EffectParticleElementRndSeedChange readparam;
						readparam.Seed = reader.readU32();				//上書きするシード値

						ParticleElementRndSeedChange *effectParam = new ParticleElementRndSeedChange();
						effectParam->setType((SsEffectFunctionType::enum_)type);				//コマンドの種類
						effectParam->Seed = readparam.Seed;							//上書きするシード値

						behavior.plist.push_back(effectParam);												//パラメータを追加
						break;
					}
					case SsEffectFunctionType::Delay:
					{
						//発生：タイミング
						EffectParticleElementDelay readparam;
						readparam.DelayTime = reader.readU32();			//遅延時間

						ParticleElementDelay *effectParam = new ParticleElementDelay();
						effectParam->setType((SsEffectFunctionType::enum_)type);				//コマンドの種類
						effectParam->DelayTime = readparam.DelayTime;			//遅延時間

						behavior.plist.push_back(effectParam);												//パラメータを追加
						break;
					}
					case SsEffectFunctionType::Gravity:
					{
						//重力を加える
						EffectParticleElementGravity readparam;
						readparam.Gravity_x = reader.readFloat();			//X方向の重力
						readparam.Gravity_y = reader.readFloat();			//Y方向の重力

						ParticleElementGravity *effectParam = new ParticleElementGravity();
						effectParam->setType((SsEffectFunctionType::enum_)type);				//コマンドの種類
						effectParam->Gravity.x = readparam.Gravity_x;			//X方向の重力
						effectParam->Gravity.y = readparam.Gravity_y;			//Y方向の重力

						behavior.plist.push_back(effectParam);												//パラメータを追加
						break;
					}
					case SsEffectFunctionType::Position:
					{
						//座標：生成時
						EffectParticleElementPosition readparam;
						readparam.OffsetXMinValue = reader.readFloat();	//X座標に加算最小
						readparam.OffsetXMaxValue = reader.readFloat();	//X座標に加算最大
						readparam.OffsetYMinValue = reader.readFloat();	//X座標に加算最小
						readparam.OffsetYMaxValue = reader.readFloat();	//X座標に加算最大

						ParticleElementPosition *effectParam = new ParticleElementPosition();
						effectParam->setType((SsEffectFunctionType::enum_)type);				//コマンドの種類
						effectParam->OffsetX.setMinMax(readparam.OffsetXMinValue, readparam.OffsetXMaxValue); 	//X座標に加算最小
						effectParam->OffsetY.setMinMax(readparam.OffsetYMinValue, readparam.OffsetYMaxValue);	//X座標に加算最小

						behavior.plist.push_back(effectParam);												//パラメータを追加
						break;
					}
					case SsEffectFunctionType::Rotation:
					{
						//Z回転を追加
						EffectParticleElementRotation readparam;
						readparam.RotationMinValue = reader.readFloat();		//角度初期値最小
						readparam.RotationMaxValue = reader.readFloat();		//角度初期値最大
						readparam.RotationAddMinValue = reader.readFloat();	//角度初期加算値最小
						readparam.RotationAddMaxValue = reader.readFloat();	//角度初期加算値最大

						ParticleElementRotation *effectParam = new ParticleElementRotation();
						effectParam->setType((SsEffectFunctionType::enum_)type);				//コマンドの種類
						effectParam->Rotation.setMinMax(readparam.RotationMinValue, readparam.RotationMaxValue);		//角度初期値最小
						effectParam->RotationAdd.setMinMax(readparam.RotationAddMinValue, readparam.RotationAddMaxValue);	//角度初期加算値最小

						behavior.plist.push_back(effectParam);												//パラメータを追加
						break;
					}
					case SsEffectFunctionType::TransRotation:
					{
						//Z回転速度変更
						EffectParticleElementRotationTrans readparam;
						readparam.RotationFactor = reader.readFloat();		//角度目標加算値
						readparam.EndLifeTimePer = reader.readFloat();		//到達時間

						ParticleElementRotationTrans *effectParam = new ParticleElementRotationTrans();
						effectParam->setType((SsEffectFunctionType::enum_)type);				//コマンドの種類
						effectParam->RotationFactor = readparam.RotationFactor;		//角度目標加算値
						effectParam->EndLifeTimePer = readparam.EndLifeTimePer;		//到達時間

						behavior.plist.push_back(effectParam);												//パラメータを追加
						break;
					}
					case SsEffectFunctionType::TransSpeed:
					{
						//速度：変化
						EffectParticleElementTransSpeed readparam;
						readparam.SpeedMinValue = reader.readFloat();			//速度目標値最小
						readparam.SpeedMaxValue = reader.readFloat();			//速度目標値最大

						ParticleElementTransSpeed *effectParam = new ParticleElementTransSpeed();
						effectParam->setType((SsEffectFunctionType::enum_)type);				//コマンドの種類
						effectParam->Speed.setMinMax(readparam.SpeedMinValue, readparam.SpeedMaxValue);			//速度目標値最小

						behavior.plist.push_back(effectParam);												//パラメータを追加
						break;
					}
					case SsEffectFunctionType::TangentialAcceleration:
					{
						//接線加速度
						EffectParticleElementTangentialAcceleration readparam;
						readparam.AccelerationMinValue = reader.readFloat();	//設定加速度最小
						readparam.AccelerationMaxValue = reader.readFloat();	//設定加速度最大

						ParticleElementTangentialAcceleration *effectParam = new ParticleElementTangentialAcceleration();
						effectParam->setType((SsEffectFunctionType::enum_)type);				//コマンドの種類
						effectParam->Acceleration.setMinMax(readparam.AccelerationMinValue, readparam.AccelerationMaxValue);	//設定加速度最小

						behavior.plist.push_back(effectParam);												//パラメータを追加
						break;
					}
					case SsEffectFunctionType::InitColor:
					{
						//カラーRGBA：生成時
						EffectParticleElementInitColor readparam;
						readparam.ColorMinValue = reader.readU32();			//設定カラー最小
						readparam.ColorMaxValue = reader.readU32();			//設定カラー最大

						ParticleElementInitColor *effectParam = new ParticleElementInitColor();
						effectParam->setType((SsEffectFunctionType::enum_)type);				//コマンドの種類

						int a = (readparam.ColorMinValue & 0xFF000000) >> 24;
						int r = (readparam.ColorMinValue & 0x00FF0000) >> 16;
						int g = (readparam.ColorMinValue & 0x0000FF00) >> 8;
						int b = (readparam.ColorMinValue & 0x000000FF) >> 0;
						SsU8Color mincol(r, g, b, a);
						a = (readparam.ColorMaxValue & 0xFF000000) >> 24;
						r = (readparam.ColorMaxValue & 0x00FF0000) >> 16;
						g = (readparam.ColorMaxValue & 0x0000FF00) >> 8;
						b = (readparam.ColorMaxValue & 0x000000FF) >> 0;
						SsU8Color maxcol(r, g, b, a);
						effectParam->Color.setMinMax(mincol, maxcol);			//設定カラー最小

						behavior.plist.push_back(effectParam);												//パラメータを追加
						break;
					}
					case SsEffectFunctionType::TransColor:
					{
						//カラーRGB：変化
						EffectParticleElementTransColor readparam;
						readparam.ColorMinValue = reader.readU32();			//設定カラー最小
						readparam.ColorMaxValue = reader.readU32();			//設定カラー最大

						ParticleElementTransColor *effectParam = new ParticleElementTransColor();
						effectParam->setType((SsEffectFunctionType::enum_)type);				//コマンドの種類

						int a = (readparam.ColorMinValue & 0xFF000000) >> 24;
						int r = (readparam.ColorMinValue & 0x00FF0000) >> 16;
						int g = (readparam.ColorMinValue & 0x0000FF00) >> 8;
						int b = (readparam.ColorMinValue & 0x000000FF) >> 0;
						SsU8Color mincol(r, g, b, a);
						a = (readparam.ColorMaxValue & 0xFF000000) >> 24;
						r = (readparam.ColorMaxValue & 0x00FF0000) >> 16;
						g = (readparam.ColorMaxValue & 0x0000FF00) >> 8;
						b = (readparam.ColorMaxValue & 0x000000FF) >> 0;
						SsU8Color maxcol(r, g, b, a);
						effectParam->Color.setMinMax(mincol, maxcol);			//設定カラー最小

						behavior.plist.push_back(effectParam);												//パラメータを追加
						break;
					}
					case SsEffectFunctionType::AlphaFade:
					{
						//フェード
						EffectParticleElementAlphaFade readparam;
						readparam.disprangeMinValue = reader.readFloat();		//表示区間開始
						readparam.disprangeMaxValue = reader.readFloat();		//表示区間終了

						ParticleElementAlphaFade *effectParam = new ParticleElementAlphaFade();
						effectParam->setType((SsEffectFunctionType::enum_)type);				//コマンドの種類
						effectParam->disprange.setMinMax(readparam.disprangeMinValue, readparam.disprangeMaxValue);		//表示区間開始

						behavior.plist.push_back(effectParam);												//パラメータを追加
						break;
					}
					case SsEffectFunctionType::Size:
					{
						//スケール：生成時
						EffectParticleElementSize readparam;
						readparam.SizeXMinValue = reader.readFloat();			//幅倍率最小
						readparam.SizeXMaxValue = reader.readFloat();			//幅倍率最大
						readparam.SizeYMinValue = reader.readFloat();			//高さ倍率最小
						readparam.SizeYMaxValue = reader.readFloat();			//高さ倍率最大
						readparam.ScaleFactorMinValue = reader.readFloat();		//倍率最小
						readparam.ScaleFactorMaxValue = reader.readFloat();		//倍率最大

						ParticleElementSize *effectParam = new ParticleElementSize();
						effectParam->setType((SsEffectFunctionType::enum_)type);				//コマンドの種類
						effectParam->SizeX.setMinMax(readparam.SizeXMinValue, readparam.SizeXMaxValue);			//幅倍率最小
						effectParam->SizeY.setMinMax(readparam.SizeYMinValue, readparam.SizeYMaxValue);			//高さ倍率最小
						effectParam->ScaleFactor.setMinMax(readparam.ScaleFactorMinValue, readparam.ScaleFactorMaxValue);		//倍率最小

						behavior.plist.push_back(effectParam);												//パラメータを追加
						break;
					}
					case SsEffectFunctionType::TransSize:
					{
						//スケール：変化
						EffectParticleElementTransSize readparam;
						readparam.SizeXMinValue = reader.readFloat();			//幅倍率最小
						readparam.SizeXMaxValue = reader.readFloat();			//幅倍率最大
						readparam.SizeYMinValue = reader.readFloat();			//高さ倍率最小
						readparam.SizeYMaxValue = reader.readFloat();			//高さ倍率最大
						readparam.ScaleFactorMinValue = reader.readFloat();		//倍率最小
						readparam.ScaleFactorMaxValue = reader.readFloat();		//倍率最大

						ParticleElementTransSize *effectParam = new ParticleElementTransSize();
						effectParam->setType((SsEffectFunctionType::enum_)type);				//コマンドの種類
						effectParam->SizeX.setMinMax(readparam.SizeXMinValue, readparam.SizeXMaxValue);			//幅倍率最小
						effectParam->SizeY.setMinMax(readparam.SizeYMinValue, readparam.SizeYMaxValue);			//高さ倍率最小
						effectParam->ScaleFactor.setMinMax(readparam.ScaleFactorMinValue, readparam.ScaleFactorMaxValue);		//倍率最小

						behavior.plist.push_back(effectParam);												//パラメータを追加
						break;
					}
					case SsEffectFunctionType::PointGravity:
					{
						//重力点の追加
						EffectParticlePointGravity readparam;
						readparam.Position_x = reader.readFloat();				//重力点X
						readparam.Position_y = reader.readFloat();				//重力点Y
						readparam.Power = reader.readFloat();					//パワー

						ParticlePointGravity *effectParam = new ParticlePointGravity();
						effectParam->setType((SsEffectFunctionType::enum_)type);				//コマンドの種類
						effectParam->Position.x = readparam.Position_x;				//重力点X
						effectParam->Position.y = readparam.Position_y;				//重力点Y
						effectParam->Power = readparam.Power;					//パワー

						behavior.plist.push_back(effectParam);												//パラメータを追加
						break;
					}
					case SsEffectFunctionType::TurnToDirectionEnabled:
					{
						//進行方向に向ける
						EffectParticleTurnToDirectionEnabled readparam;
						readparam.Rotation = reader.readFloat();					//フラグ

						ParticleTurnToDirectionEnabled *effectParam = new ParticleTurnToDirectionEnabled();
						effectParam->Rotation = readparam.Rotation;
						effectParam->setType((SsEffectFunctionType::enum_)type);				//コマンドの種類

						behavior.plist.push_back(effectParam);												//パラメータを追加
						break;
					}
					case SsEffectFunctionType::InfiniteEmitEnabled:
					{
						EffectParticleInfiniteEmitEnabled readparam;
						readparam.flag = reader.readS32();					//フラグ

						ParticleInfiniteEmitEnabled *effectParam = new ParticleInfiniteEmitEnabled();
						effectParam->setType((SsEffectFunctionType::enum_)type);				//コマンドの種類

						behavior.plist.push_back(effectParam);												//パラメータを追加
						break;
					}
					default:
						break;
					}
				}
				node->behavior = behavior;
				effectmodel->nodeList.push_back(node);
				if (nodeindex == 0)
				{
				}
			}
			//ツリーの構築
			if (effectmodel->nodeList.size() > 0)
			{
				effectmodel->root = effectmodel->nodeList[0];	//rootノードを追加
				for (size_t i = 1; i < effectmodel->nodeList.size(); i++)
				{
					int pi = effectmodel->nodeList[i]->parentIndex;
					if (pi >= 0)
					{
						effectmodel->nodeList[pi]->addChildEnd(effectmodel->nodeList[i]);
					}
				}
			}
			effectmodel->lockRandSeed = effectFile->lockRandSeed; 	 // ランダムシード固定値
			effectmodel->isLockRandSeed = effectFile->isLockRandSeed;  // ランダムシードを固定するか否か
			effectmodel->fps = effectFile->fps;             //
			effectmodel->effectName = effectFileName;
			effectmodel->layoutScaleX = effectFile->layoutScaleX;	//レイアウトスケールX
			effectmodel->layoutScaleY = effectFile->layoutScaleY;	//レイアウトスケールY



			SSLOG("effect key: %s", effectFileName.c_str());
			_dic.insert(std::map<std::string, SsEffectModel*>::value_type(effectFileName, effectmodel));
		}
	}
	//エフェクトファイル情報の削除
	void releseReference(void)
	{
		std::map<std::string, SsEffectModel*>::iterator it = _dic.begin();
		while (it != _dic.end())
		{
			SsEffectModel* effectmodel = it->second;

			if (effectmodel)
			{
				for (int nodeindex = 0; nodeindex < (int)effectmodel->nodeList.size(); nodeindex++)
				{
					SsEffectNode* node = effectmodel->nodeList.at(nodeindex);
					for (int behaviorindex = 0; behaviorindex < (int)node->behavior.plist.size(); behaviorindex++)
					{
						SsEffectElementBase* eb = node->behavior.plist.at(behaviorindex);
						delete eb;
					}
					node->behavior.plist.clear();
				}
				if (effectmodel->nodeList.size() > 0)
				{
					SsEffectNode* node = effectmodel->nodeList.at(0);
					delete node;
					effectmodel->nodeList.clear();
				}
				effectmodel->root = 0;

			}
			delete effectmodel;
			it++;
		}
		_dic.clear();
	}
protected:
	std::map<std::string, SsEffectModel*>		_dic;
};



/**
 * AnimeRef
 */
struct AnimeRef
{
	std::string				packName;
	std::string				animeName;
	const AnimationData*	animationData;
	const AnimePackData*	animePackData;
};


/**
 * AnimeCache
 */
class AnimeCache
{
public:
	AnimeCache()
	{
	}
	~AnimeCache()
	{
		releseReference();
	}
	static AnimeCache* create(const ProjectData* data)
	{
		AnimeCache* obj = new AnimeCache();
		if (obj)
		{
			obj->init(data);
		}
		return obj;
	}

	/**
	 * packNameとanimeNameを指定してAnimeRefを得る
	 */
	AnimeRef* getReference(const std::string& packName, const std::string& animeName)
	{
		std::string key = toPackAnimeKey(packName, animeName);
		AnimeRef* ref = _dic.at(key);
		return ref;
	}

	/**
	 * animeNameのみ指定してAnimeRefを得る
	 */
	AnimeRef* getReference(const std::string& animeName)
	{
		AnimeRef* ref = _dic.at(animeName);
		return ref;
	}
	
	void dump()
	{
		std::map<std::string, AnimeRef*>::iterator it = _dic.begin();
		while (it != _dic.end())
		{
			SSLOG("%s", (*it).second);
			++it;
		}
	}

protected:
	void init(const ProjectData* data)
	{
		SS_ASSERT2(data != NULL, "Invalid data");
		
		ToPointer ptr(data);
		const AnimePackData* animePacks = static_cast<const AnimePackData*>(ptr(data->animePacks));

		for (int packIndex = 0; packIndex < data->numAnimePacks; packIndex++)
		{
			const AnimePackData* pack = &animePacks[packIndex];
			const AnimationData* animations = static_cast<const AnimationData*>(ptr(pack->animations));
			const char* packName = static_cast<const char*>(ptr(pack->name));
			
			for (int animeIndex = 0; animeIndex < pack->numAnimations; animeIndex++)
			{
				const AnimationData* anime = &animations[animeIndex];
				const char* animeName = static_cast<const char*>(ptr(anime->name));
				
				AnimeRef* ref = new AnimeRef();
				ref->packName = packName;
				ref->animeName = animeName;
				ref->animationData = anime;
				ref->animePackData = pack;

				// packName + animeNameでの登録
				std::string key = toPackAnimeKey(packName, animeName);
				SSLOG("anime key: %s", key.c_str());
				_dic.insert(std::map<std::string, AnimeRef*>::value_type(key, ref));

				// animeNameのみでの登録
//				_dic.insert(std::map<std::string, AnimeRef*>::value_type(animeName, ref));
				
			}
		}
	}

	static std::string toPackAnimeKey(const std::string& packName, const std::string& animeName)
	{
		return Format("%s/%s", packName.c_str(), animeName.c_str());
	}

	//キャッシュの削除
	void releseReference(void)
	{
		std::map<std::string, AnimeRef*>::iterator it = _dic.begin();
		while (it != _dic.end())
		{
			AnimeRef* ref = it->second;
			if (ref)
			{
				delete ref;
				it->second = 0;
			}
			it++;
		}
		_dic.clear();
	}

protected:

public:
	std::map<std::string, AnimeRef*>	_dic;
};





/**
 * ResourceSet
 */
struct ResourceSet
{
	const ProjectData* data;
	bool isDataAutoRelease;
	EffectCache* effectCache;
	CellCache* cellCache;
	AnimeCache* animeCache;

	virtual ~ResourceSet()
	{
		if (isDataAutoRelease)
		{
			delete data;
			data = NULL;
		}
		if (animeCache)
		{
			delete animeCache;
			animeCache = NULL;
		}
		if (cellCache)
		{
			delete cellCache;
			cellCache = NULL;
		}
		if (effectCache)
		{
			delete effectCache;
			effectCache = NULL;
		}
	}
};


/**
 * ResourceManager
 */

static ResourceManager* defaultInstance = NULL;
const std::string ResourceManager::s_null;

ResourceManager* ResourceManager::getInstance()
{
	if (!defaultInstance)
	{
		defaultInstance = ResourceManager::create();
	}
	return defaultInstance;
}

ResourceManager::ResourceManager(void)
{
}

ResourceManager::~ResourceManager()
{
	removeAllData();
}

ResourceManager* ResourceManager::create()
{
	ResourceManager* obj = new ResourceManager();
	return obj;
}

ResourceSet* ResourceManager::getData(const std::string& dataKey)
{
	ResourceSet* rs = _dataDic.at(dataKey);
	return rs;
}

std::vector<std::string> ResourceManager::getAnimeName(const std::string& dataKey)
{
	std::vector<std::string> animename;
	ResourceSet* rs = _dataDic.at(dataKey);
	//アニメーション名を取得してリストを返す
	std::map<std::string, ss::AnimeRef*>::iterator itpairstri = rs->animeCache->_dic.begin();
	while (1)
	{
		// イテレータは pair<const string, int> 型なので、
		std::string strKey = itpairstri->first;     // イテレータからキーが得られる。

		if (strKey.find("/") == std::string::npos)	//ssae名が含まれていない場合はスキップ
		{
			continue;
		}
		animename.push_back(strKey);
		itpairstri++;
		if (itpairstri == rs->animeCache->_dic.end())
		{
			break;
		}
	}
	return animename;
}

std::string ResourceManager::addData(const std::string& dataKey, const ProjectData* data, const std::string& imageBaseDir, const std::string& zipFilepath, bool imageZipLoad)
{
	SS_ASSERT2(data != NULL, "Invalid data");
	SS_ASSERT2(data->dataId == DATA_ID, "Not data id matched");
	SS_ASSERT2(data->version == DATA_VERSION, "Version number of data does not match");
	
	// imageBaseDirの指定がないときコンバート時に指定されたパスを使用する
	std::string baseDir = imageBaseDir;
	if (imageBaseDir == s_null && data->imageBaseDir)
	{
		ToPointer ptr(data);
		const char* dir = static_cast<const char*>(ptr(data->imageBaseDir));
		baseDir = dir;
	}

	//アニメはエフェクトを参照し、エフェクトはセルを参照するのでこの順番で生成する必要がある
	CellCache* cellCache = NULL;
	if (imageZipLoad == false)
	{
		//画像はZIPファイルの中身を使用するZIPファイル名は空白にする
		cellCache = CellCache::create(data, baseDir, "");
	}
	else
	{
		cellCache = CellCache::create(data, baseDir, zipFilepath);
	}

	EffectCache* effectCache = EffectCache::create(data, baseDir, cellCache);	//

	AnimeCache* animeCache = AnimeCache::create(data);

	ResourceSet* rs = new ResourceSet();
	rs->data = data;
	rs->isDataAutoRelease = false;
	rs->cellCache = cellCache;
	rs->animeCache = animeCache;
	rs->effectCache = effectCache;
	_dataDic.insert(std::map<std::string, ResourceSet*>::value_type(dataKey, rs));

	return dataKey;
}

std::string ResourceManager::addDataWithKey(const std::string& dataKey, const std::string& ssbpFilepath, const std::string& imageBaseDir, const std::string& zipFilepath, bool imageZipLoad)
{

	std::string fullpath = ssbpFilepath;

	unsigned long nSize = 0;
	void* loadData = SSFileOpen(fullpath.c_str(), "rb", &nSize, zipFilepath.c_str());
	if (loadData == NULL)
	{
		std::string msg = "Can't load project data > " + fullpath;
		SS_ASSERT2(loadData != NULL, msg.c_str());
	}
	
	const ProjectData* data = static_cast<const ProjectData*>(loadData);
	SS_ASSERT2(data->dataId == DATA_ID, "Not data id matched");
	SS_ASSERT2(data->version == DATA_VERSION, "Version number of data does not match");
	
	std::string baseDir = imageBaseDir;
	if (imageBaseDir == s_null)
	{
		// imageBaseDirの指定がないとき
		ToPointer ptr(data);
		std::string dir = static_cast<const char*>(ptr(data->imageBaseDir));
		if (dir != "")
		{
			// コンバート時に指定されたパスを使用する
			baseDir = dir;
		}
		else
		{
			// プロジェクトファイルと同じディレクトリを指定する
			std::string directory;
			std::string filename;
			splitPath(directory, filename, ssbpFilepath);
			baseDir = directory;
		}
		//SSLOG("imageBaseDir: %s", baseDir.c_str());
	}

	addData(dataKey, data, baseDir, zipFilepath, imageZipLoad);
	
	// リソースが破棄されるとき一緒にロードしたデータも破棄する
	ResourceSet* rs = getData(dataKey);
	SS_ASSERT2(rs != NULL, "");
	rs->isDataAutoRelease = true;
	
	return dataKey;
}

std::string ResourceManager::addData(const std::string& ssbpFilepath, const std::string& imageBaseDir, const std::string& zipFilepath, bool imageZipLoad)
{
	// ファイル名を取り出す
	std::string directory;
    std::string filename;
	splitPath(directory, filename, ssbpFilepath);
	
	// 拡張子を取る
	std::string dataKey = filename;
	size_t pos = filename.find_last_of(".");
    if (pos != std::string::npos)
    {
        dataKey = filename.substr(0, pos);
    }

	//登録されている名前か判定する
	std::map<std::string, ResourceSet*>::iterator it = _dataDic.find(dataKey);
	if (it != _dataDic.end())
	{
		//登録されている場合は処理を行わない
		std::string str = "";
		return str;
	}

	return addDataWithKey(dataKey, ssbpFilepath, imageBaseDir, zipFilepath, imageZipLoad);
}

void ResourceManager::removeData(const std::string& dataKey)
{
	ResourceSet* rs = getData(dataKey);

	//バイナリデータの削除
	delete rs;
	_dataDic.erase(dataKey);
}

void ResourceManager::removeAllData()
{
	//全リソースの解放
	while (!_dataDic.empty())
	{
		std::map<std::string, ResourceSet*>::iterator it = _dataDic.begin();
		std::string ssbpName = it->first;
		removeData(ssbpName);
	}
	_dataDic.clear();
}

//データ名、セル名を指定して、セルで使用しているテクスチャを変更する
bool ResourceManager::changeTexture(char* ssbpName, char* ssceName, long texture)
{
	bool rc = false;

	ResourceSet* rs = getData(ssbpName);
	rc = rs->cellCache->setCellRefTexture(rs->data, ssceName, texture);

	return(rc);
}

//指定したデータのテクスチャを破棄します
bool ResourceManager::releseTexture(char* ssbpName)
{

	ResourceSet* rs = getData(ssbpName);
	bool rc = rs->cellCache->releseTexture(rs->data);

	return(rc);
}

//アニメーションの開始フレーム数を取得する
int ResourceManager::getStartFrame(std::string ssbpName, std::string animeName)
{
	int rc = -1;

	ResourceSet* rs = getData(ssbpName);
	AnimeRef* animeRef = rs->animeCache->getReference(animeName);
	if (animeRef == NULL)
	{
		std::string msg = Format("Not found animation > anime=%s", animeName.c_str());
		SS_ASSERT2(animeRef != NULL, msg.c_str());
	}
	rc = animeRef->animationData->startFrames;

	return(rc);
}

//アニメーションの終了フレーム数を取得する
int ResourceManager::getEndFrame(std::string ssbpName, std::string animeName)
{
	int rc = -1;

	ResourceSet* rs = getData(ssbpName);
	AnimeRef* animeRef = rs->animeCache->getReference(animeName);
	if (animeRef == NULL)
	{
		std::string msg = Format("Not found animation > anime=%s", animeName.c_str());
		SS_ASSERT2(animeRef != NULL, msg.c_str());
	}
	rc = animeRef->animationData->endFrames;

	return(rc);
}

//アニメーションの総フレーム数を取得する
int ResourceManager::getTotalFrame(std::string ssbpName, std::string animeName)
{
	int rc = -1;

	ResourceSet* rs = getData(ssbpName);
	AnimeRef* animeRef = rs->animeCache->getReference(animeName);
	if (animeRef == NULL)
	{
		std::string msg = Format("Not found animation > anime=%s", animeName.c_str());
		SS_ASSERT2(animeRef != NULL, msg.c_str());
	}
	rc = animeRef->animationData->totalFrames;

	return(rc);
}

//ssbpファイルが登録されているかを調べる
bool ResourceManager::isDataKeyExists(const std::string& dataKey) {
	// 登録されている名前か判定する
	std::map<std::string, ResourceSet*>::iterator it = _dataDic.find(dataKey);
	if (it != _dataDic.end()) {
		//登録されている
		return true;
	}

	return false;
}


/**
 * Player
 */

static const std::string s_nullString;

Player::Player(void)
	: _resman(nullptr)
	, _currentRs(nullptr)
	, _currentAnimeRef(nullptr)
	, _frameSkipEnabled(true)
	, _playingFrame(0.0f)
	, _step(1.0f)
	, _loop(0)
	, _loopCount(0)
	, _isPlaying(false)
	, _isPausing(false)
	, _prevDrawFrameNo(-1)
	, _col_r(255)
	, _col_g(255)
	, _col_b(255)
	, _instanceOverWrite(false)
	, _motionBlendPlayer(nullptr)
	, _blendTime(0.0f)
	, _blendTimeMax(0.0f)
	, _startFrameOverWrite(-1)	//開始フレームの上書き設定
	, _endFrameOverWrite(-1)		//終了フレームの上書き設定
	, _seedOffset(0)
	, _maskFuncFlag(true)
	, _maskParentSetting(true)
	, _parentMatUse(false)					//プレイヤーが持つ継承されたマトリクスがあるか？
	, _userDataCallback(nullptr)
	, _playEndCallback(nullptr)
	, _playercontrol(nullptr)
	, _maskEnable(true)
{
	int i;
	for (i = 0; i < PART_VISIBLE_MAX; i++)
	{
		_partVisible[i] = true;
		_partIndex[i] = -1;
		_cellChange[i] = -1;
	}
	_state.init();

	IdentityMatrix(_parentMat);

}

Player::~Player()
{
	if (_motionBlendPlayer)
	{
		delete (_motionBlendPlayer);
		_motionBlendPlayer = NULL;
	}

	releaseParts();
	releaseData();
	releaseResourceManager();
	releaseAnime();

	_resman = nullptr;
	_currentRs = nullptr;
	_currentAnimeRef = nullptr;
	_playercontrol = nullptr;
}

Player* Player::create(ResourceManager* resman)
{
	Player* obj = new Player();
	if (obj && obj->init())
	{
		obj->setResourceManager(resman);
		return obj;
	}
	SS_SAFE_DELETE(obj);
	return NULL;
}

bool Player::init()
{
	return true;
}

void Player::releaseResourceManager()
{
}

void Player::setResourceManager(ResourceManager* resman)
{
	if (_resman) releaseResourceManager();
	
	if (!resman)
	{
		// nullのときはデフォルトを使用する
		resman = ResourceManager::getInstance();
	}
	
	_resman = resman;
}

int Player::getStartFrame() const
{
	if (_currentAnimeRef )
	{
		return(_currentAnimeRef->animationData->startFrames);
	}
	else
	{
		return(0);
	}

}
int Player::getEndFrame() const
{
	if (_currentAnimeRef)
	{
		return(_currentAnimeRef->animationData->endFrames);
	}
	else
	{
		return(0);
	}

}
int Player::getTotalFrame() const
{
	if (_currentAnimeRef)
	{
		return(_currentAnimeRef->animationData->totalFrames);
	}
	else
	{
		return(0);
	}

}

int Player::getFPS() const
{
	if (_currentAnimeRef)
	{
		return(_currentAnimeRef->animationData->fps);
	}
	else
	{
		return(0);
	}

}

int Player::getFrameNo() const
{
	return static_cast<int>(_playingFrame);
}

void Player::setFrameNo(int frameNo)
{
	if (_currentAnimeRef)
	{
		_playingFrame = (float)frameNo;
		if (_playingFrame < _currentAnimeRef->animationData->startFrames )
		{
			_playingFrame = _currentAnimeRef->animationData->startFrames;
		}
		if (_playingFrame > _currentAnimeRef->animationData->endFrames)
		{
			_playingFrame = _currentAnimeRef->animationData->endFrames;
		}
	}
}

float Player::getStep() const
{
	return _step;
}

void Player::setStep(float step)
{
	_step = step;
}

int Player::getLoop() const
{
	return _loop;
}

void Player::setLoop(int loop)
{
	if (loop < 0) return;
	_loop = loop;
}

int Player::getLoopCount() const
{
	return _loopCount;
}

void Player::clearLoopCount()
{
	_loopCount = 0;
}

void Player::setFrameSkipEnabled(bool enabled)
{
	_frameSkipEnabled = enabled;
	_playingFrame = (float)((int)_playingFrame);
}

bool Player::isFrameSkipEnabled() const
{
	return _frameSkipEnabled;
}

void Player::setData(const std::string& dataKey)
{
	ResourceSet* rs = _resman->getData(dataKey);
	_currentdataKey = dataKey;
	if (rs == NULL)
	{
		std::string msg = Format("Not found data > %s", dataKey.c_str());
		SS_ASSERT2(rs != NULL, msg.c_str());
	}
	
	if (_currentRs != rs)
	{
//		releaseData();
		_currentRs = rs;
	}
}

std::string Player::getPlayDataName(void)
{
	return _currentdataKey;
}

void Player::releaseData()
{
	releaseAnime();
}


void Player::releaseAnime()
{
	releaseParts();
}

void Player::play(const std::string& ssaeName, const std::string& motionName, int loop, int startFrameNo)
{
	std::string animeName = Format("%s/%s", ssaeName.c_str(), motionName.c_str());
	play(animeName, loop, startFrameNo);
}

void Player::play(const std::string& animeName, int loop, int startFrameNo)
{
	SS_ASSERT2(_currentRs != NULL, "Not select data");

	//アニメデータを変更した場合は変更したステータスをもどす
	int i;
	for (i = 0; i < PART_VISIBLE_MAX; i++)
	{
		_partVisible[i] = true;
		_partIndex[i] = -1;
		_cellChange[i] = -1;
	}

	AnimeRef* animeRef = _currentRs->animeCache->getReference(animeName);
	if (animeRef == NULL)
	{
		std::string msg = Format("Not found animation > anime=%s", animeName.c_str());
		SS_ASSERT2(animeRef != NULL, msg.c_str());
	}
	_currentAnimename = animeName;

	play(animeRef, loop, startFrameNo);
}

void Player::play(AnimeRef* animeRef, int loop, int startFrameNo)
{
	if (_currentAnimeRef != animeRef)
	{
		_currentAnimeRef = animeRef;
		
		allocParts(animeRef->animePackData->numParts, false);
		setPartsParentage();
	}
	_playingFrame = static_cast<float>(startFrameNo);
	_step = 1.0f;
	_loop = loop;
	_loopCount = 0;
	_isPlaying = true;
	_isPausing = false;
	_prevDrawFrameNo = -1;
	_isPlayFirstUserdataChack = true;
	_animefps = _currentAnimeRef->animationData->fps;
	setStartFrame(-1);
	setEndFrame(-1);

	SSGetPlusDirection(_direction, _window_w, _window_h);

	setFrame((int)_playingFrame);
}

//モーションブレンドしつつ再生
void Player::motionBlendPlay(const std::string& animeName, int loop, int startFrameNo, float blendTime)
{
	if (_currentAnimename != "")
	{
		//現在のアニメーションをブレンド用プレイヤーで再生
		if (_motionBlendPlayer == NULL)
		{
			_motionBlendPlayer = ss::Player::create(_resman);
		}
		int loopnum = _loop;
		if (_loop > 0)
		{
			loopnum = _loop - _loopCount;
		}
		_motionBlendPlayer->setData(_currentdataKey);        // ssbpファイル名（拡張子不要）
		_motionBlendPlayer->play(_currentAnimename, loopnum, getFrameNo());
		_motionBlendPlayer->setStep(_step);
		if (_loop > 0)
		{
			if (_loop == _loopCount)	//アニメは最後まで終了している
			{
				_motionBlendPlayer->animePause();
			}
		}
		_blendTime = 0;
		_blendTimeMax = blendTime;

	}
	play(animeName, loop, startFrameNo);

}



void Player::animePause()
{
	_isPausing = true;
}

void Player::animeResume()
{
	_isPausing = false;
}

void Player::stop()
{
	_isPlaying = false;
}

const std::string& Player::getPlayPackName() const
{
	return _currentAnimeRef != NULL ? _currentAnimeRef->packName : s_nullString;
}

const std::string& Player::getPlayAnimeName() const
{
	return _currentAnimeRef != NULL ? _currentAnimeRef->animeName : s_nullString;
}


void Player::update(float dt)
{
	updateFrame(dt);
}

void Player::updateFrame(float dt)
{
	if (!_currentAnimeRef) return;
	if (!_currentRs->data) return;

	int startFrame = _currentAnimeRef->animationData->startFrames;
	int endFrame = _currentAnimeRef->animationData->endFrames;
	if (_startFrameOverWrite != -1)
	{
		startFrame = _startFrameOverWrite;
	}
	if (_endFrameOverWrite != -1 )
	{ 
		endFrame = _endFrameOverWrite;
	}
	SS_ASSERT2(startFrame <= endFrame, "Playframe is out of range.");

	// Setup を指定すると範囲外参照でクラッシュする対策 2020/04/06 endo
	if (endFrame + 1 > getTotalFrame())
	{
		endFrame = getTotalFrame() - 1;
	}

	bool playEnd = false;
	bool toNextFrame = _isPlaying && !_isPausing;
	if (toNextFrame && (_loop == 0 || _loopCount < _loop))
	{
		// フレームを進める.
		// forward frame.
		const int numFrames = endFrame;

		float fdt = dt;
		float s = fdt / (1.0f / _currentAnimeRef->animationData->fps);
		
		//if (!m_frameSkipEnabled) SSLOG("%f", s);
		
		float next = _playingFrame + (s * _step);

		int nextFrameNo = static_cast<int>(next);
		float nextFrameDecimal = next - static_cast<float>(nextFrameNo);
		int currentFrameNo = static_cast<int>(_playingFrame);

		//playを行って最初のupdateでは現在のフレームのユーザーデータを確認する
		if (_isPlayFirstUserdataChack == true)
		{
			checkUserData(currentFrameNo);
			_isPlayFirstUserdataChack = false;
		}

		if (_step >= 0)
		{
			// 順再生時.
			// normal plays.
			for (int c = nextFrameNo - currentFrameNo; c; c--)
			{
				int incFrameNo = currentFrameNo + 1;
				if (incFrameNo > numFrames)
				{
					// アニメが一巡
					// turned animation.
					_loopCount += 1;
					if (_loop && _loopCount >= _loop)
					{
						// 再生終了.
						// play end.
						playEnd = true;
						break;
					}
					
					incFrameNo = startFrame;
					_seedOffset++;	//シードオフセットを加算
				}
				currentFrameNo = incFrameNo;

				// このフレームのユーザーデータをチェック
				// check the user data of this frame.
				checkUserData(currentFrameNo);
			}
		}
		else
		{
			// 逆再生時.
			// reverse play.
			for (int c = currentFrameNo - nextFrameNo; c; c--)
			{
				int decFrameNo = currentFrameNo - 1;
				if (decFrameNo < startFrame)
				{
					// アニメが一巡
					// turned animation.
					_loopCount += 1;
					if (_loop && _loopCount >= _loop)
					{
						// 再生終了.
						// play end.
						playEnd = true;
						break;
					}
				
					decFrameNo = numFrames;
					_seedOffset++;	//シードオフセットを加算
				}
				currentFrameNo = decFrameNo;
				
				// このフレームのユーザーデータをチェック
				// check the user data of this frame.
				checkUserData(currentFrameNo);
			}
		}
		
		_playingFrame = static_cast<float>(currentFrameNo) + nextFrameDecimal;


	}
	else
	{
		//アニメを手動で更新する場合
		checkUserData(getFrameNo());
	}
	//モーションブレンド用アップデート
	if (_motionBlendPlayer)
	{
		_motionBlendPlayer->update(dt);
		_blendTime = _blendTime + dt;
		if (_blendTime >= _blendTimeMax)
		{
			_blendTime = _blendTimeMax;
			//プレイヤーを削除する
			delete (_motionBlendPlayer);
			_motionBlendPlayer = NULL;
		}
	}

	setFrame(getFrameNo(), dt);
	
	if (playEnd)
	{
		stop();
	
		// 再生終了コールバックの呼び出し
		if (_playEndCallback)
		{
			_playEndCallback(this);
		}
	}
}


void Player::allocParts(int numParts, bool useCustomShaderProgram)
{
	for (int i = 0; i < (int)_parts.size(); i++)
	{
		CustomSprite* sprite = _parts.at(i);
		if (sprite)
		{
			delete sprite;
			sprite = 0;
		}
	}

	_parts.clear();	//すべてのパーツを消す
	{
		// パーツ数だけCustomSpriteを作成する
//		// create CustomSprite objects.
		for (int i = 0; i < numParts; i++)
		{
			CustomSprite* sprite =  CustomSprite::create();
			sprite->_ssplayer = NULL;
			sprite->_parentPlayer = this;
			sprite->_playercontrol = _playercontrol;

			_parts.push_back(sprite);
		}
	}
}

void Player::releaseParts()
{
	// パーツの子CustomSpriteを全て削除
	// remove children CCSprite objects.
	if (_currentRs)
	{
		if (_currentAnimeRef)
		{

			ToPointer ptr(_currentRs->data);
			const AnimePackData* packData = _currentAnimeRef->animePackData;
			const PartData* parts = static_cast<const PartData*>(ptr(packData->parts));
			if (_parts.size() > 0)
			{
				for (int partIndex = 0; partIndex < packData->numParts; partIndex++)
				{
					CustomSprite* sprite = static_cast<CustomSprite*>(_parts.at(partIndex));
					SS_SAFE_DELETE(sprite->_ssplayer);
				}
			}
		}
	}

	for (auto&& i : _parts)
	{
		SS_SAFE_DELETE(i);
	}
	_parts.clear();
}

void Player::setPartsParentage()
{
	if (!_currentAnimeRef) return;

	ToPointer ptr(_currentRs->data);
	const AnimePackData* packData = _currentAnimeRef->animePackData;
	const PartData* parts = static_cast<const PartData*>(ptr(packData->parts));

	//親子関係を設定
	for (int partIndex = 0; partIndex < packData->numParts; partIndex++)
	{
		const PartData* partData = &parts[partIndex];
		CustomSprite* sprite = static_cast<CustomSprite*>(_parts.at(partIndex));
		
		sprite->_partData = *partData;

		if (partIndex > 0)
		{
			CustomSprite* parent = static_cast<CustomSprite*>(_parts.at(partData->parentIndex));
			sprite->_parent = parent;
		}
		else
		{
			sprite->_parent = NULL;
		}

		//インスタンスパーツの生成
		std::string refanimeName = static_cast<const char*>(ptr(partData->refname));

		sprite->_maskInfluence = partData->maskInfluence && _maskParentSetting;	//インスタンス時の親パーツを加味したマスク対象

		SS_SAFE_DELETE(sprite->_ssplayer);
		if (refanimeName != "")
		{
			//インスタンスパーツが設定されている
			sprite->_ssplayer = ss::Player::create(_resman);
			sprite->_ssplayer->_playercontrol = this->_playercontrol;
			sprite->_ssplayer->setMaskFuncFlag(false);
			sprite->_ssplayer->setMaskParentSetting(partData->maskInfluence);

			sprite->_ssplayer->setData(_currentdataKey);
			sprite->_ssplayer->play(refanimeName);				 // アニメーション名を指定(ssae名/アニメーション名も可能、詳しくは後述)
			sprite->_ssplayer->animePause();
		}

		//エフェクトパーツの生成
		if (sprite->refEffect)
		{
			delete sprite->refEffect;
			sprite->refEffect = 0;
		}

		std::string refeffectName = static_cast<const char*>(ptr(partData->effectfilename));
		if (refeffectName != "")
		{
			SsEffectModel* effectmodel = _currentRs->effectCache->getReference(refeffectName);
			if (effectmodel)
			{
				//エフェクトクラスにパラメータを設定する
				SsEffectRenderV2* er = new SsEffectRenderV2();
				sprite->refEffect = er;
				sprite->refEffect->setParentAnimeState(&sprite->partState);
				sprite->refEffect->setEffectData(effectmodel);
//				sprite->refEffect->setEffectSprite(&_effectSprite);	//エフェクトクラスに渡す都合上publicにしておく
//				sprite->refEffect->setEffectSpriteCount(&_effectSpriteCount);	//エフェクトクラスに渡す都合上publicにしておく
				sprite->refEffect->setSeed(getRandomSeed());
				sprite->refEffect->reload();
				sprite->refEffect->stop();
				sprite->refEffect->setLoop(false);
			}
		}

		if (partData->type == PARTTYPE_MESH)
		{
			//メッシュパーツ情報の取得
			ToPointer ptr(_currentRs->data);
			const AnimationData* animeData = _currentAnimeRef->animationData;

			{
				const ss_offset* meshsDataUV = static_cast<const ss_offset*>(ptr(animeData->meshsDataUV));
				const ss_u16* meashsDataUVArray = static_cast<const ss_u16*>(ptr(meshsDataUV[partIndex]));
				DataArrayReader reader(meashsDataUVArray);

				int isBind = reader.readU32();
				sprite->_meshIsBind = (bool)isBind;	//バインドされたメッシュか？
				int size = reader.readU32();
				sprite->_meshVertexSize = size;	//メッシュの頂点サイズ

												//メッシュ用バッファの作成
				sprite->_mesh_uvs = new float[2 * size];						// UVバッファ
//				sprite->_mesh_colors = new float[4 * size];						// カラーバッファ
				sprite->_mesh_colors = new unsigned char[4 * size];						// カラーバッファ
				sprite->_mesh_vertices = new float[3 * size];					// 座標バッファ

				int i;
				for (i = 0; i < size; i++)
				{
					float u = reader.readFloat();
					float v = reader.readFloat();
					SsVector2 uvs(u, v);
					sprite->_meshVertexUV.push_back(uvs);	//メッシュのUV

					sprite->_mesh_uvs[2 * i + 0] = u;						// UVバッファ
					sprite->_mesh_uvs[2 * i + 1] = v;						// UVバッファ

				}
			}

			{
				const ss_offset* meshsDataIndices = static_cast<const ss_offset*>(ptr(animeData->meshsDataIndices));
				const ss_u16* meshsDataIndicesArray = static_cast<const ss_u16*>(ptr(meshsDataIndices[partIndex]));
				DataArrayReader reader(meshsDataIndicesArray);

				int size = reader.readU32();
				sprite->_meshTriangleSize = size;

				//メッシュ用バッファの作成
				sprite->_mesh_indices = new unsigned short[3 * size];					// 座標バッファ

				int i;
				for (i = 0; i < size; i++)
				{
					unsigned short po1 = (unsigned short)reader.readS32();
					unsigned short po2 = (unsigned short)reader.readS32();
					unsigned short po3 = (unsigned short)reader.readS32();
					SsVector3 indices(po1, po2, po3);
					sprite->_meshIndices.push_back(indices);	//メッシュのUV

					sprite->_mesh_indices[3 * i + 0] = po1;						// UVバッファ
					sprite->_mesh_indices[3 * i + 1] = po2;						// UVバッファ
					sprite->_mesh_indices[3 * i + 2] = po3;						// UVバッファ

				}
			}
		}
	}
}

//再生しているアニメーションに含まれるパーツ数を取得
int Player::getPartsCount(void)
{
	ToPointer ptr(_currentRs->data);
	const AnimePackData* packData = _currentAnimeRef->animePackData;
	return packData->numParts;
}

//indexからパーツ名を取得
const char* Player::getPartName(int partId) const
{
	ToPointer ptr(_currentRs->data);

	const AnimePackData* packData = _currentAnimeRef->animePackData;
	SS_ASSERT2(partId >= 0 && partId < packData->numParts, "partId is out of range.");

	const PartData* partData = static_cast<const PartData*>(ptr(packData->parts));
	const char* name = static_cast<const char*>(ptr(partData[partId].name));
	return name;
}

//パーツ名からindexを取得
int Player::indexOfPart(const char* partName) const
{
	const AnimePackData* packData = _currentAnimeRef->animePackData;
	for (int i = 0; i < packData->numParts; i++)
	{
		const char* name = getPartName(i);
		if (strcmp(partName, name) == 0)
		{
			return i;
		}
	}
	return -1;
}

/*
 パーツ名から指定フレームのパーツステータスを取得します。
 必要に応じて　ResluteState　を編集しデータを取得してください。

 指定したフレームの状態にすべてのパーツのステータスを更新します。
 描画を行う前にupdateを呼び出し、パーツステータスを表示に状態に戻してからdrawしてください。
*/
bool Player::getPartState(ResluteState& result, const char* name, int frameNo)
{
	bool rc = false;
	if (_currentAnimeRef)
	{
		{
			//カレントフレームのパーツステータスを取得する
			if (frameNo == -1)
			{
				//フレームの指定が省略された場合は現在のフレームを使用する
				frameNo = getFrameNo();
			}

			if (frameNo != getFrameNo())
			{
				//取得する再生フレームのデータが違う場合プレイヤーを更新する
				//パーツステータスの更新
				setFrame(frameNo);
			}

			ToPointer ptr(_currentRs->data);

			const AnimePackData* packData = _currentAnimeRef->animePackData;
			const PartData* parts = static_cast<const PartData*>(ptr(packData->parts));

			for (int index = 0; index < packData->numParts; index++)
			{
				int partIndex = _partIndex[index];

				const PartData* partData = &parts[partIndex];
				const char* partName = static_cast<const char*>(ptr(partData->name));
				if (strcmp(partName, name) == 0)
				{
					//必要に応じて取得するパラメータを追加してください。
					//当たり判定などのパーツに付属するフラグを取得する場合は　partData　のメンバを参照してください。
					//親から継承したスケールを反映させる場合はxスケールは_mat.m[0]、yスケールは_mat.m[5]をかけて使用してください。
					CustomSprite* sprite = static_cast<CustomSprite*>(_parts.at(partIndex));
					result.x = sprite->_state.mat[12];
					result.y = sprite->_state.mat[13];

					//パーツアトリビュート
//					sprite->_state;													//SpriteStudio上のアトリビュートの値は_stateから取得してください
					result.flags = sprite->_state.flags;							// このフレームで更新が行われるステータスのフラグ
					result.cellIndex = sprite->_state.cellIndex;					// パーツに割り当てられたセルの番号
					result.x = sprite->_state.mat[12];
					result.y = sprite->_state.mat[13];
					result.z = sprite->_state.z;
					result.pivotX = sprite->_state.pivotX;							// 原点Xオフセット＋セルに設定された原点オフセットX
					result.pivotY = sprite->_state.pivotY;							// 原点Yオフセット＋セルに設定された原点オフセットY
					result.rotationX = sprite->_state.rotationX;					// X回転（親子関係計算済）
					result.rotationY = sprite->_state.rotationY;					// Y回転（親子関係計算済）
					result.rotationZ = sprite->_state.rotationZ;					// Z回転（親子関係計算済）
					result.scaleX = sprite->_state.Calc_scaleX;						// Xスケール（親子関係計算済）
					result.scaleY = sprite->_state.Calc_scaleY;						// Yスケール（親子関係計算済）
					result.localscaleX = sprite->_state.localscaleX;				// Xローカルスケール
					result.localscaleY = sprite->_state.localscaleY;				// Yローカルスケール
					result.opacity = sprite->_state.Calc_opacity;					// 不透明度（0～255）（親子関係計算済）
					result.localopacity = sprite->_state.localopacity;				// ローカル不透明度（0～255）
					result.size_X = sprite->_state.size_X;							// SS6アトリビュート：Xサイズ
					result.size_Y = sprite->_state.size_Y;							// SS6アトリビュート：Xサイズ
					result.uv_move_X = sprite->_state.uv_move_X;					// SS6アトリビュート：UV X移動
					result.uv_move_Y = sprite->_state.uv_move_Y;					// SS6アトリビュート：UV Y移動
					result.uv_rotation = sprite->_state.uv_rotation;				// SS6アトリビュート：UV 回転
					result.uv_scale_X = sprite->_state.uv_scale_X;					// SS6アトリビュート：UV Xスケール
					result.uv_scale_Y = sprite->_state.uv_scale_Y;					// SS6アトリビュート：UV Yスケール
					result.boundingRadius = sprite->_state.boundingRadius;			// SS6アトリビュート：当たり半径
					result.priority = sprite->_state.priority;						// SS6アトリビュート：優先度
					result.partsColorFunc = sprite->_state.partsColorFunc;			// SS6アトリビュート：カラーブレンドのブレンド方法
					result.partsColorType = sprite->_state.partsColorType;			// SS6アトリビュート：カラーブレンドの単色か頂点カラーか。
					result.flipX = sprite->_state.flipX;							// 横反転（親子関係計算済）
					result.flipY = sprite->_state.flipY;							// 縦反転（親子関係計算済）
					result.isVisibled = sprite->_state.isVisibled;					// 非表示（親子関係計算済）

					//パーツ設定
					result.part_type = partData->type;								//パーツ種別
					result.part_boundsType = partData->boundsType;					//当たり判定種類
					result.part_alphaBlendType = partData->alphaBlendType;			// BlendType
					//ラベルカラー
					std::string colorName = static_cast<const char*>(ptr(partData->colorLabel));
					if (colorName == COLORLABELSTR_NONE)
					{
						result.part_labelcolor = COLORLABEL_NONE;
					}
					if (colorName == COLORLABELSTR_RED)
					{
						result.part_labelcolor = COLORLABEL_RED;
					}
					if (colorName == COLORLABELSTR_ORANGE)
					{
						result.part_labelcolor = COLORLABEL_ORANGE;
					}
					if (colorName == COLORLABELSTR_YELLOW)
					{
						result.part_labelcolor = COLORLABEL_YELLOW;
					}
					if (colorName == COLORLABELSTR_GREEN)
					{
						result.part_labelcolor = COLORLABEL_GREEN;
					}
					if (colorName == COLORLABELSTR_BLUE)
					{
						result.part_labelcolor = COLORLABEL_BLUE;
					}
					if (colorName == COLORLABELSTR_VIOLET)
					{
						result.part_labelcolor = COLORLABEL_VIOLET;
					}
					if (colorName == COLORLABELSTR_GRAY)
					{
						result.part_labelcolor = COLORLABEL_GRAY;
					}

					rc = true;
					break;
				}
			}
			//パーツステータスを表示するフレームの内容で更新
			if (frameNo != getFrameNo())
			{
				//取得する再生フレームのデータが違う場合プレイヤーの状態をもとに戻す
				//パーツステータスの更新
				setFrame(getFrameNo());
			}
		}
	}
	return rc;
}


//ラベル名からラベルの設定されているフレームを取得
//ラベルが存在しない場合は戻り値が-1となります。
//ラベル名が全角でついていると取得に失敗します。
int Player::getLabelToFrame(char* findLabelName)
{
	int rc = -1;

	ToPointer ptr(_currentRs->data);
	const AnimationData* animeData = _currentAnimeRef->animationData;

	if (!animeData->labelData) return -1;
	const ss_offset* labelDataIndex = static_cast<const ss_offset*>(ptr(animeData->labelData));


	int idx = 0;
	for (idx = 0; idx < animeData->labelNum; idx++ )
	{
		if (!labelDataIndex[idx]) return -1;
		const ss_u16* labelDataArray = static_cast<const ss_u16*>(ptr(labelDataIndex[idx]));

		DataArrayReader reader(labelDataArray);

		LabelData ldata;
		ss_offset offset = reader.readOffset();
		const char* str = static_cast<const char*>(ptr(offset));
		int labelFrame = reader.readU16();
		ldata.str = str;
		ldata.frameNo = labelFrame;

		if (ldata.str.compare(findLabelName) == 0 )
		{
			//同じ名前のラベルが見つかった
			return (ldata.frameNo);
		}
	}

	return (rc);
}

//特定パーツの表示、非表示を設定します
//パーツ番号はスプライトスタジオのフレームコントロールに配置されたパーツが
//プライオリティでソートされた後、上に配置された順にソートされて決定されます。
void Player::setPartVisible(std::string partsname, bool flg)
{
	if (_currentAnimeRef)
	{
		ToPointer ptr(_currentRs->data);

		const AnimePackData* packData = _currentAnimeRef->animePackData;
		const PartData* parts = static_cast<const PartData*>(ptr(packData->parts));

		for (int index = 0; index < packData->numParts; index++)
		{
			int partIndex = _partIndex[index];

			const PartData* partData = &parts[partIndex];
			const char* partName = static_cast<const char*>(ptr(partData->name));
			if (strcmp(partName, partsname.c_str()) == 0)
			{
				_partVisible[partIndex] = flg;
				break;
			}
		}
	}
}

//パーツに割り当たるセルを変更します
void Player::setPartCell(std::string partsname, std::string sscename, std::string cellname)
{
	if (_currentAnimeRef)
	{
		ToPointer ptr(_currentRs->data);

		int changeCellIndex = -1;
		if ((sscename != "") && (cellname != ""))
		{
			//セルマップIDを取得する
			const Cell* cells = static_cast<const Cell*>(ptr(_currentRs->data->cells));

			//名前からインデックスの取得
			for (int i = 0; i < _currentRs->data->numCells; i++)
			{
				const Cell* cell = &cells[i];
				const char* name1 = static_cast<const char*>(ptr(cell->name));
				const CellMap* cellMap = static_cast<const CellMap*>(ptr(cell->cellMap));
				const char* name2 = static_cast<const char*>(ptr(cellMap->name));
				if (strcmp(cellname.c_str(), name1) == 0)
				{
					if (strcmp(sscename.c_str(), name2) == 0)
					{
						changeCellIndex = i;
						break;
					}
				}
			}
		}

		const AnimePackData* packData = _currentAnimeRef->animePackData;
		const PartData* parts = static_cast<const PartData*>(ptr(packData->parts));

		for (int index = 0; index < packData->numParts; index++)
		{
			int partIndex = _partIndex[index];

			const PartData* partData = &parts[partIndex];
			const char* partName = static_cast<const char*>(ptr(partData->name));
			if (strcmp(partName, partsname.c_str()) == 0)
			{
				//セル番号を設定
				_cellChange[partIndex] = changeCellIndex;	//上書き解除
				break;
			}
		}
	}
}

// インスタンスパーツが再生するアニメを変更します。
bool Player::changeInstanceAnime(std::string partsname, std::string animename, bool overWrite, Instance keyParam)
{
	//名前からパーツを取得
	bool rc = false;
	if (_currentAnimeRef)
	{
		ToPointer ptr(_currentRs->data);

		const AnimePackData* packData = _currentAnimeRef->animePackData;
		const PartData* parts = static_cast<const PartData*>(ptr(packData->parts));

		for (int index = 0; index < packData->numParts; index++)
		{
			int partIndex = _partIndex[index];

			const PartData* partData = &parts[partIndex];
			const char* partName = static_cast<const char*>(ptr(partData->name));
			if (strcmp(partName, partsname.c_str()) == 0)
			{
				CustomSprite* sprite = static_cast<CustomSprite*>(_parts.at(partIndex));
				if (sprite->_ssplayer)
				{
					//パーツがインスタンスパーツの場合は再生するアニメを設定する
					//アニメが入れ子にならないようにチェックする
					if (_currentAnimename != animename)
					{
						sprite->_ssplayer->play(animename);
						sprite->_ssplayer->setInstanceParam(overWrite, keyParam);	//インスタンスパラメータの設定
						sprite->_ssplayer->animeResume();		//アニメ切り替え時にがたつく問題の対応
						sprite->_liveFrame = 0;					//独立動作の場合再生位置をリセット
						rc = true;
					}
				}

				break;
			}
		}
	}

	return (rc);
}
//インスタンスパラメータを設定します
void Player::setInstanceParam(bool overWrite, Instance keyParam)
{
	_instanceOverWrite = overWrite;		//インスタンス情報を上書きするか？
	_instanseParam = keyParam;			//インスタンスパラメータ

}
//インスタンスパラメータを取得します
void Player::getInstanceParam(bool *overWrite, Instance *keyParam)
{
	*overWrite = _instanceOverWrite;		//インスタンス情報を上書きするか？
	*keyParam = _instanseParam;			//インスタンスパラメータ
}

//アニメーションの色成分を変更します
void Player::setColor(int r, int g, int b)
{
	_col_r = r;
	_col_g = g;
	_col_b = b;
}

//アニメーションのループ範囲を設定します
void Player::setStartFrame(int frame)
{
	_startFrameOverWrite = frame;	//開始フレームの上書き設定
	//現在フレームより後の場合は先頭フレームに設定する
	if (getFrameNo() < frame)
	{
		setFrameNo(frame);
	}
}
void Player::setEndFrame(int frame)
{
	_endFrameOverWrite = frame;		//終了フレームの上書き設定
}
//アニメーションのループ範囲をラベル名で設定します
void Player::setStartFrameToLabelName(char *findLabelName)
{
	int frame = getLabelToFrame(findLabelName);
	setStartFrame(frame);
}
void Player::setEndFrameToLabelName(char *findLabelName)
{
	int frame = getLabelToFrame(findLabelName);
	if (frame != -1)
	{
		frame += 1;
	}
	setEndFrame(frame);
}

//スプライト情報の取得
CustomSprite* Player::getSpriteData(int partIndex)
{
	CustomSprite* sprite = NULL;
	if ((int)_parts.size() < partIndex)
	{
	}
	else
	{
		sprite = static_cast<CustomSprite*>(_parts.at(partIndex));
	}
	return(sprite);
}

/*
* 表示を行うパーツ数を取得します
*/
int Player::getDrawSpriteCount(void)
{
	return (_draw_count);
}

void Player::setParentMatrix(float* mat, bool use )
{
	memcpy(_parentMat, mat, sizeof(float) * 16);	//
	_parentMatUse = use;					//プレイヤーが持つ継承されたマトリクスがあるか？

	if (_parentMatUse == true)
	{
		//プレイヤーが持つ継承されたマトリクスがある場合はこの時点でステータスを更新しておく
		memcpy(_state.mat, _parentMat, sizeof(float) * 16);
	}

}

void Player::setFrame(int frameNo, float dt)
{
	if (!_currentAnimeRef) return;
	if (!_currentRs->data) return;

	bool forceUpdate = false;
	{
		// フリップに変化があったときは必ず描画を更新する
		CustomSprite* root = static_cast<CustomSprite*>(_parts.at(0));
		float scaleX = root->isFlippedX() ? -1.0f : 1.0f;
		float scaleY = root->isFlippedY() ? -1.0f : 1.0f;
		root->setStateValue(root->_state.x, scaleX);
		root->setStateValue(root->_state.y, scaleY);
		forceUpdate = root->_isStateChanged;
	}
	
	// 前回の描画フレームと同じときはスキップ
	//インスタンスアニメがあるので毎フレーム更新するためコメントに変更
	//	if (!forceUpdate && frameNo == _prevDrawFrameNo) return;

	_maskIndexList.clear();

	ToPointer ptr(_currentRs->data);

	const AnimePackData* packData = _currentAnimeRef->animePackData;
	const PartData* parts = static_cast<const PartData*>(ptr(packData->parts));

	const AnimationData* animeData = _currentAnimeRef->animationData;
	const ss_offset* frameDataIndex = static_cast<const ss_offset*>(ptr(animeData->frameData));
	
	const ss_u16* frameDataArray = static_cast<const ss_u16*>(ptr(frameDataIndex[frameNo]));
	DataArrayReader reader(frameDataArray);
	
	const AnimationInitialData* initialDataList = static_cast<const AnimationInitialData*>(ptr(animeData->defaultData));


	State state;

	for (int index = 0; index < packData->numParts; index++)
	{
		int partIndex = reader.readS16();
		const PartData* partData = &parts[partIndex];
		const AnimationInitialData* init = &initialDataList[partIndex];

		// optional parameters
		int flags				= reader.readU32();
		int flags2				= reader.readU32();
		int cellIndex			= flags & PART_FLAG_CELL_INDEX ? reader.readS16() : init->cellIndex;
		float x					= flags & PART_FLAG_POSITION_X ? reader.readFloat() : init->positionX;
		float y					= flags & PART_FLAG_POSITION_Y ? reader.readFloat() : init->positionY;
		if (_direction == PLUS_DOWN)	//Y座標反転
		{
			y = -y;
		}
		float z					= flags & PART_FLAG_POSITION_Z ? reader.readFloat() : init->positionZ;
		float pivotX			= flags & PART_FLAG_PIVOT_X ? reader.readFloat() : init->pivotX;
		float pivotY			= flags & PART_FLAG_PIVOT_Y ? reader.readFloat() : init->pivotY;
		if (_direction == PLUS_DOWN)	//Y座標反転
		{
			pivotY = -pivotY;
		}
		float rotationX			= flags & PART_FLAG_ROTATIONX ? reader.readFloat() : init->rotationX;
		float rotationY			= flags & PART_FLAG_ROTATIONY ? reader.readFloat() : init->rotationY;
		float rotationZ			= flags & PART_FLAG_ROTATIONZ ? reader.readFloat() : init->rotationZ;
		if (_direction == PLUS_DOWN)	//Y座標反転
		{
			rotationX = -rotationX;
			rotationY = -rotationY;
			rotationZ = -rotationZ;
		}
		float scaleX			= flags & PART_FLAG_SCALE_X ? reader.readFloat() : init->scaleX;
		float scaleY			= flags & PART_FLAG_SCALE_Y ? reader.readFloat() : init->scaleY;
		float localscaleX		= flags & PART_FLAG_LOCALSCALE_X ? reader.readFloat() : init->localscaleX;
		float localscaleY		= flags & PART_FLAG_LOCALSCALE_Y ? reader.readFloat() : init->localscaleY;
		int opacity				= flags & PART_FLAG_OPACITY ? reader.readU16() : init->opacity;
		int localopacity		= flags & PART_FLAG_LOCALOPACITY ? reader.readU16() : init->localopacity;
		float size_X			= flags & PART_FLAG_SIZE_X ? reader.readFloat() : init->size_X;
		float size_Y			= flags & PART_FLAG_SIZE_Y ? reader.readFloat() : init->size_Y;
		float uv_move_X			= flags & PART_FLAG_U_MOVE ? reader.readFloat() : init->uv_move_X;
		float uv_move_Y			= flags & PART_FLAG_V_MOVE ? reader.readFloat() : init->uv_move_Y;
		float uv_rotation		= flags & PART_FLAG_UV_ROTATION ? reader.readFloat() : init->uv_rotation;
		float uv_scale_X		= flags & PART_FLAG_U_SCALE ? reader.readFloat() : init->uv_scale_X;
		float uv_scale_Y		= flags & PART_FLAG_V_SCALE ? reader.readFloat() : init->uv_scale_Y;
		float boundingRadius	= flags & PART_FLAG_BOUNDINGRADIUS ? reader.readFloat() : init->boundingRadius;
		float masklimen			= flags & PART_FLAG_MASK ? reader.readU16() : init->masklimen;
		float priority			= flags & PART_FLAG_PRIORITY ? reader.readU16() : init->priority;

		//インスタンスアトリビュート
		int		instanceValue_curKeyframe	= flags & PART_FLAG_INSTANCE_KEYFRAME ? reader.readS32() : init->instanceValue_curKeyframe;
		int		instanceValue_startFrame	= flags & PART_FLAG_INSTANCE_KEYFRAME ? reader.readS32() : init->instanceValue_startFrame;
		int		instanceValue_endFrame		= flags & PART_FLAG_INSTANCE_KEYFRAME ? reader.readS32() : init->instanceValue_endFrame;
		int		instanceValue_loopNum		= flags & PART_FLAG_INSTANCE_KEYFRAME ? reader.readS32() : init->instanceValue_loopNum;
		float	instanceValue_speed			= flags & PART_FLAG_INSTANCE_KEYFRAME ? reader.readFloat() : init->instanceValue_speed;
		int		instanceValue_loopflag		= flags & PART_FLAG_INSTANCE_KEYFRAME ? reader.readS32() : init->instanceValue_loopflag;
		//エフェクトアトリビュート
		int		effectValue_curKeyframe		= flags & PART_FLAG_EFFECT_KEYFRAME ? reader.readS32() : init->effectValue_curKeyframe;
		int		effectValue_startTime		= flags & PART_FLAG_EFFECT_KEYFRAME ? reader.readS32() : init->effectValue_startTime;
		float	effectValue_speed			= flags & PART_FLAG_EFFECT_KEYFRAME ? reader.readFloat() : init->effectValue_speed;
		int		effectValue_loopflag		= flags & PART_FLAG_EFFECT_KEYFRAME ? reader.readS32() : init->effectValue_loopflag;


		bool flipX = (bool)(flags & PART_FLAG_FLIP_H);
		bool flipY = (bool)(flags & PART_FLAG_FLIP_V);

		bool isVisibled = !(flags & PART_FLAG_INVISIBLE);

		if (_partVisible[partIndex] == false)
		{
			//ユーザーが任意に非表示としたパーツは非表示に設定
			isVisibled = false;
		}
		if (_cellChange[partIndex] != -1)
		{
			//ユーザーがセルを上書きした
			cellIndex = _cellChange[partIndex];
			CellRef* cellRef = cellIndex >= 0 ? _currentRs->cellCache->getReference(cellIndex) : nullptr;
			if (cellRef)
			{
				//サイズアトリビュートがない場合は差し替えたセルのサイズを設定しておく
				if ((flags & PART_FLAG_SIZE_X) == 0)
				{
					size_X = cellRef->rect.size.width;
				}
				if ((flags & PART_FLAG_SIZE_Y) == 0)
				{
					size_Y = cellRef->rect.size.height;
				}
			}
		}

		_partIndex[index] = partIndex;

		//セルの原点設定を反映させる
		CellRef* cellRef = cellIndex >= 0 ? _currentRs->cellCache->getReference(cellIndex) : nullptr;
		if (cellRef)
		{
			float cpx = 0;
			float cpy = 0;

			cpx = cellRef->cell->pivot_X;
			if (flipX) cpx = -cpx;	// 水平フリップによって原点を入れ替える
			cpy = cellRef->cell->pivot_Y;
			if (flipY) cpy = -cpy;	// 垂直フリップによって原点を入れ替える

			if (_direction == PLUS_DOWN)	//Y座標反転
			{
				cpy = -cpy;
			}

			pivotX += cpx;
			pivotY += cpy;

		}

		//モーションブレンド
		if (_motionBlendPlayer)
		{
			CustomSprite* blendSprite = _motionBlendPlayer->getSpriteData(partIndex);
			if (blendSprite)
			{ 
				float percent = _blendTime / _blendTimeMax;
				x = parcentVal(x, blendSprite->_orgState.x, percent);
				y = parcentVal(y, blendSprite->_orgState.y, percent);
				scaleX = parcentVal(scaleX, blendSprite->_orgState.scaleX, percent);
				scaleY = parcentVal(scaleY, blendSprite->_orgState.scaleY, percent);
				rotationX = parcentValRot(rotationX, blendSprite->_orgState.rotationX, percent);
				rotationY = parcentValRot(rotationY, blendSprite->_orgState.rotationY, percent);
				rotationZ = parcentValRot(rotationZ, blendSprite->_orgState.rotationZ, percent);
			}

		}

		//ステータス保存
		state.name = static_cast<const char*>(ptr(partData->name));
		state.flags = flags;
		state.flags2 = flags2;
		state.cellIndex = cellIndex;
		state.x = x;
		state.y = y;
		state.z = z;
		state.pivotX = pivotX;
		state.pivotY = pivotY;
		state.rotationX = rotationX;
		state.rotationY = rotationY;
		state.rotationZ = rotationZ;
		state.scaleX = scaleX;
		state.scaleY = scaleY;
		state.localscaleX = localscaleX;
		state.localscaleY = localscaleY;
		state.opacity = opacity;
		state.localopacity = localopacity;
		state.size_X = size_X;
		state.size_Y = size_Y;
		state.uv_move_X = uv_move_X;
		state.uv_move_Y = uv_move_Y;
		state.uv_rotation = uv_rotation;
		state.uv_scale_X = uv_scale_X;
		state.uv_scale_Y = uv_scale_Y;
		state.boundingRadius = boundingRadius;
		state.masklimen = masklimen;
		state.priority = priority;
		state.isVisibled = isVisibled;
		state.flipX = flipX;
		state.flipY = flipY;

		state.instanceValue_curKeyframe = instanceValue_curKeyframe;
		state.instanceValue_startFrame = instanceValue_startFrame;
		state.instanceValue_endFrame = instanceValue_endFrame;
		state.instanceValue_loopNum = instanceValue_loopNum;
		state.instanceValue_speed = instanceValue_speed;
		state.instanceValue_loopflag = instanceValue_loopflag;
		state.effectValue_curKeyframe = effectValue_curKeyframe;
		state.effectValue_startTime = effectValue_startTime;
		state.effectValue_speed = effectValue_speed;
		state.effectValue_loopflag = effectValue_loopflag;

		state.Calc_rotationX = state.rotationX;
		state.Calc_rotationY = state.rotationY;
		state.Calc_rotationZ = state.rotationZ;
		state.Calc_scaleX = state.scaleX;
		state.Calc_scaleY = state.scaleY;
		state.Calc_opacity = state.opacity;

		CustomSprite* sprite = static_cast<CustomSprite*>(_parts.at(partIndex));

		//反転
		//反転はUVにも反映させておくので使いやすい方で反転してください。
		sprite->setFlippedX(flipX);
		sprite->setFlippedY(flipY);

		if (cellRef)
		{
			//各パーツのテクスチャ情報を設定
			state.texture = cellRef->texture;
			state.rect = cellRef->rect;
			state.blendfunc = partData->alphaBlendType;
		}
		else
		{
			state.texture.handle = -1;
			//セルが無く通常パーツ、マスク、NULLパーツの時は非表示にする
			if ((partData->type == PARTTYPE_NORMAL) || (partData->type == PARTTYPE_MASK) || (partData->type == PARTTYPE_NULL))
			{
				state.isVisibled = false;
			}
		}
		sprite->setOpacity(opacity);

		//頂点データの設定
		//quadにはプリミティブの座標（頂点変形を含む）、UV、カラー値が設定されます。
		SSV3F_C4B_T2F_Quad quad;
		memset(&quad, 0, sizeof(quad));
		if (cellRef)
		{
			//頂点を設定する
			float width_h = cellRef->rect.size.width / 2;
			float height_h = cellRef->rect.size.height / 2;
			float x1 = -width_h;
			float y1 = -height_h;
			float x2 = width_h;
			float y2 = height_h;

			quad.tl.vertices.x = x1;
			quad.tl.vertices.y = y2;
			quad.tr.vertices.x = x2;
			quad.tr.vertices.y = y2;
			quad.bl.vertices.x = x1;
			quad.bl.vertices.y = y1;
			quad.br.vertices.x = x2;
			quad.br.vertices.y = y1;
			if (_direction == PLUS_DOWN)	//Y座標反転
			{
				quad.tl.vertices.x = x1;
				quad.tl.vertices.y = y1;
				quad.tr.vertices.x = x2;
				quad.tr.vertices.y = y1;
				quad.bl.vertices.x = x1;
				quad.bl.vertices.y = y2;
				quad.br.vertices.x = x2;
				quad.br.vertices.y = y2;
			}

			//UVを設定する
			quad.tl.texCoords.u = 0;
			quad.tl.texCoords.v = 0;
			quad.tr.texCoords.u = 0;
			quad.tr.texCoords.v = 0;
			quad.bl.texCoords.u = 0;
			quad.bl.texCoords.v = 0;
			quad.br.texCoords.u = 0;
			quad.br.texCoords.v = 0;
			if (cellRef)
			{
				quad.tl.texCoords.u = cellRef->cell->u1;
				quad.tl.texCoords.v = cellRef->cell->v1;
				quad.tr.texCoords.u = cellRef->cell->u2;
				quad.tr.texCoords.v = cellRef->cell->v1;
				quad.bl.texCoords.u = cellRef->cell->u1;
				quad.bl.texCoords.v = cellRef->cell->v2;
				quad.br.texCoords.u = cellRef->cell->u2;
				quad.br.texCoords.v = cellRef->cell->v2;
			}
		}

		//サイズ設定
		//頂点をサイズに合わせて変形させる
		if (flags & PART_FLAG_SIZE_X)
		{
			float w = 0;
			float center = 0;
			w = (quad.tr.vertices.x - quad.tl.vertices.x) / 2.0f;
			if (w!= 0.0f)
			{
				center = quad.tl.vertices.x + w;
				float scale = (size_X / 2.0f) / w;

				quad.bl.vertices.x = center - (w * scale);
				quad.br.vertices.x = center + (w * scale);
				quad.tl.vertices.x = center - (w * scale);
				quad.tr.vertices.x = center + (w * scale);
			}
		}
		if (flags & PART_FLAG_SIZE_Y)
		{
			float h = 0;
			float center = 0;
			h = (quad.bl.vertices.y - quad.tl.vertices.y) / 2.0f;
			if (h != 0.0f)
			{
				center = quad.tl.vertices.y + h;
				float scale = (size_Y / 2.0f) / h;

				quad.bl.vertices.y = center - (h * scale);
				quad.br.vertices.y = center - (h * scale);
				quad.tl.vertices.y = center + (h * scale);
				quad.tr.vertices.y = center + (h * scale);
			}
		}
		// 頂点変形のオフセット値を反映
		if (flags & PART_FLAG_VERTEX_TRANSFORM)
		{
			int vt_flags = reader.readU16();
			if (vt_flags & VERTEX_FLAG_LT)
			{
				quad.tl.vertices.x += reader.readFloat();
				quad.tl.vertices.y += reader.readFloat();
			}
			if (vt_flags & VERTEX_FLAG_RT)
			{
				quad.tr.vertices.x += reader.readFloat();
				quad.tr.vertices.y += reader.readFloat();
			}
			if (vt_flags & VERTEX_FLAG_LB)
			{
				quad.bl.vertices.x += reader.readFloat();
				quad.bl.vertices.y += reader.readFloat();
			}
			if (vt_flags & VERTEX_FLAG_RB)
			{
				quad.br.vertices.x += reader.readFloat();
				quad.br.vertices.y += reader.readFloat();
			}
		}
		
		//頂点情報の取得
		SSColor4B color4 = { 0xff, 0xff, 0xff, 0xff };

		color4.r = color4.r * _col_r / 255;
		color4.g = color4.g * _col_g / 255;
		color4.b = color4.b * _col_b / 255;

		quad.tl.colors =
		quad.tr.colors =
		quad.bl.colors =
		quad.br.colors = color4;


		// パーツカラーの反映
		if (flags & PART_FLAG_PARTS_COLOR)
		{
			int typeAndFlags = reader.readU16();
			int funcNo = typeAndFlags & 0xff;
			int cb_flags = (typeAndFlags >> 8) & 0xff;
			float blend_rate = 1.0f;

			state.partsColorFunc = funcNo;
			state.partsColorType = cb_flags;

			if (cb_flags & VERTEX_FLAG_ONE)
			{
				blend_rate = reader.readFloat();
				reader.readColor(color4);


				color4.r = color4.r * _col_r / 255;
				color4.g = color4.g * _col_g / 255;
				color4.b = color4.b * _col_b / 255;

				if ( funcNo == BLEND_MIX)
				{
					//ブレンド方法　MIX　かつ　単色の場合はAは255固定で処理する
					color4.a = 255;
				}

				quad.tl.colors =
				quad.tr.colors =
				quad.bl.colors =
				quad.br.colors = color4;

				state.rate.oneRate = blend_rate;
			}
			else
			{
				if (cb_flags & VERTEX_FLAG_LT)
				{
					blend_rate = reader.readFloat();
					reader.readColor(color4);
					quad.tl.colors = color4;

					state.rate.vartTLRate = blend_rate;
				}
				if (cb_flags & VERTEX_FLAG_RT)
				{
					blend_rate = reader.readFloat();
					reader.readColor(color4);
					quad.tr.colors = color4;

					state.rate.vartTRRate = blend_rate;
				}
				if (cb_flags & VERTEX_FLAG_LB)
				{
					blend_rate = reader.readFloat();
					reader.readColor(color4);
					quad.bl.colors = color4;

					state.rate.vartBLRate = blend_rate;
				}
				if (cb_flags & VERTEX_FLAG_RB)
				{
					blend_rate = reader.readFloat();
					reader.readColor(color4);
					quad.br.colors = color4;

					state.rate.vartBRRate = blend_rate;
				}
			}
		}

		//メッシュ情報
		if (flags2 & PART_FLAG_MESHDATA)
		{
			int i;
			for (i = 0; i < sprite->_meshVertexSize; i++)
			{
				float mesh_x = reader.readFloat();
				float mesh_y = reader.readFloat();
				float mesh_z = reader.readFloat();
				SsVector3 point(mesh_x, mesh_y, mesh_z);
				state.meshVertexPoint.push_back(point);

				sprite->_mesh_vertices[3 * i + 0] = mesh_x;					// 座標バッファ
				sprite->_mesh_vertices[3 * i + 1] = mesh_y;					// 座標バッファ
				sprite->_mesh_vertices[3 * i + 2] = mesh_z;					// 座標バッファ
			}
		}

		//UVアトリビュート処理
		if (
			   (flags & PART_FLAG_U_SCALE) || (flags & PART_FLAG_FLIP_H)
			|| (flags & PART_FLAG_V_SCALE) || (flags & PART_FLAG_FLIP_V)
			|| (flags & PART_FLAG_U_MOVE) || (flags & PART_FLAG_V_MOVE)
			|| (flags & PART_FLAG_UV_ROTATION)
			)
		{
			//アトリビュートからマトリクスを作成してUV座標を移動させる
			float u_code = 1;
			float v_code = 1;
			if (flags & PART_FLAG_FLIP_H)
			{
				//UV左右反転を行う場合は符号を逆にする
				u_code = -1;
			}
			if (flags & PART_FLAG_FLIP_V)
			{
				//UV上下反転を行う場合はテクスチャUVを逆にする
				v_code = -1;
			}

			float uvw = quad.br.texCoords.u + quad.tl.texCoords.u;
			float uvh = quad.br.texCoords.v + quad.tl.texCoords.v;
			uvw /= 2.0f;
			uvh /= 2.0f;

			SsVector2 uv_trans;
			uv_trans.x = uv_move_X;
			uv_trans.y = uv_move_Y;

			float mat[16];
			float t[16];
			IdentityMatrix(mat);
			TranslationMatrix(t, uvw + uv_trans.x, uvh + uv_trans.y, 0.0f);
			MultiplyMatrix(t, mat, mat);

			Matrix4RotationZ(t, SSRadianToDegree(uv_rotation));
			MultiplyMatrix(t, mat, mat);

			ScaleMatrix(t, uv_scale_X * u_code, uv_scale_Y * v_code, 1.0f);
			MultiplyMatrix(t, mat, mat);

			TranslationMatrix(t, -uvw, -uvh, 0.0f);
			MultiplyMatrix(t, mat, mat);

			//UV座標をマトリクスで変形させる
			TranslationMatrix(t, quad.tl.texCoords.u, quad.tl.texCoords.v, 0.0f);
			MultiplyMatrix(t, mat, t);			//プレイヤーのTRS	
			quad.tl.texCoords.u = t[12];
			quad.tl.texCoords.v = t[13];

			TranslationMatrix(t, quad.tr.texCoords.u, quad.tr.texCoords.v, 0.0f);
			MultiplyMatrix(t, mat, t);
			quad.tr.texCoords.u = t[12];
			quad.tr.texCoords.v = t[13];

			TranslationMatrix(t, quad.bl.texCoords.u, quad.bl.texCoords.v, 0.0f);
			MultiplyMatrix(t, mat, t);
			quad.bl.texCoords.u = t[12];
			quad.bl.texCoords.v = t[13];

			TranslationMatrix(t, quad.br.texCoords.u, quad.br.texCoords.v, 0.0f);
			MultiplyMatrix(t, mat, t);
			quad.br.texCoords.u = t[12];
			quad.br.texCoords.v = t[13];


		}
		state.quad = quad;




		//インスタンスパーツの場合
		if (partData->type == PARTTYPE_INSTANCE)
		{
			bool overWrite;
			Instance keyParam;
			sprite->_ssplayer->getInstanceParam(&overWrite, &keyParam);
			//描画
			int refKeyframe = instanceValue_curKeyframe;
			int refStartframe = instanceValue_startFrame;
			int refEndframe = instanceValue_endFrame;
			float refSpeed = instanceValue_speed;
			int refloopNum = instanceValue_loopNum;
			bool infinity = false;
			bool reverse = false;
			bool pingpong = false;
			bool independent = false;

			int lflags = instanceValue_loopflag;
			if (lflags & INSTANCE_LOOP_FLAG_INFINITY )
			{
				//無限ループ
				infinity = true;
			}
			if (lflags & INSTANCE_LOOP_FLAG_REVERSE)
			{
				//逆再生
				reverse = true;
			}
			if (lflags & INSTANCE_LOOP_FLAG_PINGPONG)
			{
				//往復
				pingpong = true;
			}
			if (lflags & INSTANCE_LOOP_FLAG_INDEPENDENT)
			{
				//独立
				independent = true;
			}
			//インスタンスパラメータを上書きする
			if (overWrite == true)
			{
				refStartframe = keyParam.refStartframe;		//開始フレーム
				refEndframe = keyParam.refEndframe;			//終了フレーム
				refSpeed = keyParam.refSpeed;				//再生速度
				refloopNum = keyParam.refloopNum;			//ループ回数
				infinity = keyParam.infinity;				//無限ループ
				reverse = keyParam.reverse;					//逆再選
				pingpong = keyParam.pingpong;				//往復
				independent = keyParam.independent;			//独立動作
			}

			//タイムライン上の時間 （絶対時間）
			int time = frameNo;

			//独立動作の場合
			if (independent)
			{
				float delta = dt / (1.0f / _animefps);						//	独立動作時は親アニメのfpsを使用する
//				float delta = fdt / (1.0f / sprite->_ssplayer->_animefps);

				sprite->_liveFrame += delta;
				time = (int)sprite->_liveFrame;
			}

			//このインスタンスが配置されたキーフレーム（絶対時間）
			int	selfTopKeyframe = refKeyframe;


			int	reftime = (int)((float)(time - selfTopKeyframe) * refSpeed); //開始から現在の経過時間
			if (reftime < 0) continue;							//そもそも生存時間に存在していない
			if (selfTopKeyframe > time) continue;

			int inst_scale = (refEndframe - refStartframe) + 1; //インスタンスの尺


			//尺が０もしくはマイナス（あり得ない
			if (inst_scale <= 0) continue;
			int	nowloop = (reftime / inst_scale);	//現在までのループ数

			int checkloopnum = refloopNum;

			//pingpongの場合では２倍にする
			if (pingpong) checkloopnum = checkloopnum * 2;

			//無限ループで無い時にループ数をチェック
			if (!infinity)   //無限フラグが有効な場合はチェックせず
			{
				if (nowloop >= checkloopnum)
				{
					reftime = inst_scale - 1;
					nowloop = checkloopnum - 1;
				}
			}

			int temp_frame = reftime % inst_scale;  //ループを加味しないインスタンスアニメ内のフレーム

			//参照位置を決める
			//現在の再生フレームの計算
			int _time = 0;
			if (pingpong && (nowloop % 2 == 1))
			{
				if (reverse)
				{
					reverse = false;//反転
				}
				else
				{
					reverse = true;//反転
				}
			}

			if (reverse)
			{
				//リバースの時
				_time = refEndframe - temp_frame;
			}
			else{
				//通常時
				_time = temp_frame + refStartframe;
			}

			//インスタンスパラメータを設定
			sprite->_ssplayer->setColor(_col_r, _col_g, _col_b);
//			sprite->_ssplayer->setPosition(_state.x, _state.y);
//			sprite->_ssplayer->setRotation(_state.rotationX, _state.rotationY, _state.rotationZ);
//			sprite->_ssplayer->setScale(_state.scaleX, _state.scaleY);

			//インスタンス用SSPlayerに再生フレームを設定する
			sprite->_ssplayer->setFrameNo(_time);
		}

		//スプライトステータスの保存
		sprite->setState(state);
		sprite->_orgState = sprite->_state;

		if (partData->type == PARTTYPE_MASK)
		{
			_maskIndexList.push_back(sprite);
		}
	}


	// 親に変更があるときは自分も更新するようフラグを設定する
	for (int partIndex = 1; partIndex < packData->numParts; partIndex++)
	{
		const PartData* partData = &parts[partIndex];
		CustomSprite* sprite = static_cast<CustomSprite*>(_parts.at(partIndex));
		CustomSprite* parent = static_cast<CustomSprite*>(_parts.at(partData->parentIndex));
		
		if (parent->_isStateChanged)
		{
			sprite->_isStateChanged = true;
		}
	}

	// 行列の更新
	float mat[16];
	float t[16];

	//プレイヤーのマトリクスを計算する
	//rootパーツはプレイヤーからステータスを引き継ぐ
	if (_parentMatUse == true)					//プレイヤーが持つ継承されたマトリクスがあるか？
	{
		memcpy(_state.mat, _parentMat, sizeof(float) * 16);
	}
	else
	{
		IdentityMatrix(mat);

		TranslationMatrix(t, _state.x, _state.y, 0.0f);
		MultiplyMatrix(t, mat, mat);

		Matrix4RotationX(t, SSRadianToDegree(_state.rotationX));
		MultiplyMatrix(t, mat, mat);

		Matrix4RotationY(t, SSRadianToDegree(_state.rotationY));
		MultiplyMatrix(t, mat, mat);

		Matrix4RotationZ(t, SSRadianToDegree(_state.rotationZ));
		MultiplyMatrix(t, mat, mat);

		float scale_x = _state.scaleX;
		float scale_y = _state.scaleY;
		if (_state.flipX == true)
		{
			scale_x = -scale_x;	//フラグ反転
		}
		if (_state.flipY == true)
		{
			scale_y = -scale_y;	//フラグ反転
		}
		ScaleMatrix(t, scale_x, scale_y, 1.0f);
		MultiplyMatrix(t, mat, mat);

		memcpy(_state.mat, mat, sizeof(float) * 16);	//プレイヤーのマトリクスを作成する
	}

	for (int partIndex = 0; partIndex < packData->numParts; partIndex++)
	{
		const PartData* partData = &parts[partIndex];
		CustomSprite* sprite = static_cast<CustomSprite*>(_parts.at(partIndex));

		if (sprite->_isStateChanged)
		{
			{
				int num = 1;
				if ((sprite->_state.flags & PART_FLAG_LOCALSCALE_X) || (sprite->_state.flags & PART_FLAG_LOCALSCALE_Y))
				{
					//ローカルスケール対応
					num = 2;
				}
				int matcnt;
				for (matcnt = 0; matcnt < num; matcnt++)
				{
					if (partIndex > 0)
					{
						//親のマトリクスを適用
						CustomSprite* parent = static_cast<CustomSprite*>(_parts.at(partData->parentIndex));
						memcpy(mat, parent->_mat, sizeof(float) * 16);
					}
					else
					{
						IdentityMatrix(mat);
/*
						sprite->_state.x += _state.x;
						sprite->_state.y += _state.y;
						sprite->_state.rotationX += _state.rotationX;
						sprite->_state.rotationY += _state.rotationY;
						sprite->_state.rotationZ += _state.rotationZ;
						sprite->_state.scaleX *= _state.scaleX;
						sprite->_state.scaleY *= _state.scaleY;
						//プレイヤーのフリップ
						if (_state.flipX == true)
						{
							sprite->_state.scaleX = -sprite->_state.scaleX;	//フラグ反転
						}
						if (_state.flipY == true)
						{
							sprite->_state.scaleY = -sprite->_state.scaleY;	//フラグ反転
						}
*/
						sprite->_state.Calc_rotationX = sprite->_state.rotationX;
						sprite->_state.Calc_rotationY = sprite->_state.rotationY;
						sprite->_state.Calc_rotationZ = sprite->_state.rotationZ;

						sprite->_state.Calc_scaleX = sprite->_state.scaleX;
						sprite->_state.Calc_scaleY = sprite->_state.scaleY;
					}
					TranslationMatrix(t, sprite->_state.x, sprite->_state.y, 0.0f);
					MultiplyMatrix(t, mat, mat);

					Matrix4RotationX(t, SSRadianToDegree(sprite->_state.rotationX));
					MultiplyMatrix(t, mat, mat);

					Matrix4RotationY(t, SSRadianToDegree(sprite->_state.rotationY));
					MultiplyMatrix(t, mat, mat);

					Matrix4RotationZ(t, SSRadianToDegree(sprite->_state.rotationZ));
					MultiplyMatrix(t, mat, mat);

					float sx = sprite->_state.scaleX;
					float sy = sprite->_state.scaleY;
					if (matcnt > 0)
					{
						//ローカルスケールを適用する
						sx *= sprite->_state.localscaleX;
						sy *= sprite->_state.localscaleY;
					}
					ScaleMatrix(t, sx, sy, 1.0f);
					MultiplyMatrix(t, mat, mat);

					if (matcnt > 0)
					{
						memcpy(sprite->_localmat, mat, sizeof(float) * 16);	//ローカルマトリクスを作成する
					}
					else
					{
						memcpy(sprite->_mat, mat, sizeof(float) * 16);		//継承マトリクスを作成する
					}
				}

				if (num == 1)
				{
					//ローカルスケールが使用されていない場合は継承マトリクスをローカルマトリクスに適用
					memcpy(sprite->_localmat, mat, sizeof(float) * 16);
				}

				memcpy(sprite->_state.mat, sprite->_localmat, sizeof(float) * 16);	//表示にはローカルマトリクスを適用する

				if (partIndex > 0)
				{
					CustomSprite* parent = static_cast<CustomSprite*>(_parts.at(partData->parentIndex));
					//子供は親のステータスを引き継ぐ
					//座標はマトリクスから取得する
					if ((parent->_state.Calc_scaleX * parent->_state.Calc_scaleY) < 0)	//スケールのどちらかが-の場合は回転方向を逆にする
					{
						sprite->_state.Calc_rotationZ = -sprite->_state.Calc_rotationZ;
					}
					sprite->_state.Calc_rotationX += parent->_state.Calc_rotationX;
					sprite->_state.Calc_rotationY += parent->_state.Calc_rotationY;
					sprite->_state.Calc_rotationZ += parent->_state.Calc_rotationZ;

					sprite->_state.Calc_scaleX *= parent->_state.Calc_scaleX;
					sprite->_state.Calc_scaleY *= parent->_state.Calc_scaleY;

					//ルートパーツのアルファ値を反映させる
					sprite->_state.Calc_opacity = (sprite->_state.Calc_opacity * _state.opacity) / 255;
					//インスタンスパーツの親を設定
					if (sprite->_ssplayer)
					{
						IdentityMatrix(t);
						MultiplyMatrix(sprite->_state.mat, _state.mat, t);

						sprite->_ssplayer->setParentMatrix( t, true );	//プレイヤーに対してマトリクスを設定する

						float alpha = sprite->_state.Calc_opacity;
						if (sprite->_state.flags & PART_FLAG_LOCALOPACITY)
						{
							alpha = sprite->_state.localopacity;	//ローカル不透明度対応
						}
						sprite->_ssplayer->setAlpha(alpha);
					}

				}
			}
			sprite->_isStateChanged = false;
		}
	}

	// 特殊パーツのアップデート
	for (int partIndex = 0; partIndex < packData->numParts; partIndex++)
	{
		const PartData* partData = &parts[partIndex];
		CustomSprite* sprite = static_cast<CustomSprite*>(_parts.at(partIndex));


		//インスタンスパーツのアップデート
		if (sprite->_ssplayer)
		{
			sprite->_ssplayer->setMaskFunctionUse(_maskEnable);	//マスクの有無を設定する
			sprite->_ssplayer->update(dt);
		}
		//エフェクトのアップデート
		if (sprite->refEffect)
		{
			sprite->refEffect->setParentSprite(sprite);

			//エフェクトアトリビュート
			int curKeyframe = sprite->_state.effectValue_curKeyframe;
			int refStartframe = sprite->_state.effectValue_startTime;
			float refSpeed = sprite->_state.effectValue_speed;
			bool independent = false;

			int lflags = sprite->_state.effectValue_loopflag;
			if (lflags & EFFECT_LOOP_FLAG_INDEPENDENT)
			{
				independent = true;
			}

			if (sprite->effectAttrInitialized == false)
			{
				sprite->effectAttrInitialized = true;
				sprite->effectTimeTotal = refStartframe;
			}

			sprite->refEffect->setParentSprite(sprite);	//親スプライトの設定
			if (sprite->_state.isVisibled == true)
			{

				if (independent)
				{
					//独立動作
					if (sprite->effectAttrInitialized)
					{
						float delta = dt / (1.0f / _animefps);						//	独立動作時は親アニメのfpsを使用する
						sprite->effectTimeTotal += delta * refSpeed;
						sprite->refEffect->setLoop(true);
						sprite->refEffect->setFrame(sprite->effectTimeTotal);
						sprite->refEffect->play();
						sprite->refEffect->update();
					}
				}
				else 
				{
					{
						float _time = frameNo - curKeyframe;
						if (_time < 0)
						{
						}
						else
						{
							_time *= refSpeed;
							_time = _time + refStartframe;
							sprite->effectTimeTotal = _time;

							sprite->refEffect->setSeedOffset(_seedOffset);
							sprite->refEffect->setFrame(_time);
							sprite->refEffect->play();
							sprite->refEffect->update();
						}
					}
				}
			}
		}
	}
	_prevDrawFrameNo = frameNo;	//再生したフレームを保存
}

//プレイヤーの描画
void Player::draw()
{
	_draw_count = 0;

	if (!_currentAnimeRef) return;

	if (_maskFuncFlag == true) //マスク機能が有効（インスタンスのソースアニメではない）
	{
		SSRenderSetup();
		if (getMaskFunctionUse() == true)
		{
			clearMask();	//マスクの状態を初期化する
		}
	}

	ToPointer ptr(_currentRs->data);
	const AnimePackData* packData = _currentAnimeRef->animePackData;


	if (_maskFuncFlag == true) //マスク機能が有効（インスタンスのソースアニメではない）
	{
		//初期に適用されているマスクを精製
		for (size_t i = 0; i < _maskIndexList.size(); i++)
		{
			CustomSprite* sprite = _maskIndexList[i];

			if (sprite->_state.isVisibled == true)
			{
				//ステンシルバッファの作成
				SSDrawSprite(sprite);
				_draw_count++;
			}
		}
	}
	int mask_index = 0;

	for (int index = 0; index < packData->numParts; index++)
	{

		int partIndex = _partIndex[index];
		//スプライトの表示
		CustomSprite* sprite = static_cast<CustomSprite*>(_parts.at(partIndex));

		if (sprite->_partData.type == PARTTYPE_MASK)
		{
			//マスクパーツ

			//6.2対応
			//非表示の場合でもマスクの場合は処理をしなくてはならない
			//マスクはパーツの描画より先に奥のマスクパーツから順にマスクを作成していく必要があるため
			//通常パーツの描画順と同じ箇所で非表示によるスキップを行うとマスクのバッファがクリアされずに、
			//マスクが手前の優先度に影響するようになってしまう。
			if ((_maskFuncFlag == true) && (getMaskFunctionUse() == true)) //マスク機能が有効（インスタンスのソースアニメではない）
			{
				clearMask();
				mask_index++;	//0番は処理しないので先にインクメントする

				for (size_t i = mask_index; i < _maskIndexList.size(); i++)
				{
					CustomSprite* sprite2 = _maskIndexList[i];
					if (sprite2->_state.isVisibled == true)
					{
						SSDrawSprite(sprite2);
						_draw_count++;
					}
				}
			}
		}
		if (sprite->_state.isVisibled == true)
		{
			if (sprite->_ssplayer)
			{
				//インスタンスパーツの場合は子供のプレイヤーを再生
				sprite->_ssplayer->draw();
				_draw_count += sprite->_ssplayer->getDrawSpriteCount();
			}
			else
			{
				if (sprite->refEffect)
				{ 
					//エフェクトパーツ
					sprite->refEffect->draw();
					_draw_count = sprite->refEffect->getDrawSpriteCount();
				}
				else if (sprite->_partData.type != PARTTYPE_MASK) 
				{
					//通常パーツ
					if (sprite->_state.texture.handle != -1)
					{
						SSDrawSprite(sprite);
						_draw_count++;
					}
				}
			}
		}
	}
	if (_maskFuncFlag == true) //マスク機能が有効（インスタンスのソースアニメではない）
	{
		if (getMaskFunctionUse() == true)
		{
			enableMask(false);
		}
		SSRenderEnd();
	}

}

void Player::checkUserData(int frameNo)
{
	if (!_userDataCallback) return;

	ToPointer ptr(_currentRs->data);

	const AnimePackData* packData = _currentAnimeRef->animePackData;
	const AnimationData* animeData = _currentAnimeRef->animationData;
	const PartData* parts = static_cast<const PartData*>(ptr(packData->parts));

	if (!animeData->userData) return;
	const ss_offset* userDataIndex = static_cast<const ss_offset*>(ptr(animeData->userData));

	if (!userDataIndex[frameNo]) return;
	const ss_u16* userDataArray = static_cast<const ss_u16*>(ptr(userDataIndex[frameNo]));
	
	DataArrayReader reader(userDataArray);
	int numUserData = reader.readU16();

	for (int i = 0; i < numUserData; i++)
	{
		int flags = reader.readU16();
		int partIndex = reader.readU16();

		_userData.flags = 0;

		if (flags & UserData::FLAG_INTEGER)
		{
			_userData.flags |= UserData::FLAG_INTEGER;
			_userData.integer = reader.readS32();
		}
		else
		{
			_userData.integer = 0;
		}
		
		if (flags & UserData::FLAG_RECT)
		{
			_userData.flags |= UserData::FLAG_RECT;
			_userData.rect[0] = reader.readS32();
			_userData.rect[1] = reader.readS32();
			_userData.rect[2] = reader.readS32();
			_userData.rect[3] = reader.readS32();
		}
		else
		{
			_userData.rect[0] =
			_userData.rect[1] =
			_userData.rect[2] =
			_userData.rect[3] = 0;
		}
		
		if (flags & UserData::FLAG_POINT)
		{
			_userData.flags |= UserData::FLAG_POINT;
			_userData.point[0] = reader.readS32();
			_userData.point[1] = reader.readS32();
		}
		else
		{
			_userData.point[0] =
			_userData.point[1] = 0;
		}
		
		if (flags & UserData::FLAG_STRING)
		{
			_userData.flags |= UserData::FLAG_STRING;
			int size = reader.readU16();
			ss_offset offset = reader.readOffset();
			const char* str = static_cast<const char*>(ptr(offset));
			_userData.str = str;
			_userData.strSize = size;
		}
		else
		{
			_userData.str = 0;
			_userData.strSize = 0;
		}
		
		_userData.partName = static_cast<const char*>(ptr(parts[partIndex].name));
		_userData.frameNo = frameNo;
		
		_userDataCallback(this, &_userData);
	}

}

void  Player::setPosition(float x, float y)
{
	_state.x = x;
	_state.y = y;
}
void  Player::setRotation(float x, float y, float z)
{
	_state.rotationX = x;
	_state.rotationY = y;
	_state.rotationZ = z;
}

void  Player::setScale(float x, float y)
{
	_state.scaleX = x;
	_state.scaleY = y;
}

void  Player::setAlpha(int a)
{
	_state.opacity = a;
}

void  Player::setFlip(bool flipX, bool flipY)
{
	_state.flipX = flipX;
	_state.flipY = flipY;
}

void  Player::setMaskFunctionUse(bool flg)
{
	_maskEnable = flg;
}


//割合に応じた中間値を取得します
float Player::parcentVal(float val1, float val2, float parcent)
{
	float sa = val1 - val2;
	float newval = val2 + (sa * parcent);
	return (newval);
}
float Player::parcentValRot(float val1, float val2, float parcent)
{
	int ival1 = (int)(val1 * 10.0f) % 3600;
	int ival2 = (int)(val2 * 10.0f) % 3600;
	if (ival1 < 0)
	{
		ival1 += 3600;
	}
	if (ival2 < 0)
	{
		ival2 += 3600;
	}
	int islr = ival1 - ival2;
	if (islr < 0)
	{
		islr += 3600;
	}
	int inewval;
	if (islr == 0)
	{
		inewval = ival1;
	}
	else
	{
		if (islr > 1800)
		{
			int isa = 3600 - islr;
			inewval = ival2 - ((float)isa * parcent);
		}
		else
		{
			int isa = islr;
			inewval = ival2 + ((float)isa * parcent);
		}
	}


	float newval = (float)inewval / 10.0f;
	return (newval);
}

//マスク用ステンシルバッファの初期化を行うか？
//インスタンスパーツとして再生する場合のみ設定する
void Player::setMaskFuncFlag(bool flg)
{
	_maskFuncFlag = flg;
}

//親のマスク対象
//インスタンスパーツとして再生する場合のみ設定する
//各パーツのマスク対象とアンドを取って処理する
void Player::setMaskParentSetting(bool flg)
{
	_maskParentSetting = flg;
}

void Player::setUserDataCallback(const UserDataCallback& callback)
{
	_userDataCallback = callback;
}

void Player::setPlayEndCallback(const PlayEndCallback& callback)
{
	_playEndCallback = callback;
}

State Player::getState(void)
{
	return(_state);
}

#ifdef USE_AGTK//sakihama-h, 2018.05.16
AnimeRef *Player::getAnimeRef(const std::string& animeName)
{
	SS_ASSERT2(_currentRs != NULL, "Not select data");
	return _currentRs->animeCache->getReference(animeName);
}

int Player::getFrameMax()
{
	if (_currentAnimeRef) {
		// Setup を指定すると範囲外参照でクラッシュする対策 2020/04/06 endo
		auto endFrame = _currentAnimeRef->animationData->endFrames;
		if (endFrame + 1 > getTotalFrame())
		{
			endFrame = getTotalFrame() - 1;
			if (endFrame < 0) {
				//マイナス値にならないように。
				endFrame = 0;
			}
		}
		return endFrame;
	}
	return -1;
}

int Player::getFrameMax(const std::string& animeName)
{
	auto animeRef = this->getAnimeRef(animeName);
	SS_ASSERT2(animeRef != NULL, "Not animeRef");
	// Setup を指定すると範囲外参照でクラッシュする対策 2020/04/06 endo
	auto endFrame = animeRef->animationData->endFrames;
	if (endFrame + 1 > getTotalFrame())
	{
		endFrame = getTotalFrame() - 1;
		if (endFrame < 0) {
			//マイナス値にならないように。
			endFrame = 0;
		}
	}
	return endFrame;
}
#endif

/**
 * CustomSprite
 */
CustomSprite::CustomSprite():
	  _opacity(1.0f)
	, _liveFrame(0.0f)
	, _hasPremultipliedAlpha(0)
	, refEffect(0)
	, _ssplayer(0)
	,effectAttrInitialized(false)
	,effectTimeTotal(0)
	, _maskInfluence(true)
	, _meshIsBind(false)
	, _meshVertexSize(0)
	, _mesh_uvs(nullptr)						// UVバッファ
	, _mesh_colors(nullptr)						// カラーバッファ
	, _mesh_vertices(nullptr)					// 座標バッファ
	, _mesh_indices(nullptr)
	, _playercontrol(nullptr)
{
	_meshVertexUV.clear();
}

CustomSprite::~CustomSprite()
{
	//エフェクトクラスがある場合は解放する
	SS_SAFE_DELETE(_mesh_uvs);						// UVバッファ
	SS_SAFE_DELETE(_mesh_colors);					// カラーバッファ
	SS_SAFE_DELETE(_mesh_vertices);					// 座標バッファ
	SS_SAFE_DELETE(_mesh_indices);					// 頂点順
	SS_SAFE_DELETE(refEffect);
	SS_SAFE_DELETE(_ssplayer);
}

CustomSprite* CustomSprite::create()
{
	CustomSprite *pSprite = new CustomSprite();
	if (pSprite)
	{
		pSprite->initState();
		return pSprite;
	}
	SS_SAFE_DELETE(pSprite);
	return NULL;
}

void CustomSprite::sethasPremultipliedAlpha(int PremultipliedAlpha)
{
	_hasPremultipliedAlpha = PremultipliedAlpha;
}

void CustomSprite::setOpacity(unsigned char opacity)
{
	_opacity = static_cast<float>(opacity) / 255.0f;
}

void CustomSprite::setFlippedX(bool flip)
{
	_flipX = flip;
}
void CustomSprite::setFlippedY(bool flip)
{
	_flipY = flip;
}
bool CustomSprite::isFlippedX()
{
	return (_flipX);
}
bool CustomSprite::isFlippedY()
{
	return (_flipY);
}


};
