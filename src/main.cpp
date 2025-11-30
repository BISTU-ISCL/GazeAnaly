#include <opencv2/opencv.hpp>
#include <random>
#include <chrono>
#include <string>
#include <vector>
#include <iostream>

namespace {
struct GazePoint {
    cv::Point2f position;
};

std::vector<GazePoint> generateDemoGazePoints(int width, int height, int count = 3) {
    static std::mt19937 rng(static_cast<unsigned int>(
        std::chrono::high_resolution_clock::now().time_since_epoch().count()));
    std::uniform_real_distribution<float> distX(0.0f, static_cast<float>(width - 1));
    std::uniform_real_distribution<float> distY(0.0f, static_cast<float>(height - 1));

    std::vector<GazePoint> points;
    points.reserve(static_cast<std::size_t>(count));
    for (int i = 0; i < count; ++i) {
        points.push_back({cv::Point2f(distX(rng), distY(rng))});
    }
    return points;
}

void drawGazePoints(cv::Mat &heatmap, const std::vector<GazePoint> &points, float intensity = 1.0f, int radius = 40) {
    for (const auto &p : points) {
        cv::circle(heatmap, p.position, radius, cv::Scalar(intensity), -1, cv::LINE_AA);
    }
}

cv::Mat renderHeatmapOverlay(const cv::Mat &frame, cv::Mat &heatmap) {
    cv::Mat blurred;
    cv::GaussianBlur(heatmap, blurred, cv::Size(0, 0), 25.0);

    cv::Mat normalized;
    cv::normalize(blurred, normalized, 0, 255, cv::NORM_MINMAX, CV_8UC1);

    cv::Mat colored;
    cv::applyColorMap(normalized, colored, cv::COLORMAP_JET);

    cv::Mat overlay;
    cv::addWeighted(frame, 0.6, colored, 0.4, 0.0, overlay);
    return overlay;
}
} // namespace

int main(int argc, char **argv) {
    std::string source = "0";
    if (argc > 1) {
        source = argv[1];
    }

    cv::VideoCapture cap;
    if (source == "0") {
        cap.open(0);
    } else {
        cap.open(source);
    }

    if (!cap.isOpened()) {
        std::cerr << "Failed to open video source. Use camera (default) or pass a video path." << std::endl;
        return 1;
    }

    cv::Mat frame;
    if (!cap.read(frame)) {
        std::cerr << "Unable to read initial frame." << std::endl;
        return 1;
    }

    cv::Mat heatmap = cv::Mat::zeros(frame.rows, frame.cols, CV_32FC1);

    std::cout << "Press 'q' to quit. Demo gaze points are randomly generated." << std::endl;

    while (true) {
        if (!cap.read(frame) || frame.empty()) {
            break;
        }

        // Exponential decay so older gaze points slowly fade.
        heatmap *= 0.97f;

        // Replace this demo with real gaze data (x, y in pixels) from your eye tracker.
        auto gazePoints = generateDemoGazePoints(frame.cols, frame.rows);
        drawGazePoints(heatmap, gazePoints);

        cv::Mat overlay = renderHeatmapOverlay(frame, heatmap);
        cv::putText(overlay, "Demo heatmap: replace random gaze points with tracker input", {20, 30},
                    cv::FONT_HERSHEY_SIMPLEX, 0.7, {255, 255, 255}, 2, cv::LINE_AA);
        cv::putText(overlay, "Press 'q' to exit", {20, 60}, cv::FONT_HERSHEY_SIMPLEX, 0.7, {255, 255, 255}, 2,
                    cv::LINE_AA);

        cv::imshow("Gaze Heatmap Overlay", overlay);
        const int key = cv::waitKey(1);
        if (key == 'q' || key == 27) {
            break;
        }
    }

    return 0;
}
