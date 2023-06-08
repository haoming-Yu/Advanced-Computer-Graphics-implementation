import cv2
import numpy as np
import math
from PIL import Image
#teste
tensor=np.dtype((np.float32,(2,2)))
#bilateralvariables
sst=2.0
gaussian_size=math.ceil(sst*2)*2+1
sigma_d=3.0
sigma_r=4.25
sig_e=1.0
sig_r=1.6
sig_m=3.0
tau=0.99
PI=math.pi


def GetGaussianWeights(weights,
	neighbor,
	sigma):
	if(weights is None)or(neighbor<0):
		return
	term1=1.0/(math.sqrt(2.0*PI)*sigma)
	term2=-1.0/(2*math.pow(sigma,2))
	weights[neighbor]=term1
	sum=weights[neighbor]
	for i in range(1,neighbor+1):
		weights[neighbor+i]=math.exp(math.pow(i,2)*term2)*term1
		weights[neighbor-i]=weights[neighbor+i]
		sum+=weights[neighbor+i]+weights[neighbor-i]
	#Normalization
	for j in range(0,neighbor*2+1):
		weights[j]/=sum

#Prepare1-ddifferenceofgaussiantemplate.


def GetDiffGaussianWeights(weights,
	neighbor,
	sigma_e,
	sigma_r,
	tau):
	if(weights is None)or(neighbor<0):
		return
	gaussian_e=np.zeros(neighbor*2+1)
	gaussian_r=np.zeros(neighbor*2+1)
	GetGaussianWeights(gaussian_e,neighbor,sigma_e)
	GetGaussianWeights(gaussian_r,neighbor,sigma_r)
	sum=0.0
	for i in range(0,neighbor*2+1):
		weights[i]=gaussian_e[i]-tau*gaussian_r[i]
		sum+=weights[i]
	#Normalization
	for j in range(0,neighbor*2+1):
		weights[j]/=sum


def color_quantize(src,s):
	step=1.0/s
	shades=[step*(i+0.5) for i in range(s)]

	m=np.zeros(src.shape,dtype=np.float32)
	for r in range(src.shape[0]):
		for c in range(src.shape[1]):
			n=math.floor(src[r][c]/step)

			if n==s:#evitosairdolimitedetons
				n-=1

			m[r][c]=shades[n]
	return m

def GetImageBorderArray(img_arr:np.array)->np.array:
    
    im = img_arr
    if im is None:
        print("image not exist")
        exit(-1)
    rows=im.shape[0]
    cols=im.shape[1]
    gray=cv2.cvtColor(im,cv2.COLOR_BGR2GRAY)
    smooth=cv2.bilateralFilter(gray,6,150,150)
    dx=cv2.Sobel(smooth,cv2.CV_32F,1,0)
    dy=cv2.Sobel(smooth,cv2.CV_32F,0,1)
    jacob=np.zeros((rows,cols,3),dtype=np.float32)
    for i in range(rows):
        for j in range(cols):
            gx=dx[i][j]
            gy=dy[i][j]

            jacob[i][j][0]=gx*gx
            jacob[i][j][1]=gy*gy
            jacob[i][j][2]=gx*gy

    jacob=cv2.GaussianBlur(jacob,(gaussian_size,gaussian_size),sst)

	#ETF
    ETF=np.zeros((rows,cols,3),dtype=np.float32)
    E,G,F,lambda_,v2x,v2y,v2=0.0,0.0,0.0,0.0,0.0,0.0,0.0

    for i in range(rows):
        for j in range(cols):
            E=jacob[i][j][0]
            G=jacob[i][j][1]
            F=jacob[i][j][2]
            lambda_=0.5*(E+G+math.sqrt((G-E)*(G-E)+4*F*F))
            v2x=E-lambda_
            v2y=F
            v2=math.sqrt(v2x*v2x+v2y*v2y)
            if v2 == 0:
                ETF[i][j][0]=0
                ETF[i][j][1]=0
            else:
                ETF[i][j][0]=v2x/v2
                ETF[i][j][1]=v2y/v2
            ETF[i][j][2]=math.sqrt(math.fabs(E+G-lambda_))
    #BILATERALFILTER+EDGEEXTRACTION
    FDOG=np.zeros((rows,cols),dtype=np.float32)
    #FDOG=0;
    f0=np.ones((rows,cols),dtype=np.float32)
    f1=np.ones((rows,cols),dtype=np.float32)
    u1=np.zeros(im.shape[:2],dtype=np.uint8)
    near=int(math.ceil(2*sig_r))
    sin,cos=0.0,0.0
    gauss_w=np.zeros(near*2+1,dtype=np.float32)
    sample1,sample2=None,None
    sum_diff,sum_dev,sum_1=0.0,0.0,0.0
    GetDiffGaussianWeights(gauss_w,near,sig_e,sig_r,tau)
    near2=math.ceil(2*sig_m)
    gauss_w2=np.zeros(near2*2+1,dtype=np.float32)
    GetGaussianWeights(gauss_w2,near2,sig_m)
    gr_scale=smooth.copy()


	#gradient
    for i in range(near,rows-near):
        for j in range(near,cols-near):
            cos=ETF[i][j][1]
            sin=-1*ETF[i][j][0]
            sample1=np.zeros(near*2+1,dtype=np.float32)
            sample1[near]=float(gr_scale[i][j])
            for k in range(1,near+1):
                r=round(sin*k)
                c=round(cos*k)
                sample1[near+k]=float(gr_scale[i+r][j+c])
                sample1[near-k]=float(gr_scale[i-r][j-c])
                
            sum_diff=0
            sum_dev=0
            for k in range(2*near+1):
                sum_diff+=sample1[k]*gauss_w[k]
                f0[i][j]=sum_diff

	#tangent
    for i in range(near2,rows-near2):
        for j in range(near2,cols-near2):
            cos=ETF[i][j][0]
            sin=ETF[i][j][1]
            sample2=np.zeros(near2*2+1,dtype=np.float32)
            sample2[near2]=f0[i][j]
            for k in range(1,near2+1):
                r=round(sin*k)
                c=round(cos*k)
                sample2[near2+k]=f0[i+r][j+c]
                sample2[near2-k]=f0[i-r][j-c]
                
            sum_1=0
            for k in range(0,near2*2+1):
                sum_1+=sample2[k]*gauss_w2[k]
            f1[i][j]=sum_1
            if f1[i][j]>0:
                FDOG[i][j]=255
            else:
                FDOG[i][j]=0
    return FDOG

if __name__ == "__main__":
    # # input=input("Insiracaminhodoarquivo\n")
    # input = '../image/beihaiting.jfif'
    # im=cv2.imread(input,cv2.IMREAD_COLOR)
    # if im is None:
    #     print("imagemnaopodesercarregada")
    #     exit(-1)
    # rows=im.shape[0]
    # cols=im.shape[1]
    # gray=cv2.cvtColor(im,cv2.COLOR_BGR2GRAY)
    # smooth=cv2.bilateralFilter(gray,6,150,150)
    # dx=cv2.Sobel(smooth,cv2.CV_32F,1,0)
    # dy=cv2.Sobel(smooth,cv2.CV_32F,0,1)
    # jacob=np.zeros((rows,cols,3),dtype=np.float32)
    # for i in range(rows):
    #     for j in range(cols):
    #         gx=dx[i][j]
    #         gy=dy[i][j]

    #         jacob[i][j][0]=gx*gx
    #         jacob[i][j][1]=gy*gy
    #         jacob[i][j][2]=gx*gy

    # jacob=cv2.GaussianBlur(jacob,(gaussian_size,gaussian_size),sst)

	# #ETF
    # ETF=np.zeros((rows,cols,3),dtype=np.float32)
    # E,G,F,lambda_,v2x,v2y,v2=0.0,0.0,0.0,0.0,0.0,0.0,0.0

    # for i in range(rows):
    #     for j in range(cols):
    #         E=jacob[i][j][0]
    #         G=jacob[i][j][1]
    #         F=jacob[i][j][2]
    #         lambda_=0.5*(E+G+math.sqrt((G-E)*(G-E)+4*F*F))
    #         v2x=E-lambda_
    #         v2y=F
    #         v2=math.sqrt(v2x*v2x+v2y*v2y)
    #         if v2 == 0:
    #             ETF[i][j][0]=0
    #             ETF[i][j][1]=0
    #         else:
    #             ETF[i][j][0]=v2x/v2
    #             ETF[i][j][1]=v2y/v2
    #         ETF[i][j][2]=math.sqrt(math.fabs(E+G-lambda_))
        
    # #BILATERALFILTER+EDGEEXTRACTION
    # FDOG=np.zeros((rows,cols),dtype=np.float32)
    # #FDOG=0;
    # f0=np.ones((rows,cols),dtype=np.float32)
    # f1=np.ones((rows,cols),dtype=np.float32)
    # u1=np.zeros(im.shape[:2],dtype=np.uint8)
    # near=int(math.ceil(2*sig_r))
    # sin,cos=0.0,0.0
    # gauss_w=np.zeros(near*2+1,dtype=np.float32)
    # sample1,sample2=None,None
    # sum_diff,sum_dev,sum_1=0.0,0.0,0.0
    # GetDiffGaussianWeights(gauss_w,near,sig_e,sig_r,tau)
    # near2=math.ceil(2*sig_m)
    # gauss_w2=np.zeros(near2*2+1,dtype=np.float32)
    # GetGaussianWeights(gauss_w2,near2,sig_m)
    # gr_scale=smooth.copy()


	# #gradient
    # for i in range(near,rows-near):
    #     for j in range(near,cols-near):
    #         cos=ETF[i][j][1]
    #         sin=-1*ETF[i][j][0]
    #         sample1=np.zeros(near*2+1,dtype=np.float32)
    #         sample1[near]=float(gr_scale[i][j])
    #         for k in range(1,near+1):
    #             r=round(sin*k)
    #             c=round(cos*k)
    #             sample1[near+k]=float(gr_scale[i+r][j+c])
    #             sample1[near-k]=float(gr_scale[i-r][j-c])
                
    #         sum_diff=0
    #         sum_dev=0
    #         for k in range(2*near+1):
    #             sum_diff+=sample1[k]*gauss_w[k]
    #             f0[i][j]=sum_diff

	# #tangent
    # for i in range(near2,rows-near2):
    #     for j in range(near2,cols-near2):
    #         cos=ETF[i][j][0]
    #         sin=ETF[i][j][1]
    #         sample2=np.zeros(near2*2+1,dtype=np.float32)
    #         sample2[near2]=f0[i][j]
    #         for k in range(1,near2+1):
    #             r=round(sin*k)
    #             c=round(cos*k)
    #             sample2[near2+k]=f0[i+r][j+c]
    #             sample2[near2-k]=f0[i-r][j-c]
                
    #         sum_1=0
    #         for k in range(0,near2*2+1):
    #             sum_1+=sample2[k]*gauss_w2[k]
    #         f1[i][j]=sum_1
    #         if f1[i][j]>0:
    #             FDOG[i][j]=255
    #         else:
    #             FDOG[i][j]=0
                
	#QUANTIZATION
    # lum=None
    # lab_channels=[]
    # lab=cv2.cvtColor(im,cv2.COLOR_BGR2Lab)	
    # lab1=cv2.bilateralFilter(lab,6,150,150)
    # lab2=cv2.bilateralFilter(lab1,6,150,150)
    # lab3=cv2.bilateralFilter(lab2,6,150,150)
    # lab_channels=cv2.split(lab3)
    # lab_channels = list(lab_channels)
    # lum=lab_channels[0].astype(np.float32)/255.0
    # qt=color_quantize(lum,8)
    # qt=(qt*255).astype(np.uint8)
    # lab_channels[0]=qt
    # qtzd=None
    # qtzd = cv2.merge(lab_channels)
    # qtzd=cv2.cvtColor(qtzd,cv2.COLOR_Lab2BGR)
    # FDOG=FDOG.astype(np.uint8)
    # FDOG=cv2.cvtColor(FDOG,cv2.COLOR_GRAY2BGR)
    # res=None
    # res = cv2.bitwise_and(qtzd,FDOG)
    # #SHOW
    # cv2.imshow("output",FDOG)
    # cv2.waitKey(0)
    # exit(0)
    # im = cv2.imread('../image/beihaiting.jfif')
    im = Image.open("../image/beihaiting.jfif")
    border_arr = GetImageBorderArray(np.asarray(im))
    cv2.imshow('output',border_arr)
    cv2.waitKey(0)
    exit(0)
