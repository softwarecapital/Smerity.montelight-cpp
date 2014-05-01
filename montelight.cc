// ==Montelight==
// Tegan Brennan, Stephen Merity, Taiyo Wilson
#include <cmath>
#include <string>
#include <fstream>

#define EPSILON 0.01f

using namespace std;

struct Vector {
  double x, y, z;
  //
  Vector(const Vector &o) : x(o.x), y(o.y), z(o.z) {}
  Vector(double x_=0, double y_=0, double z_=0) : x(x_), y(y_), z(z_) {}
  inline Vector operator+(const Vector &o) const {
    return Vector(x + o.x, y + o.y, z + o.z);
  }
  inline Vector operator-(const Vector &o) const {
    return Vector(x - o.x, y - o.y, z - o.z);
  }
  inline Vector operator*(double o) const {
    return Vector(x * o, y * o, z * o);
  }
  inline double dot(const Vector &o) const {
    return x * o.x + y * o.y + z * o.z;
  }
  inline Vector &norm(){
    return *this = *this * (1 / sqrt(x * x + y * y + z * z));
  }
  inline Vector cross(Vector &o){
    return Vector(y * o.z - z * o.y, z * o.x - x * o.z, x * o.y - y * o.x);
  }
};

struct Ray {
  Vector origin, direction;
  Ray(const Vector &o_, const Vector &d_) : origin(o_), direction(d_) {}
};

struct Image {
  unsigned int width, height;
  Vector *pixels;
  //
  Image(unsigned int w, unsigned int h) : width(w), height(h) {
    pixels = new Vector[width * height];
  }
  void setPixel(unsigned int x, unsigned int y, const Vector &v) {
    pixels[(height - y) * width + x] = v;
  }
  void save(std::string filePrefix) {
    std::string filename = filePrefix + ".ppm";
    std::ofstream f;
    f.open(filename.c_str(), std::ofstream::out);
    // PPM header: P3 => RGB, width, height, and max RGB value
    f << "P3 " << width << " " << height << " " << 255 << std::endl;
    // For each pixel, write the space separated RGB values
    for (int i=0; i < width * height; i++) {
      unsigned int r = pixels[i].x * 255, g = pixels[i].y * 255, b = pixels[i].z * 255;
      f << r << " " << g << " " << b << std::endl;
    }
  }
  ~Image() {
    delete[] pixels;
  }
};

struct Shape {
  Vector color;
  //
  Shape(const Vector color_) : color(color_) {}
  virtual double intersects(const Ray &r) const { return 0; }
};

struct Sphere : Shape {
  Vector center, color;
  double radius;
  //
  Sphere(const Vector center_, double radius_, const Vector color_) :
    Shape(color_), center(center_), radius(radius_) {}
  double intersects(const Ray &r) const {
    // Find if, and at what distance, the ray intersects with this object
    // Equation follows from solving quadratic equation of (r - c) ^ 2
    // http://wiki.cgsociety.org/index.php/Ray_Sphere_Intersection
    Vector offset = r.origin - center;
    double a = r.direction.dot(r.direction);
    double b = 2 * offset.dot(r.direction);
    double c = offset.dot(offset) - radius * radius;
    // Find discriminant for use in quadratic equation (b^2 - 4ac)
    double disc = b * b - 4 * a * c;
    // If the discriminant is negative, there are no real roots
    // (ray misses sphere)
    if (disc < 0) {
      return 0;
    }
    // The smallest positive root is the closest intersection point
    disc = sqrt(disc);
    double t = - b - disc;
    if (t > EPSILON) {
      return t;
    }
    t = - b + disc;
    if (t > EPSILON) {
      return t;
    }
    return 0;
  }
};

int main(int argc, const char *argv[]) {
  // Initialize the image
  int w = 256, h = 256;
  Image img(w, h);
  // Set up the scene
  // Cornell box inspired: http://graphics.ucsd.edu/~henrik/images/cbox.html
  Shape *scene[] = {//Scene: radius, position, emission, color, material
    new Sphere(Vector(1e5+1,40.8,81.6), 1e5f, Vector(.75,.25,.25)),//Left
    new Sphere(Vector(-1e5+99,40.8,81.6), 1e5f, Vector(.25,.25,.75)),//Rght
    new Sphere(Vector(50,40.8, 1e5), 1e5f, Vector(.75,.75,.75)),//Back
    new Sphere(Vector(50,40.8,-1e5+170), 1e5f, Vector()),//Frnt
    new Sphere(Vector(50, 1e5, 81.6), 1e5f, Vector(.75,.75,.75)),//Botm
    new Sphere(Vector(50,-1e5+81.6,81.6), 1e5f, Vector(.75,.75,.75)),//Top
    new Sphere(Vector(27,16.5,47), 16.5f, Vector(1,1,1) * 0.9),//Mirr
    new Sphere(Vector(73,16.5,78), 16.5f, Vector(1,1,1) * 0.9),//Glas
    new Sphere(Vector(50,681.6-.27,81.6), 600, Vector(1,1,1)) //Light
  };
  // Set up the camera
  Ray camera = Ray(Vector(50, 52, 295.6), Vector(0, -0.042612, -1).norm());
  // Upright camera with field of view angle set by 0.5135
  Vector cx = Vector((w * 0.5135) / h, 0, 0);
  // Cross product gets the vector perpendicular to cx and the "gaze" direction
  Vector cy = (cx.cross(camera.direction)).norm() * 0.5135;
  // Take a set number of samples per pixel
  for (int samples = 0; samples < 1; ++samples) {
    // For each pixel, sample a ray in that direction
    for (int y = 0; y < h; ++y) {
      for (int x = 0; x < w; ++x) {
        // Calculate the direction of the camera ray
        Vector d = (cx * ((x / float(w)) - 0.5)) + (cy * ((y / float(h)) - 0.5)) + camera.direction;
        Ray ray = Ray(camera.origin + d * 140, d.norm());
        // Check for intersection with objects in scene
        Vector color;
        double closest = 1e20f;
        for (auto obj : scene) {
          double hit = obj->intersects(ray);
          if (hit > 0 && hit < closest) {
            color = obj->color;
            closest = hit;
          }
        }
        // Add result of sample to image
        img.setPixel(x, y, color);
      }
    }
  }
  // Save the resulting raytraced image
  img.save("render");
  return 0;
}
