read.per_sample_file=function(filename) {
    read.table(filename, header=T, sep="\t")->per_sample_file;
    invisible(per_sample_file);
}

per_sample.rare_mutation_boxplot=function(x) {
    library(ggplot2);
    #this refactors for nice boxplots
    melt(x[,c(1,9,10,11)])->melted;

    ggplot(melted, aes(x=variable,y=value)) + geom_boxplot() + xlab("Minor Allele Frequency Class") + ylab("Number") + opts(title = "Minor Allele Frequency Classes By Sample")
}

per_sample.genotype_class_fraction_boxplot=function(x) {
    library(ggplot2);
    #this refactors for nice boxplots
    ratio_call = x[,c(4,5,6,7)]/x$TotalSites;
    ratio_call = cbind(x$SampleName,ratio_call);
    melt(ratio_call)->melted;

    ggplot(melted, aes(x=variable,y=value)) + geom_boxplot() + xlab("Genotype Class") + ylab("Frequency") + opts(title = "Genotype Classes By Sample");
}

per_sample.genotype_class_count_boxplot=function(x) {
    library(ggplot2);
    #this refactors for nice boxplots
    temp = x[,c(1,4,5,6,7)];
    names(temp)=c("SampleName","Heterozygous","Homozygous","Filtered","Missing");
    melt(temp)->melted;

    ggplot(melted, aes(x=variable,y=value)) + geom_boxplot() + xlab("Genotype Class") + ylab("Number of Calls") + opts(title = "Genotype Classes By Sample");
}

per_sample.ts_tv_histogram=function(x) {
    library(ggplot2);
    ggplot(x, aes(x=Transition.Transversion)) + geom_histogram() + xlab("Ts:Tv Ratio") + ylab("Number") + opts(title = "Ts:Tv Ratio By Sample") + geom_vline(xintercept=mean(x$Transition.Transversion),colour="red") + annotate("text",x=mean(x$Transition.Transversion),y=0,hjust=1.01,vjust=1.01,colour="red",label=paste("Mean Ts:Tv=",round(mean(x$Transition.Transversion),2)),size=4);
}

per_sample.known_histogram=function(x) {
    library(ggplot2);
    ggplot(x, aes(x=PercKnown)) + geom_histogram() + xlab("Percentage of Known Variants") + ylab("Number") + opts(title = "Percentage of Known Variants By Sample") + geom_vline(xintercept=mean(x$PercKnown),colour="red") + annotate("text",x=mean(x$PercKnown),y=0,hjust=1.01,vjust=1.01,colour="red",label=paste("Mean Percentage Known Variants=",round(mean(x$PercKnown),2)),size=4);
}

read.per_site_file=function(filename) {
    read.table(filename, header=T, sep="\t")->per_site_file;
    invisible(per_site_file);
}

per_site.maf_histogram=function(x) {
    library(ggplot2);
    ggplot(x, aes(x=MAF)) + geom_histogram(binwidth=0.005) + xlab("Minimum MAF (>0)") + ylab("Number") +  opts(title = "Minor Allele Frequency Distribution");
}

per_site.maf_frequency_histogram=function(x) {
    ggplot(x, aes(x=MAF,y = ..count../sum(..count..))) + geom_histogram(binwidth=0.005) + xlab("Minor Allele Frequency") + ylab("Frequency") + opts(title = "Minor Allele Frequency Distribution")
}

per_site.maf_scatter=function(x) {
    ggplot(x, aes(x=c(1:nrow(x)),y=MAF)) + geom_point() + xlab("Minor Allele Frequency") + ylab("number") + opts(title = "Minor Allele Frequency Distribution")
}

per_site.total_ts_tv=function(x) {
    transition = 0;
    transversion = 0;
    wts = 0;
    wtv = 0;
    for(i in 1:nrow(x)) {
        alts = unlist(strsplit(as.character(x[i,]$Alt),","));
        transitionStatus = as.numeric(unlist(strsplit(as.character(x[i,]$ByAltTransition),",")));
        sampleCount = as.numeric(unlist(strsplit(as.character(x[i,]$AlleleDistBySample),",")));
        #alleleCount = as.numeric(unlist(strsplit(as.character(x[i,]$AlleleDist),",")));
        for(a in 1:length(alts)) {
            if(transitionStatus[a] && sampleCount[a+1]) {
                transition = transition + 1;
                wts = wts + sampleCount[a+1]; 
            }
            else {
                if(!transitionStatus[a] && sampleCount[a+1]) {
                    transversion = transversion + 1;
                    wtv = wtv + sampleCount[a+1];
                }
            }
        }
    }
    list(transitions = transition, transversions = transversion, weightedTs = wts, weightedTv = wtv, unweighted_ts_tv_ratio = transition/transversion, weighted_ts_tv_ratio = wts/wtv);
}
