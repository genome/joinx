#include "fileformats/vcf/MergeStrategy.hpp"
#include "fileformats/vcf/Entry.hpp"
#include "fileformats/vcf/Header.hpp"
#include "fileformats/vcf/ValueMergers.hpp"
#include "fileformats/InputStream.hpp"

#include <gtest/gtest.h>
#include <stdexcept>
#include <sstream>
#include <string>

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
        "##INFO=<ID=DP,Number=1,Type=Integer,Description=\"Depth\">\n"
        "##INFO=<ID=FET,Number=1,Type=Float,Description=\"Fisher's exact test\">\n"
        "##INFO=<ID=FOO,Number=.,Type=String,Description=\"FOO\">\n"
        "##INFO=<ID=BAR,Number=1,Type=String,Description=\"BAR\">\n"
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
        "##INFO=<ID=DP,Number=1,Type=Integer,Description=\"Depth\">\n"
        "##INFO=<ID=FET,Number=1,Type=Float,Description=\"Fisher's exact test\">\n"
        "##INFO=<ID=FOO,Number=.,Type=String,Description=\"FOO\">\n"
        "##INFO=<ID=BAR,Number=1,Type=String,Description=\"BAR\">\n"
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
        "##INFO=<ID=DP,Number=1,Type=Integer,Description=\"Depth\">\n"
        "##INFO=<ID=FET,Number=1,Type=Float,Description=\"Fisher's exact test\">\n"
        "##INFO=<ID=FOO,Number=.,Type=String,Description=\"FOO\">\n"
        "##INFO=<ID=BAR,Number=1,Type=String,Description=\"BAR\">\n"
        "##INFO=<ID=VC,Number=.,Type=String,Description=\"Variant caller\">\n"
        "##FORMAT=<ID=GT,Number=1,Type=String,Description=\"Genotype\">\n"
        "##FORMAT=<ID=GQ,Number=1,Type=Integer,Description=\"Genotype Quality\">\n"
        "##FORMAT=<ID=DP,Number=1,Type=Integer,Description=\"Read Depth\">\n"
        "##FORMAT=<ID=HQ,Number=2,Type=Integer,Description=\"Haplotype Quality\">\n"
        "#CHROM\tPOS\tID\tREF\tALT\tQUAL\tFILTER\tINFO\tFORMAT\tNA5\tNA6\n"
    };

    string snvEntryText[] = {
        "20\t14370\tid1\tG\tA\t29\tPASS\tVC=Samtools\tGT:GQ:DP:HQ\t0|1:48:1:51,51\t1|0:48:8:51,51",
        "20\t14370\tid1;id2\tG\tC\t.\tPASS\tVC=Samtools\tGT:GQ:DP:HQ\t0|1:48:1:51,51\t1/1:43:5:.,.",
        "20\t14370\tid3\tG\tC\t31\tPASS\tVC=Varscan,Samtools\tGT:GQ:DP:HQ\t.\t1/0:44:6:50,40"
    };

    string indelEntryText[] = {
        "20\t14370\tid1\tTAC\tT\t29\tPASS\tVC=Samtools\tGT:GQ:DP:HQ\t0|1:48:1:51,51\t1|0:48:8:51,51",
        "20\t14370\tid1\tTACAG\tT\t.\tPASS\tVC=Samtools"
    };

}

class TestVcfMergeStrategy : public ::testing::Test {
public:
    void SetUp() {
        unsigned nHeaders = sizeof(headerText)/sizeof(headerText[0]);
        for (unsigned i = 0; i < nHeaders; ++i) {
            stringstream ss(headerText[i]);
            _headers.push_back(Header::fromStream(ss));
        }

        unsigned nSnvs = sizeof(snvEntryText)/sizeof(snvEntryText[0]);
        assert(nSnvs <= nHeaders);

       for (unsigned i = 0; i < nSnvs; ++i) {
            _mergedHeader.merge(_headers[i]);
            stringstream ss(snvEntryText[i]);
            Entry e;
            Entry::parseLine(&_headers[i], snvEntryText[i], e);
            _snvs.push_back(e);
        }

        unsigned nIndels = sizeof(indelEntryText)/sizeof(indelEntryText[0]);
        assert(nIndels <= nHeaders);

       for (unsigned i = 0; i < nIndels; ++i) {
            stringstream ss(indelEntryText[i]);
            Entry e;
            Entry::parseLine(&_headers[i], indelEntryText[i], e);
            _indels.push_back(e);
        }

        _defaultMs.reset(new MergeStrategy(&_mergedHeader));
    }
    Header _mergedHeader;
    vector<Entry> _snvs;
    vector<Entry> _indels;
    vector<Header> _headers;
    unique_ptr<MergeStrategy> _defaultMs;
};

TEST_F(TestVcfMergeStrategy, parse) {
    stringstream ss;
    ss <<
        "DP=sum\n"
        "FET=ignore\n"
        "FOO=uniq-concat\n"
        "BAR=enforce-equal\n"
        ;

    InputStream in("test", ss);
    MergeStrategy strategy(&_mergedHeader);
    ASSERT_NO_THROW(strategy.parse(in));
    const ValueMergers::Base* merger(0);

    ASSERT_TRUE( (merger = strategy.infoMerger("DP")) );
    ASSERT_EQ("sum", merger->name());

    ASSERT_TRUE( (merger = strategy.infoMerger("FET")) );
    ASSERT_EQ("ignore", merger->name());

    ASSERT_TRUE( (merger = strategy.infoMerger("FOO")) );
    ASSERT_EQ("uniq-concat", merger->name());

    ASSERT_TRUE( (merger = strategy.infoMerger("BAR")) );
    ASSERT_EQ("enforce-equal", merger->name());

    ASSERT_THROW(strategy.infoMerger("invalid"), runtime_error);
}

TEST_F(TestVcfMergeStrategy, parseInvalidInfoField) {
    stringstream ss;
    ss <<
        "DP=sum\n"
        "FET=ignore\n"
        "BADNAME=uniq-concat\n"
        ;

    InputStream in("test", ss);
    MergeStrategy strategy(&_mergedHeader);
    ASSERT_THROW(strategy.parse(in), runtime_error);
}

TEST_F(TestVcfMergeStrategy, parseInvalidMerger) {
    stringstream ss;
    ss <<
        "DP=sum\n"
        "FET=ignore\n"
        "FOO=uniq-concat\n"
        "BAR=something-bad\n"
        ;

    InputStream in("test", ss);
    MergeStrategy strategy(&_mergedHeader);
    ASSERT_THROW(strategy.parse(in), runtime_error);
}
