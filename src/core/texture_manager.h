#pragma once

#include <string>
#include <map>
#include <memory>
#include <iostream>
#include <unordered_map>

#include "texture.h"

namespace Cme
{
	class TextureManager
	{
	public:
		static TextureManager& GetInstance();

		void AddTexture(const std::string& sKey, std::vector<std::shared_ptr<Texture>> vecTextures);
		std::vector<std::shared_ptr<Texture>> GetTexture(const std::string& sKey) const;
		bool ContainTexture(const std::string& sKey) const;
		int GetTextureUnit(std::string sKey) const;

		void ClearTexture();

	private:
		TextureManager() {};
		TextureManager(const TextureManager&) = delete; // ������������
		void operator=(const TextureManager&) = delete; // ����������=

		std::unordered_map<std::string, std::vector<std::shared_ptr<Texture>>> m_mapTextureCache;
		int m_iTextureUnit = 0;    // ����Ԫ ����glActiveTextureʱʹ��
	};
}


