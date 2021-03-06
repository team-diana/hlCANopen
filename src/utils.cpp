#include "hlcanopen/utils.hpp"

namespace hlcanopen {

std::ostream& hexUppercase(std::ostream& out) {
    return out << std::hex << std::setfill('0')
           << std::uppercase
           << std::setw(2);
}

}
