/**
  \file minimalOpenGL/main.cpp
  \author Morgan McGuire, http://graphics.cs.williams.edu
  Distributed with the G3D Innovation Engine http://g3d.cs.williams.edu

  Features demonstrated:
   * Window, OpenGL, and extension initialization
   * Triangle mesh rendering (GL Vertex Array Buffer)
   * Texture map loading (GL Texture Object)
   * Shader loading (GL Program and Shader Objects)
   * Fast shader argument binding (GL Uniform Buffer Objects)
   * Offscreen rendering / render-to-texture (GL Framebuffer Object)
   * Ray tracing
   * Procedural texture
   * Tiny vector math library
   * Mouse and keyboard handling

  This is a minimal example of an OpenGL 4 program using only
  GLFW and GLEW libraries to simplify initialization. It does
  not depend on G3D or any other external libraries at all. 
  You could use SDL or another thin library instead of those two.
  If you want to use VR, this also requires the OpenVR library.
  (All dependencies are included with G3D)
  
  This is useful as a testbed when isolating driver bugs and 
  seeking a minimal context. 

  It is also helpful if you're new to computer graphics and wish to
  see the underlying hardware API without the high-level features that
  G3D provides.

  I chose OpenGL 4.1 because it is the newest OpenGL available on OS
  X, and thus the newest OpenGL that can be used across the major PC
  operating systems of Windows, Linux, OS X, and Steam.

  If you're interested in other minimal graphics code for convenience,
  also look at the stb libraries for single-header, dependency-free support
  for image loading, parsing, fonts, noise, etc.:
     https://github.com/nothings/stb

  And a SDL-based minimal OpenGL program at:
     https://gist.github.com/manpat/112f3f31c983ccddf044
  
  Reference Frames:
      Object: The object being rendered (the Shape in this example) relative to its own origin
      World:  Global reference frame
      Body:   Controlled by keyboard and mouse
      Head:   Controlled by tracking (or fixed relative to the body for non-VR)
      Camera: Fixed relative to the head. The camera is the eye.
 */

// Uncomment to add VR support
//#define _VR

////////////////////////////////////////////////////////////////////////////////

#include "matrix.h"
#include "minimalOpenGL.h"
#include <AntTweakBar.h>
#include "MeshComponent.h"
#include "helper\MatrixConvertions.h"
#include "Leap.h"
#include "LeapHandler.h"
#include "VCModels.h"

#ifdef _VR
#   include "minimalOpenVR.h"
#endif

GLFWwindow* window = nullptr;

VCText2D *helloText = nullptr;
VCCh3D *chH = nullptr;
VCCh3D *sphereModel = nullptr;
VCPSModel *headModel = nullptr;
VCPSModel *stickModel = nullptr;
VCPSModel *dollModel = nullptr;
Sky *sky = nullptr;
SphereSky *sphereSky = nullptr;
// break the sphere at (-1, 0, 0), to make the seam gone
SkySphere *skySphere = nullptr;

#ifdef _VR
    vr::IVRSystem* hmd = nullptr;
#endif

#ifndef Shape
#   define Shape Cube
#endif

TwBar *bar;         // Pointer to a tweak bar
inline void TwEventMouseButtonGLFW3(GLFWwindow* window, int button, int action, int mods) { TwEventMouseButtonGLFW(button, action); }

inline void TwEventMousePosGLFW3(GLFWwindow* window, double xpos, double ypos) { TwMouseMotion(int(xpos), int(ypos)); }

inline void TwEventMouseWheelGLFW3(GLFWwindow* window, double xoffset, double yoffset) { TwEventMouseWheelGLFW(yoffset); }

inline void TwEventKeyGLFW3(GLFWwindow* window, int key, int scancode, int action, int mods) { TwEventKeyGLFW(key, action); }

inline void TwEventCharGLFW3(GLFWwindow* window, int codepoint) { TwEventCharGLFW(codepoint, GLFW_PRESS); }


int main(const int argc, const char* argv[]) {
    std::cout << "Minimal OpenGL 4.3 Example by Morgan McGuire\n\nW, A, S, D, C, Z keys to translate\nMouse click and drag to rotate\nESC to quit\n\n";
    std::cout << std::fixed;

    uint32_t framebufferWidth = 1280, framebufferHeight = 720;
#   ifdef _VR
        const int numEyes = 2;
        hmd = initOpenVR(framebufferWidth, framebufferHeight);
        assert(hmd);
#   else
        const int numEyes = 1;
#   endif

    const int windowHeight = 720;
    const int windowWidth = (framebufferWidth * windowHeight) / framebufferHeight;


    window = initOpenGL(windowWidth, windowHeight, "minimalOpenGL");

	// Send the new window size to AntTweakBar
	TwWindowSize(windowWidth, windowHeight);
	// Initialize AntTweakBar
	TwInit(TW_OPENGL_CORE, NULL);

	double lastTime = glfwGetTime();
	float dt = 0.0016f;
	double turn = 0;    // Model turn counter
	double speed = 0.3; // Model rotation speed
	int wire = 0;       // Draw model in wireframe?
	float bgColor[] = { 0.1f, 0.2f, 0.4f };         // Background color 
	unsigned char cubeColor[] = { 255, 0, 0, 128 }; // Model color (32bits RGBA)
													// Create a tweak bar
	bar = TwNewBar("TweakBar");
	TwDefine(" TweakBar position='250 20' "); // move bar to position (200, 40)
	TwDefine(" TweakBar alpha=255 "); // opaque bar
	TwDefine(" GLOBAL fontsize=3 "); // use large font
	TwDefine(" GLOBAL fontstyle=default "); // use fixed-width font
	TwDefine(" GLOBAL help='This example shows how to integrate AntTweakBar with GLFW and OpenGL.' "); // Message added to the help bar.

																									   // Add 'speed' to 'bar': it is a modifable (RW) variable of type TW_TYPE_DOUBLE. Its key shortcuts are [s] and [S].
	TwAddVarRW(bar, "speed", TW_TYPE_DOUBLE, &speed,
		" label='Rot speed' min=0 max=2 step=0.01 keyIncr=s keyDecr=S help='Rotation speed (turns/second)' ");

	// Add 'wire' to 'bar': it is a modifable variable of type TW_TYPE_BOOL32 (32 bits boolean). Its key shortcut is [w].
	TwAddVarRW(bar, "wire", TW_TYPE_BOOL32, &wire,
		" label='Wireframe mode' key=w help='Toggle wireframe display mode.' ");

	// Add 'time' to 'bar': it is a read-only (RO) variable of type TW_TYPE_DOUBLE, with 1 precision digit
	TwAddVarRO(bar, "time", TW_TYPE_DOUBLE, &turn, " label='Time' precision=1 help='Time (in seconds).' ");

	// Add 'bgColor' to 'bar': it is a modifable variable of type TW_TYPE_COLOR3F (3 floats color)
	TwAddVarRW(bar, "bgColor", TW_TYPE_COLOR3F, &bgColor, " label='Background color' ");

	// Add 'cubeColor' to 'bar': it is a modifable variable of type TW_TYPE_COLOR32 (32 bits color) with alpha
	TwAddVarRW(bar, "cubeColor", TW_TYPE_COLOR32, &cubeColor,
		" label='Cube color' alpha help='Color and transparency of the cube.' ");

	glfwSetMouseButtonCallback(window, (GLFWmousebuttonfun)TwEventMouseButtonGLFW3);
	glfwSetCursorPosCallback(window, (GLFWcursorposfun)TwEventMousePosGLFW3);
	glfwSetScrollCallback(window, (GLFWscrollfun)TwEventMouseWheelGLFW3);
	glfwSetKeyCallback(window, (GLFWkeyfun)TwEventKeyGLFW3);
	glfwSetCharCallback(window, (GLFWcharfun)TwEventCharGLFW3);

    std::string _objPath{ "assets/quad.obj" };
    std::map<std::string, GLenum> _shaderPaths;
    _shaderPaths["shaders/simple_model.vert"] = GL_VERTEX_SHADER;
    _shaderPaths["shaders/simple_model.frag"] = GL_FRAGMENT_SHADER;
    std::vector<std::string> _uniformNames = { "MVP", "leappos", "enhanced_texture" };
    std::string _texPath{ "assets/hello.png" };
	std::string _secondaryTexPath{ "assets/hello_enhanced.png" };
    helloText = new VCText2D(_objPath, _shaderPaths, _uniformNames, _texPath);
	helloText->setEnhancedTexture(_secondaryTexPath);
    ENV_VAR.scene.push_back(helloText);
    helloText->translate(glm::vec3(0.f, 3.f, 0.f));

    _shaderPaths.clear();
    _shaderPaths["shaders/sky.vert"] = GL_VERTEX_SHADER;
    _shaderPaths["shaders/sky.frag"] = GL_FRAGMENT_SHADER;
    _uniformNames = { "light", "resolution", "cameraToWorldMatrix", "invProjectionMatrix" };
    sky = new Sky(_shaderPaths, _uniformNames);

    _objPath = std::string("assets/text_H.obj");
    _shaderPaths.clear();
    _shaderPaths["shaders/Ch3D.vert"] = GL_VERTEX_SHADER;
    _shaderPaths["shaders/Ch3D.frag"] = GL_FRAGMENT_SHADER;
    // uniform names "diffuse", "specular", "shininess", "emmissive", "ambient" in shader
    // are used exclusively for data read from .mtl file if corresponding options 
    // are turned on. More details in the definition of class VCWVObjModel
    _uniformNames = { "MVP", "ModelMat", "normalMat", "camPos", "lightPos",
        "diffuse", "specular", "shininess" };
    chH = new VCCh3D(_objPath, _shaderPaths, _uniformNames);
    ENV_VAR.scene.push_back(chH);
    chH->translate(glm::vec3(0.f, 3.f, -5.f));

    _objPath = std::string("assets/sphere.obj");
    sphereModel = new VCCh3D(_objPath, _shaderPaths, _uniformNames);
    ENV_VAR.scene.push_back(sphereModel);
    sphereModel->translate(glm::vec3(3.f, 0.f, 1.f));
	sphereModel->setScaleFactor(glm::vec3(0.1));

    ENV_VAR.envMap.load("assets/envMap.jpg");
    _shaderPaths.clear();
    _shaderPaths["shaders/sphere_sky.vert"] = GL_VERTEX_SHADER;
    _shaderPaths["shaders/sphere_sky.tes"] = GL_TESS_EVALUATION_SHADER;
    _shaderPaths["shaders/sphere_sky.frag"] = GL_FRAGMENT_SHADER;
    _uniformNames = {"MVP"};
    sphereSky = new SphereSky(_shaderPaths, _uniformNames, _objPath);
    ENV_VAR.scene.push_back(sphereSky);
    sphereSky->scale(glm::vec3(30));

    _shaderPaths.clear();
    _shaderPaths["shaders/sphere_sky.vert"] = GL_VERTEX_SHADER;
    _shaderPaths["shaders/sky_sphere.tes"] = GL_TESS_EVALUATION_SHADER;
    _shaderPaths["shaders/sphere_sky.frag"] = GL_FRAGMENT_SHADER;
    skySphere = new SkySphere(_shaderPaths, _uniformNames);
    ENV_VAR.scene.push_back(skySphere);
    skySphere->scale(glm::vec3(60));

// #define HEAD_MODEL
#define STICK_MODEL
// #define DOLL_MODEL

    _objPath = std::string("assets/head_full_tex.obj");
    _shaderPaths.clear();
    _shaderPaths["shaders/ps_model.vert"] = GL_VERTEX_SHADER;
    _shaderPaths["shaders/ps_model.frag"] = GL_FRAGMENT_SHADER;
    _uniformNames = { "MVP", "ModelMat", "normalMat", "camPos", "lightPos", "leapPos" };

#ifdef HEAD_MODEL
    headModel = new VCPSModel(_objPath, _shaderPaths, _uniformNames, ".png");
    ENV_VAR.scene.push_back(headModel);
    headModel->scale(glm::vec3(5.f));
    headModel->translate(glm::vec3(-3.f, 0.f, 0.f));
#endif

    _objPath = std::string("assets/stick_1_low.obj");

#ifdef STICK_MODEL
    stickModel = new VCPSModel(_objPath, _shaderPaths, _uniformNames, ".jpg");
	std::string _epath{ "assets/stick_1_low_enhanced.jpg" };
	stickModel->setEnhancedTexture(_epath);
    ENV_VAR.scene.push_back(stickModel);
    stickModel->scale(glm::vec3(5.f));
    stickModel->rotate(M_PI / 3.5f, glm::vec3(1.f, 0.f, 0.f));
    stickModel->translate(glm::vec3(0.f, 0.f, -4.f));
#endif

    _objPath = std::string("assets/doll.obj");

#ifdef DOLL_MODEL
    dollModel = new VCPSModel(_objPath, _shaderPaths, _uniformNames, ".jpg");
    ENV_VAR.scene.push_back(dollModel);
    dollModel->scale(glm::vec3(5.f));
    dollModel->translate(glm::vec3(1.f, 2.f, 0.f));
    dollModel->rotate(-M_PI, glm::vec3(1.f, 0.f, 0.f));
#endif

    Vector3 bodyTranslation(0.0f, 1.6f, 5.0f);
    Vector3 bodyRotation;

	Leap::Controller controller;

    //////////////////////////////////////////////////////////////////////
    // Allocate the frame buffer. This code allocates one framebuffer per eye.
    // That requires more GPU memory, but is useful when performing temporal 
    // filtering or making render calls that can target both simultaneously.

    GLuint framebuffer[numEyes];
    glGenFramebuffers(numEyes, framebuffer);

    GLuint colorRenderTarget[numEyes], depthRenderTarget[numEyes];
    glGenTextures(numEyes, colorRenderTarget);
    glGenTextures(numEyes, depthRenderTarget);
    for (int eye = 0; eye < numEyes; ++eye) {
        glBindTexture(GL_TEXTURE_2D, colorRenderTarget[eye]);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, framebufferWidth, framebufferHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);

        glBindTexture(GL_TEXTURE_2D, depthRenderTarget[eye]);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT24, framebufferWidth, framebufferHeight, 0, GL_DEPTH_COMPONENT, GL_UNSIGNED_INT, nullptr);

        glBindFramebuffer(GL_FRAMEBUFFER, framebuffer[eye]);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, colorRenderTarget[eye], 0);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT,  GL_TEXTURE_2D, depthRenderTarget[eye], 0);
    }
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

#   ifdef _VR
        vr::TrackedDevicePose_t trackedDevicePose[vr::k_unMaxTrackedDeviceCount];
#   endif

    // Main loop:
    while (! glfwWindowShouldClose(window)) 
	{
        assert(glGetError() == GL_NONE);

        const float nearPlaneZ = -0.1f;
        const float farPlaneZ = -1000.0f;
        const float verticalFieldOfView = 45.0f * PI / 180.0f;

        Matrix4x4 eyeToHead[numEyes], projectionMatrix[numEyes], headToBodyMatrix;
#       ifdef _VR
            getEyeTransformations(hmd, trackedDevicePose, nearPlaneZ, farPlaneZ, headToBodyMatrix.data, eyeToHead[0].data, eyeToHead[1].data, projectionMatrix[0].data, projectionMatrix[1].data);
#       else
            projectionMatrix[0] = Matrix4x4::perspective(float(framebufferWidth), float(framebufferHeight), nearPlaneZ, farPlaneZ, verticalFieldOfView);
#       endif

        // printf("float nearPlaneZ = %f, farPlaneZ = %f; int width = %d, height = %d;\n", nearPlaneZ, farPlaneZ, framebufferWidth, framebufferHeight);

        const Matrix4x4& bodyToWorldMatrix = 
            Matrix4x4::translate(bodyTranslation) *
            Matrix4x4::roll(bodyRotation.z) *
            Matrix4x4::yaw(bodyRotation.y) *
            Matrix4x4::pitch(bodyRotation.x);

        const Matrix4x4& headToWorldMatrix = bodyToWorldMatrix * headToBodyMatrix;



		Leap::Vector palmVelocity = getPalmVelocity(controller.frame());
		Leap::Vector palmPosition = getPalmPosition(controller.frame());

		// used as origin for Leap coordinate system. 
		glm::vec3 leapOffset = { 0.f, 0.f, -2.f }; //for Stick
		//glm::vec3 leapOffset = { 0.f, 3.f, -5.f };
		float leapHandDistance = 100.f; //Added before scaling
		float leapScale = 80.f; //the larger the scale, the slower the gesture
		glm::vec3 scaledPos = glm::vec3(palmPosition.x / leapScale, ((palmPosition.y - leapHandDistance) / leapScale), palmPosition.z / leapScale);
		scaledPos += leapOffset;
		//scaledPos.y -= leapHandDistance;

		/*pMesh->setLeapPosition(glm::vec3(palmPosition.x, palmPosition.y, palmPosition.z));
		pMesh->rotate(glm::vec3(palmVelocity.x, palmVelocity.y, palmVelocity.z));
		sphere->setLeapPosition(glm::vec3(palmPosition.x, palmPosition.y, palmPosition.z));
		sphere->moveTo(glm::vec3(palmPosition.x, palmPosition.y, palmPosition.z));*/


       // helloText->setLeapPosition(glm::vec3(palmPosition.x, palmPosition.y, palmPosition.z));
		// update the scene
		//pMesh->update(dt);
        //helloText->update(dt);
        chH->update(dt);
        // sphereModel->update(dt); // no visual effect
        // headModel->update(dt);
        // stickModel->update(dt);
        // dollModel->update(dt);

		glm::vec4 viewDirWS = Matrix4x4ToGLM(headToWorldMatrix) * glm::vec4(0.0f, 0.0f, 100.0f, 1.0f);
		//pMesh->alignToCamera(glm::vec3(viewDirWS.x, viewDirWS.y, viewDirWS.z), glm::vec3(0.0f, 1.0f, 0.0f));
        glm::vec3 headPos(Matrix4x4ToGLM(headToWorldMatrix) * glm::vec4(0.f, 0.f, 0.f, 1.f));
        glm::vec3 camUp(glm::vec4(0.f, 1.f, 0.f, 0.f));
        helloText->alignToCamera(headPos, camUp);
		// Draw the scene twice; for both eyes
        for (int eye = 0; eye < numEyes; ++eye) 
		{
            const Matrix4x4& cameraToWorldMatrix = headToWorldMatrix * eyeToHead[eye];

            const Vector3& light = Vector3(1.0f, 0.5f, 0.2f).normalize();

            const Matrix4x4 viewProjectionMatrix4x4 = projectionMatrix[eye] * cameraToWorldMatrix.inverse();
            glm::mat4 viewProjectionMatrix = Matrix4x4ToGLM(viewProjectionMatrix4x4);

            glm::mat4 _viewMat = Matrix4x4ToGLM(cameraToWorldMatrix.inverse());
            glm::mat4 _view2WorldMat = Matrix4x4ToGLM(cameraToWorldMatrix);
            glm::mat4 _projMat = Matrix4x4ToGLM(projectionMatrix[eye]);

            ENV_VAR.camPos = glm::vec3(_view2WorldMat * glm::vec4(0.f, 0.f, 0.f, 1.f));
            ENV_VAR.projMat = _projMat;
            ENV_VAR.viewMat = _viewMat;

            glBindFramebuffer(GL_FRAMEBUFFER, framebuffer[eye]);
            glViewport(0, 0, framebufferWidth, framebufferHeight);

            glClearColor(0.1f, 0.2f, 0.3f, 0.0f);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

            // Draw the background
            // drawSky(framebufferWidth, framebufferHeight, nearPlaneZ, farPlaneZ, cameraToWorldMatrix.data, projectionMatrix[eye].inverse().data, &light.x);

			// Draw the mesh
            glDepthRange(0, 0.9);
            assert(glGetError() == GL_NONE);
            //chH->draw();
            // sphereSky->draw();
            skySphere->draw();
#ifdef HEAD_MODEL
            headModel->draw();
#endif

#ifdef STICK_MODEL
			//stickModel->setLeapPosition(glm::vec3(palmPosition.x, palmPosition.y, palmPosition.z));
			stickModel->setLeapPosition(scaledPos);
            stickModel->draw();
#endif

#ifdef DOLL_MODEL
            dollModel->draw();
#endif




			//sphereModel->setLeapPosition(glm::vec3(palmPosition.x, palmPosition.y, palmPosition.z));
			sphereModel->setTranslation(scaledPos);
            sphereModel->draw();

            assert(glGetError() == GL_NONE);

            glDepthRange(0.9, 1.0);
            // Using glDepthRange to force the background stay behind other objects
            // it's really a creepy sky
            // sky->draw(framebufferWidth, framebufferHeight, cameraToWorldMatrix.data, projectionMatrix[eye].inverse().data, &light.x);

            glDepthRange(0.0, 0.9);
            // transparent objects should be draw at last, from back to front
            helloText->draw();
            glDepthRange(0.0, 1.0);

			// Draw the Anttweakbar UI
			// TwDraw();

#           ifdef _VR
            {
                const vr::Texture_t tex = { reinterpret_cast<void*>(intptr_t(colorRenderTarget[eye])), vr::API_OpenGL, vr::ColorSpace_Gamma };
                vr::VRCompositor()->Submit(vr::EVREye(eye), &tex);
            }
#           endif
        } // for each eye

        ////////////////////////////////////////////////////////////////////////
#       ifdef _VR
            // Tell the compositor to begin work immediately instead of waiting for the next WaitGetPoses() call
            vr::VRCompositor()->PostPresentHandoff();
#       endif

        // Mirror to the window
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, GL_NONE);
        glViewport(0, 0, windowWidth, windowHeight);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glBlitFramebuffer(0, 0, framebufferWidth, framebufferHeight, 0, 0, windowWidth, windowHeight, GL_COLOR_BUFFER_BIT, GL_LINEAR);
        glBindFramebuffer(GL_READ_FRAMEBUFFER, GL_NONE);


        // Display what has been drawn on the main window
        glfwSwapBuffers(window);

        // Check for events
        glfwPollEvents();

        // Handle events
        if (GLFW_PRESS == glfwGetKey(window, GLFW_KEY_ESCAPE)) {
            glfwSetWindowShouldClose(window, 1);
        }

        // WASD keyboard movement
        const float cameraMoveSpeed = 0.01f;
        if (GLFW_PRESS == glfwGetKey(window, GLFW_KEY_W)) { bodyTranslation += Vector3(headToWorldMatrix * Vector4(0, 0, -cameraMoveSpeed, 0)); }
        if (GLFW_PRESS == glfwGetKey(window, GLFW_KEY_S)) { bodyTranslation += Vector3(headToWorldMatrix * Vector4(0, 0, +cameraMoveSpeed, 0)); }
        if (GLFW_PRESS == glfwGetKey(window, GLFW_KEY_A)) { bodyTranslation += Vector3(headToWorldMatrix * Vector4(-cameraMoveSpeed, 0, 0, 0)); }
        if (GLFW_PRESS == glfwGetKey(window, GLFW_KEY_D)) { bodyTranslation += Vector3(headToWorldMatrix * Vector4(+cameraMoveSpeed, 0, 0, 0)); }
        if (GLFW_PRESS == glfwGetKey(window, GLFW_KEY_C)) { bodyTranslation.y -= cameraMoveSpeed; }
        if ((GLFW_PRESS == glfwGetKey(window, GLFW_KEY_SPACE)) || (GLFW_PRESS == glfwGetKey(window, GLFW_KEY_Z))) { bodyTranslation.y += cameraMoveSpeed; }
        if ((GLFW_PRESS == glfwGetKey(window, GLFW_KEY_F5))) {
            for (auto i : ENV_VAR.scene) {
                i->initShaderProg();
            }
        }
        // Keep the camera above the ground
        if (bodyTranslation.y < 0.01f) { bodyTranslation.y = 0.01f; }

        static bool inDrag = false;
        const float cameraTurnSpeed = 0.005f;
        if (GLFW_PRESS == glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT)) {
            static double startX, startY;
            double currentX, currentY;

            glfwGetCursorPos(window, &currentX, &currentY);
            if (inDrag) {
                bodyRotation.y -= float(currentX - startX) * cameraTurnSpeed;
                bodyRotation.x -= float(currentY - startY) * cameraTurnSpeed;
            }
            inDrag = true; startX = currentX; startY = currentY;
        } else {
            inDrag = false;
        }

		/* Timer and fps */
		double currentTime = glfwGetTime();
		dt = (float)(currentTime - lastTime);
		lastTime = currentTime;
    }

#   ifdef _VR
        if (hmd != nullptr) {
            vr::VR_Shutdown();
        }
#   endif

    delete sphereSky;

	// Terminate AntTweakBar and GLFW
	TwTerminate();

    // Close the GL context and release all resources
    glfwTerminate();

    return 0;
}


#ifdef _WINDOWS
    int WINAPI WinMain(HINSTANCE hInst, HINSTANCE hPrev, LPSTR szCmdLine, int sw) {
        return main(0, nullptr);
    }
#endif
