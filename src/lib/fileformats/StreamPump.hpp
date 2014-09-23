#pragma once

#include <memory>
#include <utility>

template<typename StreamType, typename OutputFunc>
struct StreamPump {
    typedef typename StreamType::ValueType ValueType;

    StreamPump(StreamType& s, OutputFunc& out)
        : stream_(s)
        , out_(out)
    {
    }

    void execute() {
        ValueType entry;
        while (stream_.next(entry)) {
            out_(std::move(entry));
        }
        out_.flush();
    }

    StreamType& stream_;
    OutputFunc& out_;
};

template<typename StreamType, typename OutputFunc>
StreamPump<StreamType, OutputFunc>
makeStreamPump(StreamType& s, OutputFunc& out) {
    return StreamPump<StreamType, OutputFunc>(s, out);
}


template<typename StreamType, typename OutputFunc>
struct PointerStreamPump {
    typedef typename StreamType::ValueType ValueType;

    PointerStreamPump(StreamType& s, OutputFunc& out)
        : stream_(s)
        , out_(out)
    {
    }

    void execute() {
        std::unique_ptr<ValueType> entry(new ValueType);
        while (stream_.next(*entry)) {
            out_(std::move(entry));
            entry.reset(new ValueType);
        }
        out_.flush();
    }

    StreamType& stream_;
    OutputFunc& out_;
};

template<typename StreamType, typename OutputFunc>
PointerStreamPump<StreamType, OutputFunc>
makePointerStreamPump(StreamType& s, OutputFunc& out) {
    return PointerStreamPump<StreamType, OutputFunc>(s, out);
}
