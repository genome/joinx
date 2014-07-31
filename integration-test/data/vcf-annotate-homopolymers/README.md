# Summary
This tool should annotate out all indels in (or adjacent to) homopolymer runs where the alternate allele is:

1. An indel
2. Of length less than some assigned parameter
3. Completely matches the homopolymer base

# Tests
```
     12345678910
Ref: TAGGGGGGGT
```
# Bed file

`1	2	9	G`

# Examples that should be filtered
## Adjacent insertion on left
```
Ref:  TA--GGGGGGGT
Alt1: TAG-GGGGGGGT
Alt2: TAGGGGGGGGGT

1	2	.	A	AG,AGG	.	.	.	.	.
```

## Adjacent insertion on right
```
Ref:  TAGGGGGGG--T
Alt1: TAGGGGGGGG-T
Alt2: TAGGGGGGGGGT

1	9	.	G	GG,GGG	.	.	.	.	.
```

## Adjacent insertion explicit representation
```
Ref:  TA--GGGGGGGT
Alt1: TAG-GGGGGGGT
Alt2: TAGGGGGGGGGT 

1	2	.	AGGGGGGGT	AGGGGGGGGT,AGGGGGGGGGT	.	.	.	.	.
```

## Deletion
```
Ref: TAGGGGGGGT
Alt: TA-GGGGGGT

1	2	.	AG	A	.	.	.	.	.
```

## Multiple Deletions
```
Ref:  TAGGGGGGGT
Alt1: TA-GGGGGGT
Alt2: TA--GGGGGT

1	2	.	AGG	AG,A	.	.	.	.	.
```

## Multiple Deletions (Reciprocal Representation)
```
Ref:  TAGGGGGGGT
Alt1: TA-GGGGGGT
Alt2: TA--GGGGGT

1	2	.	AGG	A,AG	.	.	.	.	.
```

# Examples that should not be filtered
## SNP
```
Ref: TAGGGGGGGT
Alt: TAAGGGGGGT
1	3	.	G	A	.	.	.	.	.
```

## Complete Removal of the homopolymer
```
Ref: TAGGGGGGGT
Alt: TA-------T

1	2	.	AGGGGGGGT	AT	.	.	.	.	.
```

## Insertions not matching the homopolymer
```
Ref: TAG-GGGGGGT
Alt: TAGCGGGGGGT

1	3	.	G	GC	.	.	.	.	.
```

## Deletions adjacent to the homopolymer
### Before
```
Ref: TAGGGGGGGT
Alt: T-GGGGGGGT

1	1	.	TA	T	.	.	.	.	.
```

### After
```
Ref: TAGGGGGGGT
Alt: TAGGGGGGG-

1	9	.	GT	G	.	.	.	.	.
```

## Insertions adjacent to the homoplymer but not matching
### Before
```
Ref: TA-GGGGGGGT
Alt: TAAGGGGGGGT

1	2	.	A	AA	.	.	.	.	.
```

### After
```
Ref: TAGGGGGGGCT
Alt: TAGGGGGGG-T

1	9	.	G	GT	.	.	.	.	.
```
