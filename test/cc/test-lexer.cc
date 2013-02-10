// Copyright (c) 2013, Per Eckerdal. All rights reserved.
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

#include <cstdio>

#include <deque>
#include <array>

#include "gtest/gtest.h"
#include "lexer.h"
#include "optional.h"

namespace {

typedef arw::Lexer::NumberType NT;

enum class Lexeme {
  ERROR,
  OPEN_BRACE,
  CLOSE_BRACE,
  OPEN_BRACKET,
  CLOSE_BRACKET,
  OPEN_PAREN,
  CLOSE_PAREN,
  SEMICOLON,
  QUOTE,
  GREATER_THAN,
  LESS_THAN,
  AT,
  COMMA,
  COLON,
  DOUBLE_COLON,
  ARROW,
  DOUBLE_ARROW,
  STRING_BEGIN,
  STRING_END,
  REGEX_BEGIN,
  REGEX_END,
  CHAR_BEGIN,
  CHAR_END,
  COMMENT_BEGIN,
  COMMENT_END,
  NUMBER_BEGIN,
  NUMBER_END,
  SYMBOL_BEGIN,
  SYMBOL_END,
  RESERVED_CHAR
};

struct Entry {
  Lexeme lexeme;
  uint64_t offset;
};

class ExpectErrorReceiver : public arw::Lexer::Receiver {
 public:
  ExpectErrorReceiver()
      : _encounteredErrors(0) {}
  virtual ~ExpectErrorReceiver() {}

  // Noncopyable
  ExpectErrorReceiver(const ExpectErrorReceiver&) = delete;
  ExpectErrorReceiver& operator=(const ExpectErrorReceiver&) = delete;

  int encounteredErrors() const {
    return _encounteredErrors;
  }

  virtual void error(const arw::Lexer::Position& position,
                     const std::string& error) {
    // printf(":: %s\n", error.data());
    encounteredError();
  }

  void encounteredError() {
    _encounteredErrors++;
  }

 private:
  int _encounteredErrors;
};

class TestReceiver : public ExpectErrorReceiver {
 public:
  TestReceiver() {}
  virtual ~TestReceiver() {}

  // Noncopyable
  TestReceiver(const TestReceiver&) = delete;
  TestReceiver& operator=(const TestReceiver&) = delete;

  virtual void error(const arw::Lexer::Position& position,
                     const std::string& error) {
    ExpectErrorReceiver::error(position, error);
    push(position, Lexeme::ERROR);
  }

  virtual void openBrace(const arw::Lexer::Position& position) {
    push(position, Lexeme::OPEN_BRACE);
  }

  virtual void closeBrace(const arw::Lexer::Position& position) {
    push(position, Lexeme::CLOSE_BRACE);
  }

  virtual void openBracket(const arw::Lexer::Position& position) {
    push(position, Lexeme::OPEN_BRACKET);
  }

  virtual void closeBracket(const arw::Lexer::Position& position) {
    push(position, Lexeme::CLOSE_BRACKET);
  }

  virtual void openParen(const arw::Lexer::Position& position) {
    push(position, Lexeme::OPEN_PAREN);
  }

  virtual void closeParen(const arw::Lexer::Position& position) {
    push(position, Lexeme::CLOSE_PAREN);
  }

  virtual void semicolon(const arw::Lexer::Position& position) {
    push(position, Lexeme::SEMICOLON);
  }

  virtual void quote(const arw::Lexer::Position& position) {
    push(position, Lexeme::QUOTE);
  }

  virtual void greaterThan(const arw::Lexer::Position& position) {
    push(position, Lexeme::GREATER_THAN);
  }

  virtual void lessThan(const arw::Lexer::Position& position) {
    push(position, Lexeme::LESS_THAN);
  }

  virtual void at(const arw::Lexer::Position& position) {
    push(position, Lexeme::AT);
  }

  virtual void comma(const arw::Lexer::Position& position) {
    push(position, Lexeme::COMMA);
  }

  virtual void colon(const arw::Lexer::Position& position) {
    push(position, Lexeme::COLON);
  }

  virtual void doubleColon(const arw::Lexer::Position& position) {
    push(position, Lexeme::DOUBLE_COLON);
  }

  virtual void arrow(const arw::Lexer::Position& position) {
    push(position, Lexeme::ARROW);
  }

  virtual void doubleArrow(const arw::Lexer::Position& position) {
    push(position, Lexeme::DOUBLE_ARROW);
  }

  virtual void stringBegin(const arw::Lexer::Position& position) {
    push(position, Lexeme::STRING_BEGIN);
  }

  virtual void stringEnd(const arw::Lexer::Position& position) {
    push(position, Lexeme::STRING_END);
  }

  virtual void regexBegin(const arw::Lexer::Position& position) {
    push(position, Lexeme::REGEX_BEGIN);
  }

  virtual void regexEnd(const arw::Lexer::Position& position) {
    push(position, Lexeme::REGEX_END);
  }

  virtual void charBegin(const arw::Lexer::Position& position) {
    push(position, Lexeme::CHAR_BEGIN);
  }

  virtual void charEnd(const arw::Lexer::Position& position) {
    push(position, Lexeme::CHAR_END);
  }

  virtual void commentBegin(const arw::Lexer::Position& position) {
    push(position, Lexeme::COMMENT_BEGIN);
  }

  virtual void commentEnd(const arw::Lexer::Position& position) {
    push(position, Lexeme::COMMENT_END);
  }

  virtual void numberBegin(const arw::Lexer::Position& position,
                           bool negative,
                           arw::Lexer::Radix radix) {
    push(position, Lexeme::NUMBER_BEGIN);
  }

  virtual void numberEnd(const arw::Lexer::Position& position,
                         const arw::Optional<NT>& numberType,
                         const arw::Optional<int>& precision) {
    push(position, Lexeme::NUMBER_END);
  }

  virtual void symbolBegin(const arw::Lexer::Position& position) {
    push(position, Lexeme::SYMBOL_BEGIN);
  }

  virtual void symbolEnd(const arw::Lexer::Position& position) {
    push(position, Lexeme::SYMBOL_END);
  }

  virtual void reservedChar(const arw::Lexer::Position& position) {
    push(position, Lexeme::RESERVED_CHAR);
  }

 protected:
  virtual void push(const arw::Lexer::Position& position,
                    Lexeme lexeme) = 0;
};

class ExpectPositionsReceiver : public TestReceiver {
 public:
  template<typename Iter>
  ExpectPositionsReceiver(Iter begin, Iter end)
      : _positions(begin, end) {}
  virtual ~ExpectPositionsReceiver() {}

  // Noncopyable
  ExpectPositionsReceiver(const ExpectPositionsReceiver&) = delete;
  ExpectPositionsReceiver& operator=(const ExpectPositionsReceiver&) = delete;

  size_t positionsLeft() const {
    return _positions.size();
  }

  virtual void error(const arw::Lexer::Position& position,
                     const std::string& error) {
    // We want to ignore lexer errors; we only care about positions.
  }

 protected:
  virtual void push(const arw::Lexer::Position& position,
                    Lexeme lexeme) {
    // printf("Got %d %lld\n", (int)lexeme, position.offset);

    if (_positions.empty()) {
      encounteredError();
      return;
    }

    auto p = _positions.front();
    _positions.pop_front();

#if 0
    printf(" %lld vs %lld  :  %lld vs %lld  :  %lld vs %lld\n",
           p.offset, position.offset,
           p.line,   position.line,
           p.column, position.column);
#endif

    if (p.offset != position.offset ||
        p.line   != position.line ||
        p.column != position.column) {
      encounteredError();
    }
  }

 private:
  std::deque<arw::Lexer::Position> _positions;
};

class ExpectLexemesReceiver : public TestReceiver {
 public:
  template<typename Iter>
  ExpectLexemesReceiver(Iter begin, Iter end)
      : _entries(begin, end) {}
  virtual ~ExpectLexemesReceiver() {}

  // Noncopyable
  ExpectLexemesReceiver(const ExpectLexemesReceiver&) = delete;
  ExpectLexemesReceiver& operator=(const ExpectLexemesReceiver&) = delete;

  size_t entriesLeft() const {
    return _entries.size();
  }

 protected:
  virtual void push(const arw::Lexer::Position& position,
            Lexeme lexeme) {
    // printf("Got %d %lld\n", (int)lexeme, position.offset);

    if (_entries.empty()) {
      encounteredError();
      return;
    }

    Entry e = _entries.front();
    _entries.pop_front();

    if (e.lexeme != lexeme || e.offset != position.offset) {
      encounteredError();
    }
  }

 private:
  std::deque<Entry> _entries;
};

class ExpectNumberReceiver : public ExpectLexemesReceiver {
 public:
  template<typename Iter>
  ExpectNumberReceiver(bool negative,
                       arw::Lexer::Radix radix,
                       const arw::Optional<NT>& numberType,
                       const arw::Optional<int>& precision,
                       Iter begin,
                       Iter end)
      : ExpectLexemesReceiver(begin, end),
        _negative(negative),
        _radix(radix),
        _numberType(numberType),
        _precision(precision) {}
  virtual ~ExpectNumberReceiver() {}

  // Noncopyable
  ExpectNumberReceiver(const ExpectNumberReceiver&) = delete;
  ExpectNumberReceiver& operator=(const ExpectNumberReceiver&) = delete;

  virtual void numberBegin(const arw::Lexer::Position& position,
                           bool negative,
                           arw::Lexer::Radix radix) {
    if (negative != _negative) {
      encounteredError();
    }

    if (radix != _radix) {
      encounteredError();
    }

    ExpectLexemesReceiver::numberBegin(position, negative, radix);
  }

  virtual void numberEnd(const arw::Lexer::Position& position,
                         const arw::Optional<NT>& numberType,
                         const arw::Optional<int>& precision) {
    if (numberType != _numberType) {
      encounteredError();
    }

    if (precision != _precision) {
      encounteredError();
    }

    ExpectLexemesReceiver::numberEnd(position, numberType, precision);
  }

 private:
  bool _negative;
  arw::Lexer::Radix _radix;
  arw::Optional<NT> _numberType;
  arw::Optional<int> _precision;
};

int countErrors(const char32_t* str) {
  ExpectErrorReceiver recv;
  arw::Lexer lex(&recv);

  const char32_t* strEnd = str;
  while (*strEnd++) {}
  lex.lex(str, strEnd);

  return recv.encounteredErrors();
}

template<typename Iter>
bool run(const char32_t* str, Iter begin, Iter end) {
  ExpectLexemesReceiver recv(begin, end);
  arw::Lexer lex(&recv);

  const char32_t* strEnd = str;
  while (*strEnd++) {}
  lex.lex(str, strEnd);

  return (0 == recv.entriesLeft() &&
          0 == recv.encounteredErrors());
}

bool run(const char32_t* str, std::vector<Entry> entries) {
  return run(str, entries.begin(), entries.end());
}

template<typename Iter>
bool positions(const char32_t* str, Iter begin, Iter end) {
  ExpectPositionsReceiver recv(begin, end);
  arw::Lexer lex(&recv);

  const char32_t* strEnd = str;
  while (*strEnd++) {}
  lex.lex(str, strEnd);

  return (0 == recv.positionsLeft() &&
          0 == recv.encounteredErrors());
}

bool positions(const char32_t* str, std::vector<arw::Lexer::Position> ps) {
  return positions(str, ps.begin(), ps.end());
}

template<typename Iter>
bool runNumber(const char32_t* str,
               bool negative,
               arw::Lexer::Radix radix,
               const arw::Optional<NT>& numberType,
               const arw::Optional<int>& precision,
               Iter begin,
               Iter end) {
  ExpectNumberReceiver recv(negative,
                            radix,
                            numberType,
                            precision,
                            begin,
                            end);
  arw::Lexer lex(&recv);

  const char32_t* strEnd = str;
  while (*strEnd++) {}
  lex.lex(str, strEnd);

  return (0 == recv.entriesLeft() &&
          0 == recv.encounteredErrors());
}

bool runNumber(const char32_t* str,
               bool negative,
               arw::Lexer::Radix radix,
               const arw::Optional<NT>& numberType,
               const arw::Optional<int>& precision,
               std::vector<Entry> entries) {
  return runNumber(str,
                   negative,
                   radix,
                   numberType,
                   precision,
                   entries.begin(),
                   entries.end());
}

}  // namespace

TEST(Lexer, Special) {
  EXPECT_TRUE(run(U"{}[]();'><@,:", {
    { Lexeme::OPEN_BRACE, 0 },
    { Lexeme::CLOSE_BRACE, 1 },
    { Lexeme::OPEN_BRACKET, 2 },
    { Lexeme::CLOSE_BRACKET, 3 },
    { Lexeme::OPEN_PAREN, 4 },
    { Lexeme::CLOSE_PAREN, 5 },
    { Lexeme::SEMICOLON, 6 },
    { Lexeme::QUOTE, 7 },
    { Lexeme::GREATER_THAN, 8 },
    { Lexeme::LESS_THAN, 9 },
    { Lexeme::AT, 10 },
    { Lexeme::COMMA, 11 },
    { Lexeme::COLON, 12 },
  }));
}

TEST(Lexer, Symbol) {
  EXPECT_TRUE(run(U"a", {
    { Lexeme::SYMBOL_BEGIN, 0 },
    { Lexeme::SYMBOL_END, 1 },
  }));

  EXPECT_TRUE(run(U"a1", {
    { Lexeme::SYMBOL_BEGIN, 0 },
    { Lexeme::SYMBOL_END, 2 },
  }));

  EXPECT_TRUE(run(U"+", {
    { Lexeme::SYMBOL_BEGIN, 0 },
    { Lexeme::SYMBOL_END, 1 },
  }));

  EXPECT_TRUE(run(U"-", {
    { Lexeme::SYMBOL_BEGIN, 0 },
    { Lexeme::SYMBOL_END, 1 },
  }));

  EXPECT_TRUE(run(U"--", {
    { Lexeme::SYMBOL_BEGIN, 0 },
    { Lexeme::SYMBOL_END, 2 },
  }));

  EXPECT_TRUE(run(U"a;a", {
    { Lexeme::SYMBOL_BEGIN, 0 },
    { Lexeme::SYMBOL_END, 1 },
    { Lexeme::SEMICOLON, 1 },
    { Lexeme::SYMBOL_BEGIN, 2 },
    { Lexeme::SYMBOL_END, 3 },
  }));
}

TEST(Lexer, Colon) {
  EXPECT_TRUE(run(U":", {
    { Lexeme::COLON, 0 },
  }));

  EXPECT_TRUE(run(U"::", {
    { Lexeme::DOUBLE_COLON, 0 },
  }));

  EXPECT_TRUE(run(U"::::", {
    { Lexeme::DOUBLE_COLON, 0 },
    { Lexeme::DOUBLE_COLON, 2 },
  }));

  EXPECT_TRUE(run(U":::::", {
    { Lexeme::DOUBLE_COLON, 0 },
    { Lexeme::DOUBLE_COLON, 2 },
    { Lexeme::COLON, 4 },
  }));

  EXPECT_TRUE(run(U"a:a", {
    { Lexeme::SYMBOL_BEGIN, 0 },
    { Lexeme::SYMBOL_END, 1 },
    { Lexeme::COLON, 1 },
    { Lexeme::SYMBOL_BEGIN, 2 },
    { Lexeme::SYMBOL_END, 3 },
  }));

  EXPECT_TRUE(run(U"a::a", {
    { Lexeme::SYMBOL_BEGIN, 0 },
    { Lexeme::SYMBOL_END, 1 },
    { Lexeme::DOUBLE_COLON, 1 },
    { Lexeme::SYMBOL_BEGIN, 3 },
    { Lexeme::SYMBOL_END, 4 },
  }));
}

TEST(Lexer, ReservedChars) {
  // Reserved chars by themselves
  EXPECT_EQ(1, countErrors(U"$"));
  EXPECT_EQ(1, countErrors(U"&"));
  EXPECT_EQ(1, countErrors(U"|"));
  EXPECT_EQ(1, countErrors(U"\\"));
  EXPECT_EQ(1, countErrors(U"?"));
  EXPECT_EQ(1, countErrors(U"`"));
  EXPECT_EQ(1, countErrors(U"~"));
  EXPECT_EQ(1, countErrors(U"^"));

  // Reserved chars immediately after symbol
  EXPECT_EQ(1, countErrors(U"a$"));
  EXPECT_EQ(1, countErrors(U"a&"));
  EXPECT_EQ(1, countErrors(U"a|"));
  EXPECT_EQ(1, countErrors(U"a\\"));
  EXPECT_EQ(1, countErrors(U"a?"));
  EXPECT_EQ(1, countErrors(U"a`"));
  EXPECT_EQ(1, countErrors(U"a~"));
  EXPECT_EQ(1, countErrors(U"a^"));
}

TEST(Lexer, Whitespace) {
  EXPECT_TRUE(run(U"", {}));
  EXPECT_TRUE(run(U" ", {}));
  EXPECT_TRUE(run(U"\n", {}));
  EXPECT_TRUE(run(U"\r", {}));
  EXPECT_TRUE(run(U"\r\n", {}));
  EXPECT_TRUE(run(U"\n\r", {}));
  EXPECT_EQ(1, countErrors(U"\t"));

  EXPECT_TRUE(run(U" @ ", {
    { Lexeme::AT, 1 },
  }));

  EXPECT_TRUE(run(U" a ", {
    { Lexeme::SYMBOL_BEGIN, 1 },
    { Lexeme::SYMBOL_END, 2 },
  }));

  EXPECT_TRUE(run(U" a a ", {
    { Lexeme::SYMBOL_BEGIN, 1 },
    { Lexeme::SYMBOL_END, 2 },
    { Lexeme::SYMBOL_BEGIN, 3 },
    { Lexeme::SYMBOL_END, 4 },
  }));
}

TEST(Lexer, String) {
  EXPECT_TRUE(run(U"\"\"", {
    { Lexeme::STRING_BEGIN, 0 },
    { Lexeme::STRING_END, 1 },
  }));

  EXPECT_TRUE(run(U"\" \"", {
    { Lexeme::STRING_BEGIN, 0 },
    { Lexeme::STRING_END, 2 },
  }));

  EXPECT_TRUE(run(U"\"\\\\\"", {
    { Lexeme::STRING_BEGIN, 0 },
    { Lexeme::STRING_END, 3 },
  }));

  EXPECT_TRUE(run(U"\"\\\\ \"", {
    { Lexeme::STRING_BEGIN, 0 },
    { Lexeme::STRING_END, 4 },
  }));

  EXPECT_TRUE(run(U"\" \\\\\"", {
    { Lexeme::STRING_BEGIN, 0 },
    { Lexeme::STRING_END, 4 },
  }));

  EXPECT_EQ(1, countErrors(U"\"\n\""));
  EXPECT_EQ(1, countErrors(U"\"\r\""));
  EXPECT_EQ(1, countErrors(U"\"\n\r\""));
  EXPECT_EQ(1, countErrors(U"\"\r\n\""));
  EXPECT_EQ(1, countErrors(U"\""));
  EXPECT_EQ(2, countErrors(U"\"\n"));
  EXPECT_EQ(2, countErrors(U"\"\r"));
  EXPECT_EQ(2, countErrors(U"\"\n\r"));
  EXPECT_EQ(2, countErrors(U"\"\r\n"));
}

TEST(Lexer, Regex) {
  EXPECT_TRUE(run(U"'//", {
    { Lexeme::REGEX_BEGIN, 0 },
    { Lexeme::REGEX_END, 2 },
  }));

  EXPECT_TRUE(run(U"'/ /", {
    { Lexeme::REGEX_BEGIN, 0 },
    { Lexeme::REGEX_END, 3 },
  }));

  EXPECT_TRUE(run(U"'/\\\\/", {
    { Lexeme::REGEX_BEGIN, 0 },
    { Lexeme::REGEX_END, 4 },
  }));

  EXPECT_TRUE(run(U"'/\\\\ /", {
    { Lexeme::REGEX_BEGIN, 0 },
    { Lexeme::REGEX_END, 5 },
  }));

  EXPECT_TRUE(run(U"'/ \\\\/", {
    { Lexeme::REGEX_BEGIN, 0 },
    { Lexeme::REGEX_END, 5 },
  }));

  EXPECT_EQ(1, countErrors(U"'/\n/"));
  EXPECT_EQ(1, countErrors(U"'/\r/"));
  EXPECT_EQ(1, countErrors(U"'/\n\r/"));
  EXPECT_EQ(1, countErrors(U"'/\r\n/"));
  EXPECT_EQ(1, countErrors(U"'/"));
  EXPECT_EQ(2, countErrors(U"'/\n"));
  EXPECT_EQ(2, countErrors(U"'/\r"));
  EXPECT_EQ(2, countErrors(U"'/\n\r"));
  EXPECT_EQ(2, countErrors(U"'/\r\n"));
}

TEST(Lexer, Comment) {
  EXPECT_TRUE(run(U"#", {
    { Lexeme::COMMENT_BEGIN, 0 },
    { Lexeme::COMMENT_END, 1 },
  }));

  EXPECT_TRUE(run(U"##", {
    { Lexeme::COMMENT_BEGIN, 0 },
    { Lexeme::COMMENT_END, 2 },
  }));

  EXPECT_TRUE(run(U"# ", {
    { Lexeme::COMMENT_BEGIN, 0 },
    { Lexeme::COMMENT_END, 2 },
  }));

  EXPECT_TRUE(run(U" #", {
    { Lexeme::COMMENT_BEGIN, 1 },
    { Lexeme::COMMENT_END, 2 },
  }));

  EXPECT_TRUE(run(U"a#", {
    { Lexeme::SYMBOL_BEGIN, 0 },
    { Lexeme::SYMBOL_END, 1 },
    { Lexeme::COMMENT_BEGIN, 1 },
    { Lexeme::COMMENT_END, 2 },
  }));

  EXPECT_TRUE(run(U"#\n", {
    { Lexeme::COMMENT_BEGIN, 0 },
    { Lexeme::COMMENT_END, 1 },
  }));

  EXPECT_TRUE(run(U"#\r", {
    { Lexeme::COMMENT_BEGIN, 0 },
    { Lexeme::COMMENT_END, 1 },
  }));

  EXPECT_TRUE(run(U"#\n\ra", {
    { Lexeme::COMMENT_BEGIN, 0 },
    { Lexeme::COMMENT_END, 1 },
    { Lexeme::SYMBOL_BEGIN, 3 },
    { Lexeme::SYMBOL_END, 4 },
  }));

  EXPECT_TRUE(run(U"#\r\na", {
    { Lexeme::COMMENT_BEGIN, 0 },
    { Lexeme::COMMENT_END, 1 },
    { Lexeme::SYMBOL_BEGIN, 3 },
    { Lexeme::SYMBOL_END, 4 },
  }));
}

TEST(Lexer, NumberRadix) {
  EXPECT_TRUE(runNumber(U"1",
                        /*negative:*/false,
                        arw::Lexer::Radix::DECIMAL,
                        arw::Optional<NT>(),
                        /*precision:*/arw::Optional<int>(), {
                          { Lexeme::NUMBER_BEGIN, 0 },
                          { Lexeme::NUMBER_END, 1 },
                        }));

  EXPECT_TRUE(runNumber(U"11",
                        /*negative:*/false,
                        arw::Lexer::Radix::DECIMAL,
                        arw::Optional<NT>(),
                        /*precision:*/arw::Optional<int>(), {
                          { Lexeme::NUMBER_BEGIN, 0 },
                          { Lexeme::NUMBER_END, 2 },
                        }));

  EXPECT_TRUE(runNumber(U"0",
                        /*negative:*/false,
                        arw::Lexer::Radix::DECIMAL,
                        arw::Optional<NT>(),
                        /*precision:*/arw::Optional<int>(), {
                          { Lexeme::NUMBER_BEGIN, 0 },
                          { Lexeme::NUMBER_END, 1 },
                        }));


  EXPECT_TRUE(runNumber(U"0x0",
                        /*negative:*/false,
                        arw::Lexer::Radix::HEX,
                        arw::Optional<NT>(),
                        /*precision:*/arw::Optional<int>(), {
                          { Lexeme::NUMBER_BEGIN, 0 },
                          { Lexeme::NUMBER_END, 3 },
                        }));

  EXPECT_TRUE(runNumber(U"0X0",
                        /*negative:*/false,
                        arw::Lexer::Radix::HEX,
                        arw::Optional<NT>(),
                        /*precision:*/arw::Optional<int>(), {
                          { Lexeme::NUMBER_BEGIN, 0 },
                          { Lexeme::NUMBER_END, 3 },
                        }));

  EXPECT_TRUE(runNumber(U"0b0",
                        /*negative:*/false,
                        arw::Lexer::Radix::BINARY,
                        arw::Optional<NT>(),
                        /*precision:*/arw::Optional<int>(), {
                          { Lexeme::NUMBER_BEGIN, 0 },
                          { Lexeme::NUMBER_END, 3 },
                        }));

  EXPECT_TRUE(runNumber(U"0B0",
                        /*negative:*/false,
                        arw::Lexer::Radix::BINARY,
                        arw::Optional<NT>(),
                        /*precision:*/arw::Optional<int>(), {
                          { Lexeme::NUMBER_BEGIN, 0 },
                          { Lexeme::NUMBER_END, 3 },
                        }));

  EXPECT_TRUE(runNumber(U"0o0",
                        /*negative:*/false,
                        arw::Lexer::Radix::OCTAL,
                        arw::Optional<NT>(),
                        /*precision:*/arw::Optional<int>(), {
                          { Lexeme::NUMBER_BEGIN, 0 },
                          { Lexeme::NUMBER_END, 3 },
                        }));

  EXPECT_TRUE(runNumber(U"0O0",
                        /*negative:*/false,
                        arw::Lexer::Radix::OCTAL,
                        arw::Optional<NT>(),
                        /*precision:*/arw::Optional<int>(), {
                          { Lexeme::NUMBER_BEGIN, 0 },
                          { Lexeme::NUMBER_END, 3 },
                        }));

  EXPECT_TRUE(runNumber(U"00",
                        /*negative:*/false,
                        arw::Lexer::Radix::OCTAL,
                        arw::Optional<NT>(),
                        /*precision:*/arw::Optional<int>(), {
                          { Lexeme::NUMBER_BEGIN, 0 },
                          { Lexeme::NUMBER_END, 2 },
                        }));


  EXPECT_EQ(1, countErrors(U"0a"));
  EXPECT_EQ(1, countErrors(U"1a"));
  EXPECT_EQ(1, countErrors(U"0x0a"));
  EXPECT_EQ(1, countErrors(U"0x1a"));
  EXPECT_EQ(1, countErrors(U"0xa"));
}


TEST(Lexer, NumberNegative) {
  EXPECT_TRUE(runNumber(U"-1",
                        /*negative:*/true,
                        arw::Lexer::Radix::DECIMAL,
                        arw::Optional<NT>(),
                        /*precision:*/arw::Optional<int>(), {
                          { Lexeme::NUMBER_BEGIN, 0 },
                          { Lexeme::NUMBER_END, 2 },
                        }));

  EXPECT_TRUE(runNumber(U"-0",
                        /*negative:*/true,
                        arw::Lexer::Radix::DECIMAL,
                        arw::Optional<NT>(),
                        /*precision:*/arw::Optional<int>(), {
                          { Lexeme::NUMBER_BEGIN, 0 },
                          { Lexeme::NUMBER_END, 2 },
                        }));

  EXPECT_TRUE(runNumber(U"-11",
                        /*negative:*/true,
                        arw::Lexer::Radix::DECIMAL,
                        arw::Optional<NT>(),
                        /*precision:*/arw::Optional<int>(), {
                          { Lexeme::NUMBER_BEGIN, 0 },
                          { Lexeme::NUMBER_END, 3 },
                        }));

  EXPECT_TRUE(runNumber(U"-0x0",
                        /*negative:*/true,
                        arw::Lexer::Radix::HEX,
                        arw::Optional<NT>(),
                        /*precision:*/arw::Optional<int>(), {
                          { Lexeme::NUMBER_BEGIN, 0 },
                          { Lexeme::NUMBER_END, 4 },
                        }));

  EXPECT_TRUE(runNumber(U"-0X0",
                        /*negative:*/true,
                        arw::Lexer::Radix::HEX,
                        arw::Optional<NT>(),
                        /*precision:*/arw::Optional<int>(), {
                          { Lexeme::NUMBER_BEGIN, 0 },
                          { Lexeme::NUMBER_END, 4 },
                        }));

  EXPECT_TRUE(runNumber(U"-0b0",
                        /*negative:*/true,
                        arw::Lexer::Radix::BINARY,
                        arw::Optional<NT>(),
                        /*precision:*/arw::Optional<int>(), {
                          { Lexeme::NUMBER_BEGIN, 0 },
                          { Lexeme::NUMBER_END, 4 },
                        }));

  EXPECT_TRUE(runNumber(U"-0B0",
                        /*negative:*/true,
                        arw::Lexer::Radix::BINARY,
                        arw::Optional<NT>(),
                        /*precision:*/arw::Optional<int>(), {
                          { Lexeme::NUMBER_BEGIN, 0 },
                          { Lexeme::NUMBER_END, 4 },
                        }));

  EXPECT_TRUE(runNumber(U"-0o0",
                        /*negative:*/true,
                        arw::Lexer::Radix::OCTAL,
                        arw::Optional<NT>(),
                        /*precision:*/arw::Optional<int>(), {
                          { Lexeme::NUMBER_BEGIN, 0 },
                          { Lexeme::NUMBER_END, 4 },
                        }));

  EXPECT_TRUE(runNumber(U"-0O0",
                        /*negative:*/true,
                        arw::Lexer::Radix::OCTAL,
                        arw::Optional<NT>(),
                        /*precision:*/arw::Optional<int>(), {
                          { Lexeme::NUMBER_BEGIN, 0 },
                          { Lexeme::NUMBER_END, 4 },
                        }));

  EXPECT_TRUE(runNumber(U"-00",
                        /*negative:*/true,
                        arw::Lexer::Radix::OCTAL,
                        arw::Optional<NT>(),
                        /*precision:*/arw::Optional<int>(), {
                          { Lexeme::NUMBER_BEGIN, 0 },
                          { Lexeme::NUMBER_END, 3 },
                        }));
}


TEST(Lexer, NumberType) {
  EXPECT_TRUE(runNumber(U"0i",
                        /*negative:*/false,
                        arw::Lexer::Radix::DECIMAL,
                        arw::Optional<NT>(NT::SIGNED),
                        /*precision:*/arw::Optional<int>(), {
                          { Lexeme::NUMBER_BEGIN, 0 },
                          { Lexeme::NUMBER_END, 2 },
                        }));

  EXPECT_TRUE(runNumber(U"0u",
                        /*negative:*/false,
                        arw::Lexer::Radix::DECIMAL,
                        arw::Optional<NT>(NT::UNSIGNED),
                        /*precision:*/arw::Optional<int>(), {
                          { Lexeme::NUMBER_BEGIN, 0 },
                          { Lexeme::NUMBER_END, 2 },
                        }));

  EXPECT_TRUE(runNumber(U"0f",
                        /*negative:*/false,
                        arw::Lexer::Radix::DECIMAL,
                        arw::Optional<NT>(NT::IMPRECISE),
                        /*precision:*/arw::Optional<int>(), {
                          { Lexeme::NUMBER_BEGIN, 0 },
                          { Lexeme::NUMBER_END, 2 },
                        }));

  EXPECT_TRUE(runNumber(U"1i",
                        /*negative:*/false,
                        arw::Lexer::Radix::DECIMAL,
                        arw::Optional<NT>(NT::SIGNED),
                        /*precision:*/arw::Optional<int>(), {
                          { Lexeme::NUMBER_BEGIN, 0 },
                          { Lexeme::NUMBER_END, 2 },
                        }));

  EXPECT_TRUE(runNumber(U"1u",
                        /*negative:*/false,
                        arw::Lexer::Radix::DECIMAL,
                        arw::Optional<NT>(NT::UNSIGNED),
                        /*precision:*/arw::Optional<int>(), {
                          { Lexeme::NUMBER_BEGIN, 0 },
                          { Lexeme::NUMBER_END, 2 },
                        }));

  EXPECT_TRUE(runNumber(U"1f",
                        /*negative:*/false,
                        arw::Lexer::Radix::DECIMAL,
                        arw::Optional<NT>(NT::IMPRECISE),
                        /*precision:*/arw::Optional<int>(), {
                          { Lexeme::NUMBER_BEGIN, 0 },
                          { Lexeme::NUMBER_END, 2 },
                        }));

  EXPECT_TRUE(runNumber(U"-0i",
                        /*negative:*/true,
                        arw::Lexer::Radix::DECIMAL,
                        arw::Optional<NT>(NT::SIGNED),
                        /*precision:*/arw::Optional<int>(), {
                          { Lexeme::NUMBER_BEGIN, 0 },
                          { Lexeme::NUMBER_END, 3 },
                        }));

  EXPECT_TRUE(runNumber(U"-0u",
                        /*negative:*/true,
                        arw::Lexer::Radix::DECIMAL,
                        arw::Optional<NT>(NT::UNSIGNED),
                        /*precision:*/arw::Optional<int>(), {
                          { Lexeme::NUMBER_BEGIN, 0 },
                          { Lexeme::NUMBER_END, 3 },
                        }));

  EXPECT_TRUE(runNumber(U"-0f",
                        /*negative:*/true,
                        arw::Lexer::Radix::DECIMAL,
                        arw::Optional<NT>(NT::IMPRECISE),
                        /*precision:*/arw::Optional<int>(), {
                          { Lexeme::NUMBER_BEGIN, 0 },
                          { Lexeme::NUMBER_END, 3 },
                        }));


  EXPECT_TRUE(runNumber(U"0i1",
                        /*negative:*/false,
                        arw::Lexer::Radix::DECIMAL,
                        arw::Optional<NT>(NT::SIGNED),
                        /*precision:*/arw::Optional<int>(1), {
                          { Lexeme::NUMBER_BEGIN, 0 },
                          { Lexeme::NUMBER_END, 3 },
                        }));

  EXPECT_TRUE(runNumber(U"0i11",
                        /*negative:*/false,
                        arw::Lexer::Radix::DECIMAL,
                        arw::Optional<NT>(NT::SIGNED),
                        /*precision:*/arw::Optional<int>(11), {
                          { Lexeme::NUMBER_BEGIN, 0 },
                          { Lexeme::NUMBER_END, 4 },
                        }));

  EXPECT_EQ(1, countErrors(U"0i0"));
  EXPECT_EQ(1, countErrors(U"0i111"));
  EXPECT_EQ(1, countErrors(U"0i1a"));
  EXPECT_EQ(2, countErrors(U"0i0a"));
}

TEST(Lexer, NumberDecimal) {
  EXPECT_TRUE(runNumber(U"1.1",
                        /*negative:*/false,
                        arw::Lexer::Radix::DECIMAL,
                        arw::Optional<NT>(),
                        /*precision:*/arw::Optional<int>(), {
                          { Lexeme::NUMBER_BEGIN, 0 },
                          { Lexeme::NUMBER_END, 3 },
                        }));

  EXPECT_TRUE(runNumber(U"0.1",
                        /*negative:*/false,
                        arw::Lexer::Radix::DECIMAL,
                        arw::Optional<NT>(),
                        /*precision:*/arw::Optional<int>(), {
                          { Lexeme::NUMBER_BEGIN, 0 },
                          { Lexeme::NUMBER_END, 3 },
                        }));

  EXPECT_TRUE(runNumber(U"1.1f",
                        /*negative:*/false,
                        arw::Lexer::Radix::DECIMAL,
                        arw::Optional<NT>(NT::IMPRECISE),
                        /*precision:*/arw::Optional<int>(), {
                          { Lexeme::NUMBER_BEGIN, 0 },
                          { Lexeme::NUMBER_END, 4 },
                        }));

  EXPECT_EQ(1, countErrors(U"1."));
  EXPECT_EQ(1, countErrors(U"0."));
  EXPECT_EQ(1, countErrors(U".1"));
  EXPECT_EQ(1, countErrors(U"1.1.1"));
  EXPECT_EQ(1, countErrors(U"."));
  EXPECT_EQ(1, countErrors(U"1.f"));
}

TEST(Lexer, Char) {
  EXPECT_TRUE(run(U"''a", {
    { Lexeme::CHAR_BEGIN, 0 },
    { Lexeme::CHAR_END, 3 },
  }));

  EXPECT_TRUE(run(U"''a a", {
    { Lexeme::CHAR_BEGIN, 0 },
    { Lexeme::CHAR_END, 3 },
    { Lexeme::SYMBOL_BEGIN, 4 },
    { Lexeme::SYMBOL_END, 5 },
  }));

  EXPECT_TRUE(run(U"''\\\\", {
    { Lexeme::CHAR_BEGIN, 0 },
    { Lexeme::CHAR_END, 4 },
  }));

  EXPECT_TRUE(run(U"''\\\\ a", {
    { Lexeme::CHAR_BEGIN, 0 },
    { Lexeme::CHAR_END, 4 },
    { Lexeme::SYMBOL_BEGIN, 5 },
    { Lexeme::SYMBOL_END, 6 },
  }));

  EXPECT_EQ(1, countErrors(U"''\n"));
  EXPECT_EQ(1, countErrors(U"''\r"));
  EXPECT_EQ(1, countErrors(U"''\n\r"));
  EXPECT_EQ(1, countErrors(U"''\r\n"));
}

TEST(Lexer, Position) {
  EXPECT_TRUE(positions(U"", {}));

  EXPECT_TRUE(positions(U"a", {
    { 0, 1, 1 },
    { 1, 1, 2 },
  }));

  EXPECT_TRUE(positions(U"a a", {
    { 0, 1, 1 },
    { 1, 1, 2 },
    { 2, 1, 3 },
    { 3, 1, 4 },
  }));

  EXPECT_TRUE(positions(U"(a)", {
    { 0, 1, 1 },
    { 1, 1, 2 },
    { 2, 1, 3 },
    { 2, 1, 3 },
  }));


  EXPECT_TRUE(positions(U"a\na", {
    { 0, 1, 1 },
    { 1, 1, 2 },
    { 2, 2, 1 },
    { 3, 2, 2 },
  }));

  EXPECT_TRUE(positions(U"a\ra", {
    { 0, 1, 1 },
    { 1, 1, 2 },
    { 2, 2, 1 },
    { 3, 2, 2 },
  }));

  EXPECT_TRUE(positions(U"a\n\ra", {
    { 0, 1, 1 },
    { 1, 1, 2 },
    { 3, 2, 1 },
    { 4, 2, 2 },
  }));

  EXPECT_TRUE(positions(U"a\r\na", {
    { 0, 1, 1 },
    { 1, 1, 2 },
    { 3, 2, 1 },
    { 4, 2, 2 },
  }));


  EXPECT_TRUE(positions(U"#\na", {
    { 0, 1, 1 },
    { 1, 1, 2 },
    { 2, 2, 1 },
    { 3, 2, 2 },
  }));

  EXPECT_TRUE(positions(U"#\ra", {
    { 0, 1, 1 },
    { 1, 1, 2 },
    { 2, 2, 1 },
    { 3, 2, 2 },
  }));

  EXPECT_TRUE(positions(U"#\n\ra", {
    { 0, 1, 1 },
    { 1, 1, 2 },
    { 3, 2, 1 },
    { 4, 2, 2 },
  }));

  EXPECT_TRUE(positions(U"#\r\na", {
    { 0, 1, 1 },
    { 1, 1, 2 },
    { 3, 2, 1 },
    { 4, 2, 2 },
  }));

  EXPECT_TRUE(positions(U"#\r\na", {
    { 0, 1, 1 },
    { 1, 1, 2 },
    { 3, 2, 1 },
    { 4, 2, 2 },
  }));


  EXPECT_TRUE(positions(U"\"\n\"", {
    { 0, 1, 1 },
    { 2, 2, 1 },
  }));

  EXPECT_TRUE(positions(U"\"\r\"", {
    { 0, 1, 1 },
    { 2, 2, 1 },
  }));

  EXPECT_TRUE(positions(U"\"\n\r\"", {
    { 0, 1, 1 },
    { 3, 2, 1 },
  }));

  EXPECT_TRUE(positions(U"\"\r\n\"", {
    { 0, 1, 1 },
    { 3, 2, 1 },
  }));

  EXPECT_TRUE(positions(U"\"\\\n\"", {
    { 0, 1, 1 },
    { 3, 2, 1 },
  }));

  EXPECT_TRUE(positions(U"\"\\\r\"", {
    { 0, 1, 1 },
    { 3, 2, 1 },
  }));

  EXPECT_TRUE(positions(U"\"\\\n\r\"", {
    { 0, 1, 1 },
    { 4, 2, 1 },
  }));

  EXPECT_TRUE(positions(U"\"\\\r\n\"", {
    { 0, 1, 1 },
    { 4, 2, 1 },
  }));


  EXPECT_TRUE(positions(U"'/\n/", {
    { 0, 1, 1 },
    { 3, 2, 1 },
  }));

  EXPECT_TRUE(positions(U"'/\r/", {
    { 0, 1, 1 },
    { 3, 2, 1 },
  }));

  EXPECT_TRUE(positions(U"'/\n\r/", {
    { 0, 1, 1 },
    { 4, 2, 1 },
  }));

  EXPECT_TRUE(positions(U"'/\r\n/", {
    { 0, 1, 1 },
    { 4, 2, 1 },
  }));

  EXPECT_TRUE(positions(U"'/\\\n/", {
    { 0, 1, 1 },
    { 4, 2, 1 },
  }));

  EXPECT_TRUE(positions(U"'/\\\r/", {
    { 0, 1, 1 },
    { 4, 2, 1 },
  }));

  EXPECT_TRUE(positions(U"'/\\\n\r/", {
    { 0, 1, 1 },
    { 5, 2, 1 },
  }));

  EXPECT_TRUE(positions(U"'/\\\r\n/", {
    { 0, 1, 1 },
    { 5, 2, 1 },
  }));


  EXPECT_TRUE(positions(U"''\na", {
    { 0, 1, 1 },
    { 4, 2, 2 },
  }));

  EXPECT_TRUE(positions(U"''\ra", {
    { 0, 1, 1 },
    { 4, 2, 2 },
  }));

  EXPECT_TRUE(positions(U"''\n\ra", {
    { 0, 1, 1 },
    { 5, 2, 2 },
  }));

  EXPECT_TRUE(positions(U"''\r\na", {
    { 0, 1, 1 },
    { 5, 2, 2 },
  }));

  EXPECT_TRUE(positions(U"''\\\na", {
    { 0, 1, 1 },
    { 5, 2, 2 },
  }));

  EXPECT_TRUE(positions(U"''\\\ra", {
    { 0, 1, 1 },
    { 5, 2, 2 },
  }));

  EXPECT_TRUE(positions(U"''\\\n\ra", {
    { 0, 1, 1 },
    { 6, 2, 2 },
  }));

  EXPECT_TRUE(positions(U"''\\\r\na", {
    { 0, 1, 1 },
    { 6, 2, 2 },
  }));
}
