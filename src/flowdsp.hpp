/************************************************
 * flowdsp - C++ DSP library
 * public domain / CC0
 ************************************************/

#ifndef FLOWDSP_H
#define FLOWDSP_H

#include <algorithm>
#include <iostream>
#include <stdexcept>
#include <vector>

namespace dspa {

using Tflow = double ;

/************************************************
 * base class for all processing nodes
 ************************************************/
class dspnode {
    public:
        // constructor
        dspnode (int numInputs, int numOutputs) : 
            inputs(numInputs), outputs(numOutputs),
            cachedData(0), lastTick(-1), inRead(false) {}
        // bind input port to node
        void bind(unsigned int slot, dspnode * node, unsigned int port) {
            if (port < inputs.size()) {
                inputs.at(slot).node = node;
                inputs.at(slot).port = port;
            } else {
                throw std::runtime_error("dspnode: binding inexistant port");
            }
        }
        // read the output; if tick didn't change since last call, return
        // the cachedData. inRead is used to break loops with stateful
        // nodes.
        Tflow read(int tick, int port = 0) {
            if (!inRead) {
                inRead = true;
                if (tick != lastTick) {
                    process(tick);
                    lastTick = tick;
                }
                inRead = false;
            }
            return outputs[port];
            // return outputs.at(port); // might be a better idea, but slower
        }
    protected:
        struct slot {dspnode*node; int port;};
        // get input from port, return 0 for invalid ports
        Tflow getInput(unsigned int slot, int tick) {
            if (inputs.at(slot).node!=nullptr) {
                return inputs.at(slot).node->read(tick, inputs.at(slot).port);
            } 
            return 0.0;
        }
        std::vector<slot> inputs;
        std::vector<Tflow> outputs;
        Tflow cachedData;
        int lastTick;
        bool inRead;
        // determines the next output value
        virtual void process(int tick) = 0;
        virtual void reset() {}
};

/************************************************
 * callback source
 ************************************************/
class callbackSrc : public dspnode {
        Tflow (*getSample_) ();
    public:
        explicit callbackSrc(Tflow (*getSample) ()) : 
            dspnode(0,1), getSample_(getSample) {}
        void process(int /*tick*/) override {outputs.at(0) = getSample_();}
};

/************************************************
 * constant source
 ************************************************/
class constSrc : public dspnode {
        Tflow c_;
    public:
        explicit constSrc(Tflow c) : dspnode(0,1), c_(c) {}
        void process(int /*tick*/) override {outputs.at(0) = c_;}
};

/************************************************
 * reference source
 ************************************************/
class refSrc : public dspnode {
        const Tflow &var_;
    public:
        explicit refSrc(const Tflow &var) : dspnode(0,1), var_(var)  {}
        void process(int /*tick*/) override {outputs.at(0) = var_;}
};
  
/************************************************
 * summation block
 ************************************************/
class sum : public dspnode {
    public:
        explicit sum(unsigned int numInputs) : dspnode(numInputs,1) {}
        void process(int tick) override {
            Tflow accu = 0;
            for (unsigned int i=0; i<inputs.size(); i++) {
                accu += getInput(i, tick);
            }
            outputs.at(0) = accu;
        }
};

/************************************************
 * multiplication block
 ************************************************/
class mul : public dspnode {
    public:
        explicit mul(unsigned int numInputs) : dspnode(numInputs,1) {}
        void process(int tick) override {
            Tflow accu = 1;
            for (unsigned int i=0; i<inputs.size(); i++) {
                accu *= getInput(i, tick);
            }
            outputs.at(0) = accu;
        }
};


/************************************************
 * callback block
 ************************************************/
class callbackFunc : public dspnode {
        Tflow (*func_) (Tflow);
    public:
        explicit callbackFunc(Tflow (*func) (Tflow)) : dspnode(1,1), func_(func) {}
        void process(int tick) override {outputs.at(0) = func_(getInput(0, tick));}
};
                
/************************************************
 * delay block
 ************************************************/
class delay : public dspnode {
        unsigned int dlyIndex;
        std::vector<Tflow> dly;
    public:
        explicit delay(unsigned int numDelay) : 
            dspnode(1,1), 
            dlyIndex(0),
            dly(numDelay, 0.0) {}
        void process(int tick) override {dly.at((dlyIndex++)%dly.size())=getInput(0,tick);outputs.at(0) = dly.at(dlyIndex%dly.size());}
};

/************************************************
 * mux block
 ************************************************/
class mux : public dspnode {
    public:
        explicit mux(int numInputs) : dspnode(numInputs, 1) {}
        void process(int tick) override {
            outputs.at(0) = getInput(
                    std::min(int(inputs.size()-1), 
                        std::max(1, 
                            int(getInput(0, tick)))), tick);
        }
};

} // namespace dspa
#endif
