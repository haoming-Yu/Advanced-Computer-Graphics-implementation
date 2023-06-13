from pre_process import GetQrCode
import copy
import itertools
from PIL import Image
from PIL import ImageEnhance
from PIL import ImageOps
from PIL import ImageChops
from PIL import ImageTransform
import random


def GeneratePatterns():
    Patterns = []
    # 生成所有可能的组合
    all_combinations = list(itertools.product([0, 1], repeat=9))

    # 按顺序枚举出所有情况
    i = 0
    for combination in all_combinations:
        combination = combination[::-1]
        matrix = [combination[i:i + 3] for i in range(0, 9, 3)]
        Patterns.append(matrix)
        i = i + 1

    return copy.deepcopy(Patterns)


Patterns = GeneratePatterns()  # 512种pattern

qr_num = 100
qr_size = 423  # 生成二维码size = 423 * 423
offsets = [(-1, -1), (0, -1), (1, -1), (-1, 0), (1, 0), (-1, 1), (0, 1), (1, 1)]
# 产生一定数量的二维码图片用于评估pattern reliability
for i in range(qr_num):
    index = str(i)
    # qr = GetQrCode("pat_relia_eva_" + index)
    # qr.save("../qrcode/initial/pat_relia_eva_" + index + ".png", format="PNG", quality=100)

    qr = Image.open("../qrcode/initial/pat_relia_eva_" + index + ".png")
    qr_copy = qr.copy()
    pixels = qr_copy.load()
    for h in range(1, 422, 27):
        for w in range(1, 422, 27):
            while True:
                random_pattern_id = random.randint(0, 511)
                pattern = Patterns[random_pattern_id]
                if pixels[h, w] == 255 * pattern[1][1]:
                    # 如果随机到的pattern中心颜色和二维码当前中心颜色相同，就替换
                    for offset in offsets:
                        dx, dy = offset
                        x = h + dx
                        y = w + dy
                        pixels[x, y] = 255 * pattern[dx][dy]
                    break

    qr_copy.save("../qrcode/random_replace/pat_relia_eva_" + index + ".png", format="PNG", quality=100)

    # 生成随机的偏航角、俯仰角、平移和缩放因子
    yaw = random.uniform(-0.3, 0.3)  # 偏航角的随机偏移范围（-3 到 3 度）
    pitch = random.uniform(-0.3, 0.3)  # 俯仰角的随机偏移范围（-3 到 3 度）
    translation_x = random.uniform(-1, 1)  # x轴方向的随机平移范围（-1 到 1 像素）
    translation_y = random.uniform(-1, 1)  # y轴方向的随机平移范围（-1 到 1 像素）
    scaling_factor = random.uniform(1, 30)  # 随机缩放因子范围（1 到 30）

    # 应用偏航角和俯仰角的扰动
    perturbed_qr = qr_copy.rotate(yaw, resample=Image.BICUBIC, expand=True)
    perturbed_qr = qr_copy.transform(perturbed_qr.size, Image.AFFINE, (1, pitch, 0, 0, 1, 0),
                                     resample=Image.BICUBIC)

    # 应用平移扰动
    perturbed_qr = perturbed_qr.transform(perturbed_qr.size, Image.AFFINE,
                                          (1, 0, translation_x, 0, 1, translation_y),
                                          resample=Image.BICUBIC)

    # 应用缩放
    # perturbed_qr = perturbed_qr.resize(
    #     (int(perturbed_qr.width * scaling_factor), int(perturbed_qr.height * scaling_factor)),
    #     resample=Image.BICUBIC)

    perturbed_qr.save("../qrcode/random_perturbation/pat_relia_eva_" + index + ".png", format="PNG", quality=100)