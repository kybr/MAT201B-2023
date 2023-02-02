#include "al/app/al_App.hpp"
#include "al/graphics/al_Shapes.hpp"
#include "al/ui/al_Gnomon.hpp"

using namespace al;

struct MyApp : App {
  Gnomon gnomon;
  Mesh mesh;
  void onCreate() override {
    addCone(mesh);
    mesh.primitive(Mesh::LINES);
  }

  void onAnimate(double dt) override {
    //
  }

  void onDraw(Graphics& g) override {
    g.clear(1);
    g.color(0);

    for (int i = 0; i < N; ++i) {
      g.pushMatrix();
      g.translate(agent[i].position);
      g.rotate(Quatd::getRotationTo(Vec3f(0, 0, 1), agent[i].velocity));
      g.scale(agent[i].size);
      g.draw(mesh);
      g.popMatrix();
    }
    gnomon.drawOrigin(g);
    gnomon.drawFloating(g, pose(), 0.1);
  }

  bool onKeyDown(const Keyboard& k) override {
    //
    return false;
  }

  void onSound(AudioIOData& io) override {
    //
  }
};

int main() {
  MyApp app;
  app.start();
}
