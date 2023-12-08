#include <Siv3D.hpp>

struct obj {
  enum class obj_type {
    STATIC,
    DYNAMIC,
    KINEMATIC
  };

  obj(double x, double y, double w, double h, double mass) {
    velocity = 0;
    _x = x;
    _y = y;
    _w = w;
    _h = h;
    _m = mass;
  }

  obj setVelocity(double _velocity) {
    velocity = _velocity;
    return *this;
  }

  obj setType(obj_type type) {
    _type = type;
    return *this;
  }

  Rect rect(void) {
    return Rect(_x, _y, _w, _h);
  }

  double velocity;
  double _x;
  double _y;
  double _w;
  double _h;
  double _m;
  obj_type _type = obj_type::STATIC;
};

bool isIntersects(const obj &a, const obj &b) {
  return (a._x < b._x + b._w && b._x < a._x) || (b._x < a._x + a._w && a._x < b._x);
}

std::set<std::pair<int, int>> getCollisions(const Array<obj> &objs) {
  std::set<std::pair<int, int>> collisions;

  for (int i = 0; i < objs.size(); i++)
    for (int j = 0; j < objs.size(); j++) {
      if (i == j)
        continue;

      if (isIntersects(objs[i], objs[j]))
        collisions.insert({std::min(i, j), std::max(i, j)});
    }

  return collisions;
}

void calcCollisions(Array<obj> &objs, std::set<std::pair<int, int>> const collisions, double delta) {
  for (auto &&[a, b] : collisions) {
    const double va = objs[a].velocity, vb = objs[b].velocity;

    if (objs[a]._type == obj::obj_type::STATIC)
      objs[b].velocity *= double(-1);
    else
      objs[a].velocity = (va * (objs[a]._m - objs[b]._m) + double(2) * objs[b]._m * vb) / (objs[a]._m + objs[b]._m);
    if (objs[b]._type == obj::obj_type::STATIC)
      objs[a].velocity *= double(-1);
    else
      objs[b].velocity = va - vb + objs[a].velocity;

    double x;
    double time;
    bool flag = false;
    if (abs(va) < abs(vb))
      flag = 0 < vb;
    else
      flag = va < 0;

    if (flag)
      x = objs[b]._x + (vb < double(0) ? double(0) : objs[b]._w) - objs[a]._x;
    else
      x = objs[a]._x + (va < double(0) ? double(0) : objs[a]._w) - objs[b]._x;

    time = x / abs(va - vb);

    objs[a]._x += -time * va + (delta - time) * objs[a].velocity;
    objs[b]._x += -time * vb + (delta - time) * objs[b].velocity;
    delta -= time;

    Console << U"va:" << va;
    Console << U"vb:" << vb;
    Console << U"vad:" << objs[a].velocity;
    Console << U"vbd:" << objs[b].velocity;
    Console << U"x:" << x;
    Console << U"time:" << time;
    // Console << U"a pos:" << Unicode::Widen(objs[a]._x.get_top().str()) << '/' << Unicode::Widen(objs[a]._x.get_bottom().str());
    // Console << U"a pos:" << (objs[a]._x.get_top().str() + '/' + objs[a]._x.get_bottom().str()).size();
    Console << U"a pos:" << objs[a]._x;
    Console << ' ';
  }
}

int count = 0;
void update(Array<obj> &objs) {
  std::set<std::pair<int, int>> collisions;
  double delta = double(Scene::DeltaTime());
  for (int i = 0; i < objs.size(); i++)
    objs[i]._x += objs[i].velocity * delta;

  collisions = getCollisions(objs);
  count += collisions.size();
  while (collisions.size() && System::Update()) {
    calcCollisions(objs, collisions, delta);
    collisions = getCollisions(objs);
    count += collisions.size();
    for (size_t i = 0; i < objs.size(); i++)
      objs[i].rect().draw(HSV(i * 30));

    ClearPrint();
    Print << objs[1].rect();
    Print << objs[1].velocity;
    Print << Profiler::FPS();
    Print << count;
  }
}

void Main() {
  Window::Resize(1000, 600);

  Array<obj> objs;
  objs << obj(double(100), double(100), double(3), double(400), double(1));
  objs << obj(double(500), double(250), double(100), double(100), double(10000)).setVelocity(double(-50)).setType(obj::obj_type::DYNAMIC);
  objs << obj(double(300), double(250), double(100), double(100), double(1)).setVelocity(double(0)).setType(obj::obj_type::DYNAMIC);

  while (System::Update()) {
    update(objs);

    for (size_t i = 0; i < objs.size(); i++)
      objs[i].rect().draw(HSV(i * 30));

    ClearPrint();
    Print << objs[1].rect();
    Print << objs[1].velocity;
    Print << Profiler::FPS();
    Print << count;
  }
}
