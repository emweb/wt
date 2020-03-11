// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2014 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#ifndef WT_DBO_JSON_H_
#define WT_DBO_JSON_H_

#include <ostream>
#include <sstream>
#include <type_traits>

#include <Wt/Dbo/ptr.h>
#include <Wt/Dbo/weak_ptr.h>
#include <Wt/Dbo/collection.h>
#include <Wt/Dbo/Field.h>
#include <Wt/Dbo/Session.h>

namespace Wt {
  namespace Dbo {
    class EscapeOStream;

/*! \class JsonSerializer Wt/Dbo/Json.h Wt/Dbo/Json.h
 *  \brief An action to serialize objects to JSON.
 *
 *  This class is an \p Action that serializes objects to an ostream.
 *  These objects must implement the \p persist() method. It also has
 *  support for serializing \link ptr ptrs\endlink to these objects,
 *  std::vectors of \link ptr ptrs\endlink, and
 *  \link collection collections\endlink of \link ptr ptrs\endlink.
 *
 *  It will follow one-to-one and \link Wt::Dbo::ManyToOne ManyToOne\endlink
 *  relations in one way: weak_ptr and \ref collection fields are followed
 *  and serialized, for \ref ptr fields only the \link Wt::Dbo::ptr::id() id\endlink
 *  is output.
 *
 *  No extraneous whitespace is output.
 *
 * \ingroup dbo
 */
class WTDBO_API JsonSerializer
{
public:
    /*! \brief Creates a JsonSerializer that writes to an std::ostream.
     *
     * Note that the std::ostream is not flushed to automatically. The
     * flush will happen automatically when this JsonSerializer is
     * destructed.
     */
    JsonSerializer(std::ostream& out);

    /*! \brief Destructor
     */
    virtual ~JsonSerializer();

    Session *session() { return session_; }

    template<typename T>
    typename std::enable_if< !std::is_enum<T>::value, void>::type
    act(FieldRef<T> field);

    template<typename T>
    typename std::enable_if< std::is_enum<T>::value, void>::type
    act(FieldRef<T> field) {
      writeFieldName(field.name());
      out(static_cast<int>(field.value()));
    }

    void act(FieldRef<std::string> field);
    void act(FieldRef<int> field);
    void act(FieldRef<long long> field);
    void act(FieldRef<bool> field);

    template<typename T>
    void actId(T& value, const std::string& name, int size) {
      field(*this, value, name, size);
    }

    template<typename T>
    void actId(ptr<T>& value, const std::string& name, int size, int fkConstraints) {
      field(*this, value, name, size);
    }

    template<typename T>
    void actPtr(const PtrRef<T>& field) {
      writeFieldName(field.name());
      if (field.value())
	outputId(field.id());
      else
	out("null");
    }

    template<typename T>
    void actWeakPtr(const WeakPtrRef<T>& field) {
      writeFieldName(session_->tableName<T>() + std::string("_") + field.joinName());
      ptr<T> v = field.value().query();
      if (v) {
	serialize(v);
      } else {
	out("null");
      }
    }

    template<typename T>
    void actCollection(const CollectionRef<T>& collec) {
      if (collec.type() == ManyToOne) {
	collection<ptr<T> > c = collec.value();
	writeFieldName(session_->tableName<T>() + std::string("s_")  + collec.joinName());
	out('[');
	bool first = true;
	for (typename collection<ptr<T> >::const_iterator i = c.begin(); i != c.end(); ++i) {
	  if (first)
	    first = false;
	  else
	    out(',');
	  serialize(*i);
	}
	out(']');
      }
    }

    bool getsValue() const {
      return true;
    }

    bool setsValue() const {
      return false;
    }

    bool isSchema() const {
      return false;
    }

    /*! \brief Serialize the given object.
     *
     * Serializes a plain object that implements the \p persist() method.
     */
    template<typename T>
    void serialize(const T& t) {
      session_ = NULL;
      out('{');
      const_cast<T&>(t).persist(*this);
      out('}');
    }

    /*! \brief Serialize the object that is pointed to by the given \ref ptr.
     *
     * This method does the same as the plain object serializer, but
     * also adds an extra \link Wt::Dbo::ptr::id() id\endlink field.
     */
    template<typename T>
    void serialize(const ptr<T>& t) {
      session_ = t.session();
      out('{');
      first_ = true;
      if (dbo_traits<T>::surrogateIdField()) {
	out('"');
	out(dbo_traits<T>::surrogateIdField());
	out("\":");
	outputId(t.id());
	first_ = false;
      }
      const_cast<T&>(*t).persist(*this);
      out('}');
    }

    /*! \brief Serialize an std::vector of \link ptr ptrs\endlink.
     *
     * Serializes each \ref ptr in the vector individually,
     * and puts it in an \p Array.
     */
    template<typename T>
    void serialize(const std::vector<ptr<T> >& v) {
      out('[');
      for (typename std::vector<ptr<T> >::const_iterator i = v.begin(); i != v.end(); ++i) {
	if (i != v.begin())
	  out(',');
	else
	  session_ = (*i).session();
	serialize(*i);
      }
      out(']');
    }

    /*! \brief Serialize a \ref collection of \link ptr ptrs\endlink.
     *
     * Serializes each \ref ptr in the \ref collection individually,
     * and puts it in an \p Array.
     *
     * The typical usage scenario of this method is to serialize
     * the results of a query to JSON.
     */
    template<typename T>
    void serialize(const collection<ptr<T> >& c) {
      session_ = c.session();
      out('[');
      bool first = true;
      for (typename collection<ptr<T> >::const_iterator i = c.begin(); i != c.end(); ++i) {
	if (first)
	  first = false;
	else
	  out(',');
	serialize(*i);
      }
      out(']');
      session_ = NULL;
    }

private:
    std::ostream &out_;
    EscapeOStream *escapeOut_, *stringLiteral_;
    bool first_;
    Session *session_;

    void out(char);
    void out(const char *);
    void out(int);
    void out(long long);

    template<typename T>
    void outputId(T id) {
      std::stringstream ss;
      ss << id;
      fastJsStringLiteral(ss.str());
    }
    void outputId(long long id) {
      out(id);
    }

    void writeFieldName(const std::string& fieldName);

    void fastJsStringLiteral(const std::string& s);
};

/*! \brief Serialize the given object to the given ostream.
 *
 * \sa JsonSerializer::serialize()
 */
template<typename C>
void jsonSerialize(const C& c, std::ostream& out) {
  JsonSerializer serializer(out);
  serializer.serialize(c);
}

/*! \brief Serialize the object pointed to by the given \ref ptr
 * to the given ostream.
 *
 * \sa JsonSerializer::serialize()
 */
template<typename C>
void jsonSerialize(const ptr<C>& c, std::ostream& out) {
  JsonSerializer serializer(out);
  serializer.serialize(c);
}

/*! \brief Serialize a vector of \link ptr ptrs\endlink to the given
 * ostream.
 *
 * \sa JsonSerializer::serialize()
 */
template<typename C>
void jsonSerialize(const std::vector<ptr<C> >& v, std::ostream& out) {
  JsonSerializer serializer(out);
  serializer.serialize(v);
}

/*! \brief Serialize a \ref collection of \link ptr ptrs\endlink to the given
 * ostream.
 *
 * \sa JsonSerializer::serialize()
 */
template<typename C>
void jsonSerialize(const collection<C>& c, std::ostream& out) {
  JsonSerializer serializer(out);
  serializer.serialize(c);
}

  }
}

#endif // WT_DBO_JSON_H_
