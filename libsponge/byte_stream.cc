#include "byte_stream.hh"

// Dummy implementation of a flow-controlled in-memory byte stream.

// For Lab 0, please replace with a real implementation that passes the
// automated checks run by `make check_lab0`.

// You will need to add private members to the class declaration in `byte_stream.hh`

template <typename... Targs>
void DUMMY_CODE(Targs &&... /* unused */) {}

using namespace std;

ByteStream::ByteStream(const size_t capacity) { 
    this->is_finished = false;
    this->capacity = capacity;
    this->_bytes_written = 0;
    this->_bytes_read = 0;
}

size_t ByteStream::write(const string &data) {
    size_t n = min(data.size(), this->remaining_capacity());
    for (size_t i=0; i<n; i++) { this->_buffer.push_back(data[i]); }
    this->_bytes_written += n;
    return n;
}

//! \param[in] len bytes will be copied from the output side of the buffer
string ByteStream::peek_output(const size_t len) const {
    string s;
    for (size_t i = 0; i < len; i++) { s.push_back(this->_buffer[i]); }
    return s;
}

//! \param[in] len bytes will be removed from the output side of the buffer
void ByteStream::pop_output(const size_t len) { 
    this->_bytes_read += len;
    for (size_t i=0; i < len; i++) { this->_buffer.pop_front(); }
}

//! Read (i.e., copy and then pop) the next "len" bytes of the stream
//! \param[in] len bytes will be popped and returned
//! \returns a string
std::string ByteStream::read(const size_t len) {
    if (this->buffer_size() < len) { return string{}; }
    string s = this->peek_output(len);
    this->pop_output(len);
    return s;
}

void ByteStream::end_input() { this->_input_ended = true; }

bool ByteStream::input_ended() const { return _input_ended; }

size_t ByteStream::buffer_size() const { return this->_buffer.size(); }

bool ByteStream::buffer_empty() const { return this->_buffer.empty(); }

bool ByteStream::eof() const { return this->input_ended() and this->buffer_empty(); }

size_t ByteStream::bytes_written() const { return _bytes_written; }

size_t ByteStream::bytes_read() const { return _bytes_read; }

size_t ByteStream::remaining_capacity() const { return this->capacity - buffer_size(); }
