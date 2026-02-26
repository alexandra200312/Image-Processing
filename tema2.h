#pragma once

#include <string>

#include "components/simple_scene.h"
#include "core/gpu/frame_buffer.h"


namespace m2
{
    class Tema2 : public gfxc::SimpleScene
    {
     public:
        Tema2();
        ~Tema2();

        void Init() override;

     private:
        void FrameStart() override;
        void Update(float deltaTimeSeconds) override;
        void FrameEnd() override;

        void OnInputUpdate(float deltaTime, int mods) override;
        void OnKeyPress(int key, int mods) override;
        void OnKeyRelease(int key, int mods) override;
        void OnMouseMove(int mouseX, int mouseY, int deltaX, int deltaY) override;
        void OnMouseBtnPress(int mouseX, int mouseY, int button, int mods) override;
        void OnMouseBtnRelease(int mouseX, int mouseY, int button, int mods) override;
        void OnMouseScroll(int mouseX, int mouseY, int offsetX, int offsetY) override;
        void OnWindowResize(int width, int height) override;

        void OpenDialog();
        void OnFileSelected(const std::string &fileName);

        // Processing effects
        void GrayScale();
        void Blur();
        void SaveImage(const std::string &fileName);

     private:
        int radiusSize;
        float threshold;
        Texture2D *originalImage;
        Texture2D *processedImage;

        int outputMode;
        bool gpuProcessing;
        bool saveScreenToImage;

        glm::vec2 mousePos;
        float focusRadius;
        float maxBlurRadius;
        // FBOs + textures
        GLuint fboPass1 = 0;
        GLuint fboPass2 = 0;

        GLuint texPass1BlurH = 0;   // color attachment 0 (horizontal blur)
        GLuint texPass1Info = 0;   // color attachment 1 (original rgb + blurAmount in A)
        GLuint texPass2Final = 0;   // color attachment (final blur)

        int fboW = 0, fboH = 0;

        // click-to-process
        bool blurEnabled = false;

        // helper
        void EnsureFBO(int w, int h);
        GLuint CreateColorTexture(int w, int h, GLenum internalFormat);
        void DestroyFBOs();

    };
}   // namespace m2
