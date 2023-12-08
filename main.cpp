#include <Siv3D.hpp>
#include <Siv3D/Scene.hpp>

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

void update(Array<obj> &objs) {
  std::set<std::pair<int, int>> collisions;
  for (int i = 0; i < objs.size(); i++) {
    objs[i]._x += objs[i].velocity * frac(Scene::DeltaTime());
    for (int j = 0; j < objs.size(); j++) {
      if (i == j)
        continue;

      if (isIntersects(objs[i], objs[j])) {
        collisions.insert({std::min(i, j), std::max(i, j)});
        if (objs[i].velocity < frac(0))
          objs[i]._x += objs[j]._x + objs[j]._w - objs[i]._x;
        else
          objs[i]._x += objs[j]._x - objs[i]._x - objs[i]._w;

        objs[i].velocity = -objs[i].velocity;
      }
    }
  }

  // for (auto &&[a, b] : collisions) {
  //   auto va = objs[a].velocity, vb = objs[b].velocity;
  //   auto ta = 0;
  //   objs[a].velocity = (va * (objs[a]._m - objs[b]._m) + frac(2) * objs[b]._m * vb) / (objs[a]._m + objs[b]._m);
  //   objs[b].velocity = va - vb + objs[a].velocity;
  // }
}

void Main() {
  Window::Resize(1000, 600);

  Array<obj> objs;
  objs << obj(frac(100), frac(100), frac(3), frac(400), frac(1));
  objs << obj(frac(450), frac(250), frac(100), frac(100), frac(1)).setVelocity(frac(-10)).setType(obj::obj_type::DYNAMIC);
  objs << obj(frac(300), frac(250), frac(100), frac(100), frac(1)).setVelocity(frac(10)).setType(obj::obj_type::DYNAMIC);

  while (System::Update()) {
    if (!KeyEnter.pressed())
      update(objs);

    for (size_t i = 0; i < objs.size(); i++)
      objs[i].rect().draw(HSV(i * 30));

    ClearPrint();
    Print << objs[1].rect();
    Print << objs[1].velocity.calc();
  }
}
