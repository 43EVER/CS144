#include "byte_stream.hh"

#include <algorithm>
#include <iterator>
#include <sstream>
#include <stdexcept>

// Dummy implementation of a flow-controlled in-memory byte stream.

// For Lab 0, please replace with a real implementation that passes the
// automated checks run by `make check_lab0`.

// You will need to add private members to the class declaration in `byte_stream.hh`

template <typename... Targs>
void DUMMY_CODE(Targs &&... /* unused */) {}

size_t ByteStream::write(const std::string &data) {
    if (_is_end) return 0;
    int len = data.length();
    if (len + buffer.size() > _capacity)
        len = _capacity - buffer.size();

    buffer.insert(buffer.end(), data.begin(), data.begin() + len);

    total_write += len;

    return len;
}

size_t ByteStream::remaining_capacity() const { return _capacity - buffer.size(); }

std::string ByteStream::peek_output(const size_t len) const {
    return std::string{buffer.begin(), buffer.begin() + len};
}

void ByteStream::pop_output(const size_t len) {
    buffer.erase(buffer.begin(), buffer.begin() + len);
    total_read += len;
}

std::string ByteStream::read(const size_t len) {
    std::string res = peek_output(len);
    pop_output(len);

    return res;
}
