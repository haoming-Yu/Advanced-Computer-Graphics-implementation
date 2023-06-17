#include "bitstream.h"
#include "config.h"
#include "mask.h"
#include "mmask.h"
#include "mqrspec.h"
#include "qrencode_inner.h"
#include "qrencode.h"
#include "qrinput.h"
#include "qrspec.h"
#include "rsecc.h"
#include "split.h"
// include the qrencode lib's header file

#include "pre_process/pre_process.h"

#include <opencv2/core/core.hpp>
#include <vector>
#include <unordered_set>
/*
 * The following defines are used 
 * as utility expansion to the original lib
 * 
 */
#define QRspec_rsCodeWords1(__spec__) (QRspec_rsDataCodes1(__spec__) + QRspec_rsEccCodes1(__spec__))
#define QRspec_rsCodeWords2(__spec__) (QRspec_rsDataCodes2(__spec__) + QRspec_rsEccCodes2(__spec__))
#define QRspec_rsEcCapacity(__spec__) (__spec__[5])

class mRSblock;

std::vector<QRmodule*> generate_qr(int version, 
char* input_str, std::unordered_set<int>& necessary, 
cv::Mat& qr_img, std::vector<mRSblock*>& QR_rs_blocks);

class CodeWord
{
public:
    CodeWord() {
        this->Tag = -1;
        this->R_val = 0.0;
        this->weight = 0.0;
        this->priority = 0;
        std::vector<QRmodule*> *tmp = new std::vector<QRmodule*>;
        this->modules = *tmp;
    }
    ~CodeWord() {
        this->Tag = -1;
        this->R_val = 0.0;
        this->weight = 0.0;
        this->priority = 0;
        this->modules.clear();
    }
public:
    int Tag; // -1 -> non-used codeword
    double R_val; // reliability
    double weight; // maybe not used by our framework, represent the avg importance
    double priority; // priority of codeword
    std::vector<QRmodule*> modules; // modules (store only the ptrs)
};

class mRSblock
{
public:
    mRSblock() {
        this->Tag = -1;
        this->R_val = 0.0;
        std::vector<CodeWord*>* tmp = new std::vector<CodeWord*>();
        this->CodeWords = *tmp;
    }
    ~mRSblock() {
        this->Tag = -1;
        this->R_val = 0.0;
        this->CodeWords.clear();
    }
public:
    int Tag; // label the usage, if -1 -> non-used RS block
    double R_val; // reliability
    std::vector<CodeWord*> CodeWords; // store the code word of this RS block
};