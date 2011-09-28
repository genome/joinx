#include "fileformats/vcf/EntryMerger.hpp"
#include "fileformats/vcf/MergeStrategy.hpp"
#include "fileformats/vcf/Entry.hpp"
#include "fileformats/vcf/Header.hpp"

#include <gtest/gtest.h>
#include <cassert>
#include <memory>
#include <sstream>
#include <string>
#include <vector>

using namespace std;
using namespace Vcf;

namespace {
    string headerText[] = {
        "##fileformat=VCFv4.1\n"
        "##fileDate=20090805\n"
        "##source=myImputationProgramV3.1\n"
        "##reference=file:///seq/references/1000GenomesPilot-NCBI36.fasta\n"
        "##contig=<ID=20,length=62435964,assembly=B36,md5=f126cdf8a6e0c7f379d618ff66beb2da,species=\"Homo sapiens\",taxonomy=x>\n"
        "##phasing=partial\n"
        "##INFO=<ID=VC,Number=.,Type=String,Description=\"Variant caller\">\n"
        "##FORMAT=<ID=GT,Number=1,Type=String,Description=\"Genotype\">\n"
        "##FORMAT=<ID=GQ,Number=1,Type=Integer,Description=\"Genotype Quality\">\n"
        "##FORMAT=<ID=DP,Number=1,Type=Integer,Description=\"Read Depth\">\n"
        "##FORMAT=<ID=HQ,Number=2,Type=Integer,Description=\"Haplotype Quality\">\n"
        "#CHROM\tPOS\tID\tREF\tALT\tQUAL\tFILTER\tINFO\tFORMAT\tNA1\tNA2\n"
        ,
        "##fileformat=VCFv4.1\n"
        "##fileDate=20090805\n"
        "##source=myImputationProgramV3.1\n"
        "##reference=file:///seq/references/1000GenomesPilot-NCBI36.fasta\n"
        "##contig=<ID=20,length=62435964,assembly=B36,md5=f126cdf8a6e0c7f379d618ff66beb2da,species=\"Homo sapiens\",taxonomy=x>\n"
        "##phasing=partial\n"
        "##INFO=<ID=VC,Number=.,Type=String,Description=\"Variant caller\">\n"
        "##FORMAT=<ID=GT,Number=1,Type=String,Description=\"Genotype\">\n"
        "##FORMAT=<ID=GQ,Number=1,Type=Integer,Description=\"Genotype Quality\">\n"
        "##FORMAT=<ID=DP,Number=1,Type=Integer,Description=\"Read Depth\">\n"
        "##FORMAT=<ID=HQ,Number=2,Type=Integer,Description=\"Haplotype Quality\">\n"
        "#CHROM\tPOS\tID\tREF\tALT\tQUAL\tFILTER\tINFO\tFORMAT\tNA3\tNA4\n"
        ,
        "##fileformat=VCFv4.1\n"
        "##fileDate=20090805\n"
        "##source=myImputationProgramV3.1\n"
        "##reference=file:///seq/references/1000GenomesPilot-NCBI36.fasta\n"
        "##contig=<ID=20,length=62435964,assembly=B36,md5=f126cdf8a6e0c7f379d618ff66beb2da,species=\"Homo sapiens\",taxonomy=x>\n"
        "##phasing=partial\n"
        "##INFO=<ID=VC,Number=.,Type=String,Description=\"Variant caller\">\n"
        "##FORMAT=<ID=GT,Number=1,Type=String,Description=\"Genotype\">\n"
        "##FORMAT=<ID=GQ,Number=1,Type=Integer,Description=\"Genotype Quality\">\n"
        "##FORMAT=<ID=DP,Number=1,Type=Integer,Description=\"Read Depth\">\n"
        "##FORMAT=<ID=HQ,Number=2,Type=Integer,Description=\"Haplotype Quality\">\n"
        "#CHROM\tPOS\tID\tREF\tALT\tQUAL\tFILTER\tINFO\tFORMAT\tNA5\tNA6\n"
    };

    string entryText[] = {
        "20\t14370\tid1\tG\tA\t29\tPASS\tVC=Samtools\tGT:GQ:DP:HQ\t0|1:48:1:51,51\t1|0:48:8:51,51",
        "20\t14370\tid1;id2\tG\tC\t31\tPASS\tVC=Samtools\tGT:GQ:DP:HQ\t0|1:48:1:51,51\t1/1:43:5:.,.",
        "20\t14370\tid3\tG\tC\t31\tPASS\tVC=Varscan,Samtools\tGT:GQ:DP:HQ\t.\t1/0:44:6:50,40"
    };
}

class TestVcfMerge : public ::testing::Test {
public:
    void SetUp() {
        unsigned nHeaders = sizeof(headerText)/sizeof(headerText[0]);
        for (unsigned i = 0; i < nHeaders; ++i) {
            stringstream ss(headerText[i]);
            _headers.push_back(Header::fromStream(ss));
        }

        unsigned nEntries = sizeof(entryText)/sizeof(entryText[0]);
        assert(nEntries <= nHeaders);

       for (unsigned i = 0; i < nEntries; ++i) {
            _mergedHeader.merge(_headers[i]);
            stringstream ss(entryText[i]);
            Entry e;
            Entry::parseLine(&_headers[i], entryText[i], e);
            _entries.push_back(e);
        }

        _defaultMs.reset(new MergeStrategy(&_mergedHeader));
    }
    Header _mergedHeader;
    vector<Entry> _entries;
    vector<Header> _headers;
    unique_ptr<MergeStrategy> _defaultMs;
};

TEST_F(TestVcfMerge, merge) {
    EntryMerger merger(*_defaultMs, &_mergedHeader, &*_entries.begin(), &*_entries.end());
    ASSERT_EQ("20", merger.chrom());
    ASSERT_EQ(14370, merger.pos());
    ASSERT_EQ(3, merger.identifiers().size());
    ASSERT_EQ("G", merger.ref());
    ASSERT_EQ(Entry::MISSING_QUALITY, merger.qual());

    Entry mergedEntry(merger);

    // check the simple fields: chrom, pos, etc.
    ASSERT_EQ("20", mergedEntry.chrom());
    ASSERT_EQ(14370, mergedEntry.pos());

    // make sure identifiers are merged properly without duplicates
    ASSERT_EQ(3, mergedEntry.identifiers().size());
    ASSERT_EQ("id1", mergedEntry.identifiers()[0]);
    ASSERT_EQ("id2", mergedEntry.identifiers()[1]);
    ASSERT_EQ("id3", mergedEntry.identifiers()[2]);

    // make sure reference allele doesn't change
    ASSERT_EQ("G", mergedEntry.ref());

    // for now, we are assigning combined quality scores of '.' to merged entries
    ASSERT_EQ(Entry::MISSING_QUALITY, mergedEntry.qual());

    // check that genotype allele references were updated
    // sample 1 (from entry 1)
    const CustomValue* v = mergedEntry.genotypeData(0, "GT");
    ASSERT_TRUE(v);
    ASSERT_EQ("0|1", v->toString());

    // sample 2 (from entry 1)
    v = mergedEntry.genotypeData(1, "GT");
    ASSERT_TRUE(v);
    ASSERT_EQ("1|0", v->toString());

    // sample 3 (from entry 2)
    v = mergedEntry.genotypeData(2, "GT");
    ASSERT_TRUE(v);
    ASSERT_EQ("0|2", v->toString());

    // sample 4 (from entry 2)
    v = mergedEntry.genotypeData(3, "GT");
    ASSERT_TRUE(v);
    ASSERT_EQ("2/2", v->toString());

    // sample 5 (from entry 3)
    v = mergedEntry.genotypeData(4, "GT");
    ASSERT_FALSE(v);

    // sample 6 (from entry 3)
    v = mergedEntry.genotypeData(5, "GT");
    ASSERT_TRUE(v);
    ASSERT_EQ("2/0", v->toString());

    // check that the VC field was merged
    v = mergedEntry.info("VC");
    ASSERT_TRUE(v);
    ASSERT_EQ("Samtools,Varscan", v->toString());
}

TEST_F(TestVcfMerge, mergeWrongPos) {
    EntryMerger merger(*_defaultMs, &_mergedHeader, &*_entries.begin(), &*_entries.end());
    Entry wrongPos(&_headers[2], "20\t14371\tid1\tG\tA\t29\t.\t.\t");
    Entry e[] = { _entries[0], wrongPos };
    ASSERT_THROW(EntryMerger(*_defaultMs, &_mergedHeader, e, e+2), runtime_error);

    Entry wrongChrom(&_headers[2], "21\t14370\tid1\tG\tA\t29\t.\t.\t");
    e[1] = wrongChrom;
    ASSERT_THROW(EntryMerger(*_defaultMs, &_mergedHeader, e, e+2), runtime_error);
}
