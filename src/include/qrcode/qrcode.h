#include "../../../qrcode/qrencode.h"
#include "../../../qrcode/qrspec.h"
// include the qrencode lib's header file

/*
 * The following defines are used 
 * as utility expansion to the original lib
 * 
 */
#define QRspec_rsCodeWords1(__spec__) (QRspec_rsDataCodes1(__spec__) + QRspec_rsEccCodes1(__spec__))
#define QRspec_rsCodeWords2(__spec__) (QRspec_rsDataCodes2(__spec__) + QRspec_rsEccCodes2(__spec__))
#define QRspec_rsEcCapacity(__spec__) (__spec__[5])

void generate_qr(int version, char* input_str);
