// Host tests for the battery guard: pio test -e native
#include <BatteryGuard.h>
#include <unity.h>

namespace {
const BatteryLimits ROVER{2.5f, 3.45f, 3.30f, 3.70f};

/// Feeds `volts` every 100 ms for `ms`, starting at `t`; returns the time after the last reading.
uint32_t feed(BatteryGuard& g, float volts, uint32_t t, uint32_t ms) {
  for (uint32_t end = t + ms; t < end; t += 100) g.update(volts, t);
  return t;
}
}  // namespace

void setUp() {}
void tearDown() {}

void test_off_without_limits() {
  BatteryGuard g;
  g.setLimits({});
  feed(g, 3.0f, 0, 10000);
  TEST_ASSERT_FALSE(g.enabled());
  TEST_ASSERT_EQUAL(BatteryState::Unknown, g.state());
}

void test_full_cell_reads_ok_after_hold() {
  BatteryGuard g;
  g.setLimits(ROVER);
  uint32_t t = feed(g, 4.1f, 0, 2000);
  TEST_ASSERT_EQUAL(BatteryState::Unknown, g.state());  // not yet held for 3 s
  feed(g, 4.1f, t, 2000);
  TEST_ASSERT_EQUAL(BatteryState::Ok, g.state());
}

void test_motor_dip_does_not_stop() {
  BatteryGuard g;
  g.setLimits(ROVER);
  uint32_t t = feed(g, 3.8f, 0, 4000);
  t = feed(g, 3.1f, t, 1500);  // a 1.5 s sag under load
  t = feed(g, 3.8f, t, 4000);
  TEST_ASSERT_EQUAL(BatteryState::Ok, g.state());
}

void test_low_then_empty() {
  BatteryGuard g;
  g.setLimits(ROVER);
  uint32_t t = feed(g, 3.8f, 0, 4000);
  t = feed(g, 3.40f, t, 5000);
  TEST_ASSERT_EQUAL(BatteryState::Low, g.state());
  feed(g, 3.20f, t, 5000);
  TEST_ASSERT_EQUAL(BatteryState::Empty, g.state());
}

void test_rested_cell_stays_empty_until_charging() {
  BatteryGuard g;
  g.setLimits(ROVER);
  uint32_t t = feed(g, 3.8f, 0, 4000);
  t = feed(g, 3.20f, t, 5000);
  TEST_ASSERT_EQUAL(BatteryState::Empty, g.state());
  t = feed(g, 3.50f, t, 10000);  // motors off, the cell rests back up: still empty
  TEST_ASSERT_EQUAL(BatteryState::Empty, g.state());
  feed(g, 3.90f, t, 5000);  // charging
  TEST_ASSERT_EQUAL(BatteryState::Ok, g.state());
}

void test_pulled_cell_is_unknown_at_once() {
  BatteryGuard g;
  g.setLimits(ROVER);
  uint32_t t = feed(g, 3.8f, 0, 4000);
  t = feed(g, 3.20f, t, 5000);
  TEST_ASSERT_EQUAL(BatteryState::Empty, g.state());
  for (int i = 0; i < 20 && g.state() != BatteryState::Unknown; i++, t += 100) g.update(0.0f, t);
  TEST_ASSERT_EQUAL(BatteryState::Unknown, g.state());  // USB only: don't block the Builder
}

void test_usb_only_never_empty() {
  BatteryGuard g;
  g.setLimits(ROVER);
  feed(g, 0.05f, 0, 10000);
  TEST_ASSERT_EQUAL(BatteryState::Unknown, g.state());
}

int main() {
  UNITY_BEGIN();
  RUN_TEST(test_off_without_limits);
  RUN_TEST(test_full_cell_reads_ok_after_hold);
  RUN_TEST(test_motor_dip_does_not_stop);
  RUN_TEST(test_low_then_empty);
  RUN_TEST(test_rested_cell_stays_empty_until_charging);
  RUN_TEST(test_pulled_cell_is_unknown_at_once);
  RUN_TEST(test_usb_only_never_empty);
  return UNITY_END();
}
