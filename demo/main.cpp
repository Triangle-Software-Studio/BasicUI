#include <basicui/basicui.h>
#include <basicui/platform/window.h>
#include <basicui/render/text_grid.h>
#include <basicui/render/glyph_atlas.h>
#include <basicui/render/renderer.h>
#include <basicui/style/font.h>
#include <basicui/style/theme.h>
#include <basicui/widget/screen.h>
#include <basicui/widget/panel.h>
#include <basicui/widget/button.h>
#include <basicui/widget/input.h>
#include <basicui/widget/label.h>
#include <basicui/widget/menubar.h>
#include <basicui/widget/checkbox.h>
#include <basicui/widget/listview.h>
#include <basicui/widget/scrollbar.h>
#include <basicui/widget/progressbar.h>
#include <basicui/widget/radiobutton.h>
#include <basicui/widget/combobox.h>
#include <basicui/widget/tab.h>
#include <basicui/widget/tooltip.h>
#include <iostream>
#include <memory>
#include <GL/glew.h>
#include <SDL.h>

using namespace bui;

int main(int argc, char* argv[]) {
    std::string fontPath = (argc > 1) ? argv[1] : "fonts/CascadiaMonoNF-Regular.ttf";

    try {
        // Register fonts through the global font manager. Sizes can be changed
        // at runtime by re-registering and refreshing the renderer atlas.
        FontManager::Instance().Register("default", fontPath, 24);
        FontManager::Instance().Register("small", fontPath, 20);
        FontManager::Instance().SetDefaultFont("default");

        Window window("BasicUI Widget Gallery", 80, 30, 24);

        Renderer renderer;
        if (!renderer.Initialize(&window)) {
            std::cerr << "Renderer init failed\n";
            return 1;
        }

        // Load atlases after the OpenGL context is active and GLEW is initialized.
        auto defaultAtlas = FontManager::Instance().GetAtlas("default");
        auto smallAtlas = FontManager::Instance().GetAtlas("small");
        if (!defaultAtlas || !smallAtlas) {
            std::cerr << "Failed to load font: " << fontPath << "\n";
            return 1;
        }

        window.SetCellSize(defaultAtlas->CellWidth(), defaultAtlas->CellHeight());
        window.ResizeToCells(80, 30);

        renderer.SetGlyphAtlas(defaultAtlas);
        renderer.SetFontAtlas("small", smallAtlas);

        auto screen = std::make_shared<Screen>();
        screen->SetBounds({0, 0, window.Cols(), window.Rows()});

        // ===== MenuBar =====
        auto menuBar = std::make_shared<MenuBar>();
        menuBar->SetBounds({0, 0, 80, 1});
        menuBar->AddMenu("File", {
            {"New", []() { std::cout << "[Menu] New\n"; }},
            {"Open", []() { std::cout << "[Menu] Open\n"; }},
            {"Exit", [&]() { window.Close(); }}
        });
        menuBar->AddMenu("Edit", {
            {"Copy", []() { std::cout << "[Menu] Copy\n"; }},
            {"Paste", []() { std::cout << "[Menu] Paste\n"; }}
        });
        menuBar->AddMenu("Help", {
            {"About", []() { std::cout << "[Menu] About BasicUI\n"; }}
        });
        screen->AddChild(menuBar);

        // ===== Title =====
        auto titleLabel = std::make_shared<Label>(" BasicUI Widget Gallery ");
        titleLabel->SetBounds({25, 2, 30, 1}).SetFont("small");
        screen->AddChild(titleLabel);

        // ===== CheckBox =====
        auto checkBox = std::make_shared<CheckBox>("Show Details");
        checkBox->SetBounds({2, 2, 18, 1});
        TooltipManager::Instance().Register(checkBox, "Toggle additional details in the output panel");
        screen->AddChild(checkBox);

        // ===== Name Label + Input =====
        auto nameLabel = std::make_shared<Label>("Name:");
        nameLabel->SetBounds({2, 4, 6, 1});
        screen->AddChild(nameLabel);

        auto input = std::make_shared<Input>();
        input->SetBounds({9, 4, 30, 1}).SetPlaceholder("Type your name...");
        TooltipManager::Instance().Register(input, "Enter your name and press Enter");
        screen->AddChild(input);

        // ===== Button =====
        auto button = std::make_shared<Button>("Greet");
        button->SetBounds({2, 6, 12, 3});
        TooltipManager::Instance().Register(button, "Click to update the greeting message");
        screen->AddChild(button);

        // ===== Output Panel =====
        auto outputPanel = std::make_shared<Panel>("Output");
        outputPanel->SetBounds({2, 10, 36, 6});
        auto outputLabel = std::make_shared<Label>("Hello, stranger!");
        outputPanel->AddChildRelative(outputLabel, {1, 1, 34, 1});
        screen->AddChild(outputPanel);

        // ===== TabControl =====
        auto tabControl = std::make_shared<TabControl>();
        tabControl->SetBounds({42, 2, 36, 20});
        TooltipManager::Instance().Register(tabControl, "Switch between Settings and Items");

        auto settingsPanel = std::make_shared<Panel>("Settings");
        auto themeLabel = std::make_shared<Label>("Theme:");
        themeLabel->SetBounds({1, 1, 7, 1});
        settingsPanel->AddChildRelative(themeLabel, {1, 1, 7, 1});

        auto comboBox = std::make_shared<ComboBox>();
        comboBox->AddItem("Dark").AddItem("Light").AddItem("High Contrast");
        settingsPanel->AddChildRelative(comboBox, {9, 1, 20, 1});

        auto radio1 = std::make_shared<RadioButton>("Classic");
        radio1->SetGroup("mode").SetChecked(true);
        settingsPanel->AddChildRelative(radio1, {1, 3, 12, 1});

        auto radio2 = std::make_shared<RadioButton>("Modern");
        radio2->SetGroup("mode");
        settingsPanel->AddChildRelative(radio2, {1, 4, 12, 1});

        auto radio3 = std::make_shared<RadioButton>("Minimal");
        radio3->SetGroup("mode");
        settingsPanel->AddChildRelative(radio3, {1, 5, 12, 1});

        auto progressLabel = std::make_shared<Label>("Progress:");
        settingsPanel->AddChildRelative(progressLabel, {1, 7, 10, 1});

        auto progressBar = std::make_shared<ProgressBar>();
        progressBar->SetValue(65);
        settingsPanel->AddChildRelative(progressBar, {1, 8, 24, 1});

        auto itemsPanel = std::make_shared<Panel>("Items");
        auto listView = std::make_shared<ListView>();
        listView->AddItem("README.md")
                .AddItem("CMakeLists.txt")
                .AddItem("src/widget/button.cpp")
                .AddItem("src/widget/input.cpp")
                .AddItem("src/render/renderer.cpp")
                .AddItem("include/basicui/style/theme.h")
                .AddItem("demo/main.cpp")
                .AddItem("fonts/CascadiaMonoNF-Regular.ttf")
                .AddItem("build/Release/basicui_demo.exe")
                .AddItem("third_party/glad/glad.h");
        itemsPanel->AddChildRelative(listView, {1, 1, 32, 16});

        auto vScroll = std::make_shared<ScrollBar>(ScrollBar::Vertical);
        vScroll->SetRange(static_cast<int>(listView->ItemCount()), 16).SetPosition(0);
        vScroll->OnScroll([&](int pos) { listView->SetScrollOffset(pos); });
        itemsPanel->AddChildRelative(vScroll, {33, 1, 1, 16});

        tabControl->WithTab("Settings", settingsPanel)
                  .WithTab("Items", itemsPanel)
                  .SelectedIndex(0);
        screen->AddChild(tabControl);

        // ===== Bottom ProgressBar =====
        auto hProgressLabel = std::make_shared<Label>("Task Progress:");
        hProgressLabel->SetBounds({2, 22, 15, 1}).SetFont("small");
        screen->AddChild(hProgressLabel);

        auto hProgressBar = std::make_shared<ProgressBar>();
        hProgressBar->SetBounds({2, 23, 76, 1}).SetValue(42);
        screen->AddChild(hProgressBar);

        // ===== Status Labels =====
        auto selectedLabel = std::make_shared<Label>("Selected: None");
        selectedLabel->SetBounds({2, 25, 40, 1}).SetFont("small");
        screen->AddChild(selectedLabel);

        auto checkLabel = std::make_shared<Label>("Checked: false");
        checkLabel->SetBounds({2, 26, 20, 1}).SetFont("small");
        screen->AddChild(checkLabel);

        auto comboLabel = std::make_shared<Label>("Theme: Dark");
        comboLabel->SetBounds({42, 25, 20, 1}).SetFont("small");
        screen->AddChild(comboLabel);

        auto radioLabel = std::make_shared<Label>("Mode: Classic");
        radioLabel->SetBounds({42, 26, 20, 1}).SetFont("small");
        screen->AddChild(radioLabel);

        // ===== Hint =====
        auto hint = std::make_shared<Label>("Tab: switch focus | Click/Enter: activate | Arrow keys: navigate | Space: toggle");
        hint->SetBounds({2, 28, 76, 1}).SetFont("small");
        screen->AddChild(hint);

        // ===== Event wiring =====
        auto updateOutput = [&]() {
            std::string name = input->GetText();
            if (name.empty()) name = "stranger";
            outputLabel->SetText("Hello, " + name + "!");
        };

        button->OnClick(updateOutput);
        input->OnSubmit([&](const std::string&) { updateOutput(); });

        listView->OnSelect([&](int idx, const std::string& item) {
            selectedLabel->SetText("Selected: " + item);
        });

        checkBox->OnToggle([&](bool checked) {
            checkLabel->SetText(checked ? "Checked: true" : "Checked: false");
            outputPanel->SetTitle(checked ? "Output (Details)" : "Output");
        });

        comboBox->OnSelect([&](int idx, const std::string& item) {
            comboLabel->SetText("Theme: " + item);
        });

        auto setRadioLabel = [&](const std::string& label) {
            radioLabel->SetText("Mode: " + label);
        };

        radio1->OnToggle([&](bool checked) {
            if (checked) {
                setRadioLabel("Classic");
                radio2->SetChecked(false);
                radio3->SetChecked(false);
            }
        });
        radio2->OnToggle([&](bool checked) {
            if (checked) {
                setRadioLabel("Modern");
                radio1->SetChecked(false);
                radio3->SetChecked(false);
            }
        });
        radio3->OnToggle([&](bool checked) {
            if (checked) {
                setRadioLabel("Minimal");
                radio1->SetChecked(false);
                radio2->SetChecked(false);
            }
        });

        TextGrid grid(window.Cols(), window.Rows());

        bool running = true;
        while (running && window.IsOpen()) {
            Event ev;
            while (window.PollEvent(ev)) {
                if (ev.type == EventType::Close) {
                    running = false;
                    break;
                }
                screen->OnEvent(ev);
            }

            if (grid.Width() != window.Cols() || grid.Height() != window.Rows()) {
                grid.Resize(window.Cols(), window.Rows());
                screen->SetBounds({0, 0, window.Cols(), window.Rows()});
            }

            grid.Clear(Theme::DefaultBg);
            screen->Render(grid);

            renderer.Render(grid, window.Cols(), window.Rows());
            renderer.SwapBuffers();

            SDL_Delay(16);
        }

    } catch (const std::exception& e) {
        std::cerr << "Exception: " << e.what() << "\n";
        return 1;
    }

    return 0;
}
