#include "hlcanopen/can_msg.hpp"
#include "hlcanopen/utils.hpp"

namespace hlcanopen {

CanMsg::CanMsg() : cobId(0, 0), data(0) {

}

// From MSB to LSB
uint8_t CanMsg::byteat(int index) const {
    return (*this)[index];
}


uint8_t& CanMsg::operator[](int index) {
    return byteAtRef(index);
}

// From MSB to LSB
uint8_t CanMsg::operator[](int index) const {
    return byteAtRef(index);
}

std::ostream& operator<<(std::ostream& os, const CanMsg& msg);

std::string CanMsg::msgDataToStr() const {
    std::ostringstream os;
    os << "<";
    for(int i=0; i < 7; i++) {
        os << hexUppercase << static_cast<uint32_t>((*this)[i]) << ":";
    }
    os << hexUppercase << static_cast<uint32_t>((*this)[7]) << ">";
    return os.str();
}

uint8_t& CanMsg::byteAtRef(int index) const {
    uint8_t* v = reinterpret_cast<uint8_t*>(const_cast<uint64_t*>(&data)); // LSB
    return *(v+7-index);
}

std::ostream& operator<<(std::ostream& os, const CanMsg& msg)
{
    std::ios::fmtflags f( os.flags() );
    os << "CanMsg[" << "cobId:" << std::hex
       << std::setfill('0') << std::uppercase
       << std::setw(2) << msg.cobId.getCobIdValue()
       << ", data:" << msg.msgDataToStr() << "]";
    os.flags(f);
    return os;
}

}

