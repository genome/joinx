#pragma once

#include "ValueMergers.hpp"
#include "namespace.hpp"

#include <map>
#include <string>

class InputStream;

VCF_NAMESPACE_BEGIN

class CustomType;
class CustomValue;
class Entry;
class Header;

/// A class for specifying how to merge Vcf::CustomValue data types 
/// from multiple Vcf::Entry records.
///
/// Merge strategies can be built explicitly in code (e.g., setMerger("DP", "sum"))
/// or parsed from an input stream (MergeStrategy::parse(header, instream)).
class MergeStrategy {
public:
    /// Parse a merge strategy from the input given stream. Call this on an existing
    /// MergeStrategy instance to add the strategies specified in the stream to the
    /// plan.
    /// \param header a merged vcf header
    /// \param description an input stream describing the desired merge strategy
    /// \exception runtime_error thrown if the description contains errors
    void parse(InputStream& description);

    /// Create a new empty merge strategy for the given header
    /// \param header a merged vcf header
    explicit MergeStrategy(const Header* header);

    /// Retrieve the ValueMerger object responsible for merging the named info field.
    /// \param id names an info field
    /// \exception runtime_error if no action can be found to handle the field
    const ValueMergers::Base* infoMerger(const std::string& id) const;

    /// Merge the info field specified by 'which' in the given range of entries
    /// \param which the name of the info field to merge
    /// \param begin the beginning of the range of entries to merge
    /// \param end the end of the range of entries to merge
    /// \return a CustomValue object representing the result of the merge
    /// \exception runtime_error thrown if the info field is invalid, or if no action can
    ///   be found to handle the field named by 'which'
    CustomValue mergeInfo(const std::string& which, const Entry* begin, const Entry* end) const;

    /// Set the handler for the info field
    /// \param id the name of the info field to set the action for
    /// \param the name of the action to set. actions are defined in ValueMergers.hpp.
    /// \exception runtime_error thrown if the action name is not known
    void setMerger(const std::string& id, const std::string& mergerName);

    /// If set to true, clearFilters will strip all filter information from merged 
    /// entries.
    /// \param boolean value indication whether or not to strip merged filters
    void clearFilters(bool value);

    /// \return a boolean value indicating whether or not merged Entries should have
    /// their filters stripped
    bool clearFilters() const;

protected:
    /// The merged Vcf header for the final output file
    const Header* _header;
    /// Map of info field names to ValueMergers (e.g., DP,sum to sum depth fields)
    std::map<std::string, const ValueMergers::Base*> _info;
    /// The default ValueMerger object to use when none is specified for a particular field
    const ValueMergers::Base* _default;
    /// This registry allows looking up ValueMergers by name 
    const ValueMergers::Registry* _registry;
    bool _clearFilters;
};

VCF_NAMESPACE_END
