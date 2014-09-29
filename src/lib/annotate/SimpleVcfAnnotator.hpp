#pragma once

#include "common/compat.hpp"
#include "fileformats/vcf/Compare.hpp"
#include "fileformats/vcf/CustomType.hpp"
#include "fileformats/vcf/CustomValue.hpp"
#include "fileformats/vcf/Entry.hpp"
#include "fileformats/vcf/Header.hpp"

#include <map>
#include <string>
#include <vector>

struct InfoTranslation {
    Vcf::CustomType const* newType;
    bool singleToPerAlt;
};

template<typename OutputType>
class SimpleVcfAnnotator {
public:
    typedef std::unique_ptr<Vcf::Entry> EntryPtr;
    typedef std::vector<EntryPtr> EntryPtrVector;

    typedef std::vector<Vcf::Entry> HitsType;
    typedef HitsType::const_iterator BestIter;
    typedef std::map<std::string, InfoTranslation> InfoFieldMapping;
    typedef Vcf::Compare::AltIntersect AltIntersect;

public:
    SimpleVcfAnnotator(OutputType& out,
            bool copyIdents,
            InfoFieldMapping const& infoMap,
            Vcf::Header const& header
            )
        : _out(out)
        , _copyIdents(copyIdents)
        , _infoMap(infoMap)
        , _header(header)
    {
    }

    size_t infoMatches(Vcf::Entry const& b) const {
        size_t rv(0);
        for (auto i = _infoMap.begin(); i != _infoMap.end(); ++i) {
            if (b.info(i->first))
                ++rv;
        }
        return rv;
    }

    BestIter bestMatch(
            Vcf::Entry const& a,
            HitsType const& b,
            AltIntersect::ResultType& altMatches) const
    {
        altMatches.clear();
        size_t maxInfoMatches(0);
        auto rv = b.end();
        for (auto iter = b.begin(); iter != b.end(); ++iter) {
            AltIntersect altIntersector;
            auto tmpMatches = altIntersector(a, *iter);
            if (tmpMatches.empty() || tmpMatches.size() < altMatches.size())
                continue;

            size_t iMatches = infoMatches(*iter);
            if (tmpMatches.size() > altMatches.size() || iMatches > maxInfoMatches) {
                altMatches.swap(tmpMatches);
                rv = iter;
                maxInfoMatches = iMatches;
            }
        }
        return rv;
    }

    void setInfo(
            Vcf::Entry& e,
            AltIntersect::ResultType const& altMatches,
            Vcf::CustomValue const& v,
            InfoTranslation const& txl)
    {
        if (txl.singleToPerAlt) {
            Vcf::CustomValue rv(txl.newType);
            rv.setNumAlts(e.alt().size());
            // TODO: make this more efficient instead of using intermediate strings
            // TODO: assert that v is of type flag or number=1
            auto const* existingValue = v.getAny(0);
            if (existingValue) {
                for (auto i = altMatches.begin(); i != altMatches.end(); ++i) {
                    rv.set(i->first, *existingValue);
                }
            } 
            e.setInfo(txl.newType->id(), rv);
        }
        else {
            std::unique_ptr<Vcf::CustomValue> newValue;
            if (v.type().tiedToAlleles()) {
                newValue = std::make_unique<Vcf::CustomValue>(&v.type());
                auto const& raw = v.getRaw();
                std::vector<Vcf::CustomValue::ValueType> newVec;
                std::size_t offset = 0;

                if (v.type().numberType() == Vcf::CustomType::PER_ALLELE_REF) {
                    offset = 1;
                    newVec.resize(e.alt().size() + offset);
                    newVec[0] = raw[0];
                }
                else {
                    newVec.resize(e.alt().size());
                }

                for (auto i = altMatches.begin(); i != altMatches.end(); ++i) {
                    newVec[i->first + offset] = raw[i->second + offset];
                }
                newValue->setRaw(newVec);
            }
            else {
                newValue = std::make_unique<Vcf::CustomValue>(v);
            }

            newValue->setType(txl.newType);
            e.setInfo(txl.newType->id(), *newValue);
        }
    }

    void operator()(std::vector<std::unique_ptr<Vcf::Entry>> entries) {
        EntryPtrVector inputs; // entries from the input file

        // FIXME: change this to have HitsType be unique_ptrs so we can just
        // move those rather than the heavier entry objects.
        HitsType annos;  // entries from the annotation file

        for (auto i = entries.begin(); i != entries.end(); ++i) {
            if ((*i)->header().sourceIndex() == 0) {
                inputs.push_back(std::move(*i));
            }
            else {
                annos.push_back(std::move(**i));
            }
        }

        for (auto i = inputs.begin(); i  != inputs.end(); ++i) {
            (*this)(**i, annos);
        }
    }

    void operator()(Vcf::Entry const& a, HitsType const& b) {
        AltIntersect::ResultType altMatches;
        auto best = bestMatch(a, b, altMatches);
        // no compatible matches in b
        if (best == b.end()) {
            _out(a);
            return;
        }

        Vcf::Entry copyA(a);
        if (_copyIdents) {
            auto idents = best->identifiers();
            for (auto id = idents.begin(); id != idents.end(); ++id)
                copyA.addIdentifier(*id);
        }

        for (auto i = _infoMap.begin(); i != _infoMap.end(); ++i) {
            Vcf::CustomValue const* inf = best->info(i->first);
            if (inf) {
                setInfo(copyA, altMatches, *inf, i->second);
            }
        }

        _out(copyA);
    }

protected:
    OutputType& _out;
    bool _copyIdents;
    InfoFieldMapping const& _infoMap;
    Vcf::Header const& _header;
};
