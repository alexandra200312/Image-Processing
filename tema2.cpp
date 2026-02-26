#include "lab_m2/tema2/tema2.h"

#include <vector>
#include <iostream>

#include "pfd/portable-file-dialogs.h"
#include <algorithm>


using namespace std;
using namespace m2;


/*
 *  To find out more about `FrameStart`, `Update`, `FrameEnd`
 *  and the order in which they are called, see `world.cpp`.
 */


Tema2::Tema2()
{
    outputMode = 0;
    gpuProcessing = false;
    saveScreenToImage = false;
    window->SetSize(600, 600);
    radiusSize = 3;
    threshold = 0.2;

    focusRadius = 100.0f;
    maxBlurRadius = 40.0f;
    mousePos = glm::vec2(300, 300);
    gpuProcessing = true;    

    blurEnabled = true;
    //blurEnabled = false;



}


Tema2::~Tema2()
{
}


void Tema2::Init()
{
    originalImage = TextureManager::LoadTexture(PATH_JOIN(window->props.selfDir, RESOURCE_PATH::TEXTURES, "cube", "pos_x.png"), nullptr, "image", true, true);
    processedImage = TextureManager::LoadTexture(PATH_JOIN(window->props.selfDir, RESOURCE_PATH::TEXTURES, "cube", "pos_x.png"), nullptr, "newImage", true, true);

    {
        Mesh* mesh = new Mesh("quad");
        mesh->LoadMesh(PATH_JOIN(window->props.selfDir, RESOURCE_PATH::MODELS, "primitives"), "quad.obj");
        mesh->UseMaterials(false);
        meshes[mesh->GetMeshID()] = mesh;
    }

    std::string shaderPath = PATH_JOIN(window->props.selfDir, SOURCE_PATH::M2, "tema2", "shaders");
    {
        Shader* shader = new Shader("ImagePass1");
        shader->AddShader(PATH_JOIN(shaderPath, "VertexShader.glsl"), GL_VERTEX_SHADER);
        shader->AddShader(PATH_JOIN(shaderPath, "FragmentPass1.glsl"), GL_FRAGMENT_SHADER);
        shader->CreateAndLink();
        shaders[shader->GetName()] = shader;
    }
    {
        Shader* shader = new Shader("ImagePass2");
        shader->AddShader(PATH_JOIN(shaderPath, "VertexShader.glsl"), GL_VERTEX_SHADER);
        shader->AddShader(PATH_JOIN(shaderPath, "FragmentPass2.glsl"), GL_FRAGMENT_SHADER);
        shader->CreateAndLink();
        shaders[shader->GetName()] = shader;
    }
    {
        Shader* shader = new Shader("ImageDisplay");

       

        shader->AddShader(PATH_JOIN(shaderPath, "VertexShader.glsl"), GL_VERTEX_SHADER);
        shader->AddShader(PATH_JOIN(shaderPath, "FragmentDisplay.glsl"), GL_FRAGMENT_SHADER);
        shader->CreateAndLink();
        shaders[shader->GetName()] = shader;
    }

}


void    Tema2::FrameStart()
{
    glClearColor(0.2, 0.2, 0.2, 1);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void Tema2::Update(float deltaTimeSeconds)
{
    ClearScreen();

    glm::ivec2 resolution = window->GetResolution();
    EnsureFBO(resolution.x, resolution.y);

    {
        glBindFramebuffer(GL_FRAMEBUFFER, fboPass1);
        glViewport(0, 0, resolution.x, resolution.y);
        glClearColor(0, 0, 0, 1);
        glClear(GL_COLOR_BUFFER_BIT);

        auto shader = shaders["ImagePass1"];
        shader->Use();

        glUniform2f(shader->GetUniformLocation("mousePos"), mousePos.x, mousePos.y);
        glUniform1f(shader->GetUniformLocation("focusRadius"), focusRadius);
        glUniform1f(shader->GetUniformLocation("maxBlurRadius"), maxBlurRadius);

        glUniform2i(shader->GetUniformLocation("screenSize"), resolution.x, resolution.y);
        glUniform1i(shader->GetUniformLocation("flipVertical"), saveScreenToImage ? 0 : 1);
        glUniform1i(shader->GetUniformLocation("doBlur"), blurEnabled ? 1 : 0);

        glUniform1i(shader->GetUniformLocation("textureImage"), 0);
        originalImage->BindToTextureUnit(GL_TEXTURE0);

        RenderMesh(meshes["quad"], shader, glm::mat4(1));
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }

    {
        glBindFramebuffer(GL_FRAMEBUFFER, fboPass2);
        glViewport(0, 0, resolution.x, resolution.y);
        glClearColor(0, 0, 0, 1);
        glClear(GL_COLOR_BUFFER_BIT);

        auto shader = shaders["ImagePass2"];
        shader->Use();

        glUniform2i(shader->GetUniformLocation("screenSize"), resolution.x, resolution.y);
        glUniform1f(shader->GetUniformLocation("maxBlurRadius"), maxBlurRadius);
        glUniform1i(shader->GetUniformLocation("flipVertical"), saveScreenToImage ? 0 : 1);
        glUniform1i(shader->GetUniformLocation("doBlur"), blurEnabled ? 1 : 0);

        glUniform1i(shader->GetUniformLocation("texBlurH"), 0);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, texPass1BlurH);

        glUniform1i(shader->GetUniformLocation("texInfo"), 1);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, texPass1Info);

        RenderMesh(meshes["quad"], shader, glm::mat4(1));
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }

    {
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glViewport(0, 0, resolution.x, resolution.y);

        auto shader = shaders["ImageDisplay"];
        shader->Use();

        glUniform2i(shader->GetUniformLocation("screenSize"), resolution.x, resolution.y);
        glUniform1i(shader->GetUniformLocation("outputMode"), outputMode);
        glUniform1i(shader->GetUniformLocation("flipVertical"), saveScreenToImage ? 0 : 1);

        glUniform2f(shader->GetUniformLocation("mousePos"), mousePos.x, mousePos.y);
        glUniform1f(shader->GetUniformLocation("focusRadius"), focusRadius);

        glUniform1i(shader->GetUniformLocation("texOriginal"), 0);
        originalImage->BindToTextureUnit(GL_TEXTURE0);

        glUniform1i(shader->GetUniformLocation("texPass1BlurH"), 1);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, texPass1BlurH);

        glUniform1i(shader->GetUniformLocation("texPass1Info"), 2);
        glActiveTexture(GL_TEXTURE2);
        glBindTexture(GL_TEXTURE_2D, texPass1Info);

        glUniform1i(shader->GetUniformLocation("texFinal"), 3);
        glActiveTexture(GL_TEXTURE3);
        glBindTexture(GL_TEXTURE_2D, texPass2Final);

        RenderMesh(meshes["quad"], shader, glm::mat4(1));
    }

    std::string title = "Bokeh Blur - Focus Radius: " + std::to_string((int)focusRadius);

    GLFWwindow* win = glfwGetCurrentContext();
    if (win)
        glfwSetWindowTitle(win, title.c_str());

}


void Tema2::FrameEnd()
{
    DrawCoordinateSystem();
}


void Tema2::OnFileSelected(const std::string &fileName)
{
    if (fileName.size())
    {
        std::cout << fileName << endl;
        originalImage = TextureManager::LoadTexture(fileName, nullptr, "image", true, true);
        processedImage = TextureManager::LoadTexture(fileName, nullptr, "newImage", true, true);

        float aspectRatio = static_cast<float>(originalImage->GetWidth()) / originalImage->GetHeight();
        window->SetSize(static_cast<int>(600 * aspectRatio), 600);
    }
}

void Tema2::SaveImage(const std::string &fileName)
{
    cout << "Saving image! ";
    processedImage->SaveToFile((fileName + ".png").c_str());
    cout << "[Done]" << endl;
}


void Tema2::OpenDialog()
{
    std::vector<std::string> filters =
    {
        "Image Files", "*.png *.jpg *.jpeg *.bmp",
        "All Files", "*"
    };

    auto selection = pfd::open_file("Select a file", ".", filters).result();
    if (!selection.empty())
    {
        std::cout << "User selected file " << selection[0] << "\n";
        OnFileSelected(selection[0]);
    }
}


/*
 *  These are callback functions. To find more about callbacks and
 *  how they behave, see `input_controller.h`.
 */


void Tema2::OnInputUpdate(float deltaTime, int mods)
{
    // Treat continuous update based on input
}


void Tema2::OnKeyPress(int key, int mods)
{
    
    if (key == GLFW_KEY_F || key == GLFW_KEY_ENTER || key == GLFW_KEY_SPACE)
    {
        OpenDialog();
        return;
    }

    if (key >= GLFW_KEY_0 && key <= GLFW_KEY_4)
    {
        outputMode = key - GLFW_KEY_0;
        return;
    }

    if (key == GLFW_KEY_KP_ADD || key == GLFW_KEY_EQUAL)
    {
        focusRadius += 10.0f;
        return;
    }

    if (key == GLFW_KEY_KP_SUBTRACT || key == GLFW_KEY_MINUS)
    {
        
        focusRadius -= 10.0f;
        if (focusRadius < 10.0f)
            focusRadius = 10.0f;

        return;
    }
    if (key == GLFW_KEY_S && (mods & GLFW_MOD_CONTROL))
    {
        saveScreenToImage = true;
        return;
    }
}


void Tema2::OnKeyRelease(int key, int mods)
{
    // Add key release event
   

}


void Tema2::OnMouseMove(int mouseX, int mouseY, int deltaX, int deltaY)
{
    // Add mouse move event
    mousePos = glm::vec2(
        (float)mouseX,
        (float)(window->GetResolution().y - mouseY)
    );
}


void Tema2::OnMouseBtnPress(int mouseX, int mouseY, int button, int mods)
{
    if (button == GLFW_MOUSE_BUTTON_LEFT)
        blurEnabled = !blurEnabled;
}


void Tema2::OnMouseBtnRelease(int mouseX, int mouseY, int button, int mods)
{
    // Add mouse button release event
}


void Tema2::OnMouseScroll(int mouseX, int mouseY, int offsetX, int offsetY)
{
    // Treat mouse scroll event
}


void Tema2::OnWindowResize(int width, int height)
{
    // Treat window resize event
}
GLuint Tema2::CreateColorTexture(int w, int h, GLenum internalFormat)
{
    GLuint tex;
    glGenTextures(1, &tex);
    glBindTexture(GL_TEXTURE_2D, tex);

    glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    glBindTexture(GL_TEXTURE_2D, 0);
    return tex;
}

void Tema2::DestroyFBOs()
{
    if (texPass1BlurH) glDeleteTextures(1, &texPass1BlurH), texPass1BlurH = 0;
    if (texPass1Info)  glDeleteTextures(1, &texPass1Info), texPass1Info = 0;
    if (texPass2Final) glDeleteTextures(1, &texPass2Final), texPass2Final = 0;

    if (fboPass1) glDeleteFramebuffers(1, &fboPass1), fboPass1 = 0;
    if (fboPass2) glDeleteFramebuffers(1, &fboPass2), fboPass2 = 0;

    fboW = fboH = 0;
}

void Tema2::EnsureFBO(int w, int h)
{
    if (w <= 0 || h <= 0) return;
    if (fboW == w && fboH == h && fboPass1 && fboPass2) return;

    DestroyFBOs();
    fboW = w; fboH = h;

    texPass1BlurH = CreateColorTexture(w, h, GL_RGBA8);
    texPass1Info = CreateColorTexture(w, h, GL_RGBA8);
    texPass2Final = CreateColorTexture(w, h, GL_RGBA8);

    glGenFramebuffers(1, &fboPass1);
    glBindFramebuffer(GL_FRAMEBUFFER, fboPass1);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texPass1BlurH, 0);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, texPass1Info, 0);

    GLenum bufs1[2] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1 };
    glDrawBuffers(2, bufs1);

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        std::cout << "[FBO] Pass1 incomplete!\n";

    glGenFramebuffers(1, &fboPass2);
    glBindFramebuffer(GL_FRAMEBUFFER, fboPass2);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texPass2Final, 0);

    GLenum bufs2[1] = { GL_COLOR_ATTACHMENT0 };
    glDrawBuffers(1, bufs2);

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        std::cout << "[FBO] Pass2 incomplete!\n";

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}
