import cv2
import numpy as np
import time
import os

# Remove previous keyframes
os.system('rm ./keyframes/*')

# Load video file
cap = cv2.VideoCapture('input_home1.mp4')

# Extract keyframes
start_time = time.time()
keyframes = []
num_keyframes = 200
keytry_cnt = 0
while len(keyframes) < num_keyframes:
    ret, frame = cap.read()
    if not ret:
        break
    # Compute frame difference
    if len(keyframes) > 0:
        diff = cv2.absdiff(frame, keyframes[-1])
        mean_diff = np.mean(diff)
    # Compute focus measure using Laplacian variance
    fm = cv2.Laplacian(frame, cv2.CV_64F).var()
    # Add frame as keyframe if it's visually different enough from previous keyframes and has sufficient focus
    if len(keyframes) == 0 or (mean_diff > 40 and fm > 300):
        keyframes.append(frame)

extraction_time = time.time() - start_time

# Save keyframes as JPEG files
start_time = time.time()
for i, frame in enumerate(keyframes):
    cv2.imwrite("keyframes/keyframe{}.jpg".format(i), frame)
    print("Keyframe {} saved as keyframe{}.jpg".format(i, i))
save_time = time.time() - start_time

# Stitch keyframes
stitcher = cv2.createStitcher() if cv2.__version__.startswith('3') else cv2.Stitcher_create()
status, stitched = stitcher.stitch(keyframes)
stitching_time = time.time() - start_time

# Save result as JPEG file
if status == cv2.STITCHER_OK:
    cv2.imwrite("output.jpg", stitched)
    print("Stitched panorama saved as output.jpg")
else:
    print("Error stitching images:", status)
save_time = time.time() - start_time

# Print execution times
print("Keyframe extraction time:", extraction_time, "seconds")
print("Saving keyframes time:", save_time, "seconds")
print("Stitching time:", stitching_time, "seconds")
print("Saving stitched panorama time:", save_time, "seconds")


### Display result
##if status == cv2.STITCHER_OK:
##    cv2.imshow("Panorama", stitched)
##    cv2.waitKey(0)
##    cv2.destroyAllWindows()
##else:
##    print("Error stitching images:", status)

# Release resources
cap.release()
