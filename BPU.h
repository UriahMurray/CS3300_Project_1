#include <cstdint>

class BPU {
    // this is a simple branch predictor that
    // predicts every branch as taken.
    // Implement your own branch predictor here,
    // and call it from your processor.
    // If you implement it in a self-contained way,
    // you can enter the branch predictor contest and
    // win a prize!
    public:
        void update(uint32_t pc, bool taken){
            // call this when you
            // figure out whether the branch is actually taken
        };
        
        bool predict(uint32_t pc){
            // call this during instruction fetch
            return 1; // always predicts taken
        }
    
};
