#include "stream_reassembler.hh"
#include <algorithm>
#include <cstddef>
#include <cstdint>

// Dummy implementation of a stream reassembler.

// For Lab 1, please replace with a real implementation that passes the
// automated checks run by `make check_lab1`.

// You will need to add private members to the class declaration in `stream_reassembler.hh`

template <typename... Targs>
void DUMMY_CODE(Targs &&... /* unused */) {}

using namespace std;

using Interval = pair<uint64_t, uint64_t>;  // [first, second) 左闭右开

// 交集
Interval intersect(const Interval& a, const Interval& b) {
    size_t l = max(a.first, b.first);
    size_t r = min(a.second, b.second);
    if (l < r) return {l, r};
    return {0, 0};  // 空区间
}
bool has_overlap(const Interval& a, const Interval& b) {
    return a.first < b.second && b.first < a.second;
}

// 并集：两个区间有重叠时返回它们的并
Interval Union(const Interval& a, const Interval& b) {
    return {min(a.first, b.first), max(a.second, b.second)};
}
bool can_union(const Interval& a, const Interval& b) {
    return has_overlap(a, b) || a.second == b.first || b.second == a.first;
}

// 补集（差集）：a \ b，返回 a 去掉 b 之后剩余的部分
vector<Interval> difference(const Interval& a, const Interval& b) {
    vector<Interval> res;
    if (!has_overlap(a, b)) {
        res.push_back(a);
        return res;
    }
    if (a.first < b.first) res.push_back({a.first, b.first});
    if (b.second < a.second) res.push_back({b.second, a.second});
    return res;
}

// 区间合并：将一组区间合并为不相交的区间集合
vector<Interval> merge_intervals(vector<Interval> intervals) {
    if (intervals.empty()) return {};
    sort(intervals.begin(), intervals.end());
    vector<Interval> res;
    Interval cur = intervals[0];
    for (size_t i = 1; i < intervals.size(); i++) {
        if (can_union(cur, intervals[i])) {
            cur = Union(cur, intervals[i]);
        } else {
            res.push_back(cur);
            cur = intervals[i];
        }
    }
    res.push_back(cur);
    return res;
}
string get_substring(const string &data, Interval range) {
    return data.substr(range.first, range.second - range.first);
}
StreamReassembler::StreamReassembler(const size_t capacity): 
    _output(capacity), 
    _capacity(capacity), 
    assembler_window({0, static_cast<uint64_t>(capacity)}),
    unassembled_index(0),
    assembled_index(0),
    ranges({{0, static_cast<uint64_t>(capacity)}}) {
        this->assembler_buffer.resize(this->_capacity, 0);
        this->received_bits.resize(this->_capacity, false);
    }

//! \details This function accepts a substring (aka a segment) of bytes,
//! possibly out-of-order, from the logical stream, and assembles any newly
//! contiguous substrings and writes them into the output stream in order.
void StreamReassembler::push_substring(const string &data, const uint64_t index, const bool eof) {
    if (eof) {
        _eof_received = true;
        _eof_index = index + static_cast<uint64_t>(data.size());
    }

    Interval data_range {index, index+static_cast<uint64_t>(data.size())};
    vector<Interval> new_ranges;
    uint64_t needed_index = this->ranges[0].first;

    for (auto p: this->ranges) {
        if (!has_overlap(p, data_range)) { new_ranges.push_back(p); continue; }
        auto intersect_range = intersect(p, data_range);
        size_t begin = static_cast<size_t>(intersect_range.first - index);
        size_t interval_len = static_cast<size_t>(intersect_range.second - intersect_range.first);
        for (size_t buffer_index = (intersect_range.first - needed_index); interval_len > 0; interval_len--, buffer_index++) {
            this->assembler_buffer[buffer_index] = data[begin];
            this->received_bits[buffer_index] = true;
            begin++;
        }
        for (auto x: difference(p, intersect_range)) { new_ranges.push_back(x); }
    }

    sort(new_ranges.begin(), new_ranges.end());

    // Filter empty ranges
    vector<Interval> filtered;
    for (auto r : new_ranges) {
        if (r.first < r.second) filtered.push_back(r);
    }
    new_ranges = filtered;

    // Write contiguous bytes to output
    uint64_t next_needed = new_ranges.empty() ? assembler_window.second : new_ranges[0].first;
    size_t loaded_size = std::min(this->_output.remaining_capacity(), static_cast<size_t>(next_needed - needed_index));
    size_t saved_loaded_size = loaded_size;
    for (; loaded_size>0; loaded_size--) {
        char new_char = this->assembler_buffer[0];
        this->assembler_buffer.pop_front();
        this->received_bits.pop_front();
        this->_output.write(std::string(1, new_char));
    }
    assembled_index += static_cast<uint64_t>(saved_loaded_size);

    assembler_window.first = assembled_index;
    assembler_window.second = assembled_index + static_cast<uint64_t>(this->_capacity);
    Interval last_range { assembler_window.second-static_cast<uint64_t>(saved_loaded_size), assembler_window.second };

    if (new_ranges.empty()) {
        new_ranges.push_back({assembler_window.first, assembler_window.second});
        unassembled_index = assembler_window.first;
        this->assembler_buffer.resize(static_cast<size_t>(assembler_window.second - assembler_window.first), 0);
        this->received_bits.resize(static_cast<size_t>(assembler_window.second - assembler_window.first), false);
    } else {
        unassembled_index = new_ranges[0].first;
        this->assembler_buffer.resize(static_cast<size_t>(assembler_window.second - new_ranges[0].first), 0);
        this->received_bits.resize(static_cast<size_t>(assembler_window.second - new_ranges[0].first), false);
    }

    if (last_range.first < last_range.second) {
        if (has_overlap(new_ranges.back(), last_range)) {
            uint64_t left = new_ranges.back().first;
            new_ranges.pop_back();
            new_ranges.push_back(Interval {left, assembler_window.second});
        } else {
            new_ranges.push_back(last_range);
        }
    }
    this->ranges = new_ranges;

    if (_eof_received && assembled_index >= _eof_index) {
        this->_output.end_input();
    }
}

size_t StreamReassembler::unassembled_bytes() const {
    size_t count = 0;
    for (auto b : received_bits) { if (b) count++; }
    return count;
}

bool StreamReassembler::empty() const { return unassembled_bytes() == this->_capacity; }
