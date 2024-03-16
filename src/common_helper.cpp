#include "common_helper.h"

namespace Cme
{
    void CommonHelper::LoadSkyboxImage(
        SkyboxImage skyboxImage, Cme::SkyboxMesh& skybox,
        Cme::EquirectCubemapConverter& equirectCubemapConverter,
        Cme::CubemapIrradianceCalculator& irradianceCalculator,
        Cme::GGXPrefilteredEnvMapCalculator& prefilteredEnvMapCalculator)
    {
        std::string hdrPath;
        switch (skyboxImage)
        {
        case SkyboxImage::ALEXS_APT:
            hdrPath = "assets//models//ibl/AlexsApt.hdr";
            break;
        case SkyboxImage::FROZEN_WATERFALL:
            hdrPath = "assets//models//ibl//FrozenWaterfall.hdr";
            break;
        case SkyboxImage::KLOPPENHEIM:
            hdrPath = "assets//models//ibl//Kloppenheim.hdr";
            break;
        case SkyboxImage::MILKYWAY:
            hdrPath = "assets//models//ibl//Milkyway.hdr";
            break;
        case SkyboxImage::MON_VALLEY:
            hdrPath = "assets//models//ibl//MonValley.hdr";
            break;
        case SkyboxImage::UENO_SHRINE:
            hdrPath = "assets//models//ibl//UenoShrine.hdr";
            break;
        case SkyboxImage::WINTER_FOREST:
            hdrPath = "assets//models//ibl//WinterForest.hdr";
            break;
        }

        Cme::Texture hdr = Cme::Texture::loadHdr(hdrPath.c_str());

        // Process HDR cubemap
        {
            Cme::DebugGroup debugGroup("HDR equirect to cubemap");
            equirectCubemapConverter.multipassDraw(hdr);
        }
        auto cubemap = equirectCubemapConverter.getCubemap();
        {
            Cme::DebugGroup debugGroup("Irradiance calculation");
            irradianceCalculator.multipassDraw(cubemap);
        }
        {
            Cme::DebugGroup debugGroup("Prefiltered env map calculation");
            prefilteredEnvMapCalculator.multipassDraw(cubemap);
        }

        skybox.setTexture(cubemap);

        // Don't need this anymore.
        hdr.free();
    }
}