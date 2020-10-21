#include "stream_reassembler.hh"

// Dummy implementation of a stream reassembler.

// For Lab 1, please replace with a real implementation that passes the
// automated checks run by `make check_lab1`.

// You will need to add private members to the class declaration in `stream_reassembler.hh`

template <typename... Targs>
void DUMMY_CODE(Targs &&... /* unused */) {}

StreamReassembler::StreamReassembler(const size_t capacity)
    : _output(capacity), _capacity(capacity), _required_index(0), _waiting() {}

//! \details This function accepts a substring (aka a segment) of bytes,
//! possibly out-of-order, from the logical stream, and assembles any newly
//! contiguous substrings and writes them into the output stream in order.
void StreamReassembler::push_substring(const std::string &data, const size_t index, const bool eof) {
    std::vector<BlockNode> res;
    for (size_t pos = 0; pos < data.size(); pos++)
        res.push_back({pos + index, data[pos], false});
    if (eof) res.push_back({data.size() + index, ' ', true});

    size_t pos = 0;
    while (pos < res.size() && pos + index < _required_index) 
        pos++;
    while (pos < res.size() && pos + index == _required_index) {
        if (res[pos].is_end)
            _output.end_input();
        else
            if (_output.remaining_capacity()) _output.write(std::string{res[pos].data});
            else break;
        pos++;
        _required_index++;
    }
    _waiting.erase(_waiting.begin(), _waiting.lower_bound({_required_index, ' ', false}));
    while (_waiting.size() && _waiting.begin()->index == _required_index) {
        auto it = _waiting.begin();
        _waiting.erase(_waiting.begin());

        if (it->is_end)
            _output.end_input();
        else
            if (_output.remaining_capacity()) _output.write(std::string{it->data});
            else break;

        _required_index++;
    }
    while (pos < res.size()) {
        _waiting.insert(res[pos]);
        if (_waiting.size() > _capacity + (--_waiting.end())->is_end)
            _waiting.erase(--_waiting.end());
        pos++;
    }
}

size_t StreamReassembler::unassembled_bytes() const { return _waiting.size(); }

bool StreamReassembler::empty() const { return _waiting.empty(); }
