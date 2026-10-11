#include <gtest/gtest.h>

#include <memory>
#include <string>

#include "esphome/core/base_automation.h"

namespace esphome::testing {

class FlagCondition : public Condition<const std::string &> {
 public:
  bool check(const std::string & /*value*/) override { return this->ready; }
  bool ready{false};
};

class RecordAction : public Action<const std::string &> {
 public:
  void play(const std::string &value) override { this->seen = value; }
  std::string seen;
};

// A trigger argument passed as `const T &` must be copied while waiting: the
// referenced object is gone once the trigger has returned.
TEST(WaitUntilActionTest, KeepsCopyOfReferenceArguments) {
  FlagCondition condition;
  WaitUntilAction<const std::string &> wait(&condition);
  RecordAction record;
  ActionList<const std::string &> actions;
  actions.add_actions({&wait, &record});

  {
    auto value = std::make_unique<std::string>("argument longer than the small string buffer");
    actions.play(*value);
  }
  EXPECT_TRUE(record.seen.empty());

  condition.ready = true;
  wait.loop();
  EXPECT_EQ(record.seen, "argument longer than the small string buffer");
}

class Counter {
 public:
  int value{0};
};

class CounterCondition : public Condition<Counter &> {
 public:
  bool check(Counter & /*counter*/) override { return this->ready; }
  bool ready{false};
};

class IncrementAction : public Action<Counter &> {
 public:
  void play(Counter &counter) override { this->seen = ++counter.value; }
  int seen{0};
};

// A non-const reference argument used to fail to compile
TEST(WaitUntilActionTest, AcceptsMutableReferenceArguments) {
  CounterCondition condition;
  WaitUntilAction<Counter &> wait(&condition);
  IncrementAction increment;
  ActionList<Counter &> actions;
  actions.add_actions({&wait, &increment});

  Counter counter;
  actions.play(counter);
  condition.ready = true;
  wait.loop();
  EXPECT_EQ(increment.seen, 1);
}

class Entity {
 public:
  Entity() = default;
  Entity(const Entity &) = delete;
  Entity &operator=(const Entity &) = delete;
};

class EntityCondition : public Condition<Entity &> {
 public:
  bool check(Entity & /*entity*/) override { return this->ready; }
  bool ready{false};
};

class EntityAction : public Action<Entity &> {
 public:
  void play(Entity &entity) override { this->seen = &entity; }
  Entity *seen{nullptr};
};

// An object that cannot be copied, such as the entity passed as `Climate &`, is kept by reference
TEST(WaitUntilActionTest, KeepsReferenceToNonCopyableArguments) {
  EntityCondition condition;
  WaitUntilAction<Entity &> wait(&condition);
  EntityAction record;
  ActionList<Entity &> actions;
  actions.add_actions({&wait, &record});

  Entity entity;
  actions.play(entity);
  EXPECT_EQ(record.seen, nullptr);

  condition.ready = true;
  wait.loop();
  EXPECT_EQ(record.seen, &entity);
}

}  // namespace esphome::testing
