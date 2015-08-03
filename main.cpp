#include "include/hlcanopen/can_open_manager.hpp"

#include <boost/core/ignore_unused.hpp>

using namespace hlcanopen;

template <typename T> class Fake {
public:
    template<class M> void write(M&& ) {
    }

    T read() {
    }
};

int main(int argc, char** argv) {
    Fake<CanMsg> asdf;
    hlcanopen::CanOpenManager<Fake<CanMsg>> a(asdf);

    boost::ignore_unused(argc);
    boost::ignore_unused(argv);

    a.setupLogging();
}
