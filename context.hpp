#pragma once

#include <asmjit/asmjit.h>
#include <cstddef>
#include <iostream>
#include <memory>
#include <vector>

namespace hayai {
using namespace asmjit;

class Handler : public ErrorHandler {
public:
  void handleError(Error, const char *message, BaseEmitter *) override {
    std::cerr << message << '\n';
  }
};
class Context {
public:
  using compiler_type = x86::Compiler;

private:
  JitRuntime m_jit;
  CodeHolder m_code;
  StringLogger m_logger;
  Handler m_handler;

  x86::Mem m_stack, m_idx;
  x86::Gp m_off;

  std::vector<std::pair<Label, Label>> m_loops;

  std::uint32_t m_size;

  void (*m_main)() = nullptr;

  static inline auto s_putchar(char c) { std::cout << c; }
  static inline auto s_getchar() { return std::cin.get(); }

public:
  Context(std::uint32_t size) : m_size(size) {
    m_code.init(m_jit.environment(), m_jit.cpuFeatures());
    m_code.setLogger(&m_logger);
    m_code.setErrorHandler(&m_handler);
  }

  const char *logs() const noexcept { return m_logger.data(); }

  template <typename Iterator = std::istreambuf_iterator<char>>
  Error compile(Iterator first, const Iterator &last) {
    compiler_type cc(&m_code);

    cc.addFunc(FuncSignatureT<void>());

    // m_stack = cc.newStack(m_size, 1);
    m_stack = cc.newStack(m_size, sizeof(std::max_align_t));

    m_off = cc.newGpz();

    cc.xor_(m_off, m_off);

    m_idx = m_stack;
    m_idx.setIndex(m_off);
    m_idx.setSize(1);

    static constexpr auto count_fn = [](Iterator &first, const Iterator &last,
                                        char c) {
      std::size_t count = 0;

      while (first != last && *first == c)
        ++first, ++count;

      return count;
    };
    
    while (first != last) {

#define COUNT(c) (count_fn(first, last, c))

      switch (*first) {
      case '>': {
        cc.add(m_off, COUNT('>'));
        continue;
      }
      case '<': {
        cc.sub(m_off, COUNT('<'));
        continue;
      }
      case '+': {
        cc.add(m_idx, COUNT('+'));
        continue;
      }
      case '-': {
        cc.sub(m_idx, COUNT('-'));
        continue;
      }
      case '.': {
        const auto tmp = cc.newGpb();
        cc.mov(tmp,
               m_idx); // move stack[off] (which is in memory) to a register
        InvokeNode *node;
        cc.invoke(&node, imm(&s_putchar), FuncSignatureT<void, char>());
        node->setArg(0, tmp);
        break;
      }
      case ',': {
        InvokeNode *node;
        cc.invoke(&node, imm(&s_getchar), FuncSignatureT<char>());
        const auto tmp = cc.newGpb();
        node->setRet(0, tmp);
        cc.mov(m_idx, tmp);
        break;
      }
      case '[': {
        m_loops.push_back(std::make_pair(cc.newLabel(), cc.newLabel()));

        cc.bind(m_loops.back().first);
        cc.cmp(m_idx, 0);
        cc.je(m_loops.back().second);
        break;
      }
      case ']': {
        if (m_loops.empty())
          throw std::runtime_error("unmatched bracket found");

        cc.jmp(m_loops.back().first);
        cc.bind(m_loops.back().second);

        m_loops.pop_back();
        break;
      }
#undef COUNT
      }
      ++first;
    }

    cc.endFunc();
    cc.finalize();
    
    return m_jit.add(&m_main, &m_code);
  }

  void run() {
    if (m_main)
      m_main();
  }
};
} // namespace hayai

// namespace hayai