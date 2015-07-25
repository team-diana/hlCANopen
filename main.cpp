#include "include/hlcanopen/can_open_manager.hpp"

using namespace hlcanopen;

template <typename T> class Fake {
public:
  template<class M> void write(M&& msg) {
  }

  T read() {
  }
};

int main(int argc, char** argv) {
  Fake<CanMsg> asdf;
  hlcanopen::CanOpenManager<Fake<CanMsg>> a(asdf);

  a.setupLogging();

}
