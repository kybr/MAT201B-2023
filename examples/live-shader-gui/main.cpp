//////////////////////////////////////////////////////////////////////////////
/// Karl Yerkes //////////////////////////////////////////////////////////////
/// MAT 201B /////////////////////////////////////////////////////////////////
/// 2023-01-27 ///////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////
/// AlloLib App //////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////

#include <fstream>

#include "TextEditor.cpp"
#include "al/app/al_App.hpp"
#include "al/io/al_Imgui.hpp"

std::string slurp(const std::string& fileName) {
  std::fstream file(fileName);
  std::string returnValue = "";
  while (file.good()) {
    std::string line;
    std::getline(file, line);
    returnValue += line + "\n";
  }
  return returnValue;
}

bool spurt(const std::string& fileName, const std::string& content) {
  std::ofstream file(fileName);
  if (file << content) return true;
  return false;
}
const char* vertex_code = R"(
#version 400
layout(location = 0) in vec3 vertexPosition; // DONT CHANGE
layout(location = 1) in vec4 vertexColor;    // DONT CHANGE
layout(location = 2) in vec2 vertexTexture;  // DONT CHANGE
uniform mat4 al_ModelViewMatrix;             // DONT CHANGE
uniform mat4 al_ProjectionMatrix;            // DONT CHANGE
void main() {
  gl_Position = al_ProjectionMatrix * al_ModelViewMatrix * vec4(vertexPosition, 1.0);
}
)";

const char* fragment_code_default = R"(
#version 400
layout(location = 0) out vec4 fragmentColor;

uniform vec3 iResolution;
uniform float iTime;
uniform int iFrame;
uniform vec4 iMouse;

void main() {
float t = fract(iTime);
  vec2 pixel = gl_FragCoord.xy / iResolution.xx;
  pixel -= vec2(0.5, 0.3);
  if (length(mod(pixel, vec2(0.1)) - vec2(0.05)) < 0.01) {
    t = 1.0 - t;
    fragmentColor = vec4(t, 1.0 - t * t, tanh(sin(t) * sin(t)), 1.0);
  }
  else {
    fragmentColor = vec4(t, 1.0 - t * t, tanh(sin(t) * sin(t)), 1.0);
  }
}
)";

using namespace al;
struct AlloApp : App {
  char fragment_code[10000];
  char error[10000];
  void onInit() override { imguiInit(); }
  void onExit() override { imguiShutdown(); }
  Shader fragment;  // (editor.GetText());

  TextEditor editor;

  ShaderProgram p;
  Mesh m{Mesh::TRIANGLES};
  //
  void onCreate() override {
    m.vertex(-1, -1);  // 0
    m.vertex(1, -1);   // 1
    m.vertex(1, 1);    // 2
    m.vertex(-1, 1);   // 3
    m.index(0, 1, 2);
    m.index(0, 2, 3);
    m.texCoord(0, 0);
    m.texCoord(1, 0);
    m.texCoord(0, 1);
    m.texCoord(1, 1);

    // load file; ideally in a background thread

    std::string fragment_code_file = slurp("fragment.glsl");
    editor.SetText(fragment_code_file.empty() ? fragment_code_default
                                              : fragment_code_file);
    p.compile(vertex_code, editor.GetText());
  }

  double time = 0;
  int frame = 0;
  void onAnimate(double dt) override {
    time += dt;
    frame++;
  }

  bool compile_successful = true;

  void onDraw(Graphics& g) override {
    imguiBeginFrame();
    ImGui::SetWindowFontScale(2.0);

    // if (editor.IsTextChanged()) {
    //   fragment.source(editor.GetText()).compile();
    //   compile_successful = fragment.compiled();
    //   if (compile_successful) {
    //     p.compile(vertex_code, editor.GetText());
    //     spurt("fragment.glsl", editor.GetText());
    //   }
    // }
    if (editor.IsTextChanged()) {
      compile_successful = p.compile(vertex_code, editor.GetText());
      if (compile_successful) {
        spurt("fragment.glsl", editor.GetText());
      }
    }

    if (!compile_successful) {
      ImGui::Text("%s", p.log());
      ImGui::Separator();
    }

    editor.Render("Text Editor");
    imguiEndFrame();

    g.clear(0.1);
    g.camera(Viewpoint::IDENTITY);
    g.shader(p);
    g.shader().uniform("iResolution", (float)width(), (float)height(), 0.0f);
    g.shader().uniform("iTime", (float)time);
    g.shader().uniform("iFrame", frame);
    g.shader().uniform("iMouse", (float)mouse().x(), (float)mouse().y(),
                       (float)mouse().left());
    g.draw(m);
    imguiDraw();
  }

  bool onKeyDown(const Keyboard& k) override {
    if (k.key() == '`') {
      // graphicsDomain().reset();
      // this->defaultWindowDomain()->
      // this->graphicsDomain()->closeWindow();
      printf("gothere\n");
    }
    return false;
  }
};

//////////////////////////////////////////////////////////////////////////////
/// Main
/// /////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////

int main() {
  AlloApp app;
  app.configureAudio(48000, 512, 2, 0);
  app.start();
}