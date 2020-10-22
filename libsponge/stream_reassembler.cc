#include "stream_reassembler.hh"

#include <algorithm>

// Dummy implementation of a stream reassembler.

// For Lab 1, please replace with a real implementation that passes the
// automated checks run by `make check_lab1`.

// You will need to add private members to the class declaration in `stream_reassembler.hh`

template <typename... Targs>
void DUMMY_CODE(Targs &&... /* unused */) {}

StreamReassembler::StreamReassembler(const size_t capacity)
    : _output(capacity), _capacity(capacity), _required_index(0), _waiting(), eof_node_count(0) {}

//! \details This function accepts a substring (aka a segment) of bytes,
//! possibly out-of-order, from the logical stream, and assembles any newly
//! contiguous substrings and writes them into the output stream in order.
void StreamReassembler::push_substring(const std::string &data, const size_t index, const bool eof) {
    if (_output.input_ended())
        return;

    std::vector<BlockNode> res;
    for (size_t pos = 0; pos < data.size(); pos++)
        res.push_back({pos + index, data[pos], false});
    if (eof)
        res.push_back({data.size() + index, ' ', true});

    auto fun = [this](const BlockNode &node) {
        if (node.is_end)
            _output.end_input();
        else if (_output.remaining_capacity())
            _output.write(std::string{node.data});
        else
            return 0;
        return 1;
    };

    size_t pos = 0;
    while (pos < res.size() && pos + index < _required_index)
        pos++;
    while (pos < res.size() && pos + index == _required_index) {
        if (fun(res[pos]) == 0)
            break;
        pos++;
        _required_index++;
    }
    eof_node_count -= std::count_if(
        _waiting.begin(), _waiting.lower_bound({_required_index, ' ', false}), [](auto node) { return node.is_end; });
    _waiting.erase(_waiting.begin(), _waiting.lower_bound({_required_index, ' ', false}));
    while (_waiting.size() && _waiting.begin()->index == _required_index) {
        auto it = _waiting.begin();
        _waiting.erase(it);
        if (it->is_end)
            eof_node_count--;
        if (fun(*it) == 0)
            break;
        _required_index++;
    }
    while (pos < res.size()) {
        _waiting.insert(res[pos]);
        if (_waiting.size() > _capacity + eof_node_count) {
            auto it = --_waiting.end();
            if (it->is_end)
                eof_node_count--;
            _waiting.erase(it);
        }
        pos++;
    }
}

size_t StreamReassembler::unassembled_bytes() const { return _waiting.size() - eof_node_count; }

bool StreamReassembler::empty() const { return _waiting.empty(); }
