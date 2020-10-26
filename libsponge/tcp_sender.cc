#include "tcp_sender.hh"

#include "../util/buffer.hh"
#include "tcp_config.hh"

#include <algorithm>
#include <random>

// Dummy implementation of a TCP sender

// For Lab 3, please replace with a real implementation that passes the
// automated checks run by `make check_lab3`.

template <typename... Targs>
void DUMMY_CODE(Targs &&... /* unused */) {}

using namespace std;

//! \param[in] capacity the capacity of the outgoing byte stream
//! \param[in] retx_timeout the initial amount of time to wait before retransmitting the oldest outstanding segment
//! \param[in] fixed_isn the Initial Sequence Number to use, if set (otherwise uses a random ISN)
TCPSender::TCPSender(const size_t capacity, const uint16_t retx_timeout, const std::optional<WrappingInt32> fixed_isn)
    : _isn(fixed_isn.value_or(WrappingInt32{random_device()()}))
    , _retransmission_timeout(retx_timeout)
    , _initial_retransmission_timeout{retx_timeout}
    , _stream(capacity) {}

void TCPSender::send_segment(TCPSegment &seg) {
    seg.header().seqno = wrap(_next_seqno, _isn);
    _segments_outstanding.push(seg);
    _segments_out.push(seg);
    _bytes_in_flight += seg.length_in_sequence_space();
    _next_seqno += seg.length_in_sequence_space();
    if (!_timer_running) {
        _timer_running = true;
        _timer = 0;
    }
}

uint64_t TCPSender::bytes_in_flight() const { return _bytes_in_flight; }

void TCPSender::fill_window() {
    if (!_syn_flag) {
        _syn_flag = true;
        TCPSegment seg;
        seg.header().syn = true;
        send_segment(seg);
        return;
    }

    size_t win = _window_size ? _window_size : 1;

    size_t remain = win - (_next_seqno - _recv_ackno);
    while ((remain = win - (_next_seqno - _recv_ackno)) > 0 && !_fin_flag) {
        size_t size = std::min(TCPConfig::MAX_PAYLOAD_SIZE, remain);
        std::string data = _stream.read(size);
        TCPSegment seg;
        seg.payload() = std::move(data);

        if (seg.length_in_sequence_space() < remain && _stream.eof()) {
            _fin_flag = true;
            seg.header().fin = true;
        }

        if (seg.length_in_sequence_space() == 0) {
            return;
        }

        send_segment(seg);
    }
}

//! \param ackno The remote receiver's ackno (acknowledgment number)
//! \param window_size The remote receiver's advertised window size
void TCPSender::ack_received(const WrappingInt32 ackno, const uint16_t window_size) {
    size_t abs_seq = unwrap(ackno, _isn, _recv_ackno);

    if (abs_seq > _next_seqno)
        return;

    _window_size = window_size;

    if (abs_seq <= _recv_ackno)
        return;
    _recv_ackno = abs_seq;

    while (!_segments_outstanding.empty()) {
        TCPSegment seg = _segments_outstanding.front();
        if (unwrap(seg.header().seqno, _isn, _next_seqno) + seg.length_in_sequence_space() <= abs_seq) {
            _segments_outstanding.pop();
            _bytes_in_flight -= seg.length_in_sequence_space();
        } else {
            break;
        }
    }
    _retransmission_timeout = _initial_retransmission_timeout;
    _consecutive_retransmission = 0;
    fill_window();

    if (!_segments_outstanding.empty()) {
        _timer_running = true;
        _timer = 0;
    }
}

//! \param[in] ms_since_last_tick the number of milliseconds since the last call to this method
void TCPSender::tick(const size_t ms_since_last_tick) {
    if (!_timer_running)
        return;

    _timer += ms_since_last_tick;
    if (_timer >= _retransmission_timeout && _segments_outstanding.size()) {
        _segments_out.push(_segments_outstanding.front());
        if (_window_size) {
            _consecutive_retransmission++;
            _retransmission_timeout *= 2;
        }
        _timer = 0;
        _timer_running = true;
    }

    if (_segments_outstanding.empty()) {
        _timer_running = false;
    }
}

unsigned int TCPSender::consecutive_retransmissions() const { return _consecutive_retransmission; }

void TCPSender::send_empty_segment() {
    TCPSegment seg;
    seg.header().seqno = wrap(_next_seqno, _isn);
    _segments_out.push(seg);
}
