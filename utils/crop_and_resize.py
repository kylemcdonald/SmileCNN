import numpy as np
from skimage.measure import block_reduce

def crop_and_resize(img, target_size=32, zoom=1):
    small_side = int(np.min(img.shape) * zoom)
    reduce_factor = small_side / target_size
    crop_size = target_size * reduce_factor
    mid = np.array(img.shape) / 2
    half_crop = crop_size / 2
    center = img[mid[0]-half_crop:mid[0]+half_crop,
    	mid[1]-half_crop:mid[1]+half_crop]
    return block_reduce(center, (reduce_factor, reduce_factor), np.mean)