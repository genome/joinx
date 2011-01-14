#include "fileformats/TranscriptStructure.hpp"

#include <string>
#include <gtest/gtest.h>

using namespace std;


TEST(TranscriptStructure, parseLine) {
    string entry = "618374,1\t6\t606\thuman\tgenbank\t54_36p\t26381,flank,-49994,5,2,,,human,genbank,54_36p,543,0,1,0,NULL,NULL,26381,21462\thuman\tgenbank\t54_36p,6,606,XM_001713842,model,-1,1,human,genbank,54_36p,LOC653884,no_start_codon:pseudogene,6,548,181";

    TranscriptStructure ts;
    TranscriptStructure::parseLine(entry, ts);

    ASSERT_EQ("1", ts.chrom());
    ASSERT_EQ(-1, ts.region().strand());
    ASSERT_EQ(-49994, ts.region().start());
    ASSERT_EQ(5, ts.region().stop());

    ASSERT_EQ("618374",
        ts.get(TranscriptStructure::transcript_structure_id));
    ASSERT_EQ(618374,
        ts.getAs<int64_t>(TranscriptStructure::transcript_structure_id));
    ASSERT_EQ("1\t6\t606\thuman\tgenbank\t54_36p\t26381",
        ts.get(TranscriptStructure::transcript_id));
    ASSERT_EQ("flank",
        ts.get(TranscriptStructure::structure_type));
    ASSERT_EQ("-49994",
        ts.get(TranscriptStructure::structure_start));
    ASSERT_EQ(-49994,
        ts.getAs<int64_t>(TranscriptStructure::structure_start));
    ASSERT_EQ("5",
        ts.get(TranscriptStructure::structure_stop));
    ASSERT_EQ("2",
        ts.get(TranscriptStructure::ordinal));
    ASSERT_EQ("",
        ts.get(TranscriptStructure::phase));
    ASSERT_EQ("",
        ts.get(TranscriptStructure::nucleotide_seq));
    ASSERT_EQ("human",
        ts.get(TranscriptStructure::species));
    ASSERT_EQ("genbank",
        ts.get(TranscriptStructure::source));
    ASSERT_EQ("54_36p",
        ts.get(TranscriptStructure::version));
    ASSERT_EQ("543",
        ts.get(TranscriptStructure::coding_bases_before));
	ASSERT_EQ("0",
        ts.get(TranscriptStructure::coding_bases_after));
	ASSERT_EQ("1",
        ts.get(TranscriptStructure::cds_exons_before));
	ASSERT_EQ("0",
        ts.get(TranscriptStructure::cds_exons_after));
	ASSERT_EQ("NULL",
        ts.get(TranscriptStructure::phase_bases_before));
	ASSERT_EQ("NULL",
        ts.get(TranscriptStructure::phase_bases_after));
	ASSERT_EQ("26381",
        ts.get(TranscriptStructure::transcript_transcript_id));
	ASSERT_EQ("21462\thuman\tgenbank\t54_36p",
        ts.get(TranscriptStructure::transcript_gene_id));
	ASSERT_EQ("6",
        ts.get(TranscriptStructure::transcript_transcript_start));
	ASSERT_EQ("606",
        ts.get(TranscriptStructure::transcript_transcript_stop));
	ASSERT_EQ("XM_001713842",
        ts.get(TranscriptStructure::transcript_transcript_name));
	ASSERT_EQ("model",
        ts.get(TranscriptStructure::transcript_transcript_status));
	ASSERT_EQ("-1",
        ts.get(TranscriptStructure::strand));
	ASSERT_EQ("1",
        ts.get(TranscriptStructure::transcript_chrom_name));
	ASSERT_EQ("human",
        ts.get(TranscriptStructure::transcript_species));
	ASSERT_EQ("genbank",
        ts.get(TranscriptStructure::transcript_source));
	ASSERT_EQ("54_36p",
        ts.get(TranscriptStructure::transcript_version));
	ASSERT_EQ("LOC653884",
        ts.get(TranscriptStructure::transcript_gene_name));
	ASSERT_EQ("no_start_codon:pseudogene",
        ts.get(TranscriptStructure::transcript_transcript_error));
	ASSERT_EQ("6",
        ts.get(TranscriptStructure::transcript_coding_region_start));
	ASSERT_EQ("548",
        ts.get(TranscriptStructure::transcript_coding_region_stop));
	ASSERT_EQ("181",
        ts.get(TranscriptStructure::transcript_amino_acid_length));
}
