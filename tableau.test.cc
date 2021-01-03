#include "gtest/gtest.h"
#include "tableau.h"
#include "vector_sim.h"

static float complex_distance(std::complex<float> a, std::complex<float> b) {
    auto d = a - b;
    return sqrtf(d.real()*d.real() + d.imag()*d.imag());
}

TEST(tableau, identity) {
    auto t = Tableau::identity(4);
    ASSERT_EQ(t.str(), ""
                       "Tableau {\n"
                       "  qubit 0_x: +X___\n"
                       "  qubit 0_y: +Y___\n"
                       "  qubit 1_x: +_X__\n"
                       "  qubit 1_y: +_Y__\n"
                       "  qubit 2_x: +__X_\n"
                       "  qubit 2_y: +__Y_\n"
                       "  qubit 3_x: +___X\n"
                       "  qubit 3_y: +___Y\n"
                       "}");
}

bool tableau_agrees_with_unitary(const Tableau &tab, const std::vector<std::vector<std::complex<float>>> &unitary) {
    auto n = tab.qubits.size();
    assert(unitary.size() == 1 << n);
    for (size_t xb = 0; xb < 2; xb++) {
        for (size_t k = 0; k < n; k++) {
            PauliString pre = PauliString::identity(n);
            const PauliString &post = xb ? tab.qubits[k].x : tab.qubits[k].y;
            if (xb) {
                pre.toggle_x_bit(k);
            } else {
                pre.toggle_y_bit(k);
            }
            VectorSim sim(n*2);
            for (size_t q = 0; q < n; q++) {
                sim.apply("H", q);
                sim.apply("CNOT", q, q + n);
            }
            sim.apply(pre, n);
            std::vector<size_t> qs;
            for (size_t q = 0; q < n; q++) {
                qs.push_back(q + n);
            }
            sim.apply(unitary, {qs});
            sim.apply(post, n);

            auto scale = powf(0.5f, 0.5f * n);
            for (size_t row = 0; row < 1u << n; row++) {
                for (size_t col = 0; col < 1u << n; col++) {
                    auto a = sim.state[(row << n) | col];
                    auto b = unitary[row][col] * scale;
                    if (complex_distance(a, b) > 1e-4) {
                        return false;
                    }
                }
            }
        }
    }
    return true;
}

TEST(tableau, gate_data) {
    ASSERT_EQ(GATE_TABLEAUS.at("H").str(),
              "Tableau {\n"
              "  qubit 0_x: +Z\n"
              "  qubit 0_y: -Y\n"
              "}");

    for (const auto &kv : GATE_TABLEAUS) {
        const auto &name = kv.first;
        const auto &tab = kv.second;
        const auto &u = GATE_UNITARIES.at(name);
        ASSERT_TRUE(tableau_agrees_with_unitary(tab, u)) << name;
    }
}
