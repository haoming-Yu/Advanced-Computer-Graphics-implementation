#include "qrcode/qrcode.h"

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

void generate_qr(int version, char* input_str)
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

    // compute the QR code data
    // update the error correction information
    int qr_spec[6];
    QRspec_getEccSpec(version, QRlv, qr_spec);
    qr_spec[5] = ecCapTable[version][QRlv];
    int ecc_value = QRspec_rsEcCapacity(qr_spec);
    
    int rs_num = QRspec_rsBlockNum(qr_spec); 
    // this variable represent the total number of the RS block
    int rs_block_num[2] = {QRspec_rsBlockNum1(qr_spec), QRspec_rsBlockNum2(qr_spec)};
    // this variable represent the block number of the RS block seperately (seperated into 2 blocks)
    int rs_block_size[2] = {QRspec_rsCodeWords1(qr_spec), QRspec_rsCodeWords2(qr_spec)};
    // this variable represent the block size of the RS block seperately (seperated into 2 blocks)

    // process the information of symbol objects acquired from the lib qrencode
    for (int row = 0; row < QRSize; ++row) {
        int row_step = row*QRSize;
        for (int col = 0; col < QRSize; ++col) {
            int idx = row_step + col;
            unsigned char pixel_val = (qrcodeB->data[idx] & 1) ? 0 : 255;
            // in the encoding system of the library, the white data is encoded as 0
        }
    }
}