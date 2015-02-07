Anisotropic total variation based image restoration using graph cuts
====================================================================

This is the code produced during my project («fordypningsemne») and master thesis at the programme for Industrial Mathematics at NTNU.

In my project I implemented a total variation image restoration algorithm. It was formulated as a continuous minimization problem, which was discretized and then minimized using a graph cut algorithm, more specifically the push-relabel algorithm. The code can be found at the following tag:

https://github.com/burk/image-restoration/releases/tag/v1.0.0

In my thesis, the total variation was extended by introducing an anisotropy tensor. This gives a finer control over how the variation is measured. We use it to be more conserving of known edges in the image.

In addition, the maximum flow algorithm of Boykov and Kolmogorov was implemented. It is taylored for the kinds of graphs appearing in imaging applications.

Compiling and running
---------------------

### Dependencies

* [OpenCV](http://opencv.org/) library for image file handling.
* `cmake` build system.

### Compiling

The flow algorithm to use has to be specified at compile time. For the Boykov-Kolmogorov algorithm, run

    cmake -DCMAKE_BUILD_TYPE=Release -DBK=ON
    
and for the push-relabel algorithm set `-DBK=OFF`. The build type can be set to `Debug` for more debug information. Then build with

    make
    
### Running

To restore an image named `noisy.pgm`, run the following

    ./image-restoration -p 2 -n 16 -r 8 -s 3 -b 20000 -g 100 noisy.pgm blur.pgm edge.pgm structure.pgm color.png out.pgm
    
where the parameters are

* `-p`: the exponent in the fidelity term
* `-n`: the size of the neighborhood stencil
* `-s`: the noise scale
* `-r`: the integration scale
* `-b`: beta, the amount of restoration applied
* `-g`: omega, the amount of anisotropy

and the output

* `blur.pgm`: input blurred by noise scale
* `edge.pgm`: edge detector
* `structure.pgm`: vector tensor visualization
* `color.png`: colorized tensor visualization
* `out.pgm`: restored image


### GUI

A very simple GUI can be found in the `fiddle/` folder. It makes it a bit easier to play with the tensor parameters. It requires a QT of version 4 or higher and is compiled by running `qmake .` followed by `make`.
