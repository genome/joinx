# Summary
This tool should annotate out all indels in (or adjacent to) homopolymer runs where the alternate allele is:
1. An indel
2. Of length less than some assigned parameter
3. Completely matches the homopolymer base

# Tests
     12345678910
Ref: TAGGGGGGGT

# Bed file
1	2	9	G

# Examples that should be filtered
## Adjacent insertion on left
Ref: TA-GGGGGGGT
Alt: TAGGGGGGGGT
1	2	.	A	AG,AGG	.	.	.	.	.

## Adjacent insertion on right
Ref: TAGGGGGGG-T
Alt: TAGGGGGGGGT
1	9	.	G	GG,GGG	.	.	.	.	.

## Adjacent insertion explicit representation
Ref: TA-GGGGGGGT
Alt: TAGGGGGGGGT
1	2	.	AGGGGGGGT	AGGGGGGGGT,AGGGGGGGGGT	.	.	.	.	.

## Deletion
Ref: TAGGGGGGGT
Alt: TA-GGGGGGT
1	2	.	AG	A	.	.	.	.	.

## Multiple Deletions
Ref: TAGGGGGGGT
Alt: TA-GGGGGGT
1	2	.	AGG	AG,A	.	.	.	.	.

## Multiple Deletions
Ref: TAGGGGGGGT
Alt: TA-GGGGGGT
1	2	.	AGG	A,AG	.	.	.	.	.

# Examples that should not be filtered
## SNP
Ref: TAGGGGGGGT
Alt: TA-GGGGGGT
1	3	.	G	A	.	.	.	.	.

