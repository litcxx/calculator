#pragma once

#include "config.hpp"

class Logger;

namespace calculator
{
class Application
{
  public:
    void run(int argc, char** argv);

    const Task& getTask() const noexcept
    {
        return task_;
    }

  private:
    void getTask(int argc, char** argv);
    void makeCalculate();
    void printResult() const;

  private:
    Task task_;
};

} // namespace calculator
