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

std::vector<QRmodule*> generate_qr(int version, char* input_str, std::unordered_set<int>& necessary, cv::Mat& qr_img);
