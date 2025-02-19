//
// > Notice: Amélie Heinrich @ 2025
// > Create Time: 2025-02-15 16:16:52
//

#include "PostProcessVolume.hpp"

#include <Core/File.hpp>

void PostProcessVolume::Load(const String& path)
{
    Path = path;

    nlohmann::json root = File::LoadJSON(path);

    if (root.contains("shadows")) {
        nlohmann::json s = root["shadows"];
        CascadeSplitLambda = s["lambda"].get<float>();
        VisualizeCascades = s["visualize"].get<bool>();
        FreezeCascades = s["freeze"].get<bool>();
    }

    if (root.contains("geometry")) {
        nlohmann::json g = root["geometry"];
        VisualizeMeshlets = g["visualizeMeshlets"].get<bool>();
        DirectLight = g["direct"].get<float>();
        IndirectLight = g["indirect"].get<float>();
    }

    if (root.contains("skybox")) {
        nlohmann::json s = root["skybox"];
        EnableSkybox = s["enable"].get<bool>();
    }
    
    if (root.contains("posterization")) {
        nlohmann::json p = root["posterization"];
        EnablePosterization = p["enable"];
        PosterizationLevels = p["levels"];
    }

    if (root.contains("colorGrading")) {
        nlohmann::json cg = root["colorGrading"];

        EnableColorGrading = cg["enable"].get<bool>();
        Brightness = cg["brightness"].get<float>();
        Exposure = cg["exposure"].get<float>();
        Saturation = cg["saturation"].get<float>();
        Contrast = cg["contrast"].get<float>();
        HueShift = cg["hueShift"].get<float>();
        Balance = cg["balance"].get<float>();
        Temperature = cg["temperature"].get<float>();
        Tint = cg["tint"].get<float>();
        
        Shadows = glm::vec4(
            cg["shadows"][0].get<float>(),
            cg["shadows"][1].get<float>(),
            cg["shadows"][2].get<float>(),
            cg["shadows"][3].get<float>()
        );
        Highlights = glm::vec4(
            cg["shadows"][0].get<float>(),
            cg["shadows"][1].get<float>(),
            cg["shadows"][2].get<float>(),
            cg["shadows"][3].get<float>()
        );
        ColorFilter = glm::vec4(
            cg["shadows"][0].get<float>(),
            cg["shadows"][1].get<float>(),
            cg["shadows"][2].get<float>(),
            cg["shadows"][3].get<float>()
        );
    }

    if (root.contains("depthOfField")) {
        nlohmann::json dof = root["depthOfField"];

        EnableDOF = dof["enable"].get<bool>();
        FocusRange = dof["focusRange"].get<float>();
        FocusPoint = dof["focusPoint"].get<float>();
    }

    if (root.contains("gammaCorrect")) {
        nlohmann::json gc = root["gammaCorrect"];

        GammaCorrection = gc["factor"].get<float>();
    }

    if (root.contains("pixelization")) {
        nlohmann::json p = root["pixelization"];
        EnablePixelization = p["enable"];
        PixelSize = p["size"];
    }

    if (root.contains("filmGrain")) {
        nlohmann::json f = root["filmGrain"];
        EnableFilmGrain = f["enable"];
        FilmGrainAmount = f["amount"];
    }
}

void PostProcessVolume::Save(const String& path)
{
    Path = path;

    nlohmann::json root;

    root["shadows"] = {
        { "lambda", CascadeSplitLambda },
        { "freeze", FreezeCascades },
        { "visualize", VisualizeCascades }
    };

    root["geometry"] = {
        { "visualizeMeshlets", VisualizeMeshlets },
        { "direct", DirectLight },
        { "indirect", IndirectLight }
    };

    root["skybox"] = {
        { "enable", EnableSkybox }
    };

    root["posterization"] = {
        { "enable", EnablePosterization },
        { "levels", PosterizationLevels }
    };

    root["colorGrading"] = {
        {"enable", EnableColorGrading},
        {"brightness", Brightness},
        {"exposure", Exposure},
        {"saturation", Saturation},
        {"contrast", Contrast},
        {"hueShift", HueShift},
        {"balance", Balance},
        {"temperature", Temperature},
        {"tint", Tint},
        {"shadows", {Shadows.x, Shadows.y, Shadows.z, Shadows.w}},
        {"highlights", {Highlights.x, Highlights.y, Highlights.z, Highlights.w}},
        {"colorFilter", {ColorFilter.x, ColorFilter.y, ColorFilter.z, ColorFilter.w}}
    };

    root["depthOfField"] = {
        {"enable", EnableDOF},
        {"focusRange", FocusRange},
        {"focusPoint", FocusPoint}
    };

    root["gammaCorrect"] = {
        {"factor", GammaCorrection}
    };

    root["pixelization"] = {
        { "enable", EnablePixelization },
        { "size", PixelSize }
    };

    root["filmGrain"] = {
        { "enable", EnableFilmGrain },
        { "amount", FilmGrainAmount }
    };

    File::WriteJSON(root, path);
}

