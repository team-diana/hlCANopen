// Copyright (C) 2015 team-diana MIT license

#ifndef BI_PIPE_HPP
#define BI_PIPE_HPP

#include "pipe.hpp"

#include <utility>
#include <thread>
#include <memory.h>

// Bidirectional Pipe
template<class T> class BiPipe {
public:

    ~BiPipe() {

    }

    template<class M> void write(M&& m) {
        writePipe->write(std::forward<M>(m));
    }

    T read() {
        return readPipe->read();
    }

    static std::tuple<std::shared_ptr<BiPipe<T>>, std::shared_ptr<BiPipe<T>>> make() {
        std::shared_ptr<Pipe<T>> apipe = std::make_shared<Pipe<T>>();
        std::shared_ptr<Pipe<T>> bpipe = std::make_shared<Pipe<T>>();

        return std::make_tuple(std::shared_ptr<BiPipe<T>>(new BiPipe<T>(apipe, bpipe)),
                               std::shared_ptr<BiPipe<T>>(new BiPipe<T>(bpipe, apipe)));
    }

private:
    BiPipe(std::shared_ptr<Pipe<T>> readPipe, std::shared_ptr<Pipe<T>> writePipe) :
        readPipe(readPipe),
        writePipe(writePipe) {
    }
    BiPipe(const BiPipe& channel) = delete;

private:
    std::shared_ptr<Pipe<T>> readPipe;
    std::shared_ptr<Pipe<T>> writePipe;

};

#endif // BI_PIPE_HPP

