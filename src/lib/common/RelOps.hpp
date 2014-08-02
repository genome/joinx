#pragma once

// This is a helper class that defines relational operators
// (>, <, ==, !=, ...) for simple structs that define value_type
// and have a single member: value_type value;
template<typename T>
struct ValueBasedRelOps {
    friend bool operator<(T const& lhs, T const& rhs) {
        return lhs.value < rhs.value;
    }

    friend bool operator>(T const& lhs, T const& rhs) {
        return lhs.value > rhs.value;
    }

    friend bool operator<=(T const& lhs, T const& rhs) {
        return lhs.value <= rhs.value;
    }

    friend bool operator>=(T const& lhs, T const& rhs) {
        return lhs.value >= rhs.value;
    }

    friend bool operator==(T const& lhs, T const& rhs) {
        return lhs.value == rhs.value;
    }

    friend bool operator!=(T const& lhs, T const& rhs) {
        return lhs.value != rhs.value;
    }
};
