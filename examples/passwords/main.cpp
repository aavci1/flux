#include <Flux.hpp>

using namespace flux;

struct PasswordItem {
    std::string icon;
    std::string name;
    std::string username;
    std::string password;
    std::string website;
    std::string notes;

};

const std::string svgString = R"(<svg width="256px" height="256px" viewBox="0 0 256 256" version="1.1" xmlns="http://www.w3.org/2000/svg" xmlns:xlink="http://www.w3.org/1999/xlink" preserveAspectRatio="xMidYMid">
<path d="M127.962816,0 C198.911815,0.558586285 255.754376,55.4725719 255.999327,129.30333 C256.222426,196.981097 200.939466,257.457031 124.744539,255.972592 C55.244106,254.617978 -1.52329281,198.006026 0.0311827848,124.08815 C1.40287938,58.8018828 54.6684376,0.674744901 127.962816,0 Z M101.003768,147.357453 C105.554452,147.357453 110.106845,147.367703 114.65753,147.352329 C116.833796,147.345496 118.196951,147.837462 117.689611,150.565481 C116.574147,156.564732 115.660252,162.599855 114.679737,168.621313 C112.832587,179.971832 110.991701,191.32349 109.157078,202.676286 C108.981132,203.757586 108.415712,204.914048 109.809616,205.660538 C112.182326,206.929741 118.670127,205.31377 120.011075,203.0743 C137.018063,174.694017 154.01651,146.30861 171.006416,117.918077 C174.535588,112.012778 172.683883,108.820124 165.760488,108.768878 C157.794512,108.711938 149.828537,108.711938 141.862561,108.768878 C139.389066,108.782544 137.708182,108.456275 138.304349,105.219207 C140.080893,95.5899998 141.703697,85.9334607 143.331626,76.2769217 C144.604815,68.7175012 145.824481,61.1489702 146.990622,53.5713287 C147.16486,52.4404904 147.853271,51.0414624 146.011815,50.3701339 C142.079426,49.2279869 137.854573,50.6944931 135.475545,54.0274221 C122.392896,75.8151342 109.320497,97.6091098 96.2583467,119.409349 C92.3157866,125.985976 88.2553597,132.494275 84.4989951,139.176812 C81.6838569,144.181882 83.5321454,147.282292 89.0565125,147.34208 C93.0383616,147.384785 97.0219188,147.350621 101.005476,147.359162 L101.003768,147.357453 Z" fill="#1D5FE6" />
</svg>)";

struct PasswordDetails {
    FLUX_VIEW_PROPERTIES;

    Property<PasswordItem> item;

    View body() const {
        auto itemVal = item.get();
        
        return VStack {
            .backgroundColor = Colors::lightGray.opacity(0.4f),
            .borderColor = Colors::gray.opacity(0.4f),
            .borderWidth = 1,
            .cornerRadius = 16,
            .padding = 16,
            .spacing = 16,
            .children = {
                HStack {
                    .justifyContent = JustifyContent::spaceBetween,
                    .children = {
                        Text { .value = "Name" },
                        Text { .value = itemVal.name, .color = Colors::gray }
                    }
                },
                Divider {},
                HStack {
                    .justifyContent = JustifyContent::spaceBetween,
                    .children = {
                        Text { .value = "Username" },
                        Text { .value = itemVal.username, .color = Colors::gray }
                    }
                },
                Divider {},
                HStack {
                    .justifyContent = JustifyContent::spaceBetween,
                    .children = {
                        Text { .value = "Password" },
                        Text { .value = itemVal.password, .color = Colors::gray }
                    }
                },
                Divider {},
                HStack {
                    .justifyContent = JustifyContent::spaceBetween,
                    .children = {
                        Text { .value = "Website" },
                        Text { .value = itemVal.website, .color = Colors::gray }
                    }
                },
                Divider {},
                HStack {
                    .justifyContent = JustifyContent::spaceBetween,
                    .children = {
                        Text { .value = "Notes" },
                        Text { .value = itemVal.notes, .color = Colors::gray }
                    }
                }
            }
        };
    }
};

int main(int argc, char* argv[]) {
    Application app(argc, argv);

    Property<std::optional<PasswordItem>> selectedItem = std::nullopt;
    Property<std::vector<PasswordItem>> passwords = {
        {
            { "", "DoorDash", "Lanchi.Pederson@icloud.com", "password123", "doordash.com", "" },
            { "", "Spotify", "Lanchi.Pederson@icloud.com", "password123", "spotify.com", "" },
            { "", "Apple", "Lanchi.Pederson@icloud.com", "password123", "apple.com", "" },
            { "", "Netflix", "Lanchi.Pederson@icloud.com", "password123", "netflix.com", "" },
            { "", "Amazon", "Lanchi.Pederson@icloud.com", "password123", "amazon.com", "" },
            { "", "Microsoft", "Lanchi.Pederson@icloud.com", "password123", "microsoft.com", "" },
            { "", "Google", "Lanchi.Pederson@icloud.com", "password123", "google.com", "" },
            { "", "Facebook", "Lanchi.Pederson@icloud.com", "password123", "facebook.com", "" },
            { "", "Twitter", "Lanchi.Pederson@icloud.com", "password123", "twitter.com", "" },
            { "", "Instagram", "Lanchi.Pederson@icloud.com", "password123", "instagram.com", "" },
            { "", "LinkedIn", "Lanchi.Pederson@icloud.com", "password123", "linkedin.com", "" },
            { "", "GitHub", "Lanchi.Pederson@icloud.com", "password123", "github.com", "" },
            { "", "YouTube", "Lanchi.Pederson@icloud.com", "password123", "youtube.com", "" },
            { "", "Twitch", "Lanchi.Pederson@icloud.com", "password123", "twitch.tv", "" },
            { "", "Reddit", "Lanchi.Pederson@icloud.com", "password123", "reddit.com", "" },
            { "", "Discord", "Lanchi.Pederson@icloud.com", "password123", "discord.com", "" },
            { "", "Steam", "Lanchi.Pederson@icloud.com", "password123", "steam.com", "" }
        }
    };

    std::vector<View> passwordViews;

    const auto& passwordsVal = passwords.get();
    for (size_t i = 0; i < passwordsVal.size(); i++) {
        const auto& password = passwordsVal[i];

        if (i > 0) {
            passwordViews.push_back(Divider {
                .orientation = DividerOrientation::Horizontal
            });
        }
        passwordViews.push_back(
            HStack {
                .backgroundColor = [&selectedItem, password]() {
                    auto selectedItemVal = selectedItem.get();

                    if (selectedItemVal && selectedItemVal->name == password.name) {
                        return Colors::blue;
                    }

                    return Colors::transparent;
                },
                .cornerRadius = 8,
                .spacing = 16,
                .padding = EdgeInsets{16, 8},
                .children = {
                    SVG {
                        .content = svgString,
                        .compressionBias = 1.0f,
                        .size = Size{40.0f, 40.0f}
                    },
                    VStack {
                        .expansionBias = 1.0f,
                        .spacing = 4,
                        .children = {
                            Text { 
                                .value = password.name,
                                .color = [&selectedItem, password]() {
                                    auto selectedItemVal = selectedItem.get();

                                    if (selectedItemVal && selectedItemVal->name == password.name) {
                                        return Colors::white;
                                    }

                                    return Colors::black;
                                },
                                .fontWeight = FontWeight::bold,
                                .fontSize = 16,
                                .horizontalAlignment = HorizontalAlignment::leading
                            },
                            Text { 
                                .value = password.username,
                                .color = [&selectedItem, password]() {
                                    auto selectedItemVal = selectedItem.get();

                                    if (selectedItemVal && selectedItemVal->name == password.name) {
                                        return Colors::white;
                                    }

                                    return Colors::gray;
                                },
                                .fontSize = 14,
                                .horizontalAlignment = HorizontalAlignment::leading
                            }
                        }
                    }
                },
                .onClick = [&selectedItem, password]() {
                    selectedItem = password;
                }
            }
        );
    }

    Window window({
        .size = {1200, 900},
        .title = "Passwords"
    });

    window.setRootView(
        HStack {
            .children = {
                VStack {
                    .padding = 16,
                    .spacing = 16,
                    .minWidth = 360.0f,
                    .maxWidth = 360.0f,
                    .children = {
                        HStack {
                            .children = {
                                VStack {
                                    .children = {
                                        Text {
                                            .value = "All",
                                            .horizontalAlignment = HorizontalAlignment::leading,
                                            .fontWeight = FontWeight::bold,
                                            .fontSize = 16
                                        },
                                        Text {
                                            .value = "Passwords",
                                            .horizontalAlignment = HorizontalAlignment::leading,
                                            .color = Colors::gray,
                                            .fontSize = 14
                                        }
                                    }
                                },
                            }
                        },

                        ScrollArea {
                            .expansionBias = 1.0f,
                            .compressionBias = 1.0f,
                            .children = passwordViews
                        }
                    }
                },
                Divider {
                    .orientation = DividerOrientation::Vertical
                },
                VStack {
                    .padding = 16,
                    .spacing = 16,
                    .expansionBias = 1.0f,
                    .children = [&selectedItem]() {
                        auto selectedItemVal = selectedItem.get();
                        std::vector<View> children;

                        if (selectedItemVal) {
                            children.push_back(PasswordDetails { .item = *selectedItemVal });
                        } else {
                            children.push_back(Text { .value = "No password selected" });
                        }

                        return children;
                    }
                }
            }
        }
    );

    return app.exec();
}