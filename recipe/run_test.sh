
cd ../*/Core/tests
# create test files
head -c $((96 + 2*3*4*1000)) /dev/random > reference.npy
head -c $((96 + 2*3*4*1000)) /dev/random > correlate.npy

test1 datacloud_input.txt

# check wrong size
