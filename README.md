# GazeAnaly

A minimal C++ demo that overlays an eye-gaze heatmap on top of a live camera feed or video file using OpenCV. Gaze points are randomly generated in the demo loop—replace them with coordinates from your eye tracker to visualize real attention hotspots.

## Prerequisites
- CMake 3.10+
- A C++17 compiler
- OpenCV (with `highgui`, `videoio`, and `imgproc` modules)

## Build
```bash
mkdir -p build
cd build
cmake ..
cmake --build .
```

## Run
From the `build` directory:
```bash
# Use the default camera
./gaze_heatmap

# Or pass a video file
./gaze_heatmap /path/to/video.mp4
```

Press `q` or `Esc` to quit. The heatmap deepens in color where the (demo) gaze points linger longer. To integrate a real eye tracker, feed its gaze coordinates (in pixel units) into the `gazePoints` vector in `src/main.cpp`.
