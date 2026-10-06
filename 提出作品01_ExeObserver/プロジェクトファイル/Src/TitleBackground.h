#pragma once
#include "GameMain.h"
#include "Object3D.h"
#include "Sprite3D.h"

class TitleBackground : public Object3D {
public:
  TitleBackground() {
    m_image = new CSpriteImage(GameDevice()->m_pShader);
    m_image->Load("Data/Image/TitleBG.png");
    m_sprite = new CSprite();
    SetDrawOrder(10000); // Draw early (Background)
  }

  ~TitleBackground() {
    if (m_image)
      delete m_image;
    if (m_sprite)
      delete m_sprite;
  }

  void Draw() override {
    if (m_image && m_sprite) {
      // Use native size
      m_sprite->Draw(m_image, 0, 0, 0, 0, m_image->m_dwImageWidth,
                     m_image->m_dwImageHeight);
    }
  }

private:
  CSpriteImage *m_image;
  CSprite *m_sprite;
};
