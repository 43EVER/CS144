#include "tcp_receiver.hh"

// Dummy implementation of a TCP receiver

// For Lab 2, please replace with a real implementation that passes the
// automated checks run by `make check_lab2`.

TCPReceiver::TCPReceiver(const size_t capacity)
    : _reassembler(capacity), _capacity(capacity), has_syn(false), isn(0), checkpoint(0) {}

void TCPReceiver::segment_received(const TCPSegment &seg) {
    if (!has_syn) {
        if (!seg.header().syn)
            return;

        has_syn = true;
        isn = WrappingInt32{seg.header().seqno};
        checkpoint = 0;
    }

    auto seqno = seg.header().seqno;
    uint64_t index = unwrap(seqno, isn, checkpoint);
    checkpoint = index;
    
    if (index > 0) index -= 1;
    _reassembler.push_substring(seg.payload().copy(), index, seg.header().fin);
}

std::optional<WrappingInt32> TCPReceiver::ackno() const {
    if (!has_syn) {
        return std::nullopt;
    }

    return wrap(_reassembler.get_required_index() + 1, isn);
}

size_t TCPReceiver::window_size() const { return _capacity - _reassembler.stream_out().buffer_size(); }
