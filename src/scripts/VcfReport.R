read.per_sample_file=function(filename) {
    read.table(filename, header=T, sep="\t")->per_sample_file;
    invisible(per_sample_file);
}

per_sample.rare_mutation_boxplot=function(x) {
    library(ggplot2);
    #this refactors for nice boxplots
    #be careful here as it doesn't use the headers
    melt(x[,c("SampleName","Singleton","VeryRare","Rare")])->melted;

    print(ggplot(melted, aes(x=variable,y=value)) + geom_boxplot() + xlab("Minor Allele Frequency Class") + ylab("Number") + opts(title = "Minor Allele Frequency Classes By Sample"));
}

per_sample.genotype_class_fraction_boxplot=function(x) {
    library(ggplot2);
    #this refactors for nice boxplots
    ratio_call = x[,c(4,5,6,7)]/x$TotalSites;
    ratio_call = cbind(x$SampleName,ratio_call);
    names(ratio_call) = c("SampleName","Heterozygous","Homozygous","Filtered","Missing");
    melt(ratio_call)->melted;

    print(ggplot(melted, aes(x=variable,y=value)) + geom_boxplot() + xlab("Genotype Class") + ylab("Frequency") + opts(title = "Genotype Classes By Sample"));
}

per_sample.genotype_class_count_boxplot=function(x) {
    library(ggplot2);
    #this refactors for nice boxplots
    temp = x[,c(1,4,5,6,7)];
    names(temp)=c("SampleName","Heterozygous","Homozygous","Filtered","Missing");
    melt(temp)->melted;

    print(ggplot(melted, aes(x=variable,y=value)) + geom_boxplot() + xlab("Genotype Class") + ylab("Number of Calls") + opts(title = "Genotype Classes By Sample"));
}

per_sample.ts_tv_histogram=function(x) {
    library(ggplot2);
    print(ggplot(x, aes(x=Transition.Transversion)) + geom_histogram() + xlab("Ts:Tv Ratio") + ylab("Number") + opts(title = "Ts:Tv Ratio By Sample") + geom_vline(xintercept=mean(x$Transition.Transversion),colour="red") + annotate("text",x=mean(x$Transition.Transversion),y=0,hjust=1.01,vjust=1.01,colour="red",label=paste("Mean Ts:Tv=",round(mean(x$Transition.Transversion),2)),size=4));
}

per_sample.known_histogram=function(x) {
    library(ggplot2);
    print(ggplot(x, aes(x=PercKnown)) + geom_histogram() + xlab("Percentage of Known Variants") + ylab("Number") + opts(title = "Percentage of Known Variants By Sample") + geom_vline(xintercept=mean(x$PercKnown),colour="red") + annotate("text",x=mean(x$PercKnown),y=0,hjust=1.01,vjust=1.01,colour="red",label=paste("Mean Percentage Known Variants=",round(mean(x$PercKnown),2)),size=4));
}

read.per_site_file=function(filename) {
    read.table(filename, header=T, sep="\t")->per_site_file;
    invisible(per_site_file);
}

per_site.maf_histogram=function(x) {
    library(ggplot2);
    print(ggplot(x, aes(x=MAF)) + geom_histogram(binwidth=0.005) + xlab("Minimum MAF (>0)") + ylab("Number") +  opts(title = "Minor Allele Frequency Distribution"));
}

per_site.maf_frequency_histogram=function(x) {
    print(ggplot(x, aes(x=MAF,y = ..count../sum(..count..))) + geom_histogram(binwidth=0.005) + xlab("Minor Allele Frequency") + ylab("Frequency") + opts(title = "Minor Allele Frequency Distribution"));
}

per_site.maf_scatter=function(x) {
    print(ggplot(x, aes(x=c(1:nrow(x)),y=MAF)) + geom_point() + xlab("Minor Allele Frequency") + ylab("number") + opts(title = "Minor Allele Frequency Distribution"));
}

per_site.total_ts_tv <- function(x) {
    transitionStatus <- as.numeric(unlist(strsplit(as.character(x$ByAltTransition),",")))
    sampleCount <- as.numeric(unlist(sapply(strsplit(as.character(x$AlleleDistBySample),","),"[",-1)))
    transition = sum(transitionStatus[sampleCount>0])
    transversion = sum(!transitionStatus[sampleCount>0])
    wts = sum(sampleCount[transitionStatus>0])
    wtv = sum(sampleCount[transitionStatus==0])
    list(transitions = transition, transversions = transversion, weightedTs = wts, weightedTv = wtv, unweighted_ts_tv_ratio = transition/transversion, weighted_ts_tv_ratio = wts/wtv);
}

per_site.total_known=function(x) {
    novelStatus = as.numeric(unlist(strsplit(as.character(x$ByAltNovel),",")));
    sampleCount = as.numeric(unlist(sapply(strsplit(as.character(x$AlleleDistBySample),","),"[",-1)));
    novel = sum(novelStatus[sampleCount>0]);
    known = sum(!novelStatus[sampleCount>0]);
    wnovel = sum(sampleCount[novelStatus>0]);
    wknown = sum(sampleCount[novelStatus==0]);
    list(Novel = novel, Known = known, weightedNovel = wnovel, weightedKnown = wknown, unweighted_known_pct = known/(known+novel)*100, weighted_known_pct = wknown/(wknown+wnovel)*100);
}

callset_metrics=function(per_site, per_sample) {
    #this whole thing would probably be more efficient in C++ as part of the report program...
    total_sites = nrow(per_site);   #sites total
    tstv = per_site.total_ts_tv(per_site); #tstv
    known = per_site.total_known(per_site);
    sample_number = nrow(per_sample);
    mean_variant = mean(per_sample$Het+per_sample$Hom);
    mean_variant_sd = sd(per_sample$Het+per_sample$Hom);
    mean_known_pct = mean(per_sample$PercKnown);
    mean_known_pct_sd = sd(per_sample$PercKnown);
    mean_filtered = mean(per_sample$Filt);
    mean_filtered_sd = sd(per_sample$Filt);
    mean_missing = mean(per_sample$Missing);
    mean_missing_sd = sd(per_sample$Missing);
    mean_singleton = mean(per_sample$Singleton);
    mean_singleton_sd = sd(per_sample$Singleton);
    mean_sample_tstv = mean(per_sample$Transition.Transversion);
    mean_sample_tstv_sd = sd(per_sample$Transition.Transversion);

    #output a bunch of stuff here
    report = data.frame();
    report = rbind(report,"Number of Samples"=list(value=sample_number))
    report = rbind(report,"Total Segregating Sites"=list(value=total_sites))
    report = rbind(report,"Call Set Ts:Tv Ratio"=list(value=tstv$unweighted_ts_tv_ratio))
    report = rbind(report,"Call Set Known Percentage"=list(value=known$unweighted_known_pct))
    report = rbind(report,"Call Set Known Percentage by Sample Genotype"=list(value=known$weighted_known_pct))
    report = rbind(report,"Mean variants per sample"=list(value=mean_variant))
    report = rbind(report,"Standard deviation of variants per sample"=list(value=mean_variant_sd))
    report = rbind(report,"Filtered variants per sample"=list(value=mean_filtered))
    report = rbind(report,"Standard deviation of filtered variants per sample"=list(value=mean_filtered_sd))
    report = rbind(report,"Mean missing genotypes per sample"=list(value=mean_missing))
    report = rbind(report,"Standard deviation of missing genotypes per sample"=list(value=mean_missing_sd))
    report = rbind(report,"Mean singletons per sample"=list(value=mean_singleton))
    report = rbind(report,"Standard deviation of singletons per sample"=list(value=mean_singleton_sd))
    report = rbind(report,"Mean Ts:Tv per sample"=list(value=mean_sample_tstv))
    report = rbind(report,"Standard deviation of Ts:Tv per sample"=list(value=mean_sample_tstv_sd))
    report = rbind(report,"Mean Percent Known variants per sample"=list(value=mean_known_pct))
    report = rbind(report,"Standard deviation of Percent Known per sample"=list(value=mean_known_pct_sd))

    invisible(format(report,scientific=F,digits=2,drop0trailing=T,trim=T));
}

write_image=function(image_type="pdf", file_basename, draw_func, func_args=...) {
    image_name = paste(file_basename,image_type,sep=".");
    if(image_type == "pdf") {
        pdf(file=image_name,height=6.5,width=9);  #1 inch margins on a default PP slide
    }
    else if(image_type == "png") {
        png(file=image_name,width=9,height=6.5,units="in",res=300);   #1 inch margins on a default PP slide. High quality resolution.
    }
    else {
        stop("Unrecognized image type");
    }
    print(dev.cur());
    draw_func(func_args);
    dev.off();
}

write_reports=function(per_site_file="per_site_report.txt",per_sample_file="per_sample_report.txt",image_type="pdf",callset_file="callset_metrics.txt") {
    library(ggplot2);
    read.per_site_file(per_site_file)->per_site;
    read.per_sample_file(per_sample_file)->per_sample;
    #per sample call set graphs
    write_image(image_type=image_type,file_basename="PerSampleTsTv",draw_func=per_sample.ts_tv_histogram,per_sample);
    write_image(image_type=image_type,file_basename="PerSampleRareMutation",draw_func=per_sample.rare_mutation_boxplot,per_sample);
    write_image(image_type=image_type,file_basename="PerSampleGenotypeClassFraction",draw_func=per_sample.genotype_class_fraction_boxplot,per_sample);
    write_image(image_type=image_type,file_basename="PerSampleGenotypeClassCount",draw_func=per_sample.genotype_class_count_boxplot,per_sample);
    write_image(image_type=image_type,file_basename="PerSampleKnown",draw_func=per_sample.known_histogram,per_sample);
    #per site call set graphs
    write_image(image_type=image_type,file_basename="PerSiteMafCount",draw_func=per_site.maf_histogram,per_site);
    write_image(image_type=image_type,file_basename="PerSiteMafFrequency",draw_func=per_site.maf_frequency_histogram,per_site);
    
    metrics = callset_metrics(per_site, per_sample);
    write.table(metrics,file=callset_file,sep="\t",quote=F,col.names=F);
}




