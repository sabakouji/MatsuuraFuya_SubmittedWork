#include "AudioManager.h"
#include "GameMain.h"
#include <algorithm>

namespace {
	struct AudioObject {
		CXAudioSource* audioSource;
		std::string name;
		AudioObject() : audioSource(nullptr) {}
	};
	std::list<AudioObject> audioObjects;
};

void AudioManager::Start()
{
	//   オーディオ別名   オーディオファイル名      重ね合わせ再生数
	Push("SeShot"  ,_T("Data/Sound/M_FIRE4.wav"), 2);
	Push("SeLaser" ,_T("Data/Sound/Lazer.wav"), 2);
	Push("SeDamage",_T("Data/Sound/Dead.wav"));
	Push("SeNitro" ,_T("Data/Sound/Nitro2.wav"), 2);

	Push("BGM_Stage"    ,_T("Data/Sound/BGM_Stage.wav"));
	Push("BGM_Editor", _T("Data/Sound/BGM_Editor.wav"));
	Push("BGM_Lobby", _T("Data/Sound/BGM_Lobby_Lofi.mp3"));

	Push("GameOver", _T("Data/Sound/GameOver.mp3"));
	Push("EnemyShot", _T("Data/Sound/EnemyMecha_Shot.mp3"));
	Push("Crick", _T("Data/Sound/Crick.wav"));
}

void AudioManager::Release()
{
	for (auto it = audioObjects.begin(); it != audioObjects.end();) {
		AudioObject& node = *it;
		SAFE_DELETE( node.audioSource);
		it = audioObjects.erase(it);
	}
	audioObjects.clear();
}

void AudioManager::Push(const std::string name, const TCHAR* audioName, int n)
{
	for (AudioObject& obj : audioObjects) {
		if (obj.name == name)
		{
			MessageBox(0, _T("オーディオ別名がダブっています!"), name.c_str(), MB_OK);
			return;
		}
	}
	AudioObject node;
	node.audioSource = new CXAudioSource(audioName, n);
	node.name = name;
	audioObjects.emplace_back(node);
}

bool AudioManager::Delete( std::string name)
{
	bool ret = false;
	for (auto it = audioObjects.begin(); it != audioObjects.end();) {
		AudioObject& node = *it;
		if (node.name == name)
		{
			SAFE_DELETE(node.audioSource);
			it = audioObjects.erase(it);
			ret = true;
			//return ret;
		}
		else {
			it++;
		}
	}
	return ret;
}

 CXAudioSource* AudioManager::Audio( std::string name)
{
	 for (AudioObject& obj : audioObjects) {
		 if (obj.name == name) return obj.audioSource;
	 }
	 MessageBox(0, _T("オーディオソースがありません!"), name.c_str(), MB_OK);
	 assert(false);
	 return nullptr;
 }

 // 指定のオーディオを停止する
 void AudioManager::Stop(std::string name)
 {
	 for (AudioObject& obj : audioObjects) {
		 if (obj.name == name) {
			 if (obj.audioSource) {
				 obj.audioSource->Stop();
			 }
			 return;
		 }
	 }
 }

 // すべてのオーディオを停止する
 void AudioManager::StopAll()
 {
	 for (AudioObject& obj : audioObjects) {
		 if (obj.audioSource) {
			 obj.audioSource->Stop();
		 }
	 }
 }