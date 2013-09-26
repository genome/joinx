#pragma once

#include <boost/cstdint.hpp>

// If you've ever met me, you'll know I like to say:
// "Never put unscoped using directives in header files!"
// Watch me break my own rule.
//
// I'm doing this to reduce reliance on C++11 features
// (the C++11 header <cstdint> provides these in the global namespace)

using boost::uint8_t;
using boost::uint16_t;
using boost::uint32_t;
using boost::uint64_t;
using boost::int8_t;
using boost::int16_t;
using boost::int32_t;
using boost::int64_t;

