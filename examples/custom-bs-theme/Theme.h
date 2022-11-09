#ifndef THEME_H_
#define THEME_H_

#include <Wt/WBootstrap5Theme.h>

#include <vector>

class Theme final : public Wt::WBootstrap5Theme {
public:
  std::vector<Wt::WLinkedCssStyleSheet> styleSheets() const override;
};

#endif // THEME_H_
