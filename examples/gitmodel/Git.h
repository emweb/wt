// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2008 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#ifndef GIT_H_
#define GIT_H_

#include <stdexcept>
#include <list>
#include <boost/array.hpp>

/**
 * @addtogroup gitmodelexample
 */
/*@{*/

/*! \brief %Git utility class for browsing git archives.
 *
 * Far from complete! Only browses git revisions.
 */
class Git {
public:
  /*! \brief %Exception class.
   */
  class Exception : public std::runtime_error {
  public:
    /*! \brief Constructor.
     */
    Exception(const std::string& msg);
  };

  /*! \brief %Git object Id.
   *
   * Class for compactly storing a 20-byte SHA1 digest.
   */
  class ObjectId : public boost::array<unsigned char, 20> {
  public:
    /*! \brief Default constructor.
     */
    ObjectId();

    /*! \brief Construct from a 40-digit hexadecimal number.
     *
     * \throws Exception : if the <i>id</i> does not represent a valid SHA1
     *         digest.
     */
    explicit ObjectId(const std::string& id);

    /*! \brief Print as a 40-digit hexadecimal number.
     */
    std::string toString() const;
  };

  /*! \brief %Git object type.
   */
  enum ObjectType { Tree, Commit, Blob };

  /*! \brief %Git object.
   */
  struct Object {
    ObjectId    id;
    ObjectType  type;
    std::string name;

    Object(const ObjectId& id, ObjectType type);
  };

  /*! \brief Constructor.
   */
  Git();

  /*! \brief Set the git repository path.
   *
   * \throws Exception : if the path does not specify a valid repository.
   */
  void setRepositoryPath(const std::string& repository);

  /*! \brief Get the tree for a particular revision
   *
   * \throws Exception : in case of a git error.
   */
  ObjectId getCommitTree(const std::string& revision) const;

  /*! \brief Get the commit for a particular revision
   *
   * \throws Exception : in case of a git error.
   */
  ObjectId getCommit(const std::string& revision) const;

  /*! \brief Get the tree for a particular commit
   *
   * \throws Exception : in case of a git error.
   */
  ObjectId getTreeFromCommit(const ObjectId& commit) const;

  /*! \brief Get some info on a tree object.
   *
   * The object is specified based on its index in the parent tree
   * object.
   *
   * \throws Exception : in case of a git error.
   */
  Object   treeGetObject(const ObjectId& tree, int index) const;

  /*! \brief Return the number of objects inside a tree object.
   *
   * \throws Exception : in case of a git error.
   */
  int      treeSize(const ObjectId& tree) const;

  /*! \brief Return the raw contents of a git object.
   *
   * \throws Exception : in case of a git error.
   */
  std::string catFile(const ObjectId& id) const;

  typedef std::list<std::pair<std::string, std::string> > Cache;

private:
  /*! \brief The path to the repository.
   */
  std::string repository_;

  /*! \brief A small LRU cache that stores results of git commands.
   */
  mutable Cache cache_;

  /*! \brief Checks the repository
   *
   * \throws Exception : in case the repository is not a valid.
   */
  void checkRepository() const;

  /*! \brief Returns a line identified by a tag from the output of a git
   *         command.
   *
   * The line is filled in <i>result</i>.
   * Returns whether a line starting with <i>tag</i> could be found.
   *
   * \throws Exception : in case the command failed
   */
  bool getCmdResult(const std::string& cmd, std::string& result,
		    const std::string& tag) const;

  /*! \brief Returns the <i>i</i>th line from the output of a git command.
   *
   * The line is filled in <i>result</i>.
   * Returns the whole git output if <i>index</i> = -1, otherwise the line
   * with line number <i>index</i>.
   *
   * \throws Exception : in case the command failed
   */
  bool getCmdResult(const std::string& cmd, std::string& result,
		    int index) const;

  /*! \brief Returns the number of lines in the output of a git command.
   *
   * \throws Exception : in case the command failed
   */
  int getCmdResultLineCount(const std::string& cmd) const;
};

/*@}*/

#endif // GIT_H_
