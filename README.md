# isometric image slicer
A tool to slice isometric images into 1x1 strips (useful in isometric sorting) 

Requires ImageMagick to compile

compile command:
cc -o slice slice.c `pkg-config --cflags --libs MagickWand`

run command:
./slice descriptor_id x_dim y_xim

The name of the input image must be equal to descriptor_id.png

Output images will be in the format of:

(for a 2x2 object)
descriptor_id=0_0.png
descriptor_id=0_1.png
descriptor_id=1_0.png
