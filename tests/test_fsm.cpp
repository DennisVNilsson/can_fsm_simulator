#include <cstdint>
#include "fsm.hpp"
#include <cstdio>
#include <cstring>

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

// TEST_BEGIN(name) / TEST_END — wrap each test with a printed label.
#define TEST_BEGIN(name) { printf("  %-50s", name);
#define TEST_END           printf("OK\n"); s_passed++; }

// Three states
static constexpr uint8_t IDLE    = 0;
static constexpr uint8_t RUNNING = 1;
static constexpr uint8_t FAULT   = 2;

// Three events
static constexpr uint8_t EVT_START = 0;
static constexpr uint8_t EVT_STOP  = 1;
static constexpr uint8_t EVT_ERROR = 2;

// Action flag — lets us verify the action was called
static bool action_fired = false;
static void on_start() { action_fired = true; }

// Transition table
static const Transition rules[] = {
    { IDLE,    EVT_START, on_start, RUNNING },
    { RUNNING, EVT_STOP,  nullptr,  IDLE    },
    { RUNNING, EVT_ERROR, nullptr,  FAULT   },
};

static Fsm fsm(rules, 3, IDLE);

static void test_initial_state_is_IDLE() {
  fsm.reset(); 
  action_fired = false; 
  TEST_BEGIN("Check that initial state is IDLE") 
  CHECK(fsm.state() == IDLE); 
  TEST_END 
}

static void test_disp_EVT_START_from_IDLE_into_RUNNING() {
  fsm.reset(); 
  action_fired = false; 
  TEST_BEGIN("Check that EVT_START brings IDLE to RUNNING on start") 
  CHECK(fsm.state() == IDLE); 
  fsm.dispatch(EVT_START);
  CHECK(fsm.state() == RUNNING); 
  CHECK(action_fired); 
  TEST_END
}

static void test_disp_unknown_evt_does_nothing() {
  fsm.reset(); 
  action_fired = false; 
  TEST_BEGIN("Dispatching an unknown event does nothing")
  static constexpr uint8_t UNK_EVT = 9;
  CHECK(fsm.state() == IDLE); 
  fsm.dispatch(UNK_EVT); 
  CHECK(fsm.state() == IDLE); 
  TEST_END
}

int main() {
  test_initial_state_is_IDLE(); 
  test_disp_EVT_START_from_IDLE_into_RUNNING(); 
  test_disp_unknown_evt_does_nothing();

  printf("\n── Results ──────────────────\n");
  printf("  Passed : %d\n", s_passed);
  printf("  Failed : %d\n", s_failed);
  printf("  Total  : %d\n", s_passed + s_failed);
  printf("─────────────────────────────\n\n");

  return s_failed == 0 ? 0 : 1;
}
