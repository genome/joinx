#pragma once

template<typename ValueType, typename StreamType, typename OutputFunc>
struct StreamPump {
    StreamPump(StreamType& s, OutputFunc& out)
        : stream_(s)
        , out_(out)
    {
    }

    void execute() {
        ValueType entry;
        while (stream_.next(entry)) {
            out_(entry);
        }
    }

    StreamType& stream_;
    OutputFunc& out_;
};


template<typename ValueType, typename StreamType, typename OutputFunc>
StreamPump<ValueType, StreamType, OutputFunc>
makeStreamPump(StreamType& s, OutputFunc& out) {
    return StreamPump<ValueType, StreamType, OutputFunc>(s, out);
}
