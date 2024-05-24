#include "ui.h"

namespace Cme
{
	void UI::SetupUI(GLFWwindow* window)
	{
		IMGUI_CHECKVERSION();
		ImGui::CreateContext();
		ImGuiIO& io = ImGui::GetIO();
		ImGui_ImplGlfw_InitForOpenGL(window, true);
		ImGui_ImplOpenGL3_Init("#version 460 core");
	}

	void UI::RenderUI(ModelRenderOptions& opts, Cme::Camera camera)
	{
        ImGui::Begin("Model Render");

        constexpr float IMAGE_BASE_SIZE = 160.0f;

        if (ImGui::CollapsingHeader("Model", ImGuiTreeNodeFlags_DefaultOpen))
        {
            // Perform some shenanigans so that the gizmo rotates along with the
            // camera while still representing the same model rotation.
            glm::quat rotViewSpace = glm::quat_cast(camera.getViewTransform()) * opts.modelRotation;
            ImGui::gizmo3D("Model rotation", rotViewSpace, IMAGE_BASE_SIZE);
            opts.modelRotation = glm::quat_cast(glm::inverse(camera.getViewTransform())) * glm::normalize(rotViewSpace);

            ImGui::SameLine();

            // Perform some shenanigans so that the gizmo rotates along with the
            // camera while still representing the same light dir..
            glm::vec3 dirViewSpace = glm::vec3(camera.getViewTransform() * glm::vec4(opts.directionalDirection, 0.0f));
            ImGui::gizmo3D("Light dir", dirViewSpace, IMAGE_BASE_SIZE);
            opts.directionalDirection =
                glm::vec3(glm::inverse(camera.getViewTransform()) *
                    glm::vec4(glm::normalize(dirViewSpace), 0.0f));
            ImGui::SliderFloat3("Light dir",
                reinterpret_cast<float*>(&opts.directionalDirection),
                -1.0f, 1.0f);

            if (ImGui::Button("Reset rotation"))
            {
                opts.modelRotation = glm::identity<glm::quat>();
            }
            CommonHelper::imguiFloatSlider("Model scale", &opts.modelScale, 0.0001f, 100.0f, "%.04f",
                Scale::LOG);
        }

        ImGui::Separator();

        // Create a child so that this section can scroll separately.
        ImGui::BeginChild("MainOptions");

        if (ImGui::CollapsingHeader("Rendering"))
        {
            ImGui::Combo("Lighting model", reinterpret_cast<int*>(&opts.lightingModel),
                "Blinn-Phong\0Cook-Torrance GGX\0\0");
            ImGui::SameLine();
            CommonHelper::imguiHelpMarker("Which lighting model to use for shading.");

            ImGui::Separator();
            if (ImGui::TreeNode("Directional light"))
            {
                static bool lockSpecular = true;
                ImGui::ColorEdit3("Diffuse color",
                    reinterpret_cast<float*>(&opts.directionalDiffuse),
                    ImGuiColorEditFlags_Float | ImGuiColorEditFlags_HDR);
                ImGui::BeginDisabled(lockSpecular);
                ImGui::ColorEdit3("Specular color",
                    reinterpret_cast<float*>(&opts.directionalSpecular),
                    ImGuiColorEditFlags_Float | ImGuiColorEditFlags_HDR);
                ImGui::EndDisabled();
                ImGui::Checkbox("Lock specular", &lockSpecular);
                ImGui::SameLine();
                CommonHelper::imguiHelpMarker("Whether to lock the specular light color to the diffuse. Usually "
                    "desired for PBR.");
                if (lockSpecular)
                {
                    opts.directionalSpecular = opts.directionalDiffuse;
                }
                CommonHelper::imguiFloatSlider("Intensity", &opts.directionalIntensity, 0.0f, 50.0f, nullptr, Scale::LINEAR);

                ImGui::TreePop();
            }

            if (ImGui::TreeNode("Emission lights"))
            {
                CommonHelper::imguiFloatSlider("Emission intensity", &opts.emissionIntensity, 0.0f,
                    1000.0f, nullptr, Scale::LOG);
                ImGui::DragFloat3("Emission attenuation",
                    reinterpret_cast<float*>(&opts.emissionAttenuation),
                    /*v_speed=*/0.01f, 0.0f, 10.0f);
                ImGui::SameLine();
                CommonHelper::imguiHelpMarker("Constant, linear, and quadratic attenuation of emission lights.");

                ImGui::TreePop();
            }

            if (ImGui::TreeNode("Environment"))
            {
                ImGui::Combo("Skybox image", reinterpret_cast<int*>(&opts.skyboxImage),
                    "Alex's apt\0Frozen waterfall\0Kloppenheim\0Milkyway\0Mon "
                    "Valley\0Ueno shrine\0Winter forest\0Six_Face\0");

                ImGui::BeginDisabled(opts.lightingModel == LightingModel::BLINN_PHONG);
                ImGui::Checkbox("Use IBL", &opts.useIBL);
                ImGui::EndDisabled();

                ImGui::BeginDisabled(opts.lightingModel != LightingModel::BLINN_PHONG && opts.useIBL);
                ImGui::ColorEdit3("Ambient color",
                    reinterpret_cast<float*>(&opts.ambientColor),
                    ImGuiColorEditFlags_Float | ImGuiColorEditFlags_HDR);
                ImGui::SameLine();
                CommonHelper::imguiHelpMarker("The color of the fixed ambient component.");
                ImGui::EndDisabled();

                ImGui::BeginDisabled(opts.lightingModel != LightingModel::BLINN_PHONG);
                CommonHelper::imguiFloatSlider("Shininess", &opts.shininess, 1.0f, 1000.0f, nullptr,
                    Scale::LOG);
                ImGui::SameLine();
                CommonHelper::imguiHelpMarker("Shininess of specular highlights. Only applies to Phong.");
                ImGui::EndDisabled();

                ImGui::TreePop();
            }

            if (ImGui::TreeNode("Post-processing"))
            {
                ImGui::Combo(
                    "Tone mapping", reinterpret_cast<int*>(&opts.toneMapping),
                    "None\0Reinhard\0Reinhard luminance\0ACES (approx)\0AMD\0\0");
                ImGui::Checkbox("Gamma correct", &opts.gammaCorrect);
                ImGui::BeginDisabled(!opts.gammaCorrect);
                CommonHelper::imguiFloatSlider("Gamma", &opts.gamma, 0.01f, 8.0f, nullptr, Scale::LOG);

                ImGui::EndDisabled();

                ImGui::Checkbox("FXAA", &opts.fxaa);

                ImGui::TreePop();
            }
        }

        if (ImGui::CollapsingHeader("Camera"))
        {
            ImGui::RadioButton("Fly controls",
                reinterpret_cast<int*>(&opts.cameraControlType),
                static_cast<int>(CameraControlType::FLY));
            ImGui::SameLine();
            ImGui::RadioButton("Orbit controls",
                reinterpret_cast<int*>(&opts.cameraControlType),
                static_cast<int>(CameraControlType::ORBIT));

            CommonHelper::imguiFloatSlider("Speed", &opts.speed, 0.1, 50.0);
            CommonHelper::imguiFloatSlider("Sensitivity", &opts.sensitivity, 0.01, 1.0, nullptr, Scale::LOG);
            CommonHelper::imguiFloatSlider("FoV", &opts.fov, Cme::MIN_FOV, Cme::MAX_FOV, "%.1f°");
            if (CommonHelper::imguiFloatSlider("Near plane", &opts.near, 0.01, 1000.0, nullptr,
                Scale::LOG))
            {
                if (opts.near > opts.far)
                {
                    opts.far = opts.near;
                }
            }
            if (CommonHelper::imguiFloatSlider("Far plane", &opts.far, 0.01, 1000.0, nullptr, Scale::LOG))
            {
                if (opts.far < opts.near)
                {
                    opts.near = opts.far;
                }
            }

            ImGui::Checkbox("Capture mouse", &opts.captureMouse);
        }

        if (ImGui::CollapsingHeader("Debug"))
        {
            ImGui::Combo(
                "G-Buffer vis", reinterpret_cast<int*>(&opts.gBufferVis),
                "Disabled\0Positions\0Ambient "
                "occlusion\0Normals\0Roughness\0Albedo\0Metallic\0Emission\0\0");
            ImGui::SameLine();
            CommonHelper::imguiHelpMarker("What component of the G-Buffer to visualize.");

            ImGui::Checkbox("Wireframe", &opts.wireframe);
            ImGui::Checkbox("Draw vertex normals", &opts.drawNormals);
        }

        if (ImGui::CollapsingHeader("Performance"))
        {
            char overlay[32];
            sprintf_s(overlay, "Avg FPS %.02f", opts.avgFPS);
            ImGui::PlotLines("Frame time", opts.frameDeltas, opts.numFrameDeltas,
                opts.frameDeltasOffset, overlay, 0.0f, 0.03f,
                ImVec2(0, 80.0f));

            ImGui::Checkbox("Enable VSync", &opts.enableVsync);
        }

        // 粒子编辑 位置 速度 形状
        if (ImGui::CollapsingHeader("ParticleSystem"))
        {
            // 引用可直接修改粒子颜色
            ImGui::ColorEdit3("Particle color", reinterpret_cast<float*>(&opts.vec3ParticleColor), ImGuiColorEditFlags_Float | ImGuiColorEditFlags_HDR);
        }

        // 爱心颜色
        if (ImGui::CollapsingHeader("Love"))
        {
            ImGui::Separator();
            // 修改粗细
            CommonHelper::imguiFloatSlider("Love Thickness", &opts.fLoveThickness, 0.05f, 0.2f, nullptr, Scale::LINEAR);
            ImGui::ColorEdit3("Light Position", reinterpret_cast<float*>(&opts.vec3LoveLightPosition), ImGuiColorEditFlags_Float | ImGuiColorEditFlags_HDR);
            ImGui::ColorEdit3("Light Ambient", reinterpret_cast<float*>(&opts.vec3LoveLightAmbient), ImGuiColorEditFlags_Float | ImGuiColorEditFlags_HDR);
            ImGui::ColorEdit3("Light Diffuse", reinterpret_cast<float*>(&opts.vec3LoveLightDiffuse), ImGuiColorEditFlags_Float | ImGuiColorEditFlags_HDR);
            ImGui::ColorEdit3("Light Specular", reinterpret_cast<float*>(&opts.vec3LoveLightSpecular), ImGuiColorEditFlags_Float | ImGuiColorEditFlags_HDR);
            ImGui::ColorEdit3("Material Ambient", reinterpret_cast<float*>(&opts.vec3LoveMaterialAmbient), ImGuiColorEditFlags_Float | ImGuiColorEditFlags_HDR);
            ImGui::ColorEdit3("Material Diffuse", reinterpret_cast<float*>(&opts.vec3LoveMaterialDiffuse), ImGuiColorEditFlags_Float | ImGuiColorEditFlags_HDR);
            ImGui::ColorEdit3("Material Specular", reinterpret_cast<float*>(&opts.vec3LoveMaterialSpecular), ImGuiColorEditFlags_Float | ImGuiColorEditFlags_HDR);
        }

        ImGui::EndChild();

        ImGui::End();
        ImGui::Render();
	}
}
