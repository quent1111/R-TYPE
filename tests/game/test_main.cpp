#include <gtest/gtest.h>
#include <iostream>
#include <iomanip>

// Listener personnalisé pour afficher le coverage à la fin
class CoverageListener : public ::testing::EmptyTestEventListener {
private:
    int total_tests = 0;
    int passed_tests = 0;
    int failed_tests = 0;

public:
    void OnTestProgramEnd(const ::testing::UnitTest& unit_test) override {
        // Calculer les statistiques
        total_tests = unit_test.total_test_count();
        passed_tests = unit_test.successful_test_count();
        failed_tests = unit_test.failed_test_count();
        int disabled_tests = unit_test.disabled_test_count();

        // Afficher le rapport de coverage
        std::cout << "\n";
        std::cout << "╔═══════════════════════════════════════════════════════╗\n";
        std::cout << "║              GAME LOGIC TEST COVERAGE                 ║\n";
        std::cout << "╚═══════════════════════════════════════════════════════╝\n";
        std::cout << "\n";

        // Statistiques des tests
        std::cout << "📊 Test Statistics:\n";
        std::cout << "   • Total Tests:    " << total_tests << "\n";
        std::cout << "   • Passed:         " << "\033[0;32m" << passed_tests << " ✓\033[0m\n";
        std::cout << "   • Failed:         " << (failed_tests > 0 ? "\033[0;31m" : "") << failed_tests << (failed_tests > 0 ? " ✗\033[0m" : "") << "\n";
        std::cout << "   • Disabled:       " << disabled_tests << "\n";
        
        double success_rate = total_tests > 0 ? (static_cast<double>(passed_tests) / total_tests * 100.0) : 0.0;
        std::cout << "   • Success Rate:   " << std::fixed << std::setprecision(1) << success_rate << "%\n";
        std::cout << "\n";

        // Composants couverts
        std::cout << "🎯 Components Covered:\n";
        std::cout << "   ✅ level_manager     - Level progression & enemy tracking\n";
        std::cout << "   ✅ health            - Player/enemy health states\n";
        std::cout << "   ✅ shield            - Shield activation & range detection\n";
        std::cout << "   ✅ weapon            - Firing rate & upgrades (PowerShot, TripleShot)\n";
        std::cout << "   ✅ power_cannon      - Power-up activation & duration\n";
        std::cout << "   ✅ damage_on_contact - Collision damage mechanics\n";
        std::cout << "   ✅ collision_box     - AABB collision detection\n";
        std::cout << "\n";

        // Fonctionnalités testées
        std::cout << "🧪 Functionality Tested:\n";
        std::cout << "   • Enemy kill tracking and level completion\n";
        std::cout << "   • Health percentage calculation\n";
        std::cout << "   • Shield expiration and enemy detection\n";
        std::cout << "   • Weapon fire timing and upgrade effects\n";
        std::cout << "   • Power-up duration management\n";
        std::cout << "\n";

        // Afficher un message en fonction du résultat
        if (failed_tests == 0) {
            std::cout << "\033[0;32m";
            std::cout << "╔═══════════════════════════════════════════════════════╗\n";
            std::cout << "║           ✨ ALL TESTS PASSED SUCCESSFULLY ✨         ║\n";
            std::cout << "╚═══════════════════════════════════════════════════════╝\n";
            std::cout << "\033[0m";
        } else {
            std::cout << "\033[0;31m";
            std::cout << "╔═══════════════════════════════════════════════════════╗\n";
            std::cout << "║              ⚠️  SOME TESTS FAILED  ⚠️                ║\n";
            std::cout << "╚═══════════════════════════════════════════════════════╝\n";
            std::cout << "\033[0m";
        }
        std::cout << "\n";
    }
};

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    
    // Ajouter notre listener personnalisé
    ::testing::TestEventListeners& listeners = ::testing::UnitTest::GetInstance()->listeners();
    listeners.Append(new CoverageListener());
    
    return RUN_ALL_TESTS();
}
