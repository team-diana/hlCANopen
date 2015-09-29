#include "include/hlcanopen/can_open_manager.hpp"

#include <boost/core/ignore_unused.hpp>
#include <hlcanopen/can_card.hpp>

using namespace hlcanopen;

class Fake : public CanCard {
  public:
    virtual void write(const CanMsg& msg) override {
    }

    virtual CanMsg read() override {
    }
};

int main(int argc, char** argv) {
    Fake asdf;
    hlcanopen::CanOpenManager a(asdf);

    boost::ignore_unused(argc);
    boost::ignore_unused(argv);

    a.setupLogging();
}
