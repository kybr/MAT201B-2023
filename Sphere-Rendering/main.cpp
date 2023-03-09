#include "al/app/al_App.hpp"
#include "al/app/al_GUIDomain.hpp"

struct MyApp : al::App {
  al::ParameterInt which{"which", "", 0, 0, 25};
  std::vector<std::vector<al::Vec3f>> pixel;
  std::vector<std::vector<float>> alpha;
  std::vector<al::Mesh> pointcloud;
  std::vector<al::Mesh> outline;  // trace the edges of projection

  struct Projection {
    int id;
    int rows, columns;
    float width, height;
    al::Pose pose;
  };
  std::vector<Projection> projection;

  al::Mesh mesh;

  void onCreate() override {
    {
      mesh.primitive(al::Mesh::LINE_LOOP);
      mesh.vertex(-1, -1);
      mesh.vertex(1, -1);
      mesh.vertex(1, 1);
      mesh.vertex(-1, 1);
    }

    // Load calibration data. Data is packed in binary arrays of floats.
    // These are 2D matricies in C order (row major) of (x, y, z, w).
    // Each file holds the data for a particular projector. We can think
    // of this data as a list of tuples (pid, (x, y), (x, y, z), w) where
    // pid and xy are projector id and pixel row/column---These are
    // implicit in the structure of the file---, and xyz is the position
    // of the pixel in real space. w is a measure of the overlap of the
    // pixel with neighboring projections.
    //
    std::vector<std::vector<al::Vec4f>> data;
    data.resize(26);
    projection.resize(data.size());
    for (int i = 0; i < data.size(); ++i) {
      int id = 1 + i;
      char filename[200];
      sprintf(filename, "../calibration-current/proj%d.bin", id);
      std::ifstream file(filename, std::ios::in | std::ios::binary);
      if (!file) {
        std::cout << "could not find the calibration data" << std::endl;
        continue;
      }

      projection[i].id = id;
      projection[i].rows = (id >= 9 && id <= 12) ? 1200 : 1050;
      projection[i].columns = (id >= 9 && id <= 12) ? 1920 : 1400;

      data[i].resize((id >= 9 && id <= 12) ? 1920 * 1200 : 1400 * 1050);
      {
        auto *byte = reinterpret_cast<char *>(data[i].data());
        file.read(byte, sizeof(al::Vec4f) * data[i].size());
      }
      file.close();
    }

    // build arrays of pixel positions and overlap/blending values
    //
    pixel.resize(data.size());
    alpha.resize(data.size());
    al::Vec3d sum;  // keep track of the sum to find the average
    int N = 0;
    for (int i = 0; i < data.size(); ++i) {
      pixel[i].resize(data[i].size());
      alpha[i].resize(data[i].size());
      for (int k = 0; k < data[i].size(); ++k) {
        auto &v(data[i][k]);
        sum += al::Vec3f(v[0], v[1], v[2]);
        pixel[i][k].set(v[0], v[1], v[2]);
        alpha[i][k] = v[3];
        ++N;
      }
    }
    sum /= N;

    // How many pixels?
    // 41556000 == 4 * 1920 * 1200 + 22 * 1400 * 1050

    // Show us the vertex count and average.
    //
    std::cout << N << std::endl;
    std::cout << sum / N << std::endl;

    outline.resize(pixel.size());
    for (int i = 0; i < pixel.size(); ++i) {
      outline[i].primitive(al::Mesh::LINE_LOOP);

      auto get_pixel = [&](int r, int c) {
        return pixel[i][r * projection[i].columns + c];
      };

      for (int c = 0; c < projection[i].columns; ++c)
        outline[i].vertex(get_pixel(0, c));
      for (int r = 0; r < projection[i].rows; ++r)
        outline[i].vertex(get_pixel(r, projection[i].columns - 1));
      for (int c = projection[i].columns - 1; c >= 0; --c)
        outline[i].vertex(get_pixel(projection[i].rows - 1, c));
      for (int r = projection[i].rows - 1; r >= 0; --r)
        outline[i].vertex(get_pixel(r, 0));
    }

    // Determine characteristics of each projection.
    //
    for (int i = 0; i < pixel.size(); ++i) {
      al::Vec3d sum;
      int n = 0;
      for (auto &v : pixel[i]) {
        sum += v;
        ++n;
      }
      projection[i].pose.set(sum / n);
      // projection[i].pose = al::Pose(sum / n);
      projection[i].pose.faceToward(al::Vec3d(0), al::Vec3d(0, 1, 0));
    }

    // Center, rotate, and flatten each projection point-cloud
    //
    pointcloud.resize(pixel.size());
    for (int i = 0; i < pixel.size(); ++i) {
      for (auto &v : pixel[i]) {
        // v -= projection[i].pose.pos();
        // v = projection[i].pose.quat().rotateTransposed(v);
        // v.z = 0;
        pointcloud[i].vertex(v);
      }
    }
    /*
     */
  }

  void onInit() override {
    auto GUIdomain = al::GUIDomain::enableGUI(defaultWindowDomain());
    auto &gui = GUIdomain->newGUI();
    gui.add(which);
  }

  void onDraw(al::Graphics &g) override {
    g.clear(1);
    // g.meshColor();
    g.color(0);

    g.draw(pointcloud[which]);

    // for (auto &p : projection) {
    //   g.pushMatrix();
    //   g.translate(p.pose.pos());
    //   g.rotate(p.pose.quat());
    //   g.draw(mesh);
    //   g.popMatrix();
    // }

    for (auto &m : outline) {
      g.draw(m);
    }
  }
};

int main() { MyApp().start(); }