#include "catch.hpp"
#include "../src/flowdsp.hpp"

using namespace dspa;

TEST_CASE("constant source") {
    constSrc csrc(3.0);
    CHECK(csrc.read(0) == Approx(3.0));
}

Tflow get2() { return 2.0; }
TEST_CASE("sum of constant and callback sources") {
    callbackSrc cs1(get2);
    constSrc cs2(3.0);
    sum s(2);
    s.bind(0, &cs1, 0);
    s.bind(1, &cs2, 0);
    CHECK(s.read(0) == Approx(5.0));
    CHECK(s.read(1) == Approx(5.0));
}

TEST_CASE("refSrc") {
    Tflow var;
    var = 1.0;
    refSrc rs(var);
    CHECK(rs.read(0) == Approx(1.0));
    var = 5.0;
    CHECK(rs.read(1) == Approx(5.0));
    var = -17;
    CHECK(rs.read(1) == Approx(5.0));
    CHECK(rs.read(0) == Approx(-17.0));
}

TEST_CASE("multiplication") {
    constSrc cs1(3.0), cs2(4.0), cs3(1.5);
    mul m(3);
    m.bind(0, &cs1, 0);
    m.bind(1, &cs2, 0);
    m.bind(2, &cs3, 0);
    CHECK(m.read(0) == Approx(3.0*4.0*1.5));
}

Tflow func(Tflow x) {return 2*x+1;}
TEST_CASE("callback function") {
    callbackFunc cf(func);
    constSrc cs(2.5);
    cf.bind(0, &cs, 0);
    CHECK(cf.read(0) == Approx(6.0));
}

TEST_CASE("integrator loop") {
    constSrc cs1(1.0);
    delay dly(1);
    sum s(2);
    s.bind(0,&cs1, 0);
    s.bind(1,&dly, 0);
    dly.bind(0,&s, 0);
    CHECK(s.read(0) == Approx(1.0));
    CHECK(s.read(1) == Approx(2.0));
    CHECK(s.read(0) == Approx(3.0));
    CHECK(s.read(0) == Approx(3.0));
    CHECK(s.read(1) == Approx(4.0));
    for (long i = 0; i<1e7; i++)
        s.read(i%1);
}


class swapPorts : public dspnode {
    public:
        swapPorts(void) : dspnode(2,2) {}
        void process(int tick) {
            outputs.at(0) = getInput(1, tick);
            outputs.at(1) = getInput(0, tick);
        }
};
TEST_CASE("swapPorts") {
    constSrc cs1(1.0), cs2(2.0);
    swapPorts swp;
    swp.bind(0,&cs1,0);
    swp.bind(1,&cs2,0);
    CHECK(swp.read(0,0) == Approx(2.0));
    CHECK(swp.read(1,1) == Approx(1.0));
}

TEST_CASE("mux") {
    constSrc cs1(1.0), cs2(2.0);
    Tflow sel;
    refSrc rs(sel);
    mux m(3);
    m.bind(0, &rs, 0);
    m.bind(1, &cs1, 0);
    m.bind(2, &cs2, 0);
    sel=0;
    CHECK(m.read(0) == Approx(1));
    sel=1;
    CHECK(m.read(1) == Approx(1));
    sel=2;
    CHECK(m.read(0) == Approx(2));
    sel=100;
    CHECK(m.read(1) == Approx(2));
    sel=-100;
    CHECK(m.read(0) == Approx(1));
}

