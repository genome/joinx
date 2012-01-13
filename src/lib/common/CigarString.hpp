#pragma once

#include <cstdint>
#include <iostream>
#include <string>
#include <deque>

enum CigarOpType {
    MATCH,
    INS,
    DEL,
    SKIP,
    SOFT_CLIP,
    HARD_CLIP,
    PADDING,
    SEQ_MATCH,
    SEQ_MISMATCH,
    N_CIGAR_OP_TYPES,
    BAD
};

class CigarString {
public:
// types
    struct Op {
        Op() {}
        Op(uint32_t length, CigarOpType type)
            : length(length)
            , type(type)
        {
        }

        uint32_t length;
        CigarOpType type;
        bool operator==(const Op& rhs) const {
            return length == rhs.length && type == rhs.type;
        }
    };

    typedef std::deque<Op>::const_iterator const_iterator;

// functions
    static char translate(CigarOpType op);
    static CigarOpType translate(char c);
    static CigarString merge(CigarString a, CigarString b, uint32_t pos);

    CigarString();
    CigarString(const std::string& data);

    CigarString& operator=(const std::string& data);

    uint32_t length() const;
    bool empty() const;

    void parse(const std::string& data);
    operator std::string() const;
    void push_back(const Op& op);
    void concatenate(const CigarString& s);
    void pop_front(uint32_t n);
    const std::deque<Op> ops() const;

    // returns a subset of the cigar string
    // note: deletions do not count towards the total length
    CigarString subset(uint32_t offset, uint32_t length) const;
    // This limits the operations to M,I,D,S. This is what bwa outputs
    CigarString structural() const;

    const Op& operator[](const uint32_t idx) const;

    const_iterator begin() const;
    const_iterator end() const;

protected:
    std::deque<Op> _ops;
};

std::ostream& operator<<(std::ostream& s, const CigarString::Op& op);
std::ostream& operator<<(std::ostream& s, const CigarString& op);

inline CigarString& CigarString::operator=(const std::string& data) {
    parse(data);
    return *this;
}

inline bool CigarString::empty() const {
    return _ops.empty();
}
