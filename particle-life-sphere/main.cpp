#include <fstream>

#include "al/app/al_App.hpp"
#include "al/graphics/al_DefaultShaders.hpp"
#include "al/graphics/al_Shapes.hpp"
#include "al/math/al_Random.hpp"
#include "al/spatial/al_HashSpace.hpp"

std::string slurp(std::string fileName) {
  std::fstream file(fileName);
  std::string returnValue = "";
  while (file.good()) {
    std::string line;
    std::getline(file, line);
    returnValue += line + "\n";
  }
  return returnValue;
}

struct MyApp : al::App {
  al::HashSpace space{6, 10000};
  al::HashSpace::Query qmany{100};

  std::vector<al::Vec3d> velocity;

  al::ShaderProgram pointShader;
  al::ShaderProgram defaultShader;

  al::Mesh box{al::Mesh::LINES};
  void onCreate() override {
    box.vertex(0, 0, 0);
    box.vertex(1, 0, 0);
    box.vertex(0, 0, 0);
    box.vertex(0, 1, 0);
    box.vertex(0, 0, 0);
    box.vertex(0, 0, 1);
    box.vertex(1, 1, 1);
    box.vertex(1, 1, 0);
    box.vertex(1, 1, 1);
    box.vertex(0, 1, 1);
    box.vertex(1, 1, 1);
    box.vertex(1, 0, 1);
    box.scale(space.dim());

    velocity.resize(space.numObjects());
    for (int i = 0; i < space.numObjects(); ++i) {
      space.move(i, al::Vec3d(space.maxRadius()) +
                        al::rnd::ball<al::Vec3d>() * space.maxRadius() * 0.9);
      velocity[i] = al::rnd::ball<al::Vec3d>();
    }

    if (!pointShader.compile(slurp("../point-vertex.glsl"),
                             slurp("../point-fragment.glsl"),
                             slurp("../point-geometry.glsl"))) {
      exit(1);
    }

    compileDefaultShader(defaultShader, al::ShaderType::COLOR);

    nav().faceToward(al::Vec3d(space.maxRadius()));
    nav().nudgeF(-0.5);
    nav().step();
    nav().step();
    nav().setHome();
  }

  void onAnimate(double dt) override {
    // std::cout << "######################################" << std::endl;
    for (int i = 0; i < space.numObjects(); ++i) {
      auto& position = space.object(i).pos;

      al::Vec3d local;

      qmany.clear();
      float r = 0.17;

      // brute force search nearest
      /*
      int count = 0;
      for (int j = i + 1; j < space.numObjects(); ++j) {
        float distance = (space.object(j).pos - position).mag();
        if (distance < r * space.maxRadius()) {
          ++count;
        }
      }
      std::cout << count << " within " << r * space.maxRadius() << std::endl;
      */

      // nearest single neighbor?
      /*
     al::HashSpace::Query q{6};
      auto* n = q.nearest(space, &space.object(i));
      if (n) {
        std::cout << "got this" << std::endl;
      }
      */

      int results = qmany(space, position, r * space.maxRadius());
      for (int k = 0; k < results; ++k) {
        float other = float(qmany[k]->id) / space.numObjects();
        float me = float(i) / space.numObjects();
        float distance =
            std::min(std::abs(other - me), 1.0f - std::abs(other - me));
        if (distance < 0.05) {
          auto v = position - qmany[k]->pos;
          local += v.normalized((v.mag() - r) * 0.003);
        }
      }

      auto foo = position - al::Vec3d(space.maxRadius());
      double mag = foo.mag();
      al::Vec3d sphere = foo * (space.maxRadius() / 2.0 - mag) / mag;
      al::Vec3d noise = al::rnd::ball<al::Vec3d>() * 0.003;
      al::Vec3d drag = velocity[i] * -0.2;
      auto acceleration = drag + local + noise + sphere * 0.3;

      // semi-implicit Euler
      velocity[i] += acceleration;
      space.move(i, position + velocity[i]);
    }
  }

  void onDraw(al::Graphics& g) override {
    g.clear(0.1);

    g.shader(pointShader);
    g.shader().uniform("pointSize", 1 / 100.0);
    g.blending(true);
    g.blendTrans();
    g.depthTesting(true);

    al::Mesh mesh{al::Mesh::POINTS};
    for (int i = 0; i < space.numObjects(); ++i) {
      mesh.vertex(space.object(i).pos);
      mesh.texCoord(0.1, 0);
      mesh.color(al::HSV(float(i) / space.numObjects(), 1, 1));
    }

    g.scale(1.0 / space.dim());
    g.draw(mesh);

    g.shader(defaultShader);
    g.color(1);
    g.draw(box);
  }
};
int main() { MyApp().start(); }