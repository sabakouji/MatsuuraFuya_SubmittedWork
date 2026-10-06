#pragma once
#include <string>

namespace ResourceManager {
	void Init();
	void Reset();
	unsigned char* Load(std::string filename);
	int Size(std::string filename);
};