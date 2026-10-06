#include "SceneBase.h"
#include "Sprite3D.h"
#include <string>
#include <vector>

class StageSelectScene : public SceneBase {
public:
  StageSelectScene();
  ~StageSelectScene();
  void Update() override;
  void Draw() override;

private:
  int m_selectIndex;
  int m_scrollOffset;

  // Execution Confirmation
  bool m_confirmExecute;
  bool m_isCrickSEPlayed;
  int m_selectedStageIndex;

  // Sprites
  CSpriteImage *m_bgImage;            // Keep as base or replace
  CSpriteImage *m_bgSelectImage;      // StageSelect_BackGround.png
  CSpriteImage *m_btnStageImage;      // StageSelect_Button.png
  CSpriteImage *m_btnStageCheckImage; // StageSelect_Button_CheckMark.png
  CSpriteImage *m_btnAIEditImage;     // AIEditor.png (Upper Right)

  // Dialog (Keep existing)
  CSpriteImage *m_tabImage;
  CSpriteImage *m_btnYesImage;
  CSpriteImage *m_btnNoImage;
  CSprite *m_sprite;

  // Stage List
  struct StageData {
    std::string name;
    std::string scriptPath;
    int sortOrder;
  };
  std::vector<StageData> m_stages;
};
