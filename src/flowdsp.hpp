/************************************************
 * flowdsp - C++ DSP library
 * public domain / CC0
 * 2015, qnzy
 ************************************************/

#ifndef FLOWDSP_H
#define FLOWDSP_H

#include <vector>
#include <stdexcept>
#include <iostream>
#include <algorithm>

namespace dspa {

typedef double Tflow;

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
            }
            else
                throw std::runtime_error("dspnode: binding inexistant port");
        }
        // read the output; if tick didn't change since last call, return
        // the cachedData. inRead is used to break loops with stateful
        // nodes.
        Tflow read(int tick, int port = 0) {
            if (inRead) {
                process(tick);
            }
            else {
                inRead = true;
                if (tick != lastTick) {
                    process(tick);
                    lastTick = tick;
                }
                update(tick);
                inRead = false;
            }
            return outputs[port];
            // return outputs.at(port); // might be a better idea, but slower
        }
    protected:
        struct slot {dspnode*node; int port;};
        // get input from port, return 0 for invalid ports
        Tflow getInput(unsigned int slot, int tick) {
            if (inputs.at(slot).node!=NULL)
                return inputs.at(slot).node->read(tick, inputs.at(slot).port);
            else
                return 0.0;
        }
        std::vector<slot> inputs;
        std::vector<Tflow> outputs;
        Tflow cachedData;
        int lastTick;
        bool inRead;
        // virtual functions. process calculates output for given tick,
        // update is used to update the state of the node. if the node
        // can be used in a loop (eg delay) only update should read the
        // inputs. reset can be used to reset the state.
        virtual void process(int tick) = 0;
        virtual void update(int tick) {}
        virtual void reset() {}
};

/************************************************
 * callback source
 ************************************************/
class callbackSrc : public dspnode {
        Tflow (*getSample_) (void);
    public:
        callbackSrc(Tflow (*getSample) (void)) : 
            dspnode(0,1), getSample_(getSample) {}
        void process(int tick) {outputs.at(0) = getSample_();}
};

/************************************************
 * constant source
 ************************************************/
class constSrc : public dspnode {
        Tflow c_;
    public:
        constSrc(Tflow c) : dspnode(0,1), c_(c) {}
        void process(int tick) {outputs.at(0) = c_;}
};

/************************************************
 * reference source
 ************************************************/
class refSrc : public dspnode {
        Tflow &var_;
    public:
        refSrc(Tflow &var) : dspnode(0,1), var_(var)  {}
        void process(int tick) {outputs.at(0) = var_;}
};
  
/************************************************
 * summation block
 ************************************************/
class sum : public dspnode {
    public:
        sum(unsigned int numInputs) : dspnode(numInputs,1) {}
        void process(int tick) {
            Tflow accu = 0;
            for (unsigned int i=0; i<inputs.size(); i++)
                accu += getInput(i, tick);
            outputs.at(0) = accu;
        }
};

/************************************************
 * multiplication block
 ************************************************/
class mul : public dspnode {
    public:
        mul(unsigned int numInputs) : dspnode(numInputs,1) {}
        void process(int tick) {
            Tflow accu = 1;
            for (unsigned int i=0; i<inputs.size(); i++)
                accu *= getInput(i, tick);
            outputs.at(0) = accu;
        }
};


/************************************************
 * callback block
 ************************************************/
class callbackFunc : public dspnode {
        Tflow (*func_) (Tflow);
    public:
        callbackFunc(Tflow (*func) (Tflow)) : dspnode(1,1), func_(func) {}
        void process(int tick) {outputs.at(0) = func_(getInput(0, tick));}
};
                
/************************************************
 * delay block
 ************************************************/
class delay : public dspnode {
        unsigned int dlyIndex;
        std::vector<Tflow> dly;
    public:
        delay(unsigned int numDelay) : 
            dspnode(1,1), 
            dlyIndex(0),
            dly(numDelay, 0.0) {}
        void process(int tick) {outputs.at(0) = dly.at(dlyIndex%dly.size());}
        void update(int tick) {dly.at((dlyIndex++)%dly.size())=getInput(0,tick);}
};

/************************************************
 * mux block
 ************************************************/
class mux : public dspnode {
    public:
        mux(int numInputs) : dspnode(numInputs, 1) {}
        void process(int tick) {
            outputs.at(0) = getInput(
                    std::min(int(inputs.size()-1), 
                        std::max(1, 
                            int(getInput(0, tick)+0.5))), tick);
        }
};

}; // namespace
#endif