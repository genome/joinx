#pragma once

#include "common/Variant.hpp"

#include <boost/format.hpp>
#include <cmath>
#include <cstdint>
#include <stdexcept>
#include <string>
#include <vector>

class RemappedContig {
public:
    RemappedContig(
            const std::string& chrom,
            uint32_t start,
            uint32_t stop,
            const std::string pre,
            const char* var,
            std::string::size_type varLen,
            const std::string post,
            int insertedBases
            )
    {
        using boost::format;
        using std::string;

        _sequence = pre;
        _sequence.append(var, varLen);
        _sequence += post;

        string cigar;
        if (insertedBases == 0) {
            cigar = str(format("%1%M") %_sequence.size());
        } else {
            char insOrDel = insertedBases > 0 ? 'I' : 'D';
            cigar = str(format("%1%M%2%%3%%4%M")
                %pre.size()
                %std::abs(insertedBases)
                %insOrDel
                %post.size()
            );
        }

        _name = str(format("REMAP-%1%,%2%,%3%-%4%")
            %chrom
            %start
            %stop
            %cigar
            );
    }

    const std::string& name() const     { return _name; }
    const std::string& sequence() const { return _sequence; }

protected:
    std::string _name;
    std::string _sequence;
};

template<typename RefSeq, typename Callback>
class RemappedContigGenerator {
public:

    RemappedContigGenerator(RefSeq& refSeq, unsigned flankSize, Callback& cb)
        : _refSeq(refSeq)
        , _flankSize(flankSize)
        , _cb(cb)
    {
    }

    void generate(const Variant& v) {
        using std::string;
        int32_t start = v.start() > _flankSize ? v.start() - _flankSize : 0;
        int32_t stop = v.stop() + _flankSize;

        string preFlank;
        string postFlank;
        _refSeq.sequence(v.chrom(), start, v.start(), preFlank);
        stop = _refSeq.sequence(v.chrom(), v.stop(), stop, postFlank);

        const string& ref = v.reference().data();
        const string& var = v.variant().data();

        if (v.type() == Variant::SNP) {
            const char* bases = translateIub(var);
            while (*bases) {
                if (*bases != ref[0]) {
                    RemappedContig ctg(
                        v.chrom(),
                        start,
                        stop,
                        preFlank,
                        bases, 1,
                        postFlank,
                        0
                        );
                    _cb(ctg);
                }
                ++bases;
            }
        } else {
            int isize = v.type() == Variant::DEL ? -ref.size() : var.size();
            int strLen = v.type() == Variant::DEL ? 0 : var.size();
            RemappedContig ctg(
                v.chrom(),
                start,
                stop,
                preFlank,
                &var[0], strLen,
                postFlank,
                isize
                );
                _cb(ctg);
        }
    }
protected:
    RefSeq& _refSeq;
    unsigned _flankSize;
    Callback& _cb;
};
