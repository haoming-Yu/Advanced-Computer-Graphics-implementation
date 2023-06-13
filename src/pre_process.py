import colorsys
from PIL import Image
from adaptive_filter import GetImageBorderArray
import numpy as np
import qrcode
import cv2

def GetBayerMat(k:int)->tuple([bool,np.array]):
    '''
    函数作用：获取bayer矩阵
    return：是否生成成功，成功的话同时返回numpy类型的矩阵
    k: 是bayer矩阵的阶数，取值一般为1 2 4 8 16
    '''
    if k & (k-1) != 0:
        return False, None
    if k == 1:
        return True, [[0.5]]
    m = [[0, 2], [3, 1]]
    m = np.array(m)
    while(m.shape[0] != k):
        m1 = np.zeros((m.shape[0]*2, m.shape[1]*2))
        m1[:m.shape[0], :m.shape[1]] += 4*m
        m1[:m.shape[0], m.shape[1]:] += 4*m+2
        m1[m.shape[0]:, :m.shape[1]] += 4*m+3
        m1[m.shape[0]:, m.shape[1]:] += 4*m+1
        m = m1
    return True, m

def HalfTone(img:Image, k:int =4, f:bool=False, useGray:bool=False)->np.array:
    '''
    k：bayer矩阵大小
    f：由于转换后图像尺寸会变大k倍，f表示是否先缩小k倍
    useGray：传入的img是否为灰度图
    '''
    if not useGray:
        img = img.convert("L");
    if f:
        shape = img.size
        img = img.resize((shape[0]//k,shape[1]//k))
    ret, bayers = GetBayerMat(k)
    if not ret:
        print(f"矩阵阶数k={k}非2的倍数")
        return
    bayers *= (256//(k*k))
    img = np.asarray(img)
    h, w = img.shape
    newImg = np.zeros((k*h, k*w), dtype='uint8')
    # 遍历图像每个像素点
    for i in range(0, h, 1):
        for j in range(0, w, 1):
            # 对于每个像素点，遍历bayer矩阵，判断是否该把矩阵中某一位置设置为纯白(255)或纯黑(0)
            for p in range(k): 
                for q in range(k):
                    if img[i][j] > bayers[p][q]:
                        newImg[k*(i)+p][k*(j)+q] = 255
                    else:
                        newImg[k*(i)+p][k*(j)+q] = 0
    return newImg

def GetQrCode(data,version:int = 31)->Image:
    qr = qrcode.QRCode(version,box_size=3,border=0,error_correction=qrcode.constants.ERROR_CORRECT_H)
    qr.add_data(data)
    qr.make()
    return qr.make_image()

class PreProcessor():
    '''
    QrImg: 二维码Image对象,要求box_size = 3
    Picture: 图片Image对象
    data: 当QrImg为空时用于生成二维码Image对象
    '''
    def __init__(self,Picture:Image,QrImg:Image = None,data = None) -> None:
        if QrImg is None and data is None:
            print('error parameter!')
        if QrImg is None:
            QrImg = GetQrCode(data)
        self.qr_arr = np.asarray(QrImg).astype(np.uint8)
        self.shape = self.qr_arr.shape
        
        # get halftone img
        Picture = Picture.resize((self.shape[0]//4,self.shape[1]//4))
        HalftoneArr = HalfTone(Picture,k=4) // 255
        padding_count = self.shape[0] % 4
        HalftoneArr = cv2.copyMakeBorder(HalftoneArr,padding_count//2,padding_count-padding_count//2,padding_count//2,padding_count-padding_count//2,borderType=cv2.BORDER_REFLECT)
        self.pic_arr = np.asarray(HalftoneArr) 
        # self.pic_arr = HalftoneArr
        
        # get boder arr
        self.ImpMapArr = GetImageBorderArray(np.asarray(Picture.resize(self.shape))).astype(np.uint8)
        self.ImpMapArr //= 255
    def GetModules(self)->list:
        result = []
        for i in range(0,self.shape[0],3):
            for j in range(0,self.shape[1],3):
                temp = []
                temp.append(self.pic_arr[i:i+3,j:j+3])
                temp.append(self.qr_arr[i+1][j+1])
                sum = 0
                for p in range(3):
                    for q in range(3):
                        sum += self.ImpMapArr[i+p][j+q]
                temp.append(sum)
                result.append(temp.copy())
        return result
    def GetBorderImg(self)->Image:
        res = Image.fromarray(self.ImpMapArr * 255,"L")
        # cv2.imshow('border',self.ImpMapArr)
        # cv2.waitKey(0)
        return res 
    def GetHalfToneImg(self)->Image:
        res = Image.fromarray(self.pic_arr * 255,"L")
        return res 
    def GetQrImg(self)->Image:
        res = Image.fromarray(self.qr_arr*255,"L")
        return res

qr = GetQrCode("Blow out to sea", 5)
qr.save("../img/zyx/qr.png", format="PNG", quality=100)
obj_img = Image.open("../img/zyx/zyx.jpg")
preprocessor = PreProcessor(obj_img, qr)
halftone = preprocessor.GetHalfToneImg()
halftone.save("../img/zyx/halftone.png", format="PNG", quality=100)
border = preprocessor.GetBorderImg()
border.save("../img/zyx/border.png", format="PNG", quality=100)