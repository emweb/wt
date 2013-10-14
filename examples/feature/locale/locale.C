/*
 * Copyright (C) 2013 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

// see http://stackoverflow.com/questions/15234527/boost-1-53-local-date-time-compiler-error-with-std-c0x
#if __cplusplus >= 201103L
#define BOOST_NO_CXX11_EXPLICIT_CONVERSION_OPERATORS
#endif

#include <Wt/WApplication>
#include <Wt/WAbstractTableModel>
#include <Wt/WEnvironment>
#include <Wt/WComboBox>
#include <Wt/WLabel>
#include <Wt/WTemplate>
#include <Wt/WTime>
#include <Wt/WReadOnlyProxyModel>
#include <Wt/WLocalDateTime>

#include <boost/algorithm/string.hpp>

/*
 * This is a model that reads the "date_time_zonespec.csv" time zone
 * database that comes with Boost.
 *
 * As additional functionality it offers the capability to propose a
 * timezone based on a known current time zone offset (obtained from
 * Wt::WEnvironment::timeZoneOffset()).
 */
class TimeZoneModel : public Wt::WAbstractTableModel
{
public:
  static const int BoostTimeZoneRole = Wt::UserRole;
  static const int PosixTimeZoneRole = Wt::UserRole + 1;

  TimeZoneModel()
    : showOffset_(true)
  { }

  void load(const std::string& fileName)
  {
    tz_db_.load_from_file(fileName);
    ids_ = tz_db_.region_list();
  }

  void setShowOffset(bool enabled)
  {
    showOffset_ = enabled;
  }

  int suggestedTimeZone(int currentOffset)
  {
    boost::posix_time::ptime nowUtc
      = boost::posix_time::microsec_clock::universal_time();
    boost::posix_time::time_duration nowOffset
      = boost::posix_time::minutes(currentOffset);

    int bestPreference = 0;
    int bestRow = -1;

    for (unsigned i = 0; i < ids_.size(); ++i) {
      std::string id = ids_[i];
      boost::local_time::time_zone_ptr tz = tz_db_.time_zone_from_region(id);
      boost::posix_time::ptime nowLocal
	= boost::local_time::local_date_time(nowUtc, tz).local_time();

      if (nowLocal - nowUtc == nowOffset) {
	int pref = computePreference(id, tz);
	if (pref > bestPreference) {
	  bestRow = i;
	  bestPreference = pref;
	}
      }
    }

    return bestRow;
  }

  virtual int rowCount(const Wt::WModelIndex& parent = Wt::WModelIndex())
    const
  {
    if (!parent.isValid())
      return ids_.size();
    else
      return 0;
  }

  virtual int columnCount(const Wt::WModelIndex& parent = Wt::WModelIndex())
    const
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

  virtual boost::any data(const Wt::WModelIndex& index,
			  int role = Wt::DisplayRole) const
  {
    std::string id = ids_[index.row()];

    switch (role) {
    case Wt::DisplayRole: {
      if (showOffset_) {
	boost::local_time::time_zone_ptr tz = tz_db_.time_zone_from_region(id);
	
	Wt::WTime t = Wt::WTime(0, 0, 0)
	  .addSecs(tz->base_utc_offset().total_seconds());

	std::string result = locality(id) + " (GMT" + 
	  t.toString("+hh:mm").toUTF8() + ")";

	return result;
      } else
	return locality(id);
    }
    case Wt::LevelRole:
      return id.substr(0, id.find('/'));
    case BoostTimeZoneRole:
      return tz_db_.time_zone_from_region(id);
    case PosixTimeZoneRole:
      return tz_db_.time_zone_from_region(id)->to_posix_string();
    default:
      return boost::any();
    }
  }

  virtual boost::any headerData(int section,
                                Wt::Orientation orientation = Wt::Horizontal,
                                int role = Wt::DisplayRole) const
  {
    if (orientation == Wt::Horizontal) {
      switch (role) {
      case Wt::DisplayRole:
	return std::string("locality");
      default:
        return boost::any();
      }
    } else
      return boost::any();
  }

protected:
  virtual int computePreference(const std::string& id,
				boost::local_time::time_zone_ptr tz)
  {
    /*
     * We implement here the following heuristic:
     *
     * If abbrev != name:
     *   preference = 6 (this seems to select 'standard' cities in USA)
     * Otherwise,
     *   take first city of the following 'preferred' list:
     *     - Europe (5)
     *     - Asia (4),
     *     - Australia (3),
     *     - America (2)
     * Otherwise, 1
     */

    if (tz->std_zone_abbrev() != tz->std_zone_name())
      return 6;
    else {
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
  }

private:
  boost::local_time::tz_database tz_db_;
  std::vector<std::string> ids_;
  bool showOffset_;
};

TimeZoneModel timeZones;

class LocaleApplication : public Wt::WApplication
{
public:
  LocaleApplication(const Wt::WEnvironment& env)
    : WApplication(env)
  {
    messageResourceBundle().use("templates");

    new Wt::WLabel("Select your time zone: ", root());
    localeCombo_ = new Wt::WComboBox(root());

    info_ = new Wt::WTemplate(Wt::WString::tr("info"), root());

    Wt::WReadOnlyProxyModel *regions = new Wt::WReadOnlyProxyModel(this);
    regions->setSourceModel(&timeZones);
    localeCombo_->setModel(regions);

    localeCombo_->setCurrentIndex
      (timeZones.suggestedTimeZone(env.timeZoneOffset()));

    localeCombo_->changed().connect(this, &LocaleApplication::updateLocale);

    updateLocale();
  }

  void updateLocale()
  {
    std::string tz = boost::any_cast<std::string>
      (timeZones.index(localeCombo_->currentIndex(), 0)
       .data(TimeZoneModel::PosixTimeZoneRole));

    Wt::WLocale l = locale();
    l.setTimeZone(tz);
    setLocale(l);

    Wt::WString format = "yyyy-MM-dd HH:mm:ss Z";

    info_->bindString("time-zone", locale().timeZone());
    info_->bindString("utc-time",
		      Wt::WDateTime::currentDateTime().toString(format));
    info_->bindString("local-time",
		      Wt::WLocalDateTime::currentDateTime().toString(format));
  }

private:
  Wt::WComboBox *localeCombo_;
  Wt::WTemplate *info_;
};

Wt::WApplication *createApplication(const Wt::WEnvironment& env)
{
  return new LocaleApplication(env);
}

int main(int argc, char **argv)
{
  timeZones.load("date_time_zonespec.csv");

  return Wt::WRun(argc, argv, &createApplication);
}
