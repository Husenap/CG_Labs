#include "assignment5.hpp"

#include "config.hpp"
#include "core/Bonobo.h"
#include "core/FPSCamera.h"
#include "core/ShaderProgramManager.hpp"
#include "core/helpers.hpp"
#include "core/node.hpp"

#include <imgui.h>
#include <tinyfiledialogs.h>

#include "libtetris.h"

#include <clocale>
#include <stdexcept>

static constexpr uint32_t COLOR_BLACK = 0;
static constexpr uint32_t COLOR_CYAN = 0xffff00;
static constexpr uint32_t COLOR_ORANGE = 0x0099ff;
static constexpr uint32_t COLOR_YELLOW = 0x00ffff;
static constexpr uint32_t COLOR_RED = 0x0000ff;
static constexpr uint32_t COLOR_MAGENTA = 0xff00ff;
static constexpr uint32_t COLOR_BLUE = 0xff0000;
static constexpr uint32_t COLOR_GREEN = 0x00ff00;
static constexpr uint32_t COLOR_GHOST = 0x5e5e5e;
static constexpr uint32_t COLOR_BORDER = 0x333333;

uint32_t get_color(int8_t v) {
  uint32_t color = COLOR_BLACK;
  switch (v) {
  case PIECE_I:
    color = COLOR_CYAN;
    break;
  case PIECE_L:
    color = COLOR_ORANGE;
    break;
  case PIECE_O:
    color = COLOR_YELLOW;
    break;
  case PIECE_Z:
    color = COLOR_RED;
    break;
  case PIECE_T:
    color = COLOR_MAGENTA;
    break;
  case PIECE_J:
    color = COLOR_BLUE;
    break;
  case PIECE_S:
    color = COLOR_GREEN;
    break;
  case PIECE_GHOST:
    color = COLOR_GHOST;
    break;
  case PIECE_EMPTY:
  default:
    color = COLOR_BLACK;
    break;
  }
  return color;
}

edaf80::Assignment5::Assignment5(WindowManager &windowManager)
    : mCamera(0.5f * glm::half_pi<float>(),
              static_cast<float>(config::resolution_x) /
                  static_cast<float>(config::resolution_y),
              0.01f, 1000.0f),
      inputHandler(), mWindowManager(windowManager), window(nullptr) {
  WindowManager::WindowDatum window_datum{inputHandler,
                                          mCamera,
                                          config::resolution_x,
                                          config::resolution_y,
                                          0,
                                          0,
                                          0,
                                          0};

  window = mWindowManager.CreateGLFWWindow("EDAF80: Assignment 5", window_datum,
                                           config::msaa_rate);
  if (window == nullptr) {
    throw std::runtime_error("Failed to get a window: aborting!");
  }

  bonobo::init();
}

edaf80::Assignment5::~Assignment5() { bonobo::deinit(); }

void edaf80::Assignment5::run() {
  // Set up the camera
  mCamera.mWorld.SetTranslate(glm::vec3(0.0f, 2.5f, 7.0f));
  mCamera.mMouseSensitivity = glm::vec2(0.003f);
  mCamera.mMovementSpeed = glm::vec3(3.0f); // 3 m/s => 10.8 km/h

  // Create the shader programs
  ShaderProgramManager program_manager;
  GLuint default_shader = 0u;
  program_manager.CreateAndRegisterProgram(
      "Default",
      {{ShaderType::vertex, "EDAF80/default.vert"},
       {ShaderType::fragment, "EDAF80/default.frag"}},
      default_shader);
  if (default_shader == 0u) {
    LogError("Failed to load fallback shader");
    return;
  }

  //
  // Todo: Load your geometry
  //
  const auto arcade_machine_meshes = bonobo::loadObjects(
      config::resources_path("../assets/arcade-machine.fbx"));
  Node arcade_machine;
  arcade_machine.set_geometry(arcade_machine_meshes.front());
  arcade_machine.set_program(&default_shader);

  const auto joystick_meshes =
      bonobo::loadObjects(config::resources_path("../assets/joystick.fbx"));
  Node joystick;
  joystick.set_geometry(joystick_meshes.front());
  joystick.set_program(&default_shader);

  static constexpr int FBW = 32;
  static constexpr int FBH = 32;
  static constexpr int BOARD_WIDTH = 10;
  static constexpr int BOARD_HEIGHT = 20;
  static constexpr int BOARD_OFFSET_X = 11;
  static constexpr int BOARD_OFFSET_Y = 3;

  tetris_t *tetris_game = create_game();
  tetris_inputs_t key_input{};
  init(tetris_game, BOARD_WIDTH, BOARD_HEIGHT, 1000000, 166667, 33333);

  uint32_t framebuffer[FBH][FBW]{0};
  {
    const auto draw_box = [&](int x, int y, int w, int h) {
      for (int i = y; i < y + h; ++i) {
        framebuffer[FBH - i][x] = COLOR_BORDER;
        framebuffer[FBH - i][x + w - 1] = COLOR_BORDER;
      }
      for (int i = x; i < x + w; ++i) {
        framebuffer[FBH - y][i] = COLOR_BORDER;
        framebuffer[FBH - (y + h - 1)][i] = COLOR_BORDER;
      }
    };
    draw_box(BOARD_OFFSET_X - 1, BOARD_OFFSET_Y - 1, BOARD_WIDTH + 2,
             BOARD_HEIGHT + 2);
    draw_box(BOARD_OFFSET_X - 6, BOARD_OFFSET_Y - 1, 6, 4);
    for (int i = 0; i < 5; ++i) {
      draw_box(BOARD_OFFSET_X + BOARD_WIDTH, BOARD_OFFSET_Y - 1 + i * 4, 6, 4);
    }
  }

  GLuint screen_texture = bonobo::createTexture(
      FBW, FBH, GL_TEXTURE_2D, GL_RGBA, GL_RGBA, GL_UNSIGNED_BYTE, framebuffer);
  auto screen_meshes =
      bonobo::loadObjects(config::resources_path("../assets/screen.fbx"));
  Node screen;
  screen_meshes[0].bindings["diffuse_texture"] = screen_texture;
  screen.set_geometry(screen_meshes.front());
  screen.set_program(&default_shader);

  GLuint screen_pbo;
  glGenBuffers(1, &screen_pbo);
  glBindBuffer(GL_PIXEL_UNPACK_BUFFER, screen_pbo);
  glBufferData(GL_PIXEL_UNPACK_BUFFER, FBW * FBH * sizeof(framebuffer[0][0]),
               framebuffer, GL_STREAM_DRAW);
  glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);

  glClearDepthf(1.0f);
  glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
  glEnable(GL_DEPTH_TEST);

  auto lastTime = std::chrono::high_resolution_clock::now();

  bool show_gui = true;
  bool shader_reload_failed = false;

  while (!glfwWindowShouldClose(window)) {
    auto const nowTime = std::chrono::high_resolution_clock::now();
    auto const deltaTimeUs =
        std::chrono::duration_cast<std::chrono::microseconds>(nowTime -
                                                              lastTime);
    const auto delta_time = deltaTimeUs.count() / 1000000.0f;
    lastTime = nowTime;

    auto &io = ImGui::GetIO();
    inputHandler.SetUICapture(io.WantCaptureMouse, io.WantCaptureKeyboard);

    glfwPollEvents();
    inputHandler.Advance();
    mCamera.Update(deltaTimeUs, inputHandler);

    if (inputHandler.GetKeycodeState(GLFW_KEY_R) & JUST_PRESSED) {
      shader_reload_failed = !program_manager.ReloadAllPrograms();
      if (shader_reload_failed)
        tinyfd_notifyPopup("Shader Program Reload Error",
                           "An error occurred while reloading shader programs; "
                           "see the logs for details.\n"
                           "Rendering is suspended until the issue is solved. "
                           "Once fixed, just reload the shaders again.",
                           "error");
    }
    if (inputHandler.GetKeycodeState(GLFW_KEY_F2) & JUST_RELEASED)
      show_gui = !show_gui;
    if (inputHandler.GetKeycodeState(GLFW_KEY_F11) & JUST_RELEASED)
      mWindowManager.ToggleFullscreenStatusForWindow(window);

    if (inputHandler.GetKeycodeState(GLFW_KEY_R) & JUST_PRESSED) {
      set_seed(tetris_game, 0);
      init(tetris_game, 10, 20, 1000000, 166667, 33333);
    }

    key_input.rotate_cw = inputHandler.GetKeycodeState(GLFW_KEY_UP) & PRESSED;
    key_input.rotate_ccw =
        inputHandler.GetKeycodeState(GLFW_KEY_LEFT_CONTROL) & PRESSED;
    key_input.hard_drop =
        inputHandler.GetKeycodeState(GLFW_KEY_SPACE) & PRESSED;
    key_input.soft_drop = inputHandler.GetKeycodeState(GLFW_KEY_DOWN) & PRESSED;
    key_input.hold =
        inputHandler.GetKeycodeState(GLFW_KEY_LEFT_SHIFT) & PRESSED;
    key_input.left = inputHandler.GetKeycodeState(GLFW_KEY_LEFT) & PRESSED;
    key_input.right = inputHandler.GetKeycodeState(GLFW_KEY_RIGHT) & PRESSED;

    // Retrieve the actual framebuffer size: for HiDPI monitors,
    // you might end up with a framebuffer larger than what you
    // actually asked for. For example, if you ask for a 1920x1080
    // framebuffer, you might get a 3840x2160 one instead.
    // Also it might change as the user drags the window between
    // monitors with different DPIs, or if the fullscreen status is
    // being toggled.
    int framebuffer_width, framebuffer_height;
    glfwGetFramebufferSize(window, &framebuffer_width, &framebuffer_height);
    glViewport(0, 0, framebuffer_width, framebuffer_height);

    mWindowManager.NewImGuiFrame();

    glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);

    // Update Screen
    if (tick(tetris_game,
             tetris_params_t{key_input,
                             static_cast<time_us_t>(deltaTimeUs.count())})) {
      glBindBuffer(GL_PIXEL_UNPACK_BUFFER, screen_pbo);
      auto ptr = (uint32_t *)glMapBuffer(GL_PIXEL_UNPACK_BUFFER, GL_WRITE_ONLY);
      if (ptr) {
        for (int y = 0; y < 20; ++y) {
          for (int x = 0; x < 10; ++x) {
            ptr[(FBH - (y + 3)) * FBW + (x + 11)] =
                get_color(read_game(tetris_game, x, y));
          }
        }
        {
          const auto fb = get_hold_blocks(tetris_game);
          if (fb) {
            const auto w = get_hold_width(tetris_game);
            const auto h = get_hold_height(tetris_game);
            for (int y = 0; y < 2; ++y) {
              for (int x = 0; x < 4; ++x) {
                const int i = (FBH - (BOARD_OFFSET_Y + y)) * FBW +
                              (x + BOARD_OFFSET_X - 5);
                ptr[i] =
                    (y < h && x < w) ? get_color(fb[y * w + x]) : COLOR_BLACK;
              }
            }
          }
        }
        for (int b = 0; b < 5; ++b) {
          const auto fb = get_next_blocks(tetris_game, b);
          if (fb) {
            const auto w = get_next_width(tetris_game, b);
            const auto h = get_next_height(tetris_game, b);
            for (int y = 0; y < 2; ++y) {
              for (int x = 0; x < 4; ++x) {
                const int i = (FBH - (BOARD_OFFSET_Y + y + b * 4)) * FBW +
                              (x + BOARD_OFFSET_X + BOARD_WIDTH + 1);
                ptr[i] =
                    (y < h && x < w) ? get_color(fb[y * w + x]) : COLOR_BLACK;
              }
            }
          }
        }
      }
      glUnmapBuffer(GL_PIXEL_UNPACK_BUFFER);

      glBindTexture(GL_TEXTURE_2D, screen_texture);
      glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, FBW, FBH, GL_RGBA,
                      GL_UNSIGNED_BYTE, nullptr);
      glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);
    }

    if (!shader_reload_failed) {
      arcade_machine.render(mCamera.GetWorldToClipMatrix());

      static float rot_angle = 0.0f;
      const float target_angle = (static_cast<float>(key_input.left) -
                                  static_cast<float>(key_input.right)) *
                                 0.2f;
      rot_angle = rot_angle + (target_angle - rot_angle) * delta_time * 20.f;

      const auto rot =
          glm::rotate(glm::mat4(1.0f), rot_angle, glm::vec3(0.f, 0.f, 1.f));
      const auto trans =
          glm::translate(glm::mat4(1.0f), glm::vec3(-0.462f, 2.35f, 0.75f));

      joystick.render(mCamera.GetWorldToClipMatrix(), trans * rot);

      screen.render(mCamera.GetWorldToClipMatrix());
    }

    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

    mWindowManager.RenderImGuiFrame(show_gui);

    glfwSwapBuffers(window);
  }
}

int main() {
  std::setlocale(LC_ALL, "");

  Bonobo framework;

  try {
    edaf80::Assignment5 assignment5(framework.GetWindowManager());
    assignment5.run();
  } catch (std::runtime_error const &e) {
    LogError(e.what());
  }
}
