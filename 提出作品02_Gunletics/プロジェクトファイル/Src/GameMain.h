//
//		ゲームメイン処理		                           ver 3.3        2024.5.20
//

#pragma once

//ヘッダーファイルのインクルード
#include <stdio.h>
#include <windows.h>
#include <vector>

#include "Main.h"
#include "Direct3D.h"
#include "Shader.h"
#include "XAudio.h"
#include "Dinput.h"
#include "Sprite.h"
#include "MyImgui.h"
#include "Macro.h"
#include <map>
#include <string>

// --------------------------------------------
class CMain;
class CDirect3D;
class CXAudio;
class CDirectInput;
class CFontTexture;
class CShader;

// --------------------------------------------------------------------------------------------------
//  ゲームメインクラス
// --------------------------------------------------------------------------------------------------
class CGameMain
{
public:
	// メンバ変数
	CMain*			m_pMain;		// メインクラス
	CDirect3D*		m_pD3D;			//	Direct3Dオブジェクト
	CXAudio*		m_pXAudio;		//	XAudioオブジェクト
	CDirectInput*	m_pDI;			//	DirectInputDオブジェクト
	CFontTexture*	m_pFont;		//	フォントテクスチャー
	CShader*		m_pShader;		//	シェーダー

	VECTOR2			Scroll;			// スクロール位置

public:
	// メソッド
	CGameMain(CMain* pMain);
	~CGameMain();

	HRESULT Init();
	void    Update();
	void	Draw();
	void    Quit();

	void    GameMain();
	HRESULT ChangeScreenMode(int nMode=-1);    // -- 2020.1.15

	std::map<std::string, CSpriteImage*> m_spriteImages;

	CSpriteImage* GetSpriteImage(const char* filename);
};

CGameMain* GameDevice();
