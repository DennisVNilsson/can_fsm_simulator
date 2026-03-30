#include <cstdio>
#include "controller_node.hpp"
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
static ControllerNode node_here(&bus, 10);

// ── Tests ─────────────────────────────────────────────────────────────────

static void test_initial_state_is_MONITORING() {
  node_here.reset(); 
  TEST_BEGIN("test initial state is MONITORING")
  CHECK(node_here.state() == controller::state::MONITORING); 
  TEST_END
}

static void test_transition_to_EVALUATING() {
  node_here.reset(); 
  TEST_BEGIN("test that recieving a sensor frame transitions to EVALUATING")
  CanFrame cf{}; 
  cf.id = can::ID_SENSOR; 
  cf.data[0] = 0x020; 
  cf.data[1] = 0x020; 
  cf.dlc = 2; 
  cf.rtr = false; 
  bus.transmit(cf);
  bus.tick(); 
  CHECK(node_here.state() == controller::state::EVALUATING); 
  TEST_END
}

static void test_val_above_threshold_state_COMMANDING() {
    node_here.reset();
    TEST_BEGIN("test that read value above threshold leads to COMMANDING state")
    
    // First get into EVALUATING by sending a frame above threshold
    CanFrame cf{};
    cf.id  = can::ID_SENSOR;
    cf.dlc = 2;
    cf.data[0] = 0x00;
    cf.data[1] = 0x0B;  // value = 11, above threshold of 10
    cf.rtr = false;
    bus.transmit(cf);
    bus.tick();  // delivers frame → EVALUATING, stores reading = 11
    
    // Now tick the controller — should detect threshold exceeded → COMMANDING
    node_here.tick();
    CHECK(node_here.state() == controller::state::COMMANDING);
    TEST_END
}

static void test_on_fault() {
  node_here.reset();  
  TEST_BEGIN("test that on_fault function moves state into fault from any state")
  node_here.on_fault(); 
  CHECK(node_here.state() == controller::state::FAULT); 
  TEST_END
}

int main() {
  test_initial_state_is_MONITORING(); 
  test_transition_to_EVALUATING();
  test_val_above_threshold_state_COMMANDING();
  test_on_fault();

  printf("\n── Results ──────────────────\n");
  printf("  Passed : %d\n", s_passed);
  printf("  Failed : %d\n", s_failed);
  printf("  Total  : %d\n", s_passed + s_failed);
  printf("─────────────────────────────\n\n");

  return s_failed == 0 ? 0 : 1;
}