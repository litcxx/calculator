#ifndef APPLICATION_H_
#define APPLICATION_H_

#include "config.hpp"

class Logger;

namespace calculator
{
class Application
{
  public:
    void run(int argc, char** argv);

    [[nodiscard]]
    const Task& getTask() const noexcept
    {
        return task_;
    }

  private:
    void getTask(int argc, char** argv);
    void makeCalculate();
    void printResult() const;

    Task task_;
};

} // namespace calculator

#endif
