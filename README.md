# Cpp_Nifty_Mountain_Paths

Feb 2018
Program reads in 844x480 values from a text file that holds the
elevation in meters of each pixel on a topographical map of
Colorado. The map is drawn out, and the best (greedy) paths
starting from each row are drawn out in red. The path with the
smallest total elevation change out of all 480 paths is then
highlighted in green.
   *NOTE* a slightly different algorithm was used than the one
provided; when a path is presented with two or more options
that are equal in elevation difference, it picks a random
direction to pursue, unlike the provided algorithm that would
always go straight instead. This specification makes the
produced greedy path significantly more accurate (between 1000
and 2000 meters less total elevation change on average).
   A second algorithm is then used do determine the path with
the lowest total elevation, which is compared against the first
algorithm (greedy path), and highlighted in yellow.
