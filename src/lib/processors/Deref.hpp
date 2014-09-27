#pragma once

template<typename OutputType>
class Deref {
public:
    Deref(OutputType& out)
        : out_(out)
    {}

    template<typename T>
    void operator()(T value) {
        out_(*value);
    }

private:
    OutputType& out_;
};

template<typename OutputType>
Deref<OutputType>
makeDeref(OutputType& out) {
    return Deref<OutputType>(out);
}
