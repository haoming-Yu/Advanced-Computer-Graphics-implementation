import colorsys
from PIL import Image
import numpy as np
import qrcode
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

def HalfTone(img:Image, k:int =4, f:bool=False, useGray:bool=False):
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
    qr = qrcode.QRCode(31,box_size=3)
    qr.add_data(data)
    qr.make()
    return qr.make_image()

img = GetQrCode('helloworld!')
img.show()

# rgb_picture = Image.open('./beihaiting.jfif',"r")
# halftone_arr = HalfTone(rgb_picture,4)
# halftone_img = Image.fromarray(halftone_arr,"L")
# halftone_img = halftone_img.resize((423,423))
# halftone_arr = np.asarray(halftone_img)
# halftone_img.save('res.png')
# # resize_pic =  rgb_picture.resize((120,120))
# gray_picture = rgb_picture.convert("L")
# # gray_picture.show()
# arr = np.asarray(gray_picture)
# print(arr.shape)
# rgb_picture_arr = np.asarray(rgb_picture)
# r,g,b = rgb_picture_arr[:,:,0],rgb_picture_arr[:,:,1],rgb_picture_arr[:,:,2]
# picture_shape = r.shape
# hsv_arr = np.empty(tuple([picture_shape[0],picture_shape[1],3]),np.float64)
# for i in range(picture_shape[0]):
#     for j in range(picture_shape[1]):
#         hsv_arr[i][j] = colorsys.rgb_to_hsv(r[i][j],g[i][j],b[i][j])
# rgb_arr = np.empty(tuple([picture_shape[0],picture_shape[1],3]),np.float64)
# for i in range(picture_shape[0]):
#     for j in range(picture_shape[1]):
#         rgb_arr[i][j] = colorsys.hsv_to_rgb(255.,hsv_arr[i][j][1],hsv_arr[i][j][2])
# new_picture = Image.fromarray(rgb_arr.astype(np.int8),'RGB')
# new_picture.show()
