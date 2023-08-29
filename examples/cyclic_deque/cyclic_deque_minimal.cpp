#include <iostream>
#include <ouroboros/cyclic_deque.hpp>

struct BufferItem {
  int id;
};

template <typename Range_>
void Print(Range_ const& range, std::string const& name) {
  std::cout << name + " contents:" << std::endl;
  for (auto const& i : range) {
    std::cout << i.id << std::endl;
  }
  std::cout << std::endl;
}

int main() {
  BufferItem buffer[] = {{-1}, {-1}, {-1}, {-1}};

  ouroboros::cyclic_deque ring(std::begin(buffer), std::end(buffer));
  ring.push_back({41});
  ring.push_front({42});

  Print(ring, "ring");
  Print(buffer, "buffer");

  return 0;
}
