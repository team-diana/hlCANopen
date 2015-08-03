// Copyright (C) 2015 team-diana MIT license

#define BOOST_TEST_MODULE Cansim_test
#include <boost/test/unit_test.hpp>

#include "cansim/bus.hpp"
#include "cansim/card.hpp"

#include <iostream>
#include <thread>
#include <vector>

#include "hlcanopen/logging/easylogging++.h"

INITIALIZE_EASYLOGGINGPP

using namespace std;

struct Msg {
    std::string data;
};

bool operator==(const Msg& lhs, const Msg& rhs) {
    return  lhs.data == rhs.data;
}

BOOST_AUTO_TEST_CASE(PipeTest) {
    Pipe<Msg> pipe1, pipe2;
    vector<string> output;

    thread otherThread([&] {
        int i;
        for(i = 0; i < 2; i++) {
            output.push_back(pipe1.read().data);
        }
        pipe2.write(Msg{"ok"});
    });

    pipe1.write(Msg {"hello"});
    pipe1.write(Msg {"world"});

    BOOST_CHECK_EQUAL("ok", pipe2.read().data);
    BOOST_CHECK_EQUAL("hello", output[0]);
    BOOST_CHECK_EQUAL("world", output[1]);
    otherThread.join();
}

BOOST_AUTO_TEST_CASE(BusTest) {
    std::shared_ptr<Bus<Msg>> bus = std::make_shared<Bus<Msg>>();

    Card<Msg> a(1, bus);
    Card<Msg> b(2, bus);
    Card<Msg> c(3, bus);

    thread at = thread([&]() {
        a.write(Msg {"msg_a_0"});
        Msg bmsg = a.read();
        BOOST_CHECK_EQUAL("msg_b_0", bmsg.data);
        bmsg = a.read();
        BOOST_CHECK_EQUAL("msg_b_1", bmsg.data);
    });

    thread bt = thread([&]() {
        Msg amsg = b.read();
        BOOST_CHECK_EQUAL("msg_a_0", amsg.data);
        b.write(Msg {"msg_b_0"});
        b.write(Msg {"msg_b_1"});
    });

    thread ct = thread([&]() {
        Msg amsg = c.read();
        BOOST_CHECK_EQUAL("msg_a_0", amsg.data);
        Msg bmsg = c.read();
        BOOST_CHECK_EQUAL("msg_b_0", bmsg.data);
        bmsg = c.read();
        BOOST_CHECK_EQUAL("msg_b_1", bmsg.data);
    });

    at.join();
    bt.join();
    ct.join();
}

