#include <set>

#include "al/app/al_App.hpp"
#include "al/app/al_GUIDomain.hpp"

struct MyApp : al::App {
  al::ParameterInt which{"which", "", 0, 0, 25};
  std::vector<std::vector<al::Vec4f>> data;
  std::vector<al::Mesh> point;

  void onCreate() override {
    data.resize(26);
    point.resize(data.size());
    for (int i = 0; i < data.size(); ++i) {
      int id = 1 + i;
      char filename[200];
      sprintf(filename, "../calibration-current/proj%d.bin", id);
      std::ifstream file(filename, std::ios::in | std::ios::binary);
      if (!file) {
        std::cout << "could not open file" << std::endl;
        continue;
      }

      data[i].resize((id >= 9 && id <= 12) ? 1920 * 1200 : 1400 * 1050);
      {
        auto *byte = reinterpret_cast<char *>(data[i].data());
        file.read(byte, sizeof(al::Vec4f) * data[i].size());
      }
      file.close();

      for (auto &p : data[i]) {
        point[i].vertex(p[0], p[1], p[2]);
        point[i].color(al::Color(al::HSV(double(i) / data.size()), p[3]));
      }
    }

    // how many pixels?
    // 41556000 == 4 * 1920 * 1200 + 22 * 1400 * 1050
  }

  void onInit() override {
    auto GUIdomain = al::GUIDomain::enableGUI(defaultWindowDomain());
    auto &gui = GUIdomain->newGUI();
    gui.add(which);
  }

  void onDraw(al::Graphics &g) override {
    g.clear(1);
    g.meshColor();
    g.draw(point[which]);
  }
};

int main() { MyApp().start(); }