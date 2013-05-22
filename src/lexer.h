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

#ifndef ARW_LEXER_H_
#define ARW_LEXER_H_

#include <string>

#include "optional.h"

namespace arw {

/**
 * This class implements a lexer for Arrow. It assumes that the input encoding
 * is UTF-32. The main input method, lex, takes input iterators, so it's
 * possible to use Lexer on UTF-8 input without converting the whole string to
 * UTF-32 using an appropriate iterator.
 *
 * Lexer does not allocate memory, and its API is fairly low level. It is
 * designed in this way in order to minimize the implementation complexity, and
 * is intended to be used along with a class that handles tokenization.
 *
 * This class does very little more than encapsulate a DFA. It is carefully
 * designed to contain as little state as possible. The rationale behind this is
 * that the implementation complexity is roughly proportional to the number of
 * states multiplied with the number of instance variables.
 */
class Lexer {
 public:
  enum class Radix {
    BINARY,
    OCTAL,
    DECIMAL,
    HEX
  };

  enum class NumberType {
    SIGNED,
    UNSIGNED,
    IMPRECISE
  };

  /**
   * POD struct representing a position in the input stream. The main purpose of
   * this class is to avoid duplication in the Receiver method parameters.
   */
  struct Position {
    uint64_t offset;
    uint64_t line;
    uint64_t column;
  };

  /**
   * Virtual base class for receiving the result of a Lexer.
   *
   * This class contains only empty method stubs; subclasses do not need to call
   * this class' method implementations when overloading them.
   */
  class Receiver {
   public:
    virtual ~Receiver() {}

    virtual void error(const Position& position,
                       const std::string& error) {}

    virtual void openBrace(const Position& position) {}
    virtual void closeBrace(const Position& position) {}
    virtual void openBracket(const Position& position) {}
    virtual void closeBracket(const Position& position) {}
    virtual void openParen(const Position& position) {}
    virtual void closeParen(const Position& position) {}
    virtual void semicolon(const Position& position) {}
    virtual void quote(const Position& position) {}
    virtual void greaterThan(const Position& position) {}
    virtual void lessThan(const Position& position) {}
    virtual void at(const Position& position) {}
    virtual void comma(const Position& position) {}
    virtual void colon(const Position& position) {}
    virtual void doubleColon(const Position& position) {}
    virtual void arrow(const Position& position) {}
    virtual void doubleArrow(const Position& position) {}

    virtual void stringBegin(const Position& position) {}
    virtual void stringEnd(const Position& position) {}
    virtual void regexBegin(const Position& position) {}
    virtual void regexEnd(const Position& position) {}
    virtual void charBegin(const Position& position) {}
    virtual void charEnd(const Position& position) {}
    virtual void commentBegin(const Position& position) {}
    virtual void commentEnd(const Position& position) {}
    virtual void numberBegin(const Position& position,
                             bool negative,
                             Radix radix) {}
    virtual void numberEnd(const Position& position,
                           const Optional<NumberType>& numberType,
                           const Optional<int>& precision) {}
    virtual void symbolBegin(const Position& position) {}
    virtual void symbolEnd(const Position& position) {}

    /**
     * A reserved token. One of $ & | \ ? ` ~ ^
     */
    virtual void reservedChar(const Position& position) {}
  };

  explicit Lexer(Receiver* recv);
  virtual ~Lexer();

  // Noncopyable
  Lexer(const Lexer&) = delete;
  Lexer& operator=(const Lexer&) = delete;

  /**
   * End of file is represented by \0.
   *
   * This method must not be called recursively; the client of this class must
   * make sure that the Receiver callbacks don't call this method.
   */
  template<typename Iter>
  void lex(Iter begin, Iter end);

 private:
  enum class State {
    /**
     * We encountered \0. Any data after this point is an error.
     */
    ENDED,
    /**
     * We have encountered data after \0 and have already reported it as an
     * error. This state is to ensure we don't do it again.
     */
    ENDED_REPORTED_ERROR,
    IN_WHITESPACE,
    /**
     * Within whitespace; got \n
     */
    IN_WHITESPACE_GOT_N,
    /**
     * Within whitespace; got \r
     */
    IN_WHITESPACE_GOT_R,
    IN_COMMENT,
    /**
     * Within comment; got \n
     */
    IN_COMMENT_GOT_N,
    /**
     * Within comment; got \r
     */
    IN_COMMENT_GOT_R,
    IN_SYMBOL,
    IN_STRING,
    /**
     * Within string; got backslash
     */
    IN_STRING_GOT_ESCAPE,
    /**
     * Within string; got \n
     */
    IN_STRING_GOT_N,
    /**
     * Within string; got \r
     */
    IN_STRING_GOT_R,
    IN_REGEX,
    /**
     * Within regex; got backslash
     */
    IN_REGEX_GOT_ESCAPE,
    /**
     * Within regex; got \n
     */
    IN_REGEX_GOT_N,
    /**
     * Within regex; got \r
     */
    IN_REGEX_GOT_R,
    IN_CHAR,
    /**
     * Within char; got backslash
     */
    IN_CHAR_GOT_ESCAPE,
    /**
     * Within char; got \n
     */
    IN_CHAR_GOT_N,
    /**
     * Within char; got \r
     */
    IN_CHAR_GOT_R,
    /**
     * Within number; may be followed by more digits, a dot (.), a number type
     * (i/u/f) or anything else (but letters are an error).
     */
    IN_NUMBER,
    /**
     * Within number; the last character was an underscore. This must be
     * followed by a digit.
     */
    IN_NUMBER_JUST_GOT_UNDERSCORE,
    /**
     * Within number; the last character was an underscore, and we have
     * previously got a dot. This must be followed by a digit.
     */
    IN_NUMBER_JUST_GOT_UNDERSCORE_GOT_DOT,
    /**
     * Within number; the last character was a dot. This must be followed by a
     * digit.
     */
    IN_NUMBER_JUST_GOT_DOT,
    /**
     * Within number; we have got a dot previously. This state is like
     * IN_NUMBER, except that a dot is an error.
     */
    IN_NUMBER_GOT_DOT,
    /**
     * Got quote; may become a regex, a char or just a quote.
     */
    GOT_QUOTE,
    /**
     * Got dash; may become arrow, a symbol or a number.
     */
    GOT_DASH,
    /**
     * Got equals; may become double arrow or a symbol
     */
    GOT_EQUALS,
    /**
     * Got colon; may become colon or double colon
     */
    GOT_COLON,
    /**
     * Got zero; may be followed by radix or become the number zero
     */
    GOT_ZERO,
    /**
     * Got dash and then zero; may be followed by radix or become the number
     * zero
     */
    GOT_ZERO_NEGATIVE,
    /**
     * Got number type (that is, the i/u/f after a number); may be followed by
     * precision or anything else (but 0 and letters are an error).
     */
    GOT_NUMBER_TYPE_I,
    GOT_NUMBER_TYPE_U,
    GOT_NUMBER_TYPE_F,
    /**
     * Got one number precision number (that is a number followed by i/u/f and
     * then one digit); may be followed by one more precision digit or anything
     * else (but 0 and letters are an error).
     */
    GOT_NUMBER_PRECISION_I_0,
    GOT_NUMBER_PRECISION_I_1,
    GOT_NUMBER_PRECISION_I_2,
    GOT_NUMBER_PRECISION_I_3,
    GOT_NUMBER_PRECISION_I_4,
    GOT_NUMBER_PRECISION_I_5,
    GOT_NUMBER_PRECISION_I_6,
    GOT_NUMBER_PRECISION_I_7,
    GOT_NUMBER_PRECISION_I_8,
    GOT_NUMBER_PRECISION_I_9,
    GOT_NUMBER_PRECISION_U_0,
    GOT_NUMBER_PRECISION_U_1,
    GOT_NUMBER_PRECISION_U_2,
    GOT_NUMBER_PRECISION_U_3,
    GOT_NUMBER_PRECISION_U_4,
    GOT_NUMBER_PRECISION_U_5,
    GOT_NUMBER_PRECISION_U_6,
    GOT_NUMBER_PRECISION_U_7,
    GOT_NUMBER_PRECISION_U_8,
    GOT_NUMBER_PRECISION_U_9,
    GOT_NUMBER_PRECISION_F_0,
    GOT_NUMBER_PRECISION_F_1,
    GOT_NUMBER_PRECISION_F_2,
    GOT_NUMBER_PRECISION_F_3,
    GOT_NUMBER_PRECISION_F_4,
    GOT_NUMBER_PRECISION_F_5,
    GOT_NUMBER_PRECISION_F_6,
    GOT_NUMBER_PRECISION_F_7,
    GOT_NUMBER_PRECISION_F_8,
    GOT_NUMBER_PRECISION_F_9,
    /**
     * Got two number precision numbers. By this point, we have already reported
     * numberEnd, and further digits are a lex error.
     */
    GOT_NUMBER_PRECISION_2,
    /**
     * Got two number precision numbers. By this point, we have already reported
     * a lex error.
     */
    GOT_NUMBER_PRECISION_3
  };

  static int numberPrecisionDigit(State state);
  static bool isNumberPrecision(State state, char32_t precision);
  void lexWhitespace(char32_t chr,
                     const Position& position,
                     char32_t gotNewline);
  void handleNewline(char32_t chr, char32_t lastNewline, State nextState);
  void lexEndOfNumber(const Optional<NumberType> type,
                      const Optional<int>& precision,
                      char32_t chr);
  void lexChar(char32_t chr);
  /**
   * The caller of this method must ensure that offset does not wrap lines; the
   * method does not handle that case.
   */
  Position makePosition(int offset = 0);

  Receiver* _recv;
  uint64_t _offset = 0;
  uint64_t _line = 1;
  uint64_t _column = 0;
  State _state = State::IN_WHITESPACE;
};

template<typename Iter>
void Lexer::lex(Iter begin, Iter end) {
  while (begin != end) {
    ++_column;

    lexChar(*begin);

    ++_offset;
    ++begin;
  }
}

}  // namespace arw

#endif  // ARW_LEXER_H_
