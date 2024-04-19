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

	/// @brief ����һ��Key�п����ж������ ���Է�����ʼ���������� ��Ϊ�����ʱ����Ǵ���ʼ����������ʼ��
	/// ��������һ����ֵ����� ����glActiveTexture����ѭ���� ��˼���˳���޹� û��Ҫ��Ҫ����AddTexture��˳���ȡ����Ӧ����������
	/// ����gbuffer��brdf gbuffer�ȼ��� brdf�Ǻ����� ����ԭ�� gbuffer����ʼ������0 brdf����ʼ������4
	/// ��������map������ṹ gbuffer����ʼ������1 brdf����ʼ������0 �����ȴ���ǶԵ�
	/// �����unordered_map(����ṹ��map) ��ʱҲ�Ǻõ� ������ӷ����Ⱥ���������˳��
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
		
		// ���sKey��Ӧ�������ǵ�һ��ֵ �����ñ��� iResult = 0 ������ʼ��������Ϊ0
		for (auto iter = m_mapTextureCache.begin(); iter != temp; ++iter)
		{
			iResult = iResult + iter->second.size();
		}

		return iResult;
	}
}
