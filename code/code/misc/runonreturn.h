#pragma once

/*
class RunOnReturn
{
  private:
    std::function<void(void)>&& fn;
  public:
    RunOnReturn(std::function<void(void)>&& fn)
      : fn(std::move(fn))
    { }
    ~RunOnReturn()
    {
      fn();
    }
};
*/
