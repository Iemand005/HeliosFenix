#pragma once
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX

#include <algorithm>
#include <string>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <imgui/imgui.h>
#include <imgui/backends/imgui_impl_sdl3.h>
#include <imgui/backends/imgui_impl_opengl3.h>

#include <EditableGame.hpp>
#include <Object.hpp>

#include "Meatball.hpp"

class Helios : public fe::EditableGame {
public:

	bool showDebugUI = false;

	std::vector<Meatball> meatballs;
	std::shared_ptr<fe::Object> meatballObject;

	// Marching-cubes grid: cubes per side, half-extent of the box, iso level.
	// The surface sits where field() == knLevel (0.5 → blob radius ≈ 1.4·radius).
	static constexpr int   knRes    = 48;
	static constexpr float knExtent = 90.0f;
	static constexpr float knLevel  = 0.5f;

	// Balls bounce inside this box (kept well clear of the grid walls so a
	// swimming blob never leaves the volume).
	static constexpr float kBounceBox = 45.0f;

	// The width/height constructor defaults to the OpenGL render device, so we
	// can boot a blank window with embedded shaders and zero resource files.
	Helios(int width = 1280, int height = 720, bool vr = false) : fe::EditableGame(width, height, vr, true) {

		SetClearColor(1.05f, 0.05f, 0.08f);

		LoadShaderTexts(kVertexShader, kFragmentShader);

		renderDevice->EnableDepthTest();

		// This shader is lit entirely by point lights; park one big soft lamp
		// high above the scene so the blob is nicely shaded.
		auto* lights = scene->GetLights();
		lights[0].position = glm::vec3(0.0f, 120.0f, 220.0f);
		lights[0].color = glm::vec3(1.0f);
		lights[0].intensity = 1.0f;
		lights[0].radius = 400.0f;

		// A handful of meatballs swimming inside a marching-cubes grid.
		// Each blob's surface sits at field() == level; the fused iso-surface
		// of all of them is remeshed every frame as they fly around.
		{
			Meatball ball;
			ball.center = glm::vec3(-14.0f, 6.0f, 0.0f);
			ball.radius = 30.0f;
			ball.velocity = glm::vec3(22.0f, 8.0f, -14.0f);
			meatballs.push_back(ball);

			ball.center = glm::vec3(18.0f, -4.0f, 6.0f);
			ball.radius = 24.0f;
			ball.velocity = glm::vec3(-18.0f, 12.0f, 20.0f);
			meatballs.push_back(ball);

			ball.center = glm::vec3(6.0f, 14.0f, -10.0f);
			ball.radius = 20.0f;
			ball.velocity = glm::vec3(10.0f, -16.0f, 24.0f);
			meatballs.push_back(ball);

			ball.center = glm::vec3(-4.0f, -12.0f, 16.0f);
			ball.radius = 26.0f;
			ball.velocity = glm::vec3(-24.0f, 6.0f, -10.0f);
			meatballs.push_back(ball);

			ball.center = glm::vec3(0.0f, 0.0f, -18.0f);
			ball.radius = 18.0f;
			ball.velocity = glm::vec3(14.0f, -6.0f, 12.0f);
			meatballs.push_back(ball);
		}

		meatballObject = std::make_shared<fe::Object>();
		meatballObject->name = "Meatball";
		meatballObject->color = glm::vec3(0.25f, 0.65f, 1.0f);
		meatballObject->PushMesh(Meatball::MakeMesh(meatballs, knRes, knExtent, knLevel));
		this->scene->AddObject(meatballObject);

		// Park the camera so the blob is in frame.
		camera->SetPos(glm::vec3(0.0f, 70.0f, 180.0f));
		camera->LookAt(glm::vec3(0.0f));
	}

	void ProcessInput() {
		SDL_Event event;
		fe::SDLWindow* window = GetWindow<fe::SDLWindow>();
		while (window->PollSDLEvent(&event)) {
			ImGui_ImplSDL3_ProcessEvent(&event);
			auto io = ImGui::GetIO();
			switch (event.type) {
				case SDL_EVENT_QUIT:
					window->PrepareClose();
					break;
				case SDL_EVENT_MOUSE_BUTTON_DOWN:
					if (event.button.button == SDL_BUTTON_LEFT && !io.WantCaptureMouse) {
						window->StartMouseCapture();
					}
					break;
				case SDL_EVENT_KEY_DOWN:
					if (event.key.key == SDLK_F11) {
						window->ToggleFullscreen();
					}
					else if (event.key.key == SDLK_F3) {
						showDebugUI = !showDebugUI;
					}
					break;
			}
		}

		if (ImGui::GetIO().WantCaptureMouse) window->StopMouseCapture();
	}

	void AnimateMeatballs(float dt) {
		if (dt <= 0.0f) dt = 1.0f / 60.0f;

		for (auto& ball : meatballs) {
			ball.center += ball.velocity * dt;
			for (int axis = 0; axis < 3; ++axis) {
				if (ball.center[axis] >  kBounceBox) { ball.center[axis] =  kBounceBox; ball.velocity[axis] = -glm::abs(ball.velocity[axis]); }
				if (ball.center[axis] < -kBounceBox) { ball.center[axis] = -kBounceBox; ball.velocity[axis] =  glm::abs(ball.velocity[axis]); }
			}
		}

		// Remesh the whole fused blob surface around the current positions.
		meatballObject->meshes.clear();
		meatballObject->PushMesh(Meatball::MakeMesh(meatballs, knRes, knExtent, knLevel));
	}

	void Run() {
		auto window = GetWindow<fe::SDLWindow>();
		window->Show();
		camera->SetAspect(camera->aspect);

		while (!window->ShouldClose()) {
			ProcessInput();
			AnimateMeatballs(static_cast<float>(scene->GetDeltaTime()));
			Redraw();
		}

		Destroy();
	}

	void InitUI() override {}

	void DrawUI() override {
		if (!showDebugUI) return;
		BeginFrame();
		DrawDebugUI();
		EndFrame();
	}

	// Same convention as the standard engine shaders (resources/shaders/*.glsl):
	// they just describe the world and the engine uploads the uniforms below.
	static constexpr const char* kVertexShader = R"glsl(#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoord;

out vec3 Normal;
out vec3 FragPos;
out vec2 TexCoord;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform float time;

void main()
{
    vec4 worldPos = model * vec4(aPos, 1.0);
    gl_Position = projection * view * worldPos;

    mat3 normalMat = transpose(inverse(mat3(model)));
    Normal = normalMat * aNormal;
    FragPos = worldPos.xyz;
    TexCoord = aTexCoord;
}
)glsl";

	static constexpr const char* kFragmentShader = R"glsl(#version 330 core
out vec4 FragColor;

in vec3 Normal;
in vec3 FragPos;
in vec2 TexCoord;

struct PointLight {
    vec3 position;
    vec3 color;
    float intensity;
    float radius;
};

uniform int lightCount;
uniform PointLight pointLights[8];
uniform vec3 objectColor;

void main()
{
    vec3 n = normalize(Normal);
    vec3 lighting = vec3(0.1);

    for (int i = 0; i < lightCount; ++i) {
        vec3 L = pointLights[i].position - FragPos;
        float dist = length(L);
        if (dist < 0.0001) continue;
        vec3 ldir = L / dist;
        float diff = max(dot(n, ldir), 0.0);
        float radius = max(pointLights[i].radius, 0.001);
        float atten = 1.0 / (1.0 + (dist * dist) / (radius * radius));
        vec3 contrib = pointLights[i].color * pointLights[i].intensity * diff * atten;
        lighting += contrib;
    }

    FragColor = vec4(objectColor * lighting, 1.0);
}
)glsl";
};