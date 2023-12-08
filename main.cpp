#include <Siv3D.hpp>

struct obj {
  enum class obj_type {
    STATIC,
    DYNAMIC,
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

std::set<std::pair<int, int>> getCollisions(const Array<obj> &objs) {
  std::set<std::pair<int, int>> collisions;

  if (objs[2]._x <= objs[0]._x + objs[0]._w)
    collisions.insert({0, 2});
  if (objs[1]._x <= objs[2]._x + objs[2]._w)
    collisions.insert({1, 2});

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
      objs[i].rect().draw(HSV(100 + i * 70, 0.5, 1));

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
  objs << obj(-1000, 100, 1100, 400, 1);
  objs << obj(500, 100, 400, 400, 1000000).setVelocity(-100).setType(obj::obj_type::DYNAMIC);
  objs << obj(300, 250, 100, 100, 1).setVelocity(0).setType(obj::obj_type::DYNAMIC);

  while (System::Update()) {
    update(objs);

    for (size_t i = 0; i < objs.size(); i++)
      objs[i].rect().draw(HSV(100 + i * 70, 0.5, 1));

    ClearPrint();
    Print << objs[1].rect();
    Print << objs[1].velocity;
    Print << Profiler::FPS();
    Print << count;
  }
}
