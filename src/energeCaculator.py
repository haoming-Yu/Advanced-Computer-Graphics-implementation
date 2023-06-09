from pre_process import PreProcessor
import qrcode
from PIL import Image
import math
import numpy as np
from data import Pattern_Similarity_Matrix
from data import pattern_reliability
import itertools
import copy
import random

def GetQrCode(data, version: int = 31) -> Image:
    qr = qrcode.QRCode(31, box_size=3)
    qr.add_data(data)
    qr.make()
    return qr.make_image()


def GeneratePatterns():
    Patterns = []
    # 生成所有可能的组合
    all_combinations = list(itertools.product([0, 1], repeat=9))
    print(all_combinations)

    # 按顺序枚举出所有情况
    i = 0
    for combination in all_combinations:
        matrix = [combination[i:i + 3] for i in range(0, 9, 3)]
        color = matrix[1][1]
        reliability = pattern_reliability[i]
        i = i + 1
        Patterns.append([matrix, color, reliability])

    return copy.deepcopy(Patterns)

Patterns = GeneratePatterns()  # 512种pattern

qrcode_img = GetQrCode('helloworld!')
obj_img = Image.open("ftm.png")

preprocessor = PreProcessor(obj_img, qrcode_img)
Modules = preprocessor.GetModules()
# 计算图像在Modules维度上的长和宽
M_height = int(math.sqrt(len(Modules)))
M_width = int(math.sqrt(len(Modules)))


def reliabilityEnerge(f, module_index):
    """
    计算一个module在当前模式分配下的reliabilityEnerge
    f: 一维数组，表示pattern分配结果，f[i]表示第i个module被分配到了第几个pattern
    module_index: 表示一个module的下标
    return: reliability energe
    """

    module = Modules[module_index]
    pattern = Patterns[f[module_index]]

    print("MODULE", module)
    print("type", type(module[1]))
    print("PATTERN", pattern)

    energe = math.exp(-1 * module[2]) * (1 - (pattern[2] if module[1] == pattern[1] else 0))

    return energe


def Diff(I1, I2):
    mse = np.mean((I1 - I2) ** 2)
    similarity = 1 / (1 + mse)
    return 1 - similarity


def regularizationEnergy(f, module_index):
    """
    计算一个module在当前分配下的regularizationEnergy
    f: 一维数组，表示pattern分配结果，f[i]表示第i个module被分配到了第几个pattern
    module_index: 表示一个module的下标
    return: regularization energy，分成unary项和binary项
    """

    module = Modules[module_index]
    # 分别获取上下左右四个module
    neighbors = []
    if module_index >= M_width:
        neighbors.append(module_index - M_width)
    if M_height * M_width - 1 - module_index >= M_width:
        neighbors.append(module_index + M_width)
    if module_index % M_width >= 1:
        neighbors.append(module_index - 1)
    if module_index % M_width < M_width - 1:
        neighbors.append(module_index + 1)

    pattern = Patterns[f[module_index]]

    unary_energe = Diff(np.array(module[0]), np.array(pattern[0]))

    binary_enerage = 0
    for nb_idx in neighbors:
        neighbor = Modules[nb_idx]

        # 这里这么写是因为，为了节省空间，相似度矩阵只生成了一半，比方说，只有 [1,5] 没有 [5,1]
        if f[module_index] < f[nb_idx]:
            tmp = f[module_index]
        else:
            tmp = f[nb_idx]

        spill = (tmp * (tmp + 1)) // 2

        if f[module_index] < f[nb_idx]:
            pattern_similiarity = Pattern_Similarity_Matrix[f[module_index] * 512 + f[nb_idx] - spill]
        else:
            pattern_similiarity = Pattern_Similarity_Matrix[f[module_index] + f[nb_idx] * 512 - spill]

        binary_enerage = binary_enerage + \
                         module[2] * math.exp(-1 * Diff(np.array(module[0]), np.array(neighbor[0]))) * pattern_similiarity

    return unary_energe, binary_enerage


def bindingConstraintEnerge(f, module_index):
    """
    计算一个module在当前分配下的bindingConstraintEnerge
    f: 一维数组，表示pattern分配结果，f[i]表示第i个module被分配到了第几个pattern
    module_index: 表示一个module的下标
    return: bindingConstraintEnerge
    """

    module = Modules[module_index]
    pattern = Patterns[f[module_index]]

    beta = 100
    delta = 1 if module[1] == pattern[1] else 0
    energe = beta * delta

    return energe

# test
min_value = 0  # 最小值
max_value = 511  # 最大值

# 生成随机一维数组，表示pattern的分配
f = [random.randint(min_value, max_value) for _ in range(len(Modules))]

for i in range(len(Modules)):
    print("Module_", i)
    print("rel_enerage", reliabilityEnerge(f, i), ';', "reg_enerage", regularizationEnergy(f, i), ";", "bind_enerage", bindingConstraintEnerge(f, i))

