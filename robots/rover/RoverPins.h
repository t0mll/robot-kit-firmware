#pragma once
// Rover wiring on the Brain (see docs/briefs/rover.md and hardware/rover-r1.md in the shop repo).
namespace rover {
constexpr int MOTOR_L_IN1 = 10, MOTOR_L_IN2 = 11;  // DRV8833 AIN1/AIN2 (left)
constexpr int MOTOR_R_IN1 = 12, MOTOR_R_IN2 = 13;  // DRV8833 BIN1/BIN2 (right)
constexpr int ENC_L_A = 43, ENC_L_B = 44;          // N20 Hall encoder, left (3.3 V supply)
constexpr int ENC_R_A = 2, ENC_R_B = 3;            // N20 Hall encoder, right
constexpr int BUTTON_SELECT = 16;
constexpr int BUZZER = 21;
constexpr int BATTERY_SENSE = 1;  // 100k/100k divider from the cell
}  // namespace rover
