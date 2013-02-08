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

#include "lexer.h"
#include "checks.h"

namespace arw {

static bool isLetter(char32_t chr) {
  return (('a' <= chr && 'z' >= chr) ||
          ('A' <= chr && 'Z' >= chr));
}

static bool isDigit(char32_t chr) {
  return '0' <= chr && '9' >= chr;
}

static bool isSymbolChar(char32_t chr) {
  return ('_' == chr ||
          '-' == chr ||
          '+' == chr ||
          '!' == chr ||
          '*' == chr ||
          '%' == chr ||
          isLetter(chr) ||
          isDigit(chr) ||
          '/' == chr);
}

static bool isReservedChar(char32_t chr) {
  return ('$' == chr ||
          '&' == chr ||
          '|' == chr ||
          '\\' == chr ||
          '?' == chr ||
          '`' == chr ||
          '~' == chr ||
          '^' == chr);
}

Lexer::Lexer(Receiver* recv) : _recv(recv) {
  ARW_CHECK(recv);
}

Lexer::~Lexer() {}

Lexer::Position Lexer::makePosition(int offset) {
  ARW_ASSERT(_column >= -offset);
  return { _offset+offset, _line, _column+offset };
}

int Lexer::numberPrecisionDigit(State state) {
  static const State states[] = {
    State::GOT_NUMBER_PRECISION_I_0,
    State::GOT_NUMBER_PRECISION_U_0,
    State::GOT_NUMBER_PRECISION_F_0
  };

  for (int i = 0; i < sizeof(states)/sizeof(*states); i++) {
    int baseState = static_cast<int>(states[i]);
    int stateInt = static_cast<int>(state);
    if (baseState <= stateInt && baseState+9 >= stateInt) {
      return stateInt-baseState;
    }
  }

  ARW_ASSERT(0);
  return -1;
}

bool Lexer::isNumberPrecision(State state, char32_t precision) {
  int baseState;
  if ('i' == precision) {
    baseState = static_cast<int>(State::GOT_NUMBER_PRECISION_I_0);
  } else if ('u' == precision) {
    baseState = static_cast<int>(State::GOT_NUMBER_PRECISION_U_0);
  } else if ('f' == precision) {
    baseState = static_cast<int>(State::GOT_NUMBER_PRECISION_F_0);
  } else {
    ARW_ASSERT(0);
    baseState = static_cast<int>(State::GOT_NUMBER_PRECISION_I_0);
  }

  for (int i = 0; i < 10; i++) {
    if (static_cast<State>(baseState+i) == state) {
      return true;
    }
  }

  return false;
}

void Lexer::lexWhitespace(char32_t chr,
                          const Position& position,
                          char32_t gotNewline) {
  if (('\r' == gotNewline && '\n' == chr) ||
      ('\n' == gotNewline && '\r' == chr) ||
      0 != gotNewline) {
  }
}

void Lexer::handleNewline(char32_t chr,
                          char32_t lastNewline,
                          Lexer::State nextState) {
  _line++;
  _column = 0;
  _state = nextState;

  if (('\n' == lastNewline && '\r' != chr) ||
      ('\r' == lastNewline && '\n' != chr)) {
    _column++;
    lexChar(chr);
  }
}

void Lexer::lexEndOfNumber(Lexer::NumberType type,
                           int precision,
                           char32_t chr) {
  if (isSymbolChar(chr)) {
    _recv->error(makePosition(),
                 "Number immediately followed by symbol character");
  }
  _recv->numberEnd(makePosition(), type, precision);
  _state = State::IN_WHITESPACE;
}

void Lexer::lexChar(char32_t chr) {
  switch (_state) {
    case State::ENDED:
      _state = State::ENDED_REPORTED_ERROR;
      _recv->error(makePosition(), "Got stray data after end");
      break;

    case State::ENDED_REPORTED_ERROR:
      // We have already reported an data after end error. Do nothing.
      break;

    case State::IN_WHITESPACE:
      if (' ' == chr) {
        // Still in whitespace
      } else if ('\n' == chr) {
        _state = State::IN_WHITESPACE_GOT_N;
      } else if ('\r' == chr) {
        _state = State::IN_WHITESPACE_GOT_R;
      } else if ('\t' == chr) {
        _recv->error(makePosition(), "Got tab in whitespace");
      } else if ('{' == chr) {
        _recv->openBrace(makePosition());
      } else if ('}' == chr) {
        _recv->closeBrace(makePosition());
      } else if ('[' == chr) {
        _recv->openBracket(makePosition());
      } else if (']' == chr) {
        _recv->closeBracket(makePosition());
      } else if ('(' == chr) {
        _recv->openParen(makePosition());
      } else if (')' == chr) {
        _recv->closeParen(makePosition());
      } else if (';' == chr) {
        _recv->semicolon(makePosition());
      } else if ('>' == chr) {
        _recv->greaterThan(makePosition());
      } else if ('<' == chr) {
        _recv->lessThan(makePosition());
      } else if ('@' == chr) {
        _recv->at(makePosition());
      } else if (',' == chr) {
        _recv->comma(makePosition());
      } else if ('-' == chr) {
        _state = State::GOT_DASH;
      } else if ('=' == chr) {
        _state = State::GOT_EQUALS;
      } else if (':' == chr) {
        _state = State::GOT_COLON;
      } else if ('#' == chr) {
        _state = State::IN_COMMENT;
        _recv->commentBegin(makePosition());
      } else if ('"' == chr) {
        _state = State::IN_STRING;
        _recv->stringBegin(makePosition());
      } else if ('\'' == chr) {
        _state = State::GOT_QUOTE;
      } else if ('\0' == chr) {
        _state = State::ENDED;
      } else if (isSymbolChar(chr) &&
                 !isDigit(chr)) {
        _recv->symbolBegin(makePosition());
        _state = State::IN_SYMBOL;
      } else if ('0' == chr) {
        _state = State::GOT_ZERO;
      } else if (isDigit(chr)) {
        _recv->numberBegin(makePosition(), false, Radix::DECIMAL);
        _state = State::IN_NUMBER;
      } else if ('.' == chr) {
        _recv->error(makePosition(), "Encountered stray dot");
      } else if (isReservedChar(chr)) {
        _recv->error(makePosition(), "Encountered reserved character");
      } else {
        _recv->error(makePosition(), "Encountered invalid character");
      }
      break;

    case State::IN_WHITESPACE_GOT_N:
    case State::IN_WHITESPACE_GOT_R:
      handleNewline(chr,
                    (State::IN_WHITESPACE_GOT_N == _state ? '\n' : '\r'),
                    State::IN_WHITESPACE);
      break;

    case State::IN_COMMENT:
      if ('\n' == chr) {
        _state = State::IN_COMMENT_GOT_N;
      } else if ('\r' == chr) {
        _state = State::IN_COMMENT_GOT_R;
      } else if ('\0' == chr) {
        _recv->commentEnd(makePosition());
        _state = State::ENDED;
      } else {
        // Still in comment.
      }
      break;

    case State::IN_COMMENT_GOT_N:
    case State::IN_COMMENT_GOT_R:
      _recv->commentEnd(makePosition(/*offset:*/-1));
      handleNewline(chr,
                    (State::IN_COMMENT_GOT_N == _state ? '\n' : '\r'),
                    State::IN_WHITESPACE);
      break;

    case State::IN_SYMBOL:
      if (isSymbolChar(chr)) {
        // Still in symbol.
      } else {
        _recv->symbolEnd(makePosition());
        _state = State::IN_WHITESPACE;
        lexChar(chr);
      }
      break;

    case State::IN_STRING:
      if ('\n' == chr) {
        _state = State::IN_STRING_GOT_N;
      } else if ('\r' == chr) {
        _state = State::IN_STRING_GOT_R;
      } else if ('\\' == chr) {
        _state = State::IN_STRING_GOT_ESCAPE;
      } else if ('"' == chr) {
        _recv->stringEnd(makePosition());
        _state = State::IN_WHITESPACE;
      } else if ('\0' == chr) {
        _recv->error(makePosition(), "Got end in string literal");
        _state = State::ENDED;
      } else {
        // Still in string.
      }
      break;

    case State::IN_STRING_GOT_ESCAPE:
      if ('\n' == chr) {
        _state = State::IN_STRING_GOT_N;
      } else if ('\r' == chr) {
        _state = State::IN_STRING_GOT_R;
      } else if ('\0' == chr) {
        _recv->error(makePosition(), "Got end in string literal");
        _state = State::ENDED;
      } else {
        _state = State::IN_STRING;
      }
      break;

    case State::IN_STRING_GOT_N:
    case State::IN_STRING_GOT_R:
      _recv->error(makePosition(), "Got newline in string literal");

      if ('\0' == chr) {
        _recv->error(makePosition(), "Got end in string literal");
        _state = State::ENDED;
      } else {
        handleNewline(chr,
                      (State::IN_STRING_GOT_N == _state ? '\n' : '\r'),
                      State::IN_STRING);
      }
      break;

    case State::IN_REGEX:
      if ('\n' == chr) {
        _state = State::IN_REGEX_GOT_N;
      } else if ('\r' == chr) {
        _state = State::IN_REGEX_GOT_R;
      } else if ('\\' == chr) {
        _state = State::IN_REGEX_GOT_ESCAPE;
      } else if ('/' == chr) {
        _recv->regexEnd(makePosition());
        _state = State::IN_WHITESPACE;
      } else if ('\0' == chr) {
        _recv->error(makePosition(), "Got end in regex literal");
        _state = State::ENDED;
      } else {
        // Still in regex.
      }
      break;

    case State::IN_REGEX_GOT_ESCAPE:
      if ('\n' == chr) {
        _state = State::IN_REGEX_GOT_N;
      } else if ('\r' == chr) {
        _state = State::IN_REGEX_GOT_R;
      } else if ('\0' == chr) {
        _recv->error(makePosition(), "Got end in regex literal");
        _state = State::ENDED;
      } else {
        _state = State::IN_REGEX;
      }
      break;

    case State::IN_REGEX_GOT_N:
    case State::IN_REGEX_GOT_R:
      _recv->error(makePosition(), "Got newline in regex literal");

      if ('\0' == chr) {
        _recv->error(makePosition(), "Got end in regex literal");
        _state = State::ENDED;
      } else {
        handleNewline(chr,
                      (State::IN_REGEX_GOT_N == _state ? '\n' : '\r'),
                      State::IN_REGEX);
      }
      break;

    case State::IN_CHAR:
      if ('\n' == chr) {
        _state = State::IN_CHAR_GOT_N;
      } else if ('\r' == chr) {
        _state = State::IN_CHAR_GOT_R;
      } else if ('\\' == chr) {
        _state = State::IN_CHAR_GOT_ESCAPE;
      } else {
        _recv->charEnd(makePosition(/*offset:*/1));
        _state = State::IN_WHITESPACE;
      }
      break;

    case State::IN_CHAR_GOT_ESCAPE:
      if ('\n' == chr) {
        _state = State::IN_CHAR_GOT_N;
      } else if ('\r' == chr) {
        _state = State::IN_CHAR_GOT_R;
      } else {
        _recv->charEnd(makePosition(/*offset:*/1));
        _state = State::IN_WHITESPACE;
      }
      break;

    case State::IN_CHAR_GOT_N:
    case State::IN_CHAR_GOT_R:
      _recv->error(makePosition(), "Got newline in character literal");

      handleNewline(chr,
                    (State::IN_CHAR_GOT_N == _state ? '\n' : '\r'),
                    State::IN_CHAR);
      break;

    case State::IN_NUMBER:
    case State::IN_NUMBER_JUST_GOT_DOT:
    case State::IN_NUMBER_GOT_DOT:
      if (State::IN_NUMBER_JUST_GOT_DOT == _state &&
          !isDigit(chr)) {
        _recv->error(makePosition(), "Got non-digit after decimal dot");
      } else if (State::IN_NUMBER_GOT_DOT == _state &&
                 '.' == chr) {
        _recv->error(makePosition(), "Got two decimal dots in number");
      }

      if (isDigit(chr)) {
        // We're still in the number
        if (State::IN_NUMBER_JUST_GOT_DOT == _state) {
          _state = State::IN_NUMBER_GOT_DOT;
        }
      } else if ('.' == chr) {
        _state = State::IN_NUMBER_JUST_GOT_DOT;
      } else if ('i' == chr || 'I' == chr) {
        _state = State::GOT_NUMBER_TYPE_I;
      } else if ('u' == chr || 'U' == chr) {
        _state = State::GOT_NUMBER_TYPE_U;
      } else if ('f' == chr || 'F' == chr) {
        _state = State::GOT_NUMBER_TYPE_F;
      } else {
        lexEndOfNumber(NumberType::P_UNSPECIFIED, -1, chr);
      }
      break;

    case State::GOT_QUOTE:
      if ('/' == chr) {
        _recv->regexBegin(makePosition(/*offset:*/-1));
        _state = State::IN_REGEX;
      } else if ('\'' == chr) {
        _state = State::IN_CHAR;
        _recv->charBegin(makePosition(/*offset:*/-1));
      } else {
        _recv->quote(makePosition(/*offset:*/-1));
        _state = State::IN_WHITESPACE;
        lexChar(chr);
      }
      break;

    case State::GOT_DASH:
      if ('>' == chr) {
        _recv->arrow(makePosition(/*offset:*/-1));
      } else if ('0' == chr) {
        _state = State::GOT_ZERO_NEGATIVE;
      } else if (isDigit(chr) && '0' != chr) {
        _state = State::IN_NUMBER;
        _recv->numberBegin(makePosition(/*offset:*/-1),
                           true,
                           Radix::DECIMAL);
      } else {
        _state = State::IN_SYMBOL;
        _recv->symbolBegin(makePosition(/*offset:*/-1));
        lexChar(chr);
      }
      break;

    case State::GOT_EQUALS:
      if ('>' == chr) {
        _recv->doubleArrow(makePosition(/*offset:*/-1));
      } else {
        _state = State::IN_SYMBOL;
        _recv->symbolBegin(makePosition());
      }
      break;

    case State::GOT_COLON:
      _state = State::IN_WHITESPACE;
      if (':' == chr) {
        _recv->doubleColon(makePosition(/*offset:*/-1));
        _state = State::IN_WHITESPACE;
      } else {
        _recv->colon(makePosition(/*offset:*/-1));
        _state = State::IN_WHITESPACE;
        lexChar(chr);
      }
      break;

    case State::GOT_ZERO:
    case State::GOT_ZERO_NEGATIVE:
      {
        bool negative = (State::GOT_ZERO_NEGATIVE == _state);

        _state = State::IN_NUMBER;
        Position pos = makePosition(/*offset:*/(negative ? -2 : -1));

        if ('x' == chr || 'X' == chr) {
          _recv->numberBegin(pos, negative, Radix::HEX);
        } else if ('b' == chr || 'B' == chr) {
          _recv->numberBegin(pos, negative, Radix::BINARY);
        } else if ('o' == chr || 'O' == chr) {
          _recv->numberBegin(pos, negative, Radix::OCTAL);
        } else if (isDigit(chr)) {
          _recv->numberBegin(pos, negative, Radix::OCTAL);
          lexChar(chr);
        } else {
          // This case works for end of number as well
          // as i/u/f and decimal dot.
          _recv->numberBegin(pos, negative, Radix::DECIMAL);
          lexChar(chr);
        }
        break;
      }

    case State::GOT_NUMBER_TYPE_I:
    case State::GOT_NUMBER_TYPE_F:
    case State::GOT_NUMBER_TYPE_U:
      if (isDigit(chr)) {
        if ('0' == chr) {
          _recv->error(makePosition(),
                       "Number precision must not start with 0");
        }
        int digit = chr - '0';

        int stateOffset;
        if (State::GOT_NUMBER_TYPE_I == _state) {
          stateOffset = static_cast<int>(State::GOT_NUMBER_PRECISION_I_0);
        } else if (State::GOT_NUMBER_TYPE_U == _state) {
          stateOffset = static_cast<int>(State::GOT_NUMBER_PRECISION_U_0);
        } else if (State::GOT_NUMBER_TYPE_F == _state) {
          stateOffset = static_cast<int>(State::GOT_NUMBER_PRECISION_F_0);
        } else {
          ARW_ASSERT(0);
          stateOffset = static_cast<int>(State::GOT_NUMBER_PRECISION_I_0);
        }
        _state = static_cast<State>(stateOffset + digit);
      } else {
        NumberType type;
        if (State::GOT_NUMBER_TYPE_I == _state) {
          type = NumberType::P_SIGNED;
        } else if (State::GOT_NUMBER_TYPE_U == _state) {
          type = NumberType::P_UNSIGNED;
        } else if (State::GOT_NUMBER_TYPE_F == _state) {
          type = NumberType::P_IMPRECISE;
        } else {
          ARW_ASSERT(0);
        }

        lexEndOfNumber(type, -1, chr);
      }
      break;

    case State::GOT_NUMBER_PRECISION_I_0:
    case State::GOT_NUMBER_PRECISION_I_1:
    case State::GOT_NUMBER_PRECISION_I_2:
    case State::GOT_NUMBER_PRECISION_I_3:
    case State::GOT_NUMBER_PRECISION_I_4:
    case State::GOT_NUMBER_PRECISION_I_5:
    case State::GOT_NUMBER_PRECISION_I_6:
    case State::GOT_NUMBER_PRECISION_I_7:
    case State::GOT_NUMBER_PRECISION_I_8:
    case State::GOT_NUMBER_PRECISION_I_9:
    case State::GOT_NUMBER_PRECISION_U_0:
    case State::GOT_NUMBER_PRECISION_U_1:
    case State::GOT_NUMBER_PRECISION_U_2:
    case State::GOT_NUMBER_PRECISION_U_3:
    case State::GOT_NUMBER_PRECISION_U_4:
    case State::GOT_NUMBER_PRECISION_U_5:
    case State::GOT_NUMBER_PRECISION_U_6:
    case State::GOT_NUMBER_PRECISION_U_7:
    case State::GOT_NUMBER_PRECISION_U_8:
    case State::GOT_NUMBER_PRECISION_U_9:
    case State::GOT_NUMBER_PRECISION_F_0:
    case State::GOT_NUMBER_PRECISION_F_1:
    case State::GOT_NUMBER_PRECISION_F_2:
    case State::GOT_NUMBER_PRECISION_F_3:
    case State::GOT_NUMBER_PRECISION_F_4:
    case State::GOT_NUMBER_PRECISION_F_5:
    case State::GOT_NUMBER_PRECISION_F_6:
    case State::GOT_NUMBER_PRECISION_F_7:
    case State::GOT_NUMBER_PRECISION_F_8:
    case State::GOT_NUMBER_PRECISION_F_9:
      {
        NumberType type;
        if (isNumberPrecision(_state, 'i')) {
          type = NumberType::P_SIGNED;
        } else if (isNumberPrecision(_state, 'u')) {
          type = NumberType::P_UNSIGNED;
        } else if (isNumberPrecision(_state, 'f')) {
          type = NumberType::P_IMPRECISE;
        } else {
          ARW_ASSERT(0);
        }

        if (isDigit(chr)) {
          int precision = numberPrecisionDigit(_state)*10+(chr-'0');
          _recv->numberEnd(makePosition(/*offset:*/1), type, precision);
          _state = State::GOT_NUMBER_PRECISION_2;
        } else {
          lexEndOfNumber(type, numberPrecisionDigit(_state), chr);
        }
        break;
      }

    case State::GOT_NUMBER_PRECISION_2:
    case State::GOT_NUMBER_PRECISION_3:
      if (isDigit(chr)) {
        if (State::GOT_NUMBER_PRECISION_2 == _state) {
          _recv->error(makePosition(),
                       "Number precision must be at most 2 digits");
        }
      } else {
        if (isSymbolChar(chr)) {
          _recv->error(makePosition(),
                       "Number immediately followed by symbol character");
        }
        _state = State::IN_WHITESPACE;
        lexChar(chr);
      }
      break;
  }
}

}  // namespace arw
