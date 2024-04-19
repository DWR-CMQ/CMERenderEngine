#include "texture_manager.h"

namespace Cme
{
	TextureManager& TextureManager::GetInstance()
	{
		static TextureManager tm;
		return tm;
	}

	void TextureManager::AddTexture(const std::string& sKey, std::vector<std::shared_ptr<Texture>> vecTextures)
	{
		if (ContainTexture(sKey))
		{
			return;
		}
		m_mapTextureCache[sKey] = std::move(vecTextures);
	}

	std::vector<std::shared_ptr<Texture>> TextureManager::GetTexture(const std::string& sKey) const
	{
		if (!ContainTexture(sKey))
		{
			auto msg = "Attempting to get non-existing texture with key '" + sKey + "'!";
			throw std::runtime_error(msg.c_str());
		}
		return m_mapTextureCache.at(sKey);
	}

	bool TextureManager::ContainTexture(const std::string& sKey) const
	{
		return m_mapTextureCache.count(sKey) > 0;
	}

	void TextureManager::ClearTexture()
	{
		m_mapTextureCache.clear();
	}

	/// @brief 由于一个Key中可能有多个纹理 所以返回起始的纹理索引 因为激活的时候就是从起始纹理索引开始的
	/// 这里碰到一个奇怪的现象 由于glActiveTexture是在循环里 因此激活顺序无关 没必要非要按照AddTexture的顺序获取到对应的纹理索引
	/// 比如gbuffer和brdf gbuffer先加入 brdf是后加入的 按照原理 gbuffer的起始纹理是0 brdf的起始纹理是4
	/// 但是由于map的有序结构 gbuffer的起始纹理是1 brdf的起始纹理是0 但结果却还是对的
	/// 如果用unordered_map(无序结构的map) 此时也是好的 这个更加符合先后添加纹理的顺序
	/// @param sKey 
	/// @return 
	int TextureManager::GetTextureUnit(std::string sKey) const
	{
		if (!ContainTexture(sKey))
		{
			std::cout << "No Key in m_mapTextureCache!" << std::endl;
			return 0;
		}

		int iResult = 0;
		auto temp = m_mapTextureCache.find(sKey);
		
		// 如果sKey对应的正好是第一个值 都不用遍历 iResult = 0 符合起始纹理索引为0
		for (auto iter = m_mapTextureCache.begin(); iter != temp; ++iter)
		{
			iResult = iResult + iter->second.size();
		}

		return iResult;
	}
}
