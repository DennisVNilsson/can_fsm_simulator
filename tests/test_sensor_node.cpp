#include <cstdio>
#include "sensor_node.hpp"
#include "can_bus.hpp"

// ── Test framework ────────────────────────────────────────────────────────
static int s_passed = 0;
static int s_failed = 0;

#define CHECK(expr)                                                           \
    do {                                                                       \
        if (!(expr)) {                                                         \
            printf("  FAILED: %s\n  at %s line %d\n", #expr, __FILE__, __LINE__); \
            s_failed++;                                                        \
            return;                                                            \
        }                                                                      \
    } while (0)

#define TEST_BEGIN(name) { printf("  %-50s", name);
#define TEST_END           printf("OK\n"); s_passed++; }

// ── Shared fixtures ───────────────────────────────────────────────────────
static CanBus bus;
static SensorNode node_sensor_test(&bus);

// ── Tests ─────────────────────────────────────────────────────────────────

static void test_initial_state_is_idle() {
  TEST_BEGIN("node_sensor_test initial state is IDLE")
  CHECK(node_sensor_test.state() == sensor::state::IDLE);
  TEST_END
}

static void test_idle_after_once_cycle() {
  TEST_BEGIN("node_sensor_test state is IDLE after ")
  node_sensor_test.tick();
  node_sensor_test.tick();
  node_sensor_test.tick();
  CHECK(node_sensor_test.state() == sensor::state::IDLE);
  bus.tick();  
  TEST_END
}

static void test_on_fault() {
  TEST_BEGIN("node_sensor_test state moves to FAULT when calling fault()")
  node_sensor_test.on_fault(); 
  CHECK(node_sensor_test.state() == sensor::state::FAULT); 
  TEST_END
}

int main() {
  test_initial_state_is_idle();
  test_idle_after_once_cycle();
  test_on_fault(); 

  printf("\n── Results ──────────────────\n");
  printf("  Passed : %d\n", s_passed);
  printf("  Failed : %d\n", s_failed);
  printf("  Total  : %d\n", s_passed + s_failed);
  printf("─────────────────────────────\n\n");

  return s_failed == 0 ? 0 : 1;
}