#pragma once

#include <atomic>
#include <chrono>
#include <string>
#include <sstream>
#include <thread>
#include <vector>

#include <boost/optional.hpp>

#include <qi/future.hpp>

namespace test {
namespace qilang {

  // Provides a mechanism to check if a function is being called concurrently.
  struct ChecksConcurrentCalls
  {
    explicit ChecksConcurrentCalls(const std::string& name) : name(name) {}

    // Call this in the function you want to check to never be called concurrently.
    // @returns An error message iff concurrent calls are detected, nothing otherwise.
    auto failOnConcurrentCalls(std::chrono::milliseconds duration = std::chrono::milliseconds{ 500 })
      -> boost::optional<std::string>
    {
      auto lock = qi::scopelock(executing);
      if (!lock)
        return failMessage();

      std::this_thread::sleep_for(duration);
      return {};
    }

  private:
    std::atomic_flag executing{ false };
    const std::string name;

    std::string failMessage() const
    {
      std::ostringstream message;
      message << name << " detected concurrent calls.";
      return message.str();
    }

  };

  // Calls a task from several threads, as many times as requested.
  template<typename Task>
  auto runTaskConcurrently(Task task,
    const int countCallsPerCaller = 1,
    const int countConcurrentCallers = std::thread::hardware_concurrency()
    )
    -> std::vector<qi::Future<void>>
  {
    std::vector<std::thread> callers(countConcurrentCallers);
    std::vector<qi::Future<void>> results;
    std::vector<qi::Future<void>> readyFutures;
    qi::Promise<void> goPromise;
    auto goFuture = goPromise.future();

    for (auto& caller : callers)
    {
      qi::Promise<void> readyPromise;
      readyFutures.push_back(readyPromise.future());

      qi::Promise<void> resultPromise;
      results.push_back(resultPromise.future());

      caller = std::thread{ [=]() mutable {
        try
        {
          readyPromise.setValue(nullptr); // Notify that this thread is ready.
          goFuture.value(); // Wait here for all threads to be ready and go.
          for (int i = 0; i < countCallsPerCaller; ++i)
          {
            task();
          }
        }
        catch (const std::runtime_error& ex)
        {
          resultPromise.setError(ex.what());
          return;
        }
        resultPromise.setValue(nullptr);
      } };
    }

    // Wait for all threads to be ready.
    for (auto&& readyFuture : readyFutures)
    {
      readyFuture.value();
    }

    // Launch all threads now.
    goPromise.setValue(nullptr);

    // Wait for all threads to be finished.
    for (auto&& caller : callers)
    {
      caller.join();
    }

    return results;
  }

}}

