#include <Siv3D.hpp>
#include <Siv3D/Profiler.hpp>
#include <Siv3D/Scene.hpp>
#include <Siv3D/System.hpp>
#include <Siv3D/Unicode.hpp>

#include "fraction/fraction.hpp"

struct obj {
  enum class obj_type {
    STATIC,
    DYNAMIC,
    KINEMATIC
  };

  obj(frac x, frac y, frac w, frac h, frac mass) {
    velocity = frac(0);
    _x = x;
    _y = y;
    _w = w;
    _h = h;
    _m = mass;
  }

  obj setVelocity(frac _velocity) {
    velocity = _velocity;
    return *this;
  }

  obj setType(obj_type type) {
    _type = type;
    return *this;
  }

  Rect rect(void) {
    return Rect(_x.calc(), _y.calc(), _w.calc(), _h.calc());
  }

  frac velocity;
  frac _x;
  frac _y;
  frac _w;
  frac _h;
  frac _m;
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

void calcCollisions(Array<obj> &objs, std::set<std::pair<int, int>> const collisions, frac delta) {
  for (auto &&[a, b] : collisions) {
    const frac va = objs[a].velocity, vb = objs[b].velocity;

    if (objs[a]._type == obj::obj_type::STATIC)
      objs[b].velocity *= frac(-1);
    else
      objs[a].velocity = (va * (objs[a]._m - objs[b]._m) + frac(2) * objs[b]._m * vb) / (objs[a]._m + objs[b]._m);
    if (objs[b]._type == obj::obj_type::STATIC)
      objs[a].velocity *= frac(-1);
    else
      objs[b].velocity = va - vb + objs[a].velocity;

    frac x;
    frac time;
    bool flag = false;
    if (va.abs() < vb.abs())
      flag = frac(0) < vb;
    else
      flag = va < frac(0);

    if (flag)
      x = objs[b]._x + (vb < frac(0) ? frac(0) : objs[b]._w) - objs[a]._x;
    else
      x = objs[a]._x + (va < frac(0) ? frac(0) : objs[a]._w) - objs[b]._x;

    time = x / (va - vb).abs();

    objs[a]._x += -time * va + (delta - time) * objs[a].velocity;
    objs[b]._x += -time * vb + (delta - time) * objs[b].velocity;
    delta -= time;

    Console << U"va:" << va.calc();
    Console << U"vb:" << vb.calc();
    Console << U"vad:" << objs[a].velocity.calc();
    Console << U"vbd:" << objs[b].velocity.calc();
    Console << U"x:" << x.calc();
    Console << U"time:" << time.calc();
    // Console << U"a pos:" << Unicode::Widen(objs[a]._x.get_top().str()) << '/' << Unicode::Widen(objs[a]._x.get_bottom().str());
    Console << U"a pos:" << (objs[a]._x.get_top().str() + '/' + objs[a]._x.get_bottom().str()).size();
    Console << U"a pos:" << objs[a]._x.calc();
    Console << ' ';
  }
}

int count = 0;
int update(Array<obj> &objs) {
  int collisionCount = 0;
  std::set<std::pair<int, int>> collisions;
  frac delta = frac(Scene::DeltaTime());
  for (int i = 0; i < objs.size(); i++)
    objs[i]._x += objs[i].velocity * delta;

  collisions = getCollisions(objs);
  collisionCount += collisions.size();
  while (collisions.size() && System::Update()) {
    calcCollisions(objs, collisions, delta);
    collisions = getCollisions(objs);
    count += collisions.size();
    for (size_t i = 0; i < objs.size(); i++)
      objs[i].rect().draw(HSV(i * 30));

    ClearPrint();
    Print << objs[1].rect();
    Print << objs[1].velocity.calc();
    Print << Profiler::FPS();
    Print << count;
  }

  return collisionCount;
}

void Main() {
  Window::Resize(1000, 600);

  Array<obj> objs;
  objs << obj(frac(100), frac(100), frac(3), frac(400), frac(1));
  objs << obj(frac(500), frac(250), frac(100), frac(100), frac(10000)).setVelocity(frac(-50)).setType(obj::obj_type::DYNAMIC);
  objs << obj(frac(300), frac(250), frac(100), frac(100), frac(1)).setVelocity(frac(0)).setType(obj::obj_type::DYNAMIC);

  while (System::Update()) {
    // if (KeyEnter.down())
    update(objs);

    for (size_t i = 0; i < objs.size(); i++)
      objs[i].rect().draw(HSV(i * 30));

    ClearPrint();
    Print << objs[1].rect();
    Print << objs[1].velocity.calc();
    Print << Profiler::FPS();
    Print << count;
  }
}
