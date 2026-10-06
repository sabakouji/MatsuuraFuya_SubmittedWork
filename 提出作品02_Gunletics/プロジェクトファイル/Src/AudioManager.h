#pragma once
/// <summary>
/// すべてのオーディオオブジェクトを管理します
/// シングルトンにしている
/// </summary>
/// <author>T.Suzuki</author>

#include <list>
#include <string>
#include "XAudio.h"

/// <summary>
/// オーディオマネージャー
/// (オーディオの管理を行う)
/// </summary>
namespace AudioManager {
	/// <summary>
	/// オーディオマネージャーの開始処理
	/// </summary>
	void Start();

	/// <summary>
	/// オーディオマネージャーの終了処理
	///  （audiolistを全て削除する）
	/// </summary>
	void Release();

	/// <summary>
	///  audiolistを追加する
	/// </summary>
	/// <param name="name">オーディオ別名</param>
	/// <param name="audioName">オーディオファイル名</param>
	/// <param name="n">重ね合わせ再生数。省略値１</param>
	void Push(const std::string name, const TCHAR* audioName, int n = 1);

	/// <summary>
	/// audioObjects からオーディオ別名をキーにしてオーディオソースを探す
	/// </summary>
	/// <param name="name">オーディオ別名</param>
	/// <returns>オーディオソース</returns>
	CXAudioSource* Audio(std::string name);

	/// <summary>
	/// audioObjects からオーディオ別名をキーにしてオーディオソースを削除する
	/// </summary>
	/// <param name="name">オーディオ別名</param>
	bool Delete(std::string name);
};
