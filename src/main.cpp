#include "memory.h"
#include "overlay.h"
#include "features.h"
#include "menu.h"
#include "config.h"
#include <iostream>
#include <chrono>
#include <thread>

const std::string BLUESTACKS_PROCESS = "HD-Player.exe";
const std::string BLUESTACKS_WINDOW = "BlueStacks App Player";

void PrintBanner() {
    std::cout << R"(
    =====================================================
      FREE FIRE MOD MENU - EDUCATIONAL PURPOSE ONLY
    =====================================================
    )" << std::endl;
}

int main() {
    PrintBanner();

    std::cout << "[Controls]" << std::endl;
    std::cout << "  INSERT  - Toggle Menu (show/hide)" << std::endl;
    std::cout << "  END     - Exit" << std::endl;
    std::cout << "  F1      - Toggle ESP" << std::endl;
    std::cout << "  F2      - Toggle Aimbot" << std::endl;
    std::cout << std::endl;

    // Memory
    auto memory = std::make_unique<MemoryManager>();

    std::cout << "[*] Buscando proceso de BlueStacks..." << std::endl;

    while (!memory->Attach(BLUESTACKS_PROCESS)) {
        std::this_thread::sleep_for(std::chrono::seconds(2));
        if (GetAsyncKeyState(VK_END) & 0x8000) {
            std::cout << "[*] Salida solicitada." << std::endl;
            return 0;
        }
    }

    std::cout << "[+] Conectado a BlueStacks!" << std::endl;

    // Overlay
    auto overlay = std::make_unique<Overlay>();

    if (!overlay->Initialize(BLUESTACKS_WINDOW)) {
        std::cout << "[-] Error al inicializar overlay." << std::endl;
        std::cout << "    Asegurate de que BlueStacks este abierto." << std::endl;
        system("pause");
        return 1;
    }

    std::cout << "[+] Overlay inicializado!" << std::endl;

    // Features
    auto features = std::make_unique<Features>(
        memory.get(),
        overlay->GetWidth(),
        overlay->GetHeight()
    );

    // Menu
    auto menu = std::make_unique<Menu>();

    if (!menu->Initialize(overlay->GetDevice(), overlay->GetWindow())) {
        std::cout << "[-] Error al inicializar menu." << std::endl;
        system("pause");
        return 1;
    }

    std::cout << "[+] Menu inicializado!" << std::endl;
    std::cout << "\n[*] Ejecutando... Presiona END para salir.\n" << std::endl;
    std::cout << "[*] Presiona INSERT para mostrar/ocultar el menu." << std::endl;
    std::cout << "[*] Menu VISIBLE = puedes hacer click en el menu" << std::endl;
    std::cout << "[*] Menu OCULTO  = clicks van al juego" << std::endl;

    // Main loop
    MSG msg;
    ZeroMemory(&msg, sizeof(msg));

    while (msg.message != WM_QUIT) {
        // Procesar mensajes de Windows
        while (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE)) {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
            if (msg.message == WM_QUIT) break;
        }
        if (msg.message == WM_QUIT) break;

        // Salir con END
        if (GetAsyncKeyState(VK_END) & 0x8000) {
            std::cout << "[*] Salida solicitada." << std::endl;
            break;
        }

        // Verificar que BlueStacks siga abierto
        if (!IsWindow(overlay->GetTargetWindow())) {
            std::cout << "[-] BlueStacks se cerro." << std::endl;
            break;
        }

        // Input
        menu->ProcessInput(overlay.get());

        // Actualizar posición del overlay
        overlay->UpdateWindowPosition();

        // Actualizar estado del juego
        features->Update();

        // Ejecutar features
        auto& aimbotSettings = menu->GetAimbotSettings();
        auto& miscSettings = menu->GetMiscSettings();

        features->RunAimbot(aimbotSettings);
        features->RunNoRecoil(miscSettings.noRecoil);
        features->RunNoSpread(miscSettings.noSpread);
        features->RunRapidFire(miscSettings.rapidFire);
        features->RunUnlimitedAmmo(miscSettings.unlimitedAmmo);
        features->RunSpeedHack(miscSettings.speedHack, miscSettings.speedMultiplier);
        features->RunCustomFOV(miscSettings.wideView, miscSettings.customFOV);

        // Render
        if (overlay->BeginFrame()) {
            menu->Render(overlay.get(), features.get());
            overlay->EndFrame();
        }

        // Limitar FPS
        std::this_thread::sleep_for(std::chrono::milliseconds(16)); // ~60 FPS
    }

    // Cleanup
    std::cout << "[*] Limpiando..." << std::endl;
    menu->Shutdown();
    overlay->Shutdown();
    memory->Detach();
    std::cout << "[*] Adios!" << std::endl;

    return 0;
}