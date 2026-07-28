#include "stream_reassembler.hh"
#include <algorithm>
#include <cstddef>
#include <cstdint>

using namespace std;

using Interval = pair<uint64_t, uint64_t>;  // [first, second) 左闭右开

// 交集
Interval intersect(const Interval& a, const Interval& b) {
    uint64_t l = max(a.first, b.first);
    uint64_t r = min(a.second, b.second);
    if (l < r) return {l, r};
    return {0, 0};
}
bool has_overlap(const Interval& a, const Interval& b) {
    return a.first < b.second && b.first < a.second;
}

// 并集
Interval Union(const Interval& a, const Interval& b) {
    return {min(a.first, b.first), max(a.second, b.second)};
}
bool can_union(const Interval& a, const Interval& b) {
    return has_overlap(a, b) || a.second == b.first || b.second == a.first;
}

// 差集：a \ b
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

StreamReassembler::StreamReassembler(const size_t capacity):
    _output(capacity),
    _capacity(capacity),
    assembled_index(0),
    ranges({{0, static_cast<uint64_t>(capacity)}}) {
        assembler_buffer.resize(_capacity, 0);
        received_bits.resize(_capacity, false);
    }

void StreamReassembler::push_substring(const string &data, const uint64_t index, const bool eof) {
    if (eof) {
        _eof_received = true;
        _eof_index = index + static_cast<uint64_t>(data.size());
    }

    Interval data_range {index, index + static_cast<uint64_t>(data.size())};
    vector<Interval> new_ranges;
    uint64_t needed_index = ranges[0].first;

    pair<uint64_t, uint64_t> assembler_window = {assembled_index, assembled_index + static_cast<uint64_t>(_capacity)};

    for (auto p : ranges) {
        if (!has_overlap(p, data_range)) { new_ranges.push_back(p); continue; }
        auto intersect_range = intersect(p, data_range);
        size_t begin = static_cast<size_t>(intersect_range.first - index);
        size_t interval_len = static_cast<size_t>(intersect_range.second - intersect_range.first);
        for (size_t buffer_index = (intersect_range.first - needed_index); interval_len > 0; interval_len--, buffer_index++) {
            if (!received_bits[buffer_index]) {
                _unassembled_count++;
                received_bits[buffer_index] = true;
            }
            assembler_buffer[buffer_index] = data[begin];
            begin++;
        }
        for (auto x : difference(p, intersect_range)) { new_ranges.push_back(x); }
    }

    sort(new_ranges.begin(), new_ranges.end());

    // 过滤空区间
    vector<Interval> filtered;
    for (auto r : new_ranges) {
        if (r.first < r.second) filtered.push_back(r);
    }
    new_ranges = filtered;

    // 写出连续字节
    uint64_t next_needed = new_ranges.empty() ? assembler_window.second : new_ranges[0].first;
    size_t loaded_size = min(_output.remaining_capacity(), static_cast<size_t>(next_needed - needed_index));
    size_t saved_loaded_size = loaded_size;
    for (; loaded_size > 0; loaded_size--) {
        char c = assembler_buffer[0];
        assembler_buffer.pop_front();
        if (received_bits[0]) _unassembled_count--;
        received_bits.pop_front();
        _output.write(string(1, c));
    }
    assembled_index += static_cast<uint64_t>(saved_loaded_size);

    assembler_window = {assembled_index, assembled_index + static_cast<uint64_t>(_capacity)};
    Interval last_range {assembler_window.second - static_cast<uint64_t>(saved_loaded_size), assembler_window.second};

    if (new_ranges.empty()) {
        new_ranges.push_back({assembler_window.first, assembler_window.second});
        assembler_buffer.resize(static_cast<size_t>(assembler_window.second - assembler_window.first), 0);
        received_bits.resize(static_cast<size_t>(assembler_window.second - assembler_window.first), false);
    } else {
        assembler_buffer.resize(static_cast<size_t>(assembler_window.second - new_ranges[0].first), 0);
        received_bits.resize(static_cast<size_t>(assembler_window.second - new_ranges[0].first), false);
    }

    if (last_range.first < last_range.second) {
        if (has_overlap(new_ranges.back(), last_range)) {
            uint64_t left = new_ranges.back().first;
            new_ranges.pop_back();
            new_ranges.push_back({left, assembler_window.second});
        } else {
            new_ranges.push_back(last_range);
        }
    }
    ranges = new_ranges;

    if (_eof_received && assembled_index >= _eof_index) {
        _output.end_input();
    }
}

size_t StreamReassembler::unassembled_bytes() const {
    return _unassembled_count;
}

bool StreamReassembler::empty() const {
    return _unassembled_count == 0;
}
