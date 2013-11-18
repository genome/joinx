#pragma once

#include "AlleleMerger.hpp"
#include "common/namespaces.hpp"

#include <boost/function.hpp>
#include <boost/noncopyable.hpp>
#include <boost/scoped_ptr.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/unordered_map.hpp>

#include <functional>
#include <memory>
#include <string>

BEGIN_NAMESPACE(Vcf)

class CustomType;
class CustomValue;
class Entry;

namespace ValueMergers {
    typedef AlleleMerger::AltIndices AltIndices;

    /// Base class for all "Value Mergers": callable structs that can be
    /// used to combine CustomValue objects extracted from Vcf::Entry objects.
    /// These are useful for doing things like merging INFO, FILTER, or FORMAT
    /// records in vcf files.
    struct Base {
        typedef boost::shared_ptr<const Base> const_ptr;
        typedef boost::function<const CustomValue*(Entry const*)> FetchFunc;
        virtual ~Base() {};
        /// the name of the merger. this is used when specifying merge
        /// strategies from a text file or from the command line
        virtual std::string name() const = 0;

        /// all value mergers must override this function to perform the actual
        /// merge
        /// \param type the Type of the CustomValue objects being merged
        /// \param fetch a functor that will extract the desired CustomValue
        ///   given a Vcf::Entry. For example, this might be an object that
        ///   calls entry->info("DP") to retrieve the depth INFO value
        /// \param begin the beginning of the range of Vcf::Entry objects to
        ///   merge
        /// \param end the end of the range of Vcf::Entry objects to merge
        /// \return the new merged CustomValue
        virtual CustomValue operator()(
            CustomType const* type,
            FetchFunc fetch,
            Entry const* begin,
            Entry const* end,
            AltIndices const& newAltIndices
            ) const = 0;
    };

    /// Registry of all available ValueMergers, which can be retrieved by name.
    /// This facilitates parsing MergeStrategies.
    /// This class functions as a singleton, so it cannot be instantiated
    /// directly. To retrieve a pointer to the one instance of it, call
    /// getInstance().
    class Registry : public boost::noncopyable {
    public:
        /// Retrieve a pointer to the instance of this class
        static const Registry* getInstance();

        /// Retrieve a pointer to the the merger named by name
        /// \param name the name of the merger to retrieve
        /// \exception runtime_error thrown when the specified merger is unknown
        Base const* getMerger(std::string const& name) const;
    protected:
        /// used internally to build the list of all mergers
        void registerMerger(Base::const_ptr const& merger);
        /// protected constructor, no outside instantiation allowed!
        Registry();
    protected:
        /// holds a pointer to the singleton instance of this class
        static boost::scoped_ptr<Registry> _instance;
        /// a map of the available mergers, keyed by name
        boost::unordered_map<std::string, Base::const_ptr> _mergers;
    };

    /// Value Merger which uses the value from the first entry, ignoring the rest
    struct UseFirst : public Base {
        CustomValue operator()(
            CustomType const* type,
            FetchFunc fetch,
            Entry const* begin,
            Entry const* end,
            AltIndices const& newAltIndices
            ) const;
        std::string name() const { return "first"; }
    };

    /// Value Merger which will return a new variable length CustomValue
    /// containing all of the unique values found in the input range.
    struct UniqueConcat : public Base {
        CustomValue operator()(
            CustomType const* type,
            FetchFunc fetch,
            Entry const* begin,
            Entry const* end,
            AltIndices const& newAltIndices
            ) const;
        std::string name() const { return "uniq-concat"; }
    };

    /// The main purpose of this object is to ensure that all entries have the
    /// same value for the specified field. An exception is thrown if this
    /// is not the case.
    struct EnforceEquality : public Base {
        CustomValue operator()(
            CustomType const* type,
            FetchFunc fetch,
            Entry const* begin,
            Entry const* end,
            AltIndices const& newAltIndices
            ) const;
        std::string name() const { return "enforce-equal"; }
    };

    /// Returns the summation of the input values. This is only meaningful for
    /// non-variable length numeric data.
    struct Sum : public Base {
        CustomValue operator()(
            CustomType const* type,
            FetchFunc fetch,
            Entry const* begin,
            Entry const* end,
            AltIndices const& newAltIndices
            ) const;
        std::string name() const { return "sum"; }
    };

    /// Returns NULL, effectively eliminating the CustomValue from the final
    /// output.
    struct Ignore : public Base {
        CustomValue operator()(
            CustomType const* type,
            FetchFunc fetch,
            Entry const* begin,
            Entry const* end,
            AltIndices const& newAltIndices
            ) const;
        std::string name() const { return "ignore"; }
    };

    struct PerAltDelimitedList : public Base {
        CustomValue operator()(
            CustomType const* type,
            FetchFunc fetch,
            Entry const* begin,
            Entry const* end,
            AltIndices const& newAltIndices
            ) const;
        std::string name() const { return "per-alt-delimited-list"; }
    };
}

END_NAMESPACE(Vcf)
