#include <gtest/gtest.h>
#include <iostream>
#include <iomanip>

class NetworkCoverageListener : public ::testing::EmptyTestEventListener {
public:
    void OnTestProgramEnd(const ::testing::UnitTest& unit_test) override {
        int total = unit_test.total_test_count();
        int passed = unit_test.successful_test_count();
        int failed = unit_test.failed_test_count();

        std::cout << "\n";
        std::cout << "╔═══════════════════════════════════════════════════════╗\n";
        std::cout << "║            NETWORK PROTOCOL TEST COVERAGE             ║\n";
        std::cout << "╚═══════════════════════════════════════════════════════╝\n";
        std::cout << "\n";

        std::cout << "📊 Test Statistics:\n";
        std::cout << "   • Total Tests:    " << total << "\n";
        std::cout << "   • Passed:         " << "\033[0;32m" << passed << " ✓\033[0m\n";
        std::cout << "   • Failed:         " << (failed > 0 ? "\033[0;31m" : "") << failed << (failed > 0 ? " ✗\033[0m" : "") << "\n";
        
        double success_rate = total > 0 ? (static_cast<double>(passed) / total * 100.0) : 0.0;
        std::cout << "   • Success Rate:   " << std::fixed << std::setprecision(1) << success_rate << "%\n";
        std::cout << "\n";

        std::cout << "🎯 Network Components Covered:\n";
        std::cout << "   ✅ BinarySerializer  - Read/write primitives & strings\n";
        std::cout << "   ✅ Protocol          - OpCode & EntityType validation\n";
        std::cout << "   ✅ Packet Structure  - Login, Input, EntityPosition\n";
        std::cout << "   ✅ Magic Numbers     - Protocol validation (0xB542)\n";
        std::cout << "\n";

        std::cout << "🧪 Functionality Tested:\n";
        std::cout << "   • Binary serialization/deserialization\n";
        std::cout << "   • Buffer overflow protection\n";
        std::cout << "   • Packet encoding/decoding\n";
        std::cout << "   • Protocol opcode validation\n";
        std::cout << "\n";

        if (failed == 0) {
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
    
    ::testing::TestEventListeners& listeners = ::testing::UnitTest::GetInstance()->listeners();
    listeners.Append(new NetworkCoverageListener());
    
    return RUN_ALL_TESTS();
}
