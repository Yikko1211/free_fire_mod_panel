// main.cpp
#include "memory.h"
#include "overlay.h"
#include "features.h"
#include "menu.h"
#include "config.h"
#include <iostream>
#include <chrono>
#include <thread>

// BlueStacks process name (varies by version)
const std::string BLUESTACKS_PROCESS = "HD-Player.exe";
const std::string BLUESTACKS_WINDOW = "BlueStacks App Player";

void PrintBanner() {
    std::cout << R"(
    ╔══════════════════════════════════════════════════════╗
    ║     FREE FIRE MOD MENU - EDUCATIONAL PURPOSE ONLY   ║
    ║                                                      ║
    ║  This tool is for learning game hacking concepts.    ║
    ║  Do NOT use this against real players.               ║
    ║  Violation of ToS will result in permanent ban.      ║
    ╚══════════════════════════════════════════════════════╝
    )" << std::endl;
}

void PrintControls() {
    std::cout << "\n[Controls]" << std::endl;
    std::cout << "  INSERT  - Toggle Menu" << std::endl;
    std::cout << "  END     - Exit Application" << std::endl;
    std::cout << "  F1      - Toggle ESP" << std::endl;
    std::cout << "  F2      - Toggle Aimbot" << std::endl;
    std::cout << std::endl;
}

int main() {
    PrintBanner();
    PrintControls();

    // ============================
    // Initialize Memory Manager
    // ============================
    auto memory = std::make_unique<MemoryManager>();
    
    std::cout << "[*] Waiting for BlueStacks process..." << std::endl;
    
    while (!memory->Attach(BLUESTACKS_PROCESS)) {
        std::this_thread::sleep_for(std::chrono::seconds(2));
        if (GetAsyncKeyState(VK_END) & 0x8000) {
            std::cout << "[*] Exit requested." << std::endl;
            return 0;
        }
    }
    
    std::cout << "[+] Attached to BlueStacks!" << std::endl;

    // ============================
    // Initialize Overlay
    // ============================
    auto overlay = std::make_unique<Overlay>();
    
    if (!overlay->Initialize(BLUESTACKS_WINDOW)) {
        std::cout << "[-] Failed to initialize overlay." << std::endl;
        return 1;
    }
    
    std::cout << "[+] Overlay initialized!" << std::endl;

    // ============================
    // Initialize Features
    // ============================
    auto features = std::make_unique<Features>(
        memory.get(), 
        overlay->GetWidth(), 
        overlay->GetHeight()
    );
    
    std::cout << "[+] Features initialized!" << std::endl;

    // ============================
    // Initialize Menu
    // ============================
    auto menu = std::make_unique<Menu>();
    
    if (!menu->Initialize(overlay->GetDevice(), overlay->GetWindow())) {
        std::cout << "[-] Failed to initialize menu." << std::endl;
        return 1;
    }
    
    std::cout << "[+] Menu initialized!" << std::endl;
    std::cout << "\n[*] Running... Press END to exit.\n" << std::endl;

    // ============================
    // Main Loop
    // ============================
    MSG msg;
    ZeroMemory(&msg, sizeof(msg));
    
    auto lastUpdateTime = std::chrono::high_resolution_clock::now();
    const double targetFrameTime = 1.0 / 60.0; // 60 FPS cap
    
    while (msg.message != WM_QUIT) {
        // Process Windows messages
        if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE)) {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
            continue;
        }
        
        // Exit check
        if (GetAsyncKeyState(VK_END) & 0x8000) {
            std::cout << "[*] Exit requested." << std::endl;
            break;
        }
        
        // Check if BlueStacks is still running
        if (!memory->IsAttached()) {
            std::cout << "[-] Lost connection to process." << std::endl;
            break;
        }
        
        // Frame rate limiting
        auto currentTime = std::chrono::high_resolution_clock::now();
        double elapsed = std::chrono::duration<double>(currentTime - lastUpdateTime).count();
        
        if (elapsed < targetFrameTime) {
            std::this_thread::sleep_for(
                std::chrono::microseconds(static_cast<int>((targetFrameTime - elapsed) * 1000000))
            );
            continue;
        }
        lastUpdateTime = currentTime;
        
        // ============================
        // Update Phase
        // ============================
        
        // Process input
        menu->ProcessInput();
        
        // Update overlay position to match BlueStacks window
        overlay->UpdateWindowPosition();
        
        // Update game state
        features->Update();
        
        // Run features
        auto& aimbotSettings = menu->GetAimbotSettings();
        auto& espSettings = menu->GetESPSettings();
        auto& miscSettings = menu->GetMiscSettings();
        
        features->RunAimbot(aimbotSettings);
        features->RunNoRecoil(miscSettings.noRecoil);
        features->RunNoSpread(miscSettings.noSpread);
        features->RunRapidFire(miscSettings.rapidFire);
        features->RunUnlimitedAmmo(miscSettings.unlimitedAmmo);
        features->RunSpeedHack(miscSettings.speedHack, miscSettings.speedMultiplier);
        features->RunCustomFOV(miscSettings.wideView, miscSettings.customFOV);
        
        // ============================
        // Render Phase
        // ============================
        if (overlay->BeginFrame()) {
            menu->Render(overlay.get(), features.get());
            overlay->EndFrame();
        }
    }

    // ============================
    // Cleanup
    // ============================
    std::cout << "[*] Cleaning up..." << std::endl;
    
    menu->Shutdown();
    overlay->Shutdown();
    memory->Detach();
    
    std::cout << "[*] Goodbye!" << std::endl;
    return 0;
}
