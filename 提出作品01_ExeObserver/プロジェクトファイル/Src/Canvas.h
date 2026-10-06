#pragma once
#include "CDrawUtil.h"
#include "MyMath.h"

class Canvas {
public:
  Canvas();
  ~Canvas();
  void Draw(const VECTOR2 &offset, float zoom);

private:
  CSpriteImage *canvasimage;
};