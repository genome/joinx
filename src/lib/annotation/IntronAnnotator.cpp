#include "IntronAnnotator.hpp"

#include "Variant.hpp"
#include "fileformats/TranscriptStructure.hpp"

#include <boost/algorithm/string.hpp>
#include <boost/lexical_cast.hpp>
#include <sstream>
#include <algorithm>

using boost::lexical_cast;

using namespace std;

string IntronAnnotator::codingRegionString(const Variant& v, const TranscriptStructure& structure) const {
    stringstream rv;
    int64_t start = v.start();
    int64_t stop = v.end();
    if (structure.region().strand() == -1)
        swap(start, stop);

    int64_t distToStart = structure.region().distanceFromStart(start) + 1;
    int64_t distToStop = structure.region().distanceFromStop(start) + 1;

    int64_t codingBasesBefore = structure.numCodingBasesBefore();
    if (structure.hasCodingRegion()) {
        const Region& cr = structure.codingRegion();
        Region::RelativePos relPos = cr.distance(start);

        if (relPos.in()) {
            if (distToStart <= distToStop) {
                rv << codingBasesBefore << "+" << distToStart;
                if (start != stop) {
                    int64_t dist = min(int64_t(1), distToStart - abs(stop-start));
                    rv << "_" << codingBasesBefore << "+" << dist;
                }
            } else {
                rv << (codingBasesBefore+1) << "-" << distToStop;
                if (start != stop) {
                    int64_t dist = min(int64_t(1), distToStop - abs(stop-start));
                    rv << "_" << (codingBasesBefore+1) << "-" << dist;
                }
            }
        } else {
            if (relPos.before())
                rv << '1';
            else
                rv << codingBasesBefore;

            if (distToStart <= distToStop) {
                rv << "+" << distToStart;
                if (start != stop)
                    rv << "_" << (distToStart + (stop-start));
            } else {
                rv << "-" << distToStop;
                if (start != stop)
                    rv << "_" << (distToStop - (stop-start));
            }
        }
    }
    
    return rv.str();
}

IntronAnnotator::PropertyMapType IntronAnnotator::annotate(const Variant& v, const TranscriptStructure& structure) const {
    PropertyMapType rv;
    rv["c_position"] = codingRegionString(v, structure);

    int64_t distToStart = v.start() - (structure.region().start() - 1);
    int64_t distToStop = (structure.region().stop() + 1) - v.end();
    int64_t edgeDist = min(distToStart, distToStop);

    int64_t intronPosition;
    if (structure.region().strand() == 1)
        intronPosition = distToStart;
    else
        intronPosition = distToStop;

    // If variant occurs within 2bp of a neighboring structure, it's splice site
    // If variant occurs within 3-10bp, it's splice region
    // Otherwise, it's intronic

    string typeSuffix;
    if (v.isIndel())
        typeSuffix = "_" + v.typeString();

    if (edgeDist <= 2)
        rv["trv_type"] = "splice_site" + typeSuffix;
    else if (edgeDist <= 10)
        rv["trv_type"] = "splice_region" + typeSuffix;
    else
        rv["trv_type"] = "intronic";

    if (structure.cdsExonsBefore() > 0 || structure.cdsExonsAfter() > 0) {
        stringstream ss;
        // For indels, the distance to the edge of the bordering exon can be negative if the indel starts in the intron
        // and carries over to the exon. This should eventually be fixed by splitting up variants in the _transcript_annotation
        // method above. For now, set the distance to edge to 1 if its negative
        // This is actually a pretty big bug, since currently the only structure annotated is the one at the variant's start. This
        // needs to be changed such that all structures within the range of the variant are annotated.
        // TODO Remove once variants spanning structures are handled correctly
        edgeDist = min(int64_t(1), edgeDist);

        // If position in the intron is equal to the distance to the closest edge, 
        // this splice site is just after an exon
        if (intronPosition == edgeDist)
            ss << "e" << structure.cdsExonsBefore() << "+" << edgeDist;
        else
            ss << "e" << (structure.cdsExonsBefore()+1) << "-" << edgeDist;
    
        rv["amino_acid_change"] = ss.str();
    } else {
        rv["amino_acid_change"] = "NULL";
    }

    rv["intron_annotation_substructure_ordinal"] = structure.get(TranscriptStructure::ordinal);
    rv["intron_annotation_substructure_size"] = lexical_cast<string>(structure.region().length());
    rv["intron_annotation_substructure_position"] = lexical_cast<string>(intronPosition);

    return rv;
}
