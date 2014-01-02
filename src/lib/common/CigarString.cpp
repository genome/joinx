#include "CigarString.hpp"

#include "Tokenizer.hpp"
#include <boost/format.hpp>
#include <algorithm>
#include <cassert>
#include <iterator>
#include <sstream>
#include <stdexcept>

using boost::format;
using namespace std;

namespace {
    void throwInvalidOp(CigarOpType type) {
        if (type >= N_CIGAR_OP_TYPES) {
            format fmt("An invalid cigar operation was requested: '%1%'");
            fmt % int(type);
            throw runtime_error(str(fmt));
        } else {
            format fmt("The requested cigar operation is unsupported: '%1%'");
            fmt % CigarString::translate(type);
            throw runtime_error(str(fmt));
        }
    }

    const char _op_to_chr[N_CIGAR_OP_TYPES+1] = "MIDNSHP=X";
    const CigarOpType _chr_to_op[256] = {
        BAD, BAD, BAD, BAD, BAD, BAD, BAD, BAD, // 0
        BAD, BAD, BAD, BAD, BAD, BAD, BAD, BAD, // 8
        BAD, BAD, BAD, BAD, BAD, BAD, BAD, BAD, // 16
        BAD, BAD, BAD, BAD, BAD, BAD, BAD, BAD, // 24
        BAD, BAD, BAD, BAD, BAD, BAD, BAD, BAD, // 32
        BAD, BAD, BAD, BAD, BAD, BAD, BAD, BAD, // 40
        BAD, BAD, BAD, BAD, BAD, BAD, BAD, BAD, // 48
        BAD, BAD, BAD, BAD, BAD, SEQ_MATCH, BAD, BAD, // 56
        BAD, BAD, BAD, BAD, DEL, BAD, BAD, BAD, // 64
        HARD_CLIP, INS, BAD, BAD, BAD, MATCH, SKIP, BAD, // 72
        PADDING, BAD, BAD, SOFT_CLIP, BAD, BAD, BAD, BAD, // 80
        SEQ_MISMATCH, BAD, BAD, BAD, BAD, BAD, BAD, BAD, // 88
        BAD, BAD, BAD, BAD, DEL, BAD, BAD, BAD, // 96
        HARD_CLIP, INS, BAD, BAD, BAD, MATCH, SKIP, BAD, // 104
        PADDING, BAD, BAD, SOFT_CLIP, BAD, BAD, BAD, BAD, // 112
        SEQ_MISMATCH, BAD, BAD, BAD, BAD, BAD, BAD, BAD, // 120
        BAD, BAD, BAD, BAD, BAD, BAD, BAD, BAD, // 128
        BAD, BAD, BAD, BAD, BAD, BAD, BAD, BAD, // 136
        BAD, BAD, BAD, BAD, BAD, BAD, BAD, BAD, // 144
        BAD, BAD, BAD, BAD, BAD, BAD, BAD, BAD, // 152
        BAD, BAD, BAD, BAD, BAD, BAD, BAD, BAD, // 160
        BAD, BAD, BAD, BAD, BAD, BAD, BAD, BAD, // 168
        BAD, BAD, BAD, BAD, BAD, BAD, BAD, BAD, // 176
        BAD, BAD, BAD, BAD, BAD, BAD, BAD, BAD, // 184
        BAD, BAD, BAD, BAD, BAD, BAD, BAD, BAD, // 192
        BAD, BAD, BAD, BAD, BAD, BAD, BAD, BAD, // 200
        BAD, BAD, BAD, BAD, BAD, BAD, BAD, BAD, // 208
        BAD, BAD, BAD, BAD, BAD, BAD, BAD, BAD, // 216
        BAD, BAD, BAD, BAD, BAD, BAD, BAD, BAD, // 224
        BAD, BAD, BAD, BAD, BAD, BAD, BAD, BAD, // 232
        BAD, BAD, BAD, BAD, BAD, BAD, BAD, BAD, // 240
        BAD, BAD, BAD, BAD, BAD, BAD, BAD, BAD, // 248
    };


}

char CigarString::translate(CigarOpType op) {
    if (op >= N_CIGAR_OP_TYPES)
        throwInvalidOp(op);
    return _op_to_chr[op];
}

CigarOpType CigarString::translate(char c) {
    CigarOpType rv = _chr_to_op[int(c)];
    if (rv == BAD)
        throwInvalidOp(rv);
    return rv;
}

CigarString CigarString::merge(CigarString a, CigarString b, uint32_t pos) {
    CigarString rv;
    a = a.subset(pos, b.length());

    while (b.length() > 0 && a.length() > 0) {
        switch (b[0].type) {
        case SEQ_MATCH:
        case MATCH: {
            uint32_t len = min(a[0].length, b[0].length);
            Op op(len, a[0].type);
            rv.push_back(op);
            a.pop_front(len);
            if (op.type != DEL)
                b.pop_front(len);
            }
            break;

        case SEQ_MISMATCH: {
            uint32_t len = min(a[0].length, b[0].length);
            rv.push_back(Op(len, SEQ_MISMATCH));
            a.pop_front(len);
            b.pop_front(len);
            }
            break;


        case INS: {
            rv.push_back(Op(b[0].length, INS));
            b.pop_front(b[0].length);
            }
            break;

        default:
            throwInvalidOp(b[0].type);
            break;
        }
    }

    return rv;
}

CigarString::CigarString() {
}

CigarString::CigarString(const string& data) {
    parse(data);
}

uint32_t CigarString::length() const {
    uint32_t len = 0;
    for (auto iter = _ops.begin(); iter != _ops.end(); ++iter) {
        switch (iter->type) {
            case MATCH:
            case INS:
            case SEQ_MATCH:
            case SEQ_MISMATCH:
                len += iter->length;
                break;

            default:
                break;
        }
    }
    return len;
}

void CigarString::parse(const string& data) {
    _ops.clear();
    Tokenizer<string> t(data, _op_to_chr);
    uint32_t length(0);
    while (t.extract(length)) {
        if (t.lastDelim() == '\0')
            break;
        Op op(length, translate(t.lastDelim()));
        if (op.type != BAD)
            push_back(op);
    }
}

CigarString::operator string() const {
    stringstream ss;
    ss << *this;
    return ss.str();
}

void CigarString::push_back(const Op& op) {
    if (!_ops.empty() && _ops.rbegin()->type == op.type)
        _ops.rbegin()->length += op.length;
    else
        _ops.push_back(op);
}

void CigarString::push_back(uint32_t len, CigarOpType op) {
    push_back(CigarString::Op(len, op));
}

void CigarString::concatenate(const CigarString& s) {
    for (auto i = s._ops.begin(); i != s._ops.end(); ++i)
        push_back(*i);
}

void CigarString::pop_front(uint32_t len) {
    while (len > 0 && !_ops.empty()) {
        if (_ops.begin()->length > len) {
            _ops.begin()->length -= len;
            len = 0;
        } else {
            len -= _ops.begin()->length;
            _ops.pop_front();
        }
    }
}

const deque<CigarString::Op> CigarString::ops() const {
    return _ops;
}

const CigarString::Op& CigarString::operator[](const uint32_t idx) const {
    assert(idx < _ops.size());
    return _ops[idx];
}

CigarString CigarString::subset(uint32_t offset, uint32_t len) const {
    CigarString rv;
    if (len == 0)
        return rv;

    uint32_t currPos = 0;
    uint32_t startIdx = 0;
    for (uint32_t i = 0; i < _ops.size() && currPos <= offset; ++i) {
        switch (_ops[i].type) {
            case MATCH:
            case INS:
            case SEQ_MATCH:
            case SEQ_MISMATCH:
                currPos += _ops[i].length;
                if (currPos > offset) {
                    startIdx = i;
                }
                break;

            case DEL:
            case SKIP:
            case SOFT_CLIP:
            case HARD_CLIP:
            case PADDING:
            default:
                break;
        }
    }

    if (currPos < offset)
        return rv;

    uint32_t diff = currPos - offset;
    Op first(diff, _ops[startIdx].type);
    rv.push_back(first);
    ++startIdx;
    len -= diff;
    for (uint32_t i = startIdx; i < _ops.size() && len != 0; ++i) {
        switch (_ops[i].type) {
            case MATCH:
            case INS:
            case SEQ_MATCH:
            case SEQ_MISMATCH: {
                uint32_t amt = min(_ops[i].length, len);
                Op op(amt, _ops[i].type);
                rv.push_back(op);
                len -= amt;
                } break;

            case DEL:
            case SKIP:
            case SOFT_CLIP:
            case HARD_CLIP:
            case PADDING:
                rv.push_back(_ops[i]);
                break;

            default:
                throwInvalidOp(_ops[i].type);
                break;
        }

    }
    return rv;
}

CigarString CigarString::structural() const {
    CigarString rv;
    for (auto iter = _ops.begin(); iter != _ops.end(); ++iter) {
        Op op;
        switch (iter->type) {
            case SEQ_MATCH:
            case SEQ_MISMATCH:
                op.type = MATCH;
                op.length = iter->length;
                rv.push_back(op);
                break;

            case MATCH:
            case INS:
            case DEL:
            case SOFT_CLIP:
                rv.push_back(*iter);
                break;

            default:
                throwInvalidOp(iter->type);
                break;
        }
    }

    return rv;
}

CigarString::const_iterator CigarString::begin() const {
    return _ops.begin();
}

CigarString::const_iterator CigarString::end() const {
    return _ops.end();
}


ostream& operator<<(ostream& s, const CigarString::Op& op) {
    s << op.length << CigarString::translate(op.type);
    return s;
}

ostream& operator<<(ostream& s, const CigarString& c) {
    const deque<CigarString::Op>& ops = c.ops();
    copy(ops.begin(), ops.end(), ostream_iterator<CigarString::Op>(s, ""));
    return s;
}
