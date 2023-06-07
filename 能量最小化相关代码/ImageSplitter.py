import numpy as np
import cv2
# 关于cv2无法自动补全的问题 https://blog.csdn.net/qq_30150579/article/details/128108678
import matplotlib.pyplot as plt

# Load the image
img = cv2.imread('ftm.png', cv2.IMREAD_GRAYSCALE).astype(float)
plt.imshow(img)
plt.pause(1)

cluster_num = 4  # 设置分类数
maxiter = 60  # 最大迭代次数

# 随机初始化标签
label = np.random.randint(1, cluster_num+1, size=img.shape)
plt.imshow(label)
plt.pause(1)

for iter in range(maxiter):
    # 计算先验概率
    label_u = cv2.filter2D(label.astype(float), -1, np.array([[0, 1, 0], [0, 0, 0], [0, 0, 0]]), borderType=cv2.BORDER_REPLICATE)
    label_d = cv2.filter2D(label.astype(float), -1, np.array([[0, 0, 0], [0, 0, 0], [0, 1, 0]]), borderType=cv2.BORDER_REPLICATE)
    label_l = cv2.filter2D(label.astype(float), -1, np.array([[0, 0, 0], [1, 0, 0], [0, 0, 0]]), borderType=cv2.BORDER_REPLICATE)
    label_r = cv2.filter2D(label.astype(float), -1, np.array([[0, 0, 0], [0, 0, 1], [0, 0, 0]]), borderType=cv2.BORDER_REPLICATE)
    label_ul = cv2.filter2D(label.astype(float), -1, np.array([[1, 0, 0], [0, 0, 0], [0, 0, 0]]), borderType=cv2.BORDER_REPLICATE)
    label_ur = cv2.filter2D(label.astype(float), -1, np.array([[0, 0, 1], [0, 0, 0], [0, 0, 0]]), borderType=cv2.BORDER_REPLICATE)
    label_dl = cv2.filter2D(label.astype(float), -1, np.array([[0, 0, 0], [0, 0, 0], [1, 0, 0]]), borderType=cv2.BORDER_REPLICATE)
    label_dr = cv2.filter2D(label.astype(float), -1, np.array([[0, 0, 0], [0, 0, 0], [0, 0, 1]]), borderType=cv2.BORDER_REPLICATE)
    p_c = np.zeros((cluster_num, label.size))

    for i in range(cluster_num):
        label_i = np.ones(label.shape) * i
        temp = np.logical_not(label_i - label_u) + np.logical_not(label_i - label_d) + \
               np.logical_not(label_i - label_l) + np.logical_not(label_i - label_r) + \
               np.logical_not(label_i - label_ul) + np.logical_not(label_i - label_ur) + \
               np.logical_not(label_i - label_dl) + np.logical_not(label_i - label_dr)
        p_c[i, :] = temp.flatten() / 8

    p_c[p_c == 0] = 0.001  # 防止出现0

    # 计算似然函数
    mu = np.zeros(cluster_num)
    sigma = np.zeros(cluster_num)

    for i in range(cluster_num):
        index = np.where(label == (i + 1))  # 找到每一类的点
        data_c = img[index]
        mu[i] = np.mean(data_c)  # 均值
        sigma[i] = np.var(data_c)  # 方差

    p_sc = np.zeros((cluster_num, label.size))

    for j in range(cluster_num):
        MU = np.tile(mu[j], img.size)
        p_sc[j, :] = 1 / np.sqrt(2 * np.pi * sigma[j]) * np.exp(-(img.flatten() - MU) ** 2 / (2 * sigma[j]))

    label = np.argmax(np.log(p_c) + np.log(p_sc), axis=0) + 1  # 找到联合一起的最大概率最为标签
    label = label.reshape(img.shape)

    # 显示结果
    if iter % 6 == 0:
        plt.figure()
        n = 1

    plt.subplot(2, 3, n)
    plt.imshow(label, cmap='gray')
    plt.title('iter = {}'.format(iter))
    plt.pause(0.1)
    n += 1

