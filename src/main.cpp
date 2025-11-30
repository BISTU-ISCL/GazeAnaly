#include <opencv2/opencv.hpp>

#include <array>
#include <chrono>
#include <iostream>
#include <random>
#include <string>
#include <vector>

namespace {
struct GazeSample {
    cv::Point2f position;
    double durationSeconds;
};

std::vector<GazeSample> generateDemoSamples(int width, int height, int count = 20) {
    std::mt19937 rng(static_cast<unsigned int>(
        std::chrono::high_resolution_clock::now().time_since_epoch().count()));
    std::uniform_real_distribution<float> distX(0.0f, static_cast<float>(width));
    std::uniform_real_distribution<float> distY(0.0f, static_cast<float>(height));
    std::uniform_real_distribution<double> distDuration(0.25, 1.5);

    std::vector<GazeSample> samples;
    samples.reserve(static_cast<std::size_t>(count));
    for (int i = 0; i < count; ++i) {
        samples.push_back({cv::Point2f(distX(rng), distY(rng)), distDuration(rng)});
    }
    return samples;
}

int regionIndexForPoint(const cv::Point2f &p, int width, int height) {
    const float centerX = static_cast<float>(width) / 2.0f;
    const float centerY = static_cast<float>(height) / 2.0f;
    const bool right = p.x >= centerX;
    const bool bottom = p.y >= centerY;

    // AOI naming matches quadrants starting at top-left and going clockwise.
    if (!right && !bottom) return 0;  // AOI1
    if (right && !bottom) return 1;   // AOI2
    if (right && bottom) return 2;    // AOI3
    return 3;                         // AOI4
}

cv::Mat makeBaseCanvas(int width, int height) {
    cv::Mat canvas(height, width, CV_8UC3, cv::Scalar(20, 20, 20));
    const cv::Point center{width / 2, height / 2};
    cv::line(canvas, {center.x, 0}, {center.x, height}, cv::Scalar(80, 80, 80), 1, cv::LINE_AA);
    cv::line(canvas, {0, center.y}, {width, center.y}, cv::Scalar(80, 80, 80), 1, cv::LINE_AA);

    const std::array<std::string, 4> labels{"AOI1", "AOI2", "AOI3", "AOI4"};
    const int padding = 12;
    const double fontScale = 0.8;
    const int thickness = 2;
    cv::putText(canvas, labels[0], {padding, padding + 20}, cv::FONT_HERSHEY_SIMPLEX, fontScale,
                {180, 180, 180}, thickness, cv::LINE_AA);
    cv::putText(canvas, labels[1], {center.x + padding, padding + 20}, cv::FONT_HERSHEY_SIMPLEX,
                fontScale, {180, 180, 180}, thickness, cv::LINE_AA);
    cv::putText(canvas, labels[2], {center.x + padding, center.y + padding + 20},
                cv::FONT_HERSHEY_SIMPLEX, fontScale, {180, 180, 180}, thickness, cv::LINE_AA);
    cv::putText(canvas, labels[3], {padding, center.y + padding + 20}, cv::FONT_HERSHEY_SIMPLEX,
                fontScale, {180, 180, 180}, thickness, cv::LINE_AA);

    return canvas;
}

cv::Scalar colorForAOI(int index) {
    switch (index) {
        case 0:
            return {0, 128, 255};  // Orange
        case 1:
            return {0, 255, 0};    // Green
        case 2:
            return {255, 0, 0};    // Blue
        default:
            return {255, 255, 0};  // Cyan-ish
    }
}
}

int main() {
    const int width = 1280;
    const int height = 720;

    const auto samples = generateDemoSamples(width, height);
    std::array<double, 4> dwellSeconds{0.0, 0.0, 0.0, 0.0};

    const cv::Mat baseCanvas = makeBaseCanvas(width, height);
    std::vector<cv::Point2f> pointsDrawn;

    std::cout << "Demo: synthetic gaze samples across four AOIs (center as origin)." << std::endl;
    std::cout << "Each circle radius reflects how long the participant stared at that location." << std::endl;

    for (const auto &sample : samples) {
        cv::Mat frame = baseCanvas.clone();
        pointsDrawn.push_back(sample.position);

        const int region = regionIndexForPoint(sample.position, width, height);
        dwellSeconds[static_cast<std::size_t>(region)] += sample.durationSeconds;

        const double radius = 12.0 + sample.durationSeconds * 55.0;  // Larger when staring longer.
        for (std::size_t i = 0; i < pointsDrawn.size(); ++i) {
            const auto r = regionIndexForPoint(pointsDrawn[i], width, height);
            cv::circle(frame, pointsDrawn[i], static_cast<int>(radius), colorForAOI(r), -1, cv::LINE_AA);
        }

        const double totalObserved = dwellSeconds[0] + dwellSeconds[1] + dwellSeconds[2] + dwellSeconds[3];
        const auto percentage = [&](int idx) {
            if (totalObserved <= 0.0) return 0.0;
            return dwellSeconds[static_cast<std::size_t>(idx)] / totalObserved * 100.0;
        };

        const int textYStart = height - 80;
        const double fontScale = 0.75;
        const int thickness = 2;
        char buffer[128];
        for (int i = 0; i < 4; ++i) {
            std::snprintf(buffer, sizeof(buffer), "AOI%d: %5.1f%% (%.2fs)", i + 1, percentage(i),
                          dwellSeconds[static_cast<std::size_t>(i)]);
            cv::putText(frame, buffer, {20, textYStart + i * 20}, cv::FONT_HERSHEY_SIMPLEX, fontScale,
                        colorForAOI(i), thickness, cv::LINE_AA);
        }

        cv::putText(frame, "Press Esc or 'q' to quit early", {20, 40}, cv::FONT_HERSHEY_SIMPLEX,
                    0.8, {255, 255, 255}, 2, cv::LINE_AA);
        cv::putText(frame, "Synthetic gaze sequence (one circle per fixation)", {20, 70},
                    cv::FONT_HERSHEY_SIMPLEX, 0.8, {255, 255, 255}, 2, cv::LINE_AA);

        cv::imshow("AOI Gaze Demo", frame);
        const int key = cv::waitKey(450);
        if (key == 'q' || key == 27) {
            break;
        }
    }

    const double totalObserved = dwellSeconds[0] + dwellSeconds[1] + dwellSeconds[2] + dwellSeconds[3];
    std::cout << "\nSummary (percentage of observed time in each AOI):\n";
    for (int i = 0; i < 4; ++i) {
        const double pct = totalObserved > 0.0 ? dwellSeconds[static_cast<std::size_t>(i)] / totalObserved * 100.0
                                               : 0.0;
        std::cout << "  AOI" << (i + 1) << ": " << pct << "% (" << dwellSeconds[static_cast<std::size_t>(i)]
                  << "s)" << std::endl;
    }

    std::cout << "\nReplace the synthetic samples with real eye-tracker coordinates and durations";
    std::cout << " to turn this into a live analysis demo.\n";
    return 0;
}
