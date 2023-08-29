// 
//  SS6Platform.cpp
//
#include "SS6PlayerPlatform.h"

/**
* 各プラットフォームに合わせて処理を作成してください
* OpenGL+glut用に作成されています。
*/

namespace ss
{
	//きれいな頂点変形に対応する場合は1にする。
	//４ポリゴンで変形します。
	//0の場合はZ型の２ポリゴンで変形します。
	#define USE_TRIANGLE_FIN (1)

	//セルマップの参照するテクスチャ割り当て管理用バッファ
	#define TEXTURE_MAX (512)						//全プレイヤーで使えるのセルマップの枚数
	cocos2d::Texture2D* texture[TEXTURE_MAX];		//セルマップの参照するテクスチャ情報の保持
	std::string textureKey[TEXTURE_MAX];			//セルマップの参照するテクスチャキャッシュに登録するキー
	int texture_index = 0;							//セルマップの参照ポインタ

	//レンダリング用ブレンドファンクションを使用するかフラグ
	static bool enableRenderingBlendFunc = false;

	//座標系設定
	int _direction;
	int _window_w;
	int _window_h;

	#define OPENGLES20	(1)	//Opengl 2.0で動作するコードにする場合は1

	//アプリケーション初期化時の処理
	void SSPlatformInit(void)
	{
		memset(texture, 0, sizeof(texture));
		int i;
		for (i = 0; i < TEXTURE_MAX; i++)
		{
			textureKey[i] = "";
		}
		texture_index = 0;

		_direction = PLUS_UP;
		_window_w = 1280;
		_window_h = 720;

		enableRenderingBlendFunc = false;
	}
	//アプリケーション終了時の処理
	void SSPlatformRelese(void)
	{
		int i;
		for (i = 0; i < TEXTURE_MAX; i++)
		{
			SSTextureRelese(i);
		}
	}

	/**
	* 上下どちらを正方向にするかとウィンドウサイズを設定します.
	* 上が正の場合はPLUS_UP、下が正の場合はPLUS_DOWN
	*
	* @param  direction      プラス方向
	* @param  window_w       ウィンドウサイズ
	* @param  window_h       ウィンドウサイズ
	*/
	void SSSetPlusDirection(int direction, int window_w, int window_h)
	{
		_direction = direction;
		_window_w = window_w;
		_window_h = window_h;
	}
	void SSGetPlusDirection(int &direction, int &window_w, int &window_h)
	{
		direction = _direction;
		window_w = _window_w;
		window_h = _window_h;
	}

	/**
	* レンダリング用のブレンドファンクションを使用する.
	* レンダリングターゲットとアルファ値がブレンドされてしまうためカラー値のみのレンダリングファンクションにする
	*
	* @param  flg	      通常描画:false、レンダリング描画:true
	*/
	void SSRenderingBlendFuncEnable(int flg)
	{
		enableRenderingBlendFunc = flg;
	}

	/**
	* ファイル読み込み
	*/
	unsigned char* SSFileOpen(const char* pszFileName, const char* pszMode, unsigned long * pSize, const char *pszZipFileName)
	{
		ssize_t nSize = 0;
		void* loadData = NULL;

		if (strcmp(pszZipFileName,"") != 0 )
		{
			//Zipファイルの読込み
			std::string fullpath = cocos2d::FileUtils::getInstance()->fullPathForFilename(pszZipFileName);
			cocos2d::Data zipdata = std::move(cocos2d::FileUtils::getInstance()->getDataFromFile(fullpath));
			cocos2d::ZipFile* zipfile = cocos2d::ZipFile::createWithBuffer(zipdata.getBytes(), zipdata.getSize());
			if (zipfile)
			{
				// ZIPファイルを読み込めた
				loadData = zipfile->getFileData(pszFileName, &nSize);
				delete zipfile;
			}
		}
		else
		{
			//直接ファイルを読む
			std::string fullpath = cocos2d::FileUtils::getInstance()->fullPathForFilename(pszFileName);

			loadData = cocos2d::FileUtils::getInstance()->getFileData(fullpath, pszMode, &nSize);
			*pSize = (long)nSize;
		}

		if (loadData == nullptr)
		{
			//ファイルの読み込みに失敗
			std::string msg = "Can't load project data > " + std::string(pszFileName);
			CCASSERT(loadData != nullptr, msg.c_str());
		}


		return (unsigned char *)loadData;
	}

	/**
	* テクスチャの読み込み
	*/
	long SSTextureLoad(const char* pszFileName, int  wrapmode, int filtermode, const char *pszZipFileName)
	{
		/**
		* テクスチャ管理用のユニークな値を返してください。
		* テクスチャの管理はゲーム側で行う形になります。
		* テクスチャにアクセスするハンドルや、テクスチャを割り当てたバッファ番号等になります。
		*
		* プレイヤーはここで返した値とパーツのステータスを引数に描画を行います。
		* ResourceManager::changeTextureを使用する場合はSSTextureLoadから取得したインデックスを設定してください。
		*/
		long rc = 0;

		//空きバッファを検索して使用する
		int start_index = texture_index;	//開始したインデックスを保存する
		bool exit = true;
		bool isLoad = false;
		while (exit)
		{
			if (texture[texture_index] == 0)	//使われていないテクスチャ情報
			{
				//読み込み処理
				cocos2d::TextureCache* texCache = cocos2d::Director::getInstance()->getTextureCache();
				cocos2d::Texture2D* tex = texCache->getTextureForKey(pszFileName);	//テクスチャキャッシュにテクスチャがあるか参照する

				if (tex == NULL)
				{
					//キャッシュにテクスチャがない場合は読み込む
#ifdef USE_AGTK
					//アクツクMVではPNGをストレートアルファで扱うため、変更不要。
#else
					cocos2d::CCImage::setPNGPremultipliedAlphaEnabled(false);	//ストーレートアルファで読み込む
#endif

					if (strcmp(pszZipFileName, "") != 0)
					{
						//Zipファイルの読込み
						std::string fullpath = cocos2d::FileUtils::getInstance()->fullPathForFilename(pszZipFileName);
						cocos2d::Data zipdata = std::move(cocos2d::FileUtils::getInstance()->getDataFromFile(fullpath));
						cocos2d::ZipFile* zipfile = cocos2d::ZipFile::createWithBuffer(zipdata.getBytes(), zipdata.getSize());
						if(zipfile)
						{
							// ZIPファイルを読み込めた
							ssize_t filesize;
							unsigned char *loadData = zipfile->getFileData(pszFileName, &filesize);
							if (loadData)
							{
								//ZIP内に指定のファイルが存在している
								cocos2d::Image* image = nullptr;
								image = new (std::nothrow) cocos2d::Image();

								bool bRet = image->initWithImageData(loadData, filesize);
								if (bRet)
								{
									tex = texCache->addImage(image, pszFileName);
								}
								CC_SAFE_RELEASE(image);
								free(loadData);
							}
						}
						//ZIPを破棄する
						delete zipfile;
					}
					else
					{
						//パスからファイルを読む
						tex = texCache->addImage(pszFileName);
					}
#ifdef USE_AGTK
					//アクツクMVではPNGをストレートアルファで扱うため、変更不要。
#else
					cocos2d::CCImage::setPNGPremultipliedAlphaEnabled(true);	//ステータスを戻しておく
#endif
				}

				texture[texture_index] = tex;
				if (!texture[texture_index]) {
					DEBUG_PRINTF("テクスチャの読み込み失敗\n");
				}
				else
				{
					textureKey[texture_index] = pszFileName;	//登録したテクスチャのキーを保存する

					//SpriteStudioで設定されたテクスチャ設定を反映させるための分岐です。
					cocos2d::Texture2D::TexParams texParams;
					switch (wrapmode)
					{
					case SsTexWrapMode::clamp:	//クランプ
						texParams.wrapS = GL_CLAMP_TO_EDGE;
						texParams.wrapT = GL_CLAMP_TO_EDGE;
						break;
					case SsTexWrapMode::repeat:	//リピート
						texParams.wrapS = GL_REPEAT;
						texParams.wrapT = GL_REPEAT;
						break;
					case SsTexWrapMode::mirror:	//ミラー
						texParams.wrapS = GL_MIRRORED_REPEAT;
						texParams.wrapT = GL_MIRRORED_REPEAT;
						break;
					}
					switch (filtermode)
					{
					case SsTexFilterMode::nearlest:	//ニアレストネイバー
						texParams.minFilter = GL_NEAREST;
						texParams.magFilter = GL_NEAREST;
						break;
					case SsTexFilterMode::linear:	//リニア、バイリニア
						texParams.minFilter = GL_LINEAR;
						texParams.magFilter = GL_LINEAR;
						break;
					}
					tex->setTexParameters(texParams);

					isLoad = true;
					rc = texture_index;	//テクスチャハンドルをリソースマネージャに設定する
				}
				exit = false;	//ループ終わり
			}
			//次のインデックスに移動する
			texture_index++;
			if (texture_index >= TEXTURE_MAX)
			{
				texture_index = 0;
			}
			if (texture_index == start_index)
			{
				//一周したバッファが開いてない
				DEBUG_PRINTF("テクスチャバッファの空きがない\n");
				exit = false;	//ループ終わり
			}
		}

		if (isLoad)
		{
		}
		return rc;
	}
	
	/**
	* テクスチャの解放
	*/
	bool SSTextureRelese(long handle)
	{
		/// 解放後も同じ番号で何度も解放処理が呼ばれるので、例外が出ないように作成してください。
		bool rc = true;

		//参照するハンドルがある
		if (texture[handle])
		{
			cocos2d::TextureCache* texCache = cocos2d::Director::getInstance()->getTextureCache();

			//テクスチャは登録されている
			//同じテクスチャを参照している場合があるので、キーを元に判断する
			cocos2d::Texture2D* tex = texCache->getTextureForKey(textureKey[handle]);	//テクスチャキャッシュにテクスチャがあるか参照する
			if ( tex )
			{
				//該当するキーのテクスチャがキャッシュされている
				//テクスチャの削除
				texCache->removeTexture(texture[handle]);
			}

			//登録情報の削除
			texture[handle] = 0;
			textureKey[handle] = "";
		}
		else
		{
			rc = false;
		}

		return rc ;
	}

	/**
	* 画像ファイル名から読み込まれているテクスチャバッファのインデックスを取得する
	* keyはResourcesフォルダからの画像ファイルまでのパスになります。
	*
	* 使用されていない場合はfalseになります。
	*/
	bool SSGetTextureIndex(std::string  key, std::vector<int> *indexList)
	{
		bool rc = false;

		indexList->clear();

		int i;
		for ( i = 0; i < TEXTURE_MAX; i++ )
		{
			if (textureKey[i] == key)
			{
				indexList->push_back(i);
				rc = true;
			}
		}


		return (rc);
	}

	/**
	* テクスチャのサイズを取得
	* テクスチャのUVを設定するのに使用します。
	*/
	bool SSGetTextureSize(long handle, int &w, int &h)
	{
		if (texture[handle])
		{
			w = texture[handle]->getPixelsWide();
			h = texture[handle]->getPixelsHigh();
		}
		else
		{
			return false;
		}
		return true;
	}

	/**
	* 描画ステータス
	*/
	struct SSDrawState
	{
		int texture;
		int partType;
		int partBlendfunc;
		int partsColorUse;
		int partsColorFunc;
		int partsColorType;
		int maskInfluence;
		void init(void)
		{
			texture = -1;
			partType = -1;
			partBlendfunc = -1;
			partsColorUse = -1;
			partsColorFunc = -1;
			partsColorType = -1;
			maskInfluence = -1;
		}
	};
	SSDrawState _ssDrawState;

	//各プレイヤーの描画を行う前の初期化処理
	GLboolean _currentStencilEnabled = GL_FALSE;
	void SSRenderSetup( void )
	{
#if OPENGLES20
#else
		glDisableClientState(GL_COLOR_ARRAY);
		glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
		glEnable(GL_TEXTURE_2D);
#endif
//		glBindTexture(GL_TEXTURE_2D, 0);	//0にするとパーツの描画がない場合に他のspritenoのテクスチャを無効にしてしまう

		glEnable(GL_BLEND);
		glDisable(GL_DEPTH_TEST);

		_currentStencilEnabled = glIsEnabled(GL_STENCIL_TEST);
#if OPENGLES20
#else
		glEnable(GL_ALPHA_TEST);
		glAlphaFunc(GL_GREATER, 0.0);
#endif
		glBlendEquation(GL_FUNC_ADD);

		cocos2d::GL::enableVertexAttribs(cocos2d::GL::VERTEX_ATTRIB_FLAG_POS_COLOR_TEX);

		_ssDrawState.init();
	}
	void SSRenderEnd(void)
	{
//		CC_INCREMENT_GL_DRAWS(1);

#if OPENGLES20
#else
		glDisable(GL_TEXTURE_2D);
		glDisable(GL_ALPHA_TEST);
#endif
		//ブレンドモード　減算時の設定を戻す
		glBlendEquation(GL_FUNC_ADD);
		//ブレンドファンクションを通常に戻しcocosにも通知する
#ifdef USE_AGTK
		glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ONE);
#else
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
#endif
		cocos2d::GL::blendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		if (!_currentStencilEnabled)
		{
			glDisable(GL_STENCIL_TEST);
		}

#if OPENGLES20
#else
		glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
#endif
	}

	//中間点を求める
	static void	CoordinateGetDiagonalIntersection(SsVector2& out, const SsVector2& LU, const SsVector2& RU, const SsVector2& LD, const SsVector2& RD)
	{
		out = SsVector2(0.f, 0.f);

		/* <<< 係数を求める >>> */
		float c1 = (LD.y - RU.y) * (LD.x - LU.x) - (LD.x - RU.x) * (LD.y - LU.y);
		float c2 = (RD.x - LU.x) * (LD.y - LU.y) - (RD.y - LU.y) * (LD.x - LU.x);
		float c3 = (RD.x - LU.x) * (LD.y - RU.y) - (RD.y - LU.y) * (LD.x - RU.x);


		if (c3 <= 0 && c3 >= 0) return;

		float ca = c1 / c3;
		float cb = c2 / c3;

		/* <<< 交差判定 >>> */
		if (((0.0f <= ca) && (1.0f >= ca)) && ((0.0f <= cb) && (1.0f >= cb)))
		{	/* 交差している */
			out.x = LU.x + ca * (RD.x - LU.x);
			out.y = LU.y + ca * (RD.y - LU.y);
		}
	}

	/**
	パーツカラー用
	ブレンドタイプに応じたテクスチャコンバイナの設定を行う

	ミックスのみコンスタント値を使う。
	他は事前に頂点カラーに対してブレンド率を掛けておく事でαも含めてブレンドに対応している。
	*/
	void setupPartsColorTextureCombiner(SSPlayerControl* pc, BlendType blendType, VertexFlag colorBlendTarget, float rate)
	{
		const auto& matrixP = cocos2d::Director::getInstance()->getMatrix(cocos2d::MATRIX_STACK_TYPE::MATRIX_STACK_PROJECTION);
		cocos2d::Mat4 matrixMVP = matrixP;
		cocos2d::GLProgram *glprogram;

		//パーツカラーの反映
		switch (blendType)
		{
		case BlendType::BLEND_MIX:
			if ((VertexFlag)colorBlendTarget == VertexFlag::VERTEX_FLAG_ONE)
			{
				//シェーダーを適用する
				pc->setGLProgram(SSPlayerControl::_partColorMIXONEShaderProgram);
				pc->getShaderProgram()->use();
				glprogram = pc->getGLProgram();	//
				glprogram->setUniformsForBuiltins();
				//マトリクスを設定
				glUniformMatrix4fv(SSPlayerControl::_MIXONE_uniform_map[(int)WVP], 1, 0, (float *)&matrixMVP.m);
				// テクスチャサンプラ情報をシェーダーに送る  
				glUniform1i(SSPlayerControl::_MIXONE_uniform_map[SAMPLER], 0);
				glUniform1f(SSPlayerControl::_MIXONE_uniform_map[RATE], rate);
			}
			else
			{
				//シェーダーを適用する
				pc->setGLProgram(SSPlayerControl::_partColorMIXVERTShaderProgram);
				pc->getShaderProgram()->use();
				glprogram = pc->getGLProgram();	//
				glprogram->setUniformsForBuiltins();
				//マトリクスを設定
				glUniformMatrix4fv(SSPlayerControl::_MIXVERT_uniform_map[WVP], 1, 0, (float *)&matrixMVP.m);
				// テクスチャサンプラ情報をシェーダーに送る  
				glUniform1i(SSPlayerControl::_MIXVERT_uniform_map[SAMPLER], 0);
				glUniform1f(SSPlayerControl::_MIXVERT_uniform_map[RATE], rate);
			}
			break;
		case BlendType::BLEND_MUL:
			//シェーダーを適用する
			pc->setGLProgram(SSPlayerControl::_partColorMULShaderProgram);
			pc->getShaderProgram()->use();
			glprogram = pc->getGLProgram();	//
			glprogram->setUniformsForBuiltins();
			//マトリクスを設定
			glUniformMatrix4fv(SSPlayerControl::_MUL_uniform_map[WVP], 1, 0, (float *)&matrixMVP.m);
			// テクスチャサンプラ情報をシェーダーに送る  
			glUniform1i(SSPlayerControl::_MUL_uniform_map[SAMPLER], 0);
			break;
		case BlendType::BLEND_ADD:
			//シェーダーを適用する
			pc->setGLProgram(SSPlayerControl::_partColorADDShaderProgram);
			pc->getShaderProgram()->use();
			glprogram = pc->getGLProgram();	//
			glprogram->setUniformsForBuiltins();
			//マトリクスを設定
			glUniformMatrix4fv(SSPlayerControl::_ADD_uniform_map[WVP], 1, 0, (float *)&matrixMVP.m);
			// テクスチャサンプラ情報をシェーダーに送る  
			glUniform1i(SSPlayerControl::_ADD_uniform_map[SAMPLER], 0);
			break;
		case BlendType::BLEND_SUB:
			//シェーダーを適用する
			pc->setGLProgram(SSPlayerControl::_partColorSUBShaderProgram);
			pc->getShaderProgram()->use();
			glprogram = pc->getGLProgram();	//
			glprogram->setUniformsForBuiltins();
			//マトリクスを設定
			glUniformMatrix4fv(SSPlayerControl::_SUB_uniform_map[WVP], 1, 0, (float *)&matrixMVP.m);
			// テクスチャサンプラ情報をシェーダーに送る  
			glUniform1i(SSPlayerControl::_SUB_uniform_map[SAMPLER], 0);
			break;
		}
	}

	//頂点バッファにパラメータを保存する
	void setClientState(SSV3F_C4B_T2F point, int index, float* uvs, unsigned char* colors, float* vertices)
	{
		uvs[0 + (index * 2)] = point.texCoords.u;
		uvs[1 + (index * 2)] = point.texCoords.v;

		colors[0 + (index * 4)] = point.colors.r;
		colors[1 + (index * 4)] = point.colors.g;
		colors[2 + (index * 4)] = point.colors.b;
		colors[3 + (index * 4)] = point.colors.a;

		vertices[0 + (index * 3)] = point.vertices.x;
		vertices[1 + (index * 3)] = point.vertices.y;
		vertices[2 + (index * 3)] = point.vertices.z;
	}

	/**
	* メッシュの表示
	*/
	void SSDrawMesh(CustomSprite *sprite, State state)
	{
		bool ispartColor = (state.flags & PART_FLAG_PARTS_COLOR);

		// 単色で処理する
		unsigned char alpha = (state.quad.tl.colors.a * state.Calc_opacity ) / 255;
		unsigned char setcol[4];
		setcol[0] = state.quad.tl.colors.r;	//cocosはbyteで処理しているので
		setcol[1] = state.quad.tl.colors.g;
		setcol[2] = state.quad.tl.colors.b;
		setcol[3] = alpha;

//		if (
//			(_ssDrawState.partsColorFunc != state.partsColorFunc)
//			|| (_ssDrawState.partsColorType != state.partsColorType)
//			|| (_ssDrawState.partsColorUse != ispartColor)
//			)
		{
			if (state.flags & PART_FLAG_PARTS_COLOR)
			{
				setupPartsColorTextureCombiner(sprite->_playercontrol, (BlendType)state.partsColorFunc, (VertexFlag)state.partsColorType, state.rate.oneRate);
			}
			else
			{
				//パーツカラーが設定されていない場合はディフォルトシェーダーを使用する
				sprite->_playercontrol->setGLProgram(sprite->_playercontrol->_defaultShaderProgram);
				sprite->_playercontrol->getShaderProgram()->use();
				auto glprogram = sprite->_playercontrol->getGLProgram();	//
				glprogram->setUniformsForBuiltins();
			}
		}


		//メッシュの座標データは親子の計算が済んでいるのでプレイヤーのTRSで変形させる
		float t[16];
		float mat[16];
		IdentityMatrix(mat);
		State pls = sprite->_parentPlayer->getState();

		MultiplyMatrix(pls.mat, mat, mat);

		for (size_t i = 0; i < sprite->_meshVertexSize; i++)
		{
			sprite->_mesh_colors[i * 4 + 0] = setcol[0];
			sprite->_mesh_colors[i * 4 + 1] = setcol[1];
			sprite->_mesh_colors[i * 4 + 2] = setcol[2];
			sprite->_mesh_colors[i * 4 + 3] = setcol[3];

			if ( sprite->_meshIsBind == true )
			{
				//プレイヤーのマトリクスをメッシュデータに与える
				TranslationMatrix(t, sprite->_mesh_vertices[i * 3 + 0], sprite->_mesh_vertices[i * 3 + 1], sprite->_mesh_vertices[i * 3 + 2]);
				MultiplyMatrix(t, mat, t);
				sprite->_mesh_vertices[i * 3 + 0] = t[12];
				sprite->_mesh_vertices[i * 3 + 1] = t[13];
				sprite->_mesh_vertices[i * 3 + 2] = 0;
			}
			else
			{
				//バインドされていないメッシュはパーツのマトリクスを与える
				TranslationMatrix(t, sprite->_mesh_vertices[i * 3 + 0], sprite->_mesh_vertices[i * 3 + 1], sprite->_mesh_vertices[i * 3 + 2]);
				MultiplyMatrix(t, state.mat, t);
				MultiplyMatrix(t, mat, t);
				sprite->_mesh_vertices[i * 3 + 0] = t[12];
				sprite->_mesh_vertices[i * 3 + 1] = t[13];
				sprite->_mesh_vertices[i * 3 + 2] = 0;
			}
		}

		// vertex
		glVertexAttribPointer(cocos2d::GLProgram::VERTEX_ATTRIB_POSITION, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)sprite->_mesh_vertices);

		// texCoods
		glVertexAttribPointer(cocos2d::GLProgram::VERTEX_ATTRIB_TEX_COORD, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)sprite->_mesh_uvs);

		// color
		glVertexAttribPointer(cocos2d::GLProgram::VERTEX_ATTRIB_COLOR, 4, GL_UNSIGNED_BYTE, GL_TRUE, 4 * sizeof(unsigned char), (void*)sprite->_mesh_colors);

		glDrawElements(GL_TRIANGLES, sprite->_meshTriangleSize * 3, GL_UNSIGNED_SHORT, sprite->_mesh_indices);
	}

	/**
	* スプライトの表示
	*/
	void SSDrawSprite(CustomSprite *sprite, State *overwrite_state)
	{
		if (sprite->_state.isVisibled == false) return; //非表示なので処理をしない
		if (sprite->_playercontrol == nullptr) return;

		//ステータスから情報を取得し、各プラットフォームに合わせて機能を実装してください。
		State state;
		if (overwrite_state)
		{
			//個別に用意したステートを使用する（エフェクトのパーティクル用）
			state = *overwrite_state;
		}
		else
		{
			state = sprite->_state;
		}
		int tex_index = state.texture.handle;
		if (texture[tex_index] == nullptr)
		{
			return;
		}

		if (sprite->_parentPlayer->getMaskFunctionUse() == true )
		{
			execMask(sprite);	//マスク初期化
		}

		/**
		* OpenGLの3D機能を使用してスプライトを表示します。
		* 下方向がプラスになります。
		* 3Dを使用する場合頂点情報を使用して再現すると頂点変形やUV系のアトリビュートを反映させる事ができます。
		*/
		//描画用頂点情報を作成
		SSV3F_C4B_T2F_Quad quad;
		quad = state.quad;

		//原点補正
		float cx = state.size_X * -state.pivotX;
		float cy = state.size_Y * -state.pivotY;

		quad.tl.vertices.x += cx;
		quad.tl.vertices.y += cy;
		quad.tr.vertices.x += cx;
		quad.tr.vertices.y += cy;
		quad.bl.vertices.x += cx;
		quad.bl.vertices.y += cy;
		quad.br.vertices.x += cx;
		quad.br.vertices.y += cy;

		float mat[16];
		IdentityMatrix(mat);
		State pls = sprite->_parentPlayer->getState();	//プレイヤーのTRSを最終座標に加える

		MultiplyMatrix(pls.mat, mat, mat);

		float t[16];
		TranslationMatrix(t, quad.tl.vertices.x, quad.tl.vertices.y, 0.0f);

		MultiplyMatrix(t, state.mat, t);	//SS上のTRS
		MultiplyMatrix(t, mat, t);			//プレイヤーのTRS	
		quad.tl.vertices.x = t[12];
		quad.tl.vertices.y = t[13];
		TranslationMatrix(t, quad.tr.vertices.x, quad.tr.vertices.y, 0.0f);
		MultiplyMatrix(t, state.mat, t);
		MultiplyMatrix(t, mat, t);
		quad.tr.vertices.x = t[12];
		quad.tr.vertices.y = t[13];
		TranslationMatrix(t, quad.bl.vertices.x, quad.bl.vertices.y, 0.0f);
		MultiplyMatrix(t, state.mat, t);
		MultiplyMatrix(t, mat, t);
		quad.bl.vertices.x = t[12];
		quad.bl.vertices.y = t[13];
		TranslationMatrix(t, quad.br.vertices.x, quad.br.vertices.y, 0.0f);
		MultiplyMatrix(t, state.mat, t);
		MultiplyMatrix(t, mat, t);
		quad.br.vertices.x = t[12];
		quad.br.vertices.y = t[13];

		//頂点カラーにアルファを設定
		float alpha = state.Calc_opacity / 255.0f;
		if (state.flags & PART_FLAG_LOCALOPACITY)
		{
			alpha = state.localopacity / 255.0f;	//ローカル不透明度対応
		}

		if (
			   (state.flags & PART_FLAG_PARTS_COLOR)
			&& ((VertexFlag)state.partsColorType != VertexFlag::VERTEX_FLAG_ONE)
			&& ((BlendType)state.partsColorFunc == BlendType::BLEND_MIX)
			)
		{
			//ver6.2 パーツカラー対応
			//パーツカラー、頂点、MIXを選択した場合は不透明度を適用しない
			//ミックスの場合、Rate として扱われるので不透明度を掛けてはいけない
		}
		else
		{
			quad.tl.colors.a = quad.tl.colors.a * alpha;
			quad.tr.colors.a = quad.tr.colors.a * alpha;
			quad.bl.colors.a = quad.bl.colors.a * alpha;
			quad.br.colors.a = quad.br.colors.a * alpha;
		}

		//テクスチャ有効
		int	gl_target = GL_TEXTURE_2D;
		if (_ssDrawState.texture != texture[tex_index]->getName())
		{
#if OPENGLES20
#else
			glEnable(gl_target);
#endif
			//テクスチャのバインド
			//cocos内部のbindTexture2Dを使用しないとならない。
			//直接バインドを変えると、カレントのテクスチャが更新されず、他のspriteの描画自にテクスチャのバインドがされない
//			glBindTexture(gl_target, texture[tex_index]->getName());
			cocos2d::GL::bindTexture2D(texture[tex_index]->getName());
		}

		//描画モード
		//
		if (_ssDrawState.partBlendfunc != state.blendfunc)
		{
			glBlendEquation(GL_FUNC_ADD);
			if (enableRenderingBlendFunc == false)
			{
				//通常の描画
				switch (state.blendfunc)
				{
				case BLEND_MIX:		///< 0 ブレンド（ミックス）
#ifdef USE_AGTK
					glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ONE);
#else
					glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
#endif
					break;
				case BLEND_MUL:		///< 1 乗算
#ifdef USE_AGTK
					glBlendFuncSeparate(GL_ZERO, GL_SRC_COLOR, GL_ZERO, GL_SRC_COLOR);
#else
					glBlendFunc(GL_ZERO, GL_SRC_COLOR);
#endif
					break;
				case BLEND_ADD:		///< 2 加算
#ifdef USE_AGTK
					glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE, GL_ONE, GL_ONE);
#else
					glBlendFunc(GL_SRC_ALPHA, GL_ONE);
#endif
					break;
				case BLEND_SUB:		///< 3 減算

					glBlendEquation(GL_FUNC_REVERSE_SUBTRACT);
#if OPENGLES20
					glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE, GL_ZERO, GL_DST_ALPHA);
#else
					glBlendFuncSeparateEXT(GL_SRC_ALPHA, GL_ONE, GL_ZERO, GL_DST_ALPHA);
#endif
					break;
				case BLEND_MULALPHA:	///< 4 α乗算
#ifdef USE_AGTK
					glBlendFuncSeparate(GL_DST_COLOR, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ONE);
#else
					glBlendFunc(GL_DST_COLOR, GL_ONE_MINUS_SRC_ALPHA);
#endif
					break;
				case BLEND_SCREEN:		///< 5 スクリーン
#ifdef USE_AGTK
					glBlendFuncSeparate(GL_ONE_MINUS_DST_COLOR, GL_ONE, GL_ZERO, GL_ONE);
#else
					glBlendFunc(GL_ONE_MINUS_DST_COLOR, GL_ONE);
#endif
					break;
				case BLEND_EXCLUSION:	///< 6 除外
#ifdef USE_AGTK
					glBlendFuncSeparate(GL_ONE_MINUS_DST_COLOR, GL_ONE_MINUS_SRC_COLOR, GL_ZERO, GL_ONE);
#else
					glBlendFunc(GL_ONE_MINUS_DST_COLOR, GL_ONE_MINUS_SRC_COLOR);
#endif
					break;
				case BLEND_INVERT:		///< 7 反転
#ifdef USE_AGTK
					glBlendFuncSeparate(GL_ONE_MINUS_DST_COLOR, GL_ZERO, GL_ZERO, GL_ONE);
#else
					glBlendFunc(GL_ONE_MINUS_DST_COLOR, GL_ZERO);
#endif
					break;
				}
			}
			else
			{
				//レンダリング用の描画
				switch (state.blendfunc)
				{
				case BLEND_MIX:		///< 0 ブレンド（ミックス）
#if OPENGLES20
					glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
#else
					glBlendFuncSeparateEXT(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
#endif
					break;
				case BLEND_MUL:		///< 1 乗算
#if OPENGLES20
					glBlendFuncSeparate(GL_ZERO, GL_SRC_COLOR, GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
#else
					glBlendFuncSeparateEXT(GL_ZERO, GL_SRC_COLOR, GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
#endif
					break;
				case BLEND_ADD:		///< 2 加算
#if OPENGLES20
					glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE, GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
#else
					glBlendFuncSeparateEXT(GL_SRC_ALPHA, GL_ONE, GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
#endif
					break;
				case BLEND_SUB:		///< 3 減算

					glBlendEquation(GL_FUNC_REVERSE_SUBTRACT);
#if OPENGLES20
					glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE, GL_ZERO, GL_ONE_MINUS_SRC_ALPHA);
#else
					glBlendFuncSeparateEXT(GL_SRC_ALPHA, GL_ONE, GL_ZERO, GL_ONE_MINUS_SRC_ALPHA);
#endif
					break;
				case BLEND_MULALPHA:	///< 4 α乗算
#if OPENGLES20
					glBlendFuncSeparate(GL_DST_COLOR, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
#else
					glBlendFuncSeparateEXT(GL_DST_COLOR, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
#endif
					break;
				case BLEND_SCREEN:		///< 5 スクリーン
#if OPENGLES20
					glBlendFuncSeparate(GL_ONE_MINUS_DST_COLOR, GL_ONE, GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
#else
					glBlendFuncSeparateEXT(GL_ONE_MINUS_DST_COLOR, GL_ONE, GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
#endif
					break;
				case BLEND_EXCLUSION:	///< 6 除外
#if OPENGLES20
					glBlendFuncSeparate(GL_ONE_MINUS_DST_COLOR, GL_ONE_MINUS_SRC_COLOR, GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
#else
					glBlendFuncSeparateEXT(GL_ONE_MINUS_DST_COLOR, GL_ONE_MINUS_SRC_COLOR, GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
#endif
					break;
				case BLEND_INVERT:		///< 7 反転
#if OPENGLES20
					glBlendFuncSeparate(GL_ONE_MINUS_DST_COLOR, GL_ZERO, GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
#else
					glBlendFuncSeparateEXT(GL_ONE_MINUS_DST_COLOR, GL_ZERO, GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
#endif
					break;
				}
			}
		}

		bool ispartColor = (state.flags & PART_FLAG_PARTS_COLOR);

		//メッシュの場合描画
		if (sprite->_partData.type == PARTTYPE_MESH)
		{
			SSDrawMesh(sprite, state);

			_ssDrawState.texture = texture[tex_index]->getName();
			_ssDrawState.partType = sprite->_partData.type;
			_ssDrawState.partBlendfunc = state.blendfunc;
			_ssDrawState.partsColorFunc = state.partsColorFunc;
			_ssDrawState.partsColorType = state.partsColorType;
			_ssDrawState.partsColorUse = (int)ispartColor;
			_ssDrawState.maskInfluence = (int)sprite->_maskInfluence;

			return;
		}

		if (sprite->_partData.type == PARTTYPE_MASK)
		{
//			if (_ssDrawState.partType != sprite->_partData.type)
			{
				//不透明度からマスク閾値へ変更
				float mask_alpha = (float)(255 - state.masklimen) / 255.0f;

				const auto& matrixP = cocos2d::Director::getInstance()->getMatrix(cocos2d::MATRIX_STACK_TYPE::MATRIX_STACK_PROJECTION);
				cocos2d::Mat4 matrixMVP = matrixP;
				//シェーダーを適用する
				sprite->_playercontrol->setGLProgram(SSPlayerControl::_MASKShaderProgram);
				sprite->_playercontrol->getShaderProgram()->use();
				auto glprogram = sprite->_playercontrol->getGLProgram();	//
				glprogram->setUniformsForBuiltins();
				//マトリクスを設定
				glUniformMatrix4fv(SSPlayerControl::_MASK_uniform_map[(int)WVP], 1, 0, (float *)&matrixMVP.m);
				// テクスチャサンプラ情報をシェーダーに送る  
				glUniform1i(SSPlayerControl::_MASK_uniform_map[SAMPLER], 0);
				glUniform1f(SSPlayerControl::_MASK_uniform_map[RATE], mask_alpha);
			}
		}
		else
		{
			//パーツカラーの適用
//			if (
//				 (_ssDrawState.partsColorFunc != state.partsColorFunc)
//			  || (_ssDrawState.partsColorType != state.partsColorType)
//			  || ( _ssDrawState.partsColorUse != ispartColor)
//			  || (sprite->_partData.type == PARTTYPE_MASK)
//			   )
			{
				//パーツカラーの反映
				if (state.flags & PART_FLAG_PARTS_COLOR)
				{
					//パーツカラーの反映
					if ((VertexFlag)state.partsColorType == VertexFlag::VERTEX_FLAG_ONE)
					{
						//単色
						setupPartsColorTextureCombiner(sprite->_playercontrol, (BlendType)state.partsColorFunc, (VertexFlag)state.partsColorType, state.rate.oneRate);
					}
					else
					{
						//頂点
						setupPartsColorTextureCombiner(sprite->_playercontrol, (BlendType)state.partsColorFunc, (VertexFlag)state.partsColorType, alpha);
					}
				}
				else
				{
					//パーツカラーが設定されていない場合はディフォルトシェーダーを使用する
					sprite->_playercontrol->setGLProgram(sprite->_playercontrol->_defaultShaderProgram);
					sprite->_playercontrol->getShaderProgram()->use();
					auto glprogram = sprite->_playercontrol->getGLProgram();	//
					glprogram->setUniformsForBuiltins();
				}
			}
		}


#if USE_TRIANGLE_FIN

		//きれいな頂点変形に対応
		if ((state.flags & PART_FLAG_PARTS_COLOR) || (state.flags & PART_FLAG_VERTEX_TRANSFORM))
		{
			// ssbpLibでは4つの頂点でスプライトの表示を実装しています。
			// SS6では５つの頂点でスプライトの表示を行っており、頂点変形時のゆがみ方が異なります。
			float			uvs[2 * 5];			// UVバッファ
			unsigned char	colors[4 * 5];		// カラーバッファ
			float			vertices[3 * 5];	// 座標バッファ

			memset(uvs, 0, sizeof(uvs));
			memset(colors, 0, sizeof(colors));
			memset(vertices, 0, sizeof(vertices));

			setClientState(quad.tl, 0, uvs, colors, vertices);
			setClientState(quad.tr, 1, uvs, colors, vertices);
			setClientState(quad.bl, 2, uvs, colors, vertices);
			setClientState(quad.br, 3, uvs, colors, vertices);

			//頂点変形、パーツカラーを使用した場合は中心に頂点を作成し4つのポリゴンに分割して描画を行う。
			//頂点の算出
			SsVector2	vertexCoordinateLU = SsVector2(quad.tl.vertices.x, quad.tl.vertices.y);// : 左上頂点座標（ピクセル座標系）
			SsVector2	vertexCoordinateRU = SsVector2(quad.tr.vertices.x, quad.tr.vertices.y);// : 右上頂点座標（ピクセル座標系）
			SsVector2	vertexCoordinateLD = SsVector2(quad.bl.vertices.x, quad.bl.vertices.y);// : 左下頂点座標（ピクセル座標系）
			SsVector2	vertexCoordinateRD = SsVector2(quad.br.vertices.x, quad.br.vertices.y);// : 右下頂点座標（ピクセル座標系）

			SsVector2 CoordinateLURU = (vertexCoordinateLU + vertexCoordinateRU) * 0.5f;
			SsVector2 CoordinateLULD = (vertexCoordinateLU + vertexCoordinateLD) * 0.5f;
			SsVector2 CoordinateLDRD = (vertexCoordinateLD + vertexCoordinateRD) * 0.5f;
			SsVector2 CoordinateRURD = (vertexCoordinateRU + vertexCoordinateRD) * 0.5f;

			SsVector2 center;
			CoordinateGetDiagonalIntersection(center, CoordinateLURU, CoordinateRURD, CoordinateLULD, CoordinateLDRD);

			SsVector2*	coodinatetable[] = { &vertexCoordinateLU , &vertexCoordinateRU , &vertexCoordinateLD , &vertexCoordinateRD , &center };

			//頂点の設定
			int i;
			vertices[4 * 3 + 0] = center.x;
			vertices[4 * 3 + 1] = center.y;
			vertices[4 * 3 + 2] = 0;
			//UVの設定
			for (i = 0; i < 4; ++i)
			{
				uvs[4 * 2] += uvs[i * 2];
				uvs[4 * 2 + 1] += uvs[i * 2 + 1];
			}
			uvs[4 * 2] /= 4.0f;
			uvs[4 * 2 + 1] /= 4.0f;

			int a, r, g, b;
			a = r = g = b = 0;
			for (int i = 0; i < 4; i++)
			{
				int idx = i * 4;
				r += colors[idx++];
				g += colors[idx++];
				b += colors[idx++];
				a += colors[idx++];
			}
			//カラー値の設定
			int idx = 4 * 4;
			colors[idx++] = (unsigned char)(r / 4.0f);
			colors[idx++] = (unsigned char)(g / 4.0f);
			colors[idx++] = (unsigned char)(b / 4.0f);
			colors[idx++] = (unsigned char)(a / 4.0f);

			//描画

			// vertex
			glVertexAttribPointer(cocos2d::GLProgram::VERTEX_ATTRIB_POSITION, 3, GL_FLOAT, GL_FALSE, 0, (void*)vertices);

			// texCoods
			glVertexAttribPointer(cocos2d::GLProgram::VERTEX_ATTRIB_TEX_COORD, 2, GL_FLOAT, GL_FALSE, 0, (void*)uvs);

			// color
			glVertexAttribPointer(cocos2d::GLProgram::VERTEX_ATTRIB_COLOR, 4, GL_UNSIGNED_BYTE, GL_TRUE, 0, (void*)colors);

			static const GLubyte indices[] = { 4, 3, 1, 0, 2, 3 };
			glDrawElements(GL_TRIANGLE_FAN, 6, GL_UNSIGNED_BYTE, indices);

		}
		else
		{
			// 変形しないスプライトはZ型の2ポリゴンで分割表示する
#define kQuadSize sizeof(quad.bl)
			long offset = (long)&quad;

			// vertex
			int diff = offsetof(cocos2d::V3F_C4B_T2F, vertices);
			glVertexAttribPointer(cocos2d::GLProgram::VERTEX_ATTRIB_POSITION, 3, GL_FLOAT, GL_FALSE, kQuadSize, (void*)(offset + diff));

			// texCoods
			diff = offsetof(cocos2d::V3F_C4B_T2F, texCoords);
			glVertexAttribPointer(cocos2d::GLProgram::VERTEX_ATTRIB_TEX_COORD, 2, GL_FLOAT, GL_FALSE, kQuadSize, (void*)(offset + diff));

			// color
			diff = offsetof(cocos2d::V3F_C4B_T2F, colors);
			glVertexAttribPointer(cocos2d::GLProgram::VERTEX_ATTRIB_COLOR, 4, GL_UNSIGNED_BYTE, GL_TRUE, kQuadSize, (void*)(offset + diff));

			glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
		}
#else
#define kQuadSize sizeof(quad.bl)
		long offset = (long)&quad;

		// vertex
		int diff = offsetof(cocos2d::V3F_C4B_T2F, vertices);
		glVertexAttribPointer(cocos2d::GLProgram::VERTEX_ATTRIB_POSITION, 3, GL_FLOAT, GL_FALSE, kQuadSize, (void*)(offset + diff));

		// texCoods
		diff = offsetof(cocos2d::V3F_C4B_T2F, texCoords);
		glVertexAttribPointer(cocos2d::GLProgram::VERTEX_ATTRIB_TEX_COORD, 2, GL_FLOAT, GL_FALSE, kQuadSize, (void*)(offset + diff));

		// color
		diff = offsetof(cocos2d::V3F_C4B_T2F, colors);
		glVertexAttribPointer(cocos2d::GLProgram::VERTEX_ATTRIB_COLOR, 4, GL_UNSIGNED_BYTE, GL_TRUE, kQuadSize, (void*)(offset + diff));

		glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
#endif

#define DRAW_DEBUG (0)
#if ( DRAW_DEBUG == 1 )
		// draw bounding box
		{
			cocos2d::Point vertices[4] = {
				cocos2d::Point(quad.tl.vertices.x,quad.tl.vertices.y),
				cocos2d::Point(quad.bl.vertices.x,quad.bl.vertices.y),
				cocos2d::Point(quad.br.vertices.x,quad.br.vertices.y),
				cocos2d::Point(quad.tr.vertices.x,quad.tr.vertices.y),
			};
			ccDrawPoly(vertices, 4, true);
		}
#elif ( DRAW_DEBUG == 2 )
		// draw texture box
		{
			cocos2d::Size s = this->getTextureRect().size;
			cocos2d::Point offsetPix = this->getOffsetPosition();
			cocos2d::Point vertices[4] = {
				cocos2d::Point(offsetPix.x,offsetPix.y), cocos2d::Point(offsetPix.x + s.width,offsetPix.y),
				cocos2d::Point(offsetPix.x + s.width,offsetPix.y + s.height), cocos2d::Point(offsetPix.x,offsetPix.y + s.height)
			};
			ccDrawPoly(vertices, 4, true);
		}
#endif // CC_SPRITE_DEBUG_DRAW

		CHECK_GL_ERROR_DEBUG();

		//レンダリングステートの保存
		_ssDrawState.texture = texture[tex_index]->getName();
		_ssDrawState.partType = sprite->_partData.type;
		_ssDrawState.partBlendfunc = state.blendfunc;
		_ssDrawState.partsColorFunc = state.partsColorFunc;
		_ssDrawState.partsColorType = state.partsColorType;
		_ssDrawState.partsColorUse = (int)ispartColor;
		_ssDrawState.maskInfluence = (int)sprite->_maskInfluence;

}


	void clearMask()
	{
		glClear(GL_STENCIL_BUFFER_BIT);
		enableMask(false);
	}

	void enableMask(bool flag)
	{

		if (flag)
		{
			glEnable(GL_STENCIL_TEST);
		}
		else {
			glDisable(GL_STENCIL_TEST);
			glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
		}
		_ssDrawState.maskInfluence = -1;		//マスクを実行する
		_ssDrawState.partType = -1;		//マスクを実行する
	}

	void execMask(CustomSprite *sprite)
	{
		if (
			(_ssDrawState.partType != sprite->_partData.type)
			|| (_ssDrawState.maskInfluence != (int)sprite->_maskInfluence)
		   )
		{
			glEnable(GL_STENCIL_TEST);
			if (sprite->_partData.type == PARTTYPE_MASK)
			{

				glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);

				//			cocos2d::Director::getInstance()->setDefaultValues

				if (!(sprite->_maskInfluence)) { //マスクが有効では無い＝重ね合わせる
					glStencilFunc(GL_ALWAYS, 1, ~0);  //常に通過
					glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);
					//描画部分を1へ
				}
				else {
					glStencilFunc(GL_ALWAYS, 1, ~0);  //常に通過
					glStencilOp(GL_KEEP, GL_KEEP, GL_INCR);
				}
#if OPENGLES20
#else
				glEnable(GL_ALPHA_TEST);
#endif
				//この設定だと
				//1.0fでは必ず抜けないため非表示フラグなし（＝1.0f)のときの挙動は考えたほうがいい

				//不透明度からマスク閾値へ変更
				float mask_alpha = (float)(255 - sprite->_state.masklimen) / 255.0f;
#if OPENGLES20
#else
				glAlphaFunc(GL_GREATER, mask_alpha);
#endif
				sprite->_state.Calc_opacity = 255;	//マスクパーツは不透明度1.0にする
			}
			else {

				if ((sprite->_maskInfluence)) //パーツに対してのマスクが有効か否か
				{
					glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
					glStencilFunc(GL_NOTEQUAL, 0x1, 0x1);  //1と等しい
					glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);
				}
				else {
					glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
					glDisable(GL_STENCIL_TEST);
				}

				// 常に無効
#if OPENGLES20
#else
				glDisable(GL_ALPHA_TEST);
#endif
			}
		}
	}

	/**
	* 文字コード変換
	*/ 
#if _WIN32
	std::string utf8Togbk(const char *src)
	{
		int len = MultiByteToWideChar(CP_UTF8, 0, src, -1, NULL, 0);
		unsigned short * wszGBK = new unsigned short[len + 1];
		memset(wszGBK, 0, len * 2 + 2);
		MultiByteToWideChar(CP_UTF8, 0, (LPCSTR)src, -1, (LPWSTR)wszGBK, len);

		len = WideCharToMultiByte(CP_ACP, 0, (LPCWSTR)wszGBK, -1, NULL, 0, NULL, NULL);
		char *szGBK = new char[len + 1];
		memset(szGBK, 0, len + 1);
		WideCharToMultiByte(CP_ACP, 0, (LPCWSTR)wszGBK, -1, szGBK, len, NULL, NULL);
		std::string strTemp(szGBK);
		if (strTemp.find('?') != std::string::npos)
		{
			strTemp.assign(src);
		}
		delete[]szGBK;
		delete[]wszGBK;
		return strTemp;
	}
#endif
	/**
	* windows用パスチェック
	*/ 
	bool isAbsolutePath(const std::string& strPath)
	{

#if _WIN32
		std::string strPathAscii = utf8Togbk(strPath.c_str());
#else
        std::string strPathAscii = strPath;
#endif
        if (strPathAscii.length() > 2
			&& ((strPathAscii[0] >= 'a' && strPathAscii[0] <= 'z') || (strPathAscii[0] >= 'A' && strPathAscii[0] <= 'Z'))
			&& strPathAscii[1] == ':')
		{
			return true;
		}
		return false;
	}

};
