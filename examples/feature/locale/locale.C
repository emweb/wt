/*
 * Copyright (C) 2013 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include <Wt/WApplication.h>
#include <Wt/WAbstractTableModel.h>
#include <Wt/WEnvironment.h>
#include <Wt/WComboBox.h>
#include <Wt/WLabel.h>
#include <Wt/WTemplate.h>
#include <Wt/WTime.h>
#include <Wt/WReadOnlyProxyModel.h>
#include <Wt/WLocalDateTime.h>
#include <Wt/WContainerWidget.h>

#include <Wt/Date/tz.h>

#include <boost/algorithm/string.hpp>

/*
 * This is a model that reads the time zone database.
 *
 * As additional functionality it offers the capability to propose a
 * timezone based on a known current time zone offset (obtained from
 * Wt::WEnvironment::timeZoneOffset()).
 */
class TimeZoneModel : public Wt::WAbstractTableModel
{
public:
  static constexpr Wt::ItemDataRole NameTimeZoneRole = Wt::ItemDataRole::User + 1;

  TimeZoneModel()
    : showOffset_(true)
  {
  }

  void load()
  {
    for (const date::time_zone& z: date::get_tzdb().zones) {
      if(z.name().size() > 3 && z.name().find("Etc/") == std::string::npos)
        if(std::any_of(std::begin(z.name()), std::end(z.name()), [](char c){return (islower(c));}))
          ids_.push_back(z.name());
    }
  }

  void setShowOffset(bool enabled)
  {
    showOffset_ = enabled;
  }

  int suggestedTimeZone(const std::string &timeZoneName,
                        std::chrono::minutes currentOffset)
  {
    if (!timeZoneName.empty()) {
      for (unsigned i = 0; i < ids_.size(); ++i) {
        if (ids_[i] == timeZoneName)
          return i;
      }
    }

    std::chrono::system_clock::time_point nowUtc = std::chrono::system_clock::now();

    int bestPreference = 0;
    int bestRow = -1;

    for (unsigned i = 0; i < ids_.size(); ++i) {
      std::string id = ids_[i];
      const date::time_zone* zone = date::locate_zone(id);
      auto zoneinfo = zone->get_info(nowUtc);

      if (zoneinfo.offset == currentOffset) {
        int pref = computePreference(id, zone);
        if (pref > bestPreference) {
          bestRow = i;
          bestPreference = pref;
        }
      }
    }

    return bestRow;
  }

  virtual int rowCount(const Wt::WModelIndex& parent = Wt::WModelIndex())
    const override
  {
    if (!parent.isValid())
      return ids_.size();
    else
      return 0;
  }

  virtual int columnCount(const Wt::WModelIndex& parent = Wt::WModelIndex())
    const override
  {
    if (!parent.isValid())
      return 1;
    else
      return 0;
  }

  static std::string locality(const std::string& id) {
    std::string result = id.substr(id.find('/') + 1);
    boost::replace_all(result, "_", " ");
    return result;
  }

  virtual Wt::cpp17::any data(const Wt::WModelIndex& index,
                          Wt::ItemDataRole role = Wt::ItemDataRole::Display) const override
  {
    std::string id = ids_[index.row()];

    switch (role.value()) {
    case Wt::ItemDataRole::Display: {
      if (showOffset_) {
          const date::time_zone * zone = date::locate_zone(id);
          auto info = zone->get_info(std::chrono::system_clock::now());
          Wt::WTime t = Wt::WTime(0, 0, 0)
                  .addSecs(info.offset.count());

	std::string result = locality(id) + " (GMT" + 
	  t.toString("+hh:mm").toUTF8() + ")";

	return result;
      } else
	return locality(id);
    }
    case Wt::ItemDataRole::Level:
      return id.substr(0, id.find('/'));
    case NameTimeZoneRole.value():
      return id;
    default:
      return Wt::cpp17::any();
    }
  }

  virtual Wt::cpp17::any headerData(int section,
                                Wt::Orientation orientation = Wt::Orientation::Horizontal,
                                Wt::ItemDataRole role = Wt::ItemDataRole::Display) const override
  {
    if (orientation == Wt::Orientation::Horizontal) {
      switch (role.value()) {
      case Wt::ItemDataRole::Display:
	return std::string("locality");
      default:
        return Wt::cpp17::any();
      }
    } else
      return Wt::cpp17::any();
  }

protected:
  virtual int computePreference(const std::string& id,
                const date::time_zone* zone)
  {
    /*
     * We implement here the following heuristic:
     *
     * Take first city of the following 'preferred' list:
     *   - Europe (5)
     *   - Asia (4),
     *   - Australia (3),
     *   - America (2)
     * Otherwise, 1
     */
    if (boost::starts_with(id, "Europe/"))
      return 5;
    else if (boost::starts_with(id, "Australia/"))
      return 4;
    else if (boost::starts_with(id, "Asia/"))
      return 3;
    else if (boost::starts_with(id, "America/"))
      return 2;
    else
      return 1;
  }

private:
  std::vector<std::string> ids_;
  bool showOffset_;
};

constexpr Wt::ItemDataRole TimeZoneModel::NameTimeZoneRole;

std::shared_ptr<TimeZoneModel> timeZones = std::make_shared<TimeZoneModel>();

class LocaleApplication : public Wt::WApplication
{
public:
  LocaleApplication(const Wt::WEnvironment& env)
    : Wt::WApplication(env)
  {
    messageResourceBundle().use("templates");

    root()->addWidget(Wt::cpp14::make_unique<Wt::WLabel>("Select your time zone: "));
    localeCombo_ = root()->addWidget(Wt::cpp14::make_unique<Wt::WComboBox>());

    info_ = root()->addWidget(Wt::cpp14::make_unique<Wt::WTemplate>
                              (Wt::WString::tr("info")));
    auto regions = std::make_shared<Wt::WReadOnlyProxyModel>();
    regions->setSourceModel(timeZones);
    localeCombo_->setModel(regions);

    localeCombo_->setCurrentIndex
      (timeZones->suggestedTimeZone(env.timeZoneName(), env.timeZoneOffset()));

    localeCombo_->changed().connect(this, &LocaleApplication::updateLocale);

    updateLocale();
  }

  void updateLocale()
  {
    std::string tz = Wt::cpp17::any_cast<std::string>
      (timeZones->index(localeCombo_->currentIndex(), 0)
       .data(TimeZoneModel::NameTimeZoneRole));

    Wt::WLocale l = locale();
    l.setTimeZone(date::locate_zone(tz));
    setLocale(l);

    Wt::WString format = "yyyy-MM-dd HH:mm:ss Z";

    auto info = l.timeZone()->get_info(std::chrono::system_clock::now());
    std::ostringstream os;
    os << info.abbrev << " " 
       << Wt::WTime(0, 0 ,0).addSecs(info.offset.count()).toString("+hh:mm").toUTF8();

    info_->bindString("time-zone", os.str());
    info_->bindString("utc-time",
                      Wt::WDateTime::currentDateTime().toString(format));
    info_->bindString("local-time",
                      Wt::WLocalDateTime::currentDateTime().toString(format));
  }

private:
  Wt::WComboBox *localeCombo_;
  Wt::WTemplate *info_;
};

std::unique_ptr<Wt::WApplication> createApplication(const Wt::WEnvironment& env)
{
  return Wt::cpp14::make_unique<LocaleApplication>(env);
}

int main(int argc, char **argv)
{
#if !USE_OS_TZDB
  date::set_install("./tzdata");
#endif

  timeZones->load();

  return Wt::WRun(argc, argv, &createApplication);
}
