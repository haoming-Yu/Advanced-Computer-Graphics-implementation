#include "qrcode/qrcode.h"
#include "pre_process/pre_process.h"
/**
 * James: Table of the capacity of error correction.
 * 
 * This is a method used to expand the functionality 
 * of the lib.
 * 
 */

static const int ecCapTable[QRSPEC_VERSION_MAX+1][4]={
	{0  ,0  ,0  ,0 },
	{2  ,4  ,6  ,8 },	//1
	{4  ,8  ,11 ,14},
	{7  ,13 ,9  ,11},
	{10 ,9  ,13 ,8 },
	{13 ,12 ,9  ,11},	//5
	{9  ,8  ,12 ,14},
	{10 ,9  ,9  ,13},
	{12 ,11 ,11 ,13},
	{15 ,11 ,10 ,12},
	{9  ,13 ,12 ,14},	//10
	{10 ,15 ,14 ,12},
	{12 ,11 ,13 ,14},
	{13 ,11 ,12 ,11},
	{15 ,12 ,10 ,12},
	{11 ,12 ,15 ,12},	//15
	{12 ,14 ,12 ,15},
	{14 ,14 ,14 ,14},
	{15 ,13 ,14 ,14},
	{14 ,13 ,13 ,13},
	{14 ,13 ,15 ,14},	//20
	{14 ,13 ,14 ,15},
	{14 ,14 ,15 ,12},
	{15 ,14 ,15 ,15},
	{15 ,14 ,15 ,15},
	{13 ,14 ,15 ,15},	//25
	{14 ,14 ,14 ,15},
	{15 ,14 ,15 ,15},
	{15 ,14 ,15 ,15},
	{15 ,14 ,15 ,15},
	{15 ,14 ,15 ,15},	//30
	{15 ,14 ,15 ,15},
	{15 ,14 ,15 ,15},  
	{15 ,14 ,15 ,15},
	{15 ,14 ,15 ,15},
	{15 ,14 ,15 ,15},	//35
	{15 ,14 ,15 ,15},
	{15 ,14 ,15 ,15},
	{15 ,14 ,15 ,15},
	{15 ,14 ,15 ,15},
	{15 ,14 ,15 ,15},	//40
};

/*
 * @return: all modules with full information, including types
 * 
 */
std::vector<QRmodule*> generate_qr(int version, char* input_str, std::unordered_set<int>& necessary, cv::Mat& qr_img)
{
    // note that the input_str must be ended with a '\0' character.
    QRcode *qrcodeB, *qrcodeC, *qrcodeT;
    // qrcodeB as QR code, qrcodeC as RS block map, qrcodeT as code word map
    // they are generated using the same string.
    int QRSize = version*4 + 17;
    QRencodeMode qrMode = QR_MODE_8;

    // the error correction level is not passed as a parameter for now
    // now it is set to be high level to ensure reliability.
    // later version will change this.
    QRecLevel QRlv = QR_ECLEVEL_H;
    
    // create the symbol object from the same string
    // here the error handling is omitted for agile development
    qrcodeB = QRcode_encodeString(input_str, version, QRlv, qrMode, 1);
    qrcodeC = QRcode_encodeString(input_str, version, QRlv, qrMode, 1);
    qrcodeT = QRcode_encodeString(input_str, version, QRlv, qrMode, 1);
    std::cout << "OK1" << std::endl;
    // compute the QR code data
    // update the error correction information
    int qr_spec[6];
    int qr_spec_arg[5];
    QRspec_getEccSpec(version, QRlv, qr_spec_arg);
    qr_spec[0] = qr_spec_arg[0];
    qr_spec[1] = qr_spec_arg[1];
    qr_spec[2] = qr_spec_arg[2];
    qr_spec[3] = qr_spec_arg[3];
    qr_spec[4] = qr_spec_arg[4];
    qr_spec[5] = ecCapTable[version][QRlv];
    int ecc_value = QRspec_rsEcCapacity(qr_spec);
    
    int rs_num = QRspec_rsBlockNum(qr_spec); 
    // this variable represent the total number of the RS block
    int rs_block_num[2] = {QRspec_rsBlockNum1(qr_spec), QRspec_rsBlockNum2(qr_spec)};
    // this variable represent the block number of the RS block seperately (seperated into 2 blocks)
    int rs_block_size[2] = {QRspec_rsCodeWords1(qr_spec), QRspec_rsCodeWords2(qr_spec)};
    // this variable represent the block size of the RS block seperately (seperated into 2 blocks)

    std::vector<QRmodule*> modules; // used to store all the modules, might be useful for further process
    // here I just use this as a variable for debug.

    cv::Mat qrcode; // this variable stores the pixel value of QR code
    
    cv::Mat qrcRS;  // this variable stores the RSMap information of the QR code
    // Note that this variable only stores the information of the data codeword type
    // If it is not a data codeword type, then just store as -1 in the corresponding place for the module

    cv::Mat qrcCW;  // this variable stores the Code Word Map information of the QR code
    // the construction of codeword mapping is exactly the same as R-S mapping
    qrcode.create(QRSize, QRSize, CV_8UC1); // 8 bit unsigned integer, 1 channel
    qrcRS.create(QRSize, QRSize, CV_32SC1); // 32 bit signed integer, 1 channel
    qrcCW.create(QRSize, QRSize, CV_32SC1); // 32 bit signed integer, 1 channel
    // process the information of symbol objects acquired from the lib qrencode
    std::cout << "OK2" << std::endl;
    for (int row = 0; row < QRSize; ++row) {
        int row_step = row*QRSize;
        unsigned char* qrcPTR = qrcode.ptr<unsigned char>(row);
        int* RSPTR = qrcRS.ptr<int>(row);
        int* CWPTR = qrcCW.ptr<int>(row);
        for (int col = 0; col < QRSize; ++col) {
            int idx = row_step + col;
            unsigned char pixel_val = (qrcodeB->data[idx] & 1) ? 0 : 255;
            // in the encoding system of the library, the white data is encoded as 0 
            // Note that only the last bit indicate the white/black trait
            qrcPTR[col] = pixel_val;
            // the above step record the white/black into the qrcode matrix, to enable image process.

            /// Central part -> divide the modules into different types
            int mType = -1;
            int rsval = (int)qrcodeC->data[idx]; // get current module's information from qr encoder
            if (rsval > 0 && rsval < 82) {
                // Data codeword type
                mType = 0;
            } else if (rsval == 82) {
                // remaining bits
                mType = 1;
            } else if (rsval == 136 || rsval == 137) {
                // indicate the version information of the qrcode
                mType = 2;
            } else if (rsval == 132 || rsval == 129) {
                // indicate the format information of the qrcode
                mType = 3;
            } else if (rsval == 160 || rsval == 161) {
                // indicate current module belongs to alignment patterns
                mType = 4;
            } else if (rsval == 144 || rsval == 145) {
                // timing patterns
                mType = 5;
            } else if (rsval == 192 || rsval == 193) {
                // Separators patterns & position detection patterns
                mType = 6;
            } else mType = 7;

            // record all data codeword type's rs value, construct the RS map
            RSPTR[col] = (mType == 0) ? rsval : -1;

            // get the codeword value from qr encoder's information
            int cwval = (int)qrcodeT->data[idx];
            CWPTR[col] = (mType == 0) ? cwval : -1;

            // create module object for further process
            QRmodule* qr_module = new QRmodule();
            qr_module->type = mType;
            qr_module->pos = cv::Point(col, row); 
            // the x(horizontal) axis should be filled with column
            // the y(vertical) axis should be filled with row
            qr_module->QRC = (pixel_val) ? 0 : 1;
            qr_module->RSID = rsval; // RS block index (start from 1, 1-based index)
            qr_module->CWID = cwval; // code word index (start from 1, 1-based index)
            modules.push_back(qr_module);

            if (mType == 0 || mType == 1 || mType == 2 || mType == 3 || mType == 4 || mType == 5 || mType == 7) {
                // do something to create the RS block.
                // omitted here for now.
            } else {
                necessary.insert(idx);
            }
        }
    }

    cv::Size changed_size = qrcode.size();
    changed_size.width *= 3;
    changed_size.height *= 3;
    cv::resize(qrcode, qr_img, changed_size, 0, 0, cv::INTER_NEAREST);

    return modules;
}