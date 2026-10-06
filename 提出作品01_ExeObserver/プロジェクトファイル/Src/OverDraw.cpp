#include "OverDraw.h"
#include "GameMain.h"

OverDraw::OverDraw() {
  timer = 0;

  image = new CSpriteImage(GameDevice()->m_pShader);
  image->Load("Data/Image/Over.png");
  imageScore = new CSpriteImage(GameDevice()->m_pShader);
  imageScore->Load("Data/Image/ClearScore.png");
  sprite = new CSprite;
}

OverDraw::~OverDraw() {
  if (image)
    delete image;
  if (imageScore)
    delete imageScore;
  if (sprite)
    delete sprite;
}

void OverDraw::Update() {}

void OverDraw::Draw() {
  float sw = (float)WINDOW_WIDTH;
  float sh = (float)WINDOW_HEIGHT;

  if (image && sprite) {
    // Native size using m_dwImageWidth/Height
    sprite->Draw(image, 0, 0, 0, 0, image->m_dwImageWidth,
                 image->m_dwImageHeight);
  }

  // Using safe strings to avoid encoding issues
  GameDevice()->m_pFont->Draw(363, 63, "GAME OVER", 90, RGB(0, 0, 0), 1.0f,
                              "Arial");
  GameDevice()->m_pFont->Draw(360, 60, "GAME OVER", 90, RGB(255, 0, 0), 1.0f,
                              "Arial");

  if (imageScore && sprite) {
    float scoreW = (float)imageScore->m_dwImageWidth;
    float scoreH = (float)imageScore->m_dwImageHeight;
    // Center native size
    sprite->Draw(imageScore, sw / 2 - scoreW / 2, sh / 2 - scoreH / 2, 0, 0,
                 scoreW, scoreH, scoreW, scoreH);
  }

  timer += 60 * SceneManager::DeltaTime();
  if ((int)timer % 8 < 4) {
    GameDevice()->m_pFont->Draw(500, 700, "Continue [T] Key!!", 40,
                                RGB(255, 0, 0), 1.0f, "Arial");
  }
  if (timer > 8)
    timer = 0;
}