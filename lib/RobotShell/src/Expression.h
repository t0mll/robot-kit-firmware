#pragma once
#include <stdint.h>

/// The shared set of Expressions every Robot's Face can show (see CONTEXT.md in the shop repo).
enum class Expression : uint8_t { Neutral, Happy, Sad, Angry, Surprised, Sleepy, Love, Confused };

inline const char* expressionName(Expression e) {
  switch (e) {
    case Expression::Neutral: return "neutral";
    case Expression::Happy: return "happy";
    case Expression::Sad: return "sad";
    case Expression::Angry: return "angry";
    case Expression::Surprised: return "surprised";
    case Expression::Sleepy: return "sleepy";
    case Expression::Love: return "love";
    case Expression::Confused: return "confused";
  }
  return "?";
}
