// Karl Yerkes
// MAT201B
// 2022-01-04
// minimal app, ready for adapting..
//

#include "al/app/al_App.hpp"
#include "al/app/al_GUIDomain.hpp"
#include "al/graphics/al_Shapes.hpp"
#include "al/math/al_Random.hpp"

using namespace al;

#include <regex>
#include <unordered_map>

// CafeMatte

std::vector<std::string> ip{
    "192.168.1.115",  // Karl
    "192.168.1.196",  // Ryan
    "192.168.1.113",  // Jinjin
    "192.168.1.78",   // Shaokang
    "192.168.1.192",  // Sabina
    "192.168.1.176",  // Olifa
};

struct Agent {
  Pose pose;
  RGB rgb;
  float scale;
};

struct MyApp : App {
  std::unordered_map<std::string, Agent> agent;
  Mesh cone{Mesh::TRIANGLES};
  std::string me;
  ParameterColor color{"Color"};
  Parameter scale{"Scale", 0.1, 0.05, 0.9};

  void onCreate() override {
    addCone(cone);
    cone.decompress();
    cone.generateNormals();

    char buffer[20];
    sprintf(buffer, "~%d~", rnd::uniform(10000));
    me += buffer;
  }

  void onMessage(osc::Message& m) override {
    // receive someones pose
    if (m.addressPattern() == "/pose") {
      // m.print();
      std::string who;
      m >> who;
      if (who != me) {
        Pose p;
        m >> p.pos().x;
        m >> p.pos().y;
        m >> p.pos().z;
        m >> p.quat().w;
        m >> p.quat().x;
        m >> p.quat().y;
        m >> p.quat().z;

        if (agent.find(who) == agent.end()) {
          agent[who] = Agent();
        }
        agent[who].pose.set(p);
      }
    }

    if (m.addressPattern() == "/color") {
      std::string who;
      m >> who;
      if (who != me) {
        RGB c;
        m >> c.r;
        m >> c.g;
        m >> c.b;

        if (agent.find(who) != agent.end()) {
          agent[who].rgb.set(c);
        }
      }
    }

    if (m.addressPattern() == "/scale") {
      std::string who;
      m >> who;
      if (who != me) {
        float s;
        m >> s;

        if (agent.find(who) != agent.end()) {
          agent[who].scale = s;
        }
      }
    }
  }

  void onAnimate(double dt) override {
    // broadcast our pose
    for (auto i : ip) {
      osc::Send(9010, i.c_str())
          .send("/pose", me,                                  //
                nav().pos().x, nav().pos().y, nav().pos().z,  //
                nav().quat().w, nav().quat().x, nav().quat().y, nav().quat().z);
      osc::Send(9010, i.c_str())
          .send("/color", me,  //
                color.get().r, color.get().g, color.get().b);
      osc::Send(9010, i.c_str()).send("/scale", me, scale.get());
    }
  }

  void onDraw(Graphics& g) override {
    g.clear(0.2);

    g.depthTesting(true);
    g.lighting(true);

    for (auto a : agent) {
      g.pushMatrix();
      g.translate(a.second.pose.pos());
      g.rotate(a.second.pose.quat());
      g.scale(a.second.scale);
      g.color(a.second.rgb);
      g.draw(cone);
      g.popMatrix();
    }
  }

  bool onKeyDown(const Keyboard& k) override { return false; }

  void onSound(AudioIOData& io) override {}

  void onInit() override {
    auto GUIdomain = GUIDomain::enableGUI(defaultWindowDomain());
    auto& gui = GUIdomain->newGUI();
    gui.add(color);
    gui.add(scale);
  }
};

int main() {
  MyApp app;
  app.configureAudio(48000, 512, 2, 0);
  app.start();
}
