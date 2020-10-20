#ifndef SPONGE_LIBSPONGE_BYTE_STREAM_HH
#define SPONGE_LIBSPONGE_BYTE_STREAM_HH

#include <cstddef>
#include <cstdint>
#include <deque>
#include <sstream>
#include <string>
#include <utility>

//! \brief An in-order byte stream.

//! Bytes are written on the "input" side and read from the "output"
//! side.  The byte stream is finite: the writer can end the input,
//! and then no more bytes can be written.
class ByteStream {
  private:
    // Your code here -- add private members as necessary.
    std::deque<char> buffer;
    const size_t _capacity;
    bool _is_end;
    bool _has_error;
    mutable size_t total_read, total_write;

  public:
    //! Construct a stream with room for `capacity` bytes.
    ByteStream(const size_t capacity)
        : buffer(), _capacity(capacity), _is_end(false), _has_error(false), total_read(0), total_write(0) {}

    //! \name "Input" interface for the writer
    //!@{

    //! Write a string of bytes into the stream. Write as many
    //! as will fit, and return how many were written.
    //! \returns the number of bytes accepted into the stream
    size_t write(const std::string &data);

    //! \returns the number of additional bytes that the stream has space for
    size_t remaining_capacity() const;

    //! Signal that the byte stream has reached its ending
    void end_input() { _is_end = true; }

    //! Indicate that the stream suffered an error.
    void set_error() { _has_error = true; }
    //!@}

    //! \name "Output" interface for the reader
    //!@{

    //! Peek at next "len" bytes of the stream
    //! \returns a string
    std::string peek_output(const size_t len) const;

    //! Remove bytes from the buffer
    void pop_output(const size_t len);

    //! Read (i.e., copy and then pop) the next "len" bytes of the stream
    //! \returns a vector of bytes read
    std::string read(const size_t len);

    //! \returns `true` if the stream input has ended
    bool input_ended() const { return _is_end; }

    //! \returns `true` if the stream has suffered an error
    bool error() const { return _has_error; }

    //! \returns the maximum amount that can currently be read from the stream
    size_t buffer_size() const { return buffer.size(); }

    //! \returns `true` if the buffer is empty
    bool buffer_empty() const { return buffer.empty(); }

    //! \returns `true` if the output has reached the ending
    bool eof() const { return _is_end && buffer.empty(); }
    //!@}

    //! \name General accounting
    //!@{

    //! Total number of bytes written
    size_t bytes_written() const { return total_write; }

    //! Total number of bytes popped
    size_t bytes_read() const { return total_read; }
    //!@}
};

#endif  // SPONGE_LIBSPONGE_BYTE_STREAM_HH
