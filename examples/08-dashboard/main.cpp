#include <Flux.hpp>
#include <cmath>
#include <algorithm>
#include <numeric>

using namespace flux;

// Line Chart Component
struct LineChart {
    FLUX_VIEW_PROPERTIES;

    Property<std::string> title = "Sales Trend";
    Property<std::vector<float>> data;
    Property<Color> lineColor = Color::hex(0x3498db);
    Property<Color> chartBackgroundColor = Color::hex(0xecf0f1);
    Property<Size> size = Size{400, 250};


    void render(RenderContext& ctx, const Rect& bounds) const {
        ViewHelpers::renderView(*this, ctx, bounds);

        // Background
        Color bgColor = backgroundColor;
        ctx.drawRoundedRect(bounds, 8, bgColor);

        std::vector<float> dataVec = data;
        if (dataVec.empty()) return;

        // Find min/max for scaling
        float minVal = *std::min_element(dataVec.begin(), dataVec.end());
        float maxVal = *std::max_element(dataVec.begin(), dataVec.end());
        float range = maxVal - minVal;
        if (range == 0) range = 1;

        // Chart area with padding
        EdgeInsets padding(20, 30, 20, 20);
        Rect chartArea = {
            bounds.x + padding.left,
            bounds.y + padding.top,
            bounds.width - padding.horizontal(),
            bounds.height - padding.vertical()
        };

        // Draw line
        Color lineCol = lineColor;
        for (size_t i = 1; i < dataVec.size(); i++) {
            float x1 = chartArea.x + (i - 1) * chartArea.width / (dataVec.size() - 1);
            float y1 = chartArea.y + chartArea.height - ((dataVec[i - 1] - minVal) / range) * chartArea.height;
            float x2 = chartArea.x + i * chartArea.width / (dataVec.size() - 1);
            float y2 = chartArea.y + chartArea.height - ((dataVec[i] - minVal) / range) * chartArea.height;

            ctx.drawLine({x1, y1}, {x2, y2}, 3, lineCol);
        }

        // Draw data points
        for (size_t i = 0; i < dataVec.size(); i++) {
            float x = chartArea.x + i * chartArea.width / (dataVec.size() - 1);
            float y = chartArea.y + chartArea.height - ((dataVec[i] - minVal) / range) * chartArea.height;
            ctx.drawCircle({x, y}, 4, lineCol);
        }

        // Title
        ctx.drawText(static_cast<std::string>(title),
                    {bounds.x + 10, bounds.y + 10},
                    16, Color::hex(0x2c3e50), FontWeight::bold, HorizontalAlignment::leading);
    }

    Size preferredSize(TextMeasurement& textMeasurer) const {
        return static_cast<Size>(size);
    }
};

// Bar Chart Component
struct BarChart {
    FLUX_VIEW_PROPERTIES;

    Property<std::string> title = "Monthly Revenue";
    Property<std::vector<float>> data;
    Property<std::vector<std::string>> labels;
    Property<Color> barColor = Color::hex(0xe74c3c);
    Property<Color> chartBackgroundColor = Color::hex(0xecf0f1);
    Property<Size> size = Size{400, 250};


    void render(RenderContext& ctx, const Rect& bounds) const {
        ViewHelpers::renderView(*this, ctx, bounds);

        // Background
        Color bgColor = backgroundColor;
        ctx.drawRoundedRect(bounds, 8, bgColor);

        std::vector<float> dataVec = data;
        if (dataVec.empty()) return;

        // Find max for scaling
        float maxVal = *std::max_element(dataVec.begin(), dataVec.end());
        if (maxVal == 0) maxVal = 1;

        // Chart area with padding
        EdgeInsets padding(20, 30, 30, 20);
        Rect chartArea = {
            bounds.x + padding.left,
            bounds.y + padding.top,
            bounds.width - padding.horizontal(),
            bounds.height - padding.vertical()
        };

        // Draw bars
        Color barCol = barColor;
        float barWidth = chartArea.width / dataVec.size() * 0.7f;
        float spacing = chartArea.width / dataVec.size() * 0.3f;

        for (size_t i = 0; i < dataVec.size(); i++) {
            float barHeight = (dataVec[i] / maxVal) * chartArea.height;
            float x = chartArea.x + i * (barWidth + spacing);
            float y = chartArea.y + chartArea.height - barHeight;

            ctx.drawRoundedRect({x, y, barWidth, barHeight}, 4, barCol);
        }

        // Title
        ctx.drawText(static_cast<std::string>(title),
                    {bounds.x + 10, bounds.y + 10},
                    16, Color::hex(0x2c3e50), FontWeight::bold, HorizontalAlignment::leading);
    }

    Size preferredSize(TextMeasurement& textMeasurer) const {
        return static_cast<Size>(size);
    }
};

// Doughnut Chart Component
struct DoughnutChart {
    FLUX_VIEW_PROPERTIES;

    Property<std::string> title = "Market Share";
    Property<std::vector<float>> data;
    Property<std::vector<Color>> colors;
    Property<Color> chartBackgroundColor = Color::hex(0xecf0f1);
    Property<Size> size = Size{300, 300};


    void render(RenderContext& ctx, const Rect& bounds) const {
        ViewHelpers::renderView(*this, ctx, bounds);

        // Background
        Color bgColor = backgroundColor;
        ctx.drawRoundedRect(bounds, 8, bgColor);

        std::vector<float> dataVec = data;
        if (dataVec.empty()) return;

        // Calculate total
        float total = std::accumulate(dataVec.begin(), dataVec.end(), 0.0f);
        if (total == 0) return;

        // Center and radius
        float centerX = bounds.x + bounds.width / 2;
        float centerY = bounds.y + bounds.height / 2;
        float radius = std::min(bounds.width, bounds.height) / 2 - 40;
        float innerRadius = radius * 0.6f;

        // Draw segments
        std::vector<Color> colorsVec = colors;
        float currentAngle = -90; // Start at top

        for (size_t i = 0; i < dataVec.size(); i++) {
            float angle = (dataVec[i] / total) * 360;
            Color segmentColor = i < colorsVec.size() ? colorsVec[i] : Color::hex(0x95a5a6);

            // Draw outer arc segment
            for (float a = 0; a < angle; a += 2) {
                float rad1 = (currentAngle + a) * M_PI / 180;
                float rad2 = (currentAngle + a + 2) * M_PI / 180;

                Point p1 = {centerX + std::cos(rad1) * radius, centerY + std::sin(rad1) * radius};
                Point p2 = {centerX + std::cos(rad2) * radius, centerY + std::sin(rad2) * radius};
                Point p3 = {centerX + std::cos(rad2) * innerRadius, centerY + std::sin(rad2) * innerRadius};
                Point p4 = {centerX + std::cos(rad1) * innerRadius, centerY + std::sin(rad1) * innerRadius};

                // Simple line approximation for arc
                ctx.drawLine(p1, p2, 3, segmentColor);
            }

            currentAngle += angle;
        }

        // Title
        ctx.drawText(static_cast<std::string>(title),
                    {bounds.x + 10, bounds.y + 10},
                    16, Color::hex(0x2c3e50), FontWeight::bold, HorizontalAlignment::leading);
    }

    Size preferredSize(TextMeasurement& textMeasurer) const {
        return static_cast<Size>(size);
    }
};

int main(int argc, char* argv[]) {
    Application app(argc, argv);

    Window window({
        .size = {1200, 800},
        .title = "Business Dashboard"
    });

    // Sample data for charts
    std::vector<float> salesData = {120, 135, 148, 142, 156, 178, 192, 201, 185, 203, 218, 225};
    std::vector<float> revenueData = {45, 52, 48, 61, 55, 67, 72, 69, 75, 82, 78, 85};
    std::vector<float> marketShareData = {35, 25, 20, 12, 8};

    std::vector<Color> doughnutColors = {
        Color::hex(0x3498db),
        Color::hex(0xe74c3c),
        Color::hex(0xf39c12),
        Color::hex(0x2ecc71),
        Color::hex(0x9b59b6)
    };

    window.setRootView(
        VStack {
            .padding = EdgeInsets{20},
            .spacing = 20,
            .children = {
                // Header
                HStack {
                    .children = {
                        Text {
                            .value = "Business Dashboard",
                            .fontSize = 28,
                            .fontWeight = FontWeight::bold,
                            .color = Color::hex(0x2c3e50)
                        },
                        Spacer {},
                        Text {
                            .value = "Last Updated: " + std::string(__DATE__),
                            .fontSize = 14,
                            .color = Color::hex(0x7f8c8d)
                        }
                    }
                },

                // First row of charts
                HStack {
                    .spacing = 20,
                    .children = {
                        LineChart {
                            .title = "Sales Trend (12 months)",
                            .data = salesData,
                            .lineColor = Color::hex(0x3498db),
                            .chartBackgroundColor = Color::hex(0xffffff),
                            .size = Size{580, 280}
                        },
                        DoughnutChart {
                            .title = "Market Share",
                            .data = marketShareData,
                            .colors = doughnutColors,
                            .chartBackgroundColor = Color::hex(0xffffff),
                            .size = Size{300, 280}
                        }
                    }
                },

                // Second row
                HStack {
                    .spacing = 20,
                    .children = {
                        BarChart {
                            .title = "Monthly Revenue ($K)",
                            .data = revenueData,
                            .barColor = Color::hex(0xe74c3c),
                            .chartBackgroundColor = Color::hex(0xffffff),
                            .size = Size{580, 280}
                        },

                        // Stats cards
                        VStack {
                            .spacing = 15,
                            .children = {
                                VStack {
                                    .padding = EdgeInsets{15},
                                    .backgroundColor = Color::hex(0xffffff),
                                    .cornerRadius = 8,
                                    .children = {
                                        Text {
                                            .value = "Total Revenue",
                                            .fontSize = 14,
                                            .color = Color::hex(0x7f8c8d)
                                        },
                                        Text {
                                            .value = "$847K",
                                            .fontSize = 24,
                                            .fontWeight = FontWeight::bold,
                                            .color = Color::hex(0x27ae60)
                                        }
                                    }
                                },
                                VStack {
                                    .padding = EdgeInsets{15},
                                    .backgroundColor = Color::hex(0xffffff),
                                    .cornerRadius = 8,
                                    .children = {
                                        Text {
                                            .value = "Growth Rate",
                                            .fontSize = 14,
                                            .color = Color::hex(0x7f8c8d)
                                        },
                                        Text {
                                            .value = "+12.5%",
                                            .fontSize = 24,
                                            .fontWeight = FontWeight::bold,
                                            .color = Color::hex(0x3498db)
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    );

    return app.exec();
}
