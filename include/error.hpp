#pragma once

#include "memory"
#include "string"

// https://codereview.stackexchange.com/questions/157448/error-handling-class
enum Status
{
  SUCCESS,
  FAILURE
};

struct Error
{
  virtual ~Error(){};
  virtual std::string get_message() = 0;
};

struct Result
{
  Status s;
  std::shared_ptr<Error> e;

  Result() : s(SUCCESS), e(nullptr){};

  Result(Error* error) : s(error == nullptr ? SUCCESS : FAILURE), e(error) {}

  std::string get_message() const
  {
    if (nullptr == e)
    {
      return "";
    }
    else
    {
      return e->get_message();
    }
  }

  int get_code() const
  {
    if (nullptr == e)
    {
      return 0;
    }
    else
    {
      return e->get_code();
    }
  }
};

struct FileNotFound : Error
{
  std::string get_message() { return "File not found"; }
  int get_code() { return 1 }
};
