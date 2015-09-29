// Copyright (C) 2015 team-diana MIT license

#define BOOST_TEST_MODULE Cansim_test
#include <boost/test/unit_test.hpp>

#include "hlcanopen/logging/easylogging++.h"
#include "hlcanopen/can_msg.hpp"

#include "cansim/bus.hpp"
#include "cansim/card.hpp"

#include <iostream>
#include <thread>
#include <vector>

INITIALIZE_EASYLOGGINGPP

using namespace std;
using namespace hlcanopen;

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
    std::shared_ptr<Bus<CanMsg>> bus = std::make_shared<Bus<CanMsg>>();

    Card a(1, bus);
    Card b(2, bus);
    Card c(3, bus);

    thread at = thread([&]() {
        CanMsg msgA;
        msgA.data = 0xA0;
        a.write(msgA);
        CanMsg bmsg = a.read();
        BOOST_CHECK_EQUAL(0xB0, bmsg.data);
        bmsg = a.read();
        BOOST_CHECK_EQUAL(0xB1, bmsg.data);
    });

    thread bt = thread([&]() {
        CanMsg amsg = b.read();
        CanMsg msgB0; msgB0.data = 0xB0;
        CanMsg msgB1; msgB1.data = 0xB1;
        BOOST_CHECK_EQUAL(0xA0, amsg.data);
        b.write(msgB0);
        b.write(msgB1);
    });

    thread ct = thread([&]() {
        CanMsg amsg = c.read();
        BOOST_CHECK_EQUAL(0xA0, amsg.data);
        CanMsg bmsg = c.read();
        BOOST_CHECK_EQUAL(0xB0, bmsg.data);
        bmsg = c.read();
        BOOST_CHECK_EQUAL(0xB1, bmsg.data);
    });

    at.join();
    bt.join();
    ct.join();
}

