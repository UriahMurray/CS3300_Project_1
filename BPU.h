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
        int take = 0;
      //  bool last = true;
        void update(uint32_t pc, bool taken){
            if (taken) {

                if (take < 3) {take++;}

            }
            else {
                if (take > 0) {take--;}

            }
        };

        bool predict(uint32_t pc){
        //    last = (take > 1);
            return (take > 1); // predicts taken when 10 or 11 else (00, 01) not taken
        }

};
