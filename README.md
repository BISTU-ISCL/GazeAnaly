# GazeAnaly

演示程序：将电脑屏幕中心作为原点，把屏幕分成 AOI1~AOI4 四个区域，使用随机生成的注视点绘制注视圆圈，圆圈面积与停留时间成正比，并实时给出各 AOI 停留时间占比。把随机数据替换成真实眼动仪的坐标即可用于快速验证。

## Prerequisites
- CMake 3.10+
- A C++17 compiler
- OpenCV (with `highgui`, `videoio`, and `imgproc` modules)

## Build
使用附带的 `CMakeLists.txt` 构建（需要 OpenCV）：
```bash
mkdir -p build
cd build
cmake ..
cmake --build .
```

## Run
在 `build` 目录下运行（按 q 或 Esc 退出）：
```bash
./gaze_heatmap
```

提示：将 `generateDemoSamples` 的输出替换为你的眼动仪数据（屏幕像素坐标与对应停留时长），即可获得真实四象限注视统计。
