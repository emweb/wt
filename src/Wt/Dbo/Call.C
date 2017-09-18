#include <Wt/Dbo/Call.h>
#include <Wt/Dbo/Session.h>
#include <Wt/Dbo/SqlStatement.h>

namespace Wt {
  namespace Dbo {

Call::~Call() noexcept(false)
{
  if (!copied_ && !run_)
    run();
}

Call::Call(const Call& other)
  : copied_(false),
    run_(false),
    statement_(other.statement_),
    column_(other.column_)
{
  const_cast<Call&>(other).copied_ = true;
}

void Call::run()
{
  try {
    run_ = true;
    statement_->execute();
    statement_->done();
  } catch (...) {
    statement_->done();
    throw;
  }
}

Call::Call(Session& session, const std::string& sql)
  : copied_(false),
    run_(false)
{
  statement_ = session.getOrPrepareStatement(sql);
  column_ = 0;
}

  }
}
