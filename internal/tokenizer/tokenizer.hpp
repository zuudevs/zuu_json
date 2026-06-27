/**
 * @file tokenizer.hpp
 * @author zuudevs (zuudevs@gmail.com)
 * @brief Policy-Based Tokenizer Orchestrator
 * @version 1.1.0
 * @date 2026-06-26
 *
 * @copyright Copyright (c) 2026
 */

#pragma once

#include "tokenizer/policies.hpp"
#include <span>

namespace zuu::tokenizer {

/**
 * @brief Tokenizer Engine berbasis Policy.
 * Desain ini mengizinkan pengguna memilih implementasi komputasi (Backend),
 * metode alokasi, dan penanganan lainnya secara ekslisit pada compile-time.
 * 
 * @tparam Policy Struct yang mendefinisikan alias tipe (misal: Backend, Allocator).
 */
template <typename Policy>
class Tokenizer {
  public:
    using Engine = typename Policy::Engine;
    using Expected = typename Engine::Expected;
    using Raw = std::span<const char>;

    /**
     * @brief Menjalankan proses tokenisasi statis berdasarkan Policy yang diberikan.
     * Tidak ada global state, instance tokenizer independen.
     * 
	 * @param json_content Raw JSON view.
     * @return Expected Berisi daftar token dan hint, atau Error.
     */
    [[nodiscard]] static Expected Tokenize(Raw json_content) noexcept {
        Engine engine(json_content);
        engine.execute();
        return std::move(engine).result();
    }
};

} // namespace zuu::tokenizer