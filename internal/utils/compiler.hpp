/**
 * @file compiler.hpp
 * @author zuudevs (zuudevs@gmail.com)
 * @brief Compiler-specific macros for micro-architectural optimizations
 * @version 1.0.0
 * @date 2026-06-21
 *
 * @copyright Copyright (c) 2026
 */

#pragma once

#if defined(__clang__) || defined(__GNUC__)
    // Menginstruksikan CPU bahwa fungsi ini adalah jalur kritis yang sering dieksekusi
    #define ZUU_HOT __attribute__((hot))
    
    // Memaksa fungsi sejajar di memori (mencegah I-Cache shift akibat LTO/IPO)
    #define ZUU_ALIGN(x) __attribute__((aligned(x)))
#else
    // Fallback aman untuk kompilator lain (misal: MSVC)
    #define ZUU_HOT
    #define ZUU_ALIGN(x)
#endif